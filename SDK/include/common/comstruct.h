/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#ifndef AFC_COM_STRUCT_HEAD_FILE
#define AFC_COM_STRUCT_HEAD_FILE

/********************************************************************************************/
#include "PropMessage.h"

///房间规则设置
//#define GRR_MEMBER_ROOM			0x00000001L							///会员房间
#define GRR_IP_LIMITED			0x00000002L							///地址限制
#define GRR_ENABLE_WATCH		0x00000004L							///允许旁观
#define GRR_UNENABLE_WATCH		0x00000008L							///不许旁观
#define GRR_AUTO_SIT_DESK		0x00000010L							///自动坐下，现在用于防作弊场使用
#define GRR_LIMIT_DESK			0x00000020L							///限制座位
#define GRR_LIMIT_SAME_IP		0x00000040L							///限制同IP
#define GRR_RECORD_GAME			0x00000080L							///记录游戏
#define GRR_STOP_TIME_CONTROL	0x00000100L							///停止时间控制
#define GRR_ALL_ON_DESK			0x00000200L							///是否所有人坐下才开始启动
#define GRR_FORBID_ROOM_TALK	0x00000400L							///禁止房间聊天
#define GRR_FORBID_GAME_TALK	0x00000800L							///禁止游戏聊天
//#define GRR_MATCH_REG			0x00001000L							///比赛报名
#define GRR_EXPERCISE_ROOM		0x00002000L							///训练场
#define GRR_VIDEO_ROOM			0x00004000L							///视频房间
///
///修改防作弊场可看见其他玩家姓名和头像问题！
#define GRR_NOTFUFENG			0x00008000L		///不允许负积分
#define GRR_NOTCHEAT			0x00010000L		///防作弊
///
///台费场
#define GRR_ALL_NEED_TAX		0x00020000L		///收台费场,所有人都需缴纳一定数额台费
#define GRR_QUEUE_GAME			0x00040000L		/// 排队机

#define GRR_NOT_COST_POINT		0x00080000L		///金币场不扣积分

#define GRR_CONTEST				0x00100000L		///定时淘汰比赛场
#define GRR_TIMINGCONTEST		0x00800000L		///定时赛		--RoomRule == 8388608

#define GRR_GAME_U3D			0x00200000L		///U3D游戏
#define GRR_GAME_COCOS			0x01000000L		///COCOS游戏

#define GRR_GAME_BUY			0x02000000L		///可购买房间33554432
///百家乐
#define DESK_TYPE_NORMAL		0		///
#define DESK_TYPE_BJL			1		///

#define		HUNDRED_PEOPLE_DESK_GAME_NAME   10301800					///百人桌游戏编号
#define		SUOHA_PEOPLE_DESK_GAME_NAME		30501800					///百变骰子梭哈ID
#define		NIUNIU_PEOPLE_DESK_GAME_NAME	10901800					///百变牛牛ID
#define		ERZHANGPAI_PEOPLE_DESK_GAME_NAME	11901800				///百变二张牌ID


#pragma pack(1)
///end of 百家乐
///
///
///游戏组件信息结构 （服务器 DLL 组件获取）
struct ServerDllInfoStruct
{
	///游戏组件信息
	UINT						uServiceVer;						///服务版本
	UINT						uNameID;							///游戏名字 ID
	UINT						uSupportType;						///支持类型
	UINT						uDeskPeople;						///游戏人数
	char						szGameName[61];						///游戏名字
	char						szGameTable[61];					///数据库表名字
	char						szDLLFileName[61];					///文件名字

	///辅助信息
	char						szWriter[61];						///软件开发者
	char						szDllNote[51];						///备注信息
};

///公共组件加载信息结构	（服务器设置数据）
struct ManageInfoStruct
{
    int                         iRoomFlag[2];						// 不使用的字段，占位用

	///游戏信息
	UINT						uNameID;							///游戏名字 ID
	UINT						dwRoomRule;							///游戏设置规则
	UINT						dwUserPower;						///游戏附加权限
	char						szLockTable[31];					///锁定表名字
	char						szIPRuleTable[31];					///IP 限制表名字
	char						szNameRuleTable[31];				///UserName限制表名字

    int                         iRoomInfoFlag[2];					// 不使用的字段，占位用

	///设置信息
	UINT						uComType;							///游戏类型
	UINT						uDeskType;							///游戏桌类型，主要为添加百家乐桌
	UINT						uBasePoint;							///游戏基础分
	UINT						uLessPoint;							///游戏的最少分数
	UINT						uMaxPoint;							///游戏的最大分数
	UINT						uMaxLimite;							///最大封顶值
	UINT						uRunPublish;						///逃跑扣分
	UINT						uTax;								///税收比例
	UINT						uListenPort;						///监听端口
	UINT						uMaxPeople;							///最大连接数目
	int							iSocketSecretKey;					///socket加密的密钥
	UINT						uMinDeskPeople;						///部分游戏支持M-N个人游戏

	//比赛专用
	int									iGameID;
	long int							iMatchID;
	int									iContestID;
	int									iRankNum;//排名数量
	int									iContestType;				///比赛类型，0人满赛，1定时赛，2循环赛
	int									*pAwards;//比赛奖励b
	int									*pAwardTypes;//比赛奖励类型
	int									iLowCount;
	__int64								i64Chip;
	__int64								i64LowChip;
	__int64								i64TimeStart;
	__int64								i64TimeEnd;
	int									iTimeout;
	int									iLeasePeople;					///比赛人数少于此值，代表结束比赛
	int									iMinPeople;
	int									iUpPeople;						///比赛人数达到此值，才开始分配桌子，并设置房间状态为开始游戏
	int									iRoomState;	
	int									iContestTime;


	//租赁房间ID与时间
	int									iLeaseID;
	__int64								i64LeaseTimeStart;
	__int64								i64LeaseTimeEnd;


	//救济金相关
	bool								bSendAlms;					//是否赠送救济金
	__int64								iAlmsMinMoney;				//救济金条件

	///
	///Vip登陆功能
	UINT uVipReseverCount;///Vip保留登陆数目
	///

	bool                        bVIPRoom;                            ///< 金葫芦二代，是否VIP房间（VIP房间设置密码）

	UINT						uDeskCount;							///游戏桌数目
	char						szGameTable[31];					///信息表名字
	UINT						uStopLogon;							///停止登陆房间
	
	int						uVirtualUser;						
	int						uVirtualGameTime;					

	
	///混战场房间ID       --begin
	UINT                        uBattleRoomID;
	///混战场房间信息表
	char						szBattleGameTable[31];	

	///-------------------  end

	char			szRoomPassWord[61];						//房间密码	房间加密功能
	///服务器信息
	UINT						uRoomID;							///游戏服务器 ID 号码
	char						szGameRoomName[61];					///服务器名字

	//开房间配置
	int							iLockType;							///1,锁定玩家，中途不能加入  2，锁定玩家位置，中途可加入空闲位置 3，不锁定玩家，中途随意进出，申请退出结算
	BYTE						bLockMaster;						///是否房主必须在座，0不需要，1必须在
	int							iPositionLimit;						///距离过近提示
	bool						bPositionCheck;						///距离过近是否提示
	bool						bIPCheck;							///IP相同是否提示
	bool						bCanPlayBack;

	char						szPlayBackTemp[256];				///回放文件临时存储区

	ManageInfoStruct()
	{
		//iSocketSecretKey = SECRET_KEY; 
	}
};

///内核数据 （服务器 DLL 组件启动设置）
struct KernelInfoStruct
{
	///游戏数据
	UINT						uNameID;							///游戏名字 ID 号码
	UINT						uDeskPeople;						///游戏人数


	UINT						uMinDeskPeople;						///部分游戏支持M-N个人游戏

	///内核数据
	BYTE						bMaxVer;							///软件最高版本
	BYTE						bLessVer;							///软件最低版本
	BYTE						bStartSQLDataBase;					///使用数据库连接
	BYTE						bNativeDataBase;					///使用本地数据库
	BYTE						bLogonDataBase;						///使用登陆数据库
	BYTE						bStartTCPSocket;					///是否启动网络
	UINT						uAcceptThreadCount;					///应答线程数目
	UINT						uSocketThreadCount;					///SOCKET 线程数目
};

/********************************************************************************************/

///列表项挂接数据类型
#define GLK_NO_DATA				0									///没有数据
#define GLK_GAME_KIND			1									///游戏类型
#define GLK_GAME_NAME			2									///游戏名字
#define GLK_GAME_ROOM			3									///游戏房间
#define GLK_GAME_USE			4									///游戏使用
#define GLK_GAME_WEB			5									///从INI读的WEB
#define GLK_GAME_ADD			12									///联系我们

//{add by rongqiufen 2010.7.2
#define GLK_GAME_KING_WEBGAME	6									///从bcf读的WEB
#define GLK_GAME_WEBGAME		7									///从bcf读的WEB
#define GLK_GAME_KING_EXEGAME	8									///从bcf读的WEB
#define GLK_GAME_EXEGAME		9									///从bcf读的WEB
#define GLK_GAME_SELLLIST		10									///销售列表
#define GLK_GAME_OPENINFO		11									///开放说明
//add by rongqiufen 2010.7.2}

///游戏列表辅助结构
struct AssistantHead
{
	UINT						uSize;								///数据大小
	UINT						bDataType;							///类型标识
};


///游戏类型de结构
struct ComKindInfo///加入游戏类型AddTreeData
{
	AssistantHead				Head;
	UINT						uKindID;							///游戏类型 ID 号码
	char						szKindName[61];						///游戏类型名字
	ComKindInfo()
	{
		memset(this,0,sizeof(ComKindInfo));
	}
};

///游戏名称结构
struct ComNameInfo
{
	AssistantHead				Head;
	UINT						uCanBuy;							///是否可购买 1只有普通房间，2只有可购买房间，3两种都有
	UINT						uKindID;							///游戏类型 ID 号码
	UINT						uNameID;							///游戏名称 ID 号码
	UINT						m_uOnLineCount;						///在线人数
	UINT						uVer;								///版本
	char						szGameName[61];						///游戏名称

	ComNameInfo()
	{
		memset(this,0,sizeof(ComNameInfo));
		m_uOnLineCount=0;
	}
	///

};

///游戏房间列表结构
struct ComRoomInfo
{
	AssistantHead				Head;
	UINT						uComType;							///游戏类型
	UINT						uKindID;							///游戏类型 ID 号码
	UINT						uNameID;							///游戏名称 ID 号码
	UINT						uRoomID;							///游戏房间 ID 号码
	UINT						uPeopleCount;						///游戏在线人数
	UINT						iUpPeople;							///比赛房间用户达到值后才开赛
	UINT						uDeskPeople;						///每桌游戏人数
	UINT						uDeskCount;							///游戏大厅桌子数目
	UINT						uServicePort;						///大厅服务端口
	char						szServiceIP[25];					///服务器 IP 地址
	char						szGameRoomName[61];					///游戏房间名称
	int							uVirtualUser;						
	int							uVirtualGameTime;					
	UINT						uVer;								///版本
	UINT						dwRoomRule;							///游戏房间规则
	bool                        bVIPRoom;                            ///< 金葫芦二代，是否VIP房间（VIP房间设置密码）
	int							iBasePoint;							//基础倍数
	UINT						uLessPoint;							//金币或积分下限
	UINT						uMaxPoint;							//金币或积分上限

	int							iContestID;
	__int64						i64TimeStart;

	bool						bHasPassword;			// 有无密码	
	UINT						dwTax;					// 房间房费	
	///
	///游戏人数初始化问题

	ComRoomInfo()
	{
		memset(this,0,sizeof(ComRoomInfo));
		uPeopleCount=0;
		bHasPassword = false;
	}
	///
};

///游戏大厅辅助结构
struct GameInsideInfo
{
	AssistantHead				Head;								///列表头
	UINT						uTrunID;							///转向 ID
	char						szDisplayName[61];					///显示名字
};

//道具类型
struct ComPropInfo
{
	//AssistantHead				Head;
	UINT						uPropID;							//道具id
	char						szPropName[50];						//道具名
	ComPropInfo()
	{
		memset(this, 0, sizeof(ComPropInfo));
	}
};

/********************************************************************************************/

///用户状态定义
#define USER_NO_STATE		 	0									///没有状态，不可以访问
#define USER_LOOK_STATE		 	1									///进入了大厅没有坐下
#define USER_SITTING		 	2									///坐在游戏桌
#define USER_ARGEE				3									///同意状态
#define USER_WATCH_GAME		 	4									///旁观游戏
#define USER_DESK_AGREE			5									///大厅同意
#define USER_CUT_GAME		 	20									///断线状态			（游戏中状态）
#define USER_PLAY_GAME		 	21									///游戏进行中状态	（游戏中状态）


static CString USER_STATESTRING[8] = {"", "空闲中", "坐下", "同意", "旁观", "大厅同意", "断线", "游戏中"};

///用户信息结构
struct UserInfoStruct
{
	int							dwUserID;							///ID 号码
	__int64						i64Money;							///金币
	__int64						i64Bank;							///银行
	int							iJewels;							///钻石
	int							iLockJewels;						///冻结的钻石
	UINT						uWinCount;							///胜利数目
	UINT						uLostCount;							///输数目
	UINT						uCutCount;							///强退数目
	UINT						uMidCount;							///和局数目
	char						szName[32];							///登录名
	UINT						bLogoID;							///头像 ID 号码
	BYTE						bDeskNO;							///游戏桌号
	BYTE						bDeskStation;						///桌子位置
	BYTE						bUserState;							///用户状态
	ULONG						dwUserIP;							///登录IP地址
	bool						bBoy;								///性别
	char						nickName[50];						///用户昵称
	int							iVipTime;							///会员时间
	bool						isVirtual;							///是否是扩展机器人 

	char						szSignDescr[128];			         ///个性签名

	char						szHeadUrl[256];	///头像URL
	float						flat;			///玩家经度
	float						flnt;			///玩家纬度
	char						szLocation[128];	///玩家地址

	//比赛专用
	__int64						i64ContestScore;
	int							iRankNum;		//排行名次
	int							iContestCount;

	bool						bLogonbyphone;		///是否手机登陆（用于判断是否主动发玩家列表）
	UserInfoStruct()
	{
		ZeroMemory(this, sizeof(UserInfoStruct));
	}
};

///用户信息结构 (小猪快跑)
struct UserInfoStructX
{
	long int					dwUserID;							///ID 号码
	long int					dwExperience;						///经验值
	int							dwAccID;							///ACC 号码
	int							dwPoint;							///分数
	__int64						i64Money;							///金币
	__int64						i64Bank;							///银行
	UINT						uWinCount;							///胜利数目
	UINT						uLostCount;							///输数目
	UINT						uCutCount;							///强退数目
	UINT						uMidCount;							///和局数目
	WCHAR						szName[64];							///登录名
	WCHAR						szClassName[64];					///游戏社团
	UINT						bLogoID;							///头像 ID 号码
	int							bDeskNO;							///游戏桌号
	int							bDeskStation;						///桌子位置
	int							bUserState;							///用户状态
	int							bMember;							///会员等级
	int							bGameMaster;						///管理等级
	ULONG						dwUserIP;							///登录IP地址
	BOOL						bBoy;								///性别
	WCHAR						nickName[64];						///用户昵称
	UINT						uDeskBasePoint;						///设置的桌子倍数
	int							dwFascination;						///魅力
	int							iVipTime;							///会员时间
	int							iDoublePointTime;					///双倍积分时间
	int							iProtectTime;						///护身符时间，保留
	int							isVirtual;							///是否是扩展机器人 ///20081211 , Fred Huang
	UINT						dwTax;								 ///房费    add by wyl    11-5-16

	///玩家信息结构调整   yjj 090319
	WCHAR                       szOccupation[64];                    ///玩家职业
	WCHAR                       szPhoneNum[64];                      ///玩家电话号码
	WCHAR                       szProvince[64];                      ///玩家所在的省
	WCHAR                       szCity[64];                          ///玩家所在的市
	WCHAR                       szZone[64];                          ///玩家所在的地区
	BOOL                        bHaveVideo;                          ///是否具有摄像头

	///duanxiaohui 20111111  
	WCHAR						szSignDescr[128];			         ///个性签名
	///end duanxiaohui

	///wushuqun 2009.6.26
	///玩家类型信息
	///0 ,普通玩家
	///1 ,电视比赛玩家
	///2 ,VIP玩家
	///3 ,电视比赛VIP玩家
	int							userType;

	///作为扩展字段,为方便以后新加功能用
	///此处为以后平台中的新加功能需要修改用户信息结构时，不用重新编译所有游戏

	UINT                         userInfoEx1;    ///扩展字段1，用于邮游钻石身份作用时间，由黄远松添加
	UINT						 userInfoEx2;    ///扩展字段2，用于GM处理之禁言时效，由zxd添加于20100805

	int							 bTrader;		 //用于判断是不是银商  add by lxl 2010-12-10 


///比赛专用
	__int64						i64ContestScore;
	int							iContestCount;
	CTime						timeLeft;
};

//用户信息结构(小猪快跑)
struct UserItemStructX
{
	BOOL							bMatchUser;							//是否比赛
	UINT							uSortIndex;							//排列索引
	UINT							uConnection;						//玩家关系
	long int						dwLogonTime;						//登陆时间
	UserInfoStructX					GameUserInfo;						//用户信息
};

//小猪快跑聊天消息
struct XzkpNormalTalk {
	int crColor;
	int iLength;
	int	dwSendID;
	int dwTargetID;
	int nDefaultIndex;
	WCHAR szMessage[108];
};

///用户规则结构
struct UserRuleStruct
{
	///变量定义
	BYTE						bSameIP;							///相同 IP
	BYTE						bIPLimite;							///不于IP前几位的玩家游戏
	BYTE						bPass;								///设置密码
	BYTE						bLimitCut;							///限制断线
	BYTE						bLimitPoint;						///限制分数
	BYTE						bCutPercent;						///逃跑率
	char						szPass[61];							///桌子密码
	long int					dwLowPoint;							///最低分数
	long int					dwHighPoint;						///最高分数 
};

struct SendUserMoneyByTimes         ///某种条件下，按次数赠送玩家游戏币
{	
	long int							dwUserID;							///用户 ID
	int                                 dwSrcGameMoney;                     ///原来的金额                    需要送进来前添加
	int                                 dwSrcMoneyInBank;                    ///银行里的金币
	int                                 dwSendGameMoney;                    ///赠送的金额  
	int                                 dwSendTimes;                        ///今天已经赠送的次数
	int                                 dwAllSendTImes;                     ///今天总共的赠送次数	
	int                                 dwSendFailedCause;                   ///赠送失败原因 0成功 
                                                                             ///1次数已满2未达到赠送标准
};

//自动赠送添加的结构体
typedef struct tag_RECEIVEMONEY
{
	__int64	  i64Money;
	__int64	  i64MinMoney;
	int		  iCount;
	int		  iTotal;
	int       iResultCode; 
	struct tag_RECEIVEMONEY()
	{		
		i64Money = 0;
		i64MinMoney = 0;
		iCount = 0;
		iTotal = 0;
		iResultCode = 0;
	}
	
}RECEIVEMONEY;

// 在线时长、局数送金币
struct SendMoneyOnTimesAndCounts
{
    int                                 dwUserID;
    int                                 dwGetMoney;             // 获赠的金币数
    int                                 dwMoneyOnTimes;         // 设置多少时长赠送的金币
    int                                 dwMoneyOnCounts;        // 设置多少局赠送的金币
    int                                 dwTimesNeed;            // 下一次差多少时长
    int                                 dwCountsNeed;           // 下一次差多少局
};

///wushuqun 2009.6.5
///混战房间记录结果
struct BattleRoomRecordRes
{
	///混战房间ID号
	///注：这里的房间ID 号是TGameRoomInfo 中的RoomID,而不是BattleRoomID
	///因为若BattleRoomID=RoomID时可能会有冲突

	UINT          uRoomID;   

	///记录结果
	UINT          uRecordResult;
};
/********************************************************************************************/


struct TLoginGameInfo
{
	int					iKindID;
	int					iGameID;
	int					iLoginCount;
	COleDateTime		oelLastTime;
};
#pragma pack()

#endif
