#include "StdAfx.h"
#include "HNCreateRoom.h"
#include "GameLogonManage.h"
#include <io.h>

CCreateRoom::CCreateRoom(CWnd *pWnd)
{
	m_pLogonManage = (CGameLogonManage*)pWnd;
}

CCreateRoom::~CCreateRoom(void)
{

}

bool CCreateRoom::OnNetMessage(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	if (!m_pLogonManage) 
	{
		return true;
	}

	if (pNetHead->bMainID != MDM_GP_DESK_VIP)
	{
		return true;
	}

	switch(pNetHead->bAssistantID)
	{
	case ASS_GP_BUY_DESK:
		{
			return OnBuyDeskRequest(pData,uSize,uIndex,dwHandleID);
		}
		break;
	case ASS_GP_ENTER_DESK:
		{
			return OnEnterDeskRequest(pData,uSize,uIndex,dwHandleID);
		}
		break;
	case ASS_GP_GETTOTALRECORD:
		{
			return OnGetTotalRecordRequest(pData,uSize,uIndex,dwHandleID);
		}
		break;
	case ASS_GP_GETSINGLERECORD:
		{
			return OnGetSingelRecordRequest(pData,uSize,uIndex,dwHandleID);
		}
		break;
	case ASS_GP_GET_CUTROOM:
		{
			return OnGetCutRoomRequest(pData,uSize,uIndex,dwHandleID);
		}
		break;
	case ASS_GP_GET_DESKCONFIG:
		{
			return OnGetBuyDeskConfigRequest(pData,uSize,uIndex,dwHandleID);
		}
		break;
	case ASS_GP_GET_RECORDURL:
		{
			return OnGetRecordURLRequest(pData,uSize,uIndex,dwHandleID);
		}
	case ASS_GP_GET_DESKLIST:
		{
			return OnGetDeskListRequest(pData,uSize,uIndex,dwHandleID);
		}
	case ASS_GP_GET_DESKUSER:
		{
			return OnGetDeskUserRequest(pData,uSize,uIndex,dwHandleID);
		}
	case ASS_GP_DISSMISSDESK:
		{
			return OnDissmissDeskRequest(pData,uSize,uIndex,dwHandleID);
		}
	case ASS_GP_DELETE_RECORD:
		{
			return OnDeleteRecordRequest(pData,uSize,uIndex,dwHandleID);
		}
		break;
	default:
		break;
	}

	return true;
}

bool CCreateRoom::OnDataBaseResult(DataBaseResultLine * pResultData)
{
	if (!m_pLogonManage) 
	{
		return true;
	}

	switch(pResultData->uHandleKind)
	{
	case DTK_GP_BUY_DESK:
		{
			return OnBuyDeskResponse(pResultData);
		}
		break;
	case DTK_GP_ENTER_VIPDESK:
		{
			return OnEnterDeskResponse(pResultData);
		}
		break;
	case DTK_GP_TOTALRECORD:
		{
			return OnGetTotalRecordResponse(pResultData);
		}
		break;
	case DTK_GP_SINGLERECORD:
		{
			return OnGetSingelRecordResponse(pResultData);
		}
		break;
	case DTK_GP_GET_CUTROOM:
		{
			return OnGetCutRoomResponse(pResultData);
		}
		break;
	case DTK_GP_GET_BUYDESKCONFIG:
		{
			return OnGetBuyDeskConfigResponse(pResultData);
		}
		break;
	case DTK_GP_GET_RECORDURL:
		{
			return OnGetRecordURLResponse(pResultData);
		}
		break;
	case DTK_GP_GET_DESKLIST:
		{
			return OnGetDeskListResponse(pResultData);
		}
		break;
	case DTK_GP_GET_DESKUSER:
		{
			return OnGetDeskUserResponse(pResultData);
		}
		break;
	case DTK_GP_DISSMISSDESK:
		{
			return OnDissmissDeskResponse(pResultData);
		}
		break;
	case DTK_GP_DELETERECORD:
		{
			return OnDeleteRecordResponse(pResultData);
		}
		break;
    case DTK_GP_DISSMISSDESKBYMS:
        {
            return OnDissmissDeskByMS(pResultData);
        }break;
	default:
		break;
	}
	return true;
}

bool CCreateRoom::OnBuyDeskRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GP_S_BUY_DESK *pReceiveData=(MSG_GP_S_BUY_DESK *)pData;
	if(!pReceiveData)
	{
		return false;
	}
	DL_I_HALL_BUY_DESK inData;
	ZeroMemory(&inData, sizeof(inData));
	memcpy(&inData._data, pData, sizeof(MSG_GP_S_BUY_DESK));
	m_pLogonManage->m_SQLDataManage.PushLine(&inData.DataBaseLineHead, sizeof(inData),DTK_GP_BUY_DESK, uIndex, dwHandleID);
}

bool CCreateRoom::OnEnterDeskRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GP_S_ENTER_VIPDESK *pReceiveData=(MSG_GP_S_ENTER_VIPDESK *)pData;
	if(!pReceiveData)
	{
		return false;
	}

	DL_I_HALL_ENTER_VIPDESK inData;
	ZeroMemory(&inData, sizeof(inData));
	memcpy(&inData._data, pData, sizeof(MSG_GP_S_ENTER_VIPDESK));
	m_pLogonManage->m_SQLDataManage.PushLine(&inData.DataBaseLineHead, sizeof(inData),DTK_GP_ENTER_VIPDESK, uIndex, dwHandleID);
}

bool CCreateRoom::OnGetTotalRecordRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GP_I_GetTotalRecord *pReceiveData=(MSG_GP_I_GetTotalRecord *)pData;
	if(!pReceiveData)
	{
		return false;
	}
	DL_I_HALL_TOTALRECORD inData;
	ZeroMemory(&inData, sizeof(inData));
	memcpy(&inData._data, pData, sizeof(MSG_GP_I_GetTotalRecord));
	m_pLogonManage->m_SQLDataManage.PushLine(&inData.DataBaseLineHead, sizeof(inData),DTK_GP_TOTALRECORD, uIndex, dwHandleID);
}

bool CCreateRoom::OnGetSingelRecordRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GP_I_GetSingleRecord *pReceiveData=(MSG_GP_I_GetSingleRecord *)pData;
	if(!pReceiveData)
	{
		return false;
	}
	DL_I_HALL_SINGLERECORD inData;
	ZeroMemory(&inData, sizeof(inData));
	memcpy(&inData._data, pData, sizeof(MSG_GP_I_GetSingleRecord));
	m_pLogonManage->m_SQLDataManage.PushLine(&inData.DataBaseLineHead, sizeof(inData),DTK_GP_SINGLERECORD, uIndex, dwHandleID);
}

bool CCreateRoom::OnGetCutRoomRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GP_S_GET_CUTNETROOMINFO *pReceiveData=(MSG_GP_S_GET_CUTNETROOMINFO *)pData;
	if(!pReceiveData)
	{
		return false;
	}
	DL_I_HALL_NETCUT inData;
	ZeroMemory(&inData, sizeof(inData));
	memcpy(&inData._data, pData, sizeof(MSG_GP_S_GET_CUTNETROOMINFO));
	m_pLogonManage->m_SQLDataManage.PushLine(&inData.DataBaseLineHead, sizeof(inData),DTK_GP_GET_CUTROOM, uIndex, dwHandleID);
}

bool CCreateRoom::OnGetBuyDeskConfigRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GP_I_BuyDeskConfig *pReceiveData=(MSG_GP_I_BuyDeskConfig *)pData;
	if(!pReceiveData)
	{
		return false;
	}
	DL_I_HALL_BuyDeskConfig inData;
	memcpy(&inData._data, pData, sizeof(MSG_GP_I_BuyDeskConfig));
	m_pLogonManage->m_SQLDataManage.PushLine(&inData.DataBaseLineHead, sizeof(inData),DTK_GP_GET_BUYDESKCONFIG, uIndex, dwHandleID);
}

bool CCreateRoom::OnGetRecordURLRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GP_I_GetRecordURL *pReceiveData=(MSG_GP_I_GetRecordURL *)pData;
	if(!pReceiveData)
	{
		return false;
	}

	DL_I_HALL_GetRecordURL inData;
	memcpy(&inData._data, pData, sizeof(MSG_GP_I_GetRecordURL));
	m_pLogonManage->m_SQLDataManage.PushLine(&inData.DataBaseLineHead, sizeof(inData),DTK_GP_GET_RECORDURL, uIndex, dwHandleID);
}

bool CCreateRoom::OnGetDeskListRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GP_I_GetBuydeskList *pReceiveData=(MSG_GP_I_GetBuydeskList *)pData;
	if(!pReceiveData)
	{
		return false;
	}

	DL_I_HALL_GetDeskList inData;
	memcpy(&inData._data, pData, sizeof(MSG_GP_I_GetBuydeskList));
	m_pLogonManage->m_SQLDataManage.PushLine(&inData.DataBaseLineHead, sizeof(inData),DTK_GP_GET_DESKLIST, uIndex, dwHandleID);
}

bool CCreateRoom::OnGetDeskUserRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GP_I_GetDeskUser *pReceiveData=(MSG_GP_I_GetDeskUser *)pData;
	if(!pReceiveData)
	{
		return false;
	}

	DL_I_HALL_GetDeskUser inData;
	memcpy(&inData._data, pData, sizeof(MSG_GP_I_GetDeskUser));
	m_pLogonManage->m_SQLDataManage.PushLine(&inData.DataBaseLineHead, sizeof(inData),DTK_GP_GET_DESKUSER, uIndex, dwHandleID);

	return true;
}

bool CCreateRoom::OnDissmissDeskRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GP_I_DissmissDesk *pReceiveData=(MSG_GP_I_DissmissDesk *)pData;
	if(!pReceiveData)
	{
		return false;
	}

	//处理数据
	LPUSER pUser = m_pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		m_pLogonManage->m_TCPSocket.SendData(uIndex, MDM_GP_DESK_VIP,ASS_GP_DISSMISSDESK, ERR_GP_NOT_FIND_USERR_CLR, dwHandleID);
		return true;
	}

	DL_I_HALL_DissmissDesk inData;
	inData.iUserID = pUser->UserID;
	memcpy(&inData._data, pData, sizeof(MSG_GP_I_DissmissDesk));
	m_pLogonManage->m_SQLDataManage.PushLine(&inData.DataBaseLineHead, sizeof(inData),DTK_GP_DISSMISSDESK, uIndex, dwHandleID);

	return true;
}

bool CCreateRoom::OnDeleteRecordRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GP_I_DeleteRecord *pReceiveData=(MSG_GP_I_DeleteRecord *)pData;
	if(!pReceiveData)
	{
		return false;
	}

	LPUSER pUser = m_pLogonManage->GetUserBySockIdx(uIndex);

	if (!pUser)
	{
		m_pLogonManage->m_TCPSocket.SendData(uIndex, MDM_GP_DESK_VIP,ASS_GP_DELETE_RECORD, ERR_GP_NOT_FIND_USERR_DEL, dwHandleID);
		return true;
	}

	DL_I_HALL_DeleteRecord inData;
	inData.iUserID = pUser->UserID;
	memcpy(&inData._data, pData, sizeof(MSG_GP_I_DeleteRecord));
	m_pLogonManage->m_SQLDataManage.PushLine(&inData.DataBaseLineHead, sizeof(inData),DTK_GP_DELETERECORD, uIndex, dwHandleID);

	return true;
}

bool CCreateRoom::OnBuyDeskResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_BUY_DESK_RES* pHandleResult= (DL_O_HALL_BUY_DESK_RES*)pResultData;
	if(!pHandleResult)
	{
		return false;
	}

	if (1 == pResultData->uHandleRusult)   //创建房间成功先通知Gserver
	{
		MSG_MG_S_BUY_DESK_NOTICE outdata;
		memcpy_s(&outdata._data,sizeof(outdata._data),&pHandleResult->_result,sizeof(MSG_GP_S_BUY_DESK_RES));
		m_pLogonManage->m_pGameManageModule->m_LogonManage.SendData(pHandleResult->_result.iRoomID,&outdata,sizeof(MSG_MG_S_BUY_DESK_NOTICE),MDM_MG_CREATEROOM,ASS_MG_BUYROOM_NOTICE,0,0);
	}
	else
	{
		m_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pHandleResult->_result, sizeof(pHandleResult->_result),MDM_GP_DESK_VIP,ASS_GP_BUY_DESK, pResultData->uHandleRusult, pResultData->dwHandleID);
	}
	return true;
}

bool CCreateRoom::OnEnterDeskResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_ENTER_VIPDESK_RES* pHandleResult= (DL_O_HALL_ENTER_VIPDESK_RES*)pResultData;
	if(!pHandleResult)
	{
		return false;
	}

	if (1 == pResultData->uHandleRusult || 3 == pResultData->uHandleRusult)
	{
		ComRoomInfo *pFindRoomInfo=m_pLogonManage->m_GameList.FindRoomInfo(pHandleResult->_result.iRoomID);
		if(pFindRoomInfo)
		{
			pHandleResult->_result.tCutNetRoomInfo=*pFindRoomInfo;
		}
		else
		{
			pResultData->uHandleRusult=0;
		}
	}
	
	if (pResultData->uHandleRusult == 3)
	{
		MSG_MG_S_RETURN_DESK_NOTICE outdata;
		memcpy_s(&outdata._data,sizeof(outdata._data),&pHandleResult->_result,sizeof(MSG_GP_S_GET_CUTNETROOMINFO_RES));
		outdata.iType = 1;
		m_pLogonManage->m_pGameManageModule->m_LogonManage.SendData(pHandleResult->_result.iRoomID,&outdata,sizeof(MSG_MG_S_RETURN_DESK_NOTICE),MDM_MG_CREATEROOM,ASS_MG_RETURNDESK_NOTICE,0,0);
	}
	else
	{
		m_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pHandleResult->_result, sizeof(pHandleResult->_result),MDM_GP_DESK_VIP,ASS_GP_ENTER_DESK, pResultData->uHandleRusult, pResultData->dwHandleID);
	}
	return true;
}

bool CCreateRoom::OnGetTotalRecordResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_TOTALRECORD_RES* pHandleResult= (DL_O_HALL_TOTALRECORD_RES*)pResultData;
	if(!pHandleResult)
	{
		return false;
	}

	if (0 == pHandleResult->iCount)
	{
		//return true;
	}

	//时间类型转换
	for (int i=0;i<pHandleResult->iCount;i++)
	{
		SYSTEMTIME systime;
		VariantTimeToSystemTime(pHandleResult->datatime[i], &systime);
		CTime timeTmp(systime);
		pHandleResult->_result[i].CreateTime = timeTmp.GetTime();
	}

	for (int i=0;i<pHandleResult->iCount;i++)
	{
		m_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pHandleResult->_result[i], sizeof(MSG_GP_O_TotalRecord),MDM_GP_DESK_VIP,ASS_GP_GETTOTALRECORD, 0, pResultData->dwHandleID);
	}
	m_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, MDM_GP_DESK_VIP,ASS_GP_GETTOTALRECORD, 1, pResultData->dwHandleID);

	SafeDeleteArray(pHandleResult->_result);
	
	return true;
}

bool CCreateRoom::OnGetSingelRecordResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_SINGLERECORD_RES* pHandleResult= (DL_O_HALL_SINGLERECORD_RES*)pResultData;
	if(!pHandleResult)
	{
		return false;
	}

	//时间类型转换
	for (int i=0;i<pHandleResult->iCount;i++)
	{
		SYSTEMTIME systime;
		VariantTimeToSystemTime(pHandleResult->datatime[i], &systime);
		CTime timeTmp(systime);
		pHandleResult->_result[i].EndTime = timeTmp.GetTime();
	}

	for (int i=0;i<pHandleResult->iCount;i++)
	{
		m_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pHandleResult->_result[i], sizeof(MSG_GP_O_SingleRecord),MDM_GP_DESK_VIP,ASS_GP_GETSINGLERECORD, 0, pResultData->dwHandleID);
	}
	m_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, MDM_GP_DESK_VIP,ASS_GP_GETSINGLERECORD, 1, pResultData->dwHandleID);

	SafeDeleteArray(pHandleResult->_result);

	return true;
}


bool CCreateRoom::OnGetCutRoomResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_NETCUT_RES* pHandleResult= (DL_O_HALL_NETCUT_RES*)pResultData;
	if(!pHandleResult)
	{
		return false;
	}

	if (1 == pResultData->uHandleRusult || 2 == pResultData->uHandleRusult || 4 == pResultData->uHandleRusult)
	{
		ComRoomInfo *pFindRoomInfo=m_pLogonManage->m_GameList.FindRoomInfo(pHandleResult->_result.iRoomID);
		if(pFindRoomInfo)
		{
			pHandleResult->_result.tCutNetRoomInfo=*pFindRoomInfo;
		}
		else
		{
			pResultData->uHandleRusult=0;
		}
	}

	if (pResultData->uHandleRusult == 2)
	{
		MSG_MG_S_RETURN_DESK_NOTICE outdata;
		memcpy_s(&outdata._data,sizeof(outdata._data),&pHandleResult->_result,sizeof(MSG_GP_S_GET_CUTNETROOMINFO_RES));
		outdata.iType = 2;
		m_pLogonManage->m_pGameManageModule->m_LogonManage.SendData(pHandleResult->_result.iRoomID,&outdata,sizeof(MSG_MG_S_RETURN_DESK_NOTICE),MDM_MG_CREATEROOM,ASS_MG_RETURNDESK_NOTICE,0,0);
	}
	
	m_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pHandleResult->_result, sizeof(pHandleResult->_result),MDM_GP_DESK_VIP,ASS_GP_GET_CUTROOM, pResultData->uHandleRusult, pResultData->dwHandleID);

	return true;
}

bool CCreateRoom::OnHandleCreateResult(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	if (!m_pLogonManage) 
	{
		return true;
	}

	if (pNetHead->bMainID != MDM_MG_CREATEROOM)
	{
		return true;
	}

	switch(pNetHead->bAssistantID)
	{
	case ASS_MG_BUYROOM_NOTICE:
		{
			if(uSize != sizeof(MSG_MG_S_BUY_DESK_NOTICE))
				return false;
			MSG_MG_S_BUY_DESK_NOTICE* pReciveData = (MSG_MG_S_BUY_DESK_NOTICE*)pData;
			if (!pReciveData)
				return true;

			LPUSER p = m_pLogonManage->GetUserByID(pReciveData->_data.iUserID);
			if (!p)
			{
				return true;
			}
			if (pReciveData->bSuccess)
            {
				m_pLogonManage->m_TCPSocket.SendData(p->iSocketIndex,&pReciveData->_data,sizeof(pReciveData->_data),MDM_GP_DESK_VIP,ASS_GP_BUY_DESK,1,0);
            }
			else  //创建失败
            {
				m_pLogonManage->m_TCPSocket.SendData(p->iSocketIndex,&pReciveData->_data,sizeof(pReciveData->_data),MDM_GP_DESK_VIP,ASS_GP_BUY_DESK,8,0);
            }
			return true;
		}
		break;
	case ASS_MG_RETURNDESK_NOTICE:
		{
			if(uSize != sizeof(MSG_MG_S_RETURN_DESK_NOTICE))
				return false;
			MSG_MG_S_RETURN_DESK_NOTICE* pReciveData = (MSG_MG_S_RETURN_DESK_NOTICE*)pData;
			if (!pReciveData)
				return true;

			LPUSER p = m_pLogonManage->GetUserByID(pReciveData->_data.iUserID);
			if (!p)
			{
				return true;
			}

			if (pReciveData->iType)
			{
				m_pLogonManage->m_TCPSocket.SendData(p->iSocketIndex,&pReciveData->_data,sizeof(pReciveData->_data),MDM_GP_DESK_VIP,ASS_GP_ENTER_DESK,1,0);
			}
			else
			{
				m_pLogonManage->m_TCPSocket.SendData(p->iSocketIndex,&pReciveData->_data,sizeof(pReciveData->_data),MDM_GP_DESK_VIP,ASS_GP_GET_CUTROOM,1,0);
			}
			
			return true;
		}
		break;
	default:
		break;
	}

	return true;
}

bool CCreateRoom::OnHandleDissmissResult(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	if (!m_pLogonManage) 
	{
		return true;
	}

	if (pNetHead->bMainID != MDM_MG_CREATEROOM)
	{
		return true;
	}

	MSG_MG_S_DissmissDesk *pReciveData = (MSG_MG_S_DissmissDesk*)pData;

	LPUSER p = m_pLogonManage->GetUserByID(pReciveData->iUserID);
	if (!p)
	{
		return true;
	}

	m_pLogonManage->m_TCPSocket.SendData(p->iSocketIndex,MDM_GP_DESK_VIP,ASS_GP_DISSMISSDESK,pReciveData->iResult,0);
}


bool CCreateRoom::OnGetBuyDeskConfigResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_BuyDeskConfig_RES* pHandleResult= (DL_O_HALL_BuyDeskConfig_RES*)pResultData;
	if(!pHandleResult)
	{
		return false;
	}
	m_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pHandleResult->_result, sizeof(pHandleResult->_result),MDM_GP_DESK_VIP,ASS_GP_GET_DESKCONFIG, pResultData->uHandleRusult, pResultData->dwHandleID);
}

bool CCreateRoom::OnGetRecordURLResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_GetRecordURL_RES* pHandleResult= (DL_O_HALL_GetRecordURL_RES*)pResultData;
	if(!pHandleResult)
	{
		return false;
	}
	if (0 == pResultData->uHandleRusult)
	{
		CString szRecordURL = m_pLogonManage->m_szDownLoadURL+pHandleResult->_result.szRecordCode+".txt";
		if( (_access( szRecordURL, 0 )) == -1 )
		{
			pResultData->uHandleRusult = 1;
		}
	}
	
	m_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pHandleResult->_result, sizeof(pHandleResult->_result),MDM_GP_DESK_VIP,ASS_GP_GET_RECORDURL, pResultData->uHandleRusult, pResultData->dwHandleID);
}

bool CCreateRoom::OnGetDeskListResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_GetDeskList_RES* pHandleResult= (DL_O_HALL_GetDeskList_RES*)pResultData;
	if(!pHandleResult)
	{
		return false;
	}

	//时间类型转换
	for (int i=0;i<pHandleResult->iCount;i++)
	{
		SYSTEMTIME systime;
		VariantTimeToSystemTime(pHandleResult->datatime[i], &systime);
		CTime timeTmp(systime);
		pHandleResult->_result[i].i64BuyTime = timeTmp.GetTime();
	}

	char bBuffer[MAX_SEND_SIZE];
	memset(bBuffer,0,sizeof(bBuffer));
	int iMax = MAX_SEND_SIZE/sizeof(MSG_GP_O_GetBuydeskList);
	int SendCount = 0;
	int SendTotalcount = 0;

	while(SendCount*iMax < pHandleResult->iCount)
	{
		int PreSend = (pHandleResult->iCount-SendCount*iMax)>iMax?iMax:(pHandleResult->iCount-SendCount*iMax);
		for (int i = 0;i<PreSend;i++)
		{
			memcpy_s(bBuffer+i*sizeof(MSG_GP_O_GetBuydeskList),sizeof(MSG_GP_O_GetBuydeskList),&pHandleResult->_result[i+SendCount*iMax],sizeof(MSG_GP_O_GetBuydeskList));
		}
		m_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, bBuffer, sizeof(MSG_GP_O_GetBuydeskList)*PreSend,MDM_GP_DESK_VIP,ASS_GP_GET_DESKLIST, 0, pResultData->dwHandleID);
		SendCount++;
	}
	m_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, MDM_GP_DESK_VIP,ASS_GP_GET_DESKLIST, 1, pResultData->dwHandleID);

	SafeDeleteArray(pHandleResult->_result);
}

bool CCreateRoom::OnGetDeskUserResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_GetDeskUser_RES* pHandleResult= (DL_O_HALL_GetDeskUser_RES*)pResultData;
	if(!pHandleResult)
	{
		return false;
	}

	char bBuffer[MAX_SEND_SIZE];
	memset(bBuffer,0,sizeof(bBuffer));
	int iMax = (MAX_SEND_SIZE-20)/sizeof(MSG_GP_O_GetDeskUser);
	int SendCount = 0;
	int SendTotalcount = 0;

	while(SendCount*iMax < pHandleResult->iCount)
	{
		int PreSend = (pHandleResult->iCount-SendCount*iMax)>iMax?iMax:(pHandleResult->iCount-SendCount*iMax);
		memcpy_s(bBuffer,sizeof(pHandleResult->szDeskPass),pHandleResult->szDeskPass,sizeof(pHandleResult->szDeskPass));
		for (int i = 0;i<PreSend;i++)
		{
			memcpy_s(bBuffer+sizeof(pHandleResult->szDeskPass)+i*sizeof(MSG_GP_O_GetDeskUser),sizeof(MSG_GP_O_GetDeskUser),&pHandleResult->_result[i+SendCount*iMax],sizeof(MSG_GP_O_GetDeskUser));
		}
		m_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, bBuffer, sizeof(pHandleResult->szDeskPass)+sizeof(MSG_GP_O_GetDeskUser)*PreSend,MDM_GP_DESK_VIP,ASS_GP_GET_DESKUSER, 0, pResultData->dwHandleID);
		SendCount++;
	}
	m_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, MDM_GP_DESK_VIP,ASS_GP_GET_DESKUSER, 1, pResultData->dwHandleID);

	SafeDeleteArray(pHandleResult->_result);
}

bool CCreateRoom::OnDissmissDeskResponse(DataBaseResultLine* pResultData)
{
	DL_O_HALL_DissmissDesk* pHandleResult= (DL_O_HALL_DissmissDesk*)pResultData;
	if(!pHandleResult)
	{
		return false;
	}

	if (ERR_GP_DISSMISSDESK_SUCCESS == pResultData->uHandleRusult && pHandleResult->_data.iRoomID!=255)  //正在游戏房间解散需先通知Gserver
	{
		m_pLogonManage->m_pGameManageModule->m_LogonManage.SendData(pHandleResult->_data.iRoomID,&pHandleResult->_data,sizeof(MSG_MG_S_DissmissDesk),MDM_MG_CREATEROOM,ASS_MG_DISSMISSDESK,0,0);
	}
	else
	{
		m_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, MDM_GP_DESK_VIP,ASS_GP_DISSMISSDESK, pResultData->uHandleRusult, pResultData->dwHandleID);
	}
	return true;
}

bool CCreateRoom::OnDeleteRecordResponse(DataBaseResultLine* pResultData)
{
	m_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, MDM_GP_DESK_VIP,ASS_GP_DELETE_RECORD, pResultData->uHandleRusult, pResultData->dwHandleID);

	return true;
}

bool CCreateRoom::OnDissmissDeskByMS(DataBaseResultLine* pResultData)
{        
    DL_O_DissmissDeskByMS *pDissData = (DL_O_DissmissDeskByMS*)pResultData;
    {
        MSG_MG_S_RETURN_DESK_NOTICE outdata;

        ComRoomInfo *pFindRoomInfo=m_pLogonManage->m_GameList.FindRoomInfo(pDissData->iRoomID);
        if(pFindRoomInfo)
        {
            outdata._data.tCutNetRoomInfo=*pFindRoomInfo;
        }
        outdata.iType = 1;
        outdata.bSuccess = true;
        outdata._data.bType = 2;
        outdata._data.bPositionLimit = pDissData->iPositionLimit;
        outdata._data.iDeskID = pDissData->iDeskNo;
        outdata._data.iRoomID = pDissData->iRoomID;
        outdata._data.iUserID = pDissData->iMasterID;
        memcpy(outdata._data.szPass, pDissData->szDeskPwd, sizeof(outdata._data.szPass));
        // 先恢复桌子
        m_pLogonManage->m_pGameManageModule->m_LogonManage.SendData(pDissData->iRoomID,&outdata,sizeof(MSG_MG_S_RETURN_DESK_NOTICE),MDM_MG_CREATEROOM,ASS_MG_RETURNDESK_NOTICE,0,0);
    }
    {
        MSG_MG_S_DissmissDesk dissdata;
        dissdata.iDeskID = pDissData->iDeskNo;
        dissdata.iRoomID = pDissData->iRoomID;
        dissdata.iUserID = pDissData->iMasterID;
        memcpy(dissdata.szDeskPass,pDissData->szDeskPwd,sizeof(dissdata.szDeskPass));

        // 解散桌子
        m_pLogonManage->m_pGameManageModule->m_LogonManage.SendData(pDissData->iRoomID,&dissdata,sizeof(MSG_MG_S_DissmissDesk),MDM_MG_CREATEROOM,ASS_MG_DISSMISSDESK,0,0);
    }
    return true;
}