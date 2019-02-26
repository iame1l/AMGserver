#pragma once

#include <afxmt.h>
#include <map>
#include "commonuse.h"
#include "GlobalData.h"
#include "UpGradeMessage.h"
#include "MJ.h"

using namespace std;

class CBaseMJLogic
{
	//成员函数
public:
	//构造函数		
	CBaseMJLogic(void);
	//析构函数
	virtual ~CBaseMJLogic();
	//初始化麻将数据
	virtual bool InitMJData();
	//每一局结束都要调用该函数进行初始化
	virtual bool ReSetMJData();

	bool SetUserID(long int UserID[]){CopyMemory(m_UserID,UserID,sizeof(m_UserID));return true;}

	//获取下一个位置
	//BYTE uport 起始点
	//BYTE ufull 总数
	//bool clockwise 是否顺时针
	virtual BYTE GetNext(BYTE uport, BYTE ufull, bool clockwise);
	//获得起手手牌张数
	BYTE GetFirstHandCard(){return m_uFirstHandCard;}
	//是否是合法逻辑位置
	bool IsTrueDesk(BYTE bDesk){return bDesk < PLAY_COUNT;}
	//整理手牌(牌墙不允许用这个函数)
	bool TidyMJ(UDT_MAP_MJ_DATA &card);
	//手牌排序
	//BYTE bDesk 玩家逻辑位置
	//bool bAscendingOrder 是否升序
	bool SortMJ(UDT_MAP_MJ_DATA &card, bool bAscendingOrder=true);

	//是万牌
	bool IsWan(BYTE uCard);
	//是筒牌
	bool IsTong(BYTE uCard);
	//是条牌
	bool IsTiao(BYTE uCard);
	//是风牌
	bool IsFeng(BYTE uCard);
	//是箭牌
	bool IsJian(BYTE uCard);
	//是花牌
	bool IsHua(BYTE uCard);
	//是合法牌
	bool IsLegal(BYTE uCard);
	//获得牌张数(排除掉值为0的牌)
	BYTE GetCardNum(UDT_MAP_MJ_DATA card);
	//在一组牌中获得指定牌张数
	BYTE GetSameCardNum(UDT_MAP_MJ_DATA card, BYTE uCard);
	//两张牌是否为同类型牌
	bool IsSameKind(BYTE uCardA, BYTE uCardB);
	//获得该牌的牌类型
	BYTE GetKind(BYTE uCard);
	//提取一组牌中第几张牌
	BYTE GetCard(UDT_MAP_MJ_DATA &card, BYTE index);
	//删除一张在一组牌中的指定的牌
	bool DeleteCard(UDT_MAP_MJ_DATA &card, BYTE uCard);
	//为指定的一组牌添加牌
	bool AddCard(UDT_MAP_MJ_DATA &card, BYTE uCard);
	//将一组牌中的一张牌换成指定的一张牌
	//BYTE uCard 手牌中存在的牌
	//BYTE uChangeCard 希望换成什么样的牌
	bool ChangeCard(UDT_MAP_MJ_DATA &card, BYTE uCard, BYTE uChangeCard);

	//增加当前可以拦牌人数
	bool SetbNowCanBreakNum(bool bAdd);
	//获得当前可以拦牌人数
	BYTE GetbNowCanBreakNum(){return m_bNowCanBreakNum;}
	//初始化当前可以拦牌人数
	bool InitbNowCanBreakNum(){m_bNowCanBreakNum = 0; return true;}
	//生成随机数
	int MyRand();
private:
	BYTE m_uFirstHandCard;//起手手牌张数
public:
	long int m_UserID[PLAY_COUNT];//玩家userid需要记录下来,为了判定连庄
	long int m_iBankerID[2];//庄家(元素1记录庄家逻辑位置，元素2记录庄家userid)
	MJ_USER_CARD m_CardInfo[PLAY_COUNT];//每个方位所有牌信息
	//成员变量
	//等待阶段
	CMyWaitLogic m_MyWaitLogic;
	//定庄阶段
	CMyFixBankerLogic m_MyFixBankerLogic;
	//发牌阶段
	CMyDealLogic m_MyDealLogic;
	//补花阶段
	CMyBuHuaLogic m_MyBuHuaLogic;
	//出牌阶段
	CMyShowCardLogic m_MyShowCardLogic;
	//胡牌阶段
	CMyHuLogic m_MyHuLogic;
	//结算阶段
	CMyResultLogic m_MyResultLogic;

	UDT_MAP_MJ_DATA m_MJDataMap;//麻将子数据

	MJ_DESK_DATABASE m_MJ_Desk_Database;//桌面上所有麻将的宏观信息

	MJ_SHAIZI_DATABASE m_MJ_ShaiZi_Database;//色子数据

	MJ_TAKE_CARD_DATABASE m_MJ_TakeCardPortInfo;//当前摸牌所在牌墙位置信息

	MJ_WILD_CARDINFO m_WildCardInfo;//万能牌信息
	
	//每局一开始摸牌的位置（方位和第几张）
	MJ_FIRST_DEAL_POSTINFO m_first_PostInfo;

	//抓牌墙牌顺序默认顺时针0
	bool m_DrawOrder;

	//打牌顺序默认逆时针1
	bool m_OutOrder;

	// 连续局数
	BYTE m_nRunNbr;

	// 连庄数
	BYTE m_nBankerNbr;

	//上把赢了的玩家
	BYTE m_iWiner;//默认为一个赢了的人，如果有多个人上把赢了，选打牌顺序第一个胡了的人为赢家

	BYTE m_iNowPlayer;//当前出牌玩家//出牌阶段（拦牌，出牌，摸牌）的时候用到
	bool m_bCanOutCard[PLAY_COUNT];//可以出牌
	BYTE m_uNumForTun;//牌墙码牌时是几张为一屯//默认麻将墙是两个上下叠着为一屯的

	//bool m_bOnlyOutBigCard[PLAY_COUNT];//当前只能打大牌
	bool m_bOnlyOutBigCard;//当前只能打大牌
	//胡牌方式
	bool m_bKindForHu;//自摸(false)还是放炮胡(true)的
	//玩家能否整个一局内放炮胡
	bool m_bCanGun[PLAY_COUNT];
	//玩家能否放炮胡,一圈内判断
	bool m_bCanGunForRound[PLAY_COUNT];
	//胡牌类型
	BYTE m_bHuKind[PLAY_COUNT];
	//是否为抢杠胡
	bool m_bIsQiangGangHu[PLAY_COUNT];
	//胡的将牌是什么牌
	BYTE m_bJiangCard[PLAY_COUNT];

	BYTE m_DeFen[2];//麻将底分，元素0庄家底分，元素1闲家底分

	Mess_Result m_UserResultInfo[PLAY_COUNT];//结果数据

	//是否起手（判断玩家一开始摸到牌可以听牌胡牌用）
	bool m_bFirstTag;

	//玩家听牌状态
	bool m_bTing[PLAY_COUNT];

	MJ_TakeWildCardInfo m_TakeWildCardInfo;//在摸万能牌时所留下的记忆

	//出牌思考时间
	BYTE m_bOutCardTime;
	//拦牌思考时间
	BYTE m_bBreakCardTime;
	//等待开始时间
	BYTE m_bWaitBeginTime;

	//打牌的时候，当前可以拦牌人数
	BYTE m_bNowCanBreakNum;
	//当前一圈内所有玩家拦牌数据
	BaseMess_BreakCard m_BreakCard[PLAY_COUNT];

};