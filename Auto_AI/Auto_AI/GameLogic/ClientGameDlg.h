#pragma once

#include "StdAfx.h"
#include "Resource.h"
#include "UpgradeMessage.h"
#include "UpGradeLogic.h"
#include "ExtensionObject.h"
#include "HNSmartPointer.h"

#include "./DataManage.h"

//消息定义
//定时器 ID
enum TimerID{
	TID_BEGIN =	100,		//开始定时器
	TID_OUT_CARD,			//出牌等待时间
	TID_CALL_SCORE,			//叫分定时器
	TID_ROB_NT,				//抢地主计时器
	TID_ADD_DOUBLE,			//加棒
	TID_SHOW_CARD,
	TID_LEAVE,

	TID_MAX,				// 最大值
};

//mark
#define GAME_RAND_TIME				(/*rand() % 5 +*/ 1)					//智能时间
//#define GAME_RAND_TIME				(0)					//智能时间

//游戏框架类 
class CClientGameDlg : public CLoveSendClass//CGameFrameDlg
{
	DECLARE_MESSAGE_MAP()
public:
	CClientGameDlg();
public:
	virtual BOOL OnInitDialog();
	// 清理所有定时器
	void KillAllTimer();
	
	void ResetExt();
public:
	//设置游戏状态
	virtual bool SetGameStation(void * pStationData, UINT uDataSize);
	//游戏消息处理函数
	virtual bool HandleGameMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//同意开始游戏 
	virtual bool OnControlHitBegin();
	//游戏计时器
	virtual bool OnGameTimer(BYTE bDeskStation, UINT uTimeID, UINT uTimeCount);

	void PreDealing(UINT uTimeID, int& iChoice);

	//重新设置游戏数据
	virtual void ResetGameStation(int iGameStation);
public:
	LRESULT	OnCallScore(WPARAM wparam, LPARAM lparam);
	//抢地主
	LRESULT OnRobNT(WPARAM wparam, LPARAM lparam);
	//加棒
	LRESULT	OnAddDouble(WPARAM wparam, LPARAM lparam);
	//亮牌
	LRESULT	OnShowCard(WPARAM wparam, LPARAM lparam);
	//开始按钮函数
	LRESULT	OnHitBegin(WPARAM wparam, LPARAM lparam);

	// 启动GameBegin定时器
	void StartupGameBeginTimer();

	// 是否排队场
	bool IsQueueGame() { return 0 != (GRR_QUEUE_GAME & m_pGameInfo->dwRoomRule); }
	// 是否比赛场
	bool IsContestGame() { return 0 != (GRR_CONTEST & m_pGameInfo->dwRoomRule); }

	// 用户出牌
	void UserOutCard();
	void OnControlHitPass(void);

private:
	/// 当同伙出牌后,自己不能保证必赢的情况下的处理
	int AI_DaTongHuo(OutCardStruct & outCard);
	void outCardResult(OutCardMsg2AI * pOutCardInfo);
public:
	HN::CSmartPointer<ExtensionObject> m_pExt;
	
private:
	//mark
	/// 记录自己和其它几人出过的牌，主要用于分析对方手中的牌，和了解对方对自己的认识
	std::vector<OutCardStruct>     vChuCard[PLAY_COUNT];

	

private:
	//数据管理
	HN::DataManage			_dataMange;
	bool					m_bHaveRobOrCall;
	int						m_iDefaultTime;
	int						m_iHandCount;

private:
	//转换手牌
	void transformPlayCard(const Card *pCard, int nCardCount, T_C2S_PLAY_CARD_REQ& tLastCard);
	//获取手牌
	void GetOutCardCount(int* iUserCardCounts);
	//牌适配器、转换器(bTo为true是转到出牌算法，false为出牌算法转正常牌值)
	void ShapeAdapter(BYTE &byShape, EArrayType &eArrayType);
	bool SetMyGameTimer(BYTE bDeskStation,UINT uTimeCount,UINT uTimeID);

};

