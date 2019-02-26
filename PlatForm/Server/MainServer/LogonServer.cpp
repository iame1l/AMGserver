/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "StdAfx.h"
#include "LogonServer.h"
#include "GameLogonManage.h"
#include "commonuse.h"

CLogonServer::CLogonServer(CGameLogonManage* pLogonManage)
	:_pLogonManage(pLogonManage)
{

}

CLogonServer::~CLogonServer(void)
{

}


bool CLogonServer::OnNetMessage(NetMessageHead* pNetHead,void* pData,UINT uSize,ULONG uAccessIP,UINT uIndex,DWORD dwHandleID)
{
	if (!_pLogonManage) 
	{
		return true;
	}

	if (MDM_GP_LOGON != pNetHead->bMainID)
	{
		return true;
	}

	switch(pNetHead->bAssistantID)
	{
	case ASS_GP_LOGON_BY_NAME:
		{
			onUserLogonRequest(pData,uSize,uAccessIP,uIndex,dwHandleID);
		}break;
	default:
		{
			return true;
		}break;
	}

	return true ;
}

bool CLogonServer::OnDataBaseResult(DataBaseResultLine* pResultData)
{
	if (!_pLogonManage) 
	{
		return true;
	}

	switch(pResultData->uHandleKind)
	{
	case DTK_GP_LOGON_BY_NAME:
		{
			onUserLogonResponse(pResultData);
		}break;
	default:
		return true;
	}

	return true;
}

bool CLogonServer::onUserLogonRequest(void* pData,UINT uSize,ULONG uAccessIP,UINT uIndex,DWORD dwHandleID)
{
	//效验数据
	if (uSize!=sizeof(MSG_GP_S_LogonByNameStruct)) return false;///验证结构是否一致

	//处理登陆
	MSG_GP_S_LogonByNameStruct * pLogonInfo=(MSG_GP_S_LogonByNameStruct *)pData;

	if (pLogonInfo->bForced)
	{
		for(int i=0; i<_pLogonManage->m_UserList.GetCount(); i++)
		{
			LPUSER p = (LPUSER)_pLogonManage->m_UserList.GetAt(i);
			if(p == NULL)
				continue;
			if(!strcmp(p->UserName,pLogonInfo->szName))
			{
				int SocketID = p->iSocketIndex;
				_pLogonManage->m_TCPSocket.CloseSocket(SocketID,0);
				_pLogonManage->OnSocketClose(uAccessIP,SocketID,0);
				break;
			}
		}
	}

	pLogonInfo->szName[sizeof(pLogonInfo->szName)/sizeof(pLogonInfo->szName[0])-1]=0;
	pLogonInfo->TML_SN[sizeof(pLogonInfo->TML_SN)/sizeof(pLogonInfo->TML_SN[0])-1]=0;
	pLogonInfo->szMD5Pass[sizeof(pLogonInfo->szMD5Pass)/sizeof(pLogonInfo->szMD5Pass[0])-1]=0;
	//加入队列
	DL_GP_I_LogonByNameStruct DL_Logon;
	DL_Logon.LogonInfo=*pLogonInfo;

	DL_Logon.ID=_pLogonManage->m_ID;

	CTCPSocketManage::ULongToIP(uAccessIP,DL_Logon.szAccessIP);//把IP弄过来

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Logon.DataBaseHead,sizeof(DL_Logon),DTK_GP_LOGON_BY_NAME,uIndex,dwHandleID);//提交数据库

	return true;
	
}

bool CLogonServer::onUserLogonResponse(DataBaseResultLine* pResultData)
{
	//效验数据
	if (pResultData->LineHead.uSize!=sizeof(DL_GP_O_LogonResultStruct)) return false;

	//判断是否还存在连接
	if (_pLogonManage->m_TCPSocket.IsConnectID(pResultData->uIndex,pResultData->dwHandleID)==false) return false;

	//处理数据
	if (pResultData->uHandleRusult==ERR_GP_LOGON_SUCCESS)	
	{
		//登陆成功
		DL_GP_O_LogonResultStruct * pLogonResult=(DL_GP_O_LogonResultStruct *)pResultData;

		//重复登陆
		for(int i=0; i<_pLogonManage->m_UserList.GetCount(); i++)
		{
			LPUSER p = (LPUSER)_pLogonManage->m_UserList.GetAt(i);
			if(p == NULL)
				continue;
			if(p->UserID == pLogonResult->LogonResult.dwUserID)
			{
				int SocketID = p->iSocketIndex;
				_pLogonManage->m_TCPSocket.CloseSocket(SocketID,0);
				_pLogonManage->OnSocketClose(0,SocketID,0);
			}
		}

		//新用户
		LPUSER p = (LPUSER)new USERS();
		p->iSocketIndex = pResultData->uIndex;
		p->UserID = pLogonResult->LogonResult.dwUserID;
		strcpy(p->UserName, pLogonResult->LogonResult.szName);
		p->iAddFriendType = pLogonResult->LogonResult.iAddFriendType;
		p->iVipTime = pLogonResult->LogonResult.iVipTime;
		_pLogonManage->m_UserList.Add(p);

		HNLOG_M("UserLoginPlace SocketID:%d,UserID:%d",p->iSocketIndex,p->UserID);

		//将用户注册到B服务器
		MSG_ZDistriInfo info;
		info.dwUserID = pLogonResult->LogonResult.dwUserID;
		info.nZID = _pLogonManage->m_ID;
		_pLogonManage->m_pDistriManager->SendNormalData(&info, sizeof(info), MDM_ZD_PACKAGE, ASS_ZD_LOGON, HDC_ZD_CONNECT);

		MSG_GP_R_LogonResult MSGLogonResult;
		MSGLogonResult = pLogonResult->LogonResult;


		_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex,&MSGLogonResult,sizeof(MSGLogonResult),MDM_GP_LOGON,
			ASS_GP_LOGON_SUCCESS,ERR_GP_LOGON_SUCCESS,pResultData->dwHandleID);

		//追加一个转账返利数据包sdp
		_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex,&_pLogonManage->m_Rebate,sizeof(_pLogonManage->m_Rebate),MDM_GP_BANK,
			ASS_GP_BANK_TRANSFER,HC_GP_BANK_TRANSFER_UPDATE_REBATE,0);

		//查询玩家返利数据并告知玩家
		DL_GP_I_UserRebateUpDateStruct UserRebate;
		UserRebate.iUserID = pLogonResult->LogonResult.dwUserID;//赋值玩家id
		_pLogonManage->m_SQLDataManage.PushLine(&UserRebate.DataBaseHead, sizeof(UserRebate), DTK_GP_BANK_UPDATE_USER_REBATE, pResultData->uIndex, 0);

		//更改写Z标志时机
		DL_GP_WriteZRecord WriteZRecord;
		memset(&WriteZRecord, 0, sizeof(WriteZRecord));
		WriteZRecord.lUserID = pLogonResult->LogonResult.dwUserID;
		WriteZRecord.ZID = _pLogonManage->m_ID;
		WriteZRecord.iSocketID = pResultData->uIndex;
		_pLogonManage->m_SQLDataManage.PushLine(&WriteZRecord.DataBaseHead,sizeof(WriteZRecord),DTK_GP_ZRECORD,pResultData->uIndex,0);

		//即将开始比赛通知玩家
		DL_I_HALL_CONTEST_NOTICE_LOGON ContestNotice;
		ContestNotice.iUserID = pLogonResult->LogonResult.dwUserID;
		_pLogonManage->m_SQLDataManage.PushLine(&ContestNotice.DataBaseLineHead,sizeof(ContestNotice),DTK_GP_CONTEST_NOTICE_LOGON,pResultData->uIndex,0);
	}
	// 异地登陆处理
	else if (pResultData->uHandleRusult==ERR_GP_ALLO_PARTY)
	{
		/*DL_GP_O_LogonResultStruct* pLogonResult=(DL_GP_O_LogonResultStruct*)pResultData;
		MSG_GP_R_IDVerify tagIDVerify;
		tagIDVerify.dwUserID = pLogonResult->LogonResult.dwUserID;
		::memcpy(tagIDVerify.szMD5IDCardNo,pLogonResult->LogonResult.szIDCardNo,sizeof(tagIDVerify.szMD5IDCardNo));

		MD5_CTX Md5;
		Md5.MD5Update((unsigned char*)tagIDVerify.szMD5IDCardNo,lstrlen(tagIDVerify.szMD5IDCardNo));
		unsigned char szMDTemp[16];
		Md5.MD5Final(szMDTemp);
		for (int i=0;i<16;i++) 
		{
			wsprintf(&tagIDVerify.szMD5IDCardNo[i*2],"%02x",szMDTemp[i]);
		}


		_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex,&tagIDVerify,sizeof(MSG_GP_R_IDVerify),MDM_GP_LOGON,
			ASS_GP_LOGON_ALLO_PART,ERR_GP_ALLO_PARTY,pResultData->dwHandleID);*/
	}
	else
	{
		// 登陆失败
		switch (pResultData->uHandleRusult)
		{
		case ERR_GP_ACCOUNT_LOCKED:
			{
				// 以往锁机不能异地登陆，现在锁机是允许异地登陆，但需要验证(如手机短信等)
				DL_GP_O_LogonResultStruct* pLogonResult=(DL_GP_O_LogonResultStruct*)pResultData;
				MSG_GP_R_MobileVerify tagMobileVerify;
				::memset(&tagMobileVerify,0,sizeof(MSG_GP_R_MobileVerify));
				tagMobileVerify.dwUserID = pLogonResult->LogonResult.dwUserID;
				::strcpy(tagMobileVerify.szMobileNo,pLogonResult->LogonResult.szMobileNo);

				_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex,&tagMobileVerify,sizeof(MSG_GP_R_MobileVerify),
					MDM_GP_LOGON,ASS_GP_LOGON_LOCK_VALID,ERR_GP_ACCOUNT_LOCKED,0);

				return true;
			}
		case ERR_GP_MOBILE_VALID:
			{
				// 以往锁机不能异地登陆，现在锁机是允许异地登陆，但需要验证(如手机短信等)
				DL_GP_O_LogonResultStruct* pLogonResult=(DL_GP_O_LogonResultStruct*)pResultData;
				MSG_GP_R_MobileVerify tagMobileVerify;
				::memset(&tagMobileVerify,0,sizeof(MSG_GP_R_MobileVerify));
				tagMobileVerify.dwUserID = pLogonResult->LogonResult.dwUserID;
				::strcpy(tagMobileVerify.szMobileNo,pLogonResult->LogonResult.szMobileNo);

				_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex,&tagMobileVerify,sizeof(MSG_GP_R_MobileVerify),
					MDM_GP_LOGON,ASS_GP_LOGON_MOBILE_VALID,0,0);

				return true;
			}
		}	 

		_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex,MDM_GP_LOGON,ASS_GP_LOGON_ERROR,pResultData->uHandleRusult,pResultData->dwHandleID);
	}

	return true;
}