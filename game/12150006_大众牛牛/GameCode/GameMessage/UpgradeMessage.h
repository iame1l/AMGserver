#pragma once

#ifndef AFC_SERVER_SHOWHAND_MESSAGE_HEAD_FILE
#define AFC_SERVER_SHOWHAND_MESSAGE_HEAD_FILE
#pragma pack( 1 )
//文件名字定义

#define GAMENAME						TEXT( "大众牛牛" )
#define NAME_ID							12150006						//名字 ID
#define PLAY_COUNT						6								//游戏人数
//版本定义
#define GAME_MAX_VER					1								//现在最高版本
#define GAME_LESS_VER					1								//现在最低版本
#define GAME_CHANGE_VER					0								//修改版本

//支持类型
#define SUPPER_TYPE						SUP_NORMAL_GAME|SUP_MATCH_GAME|SUP_MONEY_GAME
#define GAME_TABLE_NAME					"MatchTable"
//游戏信息


#define GET_STRING( NUM )                 #NUM
#define INT_TO_STR( NUM )                 GET_STRING( NUM )
#define CLIENT_DLL_NAME                 TEXT( INT_TO_STR( NAME_ID )##".ico" )        // 客户端ico名字
#define SKIN_FOLDER                     TEXT( INT_TO_STR( NAME_ID ) )                // 客户端目录
#define SKIN_CARD						TEXT( "CardSkin" )		                 // 共用图片文件夹


//游戏开发版本
#define DEV_HEIGHT_VERSION				3								//内部开发高版本号
#define DEV_LOW_VERSION					10 								//内部开发低版本号

//游戏状态定义
#define GS_WAIT_ARGEE					1				//等待同意设置
#define GS_ROB_NT						20              //抢庄状态
#define GS_NOTE                         21				//下底注状态
#define GS_SEND_CARD					22				//发牌状态
#define GS_OPEN_CARD					23				//摆牛状态
#define GS_GAME_BEGIN					24				//游戏开始状态
#define GS_WAIT_NEXT					25				//等待下一盘开始 

#define GS_OPEN_CUO_PAI					1				//用户点击了搓牌
#define GS_OPEN_FAN_PAI					2				//用户点击了翻牌
#define GS_OPEN_TI_SHI					3				//用户点击了提示

//按注类型
#define STATE_NULL					0x00			//无状态-错误状态-
#define STATE_PLAY_GAME				0x01			//正在游戏中
#define STATE_HAVE_NOTE				0x02			//已经下注状态
#define STATE_OPEN_CARD				0x03			//已经摆牛了
#define STATE_HAVE_ROBNT			0x04			//已经抢庄状态


#define MAX_SHAPE_COUNT                 0x15            //所有牌型种类
#define SH_USER_CARD					5               //牌的张数


//游戏玩法
enum Em_Play_Mode
{
	Mode_QZ_ZYQZ					=	0x00000001, 				//抢庄-自由抢庄
	Mode_QZ_NNSZ					=	0x00000002, 				//抢庄-牛牛上庄
	Mode_QZ_GDZJ					=	0x00000004, 				//抢庄-固定庄家
	Mode_QZ_MPQZ					=	0x00000008, 				//抢庄-明牌抢庄
	Mode_QZ_TBZZ					=	0x00000010, 				//抢庄-通比牛牛

	Mode_DF_CHOOSE1					=	0x00000020, 				//底分-选项1
	Mode_DF_CHOOSE2					=	0x00000040, 				//底分-选项2
	Mode_DF_CHOOSE3					=	0x00000080, 				//底分-选项3
	Mode_DF_CHOOSE4					=	0x00000100, 				//底分-选项4

	Mode_SHAPE_SHUN_ZI				=	0x00000200, 				//牌型-顺子牛
	Mode_SHAPE_WU_HUA				=	0x00000400, 				//牌型-五花牛
	Mode_SHAPE_TONG_HUA				=	0x00000800, 				//牌型-同花牛
	Mode_SHAPE_HU_LU				=	0x00001000, 				//牌型-葫芦牛
	Mode_SHAPE_ZHA_DAN				=	0x00002000, 				//牌型-炸弹牛

	Mode_MAX_QZ_CHOOSE1				=	0x00004000, 				//最大抢庄-1倍
	Mode_MAX_QZ_CHOOSE2				=	0x00008000, 				//最大抢庄-2倍
	Mode_MAX_QZ_CHOOSE3				=	0x00010000, 				//最大抢庄-3倍
	Mode_MAX_QZ_CHOOSE4				=	0x00020000, 				//最大抢庄-4倍

	Mode_TZ_CHOOSE1					=	0x00040000, 				//推注-0倍
	Mode_TZ_CHOOSE2					=	0x00080000, 				//推注-5倍
	Mode_TZ_CHOOSE3					=	0x00100000, 				//推注-10倍
	Mode_TZ_CHOOSE4					=	0x00200000, 				//推注-15倍

	Mode_GJ_NOT_IN					=	0x00400000, 				//高级-禁止加入
	Mode_GJ_NO_CUO_PAI				=	0x00800000, 				//高级-禁止搓牌
	Mode_GJ_NOTE_LIMIT				=	0x01000000, 				//高级-下注限制

	Mode_SHAPE_TONG_HUA_SHUN		=	0x02000000, 				//牌型-同花顺

	Mode_ZF_FANG_ZHU				=	0x04000000, 				//支付-有则为1房主支付，不然为AA支付

	Mode_FAN_BEI_322				=	0x08000000, 				//翻倍-有则为322模式，不然为4322模式

	Mode_SZ_0						=	0x10000000,					//上庄分数-0分
	Mode_SZ_100						=	0x20000000,					//上庄分数-100分
	Mode_SZ_150						=	0x40000000,					//上庄分数-150分
	Mode_SZ_200						=	0x80000000,					//上庄分数-200分
};

//特殊牛牛类型，服务端适用
enum  Em_NN_Type
{
	NN_ZhaDan		= 0, 			//炸弹牛
	NN_HuLu			= 1, 			//葫芦牛
	NN_TongHua		= 2, 			//同花牛
	NN_WuHua		= 3, 			//五花牛
	NN_ShunZi		= 4, 			//顺子牛
	NN_TongHuaShun	= 5				//同花顺牛
};

const int INVALID = 255;

/********************************************************************************/

//游戏数据包
/********************************************************************************/
/*------------------------------------------------------------------------------*/
/// 通知客户端是否为超级客户端状态消息结构
typedef struct SuperUserState
{
	BYTE byDeskStation;      /**< 玩家位置 */
	bool bEnable;           /**< 是否超端 */
	SuperUserState( )
	{
		memset( this, 0, sizeof( SuperUserState ) );
		byDeskStation = 255;
	}
}SUPERSTATE;
/*------------------------------------------------------------------------------*/
struct SuperUserSetData
{
	BYTE	byDeskStation;      //玩家位置 
	bool	bSetSuccess;		//是否设置成功了
	int		iShape;				//请求配置的牌型
	SuperUserSetData( )
	{
		memset( this, 0, sizeof( SuperUserSetData ) );
		byDeskStation = INVALID;
	}
};


/*------------------------------------------------------------------------------*/
//游戏状态数据包	（ 等待东家设置状态 ）
struct GameStation_Base
{
	BYTE				byGameStation;					//游戏状态
	unsigned int		iPlayMode;						//房间选项 
	int					iVipGameCount;					//购买桌子局数
	int					iRunGameCount;					//游戏运行的局数

	//游戏信息
	BYTE				iThinkTime;						//摊牌时间
	BYTE				iBeginTime;						//准备时间
	BYTE				iSendCardTime;					//发牌时间( ms )
	BYTE				iXiaZhuTime;					//下注时间
	BYTE				iRobNTTime;					    //抢庄时间
	BYTE				iTickTime;						//踢人时间

	BYTE				iAllCardCount;					//扑克数目

	int					iRoomBasePoint;					//房间倍数
	int					iBaseNote;						//底注

	bool				bSystemOperate;					//是否系统操作
	bool				bAuto[PLAY_COUNT];				//玩家是否托管

	GameStation_Base( )
	{
		memset( this, 0, sizeof( GameStation_Base ) );
		bSystemOperate = true;
	}
};
/*------------------------------------------------------------------------------*/
//游戏状态数据包	（ 等待其他玩家开始 & 游戏开始）
struct GameStation_WaiteAgree : public GameStation_Base
{
	bool                bUserReady[PLAY_COUNT] ;        //玩家是否已准备
	BYTE				byNTStation;					//庄家位置，255不显示

	GameStation_WaiteAgree( )
	{
		memset( this, 0, sizeof( GameStation_WaiteAgree ) );
		byNTStation = 0xFF;
	}
};
/*------------------------------------------------------------------------------*/
//游戏状态数据包	（ 游戏开始 ） // Not Used
struct GameStation_GameBegin : public GameStation_Base
{
	bool                bUserReady[PLAY_COUNT] ;        //玩家是否已准备
	GameStation_GameBegin( )
	{
		memset( this, 0, sizeof( GameStation_GameBegin ) );
	}
};

/*------------------------------------------------------------------------------*/
//游戏状态数据包	（ 叫庄状态 ）
struct GameStation_RobNt : public GameStation_Base
{
	BYTE			byUserRobNT[PLAY_COUNT];				//各玩家抢庄情况	-1-表示还没操作 0-表示不抢 1-4表示已经抢庄的倍数
	int				iUserStation[PLAY_COUNT];				//各玩家状态 标记是否中途加入的
	BYTE			iUserCardCount[PLAY_COUNT];				//用户手上扑克数目
	BYTE			iUserCard[PLAY_COUNT][SH_USER_CARD];	//用户手上的扑克
	BYTE			byRemainTime;							//剩余时间
	
	GameStation_RobNt( )
	{
		memset( this, 0, sizeof( GameStation_RobNt ) );
		memset( byUserRobNT, INVALID, sizeof( byUserRobNT ) );
	}
};

/*------------------------------------------------------------------------------*/
//游戏状态数据包	（ 下注状态 ）
struct GameStation_Note : public GameStation_Base
{
	BYTE			iStartPos;								//庄家位置
	int				iUserStation[PLAY_COUNT];				//各玩家状态
	int				iUserBase[PLAY_COUNT];					//用户下注数，有庄家庄家对应值就是抢庄倍数
	BYTE			iUserCardCount[PLAY_COUNT];				//用户手上扑克数目
	BYTE			iUserCard[PLAY_COUNT][SH_USER_CARD];	//用户手上的扑克
	BYTE			byRemainTime;							//剩余时间
	int				iCanNote[3];							//能下注的分数
	
	GameStation_Note( )
	{
		memset( this, 0, sizeof( GameStation_Note ) );
		memset( iUserStation, -1, sizeof( iUserStation ) );
	}
};
/*------------------------------------------------------------------------------*/
//游戏状态数据包	（ 发牌状态 ）
struct GameStation_SendCard : public GameStation_Base
{
	BYTE			iUserCardCount[PLAY_COUNT];				//用户手上扑克数目
	BYTE			iUserCard[PLAY_COUNT][SH_USER_CARD];	//用户手上的扑克
	int				iUserStation[PLAY_COUNT];				//各玩家状态 标记是否中途加入的
	int				iUserBase[PLAY_COUNT];					//用户每局压总注，有庄家庄家对应值就是抢庄倍数
	BYTE			byNTStation;							//庄家位置
	GameStation_SendCard( )
	{
		memset( this, 0, sizeof( GameStation_SendCard ) );
	}
};
/*------------------------------------------------------------------------------*/
//游戏状态数据包	（ 摊牌状态 ）
struct GameStation_OpenCard : public GameStation_Base
{
	BYTE			byNtStation;							//庄家位置
	int				iUserStation[PLAY_COUNT];				//各玩家状态
	BYTE			iUserCardCount[PLAY_COUNT];				//用户手上扑克数目
	BYTE			iUserCard[PLAY_COUNT][SH_USER_CARD];	//用户手上的扑克

	BYTE			byOpenUnderCard[PLAY_COUNT][3];			//底牌的三张牌
	BYTE			byOpenUpCard[PLAY_COUNT][2];			//升起来的2张牌
	int				iOpenShape[PLAY_COUNT];					//摆牛牌型
	int				imultiple[PLAY_COUNT];					//摆牛倍数

	int				iUserBase[PLAY_COUNT];					//用户倍数，有庄家庄家对应值就是抢庄倍数

	BYTE			byRemainTime;							//剩余时间

	BYTE			bySonStation;							//摊牌期间的子状态，用户是否点击了搓牌和翻牌
	
	GameStation_OpenCard( )
	{
		memset( this, 0, sizeof( GameStation_OpenCard ) );
		bySonStation = INVALID;
	}
};
/*------------------------------------------------------------------------------*/

//通知客户端下注
struct TMSG_NOTE_NFT
{
	int                 iNt;                            //庄家
	int					iCanNote[PLAY_COUNT][3];		//能下注的分数，0为默认值不用显示
	int					iNtRobMul;						//庄家抢庄倍数
	TMSG_NOTE_NFT()
	{
		memset(this, 0, sizeof(*this));
		iNt = INVALID;
		iNtRobMul = 1;
	}
};

/*-------------------------------------------------------------------------------*/

/// 发牌数据包，一次将扑克全部发给客户端
struct SendAllCardStruct
{
	BYTE      iStartPos;                  //发牌起始位置
	BYTE      iUserCard[PLAY_COUNT][SH_USER_CARD];   //用户扑克
	BYTE      iUserCardCount[PLAY_COUNT];

	SendAllCardStruct( )
	{
		memset( iUserCard, 0, sizeof( iUserCard ) );
		memset( iUserCardCount, 0, sizeof( iUserCardCount ) );
		iStartPos = INVALID;
	}
};
/*-------------------------------------------------------------------------------*/
//通知摊牌数据包
struct TNoticeOpenCard
{
	BYTE				byDeskStation;							//通知摊牌玩家的位置
	BYTE				byUserState[PLAY_COUNT];				//各玩家的状态( 标记是否中途进入的 还是一直在游戏当中的玩家 )

	TNoticeOpenCard( )
	{
		memset( this, 0, sizeof( TNoticeOpenCard ) );
		byDeskStation = INVALID;
		memset(byUserState, INVALID, sizeof(byUserState));
	}
};

/*
//用户准备数据包
struct UserAgreeStruct
{
	BYTE				bDeskStation;					//位置
	bool				bUserAgree[PLAY_COUNT];			//用户同意

	UserAgreeStruct( )
	{
		bDeskStation = INVALID;
		memset( bUserAgree, false, sizeof( bUserAgree ) );
	}
};
*/
//游戏结束统计数据包
struct GameEndStruct
{
	int					iUserState[PLAY_COUNT];					//玩家状态( 提前放弃, 还是梭 )
	__int64				iChangeMoney[PLAY_COUNT];				//输赢分数
};

//通知客户端抢庄
struct TMSG_ROB_NT_NTF
{
	BYTE byStation;			//通知抢庄玩家
	TMSG_ROB_NT_NTF()
	{
		byStation = INVALID;
	}
};
//客户端请求抢庄
struct TMSG_ROB_NT_REQ
{
	int	iValue;									//抢庄类型（0不抢庄, 1-4抢庄倍数）
	TMSG_ROB_NT_REQ()
	{
		iValue = 0;
	}
};
//服务端回复抢庄结果
struct TMSG_ROB_NT_RSP
{
	BYTE	byStation;								//抢庄玩家
	int		iValue;									//抢庄类型（0不抢庄, 1-4抢庄倍数）
	int		iSuccess;								//是否成功抢庄，0表示成功。
	TMSG_ROB_NT_RSP()
	{
		memset(this, 0, sizeof(*this));
		byStation = INVALID;
	}
};


//抢庄结束数据包
struct RobNTResultStruct
{	
	BYTE				bNTDeskStation;							//庄家位置
	BYTE				byUserState[PLAY_COUNT];				//各玩家的状态( 标记是否中途进入的 还是一直在游戏当中的玩家 )
	BYTE				bCatchNum[PLAY_COUNT];					//各玩家抢庄倍数
	bool				bShowDing;								//是否多人抢最大值

	RobNTResultStruct( )
	{
		memset( this, 0, sizeof( RobNTResultStruct ) );
		memset( byUserState, INVALID, sizeof( byUserState ) );
		memset( bCatchNum, INVALID, sizeof( bCatchNum ) );
		bShowDing = false;
	}
};
/*----------------------------------------------------------*/
//玩家摊牌结构体
struct UserTanPai
{
	BYTE	byDeskStation;		//摊牌玩家的位置
	BYTE	byUnderCard[3];		//底牌的三张牌
	BYTE	byUpCard[2];		//升起来的2张牌
	int		iShape;				//摆牛牌型
	int		imultiple;			//摆牛倍数

	UserTanPai( )
	{
		memset( this, 0, sizeof( UserTanPai ) );
		byDeskStation = INVALID;
		imultiple = 1;
	}
};

//用户下注请求
struct TMSG_NOTE_REQ
{
	int  iNoteType;				//下注数
	TMSG_NOTE_REQ( )
	{
		memset( this, 0, sizeof( *this ) );
	}
};

//下注回复
struct TMSG_NOTE_RSP
{
	BYTE	byStation;			//下注玩家
	int		iNoteValue;			//下注值
	TMSG_NOTE_RSP()
	{
		memset(this, 0, sizeof(*this));
		byStation = INVALID;
	}
};

struct CalculateBoardItem
{
	bool	bWinner;				//是否大赢家
	bool	bLoser;					//是否为土豪
	char	chUserName[64];			//用户昵称
	int		iUserID;				//玩家ID
	int		iZuoZhuangCount;		//坐庄次数
	int		iQiangZhuangCount;		//抢庄次数
	int		iTuiZhuCount;			//推注次数
	__int64	i64WinMoney;			//总成绩
	CalculateBoardItem()
	{
		memset( this, 0, sizeof(*this) );
	}
};

struct	CalculateBoard
{
	CalculateBoardItem tItem[PLAY_COUNT];
	CalculateBoard( )
	{
		memset( this, 0, sizeof( *this ) );
	}
};

//玩家搓牌请求
struct TMSG_CUO_PAI_REQ
{
	BYTE byUser;
	TMSG_CUO_PAI_REQ( )
	{
		memset( this, 0, sizeof( *this ) );
		byUser = INVALID;
	}
};

//玩家翻牌请求
struct TMSG_FAN_PAI_REQ
{
	BYTE byUser;
	TMSG_FAN_PAI_REQ( )
	{
		memset( this, 0, sizeof( *this ) );
		byUser = INVALID;
	}
};

//玩家搓牌回复
struct TMSG_CUO_PAI_RSP
{
	BYTE byUser;
	TMSG_CUO_PAI_RSP( )
	{
		memset( this, 0, sizeof( *this ) );
		byUser = INVALID;
	}
};

//玩家翻牌回复
struct TMSG_FAN_PAI_RSP
{
	BYTE byUser;
	TMSG_FAN_PAI_RSP( )
	{
		memset( this, 0, sizeof( *this ) );
		byUser = INVALID;
	}
};

//客户端请求提示牛牛牌型
struct TMSG_TI_SHI_REQ
{
	BYTE byStation;				//请求玩家
	TMSG_TI_SHI_REQ()
	{
		byStation = INVALID;
	}
};
//服务器回复牛牛牌型
struct TMSG_TI_SHI_RSP
{
	BYTE	byStation;			//回复玩家
	BYTE	byUnderCard[3];		//底牌的三张牌
	BYTE	byUpCard[2];		//升起来的2张牌
	int		iShape;				//摆牛牌型
	TMSG_TI_SHI_RSP()
	{
		memset(this, 0, sizeof(*this));
		byStation = INVALID;
		memset(byUpCard, INVALID, sizeof(byUpCard));
		memset(byUnderCard, INVALID, sizeof(byUnderCard));
	}
};
//所有玩家摊牌结束通知，用来最后显示庄家牌型
struct TMSG_ALL_TAN_PAI_FINISH_NTF
{
	BYTE	byNTDeskStation;	//庄家的位置
	BYTE	byUnderCard[3];		//庄家底牌的三张牌
	BYTE	byUpCard[2];		//庄家升起来的2张牌
	int		iShape;				//庄家摆牛牌型
	int		imultiple;			//摆牛倍数
	TMSG_ALL_TAN_PAI_FINISH_NTF()
	{
		memset(this, 0, sizeof(*this));
		byNTDeskStation = INVALID;
		memset(byUnderCard, INVALID, sizeof(byUnderCard));
		memset(byUpCard, INVALID, sizeof(byUpCard));
		iShape = 0;
	}
};
//房主请求开始游戏
struct TMSG_MASTER_START_GAME_REQ
{
	bool bCanBegin;					//能否开始游戏
	TMSG_MASTER_START_GAME_REQ()
	{
		memset(this, 0, sizeof(*this));
	}
};
//房主请求是否成功
struct TMSG_MASTER__RSP
{
	int	iSuccess;					//默认0表示成功，非房主请求等异常返回错误。
	TMSG_MASTER__RSP()
	{
		memset(this, 0, sizeof(*this));
	}
};
		
//上一局游戏请求
struct TMSG_LAST_GAME_REQ
{
	BYTE byUser;					//请求玩家
	TMSG_LAST_GAME_REQ()
	{
		memset(this, 0, sizeof(*this));
		byUser = 255;
	}
};
//上一局游戏回复
struct TMSG_LAST_GAME_RSP
{
	int					iSuccess;								//默认0表示成功，如果是第一局等异常返回错误。
	BYTE				byNTDeskStation;						//庄家的位置
	BYTE				byUnderCard[PLAY_COUNT][3];				//底牌的三张牌
	BYTE				byUpCard[PLAY_COUNT][2];				//升起来的2张牌
	int					iShape[PLAY_COUNT];						//摆牛牌型
	int					imultiple[PLAY_COUNT];					//摆牛倍数
	__int64				iChangeMoney[PLAY_COUNT];				//输赢分数
	int					iNoteValue[PLAY_COUNT];					//下注值
	TMSG_LAST_GAME_RSP()
	{
		Init();
	}
	void Init()
	{
		memset(this, 0, sizeof(*this));
		byNTDeskStation = INVALID;
		memset(byUnderCard, INVALID, sizeof(byUnderCard));
		memset(byUpCard, INVALID, sizeof(byUpCard));
	}
};

//赠送礼物请求和回复
struct TMSG_GIFT_REQ_AND_RSP
{
	BYTE bySender;				//赠送方
	BYTE byReceiver;			//接收方
	bool bSendAll;				//是否赠送所有玩家
	int iGiftNo;				//礼物编号
	TMSG_GIFT_REQ_AND_RSP()
	{
		memset(this, 0, sizeof(*this));
		bySender = INVALID;
		byReceiver = INVALID;
	}
};

//托管请求
struct TMSG_AUTO_REQ
{
	BYTE	byUser;							//请求玩家
	bool	bAuto;							//是否托管
	TMSG_AUTO_REQ()
	{
		memset(this, 0, sizeof(*this));
		byUser = INVALID;
	}
};
//托管回复
struct TMSG_AUTO_RSP
{
	BYTE	byUser;							 //请求玩家
	bool	bAuto;							 //是否托管
	TMSG_AUTO_RSP()
	{
		memset(this, 0, sizeof(*this));
		byUser = INVALID;
	}
};

//AI牌型通知
struct TMSG_AI_CARD_SHAPE
{
	BYTE	byUser;							//请求玩家
	int		iRobNT;							//抢庄
	int		iNote;							//下注
	int		iShape;							//牌型
	TMSG_AI_CARD_SHAPE()
	{
		memset(this, 0, sizeof(*this));
		byUser = INVALID;
	}
};

//-------------------------------------------------------------------------------


/********************************************************************************/

// 数据包处理辅助标识

/********************************************************************************/
#define ASS_CALL_NOTE					55				//通知下注		
#define ASS_USER_NOTE					56				//玩家下注		
#define ASS_CALL_NOTE_RESULT			57				//通知下注结果	
#define ASS_CALL_SEND_CARD				58				//通知发牌信息	
#define ASS_CALL_SEND_FINISH			59				//牌完发发成		
#define ASS_CALL_OPEN					60				//通知开牌摆牛	
#define ASS_USER_OPEN					61				//玩家摆牛		
#define ASS_CALL_OPEN_RESULT			62				//玩家摆牛结果	
#define ASS_CALL_ROBNT			        63				//通知抢庄	    
#define ASS_USER_ROBNT			        64				//玩家抢庄	    

#define ASS_CONTINUE_END				65				//游戏结束

#define	S_C_UPDATE_CALCULATE_BOARD_SIG	66
#define	S_C_UPDATE_CURRENT_JUSHU_SIG	67

#define ASS_CALL_ROBNT_RESULT			68				//通知抢庄结果	
#define ASS_CALL_ROBNT_FINISH			69				//通知抢庄结束	

const int MSG_GAME_BEGIN_NTF		=	70;				//游戏开始通知
//#define ASS_GAME_ZHUNBEI				71				//用户准备
#define ASS_GAME_AUTO_BEGIN				72				//自动开始通知

#define ASS_SUPER_USER					73				//通知是否为超端玩家
#define ASS_SUPER_USER_SET				74				//超端用户设置牌型
#define ASS_SUPER_USER_SET_RESULT		75				//返回设置结果消息

const int MSG_CUO_PAI_REQ			=	76;				//客户端搓牌请求	
const int MSG_CUO_PAI_RSP =	MSG_CUO_PAI_REQ;			//服务端搓牌回复	
const int MSG_FAN_PAI_REQ			=	77;				//客户端翻牌请求	
const int MSG_FAN_PAI_RSP =	MSG_FAN_PAI_REQ;			//服务端翻牌回复
const int MSG_TI_SHI_REQ			=	78;				//客户端请求提示牛牛牌型
const int MSG_TI_SHI_RSP = MSG_TI_SHI_REQ;				//服务器回复牛牛牌型

const int MSG_ALL_TAN_PAI_FINISH_NTF =	79;				//所有玩家摊牌结束通知
const int MSG_MASTER_START_GAME_REQ	 =	80;				//房主请求开始游戏
const int MSG_MASTER__RSP = MSG_MASTER_START_GAME_REQ;	//房主请求是否成功

const int MSG_LAST_GAME_REQ			=	81;				//上一局游戏请求
const int MSG_LAST_GAME_RSP	= MSG_LAST_GAME_REQ;		//上一局游戏回复

const int MSG_SEND_GIFT_REQ			=	82;				//赠送礼物请求
const int MSG_SEND_GIFT_RSP	=	MSG_SEND_GIFT_REQ;		//赠送礼物回复

const int MSG_AUTO_REQ		=			83;				//托管请求
const int MSG_AUTO_RSP		= MSG_AUTO_REQ;				//托管回复

const int MSG_AI_CARD_SHAPE =			84;				//AI牌型通知

#pragma pack( )
/********************************************************************************/
#endif
