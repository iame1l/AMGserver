#pragma once

//同楼 普通版/视频版 编译  release/视频版  ，提牌器编译提牌器版

#ifndef AFC_SERVER_SHOWHAND_MESSAGE_HEAD_FILE
#define AFC_SERVER_SHOWHAND_MESSAGE_HEAD_FILE
#pragma pack(1)
//文件名字定义
#define GAMENAME						TEXT("三人跑得快")

#ifdef VIDEO
#define NAME_ID							10100703								//名字 ID
#else
#define NAME_ID							10100703								//名字 ID
#endif


//版本定义
#define GAME_MAX_VER					1								//现在最高版本
#define GAME_LESS_VER					1								//现在最低版本
#define GAME_CHANGE_VER					0								//修改版本

//支持类型
#define SUPPER_TYPE						SUP_NORMAL_GAME|SUP_MATCH_GAME|SUP_MONEY_GAME


//游戏内部开发版本号
#define DEV_HEIGHT_VERSION				2				//内部开发高版本号(每修正一次LOW_VERSIONG+1)
#define DEV_LOW_VERSION					0				//内部开发低版本号



//游戏信息
#define PLAY_COUNT						3								//游戏人数
#define SKIN_CARD						TEXT(".\\image\\cardskin")		//共用图片文件夹
static TCHAR szTempStr[MAX_PATH] = {0};//暂存字符串，用来获取 dll_name 等的时候使用
#define GET_CLIENT_DLL_NAME(A)			(sprintf(A,"%d",NAME_ID),strcat(A,".ico"))
#define GET_SERVER_DLL_NAME(A)			(sprintf(A,"%d",NAME_ID),strcat(A,".dll"))

#define GET_CLIENT_BCF_NAME(A)			(sprintf(A,"%d",NAME_ID),strcat(A,".bcf"))

#define CARD_COUNT								48				//所用扑克总数

//游戏状态定义
#define GS_WAIT_SETGAME					0				//等待东家设置状态
#define GS_WAIT_ARGEE					1				//等待同意设置
#define GS_SEND_CARD					20				//发牌状态
//#define GS_WAIT_BACK					21				//等待扣压底牌
#define GS_PLAY_GAME					21				//游戏中状态
#define GS_WAIT_NEXT					22				//等待下一盘开始 


#define GS_FLAG_NORMAL					0				//正常情况

const  int  MAX_TASK_TYPE   = 4 ;  ///任务最大种类 
const  int  MAX_CARD_SHAPE  = 8 ;  ///牌型最大种类
const  int  MAX_CARD_TYPE   = 15 ; ///牌种类

///底牌类型
enum BackCardType
{
	TYPE_NONE        = 0 ,//什么牌型都不是
	TYPE_DOUBLE_CARD = 10 , 
	TYPE_SAME_HUA    = 11,
	TYPE_SMALL_KING  = 12 ,
	TYPE_BIG_KING    = 13 , //大王
	TYPE_TRIPLE_CARD = 14 ,
	TYPE_STRAIT      = 15,
	TYPE_ROCKET      = 16
};
///任务类型
enum LastTaskType
{
	TYPE_LAST_NONE         = 0 ,    ///无任何牌型 
	TYPE_HAVE_A_CARD       = 100 ,  ///有某张牌
	TYPE_SOME_SHAPE        = 101 ,  ///有某种牌型
	TYPE_SINGLE_SOME_CARD  = 102 ,  ///打的某张牌
	TYPE_DOUBLE_SOME_CARD  = 103   ///打的一对某种牌
};

///游戏任务结构
struct GameTaskStruct
{
	BYTE     byBackCardType ; 
	BYTE     byTaskType ; 
	BYTE     bySpecifyShape ; 
	BYTE     bySpecifyCard ;
	BYTE     byBackCardMutiple ;  ///底牌倍数

	GameTaskStruct()
	{
		memset(this , 0 , sizeof(GameTaskStruct)) ; 
	}
	void IniData(void)
	{
		memset(this , 0 ,sizeof(GameTaskStruct)) ;
	}

};
///游戏中的倍数
struct  GameMutipleStruct
{
	int       sBaseMutiple ;                 ///游戏中的基本底注
	int       sBackCardMutiple ;             ///底牌倍数
	int       sBombCount ;                   ///炸弹倍数
	int       sSprintMutiple ;               ///春天
	int       sCardShapeMutiple  ;           ///牌型倍数（一般指任务中的倍数）

	BYTE      sAddGameMutiple[PLAY_COUNT] ;  /// 游戏中加倍
	BYTE      sRobNtMutiple[PLAY_COUNT] ;    ///抢地主倍数
	BYTE      sMingPaiMutiple[PLAY_COUNT] ;  ///每个人的明牌倍数

	GameMutipleStruct()
	{
		sBaseMutiple = 1 ; 
		sBackCardMutiple = 1 ; 
		sBombCount = 0 ; 
		sSprintMutiple = 1;
		sCardShapeMutiple = 100 ; 

		memset(sAddGameMutiple , 0 , sizeof(sAddGameMutiple)) ; 
		memset(sRobNtMutiple , 0 , sizeof(sRobNtMutiple)) ; 
		memset(sMingPaiMutiple , 0 , sizeof(sMingPaiMutiple)) ; 
	};
	void  IniData(int  iBaseMutiple)
	{
		sBaseMutiple = iBaseMutiple ; 
		sBombCount = 0 ; 
		sBackCardMutiple = 1 ; 
		sSprintMutiple = 1;
		sCardShapeMutiple = 100; 

		memset(sAddGameMutiple , 0 , sizeof(sAddGameMutiple)) ; 
		memset(sRobNtMutiple , 0 , sizeof(sRobNtMutiple)) ; 
		memset(sMingPaiMutiple , 0 , sizeof(sMingPaiMutiple)) ; 
	}
	///获取明牌最大倍数
	int  GetMingMaxMutiple(void)
	{
		int iMingMutiple = max(max(sMingPaiMutiple[0] , sMingPaiMutiple[1]) ,sMingPaiMutiple[2]) ; 

		return (iMingMutiple >0?iMingMutiple :1) ; 
	}
	///获取器抢地主倍数
	int  GetRobNtMutiple(void)
	{
		int  iRobMutiple = 0 ; 
		int  iRobCount   = 0 ; 

		for(int  i = 0  ; i <PLAY_COUNT ; i++)
		{
			iRobCount += sRobNtMutiple[i] ; 
		}

		iRobMutiple = pow(2.0, iRobCount) ; 

		return iRobMutiple ; 
	}
    ///获取公共倍数
    int  GetPublicMutiple(void)
	{
		int  iBombMutiple = pow(2.0 , sBombCount) ; 

		int  iGameMutiple  = sBaseMutiple*sBackCardMutiple*iBombMutiple*sSprintMutiple*sCardShapeMutiple*GetRobNtMutiple()*GetMingMaxMutiple()/100 ;
			
		return iGameMutiple ; 
		
	}
};


/********************************************************************************/
//游戏数据包

/********************************************************************************/
//用户出牌数据包 （发向服务器）
struct C_S_OutCardStruct
{
	int					iCardCount;						//扑克数目
	BYTE				iCardList[CARD_COUNT];					//扑克信息
	C_S_OutCardStruct()
	{
		memset(this,0,sizeof(C_S_OutCardStruct));
	}
};

//用户出牌数据包 （发向客户端）
struct S_C_OutCardMsg
{
	int					iNextDeskStation;				//下一出牌者
	BYTE				iCardCount;						//扑克数目
	BYTE				bDeskStation;					//当前出牌者	
	BYTE				iCardList[CARD_COUNT];			//扑克信息
	bool				bShowNotic;						//显示提示出牌(true:显示,false:不显示)
	S_C_OutCardMsg()
	{
		memset(this,0,sizeof(S_C_OutCardMsg));
		bShowNotic = false;
	}
};

//新一轮
struct NewTurnStruct
{
	BYTE				bDeskStation;					//坐号
	bool				bEnableNotic;					//新一轮是否要显示提示(最后一手牌不用显示提示了)
	NewTurnStruct()
	{
		bDeskStation = 0;
		bEnableNotic = false;
	}
};

//游戏状态数据包	（ 等待东家设置状态 ）
struct GameStation_base
{
	//游戏版本
	BYTE				iVersion;						//游戏版本号
	BYTE				iVersion2;						//游戏版本号

	BYTE				iBeginTime;						//开始准备时间
	BYTE				iThinkTime;						//出牌思考时间
	BYTE				iLeftTime;						//剩余时间
	DWORD				iCardShape;						//牌型设置
	//__int64             gameMoneyLimit;               //游戏进入房间的限制
	UINT				iDeskBasePoint;					//桌面基础分
	UINT				iRoomBasePoint;					//房间倍数
	__int64				iRunPublish;					//逃跑扣分
	int					iZamaNum;						//玩家所选择的游戏规则
	//游戏信息
	GameStation_base()
	{
		memset(this,0,sizeof(GameStation_base));
	}
};

//游戏状态数据包	（ 等待其他玩家开始 ）
struct S_C_GameStation_Wait:public GameStation_base
{

	bool                bUserReady[PLAY_COUNT] ;        ///玩家是否已准备

	S_C_GameStation_Wait()
	{
		memset(this,0,sizeof(S_C_GameStation_Wait));
	}
};


//游戏状态数据包	（ 游戏中状态 ）
struct S_C_GameStation_Play:public GameStation_base
{
	bool				bIsLastCard;						//是否有上轮数据
	
	BYTE				bIsPass;							//是否不出

	int					iBase;								//当前炸弹个数
	
	int					iOutCardPeople;						//现在出牌用户
	int					iFirstOutPeople;					//先出牌的用户
	int					iBigOutPeople;						//先出牌的用户


	bool				bAuto[PLAY_COUNT];					//托管情况
	bool				bCanleave[PLAY_COUNT];				//能否点退出

	BYTE				iUserCardCount[PLAY_COUNT];			//用户手上扑克数目
	BYTE				iUserCardList[CARD_COUNT];					//用户手上的扑克

	BYTE				iBaseOutCount;						//出牌的数目
	BYTE                iBaseCardList[CARD_COUNT] ;                //桌面上的牌

	BYTE				iDeskCardCount[PLAY_COUNT];			//桌面扑克的数目
	BYTE                iDeskCardList[PLAY_COUNT][CARD_COUNT] ;    ///桌面上的牌

	BYTE				iLastCardCount[PLAY_COUNT];			//上轮扑克的数目
	BYTE				iLastOutCard[PLAY_COUNT][CARD_COUNT];		//上轮的扑克

	bool                bPass[PLAY_COUNT] ;                //不出
	bool                bLastTurnPass[PLAY_COUNT] ;         //上一轮不出

	S_C_GameStation_Play()
	{
		memset(this,0,sizeof(S_C_GameStation_Play));
	}

};

//游戏状态数据包	（ 等待下盘开始状态  ）
struct S_C_GameStation_Next:public GameStation_base
{

	bool                bUserReady[PLAY_COUNT] ;     ///玩家准备
	S_C_GameStation_Next()
	{
		memset(this,0,sizeof(S_C_GameStation_Next));
	}
};

//用户同意游戏
struct UserArgeeGame
{
	BYTE				iPlayCount;							//游戏盘数
	BYTE				iCardCount;							//扑克数目
};

/*----------------------------------------------------------------------*/
//游戏开始
struct	S_C_GameBeginStruct
{
	BYTE				iPlayLimit;							//游戏总局数
	BYTE				iBeenPlayGame;						//已经玩了多少局
	DWORD				iCardShape;							//牌型设置
	int					iZamaNum;							//1:	16张普通打法
															//2:	16张猴子打法
															//3:	15张普通打法
															//4:	15张猴子打法
	S_C_GameBeginStruct()
	{
		memset(this, 0, sizeof(S_C_GameBeginStruct));
	}
};

/*----------------------------------------------------------------------*/
//游戏准备数据包
struct	GamePrepareStruct
{
	BYTE				byCard;									//明牌
	GamePrepareStruct()
	{
		memset(this,255,sizeof(GamePrepareStruct));
	}
};

/*----------------------------------------------------------------------*/
//发牌数据包
struct	SendCardStruct
{
	BYTE				bDeskStation;							//玩家
	BYTE				bCard;									//牌标号
};

/*----------------------------------------------------------------------*/
//发送所有牌数据
struct	S_C_SendAllStruct
{
	BYTE             iUserCardCount[PLAY_COUNT];			//发牌数量
	BYTE             iUserCardList[CARD_COUNT];				//发牌队例

	S_C_SendAllStruct()
	{
		memset(this,0,sizeof(S_C_SendAllStruct));
	}
};

/*----------------------------------------------------------------------*/
//发牌结束
struct	S_C_SendCardFinishStruct
{
	BYTE		bReserve;	
	BYTE		byUserCardCount[PLAY_COUNT];					//用户手上扑克数目
	BYTE		byUserCard[PLAY_COUNT][CARD_COUNT];				//用户手上的扑克
	S_C_SendCardFinishStruct()
	{
		memset(this,0,sizeof(S_C_SendCardFinishStruct));
	}
};

/*----------------------------------------------------------------------*/
//游戏开始数据包
struct S_C_BeginPlayStruct
{
	BYTE				iOutDeskStation;				//出牌的位置
	bool				bMastOutBlackThree;				//是否是要出黑桃3,true:要先出黑桃3，false:不需要
};


//奖分
struct AwardPointStruct
{
	BYTE	iAwardPoint;									//奖分
	BYTE	bDeskStation;									//坐号
	int		iBase;											//倍数
};

//托管数据结构
struct AutoStruct
{
	BYTE bDeskStation;
	bool bAuto;
};

//机器人托管
struct UseAIStation
{
	BYTE bDeskStation;					//坐号
	BOOL bState;						//状态
};

//冠军结果
struct ChampionStruct
{
	BYTE bDeskStation ;
};

//游戏结束统计数据包
struct S_C_GameEndStruct
{
	BYTE			    iUserCardCount[PLAY_COUNT];	    //用户手上扑克数目
	BYTE			    iUserCard[PLAY_COUNT][CARD_COUNT];		//用户手上的扑克

    __int64  			iTurePoint[PLAY_COUNT];			//玩家得分
	__int64				iChangeMoney[8];				//玩家金币
	BYTE				iBomb[PLAY_COUNT];					//炸弹奖励
	char				szName[PLAY_COUNT][61];			///昵称
	int					iBaopeiUser	;					//包赔玩家

	S_C_GameEndStruct()
	{
		memset(this , 0 ,sizeof(S_C_GameEndStruct)) ; 
	}

};

//游戏结束统计数据包
struct S_C_GameCutStruct
{
	int					iRoomBasePoint;					//倍数
	int					iDeskBasePoint;					//桌面倍数
	int					iHumanBasePoint;				//人头倍数

	int					bDeskStation;					//退出位置
	__int64				iChangeMoney[8];				//玩家金币
	__int64				iTurePoint[PLAY_COUNT];			//庄家得分

};

struct HaveThingStruct
{
	BYTE pos;
	char szMessage[61];
};

struct LeaveResultStruct
{
	BYTE bDeskStation;
	int bArgeeLeave;
};


//游戏结算
struct GameFinishNotify
{
	TCHAR name[PLAY_COUNT][61];
	//	int	iBasePoint;
	BYTE iStyle;			//游戏类型是否为强退,还是正常结束
	int	iBasePoint;			//系统基数
	int	iDeskBasePoint;		//桌面基础分
	int iUpGradePeople;		//莊家
	int iGrade2;			//莊家盟友
	__int64	iWardPoint[PLAY_COUNT];
	__int64 iAwardPoint[PLAY_COUNT];	//讨赏
	__int64 iTotalScore[PLAY_COUNT];	//总积分
	__int64 iMoney[PLAY_COUNT];			//比赛所留钱数
	BYTE iGameStyle;					//游戏类型
};
struct UserleftDesk
{
	BYTE bDeskStation;                //断线玩家
};


struct S_C_SettlementList
{
	int iSingleMaxScore[PLAY_COUNT];			//单局最高分
	int iOutBomNumber[PLAY_COUNT];				//出炸弹的个数
	int iWinNumber[PLAY_COUNT];					//胜利局数
	int iLostNumber[PLAY_COUNT];				//失败局数
	long long llScore[PLAY_COUNT];				//总积分
	S_C_SettlementList()
	{
		Clrea();
	}
	void Clrea()
	{
		memset(this,0,sizeof(S_C_SettlementList));
	}
};

//-------------------------------------------------------------------------------
//超端信息结构体
struct	SuperUserMsg
{
	BYTE	byDeskStation;	
	bool	bIsSuper;
	SuperUserMsg()
	{
		memset(this,0,sizeof(SuperUserMsg));
	}
};

//不能出牌原因
struct S_C_OutCardError
{
	int iError;					//4:当前不是出牌玩家,不能出牌,2:当前出牌不符合基本牌型,3:第一局,玩家必须出黑3,4:三带一不能中途出,5:炸弹不能拆,6:333444555 666A型的不是最后一手牌不能出
								//6:当前出牌不合法,
	S_C_OutCardError()
	{
		iError = -1;
	}
};

//当前要不起玩家的位置
struct S_C_CannotOutCard
{
	BYTE byDeskStation;
	S_C_CannotOutCard()
	{
		byDeskStation = 255;
	}
};

//设置牌型
struct	SuperSetCardMsg
{
	bool	isMake;
	SuperSetCardMsg()
	{
		memset(this,0,sizeof(SuperSetCardMsg));
	}
};
//-------------------------------------------------------------------------------
/********************************************************************************/

// 数据包处理辅助标识
//warning:79号ID预留给超级客户端,永不使用
/********************************************************************************/
#define ASS_UG_USER_SET					50				//设置游戏
#define ASS_GAME_BEGIN					51				//游戏开始
#define ASS_GAME_PREPARE				52				//准备工作

#define ASS_SEND_CARD					53				//发牌信息
#define ASS_SEND_CARD_MSG				54				//发牌过程中处理消息
#define ASS_SEND_ALL_CARD				55				//发送所有牌(一下发放全部)
#define ASS_SEND_FINISH					56				//发牌完成

#define ASS_GAME_PLAY					76				//开始游戏
#define ASS_OUT_CARD					77				//用户出牌
#define ASS_OUT_CARD_RESULT				78				//出牌結果

#define	ASS_SUPER_USER					79				//超端消息

#define ASS_REPLACE_OUT_CARD			80				//代替出牌(79留给超级客户端发牌器)
#define ASS_ONE_TURN_OVER				81				//一轮完成(使客户端上一轮可用)
#define ASS_NEW_TURN					82				//新一轮开始
#define ASS_AWARD_POINT					83				//奖分(炸弹火箭)

#define ASS_CONTINUE_END				84				//游戏结束
#define ASS_NO_CONTINUE_END				85				//游戏结束

#define ASS_CUT_END						87				//用户强行离开
#define ASS_SAFE_END					88				//游戏安全结束
#define ASS_TERMINATE_END				89				//意外结束
#define ASS_AHEAD_END					90				//提前结束
#define ASS_AUTO						91				//用户托管
#define ASS_HAVE_THING					92				//有事
#define ASS_LEFT_RESULT					93				//有事离开结果
#define ASS_MESSAGE						94				//文字消息
#define ASS_AI_STATION					95				//机器人托管(断线户用)
#define ASS_REPLACE_GM_AGREE_GAME		96				//代替玩家开始
#define ASS_USER_LEFTDESK               97               //玩家离开桌子或断线

#define S_C_SETTLEMENTLIST				98				//总积分

#define S_C_OUTCARD_ERROR				99				//当前出牌出错的原因(不能出牌)
#define S_C_CANNT_COUT_CARD				100				//要不起了
#define ASS_REQ_SET_CARD				101				//设置牌型
#pragma pack()
/********************************************************************************/
#endif
