#pragma once

//#include "HNBaseType.h"

#ifndef AFC_SERVER_SHOWHAND_MESSAGE_HEAD_FILE
#define AFC_SERVER_SHOWHAND_MESSAGE_HEAD_FILE

//文件名字定义
#define GAMENAME						TEXT("疯狂老虎机")
#define GAME_DLL_AUTHOR                 TEXT("深圳红鸟网络科技")            //
#define GAME_DLL_NOTE                   TEXT("牌类 -- 游戏组件")


#define NAME_ID							31017001						//名字 ID
#define KIND_ID							3				                // 名字 ID

//版本定义
#define GAME_MAX_VER					1								//现在最高版本
#define GAME_LESS_VER					1								//现在最低版本
#define GAME_CHANGE_VER					0								//修改版本

//支持类型
#define SUPPER_TYPE						1
//游戏信息

#define PLAY_COUNT						1								//游戏人数


const	int CLIENT_WIDTH		=	1136;	//屏幕宽
const	int CLIENT_HEIGHT		=	640;	//屏幕高

#define  MAX_PATH                   260

//游戏内部开发版本号
#define DEV_HEIGHT_VERSION				3				//内部开发高版本号(每修正一次LOW_VERSIONG+1)
#define DEV_LOW_VERSION					1				//内部开发低版本号

//#define SKIN_FOLDER						TEXT("ddz")	//图片文件夹
//#define SKIN							TEXT("cardskin1")	//两套图片中用一套
#define SKIN_CARD						TEXT("cardskin")		//共用图片文件夹
static char szTempStr[MAX_PATH] = {0};//暂存字符串，用来获取 dll_name 等的时候使用
#define GET_CLIENT_DLL_NAME(A)			(sprintf(A,"%d",NAME_ID),strcat(A,".ico"))
#define GET_SERVER_DLL_NAME(A)			(sprintf(A,"%d",NAME_ID),strcat(A,".dll"))
#define GET_SKIN_FOLDER(A)              (sprintf(A,"%d",NAME_ID),strcat(A,""))     //获取图片文件夹
#define GET_KEY_NAME(A, B, C, D)		(sprintf(A,"%s%d%s", B, C, D),strcat(A,""))
#define GET_CLIENT_BCF_NAME(A)          (sprintf(A,"%d",NAME_ID),strcat(A,".bcf"))    

#define MAX_NAME_INFO                   256

//游戏状态定义
#define GS_WAIT_SETGAME					0				//等待东家设置状态
#define GS_WAIT_ARGEE					1				//等待同意设置
#define GS_PLAY_GAME					20				//发牌状态
#define GS_WAIT_NEXT					23				//等待下一盘开始 

#define BET_AREA                    8               //下注区域数


#define C_S_UPDATE_USERINTO                 83              //更新名单设置
#define C_S_QUERY_STORE                     89              //查询库存   // 改为 MSG_ADMIN_CONTROL_QUERY_STORE_REQ
#define C_S_QUERY_SETTING                   90              //查询名单   // 改为 MSG_ADMIN_CONTROL_REQ
#define C_S_CLEAR_STORE						91              //清空库存   // MSG_ADMIN_CONTROL_CLEAR_STORE_REQ
#define C_S_ADD_STORE                       94              //增加库存   // MSG_ADMIN_CONTROL_ADD_STORE_REQ
#define C_S_SUB_STORE                       93              //减少库存   // MSG_ADMIN_CONTROL_SUB_STORE_REQ
/********************************************************************************/
//游戏数据包

/********************************************************************************/
/*-------------------------------------------------------------------------------*/
static  int          G_iLampToBetArea[24]={6,4,0,0,7,7,5,3,3,-1,7,6,6,4,1,1,7,5,5,2,2,-1,7,4};

#pragma pack(1)

typedef struct CMD_CTRUSER
{
	long iGameID;
	long iScoreLimit;
} CtrUser;

typedef struct CMD_CTRFULLSCREEN
{
	long iGameID;
	long iScoreLimit;
	int  iTypeID;
	int  iTimes;
} CtrFullScreen;

//管理员信息包
struct CMD_S_ADMINI
{
	CtrUser superUser[45];
	CtrUser blackUser[45];
	CtrFullScreen fullScreen[45];
	CMD_S_ADMINI()
	{
		memset(this,0,sizeof(CMD_S_ADMINI));
	}
};
////////////////////////////////////////////////////////////////////////////////////////////// 管理员控制
const int CONTROL_USER_MAX       = 15;               // 控制玩家最大数


// 超端控制消息
const int MSG_ADMIN_CONTROL_BASE                              = 100;                                             // 超端控制消息

const int MSG_ADMIN_CONTROL_REQ                               = 101;                                             // 客户端 请求 管理员 控制数据
const int MSG_ADMIN_CONTROL_RSP                               = MSG_ADMIN_CONTROL_REQ;

const int MSG_ADMIN_CONTROL_ADD_LIST_UPDATE_REQ               = 103;                                             // 更新黑白名单数据 请求  加
const int MSG_ADMIN_CONTROL_ADD_LIST_UPDATE_RSP               = MSG_ADMIN_CONTROL_ADD_LIST_UPDATE_REQ;           //                  回复

const int MSG_ADMIN_CONTROL_SUB_LIST_UPDATE_REQ               = 104;                                             // 更新黑白名单数据 请求  减
const int MSG_ADMIN_CONTROL_SUB_LIST_UPDATE_RSP               = MSG_ADMIN_CONTROL_SUB_LIST_UPDATE_REQ;           //                  回复

const int MSG_ADMIN_CONTROL_QUERY_STORE_REQ                   = 105;                                             // 查询库存 请求
const int MSG_ADMIN_CONTROL_QUERY_STORE_RSP                   = MSG_ADMIN_CONTROL_QUERY_STORE_REQ;               //          回复

const int MSG_ADMIN_CONTROL_CLEAR_STORE_REQ                   = 106;                                             // 清理库存 请求
const int MSG_ADMIN_CONTROL_CLEAR_STORE_RSP                   = MSG_ADMIN_CONTROL_CLEAR_STORE_REQ;               //          回复

const int MSG_ADMIN_CONTROL_ADD_STORE_REQ                     = 107;                                             // 增加库存 请求
const int MSG_ADMIN_CONTROL_ADD_STORE_RSP                     = MSG_ADMIN_CONTROL_ADD_STORE_REQ;                 //          回复

const int MSG_ADMIN_CONTROL_SUB_STORE_REQ                     = 108;                                             // 减少库存 请求
const int MSG_ADMIN_CONTROL_SUB_STORE_RSP                     = MSG_ADMIN_CONTROL_SUB_STORE_REQ;                 //          回复

const int MSG_ADMIN_CONTROL_GIVE_FULL_SCREEN_REQ              = 109;                                             // 赠送全屏 请求
const int MSG_ADMIN_CONTROL_GIVE_FULL_SCREEN_RSP              = MSG_ADMIN_CONTROL_GIVE_FULL_SCREEN_REQ;          //          回复

const int MSG_ADMIN_CONTROL_DELETE_FULL_SCREEN_REQ            = 110;                                             // 删除赠送全屏 请求
const int MSG_ADMIN_CONTROL_DELETE_FULL_SCREEN_RSP            = MSG_ADMIN_CONTROL_DELETE_FULL_SCREEN_REQ;          //              回复

// 消息辅助结构
struct TMSG_USER_DATA
{
	DWORD			 userID;                    // ID  
	char	         userName[61];              // 名字
	BYTE			 deskNumber;                // 座位号
	int				 haveMoney;                 // 拥有金钱 
	__int64          moneyLimit;                // 金钱限制(当控制玩家 输赢达到限制分数时 将该玩家从对应列表删除。)
	bool             isWin;                     // 控制输赢
	WORD             groupID;                   // 组ID  参考 em_Player_Group_ID

	TMSG_USER_DATA( void )
	{
		memset( this, 0, sizeof(*this) );
	}
};

struct TMSG_FULL_SCREEN_DATA
{
	TMSG_USER_DATA   stUserData;
	int              typeID;                    // 参考：em_Ico_Type_ID
	int              count;                     // 赠送次数

	TMSG_FULL_SCREEN_DATA( void )
	{
		memset( this, 0, sizeof(*this) );
	}
};


// 图标类型枚举
enum em_Ico_Type_ID
{
	em_Ico_Type_ID_Boeuf = 1,          // 牛
	em_Ico_Type_ID_Pig,                // 猪
	em_Ico_Type_ID_Dog,                // 狗
	em_Ico_Type_ID_Sheep,              // 羊
	em_Ico_Type_ID_Chicken,            // 鸡
	em_Ico_Type_ID_Grape,              // 葡萄
	em_Ico_Type_ID_Tomato,             // 西红柿
	em_Ico_Type_ID_Pumpkin,            // 南瓜
	em_Ico_Type_ID_Sunflower,          // 向日葵
	em_Ico_Type_ID_Scarecrow,          // 稻草人
	em_Ico_Type_ID_Tractor,            // 拖拉机
	em_Ico_Type_ID_House,              // 房子

	em_Ico_Type_ID_Max
};

// 玩家组ID
enum em_Player_Group_ID
{
	em_Player_Group_ID_All = 0,            // 所有玩家
	em_Player_Group_ID_White,              // 白名单玩家
	em_Player_Group_ID_Backe,              // 黑名单玩家
	em_Player_Group_ID_Full,               // 赠送玩家
	em_Player_Group_ID_FullWhite,          // 1+3
	em_Player_Group_ID_FullBacke,          // 2+3

	em_Player_Group_ID_Max
};

/////////// 消息包
// MSG_ADMIN_CONTROL_BASE                                   = 100;                                             // 超端控制消息
struct TMSG_ADMIN_CONTROL_BASE
{
	BYTE       msgID;                        // 消息码

	TMSG_ADMIN_CONTROL_BASE( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_BASE;
	}
};

// MSG_ADMIN_CONTROL_REQ                  = 101;          // 客户端 请求 管理员 控制数据
struct TMSG_ADMIN_CONTROL_REQ :
	public TMSG_ADMIN_CONTROL_BASE
{
	TMSG_ADMIN_CONTROL_REQ( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_REQ;
	}
};

// MSG_ADMIN_CONTROL_RSP                  = MSG_ADMIN_CONTROL_REQ;          // 服务器 回复 管理员 控制数据
struct TMSG_ADMIN_CONTROL_RSP :
	public TMSG_ADMIN_CONTROL_BASE
{
	WORD                     errorID;
	// TMSG_USER_DATA	         stUserData[CONTROL_USER_MAX];                    // ID  MAX_SEND_SIZE
	TMSG_FULL_SCREEN_DATA    stUserData[CONTROL_USER_MAX];
	WORD                     userNum;										  // 用户数量
	__int64                  currStore;                                       // 当前库存
	__int64                  pondStore;                                       // 全屏库存

	// 在线用户列表
	TMSG_ADMIN_CONTROL_RSP( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_RSP;
	}
};

// MSG_ADMIN_CONTROL_ADD_LIST_UPDATE_REQ             = 103;                                               // 更新黑白名单数据 请求 加
struct TMSG_ADMIN_CONTROL_ADD_LIST_UPDATE_REQ :
	public TMSG_ADMIN_CONTROL_BASE
{
	bool             isWhite;                                // 是否白名单， 否则就是黑名单
	DWORD            playerID;                               // 玩家ID
	__int64          moneyLimit;                             // 金钱限制
	bool             isWin;                                  // 输赢

	TMSG_ADMIN_CONTROL_ADD_LIST_UPDATE_REQ( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_ADD_LIST_UPDATE_REQ;
	}
};

// MSG_ADMIN_CONTROL_ADD_LIST_UPDATE_RSP             = MSG_ADMIN_CONTROL_ADD_LIST_UPDATE_REQ  ;           //                  回复
struct TMSG_ADMIN_CONTROL_ADD_LIST_UPDATE_RSP :
	public TMSG_ADMIN_CONTROL_BASE
{
	WORD            errorID;
	bool            isWhite;                                    // 是否白名单
	TMSG_USER_DATA  stUserData[CONTROL_USER_MAX];               // 白名单玩家列表
	WORD            userNum;                                    // 用户数量

	TMSG_ADMIN_CONTROL_ADD_LIST_UPDATE_RSP( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_ADD_LIST_UPDATE_RSP;
	}
};

// MSG_ADMIN_CONTROL_SUB_LIST_UPDATE_REQ               = 104;                                             // 更新黑白名单数据 请求  减
struct TMSG_ADMIN_CONTROL_SUB_LIST_UPDATE_REQ :
	public TMSG_ADMIN_CONTROL_BASE
{
	DWORD            playerID;                               // 玩家ID
	bool             isWhite;                                // 是否白名单

	TMSG_ADMIN_CONTROL_SUB_LIST_UPDATE_REQ( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_SUB_LIST_UPDATE_REQ;
	}
};

// MSG_ADMIN_CONTROL_SUB_LIST_UPDATE_RSP               = MSG_ADMIN_CONTROL_SUB_LIST_UPDATE_REQ;           //                  回复
struct TMSG_ADMIN_CONTROL_SUB_LIST_UPDATE_RSP :
	public TMSG_ADMIN_CONTROL_BASE
{
	WORD            errorID;
	bool            isWhite;                                    // 是否白名单
	TMSG_USER_DATA  stUserData[CONTROL_USER_MAX];               // 名单玩家列表
	WORD            userNum;                                    // 用户数量

	TMSG_ADMIN_CONTROL_SUB_LIST_UPDATE_RSP( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_SUB_LIST_UPDATE_RSP;
	}
};

// MSG_ADMIN_CONTROL_QUERY_STORE_REQ                   = 105;                                             // 查询库存 请求
struct TMSG_ADMIN_CONTROL_QUERY_STORE_REQ :
	public TMSG_ADMIN_CONTROL_BASE
{
	TMSG_ADMIN_CONTROL_QUERY_STORE_REQ( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_QUERY_STORE_REQ;
	}
};

// MSG_ADMIN_CONTROL_QUERY_STORE_RSP                   = MSG_ADMIN_CONTROL_QUERY_STORE_REQ;               //          回复
struct TMSG_ADMIN_CONTROL_QUERY_STORE_RSP :
	public TMSG_ADMIN_CONTROL_BASE
{
	WORD             errorID;
	__int64          currStore;              // 当前库存
	__int64          pondStore;              // 全屏库存

	TMSG_ADMIN_CONTROL_QUERY_STORE_RSP( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_QUERY_STORE_RSP;
	}
};

// MSG_ADMIN_CONTROL_CLEAR_STORE_REQ                   = 106;                                             // 清理库存 请求
struct TMSG_ADMIN_CONTROL_CLEAR_STORE_REQ :
	public TMSG_ADMIN_CONTROL_BASE
{
	TMSG_ADMIN_CONTROL_CLEAR_STORE_REQ( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_CLEAR_STORE_REQ;
	}
};

// MSG_ADMIN_CONTROL_CLEAR_STORE_RSP                   = MSG_ADMIN_CONTROL_CLEAR_STORE_REQ;               //          回复
struct TMSG_ADMIN_CONTROL_CLEAR_STORE_RSP :
	public TMSG_ADMIN_CONTROL_BASE
{
	WORD            errorID;

	TMSG_ADMIN_CONTROL_CLEAR_STORE_RSP( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_CLEAR_STORE_RSP;
	}
};

// MSG_ADMIN_CONTROL_ADD_STORE_REQ                     = 107;                                             // 增加库存 请求
struct TMSG_ADMIN_CONTROL_ADD_STORE_REQ :
	public TMSG_ADMIN_CONTROL_BASE
{
	TMSG_ADMIN_CONTROL_ADD_STORE_REQ( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_ADD_STORE_REQ;
	}
};

// MSG_ADMIN_CONTROL_ADD_STORE_RSP                     = MSG_ADMIN_CONTROL_ADD_STORE_REQ;                 //          回复
struct TMSG_ADMIN_CONTROL_ADD_STORE_RSP :
	public TMSG_ADMIN_CONTROL_BASE
{
	WORD             errorID;
	__int64          currStore;              // 当前库存
	__int64          pondStore;              // 全屏库存

	TMSG_ADMIN_CONTROL_ADD_STORE_RSP( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_ADD_STORE_RSP;
	}
};

// MSG_ADMIN_CONTROL_SUB_STORE_REQ                      = 108;                                            // 减少库存 请求
struct TMSG_ADMIN_CONTROL_SUB_STORE_REQ :
	public TMSG_ADMIN_CONTROL_BASE
{
	TMSG_ADMIN_CONTROL_SUB_STORE_REQ( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_SUB_STORE_REQ;
	}
};

// MSG_ADMIN_CONTROL_SUB_STORE_RSP                      = MSG_ADMIN_CONTROL_SUB_STORE_REQ;                //          回复
struct TMSG_ADMIN_CONTROL_SUB_STORE_RSP :
	public TMSG_ADMIN_CONTROL_BASE
{
	WORD             errorID;
	__int64          currStore;              // 当前库存
	__int64          pondStore;              // 全屏库存

	TMSG_ADMIN_CONTROL_SUB_STORE_RSP( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_SUB_STORE_RSP;
	}
};

// MSG_ADMIN_CONTROL_GIVE_FULL_SCREEN_REQ              = 109;                                             // 赠送全屏 请求
struct TMSG_ADMIN_CONTROL_GIVE_FULL_SCREEN_REQ :
	public TMSG_ADMIN_CONTROL_BASE
{
	DWORD			 userID;                    // ID  
	__int64          moneyLimit;                // 金钱限制
	BYTE             typeID;                    // 赠送类型  参考：em_Ico_Type_ID
	int              count;                     // 赠送次数

	TMSG_ADMIN_CONTROL_GIVE_FULL_SCREEN_REQ( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_GIVE_FULL_SCREEN_REQ;
	}
};

// MSG_ADMIN_CONTROL_GIVE_FULL_SCREEN_RSP              = MSG_ADMIN_CONTROL_GIVE_FULL_SCREEN_REQ;          //          回复
struct TMSG_ADMIN_CONTROL_GIVE_FULL_SCREEN_RSP :
	public TMSG_ADMIN_CONTROL_BASE
{
	WORD                   errorID;
	TMSG_FULL_SCREEN_DATA  stFullData[CONTROL_USER_MAX];               // 全屏玩家列表
	WORD                   userNum;                                    // 用户数量

	TMSG_ADMIN_CONTROL_GIVE_FULL_SCREEN_RSP( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_GIVE_FULL_SCREEN_RSP;
	}
};

// MSG_ADMIN_CONTROL_DELETE_FULL_SCREEN_REQ            = 110;                                             // 删除赠送全屏 请求
struct TMSG_ADMIN_CONTROL_DELETE_FULL_SCREEN_REQ :
	public TMSG_ADMIN_CONTROL_BASE
{
	DWORD			 userID;                    // ID  

	TMSG_ADMIN_CONTROL_DELETE_FULL_SCREEN_REQ( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_DELETE_FULL_SCREEN_REQ;
	}
};

// MSG_ADMIN_CONTROL_DELETE_FULL_SCREEN_RSP            = MSG_ADMIN_CONTROL_DELETE_FULL_SCREEN_REQ;          //              回复
struct TMSG_ADMIN_CONTROL_DELETE_FULL_SCREEN_RSP :
	public TMSG_ADMIN_CONTROL_BASE
{
	WORD                   errorID;
	TMSG_FULL_SCREEN_DATA  stFullData[CONTROL_USER_MAX];               // 全屏玩家列表
	WORD                   userNum;                                    // 用户数量

	TMSG_ADMIN_CONTROL_DELETE_FULL_SCREEN_RSP( void )
	{
		memset( this, 0, sizeof(*this) );
		msgID = MSG_ADMIN_CONTROL_DELETE_FULL_SCREEN_RSP;
	}
};


//游戏状态基础数据包
struct	S_C_GameStation
{
	BYTE			    byGameStation;					//游戏状态
	__int64             i64UserMoney;
	__int64             i64Pond;
	int                 iAreaBetLimit;        //区域下注限制
	S_C_GameStation()
	{
		memset(this, 0, sizeof(S_C_GameStation));
	}
};
/*-------------------------------------------------------------------------------*/

//超端
struct S_C_SuperUserStru
{
	BYTE byDeskStation;
	S_C_SuperUserStru()
	{
		byDeskStation=255;
	}
};

//超端控制开奖结果
struct C_S_SuperControl
{
	BYTE byOpenResult;
	C_S_SuperControl()
	{
		byOpenResult=255; 
	}
};


//游戏开始数据包
struct S_C_GameBegin
{
	bool	bStart;
	S_C_GameBegin()
	{
		memset(this, 0, sizeof(S_C_GameBegin));
	}
};

/*-------------------------------------------------------------------------------*/
//开始滚动
struct	C_S_StartRoll
{
	//当前上注
	int         iAreaBetNum[BET_AREA];
	C_S_StartRoll()
	{
		memset(this, 0, sizeof(C_S_StartRoll));
	}
};

//摇奖结果
struct S_C_RollResult
{
	int iLBeilv;
	int iRBeilv;
	int iLamp; //要开的灯
	int iAreaWin;//玩家获取的奖励
	int iCount; //要送的灯数量
	S_C_RollResult()
	{
		memset(this,0,sizeof(S_C_RollResult));
	}
};
struct CMD_S_Pond
{
	bool bFlag;
	char szNickName[61];
	__int64 i64GetPond;
	__int64 i64Pond;

	CMD_S_Pond()
	{
		memset(this,0,sizeof(CMD_S_Pond));
	}
};
struct C_S_BiBeiStru
{
	int iBibeiType; //比倍类型（1-6，7-13）
	int iBibeiMoney; //比倍金币
	C_S_BiBeiStru()
	{
		iBibeiType=0;
	}
};

struct  S_C_BiBeiResultStru
{
	int iwinmoney;
	int iType; //1代表开小，2代表开大
	S_C_BiBeiResultStru()
	{
		memset(this,0,sizeof(this));
	}
};
struct   C_S_AwardLamp
{
	bool bSend;//是否可以发送送灯奖励了
	C_S_AwardLamp()
	{
		bSend=false;
	}
};

struct  S_C_AwardLampResult
{
	int iLamp;     //要开的灯
	int iAreaWin;  //玩家获取的奖励
	int iCount;    //要送的灯数量
	S_C_AwardLampResult()
	{
		memset(this,0,sizeof(S_C_AwardLampResult));
	}
};
#pragma pack()
/********************************************************************************/

// 数据包处理辅助标识

/**********************************************************************/
/**************************服务端——>客户端消息**************************/
#define S_C_GAME_BEGIN					51				//游戏开始

#define S_C_ROLL_RESULT					    52				//滚动结果

/**************************客户端——>服务端消息**************************/
#define	C_S_START_ROLL					    53				//启动游戏滚动
#define C_S_BIBEI                           54                //比倍
#define S_C_BIBEI_RESULT                     55                //比倍结果
#define C_S_AWARD_LAMP                        56               //客户端发过来的请求发送奖励灯的数据
#define S_C_AWARD_LAMP_REULT                   57               //服务端回应
#define S_C_SUPER_USER                        58               //给客户端发送超端确认信息
#define C_S_SUPER_RESULT                      59               //超端控制的结果
/**********************************************************************/
#endif
