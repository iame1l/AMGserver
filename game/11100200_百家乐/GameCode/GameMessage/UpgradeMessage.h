// GameMsgPkg.h: game message package and define const.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_UPGRADEMESSAGE_H__INCLUDED_)
#define _UPGRADEMESSAGE_H__INCLUDED_

#pragma pack(1)

#define CHECK_DEBUG   1

// 游戏名称信息
#define GAMENAME						TEXT("百家乐")                          // 游戏名称
#define GAME_DLL_AUTHOR                 TEXT("深圳红鸟网络科技")            //
#define GAME_DLL_NOTE                   TEXT("牌类 -- 游戏组件")


#define NAME_ID							11100200								 // 名字 ID


#define MAX_NAME_INFO                   256
#define GET_STRING(NUM)                 #NUM
#define INT_TO_STR(NUM)                 GET_STRING(NUM)

#define CLIENT_DLL_NAME                 TEXT(INT_TO_STR(NAME_ID)##".ico")        // 客户端ico名字
#define SKIN_FOLDER                     TEXT(INT_TO_STR(NAME_ID))                // 客户端目录
#define SKIN_CARD						TEXT("CardSkin")		                 // 共用图片文件夹

// 版本定义
#define GAME_MAX_VER					1								        // 现在最高版本
#define GAME_LESS_VER					1								        // 现在最低版本
#define GAME_CHANGE_VER					0								        // 修改版本

#define	UG_VALUE_MASK					0x0F			//0000 1111
// 支持类型
#define SUPPER_TYPE						SUP_NORMAL_GAME|SUP_MATCH_GAME|SUP_MONEY_GAME

#define PLAY_COUNT						180							        // 游戏人数

//游戏内部开发版本号
#define DEV_HIGH_VERSION				2				                        // 内部开发高版本号(每修正一次LOW_VERSIONG+1)
#define DEV_LOW_VERSION					1				                        // 内部开发低版本号

// 游戏状态定义
#define GS_WAIT_SETGAME					0				// 无庄等待
#define GS_WAIT_AGREE					1				// 等待同意设置
#define GS_NOTE_STATE					20				// 下注状态
#define GS_SEND_CARD					21				// 发牌状态
#define GS_SHOW_WIN						22				// 显示中奖区域
#define GS_WAIT_NEXT					23				// 等待下一盘开始 


//游戏记录把数
#define MAXCOUNT						50            


const	int	BET_ARES = 8;	//8个下注区域
/*--------------------------------------------------------------------------*/
struct LuziData
{
	BYTE		byZPoint;
	BYTE		byXpoint;

	BYTE		byWinResult;	//赢的区域 0=闲 1-庄 2-和


	LuziData()
	{
		memset(this,255,sizeof(LuziData));
	}
};
//所有玩家玩游戏的一盘记录  是否下注了 输赢多少
struct UserPlayResult
{
	bool	bHaveXiaZhu[PLAY_COUNT];	//是否下注了
	int		iWinMoney[PLAY_COUNT];		//输赢多少	
	UserPlayResult()
	{
		memset(this,0,sizeof(UserPlayResult));
	}
};
/*--------------------------------------------------------------------------*/
//申请上庄数据包
struct C_S_ApplyShangZhuang
{	
	bool	bShangZhuang;		//是否上庄 true -上庄  false - 下庄
	C_S_ApplyShangZhuang()
	{
		memset(this,0,sizeof(C_S_ApplyShangZhuang));
	}
};


//申请上庄数据包
struct S_C_ApplyZhuangResult
{	
	bool		bXiaZhuang;					//是否庄家申请下庄

	BYTE		byDeskStation;				//申请的玩家
	BYTE		byZhuangList[PLAY_COUNT];	//上庄列表

	int			iNowNtStation;				//庄家位置
	int			iZhuangBaShu;				//庄家把数

	__int64		i64NtMoney;					//玩家金币数量
	__int64		i64NtWin;					/// 当前庄家赢的金币  

	S_C_ApplyZhuangResult()
	{
		memset(	byZhuangList,255,sizeof(byZhuangList));
		iNowNtStation = -1;
		iZhuangBaShu = 0;
		i64NtMoney = 0;
		i64NtWin = 0;
		bXiaZhuang = false;	
		byDeskStation = 255;
	}
};
/*--------------------------------------------------------------------*/
//无庄等待消息
struct	S_C_NoNtStation
{
	int			iNowNtStation;	///当前庄家的位置
	int			iNtPlayCount;	//庄家坐庄次数
	__int64		i64NtMoney;		//庄家金币
	__int64		i64NtWinMoney;	//庄家输赢情况
	S_C_NoNtStation()
	{
		iNowNtStation = -1;	///当前庄家的位置
		iNtPlayCount = 0;	//庄家坐庄次数
		i64NtMoney	= 0;		//庄家金币
		i64NtWinMoney = 0;	//庄家输赢情况
	}
};
/*--------------------------------------------------------------------------*/

///游戏开始信息
typedef struct S_C_GameBegin
{
	BYTE		byZhuangList[PLAY_COUNT];	//上庄列表

	int			iGameCount;		///当前已经进行了几把
	int			iGameBeen;		///当前局数
	int			iNowNtStation;	///当前庄家的位置
	int			iNtPlayCount;	//庄家坐庄次数
	__int64		i64NtMoney;		//庄家金币
	__int64		i64NtWinMoney;	//庄家输赢情况
	__int64		i64UserMoney;	//玩家自身的金币数量
	__int64		i64MaxXiaZhu;	//最大下注总注
	__int64	    i64AreaMaxZhu[BET_ARES];     //每个区域能下的最大注	


	S_C_GameBegin()
	{
		memset(this, 0, sizeof(S_C_GameBegin));
		iNowNtStation=-1;
		memset(byZhuangList,255,sizeof(byZhuangList));
	}
};
/*--------------------------------------------------------------------*/
struct	C_S_UserNote
{
	BYTE	byDeskStation;
	int		iChouMaType;		//筹码类型
	int		iNoteArea;			//下注区域
	C_S_UserNote()
	{
		byDeskStation = -1;
		iChouMaType = -1;
		iNoteArea = -1;

	}
};
/*--------------------------------------------------------------------*/
struct	C_S_UserNoteResult
{
	BYTE		byDeskStation;					//下注玩家
	int			iArea;
	__int64		i64Money;
	__int64		i64ZhongZhu;					/// 本把当前总注额
	__int64		i64QuYuZhuTrue[BET_ARES];		///真实玩家的下注值
	__int64		i64QuYuZhu[BET_ARES];			/// 本把每个区域下的注额
	__int64     i64UserXiaZhuData[BET_ARES];	//玩家下注情况
	__int64		i64AreaMaxZhu[BET_ARES];		//每个区域还能下多少注
	C_S_UserNoteResult()
	{
		memset(this,0,sizeof(C_S_UserNoteResult));
	}
};
/*--------------------------------------------------------------------*/
///开牌数据包
struct S_C_SendCard
{
	BYTE	byUserCard[2][3];			//庄闲的牌，0为闲，1为庄
	int  	iZPaiXing[5];				//庄家牌型,元素0前两张牌的值，元素1总牌值，元素2天王，元素3对子，元素4和
	int     iXPaiXing[5];				//闲家牌型
	int     iWinQuYu[BET_ARES];			//游戏的赢钱区域 值=0 表示没中奖  大于0的时候 表示中奖了  0闲，1闲天王，2闲对子，3庄，4庄天王，5庄对子，6和，7同点和	
	S_C_SendCard()
	{
		memset(this, 0, sizeof(S_C_SendCard));
	}
};
/*--------------------------------------------------------------------------*/
//中奖区域数据包
struct S_C_ShowWinAreas
{	
	int		iWinQuYu[BET_ARES];			/**< 游戏的赢钱区域 0闲，1闲天王，2闲对子，3庄，4庄天王，5庄对子，6和，7同点和*/
	S_C_ShowWinAreas()
	{
		memset(this,0,sizeof(S_C_ShowWinAreas));
	}
};
/*--------------------------------------------------------------------*/
struct	S_C_GameResult
{
	BYTE		byZhuangList[PLAY_COUNT];	//上庄列表
	float		fUserPercent;				//玩家命中率
	__int64		i64UserAreaScore[BET_ARES];	//玩家各区域得分情况

	__int64		i64NtScoreSum;				//庄家本局总得分
	__int64		i64UserScoreSum;			//玩家本局得分

	__int64		i64NtWin;			//庄家输赢成绩
	__int64		i64NtMoney;			//庄家的金币数量

	__int64		i64UserWin;			//玩家输赢成绩
	__int64		i64UserMoney;		//玩家自身的金币

	LuziData	TLuziData[MAXCOUNT];	//路子信息

	S_C_GameResult()
	{
		memset(this,0,sizeof(S_C_GameResult));
		memset(byZhuangList,255,sizeof(byZhuangList));
		memset(TLuziData,255,sizeof(TLuziData));
	}
};
/*--------------------------------------------------------------------------*/
#define CHOUMANUM 7

struct sGamestationBase
{
	__int64	i64JettonVal[CHOUMANUM];
	sGamestationBase()
	{
		memset(this, 0, sizeof(*this));
	}
};

/*--------------------------------------------------------------------------*/
//游戏基础数据
struct	GameStationBase : sGamestationBase
{
	BYTE		byZhuangList[PLAY_COUNT];	//上庄列表	
	LuziData	TLuziData[MAXCOUNT];		//路子信息

	int			iXiaZhuTime;		/// 下注时间					
	int			iKaiPaiTime;     	/// 开牌时间    
	int			iFreeTime;			/// 空闲时间	
	int			iShowWinTime;		/// 显示中奖时间


	int			iNtStation;			//庄家位置
	int			iNtPlayCount;		//庄家坐庄次数
	__int64		i64NtMoney;			//庄家金币
	__int64		i64NtWinMoney;		//庄家输赢情况

	__int64		i64UserWin;			//个人输赢
	__int64		i64MyMoney;			//个人金币数 -从服务端发送过去


	__int64		i64UserMaxNote;			///玩家最大下注数
	__int64		i64ShangZhuangLimit;	/// 上庄需要的最少金币	
	


	GameStationBase()
	{
		memset(this, 0, sizeof(GameStationBase));
		memset(byZhuangList,255,sizeof(byZhuangList));
		memset(TLuziData,255,sizeof(TLuziData));
	}
};
/*--------------------------------------------------------------------------*/
//下注状态数据
struct	GameStation_Bet : sGamestationBase
{
	BYTE		byZhuangList[PLAY_COUNT];	//上庄列表	
	LuziData	TLuziData[MAXCOUNT];	//路子信息

	int			iXiaZhuTime;		/// 下注时间					
	int			iKaiPaiTime;     	/// 开牌时间    
	int			iFreeTime;			/// 空闲时间	
	int			iShowWinTime;		/// 显示中奖时间


	int			iNtStation;			//庄家位置
	int			iNtPlayCount;		//庄家坐庄次数
	__int64		i64NtMoney;			//庄家金币
	__int64		i64NtWinMoney;		//庄家输赢情况

	__int64		i64UserWin;			//个人输赢
	__int64		i64MyMoney;			//个人金币数 -从服务端发送过去

	__int64		i64UserMaxNote;			///玩家最大下注数
	__int64		i64ShangZhuangLimit;	/// 上庄需要的最少金币	

	int			iRemainTimeXZ;		/// 该玩家下注剩余时间显示
	__int64		i64ZhongZhu;   					/**< 本把当前总注额*/
	__int64		i64QuYuZhu[BET_ARES]; 			/**< 本把每个区域下的注额*/
	__int64		i64QuYuZhuTrue[BET_ARES];		///真实玩家的下注值
	__int64		i64UserXiaZhuData[BET_ARES];	//玩家区域下注信息
	__int64	    i64AreaMaxZhu[BET_ARES];		//每个区域能下的最大注	
	

	GameStation_Bet()
	{
		memset(this,0,sizeof(GameStation_Bet));
		memset(byZhuangList,255,sizeof(byZhuangList));
		memset(TLuziData,255,sizeof(TLuziData));
	}
};
/*--------------------------------------------------------------------------*/
//开牌状态
struct	GameStation_SendCard : sGamestationBase
{
	BYTE		byZhuangList[PLAY_COUNT];	//上庄列表	
	LuziData	TLuziData[MAXCOUNT];	//路子信息
	
	int			iXiaZhuTime;		/// 下注时间					
	int			iKaiPaiTime;     	/// 开牌时间    
	int			iFreeTime;			/// 空闲时间	
	int			iShowWinTime;		/// 显示中奖时间

	

	int			iNtStation;			//庄家位置
	int			iNtPlayCount;		//庄家坐庄次数
	__int64		i64NtMoney;			//庄家金币
	__int64		i64NtWinMoney;		//庄家输赢情况

	__int64		i64UserWin;			//个人输赢
	__int64		i64MyMoney;			//个人金币数 -从服务端发送过去

	__int64		i64UserMaxNote;			///玩家最大下注数
	__int64		i64ShangZhuangLimit;	/// 上庄需要的最少金币	

	int			RemainTimeKP;		///开牌剩余时间
	__int64		i64ZhongZhu;   					/**< 本把当前总注额*/
	__int64		i64QuYuZhu[BET_ARES]; 			/**< 本把每个区域下的注额*/
	__int64		i64QuYuZhuTrue[BET_ARES];		///真实玩家的下注值
	__int64		i64UserXiaZhuData[BET_ARES];	//玩家区域下注信息
	__int64	    i64AreaMaxZhu[BET_ARES];		//每个区域能下的最大注	

	BYTE		byUserCard[2][3];        //庄闲的牌，0为闲，1为庄
	int  		iZPaiXing[5];			//庄家牌型,元素0前两张牌的值，元素1总牌值，元素2天王，元素3对子，元素4和
	int			iXPaiXing[5];			//闲家牌型

	
	GameStation_SendCard()
	{
		memset(this,0,sizeof(GameStation_SendCard));
		memset(byZhuangList,255,sizeof(byZhuangList));
		memset(TLuziData,255,sizeof(TLuziData));
	}
};
/*--------------------------------------------------------------------------*/
//结算状态数据
struct	GameStation_ShowWin : sGamestationBase
{
	BYTE		byZhuangList[PLAY_COUNT];	//上庄列表	
	LuziData	TLuziData[MAXCOUNT];	//路子信息

	

	int			iXiaZhuTime;		/// 下注时间					
	int			iKaiPaiTime;     	/// 开牌时间    
	int			iFreeTime;			/// 空闲时间	
	int			iShowWinTime;		/// 显示中奖时间

	int			iNtStation;			//庄家位置
	int			iNtPlayCount;		//庄家坐庄次数
	__int64		i64NtMoney;			//庄家金币
	__int64		i64NtWinMoney;		//庄家输赢情况

	__int64		i64UserWin;			//个人输赢
	__int64		i64MyMoney;			//个人金币数 -从服务端发送过去

	__int64		i64UserMaxNote;			///玩家最大下注数
	__int64		i64ShangZhuangLimit;	/// 上庄需要的最少金币	

	int			RemainTimeZJ;		///剩余中奖区域时间显示
	__int64		i64ZhongZhu;   					/**< 本把当前总注额*/
	__int64		i64QuYuZhu[BET_ARES]; 			/**< 本把每个区域下的注额*/
	__int64		i64QuYuZhuTrue[BET_ARES];		///真实玩家的下注值
	__int64		i64UserXiaZhuData[BET_ARES];	//玩家区域下注信息
	__int64	    i64AreaMaxZhu[BET_ARES];		//每个区域能下的最大注	
	int			iWinQuYu[BET_ARES];		//游戏的赢钱区域 0闲，1闲天王，2闲对子，3庄，4庄天王，5庄对子，6和，7同点和

	GameStation_ShowWin()
	{
		memset(this,0,sizeof(GameStation_ShowWin));
		memset(byZhuangList,255,sizeof(byZhuangList));
		memset(TLuziData,255,sizeof(TLuziData));
	}
};
/*------------------------------------------------------------------------------*/
/// 通知客户端是否为超级客户端状态消息结构
struct S_C_IsSuperUser
{
	BYTE byDeskStation;      /**< 玩家位置 */
	bool bEnable;           /**< 是否开通 */
	S_C_IsSuperUser()
	{
		memset(this,0,sizeof(S_C_IsSuperUser));
	}
};
/*------------------------------------------------------------------------------*/
struct SuperUserSetData
{
	bool	bSetSuccese;			//是否设置成功了
	int		iSetResult[BET_ARES];	//设置结果

	SuperUserSetData()
	{
		memset(this,0,sizeof(SuperUserSetData));
	}
};





/*--------------------------------------------------------------------------*/
#define S_C_IS_SUPER_USER			78		//超端用户消息
#define C_S_SUPER_SET				79		//超端设置
#define S_C_SUPER_SET_RESULT		80		//超端设置结果

#define C_S_APPLY_ZHUANG			90		/// 申请上下庄
#define S_C_APPLY_ZHUANG_RESULT		91		/// 申请上下庄结果
#define C_S_XIA_ZHU					92		/// 玩家下注
#define S_C_XIA_ZHU_RESULT			93		/// 玩家下注结果
#define S_C_PLAYER_OUTRT			94		/// 超过玩家最大下注数
#define S_C_ALLMONEY_OUTRT			95		/// 已经超过本局最大下注数
#define S_C_PLAYERMONEY_LOSER		96		/// 该玩家下注金币不足

#define S_C_NO_NT_WAITE				100		/// 通知无庄等待
#define S_C_NOTICE_XIA_ZHU			101		/// 通知下注消息
#define S_C_SEND_CARD				102		//发牌消息
#define S_C_SHOW_WINAREA			104		//显示中奖区域消息
#define S_C_GAME_END				105		//游戏结束



#pragma pack()
#endif // !defined(_POKERMESSAGE_H__INCLUDED_)
