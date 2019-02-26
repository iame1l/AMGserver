#ifndef HN_WEBSOCKETASSITINTERFACE_H
#define HN_WEBSOCKETASSITINTERFACE_H

#include "WebSocketDefine.h"

namespace HN
{
        /// 根据url生成握手请求
        std::string makeHandshakeRequest(const std::string &url);

        /// 设置返回给client的握手响应
        /// seckey : client的"Sec-WebSocket-Key"对应值
        std::string makeHandshakeResponse(const char* seckey);

        /// 将从client发过来的数据帧进行解码
        /// inbuf      : 接收到的client发送的数据
        /// insize     : 接收到的数据大小
        /// consumelen : 实际使用乐多少缓存
        /// outbuf     : 解码缓冲区
        /// return     : WsFrameType
        WsFrameType decodeFrame(const char* inbuf, int insize, int &consumelen, std::vector<char>* outbuf);

        /// 将从client发过来的数据帧进行掩码解码
        /// inbuf      : 接收到的client发送的数据
        /// insize     : 接收到的数据大小
        /// consumelen : 实际使用乐多少缓存
        /// outbuf     : 解码缓冲区
        /// return     : WsFrameType
        WsFrameType decodeFrameMask(const char* inbuf, int insize, int &consumelen, std::vector<char>* outbuf);

        /// 将发回给client的数据进行编码
        /// @param[in] msg        : 发回给client的数据
        /// @param[in] msglen     : 发回的数据大小
        /// @param[in] outbuf     : 编码缓冲区
        /// @param[in] outsize    : 编码缓冲区大小（建议outsize > insize + 10
        /// @return 返回0失败,其他实际编码后长度
        int encodeFrame(WsFrameType frame_type, const char* msg, int msgsize, char* outbuf, int outsize);

        /// 将发回给client的数据进行掩码编码
        /// @param[in] msg        : 发回给client的数据
        /// @param[in] msglen     : 发回的数据大小
        /// @param[in] outbuf     : 编码缓冲区
        /// @param[in] outsize    : 编码缓冲区大小（建议outsize > insize + 10
        /// @return 返回0失败,其他实际编码后长度
        int encodeFrameMask(WsFrameType frame_type, const char* msg, int msgsize, char* outbuf, int outsize);
}

#endif // HN_WEBSOCKETASSITINTERFACE_H
