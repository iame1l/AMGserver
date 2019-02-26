#include "CardBase.h"
#include "algorithm.h"

CardBase::CardBase(){}

CardBase::~CardBase(){}

CardBase::CardBase(const CardBase& c)
{
	_card = c.getData();
}

CardBase::CardBase(uchar c)
{
	_card = c;
}

bool CardBase::GetRandOneCard(const int iValue, CardBase &OutCard)
{
	if(iValue <= 0 || iValue > 13) return false;		//只处理A->K

	if(iValue == 1)		//A
	{
		OutCard = CardBase( rand() % 4 * 16 + 13 );
	}
	else		//2->k
	{
		OutCard = CardBase( rand() % 4 * 16 + (iValue - 1) );
	}
	return true;
}

bool CardBase::GetRandTwoCard(const int iValue, vector<CardBase>& OutCard)
{
	if(iValue <= 0 || iValue > 13) return false;		//只处理A->K
	vector<int> vAllCard;
	for(int i = 0; i < 4; ++i)
	{
		if(iValue == 1)		//A
		{
			vAllCard.push_back(i * 16 + 13);
		}
		else		//2->k
		{
			vAllCard.push_back(i * 16 + (iValue - 1));
		}
	}
	random_shuffle(vAllCard.begin(), vAllCard.end());

	for(int i = 0; i < 2; ++i)
	{
		OutCard[i] = CardBase(vAllCard[i]);
	}
	return true;
}

const CardBase& CardBase:: operator = (const CardBase& c)
{
	_card = c.getData();
	return *this;
}

bool CardBase::operator ==(const CardBase& c)const
{
	return this->_equal(c);
}

bool CardBase::operator !=(const CardBase& c)const
{
	return !this->_equal(c);
}

bool CardBase::operator <(const CardBase& c)const
{
	return this->_lessThan(c);
}

bool CardBase::isValid()const
{
	if (getType() <= TYPE_SPADE )
	{
		return getVal()  <= VAL_A && getVal()  >= VAL_2;
	}
	if(getType() == TYPE_JOKER)
	{
		return getVal()  == VAL_SJ || getVal() == VAL_BJ;
	}
	return false;
}	

bool CardBase::_equal(const CardBase & c)const
{
	if(!isValid()) return false;
	if(!c.isValid()) return false;
	return _card == c.getData();
}

bool CardBase::_lessThan(const CardBase &c)const
{
	if(!isValid()) return false;
	if(!c.isValid()) return false;
	if (getSeqRealVal() != c.getSeqRealVal())
	{
		return getSeqRealVal() < c.getSeqRealVal();
	}
	return getSeqRealVal() <  c.getSeqRealVal();
}



uval CardBase::getVal()const
{
	return _card & 0x0f; 
}

uval CardBase::getRealVal()const
{
	uval val = (_card & 0x0f)  + 1;
	switch(val)
	{
	case 0x10:
		val = 17;
		break;
	default:
		break;
	}
	return val;
}

uval CardBase::getSeqRealVal()const
{
	uval val = (_card & 0x0f)  + 1;
	switch(val)
	{
	case 0x10:
		val = 17;
		break;
	case 2:
		val = 15;
		break;
	default:
		break;
	}
	return val;
}

emCardColor CardBase::getType()const
{
	return emCardColor(_card>>4);
}

uchar CardBase::getData()const
{
	return _card;
}

void CardBase::setData(const CardBase c)
{
	_card= c.getData();
}


uint CardBase::score()const
{
	if(getVal() == VAL_5) return 5;
	if(getVal() == VAL_10|| getVal() == VAL_K) return 10;
	return 0;
}

bool  CardBase::isScoreCard()const
{
	if(getVal() == VAL_5 ||getVal() == VAL_10|| getVal() == VAL_K) return true;
	return false;
}

bool CardBase::canMakeSeq(uchar bySeq)const
{
	if(!isValid()) return false;
	if (VAL_2 == getVal())
	{
		return false;
	}
	if (2==bySeq)
	{
		if (VAL_A == getVal())
		{
			return false;
		}
	}
	return true;
}