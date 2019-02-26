/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "StdAfx.h"
#include "HNMail.h"
#include "GameLogonManage.h"
#include "commonuse.h"

CHNMail::CHNMail(CGameLogonManage* pLogonManage)
	:_pLogonManage(pLogonManage)
{
	
}

CHNMail::~CHNMail(void)
{

}


bool CHNMail::OnNetMessage(NetMessageHead* pNetHead,void* pData,UINT uSize,ULONG uAccessIP,UINT uIndex,DWORD dwHandleID)
{
	if (!_pLogonManage) 
	{
		return true;
	}

	if (MDM_GP_MAIL != pNetHead->bMainID)
	{
		return true;
	}

	switch(pNetHead->bAssistantID)
	{
	case ASS_GP_MAIL_LIST:
		{
			return getMailListRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_OPEN_MAIL:
		{
			return openMailRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_GET_ATTACHMENT:
		{
			return getAttachmentRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_DEL_MAIL:
		{
			return delMailRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	case ASS_GP_GET_SYSMSG:
		{
			return getSysMsgRequest(pData,uSize,uIndex,dwHandleID);
		}break;
	default:
		break;
	}

	return true ;
}

bool CHNMail::OnDataBaseResult(DataBaseResultLine* pResultData)
{
	if (!_pLogonManage) 
	{
		return true;
	}

	switch(pResultData->uHandleKind)
	{
	case DTK_GP_MAIL_LIST:
		{
			getMailListResponse(pResultData);
		}break;	
	case DTK_GP_OPEN_MAIL:
		{
			openMailResponse(pResultData);
		}break;
	case DTK_GP_GET_ATTACHMENT:
		{
			getAttachmentResponse(pResultData);
		}break;
	case DTK_GP_DEL_MAIL:
		{
			delMailResponse(pResultData);
		}break;
	case DTK_GP_UPDATE_MAIL:
		{
			mailUpdateResponse(pResultData);
		}break;
	case DTK_GP_UPDATE_SYSTEM:
		{
			sysmsgUpdateResponse(pResultData);
		}break;
	default:
		return true;
	}

	return true;
}


bool CHNMail::getMailListRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != 0)
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_MAIL,ASS_GP_MAIL_LIST, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_Mail_List DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	DL_Data.iUserID = pUser->UserID;

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_MAIL_LIST,uIndex,dwHandleID);//提交数据库


	return true;
}


bool CHNMail::getMailListResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_Mail_List* pHandleResult= (DL_O_HALL_Mail_List*)pResultData;
	if(!pHandleResult)
	{
		return false;
	}

	char bBuffer[MAX_SEND_SIZE];
	memset(bBuffer,0,sizeof(bBuffer));
	int iMax = (MAX_SEND_SIZE-24-sizeof(MSG_GP_O_Mail_List_Head))/sizeof(MSG_GP_O_Mail_List_Data);
	int SendCount = 0;
	int SendTotalcount = 0;

	while(SendCount*iMax < pHandleResult->_HeadData.iDataNum)
	{
		int PreSend = (pHandleResult->_HeadData.iDataNum-SendCount*iMax)>iMax?iMax:(pHandleResult->_HeadData.iDataNum-SendCount*iMax);
		memcpy_s(bBuffer,sizeof(MSG_GP_O_Mail_List_Head),&pHandleResult->_HeadData,sizeof(MSG_GP_O_Mail_List_Head));
		for (int i = 0;i<PreSend;i++)
		{
			memcpy_s(bBuffer+sizeof(MSG_GP_O_Mail_List_Head)+i*sizeof(MSG_GP_O_Mail_List_Data),sizeof(MSG_GP_O_Mail_List_Data),&pHandleResult->_data[i+SendCount*iMax],sizeof(MSG_GP_O_Mail_List_Data));
		}
		_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, bBuffer, sizeof(MSG_GP_O_Mail_List_Head)+sizeof(MSG_GP_O_Mail_List_Data)*PreSend,MDM_GP_MAIL,ASS_GP_MAIL_LIST, ERR_GP_CLUB_REQUEST_SUCCESS, 0);
		SendCount++;
	}
	memcpy_s(bBuffer,sizeof(MSG_GP_O_Mail_List_Head),&pHandleResult->_HeadData,sizeof(MSG_GP_O_Mail_List_Head));
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, bBuffer, sizeof(MSG_GP_O_Mail_List_Head),MDM_GP_MAIL,ASS_GP_MAIL_LIST, ERR_GP_CLUB_REQUEST_SUCCESS, 0);

	SafeDeleteArray(pHandleResult->_data);

	return true;
}



bool CHNMail::openMailRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_Open_Mail))
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_MAIL,ASS_GP_OPEN_MAIL, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_Open_Mail DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_I_Open_Mail));
	DL_Data.iUserID = pUser->UserID;

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_OPEN_MAIL,uIndex,dwHandleID);//提交数据库


	return true;
}

bool CHNMail::openMailResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_Open_Mail* pData = (DL_O_HALL_Open_Mail*)pResultData;
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pData->_data, sizeof(pData->_data), MDM_GP_MAIL,ASS_GP_OPEN_MAIL,pResultData->uHandleRusult,pResultData->dwHandleID);

	return true;
}


bool CHNMail::getAttachmentRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_Get_Attachment))
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_MAIL,ASS_GP_GET_ATTACHMENT, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_Get_Attachment DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_I_Get_Attachment));
	DL_Data.iUserID = pUser->UserID;

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_GET_ATTACHMENT,uIndex,dwHandleID);//提交数据库


	return true;
}

bool CHNMail::getAttachmentResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_Get_Attachment* pData = (DL_O_HALL_Get_Attachment*)pResultData;
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pData->_data, sizeof(pData->_data), MDM_GP_MAIL,ASS_GP_GET_ATTACHMENT,pResultData->uHandleRusult,pResultData->dwHandleID);

	return true;
}


bool CHNMail::delMailRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_I_Delete_Mail))
	{
		return false;
	}

	LPUSER pUser = _pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		_pLogonManage->m_TCPSocket.SendData(uIndex,MDM_GP_MAIL,ASS_GP_DEL_MAIL, ERR_GP_USER_NOT_FOUND, dwHandleID);
		return true;
	}

	DL_I_HALL_Delete_Mail DL_Data;
	memset(&DL_Data, 0, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_I_Delete_Mail));
	DL_Data.iUserID = pUser->UserID;

	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseLineHead,sizeof(DL_Data),DTK_GP_DEL_MAIL,uIndex,dwHandleID);//提交数据库


	return true;
}

bool CHNMail::delMailResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_Delete_Mail* pData = (DL_O_HALL_Delete_Mail*)pResultData;
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pData->_data, sizeof(pData->_data), MDM_GP_MAIL,ASS_GP_DEL_MAIL,pResultData->uHandleRusult,pResultData->dwHandleID);

	return true;
}

bool CHNMail::mailUpdateResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_Mail_Update *pHandleResult= (DL_O_HALL_Mail_Update*)pResultData;

	for (int i=0;i<pHandleResult->iNewNumber;i++)
	{
		if (pHandleResult->_data[i].iUserID == 0)
		{
			_pLogonManage->m_TCPSocket.SendDataBatch(&pHandleResult->_data[i],sizeof(MSG_GP_O_Mail_Update),MDM_GP_MAIL,ASS_GP_NOTIFY_NEW_MAIL,0);
		}
		else
		{
			int iSocketID = 0;
			iSocketID = _pLogonManage->GetIndexByID(pHandleResult->_data[i].iUserID);

			if (iSocketID == -1)
			{
				continue;
			}

			_pLogonManage->m_TCPSocket.SendData(iSocketID,&pHandleResult->_data[i],sizeof(MSG_GP_O_Mail_Update),MDM_GP_MAIL,ASS_GP_NOTIFY_NEW_MAIL,0,0);
		}
	}

	SafeDeleteArray(pHandleResult->_data);

	return true;
}


bool CHNMail::sysmsgUpdateResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_SysMsg *pHandleResult= (DL_O_HALL_SysMsg*)pResultData;

	if (pHandleResult->bBroadcast)
		_pLogonManage->m_TCPSocket.SendDataBatch(&pHandleResult->_data, sizeof(MSG_GP_O_SysMsg), MDM_GP_MAIL, ASS_GP_GET_SYSMSG, 0);
	else
		_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pHandleResult->_data, sizeof(MSG_GP_O_SysMsg), MDM_GP_MAIL, ASS_GP_GET_SYSMSG,pResultData->uHandleRusult,pResultData->dwHandleID);


	//HNLOG_M("DTK_GP_UPDATE_SYSTEM %d %s",pHandleResult->bBroadcast,pHandleResult->_data.szSysMessage);

	return true;
}

bool CHNMail::getSysMsgRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != 0)
	{
		return false;
	}


	DataBaseLineHead  DataBaseLineHead;
	memset(&DataBaseLineHead, 0, sizeof(DataBaseLineHead));
	
	_pLogonManage->m_SQLDataManage.PushLine(&DataBaseLineHead,sizeof(DataBaseLineHead),DTK_GP_UPDATE_SYSTEM,uIndex,dwHandleID);//提交数据库

	return true;
}
