/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "Stdafx.h"
#include "HNHandleGMessage.h"
#include "GameLogonManage.h"

CHandleGMessage::CHandleGMessage(CGameLogonManage *pWnd)
{
	m_pLogonManage = pWnd;
}

CHandleGMessage::~CHandleGMessage(void)
{

}

bool CHandleGMessage::OnNetMessage(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	if (!m_pLogonManage) 
	{
		return true;
	}

	switch(pNetHead->bMainID)
	{
	case MDM_MG_CREATEROOM:
		{
			if (ASS_MG_DISSMISSDESK == pNetHead->bAssistantID)
			{
				m_pLogonManage->m_pCreateRoom->OnHandleDissmissResult(pNetHead, pData, uSize, uAccessIP, uIndex, dwHandleID);
			}
			else
			{
				m_pLogonManage->m_pCreateRoom->OnHandleCreateResult(pNetHead, pData, uSize, uAccessIP, uIndex, dwHandleID);
			}
		}
		break;
	case MDM_MG_CONTEST:
		{
			if (ASS_MG_CONTEST_APPLY == pNetHead->bAssistantID)
			{
				OnContestActiveError(pNetHead, pData, uSize, uAccessIP, uIndex, dwHandleID);
			}
			else if (ASS_MG_CONTEST_NOTICE == pNetHead->bAssistantID)
			{
				OnContestNotice(pNetHead, pData, uSize, uAccessIP, uIndex, dwHandleID);
			}
		}
		break;
	case MDM_MG_CLUB:
		{
			if (ASS_MG_CLUB_BUYDESK == pNetHead->bAssistantID)
			{
				m_pLogonManage->m_pHNClub->OnHandleClubBuyDeskResult(pNetHead, pData, uSize, uAccessIP, uIndex, dwHandleID);
			}
			else if (ASS_MG_CLUB_CLEARDESK == pNetHead->bAssistantID)
			{
				m_pLogonManage->m_pHNClub->OnHandleClubClearDeskResult(pNetHead, pData, uSize, uAccessIP, uIndex, dwHandleID);
			}
		}break;
	default:
		break;
	}

	return true;
}

bool CHandleGMessage::OnDataBaseResult(DataBaseResultLine * pResultData)
{
	if (!m_pLogonManage) 
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

bool CHandleGMessage::OnContestActiveError(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	MSG_MG_S_CONTEST_ACTIVE *p = (MSG_MG_S_CONTEST_ACTIVE*)pData;


	return true;
}

bool CHandleGMessage::OnContestNotice(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	MSG_MG_S_CONTEST_NOTICE *p = (MSG_MG_S_CONTEST_NOTICE*)pData;

	DL_I_HALL_CONTEST_NOTICE _out;
	_out.iRoomID = p->iRoomID;
	_out.iContestID = p->iCountID;

	m_pLogonManage->m_SQLDataManage.PushLine(&_out.DataBaseLineHead, sizeof(DL_I_HALL_CONTEST_NOTICE), DTK_GP_CONTEST_NOTICE, 0, 0);

	return true;
}