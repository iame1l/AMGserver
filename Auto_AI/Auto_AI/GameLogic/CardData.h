#pragma once
#include "def.h"
#include "ExtensionObject.h"

typedef struct tagPlayerCardData{
	CardList	handCard;
	CardList	deskCard;
}PlayerCardData;

typedef std::vector<PlayerCardData>		VPlayerCardData;

class CPlayerCardMgr
	: public Extension
{
public:
	bool Init();
	void Clear();
	bool SetHandCard( int nIndex, const Card * pCardList, int nCardCount );
	bool PushHandCard( int nIndex, const Card * pCardList, int nCardCount );
	const Card * GetHandCard( int nIndex, OUT int * pCardCount );
	int GetHandCardCount( int nIndex );
	int RemoveCard( int nIndex, const Card * pCardList, int nCardCount );

	bool SetDeskCard( int nIndex, const Card * pCardList, int nCardCount );
	const Card * GetDeskCard( int nIndex, OUT int * pCardCount ) const;
	void ClearDeskCard();

	void SetBackCard( const Card * pCardList, int nCardCount );
	bool IsValidIndex( int nIndex ) const { return 0 <= nIndex && (int)m_data.size() > nIndex; }
public:
	VPlayerCardData		m_data;
	CardList			m_backCard;
};
