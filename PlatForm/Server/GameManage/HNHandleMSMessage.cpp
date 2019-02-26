#include "Stdafx.h"
#include "HNHandleMSMessage.h"
#include "GameMainManage.h"

#define IDT_CHECK_DESK		51L					//检测是否有人定时器 ID

CHandleMSMessage::CHandleMSMessage(CWnd *pWnd)
{
	m_pGameManage = (CGameMainManage*)pWnd;
}

CHandleMSMessage::~CHandleMSMessage(void)
{

}

bool CHandleMSMessage::OnNetMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	if (!m_pGameManage) 
	{
		return true;
	}

	if (MDM_MG_CREATEROOM == pNetHead->bMainID)
	{
		if (ASS_MG_BUYROOM_NOTICE == pNetHead->bAssistantID)
		{
			return OnBuyDeskNotice(pNetData,uDataSize,pClientSocket);
		}
		else if (ASS_MG_RETURNDESK_NOTICE == pNetHead->bAssistantID)
		{
			return OnReturnDeskNotice(pNetData,uDataSize,pClientSocket);
		}
		else if (ASS_MG_DISSMISSDESK == pNetHead->bAssistantID)
		{
			return OnHallDissmissDesk(pNetData,uDataSize,pClientSocket);
		}
	}
	else if (MDM_MG_NETCUT_PLACE == pNetHead->bMainID)
	{
		if (ASS_MG_NETCUT_PLACE == pNetHead->bAssistantID)
		{
			return OnNetCutPlace(pNetData,uDataSize,pClientSocket);
		}
	}
	else if (MDM_MG_CONTEST == pNetHead->bMainID)
	{
		if (ASS_MG_CONTEST_APPLY == pNetHead->bAssistantID)
		{
			return OnContestActive(pNetData,uDataSize,pClientSocket);
		}
	}
	else if (MDM_MG_CLUB == pNetHead->bMainID)
	{
		if (ASS_MG_CLUB_DISSMISSDESK == pNetHead->bAssistantID)
		{
			return OnClubDissmissDesk(pNetData,uDataSize,pClientSocket);
		}
		else if (ASS_MG_CLUB_BUYDESK == pNetHead->bAssistantID)
		{
			return OnClubBuyDesk(pNetData,uDataSize,pClientSocket);
		}
	}

	return true;
}

bool CHandleMSMessage::OnDataBaseResult(DataBaseResultLine * pResultData)
{
	if (!m_pGameManage) 
	{
		return true;
	}

	switch(pResultData->uHandleKind)
	{
	case 1:
		{
			//return OnBuyDeskResponse(pResultData);
		}
		break;
	default:
		break;
	}
	return true;
}

//通知刷新桌子信息
bool CHandleMSMessage::OnBuyDeskNotice(void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
    WriteLog("OnBuyDeskNotice");
	if (sizeof(MSG_MG_S_BUY_DESK_NOTICE) != uDataSize) return false;

	MSG_MG_S_BUY_DESK_NOTICE *p = (MSG_MG_S_BUY_DESK_NOTICE*)pNetData;

	CGameDesk * pGameDesk=m_pGameManage->GetDeskInfo(p->_data.iDeskID);
	if(!pGameDesk)
	{
        WriteLog("OnBuyDeskNotice GetDeskInfo Failed.");
		return false;
	}

	pGameDesk->KillTimer(IDT_CHECK_DESK);
	pGameDesk->SetTimer(IDT_CHECK_DESK,3*60*1000);

	DL_GR_I_GET_DESKINFO input;		
	memcpy_s(&input._BuyData,sizeof(input._BuyData),p,sizeof(MSG_MG_S_BUY_DESK_NOTICE));
	input.iType = TYPE_BUYROOM_NOTICE;

	m_pGameManage->m_SQLDataManage.PushLine(&input.DataBaseHead, sizeof(DL_GR_I_GET_DESKINFO), DTK_GR_GET_DESKINFO, 0, 0);

	return true;
}

//恢复桌子信息
bool CHandleMSMessage::OnReturnDeskNotice(void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	if (sizeof(MSG_MG_S_RETURN_DESK_NOTICE) != uDataSize) return false;

	MSG_MG_S_RETURN_DESK_NOTICE *p = (MSG_MG_S_RETURN_DESK_NOTICE*)pNetData;

	CGameDesk * pGameDesk=m_pGameManage->GetDeskInfo(p->_data.iDeskID);
	if(!pGameDesk)
	{
		return false;
	}

	pGameDesk->m_bReturnDesk = true;
	pGameDesk->m_pReturnInfo.clear();
	
	DL_GR_I_GET_DESKINFO input;		
	memcpy_s(&input._ReturnData,sizeof(input._ReturnData),p,sizeof(MSG_MG_S_RETURN_DESK_NOTICE));
	input.iType = TYPE_RETURNDESK_NOTICE;

	m_pGameManage->m_SQLDataManage.PushLine(&input.DataBaseHead, sizeof(DL_GR_I_GET_DESKINFO), DTK_GR_GET_DESKINFO, 0, 0);


	if (DESK_TYPE_BJL!=pGameDesk->m_uDeskType)
	{
		DL_GR_I_ReturnDesk input;
		input.iDeskID = p->_data.iDeskID;
		input.iRoomID = m_pGameManage->m_InitData.uRoomID;
		m_pGameManage->m_SQLDataManage.PushLine(&input.DataBaseHead, sizeof(input), DTK_GR_RETURNEDESK, 0, 0);
	}
	
	pGameDesk->KillTimer(IDT_CHECK_DESK);
	pGameDesk->SetTimer(IDT_CHECK_DESK,3*60*1000);

	return true;
}

//用户大厅断线，断开房间消息
bool CHandleMSMessage::OnNetCutPlace(void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	if (sizeof(MSG_MG_S_NETCUT_PLACE) != uDataSize) return false;

	MSG_MG_S_NETCUT_PLACE *p = (MSG_MG_S_NETCUT_PLACE*)pNetData;

	CGameUserInfo * pGameUserInfo=NULL;
	pGameUserInfo=m_pGameManage->m_UserManage.FindOnLineUser(p->iUserID);
	if (pGameUserInfo!=NULL && pGameUserInfo->m_UserData.isVirtual != 1)
	{
		m_pGameManage->m_TCPSocket.CloseSocket(pGameUserInfo->m_uSocketIndex, 0);
		return true;
	}

	return true;
}

//激活比赛房间
bool CHandleMSMessage::OnContestActive(void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	if (sizeof(MSG_MG_S_CONTEST_ACTIVE) != uDataSize) return false;

	MSG_MG_S_CONTEST_ACTIVE *p = (MSG_MG_S_CONTEST_ACTIVE*)pNetData;

	if (!p->bNotify || p->iRoonID!=m_pGameManage->m_InitData.uRoomID || p->BeginTime<COleDateTime::GetCurrentTime())
	{
		m_pGameManage->m_pMainServerSocket->SendData(p,sizeof(MSG_MG_S_CONTEST_ACTIVE),MDM_MG_CONTEST,ASS_MG_CONTEST_APPLY,1);
	}

	DL_GR_I_ContestActive _in;
	memcpy_s(&_in._data,sizeof(_in._data),p,sizeof(MSG_MG_S_CONTEST_ACTIVE));
	m_pGameManage->m_SQLDataManage.PushLine(&_in.DataBaseHead, sizeof(_in), DTK_GR_CONTEST_ACTIVE, 0, 0);

    MSG_GR_I_ContestInfo msgContestInfo; //仅仅为了刷新报名人数
    msgContestInfo.iContestNum = p->iApplyNum;
    msgContestInfo.iContestBegin = false;
    m_pGameManage->m_TCPSocket.SendDataBatch(&msgContestInfo,sizeof(msgContestInfo),MDM_GR_ROOM,ASS_GR_CONTEST_APPLYINFO,0);


	return true;
}

bool CHandleMSMessage::OnHallDissmissDesk(void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	if (sizeof(MSG_MG_S_DissmissDesk) != uDataSize) return false;

	MSG_MG_S_DissmissDesk *p = (MSG_MG_S_DissmissDesk*)pNetData;

	if (p->iRoomID!=m_pGameManage->m_InitData.uRoomID)
	{
		return true;
	}


    DL_GR_I_DissmissDeskByMS input;	
    memcpy_s(&input.data,sizeof(input.data),p,sizeof(MSG_MG_S_DissmissDesk));
    m_pGameManage->m_SQLDataManage.PushLine(&input.DataBaseHead, sizeof(DL_GR_I_DissmissDeskByMS), DTK_GR_DISSMISSDESKBYMS, 0, 0);

	return true;
}


bool CHandleMSMessage::OnClubDissmissDesk(void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	if (sizeof(MSG_MG_R_ClubDissmissDesk) != uDataSize) return false;

	MSG_MG_R_ClubDissmissDesk *p = (MSG_MG_R_ClubDissmissDesk*)pNetData;

	if (p->iRoomID!=m_pGameManage->m_InitData.uRoomID)
	{
		return true;
	}

	CGameDesk * pGameDesk=m_pGameManage->GetDeskInfo(p->iDeskID);
	if(!pGameDesk)
	{
		return true;
	}

	if (strcmp(pGameDesk->m_szDeskPassWord,p->szDeskPass) != 0)		//桌子密码不对
	{
		return true;
	}

	pGameDesk->GameFinish(0,GFF_DISSMISS_FINISH);

	return true;
}

//通知刷新桌子信息
bool CHandleMSMessage::OnClubBuyDesk(void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	if (sizeof(MSG_MG_S_CLUB_BUY_DESK_NOTICE) != uDataSize) return false;

	MSG_MG_S_CLUB_BUY_DESK_NOTICE *p = (MSG_MG_S_CLUB_BUY_DESK_NOTICE*)pNetData;

	CGameDesk * pGameDesk=m_pGameManage->GetDeskInfo(p->iDeskID);
	if(!pGameDesk)
	{
		return false;
	}

	pGameDesk->KillTimer(IDT_CHECK_DESK);
	pGameDesk->SetTimer(IDT_CHECK_DESK,3*60*1000);

	DL_GR_I_GET_DESKINFO input;		
	memcpy_s(&input._ClubBuyData,sizeof(input._ClubBuyData),p,sizeof(MSG_GP_O_Club_BuyDesk));
	input.iType = TYPE_CLUB_BUYDESK;

	m_pGameManage->m_SQLDataManage.PushLine(&input.DataBaseHead, sizeof(DL_GR_I_GET_DESKINFO), DTK_GR_GET_DESKINFO, 0, 0);

	return true;
}