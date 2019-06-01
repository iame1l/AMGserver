/****************************************************************************
Copyright (c) 2014-2017 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2017 ShenZhen Redbird Network Polytron Technologies Inc.
 
http://www.hotniao.com

All of the content of the software, including code, pictures, 
resources, are original. For unauthorized users, the company 
reserves the right to pursue its legal liability.
****************************************************************************/
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             #include "CardBase.h"
#include "../util/algorithm/algorithm.h"
namespace HN
{
	CardBase::CardBase()
	{
		_card = CARD_NULL;
	}

	CardBase::~CardBase()
	{

	}

	CardBase::CardBase(const CardBase& c)
	{
		_card = c.getData();
	}

	CardBase::CardBase(ucard c)
	{
		_card = c;
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
			 return getVal() >= VAL_A && getVal() <= VAL_K;
		}
		if(getType() == TYPE_JOKER)
		{
			return getVal() == VAL_SJ || getVal() == VAL_BJ;
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
		if (getRealVal() != c.getRealVal())
		{
			return getRealVal() < c.getRealVal();
		}
		return getData() <  c.getData();
	}
	
	CardBase::emCardType CardBase::getType()const
	{
		return emCardType(_card >> CARD_MASK_TYPE);
	}

	uval CardBase::getVal()const
	{
		return _card & CARD_MASK_VALUE;
	}

	uval CardBase::getRealVal()const
	{
		uval val = (_card & CARD_MASK_VALUE);
		if( val <= VAL_K )           //// 3456789 10 JQK 12 小鬼大鬼  实际牌型大小
		{
		    val = (13 + val - 3)%13+1;  
		}
		return val;
	}

	ucard CardBase::getData()const
	{
		return _card;
	}
}