#include "CfgCard.h"
#include <random>
#include <algorithm>
#include <map>
#include <String>
#include "../Server/STDAFX.H"

using namespace std;
namespace HN
{

	CfgCard::CfgCard(void)
	{
	}


	CfgCard::~CfgCard(void)
	{
	}

	ucard CfgCard::_baseCard[PAIR_CARD_NUM]  =
	{
		0x31,
		0x02, 0x12, 0x22, 0x32,
		0x03, 0x13, 0x23, 0x33,
		0x04, 0x14, 0x24, 0x34,
		0x05, 0x15, 0x25, 0x35,
		0x06, 0x16, 0x26, 0x36,
		0x07, 0x17, 0x27, 0x37,
		0x08, 0x18, 0x28, 0x38,
		0x09, 0x19, 0x29, 0x39,
		0x0A, 0x1A, 0x2A, 0x3A,
		0x0B, 0x1B, 0x2B, 0x3B,
		0x0C, 0x1C, 0x2C, 0x3C,
		0x0D, 0x1D, 0x2D,
	};



	pMakeCard CfgCard::_makeCard [] = 
	{
		&makeCard0	,
	};


	bool CfgCard::makeCard0(vCard &card)
	{
		ucard uTop1[] = {0x31,0x0D, 0x1D, 0x2D};
		ucard uTop2[] = {
							0x08, 0x18, 0x28, 0x38,
							0x09, 0x19, 0x29, 0x39,
							0x0A, 0x1A, 0x2A, 0x3A,
							0x0B, 0x1B, 0x2B, 0x3B,
							0x0C, 0x1C, 0x2C, 0x3C
						};
		vCard card1,card2;
		card1.assign(uTop1,uTop1 + sizeof(uTop1)/sizeof(uTop1[0]));
		card2.assign(uTop2,uTop2 + sizeof(uTop2)/sizeof(uTop2[0]));

		randSel(card1,card,rand_Mersenne(2,4));

		if (0 == rand_Mersenne(0,1))
		{
			vCard bomb;
			randSelBomb(bomb,1);
			Remove(card2,bomb);
			Add(card,bomb);
			randSel(card2,card,rand_Mersenne(7,10) - card.size());
		}
		else
		{
			randSel(card2,card,rand_Mersenne(11,14) - card.size());
		}	
		return true;
	}

	bool CfgCard::makeCard(vCard & card,int typeNum)
	{
		if (typeNum < 0 || typeNum >= MAX_CARD_TYPE_NUM)
		{
			return false;
		}

		card.clear();
		//组牌前初始化
		preMakeCard();
		//组特殊牌
		bool bSuc =  (this->*(_makeCard[typeNum]))(card);
		//组剩余牌
		nxtMakeCard(card);
		return bSuc;
	}

	void CfgCard::preMakeCard()
	{
		_tCard.clear();
		_tCard.assign(_baseCard,_baseCard + PAIR_CARD_NUM);
		_SelCard.clear();
	}

	void CfgCard::nxtMakeCard(vCard & card)
	{
		Remove(_tCard,card);
		randCard(_tCard);
		_SelCard.assign(_tCard.begin(),_tCard.begin() + HAND_CARD_NUM - card.size());
		Add(card,_SelCard);
		Remove(_tCard,card);
		_SelCard.clear();
	}

	bool CfgCard::nxtSel(vCard & nxtCard,size_t len)
	{
		if (len > _tCard.size())
		{
			return false;
		}
		nxtCard.clear();
		nxtCard.assign(_tCard.begin(),_tCard.begin() + len);
		Remove(_tCard,nxtCard);
	}


	

	int CfgCard::rand_Mersenne(const int _Min,const int _Max)
	{
		if (_Min > _Max){ return 0;}
		if (_MSC_VER < 1500 ){ return 0;}
		random_device rd;
		mt19937 mt(rd());
		uniform_int_distribution<> dis(_Min,_Max);
		return dis(mt);
	}

	void CfgCard::randCard(vCard& card)
	{
		ucard tmp = 0;
		for (size_t i = 0;i < card.size();i++)
		{
			size_t index = rand_Mersenne(0,i);
			tmp = card[i];
			card[i] = card[index];
			card[index] = tmp;
		}
	}

	void CfgCard::Remove(vCard& src,const vCard& v)
	{
		for (size_t i = 0;i < v.size();i++)
		{
			auto it = find(src.begin(),src.end(),v.at(i));
			if (it != src.end())
			{
				src.erase(it);
			}
		}
	}

	void CfgCard::Add(vCard& src,const vCard& v)
	{
		for (size_t i = 0;i < v.size();i++)
		{
			src.push_back(v.at(i));
		}
	}

	void CfgCard::randSel(const vCard&srcCard,vCard& dst,size_t len)
	{
		if (len > srcCard.size())
		{
			return;
		}
		vCard t;
		t.clear();
		vCard tCard = srcCard;
		while(t.size() < len)
		{
			int index = rand_Mersenne(0,tCard.size()-1);
			t.push_back(tCard.at(index));
			auto it = find(tCard.begin(),tCard.end(),tCard.at(index));
			tCard.erase(it);
		}
		Add(dst,t);
	}

	void CfgCard::randSelVal(vCard& card,ucard uVal,size_t len)
	{
		if (len > 4)
		{
			return ;
		}
		vCard v;
		v.push_back(0x00 + uVal);
		v.push_back(0x10 + uVal);
		v.push_back(0x20 + uVal);
		v.push_back(0x30 + uVal);
		if (4 == len)
			Add(card,v);
		else
			randSel(v,card,len);
	}

	void CfgCard::randSelBomb(vCard& card,size_t len)
	{
		if (len > 0x0D)
		{
			return ;
		}
		ucard u[] = {0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C};
		vCard v,vd;	
		v.assign(u,u + sizeof(u));
		randSel(v,vd,len);

		if (vd.size() != len)
		{
			return;
		}

		for (size_t i = 0;i < vd.size();i++)
		{
			card.push_back(0X00 + vd.at(i));
			card.push_back(0X10 + vd.at(i));
			card.push_back(0X20 + vd.at(i));
			card.push_back(0X30 + vd.at(i));
		}
	}

	void CfgCard::reassign(const vCard& v,ucard *uCard,size_t len)
	{
		for (size_t i = 0;i < len && i < v.size();i++)
		{
			uCard[i] = v.at(i);
		}
	}
}

// 玩家已经抽了多少张卡
std::map<long int, int> UserDrawnCardCount;// 玩家已经抽了多少张卡

// 修改指定玩家的牌
bool GetSpecifiedCard(long int userID, unsigned char *Card,int jCardNum)
{
	CString s = CINIFile::GetAppPath ();/////本地路径
	CString nid;
	nid.Format("%d",NAME_ID);
	CINIFile f(s + nid  + _T("_s.ini"));
	CString key;
	key = TEXT("SuperSet");
	int setcard = f.GetKeyVal(key,"SetCard",0);
	//todo 
	if (setcard <= 0)
	{
		return false;
	}
	if (jCardNum == 0)
	{
		UserDrawnCardCount[userID] = 0;
	}


	if (1 == setcard)
	{
		char nameBuf[256];
		sprintf(nameBuf, "Card_%d", userID);
		CString cardListStr = f.GetKeyVal(key, nameBuf, "");

		unsigned char ret = Card[jCardNum];
		int cnt = UserDrawnCardCount[userID];
		UserDrawnCardCount[userID] = cnt + 1;

		if (cardListStr.GetLength() > 0)
		{
			int foundPos = 0;
			int lastPos = 0;
			int i = 0;
			while (i <= cnt && foundPos >= 0)
			{
				++i;
				foundPos = cardListStr.Find(',', foundPos);
				if (i <= cnt && foundPos >= 0)
				{
					foundPos += 1;
					lastPos = foundPos;
				}
			}

			if (i > cnt)
			{
				// 存在第i张牌
				if (foundPos < 0)
				{
					// 第i张牌是最后一张(没有以逗号结束的)
					foundPos = cardListStr.GetLength();
				}
				ret = _ttoi(cardListStr.Mid(lastPos, foundPos - lastPos));
			}
		}
		if (ret != Card[jCardNum])
		{
			Card[jCardNum] = ret;
			return true;
		}
	}

	//else if (2 == setcard)
	//{
	//	int cardGroupSum = f.GetKeyVal(key, "cardGroup", -1);
	//	if (cardGroupSum < 1) return false;

	//	//todo 换成梅林的来获取随机数
	//	srand((unsigned int)time(0));
	//	int randnum = rand() % cardGroupSum;


	//}

	return false;
}