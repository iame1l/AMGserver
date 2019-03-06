
// ******************************************************************************************************
//  UpgradeMessage   version:  1.0   ·  date: 03/29/2011
//  --------------------------------------------------------------------------------------------------

//	德克萨斯扑克，游戏服务器-客户端协议\
NameID：51300280

//  --------------------------------------------------------------------------------------------------
//  Copyright (C) 2011 - All Rights Reserved
// ******************************************************************************************************
// 
// ******************************************************************************************************

#pragma once

#ifndef AFC_SERVER_DKSSPK_11302800_MESSAGE_HEAD_FILE
#define AFC_SERVER_DKSSPK_11302800_MESSAGE_HEAD_FILE

#define SUPER_PLAYER                    10				// 超级玩家

//文件名字定义
#define GAMENAME						TEXT("德克萨斯扑克")

//版本定义
#define GAME_MAX_VER					1				// 现在最高版本
#define GAME_LESS_VER					1				// 现在最低版本
#define GAME_CHANGE_VER					0				// 修改版本

//游戏开发版本
#define DEV_HEIGHT_VERSION				10				// 内部开发高版本号
#define DEV_LOW_VERSION					1				// 内部开发低版本号

//支持类型
#define SUPPER_TYPE						SUP_NORMAL_GAME|SUP_MATCH_GAME|SUP_MONEY_GAME

//游戏信息
#define NAME_ID							10100008		// 名字 ID
#define PLAY_COUNT						8				// 游戏人数
#define GET_SERVER_INI_NAME(str)		sprintf(str,"%s%d_s.ini",CINIFile::GetAppPath(),NAME_ID);
#define MAX_NAME_INFO                   256

//游戏数据长度相关
#define MAX_CARDS_NUM					216				// 四副牌的总牌数，也是所支持最大的牌数
#define MAX_HANDCARDS_NUM				52				// 手上持有的最大的牌张数
#define MAX_510K_CARDS_NUM				48				// 四副牌5/10/K最大的牌张数
#define MAX_FIRST_CARDS					1				// 发牌数量
#define MAX_DEAL_CARDS					2				// 发牌数量
#define MAX_THREE_CARDS					3				// 发牌数量
#define MAX_BACKCARDS_NUM				16				// 最大的底牌张数

#define TYPE_PLAY_GAME					10000			// 是玩家

#define MAX_GAME_TURNS					64



#define TAG_INIT(tagName) \
    tagName(){Clear();} \
    void Clear()

//Num Defs
#define NUM_HAND_CARDS_NUM				2//手牌数量
#define NUM_COMMUNITY_CARDS_NUM			5//公共牌数量
#define NUM_CARDS_NUM					52//使用牌的数量

#define NUM_ONE_SECOND_MS				1000//1s=1000ms


//******************************************************************************************
//数据包处理辅助标识
//******************************************************************************************

#define ASS_UG_USER_SET					50				//用户设置游戏(未使用)
#define ASS_UG_USER_SET_CHANGE			51				//玩家设置更改(未使用)

#define ASS_GAME_BEGIN					120				//游戏开始
#define ASS_SYS_DESK_CFG				121				//系统设定桌面配置
#define ASS_CFG_DESK					122				//玩家设置底注
#define ASS_DESK_CFG					123				//玩家设定桌面配置
#define ASS_AGREE_DESK_CFG				124             //同意桌面配置(使用，但无影响)
#define ASS_BET							125             //玩家下注
#define ASS_SEND_A_CARD					130				//发底牌2张
#define ASS_SEND_3_CARD					131				//发翻牌3张
#define ASS_SEND_4_5_CARD				132				//发转牌或者河牌1张
#define ASS_SEND_CARD_FINISH			133				//发牌结束
#define ASS_BETPOOL_UP					134				//边池更新
#define ASS_BET_REQ						135				//下注请求(未使用)
#define ASS_BET_INFO					136				//下注消息
#define ASS_NEW_USER					137				//下一个玩家操作(未使用)
#define ASS_PASS_USER					138				//弃牌的玩家操作(未使用)
#define ASS_SUB_MENOY                   139             //代入金币限制
#define ASS_COMPARE_CARD                140             //比牌信息

#define ASS_CALLMANDATE				    141				//跟注托管(未使用)
#define ASS_CHECKMANDATE                142             //过牌托管(未使用)
#define ASS_CALLANYMANDATE              143				//跟任何注托管(未使用)
#define ASS_PASSABANDONMANDATE          144             //过牌或弃牌托管(未使用)

#define ASS_USER_LEFT_DESK              145             //用户离开

#define ASS_SORT_OUT					170				//清理图片

#define ASS_TOKEN						186				//令牌信息
#define ASS_REMAINING_GAME_COUNT		189				//剩余局数
#define ASS_CALCULATE_BOARD				190				//大结算
#define ASS_RESULT						191				//回合算分
#define ASS_SHOW_IN_MONEY				192				//显示代入金额
#define ASS_LOOK_CARD					193				//看牌(暂未使用)
#define ASS_CAN_SHOW_CARD				194				//能展示牌的消息(暂未使用)
#define ASS_SHOW_CARD					195				//展示牌的消息(暂未使用)


//房卡场新增消息
#define ASS_RC_OPTION					200				//房卡选项
#define ASS_RC_CUR_MONEY				201				//当前货币
#define ASS_RC_CUR_CAN_BUYIN_REQ		202				//当前可买入请求[暂时无效]
#define ASS_RC_CUR_CAN_BUYIN_RSP		203				//当前可买入应答[暂时无效]
#define ASS_RC_CUR_BUYIN_REQ			204				//当前买入请求[暂时无效]
#define ASS_RC_CUR_BUYIN_RSP			205				//当前买入应答[暂时无效]
#define ASS_RC_CANNOT_PLAY_LACK_MONEY	206				//缺乏货币不能玩耍[暂时无效]

//新增新超端消息
#define ASS_SUPER_SET_CARD_REQ			220				//超端设置牌请求
#define ASS_SUPER_SET_CARD_RSP			221				//超端设置牌应答

#define ASS_USERS_WIN_TOTAL_INFO		230				//玩家输赢统计信息

#define ASS_SET_ACTION_PROB	            240				//设定行为概率

//异常类消息
#define ASS_NO_PLAYER					155				//没有玩家进行游戏
#define ASS_AGREE						156				//玩家是否同意的消息
#define ASS_CFG_DESK_TIMEOUT			157				//设置底注超时
#define ASS_NOT_ENOUGH_MONEY			158				//玩家金币不足的清除玩家信息
#define ASS_MONEY_LACK					159				//玩家金币不足的提示

#define ASS_NEXT_ROUND_REQ				150				//玩家下一回合请求
#define ASS_NEW_ROUND_BEGIN				151				//下一回合开始


//调试类消息
#define ASS_FINISH_ROUND_REQ			180				//结束本轮游戏

#define S_C_IS_SUPER_SIG				181				//是否是超端
#define S_C_SUPER_SET_RESULT_SIG		182				//超端设置结果

#define C_S_SUPER_SET_SIG				190				//设置下盘赢得位置

//******************************************************************************************
// 下注类型
enum emType
{
    ET_UNKNOWN 		= 0,	//未知类型
    ET_BET			= 1,	//下注
    ET_CALL			= 2,	//跟注
    ET_ADDNOTE 		= 3,	//加注
    ET_CHECK		= 4,	//过牌
    ET_FOLD    		= 5,    //弃牌
    ET_ALLIN		= 6,	//全下
    ET_AUTO			= 7,    //自动下注
};

// 发牌类型
enum emTypeCard
{
    UNKNOWN			= 0,	//未知类型
    SEND_A_CAND		= 1,	//下发一张牌牌
    SEND_3_CAND		= 2,	//下发 3 张牌牌
    SEND_4_5_CAND   = 3,	//下发 4, 5 张牌牌
};

// 代入金额类型
enum emTypeMoney
{
	NT_UNKNOWN	= 0,	//未知类型
	NT_LOAD		= 1,	//确定代入金额
	NT_EXIT		= 2,    //退出游戏
	NT_ADD		= 3,    //增加金额
	NT_SUB		= 4,    //减少金额
};

// 托管状态
enum emToken
{
	TO_UNKNOWN					= 0,	// 未知类型
	TO_CALLMANDATE				= 1,	// 跟注托管按钮
	TO_CHECKMANDATE				= 2,    // 过牌托管按钮
	TO_CALLANYMANDATE			= 3,    // 跟任何注按钮
	TO_PASSABANDONMANDAT		= 4,    // 过牌/弃牌托管按钮
};

//Enum Def
enum ENUM_SET_ACTION_STATUS
{
	ENUM_SET_ACTION_STATUS_Big = 0,
	ENUM_SET_ACTION_STATUS_Small,
	ENUM_SET_ACTION_STATUS_Max,
};

#pragma pack(1)

//通知设置游戏包
struct TCfgDesk
{
    //底注相关
    struct TDizhu
    {
        bool	bCanSet;		//玩家可以是否可以设置，下发时用
        int		iLowers[5];		//存放5个可选的底注值，下发时用
        bool    bSelects[5];	//这5个值的可选择性
        BYTE	bySelect;		//玩家从iValues选择了哪一个底注值，上发时用

        void Clear()
        {
            bCanSet = false;	//玩家可以是否可以设置
            memset(iLowers,0,sizeof(iLowers));		 //存放三个可选的底注值
            memset(bSelects,false,sizeof(bSelects)); //这三个值的可选择性
            bySelect = 255;	//玩家从iValues选择了哪一个底注值
        }

    } dz;

    TAG_INIT(TCfgDesk)
    {
        dz.Clear();
    }

};

//有关桌面配置
struct TDeskCfg
{
    //底注相关
    struct TDizhu
    {
        __int64	iLowers[5];						// 存放5个可选的底注值
        __int64	iUppers[5];						// 存放5个顶注值
        __int64 iRoomMultiple;                  // 房间倍数
        __int64	iLower;							// 系统自动设置的底注值
        __int64	iUpper;							// 系统自动设置的顶注值
        __int64	iSmallBlindNote;				// 小盲注设置的底注值
        __int64	iBigBlindNote;					// 大盲注设置的底注值

        // 代入金币设置
        bool    bIsRoomSubMoney;                // 此房间是否需要代入金币才可玩
        __int64 iSubMinMoney;                   // 本房间最低代入金币
        __int64 iSubMaxMoney;                   // 本房间最高代入金币 
		__int64	iSubPlanMoney;					// 当前准备代入金币
		__int64	iMinusMoney;					// 修改注额加减金币
		__int64	iMinMoney;						// 最小下注金币
		__int64	iUntenMoney;					// 少于多少金额时带入 

		bool iUserClose;						// 一局后10秒内将玩家强退
		int iUserCloseTime;						// 关闭客户端时间
		int iUserInMoney;						// 游戏金额带入时间

		emTypeMoney nType;						// 请求包类型

        void Clear()
        {
            memset(iLowers,0,sizeof(iLowers));	// 存放5个可选的底注值
            memset(iUppers,0,sizeof(iUppers));	// 存放5个顶注值
            iLower = 0;							// 系统自动设置的底注值
            iUpper = 0;							// 系统自动设置的顶注值
            iSmallBlindNote = 0;				// 小盲注设置的底注值
            iBigBlindNote = 0;					// 大盲注设置的底注值
			nType = NT_UNKNOWN;					// 未知类型
        }

    } dz;

    //时间相关
    struct TTime
    {
        BYTE   byOperate;    	 // 玩家操作的时间

        void Clear()
        {
            byOperate = 0;	 	 // 玩家操作的时间
        }
    }Time;

    //规则相关
    struct TRule
    {
        int  nPokeNum;			//使用多少幅扑克
        void Clear()
        {
            nPokeNum = 1;		//使用多少幅扑克
        }

    }Rule;

    //其它共性
    bool bShowUserInfo;			// 是否显示玩家和身份，多用于比赛场

    TAG_INIT(TDeskCfg)
    {
        dz.Clear();
        Time.Clear();
        Rule.Clear();
    }

};

//某玩家是否意底注的消息包
struct TAgree 
{
    BYTE byUser; 				// 消息所属的玩家
    bool bAgree; 				// 是否同意
    bool bAllAgree; 			// 当前是否所有玩家都同意游戏

    TAG_INIT(TAgree)
    {
        byUser = 255;			// 消息所属的玩家
        bAgree = true;			// 是否同意
        bAllAgree = false;		// 当前是否所有玩家都同意游戏
    }
};

//请求下注包
struct TBet
{
    emType	nType;				// 下注类型
    BYTE	byUser;				// 下注的玩家
    BYTE	bUserman[8];			// 弃牌玩家
    __int64 nMoney;				// 金币
	__int64	nBetMoney;			// 下注最大的玩家

    TAG_INIT(TBet)
    { 
        nType = ET_UNKNOWN;		// 下注类型
        byUser = 255;			// 下注的玩家
        //bUserman = 255;			// 弃牌玩家
		memset(bUserman,0,sizeof(bUserman));
        nMoney = 0;				// 玩家的选择
		nBetMoney = 0;			// 下注最大的玩家
    }
};

// 代入金币
struct TSubMoney
{
	BYTE	bDeskStation;
	__int64 nMoney[PLAY_COUNT];
	bool	bIn[PLAY_COUNT];       // 准备状态
	
	TSubMoney()
	{
		memset(nMoney, 0, sizeof(nMoney));
		memset(bIn, 0, sizeof(bIn));
		bDeskStation = 255;
	}
}rmb;


// 比牌数据包
struct TCompareCard
{
    int  nCardKind[PLAY_COUNT];		 // 玩家牌型
    BYTE bCards[PLAY_COUNT][5];		 // 所有玩家组成的牌数据
	BYTE bCardsEx[PLAY_COUNT][2];    // 剩下2张没用的牌
	BYTE bHandCards[PLAY_COUNT][2];  // 玩家手牌
	int nCardCount;                  // 组合牌张数
    int nHandCardCount[PLAY_COUNT];  // 玩家手牌张数

    TCompareCard()
    {
        memset(this, 0, sizeof(TCompareCard));
		memset(bHandCards, 255, sizeof(bHandCards));
    }
};

//用于下发牌数据给客户端的数据包
struct TCards
{
    emTypeCard		nTypeCard;					//发牌类型
    BYTE   			byUser;						//该牌发给的玩家
	BYTE			byCards[PLAY_COUNT][MAX_DEAL_CARDS];	//底牌牌数据
	BYTE			byPubCards[5];				//公共牌牌数据
    int				iCardsNum;					//各玩家手上牌的数量

    TAG_INIT(TCards)
    {
        nTypeCard	= UNKNOWN;					//发牌类型
        byUser		= 255;						//该牌发给的玩家
		memset(byCards,0,sizeof(byCards));		//牌数据
		memset(byPubCards,0,sizeof(byPubCards));//牌数据
        iCardsNum = 0;							//玩家手上牌的数量
    }
};

//令牌包，用来激活用户进行活动，如出牌等等
struct TToken
{
    BYTE	  byUser;									// 所属玩家
    BYTE	  byVerbFlag;								// 允许动作标志
    int		  iSelPoints[5];							// 加注值选择表
	emToken	  nemToken;									// 托管状态类型
    bool	  bNewTurn;                                 // 是否为新的一轮开始
	bool	  bNewTurns;                                // 是否为新的一轮开始
	int		  nTurnNums;								// 当前游戏活动的圈数
	__int64   nCallMoney;                               // 当前可以跟注的金币
	__int64   iMinBetOrRaiseMoney;						// 最小下注/加注的金币
	__int64   iMaxBetOrRaiseMoney;						// 最大下注/加注的金币(暂时不使用)

    TAG_INIT(TToken)
    {
		nemToken = TO_UNKNOWN;						// 未知类型
        byUser = 255;								// 所属玩家
        byVerbFlag = 0;								// 允许动作标志
        bNewTurn = false;
		bNewTurns = false;
		nCallMoney = 0;
		nTurnNums = 0;
        memset(iSelPoints,0,sizeof(iSelPoints));	// 加注值选择表
		iMinBetOrRaiseMoney = 0;
		iMaxBetOrRaiseMoney = 0;
    }
};

// 玩家下注边池更新包
struct TBetPool
{
    __int64 iBetPools[8];			//下注边池

    TAG_INIT(TBetPool)
    {
        memset(iBetPools, 0, sizeof(iBetPools));  //边池初始化
    }
};


//结算结构包
struct TResult
{
	int  nbyUserID[PLAY_COUNT];				//赢家ID
	char szName[8][61];						//赢家昵称
    __int64  nBetPools[8];					//所有下注池
    __int64  nUserBet[PLAY_COUNT][8];		//每个玩家下注数据
	bool bWinBetPools[PLAY_COUNT][8];		//赢的下注池
	__int64  nWinPoolsMoney[PLAY_COUNT][8];	//赢的下注池金币
    __int64  nScore[PLAY_COUNT];     		//输赢积分
    __int64  nMoney[PLAY_COUNT];     		//实际输赢金币
	__int64  nMoneyEx[PLAY_COUNT];          //输赢金币 
    __int64  nSubMoney[PLAY_COUNT];  		//当前每个玩家代入金币
    __int64  nSelfMoney[PLAY_COUNT]; 		//自己的金币更新: 客户端获取的金币有可能还没有刷新, 所以在这里发下去
	//__int64  iTotalMoney[PLAY_COUNT];		//玩家累计输赢
	bool bWin[PLAY_COUNT];       			//记录赢家
	bool bExit;                  			//true:玩家需要退出-, false:不需要退出
	BYTE bOrder[PLAY_COUNT];                //玩家排名, 按金币最多的排
	BYTE mRemainingGameCount;				// 剩余局数

    TResult()
    {
        ::memset(this, 0, sizeof(TResult));
		::memset(nUserBet, 0, sizeof(nUserBet));
		::memset(nMoneyEx, 0, sizeof(nMoneyEx));
		::memset(bOrder, 255, sizeof(bOrder));
		mRemainingGameCount = 0;
    }
};

struct CalculateBoardData
{
	int  iUserID;				//玩家ID
	//char szName[61];			//玩家昵称
	//char szHeadUrl[256];		//头像URL
	//bool gameFinish;			// 是否游戏结束
	bool bWinner;				// 是否大赢家
	__int64 iMaxWinMoney;		// 单据最多赢钱数
	int iMaxContinueCount;		// 最高连胜数
	int iWinCount;				// 胜局
	__int64 i64WinMoney;		// 输赢金币

	TAG_INIT(CalculateBoardData)
	{
		memset(this, 0, sizeof(*this));
	}
};

struct RemainingGameCount
{
	int gameCount;
};

//新的一回合请求包
struct TNextRoundReq
{
    enum emType
    {
        UNKNOWN		= 0,	//未知类型
        READY		= 1,	//准备好
        EXIT		= 2,    //想退出游戏
    };

    emType nType;			//请求包类型

    TAG_INIT(TNextRoundReq)
    {
        nType = UNKNOWN;	//未知类型
    }
};

//新的一回合消息包
struct TNextRoundBeginInfo
{
    BYTE byNTUser;		//庄家位置
    BYTE bSmallBlind;	//小盲注
    BYTE bBigBlind;		//大盲注

    TAG_INIT(TNextRoundBeginInfo)
    {
        ::memset(this,0,sizeof(TNextRoundBeginInfo));
        byNTUser = 255;
        bSmallBlind = 255;
        bBigBlind = 255;
    }
};

//没有玩家在桌的通知
struct TNoPlayer
{
    bool bGameFinished;	//桌子已散掉

    TAG_INIT(TNoPlayer)
    {
        bGameFinished = false;	//桌子已散掉
    }
};

// 玩家金额不足
struct TMoneyLack
{
	int nbyUser;			// 金额不足玩家

	TAG_INIT(TMoneyLack)
	{
		nbyUser = 255;
	}
};

//Added by QiWang 20180329
//房卡选项带入倍率
struct TRCOptionLimit
{
	__int64	iSmallBlindNoteMax;		//最大小盲注
	__int64	iAllInMoneySmallRate;	//AllIn最低倍率(大盲注基数)
	__int64	iAllInMoneyBigRate;		//AllIn最高倍率(小盲注基数)

	TAG_INIT(TRCOptionLimit)
	{
		memset(this, 0, sizeof(*this));
	}
};

//Added by QiWang 20180313, 房卡场新增消息对应结构体
//房卡选项
struct TRCOption
{
	int iPlayerCount;			//人数,暂时无效
	__int64	iSmallBlindNote;	//小盲注
	__int64	iBigBlindNote;		//大盲注=小盲注*2
	__int64	iBaseTakeInMoney;	//记分带入,暂时无效
	__int64	iSmallTakeInMoney;	//最低带入,暂时无效
	__int64	iBigTakeInMoney;	//最高带入,暂时无效
	__int64	iAllInMoneyLimit;	//AllIn限制金额

	TAG_INIT(TRCOption)
	{
		memset(this, 0, sizeof(*this));
	}
};

struct TRCCurMoneyInfo
{
	__int64	iCurMoney;				//当前货币[包含此次买入]
	__int64	iHadBuyInMoney;			//已买入[包含此次买入],暂时无效
	__int64	iTheBuyInMoney;			//此次买入[显示动画使用],暂时无效
	__int64	iNextRoundBuyInMoney;	//下把准备买入,暂时无效

	TAG_INIT(TRCCurMoneyInfo)
	{
		memset(this, 0, sizeof(*this));
	}
};

//当前货币
struct TRCCurMoney
{
	TRCCurMoneyInfo tCurMoneyInfoArr[PLAY_COUNT];

	TAG_INIT(TRCCurMoney)
	{
		memset(this, 0, sizeof(*this));
	}
};

//当前可买入请求, !!不使用!!
struct TRCCurCanBuyInReq
{
	int nbyUser;

	TAG_INIT(TRCCurCanBuyInReq)
	{
		memset(this, 0, sizeof(*this));
	}
};

//当前可买入应答
struct TRCCurCanBuyInRsp
{
	__int64	iSmallBuyInMoney;		//最低买入
	__int64	iBigBuyInMoney;			//最高买入
	__int64	iHadBuyInMoney;			//已买入

	TAG_INIT(TRCCurCanBuyInRsp)
	{
		memset(this, 0, sizeof(*this));
	}
};

//当前买入请求
struct TRCCurBuyInReq
{
	//请求值为房卡选项最低带入的整数倍
	//否则服务器自动寻找(低于该值的最低带入的最大整数倍)或者(剩余可买入)
	__int64	iBuyInMoney;		//买入

	TAG_INIT(TRCCurBuyInReq)
	{
		memset(this, 0, sizeof(*this));
	}
};

//当前买入应答
struct TRCCurBuyInRsp
{
	int nbyUser;				//买入位置
	bool bBuyInSuccess;			//买入成功与否
	__int64	iBuyInMoney;		//买入货币
	bool bEffectImediately;		//是否立即生效
								//[游戏未开始时买入会立即生效]
								//[立即生效会广播，否则单独回复]
	__int64	iCurMoneyBuyInUser;	//买入位置当前货币[立即生效时，玩家的当前货币会变动]

	TAG_INIT(TRCCurBuyInRsp)
	{
		memset(this, 0, sizeof(*this));
	}
};

//买入相关信息
struct TRCCurBuyInInfo
{
	TRCCurCanBuyInRsp tRCCurCanBuyIn;
	__int64	iLimitBuyInMoney;		//买入限制
	__int64	iNextRoundBuyInMoney;	//下把准备买入

	TAG_INIT(TRCCurBuyInInfo)
	{
		memset(this, 0, sizeof(*this));
	}
};

//Added by QiWang 20180411, 新增新超端消息
//底牌、手牌结构体
struct THandCards
{
	int  iUserID;	//玩家ID
	bool bSetHandCard; //设定玩家手牌与否(因为玩家ID可以是任意值)
	BYTE byCard[NUM_HAND_CARDS_NUM]; //手牌
	//BYTE byNumSetHandCard; //设定手牌张数(1-NUM_HAND_CARDS_NUM,其他值均认为是NUM_HAND_CARDS_NUM)

	TAG_INIT(THandCards)
	{
		memset(this, 0, sizeof(*this));
		iUserID = -1;
		//byNumSetHandCard = 1;
	}
};

//超端设置牌请求
struct TSuperSetCardReq
{
	THandCards tPlayerHandCards[PLAY_COUNT]; //所有玩家手牌
	BYTE byComCards[NUM_COMMUNITY_CARDS_NUM]; //设定公共牌
	//BYTE byNumSetComCards; //设定公共牌张数(1-NUM_COMMUNITY_CARDS_NUM,其他值均认为是NUM_COMMUNITY_CARDS_NUM)

	TAG_INIT(TSuperSetCardReq)
	{
		memset(this, 0, sizeof(*this));
	}
};

enum ENUM_SuperSetCard_ErrorReason
{
	ENUM_SuperSetCard_ErrorReason_RepeatedCard = 1, //重复牌
	ENUM_SuperSetCard_ErrorReason_ErrorHandCard,	//错误手牌
	ENUM_SuperSetCard_ErrorReason_ErrorComCard,		//错误公共牌
	ENUM_SuperSetCard_ErrorReason_Max,//未知错误
};
//超端设置牌应答
struct TSuperSetCardRsp
{
	bool bSuccess; //设定成功与否
	int iErrorReason; //设定失败原因

	TAG_INIT(TSuperSetCardRsp)
	{
		memset(this, 0, sizeof(*this));
	}
};

//玩家输赢统计信息
struct TUersWinTotalInfo
{
	__int64 iTotalMoney[PLAY_COUNT]; //玩家累计输赢

	TAG_INIT(TUersWinTotalInfo)
	{
		memset(this, 0, sizeof(*this));
	}
};

//设定行为概率
struct SetActionProb
{
	BYTE byProbFold[ENUM_SET_ACTION_STATUS_Max];	//弃牌概率
	BYTE byProbFollow[ENUM_SET_ACTION_STATUS_Max];	//跟注概率
	BYTE byProbRaise[ENUM_SET_ACTION_STATUS_Max];	//加注概率
	BYTE byProbAllIn[ENUM_SET_ACTION_STATUS_Max];	//全下概率

	//bool bIsVirtualArr[PLAY_COUNT]; //是否AI

	SetActionProb()
	{
		Clear();
	}

	void Clear()
	{
		memset(this, 0, sizeof(*this));
		byProbFollow[ENUM_SET_ACTION_STATUS_Big] = 100;
		byProbFold[ENUM_SET_ACTION_STATUS_Small] = 100;
	}

	void ReSet(ENUM_SET_ACTION_STATUS nEnum)
	{
		if (nEnum < ENUM_SET_ACTION_STATUS_Big || nEnum >= ENUM_SET_ACTION_STATUS_Max)

		byProbFold[nEnum] = 0;
		byProbFollow[nEnum] = 0;
		byProbRaise[nEnum] = 0;
		byProbAllIn[nEnum] = 0;

		switch (nEnum)
		{
		case ENUM_SET_ACTION_STATUS_Big:
			byProbFollow[ENUM_SET_ACTION_STATUS_Big] = 100;
			break;
		case ENUM_SET_ACTION_STATUS_Small:
			byProbFold[ENUM_SET_ACTION_STATUS_Small] = 100;
			break;
		default:
			break;
		}
	}

	void ReSetAI()
	{
		//memset(bIsVirtualArr, true, sizeof(bIsVirtualArr));
	}
};

//游戏开始信息
struct GameBeginInfo
{
	bool bPlaying[PLAY_COUNT]; //游戏中与否

	TAG_INIT(GameBeginInfo)
	{
		memset(this, 0, sizeof(*this));
	}
};


//******************************************************************************************
// 游戏状态定义
//******************************************************************************************

#define GS_WAIT_SETGAME					0	//等待东家设置状态
#define GS_WAIT_ARGEE					1	//等待同意设置
#define GS_CONFIG_NOTE					20	//设置底注状态
#define GS_AGREE_NOTE					21	//同意底注状态
#define GS_PLAY_GAME					25	//游戏过程状态
#define GS_WAIT_NEXT_ROUND				28	//等待下一回合结束状态

/******************************************************************************************
*									德克萨斯扑克状态定义                                  *
*******************************************************************************************/

#define GS_USER_STAKE					30	// 玩家下注额状态

/******************************************************************************************
*																						  *
*******************************************************************************************/
//游戏状态包的基本
struct TGSBase 
{
    /*-GSBase基本属性-*/
    int  iEvPassTime;			//当前事件已消耗的时间（秒）
    int	 iVerbTime;				//游戏中各动作的思考时间
	bool bBoy[PLAY_COUNT];  	//玩家性别
	bool bHaveUser[PLAY_COUNT]; //桌上各位置有没有玩家
	BYTE bGameStation;			//游戏状态

    /*-桌面基本配置-*/
    TDeskCfg tagDeskCfg;

    TAG_INIT(TGSBase)
    {
        ::memset(this,0,sizeof(TGSBase));
    }

};

//游戏状态包：等待同意/未开始时
struct TGSWaitAgree : public TGSBase 
{
	bool	  bReady[PLAY_COUNT];               //是否准备
    __int64   nSubMoney[PLAY_COUNT];            //每个玩家代入的金币
	//__int64   iTotalMoney[PLAY_COUNT];			//玩家累计输赢

    TAG_INIT(TGSWaitAgree)
    {
        ::memset(this,0,sizeof(TGSBase));
    }
};

//游戏状态包：游戏中
struct TGSPlaying : public TGSBase 
{
	GameBeginInfo	tPlayInfo;				//游戏开始信息
    BYTE  byHandCard[PLAY_COUNT][2];	    //玩家手上的扑克
    int   nHandCardNums;		            //玩家手上扑克数目

    BYTE  byCards[5];                       //桌面上公共牌
    int   nCardsCount;                      //桌面公共牌数
	int   nCardsCountToMobile;              //桌面公共牌数（手游使用）
	int   nID[5];							//五个金额按钮
    __int64   nBetMoney[PLAY_COUNT][8];     //下注金币
    bool  bNoBet[PLAY_COUNT];               //记录不可下注的玩家位置
    bool  bGiveUp[PLAY_COUNT];              //记录放弃的玩家位置

    BYTE  byTokenUser;		                //得到令牌的用户
    BYTE  byNTUser;					        //庄家
    BYTE  bySmallBlind;                     //小盲家
    BYTE  byBigBlind;                       //大肓家
	__int64	  nCallMoney;					//跟注金币
	__int64   iMinBetOrRaiseMoney;			//最小下注/加注的金币
	__int64   iMaxBetOrRaiseMoney;			//最大下注/加注的金币(暂时不使用)

    BYTE  byVerbFlag;                       //操作属性
    bool  bNewTurn;                         //新的一轮操作
	bool  bNewTurns;                        //新的一轮操作

    __int64   nBetPools[8];			        //下注边池金币
    __int64   nSubMoney[PLAY_COUNT];        //每个玩家代入的金币
	//__int64   iTotalMoney[PLAY_COUNT];		//玩家累计输赢

	int   iUserStation ;
 
    TAG_INIT(TGSPlaying)
    {
        ::memset(this,0,sizeof(TGSPlaying));
    }
};

//游戏状态包：等待下一回合
struct TGSWaitNextRound : public TGSBase 
{
    __int64   nSubMoney[PLAY_COUNT];            //每个玩家代入的金币

    TAG_INIT(TGSWaitNextRound)
    {
        ::memset(this,0,sizeof(TGSWaitNextRound));
    }
};
struct UserLeftDeskStruct
{
	BYTE    bDeskStation ; 
	bool    bClearLogo ;
};

//看牌
struct UserLookCardStruct
{
	BYTE    bDeskStation; 

	UserLookCardStruct()
	{
		bDeskStation = 255;
	}
};

//展示牌
struct ShowCardStruct
{
	BYTE	byDeskStation;
	BYTE    byCardList[2]; 
	int		iCardCount;

	ShowCardStruct()
	{
		::memset(this,0,sizeof(ShowCardStruct));
		byDeskStation = 255;
	}
	
};


struct SuperSet
{
	DWORD	byWinUserID;
	SuperSet()
	{
		::memset(this,0,sizeof(SuperSet));
	}
};

//保存游戏信息，用于重新玩(基本只是用于大结算)
struct GameInfForReplay
{
	bool bLatestWinner;

	// 玩家累计输赢
	__int64 iTotalMoney;
	int iContinuouslyWin;
	int iMaxContinuouslyWin;
	__int64 iMaxWinningMoney;
	int iWinCount;

	//房卡场(时间场)新需求
	__int64 iCurMoney; //玩家拥有金币

	GameInfForReplay()
	{
		memset(this,0,sizeof(GameInfForReplay));
	}
};

#pragma pack()
#endif
