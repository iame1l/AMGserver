/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#ifndef CONTEST_SERVICE_HEAD_FILE
#define CONTEST_SERVICE_HEAD_FILE

#include "Stdafx.h"

class CGameLogonManage;

class CContestServer4Z
{
public:
	CContestServer4Z(CGameLogonManage* pLogonManage);
	~CContestServer4Z(void);

public:
	//收到网络请求
	bool OnNetMessage(NetMessageHead* pNetHead,void* pData,UINT uSize,ULONG uAccessIP,UINT uIndex,DWORD dwHandleID);

	//处理网络请求结果处理
	bool OnDataBaseResult(DataBaseResultLine* pResultData); 

public:

	bool getContestRoomIDRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		//获取比赛房间ID
	bool getApplyNumRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);			//获取已报名人数
	bool onContestApplyRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		//比赛报名
	bool onContestAwardRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		//获取比赛奖励

public:

	bool getContestRoomIDResponse(DataBaseResultLine* pResultData);		//获取比赛房间ID结果
	bool getApplyNumResponse(DataBaseResultLine* pResultData);			//获取已报名人数结果
	bool onContestApplyResponse(DataBaseResultLine* pResultData);		//比赛报名结果
	bool notifyContestResponse(DataBaseResultLine* pResultData);		//比赛开始通知
	bool notifyContestLogonResponse(DataBaseResultLine* pResultData);	//登陆后通知即将开始的比赛
	bool getContestAwardResponse(DataBaseResultLine* pResultData);		//获取比赛奖励

private:
	CGameLogonManage* _pLogonManage;
};


#endif