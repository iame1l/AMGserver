/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#ifndef UPDATE_USERINFO_SERVICE_HEAD_FILE
#define UPDATE_USERINFO_SERVICE_HEAD_FILE

#include "Stdafx.h"

class CGameLogonManage;

class CUpdateUserInfo
{
public:
	CUpdateUserInfo(CGameLogonManage* pLogonManage);
	~CUpdateUserInfo(void);

public:
	//收到网络请求
	bool OnNetMessage(NetMessageHead* pNetHead,void* pData,UINT uSize,ULONG uAccessIP,UINT uIndex,DWORD dwHandleID);

	//处理网络请求结果处理
	bool OnDataBaseResult(DataBaseResultLine* pResultData); 

public:
	bool lockAccountRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		//
	bool bindMobileRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		//
	bool setLogoRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		//
	bool updatePwdRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		//
	bool getNickNameIDRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		//
	bool updateUserInfoRequest(NetMessageHead* pNetHead,void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		//
	bool ForgetpwdRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		//
//    bool AddUserPhoneNum(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		// 用户账号关联手机号
public:
	bool lockAccountResponse(DataBaseResultLine* pResultData);		//
	bool bindMobileResponse(DataBaseResultLine* pResultData);		//
	bool setLogoResponse(DataBaseResultLine* pResultData);		//

private:
	CGameLogonManage* _pLogonManage;
};

#endif