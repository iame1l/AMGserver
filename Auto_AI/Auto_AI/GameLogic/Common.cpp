#include "stdafx.h"
#include "Common.h"

int CCommon::RemoveCard(OUT Card * pDest, int nDestCount, const Card * pSrc, int nSrcCount)
{
	int nResult = 0;

	do {
		if ( NULL == pDest || 0 >= nDestCount )
			break;

		if ( NULL == pSrc || 0 >= nSrcCount )
			break;

		for ( int i = 0; nSrcCount > i; i++ )
		{
			for ( int j = 0; nDestCount > j; j++ )
			{
				if ( pSrc[i] == pDest[j] )
				{
					nDestCount--;
					pDest[j] = pDest[nDestCount];
					pDest[nDestCount] = pSrc[i];
					nResult++;
					break;
				}
			}
		}

	} while( false );

	return nResult;
}

ComboType CCommon::GetComboType(const Card * pCardList, int nCardCount)
{

}

bool CCommon::IsValidCard(Card card)
{
	Card byValue = GetCardValue(card);
	Card bySuitValue = GetCardSuitValue(card);
	bool bResult = (0x4E == card || 0x4F == card)		// Íõ
		|| ( (CardValueBegin <= byValue && CardValueEnd > byValue
		&& CardSuitBegin <= bySuitValue && CardSuitEnd > bySuitValue )
		);

	return bResult;
}

int CCommon::GetCardRealGroup(OUT CardGroup * pGroup, int nGroupCount, const Card * pCardList, int nCardCount)
{
	int nResult = 0;

	do {
		if( NULL == pGroup || MAX_CARD_GROUP > nGroupCount )
			break;

		if( NULL == pCardList || 0 >= nCardCount )
			break;

		memset( pGroup, 0, sizeof(CardGroup) * nGroupCount );
		for( int i = 0; nCardCount > i; i++ )
		{
			if( IsValidCard(pCardList[i]) )
			{
				pGroup[GetCardReal(pCardList[i])]++;
				nResult++;
			}
		}

	} while (false);

	return nResult;
}

int CCommon::GetSpecifyNumberCard(const CardGroup * pGroup, int nCount, int nNum)
{
	int nResult = 0;
	if( pGroup && 0 < nCount )
	{
		for( int i = 0; nCount > i; i++ )
		{
			if( nNum == pGroup[i] )
				nResult++;
		}
	}
	return nResult;
}
