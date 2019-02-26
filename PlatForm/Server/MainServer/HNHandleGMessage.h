/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once
#ifndef _HN_HANDLEGMESSAGE_HEADER_
#define _HN_HANDLEGMESSAGE_HEADER_

class CGameLogonManage;

class CHandleGMessage
{
public:

	CHandleGMessage(CGameLogonManage *pWnd);

	~CHandleGMessage(void);

public:

	//收到网络请求
	bool OnNetMessage(NetMessageHead* pNetHead,void* pData,UINT uSize,ULONG uAccessIP,UINT uIndex,DWORD dwHandleID);

	//处理网络请求结果处理
	bool OnDataBaseResult(DataBaseResultLine* pResultData); 

	//所有网络请求的实现
public:
	bool OnContestActiveError(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);

	bool OnContestNotice(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);

	//所有请求处理结果实现
public:
	
	;
private:
	//登陆管理模块
	CGameLogonManage* m_pLogonManage;


};

#endif