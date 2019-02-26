/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "stdafx.h"
#include <AFCSocket.h>
#include "gamemanagesocket.h"
#include "GameLogonManage.h"

CGameManageSocket::CGameManageSocket()
{

}

CGameManageSocket::~CGameManageSocket()
{
}

bool CGameManageSocket::PreInitParameter(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData)
{
	pKernelData->bLogonDataBase = FALSE;
	pKernelData->bNativeDataBase = FALSE;
	pKernelData->bStartSQLDataBase = FALSE;

	pKernelData->bStartTCPSocket = TRUE;
	
	pInitData->uListenPort = PORT_GAME_CONNECT_MAIN;
	pInitData->uMaxPeople = 300;
	pInitData->iSocketSecretKey = SECRET_KEY;

	m_pManager = nullptr;
	return true;
}

bool CGameManageSocket::OnStart()
{
	return true;
}

bool CGameManageSocket::OnStop()
{
	return true;
}

bool CGameManageSocket::OnTimerMessage(UINT uTimerID)
{
	return true;
}

bool CGameManageSocket::HandleMessage(NetMessageHead *pNetHead,void *pData,UINT uSize,ULONG uAccessIP,UINT uIndex, DWORD dwHandleID)
{
	if (pNetHead->bMainID == MDM_MG_SENDROMMID && pNetHead->bAssistantID == ASS_MG_SENDROMMID)
	{
		Map_RoomSocketID.insert(pair<int,int>(*(int*)pData,uIndex));
		m_pManager->SetRoomConnect(*(int*)pData,true);
		return true;
	}

	m_pManager->OnHandleGMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);

	return true;
}

bool CGameManageSocket::OnSocketRead(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	return HandleMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
}

bool CGameManageSocket::OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, long int lConnectTime)
{
	map<int ,int >::iterator l_it;
	for(l_it = Map_RoomSocketID.begin(); l_it != Map_RoomSocketID.end();) 
	{
		if (l_it->second == uSocketIndex)
		{
			m_pManager->SetRoomConnect(l_it->first,false);
			l_it = Map_RoomSocketID.erase(l_it);
		}
		else
		{
			++l_it;
		}
	}

	return true;
}

bool CGameManageSocket::OnDataBaseResult(DataBaseResultLine * pResultData)
{
	return true;
}

#define REMAIN_MAINID 0
#define REMAIN_ASSID 0
void CGameManageSocket::SetUserCloseSocket(UINT uUserID)
{
	
}

int CGameManageSocket::SendData(int RoomID, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID)
{
	map<int ,int >::iterator l_it; 
	l_it=Map_RoomSocketID.find(RoomID);
	if(l_it==Map_RoomSocketID.end())
		return 0;
	else
		return m_TCPSocket.SendData(Map_RoomSocketID[RoomID],nullptr,0,bMainID,bAssistantID,bHandleCode,dwHandleID);
}

int CGameManageSocket::SendData(int RoomID, void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID)
{
	map<int ,int >::iterator l_it; 
	l_it=Map_RoomSocketID.find(RoomID);
	if(l_it==Map_RoomSocketID.end())
		return 0;
	else
		return m_TCPSocket.SendData(Map_RoomSocketID[RoomID],pData,uBufLen,bMainID,bAssistantID,bHandleCode,dwHandleID);
}

int* CGameManageSocket::GetRoomList(int* Count)
{
	*Count = Map_RoomSocketID.size();
	if (0 == *Count)
	{
		return nullptr;
	}
	int* RoomList = new int[*Count];

	int i = 0;
	for (auto l_it=Map_RoomSocketID.begin();l_it != Map_RoomSocketID.end();)
	{
		RoomList[i] = l_it->first;
		i++;
		l_it++;
	}

	return RoomList;
}