/****************************************************************************
Copyright (c) 2014-2017 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2017 ShenZhen Redbird Network Polytron Technologies Inc.
 
http://www.hotniao.com

All of the content of the software, including code, pictures, 
resources, are original. For unauthorized users, the company 
reserves the right to pursue its legal liability.
****************************************************************************/
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             #include "CardArrayBase.h"
#include "../util/algorithm/algorithm.h"

#define CARD_PAIRS						1				//几副牌
#define MAX_CARD_COUNT					54*CARD_PAIRS	//牌的个数

namespace HN
{
	byte cardArr [] = 
	{ 
		CardBase::D_2 , CardBase::C_2 ,CardBase::H_2 ,CardBase::S_2 ,
		CardBase::D_3 , CardBase::C_3 ,CardBase::H_3 ,CardBase::S_3 ,
		CardBase::D_4 , CardBase::C_4 ,CardBase::H_4 ,CardBase::S_4 ,
		CardBase::D_5 , CardBase::C_5 ,CardBase::H_5 ,CardBase::S_5 ,
		CardBase::D_6 , CardBase::C_6 ,CardBase::H_6 ,CardBase::S_6 ,
		CardBase::D_7 , CardBase::C_7 ,CardBase::H_7 ,CardBase::S_7 ,
		CardBase::D_8 , CardBase::C_8 ,CardBase::H_8 ,CardBase::S_8 ,
		CardBase::D_9 , CardBase::C_9 ,CardBase::H_9 ,CardBase::S_9 ,
		CardBase::D_10, CardBase::C_10,CardBase::H_10,CardBase::S_10,
		CardBase::D_J , CardBase::C_J ,CardBase::H_J ,CardBase::S_J ,
		CardBase::D_Q , CardBase::C_Q ,CardBase::H_Q ,CardBase::S_Q ,
		CardBase::D_K , CardBase::C_K ,CardBase::H_K ,CardBase::S_K ,
		CardBase::D_A , CardBase::C_A ,CardBase::H_A ,CardBase::S_A ,
		CardBase::SJ  , CardBase::BJ
	};

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

	void CardArrayBase::randCard(size_t iNumber)
	{
		if( iNumber <= 0 || iNumber >= Size() )
		{
		    return;
		}
		size_t iAllCount = Size()/iNumber;
		for (size_t i = 0; i < iAllCount; i++)
		{
			size_t iStart = Algor::rand_Mersenne(0,Size()- iNumber);
			vector<CardBase> tCardList;
			for( size_t j = 0; j < iNumber; j++ )
			{
				if( (_vCard.begin()+iStart) == _vCard.end() )
				{
				    break;
				}
				tCardList.push_back(_vCard[iStart]);
			    _vCard.erase(_vCard.begin()+iStart);
			}
			if( iStart%2==0 )
			{
				_vCard.insert(_vCard.end(),tCardList.begin(),tCardList.end());
			}
			else
			{
			   _vCard.insert(_vCard.begin(),tCardList.begin(),tCardList.end());
			}
		}
	}

	void CardArrayBase::fillBomb(size_t iMaxNumber)
	{
		if( iMaxNumber <= 0 || Size() != MAX_CARD_COUNT )     ///只有是一副牌的时候才可以补充炸弹
		{
			return;
		}
		//// 随机需要填补的炸弹
		size_t iNumber = Algor::rand_Mersenne(0,iMaxNumber);
		for(int i=0;i<iNumber;i++)
		{
		    size_t tIndex = Algor::rand_Mersenne(0,Size()-1);
			vector<CardBase> tCardList;
			vector<CardBase> tBombCard;
			CardBase iBombCard = _vCard[tIndex];
			if( iBombCard.getType() == CardBase::TYPE_JOKER )
			{
			    continue;
			}
			/// 获取出炸弹牌
			for(auto iter = _vCard.begin();iter!=_vCard.end();iter++)
			{
				if( iter->getVal() == iBombCard.getVal() )
				{
					tBombCard.push_back(*iter);
				}
			}
			/// 删除炸弹
			for(auto iter = _vCard.begin();iter!=_vCard.end();iter++)
			{
				if( iter->getData() == iBombCard.getData() )
				{
				    tCardList.insert(tCardList.end(),tBombCard.begin(),tBombCard.end());
				}
				else if( iter->getVal() != iBombCard.getVal() )
				{
				    tCardList.push_back(*iter);
				}
			}
			_vCard = tCardList;
		}
	}

	void CardArrayBase::Sort()
	{
		std::sort(_vCard.begin(),_vCard.end());      ///  从小打大
		std::reverse(_vCard.begin( ),_vCard.end( )); ///  从大到小
	}
	
	void CardArrayBase::Sort(CardBase tianLaiZi,CardBase diLaiZi)
	{
		std::sort(_vCard.begin(),_vCard.end());
		vector<CardBase> tVLaiZiCard;
		/// 获取出癞子牌
		for(auto iter = _vCard.begin();iter != _vCard.end();)
		{
			if( iter->getVal() == tianLaiZi.getVal() || iter->getVal() == diLaiZi.getVal()  )
			{
				tVLaiZiCard.push_back(*iter);
				iter = _vCard.erase(iter);
			}
			else
			{
				iter++;
			}
		}
		/// 排序癞子牌
		std::sort(tVLaiZiCard.begin(),tVLaiZiCard.end());
		/// 手牌添加回癞子牌
		_vCard.insert(_vCard.end(),tVLaiZiCard.begin(),tVLaiZiCard.end());
		std::reverse(_vCard.begin( ),_vCard.end( )); //// 从大到小
	}

	size_t CardArrayBase::reAssign(ucard *arrCard, size_t len) const
	{
		for (size_t i = 0; i < len && i < Size(); i++)
		{
			arrCard[i] = _vCard.at(i).getData();
		}
		return min(len,Size());
	}

	size_t CardArrayBase::reAssignVal(ucard *arrCard, size_t len) const
	{
		for (size_t i = 0; i < len && i < Size(); i++)
		{
			arrCard[i] = _vCard.at(i).getVal();
		}
		return min(len,Size());
	}

	void CardArrayBase::assign(ucard *arrCard, size_t len)
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

	bool CardArrayBase::contain(const CardBase &c)const
	{
		return _vCard.end() != find(_vCard.begin(),_vCard.end(),c);
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
		_vCard.insert(_vCard.end(),cardArr,
			cardArr + sizeof(cardArr)/sizeof(cardArr[0]));
	}

	bool CardArrayBase::getCardAllType(CardBase byCard) const
	{
		std::vector<CardBase> vecData;
		int iLen = getCardAllType(byCard,vecData);
		return iLen > 0;
	}

	int CardArrayBase::getCardAllType(CardBase byCard,std::vector<CardBase> & vecData) const
	{
		int iNumber = 0;
		for (size_t i = 0;i < Size(); i++)
		{
			if( byCard.getVal() == _vCard.at(i).getVal() )   
			{
				iNumber++;
				vecData.push_back(_vCard.at(i));
				continue;
			}
		}
		return iNumber;
	}

}