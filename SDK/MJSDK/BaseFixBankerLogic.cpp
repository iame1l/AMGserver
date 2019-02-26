#include "stdafx.h"
#include "BaseFixBankerLogic.h"
#include "BaseMJLogic.h"


//构造函数		
CBaseFixBankerLogic::CBaseFixBankerLogic(void)
{

}
//析构函数
CBaseFixBankerLogic::~CBaseFixBankerLogic()
{

}

//该模块主要动作的执行都在次函数中(扔色子，画牌墙，定庄，连局，连庄)
bool CBaseFixBankerLogic::Run()
{
	//扔色子
	InitShaiZiResult();
	//画牌墙
	RandCard();//设置好牌墙
	PaintWall();
	//定庄家(连局，连庄)
	InitBanker();
	return true;
}

//不记录游戏规则的临时一次扔色子动作结果
BYTE CBaseFixBankerLogic::RandShaiZiResult()
{
	return GetBase()->MyRand() % 6 + 1;
}

//扔色子
bool CBaseFixBankerLogic::InitShaiZiResult()
{
	//每局定色子数据
	GetBase()->m_MJ_ShaiZi_Database.ShaiZiMap.clear();
	BYTE uNum=0;
	BYTE midd=0;
	for(int i=0; i<GetBase()->m_MJ_ShaiZi_Database.utime; ++i) 
	{
		for(int j=0; j<GetBase()->m_MJ_ShaiZi_Database.uAllNum; ++j) 
		{
			midd = GetBase()->MyRand() % 6 + 1;
			GetBase()->m_MJ_ShaiZi_Database.ShaiZiMap.insert(pair<BYTE, BYTE>(uNum, midd));
			++uNum;
		}
	}

	return true;
}

//获得每一次扔色子的总点数
BYTE CBaseFixBankerLogic::GetShaiZiResult(BYTE uTime)
{
	if (uTime >= GetBase()->m_MJ_ShaiZi_Database.utime)
	{
		return 0;
	}

	map<BYTE, BYTE> ShaiZiMap;
	ShaiZiMap.clear();
	ShaiZiMap = GetBase()->m_MJ_ShaiZi_Database.ShaiZiMap;
	BYTE uAllNum = GetBase()->m_MJ_ShaiZi_Database.utime;
	BYTE uResult = 0;
	map <BYTE, BYTE>::iterator Iter;
	for ( Iter = ShaiZiMap.begin(); Iter != ShaiZiMap.end(); Iter++ )
	{
		if (Iter->first >= uTime*uAllNum && Iter->first < (uTime+1)*uAllNum )
		{
			uResult += Iter->second;
		}
	}

	return uResult;
}

//获得每一次扔色子某一个色子的点数
BYTE CBaseFixBankerLogic::GetOneShaiZiResult(BYTE uShaiZiID,BYTE uTime)
{
	if (uTime >= GetBase()->m_MJ_ShaiZi_Database.utime)
	{
		return 0;
	}

	if (uShaiZiID >= GetBase()->m_MJ_ShaiZi_Database.uAllNum)
	{
		return 0;
	}
	map<BYTE, BYTE> ShaiZiMap;
	ShaiZiMap.clear();
	ShaiZiMap = GetBase()->m_MJ_ShaiZi_Database.ShaiZiMap;
	BYTE uAllNum = GetBase()->m_MJ_ShaiZi_Database.utime;
	BYTE uResult = 0;
	map <BYTE, BYTE>::iterator Iter;
	for ( Iter = ShaiZiMap.begin(); Iter != ShaiZiMap.end(); Iter++ )
	{
		if (Iter->first == (uTime*uAllNum + uShaiZiID))
		{
			uResult = Iter->second;
		}
	}

	return uResult;
}

//说明：
//牌墙数据记录设计为四部分（假定四人麻将，一屯两张，顺时针摸牌），分别存储在四个玩家麻将数据中
//摸走的牌数据为0

//画牌墙
bool CBaseFixBankerLogic::PaintWall()
{
	//先判断几个人
	//考虑张数
	//考虑抓屯
	//考虑补花
	//考虑杠牌抓牌

	return true;
}

//定庄家(连局，连庄)
bool CBaseFixBankerLogic::InitBanker()
{
	if (GetBase()->m_nRunNbr > 255)//防止异常
	{
		GetBase()->m_nRunNbr = 0;
	}
	long int iOld_Banker[2] = {0};//原庄家信息
	CopyMemory(iOld_Banker, GetBase()->m_iBankerID, sizeof(iOld_Banker));

	if (GetBase()->m_nRunNbr == 0)//第一把 给先进来的人当庄
	{
		GetBase()->m_iBankerID[0] = 0;
		GetBase()->m_iBankerID[1] = GetBase()->m_UserID[0];
	} 
	else
	{
		//上把赢了这把当庄
		if (GetBase()->IsTrueDesk(GetBase()->m_iWiner))
		{
			GetBase()->m_iBankerID[0] = GetBase()->m_iWiner;
			GetBase()->m_iBankerID[1] = GetBase()->m_UserID[GetBase()->m_iWiner];
		}
		else
		{
			//上把的庄家不在了 则 此局随机一个庄家出来
			if (!GetBase()->IsTrueDesk(GetBase()->m_iBankerID[0]))
			{
				GetBase()->m_iBankerID[0] = GetBase()->MyRand() % PLAY_COUNT;
				GetBase()->m_iBankerID[1] = GetBase()->m_UserID[GetBase()->m_iBankerID[0]];
			}
			else
			{
				//荒庄，则上把庄家当庄 即连庄
			}
		}
	}

	if (iOld_Banker[0] == GetBase()->m_iBankerID[0] && iOld_Banker[1] == GetBase()->m_iBankerID[1])
	{
		GetBase()->m_nBankerNbr++;
	}
	GetBase()->m_nRunNbr++;

	//出现异常
	if (!GetBase()->IsTrueDesk(GetBase()->m_iBankerID[0]))
	{
		GetBase()->m_iBankerID[0] = 0;
		GetBase()->m_iBankerID[1] = GetBase()->m_UserID[0];
		GetBase()->m_nRunNbr = 1;
	}

	return true;
}

//混乱所有的麻将子
bool CBaseFixBankerLogic::RandCard()
{
	UDT_MAP_MJ_DATA MJDataMap;
	MJDataMap.clear();
	MJDataMap = GetBase()->m_MJDataMap;

	BYTE iCardCount = MJDataMap.size();
	UDT_MAP_MJ_DATA::iterator Getdata;
	UDT_MAP_MJ_DATA::iterator Getdata2;
	int nIndex = 0;
	BYTE midd;
	for (int i = 0; i < iCardCount; i++)
	{
		nIndex = GetBase()->MyRand()%iCardCount;
		Getdata = MJDataMap.find(i);
		if(Getdata != MJDataMap.end())
		{
			midd = Getdata->second;
		}
		else
		{
			midd = 0;
		}
		Getdata2 = MJDataMap.find(nIndex);
		if(Getdata2 != MJDataMap.end())
		{
			Getdata->second = Getdata2->second;
			Getdata2->second = midd;
		}
		else
		{
			Getdata->second = 0;
			Getdata2->second = midd;
		}
	}

	//将得到的牌数据分发到每个人所在的牌墙上
	BYTE uleng = 0;//每个人平均获得的牌墙牌数目
	BYTE uleave = 0;//剩下不够除的牌数目
	//默认麻将墙是两个上下叠着为一屯的
	midd = 0;
	uleng = (iCardCount / (PLAY_COUNT * GetBase()->m_uNumForTun)) * GetBase()->m_uNumForTun;
	uleave = iCardCount % (PLAY_COUNT * GetBase()->m_uNumForTun);

	for (int i=0; i<PLAY_COUNT; ++i)
	{
		GetBase()->m_CardInfo[i].wall.clear();
		for (int j=0; j<uleng; ++j)
		{
			Getdata = MJDataMap.find(j+uleng*i);
			if(Getdata != MJDataMap.end())
			{
				midd = Getdata->second;
			}
			else
			{
				midd = 0;
			}

			GetBase()->m_CardInfo[i].wall.insert(UDT_MAP_MJ_DATA::value_type(j, midd));
		}
	}
	//剩下的牌都给第一个玩家
	for (int i=0; i<uleave; ++i)
	{
		Getdata = MJDataMap.find(i+uleng*PLAY_COUNT);
		if(Getdata != MJDataMap.end())
		{
			midd = Getdata->second;
		}
		else
		{
			midd = 0;
		}

		GetBase()->m_CardInfo[0].wall.insert(UDT_MAP_MJ_DATA::value_type(uleng+i, midd));
	}

	return true;
}