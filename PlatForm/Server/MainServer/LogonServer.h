/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#ifndef LOGON_SERVICE_HEAD_FILE
#define LOGON_SERVICE_HEAD_FILE

#include "Stdafx.h"

class CGameLogonManage;

class CLogonServer
{
public:
	CLogonServer(CGameLogonManage* pLogonManage);
	~CLogonServer(void);

public:
	//收到网络请求
	bool OnNetMessage(NetMessageHead* pNetHead,void* pData,UINT uSize,ULONG uAccessIP,UINT uIndex,DWORD dwHandleID);

	//处理网络请求结果处理
	bool OnDataBaseResult(DataBaseResultLine* pResultData); 

public:
	bool onUserLogonRequest(void* pData,UINT uSize,ULONG uAccessIP,UINT uIndex,DWORD dwHandleID);		//

public:
	bool onUserLogonResponse(DataBaseResultLine* pResultData);		//

private:
	CGameLogonManage* _pLogonManage;
};

#endif