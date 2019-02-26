/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

/********************************************************************************************/

#include "BaseMessage.h"
#include "ComStruct.h"
#include <vector>
using namespace std;

#pragma pack(1)

//房间管理窗口数据修改内容
struct MSG_GR_GRM_Set
{
	long int	dwUserID;	//用户 ID
	bool		bAIWinAndLostAutoCtrl;	//机器人控制输赢开关
	__int64		iAIWantWinMoney[3];		/**<机器人输赢控制：机器人赢钱区域1,2,3 4所用断点 */
	int			iAIWinLuckyAt[4];		/**<机器人输赢控制：机器人在区域1,2,3,4赢钱的概率 */
	__int64		iReSetAIHaveWinMoney;	//机器人输赢控制：重置机器人已经赢钱的数目

	//输赢控制
	bool		bWinProbCtrl;			//开关
	long int	dwUserID_win[20];		//可读取20组，多了影响性能,玩家ID
	int			iProb_win[20];			//输赢概率[0,100]
	long int	dwUserID_los[20];		//可读取20组，多了影响性能,玩家ID
	int			iProb_los[20];			//输赢概率[0,100]

	MSG_GR_GRM_Set()
	{
		memset(this,0,sizeof(MSG_GR_GRM_Set));
	}
};

//房间管理窗口数据更新
struct MSG_GR_GRM_UpData
{
	//奖池
	bool		bAIWinAndLostAutoCtrl;	//机器人控制输赢开关
	__int64		iAIWantWinMoney[3];		/**<机器人输赢控制：机器人赢钱区域1,2,3 4所用断点 */
	int			iAIWinLuckyAt[4];			/**<机器人输赢控制：机器人在区域1,2,3,4赢钱的概率 */
	__int64		iReSetAIHaveWinMoney;		//机器人输赢控制：重置机器人已经赢钱的数目

	__int64		iAIHaveWinMoney;		//机器人赢的钱
	//输赢控制
	bool		bWinProbCtrl;			//开关
	long int	dwUserID_win[20];		//可读取20组，多了影响性能,玩家ID
	int			iProb_win[20];			//输赢概率[0,100]
	long int	dwUserID_los[20];		//可读取20组，多了影响性能,玩家ID
	int			iProb_los[20];			//输赢概率[0,100]

	MSG_GR_GRM_UpData()
	{
		memset(this,0,sizeof(MSG_GR_GRM_UpData));
	}
};

///游戏房间登陆
struct MSG_GR_S_RoomLogon
{
	UINT								uNameID;							///名字 ID
	LONG								dwUserID;							///用户 ID
	CHAR								szMD5Pass[50];						///加密密码
	float								flat;								///玩家经度
	float								flnt;								///玩家纬度
	char								szLocation[128];					///玩家地址
	bool								bLogonbyphone;						///是否手机登陆（用于判断是否主动发玩家列表）
};

///游戏房间登陆
struct MSG_GR_R_LogonResult
{
	UserInfoStruct						pUserInfoStruct;					///用户信息
	RECEIVEMONEY                        strRecMoney;                        //非比赛场玩家金币不足自动赠送
	int									nVirtualUser;						///登录房间时即时获取虚拟玩家人数
	int									iRemainPeople;						//比赛中还剩下的人数

	MSG_GR_R_LogonResult()
	{
		memset(this,0,sizeof(MSG_GR_R_LogonResult));
	}


};

///游戏房间登陆
struct MSG_GR_R_OtherRoom
{
	int									iRoomID;
	CHAR								szGameRoomName[61];					///房间名字
};

///游戏桌子状态
struct MSG_GR_DeskStation
{
	bool								bIsBuy[100];						//是否被购买(0:没被购买;1:已被购买)
	BYTE								bDeskStation[100];					///桌子状态
	BYTE								bDeskLock[100];						///锁定状态
	BYTE                                bVirtualDesk[100];					///虚拟状态
	BYTE                                bDeskUser[100];						///桌子人数
};

struct CDeskStation
{
	vector<BYTE> vLocks;
	vector<BYTE> vVirtuals;
	vector<BYTE> vPlayings;
};

///打开钱柜
struct MSG_GR_UserBank
{
	long int						dwUserID;
	__int64						    i64Bank;								///钱柜
	char							szTwoPassword[50];					//银行密码
	int								nVer;								//银行版本
};
struct MSG_GR_ContestAward
{
	long int							dwUserID;
	int									iAward;
	int									iAwardType;
	int									iRank;
};
///打开银行(使用二级密码)
struct MSG_GR_OpenBank
{
	char							szMD5Pass[50];						///加密密码
};

///取钱
struct MSG_GR_CheckMoney
{
	long int						dwUserID;							///用户 ID
	__int64							i64CheckOut;						///取出金额
	__int64							i64MoneyInBank;						///剩余
	__int64							i64MoneyInRoom;
	int								uGameID;							///游戏ID
};

//玩家返利更新数据
struct MSG_GR_UserRebateUpDate
{
	int						iUserID;				//玩家ID
	int						iTx_Money;				//剩余返利数目
	int						iTx_Smony;				//已经取走的数目
};

//玩家返利修改结果数据
struct MSG_GR_UserRebateSetResult
{
	int						iUserID;				//玩家ID
	int						iMoney;					//操作数目(作为结果则返回剩余数目)
	bool					bTag;					//0 存，1 取
	bool					bResult;				//处理结果
	char					Msg[MAX_PATH];			//错误消息
};

///用户进入房间
struct MSG_GR_R_UserCome
{
	UserInfoStruct						pUserInfoStruct;						///用户信息
};

///用户离开房间
struct MSG_GR_R_UserLeft
{
	long int							dwUserID;							///用户 ID
};

///用户坐下
struct MSG_GR_S_UserSit
{
	BYTE								bDeskIndex;							///桌子索引
	BYTE								bDeskStation;						///桌子位置
	TCHAR								szPassword[61];						///桌子密码
};

struct MSG_GR_S_UserQueueInfo
{
	BYTE								bDeskStation;						///桌子位置
	UINT								iUserID;							///用户ID
};

///取钱
struct MSG_GR_S_CheckMoneyWithPwd_t
{
	__int64								i64Count;							///金额
	long int							UserID;								///用户ID
	TCHAR								szMD5Pass[50];						///银行密码
};

///存钱
struct MSG_GR_S_CheckMoney
{
	__int64								i64Count;							///金额
	long int							UserID;								///用户ID
};

///转帐
struct MSG_GR_S_TransferMoney
{
	long int							UserID;								///用户ID
	long int							destUserID;							///转给谁
	__int64								i64Money;								///欲转帐金额
	__int64								i64ActualTransfer;					///实际到帐金额
	int                                 bTranTax;                           ///是否扣税，非0不扣税
	TCHAR								szMD5Pass[50];						///银行密码
	TCHAR								szNickName[50];						///用户昵称
	TCHAR								szDestNickName[50];					///被转账用户昵称
	bool								bUseDestID;
    UINT                                uHandleCode;        // 用来区分是否是“全转”功能
	MSG_GR_S_TransferMoney()
	{
		ZeroMemory(this, sizeof(MSG_GR_S_TransferMoney));
	}
};
///修改密码
struct MSG_GR_S_ChangePasswd_t
{
	long int							UserID;								///用户ID
	TCHAR								szMD5PassOld[50];					///银行密码旧
	TCHAR								szMD5PassNew[50];					///银行密码新
	MSG_GR_S_ChangePasswd_t()
	{
		ZeroMemory(this, sizeof(MSG_GR_S_ChangePasswd_t));
	}
};

// 根据用户ID获取昵称
struct MSG_GR_S_GetNickNameOnID_t
{
    long int    iUserID;        // 用户ID
    TCHAR       szNickName[61]; // 用户昵称
    MSG_GR_S_GetNickNameOnID_t()
    {
        ZeroMemory(this, sizeof(MSG_GR_S_GetNickNameOnID_t));
    }
};

///用户转帐记录
struct MSG_GR_S_TransferRecord_t
{
	long int							dwUserID;							///用户 ID
};
///用户转帐记录结果
struct MSG_GR_R_TransferRecord_t
{
	int				nSeriNo;				///< 序号
	UINT			dwUserID;				///< 用户 ID
	UINT			destUserID;				///< 转给谁
	TCHAR			szNickName[50];			//用户昵称
	TCHAR			szDestNickName[50];		//被转账用户昵称
	__int64			i64Money;				///< 欲转帐金额
	__int64			i64ActualTransfer;		///< 实际到帐金额
	__int64			oleDateTime;			///< 时间
	MSG_GR_R_TransferRecord_t()
	{
		ZeroMemory(this, sizeof(MSG_GR_R_TransferRecord_t));
	}
};
///付钱
struct MSG_GR_S_Pay
{
	int									uCount;								///金额
	long int							UserID;								///用户ID
	int									iPayID;								///付钱ID（为什么付钱）
};
///用户断线
struct MSG_GR_R_UserCut
{
	long int							dwUserID;							///用户 ID
	BYTE								bDeskNO;							///游戏桌号
	BYTE								bDeskStation;						///位置号码
};

///用户坐下或者起来
struct MSG_GR_R_UserSit
{
	LONG								dwUserID;							///用户 ID
	BYTE								bLock;								///是否密码
	BYTE								bDeskIndex;							///桌子索引
	BYTE								bDeskStation;						///桌子位置
	BYTE								bUserState;							///用户状态
	BYTE								bIsDeskOwner;						///台主离开（锁桌）
	bool								bLeave;								///是否离桌
	bool								bDeskMaster;						///是否是房主（开房间）
};

//发送视频服务器信息
struct SetVideoSrvStruct
{
	char szIP[30];
	int  videoport;
	int  audioport;

	SetVideoSrvStruct()
	{
		::memset(this,0,sizeof(SetVideoSrvStruct));
	}
};

///聊天结构 
struct MSG_GR_RS_NormalTalk
{
	COLORREF							crColor;							///字体颜色
	SHORT								iLength;							///信息长度
	LONG								dwSendID;							///用户 ID
	LONG								dwTargetID;							///目标 ID
	int									nDefaultIndex;						///=0，输入的内容，>0，选择的内容
	CHAR								szMessage[MAX_TALK_LEN+1];			///聊天内容
};

// 踢人卡、防踢卡 结构
struct MSG_GR_RS_KickProp
{
    long int    dwUserID;
    int         iPropID;
    int         iTotalTime;     // 总时间s
};

///用户同意结构
struct MSG_GR_R_UserAgree
{
	BYTE								bDeskNO;							///游戏桌号
	BYTE								bDeskStation;						///位置号码
	BYTE								bAgreeGame;							///同意标志
};
///用户分数
struct MSG_GR_R_InstantUpdate
{
	long int							dwUserID;							///用户 ID
	int									dwPoint;							///用户分数
	int									dwMoney;							///用户金币
};
///用户经验值
struct MSG_GR_R_UserPoint
{
	long int							dwUserID;							///用户 ID
	__int64									dwPoint;							///用户经验值
	__int64									dwMoney;							///用户金币
	__int64									dwSend;								///赠送
	BYTE								bWinCount;							///胜局
	BYTE								bLostCount;							///输局
	BYTE								bMidCount;							///平局
	BYTE								bCutCount;							///逃局
	RECEIVEMONEY                        strAutoSendMoney;                   //添加自动赠送
};

///用户比赛信息
struct MSG_GR_ContestChange
{
	long int					dwUserID;							///用户 ID	
	int							iContestCount;						///比赛局数
	__int64						i64ContestScore;					///比赛分数
	int							iRankNum;							///比赛排名
	int							iRemainPeople;						///比赛中还剩多少人
	MSG_GR_ContestChange()
	{
		ZeroMemory(this, sizeof(MSG_GR_ContestChange));
	}
};
//通知不能比赛
struct MSG_GR_NoContestChange
{
	bool						bBegin;								//是否可以开始比赛
	int							iBeginPeople;						//达到多少人开始比赛      
	MSG_GR_NoContestChange()
	{
		memset(this,0,sizeof(MSG_GR_NoContestChange));
	}
};
struct MSG_GR_O_ContestApply
{
	int		iContestID;
	int		iUserID;
	__int64 i64WalletMoney;
	__int64 i64Jewels;
	int     iApplyNum;
	BYTE	bResult;
};

struct MSG_GR_ContestApply
{
	int iApplyResult;//报名结果
	int	iContestBegin;//比赛是否开始
};

struct MSG_GR_I_ContestApply
{
	int		iContestID;
	int		iUserID;						//用户ID
	int		iTypeID;						//操作类型：0-报名，1-退赛
};

struct MSG_GR_I_ContestInfo
{
	int									iContestBegin;					//0-比赛还未开始，1-比赛开始
	int									iContestNum;					//已报名人数
};


///用户形象更改信息
struct MSG_GR_AvatarChange
{
	long int						dwUserID;						///用户 ID	
	UINT							nNewLogoID;						///新的头像ID
};

///修改用户经验值
struct MSG_GR_S_RefalshMoney
{
	long int							dwUserID;							///用户 ID
	__int64								i64Money;							///用户金币
};

///打开经验盒
struct MSG_GR_S_OpenPoint
{
	long int							dwUserID;							///用户 ID
	int									dwPoint;							///经验
};

///取出结果
struct MSG_GR_S_GetPoint
{
	long int							dwUserID;							///用户 ID
	int									dwPoint;							///经验
};

///修改用户桌子倍数设定
struct MSG_GR_S_SetDeskBasePoint
{
	long int							dwUserID;							///用户ID
	int									iDeskBasePoint;						///用户设定的桌子倍数
	BYTE								bDeskOwner;							///是否台主
};
///设置数据包
struct MSG_GR_S_UserSet
{
	UserRuleStruct						m_Rule;								///用户规则
};

///邀请加入数据包
struct MSG_GR_SR_InviteUser
{
	long int							dwUserID;							///用户 ID
	long int							dwTargetID;							///目标用户 ID
};

///比赛用户数据包
struct MSG_GR_SR_MatchUser
{
	long int							dwUserID;							///用户 ID
	char								szUserName[61];						///用户名字
	BYTE								bDeskIndex;							///桌子号码
	BYTE								bDeskStation;						///桌子位置
};

///取消比赛数据包
struct MSG_GR_SR_MatchDelete
{
	long int							dwUserID;							///用户 ID
	BYTE								bDeskIndex;							///桌子号码
	BYTE								bDeskStation;						///桌子位置
};

///踢玩家下线数据包
struct MSG_GR_S_CutUser
{
	long int							dwUserID;							///管理员用户 ID
	long int							dwTargetID;							///目标用户 ID
};

///呼叫GM
struct MSG_GR_Call_GM
{
	BYTE								bCallReason;						///呼叫类型
	UINT								uLength;							///消息长度
	char								szMsg[201];							///呼叫留言
};

///警告数据包
struct MSG_GR_SR_Warning
{
	long int							dwTargetID;							///目标用户
	UINT								uLength;							///消息长度
	char								szWarnMsg[201];						///警告消息
};

///用户权限设置数据包
struct MSG_GR_SR_GamePowerSet
{
	BYTE								bShowActive;						///是否显示
	BYTE								bBindKind;							///绑定类型
	UINT								uLockTime;							///锁定时间
	long int							dwUserID;							///用户 ID
	long int							dwGamePowerID;						///游戏权限
	long int							dwMasterPowerID;					///管理权限
};


///魅力值
struct MSG_GR_Charm_Struct
{
	long int							dwUserID;							///玩家ID
	int									iCharmValue;						///魅力变动值
};

///队例情况
struct MSG_GR_Queue
{
	int									iCount;								///当前队例中有多少人排队
	int									iReserver;							///保留
};


/********************************************************************************************/

///游戏信息
struct MSG_GM_S_ClientInfo
{
	BYTE								bEnableWatch;						///允许旁观
};

///游戏信息
struct MSG_GM_S_GameInfo
{
	BYTE								bGameStation;						///游戏状态
	BYTE								bWatchOther;						///允许旁观
	BYTE								bWaitTime;							///等待时间
	BYTE								bReserve;							///保留字段
	char								szMessage[1000];					///系统消息
};

///旁观设置
struct MSG_GM_WatchSet
{
	long int							dwUserID;							///设置对象
};

///道具
struct PropStruct
{
	BYTE							sendDeskStation;				///发放位置
	BYTE							recvDeskStation;				///接收位置
	int								iValue;							///动画id
};

///好友相关
const int DB_BUFFER_MSG_MAX_LEN = 255;

///好友消息
struct tagIM_MSG
{
	int								iMessageID;							///消息ID
	long int						dwSendUserID;						///发送者ID
	long int						dwRecvUserID;						///接收者ID

	DBTIMESTAMP						tmSendTime;							///时间戳
	int								iDataLen;							///数据长度
	BYTE							Data[DB_BUFFER_MSG_MAX_LEN];		///数据
};
/********************************************************************************************/

///消息数据包
struct MSG_GA_S_Message
{
	BYTE								bFontSize;							///字体大小
	BYTE								bCloseFace;							///关闭界面
	BYTE								bShowStation;						///显示位置
	CHAR								szMessage[1000];					///消息内容
};

// PengJiLin, 2010-9-14, 使用踢人卡踢人的结果
struct MSG_GA_NEW_KICKUSERPROP
{
    long int    dwUserID;
    long int    dwDestID;
    BYTE        byKickOther;        // 1=自己踢别人
    BYTE        byKickResult;
};



/******************************************************************************************/
//新银行相关

//游戏钱包结构
struct bank_game_wallet
{
	int	_game_id;
	char _game_name[30];
	__int64	_money;
};

//普通操作存取钱
struct bank_op_normal
{
	int _operate_type;
	int	_user_id;
	int	_game_id;
	TCHAR _szMD5Pass[50];  ///银行密码
	__int64	_money;
};

//转帐
struct bank_op_transfer
{
	int	_user_id1;
	int	_user_id2;
	int _recode_id;
	__int64	_money;
	char _user_name1[60];
	char _user_name2[60];
};

//存单,充值
struct bank_op_deposit
{
	int _operate_type;  //1为创建存单,2为充值
	int	_user_id;
	TCHAR _szNickName[50];  ///被转账用户昵称
	TCHAR _szMD5Pass[50];   ///银行密码
	int	_state;
	int	_csds;
	__int64	_money;
	char _deposit_id[16];
	char _deposit_password[16];
	char _time[32];
};

//存单查询结构
struct bank_op_deposit_node
{
	int	_serial_no;	    ///< 序号
	char _deposit_id[16];
	char _deposit_password[16];
	__int64	_money;
	COleDateTime _create_time;
	int	_csds;
	int	_user_id;
	int	_state;
	int	_used_user_id;
};


//游戏公告消息ATT
struct GameNoticeMessage
{
	BYTE			bDeskIndex;			//桌子号
	BYTE			bAwardCardShape;	//牌型奖励
	TCHAR			szMessage[255];		//未进入游戏 在房间 右边框显示公告内容
	GameNoticeMessage()
	{
		::memset(this,0,sizeof(GameNoticeMessage));
	}
};

// Unity3d消息结构
struct u3dSGameBaseInfo {
	u3dSGameBaseInfo() {
		ZeroMemory(this, sizeof(u3dSGameBaseInfo));
	}
	int					iDeskPeople;								//房间人数
	int					iLessExperience;							//最少经验值
	int					iPower;										//倍率
	long				lRoomRule;									//房间规则
	int					iGameNameID;								//游戏ID
	BOOL				bCanTalk;									//是否可以聊天
	UINT				uTaskAwardType;								//房间奖励类型序列号 以此为下标中找
	UINT				uTaskAwardUnit;								//房间奖励类型倍数，通常是正整数
	WCHAR				szGameRoomName[64];							//房间名称	
	WCHAR				money[320];									
	WCHAR				virtualMoney[320];	
	int					iContestType;								//比赛类型
	int					iDeskNO;									//桌子号
};

//用户基本信息

//用户同意消息结构
struct u3dUserAgree {
	int		bDeskNO;						//游戏桌号
	int		bDeskStation;					//位置号码
	int		bAgreeGame;						//同意标志
	int		bNotUse;						//仅仅用于对齐占位置
};
struct VoiceInfo
{
	UINT uUserID;//用户id
	UINT uVoiceID;//音频ID
	int	 iVoiceTime;//语音时长

	VoiceInfo()
	{
		::memset(this,0,sizeof(VoiceInfo));
	}
};
//桌子已到期
struct VipDeskRunOut
{
	bool bSuccess;		//是否成功
	BYTE byDeskIndex;	//桌号
	VipDeskRunOut()
	{
		bSuccess=false;
		byDeskIndex=255;
	}
};
//是否同意解散桌子
struct VipDeskDismissAgree
{
	bool bAgree;//是否同意
	VipDeskDismissAgree()
	{
		memset(this,0,sizeof(VipDeskDismissAgree));
	}
};
//同意解散桌子结果
struct VipDeskDismissAgreeRes
{
	int iUserID;
	bool bAgree;//是否同意
	VipDeskDismissAgreeRes()
	{
		memset(this,0,sizeof(VipDeskDismissAgreeRes));
	}
};
//获得桌子VIP信息
struct VipDeskInfo
{
	BYTE byDeskIndex;//桌号
	VipDeskInfo()
	{
		memset(this,0,sizeof(VipDeskInfo));
	}
};
//桌子VIP信息结果
struct VipDeskInfoResult
{
	BYTE byDeskIndex;//桌号
	int  iMasterID;//桌主ID
	int  iPlayCount; //总局数
	int  iNowCount;		//当前局数
	int  iType;			//积分方式（0局数/1胡息/2圈数/3时效）
	int  iSeconds;      //剩余秒数iType为3时有效
    BYTE bTimeKeeper;   //是否开始计时(iType为3时有效)
    char szPassWord[20];//桌子密码
	char szGameName[64]; //游戏名字
	VipDeskInfoResult()
	{
		memset(this,0,sizeof(VipDeskInfoResult));
	}
};
//广播解散桌子通知
struct VipDeskDismissNotify
{
	int iUserID;		//申请解散桌子ID
	int iTimeCount;		//超时倒计时(秒)
	VipDeskDismissNotify()
	{
		memset(this,0,sizeof(VipDeskDismissNotify));
	}
};

//断线重连玩家获取解散消息
struct NetCutDismissNotify
{
	int iUserID;//申请解散桌子ID
	int iUserAgreeID[20]; //同意玩家ID
	int iAgreeNum;
	int iTimeCount;		//超时倒计时(秒)
	NetCutDismissNotify()
	{
		memset(this,0,sizeof(NetCutDismissNotify));
	}
};
//获取解散倒计时
struct VipDeskDismissTimeCount
{
	int iTimeCount;		//超时倒计时(秒)
	VipDeskDismissTimeCount()
	{
		memset(this,0,sizeof(VipDeskDismissTimeCount));
	}
};

//离线玩家信息
struct MSG_GM_S_ReturnGameInfo
{
	BYTE								bDeskID;
	int									iUserSitNum;    //用户位置号
	int									iUserID;	
	__int64								i64Score;
	char								nickName[61];
	char								HeadUrl[256];
	int									LogoID;
	int									iUserState;		//用户状态1,在线，2断线，3离线
	MSG_GM_S_ReturnGameInfo()
	{
		memset(this,0,sizeof(MSG_GM_S_ReturnGameInfo));
	}
};

struct MSG_GR_S_ContestAbandon
{
	int		iUserID;
	__int64 i64WalletMoney; 
	int		iJewels;
	MSG_GR_S_ContestAbandon()
	{
		memset(this,0,sizeof(MSG_GR_S_ContestAbandon));
	}
};

struct MSG_GR_S_BuyRoom_CostFee
{
	int iUserID;
	int iJewels;
	int iLockJewels;
};

struct MSG_GR_S_Position_Notice
{
	int		iUserIDA;
	int		iUserIDB;
	int		iDistance;
	bool	bDistance;
	bool	bSameIP;
};

struct MSG_GR_I_GetDeskUserInfo
{
	int iUserID;			//要获取的玩家ID
	MSG_GR_I_GetDeskUserInfo()
	{
		memset(this,0,sizeof(MSG_GR_I_GetDeskUserInfo));
	}
};

struct MSG_GR_O_GetDeskUserInfo
{
	MSG_GR_R_UserSit	SitInfo;
	UserInfoStruct		UserInfo;
	MSG_GR_O_GetDeskUserInfo()
	{
		memset(this,0,sizeof(MSG_GR_O_GetDeskUserInfo));
	}
};

//交换玩家座位
struct MSG_GR_O_ChangeUserStation
{
	BYTE  bDeskID;
	int   iUserIDA;
	int   iUserIDB;
	BYTE  bUserANewStation;
	BYTE  bUserBNewStation;

	MSG_GR_O_ChangeUserStation()
	{
		memset(this,0,sizeof(MSG_GR_O_ChangeUserStation));
	}
};
/***************************************主消息ID****************************************************/
enum GR_ROOM_MAINID
{
	MDM_GR_LOGON = 100,								// 房间登陆
	MDM_GR_USER_LIST = 101,							// 用户列表
	MDM_GR_USER_ACTION = 102,						// 用户动作
	MDM_GR_ROOM = 103,								// 房间信息
	MDM_GR_MATCH_INFO = 104,						// 比赛信息
	MDM_GR_MANAGE = 105,							// 管理消息
	MDM_GR_MESSAGE = 106,                           // 信息消息
	MDM_GR_MONEY = 107,                             // 划账，扣钱
	MDM_GR_POINT = 108,								// 经验操作
	MDM_GR_FRIEND_MANAGE = 113,						// 好友消息
	MDM_GR_QUEUE_MSG = 114,							// 排队消息
	MDM_GR_DESK = 115,
	MDM_GR_NETSIGNAL = 116,							// 网络信号消息
	MDM_GR_ROOM2GAME = 120,							// 由房间（起始点）发给游戏端（接收终点）的消息
	MDM_BANK = 131,
	MDM_GM_GAME_FRAME = 150,						// 框架消息
	MDM_GM_MESSAGE = 151,							// 房间系统信息
	MDM_GR_PROP = 160,								// 房间和游戏中道具相关的消息
	MDM_GM_GAME_NOTIFY = 180,						// 游戏消息
	MDM_GR_DESKRUNOUT = 181,						// 桌子解散主消息
	MDM_GR_RETURNDESK = 182,						// 返回桌子s
	MDM_GR_VOIEC = 183,								// 语音消息
	//MDM_GAMEMASTER_MESSAGE = 255,					// GM消息主ID
};
/***************************************MDM_GR_LOGON****************************************************/
enum GR_ASSID_LOGON									
{
	ASS_GR_LOGON_SUCCESS = 2,						// 登陆成功
	ASS_GR_LOGON_ERROR = 3,							// 登陆失败
	ASS_GR_SEND_FINISH = 4,							// 登陆完成
	ASS_GR_LOGON_BY_ID = 5,							// 通过用户 ID 登陆
	ASS_GR_IS_VIPROOM = 16,                         // 是否VIP房间
	ASS_GR_VIP_PW = 17,                             // VIP房间需要密码
	ASS_GR_VIP_NO_PW = 18,                          // VIP房间不需要密码(第一个进入不需要密码)
	ASS_GR_NO_VIP = 19,                             // 不是VIP房间
	ASS_GR_VIPROOM_PW = 20,                         // VIP房间密码
	ASS_GR_VIPROOM_PW_RIGHT = 21,                   // VIP房间密码正确
};
enum E_GR_ERROR_LOGON								// 登陆错误
{
	ERR_GR_ERROR_UNKNOW = 0,						// 未知错误
	ERR_GR_LOGON_SUCCESS = 1,						// 登陆成功
	ERR_GR_USER_NO_FIND = 2,						// 用户不存在
	ERR_GR_USER_PASS_ERROR = 3,						// 用户密码错误
	ERR_GR_USER_VALIDATA = 4,						// 用户帐号禁用
	ERR_GR_USER_IP_LIMITED = 5,						// 登陆 IP 禁止
	ERR_GR_IP_NO_ORDER = 6,							// 不是指定地址
	ERR_GR_ONLY_MEMBER = 7,							// 会员游戏房间
	ERR_GR_IN_OTHER_ROOM = 8,						// 正在其他房间
	ERR_GR_ACCOUNTS_IN_USE = 9,						// 帐号正在使用
	ERR_GR_PEOPLE_FULL = 10,						// 人数已经满
	ERR_GR_LIST_PART = 11,							// 部分用户列表
	ERR_GR_LIST_FINISH = 12,						// 全部用户列表
	ERR_GR_STOP_LOGON = 13,							// 暂停登陆服务
	ERR_GR_CONTEST_NOSIGNUP = 14,					   
	ERR_GR_CONTEST_TIMEOUT = 15,					   
	ERR_GR_CONTEST_KICK = 16,						   
	ERR_GR_CONTEST_NOTSTRAT = 17,					   
	ERR_GR_CONTEST_OVER = 18,						   
	ERR_GR_CONTEST_BEGUN = 19,	
	ERR_GR_MATCH_LOGON = 160,						// 游戏房间
	ERR_GR_TIME_OVER = 161,							// 时间到期
	ERR_GR_BATTLEROOM_OUTTIME = 162,				// 不在混战场活动时间内
};
/***************************************MDM_GR_USER_LIST****************************************************/
enum GR_ASSID_USER_LIST								
{
	ASS_GR_ONLINE_USER = 1,							// 在线用户
	ASS_GR_NETCUT_USER = 2,							// 断线用户
	ASS_GR_DESK_STATION = 3,						// 桌子状态
};
/***************************************MDM_GR_USER_ACTION**************************************************/
enum GR_ASSID_USER_ACTION						
{
	ASS_GR_USER_UP = 1,								// 用户起来
	ASS_GR_USER_SIT = 2,							// 用户坐下
	ASS_GR_WATCH_UP = 3,							// 旁观起来
	ASS_GR_WATCH_SIT = 4,							// 旁观坐下
	ASS_GR_USER_COME = 5,							// 用户进入
	ASS_GR_USER_LEFT = 6,							// 用户离开
	ASS_GR_USER_CUT = 7,							// 用户断线
	ASS_GR_SIT_ERROR = 8,							// 坐下错误
	ASS_GR_SET_TABLE_BASEPOINT = 9,					// 改变桌子倍数
	ASS_GR_USER_HIT_BEGIN = 10,						// 用户同意开始
	ASS_GR_JOIN_QUEUE = 11,							// 加入排队机
	ASS_GR_QUIT_QUEUE = 12,							// 退出排队机
	ASS_GR_QUEUE_USER_SIT = 13,						// 排队用户坐下
	ASS_GR_LEASE_TIMEOVER = 14,						// 排队用户坐下
	ASS_GR_CHANGE_DESK = 15,						// 快速换桌
	ASS_GR_SIT_CUTRESIT = 16,						// 断线重连,重新坐下
	ASS_GR_FORCE_QUITE = 17,						// 强制退出
	ASS_GR_MASTER_LEAVE = 18,						// 房主主动退出
	ASS_GR_GET_USERINFO = 19,						// 手游获取玩家信息
	ASS_GR_CONTEST_APPLY = 24,						// 比赛场报名
	ASS_GR_FAST_JOIN_IN = 27,						// 快速坐桌
};
enum E_GR_ERROR_SIT									// 用户坐下错误码
{
	ERR_GR_SIT_SUCCESS		 = 50,					// 成功坐下
	ERR_GR_BEGIN_GAME		 = 51,					// 游戏已经开始
	ERR_GR_ALREAD_USER		 = 52,					// 已经有人存在
	ERR_GR_PASS_ERROR		 = 53,					// 密码错误
	ERR_GR_IP_SAME			 = 54,					// IP 相同
	ERR_GR_CUT_HIGH			 = 55,					// 断线率太高
	ERR_GR_POINT_LOW		 = 56,					// 经验值太低
	ERR_GR_POINT_HIGH		 = 57,					// 经验值太高
	ERR_GR_NO_FRIEND		 = 58,					// 不受欢迎
	ERR_GR_POINT_LIMIT		 = 59,					// 经验值不够
	ERR_GR_CAN_NOT_LEFT		 = 60,					// 不能离开
	ERR_GR_NOT_FIX_STATION	 = 61,					// 不是这位置
	ERR_GR_MATCH_FINISH		 = 62,					// 比赛结束
	ERR_GR_MONEY_LIMIT		 = 63,					// 金币太低
	ERR_GR_MATCH_WAIT		 = 64,					// 比赛场排队提示
	ERR_GR_IP_SAME_3		 = 65,					// IP前3 相同
	ERR_GR_IP_SAME_4		 = 66,					// IP前4 相同
	ERR_GR_UNENABLE_WATCH	 = 67,					// 不允许旁观
	ERR_GR_DESK_FULL		 = 68,					// 百家乐 桌子座位满了，无法分配座位给玩家
	ERR_GR_FAST_SIT			 = 69,					// 快速坐桌失败
	ERR_GR_POSITION_FAIL	 = 70,					// 定位失败
	ERR_GR_VIP_PASS_ERROR	 = 71,					// VIP桌密码错误
	ERR_GR_JEWEL_LIMIT		 = 72,					// 钻石不足
};

enum E_GR_ERROR_MASTER_LEAVE						// 房主离开错误码
{
	ERR_GR_LEAVE_SUCCESS		 = 1,				// 房主成功离开
	ERR_GR_NOT_BUY_ROOM			 = 2,				// 不是可购买房间
	ERR_GR_IN_GAME				 = 3,				// 游戏已开始
	ERR_GR_NOT_IN_DESK			 = 4,				// 用户还没坐下
	ERR_GR_CANNOT_LEAVE			 = 5,				// 房主不可以离开
	ERR_GR_NOT_MASTER			 = 6,				// 不是房主
	ERR_GR_IS_CLUBROOM			 = 7,				// 俱乐部房间开房者不能离开
};

enum E_GR_ERROR_GET_DESK_USERINFO					// 获取玩家信息
{
	ERR_GR_GET_USERINFO_SUCCESS	 = 0,				// 获取玩家信息成功
	ERR_GR_USER_NOT_ONLINE		 = 1,				// 用户不在线，返回空数据
	ERR_GR_USER_NOT_SITTING		 = 2,				// 用户没有坐下，有效的玩家信息，无效的坐下信息
	ERR_GR_USER_IN_DIFF_DESK	 = 3,				// 用户与申请人在不同桌子，有效的玩家信息，无效的坐下信息
	ERR_GR_USER_NOT_SITTING_MY	 = 4,				// 申请人不在座位上，返回空数据
};
/***************************************MDM_GR_ROOM******************************************************/
enum GR_ASSID_ROOM								
{
	ASS_GR_NORMAL_TALK = 1,							// 普通聊天
	ASS_GR_HIGH_TALK = 2,							// 高级聊天
	ASS_GR_USER_AGREE = 3,							// 用户同意
	ASS_GR_GAME_BEGIN = 4,							// 游戏开始
	ASS_GR_GAME_END = 5,							// 游戏结束
	ASS_GR_USER_POINT = 6,							// 用户经验值
	ASS_GR_SHORT_MSG = 7,							// 聊短信息
	ASS_GR_ROOM_SET = 8,							// 房间设置
	ASS_GR_INVITEUSER = 9,							// 邀请用户
	ASS_GR_INSTANT_UPDATE = 10,						// 即时更新分数金币
	ASS_GR_UPDATE_CHARM = 11,						// 即时更新魅力
	ASS_GR_ROOM_PASSWORD_SET = 12,					// 房间设置
	ASS_GR_ROOM_QUEUE_READY = 13,					// 排队机准备
	ASS_GR_GET_NICKNAME_ONID = 14,					// 根据ID获取昵称
	ASS_GR_OWNER_T_ONE_LEFT_ROOM = 15,				// 房主踢玩家离开房间
	ASS_GR_GET_NICKNAME_ONID_INGAME = 16,			// 根据ID获取昵称 游戏中
	ASS_GR_USER_CONTEST = 17,						// 用户比赛信息
	ASS_GR_AVATAR_LOGO_CHANGE = 18,					// 用户形象更改信息
	ASS_GR_CAHNGE_ROOM = 19,						// 比赛开始，未报名用户切换房间
	ASS_GR_CONTEST_GAMEOVER = 20,					// 比赛结束
	ASS_GR_CONTEST_KICK = 21,						// 用户被淘汰
	ASS_GR_CONTEST_WAIT_GAMEOVER = 22,				// 比赛结束，但是有用户还在打最后一局，通知已打完的用户等待排名
	ASS_GR_INIT_CONTEST = 23,						// 比赛开始，初始化用户排名和在线人数
	ASS_GR_BATTLEROOM_RESULT = 24,					// 混战房间结果记录信息
	ASS_GR_CONTEST_APPLYINFO = 25,					// 比赛场报名信息
	ASS_GR_CONTEST_ABANDON = 27,					// 比赛取消
	ASS_GR_MONEY_NOTENOUGH = 28,					// 游戏币不足提示
	ASS_GR_CONTEST_EXIT = 29,						// 比赛过程中退赛
	ASS_GR_CHANGE_USERSTATION = 30,					// 交换玩家座位
};

enum E_GR_ERROR_GET_NICKNAME_ONID					// ASS_GR_GET_NICKNAME_ONID
{
	RES_GR_GETNICKNAME_SUCCESS = 0,					// 操作成功
	RES_GR_GETNICKNAME_NOTEXIST = 1,				// ID 不存在
	RES_GR_GETNICKNAME_ISNULL = 2,					// 昵称为空
};

enum E_GR_ERROR_BATTLEROOM_RESULT					// ASS_GR_BATTLEROOM_RESULT
{
	RES_BATTLEROOM_TIMEOUT_ERROR = 21,				// 不在混战场活动时间内
};
/***************************************MDM_GR_MATCH_INFO****************************************************/
enum GR_ASSID_MATCH_INFO					
{
	ASS_GR_MATCH_USER = 1,							// 比赛用户
	ASS_GR_MATCH_DELETE = 2,						// 取消比赛
};
/***************************************MDM_GR_MANAGE******************************************************/
enum GR_ASSID_MANAGE				
{
	ASS_GR_CUT_USER = 1,							// 踢用户下线
	ASS_GR_WARN_MSG = 2,							// 警告消息
	ASS_GR_FORBID_USER = 4,							// 禁止用户帐号
	ASS_GR_FORBID_USER_IP = 5,						// 禁止用户IP
	ASS_GR_LIMIT_OTHER_ROOM_TALK = 6,				// 禁止用户大厅聊天
	ASS_GR_CALL_GM = 7,								// 呼叫GM
	ASS_GR_CALL_GM_SUCCESS = 8,						// 呼叫GM成功
	ASS_GR_CALL_GM_FAIL = 9,						// 呼叫GM失败
	ASS_GR_ALONE_DESK = 15,							// 封桌
	ASS_GR_DEALONE_DESK = 16,						// 解除封桌
	ASS_GR_GRM_SET = 17,							// 房间管理窗口数据修改消息
	ASS_GR_GRM_SET_RES = 18,						// 房间管理窗口数据修改结果消息
	ASS_GR_GRM_UPDATA = 19,							// 刷新房间管理窗口数据
};
enum E_GR_ERROR_POWER_SET							// ASS_GR_POWER_SET
{
	RES_GR_GAME_POWER_SET_SUCCESS = 10,				// 设置游戏权限成功
	RES_GR_GAME_POWER_SET_FAIL = 11,				// 设置游戏权限失败
	RES_GR_GAME_POWER_SET_RETURN = 12,				// 设置游戏权限反馈
	RES_GR_GAME_POWER_SET_RELEASE = 13,				// 解除游戏权限成功反馈
};
enum E_GR_ERROR_GRM_SET_RES							// ASS_GR_GRM_SET_RES
{
	RES_GR_GRM_SET_RES_SUC = 1,						// 房间管理窗口数据修改成功
	RES_GR_GRM_SET_RES_ERR1 = 2,					// 房间管理窗口数据修改失败(奖池区域)
	RES_GR_GRM_SET_RES_ERR2 = 3,					// 房间管理窗口数据修改失败(玩家输赢概率区域)
	RES_GR_GRM_SET_RES_ERR3 = 4,					// 房间管理窗口数据修改失败(不是管理员或该房间不能设置)
	RES_GR_GRM_SET_RES_ERR_ALL = 5,					// 房间管理窗口数据修改失败(全部)
};
enum E_GR_ERROR_GRM_UPDATA							// ASS_GR_GRM_UPDATA
{
	RES_GR_GRM_UP1 = 1,								// 只更新奖池的机器人总赢钱数
	RES_GR_GRM_UP2 = 2,								// 更新所有数据
};
/***************************************MDM_GR_MESSAGE******************************************************/	
enum GR_ASSID_MESSAGE							
{
	ASS_GR_SYSTEM_MESSAGE = 1,						// 系统消息
};
/***************************************MDM_GR_MONEY******************************************************/	
enum GR_ASSID_MONEY
{
	ASS_GR_OPEN_WALLET = 1,							// 察看余额
	ASS_GR_CHECK_OUT = 2,							// 取出金币
	ASS_GR_CHECK_IN = 3,							// 存入金币
	ASS_GR_REFLASH_MONEY = 4,						// 同步各房间金币
	ASS_GR_PAY_MONEY = 5,							// 扣钱
	ASS_GR_TRANSFER_MONEY = 6,						// 转帐
	ASS_GR_SENDMONEY_TIMES = 7,                     // 按次送钱
	ASS_GR_OPEN_WALLET_INGAME = 8,					// 游戏中打开平台察看余额
	ASS_GR_CHECK_OUT_INGAME = 9,					// 游戏中取出金币
	ASS_GR_TRANSFER_RECORD = 10,					// 转帐历史记录
	ASS_GR_CHECK_IN_INGAME = 11,					// 游戏中存入金币
	ASS_GR_TRANSFER_MONEY_INGAME = 12,				// 游戏中转帐
	ASS_GR_TRANSFER_RECORD_INGAME = 13,				// 游戏中转帐历史记录
	ASS_GR_CHANGE_PASSWD = 14,						// 修改密码
	ASS_GR_CHANGE_PASSWD_INGAME = 15,				// 游戏中修改密码
	ASS_GR_TRANSFER_SET_REBATE_RES = 16,			// 修改返利数据
	ASS_GR_TRANSFER_SET_REBATE_RES_INGAME = 17,		// 修改返利数据
	ASS_GR_TRANSFER_UPDATE_USER_REBATE = 18,		// 更新玩家返利数据
	ASS_GR_TRANSFER_UPDATE_USER_REBATE_INGAME = 19,	// 更新玩家返利数据
};
enum E_GR_ERROR_OPEN_WALLET							// ASS_GR_OPEN_WALLET
{
	ASS_GR_OPEN_WALLET_ERROR = 1,					// 打开钱柜错误
	ASS_GR_OPEN_WALLET_SUC = 2,						// 打开钱柜成功
	ASS_GR_OPEN_WALLET_ERROR_PSW = 3,				// 打开钱框错误
};
enum E_GR_ERROR_CHECKIN								// ASS_GR_CHECK_IN  
{
	ASS_GR_CHECKIN_ERROR = 1,						// 存钱失败
	ASS_GR_CHECKIN_SUC = 2,							// 存钱成功
	ASS_GR_CHECKIN_ERROR_PLAYING = 3,				// 游戏中，不能存款
	ASS_GR_CHECKIN_ZHUANGJIA = 4,					// 百人类游戏，庄家不能存款
};
enum E_GR_ERROR_CHECKOUT							// ASS_GR_CHECK_OUT 
{
	ASS_GR_CHECKOUT_ERROR = 1,						// 取钱错误
	ASS_GR_CHECKOUT_SUC = 2,						// 取钱成功
	ASS_GR_CHECKOUT_ERROR_PLAYING = 5,				// 游戏中  不能取款
};
enum E_GR_ERROR_TRANSFER_MONEY 						// ASS_GR_TRANSFER_MONEY 
{
	ASS_GR_TRANSFER_ERROR = 1,						// 转帐错误
	ASS_GR_TRANSFER_SUC = 2,						// 转帐成功
	ASS_GR_TRANSFER_NO_DEST = 3,					// 转帐目标不存在
	ASS_GR_PASSWORD_ERROR = 4,						// 转帐密码错误
	ASS_GR_TRANSFER_TOTAL_LESS = 5,					// 银行总额太小，不够资格
	ASS_GR_TRANSFER_TOO_LESS = 6,					// 单笔转帐数目太少
	ASS_GR_TRANSFER_MULTIPLE = 7,					// 单笔转帐数目必须是某数的倍数
	ASS_GR_TRANSFER_NOT_ENOUGH = 8,					// 银行金额不够本次转帐
	ASS_GR_TRANSFER_TOO_MANY_TIME = 9,              // 当天转账的次数太多了
};
/***************************************MDM_GR_POINT**********************************************************/	
enum GR_ASSID_POINT							
{
	ASS_GR_OPEN_POINT = 1,							// 打开经验盒
	ASS_GR_OPEN_SUC = 2,							// 打开完成
	ASS_GR_OPEN_ERROR = 3,							// 打开错误
	ASS_GR_GET_POINT = 4,							// 取出经验
	ASS_GR_GETP_SUC = 5,							// 取出经验完成
	ASS_GR_GETP_ERROR = 6,							// 取出错误
};
/***************************************MDM_GR_QUEUE_MSG**********************************************************/	
enum GR_ASSID_QUEUE_MSG							
{
	ASS_GR_QUEUE_ERROR = 0,							// 错误
	ASS_GR_ADD_QUEUE = 1,							// 加入排队
	ASS_GR_ADD_QUEUE_RESULT = 2,					// 排队结果
	ASS_GR_PLAN_SIT = 3,							// 分配桌子
	ASS_GR_AGREE_PLAN_SIT = 4,						// 服务分配桌子
	ASS_GR_REFUSE_PLAN_SIT = 5,						// 不服从分配坐号
};
/***************************************MDM_GR_ROOM2GAME******************************************************/	
enum GR_ASSID_ROOM2GAME							
{
	ASS_GR_R2G_USEPROP = 1,							// 游戏端道具的即买即用功能
};
/***************************************MDM_BANK******************************************************/	
enum GR_ASSID_BANK							
{
	ASS_BANK_NORMAL = 1,
	ASS_BANK_TRANSFER = 2,
	ASS_BANK_DEPOSIT = 3,
	ASS_BANK_WALLET = 4,
	ASS_BANK_QUERY_DEPOITS = 5,
};
enum E_GR_ERROR_BANK							
{
	HC_BANK_OP_SUC = 0,								// 操作成功
	HC_BANK_OP_NO_USER = 1,							// 没有找到用户
	HC_BANK_OP_ERR_PWD = 2,							// 错误密码
	HC_BANK_OP_CHECKMONEY_INV = 5,					// 非法存取钱
	HC_BANK_OP_CHECKIN_PLAYING = 9,					// 游戏中，不能存款
	HC_BANK_OP_CHECKOUT_PLAYING = 10,				// 游戏中，不能取款
	HC_BANK_OP_CHECKIN_ZHUANG = 11,					// 百人类游戏，庄家不能存款
};
/***************************************MDM_GM_GAME_FRAME******************************************************/	
enum GR_ASSID_GAME_FRAME						
{
	ASS_GM_GAME_INFO = 1,							// 游戏信息
	ASS_GM_GAME_STATION = 2,						// 游戏状态
	ASS_GM_FORCE_QUIT = 3,							// 强行退出
	ASS_GM_NORMAL_TALK = 4,							// 普通聊天
	ASS_GM_HIGH_TALK = 5,							// 高级聊天
	ASS_GM_WATCH_SET = 6,							// 旁观设置	
	ASS_GM_USE_KICK_PROP = 7,						// 使用踢人卡
	ASS_GM_USE_ANTI_KICK_PROP = 8,					// 使用防踢卡
	ASS_GM_CLEAN_USER = 9,                          // 比赛场清理用户信息
	ASS_GM_SET_VIDEOADDR = 10						// 设置视频服务器地址
};
enum E_GR_ERROR_KICK_PROP							// 踢人卡、防踢卡错误消息
{
	ERR_GR_KICK_PROP_NULL = 1,						// 没有道具
	ERR_GR_KICK_SET_TIME_FAIL = 2,					// 设置时间错误
	ERR_GR_KICK_SET_TIME_SUCCESS = 3,				// 设置时间成功
};
enum E_GR_ERROR_NEW_KICKUSER						// 使用踢人卡踢人错误消息
{
	ERR_GR_NEW_KICKUSER_SUCCESS = 1,				// 成功
	ERR_GR_NEW_KICKUSER_NOTIME = 2,					// 自己的道具已过期，或者没有道具
	ERR_GR_NEW_KICKUSER_HAD_ANTI = 3,				// 对方有防踢卡时间
	ERR_GR_NEW_KICKUSER_HAD_VIP = 4,				// 对方有VIP时间
	ERR_GR_NEW_KICKUSER_PLAYING = 5,				// 游戏中不踢人
};
/***************************************MDM_GM_MESSAGE******************************************************/	
enum GR_ASSID_GM_MESSAGE						
{
	ASS_GM_SYSTEM_MESSAGE = 1,						// 系统消息
	ASS_GM_SYSTEM_MESSAGE_EX = 2,					// 系统消息特殊游戏专用（ATT）
};
/***************************************MDM_GM_GAME_NOTIFY******************************************************/	
enum GR_ASSID_GM_GAME_NOTIFY						
{
	ASS_GM_MESSAGE_EX = 180,						// 游戏消息特殊游戏ATT
	ASS_GM_AGREE_GAME = 1,							// 同意游戏
	ASS_PROP = 2,	
};		
/***************************************MDM_GR_DESKRUNOUT******************************************************/
enum GR_ASSID_DESKRUNOUT					
{
	ASS_GR_GETDESKMASTER = 0,						// 获得桌子vip信息
	ASS_GR_DESKRUNOUT = 1,							// 桌子解散
	ASS_GR_DISSMISS = 2,							// 申请解散
	ASS_GR_DISSMISS_AGREE = 3,						// 用户回复解散申请
	ASS_GR_DISSMISS_CANCEL = 4,						// 用户取消同意状态
	ASS_GR_DISSMISS_NETCUT = 5,						// 重连解散消息
	ASS_GR_GETDISTANCEINFO = 6,						// 距离过近提示
	ASS_GR_DISTANCE_AGREE = 7,						// 是否同意距离过近
	ASS_GR_DISTANCE_ALLAGREE = 8,					// 所有人同意距离过近
	ASS_GR_JEWES_CHANGE = 9,						// 玩家钻石变化
	ASS_GR_GETDISMISS_TIMECOUNT = 10,				// 获取申请解散房间倒计时
};
/***************************************MDM_GR_RETURNDESK******************************************************/
enum GR_ASSID_RETURNDESK						
{
	ASS_GR_RETURNDESK = 0,
};
/***************************************MDM_GR_VOIEC***********************************************************/
enum GR_ASSID_VOIEC						
{
	ASS_GR_VOIEC = 0,								// 语音发送消息
	RES_GR_VOIEC_SUC = 1,							// 语音发送成功
};
#define MDM_GAMEMASTER_MESSAGE	255	// GM消息主ID

/********************************************************************************************/
// Unity3d消息ID 初始化数据
#define IPC_MAIN_GAMEBASEINFO				8
#define IPC_SUB_GAMEBASEINFO				0
#define IPC_SUB_DATAOVER					1
#define IPC_SUB_REINIT						2

#define IPC_MAIN_USERINFO					10
#define IPC_SUB_USERINFOLIST				0								//用户列表
#define IPC_SUB_USERAGREE					1								//用户同意
#define IPC_SUB_USERSITDOWN					2								//用户坐下
#define IPC_SUB_USERSTANDUP					3								//用户起来
#define IPC_SUB_USEOFFLINE					4								//用户断线
#define IPC_SUB_CLICKBEGIN					5								//用户点击开始游戏
#define IPC_SUB_USERDETAILINFO				6								//用户详细信息
#define IPC_SUB_UPDATEUSERINFO				7								//刷新用户信息
#define IPC_SUB_MATCHRANK					8								//比赛排名

#define IPC_SUB_USERSTANDUP_STATION			9								//用户站起位置
#define IPC_SUB_USERSIT_STATION				10								//用户坐下位置

//更新金币经验魅力
#define IPC_MAIN_UPDATE						11								//更新金币主ID
#define IPC_SUB_USERPOINT					1								//更新金币分数
#define IPC_SUB_USEREXPERIENCE				2								//更新用户经验
#define IPC_SUB_USERFASCINATION				3								//更新用户魅力

#define IPC_MAIN_QUEUE						13
#define IPC_SUB_JOIN						1
#define IPC_SUB_QUIT						2
#define IPC_SUB_BEGIN						3

#define IPC_MAIN_TASKAWARDLIST				14
#define IPC_SUB_TASKAWARDLIST				1
#define IPC_SUB_TASKAWARD					2

#define IPC_MAIN_TASKDAILY					15
#define IPC_SUB_TASKDAILY_LIST				1
#define IPC_SUB_TASKDIALY_UPDATE			2

#define IPC_MAIN_CONTESTINFO				16
#define IPC_SUB_CONTEST_REALLOTDESK			1
#define IPC_SUB_CONTEST_AUTO				2
#define IPC_SUB_CONTEST_FINISH_INFO			3
#define IPC_SUB_OPEN_RANK_PAGE				4
#define IPC_SUB_CONTEST_NOSCORE				5
#define IPC_SUB_CONTEST_FINISH_TIME			6

#define IPC_MAIN_BANK						17
#define	IPC_SUB_CHECK_IN_MONEY				1			//存钱
#define IPC_SUB_CHECK_OUT_MONEY				2			//取钱
#define IPC_SUB_CHECKIN_ERROR				3			//存钱失败
#define IPC_SUB_CHECKIN_ERROR_PLAYING		4			//游戏中不能存
#define IPC_SUB_CHECKOUT_ERROR				5			//取款失败
#define IPC_SUB_CHECKOUT_ERROR_PASSWORD		6			//二级密码错误

#define IPC_SYSTEMMSG						18
#define IPC_SUB_PUREMSG						1
#define IPC_SUB_SYSTEMMSG					2

#define IPC_MAIN_ACHIEVE					19
#define IPC_SUB_SET_USERINFO				1

#define IPC_MAIN_GAMEMASTER					20
#define IPC_SUB_GM_LIST						1
#define IPC_SUB_GM_TALK_TOGM				2
#define IPC_SUB_GM_TALK_FROMGM				3

//自己定义的协议
#define IPC_MAIN_GAMEMASSAGE				180
#define IPC_FRAME_GAMEMASSAGE				150
#define IPC_SYSTEM_GAMEMESSAGE				151
#define IPC_SUB_GAMEINFO					1

//一个游戏强制退出为安全退出
#define ASS_ONE_FINISH						96

#pragma pack()
/********************************************************************************************/
