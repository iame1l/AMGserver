/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#ifndef GAME_LOGON_DT_HEAD_FILE
#define GAME_LOGON_DT_HEAD_FILE

#include "MainManage.h"
#include "GameRoomMessage.h"
#include "HN_GP_database_io.h"
#include "rminfo.h"
/********************************************************************************************/

//	数据库通信数据包定义 

/********************************************************************************************/

// 用户注册信息（接收到的）
struct DL_GP_I_Register
{
	DataBaseLineHead				DataBaseHead;						//数据包头
	MSG_GP_S_Register				_data;
};

// 用户注册信息（服务端获取的）
struct DL_GP_O_Register
{
	DataBaseResultLine				ResultHead;							//结果数据包头
	MSG_GP_S_Register				_data;
};



//通过用户支付的RMB来领取金币奖励
struct DL_GP_I_GETMONEY_BY_PAY
{
	DataBaseLineHead				DataBaseHead;
	unsigned long					dwUserID;
};

//登陆服务器登陆信息
struct DL_GP_I_LogonByNameStruct////////按名登陆信息的结构
{
	DataBaseLineHead					DataBaseHead;						//数据包头
	MSG_GP_S_LogonByNameStruct			LogonInfo;							//登陆信息，又封状了一个结构
	char								szAccessIP[16];						//登陆 IP
	int									gsqPs;
	///////////////////////////
	//// 添加防止账户同时登陆
	int ID;//ZID
	///////////////////////////

};

//锁定机器 zxj 2009-11-13
struct DL_GP_LockAccount
{
	DataBaseLineHead				DataBaseHead;
	MSG_GP_S_LockAccount			LockAccount;
};

///绑定手机号码
struct DL_GP_BindMobile
{
	DataBaseLineHead				DataBaseHead;
	MSG_GP_S_BindMobile				data;
};

//设置用户头像
struct DL_GP_SetLogo
{
	DataBaseLineHead				DataBaseHead;
	MSG_GP_S_SET_LOGO				LogoInfo;
};

//登陆服务器登陆信息
struct DL_GP_I_LogonByAccStruct
{
	DataBaseLineHead					DataBaseHead;						//数据包头
	MSG_GP_S_LogonByAccStruct			LogonInfo;							//登陆信息
	char								szAccessIP[16];						//登陆 IP
};

//登陆服务器登陆信息
struct DL_GP_I_LogonByMobileStruct
{
	DataBaseLineHead					DataBaseHead;						//数据包头
	MSG_GP_S_LogonByMobileStruct		LogonInfo;							//登陆信息
	char								szAccessIP[16];						//登陆 IP
};

//登陆服务器登陆信息
struct DL_GP_I_RegisterStruct
{
	DataBaseLineHead					DataBaseHead;						//数据包头
	MSG_GP_S_RegisterStruct				RegisterInfo;						//注册信息
	char								szAccessIP[16];						//登陆 IP
};
//玩家操作
struct GP_User_Opt_Struct
{
	DataBaseLineHead					DataBaseHead;						//数据包头
	MSG_GP_User_Opt_Struct				stUserOpt;							//玩家操作结构
};
//更新游戏列表请求
struct DL_GP_I_UpdateDataListStruct
{
	DataBaseLineHead					DataBaseHead;						//数据包头
	void								* pGameList;						//列表指针	
};

//登陆服务器登陆信息
struct DL_GP_O_LogonResultStruct
{
	DataBaseResultLine					ResultHead;							//结果数据包头
	MSG_GP_R_LogonResult				LogonResult;						//登陆结果

};

//比赛报名信息
struct DL_GP_I_ContestListStruct
{
	DataBaseLineHead					DataBaseHead;						
	int									iUserID;							
};

struct DL_GP_O_ContestListStruct
{
	DataBaseResultLine					ResultHead;							
	MSG_GP_ContestApplyList				_data;
};

//修改写Z登录记录的时机，确保网络卡的时候，不至于导致用户登录不上
struct  DL_GP_WriteZRecord
{
	DataBaseLineHead		DataBaseHead;
	long	int				lUserID;
	int						ZID;
	int						iSocketID;
};

//锁定机器结果 zxj 2009-11-12
struct MSG_GP_S_LockAccountResult
{
	DataBaseResultLine					ResultHead;							//结果数据包头
	UINT	dwUserID;
	UINT	dwCommanType;			//请求命令类型
	UINT	dwCommanResult;			//请求的结果
};

//绑定手机结果
struct MSG_GP_S_BindMobileResult
{
	DataBaseResultLine					ResultHead;							//结果数据包头
	UINT	dwUserID;
	UINT	dwCommanType;			//请求命令类型
	UINT	dwCommanResult;			//请求的结果
};

//密码房间 add by lxl 2010-10-08
struct DL_GP_I_EnterCrypticRoom
{
	DataBaseLineHead					DataBaseHead;						//数据包头
	UINT                                uRoomID;						// 房间号
	char								szMD5PassWord[50];				// 房间密码
};
struct DL_GP_O_EnterCrypticRoom
{
	DataBaseResultLine					ResultHead;						//数据包头
	int                                nRet;						// 返回值，0表示成功，-1表示失败
	UINT                               uRoomID;						// 房间号
};

struct DL_GP_O_GetContestRoomID
{
	DataBaseResultLine					ResultHead;
	MSG_GP_GetContestRoomID_Result		_data;
	int									iRoomID;
	int									iNotifyG;
	int									iMatchID;
	COleDateTime						BeginTime;
};

struct DL_GP_I_GetContestRoomID
{
	DataBaseLineHead					DataBaseHead;
	MSG_GP_GetContestRoomID				_data;
};

//获取比赛报名人数
struct DL_GP_I_UpdateApplyNum
{
	DataBaseLineHead					DataBaseHead;						
	int                                 iContestID;					
	DL_GP_I_UpdateApplyNum()
	{
		memset(this,0,sizeof(DL_GP_I_UpdateApplyNum));
	}
};
struct DL_GP_O_UpdateApplyNum
{
	DataBaseResultLine					ResultHead;					
	MSG_GP_UpdateApplyNum				_data;
	DL_GP_O_UpdateApplyNum()
	{
		memset(this,0,sizeof(DL_GP_O_UpdateApplyNum));
	}
};


//比赛报名
struct DL_GP_I_ContestApply
{
	DataBaseLineHead					DataBaseHead;						
	MSG_GP_ContestApply					_data;
	DL_GP_I_ContestApply()
	{
		memset(this,0,sizeof(DL_GP_I_ContestApply));
	}
};
struct DL_GP_O_ContestApply
{
	DataBaseResultLine					ResultHead;					
	MSG_GP_ContestApply_Result			_data;
	MSG_MG_S_CONTEST_ACTIVE				_Notify;
	DL_GP_O_ContestApply()
	{
		memset(this,0,sizeof(DL_GP_O_ContestApply));
	}
};

//获取比赛奖励
struct DL_GP_I_ContestAward
{
	DataBaseLineHead					DataBaseHead;						
	MSG_GP_I_GetContestAward			_data;
	DL_GP_I_ContestAward()
	{
		memset(this,0,sizeof(DL_GP_I_ContestAward));
	}
};
struct DL_GP_O_ContestAward
{
	DataBaseResultLine					ResultHead;					
	MSG_GP_O_GetContestAward			_data;
	DL_GP_O_ContestAward()
	{
		memset(this,0,sizeof(DL_GP_O_ContestAward));
	}
};

//定时赛比赛开始时间
struct DL_GP_I_GetContestBeginTime
{
	DataBaseLineHead					DataBaseHead;
	UINT								iGameID;		//游戏ID
	int									iContestID;		//比赛ID	
};
struct DL_GP_O_GetContestBeginTime
{
	DataBaseResultLine					ResultHead;
	UINT								iGameID;		//游戏ID
	int									iContestID;		//比赛ID	
	COleDateTime						matchBeginTime;	//比赛开始时间
};

//用户资料修改
struct DL_GP_I_UserInfoStruct
{
	DataBaseLineHead	DataBaseHead;	//数据包头
	MSG_GP_S_UserInfo   UserInfo;		//用户资料数据
};

struct DL_GR_O_UserInfoStruct
{
	DataBaseResultLine	ResultHead;		//结果数据包头
};

struct DL_GP_I_ChPasswordStruct
{
	DataBaseLineHead	DataBaseHead;	//数据包头
	MSG_GP_S_ChPassword ChPwd;		//用户密码的数据
};

struct DL_GP_O_ChPasswordStruct
{
	DataBaseResultLine	ResultHead;	//数据包头
};

/// 检查版本
struct DL_GR_I_CheckVersion_t
{
	DataBaseLineHead					DataBaseHead;						///<数据包头
	UINT								uType;								///<操作类型
};
struct DL_GP_I_ReceiveMoney
{
	DataBaseLineHead					DataBaseHead;						///<数据包头
	int									iUserID;
};

struct DL_GP_O_ReceiveMoney
{
	DataBaseResultLine					ResultHead;						///<数据包头
	TReceiveMoney						date;
};

struct DL_GP_O_CharmExchangeList
{
	DataBaseResultLine					ResultHead;						///<数据包头
	TCharmExchange_Item					date;
};

//struct DL_GP_O_AddUserPhoneNum
//{
//    DataBaseResultLine					ResultHead;						///<数据包头
//};

struct DL_GP_I_CharmExchange
{
	DataBaseLineHead					DataBaseHead;						///<数据包头
	TCharmExchange						date;
};


struct  DL_GP_I_SetFriendType
{
	DataBaseLineHead					DataBaseHead;						///<数据包头
	int									iUserID;
	int									iType;
};

struct DL_GP_O_GetLogonPeopleCount
{
	DataBaseResultLine					ResultHead;							///<数据包头
	ONLINEUSERSCOUNT					logonUsers;
};
struct DL_GP_O_MailGet
{
	DataBaseResultLine					ResultHead;						///<数据包头
	TMailItem							date;
};

struct DL_GP_I_MailUpdate
{
	DataBaseLineHead					DataBaseHead;						///<数据包头
	int									iMsgID;
};

struct DL_GP_I_GetNickNameID
{
	DataBaseLineHead					DataBaseHead;						///<数据包头
	MSG_GP_NickNameID_t					_data;
};

struct DL_GP_O_GetNickNameID
{
	DataBaseResultLine					ResultHead;							///<数据包头
	MSG_GP_NickNameID_t					_data;
};

// 请求得到财务数据
struct DL_GP_I_FinanceInfo
{
	DataBaseLineHead		 DataBaseHead;		//数据包头
	TMSG_GP_BankFinanceInfo  _data;				//财务数据
};

struct DL_GP_O_FinanceInfo
{
	DataBaseResultLine		 ResultHead;		//数据包头
	TMSG_GP_BankFinanceInfo  _data;				//财务数据
};

struct DL_GP_I_LastLoginGame
{
	DataBaseLineHead		DataBaseHead;		//数据包头
	int						iUserID;
};

struct DL_GP_O_LastLoginGame
{
	DataBaseResultLine		ResultHead;		//数据包头
	TLoginGameInfo			_data;
};


struct DL_GP_I_GetSPRoomInfo
{
    DataBaseLineHead		DataBaseHead;		//数据包头
    MSG_GP_I_GetSPRoomInfo  _data;
};

struct DL_GP_O_GetSPRoomInfo
{
    DataBaseResultLine		ResultHead;		//数据包头
    MSG_GP_O_GetSPRoomInfo  _data;
};


//修改密码验证手机号
struct DL_GP_I_MobileIsMatch
{
    DataBaseLineHead				DataBaseHead;							///<数据包头
    MSG_GP_SmsVCode         _data;
};

struct UserMobileNotMatch
{
    bool bIsMatch;
    char szMobileNum[32];
};
struct DL_GP_O_MobileIsMatch
{
    DataBaseResultLine				ResultHead;							///<数据包头 
    MSG_GP_SmsVCode                 SmsVCodeReq;
    UserMobileNotMatch              IsMatch;
};

// 排行榜
struct DL_GP_I_PaiHangBang
{
	DataBaseLineHead				DataBaseHead;							///<数据包头
	MSG_GP_PaiHangBang_In			_param;								
};
struct DL_GP_O_PaiHangBang
{
	DataBaseResultLine				ResultHead;							///<数据包头
    int                             iCount;
	MSG_GP_MoneyPaiHangBang_Item	*_data;
};

//执行签到
struct DL_I_HALL_SIGN
{
	DataBaseLineHead					DataBaseHead;						///<数据包头
	long								dwUserID;
};

struct DL_O_HALL_SIGN_CHECK
{
	DataBaseResultLine					ResultHead;							///<数据包头
	MSG_GP_S_SIGN_CHECK_RESULT			_data;
};

struct DL_O_HALL_SIGN_DO
{
	DataBaseResultLine					ResultHead;							///<数据包头
	MSG_GP_S_SIGN_DO_RESULT				_data;
};

//在线奖励
struct DL_I_HALL_ONLINE_AWARD
{
	DataBaseLineHead					DataBaseLineHead;
	long								dwUserID;
};

struct DL_O_HALL_ONLINE_AWARD_CHECK_RESULT
{
	DataBaseResultLine					ResultHead;							///<数据包头
	MSG_GP_S_ONLINE_AWARD_CHECK_RESULT	_result;
};
struct DL_O_HALL_ONLINE_AWARD_DO_RESULT
{
	DataBaseResultLine					ResultHead;							///<数据包头
	MSG_GP_S_ONLINE_AWARD_DO_RESULT	_result;
};
struct DL_I_HALL_BUY_DESK
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_S_BUY_DESK                   _data;
	DL_I_HALL_BUY_DESK()
	{
		memset(this,0,sizeof(DL_I_HALL_BUY_DESK));
	}
};
struct DL_O_HALL_BUY_DESK_RES
{
	DataBaseResultLine					ResultHead;	
	MSG_GP_S_BUY_DESK_RES               _result;
	DL_O_HALL_BUY_DESK_RES()
	{
		memset(this,0,sizeof(DL_O_HALL_BUY_DESK_RES));
	}
};
struct DL_I_HALL_ENTER_VIPDESK
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_S_ENTER_VIPDESK				_data;
	DL_I_HALL_ENTER_VIPDESK()
	{
		memset(this,0,sizeof(DL_I_HALL_ENTER_VIPDESK));
	}
};
struct DL_O_HALL_ENTER_VIPDESK_RES
{
	DataBaseResultLine					ResultHead;	
	MSG_GP_S_GET_CUTNETROOMINFO_RES		_result;

	DL_O_HALL_ENTER_VIPDESK_RES()
	{
		memset(this,0,sizeof(DL_O_HALL_ENTER_VIPDESK_RES));
	}
};
struct DL_I_HALL_MONEY_TO_JEWEL
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_S_MONEY_TO_JEWEL				_data;
	DL_I_HALL_MONEY_TO_JEWEL()
	{
		memset(this,0,sizeof(DL_I_HALL_MONEY_TO_JEWEL));
	}
};
struct DL_O_HALL_MONEY_TO_JEWEL_RES
{
	DataBaseResultLine					ResultHead;	
	MSG_GP_S_MONEY_TO_JEWEL_RES			_result;
	DL_O_HALL_MONEY_TO_JEWEL_RES()
	{
		memset(this,0,sizeof(DL_O_HALL_MONEY_TO_JEWEL_RES));
	}
};

struct DL_I_HALL_NETCUT
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_S_GET_CUTNETROOMINFO				_data;
	DL_I_HALL_NETCUT()
	{
		memset(this,0,sizeof(DL_I_HALL_NETCUT));
	}
};
struct DL_O_HALL_NETCUT_RES
{
	DataBaseResultLine								ResultHead;	
	MSG_GP_S_GET_CUTNETROOMINFO_RES					_result;
	int												iUserID;
	DL_O_HALL_NETCUT_RES()
	{
		memset(this,0,sizeof(DL_O_HALL_NETCUT_RES));
	}
};

//战绩
struct DL_I_HALL_TOTALRECORD
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_I_GetTotalRecord				_data;
	DL_I_HALL_TOTALRECORD()
	{
		memset(this,0,sizeof(DL_I_HALL_TOTALRECORD));
	}
};

struct DL_O_HALL_TOTALRECORD_RES
{
	DataBaseResultLine								ResultHead;	
	int												iCount;
	MSG_GP_O_TotalRecord							*_result;
	COleDateTime									datatime[50];
	
	DL_O_HALL_TOTALRECORD_RES()
	{
		memset(this,0,sizeof(DL_O_HALL_TOTALRECORD_RES));
	}
};

struct DL_I_HALL_SINGLERECORD
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_I_GetSingleRecord			_data;
	DL_I_HALL_SINGLERECORD()
	{
		memset(this,0,sizeof(DL_I_HALL_SINGLERECORD));
	}
};

struct DL_O_HALL_SINGLERECORD_RES
{
	DataBaseResultLine								ResultHead;	
	int												iCount;
	MSG_GP_O_SingleRecord							*_result;
	COleDateTime									datatime[50];
	DL_O_HALL_SINGLERECORD_RES()
	{
		memset(this,0,sizeof(DL_O_HALL_SINGLERECORD_RES));
	}
};

struct DL_O_HALL_GETCONFIG_RES
{
	DataBaseResultLine								ResultHead;	
	MSG_GP_S_GET_CONFIG_RES							_data;
	DL_O_HALL_GETCONFIG_RES()
	{
		memset(this,0,sizeof(DL_O_HALL_GETCONFIG_RES));
	}
};


struct DL_I_HALL_CONTEST_NOTICE
{
	DataBaseLineHead					DataBaseLineHead;
	int									iRoomID;
	int									iContestID;
	DL_I_HALL_CONTEST_NOTICE()
	{
		memset(this,0,sizeof(DL_I_HALL_CONTEST_NOTICE));
	}
};

struct DL_O_HALL_CONTEST_NOTICE_RES
{
	DataBaseResultLine								ResultHead;	
	MSG_GP_ContestNotice							_result;
	int												*pUserID;
	int												iUserNum;

	DL_O_HALL_CONTEST_NOTICE_RES()
	{
		memset(this,0,sizeof(DL_O_HALL_CONTEST_NOTICE_RES));
	}
};

struct DL_I_HALL_CONTEST_NOTICE_LOGON
{
	DataBaseLineHead					DataBaseLineHead;
	int									iUserID;
	DL_I_HALL_CONTEST_NOTICE_LOGON()
	{
		memset(this,0,sizeof(DL_I_HALL_CONTEST_NOTICE_LOGON));
	}
};

struct DL_O_HALL_CONTEST_NOTICE_LOGON_RES
{
	DataBaseResultLine								ResultHead;	
	MSG_GP_ContestNotice							_result;

	DL_O_HALL_CONTEST_NOTICE_LOGON_RES()
	{
		memset(this,0,sizeof(DL_O_HALL_CONTEST_NOTICE_LOGON_RES));
	}
};

struct DL_I_HALL_BuyDeskConfig
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_I_BuyDeskConfig				_data;
	DL_I_HALL_BuyDeskConfig()
	{
		memset(this,0,sizeof(DL_I_HALL_BuyDeskConfig));
	}
};

struct DL_O_HALL_BuyDeskConfig_RES
{
	DataBaseResultLine								ResultHead;	
	MSG_GP_O_BuyDeskConfig							_result;

	DL_O_HALL_BuyDeskConfig_RES()
	{
		memset(this,0,sizeof(DL_O_HALL_BuyDeskConfig_RES));
	}
};

struct DL_I_HALL_GetRecordURL
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_I_GetRecordURL				_data;
	DL_I_HALL_GetRecordURL()
	{
		memset(this,0,sizeof(DL_I_HALL_GetRecordURL));
	}
};

struct DL_O_HALL_GetRecordURL_RES
{
	DataBaseResultLine								ResultHead;	
	MSG_GP_O_GetRecordURL							_result;

	DL_O_HALL_GetRecordURL_RES()
	{
		memset(this,0,sizeof(DL_O_HALL_GetRecordURL_RES));
	}
};

struct DL_I_HALL_GetDeskList
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_I_GetBuydeskList				_data;
	DL_I_HALL_GetDeskList()
	{
		memset(this,0,sizeof(DL_I_HALL_GetDeskList));
	}
};

struct DL_O_HALL_GetDeskList_RES
{
	DataBaseResultLine								ResultHead;	
	int												iCount;
	COleDateTime									datatime[50];
	MSG_GP_O_GetBuydeskList							*_result;

	DL_O_HALL_GetDeskList_RES()
	{
		memset(this,0,sizeof(DL_O_HALL_GetDeskList_RES));
	}
};

struct DL_I_HALL_GetDeskUser
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_I_GetDeskUser				_data;
	DL_I_HALL_GetDeskUser()
	{
		memset(this,0,sizeof(DL_I_HALL_GetDeskUser));
	}
};

struct DL_O_HALL_GetDeskUser_RES
{
	DataBaseResultLine								ResultHead;	
	char											szDeskPass[20];
	int												iCount;
	MSG_GP_O_GetDeskUser							*_result;

	DL_O_HALL_GetDeskUser_RES()
	{
		memset(this,0,sizeof(DL_O_HALL_GetDeskUser_RES));
	}
};

struct DL_I_HALL_DissmissDesk
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_I_DissmissDesk				_data;
	int									iUserID;
	DL_I_HALL_DissmissDesk()
	{
		memset(this,0,sizeof(DL_I_HALL_DissmissDesk));
	}
};

struct DL_O_HALL_DissmissDesk
{
	DataBaseResultLine					ResultHead;	
	MSG_MG_S_DissmissDesk				_data;
	DL_O_HALL_DissmissDesk()
	{
		memset(this,0,sizeof(DL_O_HALL_DissmissDesk));
	}
};

struct DL_I_HALL_DeleteRecord
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_I_DeleteRecord				_data;
	int									iUserID;
	DL_I_HALL_DeleteRecord()
	{
		memset(this,0,sizeof(DL_I_HALL_DeleteRecord));
	}
};

struct DL_O_HALL_ChangeConfig_RES
{
	DataBaseResultLine								ResultHead;	
	MSG_GP_O_ChangeConfig							_result;

	DL_O_HALL_ChangeConfig_RES()
	{
		memset(this,0,sizeof(DL_O_HALL_ChangeConfig_RES));
	}
};

struct DL_I_HALL_ChangeRequest
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_I_ChangeRequest				_data;
	int									itype;   //0-金币兑换钻石，1-钻石兑换金币
	DL_I_HALL_ChangeRequest()
	{
		memset(this,0,sizeof(DL_I_HALL_ChangeRequest));
	}
};

struct DL_O_HALL_ChangeResponse
{
	DataBaseResultLine								ResultHead;	
	MSG_GP_O_ChangeResponse							_result;
	int												itype;   //0-金币兑换钻石，1-钻石兑换金币
	DL_O_HALL_ChangeResponse()
	{
		memset(this,0,sizeof(DL_O_HALL_ChangeResponse));
	}
};

struct DL_I_HALL_UpdateRoomConnect
{
	DataBaseLineHead					DataBaseLineHead;
	int									iRoomID;
	bool								bConnect;
	DL_I_HALL_UpdateRoomConnect()
	{
		memset(this,0,sizeof(DL_I_HALL_UpdateRoomConnect));
	}
};

struct DL_I_HALL_UpdateRoomConnect_ALL
{
	DataBaseLineHead					DataBaseLineHead;
	int									*RoomID;
	int									iCount;
	DL_I_HALL_UpdateRoomConnect_ALL()
	{
		memset(this,0,sizeof(DL_I_HALL_UpdateRoomConnect_ALL));
	}
};

struct DL_I_HALL_ForgetPWD
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_I_ForgetPWD					_data;
	DL_I_HALL_ForgetPWD()
	{
		memset(this,0,sizeof(DL_I_HALL_ForgetPWD));
	}
};

//struct DL_I_HALL_AddUserPhoneNum
//{
//    DataBaseLineHead					DataBaseLineHead;
//    MSG_GP_I_AddUserPhoneNum			_data;
//    DL_I_HALL_AddUserPhoneNum()
//    {
//        memset(this,0,sizeof(DL_I_HALL_AddUserPhoneNum));
//    }
//};

struct DL_I_HALL_ClearOnlineInfo
{
	DataBaseLineHead					DataBaseLineHead;
	int									ZID;
	DL_I_HALL_ClearOnlineInfo()
	{
		memset(this,0,sizeof(DL_I_HALL_ClearOnlineInfo));
	}
};




struct DL_I_HALL_CreateClub
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_I_CreateClub					_data;
	int									iUserID;
	DL_I_HALL_CreateClub()
	{
		memset(this,0,sizeof(DL_I_HALL_CreateClub));
	}
};

struct DL_O_HALL_CreateClub
{
	DataBaseResultLine								ResultHead;	
	MSG_GP_O_CreateClub								_data;

	DL_O_HALL_CreateClub()
	{
		memset(this,0,sizeof(DL_O_HALL_CreateClub));
	}
};

struct DL_I_HALL_DissmissClub
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_I_DissmissClub				_data;
	int									iUserID;
	DL_I_HALL_DissmissClub()
	{
		memset(this,0,sizeof(DL_I_HALL_DissmissClub));
	}
};

struct DL_O_HALL_DissmissClub
{
	DataBaseResultLine								ResultHead;	
	int												iDissmissDeskCount;
	MSG_GP_O_DissmissClub							_data;
	MSG_MG_R_ClubDissmissDesk						*_NoticeData;

	DL_O_HALL_DissmissClub()
	{
		memset(this,0,sizeof(DL_O_HALL_DissmissClub));
	}
};

struct DL_I_HALL_JoinClub
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_I_JoinClub					_data;
	int									iUserID;
	DL_I_HALL_JoinClub()
	{
		memset(this,0,sizeof(DL_I_HALL_JoinClub));
	}
};


struct DL_O_HALL_JoinClub
{
	DataBaseResultLine					ResultHead;	
	int									iMasterID;
	MSG_GP_O_Club_UserJoin				_UserDate;
	DL_O_HALL_JoinClub()
	{
		memset(this,0,sizeof(DL_O_HALL_JoinClub));
	}
};

struct DL_I_HALL_Club_UserList
{
	DataBaseLineHead					DataBaseLineHead;
	int									iUserID;
	MSG_GP_I_Club_UserList				_data;

	DL_I_HALL_Club_UserList()
	{
		memset(this,0,sizeof(DL_I_HALL_Club_UserList));
	}
};

struct DL_O_HALL_Club_UserList
{
	DataBaseResultLine								ResultHead;	
	MSG_GP_O_Club_UserList_Head						_data;
	MSG_GP_O_Club_UserList_Data						*_UserData;

	DL_O_HALL_Club_UserList()
	{
		memset(this,0,sizeof(DL_O_HALL_Club_UserList));
	}
};


struct DL_I_HALL_Club_RoomList
{
	DataBaseLineHead					DataBaseLineHead;
	int									iUserID;
	MSG_GP_I_Club_RoomList				_data;

	DL_I_HALL_Club_RoomList()
	{
		memset(this,0,sizeof(DL_I_HALL_Club_RoomList));
	}
};

struct DL_O_HALL_Club_RoomList
{
	DataBaseResultLine								ResultHead;	
	MSG_GP_O_Club_RoomList_Head						_data;
	MSG_GP_O_Club_RoomList_Data						*_RoomData;

	DL_O_HALL_Club_RoomList()
	{
		memset(this,0,sizeof(DL_O_HALL_Club_RoomList));
	}
};

struct DL_I_HALL_Club_BuyDesk
{
	DataBaseLineHead					DataBaseLineHead;
	MSG_GP_I_Club_BuyDesk				_data;

	DL_I_HALL_Club_BuyDesk()
	{
		memset(this,0,sizeof(DL_I_HALL_Club_BuyDesk));
	}
};

struct DL_O_HALL_Club_BuyDesk
{
	DataBaseResultLine					ResultHead;	
	int									iRoomID;
	int									iDeskID;
	int									iJewels;
	int									iGameID;
	MSG_GP_O_Club_BuyDesk				_data;

	DL_O_HALL_Club_BuyDesk()
	{
		memset(this,0,sizeof(DL_O_HALL_Club_BuyDesk));
	}
};


struct DL_I_HALL_ChangeName
{
	DataBaseLineHead					DataBaseLineHead;
	int									iUserID;
	MSG_GP_I_ChangeName					_data;

	DL_I_HALL_ChangeName()
	{
		memset(this,0,sizeof(DL_I_HALL_ChangeName));
	}
};

struct DL_O_HALL_ChangeName
{
	DataBaseResultLine								ResultHead;	
	MSG_GP_O_ChangeName								_data;

	DL_O_HALL_ChangeName()
	{
		memset(this,0,sizeof(DL_O_HALL_ChangeName));
	}
};

struct DL_I_HALL_Club_KickUser
{
	DataBaseLineHead					DataBaseLineHead;
	int									iUserID;
	MSG_GP_I_Club_KickUser				_data;

	DL_I_HALL_Club_KickUser()
	{
		memset(this,0,sizeof(DL_I_HALL_Club_KickUser));
	}
};

struct DL_O_HALL_Club_KickUser
{
	DataBaseResultLine								ResultHead;	
	MSG_GP_O_Club_KickUser							_data;
	MSG_GP_O_Club_UserChange						_TarData;
	int												iDissmissDeskCount;
	MSG_MG_R_ClubDissmissDesk						*_NoticeData;

	DL_O_HALL_Club_KickUser()
	{
		memset(this,0,sizeof(DL_O_HALL_Club_KickUser));
	}
};


struct DL_I_HALL_Club_List
{
	DataBaseLineHead					DataBaseLineHead;
	int									iUserID;

	DL_I_HALL_Club_List()
	{
		memset(this,0,sizeof(DL_I_HALL_Club_List));
	}
};

struct DL_O_HALL_Club_List
{
	DataBaseResultLine								ResultHead;	
	int												iCount;
	MSG_GP_O_Club_List								*_data;

	DL_O_HALL_Club_List()
	{
		memset(this,0,sizeof(DL_O_HALL_Club_List));
	}
};

struct DL_I_HALL_Club_ReviewList
{
	DataBaseLineHead					DataBaseLineHead;
	int									iUserID;
	MSG_GP_I_Club_ReviewList			_data;

	DL_I_HALL_Club_ReviewList()
	{
		memset(this,0,sizeof(DL_I_HALL_Club_ReviewList));
	}
};

struct DL_O_HALL_Club_ReviewList
{
	DataBaseResultLine								ResultHead;	
	MSG_GP_O_Club_ReviewList_Head					_HeadData;
	MSG_GP_O_Club_ReviewList_Data					*_Data;

	DL_O_HALL_Club_ReviewList()
	{
		memset(this,0,sizeof(DL_O_HALL_Club_ReviewList));
	}
};


struct DL_I_HALL_Club_MasterOpt
{
	DataBaseLineHead					DataBaseLineHead;
	int									iUserID;
	MSG_GP_I_Club_MasterOpt				_data;

	DL_I_HALL_Club_MasterOpt()
	{
		memset(this,0,sizeof(DL_I_HALL_Club_MasterOpt));
	}
};

struct DL_O_HALL_Club_MasterOpt
{
	DataBaseResultLine								ResultHead;	
	MSG_GP_O_Club_MasterOpt							_Data;
	MSG_GP_O_Club_List								_ClubData;
	MSG_GP_O_Club_UserChange						_TarData;

	DL_O_HALL_Club_MasterOpt()
	{
		memset(this,0,sizeof(DL_O_HALL_Club_MasterOpt));
	}
};


struct DL_I_HALL_Club_Notice
{
	DataBaseLineHead					DataBaseLineHead;
	int									iUserID;
	MSG_GP_I_Club_Notice				_data;

	DL_I_HALL_Club_Notice()
	{
		memset(this,0,sizeof(DL_I_HALL_Club_Notice));
	}
};

struct DL_O_HALL_Club_Notice
{
	DataBaseResultLine					ResultHead;	
	MSG_GP_O_Club_Notice				_data;

	DL_O_HALL_Club_Notice()
	{
		memset(this,0,sizeof(DL_O_HALL_Club_Notice));
	}
};


struct DL_I_HALL_Club_EnterClub
{
	DataBaseLineHead					DataBaseLineHead;
	int									iUserID;
	MSG_GP_I_Club_EnterClub				_data;

	DL_I_HALL_Club_EnterClub()
	{
		memset(this,0,sizeof(DL_I_HALL_Club_EnterClub));
	}
};

struct DL_O_HALL_Club_EnterClub
{
	DataBaseResultLine					ResultHead;	
	MSG_GP_O_Club_EnterClub				_data;

	DL_O_HALL_Club_EnterClub()
	{
		memset(this,0,sizeof(DL_O_HALL_Club_EnterClub));
	}
};

struct DL_I_HALL_Club_LeaveClub
{
	DataBaseLineHead					DataBaseLineHead;
	int									iUserID;
	MSG_GP_I_LeaveClub					_data;

	DL_I_HALL_Club_LeaveClub()
	{
		memset(this,0,sizeof(DL_I_HALL_Club_LeaveClub));
	}
};

struct DL_O_HALL_Club_LeaveClub
{
	DataBaseResultLine					ResultHead;	
	MSG_GP_O_LeaveClub					_data;
	MSG_GP_O_Club_UserChange			_UserData;

	DL_O_HALL_Club_LeaveClub()
	{
		memset(this,0,sizeof(DL_O_HALL_Club_LeaveClub));
	}
};


struct DL_I_HALL_Club_GetRecord
{
	DataBaseLineHead					DataBaseLineHead;
	int									iUserID;
	MSG_GP_I_Club_BuyDeskRecord			_data;

	DL_I_HALL_Club_GetRecord()
	{
		memset(this,0,sizeof(DL_I_HALL_Club_GetRecord));
	}
};

struct DL_O_HALL_Club_GetRecord
{
	DataBaseResultLine								ResultHead;	
	MSG_GP_O_Club_BuyDeskRecord_Head				_HeadData;
	MSG_GP_O_Club_BuyDeskRecord_Data				*_Data;

	DL_O_HALL_Club_GetRecord()
	{
		memset(this,0,sizeof(DL_O_HALL_Club_GetRecord));
	}
};


struct DL_I_HALL_Mail_List
{
	DataBaseLineHead					DataBaseLineHead;
	int									iUserID;

	DL_I_HALL_Mail_List()
	{
		memset(this,0,sizeof(DL_I_HALL_Mail_List));
	}
};

struct DL_O_HALL_Mail_List
{
	DataBaseResultLine								ResultHead;	
	MSG_GP_O_Mail_List_Head							_HeadData;
	MSG_GP_O_Mail_List_Data							*_data;

	DL_O_HALL_Mail_List()
	{
		memset(this,0,sizeof(DL_O_HALL_Mail_List));
	}
};

struct DL_I_HALL_Open_Mail
{
	DataBaseLineHead					DataBaseLineHead;
	int									iUserID;
	MSG_GP_I_Open_Mail					_data;

	DL_I_HALL_Open_Mail()
	{
		memset(this,0,sizeof(DL_I_HALL_Open_Mail));
	}
};

struct DL_O_HALL_Open_Mail
{
	DataBaseResultLine					ResultHead;	
	MSG_GP_O_Open_Mail					_data;

	DL_O_HALL_Open_Mail()
	{
		memset(this,0,sizeof(DL_O_HALL_Open_Mail));
	}
};


struct DL_I_HALL_Get_Attachment
{
	DataBaseLineHead					DataBaseLineHead;
	int									iUserID;
	MSG_GP_I_Get_Attachment				_data;

	DL_I_HALL_Get_Attachment()
	{
		memset(this,0,sizeof(DL_I_HALL_Get_Attachment));
	}
};

struct DL_O_HALL_Get_Attachment
{
	DataBaseResultLine					ResultHead;	
	MSG_GP_O_Get_Attachment				_data;

	DL_O_HALL_Get_Attachment()
	{
		memset(this,0,sizeof(DL_O_HALL_Get_Attachment));
	}
};


struct DL_I_HALL_Delete_Mail
{
	DataBaseLineHead					DataBaseLineHead;
	int									iUserID;
	MSG_GP_I_Delete_Mail				_data;

	DL_I_HALL_Delete_Mail()
	{
		memset(this,0,sizeof(DL_I_HALL_Delete_Mail));
	}
};

struct DL_O_HALL_Delete_Mail
{
	DataBaseResultLine					ResultHead;	
	MSG_GP_O_Delete_Mail				_data;

	DL_O_HALL_Delete_Mail()
	{
		memset(this,0,sizeof(DL_O_HALL_Delete_Mail));
	}
};

struct DL_O_HALL_Mail_Update
{
	DataBaseResultLine					ResultHead;
	int									iNewNumber;
	MSG_GP_O_Mail_Update				*_data;

	DL_O_HALL_Mail_Update()
	{
		memset(this,0,sizeof(DL_O_HALL_Mail_Update));
	}
};

struct DL_O_HALL_SysMsg
{
	DataBaseResultLine					ResultHead;
	bool								bBroadcast;//是否能广播
	MSG_GP_O_SysMsg						_data;//广播信息

	DL_O_HALL_SysMsg()
	{
		memset(this,0,sizeof(DL_O_HALL_SysMsg));
	}
};

struct DL_I_HALL_LUCK_DRAW_CONFIG
{
    DataBaseLineHead                    DataBaseLineHead;
    MSG_GP_I_LuckDraw_Config            _data;   
};

struct DL_O_HALL_LUCK_DRAW_CONFIG
{
    DataBaseResultLine                    ResultHead;
    MSG_GP_O_LuckDraw_Config            _data;   
};

struct DL_I_HALL_LUCK_DRAW_DO
{
    DataBaseLineHead                    DataBaseLineHead;
    MSG_GP_I_LuckDraw_DO                _data;   
};

struct DL_O_HALL_LUCK_DRAW_DO
{
    DataBaseResultLine                    ResultHead;
    MSG_GP_O_LuckDraw_DO                _data;   
};

struct DL_O_DissmissDeskByMS
{
    DataBaseResultLine                    ResultHead;
    int iRoomID;
    int iDeskNo;
    int iMasterID;
    int iPositionLimit;
    char szDeskPwd[20];
};
/********************************************************************************************/

/********************************************************************************************/

#endif