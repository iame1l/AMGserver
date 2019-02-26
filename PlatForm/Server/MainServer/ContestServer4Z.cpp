/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "StdAfx.h"
#include "ContestServer4Z.h"
#include "GameLogonManage.h"
#include "commonuse.h"


CContestServer4Z::CContestServer4Z(CGameLogonManage* pLogonManage)
{
	_pLogonManage = pLogonManage;
}

CContestServer4Z::~CContestServer4Z(void)
{

}

bool CContestServer4Z::OnNetMessage(NetMessageHead* pNetHead,void* pData,UINT uSize,ULONG uAccessIP,UINT uIndex,DWORD dwHandleID)
{
	if (!_pLogonManage) 
	{
		return true;
	}

	if (MDM_GP_CONTEST != pNetHead->bMainID)
	{
		return true;
	}

	if (ASS_GP_GET_CONTEST_ROOMID == pNetHead->bAssistantID)
	{
		return getContestRoomIDRequest(pData,uSize,uIndex,dwHandleID);
	}
	else if (ASS_GP_UPDATE_CONTESTPEOPLE == pNetHead->bAssistantID)
	{
		return true;
	}
	else if (ASS_GP_GET_APPLY_NUM == pNetHead->bAssistantID)
	{
		return getApplyNumRequest(pData,uSize,uIndex,dwHandleID);
	}
	else if (ASS_GP_CONTEST_APPLY == pNetHead->bAssistantID)
	{
		return onContestApplyRequest(pData,uSize,uIndex,dwHandleID);
	}
	else if (ASS_GP_CONTEST_AWARDINFO == pNetHead->bAssistantID)
	{
		return onContestAwardRequest(pData,uSize,uIndex,dwHandleID);
	}

	return true;
}

bool CContestServer4Z::OnDataBaseResult(DataBaseResultLine* pResultData)
{
	if (!_pLogonManage) 
	{
		return true;
	}

	switch(pResultData->uHandleKind)
	{
	case DTK_GP_GET_CONTEST_ROOMID:
		{
			getContestRoomIDResponse(pResultData);
		}break;
	case DTK_GP_GET_APPLY_NUM:
		{
			getApplyNumResponse(pResultData);
		}break;
	case DTK_GP_CONTEST_APPLY:
		{
			onContestApplyResponse(pResultData);
		}break;
	case DTK_GP_CONTEST_NOTICE:
		{
			notifyContestResponse(pResultData);
		}break;
	case DTK_GP_CONTEST_NOTICE_LOGON:
		{
			notifyContestLogonResponse(pResultData);
		}break;
	case DTK_GP_CONTEST_GETAWARD:
		{
			getContestAwardResponse(pResultData);
		}break;
	default:
		return true;
	}

	return true;

}


bool CContestServer4Z::getContestRoomIDRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_GetContestRoomID))
		return false;

	MSG_GP_GetContestRoomID* pContestRoomID = (MSG_GP_GetContestRoomID*)pData;
	if (!pContestRoomID)
		return false;

	DL_GP_I_GetContestRoomID DL_ContestRoom;
	memcpy_s(&DL_ContestRoom._data,sizeof(DL_ContestRoom._data),pContestRoomID,sizeof(MSG_GP_GetContestRoomID));
	_pLogonManage->m_SQLDataManage.PushLine(&DL_ContestRoom.DataBaseHead, sizeof(DL_ContestRoom), DTK_GP_GET_CONTEST_ROOMID, uIndex, dwHandleID);

	return true;
}

bool CContestServer4Z::getApplyNumRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(int))
		return false;

	DL_GP_I_UpdateApplyNum _in;
	_in.iContestID = *(int*)pData;
	_pLogonManage->m_SQLDataManage.PushLine(&_in.DataBaseHead,sizeof(DL_GP_I_UpdateApplyNum),DTK_GP_GET_APPLY_NUM,uIndex,dwHandleID);

	return true;
}

bool CContestServer4Z::onContestApplyRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_ContestApply))
		return false;

	DL_GP_I_ContestApply _in;
	memcpy_s(&_in._data,sizeof(MSG_GP_ContestApply),pData,sizeof(MSG_GP_ContestApply));
	_pLogonManage->m_SQLDataManage.PushLine(&_in.DataBaseHead,sizeof(DL_GP_I_ContestApply),DTK_GP_CONTEST_APPLY,uIndex,dwHandleID);

	return true;
}

bool CContestServer4Z::onContestAwardRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_GetContestAward))
		return false;

	DL_GP_I_ContestAward _in;
	memcpy_s(&_in._data,sizeof(MSG_GP_I_GetContestAward),pData,sizeof(MSG_GP_I_GetContestAward));
	_pLogonManage->m_SQLDataManage.PushLine(&_in.DataBaseHead,sizeof(DL_GP_I_ContestAward),DTK_GP_CONTEST_GETAWARD,uIndex,dwHandleID);

	return true;
}

bool CContestServer4Z::getContestRoomIDResponse(DataBaseResultLine* pResultData)
{
	DL_GP_O_GetContestRoomID *pContestRoom = (DL_GP_O_GetContestRoomID*)pResultData;
	if (pContestRoom)
	{
		ComRoomInfo *pFindRoomInfo=_pLogonManage->m_GameList.FindRoomInfo(pContestRoom->iRoomID);
		if(pFindRoomInfo)
		{
			pContestRoom->_data.tRoomInfo=*pFindRoomInfo;
		}
		else
		{
			pResultData->uHandleRusult=1;
		}

		_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pContestRoom->_data, sizeof(pContestRoom->_data), MDM_GP_CONTEST, ASS_GP_GET_CONTEST_ROOMID, pResultData->uHandleRusult, pResultData->dwHandleID);

		if (1 == pContestRoom->iNotifyG && 0 == pResultData->uHandleRusult)
		{
			MSG_MG_S_CONTEST_ACTIVE _data;
			_data.BeginTime = pContestRoom->BeginTime;
			_data.bNotify = 1;
			_data.iMatchID = pContestRoom->iMatchID;
			_data.iRoonID = pContestRoom->iRoomID;
			_pLogonManage->m_pGameManageModule->m_LogonManage.SendData(pContestRoom->iRoomID,&_data,sizeof(MSG_MG_S_CONTEST_ACTIVE),MDM_MG_CONTEST,ASS_MG_CONTEST_APPLY,0,0);
		}
	}
	return true;
}

bool CContestServer4Z::getApplyNumResponse(DataBaseResultLine* pResultData)
{
	DL_GP_O_UpdateApplyNum* pOut = (DL_GP_O_UpdateApplyNum*)pResultData;
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pOut->_data, sizeof(pOut->_data), MDM_GP_CONTEST, ASS_GP_GET_APPLY_NUM, pResultData->uHandleRusult, pResultData->dwHandleID);
	return true;
}

bool CContestServer4Z::onContestApplyResponse(DataBaseResultLine* pResultData)
{
	DL_GP_O_ContestApply* pOut = (DL_GP_O_ContestApply*)pResultData;
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pOut->_data, sizeof(pOut->_data), MDM_GP_CONTEST, ASS_GP_CONTEST_APPLY, pResultData->uHandleRusult, pResultData->dwHandleID);

	if (pOut->_Notify.bNotify)
	{
		_pLogonManage->m_pGameManageModule->m_LogonManage.SendData(pOut->_Notify.iRoonID,&pOut->_Notify,sizeof(MSG_MG_S_CONTEST_ACTIVE),MDM_MG_CONTEST,ASS_MG_CONTEST_APPLY,0,0);
	}
	return true;
}

bool CContestServer4Z::notifyContestResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_CONTEST_NOTICE_RES* pOut = (DL_O_HALL_CONTEST_NOTICE_RES*)pResultData;

	if (pOut->iUserNum > 0)
	{
		for (int i=0;i<pOut->iUserNum;i++)
		{
			LPUSER pUser = _pLogonManage->GetUserByID(pOut->pUserID[i]);
			if (pUser)
			{
				_pLogonManage->m_TCPSocket.SendData(pUser->iSocketIndex, &pOut->_result, sizeof(pOut->_result), MDM_GP_CONTEST, ASS_GP_CONTEST_NOTICE, 0, 0);
			}
		}
		SafeDeleteArray(pOut->pUserID);
	}
	return true;
}

bool CContestServer4Z::notifyContestLogonResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_CONTEST_NOTICE_LOGON_RES* pOut = (DL_O_HALL_CONTEST_NOTICE_LOGON_RES*)pResultData;

	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pOut->_result, sizeof(pOut->_result), MDM_GP_CONTEST, ASS_GP_CONTEST_NOTICE, 0, 0);
	return true;
}

bool CContestServer4Z::getContestAwardResponse(DataBaseResultLine* pResultData)
{
	DL_GP_O_ContestAward* pOut = (DL_GP_O_ContestAward*)pResultData;

	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pOut->_data, sizeof(pOut->_data), MDM_GP_CONTEST, ASS_GP_CONTEST_AWARDINFO, 0, 0);
	return true;
}