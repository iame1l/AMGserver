/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once
#include "HNBaseType.h"
#include "MainManage.h"
#include "rminfo.h"
#include <map>

//map<RoomInfo_Base,int> Map_RoomInfoBase;

class CGameLogonManage;

class CGameManageSocket : public CBaseMainManage
{
private:
	CGameLogonManage	*m_pManager;
	CAFCSignedLock		m_cslock;
	map<int,int>		Map_RoomSocketID;	

public:
	CGameManageSocket();
	virtual ~CGameManageSocket();

	virtual bool OnStart();
	virtual bool OnStop();
	void SetCGameLogonManage(CGameLogonManage* pManager) { m_pManager = pManager; }
	void SetUserCloseSocket(UINT uUserID);

	int* GetRoomList(int* Count);
	
private:
	virtual bool PreInitParameter(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData);
	virtual bool OnSocketRead(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	virtual bool OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, long int lConnectTime);

	virtual bool OnDataBaseResult(DataBaseResultLine * pResultData);
	virtual bool OnTimerMessage(UINT uTimerID);

	bool HandleMessage(NetMessageHead *pNetHead,void *pData,UINT uSize,ULONG uAccessIP,UINT uIndex, DWORD dwHandleID);

public:
	int SendData(int RoomID, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID);
	int SendData(int RoomID, void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID);
};