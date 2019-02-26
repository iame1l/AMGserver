#include "stdafx.h"
#include "AFCWebSocket.h"
#include "AFCException.h"
#include "AFCFunction.h"
#include "HNRC4.h"
#include "MD5.h"
#include "fastlz.h"
#include "WebSocketAssitInterface.h"
#include <algorithm>
using namespace HN;

#define WEB_SOCKET_URL _T("/HNWEBSERVER")


//控制标识
#define SOCKET_SND				1								//SOCKET 发送
#define SOCKET_REV				2								//SOCKET 接收
#define SOCKET_CLO				3								//SOCKET 退出

//超时时间
#define	TIME_OUT				3000							

#define ID_SOCKET_WND			10								//SOCKET 窗口 ID
#define WM_SOCKET_MESSAGE		WM_USER+12						//SOCKET 消息

#define MAX_HANDSHAKE_SIZE 4096

/**************************************************************************************************************/

//构造函数
CTCPSocketForWeb::CTCPSocketForWeb(void)
{
    m_socketType = e_web_socket;
    m_bSending=false;
    m_uIndex=0;
    m_dwHandleID=0;
    m_lBeginTime=0;
    m_dwSendBuffLen=0;
    m_dwRecvBuffLen=0;
    m_hSocket=INVALID_SOCKET;
    m_pManage=NULL;
    memset(&m_SocketAddr,0,sizeof(m_SocketAddr));
    memset(&m_SendOverData,0,sizeof(m_SendOverData));
    memset(&m_RecvOverData,0,sizeof(m_RecvOverData));
    memset(m_szRecvBuf, 0, sizeof(m_szRecvBuf));
    memset(m_szSendBuf, 0, sizeof(m_szRecvBuf));
    m_SendOverData.uOperationType=SOCKET_SND;
    m_RecvOverData.uOperationType=SOCKET_REV;
    memset(m_szRecvMsg, 0, sizeof(m_szRecvMsg));
    m_szRecvMsgPos = 0;
    m_bHandShaked = false;
}

//析够函数
CTCPSocketForWeb::~CTCPSocketForWeb(void)
{
    CloseSocket();
}

//合并 SOCKET
bool CTCPSocketForWeb::Attach(SOCKET hSocket, sockaddr_in & Addr, DWORD dwHandleID)
{
    //锁定数据
    CSignedLockObject RecvLock(&m_csRecvLock,true);
    CSignedLockObject SendLock(&m_csSendLock,true);

    //效验数据 
    if (hSocket==INVALID_SOCKET) return false;

    //记录数据
    m_bSending=false;
    m_dwSendBuffLen=0;
    m_dwRecvBuffLen=0;
    m_hSocket=hSocket;
    m_SocketAddr=Addr;
    m_dwHandleID=dwHandleID;
    m_lBeginTime=(long int)time(NULL);
    m_SendOverData.uOperationType=SOCKET_SND;
    m_RecvOverData.uOperationType=SOCKET_REV;

    return true;
}

//关闭 SOCKET
bool CTCPSocketForWeb::CloseSocket()
{
    //锁定数据
    CSignedLockObject RecvLock(&m_csRecvLock,true);
    CSignedLockObject SendLock(&m_csSendLock,true);

    bool bClose=(m_hSocket!=INVALID_SOCKET);
    ::closesocket(m_hSocket);
    m_bSending=false;
    m_dwHandleID=0;
    m_lBeginTime=0;
    m_dwSendBuffLen=0;
    m_dwRecvBuffLen=0;
    m_hSocket=INVALID_SOCKET;
    m_socketType = e_web_socket;
    memset(&m_SocketAddr,0,sizeof(m_SocketAddr));
    memset(&m_SendOverData,0,sizeof(m_SendOverData));
    memset(&m_RecvOverData,0,sizeof(m_RecvOverData));
    m_SendOverData.uOperationType=SOCKET_SND;
    m_RecvOverData.uOperationType=SOCKET_REV;
    memset(m_szRecvBuf, 0, sizeof(m_szRecvBuf));
    memset(m_szSendBuf, 0, sizeof(m_szRecvBuf));
    memset(m_szRecvMsg, 0, sizeof(m_szRecvMsg));
    m_szRecvMsgPos = 0;
    m_bHandShaked = false;

    return bClose;
}

//发送数据函数
int CTCPSocketForWeb::SendData(const char * pData, UINT uBufLen)
{
    if(!m_bHandShaked) return 0;
    if(m_socketType == e_web_socket)
    {
        return SendDataForWeb(pData, uBufLen);
    }
    else
    {
        return SendDataForNormal(pData, uBufLen);
    }
}
//发送数据函数
int CTCPSocketForWeb::SendData(BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID)
{
    if(!m_bHandShaked) return 0;
    if(m_socketType == e_web_socket)
    {
        return SendDataForWeb(bMainID, bAssistantID, bHandleCode, dwHandleID);
    }
    else
    {
        return SendDataForNormal(bMainID, bAssistantID, bHandleCode, dwHandleID);
    }
}
//发送数据函数
int CTCPSocketForWeb::SendData(void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID)
{
    if(!m_bHandShaked) return 0;
    if(m_socketType == e_web_socket)
    {
        return SendDataForWeb(pData, uBufLen, bMainID, bAssistantID, bHandleCode, dwHandleID);
    }
    else
    {
        return SendDataForNormal(pData, uBufLen, bMainID, bAssistantID, bHandleCode, dwHandleID);
    }
}

//发送数据函数
int CTCPSocketForWeb::SendDataForNormal(BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID)
{
    //锁定数据
    CSignedLockObject SendLock(&m_csSendLock,true);

    //效验数据
    if (dwHandleID==0) dwHandleID=m_dwHandleID;
    if ((m_dwHandleID==0)||(m_hSocket==INVALID_SOCKET)||(dwHandleID!=m_dwHandleID)) return SOCKET_ERROR;
    if (sizeof(NetMessageHead)>(SED_SIZE-m_dwSendBuffLen)) return SOCKET_ERROR;

    //发送数据
    NetMessageHead * pNetHead=(NetMessageHead *)(m_szSendBuf+m_dwSendBuffLen);
    pNetHead->bReserve=0;
    pNetHead->bMainID=bMainID;
    pNetHead->bAssistantID=bAssistantID;
    pNetHead->bHandleCode=bHandleCode;
    pNetHead->uMessageSize=sizeof(NetMessageHead);
    m_dwSendBuffLen+=sizeof(NetMessageHead);
    return CheckSendData()?sizeof(NetMessageHead):0;
}

//发送数据函数
int CTCPSocketForWeb::SendDataForNormal(void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID)
{
    if (uBufLen<=MAX_SEND_SIZE)
    {
        //锁定数据
        UINT uSendSize=sizeof(NetMessageHead)+uBufLen;
        CSignedLockObject SendLock(&m_csSendLock,true);

        //效验数据
        if (dwHandleID==0) dwHandleID=m_dwHandleID;
        if ((m_dwHandleID==0)||(m_hSocket==INVALID_SOCKET)||(dwHandleID!=m_dwHandleID))
        {
            return SOCKET_ERROR;
        }
        if (uSendSize>(SED_SIZE-m_dwSendBuffLen))
        {
            return SOCKET_ERROR;
        }

        //发送数据
        NetMessageHead * pNetHead=(NetMessageHead *)(m_szSendBuf+m_dwSendBuffLen);
        pNetHead->uMessageSize = uSendSize;
        pNetHead->bMainID=bMainID;
        pNetHead->bAssistantID=bAssistantID;
        pNetHead->bHandleCode=bHandleCode;
        pNetHead->bReserve=0;

        int nNetMessageHeadLen = sizeof(NetMessageHead);
        unsigned int uSize = uBufLen;
        if (uSize > 0 && NULL != pData)
        {
            HNRC4 RC4;
            RC4.init(g_chSecretKey, g_StrSecretKey.GetLength());

            //某些情况传来的数据不能直接拿来加密
            //因此，需要维护一份本地数据
            char *pDataCopy = (char *) malloc (MAX_SEND_SIZE_OUT);
            if (!pDataCopy)
            {
                return false;
            }
            memset(pDataCopy, 0, MAX_SEND_SIZE_OUT);
            if (uSize > MIN_ZIP_SIZE)
            {
                pNetHead->uMessageSize = fastlz_compress(pData,uSize,pDataCopy);
                pNetHead->uMessageSize += sizeof(NetMessageHead);
                pNetHead->bDateRule = 1;
            }
            else
            {
                pNetHead->uMessageSize = uSendSize;
                pNetHead->bDateRule = 0;
                memcpy(pDataCopy, pData, uSize);
            }

            RC4.encrpyt((unsigned char*) pDataCopy, pNetHead->uMessageSize-NET_HEAD_SIZE);
            memcpy((unsigned char *)pNetHead + nNetMessageHeadLen, pDataCopy, pNetHead->uMessageSize-NET_HEAD_SIZE); 

            if (pDataCopy)
            {
                free(pDataCopy);
                pDataCopy = NULL;
            }
        }

        m_dwSendBuffLen+=pNetHead->uMessageSize;

        return CheckSendData()?uBufLen:0;

    }
    return 0;
}

//发送数据函数
int CTCPSocketForWeb::SendDataForNormal(const char * pData, UINT uBufLen)
{
    if (uBufLen<=MAX_SEND_SIZE)
    {
        //锁定数据
        CSignedLockObject SendLock(&m_csSendLock,true);

        //效验数据
        if (m_hSocket==INVALID_SOCKET)
        {
            return SOCKET_ERROR;
        }
        if (uBufLen>(SED_SIZE-m_dwSendBuffLen))
        {
            return SOCKET_ERROR;
        }

        //发送数据
        m_dwSendBuffLen+=uBufLen;

        CopyMemory(m_szSendBuf, pData, m_dwSendBuffLen);

        return CheckSendData()?uBufLen:0;
    }

    return 0;
}

//发送数据函数
int CTCPSocketForWeb::SendDataForWeb(BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID)
{
    //锁定数据
    CSignedLockObject SendLock(&m_csSendLock,true);

    //效验数据
    if (dwHandleID==0) dwHandleID=m_dwHandleID;
    if ((m_dwHandleID==0)||(m_hSocket==INVALID_SOCKET)||(dwHandleID!=m_dwHandleID)) return SOCKET_ERROR;

    NetMessageHead head;
    head.bReserve=0;
    head.bMainID=bMainID;
    head.bAssistantID=bAssistantID;
    head.bHandleCode=bHandleCode;
    head.uMessageSize=sizeof(NetMessageHead);

    int len = encodeFrame(WS_BINARY_FRAME, (char*)&head, sizeof(head), m_szSendBuf + m_dwSendBuffLen, sizeof(m_szSendBuf) - m_dwSendBuffLen);
    if(len <= 0) return SOCKET_ERROR;
    m_dwSendBuffLen += len;

    return CheckSendData()?sizeof(NetMessageHead):0;
}

//发送数据函数
int CTCPSocketForWeb::SendDataForWeb(void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID)
{
    if (uBufLen<=MAX_SEND_SIZE)
    {
        //锁定数据
        UINT uSendSize=sizeof(NetMessageHead)+uBufLen;
        CSignedLockObject SendLock(&m_csSendLock,true);

        //效验数据
        if (dwHandleID==0) dwHandleID=m_dwHandleID;
        if ((m_dwHandleID==0)||(m_hSocket==INVALID_SOCKET)||(dwHandleID!=m_dwHandleID))
        {
            return SOCKET_ERROR;
        }

        //发送数据
        NetMessageHead * pNetHead=(NetMessageHead *)new char[uSendSize];
        if (!pNetHead)
        {
            return false;
        }
        pNetHead->uMessageSize = uSendSize;
        pNetHead->bMainID=bMainID;
        pNetHead->bAssistantID=bAssistantID;
        pNetHead->bHandleCode=bHandleCode;
        pNetHead->bReserve=0;

        int nNetMessageHeadLen = sizeof(NetMessageHead);
        unsigned int uSize = uBufLen;
        if (uSize > 0 && NULL != pData)
        {
            /*HNRC4 RC4;
            RC4.init(g_chSecretKey, g_StrSecretKey.GetLength());*/

            //某些情况传来的数据不能直接拿来加密
            //因此，需要维护一份本地数据
            char *pDataCopy = (char *) malloc (MAX_SEND_SIZE_OUT);
            if (!pDataCopy)
            {
                delete [](char*)pNetHead;
                return false;
            }
            memset(pDataCopy, 0, MAX_SEND_SIZE_OUT);
            /*if (uSize > MIN_ZIP_SIZE)
            {
                pNetHead->uMessageSize = fastlz_compress(pData,uSize,pDataCopy);
                pNetHead->uMessageSize += sizeof(NetMessageHead);
                pNetHead->bDateRule = 1;
            }
            else*/
            {
                pNetHead->uMessageSize = uSendSize;
                pNetHead->bDateRule = 0;
                memcpy(pDataCopy, pData, uSize);
            }

            /*RC4.encrpyt((unsigned char*) pDataCopy, pNetHead->uMessageSize-NET_HEAD_SIZE);*/
            memcpy((unsigned char *)pNetHead + nNetMessageHeadLen, pDataCopy, pNetHead->uMessageSize-NET_HEAD_SIZE); 

            if (pDataCopy)
            {
                free(pDataCopy);
                pDataCopy = NULL;
            }
        }

        int len = encodeFrame(WS_BINARY_FRAME, (char*)pNetHead, uSendSize, m_szSendBuf + m_dwSendBuffLen, sizeof(m_szSendBuf) - m_dwSendBuffLen);
        if(len <= 0)
        {
            delete [](char*)pNetHead;
            return SOCKET_ERROR;
        }
        m_dwSendBuffLen += len;

        delete [](char*)pNetHead;

        return CheckSendData()?uBufLen:0;

    }
    return 0;
}

//发送数据函数
int CTCPSocketForWeb::SendDataForWeb(const char * pData, UINT uBufLen)
{
    if (uBufLen<=MAX_SEND_SIZE)
    {
        //锁定数据
        CSignedLockObject SendLock(&m_csSendLock,true);

        //效验数据
        if (m_hSocket==INVALID_SOCKET)
        {
            return SOCKET_ERROR;
        }

        int len = encodeFrame(WS_BINARY_FRAME, pData, uBufLen,  m_szSendBuf + m_dwSendBuffLen, sizeof(m_szSendBuf) - m_dwSendBuffLen);
        if(len <= 0) return SOCKET_ERROR;
        m_dwSendBuffLen += len;

        return CheckSendData()?uBufLen:0;
    }

    return 0;
}

//检测发送数据函数
bool CTCPSocketForWeb::CheckSendData()
{
    //锁定数据
    CSignedLockObject SendLock(&m_csSendLock,true);

    if ((m_bSending==false)&&(m_dwSendBuffLen>0))
    {
        DWORD dwThancferred=0;
        m_bSending=true;
        m_SendOverData.dwHandleID=m_dwHandleID;
        m_SendOverData.WSABuffer.buf=m_szSendBuf;
        m_SendOverData.WSABuffer.len=m_dwSendBuffLen;
        if ((::WSASend(m_hSocket,&m_SendOverData.WSABuffer,1,&dwThancferred,0,(LPWSAOVERLAPPED)&m_SendOverData,NULL)==SOCKET_ERROR)
            &&(::WSAGetLastError()!=WSA_IO_PENDING))
        {
            m_bSending=false;
            return false;
        }
    }
    return true;
}

//发送完成函数
bool CTCPSocketForWeb::OnSendCompleted(DWORD dwThancferred, DWORD dwHandleID)
{
    CSignedLockObject SendLock(&m_csSendLock,true);
    if ((dwHandleID==0)||(m_dwHandleID!=dwHandleID)) return true;

    //处理数据
    m_bSending=false;
    if ((dwThancferred>0)&&(m_dwSendBuffLen>=dwThancferred))
    {
        m_dwSendBuffLen-=dwThancferred;
        ::MoveMemory(m_szSendBuf,&m_szSendBuf[dwThancferred],m_dwSendBuffLen*sizeof(m_szSendBuf[0]));
        return CheckSendData();
    }

    return false;
}

//接收完成函数
bool CTCPSocketForWeb::OnRecvCompleted(DWORD dwHandleID)
{
    //锁定数据
    CSignedLockObject RecvLock(&m_csRecvLock,true);

    //效验数据
    if ((m_dwHandleID==0)||(m_dwHandleID!=dwHandleID))	return false;

    try
    {
        //接收数据
        int iRecvCode=::recv(m_hSocket,m_szRecvBuf+m_dwRecvBuffLen,RCV_SIZE-m_dwRecvBuffLen,0);
        TRACE(TEXT("RECEIVE DATA SIZE:%d\n"),iRecvCode);
        if (iRecvCode>0)
        {
            //处理数据
            m_dwRecvBuffLen+=iRecvCode;
            if(!m_bHandShaked)
            {
                if(!HandShake()) return false;
            }
            if(m_socketType == e_web_socket)
            {// 页游socket
                while(m_dwRecvBuffLen > 2)
                {
                    int consumelen = 0;
                    std::vector<char> outbuf;
                    auto type = decodeFrameMask(m_szRecvBuf, m_dwRecvBuffLen, consumelen, &outbuf);

                    if(consumelen > 0)
                    {
                        memmove(m_szRecvBuf, m_szRecvBuf + consumelen, m_dwRecvBuffLen - consumelen);
                        m_dwRecvBuffLen -= consumelen;
                    }

                    switch(type)
                    {
                    case WS_PONG_FRAME:
                    case WS_INCOMPLETE_BINARY_FRAME:
                    case WS_INCOMPLETE_TEXT_FRAME:
                    case WS_INCOMPLETE_FRAME:
                        return RecvData();
                    case WS_PING_FRAME:
                        RecvLock.UnLock();
                        {
                            CSignedLockObject SendLock(&m_csSendLock,true);
                            int nSize = encodeFrame(WS_PONG_FRAME, (char*)&outbuf[0], outbuf.size() - 1, m_szSendBuf + m_dwSendBuffLen, sizeof(m_szSendBuf) - m_dwSendBuffLen);
                            if (nSize <= 0) return false;
                            m_dwSendBuffLen += nSize;
                            CheckSendData();
                        }
                        RecvLock.Lock();
                        return RecvData();
                    case WS_TEXT_FRAME:
                    case WS_BINARY_FRAME:
                        if(m_szRecvMsgPos + outbuf.size() > sizeof(m_szRecvMsg)) throw ("非法数据包");
                        memcpy(m_szRecvMsg + m_szRecvMsgPos, &outbuf[0], outbuf.size() - 1);
                        m_szRecvMsgPos += outbuf.size() - 1;
                        break;
                    case WS_CLOSE_FRAME:
                        throw("关闭数据包");
                    default:
                        throw("非法数据包");
                    }

                    NetMessageHead * pNetHead=(NetMessageHead *)m_szRecvMsg;
                    if(pNetHead->bMainID==200)
                    {
                        m_pManage->CloseSocket(this,m_dwHandleID);
                        return false;
                    }

                    BYTE szNetBuffer[MAX_SEND_SIZE+NET_HEAD_SIZE] = {0};

                    int nNetMessageHeadLen = sizeof(NetMessageHead);
                    /*HNRC4 RC4;
                    RC4.init(g_chSecretKey, g_StrSecretKey.GetLength());*/
                    int iSize = 0;
                    while ((m_szRecvMsgPos>=sizeof(NetMessageHead))&&(m_szRecvMsgPos>=pNetHead->uMessageSize))
                    {
                        iSize = pNetHead->uMessageSize;
                        /*if (pNetHead->uMessageSize > nNetMessageHeadLen)
                        {
                            RC4.decrypt((unsigned char*) pNetHead + nNetMessageHeadLen, pNetHead->uMessageSize - nNetMessageHeadLen);
                        }*/

                        //效验数据
                        if (pNetHead->uMessageSize<sizeof(NetMessageHead)) throw ("非法数据包");

                        ///校验码
                        if (pNetHead->bReserve != (int)m_hSocket && (!((pNetHead->bMainID==MDM_CONNECT)&&(pNetHead->bAssistantID==ASS_NET_TEST)&&!m_bHandShaked)) ) throw("非法数据包");

                        //打包数据
                        UINT uMessageSize = 0;
                        //if (pNetHead->bDateRule & ZIP_FLAG)
                        //{//解压
                        //    memcpy_s(szNetBuffer,nNetMessageHeadLen,pNetHead,nNetMessageHeadLen);
                        //    uMessageSize = fastlz_decompress(pNetHead + 1, pNetHead->uMessageSize - nNetMessageHeadLen,szNetBuffer+nNetMessageHeadLen,MAX_SEND_SIZE);
                        //    uMessageSize += nNetMessageHeadLen;
                        //}
                        //else
                        {
                            uMessageSize = pNetHead->uMessageSize;
                            if (uMessageSize>sizeof(szNetBuffer)) throw TEXT("数据包过大");
                            ::CopyMemory(szNetBuffer,pNetHead,uMessageSize);
                        }

                        UINT uHandleSize=uMessageSize-sizeof(NetMessageHead);
                        if (uMessageSize>MAX_SEND_SIZE) throw TEXT("数据包太大");
                        NetMessageHead * pHandleNetData=(NetMessageHead *)szNetBuffer;
                        if (m_pManage->OnNetMessage(this,pHandleNetData,uHandleSize?pHandleNetData+1:NULL,uHandleSize,m_uIndex,m_dwHandleID)==false)
                            throw TEXT("数据处理失败");

                        //删除缓存数据
                        memmove(m_szRecvMsg,m_szRecvMsg+iSize,m_szRecvMsgPos-iSize);
                        m_szRecvMsgPos-=iSize;
                    }
                }
            }
            else
            {// 普通socket
                NetMessageHead * pNetHead=(NetMessageHead *)m_szRecvBuf;
                if(pNetHead->bMainID==200)
                {
                    m_pManage->CloseSocket(this,m_dwHandleID);
                    return false;
                }

                BYTE szNetBuffer[MAX_SEND_SIZE+NET_HEAD_SIZE] = {0};

                int nNetMessageHeadLen = sizeof(NetMessageHead);
                HNRC4 RC4;
                RC4.init(g_chSecretKey, g_StrSecretKey.GetLength());
                while ((m_dwRecvBuffLen>=sizeof(NetMessageHead))&&(m_dwRecvBuffLen>=pNetHead->uMessageSize))
                {
                    if (pNetHead->uMessageSize > nNetMessageHeadLen)
                    {
                        RC4.decrypt((unsigned char*) pNetHead + nNetMessageHeadLen, pNetHead->uMessageSize - nNetMessageHeadLen);
                    }

                    //效验数据
                    if (pNetHead->uMessageSize<sizeof(NetMessageHead)) throw ("非法数据包");

                    ///校验码
                    if (pNetHead->bReserve != (int)m_hSocket) throw("非法数据包");

                    //打包数据
                    UINT uMessageSize = 0;
                    if (pNetHead->bDateRule & ZIP_FLAG)
                    {//解压
                        memcpy_s(szNetBuffer,nNetMessageHeadLen,pNetHead,nNetMessageHeadLen);
                        uMessageSize = fastlz_decompress(pNetHead + 1, pNetHead->uMessageSize - nNetMessageHeadLen,szNetBuffer+nNetMessageHeadLen,MAX_SEND_SIZE);
                        uMessageSize += nNetMessageHeadLen;
                    }
                    else
                    {
                        uMessageSize = pNetHead->uMessageSize;
                        if (uMessageSize>sizeof(szNetBuffer)) throw TEXT("数据包过大");
                        ::CopyMemory(szNetBuffer,pNetHead,uMessageSize);
                    }

                    UINT uHandleSize=uMessageSize-sizeof(NetMessageHead);
                    if (uMessageSize>MAX_SEND_SIZE) throw TEXT("数据包太大");
                    NetMessageHead * pHandleNetData=(NetMessageHead *)szNetBuffer;
                    if (m_pManage->OnNetMessage(this,pHandleNetData,uHandleSize?pHandleNetData+1:NULL,uHandleSize,m_uIndex,m_dwHandleID)==false)
                        throw TEXT("数据处理失败");
                    //删除缓存数据
                    int iSize = pNetHead->uMessageSize;
                    ::MoveMemory(m_szRecvBuf,m_szRecvBuf+iSize,m_dwRecvBuffLen-iSize);
                    m_dwRecvBuffLen-=iSize;
                }
            }
            return RecvData();
        }
    }
    catch (...) 
    {
        TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
        DWORD dwErr=::GetLastError();
    }
    m_pManage->CloseSocket(this,m_dwHandleID);

    return false;
}

//投递接收 IO
bool CTCPSocketForWeb::RecvData()
{
    CSignedLockObject RecvLock(&m_csRecvLock,true);
    DWORD dwThancferred=0,dwFlags=0;
    m_RecvOverData.dwHandleID=m_dwHandleID;
    m_RecvOverData.WSABuffer.buf=m_szRecvBuf+m_dwRecvBuffLen;
    m_RecvOverData.WSABuffer.len=0;
    if ((::WSARecv(m_hSocket,&m_RecvOverData.WSABuffer,1,&dwThancferred,&dwFlags,(LPWSAOVERLAPPED)&m_RecvOverData,NULL))
        &&(::WSAGetLastError()!=WSA_IO_PENDING))
    {
        m_pManage->CloseSocket(this,m_dwHandleID);
        return false;
    }
    return true;
}

//判断是否连接
bool CTCPSocketForWeb::IsConnectID(DWORD dwHandleID)
{
    if ((dwHandleID!=m_dwHandleID)||(m_dwHandleID==0)||(m_hSocket==INVALID_SOCKET)) return false;
    return true;
}

static std::string getHandShakeKeyValue(std::string strHeader, const char*szkey)
{
    int keypos = strHeader.find(szkey, 0);
    if (keypos == std::string::npos) return false;
    keypos += strlen(szkey);
    keypos = strHeader.find(":", keypos);
    if (keypos == std::string::npos) return false;
    keypos += 1;
    const char*clf = "\r\n";
    int keyposend = strHeader.find(clf, keypos);
    if (keyposend == std::string::npos) return false;
    std::string key = strHeader.substr(keypos, keyposend - keypos);
    key.erase(0, key.find_first_not_of(" "));
    key.erase(key.find_last_not_of(" ") + 1);
    return key;
}

bool CTCPSocketForWeb::HandShake()
{
    if(m_bHandShaked) return true;

    std::string strHeader;
    bool bFilled = false;
    {
        CSignedLockObject lock(&m_csRecvLock, true);
        strHeader = m_szRecvBuf;
    }

    if(strHeader.length() >= 4 || strHeader.length() != m_dwRecvBuffLen)
    {
        int pos = strHeader.find_first_not_of(" ");
        std::string strMethod = strHeader.substr(pos, 3);
        if (strMethod != "GET")
        {// 普通socket
            if(m_dwRecvBuffLen >= sizeof(NetMessageHead))
            {
                NetMessageHead *p = (NetMessageHead *)m_szRecvBuf;
                if(p->bMainID == MDM_CONNECT && p->bAssistantID == ASS_NET_TEST)
                {
                    m_socketType = e_normal_socket;
                    m_bHandShaked = true;
                    p->bReserve = m_hSocket;
                    //发送连接成功消息
                    MSG_S_ConnectSuccess Message;
                    Message.bLessVer=MAKELONG(3,4);
                    Message.bReserve[0]=0;
                    Message.bMaxVer=MAKELONG(3,4);
                    Message.bReserve[1]=0;
                    //此处把hSocket进行加密，由客户端解密并在包头的bReserve字段中发过来校验。其中SECRET_KEY为密钥由客户自己管理在
                    Message.i64CheckCode = (int)m_hSocket * 23 + m_pManage->m_iSecretKey;

                    SendDataForNormal(&Message,sizeof(Message),MDM_CONNECT,ASS_CONNECT_SUCCESS, 0, m_dwHandleID);
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return true;
            }
        }
    }


    // 查找\r\n\r\n
    const char*dclf = "\r\n\r\n";
    int headlen = strHeader.find(dclf, 0);
    if (strHeader.size() >= MAX_HANDSHAKE_SIZE) bFilled = true;
    if (bFilled && headlen == std::string::npos) return false;
    if (headlen == std::string::npos) return true;
    {
        CSignedLockObject lock(&m_csRecvLock, true);
        memmove(m_szRecvBuf, m_szRecvBuf + headlen + strlen(dclf), m_dwRecvBuffLen - headlen - strlen(dclf));
        m_dwRecvBuffLen -= (headlen + strlen(dclf));
    }

    // url是否正确
    {
        int pos = strHeader.find_first_of("\r\n");
        if(pos == std::string::npos) return false;
        std::string urlline = strHeader.substr(0, pos);
        if(urlline.find(WEB_SOCKET_URL) == std::string::npos) return false;
    }

    // 获取版本
    {
        const char*wekey = "Sec-WebSocket-Version";
        std::string keyValue = getHandShakeKeyValue(strHeader, wekey);
        if (keyValue != "13") return false;
    }

    // 查找"Sec-WebSocket-Key"
    {
        const char*wekey = "Sec-WebSocket-Key";
        std::string keyValue = getHandShakeKeyValue(strHeader, wekey);
        std::string resHanddata = makeHandshakeResponse(keyValue.c_str());

        //锁定数据
        CSignedLockObject SendLock(&m_csSendLock,true);
        if(m_dwSendBuffLen + resHanddata.size() > sizeof(m_szSendBuf)) return false;
        //发送数据
        CopyMemory(m_szSendBuf + m_dwSendBuffLen, (void*)resHanddata.c_str(), resHanddata.size());
        m_dwSendBuffLen+=resHanddata.size();
        CheckSendData();
    }

    m_socketType = e_web_socket;
    m_bHandShaked = true;
    //发送连接成功消息
    MSG_S_ConnectSuccess Message;
    Message.bLessVer=MAKELONG(3,4);
    Message.bReserve[0]=0;
    Message.bMaxVer=MAKELONG(3,4);
    Message.bReserve[1]=0;
    //此处把hSocket进行加密，由客户端解密并在包头的bReserve字段中发过来校验。其中SECRET_KEY为密钥由客户自己管理在
    Message.i64CheckCode = (int)m_hSocket * 23 + m_pManage->m_iSecretKey;

    SendData(&Message,sizeof(Message),MDM_CONNECT,ASS_CONNECT_SUCCESS, 0, m_dwHandleID);
    return true;
}

//构造函数
CTCPSocketManageForWeb::CTCPSocketManageForWeb()
{
    m_uUnionLineCount=0;
	m_bRun=false;
	m_bInit=false;
	m_bCheckConnect=true;
	m_bMaxVer=0;
	m_bLessVer=0;
	m_uListenPort=0;
	m_uRSThreadCount=0;
	m_uBatchCerrenPos=0;
	m_uBatchLineCount=0;
	m_uCheckTime=15000;			//最开始是15000，后来有人改成5000经常掉线
	m_pKeep=NULL;
	m_hEventThread=NULL;
	m_hCompletionPortRS=NULL;
	m_hCompletionPortBatch=NULL;
    m_hCompletionPortUnion=NULL;
	m_hThreadKeep=NULL;
	m_hThreadAccept=NULL;
	ZeroMemory(m_SendLine,sizeof(m_SendLine));
	return;
}

//析构函数
CTCPSocketManageForWeb::~CTCPSocketManageForWeb()
{
	SafeDeleteArray(m_pKeep);
	return;
}

//初始化
bool CTCPSocketManageForWeb::Init(UINT uMaxCount, UINT uListenPort, BYTE bMaxVer, BYTE bLessVer, int iSecretKey, IServerSocketServiceForWeb * pService)
{
	//效验参数
	if ((this==NULL)||(m_bInit==true)||(m_bRun==true)) throw new CAFCException(TEXT("CTCPSocketManageForWeb::Init 状态效验失败"),0x409);
	if ((uMaxCount==0)||(uListenPort==0)) throw new CAFCException(TEXT("CTCPSocketManageForWeb::Init 输入参数效验失败"),0x40A);

	GeneratorSecretKey(); // 生成加密的密钥

	//设置参数
	m_bInit=true;
	m_pService=pService;
	m_bMaxVer=bMaxVer;
	m_bLessVer=bLessVer;
	m_uListenPort=uListenPort;
	m_iSecretKey = iSecretKey;
	::ZeroMemory(m_SendLine,sizeof(m_SendLine));

	//申请内存
	m_pKeep=new BYTE [uMaxCount];
	if (m_pKeep==NULL) throw new CAFCException(TEXT("CTCPSocketManageForWeb::Init 申请内存失败"),0x40B);
	::ZeroMemory(m_pKeep,sizeof(BYTE)*uMaxCount);

	//建立 SOCKET 数组
	if (m_SocketPtrArray.Init(uMaxCount,uMaxCount)==false) throw new CAFCException(TEXT("CTCPSocket 初始化失败"),0x40C);
	for (UINT i=0;i<uMaxCount;i++)
	{
		m_SocketPtrArray.GetArrayItem(i)->SetIndex(i);
		m_SocketPtrArray.GetArrayItem(i)->SetSocketManagePtr(this);
	}
	return true;
}

//取消初始化
bool CTCPSocketManageForWeb::UnInit()
{
	if (this==NULL) return false;

	//设置数据 
	m_bInit=false;
	m_pService=NULL;
	m_uBatchCerrenPos=0;
    m_uUnionLineCount=0;
	m_uBatchLineCount=0;
	m_uListenPort=0;
	ZeroMemory(m_SendLine,sizeof(m_SendLine));

	//清理内存
	m_SocketPtrArray.UnInit();
	SafeDeleteArray(m_pKeep);

	return true;
}

//开始服务
bool CTCPSocketManageForWeb::Start(UINT uAcceptThreadCount, UINT uSocketThradCount)
{
	if ((this==NULL)||(m_bInit==false)||(m_bRun==true)) throw new CAFCException(TEXT("CTCPSocketManageForWeb::Start 没有初始化参数"),0x40D);

	//获取系统信息
	SYSTEM_INFO SystemInfo;
	::GetSystemInfo(&SystemInfo);
	if (uSocketThradCount==0) uSocketThradCount=SystemInfo.dwNumberOfProcessors*2;
	
	//建立事件
	m_hEventThread=::CreateEvent(NULL,TRUE,false,NULL);
	if (m_hEventThread==NULL) throw new CAFCException(TEXT("CTCPSocketManageForWeb::Start m_hEventThread 建立失败"),0x40E);

	//建立完成端口
	m_hCompletionPortRS=::CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,SystemInfo.dwNumberOfProcessors);
	if (m_hCompletionPortRS==NULL) throw new CAFCException(TEXT("CTCPSocketManageForWeb::Start m_hCompletionPortRS 建立失败"),0x40F);

	//建立批量发送完成端口
	m_hCompletionPortBatch=::CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,SystemInfo.dwNumberOfProcessors);
	if (m_hCompletionPortBatch==NULL) throw new CAFCException(TEXT("CTCPSocketManageForWeb::Start m_hCompletionPortBatch 建立失败"),0x410);

    //建立组播发送完成端口
    m_hCompletionPortUnion=::CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,SystemInfo.dwNumberOfProcessors);
    if (m_hCompletionPortUnion==NULL) throw new CAFCException(TEXT("CTCPSocketManageForZ::Start m_hCompletionPortUnion 建立失败"),0x410);

	//建立变量
	UINT				uThreadID=0;
	HANDLE				hThreadHandle=NULL;
	ThreadStartStruct	ThreadStartData;
	ThreadStartData.pSocketManage=this;

	//建立读写线程
	ThreadStartData.hThreadEvent=m_hEventThread;
	ThreadStartData.hCompletionPort=m_hCompletionPortRS;
	for (UINT i=0;i<uSocketThradCount;i++)
	{
		hThreadHandle=(HANDLE)::_beginthreadex(NULL,0,ThreadRSSocket,&ThreadStartData,0,&uThreadID);
		if (hThreadHandle==NULL) throw new CAFCException(TEXT("CTCPSocketManageForWeb::Start ThreadStartData 线程建立失败"),0x411);
		m_uRSThreadCount++;
		::WaitForSingleObject(m_hEventThread,INFINITE);
		::ResetEvent(m_hEventThread);
		::CloseHandle(hThreadHandle);
	}

	//建立监听SOCKET
	if (!m_ListenSocket.Create())  throw new CAFCException(TEXT("CTCPSocketManageForWeb::Start 网络建立失败"),0x412);
	if (!m_ListenSocket.Bind(m_uListenPort))  throw new CAFCException(TEXT("CTCPSocketManageForWeb::Start 网络绑定失败"),0x413);
	if (!m_ListenSocket.Listen()) throw new CAFCException(TEXT("CTCPSocketManageForWeb::Start 网络监听失败"),0x414);

	//建立应答线程
	ThreadStartData.hCompletionPort=m_hCompletionPortRS;
	ThreadStartData.hThreadEvent=m_hEventThread;
	m_hThreadAccept=(HANDLE)::_beginthreadex(NULL,0,ThreadAccept,&ThreadStartData,0,&uThreadID);
	if (m_hThreadAccept==NULL) throw new CAFCException(TEXT("CTCPSocketManageForWeb::Start ThreadAccept 线程建立失败"),0x415);
	::WaitForSingleObject(m_hEventThread,INFINITE);
	::ResetEvent(m_hEventThread);

	//建立维护线程
	ThreadStartData.hCompletionPort=NULL;
	ThreadStartData.hThreadEvent=m_hEventThread;
	m_hThreadKeep=(HANDLE)::_beginthreadex(NULL,0,ThreadKeepActive,&ThreadStartData,0,&uThreadID);
	if (m_hThreadKeep==NULL) throw new CAFCException(TEXT("CTCPSocketManageForWeb::Start ThreadKeepActive 线程建立失败"),0x416);
	::WaitForSingleObject(m_hEventThread,INFINITE);
	::ResetEvent(m_hEventThread);

	//建立批量发送线程
	ThreadStartData.hCompletionPort=m_hCompletionPortBatch;
	ThreadStartData.hThreadEvent=m_hEventThread;
	hThreadHandle=(HANDLE)::_beginthreadex(NULL,0,ThreadBatchSend,&ThreadStartData,0,&uThreadID);
	if (hThreadHandle==NULL) throw new CAFCException(TEXT("CTCPSocketManageForWeb::Start ThreadBatchSend 线程建立失败"),0x417);
	::WaitForSingleObject(m_hEventThread,INFINITE);
	::ResetEvent(m_hEventThread);

    //建立组播发送线程
    ThreadStartData.hCompletionPort=m_hCompletionPortUnion;
    ThreadStartData.hThreadEvent=m_hEventThread;
    hThreadHandle=(HANDLE)::_beginthreadex(NULL,0,ThreadUnionSend,&ThreadStartData,0,&uThreadID);
    if (hThreadHandle==NULL) throw new CAFCException(TEXT("CTCPSocketManageForZ::Start ThreadUnionSend 线程建立失败"),0x417);
    ::WaitForSingleObject(m_hEventThread,INFINITE);
    ::ResetEvent(m_hEventThread);

	//关闭事件
	m_bRun=true;
	return true;
}

//停止服务
bool CTCPSocketManageForWeb::Stop()
{
	if (this==NULL) return false;

	//关闭 SOCKET
	m_bRun=false;
	m_ListenSocket.CloseSocket();

	//退出应答线程
	if (m_hThreadAccept!=NULL) 
	{
		DWORD dwCode=::WaitForSingleObject(m_hThreadAccept,TIME_OUT);
		if (dwCode==WAIT_TIMEOUT) ::TerminateThread(m_hThreadAccept,1);
		::CloseHandle(m_hThreadAccept);
		m_hThreadAccept=NULL;
	}

	//关闭完成端口
	if (m_hCompletionPortRS!=NULL)
	{
		for (UINT i=0;i<m_uRSThreadCount;i++)
		{
			::PostQueuedCompletionStatus(m_hCompletionPortRS,0,NULL,NULL);
			::WaitForSingleObject(m_hEventThread,TIME_OUT);
			::ResetEvent(m_hEventThread);
		}
		m_uRSThreadCount=0;
		::CloseHandle(m_hCompletionPortRS);
		m_hCompletionPortRS=NULL;
	}

	//关闭维护线程
	if (m_hThreadKeep!=NULL)
	{
		if (::WaitForSingleObject(m_hEventThread,TIME_OUT)==WAIT_TIMEOUT) ::TerminateThread(m_hThreadKeep,0);
		::CloseHandle(m_hThreadKeep);
		m_hThreadKeep=NULL;
	}

	//关闭批量发送线程
	if (m_hCompletionPortBatch!=NULL)
	{
		::PostQueuedCompletionStatus(m_hCompletionPortBatch,0,NULL,NULL);
		::WaitForSingleObject(m_hEventThread,TIME_OUT);
		::ResetEvent(m_hEventThread);
		::CloseHandle(m_hCompletionPortBatch);
		m_hCompletionPortBatch=NULL;
	}

    //关闭组播发送线程
    if (m_hCompletionPortUnion!=NULL)
    {
        ::PostQueuedCompletionStatus(m_hCompletionPortUnion,0,NULL,NULL);
        ::WaitForSingleObject(m_hEventThread,TIME_OUT);
        ::ResetEvent(m_hEventThread);
        ::CloseHandle(m_hCompletionPortUnion);
        m_hCompletionPortUnion=NULL;
    }

	//关闭 SOCKET
	for (UINT i=0;i<m_SocketPtrArray.GetPermitCount();i++) 
	{
		m_SocketPtrArray.GetArrayItem(i)->CloseSocket();
	}

	//关闭事件
	if (m_hEventThread!=NULL)
	{
		::CloseHandle(m_hEventThread);
		m_hEventThread=NULL;
	}

	return true;
}

//发送数据函数
int CTCPSocketManageForWeb::SendData(UINT uIndex,const char * pData, UINT uBufLen)
{
	if ((uIndex<m_SocketPtrArray.GetPermitCount())&&(m_bRun==true))
	{
		if (nullptr != m_SocketPtrArray.GetArrayItem(uIndex))
		{
			return m_SocketPtrArray.GetArrayItem(uIndex)->SendData(pData,uBufLen);
		}
	}
	return 0;
}

//发送数据函数
int CTCPSocketManageForWeb::SendData(UINT uIndex, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID)
{
	if ((uIndex<m_SocketPtrArray.GetPermitCount())&&(m_bRun==true))
	{
		return m_SocketPtrArray.GetArrayItem(uIndex)->SendData(bMainID,bAssistantID,bHandleCode,dwHandleID);
	}
	return 0;
}

//发送数据函数
int CTCPSocketManageForWeb::SendData(UINT uIndex, void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID)
{
	if ((uIndex<m_SocketPtrArray.GetPermitCount())&&(m_bRun==true))
	{
		return m_SocketPtrArray.GetArrayItem(uIndex)->SendData(pData,uBufLen,bMainID,bAssistantID,bHandleCode,dwHandleID);
	}
	return 0;
}

//发送数据函数
int CTCPSocketManageForWeb::SendData(CTCPSocket * pSocket, void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID)
{
	if ((pSocket!=NULL)&&(m_bRun==true)) 
	{
		return pSocket->SendData(pData,uBufLen,bMainID,bAssistantID,bHandleCode,dwHandleID);
	}
	return 0;
}

//批量发送函数
int CTCPSocketManageForWeb::SendDataBatch(void * pData, UINT uSize, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode)
{
	//效验数据
	if ((m_bRun==false)||(uSize>(sizeof(m_SendLine[0].szSendData)-sizeof(NetMessageHead)))) return 0;

	CSignedLockObject BatchLock(&m_csBacthSend,true);
	if (m_uBatchLineCount>=uBatchLineSize) return 0;

	//写入缓冲
	UINT m_uWritePos=(m_uBatchCerrenPos+m_uBatchLineCount)%uBatchLineSize;
	m_SendLine[m_uWritePos].NetHead.bMainID=bMainID;
	m_SendLine[m_uWritePos].NetHead.bAssistantID=bAssistantID;
	m_SendLine[m_uWritePos].NetHead.bHandleCode=bHandleCode;
	m_SendLine[m_uWritePos].NetHead.uMessageSize=uSize+sizeof(NetMessageHead);
	m_SendLine[m_uWritePos].NetHead.bReserve=0;
	if (uSize>0) CopyMemory(m_SendLine[m_uWritePos].szSendData,pData,uSize);

	//激发发送线程
	m_uBatchLineCount++;
	PostQueuedCompletionStatus(m_hCompletionPortBatch,uSize+sizeof(NetMessageHead),NULL,NULL);
	
	return uSize;
}

//关闭连接
bool CTCPSocketManageForWeb::CloseSocket(UINT uIndex, DWORD dwHandleID)
{ 
	if (uIndex<m_SocketPtrArray.GetPermitCount()) 
	{
		return CloseSocket(m_SocketPtrArray.GetArrayItem(uIndex),dwHandleID);
	}
	return false;
}

//关闭连接
bool CTCPSocketManageForWeb::CloseSocket(CTCPSocketForWeb * pSocket, DWORD dwHandleID) 
{ 
	if (pSocket!=NULL)
	{
		CSignedLockObject RecvLock(pSocket->GetRecvLock(),true);
		CSignedLockObject SendLock(pSocket->GetSendLock(),true);

		//效验数据
		if (dwHandleID==0) dwHandleID=pSocket->m_dwHandleID;
		if ((pSocket->m_dwHandleID==0)||(dwHandleID!=pSocket->m_dwHandleID)) return false;

		//设置数据
		ULONG dwAccessIP=pSocket->GetConnectData();
		long int lConnectTime=(long int)time(NULL)-pSocket->GetBeginTime();
		*(m_pKeep+pSocket->m_uIndex)=0;

		//关闭处理
		if ((pSocket->CloseSocket()==true)&&(m_pService!=NULL))	m_pService->OnSocketCloseEvent(dwAccessIP,pSocket->m_uIndex,lConnectTime);
		
		//清理信息
		TCPSocketDelete(pSocket);

		return true;
	}
	return false;
}

//判断是否连接
bool CTCPSocketManageForWeb::IsConnectID(UINT uIndex, DWORD dwHandleID)
{
	if (uIndex>=m_SocketPtrArray.GetPermitCount()) return false;
	return m_SocketPtrArray.GetArrayItem(uIndex)->IsConnectID(dwHandleID);
}



//消息处理函数
bool CTCPSocketManageForWeb::OnNetMessage(CTCPSocketForWeb * pSocket, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uIndex, DWORD dwHandleID)
{
	*(m_pKeep+uIndex)=0;
	//效验网络测试数据
	if ((pNetHead->bMainID==MDM_CONNECT)&&(pNetHead->bAssistantID==ASS_NET_TEST)) 
	{
		//*(m_pKeep+uIndex)=0;  以前这句是放这里的,但是应该是收到任何消息都应该把心跳清0
		return true;
	}
	if (m_pService!=NULL) return m_pService->OnSocketReadEvent(pSocket,pNetHead,pData,uSize,uIndex,dwHandleID);
	return false;
}


//SOCKET 数据接收线程
unsigned __stdcall CTCPSocketManageForWeb::ThreadRSSocket(LPVOID pThreadData)
{
	//数据定义
	ThreadStartStruct	* pStartData=(ThreadStartStruct *)pThreadData;			//线程启动数据指针
	CTCPSocketManageForWeb	* pSocketManage=pStartData->pSocketManage;				//管理指针
	HANDLE				hCompletionPort=pStartData->hCompletionPort;			//完成端口
	HANDLE				hEvent=pStartData->hThreadEvent;						//事件

	//线程数据读取完成
	::SetEvent(hEvent);

	//重叠数据
	DWORD							dwThancferred=0;							//接收数量
	CTCPSocketForWeb						* pTCPSocket=NULL;							//完成键
	CTCPSocketForWeb::OverLappedStruct	* pOverData=NULL;							//重叠数据

	while (1)
	{
		try
		{
			//等待完成端口
			dwThancferred=0;
			pTCPSocket=NULL;
			pOverData=NULL;
			::GetQueuedCompletionStatus(hCompletionPort,&dwThancferred,(PULONG_PTR)&pTCPSocket,(LPOVERLAPPED *)&pOverData,INFINITE);
			if ((pTCPSocket==NULL)||(pOverData==NULL))
			{
				if ((pTCPSocket==NULL)&&(pOverData==NULL)) ::SetEvent(hEvent);
				_endthreadex(0);
			}

			//处理完成端口
			if ((dwThancferred==0)&&(pOverData->uOperationType==SOCKET_SND))
			{
				//SOCKET 关闭
				pSocketManage->CloseSocket(pTCPSocket,pOverData->dwHandleID);
				continue;
			}

			switch (pOverData->uOperationType)
			{
			case SOCKET_REV:	//SOCKET 数据读取
				{
					pTCPSocket->OnRecvCompleted(pOverData->dwHandleID);
					break;
				}
			case SOCKET_SND:	//SOCKET 数据发送
				{
					pTCPSocket->OnSendCompleted(dwThancferred,pOverData->dwHandleID);
					break;
				}
			}
		}
		catch (...) 
		{
			TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		}
	}

	return 0;
}

//SOCKET 批量发送数据线程
unsigned __stdcall CTCPSocketManageForWeb::ThreadBatchSend(LPVOID pThreadData)
{
	//数据定义
	ThreadStartStruct		* pStartData=(ThreadStartStruct *)pThreadData;					//线程启动数据指针
	CTCPSocketManageForWeb	* pSocketManage=pStartData->pSocketManage;						//管理指针
	UINT					uMaxCount=pSocketManage->m_SocketPtrArray.GetPermitCount();		//最大数目
	HANDLE					hCompletionPort=pStartData->hCompletionPort;					//完成端口
	HANDLE					hEvent=pStartData->hThreadEvent;								//事件
	
	//启动完成
	SetEvent(pStartData->hThreadEvent);


	//临时变量
	NetMessageHead			* pNetHead=NULL;												//数据包头
	CTCPSocketForWeb		* pSendSocket=pSocketManage->m_SocketPtrArray.GetArrayItem(0);	//发送 SOCKET
	UINT					* puSendCount=&pSocketManage->m_uBatchLineCount;				//当前数量

	DWORD					dwThancferred=0;												//接收数量
	ULONG					dwCompleteKey=0L;												//重叠 IO 临时数据
	LPOVERLAPPED			OverData;														//重叠 IO 临时数据

	while (1)
	{
		try
		{
			//提取发送数据
			dwThancferred=0;
			GetQueuedCompletionStatus(hCompletionPort,&dwThancferred,&dwCompleteKey,(LPOVERLAPPED *)&OverData,INFINITE);
			if (dwThancferred==0)
			{
				SetEvent(hEvent);
				_endthreadex(0);
			}

			//发送数据
			BatchSendLineStruct * pBatchData=&pSocketManage->m_SendLine[pSocketManage->m_uBatchCerrenPos];
			for (UINT i=0;i<uMaxCount;i++) 
			{
				if ((pSendSocket+i)->IsConnect())
				{
					(pSendSocket+i)->SendData(pBatchData->szSendData,pBatchData->NetHead.uMessageSize-sizeof(NetMessageHead),
						pBatchData->NetHead.bMainID,pBatchData->NetHead.bAssistantID,pBatchData->NetHead.bHandleCode,0);
				}
			}

			//处理数据
			{
				CSignedLockObject BatchLock(&pSocketManage->m_csBacthSend,true);
				pSocketManage->m_uBatchCerrenPos=(pSocketManage->m_uBatchCerrenPos+1)%uBatchLineSize;
				pSocketManage->m_uBatchLineCount--;
			}
		}
		catch (...) {TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);}
	}

	return 0;
}

//SOCKET 维护线程
unsigned __stdcall CTCPSocketManageForWeb::ThreadKeepActive(LPVOID pThreadData)
{
	//数据定义
	ThreadStartStruct		* pStartData=(ThreadStartStruct *)pThreadData;		//启动指针
	CTCPSocketManageForWeb		* pSocketManage=pStartData->pSocketManage;			//管理指针
	HANDLE					m_hThreadEvent=pStartData->hThreadEvent;			//线程事件
	
	//启动完成
	::SetEvent(m_hThreadEvent);

	//数据定义
	UINT uCheckCount=0;
	::Sleep(pSocketManage->m_uCheckTime);

	while (1)
	{
		//发送测试数据
		pSocketManage->SendDataBatch(NULL,0,MDM_CONNECT,ASS_NET_TEST,0);

		//检查退出标志
		uCheckCount=0;
		while (uCheckCount<pSocketManage->m_uCheckTime)
		{
			if (!pSocketManage->m_bRun) 
			{
				SetEvent(m_hThreadEvent);
				_endthreadex(0);
			}
			::Sleep(1000);
			uCheckCount+=1000;
		}

		//效验连接数据
		if ((pSocketManage->m_bCheckConnect)&&(pSocketManage->m_bRun)&&(pSocketManage->m_SocketPtrArray.GetArrayItem(0)!=NULL))
		{
			for (int i=(pSocketManage->m_SocketPtrArray.GetPermitCount()-1);i>=0;i--)
			{
				if (pSocketManage->m_SocketPtrArray.GetArrayItem(i)->IsConnect())
				{
					if ((*(pSocketManage->m_pKeep+i))>=3) //如果超出3次计数，则断开该客户连接
						pSocketManage->CloseSocket(i,0);
					else 
						(*(pSocketManage->m_pKeep+i))++;//小于3次读数，则给读数器+1
				}
			}
		}
	}

	return 0;
}

//获取时间函数
unsigned __int64 CTCPSocketManageForWeb::AFCGetTimer()
{
	_asm _emit 0x0F;
	_asm _emit 0x31;
}

//获取处理 ID 号码
DWORD CTCPSocketManageForWeb::GetHandleID()
{
	static long int dwBaseID=1L;
	if (dwBaseID>3000000L) dwBaseID=1L;
	return ::InterlockedIncrement(&dwBaseID);
}

//获取本机 IP 地址
bool CTCPSocketManageForWeb::GetLoaclIPAddr(TCHAR szAddrBuffer[], UINT uStation)
{
	if (szAddrBuffer==NULL) return false;

	TCHAR szHostName[51];
	::gethostname(szHostName,sizeof(szHostName));
	HOSTENT * pHostInfo=::gethostbyname(szHostName);
	if (pHostInfo!=NULL)
	{
		LPIN_ADDR * pAddrList=(LPIN_ADDR *)(pHostInfo->h_addr_list); 
		LPIN_ADDR In_Addr=*pAddrList;

		UINT uCount=0;
		szAddrBuffer[0]=0;
		while (In_Addr!=NULL)
		{
			if (uCount==uStation)
			{
				lstrcpy(szAddrBuffer,inet_ntoa(*In_Addr));
				return true;
			}
			uCount++;
			pAddrList++;
			In_Addr=*pAddrList;
		}
	}
	return false;
}

//数值变 IP 字符
TCHAR * CTCPSocketManageForWeb::ULongToIP(ULONG dwIP, TCHAR * szIP)
{
	wsprintf(szIP,TEXT("%d.%d.%d.%d"),FOURTH_IPADDRESS(dwIP),THIRD_IPADDRESS(dwIP),SECOND_IPADDRESS(dwIP),FIRST_IPADDRESS(dwIP));
	return szIP;
}

ULONG CTCPSocketManageForWeb::IPToULong(TCHAR * szIP)
{
    char* p;  
    int sections[4]={0};  
    int i=0;  

    p = strtok(szIP,".");  
    while( p )  
    {  
        sections[i] = atoi(p);  
        p = strtok(NULL,".");  
        i++;  
    }  

    unsigned int num =0;  
    for( int j=3,i=0 ; j>=0 ; j--,i++ )  
    {  
        num += (sections[i] <<(8*j));  
    }  

    return num;  
}

//获取指定index的连接信息
string CTCPSocketManageForWeb::GetServerInfo(UINT uIndex)
{
	string s = "";
	for (UINT i=0;i<m_SocketPtrArray.GetPermitCount();i++) 
	{
		if(m_SocketPtrArray.GetArrayItem(i)->m_uIndex == uIndex)
		{
			//memcpy(pSocket, &(m_SocketPtrArray.GetArrayItem(i)->m_SocketAddr), sizeof(sockaddr_in));
			s = inet_ntoa(m_SocketPtrArray.GetArrayItem(i)->m_SocketAddr.sin_addr);
			return s;
		}
	}
	return s;
}

bool CTCPSocketManageForWeb::TCPSocketDelete(CTCPSocketForWeb * pSocket)
{
    if (pSocket!=NULL)
    {
        CSignedLockObject LockObject(m_SocketPtrArray.GetArrayLock(),true);
        pSocket->CloseSocket();
        return m_SocketPtrArray.FreeItem(pSocket);
    }
    return false;
}

// 组播发送函数
int CTCPSocketManageForWeb::SendDataUnion(UINT uUnionID, void * pData, UINT uSize, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode)
{
    // 效验数据
    if (!m_bRun || (uSize > (sizeof(m_UnionSendLine[0].szSendData) - sizeof(NetMessageHead) - sizeof(int))))
    {
        return 0;
    }

    CSignedLockObject UnionLock(&m_csUnionSend,true);
    if (m_uUnionLineCount >= uBatchLineSize)
    {
        return 0;
    }

    // 写入缓冲
    UINT m_uWritePos = (m_uUnionCerrenPos+m_uUnionLineCount)%uBatchLineSize;
    m_UnionSendLine[m_uWritePos].NetHead.bMainID=bMainID;
    m_UnionSendLine[m_uWritePos].NetHead.bAssistantID=bAssistantID;
    m_UnionSendLine[m_uWritePos].NetHead.bHandleCode=bHandleCode;
    m_UnionSendLine[m_uWritePos].NetHead.uMessageSize=uSize+sizeof(NetMessageHead);
    m_UnionSendLine[m_uWritePos].NetHead.bReserve = 0;
    m_UnionSendLine[m_uWritePos].iUnionID = uUnionID;
    if (uSize > 0)
    {
        CopyMemory(m_UnionSendLine[m_uWritePos].szSendData,pData,uSize);
    }

    // 激发发送线程
    m_uUnionLineCount++;
    PostQueuedCompletionStatus(m_hCompletionPortUnion,uSize+sizeof(NetMessageHead)+sizeof(int),NULL,NULL);

    return uSize;
}


//SOCKET 连接应答线程
unsigned __stdcall CTCPSocketManageForWeb::ThreadAccept(LPVOID pThreadData)
{
    //数据定义
    ThreadStartStruct	* pStartData=(ThreadStartStruct *)pThreadData;			//线程启动数据指针
    CTCPSocketManageForWeb	* pSocketManage=pStartData->pSocketManage;				//管理指针
    HANDLE				hCompletionPort=pStartData->hCompletionPort;			//完成端口

    sockaddr_in			SocketAddr;
    int					iAddrBuferLen=sizeof(SocketAddr);
    SOCKET				hSocket=INVALID_SOCKET;
    CTCPSocketForWeb	* pNewTCPSocket=NULL;
    HANDLE				hAcceptCompletePort=NULL;
    SOCKET				hListenScoket=pStartData->pSocketManage->m_ListenSocket.GetSafeSocket();


    //线程数据读取完成
    SetEvent(pStartData->hThreadEvent);

    while (1)
    {
        try
        {

            //接收连接
            hSocket=::WSAAccept(hListenScoket,(sockaddr *)&SocketAddr,&iAddrBuferLen,NULL,NULL);
            if (hSocket==INVALID_SOCKET) 
            {
                int iErrorCode=WSAGetLastError();
                _endthreadex(0);
            }

            //处理连接
            pNewTCPSocket=pSocketManage->TCPSocketNew();
            if ((pNewTCPSocket!=NULL)&&(pNewTCPSocket->Attach(hSocket,SocketAddr,GetHandleID())))
            {
                hAcceptCompletePort=::CreateIoCompletionPort((HANDLE)hSocket,hCompletionPort,(ULONG_PTR)pNewTCPSocket,0);
                if ((hAcceptCompletePort==NULL)||(!pNewTCPSocket->RecvData())) throw TEXT("Accept False");

            }
            else
            {
                throw TEXT("Accept False");
            }
        }
        catch (...)
        {TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
        ::closesocket(hSocket);
        if (pNewTCPSocket!=NULL) pSocketManage->TCPSocketDelete(pNewTCPSocket);
        }
    }

    return 0;
}

CTCPSocketForWeb * CTCPSocketManageForWeb::TCPSocketNew()
{
    ////if(m_SocketPtrArray.GetActiveCount() > MaxConnectCount)return NULL;
    if (m_bRun)
    {
        CSignedLockObject LockObject(m_SocketPtrArray.GetArrayLock(),true);
        return m_SocketPtrArray.NewItem();
    }
    return NULL;
}

//SOCKET 组播发送数据线程
unsigned __stdcall CTCPSocketManageForWeb::ThreadUnionSend(LPVOID pThreadData)
{
    //数据定义
    ThreadStartStruct		* pStartData=(ThreadStartStruct *)pThreadData;					//线程启动数据指针
    CTCPSocketManageForWeb	* pSocketManage=pStartData->pSocketManage;						//管理指针
    UINT					uMaxCount=pSocketManage->m_SocketPtrArray.GetPermitCount();		//最大数目
    HANDLE					hCompletionPort=pStartData->hCompletionPort;					//完成端口
    HANDLE					hEvent=pStartData->hThreadEvent;								//事件

    //启动完成
    SetEvent(pStartData->hThreadEvent);


    //临时变量
    NetMessageHead			* pNetHead=NULL;												//数据包头
    CTCPSocketForWeb		* pSendSocket=pSocketManage->m_SocketPtrArray.GetArrayItem(0);	//发送 SOCKET
    UINT					* puSendCount=&pSocketManage->m_uUnionLineCount;				//当前数量

    DWORD					dwThancferred=0;												//接收数量
    ULONG					dwCompleteKey=0L;												//重叠 IO 临时数据
    LPOVERLAPPED			OverData;														//重叠 IO 临时数据

    while (1)
    {
        try
        {
            //提取发送数据
            dwThancferred=0;
            GetQueuedCompletionStatus(hCompletionPort,&dwThancferred,&dwCompleteKey,(LPOVERLAPPED *)&OverData,INFINITE);
            if (dwThancferred==0)
            {
                SetEvent(hEvent);
                _endthreadex(0);
            }

            //发送数据
            UnionSendLineStruct * pUnionData=&pSocketManage->m_UnionSendLine[pSocketManage->m_uUnionCerrenPos];
            map<int,vector<int>>::iterator l_it;
            l_it = pSocketManage->m_UnionData.find(pUnionData->iUnionID);
            if (l_it != pSocketManage->m_UnionData.end())
            {
                for (vector<int>::iterator temp = l_it->second.begin();temp != l_it->second.end();temp++)
                {
                    if ((pSendSocket+*temp)->IsConnect())
                    {
                        (pSendSocket+*temp)->SendData(pUnionData->szSendData,pUnionData->NetHead.uMessageSize-sizeof(NetMessageHead),
                            pUnionData->NetHead.bMainID,pUnionData->NetHead.bAssistantID,pUnionData->NetHead.bHandleCode,0);
                    }
                }
            }
            //处理数据
            {
                CSignedLockObject UnionLock(&pSocketManage->m_csUnionSend,true);
                pSocketManage->m_uUnionCerrenPos=(pSocketManage->m_uUnionCerrenPos+1)%uBatchLineSize;
                pSocketManage->m_uUnionLineCount--;
            }
        }
        catch (...) {TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);}
    }

    return 0;
}

//添加用户到群组
int CTCPSocketManageForWeb::UnionAddUser(UINT uUnionID, UINT uSocketID)
{
    map<int,vector<int>>::iterator l_it;
    l_it = m_UnionData.find(uUnionID);
    if (l_it != m_UnionData.end())
    {
        auto temp = find(l_it->second.begin(),l_it->second.end(),uSocketID);
        if (temp == l_it->second.end())
        {
            l_it->second.push_back(uSocketID);
        }
    }
    else
    {
        vector<int> vtemp;
        vtemp.push_back(uSocketID);
        m_UnionData.insert(pair<int,vector<int>>(uUnionID,vtemp));
    }

    return 0;
}
//从群组删除用户
int CTCPSocketManageForWeb::UnionRemoveUser(UINT uUnionID, UINT uSocketID)
{
    map<int,vector<int>>::iterator l_it;
    l_it = m_UnionData.find(uUnionID);
    if (l_it != m_UnionData.end())
    {
        auto temp = find(l_it->second.begin(),l_it->second.end(),uSocketID);
        if (temp != l_it->second.end())
        {
            l_it->second.erase(temp);
        }
    }

    return 0;
}
//移除整个群组
int CTCPSocketManageForWeb::UnionRemoveAll(UINT uUnionID)
{
    map<int,vector<int>>::iterator l_it;
    l_it = m_UnionData.find(uUnionID);
    if (l_it != m_UnionData.end())
    {
        m_UnionData.erase(l_it);
    }

    return 0;
}

//用户下线移除用户所有群组
int CTCPSocketManageForWeb::UnionRemovUserAll(UINT uSocketID)
{
    if (m_UnionData.size()>0)
    {
        for (auto l_it = m_UnionData.begin();l_it != m_UnionData.end();l_it++)
        {
            auto temp = find(l_it->second.begin(),l_it->second.end(),uSocketID);
            if (temp != l_it->second.end())
            {
                l_it->second.erase(temp);
            }
        }
    }
    return 0;
}