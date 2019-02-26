/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "StdAfx.h"
#include "UpdateUserInfoServer.h"
#include "GameLogonManage.h"
#include "commonuse.h"

CUpdateUserInfo::CUpdateUserInfo(CGameLogonManage* pLogonManage)
	:_pLogonManage(pLogonManage)
{

}

CUpdateUserInfo::~CUpdateUserInfo(void)
{

}


bool CUpdateUserInfo::OnNetMessage(NetMessageHead* pNetHead,void* pData,UINT uSize,ULONG uAccessIP,UINT uIndex,DWORD dwHandleID)
{
	if (!_pLogonManage) 
	{
		return true;
	}

	if(pNetHead->bMainID == MDM_GP_LOCK_ACCOUNT)
	{
		return lockAccountRequest(pData,uSize,uIndex,dwHandleID);
	}
	else if(pNetHead->bMainID == MDM_GP_BIND_MOBILE )	//用户绑定/取决绑定手机
	{
		return bindMobileRequest(pData,uSize,uIndex,dwHandleID);
	}
	else if (pNetHead->bMainID == MDM_GP_SET_LOGO)
	{
		return setLogoRequest(pData,uSize,uIndex,dwHandleID);
	}
	else if (pNetHead->bMainID == MDM_GP_USERINFO)		//修改用户信息
	{
		if (pNetHead->bAssistantID == ASS_GP_USERINFO_UPDATE_PWD)
		{
			return updatePwdRequest(pData,uSize,uIndex,dwHandleID);
		}
		else if (pNetHead->bAssistantID == ASS_GP_USERINFO_NICKNAMEID)
		{
			return getNickNameIDRequest(pData,uSize,uIndex,dwHandleID);
		}
		else if (pNetHead->bAssistantID == ASS_GP_USERINFO_UPDATE_BASE || pNetHead->bAssistantID == ASS_GP_USERINFO_UPDATE_DETAIL)
		{
			return updateUserInfoRequest(pNetHead,pData,uSize,uIndex,dwHandleID);
		}
		else if (pNetHead->bAssistantID == ASS_GP_USERINFO_FORGET_PWD)
		{
			return ForgetpwdRequest(pData,uSize,uIndex,dwHandleID);
		}
      /*  else if (pNetHead->bAssistantID == ASS_GP_USERINFO_ADDPHONENUM)
        {
            WriteLog("收到 ASS_GP_USERINFO_ADDPHONENUM 请求");
            return AddUserPhoneNum(pData,uSize,uIndex,dwHandleID);
        }*/
        
		return true;
	}

	return true ;
}

bool CUpdateUserInfo::OnDataBaseResult(DataBaseResultLine* pResultData)
{
	if (!_pLogonManage) 
	{
		return true;
	}

	switch(pResultData->uHandleKind)
	{
	case DTK_GP_LOCK_UNLOCK_ACCOUNT:
		{
			return lockAccountResponse(pResultData);
		}break;
	case DTK_GP_BIND_MOBILE:
		{
			return bindMobileResponse(pResultData);
		}
	case DTK_GP_SET_LOGO:
		{
			return setLogoResponse(pResultData);
		}
	case DTK_GP_UPDATE_USERINFO_BASE:
	case DTK_GP_UPDATE_USERINFO_DETAIL:
	case DTR_GP_UPDATE_USERINFO_PWD:
		{
			if (pResultData->uHandleRusult == DTR_GP_UPDATE_USERINFO_ACCEPT)
			{
				_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex,MDM_GP_USERINFO,ASS_GP_USERINFO_ACCEPT,pResultData->uHandleRusult,pResultData->dwHandleID);
			}
			else if (pResultData->uHandleRusult == DTR_GP_UPDATE_USERINFO_NOTACCEPT)
			{
				_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex,MDM_GP_USERINFO,ASS_GP_USERINFO_NOTACCEPT,pResultData->uHandleRusult,pResultData->dwHandleID);
			}

			return true;
		}break;
   // case DTK_GP_USER_ADDPHONENUM:
   //     {
    //          _pLogonManage->m_TCPSocket.SendData(pResultData->uIndex,MDM_GP_USERINFO,ASS_GP_USERINFO_ADDPHONENUM,pResultData->uHandleRusult,pResultData->dwHandleID);
    //          return true;
    //    }break;
	case DTK_GP_USERINFO_FORGET_PWD:
		{
			_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, MDM_GP_USERINFO, ASS_GP_USERINFO_FORGET_PWD, pResultData->uHandleRusult, pResultData->dwHandleID);
		}break;
	default:
		return true;
	}

	return true;
}

bool CUpdateUserInfo::lockAccountRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GP_S_LockAccount * pLockAccount = (MSG_GP_S_LockAccount*)pData;
	if( pLockAccount == NULL )
		return TRUE;
	DL_GP_LockAccount DataLockAccount;
	memset(&DataLockAccount,0, sizeof(DataLockAccount));
	DataLockAccount.LockAccount = *pLockAccount;
	_pLogonManage->m_SQLDataManage.PushLine(&DataLockAccount.DataBaseHead, sizeof(DataLockAccount), DTK_GP_LOCK_UNLOCK_ACCOUNT, uIndex, dwHandleID);

	return true;
}

bool CUpdateUserInfo::lockAccountResponse(DataBaseResultLine* pResultData)
{
	MSG_GP_S_LockAccountResult * pLockResult = (MSG_GP_S_LockAccountResult*)pResultData;
	MSG_GP_R_LockAndUnlock LockAndUnlock;
	memset(&LockAndUnlock,0, sizeof(LockAndUnlock));
	LockAndUnlock.dwUserID = pLockResult->dwUserID;
	LockAndUnlock.dwCommanType = pLockResult->dwCommanType;
	LockAndUnlock.dwCommanResult = pLockResult->dwCommanResult;
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &LockAndUnlock, sizeof(LockAndUnlock), MDM_GP_LOCK_ACCOUNT, ASS_LOCK_ACCOUNT, 0, 0);

	return true;
}

bool CUpdateUserInfo::bindMobileRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GP_S_BindMobile * pBindMobile = (MSG_GP_S_BindMobile*)pData;
	if( pBindMobile == NULL )
		return true;
	DL_GP_BindMobile _in;
	memset(&_in,0, sizeof(DL_GP_BindMobile));
	_in.data = *pBindMobile;
	_pLogonManage->m_SQLDataManage.PushLine(&_in.DataBaseHead, sizeof(DL_GP_BindMobile), DTK_GP_BIND_MOBILE, uIndex, dwHandleID);
	
	return true;
}

bool CUpdateUserInfo::bindMobileResponse(DataBaseResultLine* pResultData)
{
	MSG_GP_S_BindMobileResult* pBindResult = (MSG_GP_S_BindMobileResult*)pResultData;
	MSG_GP_R_BindMobile BindResult;
	memset(&BindResult,0, sizeof(MSG_GP_R_BindMobile));
	BindResult.dwUserID = pBindResult->dwUserID;
	BindResult.dwCommanType = pBindResult->dwCommanType;
	BindResult.dwCommanResult = pBindResult->dwCommanResult;
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &BindResult, sizeof(MSG_GP_R_BindMobile), MDM_GP_BIND_MOBILE, ASS_GP_BIND_MOBILE, 0, 0);

	return true;
}

bool CUpdateUserInfo::setLogoRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GP_S_SET_LOGO *pLogoInfo = (MSG_GP_S_SET_LOGO*)pData;
	if (pLogoInfo == NULL)
		return TRUE;
	DL_GP_SetLogo _in;
	memset(&_in,0, sizeof(DL_GP_SetLogo));
	_in.LogoInfo.iLogoID = pLogoInfo->iLogoID;
	_in.LogoInfo.iUserID = pLogoInfo->iUserID;
	_pLogonManage->m_SQLDataManage.PushLine(&_in.DataBaseHead, sizeof(DL_GP_SetLogo), DTK_GP_SET_LOGO, uIndex, dwHandleID);

	return true;
}

bool CUpdateUserInfo::setLogoResponse(DataBaseResultLine* pResultData)
{
	

	return true;
}

bool CUpdateUserInfo::updatePwdRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GP_S_ChPassword* pUserChPwd = (MSG_GP_S_ChPassword*)pData;

	DL_GP_I_ChPasswordStruct DataChPwd;
	memset(&DataChPwd,0,sizeof(DL_GP_I_ChPasswordStruct));
	DataChPwd.ChPwd = *pUserChPwd;

	_pLogonManage->m_SQLDataManage.PushLine(&DataChPwd.DataBaseHead,sizeof(DL_GP_I_ChPasswordStruct),DTK_GP_UPDATE_USERINFO_PWD,uIndex,dwHandleID);//提交数据库
	return true;
}

bool CUpdateUserInfo::getNickNameIDRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	// 效验数据
	if (uSize != sizeof(MSG_GP_NickNameID_t)) return false;
	MSG_GP_NickNameID_t* p = (MSG_GP_NickNameID_t*)pData;

	// 加入数据库
	DL_GP_I_GetNickNameID _in;
	_in._data = *p;
	_pLogonManage->m_SQLDataManage.PushLine(&_in.DataBaseHead,sizeof(DL_GP_I_GetNickNameID),DTK_GP_UPDATE_NICKNAMEID,uIndex,dwHandleID);
	return true;
}

bool CUpdateUserInfo::updateUserInfoRequest(NetMessageHead* pNetHead,void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GP_S_UserInfo* pUser = (MSG_GP_S_UserInfo*)pData;

	DL_GP_I_UserInfoStruct DataUserInfo;
	memset(&DataUserInfo,0,sizeof(DL_GP_I_UserInfoStruct));
	DataUserInfo.UserInfo = *pUser;

	if (pNetHead->bAssistantID == ASS_GP_USERINFO_UPDATE_BASE)
	{
		_pLogonManage->m_SQLDataManage.PushLine(&DataUserInfo.DataBaseHead,sizeof(DL_GP_I_UserInfoStruct),DTK_GP_UPDATE_USERINFO_BASE,uIndex,dwHandleID);//提交数据库
	}
	else if (pNetHead->bAssistantID == ASS_GP_USERINFO_UPDATE_DETAIL)
	{
		_pLogonManage->m_SQLDataManage.PushLine(&DataUserInfo.DataBaseHead,sizeof(DL_GP_I_UserInfoStruct),DTK_GP_UPDATE_USERINFO_DETAIL,uIndex,dwHandleID);//提交数据库
	}
	return true;
}

bool CUpdateUserInfo::ForgetpwdRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (sizeof(MSG_GP_I_ForgetPWD) != uSize) return false;

	DL_I_HALL_ForgetPWD _in;
	memcpy_s(&_in._data,sizeof(_in._data),pData,sizeof(MSG_GP_I_ForgetPWD));
	_pLogonManage->m_SQLDataManage.PushLine(&_in.DataBaseLineHead,sizeof(DL_I_HALL_ForgetPWD),DTK_GP_USERINFO_FORGET_PWD,uIndex,dwHandleID);

	return true;
}



//bool CUpdateUserInfo::AddUserPhoneNum(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
//{
//    if (sizeof(MSG_GP_I_AddUserPhoneNum) != uSize)
//    {
//        WriteLog("!Size");
//         return false;
//    }
//    DL_I_HALL_AddUserPhoneNum _in;
//    memcpy_s(&_in._data,sizeof(_in._data),pData,sizeof(MSG_GP_I_AddUserPhoneNum));
//    _pLogonManage->m_SQLDataManage.PushLine(&_in.DataBaseLineHead,sizeof(DL_I_HALL_AddUserPhoneNum),DTK_GP_USER_ADDPHONENUM,uIndex,dwHandleID);
//
//    return true;
//}

