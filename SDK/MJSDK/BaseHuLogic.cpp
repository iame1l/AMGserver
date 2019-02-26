#include "stdafx.h"
#include "BaseHuLogic.h"
#include "BaseMJLogic.h"

//构造函数		
CBaseHuLogic::CBaseHuLogic(void)
{

}
//析构函数
CBaseHuLogic::~CBaseHuLogic()
{

}

//添加番型数据
bool CBaseHuLogic::AddFanInfo(BYTE uKindID, TCHAR text[], BYTE uFanValue[], BYTE uHuAllowKind) 
{
	MJ_HU_FAN cmd;
	wsprintf(cmd.Name, "%s", text);
	cmd.uHuAllowKind = uHuAllowKind;
	cmd.uFanValue[0] = uFanValue[0];
	cmd.uFanValue[1] = uFanValue[1];
	cmd.uKindID = uKindID;
	m_FanBaseInfo.insert(map<BYTE, MJ_HU_FAN>::value_type(uKindID, cmd));

	return true;
}

//判断该手牌是否胡了
bool CBaseHuLogic::CheckHu(BYTE bDesk, UDT_MAP_MJ_DATA card)
{
	UDT_MAP_MJ_DATA::iterator data;
	//先处理手牌中的万能牌
	if (GetBase()->m_WildCardInfo.bWildCard)
	{
		BYTE uWildCardNum = 0;//该组牌中万能牌数目
		uWildCardNum = GetBase()->GetSameCardNum(card, GetBase()->m_WildCardInfo.uCard);
		if (uWildCardNum > 0)
		{
			UDT_MAP_MJ_DATA middAllCard;
			middAllCard = GetBase()->m_MJDataMap;
			SetAloneCard(middAllCard);//去掉重复牌
			//每一个都得换掉
			for (data=middAllCard.begin();data!=middAllCard.end();++data)
			{
				GetBase()->ChangeCard(card, GetBase()->m_WildCardInfo.uCard, data->second);
				//MyDebugString("sdp_xmmj", "s 先处理手牌中的万能牌bDesk=%d card1=%d 被替换为 card2=%d", bDesk, GetBase()->m_WildCardInfo.uCard, data->second);
				//每次换好就判断是否可以胡
				if (CheckHuEx(bDesk, card))
				{
					return true;
				}
				else if (uWildCardNum > 1)//还有就迭代处理
				{
					if (CheckHu(bDesk, card))
					{
						return true;
					}
				}
			}
		}
		else
		{
			if (CheckHuEx(bDesk, card))
			{
				return true;
			}
		}
	}

	return false;
}

//判断该手牌是否胡了
bool CBaseHuLogic::CheckHuEx(BYTE bDesk, UDT_MAP_MJ_DATA card)
{
	//初始化胡牌结构体
	GetBase()->m_UserResultInfo[bDesk].init();

	UDT_MAP_MJ_DATA::iterator data;
	//for (data=card.begin(); data != card.end(); data++)
	//{
	//	MyDebugString("sdp_xmmj", "s 判断该手牌是否胡了 玩家%d 手牌数据 %d,%d", bDesk, data->first, data->second);
	//}
	bool btag=false;
	//普通牌型
	if (CheckNormalHu(bDesk, card))
	{
		btag = true;
	}

	//特殊牌型
	//..........
	//if()
	//{
	// btag=true;
	//}

	if (!btag)
	{
		return false;
	}

	GetBase()->m_bIsQiangGangHu[bDesk] = false;
	//番型判断
	if (!CheckFanNode(bDesk, card))
	{
		return false;
	}

	return true;
}

//判断该手牌是否胡普通牌型(一对将和几组刻子的牌)
bool CBaseHuLogic::CheckNormalHu(BYTE bDesk, UDT_MAP_MJ_DATA card)
{
	UDT_MAP_MJ_DATA::iterator data;

	//先找一对将牌
	//找的时候很有可能存在多组可以为将牌的两张
	map<BYTE, BYTE>CandBeJiangPai;//前一个是什么牌，后一个是记录牌张数
	GetBase()->SortMJ(card);
	UDT_MAP_MJ_DATA MiddCard;//临时存储牌数据变量
	UDT_MAP_MJ_DATA MiddCard2;//临时存储牌数据变量
	for (data=card.begin();data!=card.end();++data)
	{
		BYTE unum = GetBase()->GetSameCardNum(card, data->second);
		if (unum>=2 && CandBeJiangPai.find(data->second) == CandBeJiangPai.end())//张数大于等于2且还没有存储
		{
			CandBeJiangPai.insert(map<BYTE, BYTE>::value_type(data->second, unum));
		}
	}

	if (CandBeJiangPai.size() == 0)//连将牌都没有，不能胡
	{
		//MyDebugString("sdp_xmmj", "s 判断该手牌是否胡普通牌型(一对将和几组刻子的牌) error 1");
		return false;
	}

	//一对将牌就是一种情况，一个一个判断
	map<BYTE, BYTE>::iterator JiangData;
	for (JiangData=CandBeJiangPai.begin(); JiangData!=CandBeJiangPai.end();++JiangData)
	{
		MiddCard = card;
		MiddCard2 = card;
		//删掉两张
		GetBase()->DeleteCard(MiddCard, JiangData->first);
		GetBase()->DeleteCard(MiddCard, JiangData->first);

		//MyDebugString("sdp_xmmj", "s 判断该手牌是否胡普通牌型(一对将和几组刻子的牌) 此时 将牌是 %d", JiangData->first);
		map<BYTE, BYTE> SameCard;//前一个是什么牌，后一个是记录牌张数
		map<BYTE, BYTE>::const_iterator samecarddata;
		DeleteKeZi(MiddCard, SameCard);
		if (MiddCard.size() == 0)//经过处理没有牌了
		{
			GetBase()->m_bJiangCard[bDesk] = JiangData->first;

			for (samecarddata = SameCard.begin();samecarddata!=SameCard.end();++samecarddata)
			{
				if (GetBase()->m_UserResultInfo[bDesk].uKeNum < MESS_KE_NUM && samecarddata->second == 3)//暗刻
				{
					GetBase()->m_UserResultInfo[bDesk].uKe[GetBase()->m_UserResultInfo[bDesk].uKeNum++] = samecarddata->first;
				}
				if (GetBase()->m_UserResultInfo[bDesk].uGangNum < MESS_GANG_NUM && samecarddata->second == 4)//暗杠
				{
					GetBase()->m_UserResultInfo[bDesk].uGang[GetBase()->m_UserResultInfo[bDesk].uGangNum++].udata = samecarddata->first;
					GetBase()->m_UserResultInfo[bDesk].uGang[GetBase()->m_UserResultInfo[bDesk].uGangNum].bDeskStation = bDesk;
					GetBase()->m_UserResultInfo[bDesk].uGang[GetBase()->m_UserResultInfo[bDesk].uGangNum].bkind = false;
				}
			}

			return true;
		}
		//运行两次，上次优先考虑顺子，这次优先考虑三张四张的
		SameCard.clear();
		DeleteKeZiEx(MiddCard2, SameCard);
		if (MiddCard2.size() == 0)//经过处理没有牌了
		{
			GetBase()->m_bJiangCard[bDesk] = JiangData->first;
			return true;
		}
	}

	//MyDebugString("sdp_xmmj", "s 判断该手牌是否胡普通牌型(一对将和几组刻子的牌) error 2");
	return false;
}

//会改变牌数据，删除所有刻子(三个相同的 或者 连续的三张 或者 暗杠)
bool CBaseHuLogic::DeleteKeZi(UDT_MAP_MJ_DATA &card, map<BYTE, BYTE> &SameCard)
{
	//再判断连续的三张
	UDT_MAP_MJ_DATA ResultCard;
	GetShunZi(card, ResultCard);

	GetBase()->SortMJ(card);
	//先删除三个或四个相同的牌
	SameCard.clear();
	UDT_MAP_MJ_DATA::iterator data;
	for (data=card.begin();data!=card.end();++data)
	{
		BYTE unum = GetBase()->GetSameCardNum(card, data->second);
		if (unum>=3 && SameCard.find(data->second) == SameCard.end())//张数大于等于3且还没有存储
		{
			SameCard.insert(map<BYTE, BYTE>::value_type(data->second, unum));
		}
	}

	map<BYTE, BYTE>::iterator SameCardData;
	for (SameCardData=SameCard.begin(); SameCardData!=SameCard.end();++SameCardData)
	{
		//删掉
		for (int i=0;i<SameCardData->second;++i)
		{
			GetBase()->DeleteCard(card, SameCardData->first);
		}
	}

	return true;
}

//会改变牌数据，删除所有刻子(三个相同的 或者 连续的三张 或者 暗杠)
bool CBaseHuLogic::DeleteKeZiEx(UDT_MAP_MJ_DATA &card, map<BYTE, BYTE> &SameCard)
{
	GetBase()->SortMJ(card);

	//先删除三个或四个相同的牌
	SameCard.clear();
	UDT_MAP_MJ_DATA::iterator data;
	for (data=card.begin();data!=card.end();++data)
	{
		BYTE unum = GetBase()->GetSameCardNum(card, data->second);
		if (unum>=3 && SameCard.find(data->second) == SameCard.end())//张数大于等于3且还没有存储
		{
			SameCard.insert(map<BYTE, BYTE>::value_type(data->second, unum));
		}
	}

	map<BYTE, BYTE>::iterator SameCardData;
	for (SameCardData=SameCard.begin(); SameCardData!=SameCard.end();++SameCardData)
	{
		//删掉
		for (int i=0;i<SameCardData->second;++i)
		{
			GetBase()->DeleteCard(card, SameCardData->first);
		}
	}


	//再判断连续的三张
	UDT_MAP_MJ_DATA ResultCard;
	GetShunZi(card, ResultCard);

	return true;
}

//提取连续的三张牌
bool CBaseHuLogic::GetShunZi(UDT_MAP_MJ_DATA &card, UDT_MAP_MJ_DATA &ResultCard)
{	
	GetBase()->SortMJ(card);
	UDT_MAP_MJ_DATA card2;
	card2 = card;
	ResultCard.clear();
	BYTE num=card.size();
	UDT_MAP_MJ_DATA::iterator data;
	for (BYTE i=0;i<num;++i)
	{
		data = card2.find(i);
		//风牌,箭牌,花牌不考虑
		if (GetBase()->IsFeng(data->second) 
			|| GetBase()->IsJian(data->second)
			|| GetBase()->IsHua(data->second))
		{
			continue;
		}

		if (data != card2.end())
		{
			if (GetBase()->GetSameCardNum(card, data->second) > 0 
				&& GetBase()->GetSameCardNum(card, data->second+1) > 0 
				&& GetBase()->GetSameCardNum(card, data->second+2) > 0
				&& GetBase()->IsSameKind(data->second, data->second+1)
				&& GetBase()->IsSameKind(data->second, data->second+2))
			{
				GetBase()->DeleteCard(card, data->second);
				GetBase()->DeleteCard(card, data->second+1);
				GetBase()->DeleteCard(card, data->second+2);
				GetBase()->AddCard(ResultCard, data->second);
				GetBase()->AddCard(ResultCard, data->second+1);
				GetBase()->AddCard(ResultCard, data->second+2);

				//MyDebugString("sdp_xmmj", "s 提取连续的三张牌 (%d,%d,%d)", data->second, data->second+1, data->second+2);
			}
		}
	}

	return true;
}

//去除掉重复的牌
bool CBaseHuLogic::SetAloneCard(UDT_MAP_MJ_DATA &card)
{
	GetBase()->SortMJ(card);
	UDT_MAP_MJ_DATA MiddCard;
	MiddCard.clear();
	map<BYTE, BYTE>SameCard;//前一个是什么牌，后一个是记录牌张数
	UDT_MAP_MJ_DATA::iterator data;
	for (data=card.begin();data!=card.end();++data)
	{
		BYTE unum = GetBase()->GetSameCardNum(card, data->second);
		if (unum>0 && SameCard.find(data->second) == SameCard.end())//张数大于等于1且还没有存储
		{
			SameCard.insert(map<BYTE, BYTE>::value_type(data->second, unum));
		}
	}

	map<BYTE, BYTE>::iterator SameCardData;
	for (SameCardData=SameCard.begin(); SameCardData!=SameCard.end();++SameCardData)
	{
		GetBase()->AddCard(MiddCard, SameCardData->first);
	}

	card = MiddCard;
	return true;
}