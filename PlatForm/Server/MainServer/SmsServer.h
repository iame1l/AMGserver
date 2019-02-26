/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#ifndef SMS_SERVICE_HEAD_FILE
#define SMS_SERVICE_HEAD_FILE

#include "Stdafx.h"
#include "SmsIf.h"

class CGameLogonManage;

class CSmsServer
{
public:
	CSmsServer(CGameLogonManage* pLogonManage);
	~CSmsServer(void);

public:
	//收到网络请求
	bool OnNetMessage(NetMessageHead* pNetHead,void* pData,UINT uSize,ULONG uAccessIP,UINT uIndex,DWORD dwHandleID);

public:
	bool sendMessageRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		//

public:
	

private:
	CGameLogonManage* _pLogonManage;
	// 短信息处理接口
	ISms					*m_pISms;
	HMODULE					m_hModual;
	//短讯格式
	CString					m_strSmsContent;
};

#endif