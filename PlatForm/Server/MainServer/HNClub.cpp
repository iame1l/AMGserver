/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "StdAfx.h"
#include "HNClub.h"
#include "GameLogonManage.h"
#include "commonuse.h"

CHNClub::CHNClub(CGameLogonManage* pLogonManage)
	:_pLogonManage(pLogonManage)
{
	
}

CHNClub::~CHNClub(void)
{

}


bool CHNClub::OnNetMessage(NetMessageHead* pNetHead,void* pData,UINT uSize,ULONG uAccessIP,UINT uIndex,DWORD dwHandleID)
{
	if (!_pLogonManage) 
	{
		return true;
	}

	if (MDM_GP_CLUB != pNetHead->bMainID)
	{
		return true;
	}

	switch(pNetHead->bAssistantID)
	{
	case ASS_GP_CREATE_CLUB:
		{
			return createClubRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_DISSMISS_CLUB:
		{
			return dissmissClubRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_JOIN_CLUB:
		{
			return joinClubRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_CLUB_USERLIST:
		{
			return getUserListRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_CLUB_TALK:
		{
			return talkInClubRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_CLUB_ROOMLIST:
		{
			return getRoomListRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_CLUB_CREATEROOM:
		{
			return buyDeskInClubRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_CLUB_CHANGENAME:
		{
			return changeNameRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_CLUB_KICKUSER:
		{
			return kickUserRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_CLUB_LIST:
		{
			return getClubListRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_REVIEW_LIST:
		{
			return getReviewListRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_MASTER_OPTION:
		{
			return masterOptionRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_CLUB_NOTICE:
		{
			return setClubNoticeRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_ENTER_CLUB:
		{
			return enterClubRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_LEAVE_CLUB:
		{
			return leaveClubRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_GET_BUYDESKRECORD:
		{
			return getBuyDeskRecordRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	default:
		break;
	}

	return true ;
}

bool CHNClub::OnDataBaseResult(DataBaseResultLine* pResultData)
{
	if (!_pLogonManage) 
	{
		return true;
	}

	switch(pResultData->uHandleKind)
	{
	case DTK_GP_CREATE_CLUB:
		{
			createClubResponse(pResultData);
		}break;	
	case DTK_GP_DISSMISS_CLUB:
		{
			dissmissClubResponse(pResultData);
		}break;	
	case DTK_GP_JOIN_CLUB:
		{
			joinClubResponse(pResultData);
		}break;	
	case DTK_GP_CLUB_USERLIST:
		{
			getUserListResponse(pResultData);
		}break;	
	case DTK_GP_CLUB_ROOMLIST:
		{
			getRoomListResponse(pResultData);
		}break;	
	case DTK_GP_CLUB_CREATEROOM:
		{
			buyDeskInClubResponse(pResultData);
		}break;	
	case DTK_GP_CLUB_CHANGENAME:
		{
			changeNameResponse(pResultData);
		}break;	
	case DTK_GP_CLUB_KICKUSER:
		{
			kickUserResponse(pResultData);
		}break;	
	case DTK_GP_CLUB_STATISTICS:
		{
			//createClubResponse(pResultData);
		}break;	
	case DTK_GP_CLUB_LIST:
		{
			getClubListResponse(pResultData);
		}break;	
	case DTK_GP_REVIEW_LIST:
		{
			getReviewListResponse(pResultData);
		}break;	
	case DTK_GP_MASTER_OPTION:
		{
			masterOptionResponse(pResultData);
		}break;	
	case DTK_GP_CLUB_NOTICE:
		{
			setClubNoticeResponse(pResultData);
		}break;	
	case DTK_GP_ENTER_CLUB:
		{
			enterClubResponse(pResultData);
		}break;	
	case DTK_GP_LEAVE_CLUB:
		{
			leaveClubResponse(pResultData);
		}break;
	case DTK_GP_GET_DESKRECORD:
		{
			getBuyDeskRecordResponse(pResultData);
		}break;
	default:
		return true;
	}

	return true;
}

	
bool CHNClub::createClubRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_CreateClub))
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_CLUB,ASS_GP_CREATE_CLUB, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_CreateClub DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_I_CreateClub));
	DL_Data.iUserID = pUser->UserID;

	string str_temp1(DL_Data._data.szClubName);
	bool bLimite = _pLogonManage->m_HNFilter.censor(str_temp1);
	if (bLimite)
	{
		MSG_GP_O_CreateClub data;
		memset(&data,0,sizeof(MSG_GP_O_CreateClub));
		_pLogonManage->m_TCPSocket.SendData(uIndex, &data,sizeof(MSG_GP_O_CreateClub),MDM_GP_CLUB,ASS_GP_CREATE_CLUB,ERR_GP_NAME_LIMITE,0);
		return true;
	}

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_CREATE_CLUB,uIndex,dwHandleID);//提交数据库


	return true;
}
	
bool CHNClub::dissmissClubRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_DissmissClub))
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_CLUB,ASS_GP_DISSMISS_CLUB, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_DissmissClub DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_I_DissmissClub));
	DL_Data.iUserID = pUser->UserID;

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_DISSMISS_CLUB,uIndex,dwHandleID);//提交数据库

	return true;
}

bool CHNClub::joinClubRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)	
{
	if (uSize != sizeof(MSG_GP_I_JoinClub))
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_CLUB,ASS_GP_JOIN_CLUB, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_JoinClub DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_I_JoinClub));
	DL_Data.iUserID = pUser->UserID;
	 
	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_JOIN_CLUB,uIndex,dwHandleID);//提交数据库

	return true;
}

bool CHNClub::getUserListRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_Club_UserList))
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_CLUB,ASS_GP_CLUB_TALK, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_Club_UserList DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_I_Club_UserList));
	DL_Data.iUserID = pUser->UserID;

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_CLUB_USERLIST,uIndex,dwHandleID);//提交数据库

	return true;
}

bool CHNClub::talkInClubRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_Club_Talk))
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_CLUB,ASS_GP_CLUB_TALK, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	MSG_GP_I_Club_Talk	*p = (MSG_GP_I_Club_Talk*)pData;
	MSG_GP_O_Club_Talk _data;
	_data.iClubID = p->iClubID;
	_data.iUserID = pUser->UserID;
	strcpy_s(_data.szTalk,p->szTalk);

	string str_temp(_data.szTalk);
	_pLogonManage->m_HNFilter.censor(str_temp,false);
	strcpy(_data.szTalk,str_temp.c_str());

	_pLogonManage->m_TCPSocket.SendDataUnion(p->iClubID,&_data,sizeof(MSG_GP_O_Club_Talk),MDM_GP_CLUB,ASS_GP_CLUB_TALK,0);
	
	return true;
}

bool CHNClub::getRoomListRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_Club_RoomList))
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_CLUB,ASS_GP_CLUB_TALK, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_Club_RoomList DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_I_Club_RoomList));
	DL_Data.iUserID = pUser->UserID;

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_CLUB_ROOMLIST,uIndex,dwHandleID);//提交数据库


	return true;
}

bool CHNClub::buyDeskInClubRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_Club_BuyDesk))
	{
		return false;
	}

	DL_I_HALL_Club_BuyDesk DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_I_Club_BuyDesk));

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_CLUB_CREATEROOM,uIndex,dwHandleID);//提交数据库


	return true;
}

bool CHNClub::changeNameRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_ChangeName))
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_CLUB,ASS_GP_CLUB_CHANGENAME, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_ChangeName DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_I_ChangeName));
	DL_Data.iUserID = pUser->UserID;

	string str_temp1(DL_Data._data.szNewClubName);
	bool bLimite = _pLogonManage->m_HNFilter.censor(str_temp1);
	if (bLimite)
	{
		MSG_GP_O_CreateClub data;
		memset(&data,0,sizeof(MSG_GP_O_CreateClub));
		_pLogonManage->m_TCPSocket.SendData(uIndex, &data,sizeof(MSG_GP_O_CreateClub),MDM_GP_CLUB,ASS_GP_CREATE_CLUB,ERR_GP_NAME_LIMITE,0);
		return true;
	}

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_CLUB_CHANGENAME,uIndex,dwHandleID);//提交数据库

	return true;
}

bool CHNClub::kickUserRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_Club_KickUser))
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_CLUB,ASS_GP_CLUB_KICKUSER, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_Club_KickUser DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_I_Club_KickUser));
	DL_Data.iUserID = pUser->UserID;

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_CLUB_KICKUSER,uIndex,dwHandleID);//提交数据库

	return true;
}

bool CHNClub::getClubListRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_CLUB,ASS_GP_CLUB_LIST, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_Club_List DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	DL_Data.iUserID = pUser->UserID;

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_CLUB_LIST,uIndex,dwHandleID);//提交数据库


	return true;
}

bool CHNClub::getReviewListRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_Club_ReviewList))
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_CLUB,ASS_GP_REVIEW_LIST, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_Club_ReviewList DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_I_Club_ReviewList));
	DL_Data.iUserID = pUser->UserID;

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_REVIEW_LIST,uIndex,dwHandleID);//提交数据库

	return true;
}

bool CHNClub::masterOptionRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_Club_MasterOpt))
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_CLUB,ASS_GP_MASTER_OPTION, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_Club_MasterOpt DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_I_Club_MasterOpt));
	DL_Data.iUserID = pUser->UserID;

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_MASTER_OPTION,uIndex,dwHandleID);//提交数据库


	return true;
}

bool CHNClub::setClubNoticeRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_Club_Notice))
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_CLUB,ASS_GP_CLUB_NOTICE, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_Club_Notice DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_I_Club_Notice));
	DL_Data.iUserID = pUser->UserID;

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_CLUB_NOTICE,uIndex,dwHandleID);//提交数据库

	return true;
}

bool CHNClub::enterClubRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_Club_EnterClub))
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_CLUB,ASS_GP_ENTER_CLUB, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_Club_EnterClub DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_I_Club_EnterClub));
	DL_Data.iUserID = pUser->UserID;

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_ENTER_CLUB,uIndex,dwHandleID);//提交数据库

	return true;
}

bool CHNClub::leaveClubRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_LeaveClub))
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_CLUB,ASS_GP_LEAVE_CLUB, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_Club_LeaveClub DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_I_LeaveClub));
	DL_Data.iUserID = pUser->UserID;

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_LEAVE_CLUB,uIndex,dwHandleID);//提交数据库

	return true;
}

bool CHNClub::getBuyDeskRecordRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_Club_BuyDeskRecord))
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_CLUB,ASS_GP_GET_BUYDESKRECORD, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_Club_GetRecord DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_I_Club_BuyDeskRecord));
	DL_Data.iUserID = pUser->UserID;

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_GET_DESKRECORD,uIndex,dwHandleID);//提交数据库

	return true;
}

bool CHNClub::createClubResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_CreateClub* pData = (DL_O_HALL_CreateClub*)pResultData;
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pData->_data, sizeof(pData->_data), MDM_GP_CLUB,ASS_GP_CREATE_CLUB,pResultData->uHandleRusult,pResultData->dwHandleID);

	if (ERR_GP_CLUB_REQUEST_SUCCESS == pResultData->uHandleRusult)
	{
		_pLogonManage->m_TCPSocket.UnionAddUser(pData->_data.iClubID,pResultData->uIndex);
	}

	return true;
}


bool CHNClub::dissmissClubResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_DissmissClub* pData = (DL_O_HALL_DissmissClub*)pResultData;
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pData->_data, sizeof(pData->_data), MDM_GP_CLUB,ASS_GP_DISSMISS_CLUB,pResultData->uHandleRusult,pResultData->dwHandleID);

	if (pResultData->uHandleRusult == ERR_GP_CLUB_REQUEST_SUCCESS)
	{
		//通知群组成员解散消息
		_pLogonManage->m_TCPSocket.SendDataBatch(&pData->_data, sizeof(pData->_data),MDM_GP_CLUB,ASS_GP_DISSMISS_CLUB_NOTIFY,0);
		//删除群组成员登陆记录
		_pLogonManage->m_TCPSocket.UnionRemoveAll(pData->_data.iClubID);

		if (pData->iDissmissDeskCount > 0)	//解散房间
		{
			for (int i=0;i<pData->iDissmissDeskCount;i++)
			{
				_pLogonManage->m_pGameManageModule->m_LogonManage.SendData(pData->_NoticeData[i].iRoomID,&pData->_NoticeData[i],sizeof(MSG_MG_R_ClubDissmissDesk),MDM_MG_CLUB,ASS_MG_CLUB_DISSMISSDESK,0,0);
			}
			
			SafeDeleteArray(pData->_NoticeData);
		}
	}

	return true;
}

bool CHNClub::joinClubResponse(DataBaseResultLine* pResultData)
{
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex,MDM_GP_CLUB,ASS_GP_JOIN_CLUB,pResultData->uHandleRusult,pResultData->dwHandleID);

	if (ERR_GP_CLUB_REQUEST_SUCCESS == pResultData->uHandleRusult)	//通知创建者申请消息
	{
		DL_O_HALL_JoinClub *pHandleResult= (DL_O_HALL_JoinClub*)pResultData;

		int iSocketID = _pLogonManage->GetIndexByID(pHandleResult->iMasterID);

		if (iSocketID == -1)
		{
			return true;
		}
		_pLogonManage->m_TCPSocket.SendData(iSocketID, &pHandleResult->_UserDate, sizeof(pHandleResult->_UserDate), MDM_GP_CLUB,ASS_GP_JOIN_CLUB_TOMASTER,pResultData->uHandleRusult,0);
	}
	

	return true;
}

bool CHNClub::getUserListResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_Club_UserList* pHandleResult= (DL_O_HALL_Club_UserList*)pResultData;
	if(!pHandleResult)
	{
		return false;
	}

	char bBuffer[MAX_SEND_SIZE];
	memset(bBuffer,0,sizeof(bBuffer));
	int iMax = (MAX_SEND_SIZE-24-sizeof(MSG_GP_O_Club_UserList_Head))/sizeof(MSG_GP_O_Club_UserList_Data);
	int SendCount = 0;
	int SendTotalcount = 0;

	while(SendCount*iMax < pHandleResult->_data.iUserNum)
	{
		int PreSend = (pHandleResult->_data.iUserNum-SendCount*iMax)>iMax?iMax:(pHandleResult->_data.iUserNum-SendCount*iMax);
		memcpy_s(bBuffer,sizeof(MSG_GP_O_Club_UserList_Head),&pHandleResult->_data,sizeof(MSG_GP_O_Club_UserList_Head));
		for (int i = 0;i<PreSend;i++)
		{
			memcpy_s(bBuffer+sizeof(MSG_GP_O_Club_UserList_Head)+i*sizeof(MSG_GP_O_Club_UserList_Data),sizeof(MSG_GP_O_Club_UserList_Data),&pHandleResult->_UserData[i+SendCount*iMax],sizeof(MSG_GP_O_Club_UserList_Data));
		}
		_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, bBuffer, sizeof(MSG_GP_O_Club_UserList_Head)+sizeof(MSG_GP_O_Club_UserList_Data)*PreSend,MDM_GP_CLUB,ASS_GP_CLUB_USERLIST, ERR_GP_CLUB_REQUEST_SUCCESS, 0);
		SendCount++;
	}
	memcpy_s(bBuffer,sizeof(MSG_GP_O_Club_UserList_Head),&pHandleResult->_data,sizeof(MSG_GP_O_Club_UserList_Head));
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, bBuffer, sizeof(MSG_GP_O_Club_UserList_Head),MDM_GP_CLUB,ASS_GP_CLUB_USERLIST, ERR_GP_CLUB_REQUEST_SUCCESS, 0);

	SafeDeleteArray(pHandleResult->_UserData);

	return true;
}

bool CHNClub::getRoomListResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_Club_RoomList* pHandleResult= (DL_O_HALL_Club_RoomList*)pResultData;
	if(!pHandleResult)
	{
		return false;
	}

	char bBuffer[MAX_SEND_SIZE];
	memset(bBuffer,0,sizeof(bBuffer));
	int iMax = (MAX_SEND_SIZE-24-sizeof(MSG_GP_O_Club_RoomList_Head))/sizeof(MSG_GP_O_Club_RoomList_Data);
	int SendCount = 0;
	int SendTotalcount = 0;

	while(SendCount*iMax < pHandleResult->_data.iRoomNum)
	{
		int PreSend = (pHandleResult->_data.iRoomNum-SendCount*iMax)>iMax?iMax:(pHandleResult->_data.iRoomNum-SendCount*iMax);
		memcpy_s(bBuffer,sizeof(MSG_GP_O_Club_RoomList_Head),&pHandleResult->_data,sizeof(MSG_GP_O_Club_RoomList_Head));
		for (int i = 0;i<PreSend;i++)
		{
			memcpy_s(bBuffer+sizeof(MSG_GP_O_Club_RoomList_Head)+i*sizeof(MSG_GP_O_Club_RoomList_Data),sizeof(MSG_GP_O_Club_RoomList_Data),&pHandleResult->_RoomData[i+SendCount*iMax],sizeof(MSG_GP_O_Club_RoomList_Data));
		}
		_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, bBuffer, sizeof(MSG_GP_O_Club_RoomList_Head)+sizeof(MSG_GP_O_Club_RoomList_Data)*PreSend,MDM_GP_CLUB,ASS_GP_CLUB_ROOMLIST, ERR_GP_CLUB_REQUEST_SUCCESS, 0);
		SendCount++;
	}
	memcpy_s(bBuffer,sizeof(MSG_GP_O_Club_RoomList_Head),&pHandleResult->_data,sizeof(MSG_GP_O_Club_RoomList_Head));
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, bBuffer, sizeof(MSG_GP_O_Club_RoomList_Head),MDM_GP_CLUB,ASS_GP_CLUB_ROOMLIST, ERR_GP_CLUB_REQUEST_SUCCESS, 0);

	SafeDeleteArray(pHandleResult->_RoomData);
	return true;
}

bool CHNClub::buyDeskInClubResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_Club_BuyDesk* pHandleResult= (DL_O_HALL_Club_BuyDesk*)pResultData;
	if(!pHandleResult)
	{
		return false;
	}

	if (ERR_GP_CLUB_REQUEST_SUCCESS == pResultData->uHandleRusult)   //创建房间成功先通知Gserver
	{
		MSG_MG_S_CLUB_BUY_DESK_NOTICE outdata;
		outdata.iDeskID = pHandleResult->iDeskID;
		outdata.iRoomID = pHandleResult->iRoomID;
		outdata.iJewels = pHandleResult->iJewels;
		outdata.iGameID = pHandleResult->iGameID;
		memcpy_s(&outdata._data,sizeof(MSG_GP_O_Club_BuyDesk),&pHandleResult->_data,sizeof(MSG_GP_O_Club_BuyDesk));
		
		_pLogonManage->m_pGameManageModule->m_LogonManage.SendData(pHandleResult->iRoomID,&outdata,sizeof(MSG_MG_S_CLUB_BUY_DESK_NOTICE),MDM_MG_CLUB,ASS_MG_CLUB_BUYDESK,0,0);
	}
	else
	{
		_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pHandleResult->_data, sizeof(pHandleResult->_data),MDM_GP_CLUB,ASS_GP_CLUB_CREATEROOM, pResultData->uHandleRusult, pResultData->dwHandleID);
	}

	return true;
}

bool CHNClub::changeNameResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_ChangeName* pData = (DL_O_HALL_ChangeName*)pResultData;
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pData->_data, sizeof(pData->_data), MDM_GP_CLUB,ASS_GP_CLUB_CHANGENAME,pResultData->uHandleRusult,pResultData->dwHandleID);

	if (ERR_GP_CLUB_REQUEST_SUCCESS == pResultData->uHandleRusult)
	{
		_pLogonManage->m_TCPSocket.SendDataUnion(pData->_data.iClubID,&pData->_data, sizeof(pData->_data),MDM_GP_CLUB,ASS_GP_CLUB_NAME_UPDATE,0);
	}

	return true;
}

bool CHNClub::kickUserResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_Club_KickUser* pData = (DL_O_HALL_Club_KickUser*)pResultData;

	if (0 == pResultData->uHandleRusult || 4 == pResultData->uHandleRusult)
	{ 
		_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pData->_data, sizeof(pData->_data), MDM_GP_CLUB,ASS_GP_CLUB_KICKUSER,ERR_GP_CLUB_REQUEST_SUCCESS,pResultData->dwHandleID);

		int iSocketID = _pLogonManage->GetIndexByID(pData->_data.iTargetID);

		if (iSocketID != -1)
		{
			_pLogonManage->m_TCPSocket.SendData(iSocketID, &pData->_data, sizeof(pData->_data), MDM_GP_CLUB,ASS_GP_CLUB_KICKUSER_TAR,ERR_GP_CLUB_REQUEST_SUCCESS,0);
			_pLogonManage->m_TCPSocket.UnionRemoveUser(pData->_data.iClubID,iSocketID);
		}
		
		_pLogonManage->m_TCPSocket.SendDataUnion(pData->_data.iClubID,&pData->_TarData, sizeof(pData->_TarData),MDM_GP_CLUB,ASS_GP_CLUB_USERCHANGE,0);

		if (pData->iDissmissDeskCount > 0)	//解散房间
		{
			for (int i=0;i<pData->iDissmissDeskCount;i++)
			{
				_pLogonManage->m_pGameManageModule->m_LogonManage.SendData(pData->_NoticeData[i].iRoomID,&pData->_NoticeData[i],sizeof(MSG_MG_R_ClubDissmissDesk),MDM_MG_CLUB,ASS_MG_CLUB_DISSMISSDESK,0,0);
			}

			SafeDeleteArray(pData->_NoticeData);
		}
	}
	else
	{
		_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pData->_data, sizeof(pData->_data), MDM_GP_CLUB,ASS_GP_CLUB_KICKUSER,pResultData->uHandleRusult,pResultData->dwHandleID);
	}

	return true;
}

bool CHNClub::getClubListResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_Club_List* pHandleResult= (DL_O_HALL_Club_List*)pResultData;
	if(!pHandleResult)
	{
		return false;
	}

	char bBuffer[MAX_SEND_SIZE];
	memset(bBuffer,0,sizeof(bBuffer));
	int iMax = (MAX_SEND_SIZE-24)/sizeof(MSG_GP_O_Club_List);
	int SendCount = 0;
	int SendTotalcount = 0;

	while(SendCount*iMax < pHandleResult->iCount)
	{
		int PreSend = (pHandleResult->iCount-SendCount*iMax)>iMax?iMax:(pHandleResult->iCount-SendCount*iMax);
		for (int i = 0;i<PreSend;i++)
		{
			memcpy_s(bBuffer+i*sizeof(MSG_GP_O_Club_List),sizeof(MSG_GP_O_Club_List),&pHandleResult->_data[i+SendCount*iMax],sizeof(MSG_GP_O_Club_List));
			_pLogonManage->m_TCPSocket.UnionAddUser(pHandleResult->_data[i+SendCount*iMax].iClubID,pResultData->uIndex);
		}
		_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, bBuffer, sizeof(MSG_GP_O_Club_List)*PreSend,MDM_GP_CLUB,ASS_GP_CLUB_LIST, ERR_GP_CLUB_REQUEST_SUCCESS, 0);
		SendCount++;
	}
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, MDM_GP_CLUB,ASS_GP_CLUB_LIST, ERR_GP_CLUB_REQUEST_SUCCESS, 0);

	SafeDeleteArray(pHandleResult->_data);

	return true;
}

bool CHNClub::getReviewListResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_Club_ReviewList* pHandleResult= (DL_O_HALL_Club_ReviewList*)pResultData;
	if(!pHandleResult)
	{
		return false;
	}

	char bBuffer[MAX_SEND_SIZE];
	memset(bBuffer,0,sizeof(bBuffer));
	int iMax = (MAX_SEND_SIZE-24-sizeof(MSG_GP_O_Club_ReviewList_Head))/sizeof(MSG_GP_O_Club_ReviewList_Data);
	int SendCount = 0;
	int SendTotalcount = 0;

	while(SendCount*iMax < pHandleResult->_HeadData.iUserNum)
	{
		int PreSend = (pHandleResult->_HeadData.iUserNum-SendCount*iMax)>iMax?iMax:(pHandleResult->_HeadData.iUserNum-SendCount*iMax);
		memcpy_s(bBuffer,sizeof(MSG_GP_O_Club_ReviewList_Head),&pHandleResult->_HeadData,sizeof(MSG_GP_O_Club_ReviewList_Head));
		for (int i = 0;i<PreSend;i++)
		{
			memcpy_s(bBuffer+sizeof(MSG_GP_O_Club_ReviewList_Head)+i*sizeof(MSG_GP_O_Club_ReviewList_Data),sizeof(MSG_GP_O_Club_ReviewList_Data),&pHandleResult->_Data[i+SendCount*iMax],sizeof(MSG_GP_O_Club_ReviewList_Data));
		}
		_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, bBuffer, sizeof(MSG_GP_O_Club_ReviewList_Head)+sizeof(MSG_GP_O_Club_ReviewList_Data)*PreSend,MDM_GP_CLUB,ASS_GP_REVIEW_LIST, ERR_GP_CLUB_REQUEST_SUCCESS, 0);
		SendCount++;
	}
	memcpy_s(bBuffer,sizeof(MSG_GP_O_Club_ReviewList_Head),&pHandleResult->_HeadData,sizeof(MSG_GP_O_Club_ReviewList_Head));
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, bBuffer, sizeof(MSG_GP_O_Club_ReviewList_Head),MDM_GP_CLUB,ASS_GP_REVIEW_LIST, ERR_GP_CLUB_REQUEST_SUCCESS, 0);

	SafeDeleteArray(pHandleResult->_Data);
	return true;
}

bool CHNClub::masterOptionResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_Club_MasterOpt* pData = (DL_O_HALL_Club_MasterOpt*)pResultData;
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pData->_Data, sizeof(pData->_Data), MDM_GP_CLUB,ASS_GP_MASTER_OPTION,pResultData->uHandleRusult,pResultData->dwHandleID);

	if (ERR_GP_CLUB_REQUEST_SUCCESS == pResultData->uHandleRusult)
	{
		int iSocketID = _pLogonManage->GetIndexByID(pData->_Data.iTargetID);

		if (pData->_Data.bOptType == 0)		//同意
		{
			if (iSocketID != -1)
			{
				_pLogonManage->m_TCPSocket.SendData(iSocketID, &pData->_ClubData, sizeof(pData->_ClubData), MDM_GP_CLUB,ASS_GP_CLUB_JOINRESULT,0,0);
				_pLogonManage->m_TCPSocket.UnionAddUser(pData->_Data.iClubID,iSocketID);
			}

			_pLogonManage->m_TCPSocket.SendDataUnion(pData->_Data.iClubID,&pData->_TarData, sizeof(pData->_TarData),MDM_GP_CLUB,ASS_GP_CLUB_USERCHANGE,0);
		}
		else if (pData->_Data.bOptType == 1 || pData->_Data.bOptType == 2)			//拒绝
		{
			if (iSocketID != -1)
			{
				_pLogonManage->m_TCPSocket.SendData(iSocketID, &pData->_ClubData, sizeof(pData->_ClubData), MDM_GP_CLUB,ASS_GP_CLUB_JOINRESULT,1,0);
			}
		}
		

	}


	return true;
}

bool CHNClub::setClubNoticeResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_Club_Notice* pData = (DL_O_HALL_Club_Notice*)pResultData;
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pData->_data, sizeof(pData->_data), MDM_GP_CLUB,ASS_GP_CLUB_NOTICE,pResultData->uHandleRusult,pResultData->dwHandleID);

	if (ERR_GP_CLUB_REQUEST_SUCCESS == pResultData->uHandleRusult)
	{
		_pLogonManage->m_TCPSocket.SendDataUnion(pData->_data.iClubID,&pData->_data, sizeof(pData->_data),MDM_GP_CLUB,ASS_GP_CLUB_NOTICE_UPDATE,0);
	}
	return true;
}

bool CHNClub::enterClubResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_Club_EnterClub* pData = (DL_O_HALL_Club_EnterClub*)pResultData;
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pData->_data, sizeof(pData->_data), MDM_GP_CLUB,ASS_GP_ENTER_CLUB,pResultData->uHandleRusult,pResultData->dwHandleID);

	/*if (ERR_GP_CLUB_REQUEST_SUCCESS == pResultData->uHandleRusult)
	{
		_pLogonManage->m_TCPSocket.UnionAddUser(pData->_data.iClubID,pResultData->uIndex);
	}*/

	return true;
}

bool CHNClub::leaveClubResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_Club_LeaveClub* pData = (DL_O_HALL_Club_LeaveClub*)pResultData;
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pData->_data, sizeof(pData->_data), MDM_GP_CLUB,ASS_GP_LEAVE_CLUB,pResultData->uHandleRusult,pResultData->dwHandleID);

	if (pResultData->uHandleRusult == ERR_GP_CLUB_REQUEST_SUCCESS)
	{
		//删除该成员连接
		_pLogonManage->m_TCPSocket.UnionRemoveUser(pData->_data.iClubID,pResultData->uIndex);
		//通知群组成员退出消息
		_pLogonManage->m_TCPSocket.SendDataUnion(pData->_data.iClubID,&pData->_UserData, sizeof(pData->_UserData),MDM_GP_CLUB,ASS_GP_CLUB_USERCHANGE,0);
		
	}

	return true;
}






bool CHNClub::OnHandleClubBuyDeskResult(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	if(uSize != sizeof(MSG_MG_S_CLUB_BUY_DESK_NOTICE))
		return false;
	MSG_MG_S_CLUB_BUY_DESK_NOTICE* pReciveData = (MSG_MG_S_CLUB_BUY_DESK_NOTICE*)pData;
	if (!pReciveData)
		return true;

	LPUSER p = _pLogonManage->GetUserByID(pReciveData->_data.iUserID);
	if (!p)
	{
		return true;
	}
	if (pReciveData->bSuccess)
	{
		_pLogonManage->m_TCPSocket.SendData(p->iSocketIndex,&pReciveData->_data,sizeof(pReciveData->_data),MDM_GP_CLUB,ASS_GP_CLUB_CREATEROOM,0,0);

		MSG_GP_O_Club_RoomChange _RoomChange;
		_RoomChange.bCreate = true;
		_RoomChange.iClub = pReciveData->_data.iClubID;
		memcpy_s(&_RoomChange._data,sizeof(MSG_GP_O_Club_RoomList_Data),&pReciveData->_data._RoomData,sizeof(MSG_GP_O_Club_RoomList_Data));
        _RoomChange._data.bAllowEnter = true;
        _RoomChange._data.bIsPlay = false;
		_pLogonManage->m_TCPSocket.SendDataUnion(pReciveData->_data.iClubID,&_RoomChange, sizeof(_RoomChange),MDM_GP_CLUB,ASS_GP_CLUB_ROOMCHANGE,0);
	}
	else  //创建失败
		_pLogonManage->m_TCPSocket.SendData(p->iSocketIndex,&pReciveData->_data,sizeof(pReciveData->_data),MDM_GP_CLUB,ASS_GP_CLUB_CREATEROOM,9,0);
	return true;
}

bool CHNClub::OnHandleClubClearDeskResult(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	if(uSize != sizeof(MSG_GP_O_Club_RoomChange))
		return false;
	MSG_GP_O_Club_RoomChange* pReciveData = (MSG_GP_O_Club_RoomChange*)pData;
	if (!pReciveData)
		return true;

	_pLogonManage->m_TCPSocket.SendDataUnion(pReciveData->iClub,pReciveData, sizeof(MSG_GP_O_Club_RoomChange),MDM_GP_CLUB,ASS_GP_CLUB_ROOMCHANGE,0);

	return true;
}

bool CHNClub::getBuyDeskRecordResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_Club_GetRecord* pHandleResult= (DL_O_HALL_Club_GetRecord*)pResultData;
	if(!pHandleResult)
	{
		return false;
	}

	char bBuffer[MAX_SEND_SIZE];
	memset(bBuffer,0,sizeof(bBuffer));
	int iMax = (MAX_SEND_SIZE-24-sizeof(MSG_GP_O_Club_BuyDeskRecord_Head))/sizeof(MSG_GP_O_Club_BuyDeskRecord_Data);
	int SendCount = 0;
	int SendTotalcount = 0;
	 
	while(SendCount*iMax < pHandleResult->_HeadData.iDataNum)
	{
		int PreSend = (pHandleResult->_HeadData.iDataNum-SendCount*iMax)>iMax?iMax:(pHandleResult->_HeadData.iDataNum-SendCount*iMax);
		memcpy_s(bBuffer,sizeof(MSG_GP_O_Club_BuyDeskRecord_Head),&pHandleResult->_HeadData,sizeof(MSG_GP_O_Club_BuyDeskRecord_Head));
		for (int i = 0;i<PreSend;i++)
		{
			memcpy_s(bBuffer+sizeof(MSG_GP_O_Club_BuyDeskRecord_Head)+i*sizeof(MSG_GP_O_Club_BuyDeskRecord_Data),sizeof(MSG_GP_O_Club_BuyDeskRecord_Data),&pHandleResult->_Data[i+SendCount*iMax],sizeof(MSG_GP_O_Club_BuyDeskRecord_Data));
		}
		_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, bBuffer, sizeof(MSG_GP_O_Club_BuyDeskRecord_Head)+sizeof(MSG_GP_O_Club_BuyDeskRecord_Data)*PreSend,MDM_GP_CLUB,ASS_GP_GET_BUYDESKRECORD, ERR_GP_CLUB_REQUEST_SUCCESS, 0);
		SendCount++;
	}
	memcpy_s(bBuffer,sizeof(MSG_GP_O_Club_BuyDeskRecord_Head),&pHandleResult->_HeadData,sizeof(MSG_GP_O_Club_BuyDeskRecord_Head));
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, bBuffer, sizeof(MSG_GP_O_Club_BuyDeskRecord_Head),MDM_GP_CLUB,ASS_GP_GET_BUYDESKRECORD, ERR_GP_CLUB_REQUEST_SUCCESS, 0);

	SafeDeleteArray(pHandleResult->_Data);

	return true;
}
