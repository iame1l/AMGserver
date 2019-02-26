#pragma once
#include "def.h"
#pragma warning( push )
#pragma warning( disable:4482 )

class CCommon
{
public:
	// ÒÆ³ý¿¨ÅÆ
	static int RemoveCard( OUT Card * pDest, int nDestCount, const Card * pSrc, int nSrcCount );

	static ComboType GetComboType( const Card * pCardList, int nCardCount );

	static bool IsValidCard( Card card );

	static int GetCardRealGroup( OUT CardGroup * pGroup, int nGroupCount, const Card * pCardList, int nCardCount );

	static int GetSpecifyNumberCard( const CardGroup * pGroup, int nCount, int nNum );
};


#pragma warning( pop )