#include "StdAfx.h"
#include "TCPClientSocket.h"
#include "HNRC4.h"
#include "MD5.h"
#include "fastlz.h"
using namespace HN;

//宏定义
#define ID_SOCKET_WND			10								//SOCKET 窗口 ID
#define WM_SOCKET_MESSAGE		WM_USER+12						//SOCKET 消息

/**************************************************************************************************************/

static unsigned char g_chSecretKey[128] = { 0x00 };
CString g_StrSecretKey;

static void GeneratorSecretKey()
{
	static bool bGenerator = false;
	if (!bGenerator)
	{
		char strSecretKey[9];
		int nLen = sprintf((char*)strSecretKey, "%d", SECRET_KEY);

		unsigned char szMDTemp[16];
		char szMDKey[33];
		MD5_CTX Md5;
		Md5.MD5Update((unsigned char*)strSecretKey,nLen);
		Md5.MD5Final(szMDTemp);
		for (int i=0;i<16;i++) 
		{
			wsprintf(&szMDKey[i*2],"%02x",szMDTemp[i]);
		}

		memcpy(g_chSecretKey, szMDKey, sizeof(szMDKey));
		g_StrSecretKey.Format("%s", szMDKey);
		bGenerator = true;
	}
}

BEGIN_MESSAGE_MAP(CTCPClientSocket, CWnd)
	ON_MESSAGE(WM_SOCKET_MESSAGE,OnSocketNotifyMesage)
END_MESSAGE_MAP()

//构造函数
CTCPClientSocket::CTCPClientSocket(IClientSocketService * pIService)
{
	m_iReadBufLen=0;
	m_pIService=pIService;
	m_bConnectState=NO_CONNECT;
	m_hSocket=INVALID_SOCKET;
	m_iCheckCode = 0;
    memset(m_szBuffer, 0, sizeof(m_szBuffer));
	if (AfxGetMainWnd()!=NULL) InitSocketWnd();

	GeneratorSecretKey();
}

//析构函数
CTCPClientSocket::~CTCPClientSocket()
{
	CloseSocket(false);
	CloseSocketWnd();
}

//初始化 SOCKET
bool CTCPClientSocket::InitSocketWnd()
{
	try
	{
		if (GetSafeHwnd()==NULL)
		{
			if (!Create(NULL,NULL,WS_CHILD,CRect(0,0,0,0),AfxGetMainWnd(),ID_SOCKET_WND,NULL))
			{
				return false;
			}
		}
	}
	catch (...) {}
	return true;
}

//关闭 SOCKET 窗口
bool CTCPClientSocket::CloseSocketWnd()
{
	if (GetSafeHwnd()!=NULL) DestroyWindow();
	return true;
}

//SOCKET 消息处理程序
LRESULT	CTCPClientSocket::OnSocketNotifyMesage(WPARAM wParam, LPARAM lParam)
{
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_CONNECT:	//连接信息
		{
			UINT uErrorCode=WSAGETSELECTERROR(lParam);
			if (uErrorCode==0) m_bConnectState=CONNECTED;
			else CloseSocket(false);
			if (m_pIService!=NULL) m_pIService->OnSocketConnectEvent(uErrorCode,this);
            SendData(1,1,0);
			return 0;
		}
	case FD_READ:	//读取数据
		{
			
			//读取网络数据
			int iLen=::recv(wParam,(char *)(m_szBuffer+m_iReadBufLen),sizeof(m_szBuffer)-m_iReadBufLen,0);
			if (m_hSocket != wParam) return 0;
			if (iLen==SOCKET_ERROR)
			{ 
				CloseSocket(true);
				return 0; 
			}

			//处理数据
			try
			{
				m_iReadBufLen+=iLen;
				BYTE szNetBuffer[MAX_SEND_SIZE+NET_HEAD_SIZE] = {0};
                int nNetMessageHeadLen = sizeof(NetMessageHead);
                HNRC4 m_RC4;
                m_RC4.init(g_chSecretKey, g_StrSecretKey.GetLength());
				do
				{
					//效验数据
					NetMessageHead * pNetHead=(NetMessageHead *)m_szBuffer;
					if ((m_iReadBufLen < sizeof(NetMessageHead))
						||(m_iReadBufLen < (int)pNetHead->uMessageSize))
					{
						TCHAR sz[100];
						wsprintf(sz,"%d,%d,%d",m_iReadBufLen,sizeof(NetMessageHead),(int)pNetHead->uMessageSize);
						//AfxMessageBox(sz);
						return 0;
					}

					BYTE p[MAX_SEND_SIZE_OUT] = {0};
                    if(pNetHead->uMessageSize > nNetMessageHeadLen)
					{
						memcpy_s(p,MAX_SEND_SIZE_OUT,pNetHead,pNetHead->uMessageSize);
                        m_RC4.decrypt((unsigned char*) pNetHead + nNetMessageHeadLen, pNetHead->uMessageSize - nNetMessageHeadLen);
                    }

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
						::CopyMemory(szNetBuffer,m_szBuffer,uMessageSize);
					}
					
					//删除数据
					int iLength = pNetHead->uMessageSize;
					::MoveMemory(m_szBuffer,m_szBuffer+iLength,(m_iReadBufLen-iLength)*sizeof(BYTE));
					m_iReadBufLen-=iLength;

					//处理数据
					uMessageSize-=sizeof(NetMessageHead);
					NetMessageHead * pHandleNetData=(NetMessageHead *)szNetBuffer;

					//AfxMessageBox("333");

					if (m_pIService->OnSocketReadEvent(pHandleNetData,uMessageSize?pHandleNetData+1:NULL,uMessageSize,this)==false)
					{
#ifdef SUPER_VERSION
						TCHAR sz[100];
						wsprintf(sz, TEXT("网络消息处理失败, 连接即将关闭,MainID:%d bHandleCode:%d bAssistantID:%d MsgSize:%d"), pHandleNetData->bMainID, pHandleNetData->bHandleCode,
							pHandleNetData->bAssistantID, uMessageSize);
						
						//AFCMessageBox(sz);
                        DUIMessageBox(m_hWnd,MB_ICONINFORMATION|MB_OK,"系统提示",sz);
#endif
						CloseSocket(true);
						return 0;
					}
				} 
				while (
					(m_iReadBufLen>0)
					&&
					(m_hSocket!=INVALID_SOCKET)
					);
			}
			catch (...)	{
				CloseSocket(true); 
			}
			return 0;
		}
	case FD_CLOSE:	//关闭消息
		{
			if (m_hSocket != wParam) return 0;

			CloseSocket();
			return 0;
		}
	}
	return 0;
}

//连接服务器
bool CTCPClientSocket::Connect(const TCHAR * szServerIP, UINT uPort)
{
	long dwServerIP=inet_addr(szServerIP);
	if (dwServerIP==INADDR_NONE)
	{
		LPHOSTENT lpHost=::gethostbyname(szServerIP);
		if (lpHost==NULL) return false;
		dwServerIP=((LPIN_ADDR)lpHost->h_addr)->s_addr;
	}
	return Connect(dwServerIP,uPort);
}

//连接服务器
bool CTCPClientSocket::Connect(long int dwServerIP, UINT uPort)
{
	//效验数据
	if (dwServerIP==INADDR_NONE) return false;

	//初始化数据
	InitSocketWnd();
	m_hSocket=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (m_hSocket==INVALID_SOCKET) return false;
	HWND hw = GetSafeHwnd();
	if (::WSAAsyncSelect(m_hSocket,GetSafeHwnd(),WM_SOCKET_MESSAGE,FD_READ|FD_CONNECT|FD_CLOSE)==SOCKET_ERROR)
	{
		return false;
	}
	//解释服务器地址
	sockaddr_in SocketAddr;
	SocketAddr.sin_family=AF_INET;
	SocketAddr.sin_port=htons(uPort);
	SocketAddr.sin_addr.S_un.S_addr=dwServerIP;
	//连接操作
	if ((::connect(m_hSocket,(sockaddr *)&SocketAddr,sizeof(SocketAddr))==SOCKET_ERROR)
		&&(::WSAGetLastError()!=WSAEWOULDBLOCK))
	{
		CloseSocket(false);
		return false;
	}

	//设置数据
	m_bConnectState=CONNECTING;
	return true;
}

//关闭 SOCKET
bool CTCPClientSocket::CloseSocket(bool bNotify)
{
	if(m_hSocket)
		SendData(200,0,0);
	bool bClose=(m_hSocket!=INVALID_SOCKET);
	m_bConnectState=NO_CONNECT;
	if (m_hSocket!=INVALID_SOCKET)
	{
		::WSAAsyncSelect(m_hSocket,this->GetSafeHwnd(),WM_SOCKET_MESSAGE,0);
		::closesocket(m_hSocket);
		m_hSocket=INVALID_SOCKET;
		m_iReadBufLen=0;
	}
	if ((bNotify==true)&&(bClose==true)&&(m_pIService!=NULL)) m_pIService->OnSocketCloseEvent();
	return bClose;
}

//发送函数
int CTCPClientSocket::SendData(void * pData, UINT uSize, UINT bMainID, UINT bAssistantID, UINT bHandleCode)
{
	if ((m_hSocket!=INVALID_SOCKET)&&(uSize<=MAX_SEND_SIZE))
	{
		//定义数据
		int iErrorCode=0;
		char bSendBuffer[MAX_SEND_SIZE+NET_HEAD_SIZE] = {0};
		UINT uSendSize=uSize+sizeof(NetMessageHead),uSended=0;

		//打包数据
		NetMessageHead * pNetHead=(NetMessageHead *)bSendBuffer;
		pNetHead->uMessageSize=uSendSize;
		pNetHead->bMainID=bMainID;
		pNetHead->bAssistantID=bAssistantID;
		pNetHead->bHandleCode=bHandleCode;
		pNetHead->bReserve = m_iCheckCode;
        if(uSize > 0)
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
			memcpy((unsigned char *)pNetHead + sizeof(NetMessageHead), pDataCopy, pNetHead->uMessageSize-NET_HEAD_SIZE); 


            if (pDataCopy)
            {
                free(pDataCopy);
                pDataCopy = NULL;
            }
        }
		//发送数据
		int iSendCount=0;	//发送次数计数器
		uSendSize = pNetHead->uMessageSize;
		do
		{
			int length = uSendSize - uSended;
			iErrorCode=::send(m_hSocket,bSendBuffer+uSended,uSendSize-uSended,0);
			int iret = ::GetLastError();

			if (iErrorCode==SOCKET_ERROR) 
			{
				if (::WSAGetLastError()==WSAEWOULDBLOCK) //网络有阻塞，原来紧接着的代码是：return uSize,Fred Huang 2008-05-16
				{
					if(iSendCount++>100)//判断重发次数是否超过100次,Fred Huang 2008-05-16
						return SOCKET_ERROR;//太多次的重发了，直接返回 错误,Fred Huang 2008-05-16
					else
					{
						Sleep(10);	//等待 10 ms，和上面的重发100次，等于有1秒钟的时间来重发,Fred Huang 2008-05-16
						continue;   //重发数据,Fred Huang 2008-05-16
					}
				}
				else
					return SOCKET_ERROR;
			}
			uSended+=iErrorCode;
			iSendCount=0;//重要计数器置0，发下一条数据,Fred Huang 2008-05-16
		} while (uSended<uSendSize);

		return uSize;
	}
	return SOCKET_ERROR;
}

//简单命令发送函数
int CTCPClientSocket::SendData(UINT bMainID, UINT bAssistantID, UINT bHandleCode)
{
	return SendData(NULL,0,bMainID,bAssistantID,bHandleCode);
}

//解释错误
CString CTCPClientSocket::TranslateError(UINT uErrorCode)
{
	CString strMessage;
	switch (uErrorCode)
	{
	case WSANOTINITIALISED:
		{
			strMessage=TEXT("A successful WSAStartup call must occur before using this function.");
			break;
		}
	case WSAENETDOWN:
		{
			strMessage=TEXT("网络系统初始化失败！");
			break;
		}
	case WSAEINTR:
		{
			strMessage=TEXT("The blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall.");
			break;
		}
	case WSAEADDRINUSE:
		{
			strMessage=TEXT("The socket's local address is already in use and the socket was not marked to allow address reuse with SO_REUSEADDR. This error usually occurs when executing bind, but could be delayed until this function if the bind was to a partially wildcard address (involving ADDR_ANY) and if a specific address needs to be committed at the time of this function.");
			break;
		}
	case WSAEINPROGRESS:
		{
			strMessage=TEXT("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");
			break;
		}
	case WSAEALREADY:
		{
			strMessage=TEXT("A nonblocking connect call is in progress on the specified socket. ");
			break;
		}
	case WSAEADDRNOTAVAIL:
		{
			strMessage=TEXT("The remote address is not a valid address.");
			break;
		}
	case WSAEAFNOSUPPORT:
		{
			strMessage=TEXT("Addresses in the specified family cannot be used with this socket.");
			break;
		}
	case WSAECONNREFUSED:
		{
			strMessage=TEXT("服务器没有启动");
			break;
		}
	case WSAEFAULT:
		{
			strMessage=TEXT("The name or the namelen parameter is not a valid part of the user address space, the namelen parameter is too small, or the name parameter contains incorrect address format for the associated address family.");
			break;
		}
	case WSAEINVAL:
		{
			strMessage=TEXT("The parameter s is a listening socket.");
			break;
		}
	case WSAEISCONN:
		{
			strMessage=TEXT("The socket is already connected.");
			break;
		}
	case WSAENETUNREACH:
		{
			strMessage=TEXT("The network cannot be reached from this host at this time.");
			break;
		}
	case WSAENOBUFS:
		{
			strMessage=TEXT("No buffer space is available. The socket cannot be connected.");
			break;
		}
	case WSAENOTSOCK:
		{
			strMessage=TEXT("The descriptor is not a socket.");
			break;
		}
	case WSAETIMEDOUT:
		{
			strMessage=TEXT("Attempt to connect timed out without establishing a connection.");
			break;
		}
	case WSAEWOULDBLOCK:
		{
			strMessage=TEXT("The socket is marked as nonblocking and the connection cannot be completed immediately.");
			break;
		}
	case WSAEACCES:
		{
			strMessage=TEXT("Attempt to connect datagram socket to broadcast address failed because setsockopt option SO_BROADCAST is not enabled.");
			break;
		}
	default:strMessage.Format(TEXT("Unknow Socket Error: %d"),uErrorCode);
	}
	return strMessage;
}


//设置校验码，密文与密钥
void CTCPClientSocket::SetCheckCode(__int64 iCheckCode, int isecretkey)
{
	m_iCheckCode = (iCheckCode - isecretkey) / 23;
}