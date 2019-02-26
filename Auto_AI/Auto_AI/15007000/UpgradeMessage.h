#pragma once
#ifndef AFC_SERVER_SHOWHAND_MESSAGE_HEAD_FILE
#define AFC_SERVER_SHOWHAND_MESSAGE_HEAD_FILE

#pragma pack(1)

#define GAMENAME						TEXT("新龙虎斗")
#define NAME_ID							15007000					    //名字 ID
#define PLAY_COUNT						180								//游戏人数

//版本定义
#define GAME_MAX_VER					1								//现在最高版本
#define GAME_LESS_VER					1								//现在最低版本
#define GAME_CHANGE_VER					0								//修改版本


//支持类型
#define SUPPER_TYPE						SUP_NORMAL_GAME|SUP_MATCH_GAME|SUP_MONEY_GAME

#define GAME_TABLE_NAME					"MatchTable"

#define GET_STRING(NUM)                 #NUM
#define INT_TO_STR(NUM)                 GET_STRING(NUM)
#define CLIENT_DLL_NAME                 TEXT(INT_TO_STR(NAME_ID)##".ico")        // 客户端ico名字
#define SKIN_FOLDER                     TEXT(INT_TO_STR(NAME_ID))                // 客户端目录


//游戏开发版本
#define DEV_HEIGHT_VERSION				1								//内部开发高版本号
#define DEV_LOW_VERSION					0 								//内部开发低版本号

//游戏状态定义
#define GS_WAIT_SETGAME					0				// 游戏初始等待状态
#define GS_NOTE_STATE					20				// 下注状态
#define GS_SEND_CARD					21				// 发牌状态
#define GS_SEND_RESULT					22				// 结算状态
#define GS_WAIT_NEXT					23				// 等待下一盘开始 


//游戏记录把数
#define MAXCOUNT						180            

const	int	BET_ARES = 3;	//3个下注区域    下标0表示龙区域  1虎   2 和
const   int CHOUMA_NUM= 6;  //筹码个数
const   int USER_MAX_MONEY_NUM = 6;
//5个筹码值
const	__int64	G_i64ChouMaValues[CHOUMA_NUM] = {1,10,50,100,500,1000};


struct GameStation
{
	BYTE         byRunLongCount;      //中龙的局数个数
	BYTE         byRunHuCount;        //中虎的局数个数
	BYTE         byRunHeCount;        //中和的局数个数
	BYTE         iGameBeen;		      ///当前局数
	BYTE         byRunSeq[MAXCOUNT];  //每局运行结果序列  1：龙    2：虎  3：和
	BYTE		 userMaxMoney[USER_MAX_MONEY_NUM];       //金币数最多的6个人
	int			 iXiaZhuTime;		/// 下注时间					
	int			 iKaiPaiTime;     	/// 开牌时间 
	int			 iFreeTime;			/// 空闲时间
	__int64		 i64MyMoney;			//个人金币数 -从服务端发送过去

	GameStation()
	{
		memset(this,0,sizeof(GameStation));
		memset(userMaxMoney,255,sizeof(userMaxMoney));
	}
};

//下注状态数据
struct GameStation_PlayGame : GameStation
{
	bool        bIsShenSuanBet[BET_ARES];               //神算子每个区域是否下注
	int			iRemainTimeXZ;							// 该玩家下注剩余时间显示
	int			rAresData[BET_ARES][CHOUMA_NUM];		 //每个区域里的各种筹码的个数
	__int64		i64QuYuZhu[BET_ARES]; 					 /**< 本把每个区域已下的总注额*/
	__int64		i64UserXiaZhuData[BET_ARES];			 //玩家区域下注信息

	GameStation_PlayGame()
	{
		memset(this,0,sizeof(GameStation_PlayGame));
		memset(userMaxMoney,255,sizeof(userMaxMoney));
	}
};

//开牌状态
struct GameStation_SendCard : GameStation
{
	bool        bIsShenSuanBet[BET_ARES];               //神算子每个区域是否下注
	BYTE		byShowCard[2];							 //开牌数据下标0为龙的牌值，1为虎
	int			RemainTimeKP;						 	 ///开牌剩余时间
	int			rAresData[BET_ARES][CHOUMA_NUM];		 //每个区域里的各种筹码的个数
	__int64		i64QuYuZhu[BET_ARES]; 					 /**< 本把每个区域已下的总注额*/
	__int64		i64UserXiaZhuData[BET_ARES];			 //玩家区域下注信息

	GameStation_SendCard()
	{
		memset(this,0,sizeof(GameStation_SendCard));
		memset(userMaxMoney,255,sizeof(userMaxMoney));
	}
};

//结算状态
struct GameStation_Result : GameStation
{
	BYTE		byDeskStation;		//下注玩家
	BYTE		byShowCard[2];		 //开牌数据下标0为龙的牌值，1为虎
	__int64		i64UserWin;			//玩家输赢成绩
	__int64     i64MaxUserWin[USER_MAX_MONEY_NUM];   //金币最多的6个玩家输赢成绩
	GameStation_Result()
	{
		memset(this,0,sizeof(GameStation_Result));
		byDeskStation = 255;
		memset(userMaxMoney,255,sizeof(userMaxMoney));
	}
};

//超端验证消息包
struct	S_C_SuperUser
{
	BYTE byDeskStation;      //超端玩家的位置
	bool bIsSuper;           // 是否开通
	S_C_SuperUser()
	{
		memset(this,0,sizeof(S_C_SuperUser));
		byDeskStation = 255;
	}
};
//超端设置中奖的区域
struct SuperUserSetData
{
	BYTE	byDeskStation;      //超端玩家的位置 
	BYTE	byWinQuYu;			//设置中奖的区域是   1：龙   2：虎  3：和
	bool	bSetSuccess;		//是否设置成功了

	SuperUserSetData()
	{
		memset( this, 0, sizeof( SuperUserSetData ) );
		byDeskStation = 255;
	}
};


///游戏开始信息
struct S_C_GameBegin
{
	BYTE	    iGameBeen;		   ///当前局数
	BYTE		userMaxMoney[USER_MAX_MONEY_NUM];	//最多金币的6个玩家
	BYTE        byRunSeq[MAXCOUNT];
	__int64		i64UserMoney;	   //玩家自身的金币数量

	S_C_GameBegin()
	{
		memset(this, 0, sizeof(S_C_GameBegin));
		memset(userMaxMoney, 255, sizeof(userMaxMoney));
	}
};

///游戏中玩家金币数最多的6个人更新
struct S_C_UserMaxMoney
{
	BYTE		userMaxMoney[USER_MAX_MONEY_NUM];	//最多金币的6个玩家
	S_C_UserMaxMoney()
	{
		memset(this, 255, sizeof(S_C_UserMaxMoney));
	}
};
//客户端发送的下注消息
struct	C_S_UserNote
{
	BYTE	    byDeskStation;
	__int64		iChouMaValues;		//筹码值
	int		    byQuYu;			//下注区域  1：龙    2：虎  3：和
	C_S_UserNote()
	{
		byDeskStation = 255;
		iChouMaValues = -1;
		byQuYu = -1;
	}
};

//服务端发送下注结果
struct	C_S_UserNoteResult
{
	BYTE		byDeskStation;					//下注玩家
	BYTE	    byQuYu;			                //下注区域   1：龙    2：虎  3：和
	__int64     iChouMaValues;                  //下注的筹码值
	__int64		i64MyMoney;						//个人金币数 -从服务端发送过去
	__int64		i64QuYuZhu[BET_ARES]; 		    /**< 本把每个区域已下的总注额*/

	C_S_UserNoteResult()
	{
		memset(this,0,sizeof(C_S_UserNoteResult));
		byDeskStation = 255;
	}
};

///开牌数据包
struct S_C_SendCard
{
	BYTE		byShowCard[2];							 //开牌数据下标0为龙的牌值，1为虎
	S_C_SendCard()
	{
		memset(this, 0, sizeof(S_C_SendCard));
	}
};

//中奖区域数据包
struct S_C_ShowWinAreas
{	
	BYTE	     byWinQuYu;			  //中奖区域   1：龙    2：虎  3：和
	BYTE         byRunLongCount;      //中龙的局数个数
	BYTE         byRunHuCount;        //中虎的局数个数
	BYTE         byRunHeCount;        //中和的局数个数
	BYTE         iGameBeen;		      ///当前局数
	BYTE         byRunSeq[MAXCOUNT];  //每局运行结果序列  1：龙    2：虎  3：和
	S_C_ShowWinAreas()
	{
		memset(this,0,sizeof(S_C_ShowWinAreas));
	}
};

//结算数据
struct	S_C_GameResult
{
	BYTE		byDeskStation;		//下注玩家
	BYTE		userMaxMoney[6];	//最多金币的6个玩家
	__int64		i64UserWin;			//玩家输赢成绩
	__int64		i64UserMoney;		//玩家自身的金币
	__int64     i64MaxUserWin[USER_MAX_MONEY_NUM];   //金币最多的6个玩家输赢成绩

	S_C_GameResult()
	{
		memset(this,0,sizeof(S_C_GameResult));
		byDeskStation = 255;
		memset(userMaxMoney,255,sizeof(userMaxMoney));
	}
};

#define S_C_IS_SUPER_USER		    10	    //通知是超端            S_C_SuperUser
#define ASS_SUPER_USER_SET          11      //超端用户设置中奖区域   SuperUserSetData
#define ASS_SUPER_USER_SET_RESULT	12	    //返回设置结果消息       SuperUserSetData
#define C_S_XIA_ZHU					92		/// 玩家下注             C_S_UserNote
#define S_C_XIA_ZHU_RESULT			93		/// 玩家下注结果         C_S_UserNoteResult
#define S_C_PLAYERMONEY_LOSER		96		/// 该玩家下注金币不足   
#define S_C_USER_MAX_MONEY          97      //金币数最多6个人更新      S_C_UserMaxMoney
#define S_C_NOTICE_XIA_ZHU			101		/// 通知下注消息       S_C_GameBegin
#define S_C_SEND_CARD				102		//发牌消息             S_C_SendCard
#define S_C_SHOW_WINAREA			104		//显示中奖区域消息     S_C_ShowWinAreas
#define S_C_GAME_RESULT			    105		//发送结算数据         S_C_GameResult

#pragma pack()

#endif
