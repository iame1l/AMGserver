#pragma once
#include "GlobalData.h"
#pragma pack(1)

//#define MESS_FANID_NUM 50//暂时定义50个番型数目供数据包使用
#define MESS_GANG_NUM 5  //可能的杠牌数目
#define MESS_CHI_NUM  5  //可能的吃牌数目
#define MESS_PENG_NUM 5  //可能的碰牌数目
#define MESS_KE_NUM	  5  //可能的暗刻数目（三张相同的牌）
#define MESS_HUA_NUM  Eie_BASE_HUA_NUM  //可能的花牌数目

//游戏结束状态
enum _E_MJ_BASE_GAMEFINISH_ID
{
	_E_GF_NORMAL=10,//正常游戏结束
	_E_GF_SAFE,//安全结束，什么都不处理
	_E_GF_FORCE//强退结束
};

//游戏基本的消息ID

enum _E_MJ_BASE_MESSID
{
	_E_MJ_ASS_GAME_BEGIN=30,	//游戏开始
	_E_MJ_ASS_FIRST_STAT_SHAIZI,//首次定庄的色子信息
	_E_MJ_ASS_GAME_INITDATA,	//初始数据
	_E_MJ_ASS_BU_HUA,			//补花
	_E_MJ_ASS_TAKE_CARD,		//摸牌
	_E_MJ_ASS_OUT_CARD,			//出牌
	_E_MJ_ASS_OUT_CARD_RES,		//出牌结果
	_E_MJ_ASS_BREAK_CARD,		//拦牌
	_E_MJ_ASS_BREAK_CARD_RES,	//拦牌结果
	_E_MJ_ASS_UPDATE_HAND,		//更新手牌
	_E_MJ_ASS_GAMEFINISH,		//游戏结束
	_E_MJ_ASS_FAN_INFO,			//番消息
	_E_MJ_ASS_BREAK_HU,			//胡牌
	_E_MJ_ASS_NUM				//数目
};

//开始消息包
struct BaseMess_GameBegin
{
	BaseMess_GameBegin()
	{
		memset(this,0,sizeof(BaseMess_GameBegin));
	}
};

//首次定庄用的色子信息
struct BaseMess_FirstBankerShaiZi
{
	BYTE ShaiZi[2];//一次扔两个色子
	//庄家
	BYTE uBankerID;
	void init()
	{
		memset(this,0,sizeof(BaseMess_FirstBankerShaiZi));
	}

	BaseMess_FirstBankerShaiZi()
	{
		init();
	}
};

//初始化数据消息包
struct BaseMess_InitData
{
	//连局情况
	BYTE nRunNbr;
	//连庄情况
	BYTE nBankerNbr;
	//庄家
	BYTE uBankerID;
	//跳牌位置
	BYTE uPosition;//方位
	BYTE uFirstPort;//第几张
	//初始摸牌结束位置
	BYTE uEndPosition;//方位
	BYTE uEndPort;//第几张

	BaseMess_InitData()
	{
		memset(this,0,sizeof(BaseMess_InitData));
	}
};

//摸牌数据包
struct BaseMess_TakeCard
{
	bool bHu;//是否能胡牌
	bool bGang;//自己摸的牌只能补杠或者暗杠
	BYTE bDesk;//玩家
	BYTE bCard;//摸到的牌
	BYTE uPosition;//摸的方位
	BYTE uPort;//摸的第几张

	BaseMess_TakeCard()
	{
		memset(this,0,sizeof(BaseMess_TakeCard));
		bDesk = 255;
		bHu=false;
		bGang=false;
	}
};

//通知出牌数据包（还得带上玩家自己可以对自己拦牌的信息）
struct BaseMess_OutCard
{
	BYTE bDesk;//出牌玩家
	bool bCanBreakSelf[Eie_BREAK_NUM];//自己拦自己的牌
	MJ_EAT eat[3];//吃只有三种情况
	bool bkindGang;//补杠true,明杠或暗杠false
	BYTE bResCard;//听会确定目标牌(听找出的是去除掉就可以听的牌)
	void init()
	{
		memset(this,0,sizeof(BaseMess_OutCard));
		bDesk = 255;
		eat[0].init();
		eat[1].init();
		eat[2].init();
	}

	BaseMess_OutCard()
	{
		init();
	}
};

//出牌结果数据包
struct BaseMess_OutCardResult
{
	BYTE bDesk;//出牌玩家
	BYTE bCard;//打出的牌
	bool bCanBreak[Eie_BREAK_NUM];//(对接受消息的其他人来说)可以拦牌
	MJ_EAT eat[3];//(对接受消息的其他人来说)吃只有三种情况
	bool bkindGang;//补杠true,明杠或暗杠false
	bool Auto;//是否是自动出牌
	void init()
	{
		memset(this,0,sizeof(BaseMess_OutCardResult));
		memset(bCanBreak,false,sizeof(bCanBreak));
		bDesk = 255;
	}

	BaseMess_OutCardResult()
	{
		init();
	}
};

//拦牌数据包
struct BaseMess_BreakCard
{
	bool bFetchCard;//是否摸牌后的操作
	BYTE bDesk;//发起拦牌者
	BYTE uOutDesk;//被拦牌者
	BYTE uCard;//所拦的牌
	BYTE kind;//拦牌类型
	MJ_EAT eat;//如果吃牌，吃牌详细信息
	bool bkindGang;//补杠true,明杠或暗杠false
	BYTE bResCard;//听会确定目标牌(听找出的是去除掉就可以听的牌)
	void init()
	{
		memset(this,0,sizeof(BaseMess_BreakCard));
		bDesk = 255;
		uOutDesk = 255;
		bFetchCard=false;
		eat.init();
	}
	BaseMess_BreakCard()
	{
		init();
	}
};

//刷新手牌数据包
struct BaseMess_UpdateHand
{
	BaseMess_UpdateHand()
	{
		memset(this,0,sizeof(BaseMess_UpdateHand));
	}
};

//每位玩家结算的结构体
struct BaseMess_Result
{
	bool bWin;//是否赢了
	BYTE bHuKind;//是点炮还是自摸
	BYTE bWhoGun;//谁放的炮
	bool bQiangGangHu;//是否是抢杠胡
	BYTE uEat[MESS_CHI_NUM][3];  //吃牌数目
	BYTE uEatNum;
	BYTE uPeng[MESS_PENG_NUM];   //碰牌数目
	BYTE uPengNum;
	MJ_GANG uGang[MESS_GANG_NUM];//杠牌数目(明杠暗杠补杠)
	BYTE uGangNum;
	BYTE uKe[MESS_KE_NUM];		 //暗刻数目
	BYTE uKeNum;
	BYTE uHua[MESS_HUA_NUM];	 //花牌数目
	BYTE uHuaNum;
	__int64 ipoint;				//总得分
	__int64	iChangeMoney;		//玩家金币改变

	void init()
	{
		bHuKind = Eie_HU_ZIMO;
		bQiangGangHu = false;
		bWin = false;
		ipoint = 0;
		iChangeMoney = 0;
		memset(uEat,0,sizeof(uEat));
		uEatNum = 0;
		memset(uPeng,0,sizeof(uPeng));
		uPengNum = 0;
		memset(uGang,0,sizeof(uGang));
		uGangNum = 0;
		memset(uKe,0,sizeof(uKe));
		uKeNum = 0;
		memset(uHua,0,sizeof(uHua));
		uHuaNum = 0;
		bWhoGun = 255;
	}

	BaseMess_Result()
	{
		init();
	}
};
//拦牌数据包
struct Message_BreakCard
{
	bool bHu;//是否能胡牌
	BYTE bDesk;//玩家
	Message_BreakCard()
	{
		bDesk = 255;
		bHu=false;
	}
};
#pragma pack()