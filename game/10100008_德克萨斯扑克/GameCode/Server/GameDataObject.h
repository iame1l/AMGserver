
// ***************************************************************
//  GameDataObject   version:  1.0   ·  date: 03/29/2011
//  -------------------------------------------------------------

//	德克萨斯扑克，服务端游戏数据\
	NameID：德克萨斯扑克

//  -------------------------------------------------------------
//  Copyright (C) 2011 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************

#pragma once

#include "UserDataSvrObject.h"
#include "../GameMessage/UpgradeMessage.h"
#include "../GameMessage/UpGradeLogic.h"

//新增类型定义
typedef map<int,__int64> MAP_INT_INT64;
typedef map<int,TRCCurBuyInInfo> MAP_INT_BUYININFO;
typedef map<int,CalculateBoardData> MAP_INT_BOARDDATA;

class  CGameDataObject
{

public:

	//初始化数据
	CGameDataObject(void);

	//释放数据
	virtual~CGameDataObject(void);

	//初始化
	void Init();

	//半初始化
	void HalfInit();

/*-------------------------------------------------------------------------------------
    动作相关
  -----------------------------------------------------------------------------------*/
public:

	//设置玩家为未知是否同意底注状态
	void SetUnknowStateTo(BYTE bDeskStation);

	//设置玩家为同意底注状态
	void SetAgreeStateTo(BYTE bDeskStation);

	//设置玩家为不同意底注状态
	void SetDisAgreeStateTo(BYTE bDeskStation);
	
	//为某个玩家发牌
	bool DealCardsTo(BYTE bDeskStation,int iSendNum);

	//清空本轮的所有玩家出牌数据
	void ClearAllUserOutCard();

/*-----------------------------------------------------------------
    获取信息相关
  ---------------------------------------------------------------*/
public:

	//得到有多少个玩家同意游戏（同意底注）
	int GetAgreeNums();

	//得到有多少个玩家已发完牌
	bool IsAllPlayerSendCardFinish();

	//获取活动玩家的数量
	int GetActivePlayerNums();

	//获取下注金币
	__int64 GetBetMoney(BYTE bDeskStation = 255, bool bAll = true);

    //设置玩家的动作
    void MakeVerbs(BYTE byFlag);

	//设置有效加注位置强制下一轮标记
	void SetValidRaisePosForceNextTurn(bool bForceNextTurn = false);

	//获得有效加注位置强制下一轮标记
	bool GetValidRaisePosForceNextTurn();

	//重新设置最小下注/加注金币
	void ReSetMinBetOrRaiseMoney();

	//设置最小下注/加注金币
	void SetMinBetOrRaiseMoney(__int64 iMinMoney);

	//获得最小下注/加注金币
	__int64 GetMinBetOrRaiseMoney() const;

	//重新设置最大下注/加注金币
	void ReSetMaxBetOrRaiseMoney();

	//设置最大下注/加注金币
	void SetMaxBetOrRaiseMoney(__int64 iMinMoney);

	//获得最大下注/加注金币
	__int64 GetMaxBetOrRaiseMoney() const;

	//重新设置有效加注位位置
	void ReSetValidRaisePos();

	//设置有效加注位位置
	void SetValidRaisePos(BYTE byValidRaisePos);

	//获得有效加注位位置
	BYTE GetValidRaisePos() const;

	//重新设置 有效加注位上一次加注成功时的总下注额度
	void ReSetValidRaisePosEffectbet();

	//设置 有效加注位上一次加注成功时的总下注额度
	void SetValidRaisePosEffectbet(__int64 iMinMoney);

	//获得 有效加注位上一次加注成功时的总下注额度
	__int64 GetValidRaisePosEffectbet() const;

	//玩家获得好牌
	void GetGoodCard(BYTE bDeskStation);

/*-----------------------------------------------------------------
    新增房卡场货币系统相关 数据相关
  ---------------------------------------------------------------*/
public:
	bool SetMapCurMoney(int iUserID, const __int64& iUserMoney);
	bool GetMapCurMoney(int iUserID, __int64& iUserMoney) const;

	bool SetMapCurBuyInInfo(int iUserID, const TRCCurBuyInInfo& tCurBuyIn);
	bool GetMapCurBuyInInfo(int iUserID, TRCCurBuyInInfo& tCurBuyIn) const;

	bool SetMapCalBoardData(int iUserID, const CalculateBoardData& tCalBoardData);
	bool GetMapCalBoardData(int iUserID, CalculateBoardData& tCalBoardData) const;

	bool CheckCurBuyInInfo(BYTE byDeskstation);

	//检查是否可以买入
	bool CheckCanBuyIn(BYTE byDeskstation) const;

	//检查是否可以玩耍
	bool CheckCanPlay(BYTE byDeskstation) const;

	//Clear Map 数据
	bool ClearRCMapData();

/*-----------------------------------------------------------------
    大多数游戏都会有的数据
  ---------------------------------------------------------------*/
public:

	//牌源数组，最大支持四副牌
	BYTE m_byCards[MAX_CARDS_NUM];

	//牌原始索引[新超端换牌需要]
	//[暂时不用，维护这个数组太麻烦]
	//BYTE m_byCardOriginalIndex[NUM_CARDS_NUM];

	//存放底牌的数组
	BYTE m_byBackCards[PLAY_COUNT][MAX_BACKCARDS_NUM];

	//底牌的数量
	int m_iBackCardNum;

	//发牌位置
	int m_iGetCardPos;

	//用户数据
	CUserDataSvrObject m_UserData[PLAY_COUNT];

	//得到令牌的用户，即可活动的用户
	BYTE m_byTokenUser;

	//庄家
	BYTE m_byNTUser;

	//当前游戏活动的圈数
	int m_nTurnNums;


/*-----------------------------------------------------------------
    德克萨斯扑克游戏特有的数据
  ---------------------------------------------------------------*/
	
	//大盲注
	BYTE m_bBigBlind;

	// 小盲注
	BYTE m_bSmallBlind;
	
	// 下注金币
	__int64 m_nBetMoney[PLAY_COUNT][8];

	// 当前下注最多的玩家
	__int64 m_nMaxBetMoney;

	// 当前下注池
	__int64 m_nBetPool;

    // 记录最后一个全下的人
    BYTE m_bAllBetStation;

	// 记录不可下注
	bool m_bNoBet[PLAY_COUNT];

	// 记录玩家在哪个池中全下的 2011-10-28 duanxiaohui
	bool m_bAllBet[PLAY_COUNT][8];

	// 记录放弃
	bool m_bGiveUp[PLAY_COUNT];

	// 每个玩家可用下注金币
	__int64 m_UserMoney[PLAY_COUNT];

	TRCCurBuyInInfo m_tRCCurBuyInInfo[PLAY_COUNT];

	// 过牌次数
	int m_iCheckCount;

	// 下一个下注玩家
	int m_iBetUser;

	// 桌面上公共牌
	BYTE m_iCards[5];

	// 桌面公共牌数
	int m_iCardsCount;

	// 是否有玩家加注
	bool m_bRaise;
	
	// 记录加注次数
	int m_iRaiseCounts;

	// 记录全下玩家数量
	int m_iAllinUser;

	// 记录每个边池是否有全下次数
	BYTE m_byAllPools[8];
	
	//记录跟注玩家
	int m_iCallCounts;

	// 记录弃牌玩家
	int m_iFoldCount;

	// 一轮中第一个过牌玩家
	BYTE m_bFirstStation;

	// 记录第一个下注玩家
	BYTE m_bBetStation;

    // 记录最后一个加注玩家
    BYTE m_bRaiseStation;

    // 记录下注玩家数据
    int m_iBetCounts;

    // 用户当前可做的动作标记
    BYTE m_byVerbFlag;

	// 有效加注位，用于有时极端情况判断使用
	BYTE m_byValidRaisePos;

	// 有效加注位上一次加注成功时的总下注额度
	__int64 m_iValidRaisePosEffectbet;

	//有效加注位置强制下一轮标记
	bool m_bValidRaisePosForceNextTurn;

	// 最小下注/加注的金币
	__int64 m_iMinBetOrRaiseMoney;	

	// 最大下注/加注的金币(暂时不使用)
	__int64 m_iMaxBetOrRaiseMoney;						

    // 每个玩家最终牌型数据
    BYTE m_bResultCards[PLAY_COUNT][5];

	// 所有玩家弃牌结束
	bool m_bGiveUpOver;

	// 自动发牌
	bool m_bAutoSendCard;

    // 新的一轮
    bool m_bNewTurn;

	// 新的一轮
    bool m_bNewTurns;

	// 第一个发牌者
	BYTE m_iDealPeople;	

public:
	//超端设置的赢得位置
	DWORD	m_byWinUserID;

	//游戏人数
	int		m_iPlayCount;
public:

	//配置数据
	TDeskCfg m_tagDeskCfg;

	//新增房卡配置
	TRCOption m_tRCOption;

	MAP_INT_INT64 m_tMapCurMoney; //进入、买入、自动买入时、小结算确定更新
	MAP_INT_BUYININFO m_tMapCurBuyInInfo; //进入、买入、自动买入时确定更新,暂时无效
	MAP_INT_BOARDDATA m_tMapCalBoardData; //小结算更新,暂时无效

	//最近的成绩单
	TResult m_tagResult;

private:
	CUpGradeGameLogic m_GamgeLogic;	//游戏逻辑类

};


