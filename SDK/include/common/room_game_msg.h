#ifndef _ROOM_GAME_MSG_H
#define _ROOM_GAME_MSG_H
#include "HNBaseType.h"
#include "basemessage.h"
//#include "ClientComStruct.h"

#define IPC_PACKAGE 4096

enum E_IPC_MAINID
{
	ipc_mainid_kernel=1,
	ipc_mainid_config,
	ipc_mainid_socket,
	ipc_mainid_frame,
	ipc_mainid_notify,
	ipc_mainid_user,
	ipc_mainid_action,
};

enum E_MainID_U3D
{
	ipc_u3d_mainid_state = 0,
};
enum E_AssitID_U3D
{
	ipc_u3d_assid_state = 3,
};

enum E_PlatFormClient2U3D
{
	ipc_mainid_user_pc2u3d = 10,
};
enum E_PlatFormClient2U3D_ASSID
{
	ipc_assid_user_status_pc2u3d = 0,
};

enum E_PlatFormClient2U3D_gameinfo
{
	ipc_mainid_gameinfo = 8,
};
enum E_PlatFormClient2U3D_gameinfo_ass
{
	ipc_assid_gameinfo = 0,
};


enum E_IPC_ASSID
{
	ipc_assid_kernel_connect,		//游戏连接房间
	ipc_assid_kernel_close,			//游戏关闭
	ipc_assid_kernel_heart,			//游戏与房间心跳

	ipc_assid_config_serverinfo,	//房间信息

	ipc_assid_socket_send,			//游戏发送数据到服务端	
	
	ipc_assid_user_list,			//玩家列表
	ipc_assid_user_status,			//玩家状态改变

	ipc_assid_action,				//玩家动作
};

enum E_NET_MAINID
{
	net_mainid_game_frame=150,
	net_mainid_game_notify=180,
};

enum E_NET_ASSID
{
	net_assid_game_info=1,
	net_assid_game_station,
};

enum E_IPC_ACTION
{
	ipc_action_null,
	ipc_action_stand,		//起身
	ipc_action_sit,			//坐下
	ipc_action_agree,		//同意
	ipc_action_cut,			//断线
	ipc_action_begin,		//游戏开始
	ipc_action_end,			//游戏结束
};

enum E_IPC_ASS_ACTION
{
	ipc_assid_closegame = 2,		//游戏关闭
};

#define	PLATFORM_SERVER_ADDRESS				"127.0.0.1"
#define GAME_ROOM_PORT						13028

struct HNIpcMessageHead
{
	UINT	uMessageSize;	//数据包大小
	UINT	uMainID;		//处理主类型
	UINT	uAssID;			//辅助处理类型 ID
	UINT	bHandleCode;	//处理代码
	UINT	inbReserve;		//保留字段
};

struct U3DData
{
	NetMessageHead messageHead;
	UINT objectSize;
	BYTE object[4096];
};


#pragma pack(1)

struct U3D_GameQuitState
{
	BYTE		byUserState;			//用户状态 0 == USER_NO_STATE, 1==USER_LOOK_STATE,2==USER_SITTING,3==USER_AGREE,4==USER_WATCH_GAME,5==USER_DESK_AGREE,20==USER_CUT_GAME,21==USER_PLAY_GAME
};

struct U3D_MSG_GM_S_ClientInfo
{
	BYTE		bCanWatch;				//是否允许旁观
	BYTE		bDeskIndex;				//玩家桌子号
	int			iUserID;				//用户ID
};

struct U3DUserInfo
{
	UINT					dwUserID;	//玩家ID
	CHAR					szNick[61];	//昵称	
	BYTE					bDeskNO;	//游戏桌号
	BYTE					bDeskStation;//桌子位置
	BYTE					bUserState;	//用户状态
	LLONG					llMoney;	//钱包金币
	LLONG					llBank;		//银行金币
};


//用户类型
enum E_U3D_UserType
{
	E_User_VirtualType_human = 0,
	E_User_VirtualType_AI = 1,
	E_User_VirtualType_Virtual =2,
};
// 游戏信息
struct U3DGameInfo
{
	CHAR					szGameName[61];			//游戏名字
	CHAR					szGameRoomName[61];		//游戏房间名称
	UINT					uNameID;				//游戏ID
	UINT					uRoomID;				//房间ID
	BYTE					bGameStation;			//游戏状态	
	LLONG					dwRoomRule;				//设置规则	
	UINT					uDeskPeople;			//游戏人数
	U3DUserInfo				userinfo;				//玩家信息	
};



struct u3dGameInfoEx_ToGame
{
	BYTE bGameStatoin;			//游戏状态
	BYTE bWatchOther;			//允许旁观
	BYTE bWaitTime;				//等待时间
	BYTE bReserve;				//保留字段
	char szMessage[1000];		//系统消息
};
struct U3DUserInfoStruct
{
	char				szUserName[22];		//登陆名
	char				szNickName[22];		//昵称
	char				szRankName[22];		//军衔
	int					dwUserID;			//用户ID
	int					dwAccID;			/// ACC 号码
	int					dwPoint;			/// 分数
	UINT				uWinCount;			/// 胜利数目
	UINT				uLostCount;			/// 输数目
	UINT				uCutCount;			/// 强退数目
	UINT				uMidCount;			/// 和局数目
	UINT				bLogoID;			/// 头像ID
	byte				bDeskNO;			/// 游戏桌号
	byte				bDeskStation;		/// 桌子位置
	byte				bUserState;			/// 用户状态
	byte				bGameMaster;
	UINT				dwUserIP;			/// 登录IP地址
	bool				bBoy;				/// 性别
	int					dwFascination;		/// 魅力
	int					iVipTime;			/// VIP剩余时间(天为单位)	
	int					iVipGrade;			/// VIP等级
	int					iRanks;				/// 军衔等级				
	int					iProtectTime;		/// 护身符时间，保留
	int					isRobot;			/// 机器人
	E_U3D_UserType		isVirtual;			/// 判断是真人，机器人，还是虚拟封桌人
	__int64				i64Money;			/// 银行
	__int64				i64Bank;
	__int64				i64DragonMoney;		/// 龙币
	__int64				i64GamePoint;		/// 游戏经验值
	int					iSigned;			/// 是否已报名，0-未报名，1-已报名
	int					iMatchID;			/// 比赛ID，唯一的标识一场比赛
	__int64				i64ContestScore;
	int					iContestCount;
	char				timeLeft[8];	
	int					iRankNum;			/// 排行名次
	int					iRemainPeople;		/// 比赛中还剩下的人数
	/// 夺冠次数
	int					iChanpionCount;		/// 参赛次数
	int					iContestTimes;		/// 最佳名次
	int					iBestNum;
	UINT				userInfoEx2;						/// 扩展字段2，用于GM处理之禁言时效
};
struct U3DUserItemStructEx
{
	bool		bMatchUser;
	UINT		uSortIndex;
	UINT		uConnection;
	UINT		dwLogonTime;
	U3DUserInfoStruct  GameUserInfo;
};
//游戏信息
struct U3DGameInfoEx
{
	U3DGameInfoEx()
	{
		bDeskOnwer = true;												//是否台主
		bGameStation = 0;												//游戏状态
		memset(&uisMeUserInfo, 0, sizeof(UserItemStruct));				//玩家信息
//		_tcscpy(uisMeUserInfo.GameUserInfo.szNickName, TEXT("张三"));
//		_tcscpy(uisMeUserInfo.GameUserInfo.szUserName, TEXT("张三丰"));
		uisMeUserInfo.GameUserInfo.dwUserID = 100001;
		uisMeUserInfo.GameUserInfo.bDeskNO = 3;
		uisMeUserInfo.GameUserInfo.bDeskStation = 1;

		dwGamePower = 0;												//用户权限
		dwMasterPower = 0;												//管理权限
		dwRoomRule = 0;													//设置规则
		pOrderName = NULL;												//等级函数

		//设置参数
		bEnableSound = true;											//允许声音
		bEnableWatch = false;											//允许旁观
		bShowUserInfo = true;											//显示进出
		bEnableBackSound = true;										//背景音乐

		//大厅数据
		_tcscpy(szProcessName, TEXT("zjh.exe"));						//进程名字
		uDeskPeople = 3;												//游戏人数
		uRoomID = 2;													//房间表示
		dwGameMSVer = 2;												//主版本号
		dwGameLSVer = 0;												//副版本号
		uComType = 1;													//游戏类型
		uNameID = 100600302;											//名字表示
		_tcscpy(szGameName, TEXT("扎金花"));								//游戏名字
		uLessPoint = 0;													//最少经验值
		memset(szGameRoomName,0,sizeof(szGameRoomName));
		iBasePoint = 1;
		iLowCount = 0;
		i64Chip = 0;
		i64LowChip = 0;
		iRankNum = 0;
		memset(ArrAwards, 0, sizeof(int)*MAX_PEOPLE);
	}
	//共享信息
	bool			bDeskOnwer;										//是否台主
	BYTE			bGameStation;									//游戏状态
	UserItemStruct  uisMeUserInfo;									//玩家信息
	UINT			dwGamePower;									//用户权限
	UINT       dwMasterPower;										//管理权限
	UINT       dwRoomRule;											//设置规则
	void       *pOrderName;											//等级函数

	//设置参数
	bool       bEnableSound;										//允许声音
	bool       bEnableWatch;											//允许旁观
	bool       bShowUserInfo;											//显示进出
	bool       bEnableBackSound;										//背景音乐

	//大厅数据
	CHAR       szProcessName[31];										//进程名字
	UINT       uDeskPeople;												//游戏人数
	UINT       uRoomID;													//房间表示
	UINT       dwGameMSVer;												//主版本号
	UINT       dwGameLSVer;												//副版本号
	UINT       uComType;												//游戏类型
	UINT       uNameID;													//名字表示
	char       szGameName[61];											//游戏名字
	UINT       uLessPoint;												//最少经验值 
	int        iBasePoint;
	UINT       uUserType;												//用户类型（0普通玩家，1游客）

	//比赛专用
	int        iLowCount;
	__int64       i64Chip;
	__int64       i64LowChip;
	int        iRankNum;
	int        ArrAwards[MAX_PEOPLE];

	char      szGameRoomName[61];										//游戏房间名称
};

struct U3DUserItemStruct
{
	bool								bMatchUser;
	UINT								iSortIndex;
	UINT								uConnection;
	UINT								dwLogonTime;
	U3DUserInfoStruct					GameUserInfo;
};

struct CM_UserState_Change_U3D
{
	BYTE								byActionCode;
	BYTE								byDeskStation;
	U3DUserItemStruct					uiUserItem;
};


#pragma pack()

struct U3DGameStation
{
	BYTE					bGameStation;			//游戏状态
};

struct U3DUserStateChange
{
	BYTE					bActionCode;			//动作代码
	BYTE					bDeskStation;			//桌子位置
	U3DUserInfo				userinfo;				//玩家信息
};

#endif