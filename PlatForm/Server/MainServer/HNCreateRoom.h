#pragma once
#ifndef _CREATE_ROOM_HEADER_
#define _CREATE_ROOM_HEADER_

#include "Stdafx.h"

class CGameLogonManage;

class CCreateRoom
{
public:

	CCreateRoom(CWnd *pWnd);

	~CCreateRoom(void);

public:

	//收到网络请求
	bool OnNetMessage(NetMessageHead* pNetHead,void* pData,UINT uSize,ULONG uAccessIP,UINT uIndex,DWORD dwHandleID);

	//处理网络请求结果处理
	bool OnDataBaseResult(DataBaseResultLine* pResultData); 

	//处理GServer返回的创建结果
	bool OnHandleCreateResult(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	//处理GServer返回的解散房间结果
	bool OnHandleDissmissResult(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);

//所有网络请求的实现
public:
	//购买房间请求
	bool OnBuyDeskRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);
	//加入房间请求
	bool OnEnterDeskRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);
	//获取总战绩请求
	bool OnGetTotalRecordRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);
	//获取单局战绩请求
	bool OnGetSingelRecordRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);
	//获取短线信息请求
	bool OnGetCutRoomRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);
	//获取房间局数配置信息
	bool OnGetBuyDeskConfigRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);
	//获取回放地址
	bool OnGetRecordURLRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);
	//获取开放列表
	bool OnGetDeskListRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);
	//获取桌子玩家信息
	bool OnGetDeskUserRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);
	//解散房间
	bool OnDissmissDeskRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);
	//删除桌子信息
	bool OnDeleteRecordRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);

//所有请求处理结果实现
public:
	//购买房间结果
	bool OnBuyDeskResponse(DataBaseResultLine* pResultData);
	//加入房间结果
	bool OnEnterDeskResponse(DataBaseResultLine* pResultData);
	//获取总战绩结果
	bool OnGetTotalRecordResponse(DataBaseResultLine* pResultData);
	//获取单局战绩结果
	bool OnGetSingelRecordResponse(DataBaseResultLine* pResultData);
	//获取断线信息结果
	bool OnGetCutRoomResponse(DataBaseResultLine* pResultData);
	//获取房间局数配置信息结果
	bool OnGetBuyDeskConfigResponse(DataBaseResultLine* pResultData);
	//获取回放地址结果
	bool OnGetRecordURLResponse(DataBaseResultLine* pResultData);
	//获取桌子列表结果
	bool OnGetDeskListResponse(DataBaseResultLine* pResultData);
	//获取桌子玩家信息结果
	bool OnGetDeskUserResponse(DataBaseResultLine* pResultData);
	//解散房间结果
	bool OnDissmissDeskResponse(DataBaseResultLine* pResultData);
	//删除桌子信息结果
	bool OnDeleteRecordResponse(DataBaseResultLine* pResultData);
    //删除桌子
    bool OnDissmissDeskByMS(DataBaseResultLine* pResultData);

private:
	//登陆管理模块
	CGameLogonManage* m_pLogonManage;

	int aa;
};

#endif