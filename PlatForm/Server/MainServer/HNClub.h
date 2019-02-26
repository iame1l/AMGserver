/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#ifndef HNCLUB_SERVICE_HEAD_FILE
#define HNCLUB_SERVICE_HEAD_FILE

#include "Stdafx.h"

class CGameLogonManage;

class CHNClub
{
public:
	CHNClub(CGameLogonManage* pLogonManage);
	~CHNClub(void);

public:
	//收到网络请求
	bool OnNetMessage(NetMessageHead* pNetHead,void* pData,UINT uSize,ULONG uAccessIP,UINT uIndex,DWORD dwHandleID);

	//处理网络请求结果处理
	bool OnDataBaseResult(DataBaseResultLine* pResultData); 

public:
	bool createClubRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		// 创建俱乐部
	bool dissmissClubRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		// 解散俱乐部
	bool joinClubRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);			// 申请加入俱乐部
	bool getUserListRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		// 俱乐部玩家列表
	bool talkInClubRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		// 俱乐部聊天
	bool getRoomListRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		// 俱乐部房间列表
	bool buyDeskInClubRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		// 俱乐部创建房间
	bool changeNameRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		// 俱乐部更名
	bool kickUserRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);			// 会长踢人
	bool getClubListRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		// 俱乐部列表
	bool getReviewListRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		// 会长审核列表
	bool masterOptionRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		// 会长审核操作
	bool setClubNoticeRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		// 俱乐部公告
	bool enterClubRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);			// 进入俱乐部
	bool leaveClubRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);			// 退出俱乐部
	bool getBuyDeskRecordRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);	// 获取开房记录

public:
	bool createClubResponse(DataBaseResultLine* pResultData);				//
	bool dissmissClubResponse(DataBaseResultLine* pResultData);				//
	bool joinClubResponse(DataBaseResultLine* pResultData);					//
	bool getUserListResponse(DataBaseResultLine* pResultData);				//
	bool talkInClubResponse(DataBaseResultLine* pResultData);				//
	bool getRoomListResponse(DataBaseResultLine* pResultData);				//
	bool buyDeskInClubResponse(DataBaseResultLine* pResultData);			//
	bool changeNameResponse(DataBaseResultLine* pResultData);				//
	bool kickUserResponse(DataBaseResultLine* pResultData);					//
	bool getClubListResponse(DataBaseResultLine* pResultData);				//
	bool getReviewListResponse(DataBaseResultLine* pResultData);			//
	bool masterOptionResponse(DataBaseResultLine* pResultData);				//
	bool setClubNoticeResponse(DataBaseResultLine* pResultData);			//
	bool enterClubResponse(DataBaseResultLine* pResultData);				//
	bool leaveClubResponse(DataBaseResultLine* pResultData);				//
	bool getBuyDeskRecordResponse(DataBaseResultLine* pResultData);			//

public:
	//处理GServer返回的创建房间结果
	bool OnHandleClubBuyDeskResult(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	//处理GServer返回的解散房间结果
	bool OnHandleClubClearDeskResult(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);

private:
	CGameLogonManage* _pLogonManage;
};

#endif