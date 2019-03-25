/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#ifndef AFC_GAME_PLACE_MESSAGE_HEAD_FILE
#define AFC_GAME_PLACE_MESSAGE_HEAD_FILE

#include "ComStruct.h"

/********************************************************************************************/

///	网络通信数据包定义 

/********************************************************************************************/

///版本定义
#define GAME_PLACE_MAX_VER				4									///游戏大厅最新版本
#define GAME_PLACE_LESS_VER				4									///游戏大厅最低版本
#include "HNBaseType.h"
#pragma pack(1)
//用户注册
struct MSG_GP_S_Register
{
	BYTE	byFastRegister;			//0-快速注册，1-普通注册，2-微信登陆，3-QQ登录,4-手机号码注册
	char    szHardID[64];			//硬件ID	
	char	szName[32];				//用户名
	char	szPswd[33];				//密码
	char	szWeiXinName[50];		//微信昵称
	bool	bBoy;					//性别
	char	szHeadUrl[256];			//头像URL
	char	szUionID[64];			//微信、QQ登录标识
};

///用户登陆（帐号）结构
struct MSG_GP_S_LogonByNameStruct
{
	bool	bForced;
	BYTE	bLogonType;			// 0-游客登录，1-账号登录，2-微信登陆，3-QQ登录,4--手机号登录
	char	szName[32];			///登陆名字	
	char	TML_SN[128];						
	char	szMD5Pass[33];		///登陆密码
	char	szHardID[64];		///本机机器码 锁定机器
	int		iUserID;			//用户ID登录，如果ID>0用ID登录
};

///锁定机器
struct MSG_GP_S_LockAccount
{
	UINT								dwUserID;				///用户ID
	UINT								dwCommanType;			///命令请求类型,1表示要求锁定，0表示要求解除锁定
	char								szMathineCode[64];		///机器码
	char								szMD5Pass[50];			///加密密码
	char								szMobileNo[50];			///手机号码 
};

///绑定手机号码
struct MSG_GP_S_BindMobile
{
	UINT								dwUserID;				///用户ID
	UINT								dwCommanType;			///命令请求类型,1表示要求绑定，0表示要求解除绑定
	char								szMobileNo[50];			///手机号码  
};

///测网速  yjj
struct MSG_GP_NETSignal
{
        UINT                                                                dwUserID;
        UINT                                                                dwIndex;
        UINT                                                                dwTimeFlag;
};

///服务器返回后加上游戏房间的时间标签，供游戏使用
struct MSG_GP_Game_NETSignal
{
	UINT                                                                dwUserID;
	UINT                                                                dwIndex;
	UINT                                                                dwTimeFlag;
	UINT                                                                dwSignalIndex;
};

///用户登陆（ACC）
struct MSG_GP_S_LogonByAccStruct
{
	UINT								uRoomVer;							///大厅版本
	int									dwAccID;							///ACC 号码
	char								szMD5Pass[50];						///登陆密码
};

///用户登陆（手机）
struct MSG_GP_S_LogonByMobileStruct
{
	UINT								uRoomVer;							///大厅版本
	int									dwMobile;							///手机号码
	char								szMD5Pass[50];						///登陆密码
};

///用户注册信息数据包
struct MSG_GP_S_RegisterStruct
{
	UINT								uRoomVer;							///大厅版本
	BYTE								bBoy;								///是否男性别
	UINT								bLogoID;							///用户头像 ID 号码
	char								szName[61];							///用户登录名
	char								nickName[50];						///用户昵称
	char								szRecommenderName[61];				///推荐人
	char								szMD5Pass[33];						///用户加密密码
	char								szRegSQL[210];						///用户注册附加信息的SQL
};

//用户设置头像
struct MSG_GP_S_SET_LOGO
{
	long int iUserID;
	UINT     iLogoID;
};

struct MSG_GP_R_IDVerify
{
	long int							dwUserID;							///用户 ID 
	char								szMD5IDCardNo[36];					///证件号
};

struct MSG_GP_R_MobileVerify
{
	long int							dwUserID;							///用户 ID 
	char								szMobileNo[50];						///手机号
};
//统计登录人数
struct ONLINEUSERSCOUNT
{
	UINT								uiLogonPeopCount;					///登录人数
};

///大厅登陆返回数据包
struct MSG_GP_R_LogonResult
{
	int									dwUserID;							///用户 ID 
	UINT								dwNowLogonIP;						///现在登陆 IP
	BYTE								iLogonType;							///登录类型
	int									bLogoID;							///用户头像//mark
	bool								bBoy;								///性别
	char								szName[32];							///用户登录名
	//char								TML_SN[128];						///数字签名
	char								szMD5Pass[33];						///用户密码
	char								nickName[50];						///用户昵称
	__int64								i64Money;							///用户金币
	__int64								i64Bank;							///用户财富
	int									iJewels;							///钻石
	int									iLockJewels;						///冻结的钻石
	int									iLotteries;							///奖券
	//新用户资料
	char								szSignDescr[128];					///个性签名
	char								szMobileNo[20];
	char								szHeadUrl[256];						///头像URL
	int									iVipTime;							///
	bool								bLoginBulletin;						///是否有登录公告
	int									iLockMathine;						///当前帐号是否锁定了某台机器，1为锁定，0为未锁定
	int									iBindMobile;						///当前帐号是否绑定手机号码，1为绑定，0为未绑定

	int									iAddFriendType;						///是否允许任何人加为好友
	BYTE								iCutRoomID;							///断线重连房间号-1,0,无断线，1有断线
};

struct UserInfoDetail
{
	char								szRealName[20];						///真实姓名
	char								szIDCardNo[36];						///证件号
	char								szMobileNo[50];						///移动电话
	char								szQQNum[20];						///QQ号码
	char								szAdrNation[50];					///玩家的国藉
	char								szAdrProvince[50];					///玩家所在的省份
	char								szAdrCity[50];						///玩家所在的城市
	char								szZipCode[10];						///邮政编码
	char								szEmail[50];						///电子邮件
	char								szAddress[128];						///联系地址
	char								szSchooling[20];					///文化程度
	char								szHomePage[128];					///个人主页
	char								szTelNo[20];						///固定电话
	char								szMSNID[50];						///MSN帐号
};

//用户信息结构及修改信息的数据包定义
typedef MSG_GP_R_LogonResult MSG_GP_UserInfo;

struct MSG_GP_S_UserInfo
		: public MSG_GP_UserInfo
{

	MSG_GP_S_UserInfo(){::memset(this,0,sizeof(MSG_GP_S_UserInfo));}
};

struct ComContestList
{
	BYTE						bContestType;					//0-人满赛 1-定时赛 2-循环赛
	BYTE						bAwardType;						//0-金币 1-奖券
	BYTE						bEnterType;						//0-金币 2-钻石/房卡
	int							iGameID;
	int							iContestID;
	int							iContestAward;
	int							iOnlinePeople;
	int							iUpPeople;
	int							iEnterFee;
	__int64						BeginTime;
	char						szRoomName[50];
	bool						bInvalid;						//是否过期，TRUE已过期
	ComContestList(){memset(this,0,sizeof(ComContestList));}
};


struct MSG_GP_ContestApplyInfo
{
	BYTE						bContestType;
	BYTE						bAwardType;
	BYTE						bEnterType;	
	int							iGameID;
	int							iContestID;
	int							iContestAward;
	int							iOnlinePeople;
	int							iUpPeople;
	int							iEnterFee;
	__int64						BeginTime;
	char						szRoomName[50];
	bool						bInvalid;
	BYTE						bContestState;					//0-未报名，1已报名，2比赛已开始
	MSG_GP_ContestApplyInfo(){memset(this,0,sizeof(MSG_GP_ContestApplyInfo));}
};


struct MSG_GP_GetTimingMatchBeginTime
{
	UINT			iGameID;		//游戏ID
	int				iContestID;		//比赛ID
	COleDateTime	BeginTime;
};

struct MSG_GP_GETMatchBeginTime
{
	UINT		iGameID;
	int			iContestID;
	__int64     I64BeginTime;
};

struct MSG_GP_S_ChPassword
{
	long int dwUserID;		//用户ID 
	char szHardID[64];		//硬盘ID
	char szMD5OldPass[80];  //用户老密码
	char szMD5NewPass[80];  //用户新密码
	MSG_GP_S_ChPassword(){::memset(this,0,sizeof(MSG_GP_S_ChPassword));}
};



// 销售列表信息
struct MSG_GP_R_SellGameListResult
{
	char szGameName[61];						//游戏名称
	char szKindName[61];						//类型名称
	long NameID;								//游戏ID
	long KindID;								//类型ID
	long ParamID;								//游戏索引ID
};

///锁定（解除锁定）机器返回数据包
struct MSG_GP_R_LockAndUnlock
{
	UINT	dwUserID;
	UINT	dwCommanType;			///请求命令类型
	UINT	dwCommanResult;			///请求的结果
};

///绑定/解除绑定手机返回数据包
struct MSG_GP_R_BindMobile
{
	UINT	dwUserID;
	UINT	dwCommanType;			///请求命令类型
	UINT	dwCommanResult;			///请求的结果
};

///获取游戏房间数据包
struct MSG_GP_SR_GetRoomStruct
{
	UINT								uKindID;							///类型 ID
	UINT								uNameID;							///名字 ID
};
///玩家列表操作
struct MSG_GP_User_Opt_Struct
{
	LONG								dwTargetID;							///操作对像
	LONG								dwUserID;							///操作id
	UINT								uType;								///操作类型
};

///获取游戏房间数据包
struct MSG_GP_SR_OnLineStruct
{
	///UINT								uType;							///类型 ID
	///UINT								uID;							///名字 ID
	UINT								uKindID;							///类型 ID
	UINT								uNameID;							///名字 ID
	UINT								uOnLineCount;						///在线人数
};

//增加GM功能，添加普通玩家与GM间的通讯协议
/// 获取在线GM列表
struct MSG_GP_R_GmList_t
{
	int _nCount;				///< 数量
	int _arGmID[8];				///< 在线GM的ID
	MSG_GP_R_GmList_t(){memset(this, 0, sizeof(MSG_GP_R_GmList_t));}
};

/// 普通玩家与在线GM聊天
struct MSG_GP_S_Talk2GM_t
{
	int iUserID;		///< 发送者ID
	int iGMID;			///< GMID
	TCHAR szMsg[256];	///< 欲发送的信息
	MSG_GP_S_Talk2GM_t(){memset(this, 0, sizeof(MSG_GP_S_Talk2GM_t));}
};
// 增加GM功能，添加普通玩家与GM间的通讯协议}}

struct MSG_PROP_RESULT
{
	DWORD dwResult;
};

// 根据用户ID取昵称，或根据昵称取用户ID专用结构
struct MSG_GP_NickNameID_t
{
	long int _user_id;			//用户ID
	TCHAR	 _nickname[61];		//用户昵称
	BYTE	 _nType;			//0代表通过用户ID取用户昵称\
								//1代表通过用户昵称取用户ID
	BYTE	 _nUseful;			//用途标识	
	MSG_GP_NickNameID_t()
	{
		ZeroMemory(this, sizeof(MSG_GP_NickNameID_t));
	}
};

struct MSG_GP_GetContestRoomID
{
	int			iUserID;			//用户ID
	int			iContestID;			//比赛ID
};


struct ContestInfo
{
	int					iContestType;	//0-人满赛，1-定时赛, 2-循环赛
	int                 iConstestNum;   //已报名人数
	int                 iChampionCount; //夺冠次数
	int                 iBestRank;      //最佳名次
	int                 iJoinCount;     //参赛次数
	__int64				iContestTime;	//开始时间
	int                 iRankAward[3];  //前三名奖励信息
	int                 iAwardType[3];  //前三名奖励类型0-金币，1-奖券，2钻石
};

struct MSG_GP_GetContestRoomID_Result
{
	int						iUserID;			//用户ID
	ContestInfo				tContestInfo;
	ComRoomInfo				tRoomInfo;			//房间信息
};

///登陆服务器登陆信息
struct DL_GP_RoomListPeoCountStruct
{
	UINT							uID;							///用户 ID 
	UINT							uOnLineCount;						///在线人数
	int								uVirtualUser;					///扩展机器人人数
};

//进入密码房间时向服务器提交房间密码 
struct MSG_GP_CheckRoomPasswd 
{
	UINT uRoomID;			 //房间ID
	char szMD5PassWord[50];  //房间MD5密码
};


struct DL_CheckRoomPasswd
{
	bool bRet;		// 成功与否
	UINT uRoomID;	// 房间号
};

///领取金币
struct TReceiveMoney
{
	int			iUserID;
	__int64		i64Money;
	__int64		i64MinMoney;
	int			iCount;
	int			iTotal;
	int			iTime;
};

//兑换列表
struct TCharmExchange_Item
{
	int iID;
	int iPoint;
	int iPropID;
	int iPropCount;
};

struct TCharmExchange
{
	int iUserID;
	int iID;
};

struct TMailItem
{
	int iMsgID;
	int	iTargetUserID;
	int iIsSend;
	int iMsgType;
	__int64 i64TimeSend;
	char szSName[50];
	char szTitle[100];
	char szMsg[500];
};

//手机短信验证码获取
struct MSG_GP_SmsVCode
{
	char szName[61];		//用户名
	char szMobileNo[50];	//手机号码
	char szVCode[36];		//产生的验证码，使用MD5加密
	UINT nType;				//短信验证码类型，0为登陆验证\
											//1为锁机验证\
											//2为解锁机验证\
											//3为修改手机验证
											//4为绑定手机验证
											//5为解除绑定手机验证
};

//获取用户的财务消息
struct TMSG_GP_BankFinanceInfo
{
	int		iUserID;			//用户ID
	__int64 i64BankMoney;		//用户银行的钱
	__int64 i64WalletMoney;		//用户取出的钱
	int     iLotteries;			//用户奖券数
	int		iJewels;			//用户钻石
};

struct MSG_GP_PaiHangBang_In
{
	int		count;		//前几名
	int     type;		// 0:金币总合来排 1: 钱包的来 2: 银行的来 其它值，直接失败 3:魅力
};

struct MSG_GP_MoneyPaiHangBang_Item
{
	char								nickName[64];						///用户昵称
	int									iUserID;							//用户ID
    char                                szHeadUrl[256];                     //头像地址
    char                                szSignDescr[128];                   //个性签名
	long long							i64Money;							///用户金币
};

struct TZServerInfo
{
	int iZid;
	int	iServerPort;
};

struct MSG_ZDistriInfo
{
	DWORD	dwUserID;
	int		nZID;
};

//签到
struct MSG_GP_S_SIGN_CHECK_RESULT
{
	unsigned long		dwUserID;
	unsigned char		iRs;				//结果: 0:未签到 1-已经签到
	unsigned char		byCountDay;			//连续签到天数
	int					iAwardMoney[7];		//签到奖励金币
	int					iAwardJewels[7];	//签到奖励钻石
	int					iAwardLotteries[7];	//签到奖励奖券
	MSG_GP_S_SIGN_CHECK_RESULT()
	{
		memset(this,0,sizeof(MSG_GP_S_SIGN_CHECK_RESULT));
	}
};
struct MSG_GP_S_SIGN_DO_RESULT
{
	unsigned long		dwUserID;
	//unsigned char		iRs;			// 结果: 0-成功 1-失败，
	int					iGetMoney;		// 当天签到获取的金币
	int					iGetJewels;		// 当天签到获取的钻石
	int					iGetLotteries;	// 当天签到获取的奖券
};

// 在线奖励
struct MSG_GP_S_ONLINE_AWARD_CHECK_RESULT
{
	unsigned long	dwUserID;	
	unsigned int	iLeftTime;				//还差多少时间可以领取,默认为0
	unsigned int	iOnLineTimeMoney;		//能领取的金币
	MSG_GP_S_ONLINE_AWARD_CHECK_RESULT()
	{
		memset(this,0,sizeof(MSG_GP_S_ONLINE_AWARD_CHECK_RESULT));
	}
};
//查询在线奖励升级
struct MSG_GP_S_ONLINE_AWARD_CHECK_RESULT_EX
{
	unsigned long	dwUserID;	
	int				iEnable[15];				//是否可以领取0:不能领取；1:能领取；2:已经领取
	unsigned int	iLeftTime[15];				//还差多少时间可以领取,默认为0
	int				iOnlineTimeConfig[15];		//服务器配置的奖励时间
	int				iOnlineTimeMoneyConfig[15]; //服务器配置的在线奖励
	MSG_GP_S_ONLINE_AWARD_CHECK_RESULT_EX()
	{
		memset(this,0,sizeof(MSG_GP_S_ONLINE_AWARD_CHECK_RESULT_EX));
	}
};

struct MSG_GP_S_ONLINE_AWARD_DO_RESULT
{
	unsigned long	dwUserID;	
	unsigned int    iCurrentGetMoney;		//此次领取的金币
	unsigned int	iNextTime;				//下次时间
	unsigned int	iNextMoney;				//下次金币
};

struct MSG_GP_S_ONLINE_AWARD_DO_RESULT_EX
{
	unsigned long	dwUserID;	
	unsigned int    iCurrentGetMoney;		//此次领取的金币
	unsigned int	iNextTime;				//下次时间
	unsigned int	iNextMoney;				//下次金币
};
struct MSG_GP_S_ONLINE_AWARD_CHECK_EX
{
	int iUserID;//用户ID
	MSG_GP_S_ONLINE_AWARD_CHECK_EX()
	{
		memset(this,0,sizeof(MSG_GP_S_ONLINE_AWARD_CHECK_EX));
	}
};
struct MSG_GP_S_ONLINE_AWARD_DD_EX
{
	int		iUserID;//用户ID
	int     iGetMoney;//领取金额
	MSG_GP_S_ONLINE_AWARD_DD_EX()
	{
		memset(this,0,sizeof(MSG_GP_S_ONLINE_AWARD_DD_EX));
	}
};



struct MSG_GP_S_BUY_DESK//购买桌子
{
	int		iUserID;						// 用户ID
	int		iPlayeCount;					// 购买局数
	int		iGameID;						// 游戏ID
	BYTE	bFinishCondition;				// 结束条件，0局数，1胡息，2圈数, 3-时效
	BYTE	bPayType;						// 0开局之后扣除房费，1房主开房时付费，2AA制扣除房费
	BYTE	bPositionLimit;					// 0不开启定位，1开启定位
	BYTE	bPlayerNum;						// 游戏限制人数,0与游戏人数相同
	BYTE	bMidEnter;						// 0中途不可以进入，1中途可进入
    int     iBuyMinutes;                    // 购买时长
	BYTE	szDeskConfig[512];
	MSG_GP_S_BUY_DESK()
	{
		memset(this,0,sizeof(MSG_GP_S_BUY_DESK));
	}
};
struct MSG_GP_S_BUY_DESK_RES//购买桌子结果
{
	int		iUserID;						//用户ID
	int		iRoomID;						//房间号
	int		iDeskID;						//桌子号
	int		iJewels;						//所消耗钻石
	int		iGameNameID;					//游戏id
	char	szPassWord[20];					//桌子密码
	MSG_GP_S_BUY_DESK_RES()
	{
		memset(this,0,sizeof(MSG_GP_S_BUY_DESK_RES));
	}
};
struct MSG_GP_S_ENTER_VIPDESK//进入VIP桌子
{
	int iUserID;//用户ID
	char szInputPassWord[20];//桌子密码
	MSG_GP_S_ENTER_VIPDESK()
	{
		memset(this,0,sizeof(MSG_GP_S_ENTER_VIPDESK));
	}
};

//获得断线房间信息
struct MSG_GP_S_GET_CUTNETROOMINFO
{
	int iUserID;
	BYTE bType;				//0-所有房间，1-比赛房间，2-VIP房间
	MSG_GP_S_GET_CUTNETROOMINFO()
	{
		memset(this,0,sizeof(MSG_GP_S_GET_CUTNETROOMINFO));
	}
};
//获得断线房间信息/进入VIP房间
struct MSG_GP_S_GET_CUTNETROOMINFO_RES
{
	int iUserID;//用户ID
	int iRoomID;//房间号
	int iDeskID;//桌子号
	BYTE bType; //0-普通房间，1-比赛房间，2-VIP房间
	BYTE bPositionLimit;//是否开启定位0未开启，1开启
	char szPass[20];
	ComRoomInfo tCutNetRoomInfo;//断线房间信息
	MSG_GP_S_GET_CUTNETROOMINFO_RES()
	{
		iDeskID = 255;
		memset(this,0,sizeof(MSG_GP_S_GET_CUTNETROOMINFO_RES));
	}
};

//金币兑换钻石
struct MSG_GP_S_MONEY_TO_JEWEL
{
	int iUserID;//用户ID
	long long i64Money;//金币
	MSG_GP_S_MONEY_TO_JEWEL()
	{
		memset(this,0,sizeof(MSG_GP_S_MONEY_TO_JEWEL));
	}
};
//兑换结果
struct MSG_GP_S_MONEY_TO_JEWEL_RES
{
	int iResult;//兑换结果(0:未知异常；1：兑换成功;2：用户不存在；3：金币不足)
	int iJewels;//购买的钻石
	long long i64Money;//所消耗金币
	MSG_GP_S_MONEY_TO_JEWEL_RES()
	{
		memset(this,0,sizeof(MSG_GP_S_MONEY_TO_JEWEL_RES));
	}
};

//请求总战绩
struct MSG_GP_I_GetTotalRecord
{
	BYTE iType;		//0:VIP战绩 1:普通战绩
	int iUserID;	
	int iClubID;	//俱乐部ID，0为普通战绩
	int iStartCount;
	MSG_GP_I_GetTotalRecord()
	{
		memset(this,0,sizeof(MSG_GP_I_GetTotalRecord));
	}
};

struct MSG_GP_O_TotalRecord
{
	BYTE	iType;		//0:VIP战绩 1:普通战绩
	int		ID;
	int		iClubID;	//俱乐部ID，0为普通战绩
	int		UserNum;
	int		iTotleCount;
	int		iScore[10];
	__int64 CreateTime;
	char    szGameName[32];
	char    szPassWord[10];
	char	szNickName[10][50];
	MSG_GP_O_TotalRecord()
	{
		memset(this,0,sizeof(MSG_GP_O_TotalRecord));
	}
};

//请求单局战绩
struct MSG_GP_I_GetSingleRecord
{
	int ID;
	int iUserID;
	int iStartCount;
	MSG_GP_I_GetSingleRecord()
	{
		memset(this,0,sizeof(MSG_GP_I_GetSingleRecord));
	}
};

struct MSG_GP_O_SingleRecord
{
	bool	bPlayBack;		//是否有回放，FALSE没有，TRUE有
	int		UserNum;
	int		iTotleCount;
	int		iGameID;
	int		iScore[10];
	__int64 EndTime;
	char    szRecordCode[10];
	char    szPassWord[10];
	char	szNickName[10][50];
	MSG_GP_O_SingleRecord()
	{
		memset(this,0,sizeof(MSG_GP_O_SingleRecord));
	}
};

//请求回放地址
struct MSG_GP_I_GetRecordURL
{
	char    szRecordCode[10];
	MSG_GP_I_GetRecordURL()
	{
		memset(this,0,sizeof(MSG_GP_I_GetRecordURL));
	}
};

struct MSG_GP_O_GetRecordURL
{
	char    szRecordCode[10];
	int		iGameID;
	MSG_GP_O_GetRecordURL()
	{
		memset(this,0,sizeof(MSG_GP_O_GetRecordURL));
	}
};

//解散房间
struct MSG_GP_I_DissmissDesk
{
	char    szDeskPass[20];
	MSG_GP_I_DissmissDesk()
	{
		memset(this,0,sizeof(MSG_GP_I_DissmissDesk));
	}
};

//删除房间记录
struct MSG_GP_I_DeleteRecord
{
	char    szDeskPass[20];
	MSG_GP_I_DeleteRecord()
	{
		memset(this,0,sizeof(MSG_GP_I_DeleteRecord));
	}
};

struct MSG_GP_S_ModifUserInfo_PHONE_REQUEST//修改用户个人信息请求
{
	int iUserID;			//用户ID
	char szNickName[50];	//玩家昵称
	bool bySex;				//玩家性别
	MSG_GP_S_ModifUserInfo_PHONE_REQUEST()
	{
		memset(this,0,sizeof(MSG_GP_S_ModifUserInfo_PHONE_REQUEST));
	}
};
struct MSG_GP_S_ModifUserInfo_PHONE_RES//修改用户个人信息结果
{
	bool bSuccess;			//是否修改成功
	int iUserID;			//用户ID
	char szNewNickName[50];	//玩家新昵称
	bool byNewSex;			//玩家新性别
	MSG_GP_S_ModifUserInfo_PHONE_RES()
	{
		memset(this,0,sizeof(MSG_GP_S_ModifUserInfo_PHONE_RES));
	}
};
struct MSG_GP_S_PAYMONEY_FANLI_EX
{
	int iUserID;
	MSG_GP_S_PAYMONEY_FANLI_EX()
	{
		memset(this,0,sizeof(MSG_GP_S_PAYMONEY_FANLI_EX));
	}
};
struct MSG_GP_S_SPEAKER_MONEY//查询喇叭消耗金币
{
	int iUserID; 
	MSG_GP_S_SPEAKER_MONEY()
	{
		memset(this,0,sizeof(MSG_GP_S_SPEAKER_MONEY));
	}
};
struct MSG_GP_S_SPEAKER_MONEY_RES//查询喇叭消耗金币结果
{
	int iUserID; 
	int iMoney;//消耗的金币
	MSG_GP_S_SPEAKER_MONEY_RES()
	{
		memset(this,0,sizeof(MSG_GP_S_SPEAKER_MONEY_RES));
	}
};
struct MSG_GP_S_SPEAKER_SEND
{
	int iUserID; 
	int iMessageLen;//信息长度
	char szMessage[500];
	MSG_GP_S_SPEAKER_SEND()
	{
		memset(this,0,sizeof(MSG_GP_S_SPEAKER_SEND));
	}
};
struct MSG_GP_S_SPEAKER_SEND_RES
{
	int	 iSuccess;//操作结果(0:未知错误;1:发送成功;2:用户不存在3:钱包金币不足)
	int  iMessageLen;//信息长度
	char szNickName[50];//玩家昵称
	char szMessage[500];//信息
	MSG_GP_S_SPEAKER_SEND_RES()
	{
		memset(this,0,sizeof(MSG_GP_S_SPEAKER_SEND_RES));
	}
};

struct TFuncConfig
{
	char name[15];
	bool bShowAndroid;
	bool bShowIOS;
};

struct MSG_GP_S_GET_CONFIG_RES
{
	int	 iCount;			//配置的功能个数
	TFuncConfig _data[20];
	MSG_GP_S_GET_CONFIG_RES()
	{
		memset(this,0,sizeof(MSG_GP_S_GET_CONFIG_RES));
	}
};

struct MSG_GP_ContestApplyList
{
	int		Num;
	int		ContestID[30];
};

struct MSG_GP_UpdateApplyNum
{
	int		iContestID;
	int		iApplyNum;
};


struct MSG_GP_ContestApply
{
	int		iContestID;
	int		iUserID;
	int		iType;			//0报名，1退赛
};


struct MSG_GP_ContestApply_Result
{
	int		iContestID;
	int		iUserID;
	__int64 i64WalletMoney;
	__int64 i64Jewels;
	int     iApplyNum;
	BYTE	bResult;
};


struct MSG_GP_UpdateContestPeople 
{
	int		ContestID[30];
	int		OnlinePeople[30];
};

struct MSG_GP_ContestNotice
{
	int			ContestID;
	int			iRoomID;
	__int64		BeginTime;
	char		szRoomName[50];
};

struct MSG_GP_I_GetContestAward
{
	int		ContestID;
};

struct MSG_GP_O_GetContestAward
{
	int			iAward[10];
	BYTE		bAwardType[10];
};

struct MSG_GP_I_BuyDeskConfig
{
	int		iGameID;
};

struct MSG_GP_DeskBuyCountItem
{
    int		iBuyCount;
    int		iJewels;
    int		iAAJewels;
};

struct MSG_GP_O_BuyDeskConfig
{
    int		iGameID;
    MSG_GP_DeskBuyCountItem buyCounts[5];
    BYTE    szGameDynamicInfo1[128];    // 对应游戏支持的玩法1(具体玩法与游戏对接)
    BYTE	bInvoice;			        // 是否有代开功能，0-无 1-有
    BYTE    bEveryJewelForMinutes;      // 多少分钟一个钻石, 0-代表不支持时效房间
    BYTE    bFinishCondition;           // 0-局数,1-胡息,2-圈数,3-时效
    MSG_GP_O_BuyDeskConfig()
    {
        memset(this,0,sizeof(MSG_GP_O_BuyDeskConfig));
    }
};

struct MSG_GP_O_ChangeConfig
{
	int		iCoinToJewel;		//多少金币兑换1钻石，0为关闭此功能
	int		iJewelToCoin;		//1钻石兑换多少金币，0为关闭此功能
};

struct MSG_GP_I_ChangeRequest
{
	int		iUserID;			//用户ID
	int		iChangeJewels;		//消耗或兑换的钻石数量
};

struct MSG_GP_O_ChangeResponse
{
	int		iUserID;			//用户ID
	int		iJewels;			//玩家拥有钻石数
	__int64	i64Money;			//玩家钱包金币数
};

struct MSG_GP_I_ForgetPWD
{
	BYTE    bUseID;				//找回方式 0-使用用户名，1-使用用户ID
	int		iUserID;			//用户ID
	char	szUserName[64];		//用户名
	char    szPhoneNum[20];		//绑定手机号
	char	szNewPWD[64];       //新密码
};

//用户账号关联手机号
//struct MSG_GP_I_AddUserPhoneNum
//{
//    int     UserID;             //用户ID
//    char    szPhoneNum[20];     //手机号
//};
struct MSG_GP_I_GetBuydeskList
{
	int		iUserID;
	BYTE	bType;				//获取类型，0-开启房间，1-已关闭房间
};

struct MSG_GP_O_GetBuydeskList
{
	BYTE	btype;				//0-请求开房列表，1-请求已关闭房间列表
	int		iUserID;			//用户ID
	bool	bIsPlay;
	__int64	i64BuyTime;
	char	szDeskPass[20];
	char	szGameName[32];
    BYTE    bFinishCondition;   //结束条件，0局数，1胡息，2圈数, 3-时效
    int     iLeaveSeconds;      //剩余时间，单位秒，bFinishCondition为3时有效,bIsPlay为1时才开始计时
};

struct MSG_GP_I_GetDeskUser
{
	char	szDeskPass[20];
};

struct MSG_GP_O_GetDeskUser
{
	BYTE	bSex;
	int		iLogoID;
	int		iUserID;
	char	szUserName[50];
	char	szUserHeadURL[256];
};

// 创建俱乐部
struct MSG_GP_I_CreateClub
{
	char	szClubName[32];
};
struct MSG_GP_O_CreateClub
{
	int		iClubID;
	char	szClubName[32];
};

// 解散俱乐部
struct MSG_GP_I_DissmissClub
{
	int		iClubID;
};

struct MSG_GP_O_DissmissClub
{
	int		iClubID;
};

// 申请加入俱乐部
struct MSG_GP_I_JoinClub
{
	int		iClubID;
};	

// 申请退出俱乐部
struct MSG_GP_I_LeaveClub
{
	int		iClubID;
};	
struct MSG_GP_O_LeaveClub
{
	int		iClubID;
};	

// 俱乐部玩家列表
struct MSG_GP_I_Club_UserList
{
	int		iClubID;
};
struct MSG_GP_O_Club_UserList_Head
{
	int		iClubID;
	int		iUserNum;
	int		iCreaterID;
};
struct MSG_GP_O_Club_UserList_Data
{
	int		iUserID;
	int		iLogoID;
	bool	bSex;
	char	szUserNickName[30];
	char	szHeadURL[256];
};

// 俱乐部聊天
struct MSG_GP_I_Club_Talk
{
	int		iClubID;
	char	szTalk[256];
};
struct MSG_GP_O_Club_Talk
{
	int		iClubID;
	int		iUserID;
	char	szTalk[256];
};

// 俱乐部房间列表		 ASS_GP_CLUB_ROOMLIST = 5,	
struct MSG_GP_I_Club_RoomList
{
	int		iClubID;
};
struct MSG_GP_O_Club_RoomList_Head
{
	int		iClubID;
	int		iRoomNum;
};
struct MSG_GP_O_Club_RoomList_Data
{
	BYTE	bPayType;				//付费方式
	int		iCount;					//创建局数
	int		iMaterID;
	int		iLogoID;
	bool	bSex;
    bool	bIsPlay;
	char	szDeskPass[20];			//桌子密码
	char	szGameName[30];			//游戏名称
	char	szUserNickName[30];		//房主昵称
	char	szHeadURL[256];			//房主头像
    bool    bAllowEnter;            //是否允许加入游戏,0-不允许,1-允许
    BYTE    bFinishCondition;       //结束条件，0局数，1胡息，2圈数, 3-时效
    int     iLeaveSeconds;          //剩余时间，单位秒，bFinishCondition为3时有效,bIsPlay为true时才开始计时
};

// 俱乐部创建房间
struct MSG_GP_I_Club_BuyDesk
{
	int		iUserID;						// 用户ID
	int		iClubID;						// 俱乐部ID
	int		iPlayeCount;					// 购买局数
	int		iGameID;						// 游戏ID
	BYTE	bFinishCondition;				// 结束条件，0局数，1胡息
	BYTE	bPayType;						// 0开局之后扣除房费，1房主开房时付费，2AA制扣除房费
	BYTE	bPositionLimit;					// 0不开启定位，1开启定位
	BYTE	bPlayerNum;						// 游戏限制人数,0与游戏人数相同
	BYTE	bMidEnter;						// 0中途不可以进入，1中途可进入
	BYTE	szDeskConfig[512];
};
struct MSG_GP_O_Club_BuyDesk
{
	int								iUserID;						// 用户ID
	int								iClubID;						// 俱乐部ID
	MSG_GP_O_Club_RoomList_Data		_RoomData;
};

// 俱乐部更名
struct MSG_GP_I_ChangeName
{
	int		iClubID;
	char	szNewClubName[32];
};
struct MSG_GP_O_ChangeName
{
	int		iClubID;
	char	szNewClubName[32];
};

// 会长踢人
struct MSG_GP_I_Club_KickUser
{
	int		iClubID;
	int		iTargetID;
};
struct MSG_GP_O_Club_KickUser
{
	int		iClubID;
	int		iTargetID;
};

// 俱乐部统计信息		 ASS_GP_CLUB_STATISTICS = 9,
// 俱乐部列表
struct MSG_GP_O_Club_List
{
	bool	bCreater;
	bool	bSex;
	int		iClubID;
	int		iMaterID;
	int		iClubUserNum;
	int		iRoomNum;
	int		iLogoID;
	char	szClubName[32];
	char	szHeadURL[256];
};

// 会长审核列表
struct MSG_GP_I_Club_ReviewList
{
	int		iClubID;
};
struct MSG_GP_O_Club_ReviewList_Head
{
	int		iClubID;
	int		iUserNum;
};
struct MSG_GP_O_Club_ReviewList_Data
{
	BYTE	bUserType;				//0-申请名单，1-屏蔽名单
	int		iUserID;
	int		iLogoID;
	bool	bSex;
	char	szUserNickName[30];
	char	szHeadURL[256];
};

// 会长审核操作			同意之后给身请人发送MSG_GP_O_Club_List
struct MSG_GP_I_Club_MasterOpt
{
	int		iClubID;
	int		iTargetID;
	BYTE	bOptType;				//0-同意，1-拒绝，2-移入黑名单，3-移出黑名单
};
struct MSG_GP_O_Club_MasterOpt
{
	int		iClubID;
	int		iTargetID;
	BYTE	bOptType;				//0-同意，1-拒绝，2-移入黑名单，3-移出黑名单
};

// 俱乐部公告
struct MSG_GP_I_Club_Notice
{
	int		iClubID;
	char	szClubNotice[100];
};
struct MSG_GP_O_Club_Notice
{
	int		iClubID;
	char	szClubNotice[100];
};

// 退出/加入俱乐部
struct MSG_GP_O_Club_UserChange
{
	bool							bJoin;					//true-加入，false-退出
	int								iClub;
	MSG_GP_O_Club_UserList_Data		_data;
};	

// 进入俱乐部（开始接受消息）
struct MSG_GP_I_Club_EnterClub
{
	int		iClubID;
};

struct MSG_GP_O_Club_EnterClub
{

	bool	bHaveapplication;
	int		iClubID;
	int		iMasterID;
	char	szClubNotice[100];
};

// 俱乐部创建/解散房间
struct MSG_GP_O_Club_RoomChange
{	
	bool							bCreate;						// true-创建，false-解散
	int								iClub;
	MSG_GP_O_Club_RoomList_Data		_data;
};

// 申请加入俱乐部通知创建者
struct MSG_GP_O_Club_UserJoin
{
	bool							bJoin;					//true-加入，false-退出
	int								iClub;
	MSG_GP_O_Club_UserList_Data		_data;
};	

// 会长获取开房信息
struct MSG_GP_I_Club_BuyDeskRecord
{
	int			iClubID;
	__int64		i64StartTime;
	__int64		i64EndTime;
};

struct MSG_GP_O_Club_BuyDeskRecord_Head
{
	int		iClubID;
	int		iDataNum;
};
struct MSG_GP_O_Club_BuyDeskRecord_Data
{
	int		iUserID;						// 用户ID
	int		iCostJewels;
	__int64	i64BuyTime;
	char	szDeskPass[20];
	char	szUserNickName[30];		//房主昵称
};

// 获取邮件列表
struct MSG_GP_O_Mail_List_Head
{	
	int		iDataNum;				//邮件总数
};

struct MSG_GP_O_Mail_List_Data
{			
	int		iMailID;
	char	szMailTitle[25];
	char	szSendName[25];
	__int64	i64MailTime;
	BYTE	bMailState;				//0无附件未读，1无附件已读，2有附件未读，3有附件已读未领取，4有附件已读已领取
	BYTE	bKeepTime;				//保存天数
};

// 打开邮件
struct MSG_GP_I_Open_Mail
{
	int		iMailID;
};

struct MSG_GP_O_Open_Mail
{
	int		iMailID;
	BYTE	bMailState;
	int		iJewels;
	int		iLotteries;
	__int64	i64Money;
	char	szMailMessage[650];
};

// 获取附件
struct MSG_GP_I_Get_Attachment
{
	int		iMailID;
};

struct MSG_GP_O_Get_Attachment
{
	int		iMailID;
	int		iJewels;
	int		iLotteries;
	__int64	i64Money;
	BYTE	bMailState;
};

// 删除邮件
struct MSG_GP_I_Delete_Mail
{
	int		iMailID;
};

struct MSG_GP_O_Delete_Mail
{
	int		iMailID;
	BYTE	bMailState;
};

//更新邮件
struct MSG_GP_O_Mail_Update
{
	int									iUserID;
	bool								bNew;
	MSG_GP_O_Mail_List_Data				MailData;
};

//系统公告
struct MSG_GP_O_SysMsg
{
	char	szSysMessage[150];
};

struct MSG_GP_I_LuckDraw_Config
{
    int     iUserID;
};

struct MSG_GP_O_LuckDraw_Config
{
    BYTE    iFreeTotalCount; // 总免费次数
    BYTE    iLeaveFreeCount; // 剩余免费抽奖次数
    int     i64Money;        // 金币抽奖消耗
    BYTE    type[10];        // 类别 0-金币,1-房卡,2-奖卷
    int     value[10];       // 值
};

struct MSG_GP_I_LuckDraw_DO
{
    int     iUserID;
};

struct MSG_GP_O_LuckDraw_DO
{
    int     index;
};

struct MSG_GP_I_GetSPRoomInfo
{
    int     iRoomID;
};

struct MSG_GP_O_GetSPRoomInfo
{
    int     iRoomID;
    BYTE    bRoomConfig[128];
};

#pragma pack()
/*----------------------------------------------------------------------------------------*/
//	通信标识定义 
/*----------------------------------------------------------------------------------------*/
//主ID定义
enum E_HALL_MAINID
{
	MDM_GP_CONTEST = 51,					// 比赛相关功能
	MDM_GP_REGISTER	= 99,				    // 玩家注册
	MDM_GP_LOGON = 100,						// 大厅登陆
	MDM_GP_LIST	= 101,						// 游戏列表
	MDM_GP_REQURE_GAME_PARA	= 102,			// 请求游戏全局参数
	MDM_GP_MESSAGE = 103,					// 系统消息
	MDM_GP_SYSMSG = 104,					// 系统消息
	MDM_GP_USER_OPT	= 105,					// 大厅增加好友
	MDM_GP_LOCK_ACCOUNT	= 106,              // 锁定机器
	MDM_GP_MONEY = 107,                     // 银行转账提示
	MDM_GP_MONEY_CHANGE = 108,              // 金币兑换功能
	MDM_GP_CHARMEXCHANGE = 109,             // 魅力兑换
	MDM_GP_NETSIGNAL = 110,                 // 网络信号消息	
	MDM_GP_NOTICE_MSG = 111,				// 提示消息
	MDM_GP_BIND_MOBILE = 112,               // 绑定手机
	MDM_GP_SET_LOGO	= 113,                  // 设置头像
	MDM_GP_USERINFO	= 115,                  // 修改用户信息
	MDM_GP_MAIL = 116,                      // 邮件系统
	MDM_GP_SMS = 117,                       // 手机短信验证s
	MDM_GP_LASTLOINGGAME = 118,             // 获取我的游戏列表
	MDM_GP_LOGONUSERS = 119,                // 统计登陆人数
	MDM_GR_USER_LOGO = 120,					// 头像相关
	MDM_GP_BANK = 121,                      // 大厅银行系统
	MDM_ZD_PACKAGE = 123,                   // 与中心服务器通讯
	MDM_GP_IM = 130,                        // 好友系统
	MDM_GP_PAIHANGBANG = 133,               // 排行榜
	MDM_GP_SIGN = 134,                      // 签到
	MDM_GP_ONLINE_AWARD = 135,              // 手游在线奖励
	MDM_GP_MODIFUSERINFO_PHONE = 136,       // 移动端修改个人数据
	MDM_GP_SPEAKER = 137,                   // 手游端喇叭
	MDM_GP_PLAYER_2_GM = 139,				// 普通玩家与GM之间的通信协议
	MDM_GP_PROP = 140,					    // 大厅中道具相关消息
	MDM_GP_DESK_VIP = 141,					// VIP桌子
	MDM_GP_GET_CONFIG = 142,				// 获取配置信息
	MDM_GP_CLUB = 143,						// 俱乐部
    MDM_GP_LUCKDRAW = 144,                  // 转盘抽奖
	//MDM_GAMEMASTER_MESSAGE=255,   	    // GM消息主ID
};


/********************************************************************************************/

enum HALL_ASSID_CONTEST							// MDM_GP_CONTEST
{
	ASS_GP_GET_CONTEST_ROOMID = 0,				// 获取比赛房间ID
	ERR_GP_GET_CONTEST_ROOMID = 1,				// 比赛房间，用户双击列表时向服务器索要房间号
	ASS_GP_GET_TIMINGMATCH_BEGIN_TIME = 2,      // 获取定时赛比赛开始时间
	ASS_GP_UPDATE_CONTESTPEOPLE = 3,			// 刷新比赛列表人数
	ASS_GP_GET_APPLY_NUM = 4,					// 获取已报名人数
	ASS_GP_CONTEST_APPLY = 5,					// 大厅报名
	ASS_GP_CONTEST_NOTICE = 6,					// 赛前通知
	ASS_GP_CONTEST_AWARDINFO = 7,				// 获取比赛奖励信息
};

enum HALL_ASSID_REGISTER						// MDM_GP_REGISTER
{
	ASS_GP_REGISTER = 1,						
};

enum E_HALL_ERROR_REGISTER						// MDM_GP_REGISTER
{
	ERR_REGISTER_ERROR = 0,						 
	ERR_REGISTER_SUCCESS = 1,					
	ERR_REGISTER_NAME_EXIST = 2,		
	ERR_REGISTER_NAME_LIMITE = 3,	
};

enum HALL_ASSID_LOGON							// MDM_GP_LOGON
{
	ASS_GP_LOGON_BY_NAME = 1,					// 通过用户名字登陆
	ASS_GP_LOGON_BY_ACC = 2,					// 通过用户ACC 登陆
	ASS_GP_LOGON_BY_MOBILE = 3,					// 通过用户手机登陆
	ASS_GP_LOGON_REG = 4,						// 用户注册
	ASS_GP_LOGON_SUCCESS = 5,					// 登陆成功
	ASS_GP_LOGON_ERROR = 6,						// 登陆失败
	ASS_GP_LOGON_ALLO_PART = 7,					// 异地登陆
	ASS_GP_LOGON_LOCK_VALID = 8,				// 锁机验证										
	ASS_GP_LOGON_BY_SOFTWARE = 10,				// 如果是第三方启动，发此消息通知服务端
	ASS_GP_LOGON_MOBILE_VALID = 11,				// 手机验证
};

enum HALL_ASSID_LIST							// MDM_GP_LIST
{
	ASS_GP_LIST_KIND = 1,						// 获取游戏类型列表
	ASS_GP_LIST_NAME = 2,						// 获取游戏名字列表
	ASS_GP_LIST_ROOM = 3,						// 获取游戏房间列表
	ASS_GP_LIST_COUNT = 4,						// 获取游戏人数列表
	ASS_GP_ROOM_LIST_PEOCOUNT = 5,				// 获取游戏人数列表
	ASS_GP_ROOM_PASSWORD = 6,					// 发送房间密码，试图进入密码房间时发送此消息
	ASS_GP_LIST_CONTEST = 8,					// 获取比赛列表
    ASS_GP_GET_SPROOMINFO=9,                    // 获取房间设置信息
};

enum HALL_ASSID_MESSAGE							// MDM_GP_MESSAGE
{
	ASS_GP_NEWS_SYSMSG = 1,						// 新闻和系统消息
	ASS_GP_DUDU = 2,							// 小喇叭
	ASS_GP_TALK_MSG = 3,						// 聊天消息
};

enum HALL_ASSID_USER_OPT						// MDM_GP_USER_OPT
{
	ASS_UPDATE_FRIEND = 1,						// 更新好友
	ASS_UPDATE_EMENY = 2,						// 更新坏人
};

enum HALL_ASSID_LOCK_ACCOUNT					// MDM_GP_LOCK_ACCOUNT
{
	ASS_LOCK_ACCOUNT = 1,						
};

enum HALL_ASSID_CHARMEXCHANGE					// MDM_GP_CHARMEXCHANGE
{
	ASS_GETLIST = 1,							 
	ASS_EXCHANGE = 2,							
};

enum HALL_ASSID_BIND_MOBILE						// MDM_GP_BIND_MOBILE
{
	ASS_GP_BIND_MOBILE = 1,						
};

enum HALL_ASSID_SET_LOGO						// MDM_GP_SET_LOGO
{
	ASS_GP_SET_LOGO = 1,						
};

enum HALL_ASSID_USERINFO						// MDM_GP_USERINFO
{
	ASS_GP_USERINFO_UPDATE_BASE = 1,			// 用户更新基本信息
	ASS_GP_USERINFO_UPDATE_DETAIL = 2,			// 用户更新详细信息
	ASS_GP_USERINFO_UPDATE_PWD = 3,				// 用户修改密码
	ASS_GP_USERINFO_FORGET_PWD = 4,				// 忘记密码
	ASS_GP_USERINFO_ACCEPT = 5,					// 服务端已接受
	ASS_GP_USERINFO_NOTACCEPT = 6,				// 服务端未能接受
    //ASS_GP_USERINFO_ADDPHONENUM = 7,            // 关联账号
	ASS_GP_USERINFO_NICKNAMEID = 10,			// 根据玩家昵称找ID或ID找昵称
};

enum HALL_ASSID_SMS								// MDM_GP_SMS
{
	ASS_GP_SMS_VCODE = 1,						// 手机验证码短信
};

enum HALL_ASSID_LASTLOINGGAME					// MDM_GP_LASTLOINGGAME
{
	ASS_GP_LASTLOINGGAME = 1,					
};

enum HALL_ASSID_LOGONUSERS						// MDM_GP_LOGONUSERS
{
	ASS_GP_LOGONUSERS_COUNT = 1,				
};

enum HALL_ASSID_ZD_PACKAGE						// MDM_ZD_PACKAGE
{
	ASS_ZD_CONNECT = 1,							 
	ASS_ZD_LOGON = 2,							 
	ASS_ZD_BATCH = 3,							 
	ASS_ZD_SINGLE = 4,							 
};

enum E_HALL_ERROR_ZD_LOGON						// ASS_ZD_LOGON
{
	HDC_ZD_CONNECT = 1,
	HDC_ZD_DISCONNECT = 2,
	HDC_ZD_KICK = 3,
};

enum E_HALL_ERROR_ZD_SINGLE						// ASS_ZD_SINGLE
{
	HDC_ZD_FAIL = 2,
};

enum HALL_ASSID_IM								// MDM_GP_IM
{
	ASS_IMC_GETCOUNT			 = 0x01,		// 取得服务器上好友数，陌生人中的不取，需要将本地的数量发给服务器
	ASS_IMC_GETUSERINFO			 = 0x02,		// 取得某个好友的详细资料
	ASS_IMC_ADDREQUEST			 = 0x03,		// 请求加为好友，含有是否是第一次发出，这在对方需要确认时有用
	ASS_IMC_ADDREQUESTRESPONSE	 = 0x04,		// 有人请求加为好友，返回是否同意
	ASS_IMC_SEARCH				 = 0x05,		// 查询好友，分为在线与不在线（通过ID号或姓名），保留
	ASS_IMC_MSG					 = 0x0F,		// 聊天消息
	ASS_IMC_SETGROUP			 = 0x06,		// 设置好友组
	ASS_IMS_GETCOUNT			 = 0x11,		// 返回好友数，不取黑名单
	ASS_IMS_GETUSERINFO			 = 0x12,		// 返回好友的详细资料,最少要以下内容
	ASS_IMS_GETLIST				 = 0x13,		// 返回好友列表，包括：ID号，姓名，性别，是否在线
	ASS_IMS_USERONLINE			 = 0x14,		// 上下线后，通知好友
	ASS_IMS_GETONLINELIST		 = 0x15,		// 上线后，返回在线的好友，如果已取得完整列表，则不再返回此消息
	ASS_IMS_ADDREQUEST			 = 0x16,		// 有人请求加为好友，需要请确认
	ASS_IMS_ADDREQUESTRESPONSE	 = 0x17,		// 转发是否同意加为好友，如果是同意的，则服务器还要操作记录
	ASS_IMS_USERNOTONLINE		 = 0x1E,		// 转发聊天消息失败，用户不在线
	ASS_IMS_MSG					 = 0x1F,		// 转发聊天消息
	ASS_IMS_NOAC				 = 0x23,		// 转发聊天消息
	ASS_SET_FRIENDTYPE			 = 0x20,		// 设置个人好友和聊天的接受类型
};

enum HALL_ASSID_PAIHANGBANG						// MDM_GP_PAIHANGBANG
{
	ASS_GP_PAIHANGBANG_EX = 1,					// 排行榜升级
};

enum HALL_ASSID_SIGN							// MDM_GP_SIGN
{
	ASS_GP_SIGN_CHECK = 1,					
	ASS_GP_SIGN_DO = 2,
	ASS_GP_SIGN_CHECK_EX =3,
};

enum HALL_ASSID_ONLINE_AWARD					// MDM_GP_ONLINE_AWARD
{
	ASS_GP_ONLINE_AWARD_CHECK = 1,
	ASS_GP_ONLINE_AWARD_DO = 2,
	ASS_GP_ONLINE_AWARD_CHECK_EX = 3,
	ASS_GP_ONLINE_AWARD_DO_EX = 4,
};

enum HALL_ASSID_MODIFUSERINFO_PHONE				// MDM_GP_MODIFUSERINFO_PHONE
{
	ASS_GP_MODIFUSERINFO_PHONE = 0,				
};

enum HALL_ASSID_SPEAKER							// MDM_GP_SPEAKER
{
	ASS_GP_SPEAKER_REQUEST = 0,					// 查询
	ASS_GP_SPEAKER_RESULT = 1,					// 查询结果
	ASS_GP_SPEAKER_SEND = 2,					// 广播消息
	ASS_GP_SPEAKER_SEND_RESULT = 3,				// 广播消息效果
};

enum HALL_ASSID_PLAYER_2_GM						// MDM_GP_PLAYER_2_GM
{
	ASS_GP_GET_GM_LIST = 1,						// 获取当前在线GM列表
	ASS_GP_TALK_2_GM = 2,						// 向指定编号的GM发送消息
	ASS_GP_MSG_FROM_GM = 3,						// 接收从GM处发来的消息
	ASS_GR_MSG_BE_KICKED = 4,					// 被GM踢出房间
	ASS_GR_MSG_BE_BANDID = 5,					// 被GM封号
	ASS_GR_MSG_BE_AWARD = 6,					// 被GM奖励
	ASS_GR_MSG_PRINTLOG = 7,					// GM打印日志
};

enum HALL_ASSID_DESK_VIP						// MDM_GP_DESK_VIP
{
	ASS_GP_BUY_DESK	= 0,						// 购买桌子
	ASS_GP_ENTER_DESK = 1,						// 进入vip桌子
	ASS_GP_GETTOTALRECORD = 3,					// 获取总战绩
	ASS_GP_GETSINGLERECORD = 4,					// 获取单局战绩
	ASS_GP_GET_CUTROOM = 5,						// 获取断线信息
	ASS_GP_GET_DESKCONFIG = 6,					// 获取局数钻石配置信息
	ASS_GP_GET_RECORDURL = 7,					// 获取回放文件地址
	ASS_GP_GET_DESKLIST = 8,					// 获取桌子列表
	ASS_GP_GET_DESKUSER = 9,					// 获取桌子玩家信息
	ASS_GP_DISSMISSDESK = 10,					// 解散房间
	ASS_GP_DELETE_RECORD = 11,					// 删除桌子信息
};
enum HALL_ASSID_GET_CONFIG						// MDM_GP_GET_CONFIG
{
	ASS_GP_GET_CONFIG = 0,						// 手游获取大厅功能配置
};

enum HALL_ASSID_MONEY_CHANGE					// MDM_GP_MONEY_CHANGE
{
	ASS_GP_CHANGE_CONFIG = 0,					// 获取金币钻石兑换比例
	ASS_GP_COIN2JEWEL = 1,						// 金币兑换钻石
	ASS_GP_JEWEL2COIN = 2,						// 钻石兑换金币
};


enum HALL_ASSID_CLUB							// MDM_GP_CLUB
{
	ASS_GP_CREATE_CLUB = 0,						// 创建俱乐部
	ASS_GP_DISSMISS_CLUB = 1,					// 解散俱乐部
	ASS_GP_JOIN_CLUB = 2,						// 申请加入俱乐部
	ASS_GP_CLUB_USERLIST = 3,					// 俱乐部玩家列表
	ASS_GP_CLUB_TALK = 4,						// 俱乐部聊天
	ASS_GP_CLUB_ROOMLIST = 5,					// 俱乐部房间列表
	ASS_GP_CLUB_CREATEROOM = 6,					// 俱乐部创建房间
	ASS_GP_CLUB_CHANGENAME = 7,					// 俱乐部更名
	ASS_GP_CLUB_KICKUSER = 8,					// 会长踢人
	ASS_GP_CLUB_STATISTICS = 9,					// 俱乐部统计信息
	ASS_GP_CLUB_LIST = 10,						// 俱乐部列表
	ASS_GP_REVIEW_LIST = 11,					// 会长审核列表
	ASS_GP_MASTER_OPTION = 12,					// 会长审核操作
	ASS_GP_CLUB_NOTICE = 13,					// 会长更改俱乐部公告
	ASS_GP_CLUB_USERCHANGE = 14,				// 退出/加入俱乐部
	ASS_GP_ENTER_CLUB = 15,						// 进入俱乐部（开始接受消息）
	ASS_GP_CLUB_ROOMCHANGE= 16,					// 俱乐部创建/解散房间
	ASS_GP_CLUB_JOINRESULT= 17,					// 俱乐部进入成功/失败，成功返回俱乐部信息，失败返回空
	ASS_GP_JOIN_CLUB_TOMASTER = 18,				// 申请加入俱乐部通知创建者
	ASS_GP_LEAVE_CLUB = 19,						// 申请离开俱乐部
	ASS_GP_CLUB_KICKUSER_TAR = 20,				// 会长踢人通知被踢者
	ASS_GP_CLUB_NOTICE_UPDATE = 21,				// 更改俱乐部公告后更新消息
	ASS_GP_CLUB_NAME_UPDATE = 22,				// 更改俱乐部名字后更新消息
	ASS_GP_DISSMISS_CLUB_NOTIFY = 23,			// 解散俱乐部后更新消息
	ASS_GP_GET_BUYDESKRECORD = 24,				// 会长查询开房记录
};

enum E_HALL_ERROR_CLUB							// MDM_GP_CLUB
{
	ERR_GP_CLUB_REQUEST_SUCCESS = 0,			// 操作成功
	ERR_GP_USER_NOT_FOUND = 1,					// 用户信息错误
	ERR_GP_NAME_LIMITE = 2,						// 俱乐部名称包含屏蔽字
};

enum HALL_ASSID_MAIL							// MDM_GP_MAIL
{
	ASS_GP_MAIL_LIST = 0,						// 获取邮件列表
	ASS_GP_OPEN_MAIL = 1,						// 打开邮件
	ASS_GP_GET_ATTACHMENT = 2,					// 获取附件
	ASS_GP_DEL_MAIL = 3,						// 删除邮件
	ASS_GP_NOTIFY_NEW_MAIL = 4,					// 提示新邮件
	ASS_GP_GET_SYSMSG = 5,						// 
};

enum HALL_ASSID_LUCK_DRAW
{
    ASS_GP_LUCK_DRAW_CONFIG = 0,                 // 获取抽奖信息
    ASS_GP_LUCK_DRAW_DO = 1                      // 抽奖操作
};

/*
enum E_HALL_ERROR_CLUB_CREATE					// ASS_GP_CREATE_CLUB
{
	ERR_GP_USER_NOT_FOUND = 1,					// 用户信息错误
};*/

/********************************************************************************************/

// 错误代码
enum E_HALL_ERROR_CODE					
{
	ERR_GP_ERROR_UNKNOW = 0,					// 未知错误
	ERR_GP_LOGON_SUCCESS = 1,					// 登陆成功
	ERR_GP_USER_NO_FIND = 2	,					// 登陆名字错误
	ERR_GP_USER_PASS_ERROR = 3,					// 用户密码错误
	ERR_GP_USER_VALIDATA = 4,					// 用户帐号禁用
	ERR_GP_USER_IP_LIMITED = 5,					// 登陆 IP 禁止
	ERR_GP_USER_EXIST = 6,						// 用户已经存在
	ERR_GP_PASS_LIMITED = 7,					// 密码禁止效验
	ERR_GP_IP_NO_ORDER = 8,						// 不是指定地址 
	ERR_GP_LIST_PART = 9,						// 部分游戏列表
	ERR_GP_LIST_FINISH = 10,					// 全部游戏列表
	ERR_GP_USER_LOGON = 11,						// 此帐号已经登录
	ERR_GP_USERNICK_EXIST = 12,					// 此昵称已经存在
	ERR_GP_USER_BAD = 13,						// 未法字符
	ERR_GP_IP_FULL = 14,						// IP已满
	ERR_GP_LOCK_SUCCESS = 15,					// 锁定机器成功
	ERR_GP_ACCOUNT_HAS_LOCK = 16,				// 机器已经处于锁定状态
	ERR_GP_UNLOCK_SUCCESS = 17,					// 解除锁定成功
	ERR_GP_NO_LOCK = 18,						// 机器根本就没有锁定，所以解锁失败
	ERR_GP_CODE_DISMATCH = 19,					// 机器码不匹配，解锁失败。
	ERR_GP_ACCOUNT_LOCKED = 20,					// 本账号锁定了某台机器，登录失败
	ERR_GP_MATHINE_LOCKED = 21,
	ERR_GP_MOBILE_VALID	= 22,					// 需要短信验证
	ERR_GP_VISITER_LOCKED = 23,					// 游客登录锁定机器
	ERR_GP_LOGONTYPE_ERROR = 24,				// 登录类型错误
	ERR_GP_USER_NOT_EXIST = 30,					// 用户不存在
	ERR_GP_USER_OVERDATE = 31,					// 用户已过期
	ERR_GP_DATABASE_FUNC_ERROR = 32,			// 数据库操作失败
	ERR_GP_DATABASE_RETURN_ERROR = 33,			// 数据库返回执行错误结果
	ERR_GP_ALLO_PARTY = 50,						// 本账号异地登陆
};

enum E_HALL_ERROR_DISSMISSDESK					// 解散房间
{
	ERR_GP_DISSMISSDESK_SUCCESS	 = 0,			// 解散房间成功
	ERR_GP_NOT_FIND_USERR_CLR = 1,				// 没有查到这个用户
	ERR_GP_NOT_DESK_MASTER = 2,					// 用户不是房主
	ERR_GP_DESKPASS_ERROR = 3,					// 桌子号错误
	ERR_GP_DESK_INGAME = 4,						// 桌子正在游戏中
};

enum E_HALL_ERROR_DELETE_RECORD					// 删除桌子信息
{
	ERR_GP_DELETE_RECORD_SUCCESS = 0,			// 删除桌子信息成功
	ERR_GP_NOT_FIND_USERR_DEL = 1,				// 没有查到这个用户
	ERR_GP_NOT_DESK_MASTER_DEL = 2,				// 用户不是房主
	ERR_GP_DESKPASS_ERROR_DEL = 3,				// 桌子号错误
};


static CString ERR_RP_STRING[21] = 
{
	"未知错误", "登录成功", "登录用户名错误", "用户密码错误", "用户帐号被禁", 
	"登陆IP被禁", "用户已经存在", "密码禁止效验", "不是指定地址", "部分游戏列表", 
	"全部游戏列表", "此帐号已经登录", "此昵称已经存在", "内含有非法字符", "同一IP每天只能注册20个帐号", 
	"锁定机器成功", "机器已经处于锁定状态", "解除锁定成功", "机器没有锁定，解锁失败", 
	"机器码不匹配，解锁失败", "本账号锁定了某台机器，登录失败"
};

/********************************************************************************************/



#include"CenterServerhead.h"


///

#endif
