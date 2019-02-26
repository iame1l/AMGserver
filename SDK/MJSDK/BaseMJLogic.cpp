#include "stdafx.h"
#include "BaseMJLogic.h"

//static CBaseMJLogic* pgBaseMJLogic = NULL;
//
//CBaseMJLogic* GetBase()
//{
//	if (pgBaseMJLogic)
//	{
//		return pgBaseMJLogic;
//	}
//	return NULL;
//}

//构造函数		
CBaseMJLogic::CBaseMJLogic(void)
{
	//pgBaseMJLogic = this;
	m_MJDataMap.clear();
	m_iBankerID[0] = -1;//未定庄
	m_iBankerID[1] = 0;
	m_WildCardInfo.bWildCard = false;
	m_DrawOrder = false;
	m_OutOrder =true;
	m_nRunNbr = 0;
	m_iWiner = 255;
	m_uFirstHandCard = 16;
	m_uNumForTun = 2;
	m_nBankerNbr = 0;
	m_bOutCardTime = 10;
	m_bBreakCardTime = 10;
	m_bWaitBeginTime = 10;

	ReSetMJData();
}
//析构函数
CBaseMJLogic::~CBaseMJLogic()
{

}
//初始化麻将数据
bool CBaseMJLogic::InitMJData()
{
	CString nid;
	nid.Format("%d",NAME_ID);
	CString s;
	s = CINIFile::GetAppPath() + nid + "_s.ini";/////本地路径
	CINIFile f(s);
	CString key = SET_KIND;

	//m_MJ_Desk_Database.uAllNum = f.GetKeyVal(key,SET_NAME_ALLNUM,0);

	m_MJ_Desk_Database.bExistKind[Eie_BASEKIND_WAN-1] = f.GetKeyVal(key,SET_NAME_EXIST_WAN,1);
	m_MJ_Desk_Database.bExistKind[Eie_BASEKIND_TONG-1] = f.GetKeyVal(key,SET_NAME_EXIST_TONG,1);
	m_MJ_Desk_Database.bExistKind[Eie_BASEKIND_TIAO-1] = f.GetKeyVal(key,SET_NAME_EXIST_TIAO,1);
	m_MJ_Desk_Database.bExistKind[Eie_BASEKIND_FENG-1] = f.GetKeyVal(key,SET_NAME_EXIST_FENG,1);
	m_MJ_Desk_Database.bExistKind[Eie_BASEKIND_JIAN-1] = f.GetKeyVal(key,SET_NAME_EXIST_JIAN,1);
	m_MJ_Desk_Database.bExistKind[Eie_BASEKIND_HUA-1] = f.GetKeyVal(key,SET_NAME_EXIST_HUA,1);
	
	BYTE num = 0;
	if (m_MJ_Desk_Database.bExistKind[Eie_BASEKIND_WAN-1])//万牌
	{
		for (BYTE i=0; i<Eie_BASE_WAN_NUM; ++i)
		{
			for (BYTE j=0; j<4;++j)//存在四张
			{
				m_MJDataMap.insert(UDT_MAP_MJ_DATA::value_type(num++, Eie_BASE_WAN1+i));
			}
		}
	}
	if (m_MJ_Desk_Database.bExistKind[Eie_BASEKIND_TONG-1])//筒牌
	{
		for (BYTE i=0; i<Eie_BASE_TONG_NUM; ++i)
		{
			for (BYTE j=0; j<4;++j)//存在四张
			{
				m_MJDataMap.insert(UDT_MAP_MJ_DATA::value_type(num++, Eie_BASE_TONG1+i));
			}
		}
	}
	if (m_MJ_Desk_Database.bExistKind[Eie_BASEKIND_TIAO-1])//条牌
	{
		for (BYTE i=0; i<Eie_BASE_TIAO_NUM; ++i)
		{
			for (BYTE j=0; j<4;++j)//存在四张
			{
				m_MJDataMap.insert(UDT_MAP_MJ_DATA::value_type(num++, Eie_BASE_TIAO1+i));
			}
		}
	}
	if (m_MJ_Desk_Database.bExistKind[Eie_BASEKIND_FENG-1])//风牌
	{
		for (BYTE i=0; i<Eie_BASE_FENG_NUM; ++i)
		{
			for (BYTE j=0; j<4;++j)//存在四张
			{
				m_MJDataMap.insert(UDT_MAP_MJ_DATA::value_type(num++, Eie_BASE_FENG_DONG+i));
			}
		}
	}
	if (m_MJ_Desk_Database.bExistKind[Eie_BASEKIND_JIAN-1])//箭牌
	{
		for (BYTE i=0; i<Eie_BASE_JIAN_NUM; ++i)
		{
			for (BYTE j=0; j<4;++j)//存在四张
			{
				m_MJDataMap.insert(UDT_MAP_MJ_DATA::value_type(num++, Eie_BASE_JIAN_ZHONG+i));
			}
		}
	}
	if (m_MJ_Desk_Database.bExistKind[Eie_BASEKIND_HUA-1])//花牌
	{
		for (BYTE i=0; i<Eie_BASE_HUA_NUM; ++i)
		{
			m_MJDataMap.insert(UDT_MAP_MJ_DATA::value_type(num++, Eie_BASE_HUA_CHUN+i));
		}
	}
	m_MJ_Desk_Database.uAllNum = num;//总张数

	m_WildCardInfo.bWildCard = f.GetKeyVal(key,SET_NAME_WILDCARD,0);

	//色子
	m_MJ_ShaiZi_Database.uAllNum = f.GetKeyVal(key,SET_NAME_SHAIZI_NUM,2);

	m_MJ_ShaiZi_Database.utime = f.GetKeyVal(key,SET_NAME_SHAIZI_TIME,1);

	if (m_MJ_ShaiZi_Database.uAllNum < 0)
	{
		m_MJ_ShaiZi_Database.uAllNum = 0;
	}
	if (m_MJ_ShaiZi_Database.utime < 0)
	{
		m_MJ_ShaiZi_Database.utime = 0;
	}

	m_DrawOrder = f.GetKeyVal(key,SET_NAME_DRAWORDER,0);//默认抓牌顺时针

	m_OutOrder = f.GetKeyVal(key,SET_NAME_OUTORDER,1);//默认出牌逆时针

	m_uFirstHandCard = f.GetKeyVal(key,SET_NAME_FIRSTHANDCARD,16);

	m_uNumForTun = f.GetKeyVal(key,SET_NAME_NUM_FOR_TUN,2);//默认麻将墙是两个上下叠着为一屯的
	if (m_uNumForTun == 0)
	{
		m_uNumForTun = 2;
	}

	m_bOutCardTime = f.GetKeyVal(key,SET_NAME_OUT_CARD_TIME,20);
	m_bBreakCardTime = f.GetKeyVal(key,SET_NAME_BREAK_CARD_TIME,10);
	m_bWaitBeginTime = f.GetKeyVal(key,SET_NAME_WAIT_BEGIN_TIME,20);

	m_DeFen[0] = f.GetKeyVal(key,SET_NAME_DE_FEN0,1);//庄家底分
	m_DeFen[1] = f.GetKeyVal(key,SET_NAME_DE_FEN1,1);//闲家底分

	//胡牌配置文件设置
	m_MyHuLogic.InitData(s, key);

	return true;
}

bool CBaseMJLogic::ReSetMJData()
{
	for (int i=0;i<PLAY_COUNT;++i)
	{
		m_CardInfo[i].init();
	}
	m_iNowPlayer = 255;
	m_bKindForHu = false;
	m_bFirstTag = false;
	ZeroMemory(m_bTing, sizeof(m_bTing));
	ZeroMemory(m_UserID, sizeof(m_UserID));

	m_first_PostInfo.init();
	m_TakeWildCardInfo.init();

	m_bNowCanBreakNum = 0;
	for (int i=0;i<PLAY_COUNT;++i)
	{
		m_BreakCard[i].init();
		m_UserResultInfo[i].init();
		m_bCanGun[i] = true;
		m_bCanGunForRound[i] = true;
	}

	m_MJ_TakeCardPortInfo.init();
	m_WildCardInfo.uCard = 0;
	ZeroMemory(m_bIsQiangGangHu, sizeof(m_bIsQiangGangHu));
	ZeroMemory(m_bJiangCard, sizeof(m_bJiangCard));
	ZeroMemory(m_bHuKind, sizeof(m_bHuKind));
	m_bOnlyOutBigCard = false;
	//ZeroMemory(m_bOnlyOutBigCard, sizeof(m_bOnlyOutBigCard));
	ZeroMemory(m_bCanOutCard, sizeof(m_bCanOutCard));
	return true;
}

//获取下一个位置
BYTE CBaseMJLogic::GetNext(BYTE uport, BYTE ufull, bool clockwise)
{
	if (uport >= ufull)
	{
		return 0;
	}
	if(clockwise)//顺时针
	{
		return (uport + 1 ) % ufull;
	}
	//逆时针
	return 	(uport + (ufull - 1)) % ufull;
}

//整理手牌
bool CBaseMJLogic::TidyMJ(UDT_MAP_MJ_DATA &card)
{
	UDT_MAP_MJ_DATA hand;
	UDT_MAP_MJ_DATA::iterator Getdata;
	hand.clear();
	BYTE index = 0;
	for (BYTE i=0; i<card.size();++i)
	{
		Getdata = card.find(i);
		if (Getdata != card.end())
		{
			if (Getdata->second != 0)
			{
				hand.insert(UDT_MAP_MJ_DATA::value_type(index++, Getdata->second));
			}
		}
	}

	card = hand;

	return true;
}

//手牌排序
bool CBaseMJLogic::SortMJ(UDT_MAP_MJ_DATA &card, bool bAscendingOrder)
{
	//先整理
	if (!TidyMJ(card))
	{
		return false;
	}

	BYTE iCardCount = card.size();
	UDT_MAP_MJ_DATA::iterator Getdata;
	UDT_MAP_MJ_DATA::iterator Getdata2;
	BYTE temp = 0;
	//冒泡排序
	for(int j=0;j<iCardCount-1;j++) 
	{ 
		for (int i=0;i<iCardCount-1-j;i++) 
		{
			Getdata = card.find(i);
			Getdata2 = card.find(i+1);
			if (Getdata != card.end() && Getdata2 != card.end())
			{
				if (Getdata->second != 0 && Getdata2->second != 0)
				{
					if (bAscendingOrder)//升序
					{
						if (Getdata->second>Getdata2->second) 
						{ 
							temp=Getdata->second; 
							Getdata->second=Getdata2->second; 
							Getdata2->second=temp;
						} 
					}						
					else//降序
					{
						if (Getdata->second<Getdata2->second) 
						{ 
							temp=Getdata->second; 
							Getdata->second=Getdata2->second; 
							Getdata2->second=temp;
						} 
					}
				}
			}
		}
	}

	return true;
}

//是万牌
bool CBaseMJLogic::IsWan(BYTE uCard)
{
	if (uCard >= Eie_BASE_WAN1 && uCard <= Eie_BASE_WAN9)
	{
		return true;
	}

	return false;
}
//是筒牌
bool CBaseMJLogic::IsTong(BYTE uCard)
{
	if (uCard >= Eie_BASE_TONG1 && uCard <= Eie_BASE_TONG9)
	{
		return true;
	}

	return false;
}
//是条牌
bool CBaseMJLogic::IsTiao(BYTE uCard)
{
	if (uCard >= Eie_BASE_TIAO1 && uCard <= Eie_BASE_TIAO9)
	{
		return true;
	}

	return false;
}
//是风牌
bool CBaseMJLogic::IsFeng(BYTE uCard)
{
	if (uCard >= Eie_BASE_FENG_DONG && uCard <= Eie_BASE_FENG_BEI)
	{
		return true;
	}

	return false;
}
//是箭牌
bool CBaseMJLogic::IsJian(BYTE uCard)
{
	if (uCard >= Eie_BASE_JIAN_ZHONG && uCard <= Eie_BASE_JIAN_BAI)
	{
		return true;
	}

	return false;
}
//是花牌
bool CBaseMJLogic::IsHua(BYTE uCard)
{
	if (uCard >= Eie_BASE_HUA_CHUN && uCard <= Eie_BASE_HUA_JU)
	{
		return true;
	}

	return false;
}

//获得该牌的牌类型
BYTE CBaseMJLogic::GetKind(BYTE uCard)
{
	if (IsWan(uCard))
	{
		return Eie_BASEKIND_WAN;
	}
	else if (IsTong(uCard))
	{
		return Eie_BASEKIND_TONG;
	}
	else if (IsTiao(uCard))
	{
		return Eie_BASEKIND_TIAO;
	}
	else if (IsFeng(uCard))
	{
		return Eie_BASEKIND_FENG;
	}
	else if (IsJian(uCard))
	{
		return Eie_BASEKIND_JIAN;
	}
	else if (IsHua(uCard))
	{
		return Eie_BASEKIND_HUA;
	}

	return 0;
}

//是合法牌
bool CBaseMJLogic::IsLegal(BYTE uCard)
{
	UDT_MAP_MJ_DATA::iterator Getdata;
	for (Getdata = m_MJDataMap.begin(); Getdata != m_MJDataMap.end(); ++Getdata)
	{
		if (Getdata->second == uCard)//存在
		{
			return true;
		}
	}

	return false;
}

//获得牌张数(排除掉值为0的牌)
BYTE CBaseMJLogic::GetCardNum(UDT_MAP_MJ_DATA card)
{
	BYTE unum=0;
	/*UDT_MAP_MJ_DATA::iterator Getdata;
	for (BYTE i=0;i<card.size();++i)
	{
		Getdata = card.find(i);
		if (Getdata != card.end())
		{
			if (Getdata->second != 0)
			{
				unum++;
			}
		} 
	}*/
	UDT_MAP_MJ_DATA::iterator ite=card.begin();
	for(;ite!=card.end();ite++)
	{
		if (0!=ite->second)
		{
			unum++;
		}
	}	
	return unum;
}

//在一组牌中获得指定牌张数
BYTE CBaseMJLogic::GetSameCardNum(UDT_MAP_MJ_DATA card, BYTE uCard)
{
	BYTE unum=0;
	/*UDT_MAP_MJ_DATA::iterator Getdata;
	for (BYTE i=0;i<card.size();++i)
	{
	Getdata = card.find(i);
	if (Getdata != card.end())
	{
	if (Getdata->second != 0 && Getdata->second == uCard)
	{
	unum++;
	}
	} 
	}*/
	UDT_MAP_MJ_DATA::iterator ite=card.begin();
	for(;ite!=card.end();ite++)
	{
		if(uCard==ite->second)
		{
			unum++;
		}
	}	
	return unum;
}

//得到一组牌中第几张牌
BYTE CBaseMJLogic::GetCard(UDT_MAP_MJ_DATA &card, BYTE index)
{
	//先整理
	if (!TidyMJ(card))
	{
		return 0;
	}

	if (card.size() == 0)
	{
		return 0;//本来就没牌了
	}

	if (index >= card.size())
	{
		if (card.size() != 0)
		{
			index = card.size()-1;
		} 
		else
		{
			return 0;//都为0了肯定没牌
		}

	}

	UDT_MAP_MJ_DATA::iterator Getdata;
	for (BYTE i=0;i<card.size();++i)
	{
		Getdata = card.find(i);
		if (Getdata != card.end())
		{
			if (Getdata->first == index)
			{
				return Getdata->second;
			}
		} 
	}

	return 0;
}

//删除一张在一组牌中的指定的牌
bool CBaseMJLogic::DeleteCard(UDT_MAP_MJ_DATA &card, BYTE uCard)
{
	bool tag = false;
	UDT_MAP_MJ_DATA::iterator Getdata;
	for (BYTE i=0;i<card.size();++i)
	{
		Getdata = card.find(i);
		if (Getdata != card.end())
		{
			if (Getdata->second != 0 && Getdata->second == uCard)
			{
				Getdata->second = 0;
				tag = true;
				break;
			}
		} 
	}

	//再整理
	TidyMJ(card);

	return tag;
}

//为指定的一组牌添加牌
bool CBaseMJLogic::AddCard(UDT_MAP_MJ_DATA &card, BYTE uCard)
{
	if (!IsLegal(uCard))//不是合法牌
	{
		return false;
	}

	BYTE iSum = card.size();
	card.insert(UDT_MAP_MJ_DATA::value_type(iSum, uCard));
	return true;
}

//将一组牌中的一张牌换成指定的一张牌
bool CBaseMJLogic::ChangeCard(UDT_MAP_MJ_DATA &card, BYTE uCard, BYTE uChangeCard)
{
	if (!IsLegal(uChangeCard))//不是合法牌
	{
		return false;
	}

	UDT_MAP_MJ_DATA::iterator Getdata;
	for (Getdata = card.begin(); Getdata != card.end(); ++Getdata)
	{
		if (Getdata->second == uCard)//存在
		{
			Getdata->second = uChangeCard;
			return true;
		}
	}

	return false;
}

//两张牌是否为同类型牌
bool CBaseMJLogic::IsSameKind(BYTE uCardA, BYTE uCardB)
{
	if (GetKind(uCardA) == GetKind(uCardB))
	{
		return true;
	}
	else
	{
		return false;
	}
}

//增加当前可以拦牌人数
bool CBaseMJLogic::SetbNowCanBreakNum(bool bAdd)
{
	if (bAdd)//增加
	{
		++m_bNowCanBreakNum;
	} 
	else//减少
	{
		if (m_bNowCanBreakNum > 0)
		{
			--m_bNowCanBreakNum;
		}
		else
		{
			m_bNowCanBreakNum = 0;
		}
	}

	return true;
}

/*************************************************
*Function: 生成随机数
*explain:该函数利用一个固定的值来获取随机值，有效避免大量随机运算时出现规律性数据
*writer:
*		帅东坡
*Parameters:
*		void
*Return:
*		返回随机数
*************************************************/
int CBaseMJLogic::MyRand()
{
	static int innerSeed = GetTickCount();

	return (((innerSeed = innerSeed * 214013L + 2531011L) >> 16) & 0x7fff );
}