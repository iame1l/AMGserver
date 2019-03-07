#pragma once

//同楼 普通版/视频版 编译  release/视频版  ，提牌器编译提牌器版

#ifndef AFC_SERVER_SHOWHAND_MESSAGE_HEAD_FILE
#define AFC_SERVER_SHOWHAND_MESSAGE_HEAD_FILE

//文件名字定义
#define GAMENAME						TEXT("扎金花")

#define NAME_ID							12101106								//名字 ID

//版本定义
#define GAME_MAX_VER					1								//现在最高版本
#define GAME_LESS_VER					1								//现在最低版本
#define GAME_CHANGE_VER					0								//修改版本

//支持类型
#define SUPPER_TYPE						SUP_NORMAL_GAME|SUP_MATCH_GAME|SUP_MONEY_GAME
//游戏信息

#define PLAY_COUNT						6								//游戏人数
#define MAX_CARD_COUNT                  3


#define SKIN_CARD						TEXT("cardskin")		//共用图片文件夹
static TCHAR szTempStr[MAX_PATH] = {0};//暂存字符串，用来获取 dll_name 等的时候使用
#define GET_CLIENT_DLL_NAME(A)			(sprintf(A,"%d",NAME_ID),strcat(A,".ico"))
#define GET_SERVER_DLL_NAME(A)			(sprintf(A,"%d",NAME_ID),strcat(A,".dll"))
#define GET_SKIN_FOLDER(A)              (sprintf(A,"%d",NAME_ID),strcat(A,""))     //获取图片文件夹
#define GET_KEY_NAME(A, B, C, D)		(sprintf(A,"%s%d%s", B, C, D),strcat(A,""))
#define GET_CLIENT_BCF_NAME(A)          (sprintf(A,"%d",NAME_ID),strcat(A,".bcf"))    


//游戏状态定义
#define GS_WAIT_SETGAME					0				//等待东家设置状态
#define GS_WAIT_ARGEE					1				//等待同意设置
#define GS_SEND_CARD					20				//发牌状态
#define GS_PLAY_GAME					21				//游戏中状态
#define GS_WAIT_NEXT					23				//等待下一盘开始 

//按注类型
#define STATE_ERR						0x00			//错误状态
#define STATE_NORMAL					0x01			//正常状态
#define STATE_LOOK						0x02			//看牌了状态
#define STATE_WAITE_OPEN				0x03			//下注满了 等待开牌
#define STATE_GIVE_UP					0x04			//不看牌后弃牌状态
#define STATE_LOOK_GIVE_UP				0x05			//看牌后弃牌状态
#define STATE_LOST                      0x06            //不看牌输后状态
#define STATE_LOOK_LOST                 0x07            //看牌输后状态

#define ACTION_LOOK						0x00			//看牌
#define ACTION_FOLLOW					0x01			//跟注
#define ACTION_ADD						0x02			//加注
#define ACTION_OPEN						0x03			//比牌
#define ACTION_GIVEUP					0x04			//弃牌
#define ACTION_WIN						0x05			//赢牌
#define ACTION_LOST						0x06			//输牌
#define ACTION_NO						0x07			//无

/*--------------------------------------------------------------------------------*/
//牌型名称
static TCHAR * G_szShapeName[9]=
{
	TEXT(""),TEXT("特殊"),TEXT("单张"),TEXT("对子"),
	TEXT("顺子"),TEXT("金花"),	TEXT("顺金"),TEXT("豹子"),TEXT("")
};

#define E_CHOUMA_COUNT 4
static __int64 G_iChouMaMoney[E_CHOUMA_COUNT] = { 0, 0, 0, 0/*50,100,200,500*/ };

#pragma pack(1)

/*------------------------S和C都需要用到的数据------------------------------------*/
struct	TGameBaseData
{
	/*服务端配置文件配置参数*/
	BYTE				m_byThinkTime;			//游戏思考时间
	BYTE				m_byBeginTime;			//游戏开始时间	

	int					m_iGuoDi;				//锅底值	
	int					m_iDiZhu;				//底注     
	__int64				m_i64DingZhu;			//顶注--总下注数达到这个值 就强制开牌		
	//__int64				m_i64ShangXian;			//个人下注上限(即 最大每轮最大下注数不能超过这个值)  暗注的，但显示为明注的
    byte                m_byCanSet;
	int					m_iBaseRatio;			//房间倍率

	/*每局游戏结束/开始需要重置的数据*/
	BYTE				m_byNtPeople;           //庄家位置
	BYTE				m_byCurrHandleDesk;		//当前操作的用户
	bool				m_bCanLook;				//是否可以看牌	
	bool				m_bCanFollow;			//是否可以跟注	
	bool				m_bCanAdd[E_CHOUMA_COUNT];//是否可以加注	
	bool				m_bCanOpen;				//是否可以比牌	
	bool				m_bCanGiveUp;			//是否可以弃牌	

	int					m_iUserState[PLAY_COUNT];	//记录用户状态	

	__int64				m_i64ZongXiaZhu;				//总注	
	__int64				m_i64XiaZhuData[PLAY_COUNT];	//玩家下注数据
	//__int64				m_i64GuessXiaZhu[PLAY_COUNT];	//玩家的蒙注

	BYTE				m_byUserCard[PLAY_COUNT][MAX_CARD_COUNT];	//用户手上的扑克
	BYTE				m_byUserCardCount[PLAY_COUNT];				//用户手上扑克数目

	bool				m_bFirstNote;                       //是否为第一次下注
	int					m_iAddNoteTime;							//下注次数，用来给客户端判断是否可以比牌
	__int64				m_i64CurrZhuBase;					//当前下注的基数
	int                 m_NoteKinds[4];                     //下注时选择的四种注型
	int                 m_iNoteCount[E_CHOUMA_COUNT][2];


	
	TGameBaseData()
	{
		InitAllData();
	}

	void	InitAllData()
	{
		m_iBaseRatio = 1;
		m_byThinkTime	= 15;			
		m_byBeginTime	= 15;		

		m_iGuoDi	= 50;				
		m_iDiZhu	= 100;			
		m_i64DingZhu= 1000;					
		//m_i64ShangXian=10000;	
        m_byCanSet = 0;
		m_byNtPeople = 255;

		InitSomeData();
	}

	/*每局游戏结束/开始需要重置的数据*/
	void	InitSomeData()
	{
        
		m_byCurrHandleDesk	= 255;		
		m_bCanLook	= false;				
		m_bCanFollow= false;			
		//m_bCanAdd	= false;				
		m_bCanOpen	= false;				
		m_bCanGiveUp= false;

		memset(m_bCanAdd,0,sizeof(m_bCanAdd));
		memset(m_iUserState,STATE_ERR,sizeof(m_iUserState));
		m_i64ZongXiaZhu = 0;	
		memset(m_i64XiaZhuData,0,sizeof(m_i64XiaZhuData));
		//memset(m_i64GuessXiaZhu,0,sizeof(m_i64GuessXiaZhu));

		memset(m_byUserCard,0,sizeof(m_byUserCard));
		memset(m_byUserCardCount,0,sizeof(m_byUserCardCount));
        memset(m_iNoteCount,0,sizeof(m_iNoteCount));
		m_bFirstNote	= true;
		m_iAddNoteTime	= 0;							
		m_i64CurrZhuBase= 0;					
	}
};
/*--------------------------------------------------------------------------------*/
/********************************************************************************/
//游戏数据包
/********************************************************************************/
/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------*/

struct S_C_BaseStation 
{
	BYTE		byThinkTime;			//游戏思考时间
	BYTE		byBeginTime;			//游戏开始时间
	int         iNoteKinds[4];          //下注时选择的四种注型
    int         iNoteCount[E_CHOUMA_COUNT][2];
	S_C_BaseStation()
	{
		memset(this, 0, sizeof(S_C_BaseStation));
	}
};


//断线重连游戏状态数据包（ 等待其他玩家开始 ）
//断线重连游戏状态数据包（ 等待其他玩家开始 ）
struct S_C_GameStation_WaiteAgree : S_C_BaseStation
{
	bool		bAgree[PLAY_COUNT];		//玩家准备状态
	int			iGuoDi;					//锅底值	
	int			iDiZhu;					//底注      
	__int64		i64ShangXian;			//个人下注上限	明注
	__int64		i64DingZhu;				//顶注

	S_C_GameStation_WaiteAgree()
	{
		memset(this,0,sizeof(S_C_GameStation_WaiteAgree));
	}
};


/*-----------------------------------------------------------------------------*/
//断线重连游戏状态数据包	（ 游戏正在发牌状态 ）
struct S_C_GameStation_SendCard : S_C_BaseStation
{
	BYTE		byNtStation;					//庄家位置
	BYTE		byUserCardCount[PLAY_COUNT];	//用户手上的牌数
	int			iUserState[PLAY_COUNT];		//玩家状态
	int			iGuoDi;						//锅底值	
	int			iDiZhu;						//底注      
	//__int64		i64ShangXian;				//个人下注上限	明注
	__int64		i64DingZhu;					//顶注
	__int64		i64AllZongZhu;				//总注
	__int64		i64XiaZhuData[PLAY_COUNT];	//用户当前下注信息
	S_C_GameStation_SendCard()
	{
		memset(this,0,sizeof(S_C_GameStation_SendCard));
	}
};
/*-----------------------------------------------------------------------------*/
//断线重连游戏状态数据包	（ 游戏中状态 ）
struct S_C_GameStation_PlayGame : S_C_BaseStation
{
	bool		bCanLook;				//可否看牌
	bool		bCanFollow;				//可否跟注
	bool		bCanAdd[E_CHOUMA_COUNT];				//可否加注
	bool		bCanOpen;				//可否比牌
	bool		bCanGiveUp;				//可否弃牌
	BYTE		byNtStation;					//庄家位置
	BYTE		byCurrHandleDesk;				//当前操作的玩家
	BYTE		byUserCardCount[PLAY_COUNT];				//用户手上的牌数
	BYTE		byUserCard[PLAY_COUNT][MAX_CARD_COUNT];		//用户手上的牌
	int			iUserState[PLAY_COUNT];		//玩家状态
	int			iRemainderTime;				//剩余时间
	int			iGuoDi;						//锅底值	
	int			iDiZhu;						//底注      
	__int64		i64ShangXian;				//个人下注上限	明注
	__int64		i64DingZhu;					//顶注

	__int64		i64AllZongZhu;				//总注
	__int64		i64XiaZhuData[PLAY_COUNT];	//用户当前下注信息

	S_C_GameStation_PlayGame()
	{
		memset(this,0,sizeof(S_C_GameStation_PlayGame));
	}
};


/*-----------------------------------------------------------------------------*/
//游戏开始
struct	S_C_GameBegin
{
	BYTE                byNtStation;                      //庄家 

	int                 iGuoDi;                         //本局锅底值
	int					iDiZhu;							//本局底注

	int					iUserState[PLAY_COUNT];			//玩家状态
	
	//__int64				i64ShangXian;					//本局下注上限 (即 最大每轮最大下注数不能超过这个值) 暗注的，但显示为明注的
	__int64				i64DingZhu;						//本局顶注 -总下注上限	达到上限 就要开牌	

	__int64				i64ZongXiaZhu;					//当前总下注;
	__int64				i64XiaZhuData[PLAY_COUNT];		//玩家下注数据

	S_C_GameBegin()
	{
		memset(this,0,sizeof(S_C_GameBegin));
	}
};
/*-----------------------------------------------------------------------------*/
//发牌数据包
struct S_C_SendCard
{
	BYTE		byCard[PLAY_COUNT][MAX_CARD_COUNT];				//牌数据
	BYTE		byCardCount[PLAY_COUNT];                        //牌张数
	BYTE		bySendCardTurn[PLAY_COUNT*MAX_CARD_COUNT];		//发牌顺序--记录的是发牌的位置  255表示不需要发牌

	S_C_SendCard()
	{
		memset(this,0,sizeof(S_C_SendCard));
		memset(bySendCardTurn,255,sizeof(bySendCardTurn));
	}

};
/*-----------------------------------------------------------------------------*/
//游戏开始数据包
struct S_C_BeginPlay
{
	BYTE		byCurrHandleStation;			//出牌的位置
	bool		bLook;		//是否可看牌
	bool		bFollow;	//是否可跟注
	bool		bAdd[E_CHOUMA_COUNT];		//是否可加注
	bool		bOpen;		//是否可比牌
	bool		bGiveUp;	//是否可弃牌
			
	S_C_BeginPlay()
	{
		memset(this,0,sizeof(S_C_BeginPlay));
	}
};
/*-----------------------------------------------------------------------------*/
//通知操作
struct S_C_NoticeAction
{
	BYTE				byCurrHandleStation;	//出牌的位置
	bool				bCanLook;				//是否可以看牌	
	bool				bCanFollow;				//是否可以跟注	
	bool				bCanAdd[E_CHOUMA_COUNT];
	bool				bCanOpen;				//是否可以比牌	
	bool				bCanGiveUp;				//是否可以弃牌	

	S_C_NoticeAction()
	{
		memset(this,0,sizeof(S_C_NoticeAction));
	}
};
/*-----------------------------------------------------------------------------*/
//用户看牌
struct C_S_LookCard
{
	BYTE	byDeskStation;	
	C_S_LookCard()
	{
		memset(this,255,sizeof(C_S_LookCard));
	}
};

//看牌数据
struct S_C_LookCardResult
{	
	BYTE            byDeskStation;				//玩家位置
	BYTE			byUserCardCount;			//用户手上的牌数
	BYTE			byUserCard[MAX_CARD_COUNT];	//用户手上的牌
	int				iUserState;					//玩家状态
	S_C_LookCardResult()
	{
		memset(this,-1,sizeof(S_C_LookCardResult));
		byUserCardCount = 0;
	}
};
/*-----------------------------------------------------------------------------*/
//用户跟注
struct C_S_UserGenZhu
{
	BYTE	byDeskStation;		//玩家位置
};
/*-----------------------------------------------------------------------------*/
//用户跟注结果
struct S_C_UserGenZhuResult
{

	BYTE		byDeskStation;		//玩家位置
	int			iUserState;			//该位置玩家的状态
	__int64		i64FollowNum;		//跟注的大小
	__int64		i64UserZongZhu;		//玩家的总下注
	__int64		i64AllZongZhu;		//所有总下注

	S_C_UserGenZhuResult()
	{
		memset(this,0,sizeof(S_C_UserGenZhuResult));
	}
};
/*-----------------------------------------------------------------------------*/
//用户下注
struct C_S_JiaZhu
{

	BYTE		byDeskStation;	
	int			iAddType;			//加注类型

	C_S_JiaZhu()
	{
		memset(this,0,sizeof(C_S_JiaZhu));
	}
};
/*-----------------------------------------------------------------------------*/
//用户下注结果
struct S_C_JiaZhuResult
{
	BYTE		byDeskStation;	
	int			iUserState;			//该位置玩家的状态
	__int64		i64AddNum;			//跟注的大小
	__int64		i64UserZongZhu;		//玩家的总下注
	__int64		i64AllZongZhu;		//所有总下注

	S_C_JiaZhuResult()
	{
		memset(this,0,sizeof(S_C_JiaZhuResult));
	}
};
/*-----------------------------------------------------------------------------*/
//用户弃牌、弃牌结果
struct C_S_UserGiveUp
{
	BYTE	byDeskStation;	
};

struct S_C_UserGiveUpResult
{
	BYTE	byDeskStation;	
	int		iUserState;					//玩家状态
};
/*-----------------------------------------------------------------------------*/
//用户比牌
struct C_S_UserBiPai
{
	BYTE	byDeskStation;		//主动比牌的玩家
	BYTE	byBeDeskStation;	//被动比牌玩家
	
	C_S_UserBiPai()
	{
		memset(this, 255, sizeof(C_S_UserBiPai));
	}
};
/*-----------------------------------------------------------------------------*/
//用户比牌
struct S_C_UserBiPaiResult
{
	BYTE	byDeskStation;		//主动比牌的玩家
	BYTE	byBeDeskStation;	//被动比牌玩家

	BYTE	byWinDesk;			//比牌赢的玩家
	BYTE	byLostDesk;			//比牌输的玩家

	int		iUserStation[PLAY_COUNT];			//记录用户状态

	__int64		i64AddNum;			//比牌需要下注的数
	__int64		i64UserZongZhu;		//玩家的总下注
	__int64		i64AllZongZhu;		//所有总下注



	S_C_UserBiPaiResult()
	{
		memset(this,0,sizeof(S_C_UserBiPaiResult));
	}
};
/*-----------------------------------------------------------------------------*/
//游戏结束统计数据包
struct S_C_GameEnd
{
	BYTE				byWiner;							//赢家

	BYTE				byCard[PLAY_COUNT][MAX_CARD_COUNT];	//牌数据
	BYTE				byCardShape[PLAY_COUNT];			//牌型
	
	int					iOperateCount[PLAY_COUNT];			//操作次数
	__int64				i64UserScore[PLAY_COUNT];			//各玩家得分-扣税前
	__int64				i64ChangeMoney[PLAY_COUNT];			//各玩家金币变化-扣税后

	S_C_GameEnd()
	{
		memset(this,0,sizeof(S_C_GameEnd));
	}
};
/*-----------------------------------------------------------------------------*/
//游戏结束统计数据包
struct GameCutStruct
{
	BYTE					byDeskStation;					//退出位置
	__int64					iTurePoint[PLAY_COUNT];					//庄家得分
	__int64					iChangeMoney[PLAY_COUNT];
	GameCutStruct()
	{
		memset(this,0,sizeof(GameCutStruct));
	}
};
//-------------------------------------------------------------------------------
//超端信息结构体
struct	S_C_SuperUserProof
{
	BYTE	byDeskStation;	
	bool	bIsSuper;
	S_C_SuperUserProof()
	{
		memset(this,0,sizeof(S_C_SuperUserProof));
		byDeskStation = 255;
	}
};
//超端信息结构体
struct	C_S_SuperUserSet
{
	BYTE	byDeskStation;	
	BYTE	byMaxDesk;		//设定最大的玩家
	BYTE	byMinDesk;		//设定最小的玩家
	C_S_SuperUserSet()
	{
		memset(this,255,sizeof(C_S_SuperUserSet));
	}
};
//超端设置结果
struct	S_C_SuperUserSetResult
{
	bool	bSuccess;		//是否设定成功
	BYTE	byDeskStation;	
	BYTE	byMaxDesk;		//设定最大的玩家
	BYTE	byMinDesk;		//设定最小的玩家
	S_C_SuperUserSetResult()
	{
		memset(this,255,sizeof(S_C_SuperUserSetResult));
		bSuccess = false;
	}
};

struct	CalculateBoard
{
	bool	bWinner;				//大赢家
	int		iBombCount;				//炸弹次数	
	int		iMaxWinMoney;			//最多赢钱数目
	int		iMaxContinueCount;		//最高连胜
	int		iWinCount;				//胜利局数
	__int64	i64WinMoney;			//输赢金币
	CalculateBoard()
	{
		memset(this,0,sizeof(CalculateBoard));
	}
};

/*-----------------------------------------------------------------------------*/
#pragma pack()

/********************************************************************************/

// 数据包处理辅助标识

/********************************************************************************/
#define S_C_GAME_BEGIN					50				//游戏开始		
#define S_C_SEND_CARD					51				//开始发牌
#define S_C_GAME_PLAY					52				//开始游戏		
#define S_C_CALL_ACTION					53				//通知玩家操作	
#define S_C_GAME_END					54				//游戏结束

#define S_C_SUPER_PROOF					70				//超端校验
#define C_S_SUPER_SET					71				//超端设置
#define S_C_SUPER_SET_RESULT			72				//超端设置结果

#define C_S_LOOK_CARD					80				//玩家看牌
#define S_C_LOOK_CARD_RESULT			81				//玩家看牌结果
#define C_S_GEN_ZHU						82				//玩家跟注
#define S_C_GEN_ZHU_RESULT				83				//玩家跟注结果
#define C_S_JIA_ZHU						84				//加注
#define S_C_JIA_ZHU_RESULT				85				//加注结果
#define C_S_BI_PAI						86				//玩家比牌
#define S_C_BI_PAI_RESULT				87				//玩家比牌结果
#define C_S_GIVE_UP						88				//玩家弃牌
#define S_C_GIVE_UP_RESULT				89				//玩家弃牌结果

#define S_C_UPDATE_CALCULATE_BOARD_SIG	90				//更新结算榜
#define S_C_UPDATE_REMAIN_JUSHU_SIG		91				//更新剩余局数


/********************************************************************************/
#endif
