#pragma once
#ifndef _HN_HANDLEMSMESSAGE_HEADER_
#define _HN_HANDLEMSMESSAGE_HEADER_

#include "AFCDataBase.h"
#include "BaseMessage.h"
#include "ComStruct.h"

class CGameMainManage;

class CHandleMSMessage
{
public:

	CHandleMSMessage(CWnd *pWnd);

	~CHandleMSMessage(void);

public:

	//收到网络请求
	bool OnNetMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);

	//处理网络请求结果处理
	bool OnDataBaseResult(DataBaseResultLine* pResultData); 

	//所有网络请求的实现
public:
	//通知刷新桌子信息
	bool OnBuyDeskNotice(void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//恢复桌子信息
	bool OnReturnDeskNotice(void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//用户大厅断线，断开房间消息
	bool OnNetCutPlace(void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//激活比赛房间
	bool OnContestActive(void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//大厅解散房间
	bool OnHallDissmissDesk(void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//俱乐部解散房间
	bool OnClubDissmissDesk(void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//俱乐部创建房间
	bool OnClubBuyDesk(void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);

	//所有请求处理结果实现
public:


private:
	//登陆管理模块
	CGameMainManage* m_pGameManage;


};

#endif