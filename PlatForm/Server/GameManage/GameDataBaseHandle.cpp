/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "Stdafx.h"
#include "BankMessage.h"
#include "PropDatabaseMessage.h"
#include "GameDataBaseHandle.h"

std::vector<ULONG> CGameDataBaseHandle::m_vecRandIPList;

CGameDataBaseHandle::CGameDataBaseHandle()
{
	m_ErrorSQLCount = 0;
}

//数据库处理接口
UINT CGameDataBaseHandle::HandleDataBase(DataBaseLineHead * pSourceData)
{
	switch (pSourceData->uHandleKind)
	{
    case DTK_GR_SET_SPROOMINFO:
        {
            return OnSetSPRoomInfo(pSourceData);
        }
	case DTK_GR_BANK_UPDATE_REBATE:
		{
			return 	OnUpdateRebate(pSourceData);
		}
	case DTK_GR_BANK_UPDATE_USER_REBATE:
	case DTK_GR_BANK_UPDATE_USER_REBATE_INGAME:
		{
			return OnUpdateUserRebate(pSourceData, pSourceData->uHandleKind);
		}
	case DTK_GR_BANK_SET_USER_REBATE://玩家修改返利数据
	case DTK_GR_BANK_SET_USER_REBATE_INGAME:
		{
			return OnBankTransferRebate(pSourceData, pSourceData->uHandleKind);
		}
	case DTK_GR_DEMAND_MONEY:								//打开钱柜
	case DTK_GR_DEMAND_MONEY_IN_GAME:						//从游戏界面中打开钱柜
		{
			return OnOpenWallet(pSourceData, pSourceData->uHandleKind);				
		}
	case DTK_GR_CHECKOUT_MONEY:								//取出金币
	case DTK_GR_CHECKOUT_MONEY_INGAME:						//从游戏界面中取出金币
		{	
			return OnCheckOutMoney(pSourceData, pSourceData->uHandleKind);
		}
	case DTK_GR_CHECKIN_MONEY:								//存入金币
	case DTK_GR_CHECKIN_MONEY_INGAME:						//从游戏界面中存入金币
		{
			return OnCheckInMoney(pSourceData, pSourceData->uHandleKind);
		}
	case DTK_GR_TRANSFER_MONEY:								//转帐
	case DTK_GR_TRANSFER_MONEY_INGAME:						//游戏中转帐
		{
			return OnTransferMoney(pSourceData, pSourceData->uHandleKind);
		}
	case DTK_GR_CHANGE_PASSWD:								///< 修改密码
	case DTK_GR_CHANGE_PASSWD_INGAME:						///< 修改密码
		{
			return OnChangePasswd(pSourceData, pSourceData->uHandleKind);
		}
	case DTK_GR_TRANSFER_RECORD:							///< 转帐记录
	case DTK_GR_TRANSFER_RECORD_INGAME:						///< 转帐记录
		{
			return OnTransferRecord(pSourceData, pSourceData->uHandleKind);
		}

	case DTK_GR_UPDATE_INSTANT_MONEY:						//即时更新金币积分
		{
			return OnModifyUserMoney(pSourceData);
		}
	case DTK_GR_UPDATE_CHARM:								//更新魅力值
		{
			return OnUpdateUserCharm(pSourceData);
		}
		//加上ZXJ 的方案
	case DTK_GR_CLEAR_ONLINE_FLAG:	//添加
		{
			return OnClearNetCutUserOnlineFlag(pSourceData);
		}
	case DTK_GR_CONTEST_RETURN_FEE://退回报名费
		{
			return OnContestReturnFee(pSourceData);
		}
	case DTK_GR_USER_RECOME:	//用户断线重连后写W记录
		{
			return OnReWriteWLoginRecord(pSourceData);
		}
	case DTK_GR_USER_CONTEST_LEFT:
		{
			return OnContestUserLeft(pSourceData);
		}
	case DTK_GR_PROP_USE:// DTK_GR_USER_USE_ONE_ITEM:							//玩家使用道俱
		{
			return OnUserUseOneProp(pSourceData);
		}
	case DTK_GPR_PROP_SMALL_BRD:
		{
			return OnUseSmallBoardcast(pSourceData);
		}

	case DTK_GR_LOGON_BY_ID:								//通过用户 ID 登陆
		{
			return OnRoomLogon(pSourceData);
		}
	case DTK_GR_USER_LEFT:									//用户离开房间
		{
			return OnUserLeft(pSourceData);
		}
	case DTK_GR_UPDATE_INFO:								//更新服务器信息
		{
			return OnUpdateServerInfo(pSourceData);
		}
	case DTK_GR_RECORD_GAME:		//记录游戏数据
		{
			return OnRecordGameInfo(pSourceData);
		}
	case DTK_GR_CHECK_CONNECT:		//检测数据库连接
		{
			return OnCheckDataConnect(pSourceData);
		}
	case DTK_GR_CONTEST_BEGIN:
		{
			return OnContestBegin(pSourceData);
		}
	case DTK_GR_CONTEST_GAME_OVER:
		{
			return OnContestGameOver(pSourceData);
		}
	case DTK_GR_UPDATE_GAME_RESULT:
		{
			return OnUpdateUserResult(pSourceData);
		}
	case DTK_GR_UPDATE_GAME_RESULT_ALLUSER:
		{
			return OnUpdateAllUserResult(pSourceData);
		}
	case DTK_GR_UPDATE_CONTEST_RESULT:
		{
			return OnUpdateContestInfo(pSourceData);
		}
	case DTK_GR_GET_CONTEST_RESULT:
		{
			return OnGetContestInfo(pSourceData);
		}
	case DTK_GR_CONTEST_APPLY:
		{
			return OnContestApply(pSourceData);
		}
	case DTK_GR_CONTEST_ABANDON:
		{
			return OnContestAbandon(pSourceData);
		}
	case DTK_GR_CALL_GM:						//呼叫GM
		{
			return OnCallGM(pSourceData);
		}
	case DTK_GR_GM_KICK_USER_RECORD:			//管理员踢人行为纪录
		{
			return OnGMKickUserRecord(pSourceData);
		}
	case DTK_GR_GM_WARNING_USER_RECORD:			//管理员发警告消息行为纪录
		{
			return OnGMWarningUserRecord(pSourceData);
		}
	case DTK_GR_SENDMONEY_TIMES:
		{
			return OnSendUserMoenyByTimes(pSourceData);
		}
	case DTK_GR_UNREGISTER_ONLINE:	//清理用户断线
		{
			return OnUnRegisterOnLine(pSourceData);
		}
	//修改禁止登录问题，修改插入TWLoginRecord 记录时机
	case DTK_GR_WRITE_WLOGINRECORD:
		{
			return OnWriteTWLoginRecord(pSourceData);
		}
	//wushuqun 2009.6.6
	//即时封桌功能
	case DTR_GR_ALONEDESK_INTIME:
		{
			return OnAloneDeskInTime(pSourceData);
		}
	case DTK_GR_ROOM_PW_CHANGE:
		{
			return OnRoomPWChangeRecord(pSourceData);
		}
    case DTK_GR_GETNICKNAME_ONID: // 根据用户ID获取昵称
	case DTK_GR_GETNICKNAME_ONID_INGAME:
        {
            return OnGetNickNameOnID(pSourceData);
        }
    case DTK_GR_USE_KICK_PROP:      // 使用踢人卡道具
        {
            return OnUseKickProp(pSourceData, TRUE);
        }
    case DTK_GR_USE_ANTI_KICK_PROP: // 使用防踢卡道具
        {
            return OnUseKickProp(pSourceData, FALSE);
        }
    case DTK_GR_USER_NEW_KICK_USER_PROP:    // 使用踢人卡踢人功能
        {
            return OnNewKickUserProp(pSourceData);
        }
	case DTK_GR_BANK_NORMAL:
		{
			return OnBankOperateNormal(pSourceData);
		}
	case DTK_GR_BANK_TRANSFER:
		{
			return OnBankOperateTransfer(pSourceData);
		}
	case DTK_GR_BANK_DEPOSIT:
		{
			return OnBankOperateDeposit(pSourceData);
		}
	case DTK_GR_BANK_WALLET:
		{
			return OnBankGetWallet(pSourceData);
		}
        break;
	case DTK_GR_BANK_QUERY_DEPOSIT:
		{
			return OnBankQueryDeposits(pSourceData);
		}
		break;
	case DTK_GR_PROP_CHANGE:
		{
			return OnPropChange(pSourceData);
		}
		break;
	case DTK_GR_TIMINGMATCH_GETQUEQUEUSERS:
		{
			return FillTimingMatchQueueUser(pSourceData);
		}break;
	case DTK_GR_FINDDESKBUYSTATION:
		{
			return OnFindBuyDeskStation(pSourceData);
		}break;
	case DTK_GR_CLEARBUYDESK:
		{
			return OnClearBuyDesk(pSourceData);
		}break;
	case DTK_GR_RESLEASEDESK:
		{
			return OnReleaseBuyDesk(pSourceData);
		}break;
	case DTK_GR_RETURNEDESK:
		{
			return OnReturnBuyDesk(pSourceData);
		}break;
	case DTK_GR_CUTNETRECORD:
		{
			return OnCutNet(pSourceData);
		}break;
	case DTK_GR_CUTNETRECORDSTART:
		{
			return OnCutNetRecordStart(pSourceData);
		}break;
	case DTK_GR_GAMERECORD:
		{
			return OnRecordGame(pSourceData);
		}break;
	case DTK_GR_CREATE_GAMESN:
		{
			return OnCreateGameSN(pSourceData);
		}break;
	case DTK_GR_RELEASERECORD:
		{
			return OnReleaseRecord(pSourceData);
		}break;
	case DTK_GR_GETRELEASEINFO:
		{
			return OnGetReleaseRecord(pSourceData);
		}break;
	case DTK_GR_GET_DESKINFO:
		{
			return OnGetDeskInfo(pSourceData);
		}break;
	case DTK_GR_CONTEST_ACTIVE:
		{
			return OnContestActive(pSourceData);
		}break;
	case DTK_GR_CONTEST_REGIST:
		{
			return OnContestRegist(pSourceData);
		}break;
	case DTK_GR_BUYROOM_COSTFEE:
		{
			return OnCostJewels(pSourceData);
		}break;
	case DTK_GR_MIDCOSTFEE:
		{
			return OnMiddleCostFee(pSourceData);
		}break;
	case DTK_GR_CLEAR_DATA:
		{
			return OnClearData(pSourceData);
		}break;
	case DTK_GR_GET_ROOMINFO:
		{
			return OnGetRoomInfo(pSourceData);
		}break;
	case DTK_GR_GET_CONTESTINFO:
		{
			return OnSetContestInfo(pSourceData);
		}break;
	case DTK_GR_UPDATE_NETCUTTIME:
		{
			return OnUpdateNetCutTime(pSourceData);
		}
	case DTK_GR_MASTER_LEAVE:
		{
			return OnMasterLeave(pSourceData);
		}
	case DTK_GR_CONTEST_EXIT:
		{
			return OnContestExit(pSourceData);
		}
	case DTK_GR_CHANGE_USERSTATION:
		{
			return OnChangeUserStation(pSourceData);
		}
    case DTK_GR_SETDYNAMICINFO:
        {
            return OnSetGameDynamicInfo(pSourceData);
        }
    case DTK_GR_UPDATEBEGINTIME:
        {
            return OnUpdateBeginTime(pSourceData);
        }
    case DTK_GR_DISSMISSDESKBYMS:
        {
            return OnDissmissDeskByMS(pSourceData);
        }
	case DTK_ROBOT_RAND_IP_LIST:
		{
			return OnRobotRandIPList(pSourceData);
		}
	}
	return 0;
}



//检测数据库连接
UINT CGameDataBaseHandle::OnCheckDataConnect(DataBaseLineHead * pSourceData)
{
	try 
	{ 
		//检测连接
		m_pDataBaseManage->CheckSQLConnect();

		//处理结果
		DataBaseResultLine ResultHead;
		m_pRusultService->OnDataBaseResultEvent(&ResultHead,DTK_GR_CHECK_CONNECT,0,sizeof(ResultHead),0,0);
	} 
	catch (...) {TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);}

	return 0;
}

//记录游戏信息
UINT CGameDataBaseHandle::OnRecordGameInfo(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_GameRecord)) throw DTR_GR_ERROR_UNKNOW;
	DL_GR_I_GameRecord * pGameRecord=(DL_GR_I_GameRecord *)pSourceData;

	// 体验场(免费场)不进行游戏记录
	if (m_pInitInfo->dwRoomRule & GRR_EXPERCISE_ROOM || m_pInitInfo->dwRoomRule & GRR_CONTEST || m_pInitInfo->dwRoomRule & GRR_TIMINGCONTEST)
	{
		return 0;
	}
		long int dwNowTime=(long int)time(NULL);
		//更新数据库
	
		bool bres=sqlSPSetNameEx("SP_RecordGameInfo_new",true);
		if(!bres)
		{
			return 0;
		}

		//更新数据库

		addInputParameter(TEXT("@GameTime"),dwNowTime-pGameRecord->dwBeginTime );
		addInputParameter(TEXT("@RoomID"),pGameRecord->uRoomID );
		addInputParameter(TEXT("@DeskIndex"),pGameRecord->bDeskIndex );
		addInputParameter(TEXT("@Tax"),pGameRecord->dwTax );

		CString strParam = "";
		TCHAR szTmp[128];
		int nUserCount = 0;
		CString dwjlog;
		for (int i=0; i<MAX_PEOPLE; ++i)
		{
			if (pGameRecord->dwUserID[i]==0)
			{
				continue;
			}
			wsprintf(szTmp, TEXT("%d,%I64d,%I64d,%I64d,%I64d,%I64d,")
				, pGameRecord->dwUserID[i]
				, pGameRecord->dwScrPoint[i]
				, pGameRecord->dwChangePoint[i]
				, pGameRecord->i64ScrMoney[i]
				, pGameRecord->dwChangeMoney[i]
				, pGameRecord->dwTaxCom[i]
			);
			//_tcscat(szParam, szTmp);
			strParam += szTmp;
			++nUserCount;
		}

		addInputParameter(TEXT("@UserCount"), nUserCount );
		addInputParameter(TEXT("@Param"), strParam.GetBuffer(), sizeof(TCHAR) * strParam.GetLength());

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		
		//检查混战场活动时间是否有效
		int ret=getReturnValue();
	
		//为0时不用处理，不存在该混战房间
		// =1 时,表示该混战房间已经不在活动时间内了
		if (ret == 1)
		{
			int iHandleResult = DTR_GR_BATTLEROOM_TIMEOUT_ERROR;
			DL_GR_O_BattleRecordResult BattleRecordRet;
			memset(& BattleRecordRet,0,sizeof(BattleRecordRet));
			BattleRecordRet.uRoomID = pGameRecord->uRoomID;
			BattleRecordRet.uRecordResult = DTR_GR_BATTLEROOM_TIMEOUT_ERROR;

			m_pRusultService->OnDataBaseResultEvent(&BattleRecordRet.ResultHead,DTK_GR_BATTLEROOM_RES,iHandleResult,
				sizeof(BattleRecordRet),pSourceData->uIndex,pSourceData->dwHandleID);
			closeRecord();
			return 0;
		}
		////////////////////////////
		closeRecord();
		return 0;

	return 0;
}

//用于清除断线用户在线标志
UINT CGameDataBaseHandle::OnClearNetCutUserOnlineFlag(DataBaseLineHead * pSourceData)
{
	DL_GR_I_ClearOnlineFlag *pClearOnlineFlag = (DL_GR_I_ClearOnlineFlag*)pSourceData;
	if(pClearOnlineFlag == NULL )
		return -1;

	bool bres=sqlSPSetNameEx("SP_DeleteAbnormalFlag",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", pClearOnlineFlag->lUserID);
	if(m_pInitInfo->dwRoomRule & GRR_CONTEST || m_pInitInfo->dwRoomRule & GRR_TIMINGCONTEST)
		addInputParameter("@MatchID", m_pInitInfo->iMatchID);
	else 
		addInputParameter("@MatchID", 0);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	closeRecord();
	return 0;
}

UINT CGameDataBaseHandle::OnReWriteWLoginRecord(DataBaseLineHead * pSourceData)
{
	WriteStr("用户断线重连，写W记录");
	DL_GR_I_UserRecome * pUserRecome = (DL_GR_I_UserRecome*)pSourceData;
	if( pSourceData == NULL )
		return -1;
	
	bool bres=sqlSPSetNameEx("SP_InsertAbnormalOffline",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", pUserRecome->lUserID);
	addInputParameter("@RoomID", m_pInitInfo->uRoomID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	closeRecord();
	return 0;
}

UINT CGameDataBaseHandle::OnContestReturnFee(DataBaseLineHead * pSourceData)
{
	DL_GR_I_ContestReturnFee * _p = (DL_GR_I_ContestReturnFee*)pSourceData;
	if(_p == NULL )
		return -1;
	bool bres=sqlSPSetNameEx("SP_ContestReturnFee",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _p->iUserID);
	addInputParameter("@ContestID", _p->iContestID);
	addInputParameter("@RoomID", _p->iRoomID);
	addInputParameter("@GameID", _p->iGameID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	closeRecord();
	return 0;
}

UINT CGameDataBaseHandle::OnContestUserLeft(DataBaseLineHead * pSourceData)
{
	DL_GR_I_ContestUserLeft * _p = (DL_GR_I_ContestUserLeft*)pSourceData;
	if(_p == NULL )
		return -1;

	bool bres=sqlSPSetNameEx("SP_ContestUserLeft",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID", _p->iUserID);
	addInputParameter("@ContestID", _p->iContestID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	closeRecord();
	return 0;
}


//更新服务器信息
UINT CGameDataBaseHandle::OnUpdateServerInfo(DataBaseLineHead * pSourceData)
{


	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_UpdateServerInfo)) throw DTR_GR_ERROR_UNKNOW;
	DL_GR_I_UpdateServerInfo * pUpdateInfo=(DL_GR_I_UpdateServerInfo *)pSourceData;

	bool bres=sqlSPSetNameEx("SP_UpdateRoomOnlinePeople",true);
	if(!bres)
	{
		return 0;
	}

	pUpdateInfo->uOnLineUserCount=pUpdateInfo->uOnLineUserCount;
	pUpdateInfo->uOnLineUserCount=__min(pUpdateInfo->uOnLineUserCount,m_pInitInfo->uMaxPeople);

	addInputParameter("@OnlineCount",pUpdateInfo->uOnLineUserCount);
	addInputParameter("@RoomID",pUpdateInfo->uRoomID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	closeRecord();
	HNLOG_G(m_pInitInfo->uRoomID,"OnUpdateServerInfo end uRoomID = %d uOnLineUserCount = %d", pUpdateInfo->uRoomID, pUpdateInfo->uOnLineUserCount);
	return 0;
}


UINT CGameDataBaseHandle::OnRoomLogon(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_LogonByIDStruct)) throw DTR_GR_ERROR_UNKNOW;
	DL_GR_I_LogonByIDStruct * pLogonInfo=(DL_GR_I_LogonByIDStruct *)pSourceData;

	//初始化登陆结果
	DL_GR_O_LogonResult UserLogonResult;
	memset(&UserLogonResult,0,sizeof(UserLogonResult));

	//定义变量
	BYTE bRegisterLogon=FALSE;

	int iHandleResult=DTR_GR_LOGON_SUCCESS;

	//IP 效验（每个房间的IP限制）

	TCHAR szIP[16];
	CTCPSocketManage::ULongToIP(pLogonInfo->uAccessIP,szIP);

	bool bres=sqlSPSetNameEx("SP_UserLoginRoom",true);
	if(!bres)
	{
		iHandleResult=DTR_GR_DATABASE_CUT;
		m_pRusultService->OnDataBaseResultEvent(&UserLogonResult.ResultHead,DTK_GR_LOGON_BY_ID,iHandleResult,
			sizeof(UserLogonResult),pSourceData->uIndex,pSourceData->dwHandleID);
		return 0;
	}

	addInputParameter(TEXT("@UserID"),pLogonInfo->LogonInfo.dwUserID);
	addInputParameter(TEXT("@PassMD5"),pLogonInfo->LogonInfo.szMD5Pass,sizeof(pLogonInfo->LogonInfo.szMD5Pass));
	addInputParameter(TEXT("@LoginIP"),szIP,sizeof(szIP));
	addInputParameter(TEXT("@RoomID"),m_pInitInfo->uRoomID);
	addInputParameter(TEXT("@KernelNameID"),pLogonInfo->LogonInfo.uNameID);
	addInputParameter(TEXT("@MatchTable"),m_pInitInfo->szGameTable,sizeof(m_pInitInfo->szGameTable));
	addInputParameter(TEXT("@IPRuleTable"),m_pInitInfo->szIPRuleTable,sizeof(m_pInitInfo->szIPRuleTable));
	addInputParameter(TEXT("@NameRuleTable"),m_pInitInfo->szNameRuleTable,sizeof(m_pInitInfo->szNameRuleTable));
	addInputParameter(TEXT("@LockTable"),m_pInitInfo->szLockTable,sizeof(m_pInitInfo->szLockTable));
	addInputParameter(TEXT("@MatchID"),1==m_pInitInfo->iRoomState?m_pInitInfo->iMatchID:0);
	addInputParameter(TEXT("@RoomState"),m_pInitInfo->iRoomState);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	int ret=getReturnValue();
	
	if(ret!=0)
	{
		iHandleResult = DTR_GR_ERROR_UNKNOW;
		if(ret==1)
			iHandleResult = DTR_GR_USER_IP_LIMITED;
		if(ret==2)
			iHandleResult = DTR_GR_USER_VALIDATA;
		if(ret==11)
		{
			//如果用户已经在其它房间，返回所在的房间ID
			DL_GR_O_LogonError _p;
			getValue("OldGameRoom", &_p.uRoomID);
			getValue("RoomName", _p.szGameRoomName, sizeof(_p.szGameRoomName));
			iHandleResult = DTR_GR_IN_OTHER_ROOM;
			m_pRusultService->OnDataBaseResultEvent(&_p.ResultHead,DTK_GR_LOGON_BY_ID,iHandleResult,
				sizeof(DL_GR_O_LogonError),pSourceData->uIndex,pSourceData->dwHandleID);
			closeRecord();
			return 0;
		}
		if(ret==3)
			iHandleResult = DTR_GR_USER_NO_FIND;
		if(ret==4)
			iHandleResult = DTR_GR_USER_PASS_ERROR;
		//不在混战场活动时间内
		if(ret==5)
		{
			iHandleResult = DTR_GR_BATTLEROOM_TIMEOUT;
		}
		if (ret == 9)
			iHandleResult = DTR_GR_CONTEST_NOSIGNUP;
		if (ret == 10)
			iHandleResult = DTR_GR_CONTEST_TIMEROUT;
		if (ret == 12)
			iHandleResult = DTR_GR_CONTEST_NOSTART;
		if (ret == 13)
			iHandleResult = DTR_GR_CONTEST_OVER;
		if (ret == 14)
			iHandleResult = DTR_GR_CONTEST_BEGUN;
		if (ret != 9 && ret != 12)
		{
			m_pRusultService->OnDataBaseResultEvent(&UserLogonResult.ResultHead,DTK_GR_LOGON_BY_ID,iHandleResult,
				sizeof(UserLogonResult),pSourceData->uIndex,pSourceData->dwHandleID);
			closeRecord();
			return 0;
		}
	}


	//注册登陆信息
	DL_GR_I_RegisterLogon RegUserOnLine;
	DL_GR_O_RegisterLogon RegUserResult;
	memset(&RegUserOnLine,0,sizeof(RegUserOnLine));
	memset(&RegUserResult,0,sizeof(RegUserResult));
	lstrcpy(RegUserOnLine.szAccessIP,szIP);
	RegUserOnLine.dwUserID=pLogonInfo->LogonInfo.dwUserID;
	UserLogonResult.bRegOnline=TRUE;


	//登陆成功
	long int dwGamePower=0,dwMasterPower=0,uMaster=0,uMemberPower=0;
    __int64 i64Money=0;
	long int dwFascination = 0L;//
	UINT uMatch=0;
	UserLogonResult.pUserInfoStruct.bDeskNO=255;///游戏桌号
	UserLogonResult.pUserInfoStruct.bDeskStation=255;//桌子位置
	UserLogonResult.pUserInfoStruct.bUserState=USER_LOOK_STATE;//用户状态
	UserLogonResult.pUserInfoStruct.dwUserIP=pLogonInfo->uAccessIP;//登录IP地址
	UserLogonResult.pUserInfoStruct.dwUserID=pLogonInfo->LogonInfo.dwUserID;//ID 号码

	getValue("UserName",UserLogonResult.pUserInfoStruct.szName,sizeof(UserLogonResult.pUserInfoStruct.szName));
	getValue("NickName",UserLogonResult.pUserInfoStruct.nickName,sizeof(UserLogonResult.pUserInfoStruct.nickName));
	int bb;
	getValue("sex",&bb);
	UserLogonResult.pUserInfoStruct.bBoy=(bb==0)?false:true;
	getValue("LogoID",&UserLogonResult.pUserInfoStruct.bLogoID);
	int iTime=0;
	getValue("viptime",&iTime);
	UserLogonResult.pUserInfoStruct.iVipTime=iTime;
	getValue("MatchMember",&uMatch);

	getValue("Wallet",&i64Money);
	if(m_pInitInfo->uComType==TY_MONEY_GAME && (m_pInitInfo->dwRoomRule & GRR_EXPERCISE_ROOM))
	{
		getValue("ExercisePoint",&i64Money);
	}

	getValue("BankMoney",&UserLogonResult.pUserInfoStruct.i64Bank);
	getValue("MasterPowerInRoom",&dwMasterPower);

	getValue("Win",&UserLogonResult.pUserInfoStruct.uWinCount);
	getValue("Lost",&UserLogonResult.pUserInfoStruct.uLostCount);
	getValue("Cut",&UserLogonResult.pUserInfoStruct.uCutCount);
	getValue("Mid",&UserLogonResult.pUserInfoStruct.uMidCount);

	// 获取个性签名
	getValue("SignDescr",UserLogonResult.pUserInfoStruct.szSignDescr,sizeof(UserLogonResult.pUserInfoStruct.szSignDescr));

	//登录房间时即时获取虚拟玩家人数
	getValue("VirualUser",&UserLogonResult.nVirtualUser);

	/// 用存储过程的返回值来给用户数据结构的isVirtual字段赋值，指示该用户是否为机器人帐号
	getValue("IsRobot", &UserLogonResult.pUserInfoStruct.isVirtual);

	getValue("ContestScore",&UserLogonResult.pUserInfoStruct.i64ContestScore);

	getValue("ContestCount",&UserLogonResult.pUserInfoStruct.iContestCount);
	getValue("RankNum",&UserLogonResult.pUserInfoStruct.iRankNum);

	getValue("HeadUrl",UserLogonResult.pUserInfoStruct.szHeadUrl,sizeof(UserLogonResult.pUserInfoStruct.szHeadUrl));//微信图像地址
	getValue("Jewels",&UserLogonResult.pUserInfoStruct.iJewels);//钻石
	getValue("LockJewels",&UserLogonResult.pUserInfoStruct.iLockJewels);//锁定钻石
	int nYear = 0, nMonth = 0, nDate = 0, nHour = 0, nMin = 0, nSec = 0, nss = 0;  
	CString temp;
	if (temp != "")
	{
		sscanf(temp.GetBuffer(), TEXT("%d-%d-%d %d:%d:%d.%d"), &nYear, &nMonth, &nDate, &nHour, &nMin, &nSec, &nss);

		CTime _t(nYear, nMonth, nDate, nHour, nMin, nSec); 
		UserLogonResult.timeLeft = _t.GetTime();
	}
	else
	{
		UserLogonResult.timeLeft = 0;
	}
	//添加登陆金币不足赠送
	if (1 == UserLogonResult.strRecMoney.iResultCode)
	{
		i64Money += UserLogonResult.strRecMoney.i64Money;
	}
	UserLogonResult.pUserInfoStruct.i64Money=i64Money;	
	UserLogonResult.dwGamePower|=dwGamePower;
	UserLogonResult.dwGamePower|=m_pInitInfo->dwUserPower;//游戏附加权限，所有的人都有
	UserLogonResult.dwMasterPower|=dwMasterPower;//总的管理权限
	CopyMemory(UserLogonResult.szMD5Pass, pLogonInfo->LogonInfo.szMD5Pass, sizeof(UserLogonResult.szMD5Pass));
	UserLogonResult.bGRMRoomID = m_pInitInfo->uNameID;

	UserLogonResult.pUserInfoStruct.flat = pLogonInfo->LogonInfo.flat;
	UserLogonResult.pUserInfoStruct.flnt = pLogonInfo->LogonInfo.flnt;
	UserLogonResult.pUserInfoStruct.bLogonbyphone = pLogonInfo->LogonInfo.bLogonbyphone;
	strcpy_s(UserLogonResult.pUserInfoStruct.szLocation,pLogonInfo->LogonInfo.szLocation);

	//机器人随机IP
	if (1==UserLogonResult.pUserInfoStruct.isVirtual) 
	{
		int iIPCount = CGameDataBaseHandle::m_vecRandIPList.size();
		if (iIPCount > 0)
		{
			int index = 0;
			index = rand()%iIPCount;
			UserLogonResult.pUserInfoStruct.dwUserIP = CGameDataBaseHandle::m_vecRandIPList.at(index);
		}		
	}

	closeRecord();
	
	//登陆房间发现金币不足，自动赠送,限金币场	
	if ((0 == ret) && (TY_MONEY_GAME == m_pInitInfo->uComType)&& !(m_pInitInfo->dwRoomRule & GRR_EXPERCISE_ROOM))
	{
		if (m_pInitInfo->bSendAlms && i64Money <= m_pInitInfo->iAlmsMinMoney)
		{
			DWORD dwUserID = pLogonInfo->LogonInfo.dwUserID;		
			GetWalletMoney(UserLogonResult.strRecMoney,dwUserID);
		}
	}

	//处理登陆
	m_pRusultService->OnDataBaseResultEvent(&UserLogonResult.ResultHead,DTK_GR_LOGON_BY_ID,iHandleResult,
		sizeof(UserLogonResult),pSourceData->uIndex,pSourceData->dwHandleID);
	closeRecord();

	return 0;
}

// PengJiLin, 2010-8-23, 根据用户ID获取昵称
UINT CGameDataBaseHandle::OnGetNickNameOnID(DataBaseLineHead * pSourceData)
{
    // 效验数据
    if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_GetNickNameOnID)) throw DTR_GR_ERROR_UNKNOW;
    DL_GR_I_GetNickNameOnID * pGetNickNameOnID =(DL_GR_I_GetNickNameOnID *)pSourceData;

    // 初始化结果
    DL_GR_O_GetNickNameOnIDResult GetNickNameOnIDResult;
    memset(&GetNickNameOnIDResult,0,sizeof(GetNickNameOnIDResult));

    int iHandleResult=DTR_GR_LOGON_SUCCESS;

	bool bres=sqlSPSetNameEx("SP_GetNickNameOnID",true);
	if(!bres)
	{
		iHandleResult=DTR_GR_DATABASE_CUT;
		m_pRusultService->OnDataBaseResultEvent(&GetNickNameOnIDResult.ResultHead,pSourceData->uHandleKind,
			iHandleResult,sizeof(GetNickNameOnIDResult),pSourceData->uIndex,pSourceData->dwHandleID);
		return 0;
	}

    addInputParameter(TEXT("@UserID"),pGetNickNameOnID->stGetNickNameOnID.iUserID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
    int ret=getReturnValue();
   getValue("NickName",GetNickNameOnIDResult.szNickName,sizeof(GetNickNameOnIDResult.szNickName));
    closeRecord();
    // 操作失败
    if(ret!=0)
    {
        iHandleResult = DTR_GR_ERROR_UNKNOW;
        if(ret==1)      // ID 不存在
            iHandleResult = DTR_GR_GETNICKNAME_NOTEXIST;
        if(ret==2)      // 昵称为空
            iHandleResult = DTR_GR_GETNICKNAME_ISNULL;

		m_pRusultService->OnDataBaseResultEvent(&GetNickNameOnIDResult.ResultHead,pSourceData->uHandleKind,
                            iHandleResult,sizeof(GetNickNameOnIDResult),pSourceData->uIndex,pSourceData->dwHandleID);
        return 0;
    }

    // 操作成功
    iHandleResult = DTR_GR_GETNICKNAME_SUCCESS;
    GetNickNameOnIDResult.iUserID = pGetNickNameOnID->stGetNickNameOnID.iUserID;    
	m_pRusultService->OnDataBaseResultEvent(&GetNickNameOnIDResult.ResultHead,pSourceData->uHandleKind,iHandleResult,
                            sizeof(GetNickNameOnIDResult),pSourceData->uIndex,pSourceData->dwHandleID);
    return 0;
}

// PengJiLin, 2010-9-10, 使用踢人卡、防踢卡道具
UINT CGameDataBaseHandle::OnUseKickProp(DataBaseLineHead * pSourceData, BOOL bIsKickUser)
{
    // 效验数据
    if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_UseKickProp)) throw DTR_GR_ERROR_UNKNOW;
    DL_GR_I_UseKickProp* pUseKickProp =(DL_GR_I_UseKickProp *)pSourceData;

    // 是踢人卡还是防踢卡
    int iHandleIDNum = DTK_GR_USE_KICK_PROP;
    if(FALSE == bIsKickUser)
    {
        iHandleIDNum = DTK_GR_USE_ANTI_KICK_PROP;
    }

    // 初始化结果
    DL_GR_O_UseKickProp UseKickProp;
    memset(&UseKickProp,0,sizeof(UseKickProp));

    UseKickProp.dwUserID = pUseKickProp->dwUserID;
    UseKickProp.iPropID = pUseKickProp->iPropID;

	bool bres=sqlSPSetNameEx("SP_UseProp",true);
	if(!bres)
	{
		return 0;
	}

    addInputParameter("@UserID", pUseKickProp->dwUserID);
    addInputParameter("@PropID", pUseKickProp->iPropID);
    addInputParameter("@TargetID", 0);

    //添加NameID，用于负分清零
    addInputParameter("@GameNameID",0);
    char chNotUse[10] = {0};
    addInputParameter("@GameTable",chNotUse,sizeof(chNotUse));
    //是否负分清零
    addInputParameter("@IsClearMinus", 0);
    //是否大小喇叭 // PengJiLin, 2010-9-10, 置1，会直接返回不做其他处理
    addInputParameter("@IsBoard",1);
    //双倍积分变化

    addInputParameter("@DoubleTime",0);
    //护身符变化
    addInputParameter("@ProtectTime",0);
    //魅力值变化
    addInputParameter("@FasciChange",0);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

    int ret=getReturnValue();
    closeRecord();
    int iHandleResult = DTR_GR_USE_KICKPROP_SUCCESS;

    // 操作失败，没有道具了
    if(0 != ret)
    {
        iHandleResult = DTR_GR_USE_KICKPROP_NULL;
        m_pRusultService->OnDataBaseResultEvent(&UseKickProp.ResultHead, iHandleIDNum, 
                                                iHandleResult, sizeof(DL_GR_O_UseKickProp), 
                                                pSourceData->uIndex, pSourceData->dwHandleID);
        return 0;
    }

    // 操作成功，处理数值
	bres=sqlSPSetNameEx("SP_UseKickProp",true);
	if(!bres)
	{
		return 0;
	}

    addInputParameter("@UserID", pUseKickProp->dwUserID);
    addInputParameter("@PropID", pUseKickProp->iPropID);
    addInputParameter("@bIsKick", bIsKickUser?1:0);  // 0 = 防踢卡, 1 = 踢人卡

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

    ret=getReturnValue();
    getValue("TotalTime", &UseKickProp.iTotalTime);
    closeRecord();
    iHandleResult = DTR_GR_USE_KICKPROP_SET_SUCCESS;

    // 操作失败
    if(0 != ret)
    {
        iHandleResult = DTR_GR_USE_KICKPROP_SET_ERROR;
    }

    m_pRusultService->OnDataBaseResultEvent(&UseKickProp.ResultHead, iHandleIDNum, 
                                            iHandleResult, sizeof(DL_GR_O_UseKickProp), 
                                            pSourceData->uIndex, pSourceData->dwHandleID);
    return 0;
}

// 使用踢人卡踢人功能
UINT CGameDataBaseHandle::OnNewKickUserProp(DataBaseLineHead * pSourceData)
{
    // 效验数据
    if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_NewKickUserProp)) throw DTR_GR_ERROR_UNKNOW;
    DL_GR_I_NewKickUserProp* pUseKickProp =(DL_GR_I_NewKickUserProp *)pSourceData;

    // 初始化结果
    DL_GR_O_UseNewKickUserProp UseKickProp;
    memset(&UseKickProp,0,sizeof(UseKickProp));

    UseKickProp.dwDestIndex = pUseKickProp->dwDestIndex;
    UseKickProp.dwDestHandleID = pUseKickProp->dwDestHandleID;
    UseKickProp.dwUserID = pUseKickProp->dwUserID;
    UseKickProp.dwDestID = pUseKickProp->dwDestID;

	bool bres=sqlSPSetNameEx("SP_NewKickUserProp",true);
	if(!bres)
	{
		return 0;
	}

    addInputParameter("@UserID", pUseKickProp->dwUserID);
    addInputParameter("@TargetID", pUseKickProp->dwDestID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

    int ret=getReturnValue();
    closeRecord();
    int iHandleResult = DTR_GR_NEW_KICKUSER_SUCCESS;
    switch(ret)
    {
    case 1:
        iHandleResult = DTR_GR_NEW_KICKUSER_NOTIME; // 自己的道具已过期，或者没有道具
        break;
    case 2:
        iHandleResult = DTR_GR_NEW_KICKUSER_HAD_ANTI; // 对方有防踢卡时间
        break;
    case 3:
        iHandleResult = DTR_GR_NEW_KICKUSER_HAD_ANTI; // 对方有VIP时间
        break;
    }

    m_pRusultService->OnDataBaseResultEvent(&UseKickProp.ResultHead, DTK_GR_USER_NEW_KICK_USER_PROP, 
                                        iHandleResult, sizeof(DL_GR_O_UseNewKickUserProp), 
                                        pSourceData->uIndex, pSourceData->dwHandleID);

	return 0;
}


UINT CGameDataBaseHandle::OnBankOperateNormal(DataBaseLineHead * pSourceData)
{
	try
	{
		int iHandleCode = DTR_GR_ERROR_UNKNOW;
		db_bank_op_normal_in* _p = (db_bank_op_normal_in*)pSourceData;
		db_bank_op_normal_out _out;
		ZeroMemory(&_out, sizeof(_out));
		_out._data = _p->_data;

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
		iHandleCode = iRet;

		if (iHandleCode == HC_BANK_OP_SUC)
		{
			getValue("MoneyOp", &_out._data._money);
		}

		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GR_BANK_NORMAL, iHandleCode,
			sizeof(db_bank_op_normal_out), pSourceData->uIndex, pSourceData->dwHandleID);
		closeRecord();
	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}

UINT CGameDataBaseHandle::OnBankOperateTransfer(DataBaseLineHead * pSourceData)
{
	try
	{
		int iHandleCode = DTR_GR_ERROR_UNKNOW;
		db_bank_op_transfer_in* _p = (db_bank_op_transfer_in*)pSourceData;
		db_bank_op_transfer_out _out;
		ZeroMemory(&_out, sizeof(_out));
		_out._data = _p->_data;

		bool bres=sqlSPSetNameEx("SP_BankOpTransfer",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID1", _p->_data._user_id1);
		addInputParameter("@UserID2", _p->_data._user_id2);
		CString s;
		s.Format("%I64d", _p->_data._money);
		addInputParameter("@Money", s.GetBuffer(), sizeof(TCHAR)*s.GetLength());

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		int iRet = getReturnValue();
		iHandleCode = iRet;

		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GR_BANK_TRANSFER, iHandleCode,
			sizeof(db_bank_op_normal_out), pSourceData->uIndex, iRet);
		closeRecord();
	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}

UINT CGameDataBaseHandle::OnBankOperateDeposit(DataBaseLineHead * pSourceData)
{
	try
	{
		int iHandleCode = DTR_GR_ERROR_UNKNOW;
		db_bank_op_deposit_in* _p = (db_bank_op_deposit_in*)pSourceData;
		db_bank_op_deposit_out _out;
		ZeroMemory(&_out, sizeof(_out));
		_out._data = _p->_data;

		HANDLE hDatabase;
		HANDLE hRecord;
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
			iHandleCode = iRet;

			if (iHandleCode == 0)
			{
				getValue("DepositID", _out._data._deposit_id,sizeof(_out._data._deposit_id));
				getValue("DepositPassWord",_out._data._deposit_password,sizeof(_out._data._deposit_password));
				_out._data._state = 0;
				getValue("CDSC", &_out._data._csds);
				getValue("Money",& _out._data._money);
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
			iHandleCode = iRet;

			if (iHandleCode == 0)
			{
				_out._data._state = 1;
				getValue("UserID", &_out._data._user_id);
				getValue("NickName", _out._data._szNickName,sizeof(_out._data._szNickName));
				getValue("Money", &_out._data._money);
			}
		}

		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GR_BANK_DEPOSIT, iHandleCode,
			sizeof(db_bank_op_deposit_out), pSourceData->uIndex, iRet);
		closeRecord();
	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}

UINT CGameDataBaseHandle::OnPropChange(DataBaseLineHead * pSourceData)
{
//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(TpropChange_I)) throw DTR_GR_ERROR_UNKNOW;
	TpropChange_I * _p = (TpropChange_I *)pSourceData;

	if (_p == NULL) return 0;

	TpropChange_O _q;
	try
	{
		bool bres=sqlSPSetNameEx("SP_PropChange",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserId", _p->info.iUserId);
		addInputParameter("@PropId", _p->info.iPropId);
		addInputParameter("@PropCount", _p->info.iPropCount);
		addInputParameter("@OpType", _p->info.iOpType);
		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		int ret=getReturnValue();
		int iHandleResult = ret;

		_q.info = _p->info; 
		_q.info.iPropCount = 0;

		if(!adoEndOfFile())
		{
			getValue("PropCount", &_q.info.iPropCount);
		}

		closeRecord();

		m_pRusultService->OnDataBaseResultEvent(&_q.ResultHead, DTK_GR_PROP_CHANGE, 
			iHandleResult, sizeof(TpropChange_O), 
			pSourceData->uIndex, pSourceData->dwHandleID);

	}
	catch (...) {}
	return 0;
}
UINT CGameDataBaseHandle::OnBankQueryDeposits(DataBaseLineHead * pSourceData)
{
	try
	{
		int iHandleCode = DTR_GR_ERROR_UNKNOW;
		db_bank_op_query_deposit_in* _p = (db_bank_op_query_deposit_in*)pSourceData;
		db_bank_op_query_deposit_out _out;
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
		iHandleCode = iRet;

		int nSeriNo = 0;

		while(!adoEndOfFile())
		{
			ZeroMemory(&_out,sizeof(db_bank_op_query_deposit_out));

			_out._data._serial_no = nSeriNo++;
			getValue("DepositID",_out._data._deposit_id,sizeof(_out._data._deposit_id));
			getValue("DepositPassWord",_out._data._deposit_password,sizeof(_out._data._deposit_password));
			getValue("Money",&_out._data._money);
			getValue("UserID",&_out._data._user_id);
			getValue("GreateTime",&_out._data._create_time);
			getValue("CDSC",&_out._data._csds);
			getValue("State",&_out._data._state);
			getValue("UsedUserID",&_out._data._used_user_id);

			m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GR_BANK_QUERY_DEPOSIT, 0, sizeof(db_bank_op_query_deposit_out),pSourceData->uIndex,pSourceData->dwHandleID);
			moveNext();
		}

		ZeroMemory(&_out,sizeof(db_bank_op_query_deposit_out));
		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GR_BANK_QUERY_DEPOSIT, 1, sizeof(db_bank_op_query_deposit_out), pSourceData->uIndex, pSourceData->dwHandleID);
		closeRecord();
	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}

UINT CGameDataBaseHandle::OnBankGetWallet(DataBaseLineHead * pSourceData)
{
	try
	{
		int iHandleCode = DTR_GR_ERROR_UNKNOW;
		db_bank_op_wallet_in* _p = (db_bank_op_wallet_in*)pSourceData;
		db_bank_game_wallet_out _out;
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
		iHandleCode = iRet;

		while(!adoEndOfFile())
		{
			ZeroMemory(&_out,sizeof(db_bank_game_wallet_out));

			getValue("GameID",&_out._data._game_id);
			getValue("ComName",_out._data._game_name,sizeof(_out._data._game_name));
			getValue("WalletMoney",&_out._data._money);

			m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GR_BANK_WALLET, 0, sizeof(db_bank_game_wallet_out),pSourceData->uIndex,pSourceData->dwHandleID);
			moveNext();
		}

		ZeroMemory(&_out,sizeof(db_bank_game_wallet_out));
		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GR_BANK_WALLET, 1, sizeof(db_bank_game_wallet_out), pSourceData->uIndex, pSourceData->dwHandleID);
		closeRecord();
	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}

//取款
UINT CGameDataBaseHandle::OnCheckOutMoney(DataBaseLineHead *pSourceData, UINT uCode)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_CheckOutMoneyWithPwd_t)) throw DTR_GR_ERROR_UNKNOW;//////接受的结构和定义的是否一致
	DL_GR_I_CheckOutMoneyWithPwd_t * pCheckOutMoney=(DL_GR_I_CheckOutMoneyWithPwd_t * )pSourceData;
	DL_GR_O_CheckOutMoney pCheckOutMoneyResult;///结果
	memset(&pCheckOutMoneyResult,0,sizeof(pCheckOutMoneyResult));
	if(pCheckOutMoney->i64Count<0) return 0; 

	try
	{
		__int64 i64Bank=0,i64Money=0,i64CheckInMoney = 0;

		bool bres=sqlSPSetNameEx("SP_BankMoneyOpera",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID",pCheckOutMoney->dwUserID);
		CString s;
		s.Format("%I64d", pCheckOutMoney->i64Count);
		addInputParameter("@MoneyQuantitys", s.GetBuffer(), sizeof(TCHAR)*s.GetLength());
		addInputParameter("@RoomID",m_pInitInfo->uRoomID);
		addInputParameter("@IsSave",0);	//0-取款,1-存款
		addInputParameter("@MD5Pass",pCheckOutMoney->szMD5Pass, sizeof(pCheckOutMoney->szMD5Pass));//密码

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}

		int ret=getReturnValue();
		
		if(ret!=0)
		{
			closeRecord();
			if (ret == 5)
			{
				throw DTR_GR_CHECKOUT_MONEY_INV;
			}
			else
			{
				throw DTR_GR_ERROR_UNKNOW;
			}
			
		}
		 
		getValue("MoneyInBank",&i64Bank);
		getValue("MoneyChange",&i64CheckInMoney);
		getValue("MoneyInWallet",&i64Money);

		closeRecord();

		pCheckOutMoneyResult.dwUserID=pCheckOutMoney->dwUserID;
		pCheckOutMoneyResult.i64CheckOut=i64CheckInMoney;
		pCheckOutMoneyResult.i64MoneyInBank=i64Bank /*- i64CheckInMoney*/;//取消i64CheckInMoney，服务器返回的已经是操作后的结果 [2016/6/1/15 lwz]
		pCheckOutMoneyResult.i64MoneyInRoom=i64Money/* + i64CheckInMoney*/;
		throw DTR_GR_CHECKOUT_MONEY_SUC;

	}
	catch (int iHandleResult)
	{TRACE("EXCEPT CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	m_pRusultService->OnDataBaseResultEvent(&pCheckOutMoneyResult.ResultHead, uCode, iHandleResult,
		sizeof(pCheckOutMoneyResult),pSourceData->uIndex,pSourceData->dwHandleID);
	}
	return 0;
}

//存款
UINT CGameDataBaseHandle::OnCheckInMoney(DataBaseLineHead *pSourceData, UINT uCode)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_CheckOutMoney)) throw DTR_GR_ERROR_UNKNOW;//////接受的结构和定义的是否一致
	DL_GR_I_CheckOutMoney * pCheckInMoney=(DL_GR_I_CheckOutMoney * )pSourceData;
	DL_GR_O_CheckOutMoney pCheckInMoneyResult;///结果
	memset(&pCheckInMoneyResult,0,sizeof(pCheckInMoneyResult));
	if(pCheckInMoney->i64Count<0) return 0; 


	try
	{
		__int64 i64Bank=0,i64Money=0, i64CheckInMoney = 0;

		bool bres=sqlSPSetNameEx("SP_BankMoneyOpera",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID",pCheckInMoney->dwUserID);
		//TCHAR _num[32];
		//ZeroMemory(_num, sizeof(_num));
		//_i64tow(pCheckInMoney->i64Count, _num, 10);
		CString s;
		s.Format("%I64d", pCheckInMoney->i64Count);
		addInputParameter("@MoneyQuantitys", s.GetBuffer(), sizeof(TCHAR)*s.GetLength());
		addInputParameter("@RoomID",m_pInitInfo->uRoomID);
		addInputParameter("@IsSave",1);	//0-取款,1-存款
		addInputParameter("@MD5Pass", "", sizeof(""));//密码

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}

		int ret=getReturnValue();

		if(ret!=0)
		{
			closeRecord();
			if (ret == 5)
			{
				throw DTR_GR_CHECKIN_MONEY_INV;
			}
			else
			{
				throw DTR_GR_ERROR_UNKNOW;
			}
		}

		getValue("MoneyInBank",&i64Bank);
		getValue("MoneyChange",&i64CheckInMoney);
		getValue("MoneyInWallet",&i64Money);
		
		closeRecord();

		pCheckInMoneyResult.dwUserID=pCheckInMoney->dwUserID;
		pCheckInMoneyResult.i64CheckOut=i64CheckInMoney;
		pCheckInMoneyResult.i64MoneyInBank=i64Bank/* + i64CheckInMoney*/;// 取消i64CheckInMoney，服务器返回的已经是操作后的结果 [2016/6/1/15 lwz]
		pCheckInMoneyResult.i64MoneyInRoom=i64Money /*- i64CheckInMoney*/;
		throw DTR_GR_CHECKIN_MONEY_SUC;

	}
	catch (int iHandleResult)
	{TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		m_pRusultService->OnDataBaseResultEvent(&pCheckInMoneyResult.ResultHead, uCode, iHandleResult,
			sizeof(pCheckInMoneyResult),pSourceData->uIndex,pSourceData->dwHandleID);
	}
	return 0;
}
//转帐记录
UINT CGameDataBaseHandle::OnTransferRecord(DataBaseLineHead *pSourceData, UINT uCode)
{
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_TransferRecord_t)) throw DTR_GR_ERROR_UNKNOW;//////接受的结构和定义的是否一致
	DL_GR_I_TransferRecord_t *pTransferRecord = (DL_GR_I_TransferRecord_t *)pSourceData;
	DL_GR_O_TransferRecord_t stTransferRecordResult;
	ZeroMemory(&stTransferRecordResult, sizeof(DL_GR_O_TransferRecord_t));
	if (pTransferRecord->stTransfer.dwUserID == 0)
	{
		return 0;
	}
	{
		bool bres=sqlSPSetNameEx("SP_GetTransferRecord",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID",pTransferRecord->stTransfer.dwUserID);
		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		int ret=getReturnValue();
		if (ret>0)
		{
			closeRecord();
			return 0;
		}
		/// 每读取一条记录，就返回一次，直到读取结束，smalldatetime类型的数据在客户端解析
		int nSeriNo = 0;
		while(!adoEndOfFile())
		{
			stTransferRecordResult.transferRecord.nSeriNo = nSeriNo++;
			getValue("UserID", &stTransferRecordResult.transferRecord.dwUserID);
			getValue("DestUserID", &stTransferRecordResult.transferRecord.destUserID);
			getValue("TransferMoney", &stTransferRecordResult.transferRecord.i64Money);
			getValue("ActualTransfer", &stTransferRecordResult.transferRecord.i64ActualTransfer);
			getValue("TransTime", &stTransferRecordResult.transferRecord.oleDateTime);

			m_pRusultService->OnDataBaseResultEvent(&stTransferRecordResult.ResultHead,
				uCode, DTR_GR_OPEN_WALLET_SUC,
				sizeof(stTransferRecordResult),
				pSourceData->uIndex,pSourceData->dwHandleID);
			moveNext();
		}
		closeRecord();
	}

	return 0;
}
//修改密码
UINT CGameDataBaseHandle::OnChangePasswd(DataBaseLineHead *pSourceData, UINT uCode)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_ChangePasswd_t))
	{
		return 0;
	}
	DL_GR_I_ChangePasswd_t *pChangePasswd = (DL_GR_I_ChangePasswd_t *)pSourceData;
	DL_GR_O_ChangePasswd_t changePasswdResult;

	{
		bool bres=sqlSPSetNameEx("SP_ChangeBankPasswd",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID",pChangePasswd->stChangePasswd.UserID);
		addInputParameter("@MD5PassOld",pChangePasswd->stChangePasswd.szMD5PassOld, sizeof(pChangePasswd->stChangePasswd.szMD5PassOld));//旧密码
		addInputParameter("@MD5PassNew",pChangePasswd->stChangePasswd.szMD5PassNew, sizeof(pChangePasswd->stChangePasswd.szMD5PassNew));//新密码
		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}

		int nRet = getReturnValue();
		changePasswdResult.nResult = nRet;
		closeRecord();
		UINT uResult = DTR_GR_OPEN_WALLET_SUC;
		if (nRet != 0)
		{
			uResult = DTR_GR_OPEN_WALLET_ERROR_PSW;
		}
		// 只有成功和失败两种结果
		m_pRusultService->OnDataBaseResultEvent(&changePasswdResult.ResultHead,
			uCode, uResult,
			sizeof(changePasswdResult),
			pSourceData->uIndex,pSourceData->dwHandleID);
	}

	return 0;
}
//转帐
UINT CGameDataBaseHandle::OnTransferMoney(DataBaseLineHead *pSourceData, UINT uCode)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_TransferMoney))
	{
		return 0;
	}
	DL_GR_I_TransferMoney * pTransferMoney=(DL_GR_I_TransferMoney * )pSourceData;
	DL_GR_O_TransferMoney stTransferMoneyResult;///结果
	memset(&stTransferMoneyResult,0,sizeof(stTransferMoneyResult));
	if(pTransferMoney->stTransfer.i64Money <= 0) return 0; 

    // PengJiLin, 2010-8-23, 增加银行“全转”功能

    // PengJiLin, 2011-7-13, 32位数据转到64位数据
    UINT u32Data = 0;

	try
	{
		bool bres;
        if(1 == pTransferMoney->stTransfer.uHandleCode)
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

		addInputParameter("@UserID",pTransferMoney->stTransfer.UserID);
		addInputParameter("@UseDestID",pTransferMoney->stTransfer.bUseDestID?1:0);
		addInputParameter("@DestUserID",pTransferMoney->stTransfer.destUserID);
		addInputParameter("@DestNickName",pTransferMoney->stTransfer.szDestNickName, sizeof(pTransferMoney->stTransfer.szDestNickName));//密码
		if(0 == pTransferMoney->stTransfer.uHandleCode)     // PengJiLin, 2010-8-23, 全转功能
           addInputParameter("@TranMoney",pTransferMoney->stTransfer.i64Money);
		addInputParameter("@TranTax",pTransferMoney->stTransfer.bTranTax);
		addInputParameter("@MD5Pass",pTransferMoney->stTransfer.szMD5Pass, sizeof(pTransferMoney->stTransfer.szMD5Pass));//密码

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		int ret=getReturnValue();
		UINT uResult = DTR_GR_TRANSFER_MONEY_SUC;
		switch(ret)
		{
		case 0: ///<转帐成功
			{
			getValue("DestUserID",&pTransferMoney->stTransfer.destUserID);
			getValue("TranMoney",&pTransferMoney->stTransfer.i64Money);
			getValue("ActualTransfer",&pTransferMoney->stTransfer.i64ActualTransfer);
			break;
			}
		case 1:	///<密码错误
			{
			uResult = DTR_GR_OPEN_WALLET_ERROR_PSW;
			break;
			}
		case 2: ///<银行中钱少于多少不能转帐
			{
			uResult = DTR_GR_TRANSFER_TOTAL_NOT_ENOUGH;
			getValue("LessAllMoney", &pTransferMoney->stTransfer.i64ActualTransfer);
			break;
			}
		case 3: ///<单笔转帐必须大于多少
			uResult = DTR_GR_TRANSFER_TOO_LESS;
			getValue("LessMoney", &pTransferMoney->stTransfer.i64ActualTransfer);
			break;
		case 4: ///<单笔转帐是某数的整数倍
			uResult = DTR_GR_TRANSFER_MULTIPLE;
			getValue("Multiple", &u32Data);
            pTransferMoney->stTransfer.i64ActualTransfer = u32Data;
			break;
		case 5: ///<银行里只有多少钱，本次想要转多少钱，不够转
			uResult = DTR_GR_TRANSFER_NO_ENOUGH_MONEY;
			getValue("TransMoney", &pTransferMoney->stTransfer.i64Money);
			getValue("BankMoney", &pTransferMoney->stTransfer.i64ActualTransfer);
			break;
		case 6: ///<目标帐户不存在
			uResult = DTR_GR_TRANSFER_MONEY_NO_DEST;
			break;
		case 7:
			uResult = DTR_GR_TRANSFER_TOO_MANY_TIME;
			break;
		default:
			return 0;
		}
		closeRecord();
		::memcpy(&(stTransferMoneyResult.stTransfer),&(pTransferMoney->stTransfer),sizeof(MSG_GR_S_TransferMoney));
		m_pRusultService->OnDataBaseResultEvent(&stTransferMoneyResult.ResultHead,
			 uCode, uResult, sizeof(stTransferMoneyResult), pSourceData->uIndex,pSourceData->dwHandleID);
	}
	catch (...)
	{
		return 0;
	}
	return 0;
}


//提取或者存储返利
UINT CGameDataBaseHandle::OnBankTransferRebate(DataBaseLineHead* pSourceData, UINT uCode)
{
	try
	{
		DL_GR_I_UserRebateSetStruct* pCmd = (DL_GR_I_UserRebateSetStruct*)pSourceData;
		DL_GR_O_UserRebateSetStruct cmd;
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

		m_pRusultService->OnDataBaseResultEvent(&cmd.ResultHead,uCode,iRet,
			sizeof(DL_GR_O_UserRebateSetStruct),pSourceData->uIndex,pSourceData->dwHandleID);
		closeRecord();
	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}

//更新转账返利数据
UINT CGameDataBaseHandle::OnUpdateRebate(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_RebateUpDateStruct)) return 0;

	execSQL(TEXT("select Min_money,Min_money from Web_Config"), true);
	//结果数据定义
	DL_GR_O_RebateUpDateStruct Rebate;
	::memset(&Rebate,0,sizeof(Rebate));
	//处理数据
	int iHandleResult=0;//初始化
	//先检测数据库连接是否还在，不在就再连
	getValue(TEXT("Min_money"),&Rebate.iMin_money);
	getValue(TEXT("VIP_Rate"),&Rebate.iVIP_Rate);

	iHandleResult=1;//成功

	closeRecord();


	m_pRusultService->OnDataBaseResultEvent(&Rebate.ResultHead,DTK_GR_BANK_UPDATE_REBATE,iHandleResult,
		sizeof(Rebate),pSourceData->uIndex,pSourceData->dwHandleID);

	return 0;
}

//更新玩家返利数据
UINT CGameDataBaseHandle::OnUpdateUserRebate(DataBaseLineHead * pSourceData, UINT uCode)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_UserRebateUpDateStruct)) 
		return 0;
	DL_GR_I_UserRebateUpDateStruct * pCmd=(DL_GR_I_UserRebateUpDateStruct *)pSourceData;
	
	CString sql;
	sql.Format("select * from Web_RebateInfo where UserID='%d'", pCmd->iUserID);
	execSQL(sql,true);
	//结果数据定义
	DL_GR_O_UserRebateUpDateStruct UserRebate;
	::memset(&UserRebate,0,sizeof(UserRebate));
	//处理数据
	int iHandleResult=0;//初始化
	
	getValue(TEXT("UserID"),&UserRebate.iUserID);
	getValue(TEXT("Tx_Money"),&UserRebate.iTx_Money);
	getValue(TEXT("Tx_Smony"),&UserRebate.iTx_Smony);

	iHandleResult=1;//成功

	closeRecord();


	m_pRusultService->OnDataBaseResultEvent(&UserRebate.ResultHead,uCode,iHandleResult,
		sizeof(UserRebate),pSourceData->uIndex,pSourceData->dwHandleID);

	return 0;
}

//打开钱包
UINT CGameDataBaseHandle::OnOpenWallet(DataBaseLineHead *pSourceData, UINT uCode)
{
	//效验数据

	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_UnRegisterUser)) throw DTR_GR_OPEN_WALLET_ERROR;//////接受的结构和定义的是否一致
	DL_GR_I_UnRegisterUser * pOpenWallet=(DL_GR_I_UnRegisterUser * )pSourceData;
	DL_GR_O_OpenWalletResult OpenWallet;///打开结果
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
			throw DTR_GR_OPEN_WALLET_ERROR;
		}

		getValue("MoneyInBank",&OpenWallet.stUserBank.i64Bank);
		getValue("TwoPassword",OpenWallet.stUserBank.szTwoPassword,sizeof(OpenWallet.stUserBank.szTwoPassword));
		getValue("BankVer",&OpenWallet.stUserBank.nVer);

		closeRecord();
		//if (lstrcmp(szMD5Pass,pOpenWallet->szMD5Pass)!=0) 
		//	throw DTR_GR_OPEN_WALLET_ERROR_PSW;
		OpenWallet.stUserBank.dwUserID=pOpenWallet->dwUserID;
		throw DTR_GR_OPEN_WALLET_SUC;

	}
	catch (int iHandleResult)
	{TRACE("EXCEPT CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		m_pRusultService->OnDataBaseResultEvent(&OpenWallet.ResultHead, uCode, iHandleResult,
			sizeof(OpenWallet),pSourceData->uIndex,pSourceData->dwHandleID);
	}
	return 0;
}


UINT CGameDataBaseHandle::OnGMWarningUserRecord(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_Warn_User_Record)) throw DTR_GR_ERROR_UNKNOW;//////接受的结构和定义的是否一致
	DL_GR_I_Warn_User_Record * pWarningUser=(DL_GR_I_Warn_User_Record *)pSourceData;//结构赋值

	bool bres=sqlSPSetNameEx("SP_GMWarring",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter(TEXT("@GMID"),pWarningUser->dwGMID);
	addInputParameter(TEXT("@UserID"),pWarningUser->dwTargetID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	closeRecord();
	return 0;
}


//管理员踢人动作纪录
UINT CGameDataBaseHandle::OnGMKickUserRecord(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_Kick_User_Record))
		throw DTR_GR_ERROR_UNKNOW;//////接受的结构和定义的是否一致
	DL_GR_I_Kick_User_Record * pKickUser=(DL_GR_I_Kick_User_Record *)pSourceData;//结构赋值

	bool bres=sqlSPSetNameEx("SP_GMKickUser",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter(TEXT("@GMID"),pKickUser->GMID);
	addInputParameter(TEXT("@UserName"),pKickUser->szTargetUserName,sizeof(pKickUser->szTargetUserName));
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	closeRecord();
	return 0;
}

//呼叫GM处理
UINT CGameDataBaseHandle::OnCallGM(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_CallGM)) throw DTR_GR_ERROR_UNKNOW;//////接受的结构和定义的是否一致
	DL_GR_I_CallGM * pCallGM=(DL_GR_I_CallGM *)pSourceData;//结构赋值

	try
	{
		bool bres=sqlSPSetNameEx("SP_RecordCallGM",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID",pCallGM->dwUserID);
		addInputParameter("@UserName",pCallGM->szName,sizeof(pCallGM->szName));
		addInputParameter("@RoomID",m_pInitInfo->uRoomID);
		addInputParameter("@NameID",m_pKernelInfo->uNameID);
		addInputParameter("@RoomType",m_pInitInfo->uComType);
		addInputParameter("@DeskNumber",pCallGM->uDeskNO+1);
		addInputParameter("@DeskStation",pCallGM->uDeskStation);
		addInputParameter("@CallType",pCallGM->bCallReason);
		addInputParameter("@CallMsg",pCallGM->szMsg,sizeof(pCallGM->szMsg));

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		int ret=getReturnValue();

		closeRecord();
		if(ret==1)
			throw DTK_GR_CALL_GM_IN_ONE_MI;
		throw DTK_GR_CALL_GM_SUCCESS;
	}
	catch (int iHandleResult)
	{TRACE("EXCEPT CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		DataBaseResultLine DT_CallGMResult;
		::memset(&DT_CallGMResult,0,sizeof(DT_CallGMResult));
		m_pRusultService->OnDataBaseResultEvent(&DT_CallGMResult,DTK_GR_CALL_GM_RESULT,iHandleResult,
			sizeof(DT_CallGMResult),pSourceData->uIndex,pSourceData->dwHandleID);
	}
	return 0;
}

///
/// 记录修改房间密码
///
/// @param pSourceData
///
/// @ return 0成功返回0

UINT CGameDataBaseHandle::OnRoomPWChangeRecord(DataBaseLineHead * pSourceData)
{
	///< 效验数据
	if (pSourceData->DataLineHead.uSize != sizeof(DL_GR_RoomPWChange))
		throw DTR_GR_ERROR_UNKNOW;
	///< 结构赋值
	DL_GR_RoomPWChange * pRoomPW =(DL_GR_RoomPWChange *)pSourceData;

	try
	{
		bool bres=sqlSPSetNameEx("SP_RoomPWChangeRecord",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID",pRoomPW->dwUserID);
		addInputParameter("@RoomID",pRoomPW->dwRoomID);

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		getReturnValue();

		closeRecord();
	}
	catch (...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}
	return 0;
}

UINT CGameDataBaseHandle::OnUserLeft(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_UserLeft)) throw DTR_GR_ERROR_UNKNOW;
	DL_GR_I_UserLeft * pUserLeft=(DL_GR_I_UserLeft *)pSourceData;

	try
	{
		bool bres=sqlSPSetNameEx("SP_RecordUserLeft",true);
		if(!bres)
		{
			return 0;
		}

		int IsRoomRecord=0;
		UINT uAllCount=pUserLeft->uWinCount+pUserLeft->uLostCount+pUserLeft->uMidCount+pUserLeft->uCutCount;

		if((pUserLeft->dwChangePoint!=0L)||(uAllCount>0L)||(pUserLeft->dwChangeMoney!=0L))
			if ((m_pInitInfo->dwRoomRule|GRR_RECORD_GAME)!=0)
				IsRoomRecord=1;

		//修改分数

		addInputParameter(TEXT("@UserID"),pUserLeft->dwUserID);
		addInputParameter(TEXT("@GameTable"),m_pInitInfo->szGameTable,sizeof(m_pInitInfo->szGameTable));
		addInputParameter(TEXT("@OnlineTimeCount"),pUserLeft->dwOnLineTimeCount);
		addInputParameter(TEXT("@UserIsMaster"),pUserLeft->uMaster);
		addInputParameter(TEXT("@NameID"),m_pKernelInfo->uNameID);
		addInputParameter(TEXT("@RoomID"),m_pInitInfo->uRoomID);
		addInputParameter(TEXT("@IsRoomRecord"),IsRoomRecord);

		addInputParameter(TEXT("@SrcPoint"),pUserLeft->dwScrPoint);
		addInputParameter(TEXT("@ChangePoint"),pUserLeft->dwChangePoint);
		addInputParameter(TEXT("@ChangeMoney"),pUserLeft->dwChangeMoney);
		addInputParameter(TEXT("@ChangeTax"),pUserLeft->dwChangeTaxCom);
		addInputParameter(TEXT("@ResultPoint"),pUserLeft->dwResultPoint);
		addInputParameter(TEXT("@ChangeWin"),pUserLeft->uWinCount);
		addInputParameter(TEXT("@ChangeLost"),pUserLeft->uLostCount);
		addInputParameter(TEXT("@ChangeMid"),pUserLeft->uMidCount);
		addInputParameter(TEXT("@ChangeCut"),pUserLeft->uCutCount);
		addInputParameter(TEXT("@ClearLogonInfo"),1);
		addInputParameter(TEXT("@LockTable"),m_pInitInfo->szLockTable,sizeof(m_pInitInfo->szLockTable));
	
		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}

		closeRecord();
		///< 记录在线时间,百乐门
		CBcfFile f(CBcfFile::GetAppPath() + "Function.bcf");
		CString strValue = f.GetKeyVal("OnlineCoin","Available","0");
		if (atoi(strValue))
		{
			bool bres=sqlSPSetNameEx("SP_RecordOnlineTime",true);
			if(!bres)
			{
				return 0;
			}

			addInputParameter("@UserID",pUserLeft->dwUserID);
			if(0 != execStoredProc())
			{
				closeRecord();
				return 0;
			}
			closeRecord();
		}
	}
	catch (...) {TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);}

	return 0;
}
///一次性更新同桌所有用户信息
UINT CGameDataBaseHandle::OnUpdateAllUserResult(DataBaseLineHead * pSourceData)
{
	
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_AllUserData_t)) throw DTR_GR_ERROR_UNKNOW;
	DL_GR_I_AllUserData_t * pUserResult=(DL_GR_I_AllUserData_t *)pSourceData;

	//需要自动赠送玩家个数
	int iAutoNum = 0;	

	// 体验场(免费场)不进行金币、积分游戏记录
	if (m_pInitInfo->dwRoomRule & GRR_EXPERCISE_ROOM)
	{
		delete []pUserResult->pUpdateUserInfo;
		pUserResult->pUpdateUserInfo = NULL;
		return 0;
	}
	
	UpdateUserInfo_t *pUserInfo = pUserResult->pUpdateUserInfo;
	int nUserCount = pUserResult->nUserCount;
	int moneyGame=0;
	if(m_pInitInfo->uComType==TY_MONEY_GAME)
		moneyGame=1;

	CString strParam = "";
	CString strParam1 = "";
	CString strParam2 = "";
	CString strParam3 = "";
	int nUserCount0 = 0;
	int nUserCount1 = 0;
	int nUserCount2 = 0;
	int nUserCount3 = 0;
	TCHAR szTmp[256];
	
	int tempUserCount=0;
	CString TempStrParam="";
	
	//人数过大时，拆分一下
	for (int i=0; i<nUserCount; i++)
	{
			wsprintf(szTmp, TEXT("%d,%d,%I64d,%I64d,%I64d,%d,%d,%d,%d,")
			, pUserInfo->dwUserID
			, pUserInfo->dwOnLineTimeCount
			, pUserInfo->dwChangePoint
			, pUserInfo->dwChangeMoney
			, pUserInfo->dwChangeTaxCom
			, pUserInfo->uWinCount
			, pUserInfo->uLostCount
			, pUserInfo->uMidCount
			, pUserInfo->uCutCount
			);
			//发现有玩家需要自动赠送
			if (pUserInfo->bISAutoSendMoney)
			{
				iAutoNum++;
			}
		    
			strParam += szTmp;
			
		++pUserInfo;	///< 指针前移
	}
	
	int i = 0;
	//for (int i=0; i<4 && nUserCount>i*50;i++ )
	{
		bool bres=sqlSPSetNameEx("SP_UserInfoUpdateAll",true);
		if(!bres)
		{
			return 0;
		}

		tempUserCount = nUserCount;
		TempStrParam = strParam;
		addInputParameter(TEXT("@UserCount"), tempUserCount );
		addInputParameter(TEXT("@NameID"),m_pKernelInfo->uNameID);
		//混战场房间
		char szGameTableTmp[31];
		ZeroMemory(szGameTableTmp, sizeof(szGameTableTmp));
		if (m_pInitInfo->uBattleRoomID > 0)
		{
			//是混战房间
			memcpy(szGameTableTmp,m_pInitInfo->szBattleGameTable,sizeof(szGameTableTmp));
		}
		else
		{
			//普通房间
			memcpy(szGameTableTmp,m_pInitInfo->szGameTable,sizeof(szGameTableTmp));
		}
		addInputParameter(TEXT("@GameTable"),szGameTableTmp,sizeof(szGameTableTmp));
		addInputParameter(TEXT("@PlayTimeCount"),pUserResult->nPlayTime);
		addInputParameter(TEXT("@IsExperciseRoom"),(m_pInitInfo->dwRoomRule & GRR_EXPERCISE_ROOM)?1:0);
		addInputParameter(TEXT("@IsMoneyGame"),(m_pInitInfo->uComType==TY_MONEY_GAME)?1:0);
		addInputParameter(TEXT("@Param"), TempStrParam.GetBuffer(), sizeof(TCHAR) * TempStrParam.GetLength());
	
	
		// 增加房间ID
		addInputParameter(TEXT("@RoomID"),m_pInitInfo->uRoomID);


		int IsExperciseRoom = (m_pInitInfo->dwRoomRule & GRR_EXPERCISE_ROOM)?1:0;
		CString szSQL;

		szSQL.Format(TEXT("nUserCount = %d EXEC SP_UserInfoUpdateAll @UserCount=%d,@NameID=%d,@GameTable='%s',@PlayTimeCount=%d,@IsExperciseRoom=%d,@IsMoneyGame=%d,@Param='%s',@RoomID=%d"),nUserCount,tempUserCount,m_pKernelInfo->uNameID,szGameTableTmp,pUserResult->nPlayTime,IsExperciseRoom
			,(m_pInitInfo->uComType==TY_MONEY_GAME)?1:0,TempStrParam.GetBuffer(),m_pInitInfo->uRoomID);

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}

		int ret=getReturnValue();
		if (ret != 0)
		{
			if (m_ErrorSQLCount < 5)
			{
				closeRecord();

				Sleep(20);
				m_ErrorSQLCount++;
				OnUpdateAllUserResult(pSourceData);
				
				return 0;
			}

			delete []pUserResult->pUpdateUserInfo;
			pUserResult->pUpdateUserInfo = NULL;



		}

		m_ErrorSQLCount = 0;
		closeRecord();

	}

    // 是否发放救济金
    if(m_pInitInfo->bSendAlms && iAutoNum > 0) //添加自动赠送
    {
        pUserInfo = pUserResult->pUpdateUserInfo;
        DWORD dwUserID = 0;		
        for (int i=0; i<nUserCount; ++i)
        {
			dwUserID = pUserInfo->dwUserID;

            DL_GR_O_SendMoney_On_TC stSendMoney;
            memset(&stSendMoney, 0, sizeof(stSendMoney));

			if (pUserInfo->bISAutoSendMoney)
			{	
				GetWalletMoney(stSendMoney.strAutoSendMoney,dwUserID);				
			}

			++pUserInfo;	
		    
			closeRecord();
        }
    }
	else
	{
		closeRecord();
	}
	delete []pUserResult->pUpdateUserInfo;
	pUserResult->pUpdateUserInfo = NULL;
	return 0;
}

//获取赠送的金币值
void CGameDataBaseHandle::GetWalletMoney(RECEIVEMONEY &strRecMoney,DWORD dwUserID)
{
	try
	{	
		bool bres=sqlSPSetNameEx("SP_ReceiveMoney",true);
		if(!bres)
		{
			return ;
		}

		addInputParameter("@UserID", (int)dwUserID);

		if(0 != execStoredProc())
		{
			closeRecord();
			return ;
		}
		strRecMoney.iResultCode = getReturnValue();

		getValue(TEXT("ReceiveCount"),&strRecMoney.iCount);
		getValue(TEXT("ReceiveMoney"),&strRecMoney.i64Money);
		getValue(TEXT("MinMoney"),&strRecMoney.i64MinMoney);
		getValue(TEXT("ReceiveTotal"),&strRecMoney.iTotal);	
		closeRecord();

	}
	catch(...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}
}

//比赛开始，做比赛初始化
UINT CGameDataBaseHandle::OnContestBegin(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_ContestBegin)) throw DTR_GR_ERROR_UNKNOW;
	DL_GR_I_ContestBegin* _p=(DL_GR_I_ContestBegin *)pSourceData;

	try
	{
		bool bres=sqlSPSetNameEx("SP_ContestBegin",true);
		if(!bres)
		{
			return 0;
		}

		//修改分数
		addInputParameter(TEXT("@ContestID"), _p->iContestID);
		addInputParameter(TEXT("@GameID"), _p->uNameID);
		addInputParameter(TEXT("@RoomID"), _p->uRoomID);

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}

		int ret=getReturnValue();
		if (0 >= ret)
		{
			closeRecord();
			throw DTR_GR_ERROR_UNKNOW;
		}
		DL_GR_O_ContestBegin DL_OContest;
		DL_OContest.uMatchID = ret;
		DL_OContest.iUserNum = getRecordCount();
		if (0 >= DL_OContest.iUserNum)
		{
			closeRecord();
			throw DTR_GR_ERROR_UNKNOW;
		}
		DL_OContest.pUserIDs = new int[sizeof(int)*DL_OContest.iUserNum];
		int iCount = 0;
		while(!adoEndOfFile())
		{
			getValue(TEXT("UserID"), (DL_OContest.pUserIDs+iCount));
			moveNext();
			++iCount;
		}
		closeRecord();
		m_pRusultService->OnDataBaseResultEvent(&DL_OContest.ResultHead,DTK_GR_CONTEST_BEGIN_RESULT,0,sizeof(DL_GR_O_ContestBegin),pSourceData->uIndex,pSourceData->dwHandleID);
	}
	catch (...)
	{
		m_pInitInfo->iRoomState = 0;
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}

//比赛结束，给获奖玩家颁发奖励
UINT CGameDataBaseHandle::OnContestGameOver(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_UserContestRank)) throw DTR_GR_ERROR_UNKNOW;
	try
	{
		bool bres=sqlSPSetNameEx("SP_ContestFinish",true);
		if(!bres)
		{
			return 0;
		}

		//修改分数
		addInputParameter(TEXT("@ContestID"),m_pInitInfo->iContestID);
		addInputParameter(TEXT("@GameID"),m_pInitInfo->iGameID);
		addInputParameter(TEXT("@RoomID"),m_pInitInfo->uRoomID);
		addInputParameter(TEXT("@MatchID"),m_pInitInfo->iMatchID);
		addInputParameter(TEXT("@LeastPeople"),m_pInitInfo->iLeasePeople);

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}

		int ret=getReturnValue();

		m_pInitInfo->iRoomState = 0;
		DL_GR_O_ContestGameOver DL_OContestGameOver;
		while(0 == ret &&!adoEndOfFile())
		{
			ZeroMemory(&DL_OContestGameOver,sizeof(DL_GR_O_ContestGameOver));

			getValue("UserID",&DL_OContestGameOver.oAward.dwUserID);
			getValue("RankNum",&DL_OContestGameOver.oAward.iRank);
			DL_OContestGameOver.oAward.iAward = ret;
			m_pRusultService->OnDataBaseResultEvent(&DL_OContestGameOver.ResultHead,DTK_GR_CONTEST_GAME_OVER_RESULT,0,sizeof(DL_GR_O_ContestGameOver),pSourceData->uIndex,pSourceData->dwHandleID);
			moveNext();
		}

		if (1 == ret)
		{
			m_pRusultService->OnDataBaseResultEvent(&DL_OContestGameOver.ResultHead,DTK_GR_CONTEST_GAME_OVER_RESULT,1,sizeof(DL_GR_O_ContestGameOver),pSourceData->uIndex,pSourceData->dwHandleID);
		}

		closeRecord();
	}
	catch (...)
	{
		m_pInitInfo->iRoomState = 0;
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}

	return 0;
}

///更新用户比赛信息
UINT CGameDataBaseHandle::OnUpdateContestInfo(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_UserContestData)) throw DTR_GR_ERROR_UNKNOW;
	DL_GR_I_UserContestData* _p=(DL_GR_I_UserContestData *)pSourceData;

	try
	{
		bool bres=sqlSPSetNameEx("SP_ContestUpdateRecord",true);
		if(!bres)
		{
			return 0;
		}

		//修改分数
		addInputParameter(TEXT("@UserID"), _p->iUserID);
		addInputParameter(TEXT("@ContestID"), _p->iContestID);
		addInputParameter(TEXT("@LowChip"), _p->iLowChip);
		addInputParameter(TEXT("@CheckedScore"), _p->iCheckedScore);
		addInputParameter(TEXT("@ContestScore"), _p->ChangeScore);
		addInputParameter(TEXT("@ContestPoint"), _p->ChangePoint);
		addInputParameter(TEXT("@DeskNo"), _p->iDeskNo);
		addInputParameter(TEXT("@SitNO"), _p->iSitNo);
		addInputParameter(TEXT("@RoomID"), _p->iRoomID);
		addInputParameter(TEXT("@MatchID"), _p->iMatchID);
		addInputParameter(TEXT("@GameID"), _p->iGameID);

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		closeRecord();

	}
	catch (...) {TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);}

	return 0;
}

///得到用户比赛信息
UINT CGameDataBaseHandle::OnGetContestInfo(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_UserContestRank)) throw DTR_GR_ERROR_UNKNOW;
	DL_GR_I_UserContestRank* _p = (DL_GR_I_UserContestRank*)pSourceData;
	try
	{
		bool bres=sqlSPSetNameEx("SP_Contest_GetSomebodyRank",true);
		if(!bres)
		{
			return 0;
		}

		//获取排名
		addInputParameter(TEXT("@ContestID"),m_pInitInfo->iContestID);
		addInputParameter(TEXT("@MatchID"),m_pInitInfo->iMatchID);
		addInputParameter(TEXT("@GameID"),m_pInitInfo->iGameID);
		addInputParameter(TEXT("@RoomID"),m_pInitInfo->uRoomID);

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}

		int iPeople = getReturnValue();//返回值是当前剩余人数		
		DL_GR_O_UserContestRank _out;
		while(!adoEndOfFile())
		{
			ZeroMemory(&_out,sizeof(DL_GR_O_UserContestRank));

			getValue("UserID",&_out._data.dwUserID);
			getValue("RankNum",&_out._data.iRankNum);
			getValue("ContestScore",&_out._data.i64ContestScore);
			getValue("ContestCount",&_out._data.iContestCount);
			getValue("RemainPeople",&_out._data.iRemainPeople);
			_out._data.iRemainPeople = iPeople;

			if (_out._data.iRankNum > 0)
			{
				m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GR_GET_CONTEST_RESULT,0,sizeof(DL_GR_O_UserContestRank),pSourceData->uIndex,pSourceData->dwHandleID);
			}

			moveNext();
		}

		closeRecord();

	}
	catch (...) {TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);}

	return 0;
}

UINT CGameDataBaseHandle::OnContestApply(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_ContestApply)) throw DTR_GR_ERROR_UNKNOW;
	DL_GR_I_ContestApply *pContestApply = (DL_GR_I_ContestApply*)pSourceData;
	
	try
	{
		//以下两个存储过程都是借用网站上面报名的存储过程

		//报名
		bool bres=sqlSPSetNameEx("SP_ContestApply",true);
		if(!bres)
		{
			return 0;
		}
	
		addInputParameter(TEXT("@gameID"), m_pInitInfo->iGameID);
		addInputParameter(TEXT("@UserID"), pContestApply->iUserID);
		addInputParameter(TEXT("@TypeID"), pContestApply->iTypeID);
		addInputParameter(TEXT("@roomID"), m_pInitInfo->uRoomID);
		addInputParameter(TEXT("@ContestID"), m_pInitInfo->iContestID);
		
		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}

		DL_GR_O_ContestApply DL_OContestApply;
		DL_GR_O_ContestInfo DL_OContestInfo;
		DL_OContestApply._data.iUserID = pContestApply->iUserID;
		DL_OContestApply._data.iContestID = m_pInitInfo->iContestID;
		getValue(("rs"), &DL_OContestApply._data.bResult);	//执行情况
		getValue(("Jewels"), &DL_OContestApply._data.i64Jewels);
		getValue(("UserMoney"), &DL_OContestApply._data.i64WalletMoney);
		getValue(("@yNum"), &DL_OContestApply._data.iApplyNum);

		getValue(TEXT("ynum"), &DL_OContestInfo._data.iContestNum);	//执行情况
		getValue(TEXT("isOK"), &DL_OContestApply.iContestBegin);	//执行情况
		DL_OContestInfo._data.iContestBegin = DL_OContestApply.iContestBegin;

		m_pRusultService->OnDataBaseResultEvent(&DL_OContestApply.ResultHead,DTK_GR_CONTEST_APPLY,0,sizeof(DL_OContestApply),pSourceData->uIndex,pSourceData->dwHandleID);	
		if(2 == DL_OContestApply._data.bResult || 1 == DL_OContestApply._data.bResult)
		{
			m_pRusultService->OnDataBaseResultEvent(&DL_OContestInfo.ResultHead,DTK_GR_CONTEST_BROADCAST,0,sizeof(DL_OContestInfo),pSourceData->uIndex,pSourceData->dwHandleID);	
		}
		
		closeRecord();
		
		return 0;
	}
	catch (...) 
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}
	return 0;
}



UINT CGameDataBaseHandle::OnContestAbandon(DataBaseLineHead * pSourceData)
{
	//效验数据
	try
	{
		bool bres=sqlSPSetNameEx("SP_ContestAbandon",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter(TEXT("@gameID"), m_pInitInfo->iGameID);
		addInputParameter(TEXT("@roomID"), m_pInitInfo->uRoomID);
		addInputParameter(TEXT("@ContestID"), m_pInitInfo->iContestID);

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}

		DL_GR_O_ContestAbandon _out;

		_out.iUserNum = getRecordCount();
		_out.pData = new MSG_GR_S_ContestAbandon[_out.iUserNum];
		int count = 0;
		while(!adoEndOfFile())
		{
			getValue("UserID",&_out.pData[count].iUserID);
			getValue("UserMoney",&_out.pData[count].i64WalletMoney);
			getValue("UserJewels",&_out.pData[count].iJewels);
			count++;
			moveNext();
		}

		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead,DTK_GR_CONTEST_ABANDON,0,sizeof(DL_GR_O_ContestAbandon),pSourceData->uIndex,pSourceData->dwHandleID);	
		
		closeRecord();

		return 0;
	}
	catch (...) 
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}
	return 0;
}

UINT CGameDataBaseHandle::OnUpdateUserResult(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_UserLeft)) throw DTR_GR_ERROR_UNKNOW;
	DL_GR_I_UserLeft * pUserResult=(DL_GR_I_UserLeft *)pSourceData;

	// 体验场(免费场)不进行金币、积分游戏记录
	if (m_pInitInfo->dwRoomRule & GRR_EXPERCISE_ROOM || m_pInitInfo->dwRoomRule & GRR_CONTEST || m_pInitInfo->dwRoomRule & GRR_TIMINGCONTEST)
	{
		return 0;
	}
	
	int moneyGame=0;
	if(m_pInitInfo->uComType==TY_MONEY_GAME)
		moneyGame=1;

	bool bres=sqlSPSetNameEx("SP_UserInfoUpdate",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter(TEXT("@UserID"),pUserResult->dwUserID);
	addInputParameter(TEXT("@NameID"),m_pKernelInfo->uNameID);
	addInputParameter(TEXT("@OnlineTime"),pUserResult->dwOnLineTimeCount);
	addInputParameter(TEXT("@IsMoneyGame"),moneyGame);

	//混战场房间
	char szGameTableTmp[31];
	memset(szGameTableTmp,0,sizeof(szGameTableTmp));
	if (m_pInitInfo->uBattleRoomID > 0)
	{
		//是混战房间
		memcpy(szGameTableTmp,m_pInitInfo->szBattleGameTable,sizeof(szGameTableTmp));
	}
	else
	{
		//普通房间
		memcpy(szGameTableTmp,m_pInitInfo->szGameTable,sizeof(szGameTableTmp));
	}
    addInputParameter(TEXT("@GameTable"),szGameTableTmp,sizeof(szGameTableTmp));
	///////////////////////////////////

	addInputParameter(TEXT("@Point"),pUserResult->dwChangePoint);
	addInputParameter(TEXT("@Money"),pUserResult->dwChangeMoney);
	addInputParameter(TEXT("@TaxCom"),pUserResult->dwChangeTaxCom);
	addInputParameter(TEXT("@Win"),pUserResult->uWinCount);
	addInputParameter(TEXT("@Lost"),pUserResult->uLostCount);
	addInputParameter(TEXT("@Mid"),pUserResult->uMidCount);
	addInputParameter(TEXT("@Cut"),pUserResult->uCutCount);
	addInputParameter(TEXT("@PlayTimeCount"),pUserResult->dwPlayTimeCount);
	addInputParameter(TEXT("@IsExperciseRoom"),(m_pInitInfo->dwRoomRule & GRR_EXPERCISE_ROOM || m_pInitInfo->dwRoomRule & GRR_CONTEST ||m_pInitInfo->dwRoomRule & GRR_TIMINGCONTEST)?1:0);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	closeRecord();
	return 0;
}


//即时更新分数和金币
UINT CGameDataBaseHandle::OnModifyUserMoney(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_Update_InstantMoney)) throw DTR_GR_ERROR_UNKNOW;
	DL_GR_Update_InstantMoney * pUserResult = (DL_GR_Update_InstantMoney *)pSourceData;

	int cut=0;
	if(pUserResult->bCutValue)
		cut=1;

	bool bres=sqlSPSetNameEx("SP_ChangeMoneyInTime",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter(TEXT("@UserID"),pUserResult->dwUserID);
	addInputParameter(TEXT("@GameNameID"),m_pKernelInfo->uNameID);
	addInputParameter(TEXT("@GameTable"),m_pInitInfo->szGameTable,sizeof(m_pInitInfo->szGameTable));
	addInputParameter(TEXT("@Wallet"),pUserResult->dwChangeMoney);
	addInputParameter(TEXT("@Point"),pUserResult->dwChangePoint);
	addInputParameter(TEXT("@Cut"),cut);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	closeRecord();
	return 0;
}

//更新魅力值
UINT CGameDataBaseHandle::OnUpdateUserCharm(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_AddCharm)) throw DTR_GR_ERROR_UNKNOW;
	DL_GR_I_AddCharm * pCharm = (DL_GR_I_AddCharm *)pSourceData;

	TCHAR szSQL[1000];
	if(pCharm->iCharmValue != 0)
	{
		wsprintf(szSQL,TEXT("UPDATE TUserInfo SET Fascination=Fascination+%d WHERE UserID=%ld"),pCharm->iCharmValue,pCharm->dwUserID);
		execSQL(szSQL);
	}
	return 0;
}


UINT CGameDataBaseHandle::OnUseSmallBoardcast(DataBaseLineHead * pSourceData)
{
	PROP_GPR_I_BOARDCAST *iBoard=(PROP_GPR_I_BOARDCAST*)pSourceData;
	PROP_GPR_O_BOARDCAST oBoard;
	memcpy(&oBoard.boardCast,&iBoard->boardCast,sizeof(_TAG_BOARDCAST));

	bool bres=sqlSPSetNameEx("SP_UsePropForSelf",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID",iBoard->boardCast.dwUserID);
	addInputParameter("@PropID",iBoard->boardCast.nPropID);
	addInputParameter("@PropCount",1);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	int ret=getReturnValue();

	closeRecord();
	int iHandleResult=DTK_GR_PROP_USE_SUCCEED;
	if(ret!=0)
		iHandleResult=DTR_GR_PROP_USE_ERR_NOPROP;

	m_pRusultService->OnDataBaseResultEvent(&oBoard.ResultHead,DTK_GPR_PROP_SMALL_BRD,iHandleResult,sizeof(PROP_GPR_O_BOARDCAST),pSourceData->uIndex,pSourceData->dwHandleID);

	return 0;
}

UINT CGameDataBaseHandle::OnSendUserMoenyByTimes(DataBaseLineHead *pSourceData)
{
	//QR

	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_SendMonet_By_Times)) throw DTR_GR_ERROR_UNKNOW;
	DL_GR_I_SendMonet_By_Times * pSendMoeny=(DL_GR_I_SendMonet_By_Times *)pSourceData;

	DL_GR_O_SendMonet_By_Times sSendMoenyResult;
	memset(&sSendMoenyResult,0,sizeof(sSendMoenyResult));

	//comroominfo表中关于对送金币的配置
	int iPerMoney = 0;       //一次送多少
	int iDayMoneyTimes = 0;  //一天送多少次
	//int iDayLessMoney = 0;   //金币少于多少就送
	//ComRoomInfo中金币房间的配置
	sSendMoenyResult.dwUserID = pSendMoeny->dwUserID;
	sSendMoenyResult.i64SrcGameMoney = pSendMoeny->i64SrcGameMoney;
	sSendMoenyResult.i64SrcMoneyInBank = pSendMoeny->i64SrcMoneyInBank;
	sSendMoenyResult.dwAllSendTImes = 0;
	sSendMoenyResult.dwSendFailedCause = 0;
	sSendMoenyResult.dwSendGameMoney = 0;
	sSendMoenyResult.dwSendTimes = pSendMoeny->dwGameFinishCount;

	int FinishCount = pSendMoeny->dwGameFinishCount;//获得当前是多少次游戏了

	bool bres=sqlSPSetNameEx("SP_SendMoney",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@RoomID",m_pInitInfo->uRoomID);
	addInputParameter("@UserID",pSendMoeny->dwUserID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	sSendMoenyResult.dwSendFailedCause=getReturnValue();

	if(sSendMoenyResult.dwSendFailedCause==0)
	{
		getValue("TMoney",&sSendMoenyResult.dwSendGameMoney);
		getValue("DaySendTimes",&sSendMoenyResult.dwSendTimes);
		getValue("DayMoneyTimes",&sSendMoenyResult.dwAllSendTImes);
	}

	closeRecord();

	m_pRusultService->OnDataBaseResultEvent(&sSendMoenyResult.ResultHead,DTK_GR_SENDMONEY_TIMES,sSendMoenyResult.dwSendFailedCause,
		sizeof(sSendMoenyResult),pSourceData->uIndex,pSourceData->dwHandleID);
	return 0;
}
//玩家使用道俱
UINT CGameDataBaseHandle::OnUserUseOneProp(DataBaseLineHead *pSourceData)
{

	//////////////////////////////////////////////////////////////////////////
	//效验数据
	//2008-08-11
	if (pSourceData->DataLineHead.uSize != sizeof(DL_GR_I_USEPROP))
		throw DTR_GR_PROP_USE_ERR_PARAM_INVALID;//////接受的结构和定义的是否一致

	DL_GR_I_USEPROP	*useIProp = (DL_GR_I_USEPROP * )pSourceData;
	DL_GR_O_USEPROP useOProp;
	ZeroMemory(&useOProp,sizeof(DL_GR_O_USEPROP));
	memcpy(&useOProp.usingProp,&useIProp->usingProp,sizeof(_TAG_USINGPROP));

	bool bres=sqlSPSetNameEx("SP_UseProp",true);
	if(!bres)
	{
		return 0;
	}

	//取值属性
	int attribValue=useIProp->usingProp.nPropValueAttrib;
	//操作属性
	int attribAction=useIProp->usingProp.nPropActionAttrib;
	//以下几个操作需要计算时间
	CString stime;
	stime.Format("%d",CTime::GetCurrentTime());
	int curTime=atoi(stime);

	//影响时间的时间值，以秒为单位
	int		addSecond=attribValue & 0xFFFF;
	addSecond*=3600;//本身是以小时为单位的，所以要再乘上60*60=3600秒
	//双倍积分
	if(attribAction & 0x4)
	{
		if(curTime>useOProp.usingProp.iDoubleTime)
			//如果以前的双倍积分时间已过期，则直接在现在的开始时间上加上相应的时间
			useOProp.usingProp.iDoubleTime=curTime+addSecond;
		else
			//如果以前的双倍积分时间还未到期，则在原有的到期时间上加上相应的时间
			useOProp.usingProp.iDoubleTime+=addSecond;
	}
	//护身符道具，输了不扣分
	if(attribAction & 0x8)
	{
		if(curTime>useOProp.usingProp.iProtectTime)
			//如果以前的护身符时间已过期，则直接在现在的开始时间上加上相应的时间
			useOProp.usingProp.iProtectTime=curTime+addSecond;
		else
			//如果以前的护身符时间还未到期，则在原有的到期时间上加上相应的时间
			useOProp.usingProp.iProtectTime+=addSecond;
	}
	//魅力值变化
	int fasciChange=0;
	if(attribAction & 0x10)
	{
		//魅力值变化量
		fasciChange=(attribValue & 0xFF0000)>>16;
		//增加魅力值还是减少魅力值
		if(attribAction & 0x20)
			fasciChange=-fasciChange;
	}

	addInputParameter("@UserID",useIProp->usingProp.dwUserID);
	addInputParameter("@PropID",useIProp->usingProp.nPropID);
	addInputParameter("@TargetID",useIProp->usingProp.dwTargetUserID);
	//添加NameID，用于负分清零
	addInputParameter("@GameNameID",m_pKernelInfo->uNameID);
	addInputParameter("@GameTable",m_pInitInfo->szGameTable,sizeof(m_pInitInfo->szGameTable));
	//是否负分清零
	addInputParameter("@IsClearMinus", (attribAction & 0x2)?1:0);
	//是否大小喇叭 // PengJiLin, 2010-6-1, 增加 记牌器
	addInputParameter("@IsBoard",(attribAction & 0x80 || attribAction & 0x100 || attribAction & 0x200)?1:0);
	//双倍积分变化
	addInputParameter("@DoubleTime",useOProp.usingProp.iDoubleTime);
	//护身符变化
	addInputParameter("@ProtectTime",useOProp.usingProp.iProtectTime);
	//魅力值变化
	addInputParameter("@FasciChange",fasciChange);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	int ret=getReturnValue();

	closeRecord();

	int iHandleResult=DTK_GR_PROP_USE_SUCCEED;
	if(ret!=0)
		iHandleResult=DTR_GR_PROP_USE_ERR_NOPROP;

	m_pRusultService->OnDataBaseResultEvent(&useOProp.ResultHead,DTK_GR_PROP_USE,iHandleResult,sizeof(DL_GR_O_USEPROP),pSourceData->uIndex,pSourceData->dwHandleID);

	return 0;

}
//////////////////////////////
//20090203 Kylin 防止更新数据库后，用户登陆失败
//清理用户断线
UINT CGameDataBaseHandle::OnUnRegisterOnLine(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_UnRegisterUser)) throw DTR_GR_ERROR_UNKNOW;
	DL_GR_I_UnRegisterUser * pUnRegisterUser=(DL_GR_I_UnRegisterUser *)pSourceData;

	//处理数据
	try	{ UnRegisterUserLogon(pUnRegisterUser->dwUserID,false); }
	catch (...) {}

	return 0;
}

//wushuqun 2009.5.22
//修改禁止登录问题，修改插入TWLoginRecord 记录时机
//执行时机：登录房间后发送完游戏房间列表执行
UINT CGameDataBaseHandle::OnWriteTWLoginRecord(DataBaseLineHead * pSourceData)
{
	//效验数据
	if (pSourceData->DataLineHead.uSize!=sizeof(DL_GR_I_WriteWLoginRecord))
	{
		throw DTR_GR_ERROR_UNKNOW;
	}
	DL_GR_I_WriteWLoginRecord * pDlWriteWLogin = (DL_GR_I_WriteWLoginRecord *)pSourceData;
	//处理数据
	try
	{
		bool bres=sqlSPSetNameEx("SP_InsertWLoginRecord",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID", pDlWriteWLogin->dwUserID);
		addInputParameter("@RoomID", pDlWriteWLogin->uRoomID);
		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		closeRecord();
	}
	catch (...) {}

	return 0;
}

//wushuqun 2009.6.6
//即时封桌功能
//数据中该房间增加虚拟人数
UINT CGameDataBaseHandle::OnAloneDeskInTime(DataBaseLineHead * pSourceData)
{
	//交验数据
	if (pSourceData->DataLineHead.uSize != sizeof(DL_GR_I_ALone_Data))
	{
		throw DTR_GR_ERROR_UNKNOW;;
	}
	DL_GR_I_ALone_Data * pAloneDeskData = (DL_GR_I_ALone_Data * )pSourceData;
	try
	{
		bool bres=sqlSPSetNameEx("SP_AloneDeskInTime",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@RoomID", pAloneDeskData->uRoomID);
		addInputParameter("@RoomVirtualPeople", pAloneDeskData->bDeskPeople);
		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		int bRet = getReturnValue();
		//SP_AloneDeskInTime 存储过程返回值说明
		// 0 :添加房间虚拟人失败
		// 1 :添加房间虚拟人成功
		
		closeRecord();
		
	}
	catch(...)
	{
	}
	return 0;
}

//删除登陆信息
bool CGameDataBaseHandle::UnRegisterUserLogon(long int dwUserID, bool bAllLogonInfo)
{
	if (m_pInitInfo->szLockTable[0]!=0)
	{
		bool bres=sqlSPSetNameEx("SP_SetUserOnlineFlag",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@UserID",dwUserID);
		///////////////////////////////////////////////
		addInputParameter("@FlagID",-2);
		///////////////////////////////////////////////
		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}
		closeRecord();

		//////////////////////////////

	}

	return true;
}

UINT CGameDataBaseHandle::FillTimingMatchQueueUser(DataBaseLineHead *pSourceData)
{
	DL_GR_I_TimingMatchUsers *pQueryInfo = (DL_GR_I_TimingMatchUsers*)pSourceData;
	if(!pQueryInfo)
		return 0;
	bool bres=sqlSPSetNameEx("SP_GetTimingMatchUsers",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@gameID", pQueryInfo->iGameID);
	addInputParameter("@roomID", pQueryInfo->iRoomID);
	addInputParameter("@contestID",pQueryInfo->iContestID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	int iRecordCount = getRecordCount();
	DL_GR_O_TimingMatchUsers outMatchUsers;
	memset(&outMatchUsers,0,sizeof(DL_GR_O_TimingMatchUsers));
	outMatchUsers.iUserCount = iRecordCount;
	int icounter = 0;

	while(!adoEndOfFile())
	{
		if(icounter >= 100)
			break;
		getValue("UserID",&outMatchUsers.iArrayUserid[icounter]);

		moveNext();
		++icounter;
	}
	m_pRusultService->OnDataBaseResultEvent(&outMatchUsers.ResultHead, DTK_GR_TIMINGMATCH_GETQUEQUEUSERS, 0, sizeof(DL_GR_O_TimingMatchUsers), pSourceData->uIndex, pSourceData->dwHandleID);
	closeRecord();

	return true;
}

int CGameDataBaseHandle::OnFindBuyDeskStation(DataBaseLineHead *pSourceData)
{
	DL_GR_I_FindDeskBuyStation *pReveiveData = (DL_GR_I_FindDeskBuyStation *)pSourceData;
	if(!pReveiveData)
	{
		return 0;
	}
	CString szSQL;
	szSQL.Format("select * from TBuyGameDeskRecord where RoomID=%d",m_pInitInfo->uRoomID);
	
	if (execSQL(szSQL,true))
	{
		m_pDataBaseManage->SQLConnectReset();
		closeRecord();
		return 0;
	}

	DL_GR_O_FindDeskBuyStationResult out_data;
	int iRecordCount = getRecordCount();
	for(int i=0;i<iRecordCount;i++)
	{
		memset(&out_data,0,sizeof(out_data));

		getValue("TableNumber",&out_data.iDeskID);
		getValue("MasterID",&out_data.iUserID);
		getValue("szDeskPassWord",out_data.szDeskPassWord,sizeof(out_data.szDeskPassWord));
		getValue("DeskConfig",out_data.szDeskConfig,sizeof(out_data.szDeskConfig));
		getValue("BuyGameCount",&out_data.iBuyCount);
		getValue("RunCount",&out_data.iNowCount);
		getValue("PayType",&out_data.iPayType);
		getValue("FinishCondition",&out_data.bFinishCondition);
		getValue("PositionLimit",&out_data.bPositionLimit);
		getValue("PlayerNum",&out_data.bPlayerNum);
		getValue("Jewels",&out_data.iAANeedJewels);
		getValue("MasterState",&out_data.bMasterState);
		getValue("MidEnter", &out_data.bMidEnter);
        getValue("BuyMinutes", &out_data.iBuyMinutes);
        getValue("BeginTime", &out_data.BeginTime);
		getValue("IsPlay", &out_data.bIsPlay);
		getValue("ClubID", &out_data.iClubID);

		if(out_data.iDeskID<0 || out_data.iDeskID>=100)
		{
			continue;
		}
		out_data.bIsBuy = true;
		m_pRusultService->OnDataBaseResultEvent(&out_data.ResultHead, DTK_GR_FINDDESKBUYSTATION,0, sizeof(out_data), pSourceData->uIndex, pSourceData->dwHandleID);
		moveNext();
	}
	closeRecord();
	return true;
}

int CGameDataBaseHandle::OnGetDeskInfo(DataBaseLineHead *pSourceData)
{
	DL_GR_I_GET_DESKINFO *pReveiveData = (DL_GR_I_GET_DESKINFO *)pSourceData;
	if(!pReveiveData)
	{
		return 0;
	}

	CString szSQL;
	if (TYPE_BUYROOM_NOTICE == pReveiveData->iType)
	{
		szSQL.Format("select * from TBuyGameDeskRecord where RoomID=%d and TableNumber=%d and szDeskPassWord='%s'",pReveiveData->_BuyData._data.iRoomID,pReveiveData->_BuyData._data.iDeskID,pReveiveData->_BuyData._data.szPassWord);
	}
	else if (TYPE_CLUB_BUYDESK == pReveiveData->iType)
	{
		szSQL.Format("select * from TBuyGameDeskRecord where RoomID=%d and TableNumber=%d and szDeskPassWord='%s'",pReveiveData->_ClubBuyData.iRoomID,pReveiveData->_ClubBuyData.iDeskID,pReveiveData->_ClubBuyData._data._RoomData.szDeskPass);
	}
	else
	{
		szSQL.Format("select * from TBuyGameDeskRecord where RoomID=%d and TableNumber=%d and szDeskPassWord='%s'",pReveiveData->_ReturnData._data.iRoomID,pReveiveData->_ReturnData._data.iDeskID,pReveiveData->_ReturnData._data.szPass);
	}
	
	if (execSQL(szSQL,true))
	{
		m_pDataBaseManage->SQLConnectReset();
		closeRecord();
		return 0;
	}

	DL_GR_O_GET_DESKINFO _out;

	getValue("MasterID",&_out.iMasterID);
	if	(_out.iMasterID>0)
	{
		getValue("BuyGameCount",&_out.iBuyCount);
		getValue("RunCount",&_out.iNowCount);
		getValue("szDeskPassWord",_out.szDeskPass,sizeof(_out.szDeskPass));
		getValue("PayType",&_out.iPayType);
		getValue("DeskConfig",_out.szDeskConfig,sizeof(_out.szDeskConfig));
		getValue("FinishCondition",&_out.bFinishCondition);
		getValue("PositionLimit",&_out.bPositionLimit);
		getValue("PlayerNum",&_out.bPlayerNum);
		getValue("Jewels",&_out.iAANeedJewels);
		getValue("MasterState",&_out.bMasterState);
		getValue("MidEnter", &_out.bMidEnter);
        getValue("BuyMinutes", &_out.iBuyMinutes);
        getValue("BeginTime", &_out.BeginTime);
		getValue("IsPlay", &_out.bIsPlay);
		getValue("ClubID", &_out.iClubID);

		if (TYPE_BUYROOM_NOTICE == pReveiveData->iType)
		{
			_out.iDeskID = pReveiveData->_BuyData._data.iDeskID;
			memcpy_s(&_out._BuyData,sizeof(_out._BuyData),&pReveiveData->_BuyData,sizeof(MSG_MG_S_BUY_DESK_NOTICE));
		}
		else if (TYPE_CLUB_BUYDESK == pReveiveData->iType)
		{
			_out.iDeskID = pReveiveData->_ClubBuyData.iDeskID;
			memcpy_s(&_out._ClubBuyData,sizeof(_out._ClubBuyData),&pReveiveData->_ClubBuyData,sizeof(MSG_MG_S_CLUB_BUY_DESK_NOTICE));
		}	
		else
		{
			_out.iDeskID = pReveiveData->_ReturnData._data.iDeskID;
			memcpy_s(&_out._ReturnData,sizeof(_out._ReturnData),&pReveiveData->_ReturnData,sizeof(MSG_MG_S_RETURN_DESK_NOTICE));
		}	
		_out.iType = pReveiveData->iType;
	}
	
	int ret = _out.iMasterID>0?true:false;

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GR_GET_DESKINFO,ret, sizeof(_out), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();
	return true;
}

int CGameDataBaseHandle::OnContestActive(DataBaseLineHead *pSourceData)
{
	DL_GR_I_ContestActive *_in = (DL_GR_I_ContestActive *)pSourceData;
	if(!_in)
	{
		return 0;
	}

	bool bres=sqlSPSetNameEx("SP_ContestActive",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@MatchID",_in->_data.iMatchID);
	addInputParameter("@RoomID",_in->_data.iRoonID);
	addInputParameter("@ContestID",m_pInitInfo->iContestID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	int ret = getReturnValue();

	DL_GR_O_ContestActive _out;
	memcpy_s(&_out._data,sizeof(_out._data),&_in->_data,sizeof(MSG_MG_S_CONTEST_ACTIVE));
	getValue("BeginTime",&_out._data.BeginTime);

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GR_CONTEST_ACTIVE,ret, sizeof(DL_GR_O_ContestActive), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();
	return true;
}

int CGameDataBaseHandle::OnContestRegist(DataBaseLineHead *pSourceData)
{
	bool bres=sqlSPSetNameEx("SP_ContestRegist",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@RoomID",m_pInitInfo->uRoomID);
	addInputParameter("@ContestID",m_pInitInfo->iContestID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	int ret = getReturnValue();

	DataBaseResultLine _out;
	memset(&_out,0,sizeof(DataBaseResultLine));
	m_pRusultService->OnDataBaseResultEvent(&_out, DTK_GR_CONTEST_REGIST,ret, sizeof(DataBaseResultLine), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();
	return true;
}

int CGameDataBaseHandle::OnCostJewels(DataBaseLineHead *pSourceData)
{
	DL_GR_I_BuyRoom_CostFee *_in = (DL_GR_I_BuyRoom_CostFee*)pSourceData;
	DL_GR_O_BuyRoom_CostFee _out;

	bool bres=sqlSPSetNameEx("SP_CostJewels",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@DeskPass",_in->szDeskPassWord,sizeof(_in->szDeskPassWord));
	addInputParameter("@Type",_in->iType);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	int ret = getReturnValue();
	while (0 == ret && !adoEndOfFile())
	{
		getValue("UserID",&_out._Data.iUserID);
		getValue("Jewels",&_out._Data.iJewels);
		getValue("LockJewels",&_out._Data.iLockJewels);

		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GR_BUYROOM_COSTFEE,ret, sizeof(DL_GR_O_BuyRoom_CostFee), pSourceData->uIndex, pSourceData->dwHandleID);

		moveNext();
	}


	closeRecord();
	return true;
}

int CGameDataBaseHandle::OnMiddleCostFee(DataBaseLineHead *pSourceData)
{
	DL_GR_I_MidCostJewel *pReveiveData = (DL_GR_I_MidCostJewel*)pSourceData;
	DL_GR_O_BuyRoom_CostFee _out;

	bool bres=sqlSPSetNameEx("SP_MiddleCostJewels",true);
	if(!bres)
	{                                                                               
		return 0;
	}

	addInputParameter("@UserID",pReveiveData->iUserID);
	addInputParameter("@AAJewels",pReveiveData->iAAJewels);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	int ret = getReturnValue();
	while (0 == ret && !adoEndOfFile())
	{
		getValue("UserID",&_out._Data.iUserID);
		getValue("Jewels",&_out._Data.iJewels);
		getValue("LockJewels",&_out._Data.iLockJewels);

		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GR_BUYROOM_COSTFEE,ret, sizeof(DL_GR_O_BuyRoom_CostFee), pSourceData->uIndex, pSourceData->dwHandleID);

		moveNext();
	}


	closeRecord();
	return true;

}


int CGameDataBaseHandle::OnClearBuyDesk(DataBaseLineHead *pSourceData)
{
	DL_GR_I_BuyDeskOut *pReveiveData = (DL_GR_I_BuyDeskOut *)pSourceData;
	if(!pReveiveData)
	{
		return 0;
	}

	bool bres=sqlSPSetNameEx("SP_ClearDesk");
	if(!bres)
	{
		return 0;
	}
	
	addInputParameter("@DeskPassWord",pReveiveData->szDeskPassWord,sizeof(pReveiveData->szDeskPassWord));
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	closeRecord();
	return true;
}


int CGameDataBaseHandle::OnClearData(DataBaseLineHead *pSourceData)
{
	bool bres=sqlSPSetNameEx("SP_ClearGameData");
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@RoomID",m_pInitInfo->uRoomID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	closeRecord();
	return true;
}


int CGameDataBaseHandle::OnReleaseBuyDesk(DataBaseLineHead *pSourceData)
{
	DL_GR_I_ReleaseDesk *pReveiveData = (DL_GR_I_ReleaseDesk *)pSourceData;
	if(!pReveiveData)
	{
		return 0;
	}

	bool bres=sqlSPSetNameEx("SP_ReleaseDesk");
	if(!bres)
	{
		return 0;
	}
	
	addInputParameter("@DeskPassWord",pReveiveData->szDeskPassWord,sizeof(pReveiveData->szDeskPassWord));
	addInputParameter("@RunCount",pReveiveData->iRunCount);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	closeRecord();
	return true;
}

int CGameDataBaseHandle::OnReleaseRecord(DataBaseLineHead *pSourceData)
{
	DL_GR_I_RECORD_GAMEINFO *pReveiveData = (DL_GR_I_RECORD_GAMEINFO *)pSourceData;
	if(!pReveiveData)
	{
		return 0;
	}

	bool bres=sqlSPSetNameEx("SP_RecordGameMSG",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID",pReveiveData->iUserID);
	addInputParameter("@DeskPass",pReveiveData->szPass,sizeof(pReveiveData->szPass));
	addInputParameter("@GameMSG",pReveiveData->GameInfo,sizeof(pReveiveData->GameInfo));
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	closeRecord();
	return true;
}

int CGameDataBaseHandle::OnGetReleaseRecord(DataBaseLineHead *pSourceData)
{
	DL_GR_I_GET_GAMEINFO *pReveiveData = (DL_GR_I_GET_GAMEINFO *)pSourceData;
	if(!pReveiveData)
	{
		return 0;
	}

	bool bres=sqlSPSetNameEx("SP_GetReleaseInfo",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@RoomID",pReveiveData->iRoomID);
	addInputParameter("@DeskID",pReveiveData->iDeskID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_GR_O_GET_GAMEINFO _out;

	int ret = getRecordCount();

	while(!adoEndOfFile())
	{
		int station;
		getValue("UserID",&_out.iUserID);
		getValue("GameInfo",_out.GameInfo,sizeof(_out.GameInfo));
		_out.iCount = ret;
		_out.iDeskID = pReveiveData->iDeskID;
		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GR_GETRELEASEINFO,0, sizeof(_out), pSourceData->uIndex, pSourceData->dwHandleID);

		moveNext();
	}

	closeRecord();
	return true;
}

int CGameDataBaseHandle::OnReturnBuyDesk(DataBaseLineHead *pSourceData)
{
	DL_GR_I_ReturnDesk *pReveiveData = (DL_GR_I_ReturnDesk *)pSourceData;
	if(!pReveiveData)
	{
		return 0;
	}

	bool bres=sqlSPSetNameEx("SP_ReturnDesk",true);
	if(!bres)
	{
		return 0;
	}
	
	addInputParameter("@RoomID",pReveiveData->iRoomID);
	addInputParameter("@DeskID",pReveiveData->iDeskID);
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_GR_O_ReturnDeskResult _out;

	int ret = getReturnValue();

	while(!ret&&!adoEndOfFile())
	{
		int station;
		getValue("Station",&station);
		getValue("UserID",&_out.iUserID[station]);
		getValue("Score",&_out.i64Score[station]);
		getValue("LogoID",&_out.LogoID[station]);
		getValue("Nickname",_out.nickName[station],sizeof(_out.nickName[station]));
		getValue("HeadUrl",_out.HeadUrl[station],sizeof(_out.HeadUrl[station]));
		getValue("RunCount",&_out.iRunCount);

		moveNext();
	}

	_out.iDeskID = pReveiveData->iDeskID;

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GR_RETURNEDESK,0, sizeof(_out), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();
	return true;
}

int CGameDataBaseHandle::OnRecordGame(DataBaseLineHead *pSourceData)
{
	DL_GR_I_BUYDESK_GameRecord *pReveiveData = (DL_GR_I_BUYDESK_GameRecord *)pSourceData;
	if(!pReveiveData)
	{
		return 0;
	}

	CString UserIDbuffer="";
	CString UserStationbuffer="";
	TCHAR Temp[20];

	for (int i=0;i<pReveiveData->iUserCount;i++)
	{
		wsprintf(Temp, TEXT("%d,"), pReveiveData->pGameUserRecord[i].iUserID);
		UserIDbuffer += Temp;
		wsprintf(Temp, TEXT("%d,"), pReveiveData->pGameUserRecord[i].iScore);
		UserStationbuffer += Temp;
	}

	bool bres=sqlSPSetNameEx("SP_RecordGame",false);
	if(!bres)
	{
		return 0;
	}
	addInputParameter("@DeskPassWord",pReveiveData->szDeskPassWord,sizeof(pReveiveData->szDeskPassWord));
	addInputParameter("@DeskUserID",UserIDbuffer.GetBuffer(),sizeof(TCHAR) * (UserIDbuffer.GetLength()));
	addInputParameter("@UserScore",UserStationbuffer.GetBuffer(),sizeof(TCHAR) * (UserStationbuffer.GetLength()));
	addInputParameter("@GameCount",pReveiveData->iGameCount);
	addInputParameter("@GameSN",pReveiveData->szGameSN,sizeof(pReveiveData->szGameSN));
	addInputParameter("@GameType",pReveiveData->iGameType);
	addInputParameter("@GameID",m_pInitInfo->iGameID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	closeRecord();
	return true;
}

int CGameDataBaseHandle::OnCreateGameSN(DataBaseLineHead *pSourceData)
{
	DL_GR_I_CREATE_GAMESN *pReveiveData = (DL_GR_I_CREATE_GAMESN *)pSourceData;
	if(!pReveiveData)
	{
		return 0;
	}

	bool bres=sqlSPSetNameEx("SP_CreateGameSN",true);
	if(!bres)
	{
		return 0;
	}
	addInputParameter("@RoomID",pReveiveData->iRoomID);
	addInputParameter("@DeskID",pReveiveData->iDeskID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	DL_GR_O_CREATE_GAMESN _out;

	getValue("GameSN",_out.szGameSN,sizeof(_out.szGameSN));
	_out.iRoomID = pReveiveData->iRoomID;
	_out.iDeskID = pReveiveData->iDeskID;

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GR_CREATE_GAMESN,0, sizeof(_out), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();
	return true;
}

int CGameDataBaseHandle::OnCutNet(DataBaseLineHead *pSourceData)
{
	DL_GR_I_CutNetRecord *pReveiveData = (DL_GR_I_CutNetRecord *)pSourceData;
	if(!pReveiveData)
	{
		return 0;
	}
	CString UserIDbuffer="";
	CString UserStationbuffer="";
	TCHAR Temp[20];

	for (int i=0;i<pReveiveData->iPeopleCount;i++)
	{
		wsprintf(Temp, TEXT("%d,"), pReveiveData->iUserID[i]);
		UserIDbuffer += Temp;
		wsprintf(Temp, TEXT("%d,"), i);
		UserStationbuffer += Temp;
	}

	bool bres=sqlSPSetNameEx("SP_RecordNetCut",false);
	if(!bres)
	{
		return 0;
	}
	
	addInputParameter("@DeskPassWord",pReveiveData->szDeskPass,sizeof(pReveiveData->szDeskPass));
	addInputParameter("@DeskUserID",UserIDbuffer.GetBuffer(),sizeof(TCHAR) * UserIDbuffer.GetLength());
	addInputParameter("@UserStation",UserStationbuffer.GetBuffer(),sizeof(TCHAR) * UserStationbuffer.GetLength());
	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}
	closeRecord();
	return true;
}

int CGameDataBaseHandle::OnCutNetRecordStart(DataBaseLineHead *pSourceData)
{
	DL_GR_I_CutNetRecordStart *pReveiveData = (DL_GR_I_CutNetRecordStart *)pSourceData;
	if(!pReveiveData)
	{
		return 0;
	}
	HANDLE hDatabase=m_pDataBaseManage->hDBNative;
	CString sql="";
	sql.Format("update TBuyGameDeskRecord set IsPlay=%d where MasterID=%d and RoomID=%d and TableNumber=%d",pReveiveData->bStart,pReveiveData->iUserID,pReveiveData->iRoomID,pReveiveData->byDeskIndex);
	execSQL(sql,false);
	closeRecord();
	return true;
}


int CGameDataBaseHandle::OnGetRoomInfo(DataBaseLineHead *pSourceData)
{
	try
	{
		DL_GR_O_Get_RoomInfo _out;

		bool bres=sqlSPSetNameEx("SP_RoomStartProcess",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@RoomID",m_pInitInfo->uRoomID);
		addInputParameter("@LockTable",m_pInitInfo->szLockTable,sizeof(m_pInitInfo->szLockTable));

		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}

		if(!adoEndOfFile())
		{
			getValue("RoomWelcome",_out.szMsgRoomLogon,sizeof(_out.szMsgRoomLogon));
			getValue("RallWelcome",_out.szMsgGameLogon,sizeof(_out.szMsgGameLogon));
		}

		closeRecord();

		//获取游戏名字
		CString strSql;
		strSql.Format(TEXT("SELECT ComName FROM dbo.TGameNameInfo WHERE NameID=%d"), m_pInitInfo->uNameID);
		execSQL(strSql,true);	
		getValue("ComName",_out.szGameName,sizeof(_out.szGameName));
		closeRecord();

		strSql.Format(TEXT("SELECT TOP 1 SendCount_Alms,MinMoney_Alms,Invoice FROM dbo.Web_Config"));
		execSQL(strSql,true);
		int iSendCount = 0;
		int iInvoice = 0; 
		getValue("Invoice", &iInvoice);
		m_pInitInfo->bLockMaster = (iInvoice==0)?1:0;
		getValue(TEXT("SendCount_Alms"), &iSendCount);
		if (iSendCount>0)
		{
			m_pInitInfo->bSendAlms = true;
			getValue(TEXT("MinMoney_Alms"), &m_pInitInfo->iAlmsMinMoney); 
		}
		else
		{
			m_pInitInfo->bSendAlms = false;
			m_pInitInfo->iAlmsMinMoney = 0;
		}
		
		closeRecord();

		m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GR_GET_ROOMINFO,0, sizeof(_out), pSourceData->uIndex, pSourceData->dwHandleID);

		closeRecord();
	}
	catch (...)
	{
		::Sleep(500);
		CString str;
		str.Format("ycl:: OnGetRoomInfo retry room:%d",m_pInitInfo->uRoomID);
		OutputDebugString(str);
		closeRecord();
		OnGetRoomInfo(pSourceData);
	}
	return true;
}


int CGameDataBaseHandle::OnSetContestInfo(DataBaseLineHead *pSourceData)
{
	DL_GR_O_Get_ContestInfo _out;

	//获取比赛奖励
	CString strSql;
	int *pTemp = NULL;
	int *pTemp1 = NULL;
	
	strSql.Format(TEXT("select a.GiveNum, a.GiveType, b.StartTime,b.ContestType from Web_MatchAwardConfig a inner join TContestInfo b on a.TypeID=b.SendTypeID and b.ContestID=%d ORDER BY a.Rank"), m_pInitInfo->iContestID);
	execSQL(strSql, true);
	m_pInitInfo->iRankNum = getRecordCount();
	if (0 < m_pInitInfo->iRankNum)
	{
		m_pInitInfo->pAwards = new int[m_pInitInfo->iRankNum];
		pTemp = m_pInitInfo->pAwards;
		m_pInitInfo->pAwardTypes = new int[m_pInitInfo->iRankNum];
		pTemp1 = m_pInitInfo->pAwardTypes;
	}	
	while(!adoEndOfFile())
	{
		getValue("GiveNum",(m_pInitInfo->pAwards++));
		getValue("GiveType",(m_pInitInfo->pAwardTypes++));
		getValue("StartTime",&_out.MatchStartTime);
		getValue("ContestType",&m_pInitInfo->iContestType);
		moveNext();
	}
	closeRecord();
	if (pTemp)
	{
		m_pInitInfo->pAwards = pTemp;
	}
	if (pTemp1)
	{
		m_pInitInfo->pAwardTypes = pTemp1;
	}

	m_pRusultService->OnDataBaseResultEvent(&_out.ResultHead, DTK_GR_GET_CONTESTINFO,0, sizeof(_out), pSourceData->uIndex, pSourceData->dwHandleID);

	closeRecord();

	if (2 == m_pInitInfo->iContestType)
	{
		bool bres=sqlSPSetNameEx("SP_ContestRegist",true);
		if(!bres)
		{
			return 0;
		}

		addInputParameter("@RoomID",m_pInitInfo->uRoomID);
		addInputParameter("@ContestID",m_pInitInfo->iContestID);
		if(0 != execStoredProc())
		{
			closeRecord();
			return 0;
		}

		int ret = getReturnValue();

		DataBaseResultLine _out1;
		memset(&_out1,0,sizeof(DataBaseResultLine));
		m_pRusultService->OnDataBaseResultEvent(&_out1, DTK_GR_CONTEST_REGIST,ret, sizeof(DataBaseResultLine), pSourceData->uIndex, pSourceData->dwHandleID);

		closeRecord();
	}
	
	return true;
}



int CGameDataBaseHandle::OnUpdateNetCutTime(DataBaseLineHead *pSourceData)
{
	DL_GR_I_UpdateNetCutTime *_pIn = (DL_GR_I_UpdateNetCutTime *)pSourceData;

	CString strSql;
	strSql.Format(TEXT("UPDATE dbo.TContestUserRecord SET NetCutTime = GETDATE() WHERE UserID=%d AND MatchID = %d "), _pIn->iUserID,_pIn->iMatchID);
	execSQL(strSql, true);
	
	closeRecord();
	return true;
}

int CGameDataBaseHandle::OnMasterLeave(DataBaseLineHead *pSourceData)
{
	DL_GR_I_MasterLeave *pReveiveData = (DL_GR_I_MasterLeave *)pSourceData;
	if(!pReveiveData)
	{
		return 0;
	}

	bool bres=sqlSPSetNameEx("SP_MasterLeave",true);
	if(!bres)
	{
		return 0;
	}
	addInputParameter("@UserID",pReveiveData->iUserID);
	addInputParameter("@DeskID",pReveiveData->iDeskID);
	addInputParameter("@RoomID",m_pInitInfo->uRoomID);
	addInputParameter("@Leave",pReveiveData->bLeave);
	addInputParameter("@bMaster",pReveiveData->bMaster);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	closeRecord();
	return true;
}



int CGameDataBaseHandle::OnContestExit(DataBaseLineHead *pSourceData)
{
	DL_GR_I_ContestExit *pReveiveData = (DL_GR_I_ContestExit *)pSourceData;
	if(!pReveiveData)
	{
		return 0;
	}

	bool bres=sqlSPSetNameEx("SP_ContestExit",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@UserID",pReveiveData->iUserID);
	addInputParameter("@RoomID",m_pInitInfo->uRoomID);
	addInputParameter("@ContestID",m_pInitInfo->iContestID);
	addInputParameter("@MatchID",m_pInitInfo->iMatchID);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	closeRecord();
	return true;
}

int CGameDataBaseHandle::OnChangeUserStation(DataBaseLineHead *pSourceData)
{
	DL_GR_I_ChangeUserStation *pReveiveData = (DL_GR_I_ChangeUserStation *)pSourceData;
	if(!pReveiveData)
	{
		return 0;
	}

	bool bres=sqlSPSetNameEx("SP_ChangeUserStation",true);
	if(!bres)
	{
		return 0;
	}

	addInputParameter("@DeskPass",pReveiveData->szDeskPass,sizeof(pReveiveData->szDeskPass));
	addInputParameter("@UserIDA",pReveiveData->_data.iUserIDA);
	addInputParameter("@UserStationA",pReveiveData->_data.bUserANewStation);
	addInputParameter("@UserIDB",pReveiveData->_data.iUserIDB);
	addInputParameter("@UserStationB",pReveiveData->_data.bUserBNewStation);

	if(0 != execStoredProc())
	{
		closeRecord();
		return 0;
	}

	closeRecord();
	return true;
}

int CGameDataBaseHandle::OnSetGameDynamicInfo(DataBaseLineHead *pSourceData)
{
    DL_GR_Set_DynamicInfo *pReveiveData = (DL_GR_Set_DynamicInfo *)pSourceData;
    if(!pReveiveData)
    {
        return 0;
    }

    bool bres=sqlSPSetNameEx("SP_SetGameDynamicInfo",true);
    if(!bres)
    {
        return 0;
    }
    addInputParameter("@GameID",pReveiveData->iGameID);
    addInputParameter("@DynamicInfo1",(BYTE*)pReveiveData->szDynamicInfo1, sizeof(pReveiveData->szDynamicInfo1));
    if(0 != execStoredProc())
    {
        closeRecord();
        return 0;
    }

    closeRecord();
    return true;
}

int CGameDataBaseHandle::OnUpdateBeginTime(DataBaseLineHead *pSourceData)
{
    DL_GR_I_UpdateBeginTime *pReveiveData = (DL_GR_I_UpdateBeginTime *)pSourceData;
    if(!pReveiveData)
    {
        return 0;
    }

    CString strSql;
    strSql.Format(TEXT("UPDATE dbo.TBuyGameDeskRecord SET BeginTime = GETDATE() WHERE RoomID=%d AND TableNumber = %d AND szDeskPassword=%s")
    ,pReveiveData->iRoomID,pReveiveData->iDeskID,pReveiveData->szDeskPass);
    execSQL(strSql, true);

    closeRecord();
    return true;
}

int CGameDataBaseHandle::OnDissmissDeskByMS(DataBaseLineHead *pSourceData)
{
    DL_GR_I_DissmissDeskByMS *pReveiveData = (DL_GR_I_DissmissDeskByMS *)pSourceData;
    if(!pReveiveData)
    {
        return 0;
    }
    DL_GR_O_DissmissDeskByMS outdata;

    memcpy(&outdata.data, &pReveiveData->data, sizeof(outdata.data));

    m_pRusultService->OnDataBaseResultEvent(&outdata.ResultHead, DTK_GR_DISSMISSDESKBYMS,0, sizeof(DL_GR_O_DissmissDeskByMS), pSourceData->uIndex, pSourceData->dwHandleID);
    return true;
}

int CGameDataBaseHandle::OnRobotRandIPList(DataBaseLineHead *pSourceData)
{
	CGameDataBaseHandle::m_vecRandIPList.clear();	
	CString sql = "select id,RandIP from TRobotRandIPList";
	execSQL(sql, true);

	int		id;
	char	randIP[30] = {0};
	ULONG   uIP = 0;
	while(!adoEndOfFile())
	{
		id = 0;
		memset(randIP, 0, sizeof(randIP));

		getValue("id",&id);
		getValue("RandIP",randIP, sizeof(randIP));
		uIP =  CTCPSocketManage::IPToULongEx(randIP);
		CGameDataBaseHandle::m_vecRandIPList.push_back(uIP);		
		moveNext();
	}
	closeRecord();
	return true;
}


int CGameDataBaseHandle::OnSetSPRoomInfo(DataBaseLineHead *pSourceData)
{
    DL_GR_I_SetGameRoomInfo *pReveiveData = (DL_GR_I_SetGameRoomInfo *)pSourceData;
    if(!pReveiveData)
    {
        return 0;
    }

    bool bres=sqlSPSetNameEx("SP_SetSPGameInfo",true);
    if(!bres)
    {
        return 0;
    }
    addInputParameter("@RoomID",pReveiveData->iRoomID);
    addInputParameter("@RoomConfig",(BYTE*)pReveiveData->bRoomConfig,sizeof(pReveiveData->bRoomConfig));
    if(0 != execStoredProc())
    {
        closeRecord();
        return 0;
    }

    closeRecord();
    return true;
}