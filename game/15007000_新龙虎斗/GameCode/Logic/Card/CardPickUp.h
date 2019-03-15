                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
#ifndef CARD_PICK_UP
#define CARD_PICK_UP

#include "CardArrayBase.h"
#include <vector>
using namespace std;

class CardPickUp
{
public:
	CardPickUp();
	virtual ~CardPickUp();

public:	
	//找到最大的牌型
	static void PickUpMaxCardType(__in const CardArrayBase& srcCard,
								  __out CardArrayBase& maxCard,
								  CardArrayBase & tmpCard,
								  size_t offset);
};

#endif//CARD_PICK_UP
