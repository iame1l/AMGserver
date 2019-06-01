#pragma once
#include <vector>
typedef unsigned char			Card;
typedef std::vector<Card>		CardList;
typedef int						CardGroup;


#define INVALID_CARD			((Card)-1)
#define MASK_VALUE				0x0F
#define MASK_SUIT				0xF0
#define MAX_CARD_GROUP			20

#define GetCardValue(card)		( ((Card)card) & MASK_VALUE )
#define GetCardReal(card)		( GetCardValue(card) + 1 )

#define GetCardSuit(card)		( ((Card)card) & MASK_SUIT )
#define GetCardSuitValue(card)	( GetCardSuit(card) >> 4 )

enum {
	CardValueBegin = 0x1,
	CardValueEnd = 0xE,
	CardSuitBegin = 0x0,
	CardSuitEnd = 0x4,

	SequenceValueBegin = 0x2,		// 	ÆË¿Ë3
	SequenceValueEnd = 0xE,

	SequenceRealBegin = GetCardReal(SequenceValueBegin),	// 0x3 ÆË¿Ë3
	SequenceRealEnd = GetCardReal(SequenceValueEnd),
};
