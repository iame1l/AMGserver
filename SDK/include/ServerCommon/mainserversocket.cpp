/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "stdafx.h"
#include "AFCSocket.h"
#include "mainserversocket.h"
#include "GameMainManage.h"
#include "commonuse.h"

CMainServerSocket::CMainServerSocket(CGameMainManage* pManager)
{
	m_pManager = pManager;
	m_pSocket = 0;
	m_bConnect = 0;
	m_nKeepAliveCount = 0;

	Initial();
}

CMainServerSocket::~CMainServerSocket()
{
	SafeDelete(m_pSocket);
}

INT32 CMainServerSocket::Initial()
{
	if(m_pSocket == NULL)
		m_pSocket = new CTCPClientSocket(this);

	if(m_pSocket != NULL)	{
		CINIFile fconf(CINIFile::GetAppPath() + "HNGameGate.ini");
		m_strHostIP = fconf.GetKeyVal("GateServer","MainServerAddress","127.0.0.1");
		if(m_strHostIP.IsEmpty())
			return -1;

		UINT32 uPort = PORT_GAME_CONNECT_MAIN;

		if(!Connect(m_strHostIP.GetBuffer(m_strHostIP.GetLength()),uPort))
		{
			return -1;
		}
	}
	return 0;
}

INT32 CMainServerSocket::Connect(const char* szServerIP,UINT32 uPort)
{
	if(m_pSocket != NULL)
	{
		do
		{
			bool ret = m_pSocket->Connect(szServerIP,uPort);
			if(ret)
			{
				m_bConnect = 1;
				return (INT32)ret;
			}
		}while(GetLastError() == WSAEWOULDBLOCK);
	}
	
	return -1;
}

int CMainServerSocket::SendData(void *pData,UINT uSize,UINT bMainID,UINT bAssistID,UINT uHandleCode)
{
	if(m_pSocket != NULL)
	{
		return m_pSocket->SendData(pData,uSize,bMainID,bAssistID,uHandleCode);
	}
	return 0;
}

int CMainServerSocket::SendData(UINT uMainID, UINT uAssistantID,UINT uHandleCode)
{
	if(m_pSocket != NULL)
	{
		return m_pSocket->SendData(uMainID,uAssistantID,uHandleCode);
	}
	return 0;
}

void CMainServerSocket::CloseSocket(bool BNotify)
{
	if(m_pSocket != NULL)
	{
		m_pSocket->CloseSocket(BNotify);
	}
}

bool CMainServerSocket::CheckReConnect()
{
	if(m_bConnect == 0 && m_pSocket != NULL)
	{
		if(m_strHostIP.IsEmpty())
		{
			return false;
		}
		UINT32 uPort = PORT_GAME_CONNECT_MAIN;

		if(!Connect(m_strHostIP.GetBuffer(m_strHostIP.GetLength()),uPort))
		{
			return false;
		}
	}
	return true;
}

bool CMainServerSocket::CheckKeepLive()
{
	if(m_bConnect == 2 && m_pSocket != NULL)
	{
		m_nKeepAliveCount++;

		//SendData(MDM_CONNECT,ASS_NET_TEST,0);

		if(m_nKeepAliveCount > 5)
			OnSocketCloseEvent();
	}

	return true;
}

bool CMainServerSocket::OnSocketReadEvent(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	m_nKeepAliveCount = 0;

	if(pNetHead->bMainID == MDM_CONNECT)
	{
		if(pNetHead->bAssistantID == ASS_CONNECT_SUCCESS/*ASS_NET_TEST*/)
		{
			m_bConnect = 2;
			MSG_S_ConnectSuccess *_p = (MSG_S_ConnectSuccess*)pNetData;
			if(_p != NULL) 
				m_pSocket->SetCheckCode(_p->i64CheckCode,SECRET_KEY);

			int RoomID = m_pManager->m_InitData.uRoomID;
			pClientSocket->SendData(&RoomID,sizeof(int),MDM_MG_SENDROMMID,ASS_MG_SENDROMMID,0);
		}
		else if (pNetHead->bAssistantID == ASS_NET_TEST)
		{
			pClientSocket->SendData(MDM_CONNECT,ASS_NET_TEST,0);//连接消息类型
		}
	}
	else
	{
		m_pManager->OnHandleMSMessage(pNetHead,pNetData,uDataSize,pClientSocket);
	}

	return true;
}

bool CMainServerSocket::OnSocketConnectEvent(UINT uErrorCode, CTCPClientSocket * pClientSocket)
{
	if(uErrorCode != 0)
		m_bConnect = 0;
	else
	{
		m_bConnect = 2;
	}

	return true;
}

bool CMainServerSocket::OnSocketCloseEvent()
{
	m_bConnect = 0;
	//CloseSocket(true);
	return true;
}
