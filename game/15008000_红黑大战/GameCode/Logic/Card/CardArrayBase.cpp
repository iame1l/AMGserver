#include "algorithm.h"
#include "CardArrayBase.h"
#include <algorithm>
uchar one_pair_card [] = 
{ 
	D_2 , C_2 ,H_2 ,S_2 ,//2
	D_3 , C_3 ,H_3 ,S_3 ,
	D_4 , C_4 ,H_4 ,S_4 ,
	D_5 , C_5 ,H_5 ,S_5 ,
	D_6 , C_6 ,H_6 ,S_6 ,
	D_7 , C_7 ,H_7 ,S_7 ,
	D_8 , C_8 ,H_8 ,S_8 ,
	D_9 , C_9 ,H_9 ,S_9 ,
	D_10, C_10,H_10,S_10,
	D_J , C_J ,H_J ,S_J ,
	D_Q , C_Q ,H_Q ,S_Q ,
	D_K , C_K ,H_K ,S_K ,
	D_A , C_A ,H_A ,S_A ,//14
};
//D方块，C梅花，H红桃，S黑桃
//SJ  , BJ         小鬼、大鬼
CardArrayBase::CardArrayBase()
{
	_vCard.clear();
}

CardArrayBase::CardArrayBase(const CardArrayBase& v)
{
	_vCard = v.getArray();
}

CardArrayBase::CardArrayBase(const vector<CardBase>& v)
{
	_vCard = v;
}

CardArrayBase::~CardArrayBase()
{
	Clear();
}

const CardArrayBase& CardArrayBase::operator = (const CardArrayBase& v)
{
	_vCard = v.getArray();
	return *this;
}

bool CardArrayBase::operator ==(const CardArrayBase& v)const
{
	return _equal(v);
}

bool CardArrayBase::operator <(const CardArrayBase& v)const
{
	return _lessThanR(v);
}

std::ostream & operator << (std::ostream &ostrm, const CardArrayBase &cardList)
{
	for(auto itr = cardList.getArray().begin();
		itr != cardList.getArray().end();
		++itr)
	{
		ostrm<<(int)(*itr).getData();
		if(itr + 1 != cardList.getArray().end())
		{
			ostrm<<",";
		}
	}
	return ostrm;
}


size_t CardArrayBase::Size()const
{
	return _vCard.size();
}

bool CardArrayBase::Empty()const
{
	return _vCard.empty();
}

void CardArrayBase::Clear()
{
	vector<CardBase> v;
	_vCard.clear();
	_vCard.swap(v);
}

const vector<CardBase>& CardArrayBase::getArray()const
{
	return _vCard;
}

bool CardArrayBase::_equal(const CardArrayBase& v)const
{
	if (Size() != v.Size()) return false;
	vector<CardBase> va = _vCard;
	vector<CardBase> vb = v.getArray();
	sort(va.begin(),va.end());
	sort(vb.begin(),vb.end());
	for (size_t i = 0;i < Size();i++)
	{
		if (va.at(i) != vb.at(i))
		{
			return false;
		}
	}
	return true;
}

bool CardArrayBase::equal(const CardArrayBase& v)const
{
	if (Size() != v.Size()) return false;
	CardArrayBase va = _vCard;
	CardArrayBase vb = v;

	va.erase(vb);

	if(!va.Empty()) return false;
	
	return true;
}

bool CardArrayBase::_lessThan(const CardArrayBase& v)const
{
	Param ua, ub;
	eArrayType ta = type(ua);
	eArrayType tb = v.type(ub);
	if(ta < ARRAY_TYPE_510K_DIFF && tb < ARRAY_TYPE_510K_DIFF)
	{
		if(ua.val < ub.val) return true;
		else if(ua.val > ub.val) return false;
		else return ua.size > ub.size;
	}
	else	return ua < ub;
}

bool CardArrayBase::_lessThanN(const CardArrayBase& v)const
{
	Param ua, ub;
	eArrayType ta = type(ua);
	eArrayType tb = v.type(ub);
	if(ta < ARRAY_TYPE_510K_DIFF && tb < ARRAY_TYPE_510K_DIFF)
	{
		return ua.val * ua.size < ub.val*ub.size;
	}
	else	return ua < ub;
}

bool CardArrayBase::_lessThanR(const CardArrayBase& v)const
{
	Param ua, ub;
	eArrayType ta = type(ua);
	eArrayType tb = v.type(ub);
	return ua < ub;
}

 eArrayType CardArrayBase::type(Param &p)const 
 {
	 if(isSingle(p))			return (eArrayType)p.bType;
	 if(isDouble(p))			return (eArrayType)p.bType;
	 if(isThree(p))				return (eArrayType)p.bType;
	 if(isBomb_N(p))			return (eArrayType)p.bType;
	 if(isSameSeq(p))			return (eArrayType)p.bType;
	 if(isSequence(p))			return (eArrayType)p.bType;
	 if(isSameColor(p))         return (eArrayType)p.bType;
	 if(is3W2(p))				return (eArrayType)p.bType;
	 if(is4W1(p))				return (eArrayType)p.bType;
	 
	 return ARRAY_TYPE_ERROR;
 }

 uint  CardArrayBase::score()const
 {
	 uint score = 0;
	 for (auto it = _vCard.begin();it != _vCard.end();it++)
	 {
		 score += it->score(); 
	 }
	 return score;
 }

 CardArrayBase  CardArrayBase::getScoreCards()const
 {
	 CardArrayBase res;
	 for (auto it = _vCard.begin();it != _vCard.end();it++)
	 {
		 if(it->isScoreCard())
		 {
			 res.push_back(*it);
		 }
	 }
	 return res;
 }
  

void CardArrayBase::randCard()
{
	for (size_t i = 0; i < Size(); i++)
	{
		size_t index = Algor::rand_Mersenne(0, Size() - 1);
		std::swap(_vCard[i], _vCard[index]);
	}
}

size_t CardArrayBase::reAssign(uchar *arrCard, size_t len)
{
	for (size_t i = 0; i < len && i < Size(); i++)
	{
		arrCard[i] = _vCard.at(i).getData();
	}
	return min(len,Size());
}

void CardArrayBase::assign(uchar *arrCard, size_t len)
{
	_vCard.clear();
	for (size_t i = 0;i < len;i++)
	{
		CardBase tmp = arrCard[i];
		if(!tmp.isValid()) continue;
		_vCard.push_back(tmp);
	}
}


bool CardArrayBase::contain(const CardArrayBase&v)const
{
	if (Size() < v.Size()) return false;

	vector<CardBase> vTmp = _vCard;
	for (auto itb = v.getArray().begin();
		itb != v.getArray().end();itb++)
	{
		auto ita = find(vTmp.begin(),vTmp.end(),*itb);
		if (ita == vTmp.end()) return false;
		else
		{
			vTmp.erase(ita);
		}
	}

	return true;
}

bool CardArrayBase::contain(uval v)const
{
	auto ita = find_if(_vCard.begin(),_vCard.end(),[v](CardBase c)->bool
	{
		if(c.getVal() == v) return true;
		return false;
	});
	if(ita != _vCard.end()) return true;
	return false;
}

size_t CardArrayBase::has(const CardArrayBase&v)const
{
	size_t count = 0;
	vector<CardBase> vTmp = _vCard;
	for (auto itb = vTmp.begin();
		itb != vTmp.end();itb++)
	{
		auto ita = find(v.getArray().begin(),v.getArray().end(),*itb);
		if (ita != v.getArray().end()) count++;
	}
	return count;
}

bool CardArrayBase::contain(const CardBase &c)const
{
	return _vCard.end() != find(_vCard.begin(),_vCard.end(),c);
}

size_t CardArrayBase::count_of(const CardBase &c)const
{
	size_t count = 0;
	for (auto itb = _vCard.begin();
		itb != _vCard.end();itb++)
	{
		if(c == *itb) count++;
	}
	return count;
}

bool CardArrayBase::erase(const CardArrayBase&v)
{
	vector<CardBase> vTmp = _vCard;
	for (auto itb = v.getArray().begin();
		itb != v.getArray().end();itb++)
	{
		auto ita = find(vTmp.begin(),vTmp.end(),*itb);
		if (ita == vTmp.end()) continue;
		else
		{
			vTmp.erase(ita);
		}
	}
	_vCard.clear();
	_vCard = vTmp;
	return true;
}

bool CardArrayBase::eraseAll(const CardArrayBase&v)
{
	vector<CardBase> vTmp = _vCard;
	for (auto itb = vTmp.begin();
		itb != vTmp.end();)
	{
		auto ita = find(v.getArray().begin(),v.getArray().end(),*itb);
		if (ita != v.getArray().end())
		{
			itb = vTmp.erase(itb);
		}
		else
			itb++;
	}
	_vCard = vTmp;
	return true;
}

bool CardArrayBase::erase(const CardBase& c)
{
	for (auto it = _vCard.begin();it != _vCard.end();)
	{
		if (*it == c)
		{
			it = _vCard.erase(it);
		}
		else
			it++;
	}
	return false;
}


bool CardArrayBase::push_back(const CardBase& c)
{
	if(!c.isValid()) return false;
	_vCard.push_back(c);
	return true;
}

bool CardArrayBase::pop_back()
{
	if(_vCard.empty()) return false;
	_vCard.pop_back();
	return true;
}

bool CardArrayBase::add(const CardArrayBase&v)
{
	vector<CardBase> tmp = _vCard;
	for (auto it = v.getArray().begin();it != v.getArray().end();it++)
	{
		if(!it->isValid()) return false;
		tmp.push_back(*it);
	}
	_vCard = tmp;
	return true;
}

void CardArrayBase::addOnePairCard()
{
	_vCard.insert(_vCard.end(),one_pair_card,one_pair_card + sizeof(one_pair_card));
}

CardBase CardArrayBase::randGetCard(size_t t)
{
	//to test
	CardBase card;
	if(Size() < t) return card;
	map<CardBase,size_t> mcard;
	for (auto it = _vCard.begin();
		it != _vCard.end();it++)
	{
		if(it->isValid() )
			mcard[*it]++;
	}

	for (auto it = mcard.begin();it != mcard.end();)
	{
		if(it->second != 1)	it = mcard.erase(it);
		else				it++;
	}
	if (0 == mcard.size()) return card;
	size_t t = Algor::rand_Mersenne(0,mcard.size()-1);
	int i = 0;
	for (auto it = mcard.begin();
		it != mcard.end();i++,it++)
	{
		if(t == i)
		{
			card = it->first;
			break;
		}
	}
	return card;
}


em_CmpResult CardArrayBase::compareTo(const CardArrayBase&other)const
{
	em_CmpResult res = e_cmp_error;

	return res;
}

bool CardArrayBase::isSingle(Param &p)const
{
	p.clear();
	if(!_isSingle()) return false;
	p.val = _vCard[0].getRealVal();
	p.bType = ARRAY_TYPE_SINGLE;
	p.bCardColor = _vCard[0].getType();
	p.size = Size();
	return true;
}

bool CardArrayBase::isDouble(Param &p)const
{
	p.clear();
	bool flag = _isSameVal() && 2 == Size()	&& _vCard[0].getVal() <= VAL_A;
	if(!flag) return false;
	p.val = _vCard[0].getRealVal();
	p.bCardColor =(max(_vCard[0].getData(),_vCard[1].getData())>>4);
	p.bType = ARRAY_TYPE_DOUBLE;
	p.size = Size();
	return true;
}

bool CardArrayBase::isThree(Param &p)const
{
	p.clear();
	bool flag = _isSameVal() && 3 == Size()	&& _vCard[0].getVal() <= VAL_A;
	if(!flag) return false;
	p.val = _vCard[0].getRealVal();
	p.bType = ARRAY_TYPE_3;
	p.bCardColor = _vCard[0].getType();
	p.size = Size();
	return true;
}

bool CardArrayBase::is3W2(Param &p)const
{
	p.clear();
	if (5 !=Size()) return false;
	if(!is3W_(p.val)) return false;
	p.bType = ARRAY_TYPE_3W2;
	std::for_each(_vCard.begin(),_vCard.end(),[&](CardBase it)
	{
		if (it.getRealVal()==p.val)
		{
			p.bCardColor=it.getType();
		}
	});
	p.size = Size();
	return true;
}

bool CardArrayBase::is4W1(Param &p)const
{
	p.clear();
	if (5 !=Size()) return false;
	if(!is4W_(p.val)) return false;
	p.bType = ARRAY_TYPE_4W1;
	std::for_each(_vCard.begin(),_vCard.end(),[&](CardBase it)
	{
		if (it.getRealVal()==p.val)
		{
			p.bCardColor=it.getType();
		}
	});
	p.size = Size();
	return true;
}

bool CardArrayBase::is4W_(uval &v)const
{
	map<uval,int> mc;
	for(auto it = _vCard.begin();it != _vCard.end();it++)
	{
		if(!it->isValid())return false;
		mc[it->getRealVal()]++;	
	}
	if (mc.size()!=2) return false;
	for (auto it = mc.begin();it != mc.end();it ++)
	{
		if (4 != it->second && 1!=it->second) 
		{
			return false;
		}
		if (4 == it->second)
		{
			v=it->first;
		}
	}
	return true;
}

bool CardArrayBase::is3W_(uval &v)const
{
	if (5 < Size()) return false;
	map<uval,int> mc;
	for(auto it = _vCard.begin();it != _vCard.end();it++)
	{
		if(!it->isValid())return false;
		mc[it->getRealVal()]++;	
	}
	if (mc.size()!=2) return false;
	for (auto it = mc.begin();it != mc.end();it ++)
	{
		if (3 != it->second && 2!=it->second) 
		{
			return false;
		}
		if (3 == it->second)
		{
			v=it->first;
		}
	}
	return true;
}

bool CardArrayBase::isSameSeq(Param &p)const
{
	p.clear();
	if (ARRAY_TYPE_SEQ!=Size()) return false;
	if(!_isSameSeq(p.val,p.bCardColor)) return false;
	p.bType = ARRAY_TYPE_SAMESEQ;
	p.size = Size();
	return true;
}

bool CardArrayBase::isSequence(Param &p)const
{
	p.clear();
	if (ARRAY_TYPE_SEQ!=Size()) return false;
	if(!_isSequence(p.val,p.bCardColor)) return false;
	p.bType = ARRAY_TYPE_SEQ;
	p.size = Size();
	return true;
}

bool CardArrayBase::isSameColor(Param &p)const
{
	p.clear();
	if (ARRAY_TYPE_SEQ!=Size()) return false;
	if(!_isSameType()) return false;
	p.val = _vCard.back().getRealVal();
	std::for_each(_vCard.begin(),_vCard.end(),[&](CardBase it)
	{
		if (it.getRealVal()>p.val)
		{
			p.val = it.getRealVal();
		}
	});
	p.bType = ARRAY_TYPE_SAMET;
	p.bCardColor = _vCard.back().getType();
	p.size = Size();
	return true;
}

bool CardArrayBase::isSeq_2(Param &p)const
{
	

	return true;
}

bool CardArrayBase::isSeq_3(Param &p)const
{
	p.clear();
	if(MIN_COUNT_OF_SEQ3 *3 >Size()) return false;
	if(!isSeq_N(p.val,3))return false;
	p.bType = ARRAY_TYPE_SEQ3;
	p.size = Size();
	return true;
}

bool CardArrayBase::isSeq_N(uval &v,size_t n)const
{

	return true;
}

bool CardArrayBase::is510K_N(Param & p,size_t n)const
{
	
	return true;
}



bool CardArrayBase::isBomb_N(Param &p)const
{
	
	return true;
}

bool CardArrayBase::isBombWithJoker_N(Param &p)const
{

	return true;
}

bool CardArrayBase::isJokers(Param &p,size_t n)const
{
	p.clear();
	if(n < MIN_COUNT_OF_JOKERS) return false;
	if(n != Size()) return false;
	for(auto it = _vCard.begin();it != _vCard.end();it++)
	{
		if(!it->isValid())continue;
		if (it->getData() != SJ && it->getData() != BJ)
		{
			return false;
		}
	}
	p.bType = ARRAY_TYPE_JOKER_4;
	p.size = Size();
	p.bBomb = true;
	p.bJokers = true;
	p.val = VAL_SJ;
	return true;
}

bool CardArrayBase::isPlane(Param &p)const
{
	p.clear();
	if (Size()<10 || 0 != Size() %5) return false;
	if(!isPlane_(p.val)) return false;
	p.bType = ARRAY_TYPE_PLANE;
	p.size = Size();
	return true;
}

bool CardArrayBase::isPlane_(uval &v)const
{
	v = VAL_E;
	if(Size() < 6) return false;
	size_t n = (0 == Size() % 5)?Size()/5:(Size()/5+1);
	if(n < 2) return false;
	CardArrayBase tmp = _vCard;
	CardArrayBase vp;

	if (!tmp.canPickUp_Seq_N(vp,VAL_3,3,n)) return false;
	if(vp.Size()== 0)return false;
	vp.Sort();
	v = vp.getArray().begin()->getRealVal();
	do 
	{
		if (!tmp.canPickUp_Seq_N(vp, vp.getArray().begin()->getVal() + 1,3,n)) break;
		if(vp.Size()== 0)break;
		vp.Sort();
		v = vp.getArray().begin()->getRealVal();
	} while (true);

	return true;
}

bool CardArrayBase::canPickUp_Bomb_N(CardArrayBase&v,uval vBegin,size_t w)
{
	CardArrayBase vTmp = _vCard;
	vTmp.erase(SJ);
	vTmp.erase(BJ);
	return vTmp.canPickUp_W_L(v,vBegin,w,1);
}

bool CardArrayBase::canPickUp_Jokers_N(CardArrayBase&V,size_t w)
{
	V.Clear();
	for(auto it = _vCard.begin();it != _vCard.end();it++)
	{
		if(!it->isValid()) continue;
		if(V.Size() == w) break;
		if(it->getVal() == VAL_BJ||it->getVal() == VAL_SJ)
		{
			V.push_back(*it);
		}
	}
	if(V.Size() < w)
	{
		V.Clear();
		return false;
	}
	return true;
}

bool CardArrayBase::pickUpByCardVal(CardArrayBase&V,vector<uval> vals)
{
	V.Clear();
	for(auto it = _vCard.begin();it != _vCard.end();it++)
	{
		if(find(vals.begin(),vals.end(),it->getVal()) != vals.end())
		{
			V.push_back(*it);
		}
	}
	return !V.Empty();
}

bool CardArrayBase::canPickUp_Seq_N(CardArrayBase&v,uval vBegin,size_t w, size_t l)
{																																																																	
	CardArrayBase vTmp = _vCard;
	CardArrayBase vPass;
	vPass.push_back(SJ);
	vPass.push_back(BJ);
	vPass.push_back(S_2);
	vPass.push_back(D_2);
	vPass.push_back(H_2);
	vPass.push_back(C_2);
	vTmp.erase(vPass);
	return vTmp.canPickUp_W_L(v,vBegin,w,l);
}

//取数列 w 表示每张牌的个数，表示长度
bool CardArrayBase::canPickUp_W_L(CardArrayBase&V,uval vBegin, size_t w,size_t l)
{

	return true;
}


void CardArrayBase::promptD_4Type(uchar (*pCard)[ONE_HAND_CARD_COUNT],uchar& CardCount)
{
	
}

void CardArrayBase::promptType(uchar (*pCard)[ONE_HAND_CARD_COUNT],uchar& CardCount)
{

}

void CardArrayBase::promptType(CardArrayBase &lastCard,uchar (*pCard)[ONE_HAND_CARD_COUNT],uchar& CardCount)
{
	
	



}


void CardArrayBase::canPickComposes(vector<CardArrayBase> &v,uval vSrc,size_t w)
{
	
}


void CardArrayBase::canPickD_4Compose(vector<CardArrayBase> &v,uval vSrc,size_t w)
{
	

}

void CardArrayBase::_3w2Or4w1PickD_4Compose(vector<CardArrayBase> &v,uval vSrc,size_t w)
{
	

}

void CardArrayBase::_3w2Or4w1D_4Compose(vector<CardArrayBase> &v,size_t w)
{
	

}
void CardArrayBase::seqPickD_4Compose(vector<CardArrayBase> &v,uval vSrc,size_t w)
{
	
}


void CardArrayBase::seqPickComposes(vector<CardArrayBase> &v,size_t w)
{

}


void CardArrayBase::samePickD_4Compose(vector<CardArrayBase> &v,uval vSrc,size_t w)
{
	
}

void CardArrayBase::samePickCompose(vector<CardArrayBase> &v,uval vSrc,size_t w)
{


}


void CardArrayBase::sameSeqPickD_4Compose(vector<CardArrayBase> &v,uval vSrc,size_t w)
{
	
}

void CardArrayBase::sameSeqPickCompose(vector<CardArrayBase> &v,uval vSrc,size_t w)
{
	
}


bool CardArrayBase::classifyByType(vector<CardArrayBase>&v)
{
	
	return true;
}

bool CardArrayBase::canPickUp_3W2(CardArrayBase&v,uval vBegin,bool full)
{
	v.Clear();

	return false;
}

bool CardArrayBase::canPickUp_Plane(CardArrayBase&v,uval vBegin,bool full)
{

	return true;
}

bool CardArrayBase::canPickUp_WSQ_N(CardArrayBase&V,size_t w)
{

	return true;
}

bool CardArrayBase::canPickUp_WSQ_T(CardArrayBase&V,emCardColor type)
{
	V.Clear();

	return true;
}

bool CardArrayBase::canPickUpBombs(vector<CardArrayBase>&v)
{
	
	return true;
}

bool CardArrayBase::canPickUp(eArrayType type,vector<CardArrayBase>&v)
{


	return true;
}

bool CardArrayBase::_isSameVal()const
{
	auto begin = _vCard.begin();
	for (auto i = begin;i != _vCard.end();i++)
	{
		if (i->getVal() != begin->getVal())
		{
			return false;
		}
	}
	return true;
}

bool CardArrayBase::_isSameType()const
{
	auto begin = _vCard.begin();
	for (auto i = begin;i != _vCard.end();i++)
	{
		if (i->getType() != begin->getType())
		{
			return false;
		}
	}
	return true;
}

bool CardArrayBase::_isSame()const
{
	auto begin = _vCard.begin();
	for (auto i = begin;i != _vCard.end();i++)
	{
		if (i->getData() != begin->getData())
		{
			return false;
		}
	}
	return true;
}

bool CardArrayBase::_isSequence(uval & v,char &t)const
{
	auto vcard = _vCard;
	std::sort(vcard.begin(),vcard.end());
	if (Size() <= 1)	return false;
	auto left = vcard.begin();
	v = vcard.back().getSeqRealVal();
	t = vcard.back().getType();
	for (auto i = left + 1;i != vcard.end();i++)
	{
		if (i->getSeqRealVal()  !=  left->getSeqRealVal() + 1
			|| false == i->canMakeSeq(bySeq) )
		{
			return false;
		}
		left = i;
	}
	return true;
}

bool CardArrayBase::_isSameSeq(uval& v,char &t)const
{
	return _isSameType() && _isSequence(v,t);
}

bool CardArrayBase::_isSingle()const
{
	return Size() == 1;
}