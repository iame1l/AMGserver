#include "stdafx.h"
#include "WebSocketAssitInterface.h"
#include "SHA1.h"
#include "Base64.h"

#define HNSWAP16(A) ((((A) & 0xff00) >> 8) | (((A) & 0x00ff) << 8))

#define HNSWAP32(A) ((HNSWAP16(((A) & 0xffff0000) >> 16)) | HNSWAP16(((A) & 0x0000ffff)) << 16)

#define HNSWAP64(A) (((long long)HNSWAP32(((A) & 0xffffffff00000000) >> 32)) | ((long long)HNSWAP32(((A) & 0x00000000ffffffff)) << 32))

// 判断主机是否为大端
static bool IsBigEnd()
{
    static bool s_IsBig = false;
    static int s_state = 0;
    if (s_state != 0) return s_IsBig;
    char sznum[4] = { 0 };
    int *num = (int*)sznum;
    *num = 1;
    s_state = 1;
    s_IsBig = sznum[0]!=0?1:0; // 本机返回1：为大端
    return s_IsBig;
}

namespace HN
{
    static void generateHash(char buffer[], size_t bufferlen)
    {
        int8_t bytes[16] = {0};
        for(int i = 0; i < 16; i++)
        {
            bytes[i] = rand() % 255;
        }
        base64Encode((const char*)bytes, 16, buffer);
    }

    std::string makeHandshakeRequest(const std::string &url)
    {
        std::string buffer;
        buffer.reserve(4096);
        buffer += std::string("GET ") +  url + " HTTP/1.1\r\n";
        buffer += "Upgrade: websocket\r\n";
        buffer += "Connection: Upgrade\r\n";

        char hash[45] = { 0 };
        generateHash(hash, 45);
        buffer += "Sec-WebSocket-Key: %s\r\n" + std::string(hash);
        buffer += "Connection: Upgrade\r\n";

        buffer += "Sec-WebSocket-Version: 13\r\n";

        buffer += "\r\n";
        return buffer;
    }

    std::string makeHandshakeResponse(const char* seckey)
    {
        std::string answer("");
        answer += "HTTP/1.1 101 Switching Protocols\r\n";
        answer += "Upgrade: WebSocket\r\n";
        answer += "Connection: Upgrade\r\n";
        answer += "Sec-WebSocket-Version: 13\r\n";
        if(seckey)
        {
            std::string key(seckey);
            key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
            char shakey[20] = {0};
            HN::SHA1 sha1;
            sha1.update(key);
            sha1.final(shakey);
            key = base64Encode(shakey, 20);
            answer += ("Sec-WebSocket-Accept: "+ key + "\r\n");
        }
        answer += "\r\n";
        return answer;
    }

    /*
           0                   1                   2                   3
           0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
           +-+-+-+-+-------+-+-------------+-------------------------------+
           |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
           |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
           |N|V|V|V|       |S|             |   (if payload len==126/127)   |
           | |1|2|3|       |K|             |                               |
           +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
           |     Extended payload length continued, if payload len == 127  |
           + - - - - - - - - - - - - - - - +-------------------------------+
           |                               |Masking-key, if MASK set to 1  |
           +-------------------------------+-------------------------------+
           | Masking-key (continued)       |          Payload Data         |
           +-------------------------------- - - - - - - - - - - - - - - - +
           :                     Payload Data continued ...                :
           + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
           |                     Payload Data continued ...                |
           +---------------------------------------------------------------+

            opcode:
                 *  %x0 denotes a continuation frame
                 *  %x1 denotes a text frame
                 *  %x2 denotes a binary frame
                 *  %x3-7 are reserved for further non-control frames
                 *  %x8 denotes a connection close
                 *  %x9 denotes a ping
                 *  %xA denotes a pong
                 *  %xB-F are reserved for further control frames

           Payload length:  7 bits, 7+16 bits, or 7+64 bits

           Masking-key:  0 or 4 bytes
    */
    WsFrameType decodeFrame(const char* inbuf, int inlength, int &consumelen, std::vector<char>* outbuf)
    {
        if(inlength < 2) return WS_INCOMPLETE_FRAME;

        const unsigned char* inp = (const unsigned char*)(inbuf);

        unsigned char msg_opcode = inp[0] & 0x0F;
        unsigned char msg_fin = (inp[0] >> 7) & 0x01;
        unsigned char msg_masked = (inp[1] >> 7) & 0x01;
        if (msg_masked == 1) return WS_ERROR_FRAME;

        unsigned long long payload_length = 0;
        int pos = 2;
        int length_field = inp[1] & (~0x80);
        unsigned int mask = 0;

        if(length_field <= 125)
        {
            payload_length = length_field;
        }
        else if(length_field == 126)
        { 
            if (inlength < pos + 2) return WS_INCOMPLETE_FRAME;
            payload_length = *(short *)(&inp[2]);
            if (IsBigEnd()) payload_length = HNSWAP16(payload_length);
            pos += 2;
        }
        else if(length_field == 127)
        {
            if (inlength < pos + 8) return WS_INCOMPLETE_FRAME;
            payload_length = *(long long *)(&inp[2]);
            if (IsBigEnd()) payload_length = HNSWAP64(payload_length);
            pos += 8;
        }

        if(inlength < payload_length+pos)
        {
            return WS_INCOMPLETE_FRAME;
        }
        if (outbuf->size() > 0) outbuf->pop_back();
        for (unsigned int i = 0; i < payload_length; ++i)
        {
            outbuf->push_back(*(char*)(inp + pos + i));
        }
        outbuf->push_back('\0');

        consumelen += pos + (unsigned int)payload_length;

        if (msg_opcode == 0x0)
        {
             WsFrameType type = (msg_fin) ? WS_TEXT_FRAME : inlength < payload_length + pos + 2 ? WS_INCOMPLETE_TEXT_FRAME :
                decodeFrame(inbuf + payload_length + pos, inlength - payload_length - pos, consumelen, outbuf);
             if (type == WS_INCOMPLETE_TEXT_FRAME) consumelen = 0;
             return type;
        }
        if (msg_opcode == 0x1)
        {
            WsFrameType type = (msg_fin) ? WS_TEXT_FRAME : inlength < payload_length + pos + 2 ? WS_INCOMPLETE_TEXT_FRAME :
                decodeFrame(inbuf + payload_length + pos, inlength - payload_length - pos, consumelen, outbuf);
            if (type == WS_INCOMPLETE_TEXT_FRAME) consumelen = 0;
            return type;
        }
        if (msg_opcode == 0x2)
        {
            WsFrameType type = (msg_fin) ? WS_BINARY_FRAME : inlength < payload_length + pos + 2 ? WS_INCOMPLETE_BINARY_FRAME :
                decodeFrame(inbuf + payload_length + pos, inlength - payload_length - pos, consumelen, outbuf);
            if (type == WS_INCOMPLETE_BINARY_FRAME) consumelen = 0;
            return type;
        }

        if(msg_opcode == 0x8) return WS_CLOSE_FRAME;
        if(msg_opcode == 0x9) return WS_PING_FRAME;
        if(msg_opcode == 0xA) return WS_PONG_FRAME;

        return WS_ERROR_FRAME;
    }

    WsFrameType decodeFrameMask(const char* inbuf, int inlength, int &consumelen, std::vector<char>* outbuf)
    {
        if (inlength < 2) return WS_INCOMPLETE_FRAME;

        const unsigned char* inp = (const unsigned char*)(inbuf);

        unsigned char msg_opcode = inp[0] & 0x0F;
        unsigned char msg_fin = (inp[0] >> 7) & 0x01;
        unsigned char msg_masked = (inp[1] >> 7) & 0x01;
        if (msg_masked == 0) return WS_ERROR_FRAME;

        unsigned long long payload_length = 0;
        int pos = 2;
        int length_field = inp[1] & (~0x80);
        unsigned int mask = 0;

        if (length_field <= 125)
        {
            payload_length = length_field;
        }
        else if (length_field == 126)
        {
            if (inlength < pos + 2) return WS_INCOMPLETE_FRAME;
            payload_length = *(short *)(&inp[2]);
            if (IsBigEnd()) payload_length = HNSWAP16(payload_length);
            pos += 2;
        }
        else if (length_field == 127)
        {
            if (inlength < pos + 8) return WS_INCOMPLETE_FRAME;
            payload_length = *(long long *)(&inp[2]);
            if (IsBigEnd()) payload_length = HNSWAP64(payload_length);
            pos += 8;
        }
        
        if (inlength < pos + 4) return WS_INCOMPLETE_FRAME;

        mask = *((unsigned int*)(inp + pos));
        pos += 4;

        if (inlength < payload_length + pos)
        {
            return WS_INCOMPLETE_FRAME;
        }
        if (outbuf->size() > 0) outbuf->pop_back();
        for (unsigned int i = 0; i < payload_length; ++i)
        {
            char c = *(char*)(inp + pos + i);
            c ^= ((char*)&mask)[i % 4];
            outbuf->push_back(c);
        }
        outbuf->push_back('\0');

        consumelen += pos + (unsigned int)payload_length;

        if (msg_opcode == 0x0)
        {
            WsFrameType type = (msg_fin) ? WS_TEXT_FRAME : inlength < payload_length + pos + 2 ? WS_INCOMPLETE_TEXT_FRAME :
                decodeFrameMask(inbuf + payload_length + pos, inlength - payload_length - pos, consumelen, outbuf);
            if (type == WS_INCOMPLETE_TEXT_FRAME) consumelen = 0;
            return type;
        }
        if (msg_opcode == 0x1)
        {
            WsFrameType type = (msg_fin) ? WS_TEXT_FRAME : inlength < payload_length + pos + 2 ? WS_INCOMPLETE_TEXT_FRAME :
                decodeFrameMask(inbuf + payload_length + pos, inlength - payload_length - pos, consumelen, outbuf);
            if (type == WS_INCOMPLETE_TEXT_FRAME) consumelen = 0;
            return type;
        }
        if (msg_opcode == 0x2)
        {
            WsFrameType type = (msg_fin) ? WS_BINARY_FRAME : inlength < payload_length + pos + 2 ? WS_INCOMPLETE_BINARY_FRAME :
                decodeFrameMask(inbuf + payload_length + pos, inlength - payload_length - pos, consumelen, outbuf);
            if (type == WS_INCOMPLETE_BINARY_FRAME) consumelen = 0;
            return type;
        }
        if (msg_opcode == 0x8) return WS_CLOSE_FRAME;
        if (msg_opcode == 0x9) return WS_PING_FRAME;
        if (msg_opcode == 0xA) return WS_PONG_FRAME;

        return WS_ERROR_FRAME;
    }

    int encodeFrame(WsFrameType frame_type, const char* msg, int msgsize, char* outbuf, int bufsize)
    {
        int iRealsize = 2 + (msgsize >= 126 ? 2 : 0) + (msgsize >= 65536 ? 6 : 0) + msgsize;
        if (bufsize < iRealsize) return 0;
        int pos = 0;
        outbuf[pos++] = 0x80 | (unsigned char)frame_type;
        if(msgsize <= 125)
        {
            outbuf[pos++] = msgsize;
        }
        else if(msgsize <= 0xFFFF)
        {
            outbuf[pos++] = 126;
            *(short *)(&outbuf[pos]) = msgsize;
            if (IsBigEnd()) *(short *)(&outbuf[pos]) = HNSWAP16(msgsize);
            pos += 2;
        }
        else
        {
            outbuf[pos++] = 127;
            *(long long *)(&outbuf[pos]) = msgsize;
            if (IsBigEnd()) *(long long *)(&outbuf[pos]) = HNSWAP64(msgsize);
            pos += 8;
        }
        if(msgsize > 0) memcpy((void*)(outbuf+pos), msg, msgsize);
        return iRealsize;
    }

    int encodeFrameMask(WsFrameType frame_type, const char* msg, int msgsize, char* outbuf, int outsize)
    {
        int iRealsize = 6 + (msgsize >= 126 ? 2 : 0) + (msgsize >= 65536 ? 6 : 0) + msgsize;
        if (outsize < iRealsize) return 0;

        const static char masking_key[4] = { 0x12, 0x34, 0x56, 0x78 };
        int pos = 0;
        outbuf[pos++] = 0x80 | frame_type;
        if (msgsize < 126)
        {
            outbuf[pos++] = msgsize & 0xff | 0x80;
        }
        else if (msgsize < 65536)
        {
            outbuf[pos++] = (char)(126 | 0x80);
            *(short *)(&outbuf[pos]) = msgsize;
            if (IsBigEnd()) *(short *)(&outbuf[pos]) = HNSWAP16(msgsize);
            pos += 2;
        }
        else
        {
            outbuf[pos++] = (char)(127 | 0x80);
            *(long long *)(&outbuf[pos]) = msgsize;
            if (IsBigEnd()) *(long long *)(&outbuf[pos]) = HNSWAP64(msgsize);
            pos += 8;
        }
        for (unsigned int i = 0; i < 4; ++i)
        {
            outbuf[pos++] = masking_key[i];
        }
        if (msgsize > 0) memcpy((void*)(outbuf + pos), msg, msgsize);
        char* c = outbuf + pos;
        for (int i = 0; i < msgsize; ++i)
        {
            c[i] ^= ((unsigned char*)(&masking_key))[i % 4];
        }
        return (unsigned int)iRealsize;
    } 
}  // namespace HN