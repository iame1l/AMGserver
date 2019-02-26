#include "stdafx.h"
#include "BaseBuHuaLogic.h"
#include "BaseMJLogic.h"


//构造函数		
CBaseBuHuaLogic::CBaseBuHuaLogic(void)
{

}
//析构函数
CBaseBuHuaLogic::~CBaseBuHuaLogic()
{

}

//处理手中现有的花牌(只补一张)
bool CBaseBuHuaLogic::BuHua(BYTE uDesk)
{
	if (!GetBase()->IsTrueDesk(uDesk))
	{
		return false;
	}
	//先整理
	GetBase()->TidyMJ(GetBase()->m_CardInfo[uDesk].hand);

	UDT_MAP_MJ_DATA::iterator Getdata;
	for (int i=0;i<GetBase()->m_CardInfo[uDesk].hand.size();++i)
	{
		Getdata = GetBase()->m_CardInfo[uDesk].hand.find(i);
		if (Getdata != GetBase()->m_CardInfo[uDesk].hand.end())
		{
			if (GetBase()->IsHua(Getdata->second))
			{
				//记录花牌
				GetBase()->AddCard(GetBase()->m_CardInfo[uDesk].hua, Getdata->second);
				Getdata->second = 0;
				break;
			}
		}
	}
	//再次整理
	GetBase()->TidyMJ(GetBase()->m_CardInfo[uDesk].hand);
	//补花抓牌
	BYTE uCard=0;
	GetBase()->m_MyShowCardLogic.Take(uDesk, Eie_TAKE_BUHUA, uCard);
	GetBase()->AddCard(GetBase()->m_CardInfo[uDesk].hand, uCard);
	//再整理
	GetBase()->TidyMJ(GetBase()->m_CardInfo[uDesk].hand);

	return true;
}

//处理手中所有的花牌
bool CBaseBuHuaLogic::BuHuaAll(BYTE uDesk)
{
	if (!GetBase()->IsTrueDesk(uDesk))
	{
		return false;
	}
	//先整理
	GetBase()->TidyMJ(GetBase()->m_CardInfo[uDesk].hand);
	BYTE unum=0;
	UDT_MAP_MJ_DATA::iterator Getdata;
	for (int i=0;i<GetBase()->m_CardInfo[uDesk].hand.size();++i)
	{
		Getdata = GetBase()->m_CardInfo[uDesk].hand.find(i);
		if (Getdata != GetBase()->m_CardInfo[uDesk].hand.end())
		{
			if (GetBase()->IsHua(Getdata->second))
			{
				//记录花牌
				GetBase()->AddCard(GetBase()->m_CardInfo[uDesk].hua, Getdata->second);
				Getdata->second = 0;
				++unum;
			}
		}
	}
	//再次整理
	GetBase()->TidyMJ(GetBase()->m_CardInfo[uDesk].hand);
	//补花抓牌
	for (int i=0;i<unum;++i)
	{
		BYTE uCard=0;
		GetBase()->m_MyShowCardLogic.Take(uDesk, Eie_TAKE_BUHUA, uCard);
		GetBase()->AddCard(GetBase()->m_CardInfo[uDesk].hand, uCard);
		//再整理
		GetBase()->TidyMJ(GetBase()->m_CardInfo[uDesk].hand);
	}

	//判断还有没有花牌在手上
	bool btag = false;
	for (int i=0;i<GetBase()->m_CardInfo[uDesk].hand.size();++i)
	{
		Getdata = GetBase()->m_CardInfo[uDesk].hand.find(i);
		if (Getdata != GetBase()->m_CardInfo[uDesk].hand.end())
		{
			if (GetBase()->IsHua(Getdata->second))
			{
				btag = true;
				break;
			}
		}
	}

	//迭代计算
	if (btag)
	{
		BuHuaAll(uDesk);
	}
	return true;
}