#include "stdafx.h"
#include "ServiceDataBaseHandle.h"

const char *szWalletMoney = "Walletmoney";
const char *szTChangeRecord = "TChangeRecord";
const char *szUpdate = "Update";
const char *szDelete_ = "delete";
const char *szAgency = "AgencyID";
const char *szSet = "set";
const char *szTUserInfo = "TUserInfo";
const char *szTUsers = "TUsers";
const char *szBankMoney = "BankMoney";
const char *szPass = "Pass";
const char *szWeb_Users = "Web_Users";

CServiceDataBaseHandle::CServiceDataBaseHandle()
{

}

CServiceDataBaseHandle::~CServiceDataBaseHandle()
{

}



/// 用于GM工具
/// 以下几个函数都是调用存储过程并返回数据集，流程如下
///    1、获取输入参数，定义输出参数
///    2、确定存储过程
///    3、准备存储过程的输入参数
///    4、执行存储过程
///    5、判断存储过程返回值
///    6、获取返回的数据集
///    7、发出通知消息给下一步处理过程

///GM按玩家信息获取已经操作过的情况，玩家ID和昵称对应关系
UINT CServiceDataBaseHandle::OnGMGetOperatedUser(DataBaseLineHead * pSourceData)
{
	// 获取输入参数，定义输出参数
	GM_DT_I_GetOperatedUser_t *pGetHistory = (GM_DT_I_GetOperatedUser_t *)pSourceData;
	GM_DT_O_OperatedUser_t getHistoryResult;

	// 确定存储过程 
	if (pGetHistory->requestPlayer.bType == 0)
	{
		bool bres=sqlSPSetNameEx("SP_GMGetOperatedUserByID",true);
		if(!bres)
		{
			return 0;
		}
		
		// 准备存储过程的输入参数
		addInputParameter("@UserID",pGetHistory->requestPlayer.playerInfo.UserID);
	}
	else
	{
		bool bres=sqlSPSetNameEx("SP_GMGetOperatedUserByName",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@Type", pGetHistory->requestPlayer.bType);
		addInputParameter("@Name", pGetHistory->requestPlayer.playerInfo.szName, sizeof(pGetHistory->requestPlayer.playerInfo.szName));
	}


	// 执行存储过程，带参数true才能返回数据集
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	int ret = getReturnValue();
	int handleResult = DTR_GM_GET_RECORD_END;
	
	// 判断存储过程返回值
	if (ret == 0 )
	{
		while(!adoEndOfFile())
		{
			// 获取返回的数据集
			handleResult = DTR_GM_GET_RECORD_OK;
			getValue("UserID",&getHistoryResult.operatedPlayer.iUserID);
			getValue("UserName",getHistoryResult.operatedPlayer.szUserName, sizeof(getHistoryResult.operatedPlayer.szUserName));
			getValue("NickName",getHistoryResult.operatedPlayer.szNickName, sizeof(getHistoryResult.operatedPlayer.szNickName));
			getValue("GMID",&getHistoryResult.operatedPlayer.iGMID);
			getValue("OperationType",&getHistoryResult.operatedPlayer.iOperation);
			getValue("BeginTime",&getHistoryResult.operatedPlayer.dtDate);
			getValue("LastMinutes",&getHistoryResult.operatedPlayer.iPeriod);
			getValue("Reason",getHistoryResult.operatedPlayer.szReason, sizeof(getHistoryResult.operatedPlayer.szReason));
			char szTmp[16];
			getValue("ZID", szTmp, sizeof(szTmp));
			getHistoryResult.operatedPlayer.ZID = atoi(szTmp);
			getValue("WID",&getHistoryResult.operatedPlayer.WID);
		}
	}
	closeRecord();
	
	// 发出通知消息给下一步处理过程
	m_pRusultService->OnDataBaseResultEvent(&getHistoryResult.ResultHead,DTK_GET_OPEATED_USER_GM, handleResult, sizeof(GM_DT_O_OperatedUser_t),pSourceData->uIndex,pSourceData->dwHandleID);

	return 0;
}

// GM获取奖励方案
UINT CServiceDataBaseHandle::OnGMGetAwardType(DataBaseLineHead * pSourceData)
{
	GM_DT_O_AwardType_t awardTypeResult;
	// 确定存储过程 
	bool bres=sqlSPSetNameEx("SP_GMGetAwardTypeList",true);
	if(!bres)
	{
		return 0;
	}
	// 执行存储过程
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	int ret = getReturnValue();
	
	// 判断存储过程返回值
	if (ret == 0 )
	{
		// 获取返回的数据集
		while(!adoEndOfFile())
		{
			ZeroMemory(&awardTypeResult,sizeof(GM_DT_O_AwardType_t));
			getValue("SeriNo", &awardTypeResult.awardType.iSeriNo);
			getValue("Gold", &awardTypeResult.awardType.iGoldNum);
			getValue("PropID1", &awardTypeResult.awardType.iPropAward[0].iPropID);
			getValue("PropID2", &awardTypeResult.awardType.iPropAward[1].iPropID);
			getValue("PropID3", &awardTypeResult.awardType.iPropAward[2].iPropID);
			getValue("PropID4", &awardTypeResult.awardType.iPropAward[3].iPropID);
			getValue("PropCount1", &awardTypeResult.awardType.iPropAward[0].iCount);
			getValue("PropCount2", &awardTypeResult.awardType.iPropAward[1].iCount);
			getValue("PropCount3", &awardTypeResult.awardType.iPropAward[2].iCount);
			getValue("PropCount4", &awardTypeResult.awardType.iPropAward[3].iCount);

			// 发出通知消息给下一步处理过程
			m_pRusultService->OnDataBaseResultEvent(&awardTypeResult.ResultHead, DTK_GET_AWARD_TYPE_GM, DTR_GM_GET_RECORD_OK, sizeof(GM_DT_O_AwardType_t),pSourceData->uIndex,pSourceData->dwHandleID);
			moveNext();
		}

		// 发出通知消息给下一步处理过程
		m_pRusultService->OnDataBaseResultEvent(&awardTypeResult.ResultHead, DTK_GET_AWARD_TYPE_GM, DTR_GM_GET_RECORD_END, sizeof(GM_DT_O_AwardType_t),pSourceData->uIndex,pSourceData->dwHandleID);
	}
	closeRecord();

	return 0;
}

// GM获取道具ID与名称对应关系
UINT CServiceDataBaseHandle::OnGMGetPropInfo(DataBaseLineHead * pSourceData)
{
	GM_DT_O_GetPropInfoResult_t propInfoResult;
	
	// 确定存储过程 
	bool bres=sqlSPSetNameEx("SP_GMGetPropInfo",true);
	if(!bres)
	{
		return 0;
	}
	// 执行存储过程
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	int ret = getReturnValue();
	
	// 判断存储过程返回值
	if (ret == 0)
	{
		// 获取返回的数据集
		while(!adoEndOfFile())
		{
			memset(&propInfoResult, 0, sizeof(GM_DT_O_GetPropInfoResult_t));
			getValue("PropID", &propInfoResult.propInfo.iPropID);
			getValue("PropName", propInfoResult.propInfo.szPropName, sizeof(propInfoResult.propInfo.szPropName));

			// 发出通知消息给下一步处理过程
			m_pRusultService->OnDataBaseResultEvent(&propInfoResult.ResultHead, DTK_GET_PROP_INFO_GM, DTR_GM_GET_RECORD_OK, sizeof(GM_DT_O_GetPropInfoResult_t),pSourceData->uIndex,pSourceData->dwHandleID);
			moveNext();
		}
		
		// 发出通知消息给下一步处理过程
		m_pRusultService->OnDataBaseResultEvent(&propInfoResult.ResultHead, DTK_GET_PROP_INFO_GM, DTR_GM_GET_RECORD_END, sizeof(GM_DT_O_GetPropInfoResult_t),pSourceData->uIndex,pSourceData->dwHandleID);
	}
	closeRecord();	

	return 0;
}

// GM对玩家进行奖惩操作0
UINT CServiceDataBaseHandle::OnGMOperateToUser(DataBaseLineHead * pSourceData)
{
	// 获取输入参数，定义输出参数
	GM_DT_I_OperateUser_t *pOperation = (GM_DT_I_OperateUser_t *)pSourceData;

	// 确定存储过程 
	bool bres=sqlSPSetNameEx("SP_GMOperateUser",true);
	if(!bres)
	{
		return 0;
	}

	// 准备存储过程的输入参数
	addInputParameter("@UserID",pOperation->operation.iUserID);
	addInputParameter("@GMID",pOperation->operation.iGMID);
	addInputParameter("@OperationType",pOperation->operation.iOperation);
	addInputParameter("@LastMinutes",pOperation->operation.iWorkMinutes);
	addInputParameter("@AwardType",pOperation->operation.iWorkMinutes);
	addInputParameter("@Reason", pOperation->operation.szReason, sizeof(pOperation->operation.szReason));


	// 执行存储过程
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	int ret = getReturnValue();
	int handleResult = ERR_GM_OPERATOR_FAILED;
	if (ret == 0)
	{
		handleResult = ERR_GM_OPERATOR_SUCCESS;
	}
	DataBaseResultLine ResultHead;
	m_pRusultService->OnDataBaseResultEvent(&ResultHead,DTK_GET_OPEATED_USER_GM, handleResult, sizeof(ResultHead), pSourceData->uIndex,pSourceData->dwHandleID);
	closeRecord();	
	
	return 0;
}

// GM登陆处理
UINT CServiceDataBaseHandle::OnGMLogon(DataBaseLineHead * pSourceData, UINT uLogonType)
{
	GM_DT_I_LogonByName_t *pLogonInfo = (GM_DT_I_LogonByName_t *)pSourceData;
	GM_DT_O_LogonResult_t logResult;

	// 确定存储过程 
	bool bres=sqlSPSetNameEx("SP_GMLogon",true);
	if(!bres)
	{
		return 0;
	}

	// 准备存储过程的输入参数
	addInputParameter("@UserName",pLogonInfo->logonInfo.szName, sizeof(pLogonInfo->logonInfo.szName));
	addInputParameter("@MD5Password",pLogonInfo->logonInfo.szMD5Pass, sizeof(pLogonInfo->logonInfo.szMD5Pass));
	addInputParameter("@UserLoginIP",pLogonInfo->szAccessIP, sizeof(pLogonInfo->szAccessIP));
	addInputParameter("@MachineCode",pLogonInfo->logonInfo.szMathineCode, sizeof(pLogonInfo->logonInfo.szMathineCode));
	addInputParameter("@ZID", 0);

	int handleResult = DTR_GM_LOGON_SUCCESS;

	// 执行存储过程
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	int ret = getReturnValue();

    WriteLog("SP_GMLogon, UserName = %s, PWD = %s, IP = %s, MachineCode = %s, ZID = 0, Ret = %d", pLogonInfo->logonInfo.szName, pLogonInfo->logonInfo.szMD5Pass, pLogonInfo->szAccessIP, pLogonInfo->logonInfo.szMathineCode, ret);

	if (ret == 0)
	{
		if(!adoEndOfFile())
		{
			getValue("GMID", &logResult.logonResult.dwUserID);
			getValue("NickName", logResult.logonResult.szName, sizeof(logResult.logonResult.szName));
			TCHAR szLastLogonIP[16]={0};
			getValue(TEXT("LastLoginIP"),szLastLogonIP,sizeof(szLastLogonIP));
			logResult.logonResult.dwLastLogonIP=inet_addr(szLastLogonIP);
			getValue("Master", &logResult.logonResult.iMasterRight);
			getValue("MD5Password", logResult.logonResult.szMD5Pass, sizeof(logResult.logonResult.szMD5Pass));
		}
		else
		{
			handleResult = DTR_GM_HAS_NOT_RIGHT;
		}
	}
	else
	{
		switch (ret)
		{
		// 用户不存在
		case 1: 
			{
				handleResult = DTR_GM_USER_NOT_FOUND; 
				break;
			}

		// 密码不正确
		case 2: 
			{
				handleResult = DTR_GM_PASSWORD_ERROR;
				break;
			}

		// 没有GM权限
		case 3: 
			{
				handleResult = DTR_GM_HAS_NOT_RIGHT;
				break;
			}

		// 机器码不正确
		case 4: 
			{
				handleResult = DTR_GM_MACHINECODE_ERR;
				break;
			}

		// 已经有另一个同名GM登录中
		case 5: 
			{
				handleResult = DTR_GM_ALREADY_LOGON; 
				break;
			}
		default:
			break;
		}
	}
	closeRecord();

	// 发出通知消息给下一步处理过程
	m_pRusultService->OnDataBaseResultEvent(&logResult.ResultHead,DTK_LOGON_BY_NAME_GM, handleResult, sizeof(GM_DT_O_LogonResult_t),pSourceData->uIndex,pSourceData->dwHandleID);
	//DTK_LOGON_BY_NAME_GM
	return 0;

}


UINT CServiceDataBaseHandle::OnOneKeyRegister(DataBaseLineHead * pSourceData, UINT uLogonType)
{
	DL_GP_I_Register *pRegister = (DL_GP_I_Register *)pSourceData;
	DL_GP_O_Register dl_out;
	memset(&dl_out, 0, sizeof(dl_out));
	
	// 确定存储过程 
	bool bres=sqlSPSetNameEx("SP_PhoneAddUser",true);
	if(!bres)
	{
		return 0;
	}

	/// 准备存储过程的输入参数	
	addInputParameter("@FastRegister", pRegister->_data.byFastRegister);
	addInputParameter("@UserName", pRegister->_data.szName, sizeof(pRegister->_data.szName));
	addInputParameter("@Pass", pRegister->_data.szPswd, sizeof(pRegister->_data.szPswd));
	addInputParameter("@HardID", pRegister->_data.szHardID, sizeof(pRegister->_data.szHardID));
	addInputParameter("@WeiXinName", pRegister->_data.szWeiXinName, sizeof(pRegister->_data.szWeiXinName));
	addInputParameter("@Boy",  pRegister->_data.bBoy);
	addInputParameter("@HeadUrl", pRegister->_data.szHeadUrl, sizeof(pRegister->_data.szHeadUrl));
	addInputParameter("@UionID", pRegister->_data.szUionID, sizeof(pRegister->_data.szUionID));
	// 执行存储过程
	if(0 != execStoredProc())
	{
		closeRecord();
		m_pDataBaseManage->SQLConnectReset();
		DataBaseResultLine ResultHead;
		m_pRusultService->OnDataBaseResultEvent(&ResultHead,DTK_GP_RESET_SQL, 0, sizeof(ResultHead),0,0);
		return 0;
	}

	int ret = getReturnValue();
	memcpy(&dl_out._data, &pRegister->_data, sizeof(pRegister->_data));

	char str[100] = {0};

	if (1 == ret)
	{
		getValue("UserName", dl_out._data.szName, sizeof(dl_out._data.szName));
		getValue("Pswd", dl_out._data.szPswd, sizeof(dl_out._data.szPswd));
	}
	closeRecord();

	// 发出通知消息给下一步处理过程
	m_pRusultService->OnDataBaseResultEvent(&dl_out.ResultHead,pSourceData->uHandleKind, ret, sizeof(DL_GP_O_Register),pSourceData->uIndex,pSourceData->dwHandleID);
	return 0;
}

// 数据库处理接口
UINT CServiceDataBaseHandle::HandleDataBase(DataBaseLineHead * pSourceData)
{
	switch (pSourceData->uHandleKind)
	{
	// 获取道具信息（用于GM模块处理过程）
	case DTK_GET_PROP_INFO_GM:
		return OnGMGetPropInfo(pSourceData);
		
	// 对玩家进行奖惩操作（GM）
	case DTK_PUT_OPEATION_USER_GM:
		return OnGMOperateToUser(pSourceData);

	// 获取指定的玩家的被操作过的信息及当前状态
	case DTK_GET_OPEATED_USER_GM:
		return OnGMGetOperatedUser(pSourceData);

	// GM获取奖励方案
	case DTK_GET_AWARD_TYPE_GM:
		return OnGMGetAwardType(pSourceData);

	// 调用GM专用登陆函数
	case DTK_LOGON_BY_NAME_GM:
		return OnGMLogon(pSourceData,pSourceData->uHandleKind);

	// 注册
	case DTK_GP_REGISTER:
		return OnOneKeyRegister(pSourceData, pSourceData->uHandleKind);

	case DTK_GP_LOGON_BY_NAME:		// 通过名字登陆   
		return OnPlaceLogon(pSourceData,pSourceData->uHandleKind);// 调用登陆函数

	// 玩家返利数据
	case DTK_GP_BANK_UPDATE_USER_REBATE:
		return OnUpdateUserRebate(pSourceData);

	// 转账返利
	case DTK_GP_BANK_UPDATE_REBATE:
		return 	OnUpdateRebate(pSourceData);

	// 更新构件列表
	case DTK_GP_UPDATE_LIST:
		return OnUpdateGameList(pSourceData);

	// 检测数据库连接
	case DTK_GP_CHECK_CONNECT:
		return OnCheckDataConnect(pSourceData);

	// 房间列表玩家人数
	case DTK_GP_GET_ROOM_LIST_PEOCOUNT:
		return OnGetRoomListPeoCount(pSourceData);

	// 获取登录人数
	case DTK_GP_LOGONPEOPLE_GET:
		return OnGetLogonPeopCount(pSourceData);

	 // 试图进入加密房间
	case DTK_GP_CHECK_ROOM_PASSWD:
		OnCheckRoomPasswd(pSourceData);
		break;

	case DTK_GP_CONTEST_APPLY_LIST:
		OnGetContestInfo(pSourceData);
		break;

	// 以下是对用户头像的处理
	case DTK_GP_USERLOGO_UPDATE:
		OnUpdateUserLogoID(pSourceData);
		break;

	// 取头像的MD5值
	case DTK_GP_USERLOGO_GETMD5:
		OnRequestUserLogoMD5(pSourceData);
		break;
	
	// 获取比赛房间ID
	case DTK_GP_GET_CONTEST_ROOMID:
		OnGetContestRoomID(pSourceData);
		break;	

	case DTK_GP_CONTEST_NOTICE:
		OnContestNoticeUser(pSourceData);
		break;	
	
	case DTK_GP_GET_APPLY_NUM:
		OnGetApplyNum(pSourceData);
		break;

	case DTK_GP_CONTEST_GETAWARD:
		OnGetApplyAward(pSourceData);
		break;

	case DTK_GP_CONTEST_APPLY:
		OnContestApply(pSourceData);
		break;

	// 以下是对好友的操作处理
	// 返回好友数据
	case DTK_GP_IM_GETCOUNT:
		OnDBIMGetUserCount(pSourceData);
		break;

	// 服务器启动清除在线标志
	case DTK_GP_IM_CLEAR:
		OnClearOnlineFlag(pSourceData);
		break;

	// 用户下线
	case DTK_GP_IM_OFFLINE:
		OnUserOffline(pSourceData);
		break;

	// 设置好友分组
	case DTK_GP_IM_SETUSER:
		OnIMSetUser(pSourceData);
		break;

	// 修改写Z登录记录的时机，确保网络卡的时候，不至于导致用户登录不上
	case DTK_GP_ZRECORD:
		OnWriteUserLogonRecord(pSourceData);
		break;

	// 以下是道具操作
	// 道具界面购买VIP
	case DTK_GP_PROP_BUY_VIP:
		OnPropBuyVIP(pSourceData);    
		break;

	// 获取道具
	case DTK_GP_PROP_GETPROP:
		OnPropGetList(pSourceData);
		break;
	
	// 购买道具
	case DTK_GP_PROP_BUY:
		OnPropBuy(pSourceData);
		break;

	// 商店道具的即时购买功能
    case DTK_GP_PROP_BUY_NEW:
        OnPropBuyNew(pSourceData);
        break;

	// 赠送道具
	case DTK_GP_PROP_GIVE:
		OnPropGive(pSourceData);
		break;

	// 大喇叭
	case DTK_GPR_PROP_BIG_BRD:
		OnPropBigBrd(pSourceData);
		break;

	case DTK_GPR_PROP_BIG_BRD_BUYADNUSE:
		OnPropBigBrd_BuyAndUse(pSourceData);
		break;

	// 锁定机器
	case DTK_GP_LOCK_UNLOCK_ACCOUNT:
		OnUserLockAccount(pSourceData);	
		break;

	// 手机绑定
	case DTK_GP_BIND_MOBILE:
		OnUserBindMobile(pSourceData);
		break;

	// 设置用户头像
	case DTK_GP_SET_LOGO:
		OnSetUserLogo(pSourceData);
		break;

	case DTK_GP_UPDATE_USERINFO_BASE:
		return OnUpdateUserInfo(pSourceData,DTK_GP_UPDATE_USERINFO_BASE);

	case DTK_GP_UPDATE_USERINFO_DETAIL:
		return OnUpdateUserInfo(pSourceData,DTK_GP_UPDATE_USERINFO_DETAIL);

  //  case DTK_GP_USER_ADDPHONENUM:                        // 用户账号关联手机号
  //      return OnAddUserPhoneNum(pSourceData);
	case DTK_GP_UPDATE_USERINFO_PWD:
		return OnUpdateUserPwd(pSourceData);

	case DTK_GP_UPDATE_NICKNAMEID:
		return OnQueryUserNickNameID(pSourceData);

	case DTK_GP_CHARMEXCHANGE_LIST:
		return OnCharmExchangeList(pSourceData);

	case DTK_GP_CHARMEXCHANGE:
		return OnCharmExchange(pSourceData);

	case DTK_GP_SETFRIENDTYPE:
		return OnSetFriendType(pSourceData);

	// 用于银行处理模块
	case DTK_GP_BANK_OPEN:
		return OnBankOpen(pSourceData);

	case DTK_GP_BANK_SET_USER_REBATE:
		return OnBankTransferRebate(pSourceData);

	case DTK_GP_BANK_GETGM_WALLET:
		return OnBankGetGameWallet(pSourceData);

	case DTK_GP_BANK_CHECK:
		return OnBankCheck(pSourceData);

	case DTK_GP_BANK_TRANSFER:
		return OnBankTransfer(pSourceData);

	case DTK_GP_BANK_TRANS_RECORD:
		return OnBankTransRecord(pSourceData);

	case DTK_GP_BANK_DEPOSIT:
		return OnBankDeposit(pSourceData);

	case DTK_GP_BANK_QUERY_DEPOSIT:
		return OnBankQueryDeposits(pSourceData);

	case DTK_GP_BANK_CHPWD:
		return OnBankChangePassword(pSourceData);

	case DTK_GP_BANK_FINANCE_INFO:
		return OnBankGetFinanceInfo(pSourceData);

	case DTK_GP_LASTLOGINGAME:
		return OnLastLoginGame(pSourceData);

	case DTK_GP_PAIHANGBANG:
		return OnPaiHangBang(pSourceData);

	case DTK_GP_SIGNIN_CHECK:
		return OnSignInCheck(pSourceData);

	case DTK_GP_SIGNIN_DO:
		return OnSignInDo(pSourceData);

	case DTK_GP_ONLINE_AWARD_CHECK:
		return OnLineAwardCheck(pSourceData);
		
	case DTK_GP_ONLINE_AWARD_DO:
		return OnLineAwardDo(pSourceData);

	case DTK_GP_BUY_DESK:
		return OnBuyDesk(pSourceData);

	case DTK_GP_ENTER_VIPDESK:
		return OnEnterVIPDesk(pSourceData);

	case DTK_GP_TOTALRECORD:
		return OnTotalRecord(pSourceData);

	case DTK_GP_SINGLERECORD:
		return OnSingleRecord(pSourceData);

	case DTK_GP_GET_CUTROOM:
		return OnGetCutRoom(pSourceData);

	case DTK_GP_GET_CONFIG:
		return OnGetConfig(pSourceData);

	case DTK_GP_CONTEST_NOTICE_LOGON:
		return OnContestNoticeLogon(pSourceData);

	case DTK_GP_GET_BUYDESKCONFIG:
		return OnGetDeskConfig(pSourceData);

	case DTK_GP_GET_RECORDURL:
		return OnGetRecordURL(pSourceData);

	case DTK_GP_GET_DESKLIST:
		return OnGetDeskList(pSourceData);

	case DTK_GP_GET_DESKUSER:
		return OnGetDeskUser(pSourceData);

	case DTK_GP_DISSMISSDESK:
		return OnDissmissDesk(pSourceData);

	case DTK_GP_DELETERECORD:
		return OnDeleteRecord(pSourceData);

	case DTK_GP_CHANGE_CONFIG:
		return OnGetChangeConfig(pSourceData);

	case DTK_GP_MONEY_CHANGE:
		return OnChangeMoney(pSourceData);

	case DTK_GP_UPDATE_CONNECT:
		return OnUpdateRoomConnect(pSourceData);

	case DTK_GP_UPDATE_CONNECT_ALL:
		return OnUpdateRoomConnectAll(pSourceData);

	case DTK_GP_USERINFO_FORGET_PWD:
		return OnForgetPWD(pSourceData);

	case DTK_GP_CREATE_CLUB:
		return OnCreateClub(pSourceData);

	case DTK_GP_DISSMISS_CLUB:
		return OnDissmissClub(pSourceData);

	case DTK_GP_JOIN_CLUB:
		return OnJoinClub(pSourceData);

	case DTK_GP_CLUB_USERLIST:
		return OnClubUserList(pSourceData);

	case DTK_GP_CLUB_ROOMLIST:
		return OnClubRoomList(pSourceData);

	case DTK_GP_CLUB_CREATEROOM:
		return OnClubBuyDesk(pSourceData);

	case DTK_GP_CLUB_CHANGENAME:
		return OnClubChangename(pSourceData);

	case DTK_GP_CLUB_KICKUSER:
		return OnClubKickUser(pSourceData);

	case DTK_GP_CLUB_LIST:
		return OnClubList(pSourceData);

	case DTK_GP_REVIEW_LIST:
		return OnClubReviewList(pSourceData);

	case DTK_GP_MASTER_OPTION:
		return OnClubMasterOPT(pSourceData);

	case DTK_GP_CLUB_NOTICE:	
		return OnClubChangeNotice(pSourceData);

	case DTK_GP_ENTER_CLUB:
		return OnEnterClub(pSourceData);

	case DTK_GP_GET_DESKRECORD:
		return OnGetBuyDeskRecord(pSourceData);

	case DTK_GP_LEAVE_CLUB:
		return OnLeaveClub(pSourceData);

	case DTK_GP_MAIL_LIST:
		return OnMailList(pSourceData);

	case DTK_GP_OPEN_MAIL:
		return OnOpenMail(pSourceData);

	case DTK_GP_GET_ATTACHMENT:
		return OnGetAttachment(pSourceData);

	case DTK_GP_DEL_MAIL:
		return OnMailDelete(pSourceData);

	case DTK_GP_UPDATE_MAIL:
		return OnMailUpdate(pSourceData);

	case DTK_GP_UPDATE_SYSTEM:
		return OnGetSysMsg(pSourceData);
    case DTK_GP_LUCK_DRAW_CONFIG:
        return OnLuckDrawConfig(pSourceData);
    case DTK_GP_LUCK_DRAW_DO:
        return OnLuckDrawDo(pSourceData);
    case DTK_GP_DISSMISS_TIMEOUTDESK:
        return OnDissmissTimeOutDesk(pSourceData);
    case DTK_GP_MOBILE_IS_MATCH:
        return OnMobileIsMatch(pSourceData);

    case DTK_GP_GET_SPROOMINFO:
        return OnGetSpRoomInfo(pSourceData);
	}

	return 0;
}

UINT CServiceDataBaseHandle::OnMobileIsMatch(DataBaseLineHead *pSourceData)
{
    DL_GP_I_MobileIsMatch *pMobileIsMatch = (DL_GP_I_MobileIsMatch *)pSourceData;
    if (NULL == pMobileIsMatch)
    {
        return 0;
    }
    CString strSql = "";
    strSql.Format("SELECT PhoneNum FROM TUserInfo WHERE UserID = (SELECT UserID FROM TUsers WHERE UserName = \'%s\')", pMobileIsMatch->_data.szName);
    if (execSQL(strSql,true))
    {
        m_pDataBaseManage->SQLConnectReset();
        closeRecord();
        return 0;
    }
    DL_GP_O_MobileIsMatch _outData;
    memset(&_outData, 0, sizeof(_outData));
    memcpy(&_outData.SmsVCodeReq, (void *)&pMobileIsMatch->_data, sizeof(MSG_GP_SmsVCode));
    getValue("PhoneNum", _outData.IsMatch.szMobileNum);
    if (0 != strcmp(_outData.IsMatch.szMobileNum, "")
        &&
        0 != strcmp(pMobileIsMatch->_data.szMobileNo, _outData.IsMatch.szMobileNum)
        )
    {
        _outData.IsMatch.bIsMatch = false;
    }
    else
    {
        _outData.IsMatch.bIsMatch = true;
    }
    closeRecord();

    m_pRusultService->OnDataBaseResultEvent(&_outData.ResultHead, DTK_GP_MOBILE_IS_MATCH, 0, sizeof(DL_GP_O_MobileIsMatch), pSourceData->uIndex, pSourceData->dwHandleID);
}

// 大喇叭
void CServiceDataBaseHandle::OnPropBigBrd(DataBaseLineHead *pSourceData)
{
	PROP_GPR_I_BOARDCAST *iBoard = (PROP_GPR_I_BOARDCAST*)pSourceData;
	PROP_GPR_O_BOARDCAST oBoard;
	memcpy(&oBoard.boardCast, &iBoard->boardCast, sizeof(_TAG_BOARDCAST));

	int iHandleResult = DTK_GR_PROP_USE_SUCCEED;

	bool bres=sqlSPSetNameEx("SP_PropOpera",true);
	if(!bres)
	{
		return;
	}

	addInputParameter("@UserID",iBoard->boardCast.dwUserID);
	addInputParameter("@PropID",iBoard->boardCast.nPropID);

	// 只使用1个
	addInputParameter("@PropCount",1);			
	TCHAR szTarget[32]={0};

	// 赠送对象或使用对象，只有当isGive=1时有效
	addInputParameter("@TargetName",szTarget,sizeof(szTarget));
	
	// 是否赠送，只有=1时才是赠送
	addInputParameter("@IsGive",0);
	if(0 != execStoredProc())
	{
		closeRecord();
		return;
	}

	int ret = getReturnValue();
	closeRecord();
	if(ret != 0)
	{
		iHandleResult = DTR_GR_PROP_USE_ERR_NOPROP;
	}
	m_pRusultService->OnDataBaseResultEvent(&oBoard.ResultHead,DTK_GPR_PROP_BIG_BRD,iHandleResult,sizeof(PROP_GPR_O_BOARDCAST),pSourceData->uIndex,pSourceData->dwHandleID);
}

void CServiceDataBaseHandle::OnPropBigBrd_BuyAndUse(DataBaseLineHead *pSourceData)
{
	PROP_GPR_I_BOARDCAST *iBoard = (PROP_GPR_I_BOARDCAST*)pSourceData;
	PROP_GPR_O_BOARDCAST oBoard;
	memcpy(&oBoard.boardCast,&iBoard->boardCast,sizeof(_TAG_BOARDCAST));

	PROP_GP_O_BUY propOBuy;
	ZeroMemory(&propOBuy,sizeof(PROP_GP_O_BUY));

	propOBuy.msgPropGet.dwUserID=iBoard->boardCast.dwUserID;
	try
	{
		
		bool bres=sqlSPSetNameEx("SP_NewPropBuy",true);
		if(!bres)
		{
			return;
		}

		addInputParameter("@UserID",iBoard->boardCast.dwUserID);
		addInputParameter("@PropID",iBoard->boardCast.nPropID);
		addInputParameter("@PropCount",1);

		if(0 != execStoredProc())
		{
			closeRecord();
			return;
		}

		int ret=getReturnValue();

		if(ret!=0)
		{
			closeRecord();
			throw DTK_GP_PROP_BUY_NOMONEY;
		}

		// 操作属性此处不需要，使用这个值区分道具是在房间还是游戏端购买的
		// 0表示在房间购买，1表示在游戏端购买
		//propOBuy.msgPropGet.attribAction = propIBuy->propBuy.iPropPayMoney;

		propOBuy.msgPropGet.dwUserID = iBoard->boardCast.dwUserID;
		propOBuy.msgPropGet.nPropID = iBoard->boardCast.nPropID;
		getValue("HoldCount",&propOBuy.msgPropGet.nHoldCount);
		getValue("PropCost",&propOBuy.msgPropGet.dwCost);
		closeRecord();
		m_pRusultService->OnDataBaseResultEvent(&propOBuy.ResultHead,DTK_GP_PROP_BUY_NEW,
			DTK_GP_PROP_BUYANDUSE_SUCCEED,sizeof(PROP_GP_O_BUY),pSourceData->uIndex,pSourceData->dwHandleID);


		int iHandleResult=DTK_GR_PROP_USE_SUCCEED;

		bres=sqlSPSetNameEx("SP_PropOpera",true);
		if(!bres)
		{
			return;
		}

		addInputParameter("@UserID",iBoard->boardCast.dwUserID);
		addInputParameter("@PropID",iBoard->boardCast.nPropID);
		addInputParameter("@PropCount",1);			//只使用1个
		TCHAR szTarget[32]={0};
		addInputParameter("@TargetName",szTarget,sizeof(szTarget));		//赠送对象或使用对象，只有当isGive=1时有效
		addInputParameter("@IsGive",0);				//是否赠送，只有=1时才是赠送
		if(0 != execStoredProc())
		{
			closeRecord();
			return ;
		}

		ret=getReturnValue();
		closeRecord();
		if(ret!=0)
			iHandleResult=DTR_GR_PROP_USE_ERR_NOPROP;
		m_pRusultService->OnDataBaseResultEvent(&oBoard.ResultHead,DTK_GPR_PROP_BIG_BRD,iHandleResult,sizeof(PROP_GPR_O_BOARDCAST),pSourceData->uIndex,pSourceData->dwHandleID);
		
	}
	catch (int &iHandleResult)
	{
		TRACE("EXCEPT CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		m_pRusultService->OnDataBaseResultEvent(&propOBuy.ResultHead,DTK_GP_PROP_BUY_NEW,
			iHandleResult,sizeof(PROP_GP_O_BUY),pSourceData->uIndex,pSourceData->dwHandleID);
	}


	


}

// 锁定机器
void CServiceDataBaseHandle::OnUserLockAccount(DataBaseLineHead * pSourceData)
{
	if( pSourceData == NULL )
		return;
	DL_GP_LockAccount * pLockData = (DL_GP_LockAccount*)pSourceData;

	bool bres=sqlSPSetNameEx("SP_UserLockOrUnlockMathine",true);
	if(!bres)
	{
		return ;
	}

	addInputParameter("@UserID", pLockData->LockAccount.dwUserID);
	addInputParameter("@CommanType", pLockData->LockAccount.dwCommanType);
	addInputParameter("@MathineCode", pLockData->LockAccount.szMathineCode, sizeof(pLockData->LockAccount.szMathineCode));
	addInputParameter("@Pwd", pLockData->LockAccount.szMD5Pass, sizeof(pLockData->LockAccount.szMD5Pass));
	addInputParameter("@MobileNo",pLockData->LockAccount.szMobileNo,sizeof(pLockData->LockAccount.szMobileNo));
	if(0 != execStoredProc())
	{
		closeRecord();
		return ;
	}
	int nRet = getReturnValue();
	closeRecord();
	int iHandleResult = 0;
	MSG_GP_S_LockAccountResult LockResult;
	::ZeroMemory(&LockResult, sizeof(LockResult));
	LockResult.dwCommanType = pLockData->LockAccount.dwCommanType;
	LockResult.dwUserID = pLockData->LockAccount.dwUserID;
	LockResult.dwCommanResult = nRet;
	switch(nRet)
	{
	case 0:
		{
			iHandleResult = DTR_GP_ERROR_UNKNOW;
			LockResult.dwCommanResult = ERR_GP_ERROR_UNKNOW;
			break;
		}
	case 1:
		{
			LockResult.dwCommanResult = ERR_GP_LOCK_SUCCESS;
			iHandleResult = DTR_GP_LOCK_SUCCESS;
			break;
		}
	case 2:
		{
			LockResult.dwCommanResult = ERR_GP_ACCOUNT_LOCKED;
			iHandleResult = ERR_GP_ACCOUNT_LOCKED;
			break;
		}
	case 3:
		{
			LockResult.dwCommanResult = ERR_GP_UNLOCK_SUCCESS;
			iHandleResult = DTR_GP_UNLOCK_SUCCESS;
			break;
		}
	case 4:
		{
			LockResult.dwCommanResult = ERR_GP_NO_LOCK;
			iHandleResult = DTR_GP_MATHINE_UNLOCK;
			break;
		}
	case 5:
		{
			LockResult.dwCommanResult = ERR_GP_CODE_DISMATCH;
			iHandleResult = DTR_GP_CODE_ERROR;
			break;
		}
	case 6:
		{
			LockResult.dwCommanResult = ERR_GP_USER_PASS_ERROR;
			iHandleResult = ERR_GP_USER_PASS_ERROR;
			break;
		}
	default:
		LockResult.dwCommanResult = ERR_GP_ERROR_UNKNOW;
		iHandleResult = DTR_GP_ERROR_UNKNOW;
		break;
	}
	m_pRusultService->OnDataBaseResultEvent(&LockResult.ResultHead, DTK_GP_LOCK_UNLOCK_ACCOUNT, iHandleResult, sizeof(LockResult),pSourceData->uIndex, pSourceData->dwHandleID);
}

//绑定手机
void CServiceDataBaseHandle::OnUserBindMobile(DataBaseLineHead * pSourceData)
{
	if( pSourceData == NULL )
		return;
	DL_GP_BindMobile * _pin = (DL_GP_BindMobile*)pSourceData;

	bool bres=sqlSPSetNameEx("SP_UserBindMobile",true);
	if(!bres)
	{
		return ;
	}

	addInputParameter("@UserID", _pin->data.dwUserID);
	addInputParameter("@CommanType", _pin->data.dwCommanType);
	addInputParameter("@MobileNo",_pin->data.szMobileNo,sizeof(_pin->data.szMobileNo));
	if(0 != execStoredProc())
	{
		closeRecord();
		return ;
	}
	int nRet = getReturnValue();
	closeRecord();

	int iHandleResult = 0;

	MSG_GP_S_BindMobileResult BindResult;
	::ZeroMemory(&BindResult, sizeof(MSG_GP_S_BindMobileResult));
	BindResult.dwCommanType = _pin->data.dwCommanType;
	BindResult.dwUserID = _pin->data.dwUserID;
	BindResult.dwCommanResult = nRet;

	m_pRusultService->OnDataBaseResultEvent(&BindResult.ResultHead, DTK_GP_BIND_MOBILE, iHandleResult, sizeof(MSG_GP_S_BindMobileResult),
		pSourceData->uIndex, pSourceData->dwHandleID);

}

// 绑定手机
void CServiceDataBaseHandle::OnSetUserLogo(DataBaseLineHead * pSourceData)
{
	if( pSourceData == NULL )
		return;
	DL_GP_SetLogo * _pin = (DL_GP_SetLogo*)pSourceData;

	CString strSql;
	strSql.Format("update TUsers set LogoID=%d where UserID=%d", _pin->LogoInfo.iLogoID, _pin->LogoInfo.iUserID);
	execSQL(strSql);
}


// 赠送道具
void CServiceDataBaseHandle::OnPropGive(DataBaseLineHead *pSourceData)
{
	PROP_GP_I_GIVE *propIGive=(PROP_GP_I_GIVE*)pSourceData;
	_TAG_PROP_GIVE * propGive=(_TAG_PROP_GIVE*)&propIGive->propGive;

	PROP_GP_O_GIVE propOGive;
	PROP_GP_O_GIVE_FOR propOGiveFor;
	ZeroMemory(&propOGive,sizeof(PROP_GP_O_GIVE));
	ZeroMemory(&propOGiveFor,sizeof(PROP_GP_O_GIVE_FOR));

	memcpy(&propOGiveFor.propBeGive.propGiveInfo,propGive,sizeof(_TAG_PROP_GIVE));
	memcpy(&propOGive.propGive,propGive,sizeof(_TAG_PROP_GIVE));

	try
	{
		bool bres=sqlSPSetNameEx("SP_PropOpera",true);
		if(!bres)
		{
			return ;
		}

		addInputParameter("@UserID",propGive->dwUserID);
		addInputParameter("@PropID",propGive->nPropID);
		addInputParameter("@PropCount",propGive->nGiveCount);			//只使用1个
		addInputParameter("@TargetName",propGive->szTargetUser,sizeof(propGive->szTargetUser));		//赠送对象或使用对象，只有当isGive=1时有效
		addInputParameter("@IsGive",1);				//是否赠送，只有=1时才是赠送
		if(0 != execStoredProc())
		{
			closeRecord();
			return ;
		}

		int ret=getReturnValue();


		if(ret==1)
		{
			closeRecord();
			throw DTR_GR_PROP_USE_ERR_NOPROP;
		}
		if(ret==2)
		{
			closeRecord();
			throw DTK_GP_PROP_GIVE_NOENOUGH;
		}
		if(ret==3)
		{
			closeRecord();
			throw DTK_GP_PROP_GIVE_ERROR;
		}

		_TAG_USERPROP * userProp=&propOGiveFor.propBeGive.propInfo;
		getValue("UserID",&userProp->dwUserID);
		getValue("PROPID",&userProp->nPropID);
		getValue("HoldCount",&userProp->nHoldCount);
		getValue("PropName",userProp->szPropName,sizeof(userProp->szPropName));
		getValue("ActionAttribute",&userProp->attribAction);
		getValue("ValueAttribute",&userProp->attribValue);
		closeRecord();
		m_pRusultService->OnDataBaseResultEvent(&propOGiveFor.ResultHead,DTK_GP_PROP_GIVE,DTK_GP_PROP_GIVE_SUCCEED_BEGIVER,sizeof(PROP_GP_O_GIVE_FOR),pSourceData->uIndex,pSourceData->dwHandleID);
		throw DTK_GP_PROP_GIVE_SUCCEED_GIVER;
		
	}
	catch(int& iHandleResult)
	{
		m_pRusultService->OnDataBaseResultEvent(&propOGive.ResultHead,DTK_GP_PROP_GIVE,iHandleResult,sizeof(PROP_GP_O_GIVE),pSourceData->uIndex,pSourceData->dwHandleID);
	}
}

// 购买VIP
void CServiceDataBaseHandle::OnPropBuyVIP(DataBaseLineHead * pSourceData)
{
	
	PROP_GP_I_BUY_VIP * propIBuy=(PROP_GP_I_BUY_VIP*)pSourceData;
	PROP_GP_O_BUY_VIP propOBuy;
	ZeroMemory(&propOBuy,sizeof(PROP_GP_O_BUY_VIP));
	memcpy(propOBuy.msgPropGet.szTargetUser,propIBuy->propBuyVIP.szTargetUser,32);

	try
	{
		
		bool bres=sqlSPSetNameEx("SP_VipOpera",true);
		if(!bres)
		{
			return ;
		}

		addInputParameter("@UserID",propIBuy->propBuyVIP.dwUserID);
		addInputParameter("@TargetNickName",propIBuy->propBuyVIP.szTargetUser,32);
		addInputParameter("@VipType",propIBuy->propBuyVIP.nPropType);

		if(0 != execStoredProc())
		{
			closeRecord();
			return ;
		}

		int ret=getReturnValue();


		if( 1== ret)
		{
			closeRecord();
			throw DTK_GP_VIP_BUY_NOMONEY;
		}
		if (2 == ret)
		{
			closeRecord();
			throw DTK_GR_VIP_BUY_NOUSER;
		}
		if (10 == ret)
		{
			closeRecord();
			throw DTK_GR_VIP_GIVE_SUCCESS;
		}
		if (0 != ret)
		{
			closeRecord();
			throw DTK_GP_VIP_BUY_ERROR;
		}
		closeRecord();
		m_pRusultService->OnDataBaseResultEvent(&propOBuy.ResultHead,DTK_GP_PROP_BUY_VIP,DTK_GR_VIP_BUY_SUCCEED,sizeof(PROP_GP_O_BUY_VIP),pSourceData->uIndex,pSourceData->dwHandleID);
		return;
		
	}
	catch (int &iHandleResult)
	{
		TRACE("EXCEPT CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		m_pRusultService->OnDataBaseResultEvent(&propOBuy.ResultHead,DTK_GP_PROP_BUY_VIP,iHandleResult,sizeof(PROP_GP_O_BUY_VIP),pSourceData->uIndex,pSourceData->dwHandleID);
	}
	return ;
}


// 购买道具
void CServiceDataBaseHandle::OnPropBuy(DataBaseLineHead * pSourceData)
{
	PROP_GP_I_BUY * propIBuy=(PROP_GP_I_BUY*)pSourceData;
	PROP_GP_O_BUY propOBuy;
	ZeroMemory(&propOBuy,sizeof(PROP_GP_O_BUY));

	propOBuy.msgPropGet.dwUserID=propIBuy->propBuy.dwUserID;

	if (!propIBuy->propBuy.bUse)
	{

	
		try
		{
			bool bres=sqlSPSetNameEx("SP_PropBuy",true);
			if(!bres)
			{
				return ;
			}

			addInputParameter("@UserID",propIBuy->propBuy.dwUserID);
			addInputParameter("@PropID",propIBuy->propBuy.nPropID);
			addInputParameter("@PropCount",propIBuy->propBuy.nPropBuyCount);
			addInputParameter("@PropCost",propIBuy->propBuy.iPropPayMoney);
			
			if(0 != execStoredProc())
			{
				closeRecord();
				return ;
			}

			int ret=getReturnValue();
			
			if(ret!=0)
			{
				closeRecord();
				throw DTK_GP_PROP_BUY_NOMONEY;
			}

			propOBuy.msgPropGet.dwUserID=propIBuy->propBuy.dwUserID;
			getValue("PROPID",&propOBuy.msgPropGet.nPropID);
			getValue("HoldCount",&propOBuy.msgPropGet.nHoldCount);
			getValue("PropName",propOBuy.msgPropGet.szPropName,sizeof(propOBuy.msgPropGet.szPropName));
			getValue("ActionAttribute",&propOBuy.msgPropGet.attribAction);
			getValue("ValueAttribute",&propOBuy.msgPropGet.attribValue);
			closeRecord();
				
			// 把该笔交易花了多少钱也传下去
			propOBuy.msgPropGet.dwCost = propIBuy->propBuy.iPropPayMoney;
			
			m_pRusultService->OnDataBaseResultEvent(&propOBuy.ResultHead,DTK_GP_PROP_BUY,DTK_GP_PROP_BUY_SUCCEED,sizeof(PROP_GP_O_BUY),pSourceData->uIndex,pSourceData->dwHandleID);

			return;
			
		}
		catch (int &iHandleResult)
		{
			TRACE("EXCEPT CATCH:%s with %s\n",__FILE__,__FUNCTION__);
			m_pRusultService->OnDataBaseResultEvent(&propOBuy.ResultHead,DTK_GP_PROP_BUY,iHandleResult,sizeof(PROP_GP_O_BUY),pSourceData->uIndex,pSourceData->dwHandleID);
		}
	}
	else
	{
		try
		{
			bool bres=sqlSPSetNameEx("SP_NewPropBuy",true);
			if(!bres)
			{
				return ;
			}

			addInputParameter("@UserID",propIBuy->propBuy.dwUserID);
			addInputParameter("@PropID",propIBuy->propBuy.nPropID);
			addInputParameter("@PropCount",propIBuy->propBuy.nPropBuyCount);

			if(0 != execStoredProc())
			{
				closeRecord();
				return ;
			}

			int ret=getReturnValue();

			if(ret!=0)
			{
				closeRecord();
				throw DTK_GP_PROP_BUY_NOMONEY;
			}

			// 操作属性此处不需要，使用这个值区分道具是在房间还是游戏端购买的
			// 0表示在房间购买，1表示在游戏端购买
			propOBuy.msgPropGet.attribAction = propIBuy->propBuy.iPropPayMoney;

			propOBuy.msgPropGet.dwUserID = propIBuy->propBuy.dwUserID;
			propOBuy.msgPropGet.nPropID = propIBuy->propBuy.nPropID;
			getValue("HoldCount",&propOBuy.msgPropGet.nHoldCount);
			getValue("PropCost",&propOBuy.msgPropGet.dwCost);
			closeRecord();

			m_pRusultService->OnDataBaseResultEvent(&propOBuy.ResultHead,DTK_GP_PROP_BUY,
				DTK_GP_PROP_BUYANDUSE_SUCCEED,sizeof(PROP_GP_O_BUY),pSourceData->uIndex,pSourceData->dwHandleID);

			return;
			
		}
		catch (int &iHandleResult)
		{
			TRACE("EXCEPT CATCH:%s with %s\n",__FILE__,__FUNCTION__);
			m_pRusultService->OnDataBaseResultEvent(&propOBuy.ResultHead,DTK_GP_PROP_BUY_NEW,
				iHandleResult,sizeof(PROP_GP_O_BUY),pSourceData->uIndex,pSourceData->dwHandleID);
		}
	}

}

// 商店道具的即时购买功能
void CServiceDataBaseHandle::OnPropBuyNew(DataBaseLineHead * pSourceData)
{
    PROP_GP_I_BUY * propIBuy=(PROP_GP_I_BUY*)pSourceData;
    PROP_GP_O_BUY propOBuy;
    ZeroMemory(&propOBuy,sizeof(PROP_GP_O_BUY));

    propOBuy.msgPropGet.dwUserID=propIBuy->propBuy.dwUserID;
    try
    {
		bool bres=sqlSPSetNameEx("SP_NewPropBuy",true);
		if(!bres)
		{
			return ;
		}

        addInputParameter("@UserID",propIBuy->propBuy.dwUserID);
        addInputParameter("@PropID",propIBuy->propBuy.nPropID);
        addInputParameter("@PropCount",propIBuy->propBuy.nPropBuyCount);
		addInputParameter("@Buse",propIBuy->propBuy.bUse);

		if(0 != execStoredProc())
		{
			closeRecord();
			return ;
		}

        int ret=getReturnValue();

        if(ret!=0)
        {
			closeRecord();
            throw DTK_GP_PROP_BUY_NOMONEY;

        }

        // 操作属性此处不需要，使用这个值区分道具是在房间还是游戏端购买的
        // 0表示在房间购买，1表示在游戏端购买
        propOBuy.msgPropGet.attribAction = propIBuy->propBuy.iPropPayMoney;

        propOBuy.msgPropGet.dwUserID = propIBuy->propBuy.dwUserID;
        propOBuy.msgPropGet.nPropID = propIBuy->propBuy.nPropID;
        getValue("HoldCount",&propOBuy.msgPropGet.nHoldCount);
		getValue("PropCost",&propOBuy.msgPropGet.dwCost);
		closeRecord();
            
		if (!propIBuy->propBuy.bUse)
		{
			m_pRusultService->OnDataBaseResultEvent(&propOBuy.ResultHead,DTK_GP_PROP_BUY_NEW,
				DTK_GP_PROP_BUY_SUCCEED,sizeof(PROP_GP_O_BUY),pSourceData->uIndex,pSourceData->dwHandleID);
		}
		else
		{
			m_pRusultService->OnDataBaseResultEvent(&propOBuy.ResultHead,DTK_GP_PROP_BUY_NEW,
				DTK_GP_PROP_BUYANDUSE_SUCCEED,sizeof(PROP_GP_O_BUY),pSourceData->uIndex,pSourceData->dwHandleID);
		}



        return;
        
    }
    catch (int &iHandleResult)
    {
        TRACE("EXCEPT CATCH:%s with %s\n",__FILE__,__FUNCTION__);
        m_pRusultService->OnDataBaseResultEvent(&propOBuy.ResultHead,DTK_GP_PROP_BUY_NEW,
                          iHandleResult,sizeof(PROP_GP_O_BUY),pSourceData->uIndex,pSourceData->dwHandleID);
    }
}


// 返回玩家自己的道具信息
void CServiceDataBaseHandle::OnPropGetList(DataBaseLineHead * pSourceData)
{
	PROP_GP_I_GETPROP *propIGet=(PROP_GP_I_GETPROP*)pSourceData;

	bool bres=sqlSPSetNameEx("SP_NewPropGetByUser",true);
	if(!bres)
	{
		return ;
	}

	try
	{
		addInputParameter("@UserID",propIGet->dwUserID);
		if(0 != execStoredProc())
		{
			closeRecord();
			return ;
		}
		PROP_GP_O_GETPROP propOGet;
		while(!adoEndOfFile())
		{
			ZeroMemory(&propOGet,sizeof(PROP_GP_O_GETPROP));
			propOGet.msgPropGet.dwUserID=propIGet->dwUserID;
			getValue("PropID",&propOGet.msgPropGet.nPropID);
			getValue("HoldCount",&propOGet.msgPropGet.nHoldCount);
			getValue("PropName",propOGet.msgPropGet.szPropName,sizeof(propOGet.msgPropGet.szPropName));
			getValue("ActionAttribute",&propOGet.msgPropGet.attribAction);
			getValue("ValueAttribute",&propOGet.msgPropGet.attribValue);
			getValue("Price",&propOGet.msgPropGet.iPrice);
			getValue("VipPrice",&propOGet.msgPropGet.iVipPrice);

			//if(propOGet.msgPropGet.nHoldCount>0)  // 注释，使用新的道具系统功能
				m_pRusultService->OnDataBaseResultEvent(&propOGet.ResultHead,DTK_GP_PROP_GETPROP,0,sizeof(PROP_GP_O_GETPROP),pSourceData->uIndex,pSourceData->dwHandleID);
			moveNext();
		}
		closeRecord();

	}
	catch (...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		closeRecord();
	}
	return;	
}

// 进入密码房间 
void CServiceDataBaseHandle::OnCheckRoomPasswd(DataBaseLineHead *pSourceData)
{
	DL_GP_I_EnterCrypticRoom *pEnterCrypticRoom = (DL_GP_I_EnterCrypticRoom *)pSourceData;
	DL_GP_O_EnterCrypticRoom stRet;
	memset(&stRet, 0, sizeof(DL_GP_O_EnterCrypticRoom));

	bool bres=sqlSPSetNameEx("SP_EnterCrypticRoom",true);
	if(!bres)
	{
		return ;
	}

	try
	{
		addInputParameter("@RoomID" ,pEnterCrypticRoom->uRoomID);
		addInputParameter("@MD5PassWord", pEnterCrypticRoom->szMD5PassWord, sizeof(pEnterCrypticRoom->szMD5PassWord));

		if(0 != execStoredProc())
		{
			closeRecord();
			return ;
		}

		stRet.nRet = getReturnValue();
		closeRecord();

		// 根据返回值来判断是否成功登录该房间
		stRet.uRoomID = pEnterCrypticRoom->uRoomID;
		if(stRet.nRet!=0)	// 不为零表示失败
		{
			m_pRusultService->OnDataBaseResultEvent(&stRet.ResultHead, DTK_GP_CHECK_ROOM_PASSWD, DTK_GP_PASSWD_ERROR, sizeof(DL_GP_O_EnterCrypticRoom),pSourceData->uIndex,pSourceData->dwHandleID);
		}
		else
		{
			m_pRusultService->OnDataBaseResultEvent(&stRet.ResultHead, DTK_GP_CHECK_ROOM_PASSWD, DTK_GP_PASSWD_SUCC, sizeof(DL_GP_O_EnterCrypticRoom),pSourceData->uIndex,pSourceData->dwHandleID);
		}
		
	}
	catch (int &iHandleResult)
	{
		closeRecord();
		TRACE("EXCEPT CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		stRet.uRoomID = 0;
		m_pRusultService->OnDataBaseResultEvent(&stRet.ResultHead, DTK_GP_CHECK_ROOM_PASSWD, DTK_GP_PASSWD_ERROR, sizeof(DL_GP_O_EnterCrypticRoom),pSourceData->uIndex,pSourceData->dwHandleID);
		//m_pRusultService->OnDataBaseResultEvent(&UserInfoRet.ResultHead,DTK_GP_GET_USER_MONEY,iHandleResult,sizeof(DL_GP_O_GetUserMoney_Ret),pSourceData->uIndex,pSourceData->dwHandleID);
	}
	return;
}

// 获取比赛信息
void CServiceDataBaseHandle::OnGetContestInfo(DataBaseLineHead *pSourceData)
{
	DL_GP_I_ContestListStruct *_in = (DL_GP_I_ContestListStruct *)pSourceData;
	DL_GP_O_ContestListStruct _out;
	memset(&_out, 0, sizeof(DL_GP_O_ContestListStruct));

	CString sql;
	sql.Format("SELECT ContestID FROM dbo.TSignUp WHERE UserID=%d AND SignState=0",_in->iUserID);
	if(0 != execSQL(sql,true))
	{
		closeRecord();
		return ;
	}

	_out._data.Num = getRecordCount();

	int count = 0;
	while(!adoEndOfFile())
	{
		getValue("ContestID",&_out._data.ContestID[count]);				
		
		count++;
		moveNext();
	}
	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_CONTEST_APPLY_LIST,0,sizeof(DL_GP_O_ContestListStruct),pSourceData->uIndex,pSourceData->dwHandleID);

	closeRecord();
}

// 设置好友分组
void CServiceDataBaseHandle::OnIMSetUser(DataBaseLineHead * pSourceData)
{
	IM_GP_I_SETUSER *imSetUser=(IM_GP_I_SETUSER*)pSourceData;

	bool bres=sqlSPSetNameEx("SP_IMSetUser",true);
	if(!bres)
	{
		return ;
	}

	addInputParameter("@UserID",imSetUser->dwUserID);
	addInputParameter("@FriendID",imSetUser->dwRemoteID);
	addInputParameter("@GroupID",imSetUser->groupID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return ;
	}
	closeRecord();
}


// 用户下线，数据库清0，还要通知各个好友
void CServiceDataBaseHandle::OnUserOffline(DataBaseLineHead * pSourceData)
{
	IM_GP_I_USEROFFLINE * imIUseroffline=(IM_GP_I_USEROFFLINE*)pSourceData;

	bool bres=sqlSPSetNameEx("SP_SetUserOnlineFlag",true);
	if(!bres)
	{
		return ;
	}

	HNLOG_M("OnUserOffline UserID:%d",imIUseroffline->dwUserID);

	addInputParameter("@UserID",imIUseroffline->dwUserID);
	// 防止账号同时登陆
	addInputParameter("@FlagID",-1);
	addInputParameter("@SocketID",imIUseroffline->iSocketID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return ;
	}
	closeRecord();

	// 更新自己下线标志
	bres=sqlSPSetNameEx("SP_IMFriendOnline",true);
	if(!bres)
	{
		return ;
	}

	addInputParameter("@UserID",imIUseroffline->dwUserID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return ;
	}
	IM_GP_O_USERINFO   imOUserinfo;
	while(!adoEndOfFile())
	{
		memset(&imOUserinfo,0,sizeof(IM_GP_O_USERINFO));
		getValue("UserID",&imOUserinfo.userInfo.dwRemoteID);				//接收人的ID，这里是自己上线后发送给好友
		imOUserinfo.userInfo.dwUserID=imIUseroffline->dwUserID;					//这是下线人的ID
		getValue("NickName",imOUserinfo.userInfo.sUserName,sizeof(imOUserinfo.userInfo.sUserName));//上线人的名称
		getValue("Sex",&imOUserinfo.userInfo.nSex);						//上线人的性别
		getValue("GroupID",&imOUserinfo.userInfo.GroupID);				//分组
		imOUserinfo.userInfo.nOnline=0;												//下线了，自然=0
		m_pRusultService->OnDataBaseResultEvent(&imOUserinfo.ResultHead,DTK_GP_IM_OFFLINE,0,sizeof(IM_GP_O_USERINFO),pSourceData->uIndex,pSourceData->dwHandleID);
		moveNext();
	}
	closeRecord();
	return;	
}

// 服务器启动清除在线标志
void CServiceDataBaseHandle::OnClearOnlineFlag(DataBaseLineHead * pSourceData)
{
	CString sql;
	sql.Format("update TUsers set OnlineFlag=0");
	execSQL(sql);
	closeRecord();

	DL_I_HALL_ClearOnlineInfo* pIn = (DL_I_HALL_ClearOnlineInfo*)pSourceData;
	sql.Format("DELETE FROM dbo.TZLoginRecord WHERE ZID=%d",pIn->ZID);
	execSQL(sql);
	closeRecord();
}

/*
函数名：	OnGetContestRoomID
备注：		客户端双击游戏列表，如果是比赛房间，则先向服务器索取比赛房间号，
			服务端通过查找T_SignUp表（报名表），选择一个报名人数最多但是未
			开赛的房间号发给客户端		
*/
void CServiceDataBaseHandle::OnGetContestRoomID(DataBaseLineHead * pSourceData)
{
	DL_GP_O_GetContestRoomID DL_OContestRoom;
	memset(&DL_OContestRoom, 0, sizeof(DL_OContestRoom));
	
	try
	{
		DL_GP_I_GetContestRoomID* pContestRoom = (DL_GP_I_GetContestRoomID*)pSourceData;
		
		bool bres=sqlSPSetNameEx("SP_ContestGetRoomID",true);
		if(!bres)
		{
			return ;
		}

		addInputParameter("@UserID", pContestRoom->_data.iUserID);
		addInputParameter("@ContestID", pContestRoom->_data.iContestID);
		if(0 != execStoredProc())
		{
			closeRecord();
			return ;
		}

		int iret = getReturnValue();
		int iCount = 0;

		if (0 == iret)
		{
			while(!adoEndOfFile() && iCount<3)
			{
				if (iCount == 0)
				{
					getValue(TEXT("RoomID"), &DL_OContestRoom.iRoomID);
					getValue(TEXT("ContestType"), &DL_OContestRoom._data.tContestInfo.iContestType);
					getValue(TEXT("ConstestNum"), &DL_OContestRoom._data.tContestInfo.iConstestNum);
					getValue(TEXT("ChampionCount"), &DL_OContestRoom._data.tContestInfo.iChampionCount);
					getValue(TEXT("BestRank"), &DL_OContestRoom._data.tContestInfo.iBestRank);
					getValue(TEXT("JoinCount"), &DL_OContestRoom._data.tContestInfo.iJoinCount);
					getValue(TEXT("ContestTime"), &DL_OContestRoom.BeginTime);
					getValue(TEXT("bNotifyG"), &DL_OContestRoom.iNotifyG);
					getValue(TEXT("MatchID"), &DL_OContestRoom.iMatchID);
					DL_OContestRoom._data.iUserID = pContestRoom->_data.iUserID;

					SYSTEMTIME systime;
					VariantTimeToSystemTime(DL_OContestRoom.BeginTime, &systime);
					CTime timeTmp(systime);
					DL_OContestRoom._data.tContestInfo.iContestTime = timeTmp.GetTime();
				}

				getValue("GiveNum",&DL_OContestRoom._data.tContestInfo.iRankAward[iCount]);
				getValue("GiveType",&DL_OContestRoom._data.tContestInfo.iAwardType[iCount]);

				iCount++;
				moveNext();
			}
		}
		closeRecord();

		m_pRusultService->OnDataBaseResultEvent(&DL_OContestRoom.ResultHead, DTK_GP_GET_CONTEST_ROOMID, iret, sizeof(DL_OContestRoom),
			pSourceData->uIndex, pSourceData->dwHandleID);

	}
	catch (int &iHandleResult)	//错误结果处理
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}	
}

void CServiceDataBaseHandle::OnContestNoticeUser(DataBaseLineHead * pSourceData)
{
	DL_I_HALL_CONTEST_NOTICE *_in = (DL_I_HALL_CONTEST_NOTICE*)pSourceData;
	DL_O_HALL_CONTEST_NOTICE_RES _out;

	try
	{
		bool bres=sqlSPSetNameEx("SP_GetApplyUser",true);
		if(!bres)
		{
			return ;
		}

		addInputParameter("@RoomID", _in->iRoomID);
		addInputParameter("@ContestID", _in->iContestID);
		if(0 != execStoredProc())
		{
			closeRecord();
			return ;
		}

		int iret = getReturnValue();

		if (0 == iret)
		{
			_out.iUserNum = getRecordCount();
			_out.pUserID = new int[_out.iUserNum];
			int iCount = 0;
			while(!adoEndOfFile())
			{
				if (iCount == 0)
				{
					COleDateTime cBeginTime;
					getValue("BeginTime",&cBeginTime);		
					getValue("RoomName",_out._result.szRoomName,sizeof(_out._result.szRoomName));
					_out._result.ContestID = _in->iContestID;
					_out._result.iRoomID = _in->iRoomID;

					SYSTEMTIME systime;
					VariantTimeToSystemTime(cBeginTime, &systime);
					CTime timeTmp(systime);
					_out._result.BeginTime = timeTmp.GetTime();
				}
				
				getValue("UserID",&_out.pUserID[iCount]);

				iCount++;
				moveNext();
			}
			m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_CONTEST_NOTICE, iret, sizeof(DL_O_HALL_CONTEST_NOTICE_RES),
				pSourceData->uIndex, pSourceData->dwHandleID);
		}
		closeRecord();

	}
	catch (int &iHandleResult)	//错误结果处理
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}	
}

void CServiceDataBaseHandle::OnGetApplyNum(DataBaseLineHead *pSourceData)
{
	DL_GP_O_UpdateApplyNum _out;
	DL_GP_I_UpdateApplyNum *pIn = (DL_GP_I_UpdateApplyNum*)pSourceData;	

	CString strSql;
	strSql.Format("SELECT COUNT(*) as ApplyNum FROM dbo.TSignUp WHERE ContestID=%d AND (SignState=0 or SignState=2)", pIn->iContestID);
	if (execSQL(strSql,true))
	{
		m_pDataBaseManage->SQLConnectReset();
		closeRecord();
		return;
	}

	getValue("ApplyNum", &_out._data.iApplyNum);
	_out._data.iContestID = pIn->iContestID;

	closeRecord();

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_GET_APPLY_NUM, 0, sizeof(DL_GP_O_UpdateApplyNum),
		pSourceData->uIndex, pSourceData->dwHandleID);
}

void CServiceDataBaseHandle::OnGetApplyAward(DataBaseLineHead *pSourceData)
{
	DL_GP_O_ContestAward _out;
	DL_GP_I_ContestAward *pIn = (DL_GP_I_ContestAward*)pSourceData;	

	CString strSql;
	strSql.Format("SELECT * FROM dbo.Web_MatchAwardConfig WHERE TypeID=(SELECT SendTypeID FROM dbo.TContestInfo WHERE ContestID=%d)", pIn->_data.ContestID);
	if (execSQL(strSql,true))
	{
		m_pDataBaseManage->SQLConnectReset();
		closeRecord();
		return;
	}

	int i=0;
	while(!adoEndOfFile())
	{
		getValue("Rank",&i);			
		if (i<=0 || i>10)
		{
			moveNext();
			continue;
		}
		getValue("GiveNum",&_out._data.iAward[i-1]);
		getValue("GiveType",&_out._data.bAwardType[i-1]);						
		moveNext();
	}

	closeRecord();

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_CONTEST_GETAWARD, 0, sizeof(DL_GP_O_ContestAward),
		pSourceData->uIndex, pSourceData->dwHandleID);
}

void CServiceDataBaseHandle::OnContestApply(DataBaseLineHead *pSourceData)
{
	DL_GP_O_ContestApply _out;
	DL_GP_I_ContestApply *pIn = (DL_GP_I_ContestApply*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_ContestApplyHALL",true);
	if(!bres)
	{
		return ;
	}

	addInputParameter("@UserID", pIn->_data.iUserID);
	addInputParameter("@TypeID", pIn->_data.iType);
	addInputParameter("@ContestID", pIn->_data.iContestID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return ;
	}
	_out._data.bResult = getReturnValue();

	getValue(TEXT("UserMoney"), &_out._data.i64WalletMoney);
	getValue(TEXT("Jewels"), &_out._data.i64Jewels);
	getValue(TEXT("RoomID"), &_out._Notify.iRoonID);
	getValue(TEXT("BeginTime"), &_out._Notify.BeginTime);
	getValue(TEXT("bNotifyG"), &_out._Notify.bNotify);
	getValue(TEXT("MatchID"), &_out._Notify.iMatchID);
	getValue(TEXT("yNum"), &_out._data.iApplyNum);
	_out._data.iContestID = pIn->_data.iContestID;
	_out._data.iUserID = pIn->_data.iUserID;
    _out._Notify.iApplyNum = _out._data.iApplyNum;

	closeRecord();
	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_CONTEST_APPLY, 0, sizeof(DL_GP_O_ContestApply),
		pSourceData->uIndex, pSourceData->dwHandleID);
}

/*
Notes		:取用户好友数量，在登录的时候调用的
			 此时，除了返回用户的好友数量，还需要一并返回在线好友的列表，如果数量不对，就要继续返回不在线的好友。
Parameter	:
DataBaseLineHead*	:数据结构指针，指向用户要更新的内容

return		:void
*/
void CServiceDataBaseHandle::OnDBIMGetUserCount(DataBaseLineHead * pSourceData)
{
	IM_GP_I_GETUSERCOUNT *imIGetUserCount=(IM_GP_I_GETUSERCOUNT*)pSourceData;

	bool bres=sqlSPSetNameEx("SP_IMOnlineFriend",true);
	if(!bres)
	{
		return ;
	}

	addInputParameter("@UserID",imIGetUserCount->dwUserID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return ;
	}

	IM_GP_O_USERLIST	imOUserList;
	MSG_IM_S_GETUSERINFO	userInfo;
	int p=0;

	ZeroMemory(&imOUserList,sizeof(IM_GP_O_USERLIST));
	imOUserList.dwListCount=0;

	while(!adoEndOfFile())
	{
		ZeroMemory(&userInfo,sizeof(MSG_IM_S_GETUSERINFO));
		imOUserList.dwListCount++;
		userInfo.dwRemoteID=imIGetUserCount->dwUserID;
		getValue("RemoteID",&userInfo.dwUserID);
		
		// 修改好友可以重复添加的BUG，由于数据库记录中没有记录好友添加的方向性
		if (imIGetUserCount->dwUserID == userInfo.dwUserID)
		{
			getValue("UserID",&userInfo.dwUserID);
		}

		getValue("NickName",userInfo.sUserName,sizeof(userInfo.sUserName));
		getValue("Sex",&userInfo.nSex);
		getValue("GroupID",&userInfo.GroupID);
		getValue("OnlineFlag",&userInfo.nOnline);
		moveNext();
		::CopyMemory(imOUserList.szData+p,&userInfo,sizeof(MSG_IM_S_GETUSERINFO));
		p+=sizeof(MSG_IM_S_GETUSERINFO);
		imOUserList.dwLength=p;

		if(p+sizeof(MSG_IM_S_GETUSERINFO)>MAX_BLOCK_MSG_SIZE)
		{
			// 长度放不下了，要先发送一组
			m_pRusultService->OnDataBaseResultEvent(&imOUserList.ResultHead,DTK_GP_IM_USERLIST,0,sizeof(IM_GP_O_USERLIST),pSourceData->uIndex,pSourceData->dwHandleID);
			p=0;
			imOUserList.dwLength=0;
		}
	}
	closeRecord();

	// 如果还有数据，则要发送完成
	if(p!=0)
		m_pRusultService->OnDataBaseResultEvent(&imOUserList.ResultHead,DTK_GP_IM_USERLIST,0,sizeof(IM_GP_O_USERLIST),pSourceData->uIndex,pSourceData->dwHandleID);
	// 向在线好友发送

	bres=sqlSPSetNameEx("SP_IMFriendOnline",true);
	if(!bres)
	{
		return ;
	}

	addInputParameter("@UserID",imIGetUserCount->dwUserID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return ;
	}

	//	Record.Open(sql);
	IM_GP_O_USERINFO   imOUserinfo;

	while(!adoEndOfFile())
	{
		memset(&imOUserinfo,0,sizeof(IM_GP_O_USERINFO));
		getValue("UserID",&imOUserinfo.userInfo.dwRemoteID);				//接收人的ID，这里是自己上线后发送给好友
		imOUserinfo.userInfo.dwUserID=imIGetUserCount->dwUserID;					//这是上线人的ID
		getValue("NickName",imOUserinfo.userInfo.sUserName,sizeof(imOUserinfo.userInfo.sUserName));//上线人的名称
		getValue("Sex",&imOUserinfo.userInfo.nSex);						//上线人的性别
		getValue("GroupID",&imOUserinfo.userInfo.GroupID);				//分组
		imOUserinfo.userInfo.nOnline=1;												//上线了，自然=1
		moveNext();
		m_pRusultService->OnDataBaseResultEvent(&imOUserinfo.ResultHead,DTK_GP_IM_ONLINE,0,sizeof(IM_GP_O_USERINFO),pSourceData->uIndex,pSourceData->dwHandleID);
	}
	closeRecord();
}

// 更新构件列表
UINT CServiceDataBaseHandle::OnGetRoomListPeoCount(DataBaseLineHead * pSourceData)
{
	// 效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GP_I_UpdateDataListStruct)) return 0;
	CServerGameListManage * pGameList=(CServerGameListManage *)(((DL_GP_I_UpdateDataListStruct *)pSourceData)->pGameList);

	// 结果数据定义
	DataBaseResultLine DT_UpdateResult;
	::memset(&DT_UpdateResult,0,sizeof(DT_UpdateResult));

	int iHandleResult=DTR_GP_GET_ROOM_LIST_PEOCOUNT_ERROR;

	// 处理数据
	if (!m_pDataBaseManage->m_pEngineSink)
	{
		return 0;
	}

	if (pGameList->UpdateRoomListPeoCount(m_pDataBaseManage->m_pEngineSink)==true)
		iHandleResult=DTR_GP_GET_ROOM_LIST_PEOCOUNT_SCUESS;

	m_pRusultService->OnDataBaseResultEvent(&DT_UpdateResult,DTK_GP_GET_ROOM_LIST_PEOCOUNT,iHandleResult,
		sizeof(DT_UpdateResult),pSourceData->uIndex,pSourceData->dwHandleID);
	return 0;
}

UINT CServiceDataBaseHandle::OnGetLogonPeopCount(DataBaseLineHead * pSourceData)
{
	bool bres=sqlSPSetNameEx("SP_GetLogOnPeopleCount",true);
	if(!bres)
	{
		return 0;
	}

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	//4.读值
	DL_GP_O_GetLogonPeopleCount struOnLinePeople;
	struOnLinePeople.logonUsers.uiLogonPeopCount = 0;
	while(!adoEndOfFile())
	{
		++struOnLinePeople.logonUsers.uiLogonPeopCount;
		moveNext();
	}

	m_pRusultService->OnDataBaseResultEvent(&struOnLinePeople.ResultHead, DTK_GP_LOGONPEOPLE_GET, 1, sizeof(IM_GP_O_USERLIST),pSourceData->uIndex,pSourceData->dwHandleID);
	closeRecord();
	return 0;
}


// 修改写Z登录记录的时机，确保网络卡的时候，不至于导致用户登录不上
UINT CServiceDataBaseHandle::OnWriteUserLogonRecord(DataBaseLineHead * pSourceData)
{
	DL_GP_WriteZRecord * pZRecord = (DL_GP_WriteZRecord*)pSourceData;

	bool bres=sqlSPSetNameEx("SP_InsertAbnormalRecord",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", pZRecord->lUserID);
	addInputParameter("@ZID", pZRecord->ZID);
	addInputParameter("@SocketID", pZRecord->iSocketID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	closeRecord();
	return 0;
}

/*
 * 用户头像上传完毕后需要更新数据库
 * 参数：数据结构指针，指向用户要更新的内容
 */
void CServiceDataBaseHandle::OnUpdateUserLogoID(DataBaseLineHead *pSourceData)
{
	UL_GP_I_UpdateUserLogo *UL_Update=(UL_GP_I_UpdateUserLogo*)pSourceData;

	bool bres=sqlSPSetNameEx("SP_SetUserLogoInfo",true);
	if(!bres)
	{
		return ;
	}

	addInputParameter("@UserID",UL_Update->dwUserID);
	addInputParameter("@LogoID",UL_Update->nIconIndex);
	addInputParameter("@LogoMD5",UL_Update->szLogoMD5,sizeof(UL_Update->szLogoMD5));
	if(0 != execStoredProc())
	{
		closeRecord();
		return ;
	}
	closeRecord();
}

/*
 * 从数据库中取用户的LOGOFILEMD5值
 * 参数：数据结构指针，指向用户要更新的内容
 */
void CServiceDataBaseHandle::OnRequestUserLogoMD5(DataBaseLineHead *pSourceData)
{
	UL_GP_I_RequestInformationLogoMD5 *UL_LogoMD5=(UL_GP_I_RequestInformationLogoMD5*)pSourceData;
	try
	{
		bool bres=sqlSPSetNameEx("SP_GetUserLogoMD5",true);
		if(!bres)
		{
			return ;
		}

		addInputParameter("@UserID",UL_LogoMD5->dwRequestUserID);

		if(0 != execStoredProc())
		{
			closeRecord();
			return ;
		}
		if(!adoEndOfFile())
		{
			closeRecord();
			throw 2;
		}

		int logoId=1;

		getValue("LogoID",&logoId);
				CString strmsg,temp;

		TCHAR buf[50]={0};

		// 上传头像其他用户看不到问题
		if(getValue("LogoFileMD5",buf,sizeof(buf))==false)
		{
			ZeroMemory(buf,sizeof(buf));
		}		

		closeRecord();
		UL_GP_I_UserLogoInformation UL_Info;
		ZeroMemory(&UL_Info,sizeof(UL_GP_I_UserLogoInformation));
		UL_Info.UserLogoInformation.dwUserID=UL_LogoMD5->dwRequestUserID;
		UL_Info.UserLogoInformation.dwUserLogoID=logoId;
		for(int i=0;i<32;i++)
			UL_Info.UserLogoInformation.szFileMD5[i]=buf[i];
		m_pRusultService->OnDataBaseResultEvent(&UL_Info.ResultHead,DTK_GP_USERLOGO_GETMD5,0,sizeof(UL_GP_I_UserLogoInformation),pSourceData->uIndex,pSourceData->dwHandleID);

	}
	catch (...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}
}



// 更新玩家返利数据
UINT CServiceDataBaseHandle::OnUpdateUserRebate(DataBaseLineHead * pSourceData)
{
	// 效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GP_I_UserRebateUpDateStruct)) 
		return 0;
	DL_GP_I_UserRebateUpDateStruct * pCmd=(DL_GP_I_UserRebateUpDateStruct *)pSourceData;

	CString sql;
	sql.Format("select * from Web_RebateInfo where UserID='%d'", pCmd->iUserID);
	execSQL(sql, true);
	
	// 结果数据定义
	DL_GP_O_UserRebateUpDateStruct UserRebate;
	::memset(&UserRebate,0,sizeof(UserRebate));
	
	// 处理数据
	int iHandleResult=DTR_GP_LIST_ERROR;//初始化
	
	// 先检测数据库连接是否还在，不在就再连
	
	getValue(TEXT("UserID"),&UserRebate.iUserID);
	getValue(TEXT("Tx_Money"),&UserRebate.iTx_Money);
	getValue(TEXT("Tx_Smony"),&UserRebate.iTx_Smony);

	iHandleResult=DTR_GP_LIST_SCUESS;//成功
	
	closeRecord();


	m_pRusultService->OnDataBaseResultEvent(&UserRebate.ResultHead,DTK_GP_BANK_UPDATE_USER_REBATE,iHandleResult,
		sizeof(UserRebate),pSourceData->uIndex,pSourceData->dwHandleID);

	return 0;
}

// 更新转账返利数据
UINT CServiceDataBaseHandle::OnUpdateRebate(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GP_I_RebateUpDateStruct)) return 0;

	execSQL(TEXT("select Min_money,VIP_Rate from Web_Config"), true);//sql语句
	//结果数据定义
	DL_GP_O_RebateUpDateStruct Rebate;
	::memset(&Rebate,0,sizeof(Rebate));
	//处理数据
	int iHandleResult=DTR_GP_LIST_ERROR;//初始化
	//先检测数据库连接是否还在，不在就再连
	
	getValue(TEXT("Min_money"),&Rebate.iMin_money);
	getValue(TEXT("VIP_Rate"),&Rebate.iVIP_Rate);

	iHandleResult=DTR_GP_LIST_SCUESS;//成功
	
	closeRecord();


	m_pRusultService->OnDataBaseResultEvent(&Rebate.ResultHead,DTK_GP_BANK_UPDATE_REBATE,iHandleResult,
		sizeof(Rebate),pSourceData->uIndex,pSourceData->dwHandleID);

	return 0;
}

// 更新构件列表
UINT CServiceDataBaseHandle::OnUpdateGameList(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GP_I_UpdateDataListStruct)) 
		return 0;
	CServerGameListManage * pGameList=(CServerGameListManage *)(((DL_GP_I_UpdateDataListStruct *)pSourceData)->pGameList);
	//结果数据定义
	DataBaseResultLine DT_UpdateResult;
	::memset(&DT_UpdateResult,0,sizeof(DT_UpdateResult));
	//处理数据
	int iHandleResult=DTR_GP_LIST_ERROR;
	if (!m_pDataBaseManage->m_pEngineSink)
		return 0;
	if (pGameList->UpdateGameListBuffer(m_pDataBaseManage->m_pEngineSink)==true)
		iHandleResult=DTR_GP_LIST_SCUESS;
	m_pRusultService->OnDataBaseResultEvent(&DT_UpdateResult,DTK_GP_UPDATE_LIST,iHandleResult,
		sizeof(DT_UpdateResult),pSourceData->uIndex,pSourceData->dwHandleID);

	return 0;
}

// 大厅登陆处理
UINT CServiceDataBaseHandle::OnPlaceLogon(DataBaseLineHead * pSourceData, UINT uLogonType)
{
	//结果数据定义
	DL_GP_O_LogonResultStruct DT_LogonResult;
	memset(&DT_LogonResult,0,sizeof(DT_LogonResult));
	DT_LogonResult.LogonResult.iCutRoomID=0;
	try
	{
		//定义变量
		int iHandleCode=ERR_GP_ERROR_UNKNOW;//执行结果

		//IP效验也放在SP中
		if (pSourceData->DataLineHead.uSize!=sizeof(DL_GP_I_LogonByNameStruct)) 
		{		
			throw iHandleCode;
		}
		//把传来的数据赋给登陆信息DL_GP_I_LogonByNameStruct
		DL_GP_I_LogonByNameStruct * pLogonInfo=(DL_GP_I_LogonByNameStruct *)pSourceData;

		CString sname=pLogonInfo->LogonInfo.szName;

		sname.TrimLeft();sname.TrimRight();
		//先处理名字效验
      //  if(pLogonInfo->LogonInfo.bLogonType!=4)   //如果是手机号码登录  不用平台判断  和手游确认过
        {
		    if (CheckChar(sname.GetBuffer(),false)==false) 
		    {
			    iHandleCode = ERR_GP_USER_NO_FIND;
			    throw iHandleCode;
		    }
        }
		//大厅登
		bool bres=sqlSPSetNameEx("SP_UserLoginPlace",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserName",sname,sname.GetLength());
		addInputParameter("@MD5Password",pLogonInfo->LogonInfo.szMD5Pass,sizeof(pLogonInfo->LogonInfo.szMD5Pass));
		addInputParameter("@UserLoginIP",pLogonInfo->szAccessIP,sizeof(pLogonInfo->szAccessIP));
		addInputParameter("@UserToken",pLogonInfo->LogonInfo.TML_SN,sizeof(pLogonInfo->LogonInfo.TML_SN));

		CString zid;
		
		zid.Format("%d",pLogonInfo->ID);
		
		addInputParameter("@ZID",zid,zid.GetLength());

		// CPU、硬盘 ID
		addInputParameter("@HardID", pLogonInfo->LogonInfo.szHardID, sizeof(pLogonInfo->LogonInfo.szHardID));

		addInputParameter("@UserIDIN", pLogonInfo->LogonInfo.iUserID);
		addInputParameter("@Forced", pLogonInfo->LogonInfo.bForced);
		addInputParameter("@LogonType", pLogonInfo->LogonInfo.bLogonType);
        ULONG uIP = CTCPSocketManage::IPToULong(pLogonInfo->szAccessIP); 
        DT_LogonResult.LogonResult.dwNowLogonIP = htonl(uIP);
        strcpy(DT_LogonResult.LogonResult.szName, pLogonInfo->LogonInfo.szName);
		strcpy(DT_LogonResult.LogonResult.szMD5Pass, pLogonInfo->LogonInfo.szMD5Pass);
		//strcpy(DT_LogonResult.LogonResult.TML_SN, pLogonInfo->LogonInfo.TML_SN);    
        
		///////////////////////////
		if(0 != execStoredProc())
		{
			closeRecord();
			throw iHandleCode;
		}

		int ret=getReturnValue();
		
		if(ret==1) //IP限制
			iHandleCode= ERR_GP_USER_IP_LIMITED;
		if(ret==2)//用户不存在
			iHandleCode= ERR_GP_USER_NO_FIND;
		if(ret==3)//帐号被禁用
			iHandleCode= ERR_GP_USER_VALIDATA;
		if(ret==4) //密码错误
			iHandleCode= ERR_GP_USER_PASS_ERROR;
		/////////////////////////////
		/// 用户已经登陆
		if(ret==5) //用户已经登陆
			iHandleCode= ERR_GP_USER_LOGON;

		//{由于锁定机器的原因，所以增加几个报错的功能
		if(ret==6)
		{
			iHandleCode = ERR_GP_ACCOUNT_LOCKED;
		}	
		// 锁定机器}
		if(ret==7)
			iHandleCode = ERR_GP_MATHINE_LOCKED;

		//玩家异地登陆 
		if(ret==8)
		{
			/*iHandleCode = ERR_GP_ALLO_PARTY;
			getValue(TEXT("UserID"),&DT_LogonResult.LogonResult.dwUserID);
			getValue(TEXT("ZJ_Number"),DT_LogonResult.LogonResult.szIDCardNo,sizeof(DT_LogonResult.LogonResult.szIDCardNo));*/
		}

		//{手机绑定
		if(ret==9)
		{
			/*iHandleCode = ERR_GP_MOBILE_VALID;
			getValue(TEXT("UserID"),&DT_LogonResult.LogonResult.dwUserID);
			getValue(TEXT("PhoneNum"),DT_LogonResult.LogonResult.szMobileNo,sizeof(DT_LogonResult.LogonResult.szMobileNo));*/
		}
		if (ret==10)
			iHandleCode = ERR_GP_VISITER_LOCKED;
		if (ret==11)
			iHandleCode = ERR_GP_LOGONTYPE_ERROR;
		/////////////////////////////
		if(ret==0)
		{
			iHandleCode=ERR_GP_LOGON_SUCCESS;
			
			getValue(TEXT("UserID"),&DT_LogonResult.LogonResult.dwUserID);
			getValue(TEXT("LogoID"),&DT_LogonResult.LogonResult.bLogoID);
			getValue(TEXT("Sex"),&DT_LogonResult.LogonResult.bBoy);
			getValue(TEXT("wallet"),&DT_LogonResult.LogonResult.i64Money);
			getValue(TEXT("BankMoney"),&DT_LogonResult.LogonResult.i64Bank);

			getValue(TEXT("viptime"),&DT_LogonResult.LogonResult.iVipTime);

			getValue(TEXT("nickName"),DT_LogonResult.LogonResult.nickName,sizeof(DT_LogonResult.LogonResult.nickName));
			getValue(TEXT("UserName"),DT_LogonResult.LogonResult.szName,sizeof(DT_LogonResult.LogonResult.szName));

			//新用户资料
			getValue(TEXT("SignDescr"),DT_LogonResult.LogonResult.szSignDescr,sizeof(DT_LogonResult.LogonResult.szSignDescr));
			getValue(TEXT("PhoneNum"),DT_LogonResult.LogonResult.szMobileNo,sizeof(DT_LogonResult.LogonResult.szMobileNo));

			getValue(TEXT("HAVENEWS"),&DT_LogonResult.LogonResult.bLoginBulletin);
			getValue(TEXT("LockMathine"), &DT_LogonResult.LogonResult.iLockMathine);	//返回锁定状态
			getValue(TEXT("BindMobile"), &DT_LogonResult.LogonResult.iBindMobile);	//返回手机的绑定状态

			getValue(TEXT("AddFriendType"),&DT_LogonResult.LogonResult.iAddFriendType);

			getValue(TEXT("Lotteries"),&DT_LogonResult.LogonResult.iLotteries);		//奖券
			getValue(TEXT("LogonType"),&DT_LogonResult.LogonResult.iLogonType);		//登录类型
			getValue(TEXT("Jewels"),&DT_LogonResult.LogonResult.iJewels);			//钻石
			getValue(TEXT("LockJewels"),&DT_LogonResult.LogonResult.iLockJewels);	//冻结钻石
			getValue(TEXT("HeadUrl"),DT_LogonResult.LogonResult.szHeadUrl,sizeof(DT_LogonResult.LogonResult.szHeadUrl));			//图像URL

			getValue(TEXT("CutRoomID"),&DT_LogonResult.LogonResult.iCutRoomID);	//断线重连房间号
		}
		closeRecord();
		//处理结果
		m_pRusultService->OnDataBaseResultEvent(&DT_LogonResult.ResultHead,uLogonType,iHandleCode,
			sizeof(DT_LogonResult),pSourceData->uIndex,pSourceData->dwHandleID);
	
	}
	catch (int &iHandleResult)	//错误结果处理
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		//重启数据库连接
		if (iHandleResult == ERR_GP_ERROR_UNKNOW)
		{
			m_pDataBaseManage->SQLConnectReset();
		}
		m_pRusultService->OnDataBaseResultEvent(&DT_LogonResult.ResultHead,uLogonType,iHandleResult,
			sizeof(DT_LogonResult),pSourceData->uIndex,pSourceData->dwHandleID);
	}

	return 0;
}



// 效验字符
bool CServiceDataBaseHandle::CheckChar(TCHAR * szChar, bool bStrCheck)
{
	int iLength=lstrlen(szChar);
	for (int i=0;i<iLength;)
	{
		if (((szChar[i]>=-1)&&(szChar[i]<47))||(szChar[i]==127)||(szChar[i]=='\\')||(szChar[i]=='/')) 
			return false;
		byte a=szChar[i];
		byte b=szChar[i+1];
		//判断是否汉字
		if((a>=176&&a<=247&&b>=160&&b<=254)||//GB2312
			(((a>=129&&a<=255)||(a>=64&&a<=126))&&b>=161&&b<=254)||///GB5
			(a>=129&&a<=254&&b>=64&&b<=254))
		{
			i += 2;
			continue;
		}
		else if(szChar[i] >='a'&&szChar[i]<='z'||szChar[i] >='A'&&szChar[i]<='Z' || szChar[i] >= '0'&&szChar[i] <= '9')
		{
			i += 1;
			continue;
		}
		return false;
	}

	if (bStrCheck)
	{
		const TCHAR * szLimitName[]={TEXT("快乐"),TEXT("江泽民"),TEXT("姚则国"),TEXT("毛泽东"),TEXT("系统"),TEXT("管理"),
			TEXT("阴道"),TEXT("日"),TEXT("你妈"),TEXT("叼"),TEXT("我日"),TEXT("FUCK"),TEXT("妈B"),TEXT("操"),TEXT("靠"),
			TEXT("外管"),TEXT("内管"),TEXT("系统"),TEXT("金币"),TEXT("游戏中心"),TEXT("消息"),
			TEXT("中奖"),TEXT("信息"),TEXT("客服"),TEXT("腾奥"),TEXT("腾澳"),TEXT("腾澳"),TEXT("管理员"),TEXT("官方"),TEXT("game0437"),TEXT("客服中心"),TEXT("客服服务")
			TEXT("GM"),TEXT(" "),TEXT("名字"),TEXT("昵称"),TEXT("名称"),TEXT("畜生"),TEXT("去死")
		};
		CString strChar=szChar;
		strChar.MakeUpper();
		for (int i=0;i<sizeof(szLimitName)/sizeof(szLimitName[0]);i++)
		{
			if (strChar.Find(szLimitName[i])!=-1) return false;
		}
	}
	return true;
}


// 检测数据库连接
UINT CServiceDataBaseHandle::OnCheckDataConnect(DataBaseLineHead * pSourceData)
{
	try 
	{ 
		// 检测连接
		m_pDataBaseManage->CheckSQLConnect();

		// 处理结果
		DataBaseResultLine ResultHead;
		m_pRusultService->OnDataBaseResultEvent(&ResultHead,DTK_GP_CHECK_CONNECT,0,sizeof(ResultHead),0,0);
	} 
	catch (...) {TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);}

	return 0;
}

// 修改用户资料
UINT CServiceDataBaseHandle::OnUpdateUserInfo(DataBaseLineHead* pSourceData,UINT nType)
{
	// 结果数据定义
	DL_GR_O_UserInfoStruct DT_UpdateResult;
	memset(&DT_UpdateResult,0,sizeof(DL_GR_O_UserInfoStruct));

	int iHandleCode = nType;

	if (pSourceData->DataLineHead.uSize != sizeof(DL_GP_I_UserInfoStruct))
	{
		throw DTR_GP_ERROR_UNKNOW;
	}

	DL_GP_I_UserInfoStruct* pUpdateStruct = (DL_GP_I_UserInfoStruct*)pSourceData;
	MSG_GP_S_UserInfo* pUserNewInfo = &pUpdateStruct->UserInfo;

	try
	{
		//大厅登录
		bool bres=sqlSPSetNameEx("SP_UpdateUserDetail",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID",pUserNewInfo->dwUserID);

		if (nType == DTK_GP_UPDATE_USERINFO_BASE)
		{
			addInputParameter("@UpdateType",1);
		}
		else
		{
			addInputParameter("@UpdateType",2);
		}

		addInputParameter("@UserPassword",pUserNewInfo->szMD5Pass,sizeof(pUserNewInfo->szMD5Pass));
		addInputParameter("@nickName",pUserNewInfo->nickName,sizeof(pUserNewInfo->nickName));
		addInputParameter("@Sex",pUserNewInfo->bBoy);
		addInputParameter("@SignDescr",pUserNewInfo->szSignDescr,sizeof(pUserNewInfo->szSignDescr));
		addInputParameter("@PhoneNum",pUserNewInfo->szMobileNo,sizeof(pUserNewInfo->szMobileNo));

		if(0 != execStoredProc())
		{
			closeRecord();
			throw DTR_GP_ERROR_UNKNOW;
		}

		int ret = getReturnValue();

		if(ret == 1)
		{
			iHandleCode = DTR_GP_UPDATE_USERINFO_ACCEPT;
		}
		else
		{
			iHandleCode = DTR_GP_UPDATE_USERINFO_NOTACCEPT;
		}

		//处理结果
		m_pRusultService->OnDataBaseResultEvent(&DT_UpdateResult.ResultHead,nType,iHandleCode,
			sizeof(DL_GR_O_UserInfoStruct),pSourceData->uIndex,pSourceData->dwHandleID);
		closeRecord();
		
	}
	catch(int &iHandleResult)
	{

		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		//处理结果
		m_pRusultService->OnDataBaseResultEvent(&DT_UpdateResult.ResultHead,nType,iHandleResult,
			sizeof(DL_GR_O_UserInfoStruct),pSourceData->uIndex,pSourceData->dwHandleID);
	}


	return 0;
}
//用户账号关联手机号
//UINT CServiceDataBaseHandle::OnAddUserPhoneNum(DataBaseLineHead * pSourceData)
//{
//    try
//    {
//        int iHandleCode=DTR_GP_ERROR_UNKNOW;
//        DL_I_HALL_AddUserPhoneNum* _p = (DL_I_HALL_AddUserPhoneNum*)pSourceData;
//        DL_GP_O_AddUserPhoneNum _out;
//        ZeroMemory(&_out, sizeof(DL_GP_O_AddUserPhoneNum));
//
//        bool bres=sqlSPSetNameEx("SP_AddUserPhoneNum",true);
//        if(!bres)
//        {
//            return 0;
//        }
//
//        addInputParameter("@UserID", _p->_data.UserID);
//        addInputParameter("@PhoneNum",_p->_data.szPhoneNum,sizeof(_p->_data.szPhoneNum));
//        WriteLog("SP_AddUserPhoneNum 参数 UserID=%d,szPhoneNum=%s",_p->_data.UserID,_p->_data.szPhoneNum);
//        if(0 != execStoredProc())
//        {
//            closeRecord();
//            return 0;
//        }
//        int iRet = getReturnValue();
//        iHandleCode = iRet;
//        WriteLog("ret=%d",iRet);
//        m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_USER_ADDPHONENUM, iHandleCode, sizeof(DL_GP_O_AddUserPhoneNum),pSourceData->uIndex,pSourceData->dwHandleID);
//
//        closeRecord();
//
//    }
//    catch(...)
//    {
//        TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
//    }
//
//    return 0;
//}
//修改密码
UINT CServiceDataBaseHandle::OnUpdateUserPwd(DataBaseLineHead* pSourceData)
{
	//结果数据定义
	DL_GP_O_ChPasswordStruct DT_UpdateResult;
	memset(&DT_UpdateResult,0,sizeof(DL_GP_O_ChPasswordStruct));

	int iHandleCode = DTR_GP_UPDATE_USERINFO_PWD;

	if (pSourceData->DataLineHead.uSize != sizeof(DL_GP_I_ChPasswordStruct))
	{
		throw DTR_GP_ERROR_UNKNOW;
	}

	DL_GP_I_ChPasswordStruct* pUpdateStruct = (DL_GP_I_ChPasswordStruct*)pSourceData;
	MSG_GP_S_ChPassword* pUserChPwd = &pUpdateStruct->ChPwd;

	try
	{
		//大厅登录
		bool bres=sqlSPSetNameEx("SP_UpdateUserPassword",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID",pUserChPwd->dwUserID);
		addInputParameter("@HardID",pUserChPwd->szHardID,sizeof(pUserChPwd->szHardID));
		addInputParameter("@UserOldPassword",pUserChPwd->szMD5OldPass,sizeof(pUserChPwd->szMD5OldPass));
		addInputParameter("@UserNewPassword",pUserChPwd->szMD5NewPass,sizeof(pUserChPwd->szMD5NewPass));

		if(0 != execStoredProc())
		{
			closeRecord();
			throw DTR_GP_ERROR_UNKNOW;
		}

		int ret = getReturnValue();

		if(ret == 1)
		{
			iHandleCode = DTR_GP_UPDATE_USERINFO_ACCEPT;
		}
		else
		{
			iHandleCode = DTR_GP_UPDATE_USERINFO_NOTACCEPT;
		}

		//处理结果
		m_pRusultService->OnDataBaseResultEvent(&DT_UpdateResult.ResultHead,DTR_GP_UPDATE_USERINFO_PWD,iHandleCode,
			sizeof(DL_GP_O_ChPasswordStruct),pSourceData->uIndex,pSourceData->dwHandleID);
		closeRecord();
	}
	catch(int &iHandleResult)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		//处理结果
		m_pRusultService->OnDataBaseResultEvent(&DT_UpdateResult.ResultHead,DTR_GP_UPDATE_USERINFO_PWD,DTR_GP_UPDATE_USERINFO_NOTACCEPT,
			sizeof(DL_GP_O_ChPasswordStruct),pSourceData->uIndex,pSourceData->dwHandleID);
	}

	return 0;
}

UINT CServiceDataBaseHandle::OnCharmExchangeList(DataBaseLineHead * pSourceData)
{
	try
	{
		int iHandleCode=DTR_GP_ERROR_UNKNOW;
		DataBaseLineHead* _p = (DataBaseLineHead*)pSourceData;
		DL_GP_O_CharmExchangeList _out;
		ZeroMemory(&_out, sizeof(_out));

		bool bres=sqlSPSetNameEx("SP_CharmExchange_Get",true);
		if(!bres)
		{
			return 0;
		}

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		int iRet = getReturnValue();
		iHandleCode = iRet;

		while(!adoEndOfFile())
		{
			ZeroMemory(&_out,sizeof(DL_GP_O_CharmExchangeList));

			getValue("id",&_out.date.iID);
			getValue("CharmValue",&_out.date.iPoint);
			getValue("PropID",&_out.date.iPropID);
			getValue("PropCount",&_out.date.iPropCount);

			m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_CHARMEXCHANGE_LIST, 0, sizeof(DL_GP_O_CharmExchangeList),pSourceData->uIndex,pSourceData->dwHandleID);
			moveNext();
		}

		ZeroMemory(&_out,sizeof(DL_GP_O_CharmExchangeList));
		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_CHARMEXCHANGE_LIST, 1, sizeof(DL_GP_O_CharmExchangeList), pSourceData->uIndex, pSourceData->dwHandleID);

		closeRecord();

	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}

UINT CServiceDataBaseHandle::OnCharmExchange(DataBaseLineHead * pSourceData)
{
	try
	{
		int iHandleCode=DTR_GP_ERROR_UNKNOW;
		DL_GP_I_CharmExchange* _p = (DL_GP_I_CharmExchange*)pSourceData;
		DL_GP_O_CharmExchangeList _out;
		ZeroMemory(&_out, sizeof(DL_GP_O_CharmExchangeList));

		bool bres=sqlSPSetNameEx("SP_CharmExchange_Do",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID", _p->date.iUserID);
		addInputParameter("@id", _p->date.iID);

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		int iRet = getReturnValue();
		iHandleCode = iRet;

		ZeroMemory(&_out,sizeof(DL_GP_O_CharmExchangeList));
		getValue("id",&_out.date.iID);
		getValue("CharmValue",&_out.date.iPoint);
		getValue("PropID",&_out.date.iPropID);
		getValue("PropCount",&_out.date.iPropCount);

		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_CHARMEXCHANGE, iHandleCode, sizeof(DL_GP_O_CharmExchangeList),pSourceData->uIndex,pSourceData->dwHandleID);

		closeRecord();

	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}


UINT CServiceDataBaseHandle::OnSetFriendType(DataBaseLineHead * pSourceData)
{
	try
	{
		int iHandleCode=DTR_GP_ERROR_UNKNOW;
		DL_GP_I_SetFriendType* _p = (DL_GP_I_SetFriendType*)pSourceData;

		bool bres=sqlSPSetNameEx("SP_FriendType_Set",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID", _p->iUserID);
		addInputParameter("@Type", _p->iType);

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		int iRet = getReturnValue();
		iHandleCode = iRet;
		closeRecord();

	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}

// 查询玩家的ID或昵称
UINT CServiceDataBaseHandle::OnQueryUserNickNameID(DataBaseLineHead* pSourceData)
{
	// 效验数据
	if (pSourceData->DataLineHead.uSize != sizeof(DL_GP_I_GetNickNameID))
	{
		throw DTR_GP_ERROR_UNKNOW;
	}

	DL_GP_I_GetNickNameID* pInData =(DL_GP_I_GetNickNameID*)pSourceData;

	// 初始化结果
	DL_GP_O_GetNickNameID _out;
	memset(&_out,0,sizeof(DL_GP_O_GetNickNameID));
	_out._data._nType = pInData->_data._nType;
	_out._data._nUseful = pInData->_data._nUseful;

	bool bres=sqlSPSetNameEx("SP_GetNickNameID",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter(TEXT("@UserID"),pInData->_data._user_id);
	addInputParameter(TEXT("@NickName"),pInData->_data._nickname,sizeof(pInData->_data._nickname));
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	int ret = getReturnValue();
	
	if(ret == 0) //操作成功
	{
		getValue("UserID",&_out._data._user_id);
		getValue("NickName",_out._data._nickname,sizeof(_out._data._nickname));
	}

	closeRecord();
	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_UPDATE_NICKNAMEID,
		0,sizeof(DL_GP_O_GetNickNameID),pSourceData->uIndex,pSourceData->dwHandleID);

	return 0;
}


//关于银行处理
UINT CServiceDataBaseHandle::OnBankOpen(DataBaseLineHead* pSourceData)
{
	//效验数据

	if (pSourceData->DataLineHead.uSize != sizeof(DL_GP_I_BankOpen))
	{
		throw DTR_GP_BANK_OPEN_ERROR; //接受的结构和定义的是否一致
	}

	DL_GP_I_BankOpen* pOpenWallet = (DL_GP_I_BankOpen*)pSourceData;
	DL_GP_O_BankOpen OpenWallet;///打开结果
	memset(&OpenWallet,0,sizeof(OpenWallet));

	try
	{
		bool bres=sqlSPSetNameEx("SP_GetMoney",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID",pOpenWallet->dwUserID);

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}

		int ret=getReturnValue();

		if(ret!=0)
		{
			closeRecord();
			m_pRusultService->OnDataBaseResultEvent(&OpenWallet.ResultHead, DTK_GP_BANK_OPEN, DTR_GP_BANK_OPEN_ERROR,
				sizeof(DL_GP_O_BankOpen),pSourceData->uIndex,pSourceData->dwHandleID);
		}

		getValue("MoneyInBank",&OpenWallet._data.i64Bank);
		getValue("MoneyInWallet",&OpenWallet._data.i64Wallet);
		getValue("TwoPassword",OpenWallet._data.szTwoPassword,sizeof(OpenWallet._data.szTwoPassword));
		getValue("BankVer",&OpenWallet._data.nVer);


		closeRecord();
		OpenWallet._data.dwUserID = pOpenWallet->dwUserID;
		m_pRusultService->OnDataBaseResultEvent(&OpenWallet.ResultHead, DTK_GP_BANK_OPEN, DTR_GP_BANK_OPEN_SUC,
			sizeof(DL_GP_O_BankOpen),pSourceData->uIndex,pSourceData->dwHandleID);

	}
	catch (int iHandleResult)
	{
		TRACE("EXCEPT CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		m_pRusultService->OnDataBaseResultEvent(&OpenWallet.ResultHead, DTK_GP_BANK_OPEN, iHandleResult,
			sizeof(DL_GP_O_BankOpen),pSourceData->uIndex,pSourceData->dwHandleID);
	}

	return 0;
}

// 查询钱包
UINT CServiceDataBaseHandle::OnBankGetGameWallet(DataBaseLineHead* pSourceData)
{
	try
	{
		DL_GP_I_BankGetWallet* _p = (DL_GP_I_BankGetWallet*)pSourceData;
		DL_GP_O_BankGetWallet _out;
		ZeroMemory(&_out, sizeof(_out));

		bool bres=sqlSPSetNameEx("SP_BankOpWallet",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID", _p->_UserID);

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		int iRet = getReturnValue();

		while(!adoEndOfFile())
		{
			ZeroMemory(&_out,sizeof(DL_GP_O_BankGetWallet));

			getValue("GameID",&_out._data._game_id);
			getValue("ComName",_out._data._game_name,sizeof(_out._data._game_name));
			getValue("WalletMoney",&_out._data._money);

			m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_BANK_GETGM_WALLET,0,sizeof(DL_GP_O_BankGetWallet),pSourceData->uIndex,pSourceData->dwHandleID);
			moveNext();
		}

		ZeroMemory(&_out,sizeof(DL_GP_O_BankGetWallet));
		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_BANK_GETGM_WALLET,1,sizeof(DL_GP_O_BankGetWallet),pSourceData->uIndex,pSourceData->dwHandleID);
		closeRecord();
	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}

// 银行存取
UINT CServiceDataBaseHandle::OnBankCheck(DataBaseLineHead* pSourceData)
{
	try
	{
		DL_GP_I_BankCheck* _p = (DL_GP_I_BankCheck*)pSourceData;
		DL_GP_O_BankCheck _out;
		ZeroMemory(&_out, sizeof(_out));
		_out._data = _p->_data;

		if(_out._data._money < 0)
		{
			return 0;
		}

		bool bres=sqlSPSetNameEx("SP_BankOpNormal",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID", _p->_data._user_id);
		addInputParameter("@GameID", _p->_data._game_id);
		addInputParameter("@OperateType", _p->_data._operate_type);
		addInputParameter("@Money", _p->_data._money);
		addInputParameter("@MD5Pass", _p->_data._szMD5Pass,sizeof(_p->_data._szMD5Pass));

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		int iRet = getReturnValue();

		if (iRet == HC_GP_BANK_CHECK_SUC)
		{
			getValue("MoneyOp", &_out._data._money);
		}
		_out.ResultHead.uHandleRusult = iRet;

		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_BANK_CHECK,iRet,
			sizeof(DL_GP_O_BankCheck),pSourceData->uIndex,pSourceData->dwHandleID);
		closeRecord();
	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}


// 提取或者存储返利
UINT CServiceDataBaseHandle::OnBankTransferRebate(DataBaseLineHead* pSourceData)
{
	try
	{
		DL_GP_I_UserRebateSetStruct* pCmd = (DL_GP_I_UserRebateSetStruct*)pSourceData;
		DL_GP_O_UserRebateSetStruct cmd;
		bool bres=sqlSPSetNameEx("proc_RebateUpdate",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID", pCmd->iUserID);
		addInputParameter("@TypeN", pCmd->bTag);
		addInputParameter("@num", pCmd->iMoney);

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		int iRet = getReturnValue();
		cmd.bTag = pCmd->bTag;
		cmd.iUserID = pCmd->iUserID;
		if (iRet == 1)//成功
		{
			cmd.bResult = true;		
		}
		else if (iRet == -1)//失败
		{
			cmd.bResult = false;
		}
		else
		{
			return 0;
		}
		getValue("Msg", cmd.Msg, sizeof(cmd.Msg));
		cmd.iMoney = 0;
		m_pRusultService->OnDataBaseResultEvent(&cmd.ResultHead,DTK_GP_BANK_SET_USER_REBATE,iRet,
			sizeof(DL_GP_O_UserRebateSetStruct),pSourceData->uIndex,pSourceData->dwHandleID);
		closeRecord();
	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}

// 银行存取
UINT CServiceDataBaseHandle::OnBankTransfer(DataBaseLineHead* pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize != sizeof(DL_GP_I_BankTransfer))
	{
		return 0;
	}

	DL_GP_I_BankTransfer* pTransfer = (DL_GP_I_BankTransfer*)pSourceData;
	DL_GP_O_BankTransfer _out;
	::memset(&_out,0,sizeof(DL_GP_O_BankTransfer));
	_out._data = pTransfer->_data;

	if(_out._data.i64Money <= 0)
	{
		return 0;
	}

	// 增加银行“全转”功能
	// 32位数据转到64位数据
	UINT u32Data = 0;

	try
	{
		bool bres;
		if(1 == _out._data.uHandleCode)
		{
			bres=sqlSPSetNameEx("SP_TransferAllMoney",true);
		}
		else
		{
			bres=sqlSPSetNameEx("SP_TransferMoney",true);
		}

		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID",pTransfer->_data.UserID);
		addInputParameter("@UseDestID",pTransfer->_data.bUseDestID?1:0);
		addInputParameter("@DestUserID",pTransfer->_data.destUserID);
		addInputParameter("@DestNickName",pTransfer->_data.szDestNickName,sizeof(pTransfer->_data.szDestNickName));//密码
		if(0 == pTransfer->_data.uHandleCode)     // 全转功能
			addInputParameter("@TranMoney",pTransfer->_data.i64Money);
		addInputParameter("@TranTax",pTransfer->_data.bTranTax);
		addInputParameter("@MD5Pass",pTransfer->_data.szMD5Pass, sizeof(pTransfer->_data.szMD5Pass));//密码

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		int ret = getReturnValue();
		UINT uResult = HC_GP_BANK_TRANSFER_SUC;

		switch(ret)
		{
		case 0: //转帐成功
			{
				CopyMemory(_out._data.szDestNickName,pTransfer->_data.szDestNickName,sizeof(_out._data.szDestNickName));
				getValue("DestUserID",&_out._data.destUserID);
				getValue("TranMoney",&_out._data.i64Money);
				getValue("ActualTransfer",&_out._data.i64ActualTransfer);
				break;
			}
		case 1:	//密码错误
			{
				uResult = HC_GP_BANK_TRANSFER_PASSWORD_ERROR;
				break;
			}
		case 2: //银行中钱少于多少不能转帐
			{
				uResult = HC_GP_BANK_TRANSFER_TOTAL_LESS;
				getValue("LessAllMoney",&_out._data.i64ActualTransfer);
				break;
			}
		case 3: //单笔转帐必须大于多少
			{
				uResult = HC_GP_BANK_TRANSFER_TOO_LESS;
				getValue("LessMoney",&_out._data.i64ActualTransfer);
			}
			break;
		case 4: //单笔转帐是某数的整数倍
			{
				uResult = HC_GP_BANK_TRANSFER_MULTIPLE;
				getValue("Multiple",&u32Data);
				_out._data.i64ActualTransfer = u32Data;
			}
			
			break;
		case 5: //银行里只有多少钱，本次想要转多少钱，不够转
			{
				uResult = HC_GP_BANK_TRANSFER_NOT_ENOUGH;
				getValue("TransMoney", &_out._data.i64Money);
				getValue("BankMoney",&_out._data.i64ActualTransfer);
			}
			break;
		case 6: ///<目标帐户不存在
			{
				uResult = HC_GP_BANK_TRANSFER_NO_DEST;
			}
			break;
		case 7: //当天转账的次数太多了;
			{
				uResult = HC_GP_BANK_TRANSFER_TOO_MANY_TIME;
			}
			break;
		default:
			return 0;
		}
		closeRecord();
		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_BANK_TRANSFER,uResult,
			sizeof(DL_GP_O_BankTransfer),pSourceData->uIndex,pSourceData->dwHandleID);
	}
	catch (...)
	{
		return 0;
	}

	return 0;
}

// 查询银行转账记录
UINT CServiceDataBaseHandle::OnBankTransRecord(DataBaseLineHead* pSourceData)
{
	DL_GP_I_BankTransRecord* pTransfer = (DL_GP_I_BankTransRecord*)pSourceData;
	DL_GP_O_BankTransRecord _out;

	if (pTransfer->_data.dwUserID == 0)
	{
		return 0;
	}

	try
	{
		bool bres=sqlSPSetNameEx("SP_GetTransferRecord",true);
		if(!bres)
		{
			return 0;
		}
		addInputParameter("@UserID",pTransfer->_data.dwUserID);
		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}

		int ret = getReturnValue();
		if (ret > 0)
		{
			closeRecord();
			return 0;
		}

		_out.iCount = getRecordCount();

		_out._data = new TMSG_GP_BankTranRecord[_out.iCount];

		int iCount = 0;
		while(!ret && !adoEndOfFile() && iCount<_out.iCount)
		{
			_out._data[iCount].nSeriNo = iCount;
			getValue("UserID", &_out._data[iCount].dwUserID);
			getValue("DestUserID", &_out._data[iCount].destUserID);
			getValue("TransferMoney", &_out._data[iCount].i64Money);
			getValue("ActualTransfer",&_out._data[iCount].i64ActualTransfer);
			getValue("TransTime",&_out.datatime[iCount]);
			getValue("UserName",_out._data[iCount].szNickName,sizeof(_out._data[iCount].szNickName));
			getValue("UserNameZZ",_out._data[iCount].szDestNickName,sizeof(_out._data[iCount].szDestNickName));

			iCount++;
			moveNext();
		}
		
		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_BANK_TRANS_RECORD,0,
			sizeof(DL_GP_O_BankTransRecord),pSourceData->uIndex,pSourceData->dwHandleID);

		closeRecord();
	}
	catch (...)
	{
		return 0;
	}

	return 0;
}

// 存单业务
UINT CServiceDataBaseHandle::OnBankDeposit(DataBaseLineHead* pSourceData)
{
	try
	{
		DL_GP_I_BankDeposit* _p = (DL_GP_I_BankDeposit*)pSourceData;
		DL_GP_O_BankDeposit _out;
		ZeroMemory(&_out, sizeof(_out));
		_out._data = _p->_data;

		int iRet;

		if (_p->_data._operate_type == 1)
		{
			bool bres=sqlSPSetNameEx("SP_Bank_NewDeposit",true);
			if(!bres)
			{
				return 0;
			}
			addInputParameter("@CreaterUserID", _p->_data._user_id);
			addInputParameter("@Money", _p->_data._money);
			addInputParameter("@MD5Pass",_p->_data._szMD5Pass,sizeof(_p->_data._szMD5Pass));

			if(0 != execStoredProc())
			{
				closeRecord();
				return 0;
			}
			iRet = getReturnValue();

			if (iRet == HC_GP_BANK_DEPOSIT_SUC)
			{
				getValue("DepositID", _out._data._deposit_id, sizeof(_out._data._deposit_id));
				getValue("DepositPassWord", _out._data._deposit_password, sizeof(_out._data._deposit_password));
				_out._data._state = 0;
				getValue("CDSC", &_out._data._csds);
				getValue("Money", &_out._data._money);
			}
		}
		else if (_p->_data._operate_type == 2)
		{
			bool bres=sqlSPSetNameEx("SP_Bank_UseDeposit",true);
			if(!bres)
			{
				return 0;
			}

			addInputParameter("@UserID", _p->_data._user_id);
			CString s = _p->_data._deposit_id;
			addInputParameter("@DepositID", s.GetBuffer(), sizeof(TCHAR)*s.GetLength());
			s = _p->_data._deposit_password;
			addInputParameter("@DepositPwd", s.GetBuffer(), sizeof(TCHAR)*s.GetLength());

			if(0 != execStoredProc())
			{
				closeRecord();
				return 0;
			}
			iRet = getReturnValue();

			if (iRet == HC_GP_BANK_DEPOSIT_SUC)
			{
				_out._data._state = 1;
				getValue("UserID", &_out._data._user_id);
				getValue("NickName", _out._data._szNickName,sizeof(_out._data._szNickName));
				getValue("Money", &_out._data._money);
			}
		}

		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_BANK_DEPOSIT,iRet,
			sizeof(DL_GP_O_BankDeposit),pSourceData->uIndex,iRet);
		closeRecord();
	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}

// 查询存单
UINT CServiceDataBaseHandle::OnBankQueryDeposits(DataBaseLineHead* pSourceData)
{
	try
	{
		DL_GP_I_BankQueryDeposit* _p = (DL_GP_I_BankQueryDeposit*)pSourceData;
		DL_GP_O_BankQueryDeposit _out;
		ZeroMemory(&_out, sizeof(_out));

		bool bres=sqlSPSetNameEx("SP_Bank_QueryDeposits",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID", _p->_UserID);

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		int iRet = getReturnValue();

		int nSeriNo = 0;

		while(!adoEndOfFile())
		{
			ZeroMemory(&_out,sizeof(DL_GP_O_BankQueryDeposit));

			_out._data._serial_no = nSeriNo++;
			getValue("DepositID",_out._data._deposit_id,sizeof(_out._data._deposit_id));
			getValue("DepositPassWord",_out._data._deposit_password,sizeof(_out._data._deposit_password));
			getValue("Money",&_out._data._money);
			getValue("UserID",&_out._data._user_id);
			getValue("GreateTime",&_out._data._create_time);
			getValue("CDSC",&_out._data._csds);
			getValue("State",&_out._data._state);
			getValue("UsedUserID",&_out._data._used_user_id);

			m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_BANK_QUERY_DEPOSIT,0,sizeof(DL_GP_O_BankQueryDeposit),pSourceData->uIndex,pSourceData->dwHandleID);
			moveNext();
		}

		ZeroMemory(&_out,sizeof(DL_GP_O_BankQueryDeposit));
		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_BANK_QUERY_DEPOSIT,1,sizeof(DL_GP_O_BankQueryDeposit), pSourceData->uIndex, pSourceData->dwHandleID);
		closeRecord();
	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}

// 修改银行密码
UINT CServiceDataBaseHandle::OnBankChangePassword(DataBaseLineHead* pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize != sizeof(DL_GP_I_BankChPwd))
	{
		return 0;
	}

	DL_GP_I_BankChPwd* in_ptr = (DL_GP_I_BankChPwd*)pSourceData;
	DL_GP_O_BankChPwd _out;

	try
	{
		bool bres=sqlSPSetNameEx("SP_ChangeBankPasswd",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID",in_ptr->_data._user_id);
		addInputParameter("@MD5PassOld",in_ptr->_data._MD5Pass_old, sizeof(in_ptr->_data._MD5Pass_old));//旧密码
		addInputParameter("@MD5PassNew",in_ptr->_data._MD5Pass_new, sizeof(in_ptr->_data._MD5Pass_new));//新密码
		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}

		int nRet = getReturnValue();

		closeRecord();
		// 只有成功和失败两种结果
		if (nRet == 0)
		{
			m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_BANK_CHPWD,HC_GP_BANK_CHPWD_SUC,
				sizeof(DL_GP_O_BankChPwd),pSourceData->uIndex,pSourceData->dwHandleID);
		}
		else
		{
			m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_BANK_CHPWD,HC_GP_BANK_CHPWD_ERR_PWD,
				sizeof(DL_GP_O_BankChPwd),pSourceData->uIndex,pSourceData->dwHandleID);
		}
	
	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}

// 获取财务状况
UINT CServiceDataBaseHandle::OnBankGetFinanceInfo(DataBaseLineHead* pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize != sizeof(DL_GP_I_FinanceInfo))
	{
		return 0;
	}

	DL_GP_I_FinanceInfo* in_ptr = (DL_GP_I_FinanceInfo*)pSourceData;
	DL_GP_O_FinanceInfo _out;

	try
	{
		bool bres=sqlSPSetNameEx("SP_Bank_GetFinance",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID",in_ptr->_data.iUserID);
		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}

		int nRet = getReturnValue();

		// 只有成功和失败两种结果
		_out._data.iUserID = in_ptr->_data.iUserID;
		if (nRet == 0)
		{
			getValue("BankMoney",&_out._data.i64BankMoney);
			getValue("WalletMoney",&_out._data.i64WalletMoney);
			getValue("Lotteries",&_out._data.iLotteries);
			getValue("UserJewels",&_out._data.iJewels);

			m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_BANK_FINANCE_INFO,HC_GP_BANK_FINANCE_INFO_SUC,
				sizeof(DL_GP_O_FinanceInfo),pSourceData->uIndex,pSourceData->dwHandleID);
		}
		else
		{
			m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_BANK_FINANCE_INFO,HC_GP_BANK_FINANCE_INFO_ERR,
				sizeof(DL_GP_O_FinanceInfo),pSourceData->uIndex,pSourceData->dwHandleID);
		}

		closeRecord();
	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}

// 获取最近登录的游戏
UINT CServiceDataBaseHandle::OnLastLoginGame(DataBaseLineHead* pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize != sizeof(DL_GP_I_LastLoginGame))
	{
		return 0;
	}

	DL_GP_I_LastLoginGame* in_ptr = (DL_GP_I_LastLoginGame*)pSourceData;
	DL_GP_O_LastLoginGame _out;

	try
	{
		bool bres=sqlSPSetNameEx("SP_GetLastLoginGame",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID",in_ptr->iUserID);
		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}

		int nRet = getReturnValue();


		while(!adoEndOfFile())
		{
			ZeroMemory(&_out,sizeof(DL_GP_O_LastLoginGame));

			getValue("KindID",&_out._data.iKindID);
			getValue("GameID",&_out._data.iGameID);
			getValue("LastTime",&_out ._data.oelLastTime);	
			getValue("LoginCount",&_out ._data.iLoginCount);

			m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_LASTLOGINGAME,0,sizeof(DL_GP_O_LastLoginGame),pSourceData->uIndex,pSourceData->dwHandleID);
			moveNext();
		}

		ZeroMemory(&_out,sizeof(DL_GP_O_LastLoginGame));
		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GP_LASTLOGINGAME,1,sizeof(DL_GP_O_LastLoginGame), pSourceData->uIndex, pSourceData->dwHandleID);
		closeRecord();

	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}

UINT CServiceDataBaseHandle::OnPaiHangBang(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_GP_I_PaiHangBang *_in = (DL_GP_I_PaiHangBang*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_MoneyPaiHangBang",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@Count", _in->_param.count);
	addInputParameter("@MoneyType", _in->_param.type);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	MSG_GP_MoneyPaiHangBang_Item paiHangBangItem;
	ZeroMemory(&paiHangBangItem,sizeof(paiHangBangItem));

	DL_GP_O_PaiHangBang _out;
	ZeroMemory(&_out, sizeof(_out));

	int nRet = getRecordCount();
	int index = 0;
    MSG_GP_MoneyPaiHangBang_Item *pItem = NULL;
    if(nRet != 0)
    {
        _out._data = new MSG_GP_MoneyPaiHangBang_Item[nRet];
        pItem = _out._data;
    }
	while(pItem != NULL && !adoEndOfFile() && index < nRet)
	{
		ZeroMemory(&paiHangBangItem,sizeof(MSG_GP_MoneyPaiHangBang_Item));

		{
			getValue("NickName",paiHangBangItem.nickName, sizeof(paiHangBangItem.nickName));
            if(_in->_param.type == 0)
			    getValue("MoneySum",&paiHangBangItem.i64Money);
            else if (_in->_param.type == 1)
                getValue("WalletMoney",&paiHangBangItem.i64Money);
            else
                getValue("BankMoney",&paiHangBangItem.i64Money);
			getValue("UserID", &paiHangBangItem.iUserID);
			getValue("HeadUrl",paiHangBangItem.szHeadUrl, sizeof(paiHangBangItem.szHeadUrl));
            getValue("SignDescr",paiHangBangItem.szSignDescr, sizeof(paiHangBangItem.szSignDescr));
		}			
		
        *pItem = paiHangBangItem;
        
        pItem++;

	    index++;

		moveNext();
	}
    _out.iCount = index;
	
	// 1: ok, 0: fail
	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_PAIHANGBANG, nRet, sizeof(DL_GP_O_PaiHangBang), pSourceData->uIndex, pSourceData->dwHandleID);
	closeRecord();

	return 0;
}

UINT CServiceDataBaseHandle::OnSignInCheck(DataBaseLineHead* pSourceData)
{
	if (pSourceData->DataLineHead.uSize != sizeof(DL_I_HALL_SIGN))	return 0;
	DL_I_HALL_SIGN* in_ptr = (DL_I_HALL_SIGN*)pSourceData;

	DL_O_HALL_SIGN_CHECK _out;
	ZeroMemory(&_out,sizeof(_out));

	bool bres=sqlSPSetNameEx("SP_SignAward",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID",in_ptr->dwUserID);
	addInputParameter("@Type", 0); //查询签到
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	_out._data.dwUserID = in_ptr->dwUserID;
	int iday = 0;
	while(!adoEndOfFile())
	{
		getValue("SignDay",&iday);
		if (iday <= 0 || iday >7)
		{
			moveNext();
			continue;
		}
		getValue("AwardMoney",&_out._data.iAwardMoney[iday-1]);
		getValue("AwardJewels",&_out ._data.iAwardJewels[iday-1]);	
		getValue("AwardLotteries",&_out ._data.iAwardLotteries[iday-1]);	
		getValue("CountDay",&_out ._data.byCountDay);
		getValue("HaveSign",&_out ._data.iRs);

		moveNext();
	}
		
	
	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_SIGNIN_CHECK,0,sizeof(DL_O_HALL_SIGN_CHECK),pSourceData->uIndex,pSourceData->dwHandleID);	
	closeRecord();
	return 0;
}

UINT CServiceDataBaseHandle::OnSignInDo(DataBaseLineHead* pSourceData)
{
	if (pSourceData->DataLineHead.uSize != sizeof(DL_I_HALL_SIGN))	return 0;
	DL_I_HALL_SIGN* in_ptr = (DL_I_HALL_SIGN*)pSourceData;

	DL_O_HALL_SIGN_DO _out;
	ZeroMemory(&_out,sizeof(_out));

	bool bres=sqlSPSetNameEx("SP_SignAward",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID",in_ptr->dwUserID);
	addInputParameter("@Type", 1); //执行签到
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	_out._data.dwUserID = in_ptr->dwUserID;
	int ret = getReturnValue();
	
	if (0 == ret)
	{
		getValue("AwardMoney",&_out._data.iGetMoney);
		getValue("AwardJewels",&_out ._data.iGetJewels);	
		getValue("AwardLotteries",&_out ._data.iGetLotteries);	
	}		

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_SIGNIN_DO,ret,sizeof(DL_O_HALL_SIGN_DO),pSourceData->uIndex,pSourceData->dwHandleID);	
	closeRecord();
	return 0;
}

UINT CServiceDataBaseHandle::OnLineAwardDo(DataBaseLineHead* pSourceData)
{
	if (pSourceData->DataLineHead.uSize != sizeof(DL_I_HALL_ONLINE_AWARD))	return 0;
	DL_I_HALL_ONLINE_AWARD* in_ptr = (DL_I_HALL_ONLINE_AWARD*)pSourceData;

	DL_O_HALL_ONLINE_AWARD_DO_RESULT outData;
	ZeroMemory(&outData,sizeof(outData));

	bool bres=sqlSPSetNameEx("SP_OnLineAwardFromPhone",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID",in_ptr->dwUserID);
	addInputParameter("@Type", 2); 
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	int ret = getReturnValue();
	outData._result.dwUserID = in_ptr->dwUserID;	
	if (1 == ret) //0:失败 1:成功
	{
		getValue("GetMoney", &outData._result.iCurrentGetMoney);
		getValue("NextTime", &outData._result.iNextTime);
		getValue("NextMoney", &outData._result.iNextMoney);
	}
	m_pRusultService->OnDataBaseResultEvent(&outData.ResultHead,DTK_GP_ONLINE_AWARD_DO,ret,sizeof(DL_O_HALL_ONLINE_AWARD_DO_RESULT),pSourceData->uIndex,pSourceData->dwHandleID);	
	closeRecord();
	return 0;
}
UINT CServiceDataBaseHandle::OnBuyDesk(DataBaseLineHead* pSourceData)//购买桌子
{
	DL_I_HALL_BUY_DESK* in_ptr = (DL_I_HALL_BUY_DESK*)pSourceData;
	if(!in_ptr)
	{
		return 0;
	}
	bool bres=sqlSPSetNameEx("SP_BuyDesk",true);
	if(!bres)
	{
		return 0;
	}
	
	addInputParameter("@UserID",in_ptr->_data.iUserID);
	addInputParameter("@BuyCount",in_ptr->_data.iPlayeCount); 
	addInputParameter("@PayType",in_ptr->_data.bPayType); 
	addInputParameter("@GameID",in_ptr->_data.iGameID); 
	addInputParameter("@FinishCondition",in_ptr->_data.bFinishCondition); 
	addInputParameter("@PositionLimit",in_ptr->_data.bPositionLimit); 
	addInputParameter("@PlayerNum",in_ptr->_data.bPlayerNum); 
	addInputParameter("@MidEnter",in_ptr->_data.bMidEnter); 
    addInputParameter("@BuyMinutes",in_ptr->_data.iBuyMinutes); 
	addInputParameter("@DeskConfig",(BYTE*)in_ptr->_data.szDeskConfig,sizeof(in_ptr->_data.szDeskConfig));
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	int ret = getReturnValue();
	DL_O_HALL_BUY_DESK_RES outData;
	if(1==ret)
	{
		getValue("iRoomID",&outData._result.iRoomID);
		getValue("iDeskID",&outData._result.iDeskID);
		getValue("szPassWord",outData._result.szPassWord,sizeof(outData._result.szPassWord));
		getValue("Jewels", &outData._result.iJewels);
	}
	else if(6==ret)
	{
		getValue("szPassWord",outData._result.szPassWord,sizeof(outData._result.szPassWord));
		getValue("NameID",&outData._result.iGameNameID);
	}
	outData._result.iUserID=in_ptr->_data.iUserID;
	m_pRusultService->OnDataBaseResultEvent(&outData.ResultHead,DTK_GP_BUY_DESK,ret,sizeof(DL_O_HALL_BUY_DESK_RES),pSourceData->uIndex,pSourceData->dwHandleID);
	closeRecord();
	return 1;
}
UINT CServiceDataBaseHandle::OnEnterVIPDesk(DataBaseLineHead* pSourceData)
{
	DL_I_HALL_ENTER_VIPDESK* in_ptr = (DL_I_HALL_ENTER_VIPDESK*)pSourceData;
	if(!in_ptr)
	{
		return 0;
	}

	bool bres=sqlSPSetNameEx("SP_EnterVIPDesk",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID",in_ptr->_data.iUserID);
	addInputParameter("@InputPassWord",in_ptr->_data.szInputPassWord,sizeof(in_ptr->_data.szInputPassWord)); 
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	int ret = getReturnValue();
	DL_O_HALL_ENTER_VIPDESK_RES outData;
	outData._result.iUserID=in_ptr->_data.iUserID;
	outData._result.bType = 2;
	strcpy_s(outData._result.szPass,in_ptr->_data.szInputPassWord);
	if(1==ret)
	{
		getValue("RoomID", &outData._result.iRoomID);
		getValue("TableNumber", &outData._result.iDeskID);
		getValue("PositionLimit", &outData._result.bPositionLimit);
	}
	else if(3==ret)
	{
		getValue("RoomID", &outData._result.iRoomID);
		getValue("TableNumber", &outData._result.iDeskID);
		getValue("PositionLimit", &outData._result.bPositionLimit);
	}
	else if (6==ret)
	{
		getValue("szPassWord", outData._result.szPass,sizeof(outData._result.szPass));
	}
	
	m_pRusultService->OnDataBaseResultEvent(&outData.ResultHead,DTK_GP_ENTER_VIPDESK,ret,sizeof(DL_O_HALL_ENTER_VIPDESK_RES),pSourceData->uIndex,pSourceData->dwHandleID);
	closeRecord();
	return 1;
}

UINT CServiceDataBaseHandle::OnGetCutRoom(DataBaseLineHead* pSourceData)
{
	DL_I_HALL_NETCUT* in_ptr = (DL_I_HALL_NETCUT*)pSourceData;
	if(!in_ptr)
	{
		return 0;
	}

	bool bres=sqlSPSetNameEx("SP_NetCut",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID",in_ptr->_data.iUserID);
	addInputParameter("@Type",in_ptr->_data.bType);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	int ret = getReturnValue();
	DL_O_HALL_NETCUT_RES outData;
	if(4==ret)
	{
		getValue("RoomID", &outData._result.iRoomID);
		getValue("Type",&outData._result.bType);
		outData._result.iDeskID = 255;
	}
	else if(2==ret||1==ret)
	{
		getValue("RoomID", &outData._result.iRoomID);
		getValue("TableNumber", &outData._result.iDeskID);
		getValue("PassWord", outData._result.szPass,sizeof(outData._result.szPass));
		getValue("PositionLimit", &outData._result.bPositionLimit);
		outData._result.bType = 2;
	}
	outData.iUserID = in_ptr->_data.iUserID;
	m_pRusultService->OnDataBaseResultEvent(&outData.ResultHead,DTK_GP_GET_CUTROOM,ret,sizeof(DL_O_HALL_NETCUT_RES),pSourceData->uIndex,pSourceData->dwHandleID);
	closeRecord();
	return 1;
}

UINT CServiceDataBaseHandle::OnLineAwardCheck(DataBaseLineHead* pSourceData)
{
	if (pSourceData->DataLineHead.uSize != sizeof(DL_I_HALL_ONLINE_AWARD))	return 0;
	DL_I_HALL_ONLINE_AWARD* in_ptr = (DL_I_HALL_ONLINE_AWARD*)pSourceData;

	DL_O_HALL_ONLINE_AWARD_CHECK_RESULT outData;
	ZeroMemory(&outData,sizeof(outData));

	bool bres=sqlSPSetNameEx("SP_OnLineAwardFromPhone",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID",in_ptr->dwUserID);
	addInputParameter("@Type", 1); 
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	int ret = getReturnValue();
	outData._result.dwUserID = in_ptr->dwUserID;	
	if (1 == ret) //0:失败 1:成功
	{
		getValue("LeftTime",&outData._result.iLeftTime);
		getValue("OnLineTimeMoney",&outData._result.iOnLineTimeMoney);		
	}

	m_pRusultService->OnDataBaseResultEvent(&outData.ResultHead,DTK_GP_ONLINE_AWARD_CHECK,ret,sizeof(DL_O_HALL_ONLINE_AWARD_CHECK_RESULT),pSourceData->uIndex,pSourceData->dwHandleID);
	closeRecord();
	return 0;
}

UINT CServiceDataBaseHandle::OnTotalRecord(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_TOTALRECORD *_in = (DL_I_HALL_TOTALRECORD*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_GetGameRecord",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->_data.iUserID);
	addInputParameter("@BeginNum", _in->_data.iStartCount);
	addInputParameter("@GameID", 0);
	addInputParameter("@Optype", _in->_data.iType);
	addInputParameter("@ClubID", _in->_data.iClubID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_TOTALRECORD_RES _out;
	ZeroMemory(&_out, sizeof(_out));

	map<int,int> map_ID;
	int ID;
	int count = 0;
	int iUserCount[50] = {0};

	int nRet = getReturnValue();

	_out._result = new MSG_GP_O_TotalRecord[20];

	while(!nRet && !adoEndOfFile())
	{
		getValue("ID",&ID);
		map<int,int>::iterator result = map_ID.find(ID);
		if (result == map_ID.end())
		{
			map_ID[ID] = count;
			count++;
		}
		int RecordNum = map_ID[ID];

        if(RecordNum >= 20) break;

        if(iUserCount[RecordNum] < sizeof(_out._result[RecordNum-1].szNickName) / sizeof(_out._result[RecordNum-1].szNickName[0]))
        {
            _out._result[RecordNum].ID = ID;
			_out._result[RecordNum].iClubID = _in->_data.iClubID;
            getValue("DeskPassword",_out._result[RecordNum].szPassWord,sizeof(_out._result[RecordNum].szPassWord));
            getValue("NickName",_out._result[RecordNum].szNickName[iUserCount[RecordNum]],sizeof(_out._result[RecordNum].szNickName[iUserCount[RecordNum]]));
            getValue("UserScore",&_out._result[RecordNum].iScore[iUserCount[RecordNum]]);
            getValue("CreateTime",&_out.datatime[RecordNum]);
            getValue("iCount",&_out._result[RecordNum].iTotleCount);
            getValue("GameName",_out._result[RecordNum].szGameName,sizeof(_out._result[RecordNum].szGameName));
            getValue("PlayNum",&_out._result[RecordNum].UserNum);

            iUserCount[RecordNum] = iUserCount[RecordNum]+1;
        }

		_out._result[RecordNum].iType = _in->_data.iType;

		moveNext();
	}
	_out.iCount = count;
	//1: ok, 0: fail
	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_TOTALRECORD, nRet, sizeof(DL_O_HALL_TOTALRECORD_RES), pSourceData->uIndex, pSourceData->dwHandleID);
	closeRecord();

	return 0;
}


UINT CServiceDataBaseHandle::OnSingleRecord(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_SINGLERECORD *_in = (DL_I_HALL_SINGLERECORD*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_GetGameRecord",true);
	if(!bres)
	{
		return 0;
	}
	
	addInputParameter("@UserID", _in->_data.iUserID);
	addInputParameter("@BeginNum", _in->_data.iStartCount);
	addInputParameter("@GameID", _in->_data.ID);
	addInputParameter("@Optype", 2);
	addInputParameter("@ClubID", 0);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_SINGLERECORD_RES _out;
	ZeroMemory(&_out,sizeof(_out));

	int iUserCount[50] = {0};

	int nRet = getReturnValue();

	_out._result = new MSG_GP_O_SingleRecord[20];
	_out.iCount = 0;

	while(!nRet && !adoEndOfFile())
	{
		int RecordNum = 0;
		int iUserID = 0;
		getValue("PlayCount",&RecordNum);
		if (RecordNum <= 0)
		{
			moveNext();
			continue;
        }

        if(RecordNum >= 20) break;

		if (RecordNum > _out.iCount)
		{
			_out.iCount = RecordNum;
		}

        if(iUserCount[RecordNum-1] < sizeof(_out._result[RecordNum-1].szNickName) / sizeof(_out._result[RecordNum-1].szNickName[0]))
        {
            getValue("iCount",&_out._result[RecordNum-1].iTotleCount);
            getValue("EndTime",&_out.datatime[RecordNum-1]);
            getValue("CanPlayback",&_out._result[RecordNum-1].bPlayBack);
            getValue("NickName",_out._result[RecordNum-1].szNickName[iUserCount[RecordNum-1]],sizeof(_out._result[RecordNum-1].szNickName[iUserCount[RecordNum-1]]));
            getValue("UserScore",&_out._result[RecordNum-1].iScore[iUserCount[RecordNum-1]]);
            getValue("DeskPassword",_out._result[RecordNum-1].szPassWord,sizeof(_out._result[RecordNum-1].szPassWord));
            getValue("PlayNum",&_out._result[RecordNum-1].UserNum);

            getValue("DeskUserID",&iUserID);
			if (iUserID == _in->_data.iUserID)
			{
				getValue("GameSN",_out._result[RecordNum-1].szRecordCode,sizeof(_out._result[RecordNum-1].szRecordCode));
				getValue("GameID",&_out._result[RecordNum-1].iGameID);
			}

            iUserCount[RecordNum-1] = iUserCount[RecordNum-1]+1;
        }

		moveNext();
	}
	//1: ok, 0: fail
	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_SINGLERECORD, nRet, sizeof(DL_O_HALL_SINGLERECORD_RES), pSourceData->uIndex, pSourceData->dwHandleID);
	closeRecord();

	return 0;
}

UINT CServiceDataBaseHandle::OnGetConfig(DataBaseLineHead* pSourceData)
{
	CString szSQL;
	szSQL.Format("select * from T_AuditConfig");

	if (execSQL(szSQL,true))
	{
		m_pDataBaseManage->SQLConnectReset();
		closeRecord();
		return 0;
	}

	DL_O_HALL_GETCONFIG_RES _out;
	_out._data.iCount = getRecordCount();
	int i = 0;
	while(!adoEndOfFile() && i<20)
	{
		getValue("AttrKey",_out._data._data[i].name,sizeof(_out._data._data[i].name));
		getValue("AdrOpen",&_out._data._data[i].bShowAndroid);
		getValue("IosOpen",&_out._data._data[i].bShowIOS);

		i++;
		moveNext();
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_GET_CONFIG,0, sizeof(_out), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();
	return true;
}


UINT CServiceDataBaseHandle::OnContestNoticeLogon(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_CONTEST_NOTICE_LOGON *_in = (DL_I_HALL_CONTEST_NOTICE_LOGON*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_ContestApplyList",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_CONTEST_NOTICE_LOGON_RES _out;

	while(!adoEndOfFile())
	{
		COleDateTime cBeginTime;
		getValue("ContestID",&_out._result.ContestID);
		getValue("RoomID",&_out._result.iRoomID);
		getValue("BeginTime",&cBeginTime);
		getValue("RoomName",_out._result.szRoomName,sizeof(_out._result.szRoomName));

		SYSTEMTIME systime;
		VariantTimeToSystemTime(cBeginTime, &systime);
		CTime timeTmp(systime);
		_out._result.BeginTime = timeTmp.GetTime();

		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_CONTEST_NOTICE_LOGON, 0, sizeof(DL_O_HALL_CONTEST_NOTICE_LOGON_RES), pSourceData->uIndex, pSourceData->dwHandleID);

		moveNext();
	}

	closeRecord();

	return 0;
}

UINT CServiceDataBaseHandle::OnGetDeskConfig(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_BuyDeskConfig *_in = (DL_I_HALL_BuyDeskConfig*)pSourceData;	

    DL_O_HALL_BuyDeskConfig_RES _out;
    _out._result.iGameID = _in->_data.iGameID;

    bool bres=sqlSPSetNameEx("SP_GetVipDeskConfig",true);
    if(!bres)
    {
        m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_GET_BUYDESKCONFIG, 1, sizeof(DL_O_HALL_BuyDeskConfig_RES), pSourceData->uIndex, pSourceData->dwHandleID);
        return 0;
    }

    addInputParameter("@GameID", _in->_data.iGameID);
    if(0 != execStoredProc())
    {
        m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_GET_BUYDESKCONFIG, 1, sizeof(DL_O_HALL_BuyDeskConfig_RES), pSourceData->uIndex, pSourceData->dwHandleID);
        closeRecord();
        return 0;
    }

    bool bOnlyGet = false;
    int iCount = 0;
    int iCoinBasePointindex = 0;
    int iCoreBasePointindex = 0;
    while(!adoEndOfFile() && iCount<sizeof(_out._result.buyCounts) / sizeof(_out._result.buyCounts[0]))
    {
        getValue("BuyGameCount",&_out._result.buyCounts[iCount].iBuyCount);
        getValue("AANeedJewels",&_out._result.buyCounts[iCount].iAAJewels);
        /*if(iCount > 0 && _out._result.buyCounts[iCount].iBuyCount == _out._result.buyCounts[iCount-1].iBuyCount
            && _out._result.buyCounts[iCount].iAAJewels == _out._result.buyCounts[iCount-1].iAAJewels)
        {
            _out._result.buyCounts[iCount].iBuyCount = 0;
            _out._result.buyCounts[iCount].iAAJewels = 0;
        }
        else*/
        {
            getValue("NeedJewels",&_out._result.buyCounts[iCount].iJewels);
            iCount++;
        }
        if(!bOnlyGet)
        {
            bOnlyGet = true;
            getValue("Invoice",&_out._result.bInvoice);
            getValue("EveryJewelForMinutes",&_out._result.bEveryJewelForMinutes);
            getValue("FinishCondition",&_out._result.bFinishCondition);
            getValue("DynamicInfo1",(BYTE*)_out._result.szGameDynamicInfo1,sizeof(_out._result.szGameDynamicInfo1));
        }
        moveNext();
    }

    closeRecord();

    _out._result.iGameID = _in->_data.iGameID;
    m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_GET_BUYDESKCONFIG, 0, sizeof(DL_O_HALL_BuyDeskConfig_RES), pSourceData->uIndex, pSourceData->dwHandleID);

    return 0;
}


UINT CServiceDataBaseHandle::OnGetRecordURL(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_GetRecordURL *_in = (DL_I_HALL_GetRecordURL*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_GetRecordURL",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@RecordID", _in->_data.szRecordCode,sizeof(_in->_data.szRecordCode));
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_GetRecordURL_RES _out;
	
	int ret = getReturnValue();

	if (0 == ret)
	{
		getValue("GameID",&_out._result.iGameID);
		strcpy_s(_out._result.szRecordCode,_in->_data.szRecordCode);
	}
	
	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_GET_RECORDURL, ret, sizeof(DL_O_HALL_GetRecordURL_RES), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}

UINT CServiceDataBaseHandle::OnGetDeskList(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_GetDeskList *_in = (DL_I_HALL_GetDeskList*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_GetDeskList",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->_data.iUserID);
	addInputParameter("@Type", _in->_data.bType);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_GetDeskList_RES _out;

	int ret = getReturnValue();

    int iMinutes = 0;

	if (0 == ret)
	{
		_out.iCount = getRecordCount();
		_out._result = new MSG_GP_O_GetBuydeskList[_out.iCount];

		int iCount=0;
		while(!adoEndOfFile() && iCount<50)
		{
			getValue("IsPlay",&_out._result[iCount].bIsPlay);
			getValue("BuyTime",&_out.datatime[iCount]);
			getValue("szDeskPassWord",_out._result[iCount].szDeskPass,sizeof(_out._result[iCount].szDeskPass));
			getValue("GameName",_out._result[iCount].szGameName,sizeof(_out._result[iCount].szGameName));
            getValue("FinishCondition",&_out._result[iCount].bFinishCondition);
            getValue("LeaveSeconds",&_out._result[iCount].iLeaveSeconds);
            getValue("BuyMinutes",&iMinutes);
            if(_out._result[iCount].iLeaveSeconds < 0) _out._result[iCount].iLeaveSeconds = 0;
            if(!_out._result[iCount].bIsPlay) _out._result[iCount].iLeaveSeconds = iMinutes * 60;
            
			_out._result[iCount].btype = _in->_data.bType;
			_out._result[iCount].iUserID = _in->_data.iUserID;
			iCount++;

			moveNext();
		}
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_GET_DESKLIST, ret, sizeof(DL_O_HALL_GetDeskList_RES), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}

UINT CServiceDataBaseHandle::OnGetDeskUser(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_GetDeskUser *_in = (DL_I_HALL_GetDeskUser*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_GetDeskUser",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@DeskPass", _in->_data.szDeskPass,sizeof(_in->_data.szDeskPass));
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_GetDeskUser_RES _out;

	int ret = getReturnValue();

	if (0 == ret)
	{
		_out.iCount = getRecordCount();
		_out._result = new MSG_GP_O_GetDeskUser[_out.iCount];

		int iCount=0;
		while(!adoEndOfFile())
		{
			getValue("Sex",&_out._result[iCount].bSex);
			getValue("LogoID",&_out._result[iCount].iLogoID);
			getValue("UserID",&_out._result[iCount].iUserID);
			getValue("NickName",_out._result[iCount].szUserName,sizeof(_out._result[iCount].szUserName));
			getValue("HeadUrl",_out._result[iCount].szUserHeadURL,sizeof(_out._result[iCount].szUserHeadURL));

			iCount++;

			moveNext();
		}
	}

	strcpy_s(_out.szDeskPass,_in->_data.szDeskPass);
	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_GET_DESKUSER, ret, sizeof(DL_O_HALL_GetDeskUser_RES), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}

UINT CServiceDataBaseHandle::OnDissmissDesk(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_DissmissDesk *_in = (DL_I_HALL_DissmissDesk*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_ClearDeskbyHall",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@DeskPass", _in->_data.szDeskPass,sizeof(_in->_data.szDeskPass));

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_DissmissDesk _out;

	int ret = getReturnValue();

	if (ERR_GP_DISSMISSDESK_SUCCESS == ret)
	{
		getValue("RoomID",&_out._data.iRoomID);
		getValue("DeskID",&_out._data.iDeskID);
	}

	_out._data.iUserID = _in->iUserID;
	strcpy_s(_out._data.szDeskPass,_in->_data.szDeskPass);

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_DISSMISSDESK, ret, sizeof(DL_O_HALL_DissmissDesk), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}

UINT CServiceDataBaseHandle::OnDeleteRecord(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_DeleteRecord *_in = (DL_I_HALL_DeleteRecord*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_DeleteRecord",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@DeskPass", _in->_data.szDeskPass,sizeof(_in->_data.szDeskPass));

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	int ret = getReturnValue();

	DataBaseResultLine ResultHead;	

	m_pRusultService->OnDataBaseResultEvent(&ResultHead, DTK_GP_DELETERECORD, ret, sizeof(DataBaseResultLine), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}

UINT CServiceDataBaseHandle::OnGetChangeConfig(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;

	CString sql;
	sql.Format("select top 1 Money2JewelRate,Jewel2MoneyRate from Web_Config");
	if(execSQL(sql, true))
	{
		return 0;
	}

	DL_O_HALL_ChangeConfig_RES _out;

	getValue("Money2JewelRate",&_out._result.iCoinToJewel);
	getValue("Jewel2MoneyRate",&_out._result.iJewelToCoin);
	
	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_CHANGE_CONFIG, 0, sizeof(DL_O_HALL_ChangeConfig_RES), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}



UINT CServiceDataBaseHandle::OnChangeMoney(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_ChangeRequest *_in = (DL_I_HALL_ChangeRequest*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_MoneyChange",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->_data.iUserID);
	addInputParameter("@ChangeJewels", _in->_data.iChangeJewels);
	addInputParameter("@Type", _in->itype);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_ChangeResponse _out;

	int ret = getReturnValue();

	if (0 == ret)
	{
		getValue("Jewels",&_out._result.iJewels);
		getValue("WalletMoney",&_out._result.i64Money);
	}
	_out._result.iUserID = _in->_data.iUserID;
	_out.itype = _in->itype;

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_MONEY_CHANGE, ret, sizeof(DL_O_HALL_ChangeResponse), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}

UINT CServiceDataBaseHandle::OnUpdateRoomConnect(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_UpdateRoomConnect *_in = (DL_I_HALL_UpdateRoomConnect*)pSourceData;

	CString sql;
	sql.Format("UPDATE dbo.TGameRoomInfo SET IsConnected=%d WHERE RoomID=%d", _in->bConnect?1:0, _in->iRoomID);
	if(execSQL(sql, true))
	{
		return 0;
	}

	closeRecord();

	return 0;
}

UINT CServiceDataBaseHandle::OnUpdateRoomConnectAll(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_UpdateRoomConnect_ALL *_in = (DL_I_HALL_UpdateRoomConnect_ALL*)pSourceData;	
	if (0 == _in->iCount || nullptr == _in->RoomID)
	{
		CString sql;
		sql.Format("UPDATE dbo.TGameRoomInfo SET IsConnected=0");
		if(execSQL(sql, true))
		{
			return 0;
		}

		closeRecord();

		return 0;
	}

	CString szRoomList = "";
	TCHAR Temp[10];

	for (int i=0;i<_in->iCount;i++)
	{
		wsprintf(Temp, TEXT("%d,"), _in->RoomID[i]);
		szRoomList += Temp;
	}

	SafeDeleteArray(_in->RoomID);

	bool bres=sqlSPSetNameEx("SP_UpdateRoomConnect");
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@RoomList", szRoomList.GetBuffer(),sizeof(TCHAR) * (szRoomList.GetLength()));
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	closeRecord();

	return 0;
}


UINT CServiceDataBaseHandle::OnForgetPWD(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_ForgetPWD *_in = (DL_I_HALL_ForgetPWD*)pSourceData;	
	
	bool bres=sqlSPSetNameEx("SP_ForgetPWD",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UseID", _in->_data.bUseID);
	addInputParameter("@UserID", _in->_data.iUserID);
	addInputParameter("@UserName", _in->_data.szUserName,sizeof(_in->_data.szUserName));
	addInputParameter("@PhoneNum", _in->_data.szPhoneNum,sizeof(_in->_data.szPhoneNum));
	addInputParameter("@NewPassword", _in->_data.szNewPWD,sizeof(_in->_data.szNewPWD));
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	int ret = getReturnValue();

	closeRecord();

	DataBaseResultLine   ResultHead;	
	m_pRusultService->OnDataBaseResultEvent(&ResultHead, DTK_GP_USERINFO_FORGET_PWD, ret, sizeof(DataBaseResultLine), pSourceData->uIndex, pSourceData->dwHandleID);

	return 0;
}

//俱乐部
UINT CServiceDataBaseHandle::OnCreateClub(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_CreateClub *_in = (DL_I_HALL_CreateClub*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_CreateClub",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@ClubName", _in->_data.szClubName,sizeof(_in->_data.szClubName));

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_CreateClub _out;

	int ret = getReturnValue();

	if (0 == ret)
	{
		getValue("ClubID",&_out._data.iClubID);
		getValue("ClubName",_out._data.szClubName,sizeof(_out._data.szClubName));
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_CREATE_CLUB, ret, sizeof(DL_O_HALL_CreateClub), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}
UINT CServiceDataBaseHandle::OnDissmissClub(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_DissmissClub *_in = (DL_I_HALL_DissmissClub*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_DissmissClub",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@ClubID", _in->_data.iClubID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_DissmissClub _out;
	_out._data.iClubID = _in->_data.iClubID;

	int ret = getReturnValue();

	if (0 == ret)
	{
		_out.iDissmissDeskCount = getRecordCount();
		_out._NoticeData = new MSG_MG_R_ClubDissmissDesk[_out.iDissmissDeskCount];

		int iCount=0;
		while(!adoEndOfFile())
		{
			getValue("RoomID",&_out._NoticeData[iCount].iRoomID);
			getValue("TableNumber",&_out._NoticeData[iCount].iDeskID);
			getValue("szDeskPassWord",_out._NoticeData[iCount].szDeskPass,sizeof(_out._NoticeData[iCount].szDeskPass));

			iCount++;

			moveNext();
		}
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_DISSMISS_CLUB, ret, sizeof(DL_O_HALL_DissmissClub), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}
UINT CServiceDataBaseHandle::OnJoinClub(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_JoinClub *_in = (DL_I_HALL_JoinClub*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_JoinClub",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@ClubID", _in->_data.iClubID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_JoinClub _out;

	int ret = getReturnValue();
	_out._UserDate.iClub = _in->_data.iClubID;
	_out._UserDate.bJoin = true;
	_out._UserDate._data.iUserID = _in->iUserID;

	if (0 == ret)
	{
		getValue("MasterID",&_out.iMasterID);
		getValue("LogonID",&_out._UserDate._data.iLogoID);
		getValue("Sex",&_out._UserDate._data.bSex);
		getValue("NickName",_out._UserDate._data.szUserNickName,sizeof(_out._UserDate._data.szUserNickName));
		getValue("HeadURL",_out._UserDate._data.szHeadURL,sizeof(_out._UserDate._data.szHeadURL));
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_JOIN_CLUB, ret, sizeof(DL_O_HALL_JoinClub), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}
UINT CServiceDataBaseHandle::OnClubUserList(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_Club_UserList *_in = (DL_I_HALL_Club_UserList*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_ClubUserList",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@ClubID", _in->_data.iClubID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_Club_UserList _out;
	_out._data.iClubID = _in->_data.iClubID;

	int ret = getReturnValue();

	if (0 == ret)
	{
		_out._data.iUserNum = getRecordCount();
		_out._UserData = new MSG_GP_O_Club_UserList_Data[_out._data.iUserNum];
		getValue("MasterID",&_out._data.iCreaterID);

		int iCount=0;
		while(!adoEndOfFile())
		{
			getValue("Sex",&_out._UserData[iCount].bSex);
			getValue("LogoID",&_out._UserData[iCount].iLogoID);
			getValue("UserID",&_out._UserData[iCount].iUserID);
			getValue("NickName",_out._UserData[iCount].szUserNickName,sizeof(_out._UserData[iCount].szUserNickName));
			getValue("HeadUrl",_out._UserData[iCount].szHeadURL,sizeof(_out._UserData[iCount].szHeadURL));

			iCount++;

			moveNext();
		}
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_CLUB_USERLIST, ret, sizeof(DL_O_HALL_Club_UserList), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}
UINT CServiceDataBaseHandle::OnClubRoomList(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_Club_RoomList *_in = (DL_I_HALL_Club_RoomList*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_ClubRoomList",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@ClubID", _in->_data.iClubID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_Club_RoomList _out;
	_out._data.iClubID = _in->_data.iClubID;

	int ret = getReturnValue();

    int iMinutes = 0;

	if (0 == ret)
	{
		_out._data.iRoomNum = getRecordCount();
		_out._RoomData = new MSG_GP_O_Club_RoomList_Data[_out._data.iRoomNum];

		int iCount=0;
		while(!adoEndOfFile())
		{
            getValue("IsPlay",&_out._RoomData[iCount].bIsPlay);
			getValue("Sex",&_out._RoomData[iCount].bSex);
			getValue("LogoID",&_out._RoomData[iCount].iLogoID);
			getValue("UserID",&_out._RoomData[iCount].iMaterID);
			getValue("BuyGameCount",&_out._RoomData[iCount].iCount);
			getValue("ClubPayType",&_out._RoomData[iCount].bPayType);
			getValue("NickName",_out._RoomData[iCount].szUserNickName,sizeof(_out._RoomData[iCount].szUserNickName));
			getValue("HeadUrl",_out._RoomData[iCount].szHeadURL,sizeof(_out._RoomData[iCount].szHeadURL));
			getValue("szDeskPassWord",_out._RoomData[iCount].szDeskPass,sizeof(_out._RoomData[iCount].szDeskPass));
			getValue("GameName",_out._RoomData[iCount].szGameName,sizeof(_out._RoomData[iCount].szGameName));
            getValue("AllowEnter",&_out._RoomData[iCount].bAllowEnter);
            getValue("FinishCondition",&_out._RoomData[iCount].bFinishCondition);
            getValue("LeaveSeconds",&_out._RoomData[iCount].iLeaveSeconds);
            getValue("BuyMinutes",&iMinutes);
            if(_out._RoomData[iCount].iLeaveSeconds < 0) _out._RoomData[iCount].iLeaveSeconds = 0;
            if(!_out._RoomData[iCount].bIsPlay) _out._RoomData[iCount].iLeaveSeconds = iMinutes * 60;

			iCount++;

			moveNext();
		}
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_CLUB_ROOMLIST, ret, sizeof(DL_O_HALL_Club_RoomList), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}
UINT CServiceDataBaseHandle::OnClubBuyDesk(DataBaseLineHead* pSourceData)
{
	DL_I_HALL_Club_BuyDesk* in_ptr = (DL_I_HALL_Club_BuyDesk*)pSourceData;
	if(!in_ptr)
	{
		return 0;
	}
	bool bres=sqlSPSetNameEx("SP_ClubBuyDesk",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID",in_ptr->_data.iUserID);
	addInputParameter("@ClubID",in_ptr->_data.iClubID);
	addInputParameter("@BuyCount",in_ptr->_data.iPlayeCount); 
	addInputParameter("@PayType",in_ptr->_data.bPayType); 
	addInputParameter("@GameID",in_ptr->_data.iGameID); 
	addInputParameter("@FinishCondition",in_ptr->_data.bFinishCondition); 
	addInputParameter("@PositionLimit",in_ptr->_data.bPositionLimit); 
	addInputParameter("@PlayerNum",in_ptr->_data.bPlayerNum); 
	addInputParameter("@MidEnter",in_ptr->_data.bMidEnter); 
	addInputParameter("@DeskConfig",(BYTE*)in_ptr->_data.szDeskConfig,sizeof(in_ptr->_data.szDeskConfig));
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	int ret = getReturnValue();
	DL_O_HALL_Club_BuyDesk outData;
	if(0==ret)
	{
		outData._data.iClubID = in_ptr->_data.iClubID;
		outData._data._RoomData.bPayType = in_ptr->_data.bPayType;
		outData._data.iUserID = in_ptr->_data.iUserID;
		outData._data._RoomData.iMaterID = in_ptr->_data.iUserID;
		outData._data._RoomData.iCount = in_ptr->_data.iPlayeCount;
		outData.iGameID = in_ptr->_data.iGameID;

		getValue("iRoomID",&outData.iRoomID);
		getValue("iDeskID",&outData.iDeskID);
		getValue("Jewels",&outData.iJewels);
		getValue("LogonID",&outData._data._RoomData.iLogoID);
		getValue("Sex",&outData._data._RoomData.bSex);
		getValue("NickName",outData._data._RoomData.szUserNickName,sizeof(outData._data._RoomData.szUserNickName));
		getValue("HeadURL",outData._data._RoomData.szHeadURL,sizeof(outData._data._RoomData.szHeadURL));
		getValue("szPassWord",outData._data._RoomData.szDeskPass,sizeof(outData._data._RoomData.szDeskPass));
		getValue("GameName",outData._data._RoomData.szGameName,sizeof(outData._data._RoomData.szGameName));
	}
	
	m_pRusultService->OnDataBaseResultEvent(&outData.ResultHead,DTK_GP_CLUB_CREATEROOM,ret,sizeof(DL_O_HALL_Club_BuyDesk),pSourceData->uIndex,pSourceData->dwHandleID);
	closeRecord();

	return 0;
}
UINT CServiceDataBaseHandle::OnClubChangename(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_ChangeName *_in = (DL_I_HALL_ChangeName*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_ClubChangeName",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@ClubID", _in->_data.iClubID);
	addInputParameter("@ClubName",_in->_data.szNewClubName,sizeof(_in->_data.szNewClubName));

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_ChangeName _out;

	int ret = getReturnValue();
	_out._data.iClubID = _in->_data.iClubID;

	if (0 == ret)
	{
		strcpy_s(_out._data.szNewClubName,_in->_data.szNewClubName);
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_CLUB_CHANGENAME, ret, sizeof(DL_O_HALL_ChangeName), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}
UINT CServiceDataBaseHandle::OnClubKickUser(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_Club_KickUser *_in = (DL_I_HALL_Club_KickUser*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_ClubKickUser",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@ClubID", _in->_data.iClubID);
	addInputParameter("@TargetID", _in->_data.iTargetID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_Club_KickUser _out;

	int ret = getReturnValue();
	_out._data.iClubID = _in->_data.iClubID;
	_out._data.iTargetID = _in->_data.iTargetID;

	if (0 == ret || 4 == ret)
	{
		_out._TarData.bJoin = false;
		_out._TarData.iClub = _in->_data.iClubID;
		_out._TarData._data.iUserID = _in->_data.iTargetID;
		getValue("LogonID",&_out._TarData._data.iLogoID);
		getValue("Sex",&_out._TarData._data.bSex);
		getValue("NickName",_out._TarData._data.szUserNickName,sizeof(_out._TarData._data.szUserNickName));
		getValue("HeadURL",_out._TarData._data.szHeadURL,sizeof(_out._TarData._data.szHeadURL));
		if (4 == ret)
		{
			_out.iDissmissDeskCount = getRecordCount();
			_out._NoticeData = new MSG_MG_R_ClubDissmissDesk[_out.iDissmissDeskCount];

			int iCount=0;
			while(!adoEndOfFile())
			{
				getValue("RoomID",&_out._NoticeData[iCount].iRoomID);
				getValue("TableNumber",&_out._NoticeData[iCount].iDeskID);
				getValue("szDeskPassWord",_out._NoticeData[iCount].szDeskPass,sizeof(_out._NoticeData[iCount].szDeskPass));

				iCount++;

				moveNext();
			}
		}
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_CLUB_KICKUSER, ret, sizeof(DL_O_HALL_Club_KickUser), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}
UINT CServiceDataBaseHandle::OnClubList(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_Club_List *_in = (DL_I_HALL_Club_List*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_ClubList",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_Club_List _out;

	int ret = getReturnValue();

	if (0 == ret)
	{
		_out.iCount = getRecordCount();
		_out._data = new MSG_GP_O_Club_List[_out.iCount];

		int iCount=0;
		while(!adoEndOfFile())
		{
			getValue("ClubID",&_out._data[iCount].iClubID);
			getValue("MasterID",&_out._data[iCount].iMaterID);
			getValue("UserNum",&_out._data[iCount].iClubUserNum);
			getValue("RoomNum",&_out._data[iCount].iRoomNum);
			getValue("LogoID",&_out._data[iCount].iLogoID);
			getValue("Sex",&_out._data[iCount].bSex);
			getValue("HeadURL",_out._data[iCount].szHeadURL,sizeof(_out._data[iCount].szHeadURL));
			getValue("ClubName",_out._data[iCount].szClubName,sizeof(_out._data[iCount].szClubName));
			_out._data[iCount].bCreater = (_out._data[iCount].iMaterID==_in->iUserID)?true:false;

			iCount++;

			moveNext();
		}
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_CLUB_LIST, ret, sizeof(DL_O_HALL_Club_List), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}
UINT CServiceDataBaseHandle::OnClubReviewList(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_Club_ReviewList *_in = (DL_I_HALL_Club_ReviewList*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_ClubReviewList",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@ClubID", _in->_data.iClubID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_Club_ReviewList _out;
	_out._HeadData.iClubID = _in->_data.iClubID;

	int ret = getReturnValue();

	if (0 == ret)
	{
		_out._HeadData.iUserNum = getRecordCount();
		_out._Data = new MSG_GP_O_Club_ReviewList_Data[_out._HeadData.iUserNum];

		int iCount=0;
		while(!adoEndOfFile())
		{
			getValue("LogoID",&_out._Data[iCount].iLogoID);
			getValue("Sex",&_out._Data[iCount].bSex);
			getValue("UserID",&_out._Data[iCount].iUserID);
			getValue("bTYPE",&_out._Data[iCount].bUserType);
			getValue("NickName",_out._Data[iCount].szUserNickName,sizeof(_out._Data[iCount].szUserNickName));
			getValue("HeadURL",_out._Data[iCount].szHeadURL,sizeof(_out._Data[iCount].szHeadURL));

			iCount++;

			moveNext();
		}
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_REVIEW_LIST, ret, sizeof(DL_O_HALL_Club_ReviewList), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}
UINT CServiceDataBaseHandle::OnClubMasterOPT(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_Club_MasterOpt *_in = (DL_I_HALL_Club_MasterOpt*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_ClubMasterOPT",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@ClubID", _in->_data.iClubID);
	addInputParameter("@TargetID", _in->_data.iTargetID);
	addInputParameter("@Type", _in->_data.bOptType);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_Club_MasterOpt _out;

	int ret = getReturnValue();
	_out._Data.iClubID = _in->_data.iClubID;
	_out._Data.iTargetID = _in->_data.iTargetID;
	_out._Data.bOptType = _in->_data.bOptType;

	if (0 == ret)
	{
		_out._TarData.bJoin = true;
		_out._TarData.iClub = _in->_data.iClubID;
		_out._TarData._data.iUserID = _in->_data.iTargetID;
		getValue("LogonID",&_out._TarData._data.iLogoID);
		getValue("Sex",&_out._TarData._data.bSex);
		getValue("NickName",_out._TarData._data.szUserNickName,sizeof(_out._TarData._data.szUserNickName));
		getValue("HeadURL",_out._TarData._data.szHeadURL,sizeof(_out._TarData._data.szHeadURL));

		int iMasterID;
		_out._ClubData.iClubID = _in->_data.iClubID;
		getValue("MasterID",&iMasterID);
		getValue("UserNum",&_out._ClubData.iClubUserNum);
		getValue("RoomNum",&_out._ClubData.iRoomNum);
		getValue("MasterLogoID",&_out._ClubData.iLogoID);
		getValue("MasterSex",&_out._ClubData.bSex);
		getValue("MasterHeadURL",_out._ClubData.szHeadURL,sizeof(_out._ClubData.szHeadURL));
		getValue("ClubName",_out._ClubData.szClubName,sizeof(_out._ClubData.szClubName));
		_out._ClubData.bCreater = (iMasterID==_in->iUserID)?true:false;
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_MASTER_OPTION, ret, sizeof(DL_O_HALL_Club_MasterOpt), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}
UINT CServiceDataBaseHandle::OnClubChangeNotice(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_Club_Notice *_in = (DL_I_HALL_Club_Notice*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_ClubChangeNotice",true);
	if(!bres)
	{
		return 0;
	}

	_in->_data.szClubNotice[100] = '\0';

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@ClubID", _in->_data.iClubID);
	addInputParameter("@ClubNotice", _in->_data.szClubNotice,sizeof(_in->_data.szClubNotice));

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_Club_Notice _out;

	int ret = getReturnValue();
	_out._data.iClubID = _in->_data.iClubID;
	strcpy_s(_out._data.szClubNotice,_in->_data.szClubNotice);

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_CLUB_NOTICE, ret, sizeof(DL_O_HALL_Club_Notice), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}
UINT CServiceDataBaseHandle::OnEnterClub(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_Club_EnterClub *_in = (DL_I_HALL_Club_EnterClub*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_EnterClub",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@ClubID", _in->_data.iClubID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_Club_EnterClub _out;

	int ret = getReturnValue();
	_out._data.iClubID = _in->_data.iClubID;

	if (0 == ret)
	{
		getValue("MasterID",&_out._data.iMasterID);
		getValue("Haveapplication",&_out._data.bHaveapplication);
		getValue("ClubNotice",_out._data.szClubNotice,sizeof(_out._data.szClubNotice));
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_ENTER_CLUB, ret, sizeof(DL_O_HALL_Club_EnterClub), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}


UINT CServiceDataBaseHandle::OnLeaveClub(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_Club_LeaveClub *_in = (DL_I_HALL_Club_LeaveClub*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_ClubUserLeave",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@ClubID", _in->_data.iClubID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_Club_LeaveClub _out;
	_out._data.iClubID = _in->_data.iClubID;

	int ret = getReturnValue();

	if (0 == ret)
	{
		_out._UserData.bJoin = false;
		_out._UserData.iClub = _in->_data.iClubID;
		_out._UserData._data.iUserID = _in->iUserID;
		getValue("LogonID",&_out._UserData._data.iLogoID);
		getValue("Sex",&_out._UserData._data.bSex);
		getValue("NickName",_out._UserData._data.szUserNickName,sizeof(_out._UserData._data.szUserNickName));
		getValue("HeadURL",_out._UserData._data.szHeadURL,sizeof(_out._UserData._data.szHeadURL));
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_LEAVE_CLUB, ret, sizeof(DL_O_HALL_DissmissClub), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}

UINT CServiceDataBaseHandle::OnGetBuyDeskRecord(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_Club_GetRecord *_in = (DL_I_HALL_Club_GetRecord*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_GetClubBuyDeskRecord",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@ClubID", _in->_data.iClubID);
	addInputParameter("@StartTime", _in->_data.i64StartTime);
	addInputParameter("@EndTime", _in->_data.i64EndTime);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_Club_GetRecord _out;
	_out._HeadData.iClubID = _in->_data.iClubID;

	int ret = getReturnValue();
	if (0 == ret)
	{
		_out._HeadData.iDataNum = getRecordCount();
		_out._Data = new MSG_GP_O_Club_BuyDeskRecord_Data[_out._HeadData.iDataNum];

		int iCount=0;
		while(!adoEndOfFile())
		{
			COleDateTime cBeginTime;

			getValue("UserID",&_out._Data[iCount].iUserID);
			getValue("CostJewels",&_out._Data[iCount].iCostJewels);
			getValue("BuyTime",&cBeginTime);
			getValue("DeskPass",_out._Data[iCount].szDeskPass,sizeof(_out._Data[iCount].szDeskPass));
			getValue("NickName",_out._Data[iCount].szUserNickName,sizeof(_out._Data[iCount].szUserNickName));

			SYSTEMTIME systime;
			VariantTimeToSystemTime(cBeginTime, &systime);
			CTime timeTmp(systime);
			_out._Data[iCount].i64BuyTime = timeTmp.GetTime();

			iCount++;

			moveNext();
		}
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_GET_DESKRECORD, ret, sizeof(DL_O_HALL_Club_GetRecord), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}



UINT CServiceDataBaseHandle::OnMailList(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_Mail_List *_in = (DL_I_HALL_Mail_List*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_GetMailList",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_Mail_List _out;

	int ret = getReturnValue();
	if (0 == ret)
	{
		_out._HeadData.iDataNum = getRecordCount();
		if (_out._HeadData.iDataNum > 0)
		{
			_out._data = new MSG_GP_O_Mail_List_Data[_out._HeadData.iDataNum];

			int iCount=0;
			while(!adoEndOfFile())
			{
				COleDateTime cBeginTime;

				getValue("MailID",&_out._data[iCount].iMailID);
				getValue("IsGet",&_out._data[iCount].bMailState);
				getValue("KeepTime",&_out._data[iCount].bKeepTime);
				getValue("SendTime",&cBeginTime);
				getValue("MailTitle",_out._data[iCount].szMailTitle,sizeof(_out._data[iCount].szMailTitle));
				getValue("SenderName",_out._data[iCount].szSendName,sizeof(_out._data[iCount].szSendName));

				SYSTEMTIME systime;
				VariantTimeToSystemTime(cBeginTime, &systime);
				CTime timeTmp(systime);
				_out._data[iCount].i64MailTime = timeTmp.GetTime();

				iCount++;

				moveNext();
			}
		}
	}

	closeRecord();

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_MAIL_LIST, ret, sizeof(DL_O_HALL_Mail_List), pSourceData->uIndex, pSourceData->dwHandleID);


	return 0;
}


UINT CServiceDataBaseHandle::OnOpenMail(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_Open_Mail *_in = (DL_I_HALL_Open_Mail*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_OpenMail",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@MailID", _in->_data.iMailID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_Open_Mail _out;
	_out._data.iMailID = _in->_data.iMailID;

	int ret = getReturnValue();
	if (0 == ret)
	{
		getValue("IsGet",&_out._data.bMailState);
		getValue("SendLotteries",&_out._data.iLotteries);
		getValue("SendMoney",&_out._data.i64Money);
		getValue("SendJewels",&_out._data.iJewels);
		getValue("MailMessage",_out._data.szMailMessage,sizeof(_out._data.szMailMessage));
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_OPEN_MAIL, ret, sizeof(DL_O_HALL_Open_Mail), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}




UINT CServiceDataBaseHandle::OnGetAttachment(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_Get_Attachment *_in = (DL_I_HALL_Get_Attachment*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_GetAttachMent",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@MailID", _in->_data.iMailID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_Get_Attachment _out;
	_out._data.iMailID = _in->_data.iMailID;

	int ret = getReturnValue();
	if (0 == ret)
	{
		getValue("IsGet",&_out._data.bMailState);
		getValue("SendLotteries",&_out._data.iLotteries);
		getValue("SendMoney",&_out._data.i64Money);
		getValue("SendJewels",&_out._data.iJewels);
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_GET_ATTACHMENT, ret, sizeof(DL_O_HALL_Get_Attachment), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}


UINT CServiceDataBaseHandle::OnMailDelete(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	DL_I_HALL_Delete_Mail *_in = (DL_I_HALL_Delete_Mail*)pSourceData;	

	bool bres=sqlSPSetNameEx("SP_MailDelete",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _in->iUserID);
	addInputParameter("@MailID", _in->_data.iMailID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_Delete_Mail _out;
	_out._data.iMailID = _in->_data.iMailID;

	int ret = getReturnValue();
	if (0 == ret)
	{
		getValue("IsGet",&_out._data.bMailState);
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_DEL_MAIL, ret, sizeof(DL_O_HALL_Delete_Mail), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}



UINT CServiceDataBaseHandle::OnMailUpdate(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;
	HNLOG_M("OnMailUpdate Begin");
	bool bres=sqlSPSetNameEx("SP_MailUpdate",true);
	if(!bres)
	{
		return 0;
	}

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_Mail_Update _out;

	int ret = getReturnValue();

	if (0 == ret)
	{
		_out.iNewNumber = getRecordCount();
		if (_out.iNewNumber > 0)
		{
			_out._data = new MSG_GP_O_Mail_Update[_out.iNewNumber];

			int iCount=0;
			while(!adoEndOfFile())
			{
				COleDateTime cBeginTime;

				getValue("MailID",&_out._data[iCount].MailData.iMailID);
				getValue("UserID",&_out._data[iCount].iUserID);
				getValue("IsNew",&_out._data[iCount].bNew);
				if (_out._data[iCount].bNew)
				{
					getValue("IsGet",&_out._data[iCount].MailData.bMailState);
					getValue("SendTime",&cBeginTime);
					getValue("KeepTime",&_out._data[iCount].MailData.bKeepTime);
					getValue("MailTitle",_out._data[iCount].MailData.szMailTitle,sizeof(_out._data[iCount].MailData.szMailTitle));
					getValue("SenderName",_out._data[iCount].MailData.szSendName,sizeof(_out._data[iCount].MailData.szSendName));

					SYSTEMTIME systime;
					VariantTimeToSystemTime(cBeginTime, &systime);
					CTime timeTmp(systime);
					_out._data[iCount].MailData.i64MailTime = timeTmp.GetTime();
				}
				
				iCount++;

				moveNext();
			}
			m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_UPDATE_MAIL, ret, sizeof(DL_O_HALL_Mail_Update), pSourceData->uIndex, pSourceData->dwHandleID);
		}
	}
	closeRecord();

	bres=sqlSPSetNameEx("SP_GetSysMsg",true);
	if(!bres)
	{
		return 0;
	}
	
	addInputParameter("@IsAllUpdate", 1);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_SysMsg _Sysout;
	_Sysout.bBroadcast = true;
	ret = getReturnValue();

	if (0 == ret)
	{
		getValue("MContent",_Sysout._data.szSysMessage,sizeof(_Sysout._data.szSysMessage));

		m_pRusultService->OnDataBaseResultEvent(&_Sysout.ResultHead, DTK_GP_UPDATE_SYSTEM, ret, sizeof(DL_O_HALL_SysMsg), pSourceData->uIndex, pSourceData->dwHandleID);
	}

	closeRecord();
	HNLOG_M("OnMailUpdate End");
	return 0;
}


UINT CServiceDataBaseHandle::OnGetSysMsg(DataBaseLineHead* pSourceData)
{
	if( pSourceData == NULL )	return 0;

	bool bres=sqlSPSetNameEx("SP_GetSysMsg",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@IsAllUpdate", 0);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_O_HALL_SysMsg _Sysout;
	_Sysout.bBroadcast = false;
	int ret = getReturnValue();

	if (0 == ret)
	{
		getValue("MContent",_Sysout._data.szSysMessage,sizeof(_Sysout._data.szSysMessage));
	}

	m_pRusultService->OnDataBaseResultEvent(&_Sysout.ResultHead, DTK_GP_UPDATE_SYSTEM, ret, sizeof(DL_O_HALL_SysMsg), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	return 0;
}

UINT CServiceDataBaseHandle::OnLuckDrawConfig(DataBaseLineHead* pSourceData)
{
    if( pSourceData == NULL )	return 0;
    DL_I_HALL_LUCK_DRAW_CONFIG*_in = (DL_I_HALL_LUCK_DRAW_CONFIG*)pSourceData;	

    bool bres=sqlSPSetNameEx("SP_GetLuckDrawInfo",true);
    if(!bres)
    {
        return 0;
    }

    addInputParameter("@UserID", _in->_data.iUserID);
    if(0 != execStoredProc())
    {
        closeRecord();
        return 0;
    }

    DL_O_HALL_LUCK_DRAW_CONFIG _out;
    ZeroMemory(&_out,sizeof(_out));

    int nRet = getReturnValue();

    if(nRet == 0)
    {
        int iLeaveFreeCount = 0;
        int iFreeTotalCount = 0;
        int i64Money = 0;
        getValue("FreeLuckDrawLeaveCount",&iLeaveFreeCount);
        _out._data.iLeaveFreeCount = iLeaveFreeCount;
        getValue("FreeTotalCount",&iFreeTotalCount);
        _out._data.iFreeTotalCount = iFreeTotalCount;
        getValue("LuckDrawChargeMoney",&i64Money);
        _out._data.i64Money = i64Money;

        int index = 0;
        int type = 0;
        int value = 0;
        while(!adoEndOfFile() && index < sizeof(_out._data.type) / sizeof(_out._data.type[0]) && index < sizeof(_out._data.value) / sizeof(_out._data.value[0]))
        {
            getValue("LuckDrawType",&type);
            getValue("LuckDrawValue",&value);
            _out._data.type[index] = type;
            _out._data.value[index] = value;
            moveNext();
            index++;
        }
    }
    m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_LUCK_DRAW_CONFIG, nRet, sizeof(DL_O_HALL_LUCK_DRAW_CONFIG), pSourceData->uIndex, pSourceData->dwHandleID);
    closeRecord();
    return true;
}

UINT CServiceDataBaseHandle::OnLuckDrawDo(DataBaseLineHead* pSourceData)
{
    if( pSourceData == NULL )	return 0;
    DL_I_HALL_LUCK_DRAW_DO*_in = (DL_I_HALL_LUCK_DRAW_DO*)pSourceData;	

    bool bres=sqlSPSetNameEx("SP_DoLuckDrawInfo",true);
    if(!bres)
    {
        return 0;
    }

    addInputParameter("@UserID", _in->_data.iUserID);
    if(0 != execStoredProc())
    {
        closeRecord();
        return 0;
    }

    DL_O_HALL_LUCK_DRAW_DO _out;
    ZeroMemory(&_out,sizeof(_out));

    int nRet = getReturnValue();

    if(nRet == 0)
    {
        int index = 0;
        getValue("awardindex",&index);
        _out._data.index = index;
    }
    m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GP_LUCK_DRAW_DO, nRet, sizeof(DL_O_HALL_LUCK_DRAW_DO), pSourceData->uIndex, pSourceData->dwHandleID);
    closeRecord();
    return true;
}

UINT CServiceDataBaseHandle::OnDissmissTimeOutDesk(DataBaseLineHead *pSourceData)
{
    bool bres=sqlSPSetNameEx("SP_DissmissDeskByMS",true);
    if(!bres)
    {
        return 0;
    }

    if(0 != execStoredProc())
    {
        closeRecord();
        return 0;
    }

    DL_O_DissmissDeskByMS outdata;
    while(!adoEndOfFile())
    {
        getValue("RoomID",&outdata.iRoomID);
        getValue("deskNo",&outdata.iDeskNo);
        getValue("deskpwd",outdata.szDeskPwd,sizeof(outdata.szDeskPwd));
        getValue("MasterID",&outdata.iMasterID);
        getValue("PositionLimit",&outdata.iPositionLimit);
        m_pRusultService->OnDataBaseResultEvent(&outdata.ResultHead, DTK_GP_DISSMISSDESKBYMS, 0, sizeof(DL_O_DissmissDeskByMS), 0, 0);
        moveNext();
    }

    closeRecord();
    return true;
}

UINT CServiceDataBaseHandle::OnGetSpRoomInfo(DataBaseLineHead* pSourceData)
{
    DL_GP_O_GetSPRoomInfo outdata;

    bool bres=sqlSPSetNameEx("SP_GetSPGameInfo",true);
    if(!bres)
    {
        m_pRusultService->OnDataBaseResultEvent(&outdata.ResultHead, DTK_GP_GET_SPROOMINFO, 1, sizeof(DL_GP_O_GetSPRoomInfo), pSourceData->uIndex, pSourceData->dwHandleID);
        return 0;
    }

    DL_GP_I_GetSPRoomInfo *pRoomInfo = (DL_GP_I_GetSPRoomInfo*)pSourceData;

    addInputParameter("@RoomID", pRoomInfo->_data.iRoomID);

    if(0 != execStoredProc())
    {
        m_pRusultService->OnDataBaseResultEvent(&outdata.ResultHead, DTK_GP_GET_SPROOMINFO, 2, sizeof(DL_GP_O_GetSPRoomInfo), pSourceData->uIndex, pSourceData->dwHandleID);
        closeRecord();
        return 0;
    }

    outdata._data.iRoomID = pRoomInfo->_data.iRoomID;
    getValue("RoomConfig",outdata._data.bRoomConfig,sizeof(outdata._data.bRoomConfig));
    m_pRusultService->OnDataBaseResultEvent(&outdata.ResultHead, DTK_GP_GET_SPROOMINFO, 0, sizeof(DL_GP_O_GetSPRoomInfo), pSourceData->uIndex, pSourceData->dwHandleID);


    closeRecord();
    return 1;
}