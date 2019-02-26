#include "stdafx.h"
#include "CardData.h"
#include "def.h"
#include "Cfg.h"
#include "Common.h"

bool CPlayerCardMgr::Init()
{
	OBJ_GET_EXT(m_pExt, CCfg, cfg);

	int nCount = cfg->GetValue(CFG_PLAYER_COUNT);
	m_data.resize(nCount);

	return 0 < nCount && m_data.size() == nCount;
}

void CPlayerCardMgr::Clear()
{
	m_data.clear();
	m_backCard.clear();
}

bool CPlayerCardMgr::SetHandCard( int nIndex, const Card * pCardList, int nCardCount )
{
	bool bResult = IsValidIndex(nIndex);
	if( bResult )
	{
		if( pCardList && 0 < nCardCount )
			m_data[nIndex].handCard.assign(pCardList, pCardList + nCardCount);
		else
			m_data[nIndex].handCard.clear();
	}
	return bResult;
}

bool CPlayerCardMgr::PushHandCard(int nIndex, const Card * pCardList, int nCardCount)
{
	bool bResult = IsValidIndex(nIndex);
	if( bResult && pCardList && 0 < nCardCount )
	{
		CardList & cl = m_data[nIndex].handCard;
		for( int i = 0; nCardCount > i; i++ )
			cl.push_back(pCardList[i]);
	}
	return bResult;
}

const Card * CPlayerCardMgr::GetHandCard(int nIndex, OUT int * pCardCount)
{
	const Card * pResult = NULL;
	if( IsValidIndex(nIndex) )
	{
		const CardList & cl = m_data[nIndex].handCard;
		pResult = cl.data();
		if( pCardCount )
			*pCardCount = (int)cl.size();
	}
	else if( pCardCount )
		*pCardCount = 0;
	return pResult;
}

int CPlayerCardMgr::GetHandCardCount(int nIndex)
{
	return IsValidIndex(nIndex) ? m_data[nIndex].handCard.size() : -1;
}

int CPlayerCardMgr::RemoveCard(int nIndex, const Card * pCardList, int nCardCount)
{
	int nResult = 0;
	if( IsValidIndex( nIndex ) )
	{
		CardList & cl = m_data[nIndex].handCard;
		nResult = CCommon::RemoveCard( cl.data(), (int)cl.size(), pCardList, nCardCount );
		if( nResult == nCardCount )
			cl.erase( cl.end() - nResult, cl.end() );
		else
			nResult = 0;
	}

	return nResult;
}

bool CPlayerCardMgr::SetDeskCard(int nIndex, const Card * pCardList, int nCardCount)
{
	bool bResult = IsValidIndex(nIndex);
	if( bResult )
	{
		if( pCardList && 0 < nCardCount )
			m_data[nIndex].deskCard.assign(pCardList, pCardList + nCardCount);
		else
			m_data[nIndex].deskCard.clear();
	}
	return bResult;
}

const Card * CPlayerCardMgr::GetDeskCard(int nIndex, OUT int * pCardCount) const
{
	const Card * pResult = NULL;
	if( IsValidIndex(nIndex) )
	{
		const CardList & cl = m_data[nIndex].deskCard;
		pResult = cl.data();
		if( pCardCount )
			*pCardCount = (int)cl.size();
	}
	else if( pCardCount )
		*pCardCount = 0;
	return pResult;
}

void CPlayerCardMgr::ClearDeskCard()
{
	for( int i = 0; (int)m_data.size() > i; i++ )
	{
		m_data[i].deskCard.clear();
	}
}

void CPlayerCardMgr::SetBackCard(const Card * pCardList, int nCardCount)
{
	m_backCard.assign(pCardList, pCardList + nCardCount);
}
