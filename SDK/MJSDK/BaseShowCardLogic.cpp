#include "stdafx.h"
#include "BaseShowCardLogic.h"
#include "BaseMJLogic.h"
#include "UpGradeLogic.h"
//构造函数		
CBaseShowCardLogic::CBaseShowCardLogic(void)
{
	/*memset(m_byArHandPai,255,sizeof(m_byArHandPai));
	m_HuTempData.Init();
	m_NoJingHuTempData.Init();
	m_hupaistruct.Init();
	m_TempHupaistruct.Init();
	m_byJingNum=0;
	m_iMaxFan=0;*/
}

//析构函数
CBaseShowCardLogic::~CBaseShowCardLogic()
{

}

//能否吃
bool CBaseShowCardLogic::CanBreakForChi(BYTE bDesk, BYTE uOutDesk, BYTE uCard, MJ_EAT eat[],bool bBaiBanFlag)
{
	if (!GetBase()->IsTrueDesk(bDesk) || !GetBase()->IsTrueDesk(uOutDesk))
	{
		return false;
	}
	if (!GetBase()->IsLegal(uCard))
	{
		return false;
	}
	if (GetBase()->m_bTing[bDesk])
	{
		return false;//一局听后只能判断胡
	}
	//只能吃上家的牌
	if (bDesk != GetBase()->GetNext(uOutDesk, PLAY_COUNT, GetBase()->m_DrawOrder))
	{
		return false;
	}
	if (!GetBase()->IsWan(uCard) 
		&& !GetBase()->IsTong(uCard)
		&& !GetBase()->IsTiao(uCard))
	{
		return false;
	}
	BYTE byHandleCard[20];//手中牌
	bool bCaiShenFlag[20];//对应的下标是否财神牌
	int index=0;
	memset(byHandleCard,255,sizeof(byHandleCard));
	GetBase()->m_CardInfo[bDesk].hand;
	UDT_MAP_MJ_DATA::iterator ite=GetBase()->m_CardInfo[bDesk].hand.begin();
	for (; ite!=GetBase()->m_CardInfo[bDesk].hand.end();ite++)//提取牌
	{
		byHandleCard[index]=ite->second ;
		index++;
	}
	int iBaiBanNums=0;//白板个数
	int iCaiShenNums=0;//财神个数
	for(int i=0;i<20;i++)
	{
		if(255==byHandleCard[i])
		{
			continue;
		}
		if(Eie_BASE_JIAN_BAI==byHandleCard[i])
		{
			iBaiBanNums++;
		}
		if(GetBase()->m_WildCardInfo.uCard==byHandleCard[i])
		{
			iCaiShenNums++;
		}
	}
	//将手上所以白板换成万能牌牌值,并返回替换的张数
	GetBase()->m_bBadWildCardNum[bDesk] = GetBase()->ChangeAllBaiBanCard(bDesk);
	bool bCan = false;
	if(iCaiShenNums>0)//防止财神牌也作为被吃的牌
	{
		BYTE uMinCard = uCard-1;
		BYTE uMaxCard = uCard+1;
		if (GetBase()->IsSameKind(uMinCard, uMinCard-1) && GetBase()->IsSameKind(uMinCard, uCard)
			&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMinCard) > 0
			&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMinCard-1) > 0)//前
		{
			eat[Eie_EAT_LATER].init();
			eat[Eie_EAT_LATER].bDeskStation = uOutDesk;
			eat[Eie_EAT_LATER].udata2 = uCard;
			eat[Eie_EAT_LATER].udata[0] = uMinCard-1;
			eat[Eie_EAT_LATER].udata[1] = uMinCard;
			eat[Eie_EAT_LATER].udata[2] = uCard;
			bCan=true;
			if(!bBaiBanFlag)//别人没有打白板,检测吃的牌中是否有万能牌,因为自己有可能打掉万能牌
			{
				//检测吃的牌中是否有万能牌
				for(int i=0;i<3;i++)
				{
					if(GetBase()->m_WildCardInfo.uCard==eat[Eie_EAT_LATER].udata[i])//吃的牌中有万能牌，如果有白板，就用白板替换
					{
						if(iBaiBanNums>0)
						{
							eat[Eie_EAT_LATER].udata[i]=Eie_BASE_JIAN_BAI;
							break;
						}
						else//没有白板，那就是用的财神牌，不能吃
						{
							memset(eat[Eie_EAT_LATER].udata,255,sizeof(eat[Eie_EAT_LATER].udata));
							bCan=false;
						}
					}
				}
			}
			else
			{
				eat[Eie_EAT_LATER].udata[2]=Eie_BASE_JIAN_BAI;
			}
			
		}
		if (GetBase()->IsSameKind(uMinCard, uCard) && GetBase()->IsSameKind(uMaxCard, uCard)
			&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMinCard) > 0
			&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMaxCard) > 0)//中
		{
			eat[Eie_EAT_MIDD].bDeskStation = uOutDesk;
			eat[Eie_EAT_MIDD].udata2 = uCard;
			eat[Eie_EAT_MIDD].udata[0] = uMinCard;
			eat[Eie_EAT_MIDD].udata[1] = uCard;
			eat[Eie_EAT_MIDD].udata[2] = uMaxCard;
			bCan=true;
			if(!bBaiBanFlag)//别人没有打白板,检测吃的牌中是否有万能牌,因为自己有可能打掉万能牌
			{
				//检测吃的牌中是否有万能牌
				for(int i=0;i<3;i++)
				{
					if(GetBase()->m_WildCardInfo.uCard==eat[Eie_EAT_MIDD].udata[i])//吃的牌中有万能牌，如果有白板，就用白板替换
					{
						if(iBaiBanNums>0)
						{
							eat[Eie_EAT_MIDD].udata[i]=Eie_BASE_JIAN_BAI;
							break;
						}
						else//没有白板，那就是用的财神牌，不能吃
						{
							memset(eat[Eie_EAT_MIDD].udata,255,sizeof(eat[Eie_EAT_MIDD].udata));
							bCan=false;
						}
					}
				}
			}
			else
			{
				eat[Eie_EAT_MIDD].udata[1] = Eie_BASE_JIAN_BAI;
			}
		}
		if (GetBase()->IsSameKind(uMaxCard, uMaxCard+1) && GetBase()->IsSameKind(uMaxCard, uCard)
			&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMaxCard) > 0
			&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMaxCard+1) > 0)//后
		{
			eat[Eie_EAT_FRONT].bDeskStation = uOutDesk;
			eat[Eie_EAT_FRONT].udata2 = uCard;
			eat[Eie_EAT_FRONT].udata[0] =uCard;
			eat[Eie_EAT_FRONT].udata[1] = uMaxCard;
			eat[Eie_EAT_FRONT].udata[2] = uMaxCard+1;
			bCan=true;
			if(!bBaiBanFlag)//别人没有打白板,检测吃的牌中是否有万能牌,因为自己有可能打掉万能牌
			{
				//检测吃的牌中是否有万能牌
				for(int i=0;i<3;i++)
				{
					if(GetBase()->m_WildCardInfo.uCard==eat[Eie_EAT_FRONT].udata[i])//吃的牌中有万能牌，如果有白板，就用白板替换
					{
						if(iBaiBanNums>0)
						{
							eat[Eie_EAT_FRONT].udata[i]=Eie_BASE_JIAN_BAI;
							break;
						}
						else//没有白板，那就是用的财神牌，不能吃
						{
							memset(eat[Eie_EAT_FRONT].udata,255,sizeof(eat[Eie_EAT_FRONT].udata));
							bCan=false;
						}
					}
				}
			}
			else
			{
				eat[Eie_EAT_FRONT].udata[0] =Eie_BASE_JIAN_BAI;
			}
		}
	}
	else//没有财神牌，有财神牌的地方用白板替换
	{
		BYTE uMinCard = uCard-1;
		BYTE uMaxCard = uCard+1;
		if (GetBase()->IsSameKind(uMinCard, uMinCard-1) && GetBase()->IsSameKind(uMinCard, uCard)
			&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMinCard) > 0
			&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMinCard-1) > 0)//前
		{
			eat[Eie_EAT_LATER].init();
			eat[Eie_EAT_LATER].bDeskStation = uOutDesk;
			eat[Eie_EAT_LATER].udata2 = uCard;
			eat[Eie_EAT_LATER].udata[0] = uMinCard-1;
			eat[Eie_EAT_LATER].udata[1] = uMinCard;
			eat[Eie_EAT_LATER].udata[2] = uCard;
			bCan=true;
			//if(!bBaiBanFlag)//别人没有打白板,检测吃的牌中是否有万能牌,因为自己有可能打掉万能牌
			{
				//检测吃的牌中是否有万能牌
				for(int i=0;i<3;i++)
				{
					if(GetBase()->m_WildCardInfo.uCard==eat[Eie_EAT_LATER].udata[i])//吃的牌中有万能牌，如果有白板，就用白板替换
					{
						eat[Eie_EAT_LATER].udata[i]=Eie_BASE_JIAN_BAI;
						break;
					}
				}
			}
		}
		if (GetBase()->IsSameKind(uMinCard, uCard) && GetBase()->IsSameKind(uMaxCard, uCard)
			&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMinCard) > 0
			&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMaxCard) > 0)//中
		{
			eat[Eie_EAT_MIDD].bDeskStation = uOutDesk;
			eat[Eie_EAT_MIDD].udata2 = uCard;
			eat[Eie_EAT_MIDD].udata[0] = uMinCard;
			eat[Eie_EAT_MIDD].udata[1] = uCard;
			eat[Eie_EAT_MIDD].udata[2] = uMaxCard;
			bCan=true;
			//if(!bBaiBanFlag)//别人没有打白板,检测吃的牌中是否有万能牌,因为自己有可能打掉万能牌
			{
				//检测吃的牌中是否有万能牌
				for(int i=0;i<3;i++)
				{
					if(GetBase()->m_WildCardInfo.uCard==eat[Eie_EAT_MIDD].udata[i])//吃的牌中有万能牌，如果有白板，就用白板替换
					{
						eat[Eie_EAT_MIDD].udata[i]=Eie_BASE_JIAN_BAI;
						break;

					}
				}
			}
		}
		if (GetBase()->IsSameKind(uMaxCard, uMaxCard+1) && GetBase()->IsSameKind(uMaxCard, uCard)
			&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMaxCard) > 0
			&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMaxCard+1) > 0)//后
		{
			eat[Eie_EAT_FRONT].bDeskStation = uOutDesk;
			eat[Eie_EAT_FRONT].udata2 = uCard;
			eat[Eie_EAT_FRONT].udata[0] =uCard;
			eat[Eie_EAT_FRONT].udata[1] = uMaxCard;
			eat[Eie_EAT_FRONT].udata[2] = uMaxCard+1;
			bCan=true;
			//if(!bBaiBanFlag)//别人没有打白板,检测吃的牌中是否有万能牌,因为自己有可能打掉万能牌
			{
				//检测吃的牌中是否有万能牌
				for(int i=0;i<3;i++)
				{
					if(GetBase()->m_WildCardInfo.uCard==eat[Eie_EAT_FRONT].udata[i])//吃的牌中有万能牌，如果有白板，就用白板替换
					{
						eat[Eie_EAT_FRONT].udata[i]=Eie_BASE_JIAN_BAI;
						break;
					}
				}
			}
		}
	}
	//还原刚才被替换的白板
	if (GetBase()->m_bBadWildCardNum[bDesk] > 0)
	{
		for (int i=0;i<GetBase()->m_bBadWildCardNum[bDesk];++i)
		{
			GetBase()->ChangeCard(GetBase()->m_CardInfo[bDesk].hand, GetBase()->m_WildCardInfo.uCard, Eie_BASE_JIAN_BAI);
		}
	}
	
	//UDT_MAP_MJ_DATA data;
	//GetBase()->TidyMJ(GetBase()->m_CardInfo[bDesk].hand);
	//data = GetBase()->m_CardInfo[bDesk].hand;
	//GetBase()->AddCard(data, uCard);
	//GetBase()->TidyMJ(data);
	////0x0
	////x00
	////00x
	//BYTE uMinCard = uCard-1;
	//BYTE uMaxCard = uCard+1;
	//bool bCan = false;
	//if (GetBase()->IsSameKind(uMinCard, uMinCard-1) && GetBase()->IsSameKind(uMinCard, uCard)
	//	&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMinCard) > 0
	//	&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMinCard-1) > 0)//00x
	//{
	//	eat[Eie_EAT_FRONT].init();
	//	eat[Eie_EAT_FRONT].bDeskStation = uOutDesk;
	//	eat[Eie_EAT_FRONT].udata2 = uCard;
	//	eat[Eie_EAT_FRONT].udata[0] = uMinCard-1;
	//	eat[Eie_EAT_FRONT].udata[1] = uMinCard;
	//	eat[Eie_EAT_FRONT].udata[2] = uCard;
	//	bCan = true;
	//}
	//if (GetBase()->IsSameKind(uMinCard, uCard) && GetBase()->IsSameKind(uMaxCard, uCard)
	//	&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMinCard) > 0
	//	&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMaxCard) > 0)//0x0
	//{
	//	eat[Eie_EAT_MIDD].bDeskStation = uOutDesk;
	//	eat[Eie_EAT_MIDD].udata2 = uCard;
	//	eat[Eie_EAT_MIDD].udata[0] = uMinCard;
	//	eat[Eie_EAT_MIDD].udata[1] = uCard;
	//	eat[Eie_EAT_MIDD].udata[2] = uMaxCard;
	//	bCan = true;
	//}
	//if (GetBase()->IsSameKind(uMaxCard, uMaxCard+1) && GetBase()->IsSameKind(uMaxCard, uCard)
	//	&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMaxCard) > 0
	//	&& GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uMaxCard+1) > 0)//x00
	//{
	//	eat[Eie_EAT_LATER].bDeskStation = uOutDesk;
	//	eat[Eie_EAT_LATER].udata2 = uCard;
	//	eat[Eie_EAT_LATER].udata[0] =uCard;
	//	eat[Eie_EAT_LATER].udata[1] = uMaxCard;
	//	eat[Eie_EAT_LATER].udata[2] = uMaxCard+1;
	//	bCan = true;
	//}
	////吃里面的牌不能有金牌
	//for(int i=0;i<3;i++)
	//{
	//	for(int index=0;index<3;index++)
	//	{
	//		if(GetBase()->m_WildCardInfo.uCard==eat[i].udata[index])
	//		{
	//			return false;
	//		}
	//	}
	//}
	return bCan;
}

//此牌先前碰过
bool CBaseShowCardLogic::BePeng(BYTE bDesk, BYTE uCard)
{
	if (!GetBase()->IsTrueDesk(bDesk))
	{
		return false;
	}

	if (!GetBase()->IsLegal(uCard))
	{
		return false;
	}
	
	map<BYTE, MJ_PENG>::iterator Getdata;
	BYTE unum = 0;
	for (Getdata=GetBase()->m_CardInfo[bDesk].peng.begin(); Getdata!=GetBase()->m_CardInfo[bDesk].peng.end();++Getdata)
	{
		if (Getdata->second.udata == uCard)
		{
			unum++;
			break;
		}
	}

	if (unum > 0)
	{
		return true;
	}

	return false;
}
//能否碰
bool CBaseShowCardLogic::CanBreakForPeng(BYTE bDesk, BYTE uCard)
{
	if (!GetBase()->IsTrueDesk(bDesk))
	{
		return false;
	}

	if (!GetBase()->IsLegal(uCard))
	{
		return false;
	}

	if (GetBase()->m_bTing[bDesk])
	{
		return false;//一局听后只能判断胡
	}
	if (GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uCard) >= 2)//手上有两个，即可碰
	{
		return true;
	}

	return false;
}
//能否杠
bool CBaseShowCardLogic::CanBreakForGang(BYTE bDesk, BYTE uOutDesk, BYTE uCard, bool &bkindGang,bool bFetchFlag)
{
	bkindGang = false;
	if (!GetBase()->IsTrueDesk(bDesk))
	{
		return false;
	}
	if (!GetBase()->IsLegal(uCard))
	{
		return false;
	}
	if (GetBase()->m_bTing[bDesk])
	{
		return false;//一局听后只能判断胡
	}
	//判断手牌(可明杠 或 暗杠)
	if (GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uCard) >= 3)//手上有三个，即可杠
	{
		//test
		//记录杠牌数据
		GetBase()->DebugPrintf("杠之前要杠的牌=%d",uCard);
		BYTE byArHandPai[20];
		int index=0;
		memset(byArHandPai,255,sizeof(byArHandPai));
		UDT_MAP_MJ_DATA card;
		card = GetBase()->m_CardInfo[bDesk].hand;
		UDT_MAP_MJ_DATA::iterator ite=card.begin();
		for(;ite!=card.end();ite++)
		{
			byArHandPai[index]=ite->second;
			index++;
		}	
		MaoPaoSort(byArHandPai,20,false);
		CString cs="";
		CString cs1="";
		for(int i=0;i<20;i++)
		{
			if(255!=byArHandPai[i])
			{
				cs1.Format(" %d ",byArHandPai[i]);
				cs.Append(cs1);
			}
		}
		GetBase()->DebugPrintf("杠之前杠牌玩家手里的牌=%s",cs);
		bkindGang = false;
		return true;
	}
	if(bFetchFlag)//自己摸的牌
	{
		//判断先前碰的牌(可补杠)
		if (BePeng(bDesk, uCard) && uOutDesk != bDesk)
		{
			bkindGang = true;
			return true;
		}
	}
	return false;
}
//能否听
bool CBaseShowCardLogic::CanBreakForTing(BYTE bDesk, BYTE uCard, BYTE &bResCard)
{
	//MyDebugString("sdp_xmmj", "s 能否听---------------");

	if (!GetBase()->IsTrueDesk(bDesk))
	{
		return false;
	}

	//if (!GetBase()->IsLegal(uCard))
	//{
	//	return false;
	//}

	if (GetBase()->m_bTing[bDesk])
	{
		return false;//一局不能重复听
	}

	//拿一张任意牌加进去判断是否胡
	UDT_MAP_MJ_DATA card;
	card = GetBase()->m_CardInfo[bDesk].hand;
	GetBase()->AddCard(card, uCard);//加上这张牌
	GetBase()->TidyMJ(card);//整理下
	UDT_MAP_MJ_DATA middAllCard;
	middAllCard = GetBase()->m_MJDataMap;
	GetBase()->m_MyHuLogic.SetAloneCard(middAllCard);//去掉重复牌
	BYTE unum = 0;
	UDT_MAP_MJ_DATA middcard;

	do 
	{
		middcard = card;
		UDT_MAP_MJ_DATA::iterator Getdata;
		Getdata = middcard.find(unum);
		if (Getdata != middcard.end())
		{
			GetBase()->DeleteCard(middcard, Getdata->second);
			bResCard = Getdata->second;
		}

		UDT_MAP_MJ_DATA::iterator Getdata2;
		for (Getdata2=middAllCard.begin();Getdata2!=middAllCard.end();++Getdata2)//再添加任何一张牌看是否可以胡
		{
			//在这里找出来的如果为万能牌面值，显然不能当万能牌用
			bool badtag = false;//异常标记
			if (Getdata2->second == GetBase()->m_WildCardInfo.uCard)
			{
				GetBase()->m_bBadWildCardNum[bDesk] -= 1;	
				if (GetBase()->m_bBadWildCardNum[bDesk] == 255)
				{
					GetBase()->m_bBadWildCardNum[bDesk] = 0;
					badtag = true;//出现异常
				}
			}

			GetBase()->AddCard(middcard, Getdata2->second);
			if (GetBase()->m_MyHuLogic.CheckHu(bDesk, middcard))
			{
				return true;
			}

			//还原
			if (Getdata2->second == GetBase()->m_WildCardInfo.uCard && !badtag)
			{
				GetBase()->m_bBadWildCardNum[bDesk] += 1;
			}

			GetBase()->DeleteCard(middcard, Getdata2->second);//还原
		}

		++unum;
	} while (unum >= card.size());

	bResCard = 0;
	return false;
}
//能否胡
bool CBaseShowCardLogic::CanBreakForHu(BYTE bDesk, BYTE uCard)
{
	//MyDebugString("sdp_xmmj", "s 能否胡---------------");

	if (!GetBase()->IsTrueDesk(bDesk))
	{
		return false;
	}

	if (!GetBase()->IsLegal(uCard))
	{
		return false;
	}

	UDT_MAP_MJ_DATA card;
	card = GetBase()->m_CardInfo[bDesk].hand;
	GetBase()->AddCard(card, uCard);
	GetBase()->TidyMJ(card);//整理下
	if (GetBase()->m_MyHuLogic.CheckHu(bDesk, card))
	{
		return true;
	}

	return false;
}

//能否拦牌
bool CBaseShowCardLogic::CanBreak(BYTE bDesk, BYTE uOutDesk, BYTE uCard, MJ_BREAK_ALLOW &kind, MJ_EAT eat[], bool &bkindGang, BYTE &bResCard,bool ZiMo)
{
	bool bTag = false;
	bool bSelf = false;//是否是自己拦自己牌
	if (bDesk == uOutDesk)
	{
		bSelf = true;
	}

	if (!bSelf && uCard == GetBase()->m_WildCardInfo.uCard)//拦别人的牌，如果是金牌不能拦
	{
		return false;
	}



	kind.init();
	if (!bSelf)
	{
		kind.data[Eie_BREAK_CHI] = CanBreakForChi(bDesk, uOutDesk, uCard, eat,false);//能否吃
		kind.data[Eie_BREAK_PENG] = CanBreakForPeng(bDesk, uCard);//能否碰
	}
	else
	{
		kind.data[Eie_BREAK_TING] = CanBreakForTing(bDesk, uCard, bResCard);//能否听
	}
	kind.data[Eie_BREAK_GANG] = CanBreakForGang(bDesk, uOutDesk, uCard, bkindGang);//能否杠
	kind.data[Eie_BREAK_HU] = CanBreakForHu(bDesk, uCard);//能否胡

	for (int i=0; i<Eie_BREAK_NUM; ++i)
	{
		if (kind.data[i])
		{
			kind.data[Eie_BREAK_GUO] = true;//始终是可以过的,前提是有其他拦牌成立，“过”是不能单独存在的
			bTag = true;
			break;
		}
	}



	return bTag;
}

//多人拦牌时判断谁拦牌成功
bool CBaseShowCardLogic::GetBreakResult(BYTE uOutDesk, BaseMess_BreakCard &BreakCard, bool clockwise)
{
	if (!GetBase()->IsTrueDesk(uOutDesk))
	{
		return false;
	}
	//每个人成立的拦牌类型
	BYTE bkind[PLAY_COUNT];
	ZeroMemory(bkind, sizeof(bkind));
	for (int i=0;i<PLAY_COUNT;++i)//这个for循环的目的就是给bkind赋值
	{
		if (GetBase()->m_BreakCard[i].kind == Eie_BREAK_GUO || uOutDesk==i)
		{
			continue;
		}
		UDT_MAP_MJ_DATA card;//打出牌玩家的出牌信息
		BYTE index;
		MJ_BREAK_ALLOW kind;
		MJ_EAT eat[3];
		BYTE bResCard = 0;
		////可能自己拦自己
		//if (i == uOutDesk)
		//{
		//	//自己确实拦自己的牌了(明杠,暗杠,听,胡)
		//	UDT_MAP_MJ_DATA middhand;
		//	bool tag = false;
		//		
		//	middhand = GetBase()->m_CardInfo[i].hand;//临时存储手牌
		//	GetBase()->DeleteCard(GetBase()->m_CardInfo[i].hand, GetBase()->m_CardInfo[i].bTakeCard);
		//	//验证该玩家是否能拦牌(牌已经在自己手牌中)
		//	if(!CanBreak(i, uOutDesk, GetBase()->m_CardInfo[i].bTakeCard, kind, eat, GetBase()->m_BreakCard[i].bkindGang, bResCard))
		//	{
		//		tag = true;
		//	}
		//	GetBase()->m_CardInfo[i].hand = middhand;//还原手牌
		//	if (tag)
		//	{
		//		continue;
		//	}
		//}
		//else
		//{
			if (GetBase()->m_CardInfo[uOutDesk].out.size()==0)//无打出牌，异常
			{
				//continue;
				GetBase()->DebugPrintf("GetBreakResult函数中判断拦牌成功异常：当前出牌者无打出牌");
				return false;
			}
			card = GetBase()->m_CardInfo[uOutDesk].out;
			index = GetBase()->m_CardInfo[uOutDesk].out.size()-1;
			//验证该玩家是否能拦牌
			if(!CanBreak(i, uOutDesk, GetBase()->GetCard(card, index), kind, eat, GetBase()->m_BreakCard[i].bkindGang, bResCard))
			{
				continue;
			}
		//}
		//验证该拦牌申请是在自己成立的拦牌类型内
		if (GetBase()->m_BreakCard[i].kind <= Eie_BREAK_HU)
		{
			if (kind.data[GetBase()->m_BreakCard[i].kind])//成立
			{
				bkind[i] = GetBase()->m_BreakCard[i].kind;
			}
		}
	}
	
	BYTE bPriorityPlayer = 255;
	BYTE bPriorityKind = Eie_BREAK_GUO;
	BYTE num=0;
	for(BYTE i = uOutDesk ;;i = GetBase()->GetNext(i, PLAY_COUNT, clockwise))
	{
		if (num == PLAY_COUNT)
		{
			break;
		}
		//即便是过也应该考虑在内
		if (GetPriorityBreak(bkind[i], bPriorityKind) == 1)
		{
			bPriorityKind = bkind[i];
			bPriorityPlayer = i;
		}
		++num;
	}
	if (GetBase()->IsTrueDesk(bPriorityPlayer))
	{
		CopyMemory(&BreakCard, &GetBase()->m_BreakCard[bPriorityPlayer], sizeof(BaseMess_BreakCard));
		return true;
	}
	return false;
}

//比较两个拦牌操作的优先级
BYTE CBaseShowCardLogic::GetPriorityBreak(BYTE kindA, BYTE kindB)
{
	if (kindA > Eie_BREAK_HU || kindB > Eie_BREAK_HU)
	{
		return 255;
	}

	if (kindA > kindB)
	{
		return 1;
	}
	else if (kindA < kindB)
	{
		return 2;
	}
	else
	{
		return 0;
	}
}

//拦牌
bool CBaseShowCardLogic::Break(BYTE bDesk, BYTE uOutDesk, BYTE uCard, BYTE kind, MJ_EAT ea, BYTE &bResCard, bool bkindGang)
{
	bool bTag = false;

	if (kind == Eie_BREAK_GUO)
	{
		bTag = false;
	}
	else if (kind == Eie_BREAK_CHI)
	{
		bTag = BreakForChi(bDesk, uOutDesk, uCard, ea);
	}
	else if (kind == Eie_BREAK_PENG)
	{
		bTag = BreakForPeng(bDesk, uOutDesk, uCard);
	}
	else if (kind == Eie_BREAK_GANG)
	{
		bTag = BreakForGang(bDesk, uOutDesk, uCard, bkindGang);
	}
	else if (kind == Eie_BREAK_TING)
	{
		bTag = BreakForTing(bDesk, uOutDesk, uCard);
	}
	else if (kind == Eie_BREAK_HU)
	{
		bTag = BreakForHu(bDesk, uOutDesk, uCard);
	}

	return bTag;
}

//吃
bool CBaseShowCardLogic::BreakForChi(BYTE bDesk, BYTE uOutDesk, BYTE uCard, MJ_EAT ea)
{
	MJ_EAT eatdata[3];

	for (int i=0;i<3;++i)
	{
		//手牌不存在该牌
		if (ea.udata[i] != uCard && GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, ea.udata[i]) == 0)
		{
			return false;
		}
	}
	for (int i=0;i<3;++i)
	{
		if (ea.udata[i] != uCard)
		{
			//手牌删除该牌
			GetBase()->DeleteCard(GetBase()->m_CardInfo[bDesk].hand, ea.udata[i]);
		}
	}


	//存储数据
	GetBase()->m_CardInfo[bDesk].eat.insert(map<BYTE, MJ_EAT>::value_type(GetBase()->m_CardInfo[bDesk].eat.size(), ea));
	
	GetBase()->m_iNowPlayer = bDesk;

	return true;
}
//碰
bool CBaseShowCardLogic::BreakForPeng(BYTE bDesk, BYTE uOutDesk, BYTE uCard)
{
	if (!GetBase()->IsTrueDesk(uOutDesk))
	{
		return false;
	}

	for (int i=0;i<2;++i)
	{
		//手牌删除该牌
		GetBase()->DeleteCard(GetBase()->m_CardInfo[bDesk].hand, uCard);
	}

	//出牌者的打出牌队列也应该删除该牌
	GetBase()->DeleteCard(GetBase()->m_CardInfo[uOutDesk].out, uCard);

	//存储数据
	MJ_PENG peng;
	peng.bDeskStation = uOutDesk;
	peng.udata = uCard;
	GetBase()->m_CardInfo[bDesk].peng.insert(map<BYTE, MJ_PENG>::value_type(GetBase()->m_CardInfo[bDesk].peng.size(), peng));

	GetBase()->m_iNowPlayer = bDesk;
	return true;
}
//杠
bool CBaseShowCardLogic::BreakForGang(BYTE bDesk, BYTE uOutDesk, BYTE uCard, bool bkindGang)
{
	if (!GetBase()->IsTrueDesk(uOutDesk))
	{
		return false;
	}

	MJ_GANG gang;
	gang.bDeskStation = uOutDesk;
	gang.bkind = bkindGang;
	gang.udata = uCard;
	if (bkindGang)//补杠
	{
		//碰的数据要删除掉
		map<BYTE, MJ_PENG>::iterator Getdata;
		for (BYTE i=0;i<GetBase()->m_CardInfo[bDesk].peng.size();++i)
		{
			Getdata = GetBase()->m_CardInfo[bDesk].peng.find(i);
			if (Getdata != GetBase()->m_CardInfo[bDesk].peng.end())
			{
				if (Getdata->second.udata == uCard)
				{
					Getdata->second.udata = 0;
					break;
				}
			} 
		}
		//删除掉
		DeletePeng(GetBase()->m_CardInfo[bDesk].peng, Getdata->second.udata);
		//手牌删除该牌
		GetBase()->DeleteCard(GetBase()->m_CardInfo[bDesk].hand, uCard);
	} 
	else//明杠 或 暗杠
	{
		//暗杠
		if (GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uCard) == 4)
		{
			for (int i=0;i<4;++i)
			{
				//手牌删除该牌
				GetBase()->DeleteCard(GetBase()->m_CardInfo[bDesk].hand, uCard);
			}
		}
		else if (GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uCard) == 3)
		{
			for (int i=0;i<3;++i)
			{
				//手牌删除该牌
				GetBase()->DeleteCard(GetBase()->m_CardInfo[bDesk].hand, uCard);
			}

			//出牌者的打出牌队列也应该删除该牌
			GetBase()->DeleteCard(GetBase()->m_CardInfo[uOutDesk].out, uCard);
		}
		else
		{
			return false;
		}

	}

	GetBase()->m_CardInfo[bDesk].gang.insert(map<BYTE, MJ_GANG>::value_type(GetBase()->m_CardInfo[bDesk].gang.size(), gang));
	GetBase()->m_iNowPlayer = bDesk;
	return true;
}
//听
bool CBaseShowCardLogic::BreakForTing(BYTE bDesk, BYTE uOutDesk, BYTE uCard)
{
	if (!GetBase()->IsTrueDesk(uOutDesk))
	{
		return false;
	}

	//手牌删除该牌
	//GetBase()->DeleteCard(GetBase()->m_CardInfo[bDesk].hand, bResCard);
	GetBase()->m_bTing[bDesk] = true;
	return true;
}
//胡
bool CBaseShowCardLogic::BreakForHu(BYTE bDesk, BYTE uOutDesk, BYTE uCard,bool bZiMo)
{
	if(!bZiMo)//放炮
	{
		if (!GetBase()->IsTrueDesk(uOutDesk))
		{
			return false;
		}
	}
	GetBase()->m_iNowPlayer = bDesk;

	GetBase()->m_UserResultInfo[bDesk].bWin = true;
	GetBase()->m_UserResultInfo[bDesk].bHuKind = GetBase()->m_bHuKind[bDesk];
	GetBase()->m_UserResultInfo[bDesk].bQiangGangHu = GetBase()->m_bIsQiangGangHu[bDesk];
	if (uOutDesk != bDesk)
	{
		if(!bZiMo)//放炮
		{
			GetBase()->m_UserResultInfo[bDesk].bWhoGun = uOutDesk;
		}
	}

	map<BYTE, MJ_EAT>::const_iterator eatdata;//吃的牌
	map<BYTE, MJ_PENG>::const_iterator pengdata;//碰的牌
	map<BYTE, MJ_GANG>::const_iterator gangdata;//杠的牌
	UDT_MAP_MJ_DATA::const_iterator huadata;//花牌
	UDT_MAP_MJ_DATA::const_iterator handdata;//手牌
	map<BYTE, MJ_EAT> eat;
	map<BYTE, MJ_PENG> peng;
	map<BYTE, MJ_GANG> gang;
	UDT_MAP_MJ_DATA hua;
	UDT_MAP_MJ_DATA hand;
	BYTE handnum = 0;
	eat = GetBase()->m_CardInfo[bDesk].eat;
	peng = GetBase()->m_CardInfo[bDesk].peng;
	gang = GetBase()->m_CardInfo[bDesk].gang;
	hua = GetBase()->m_CardInfo[bDesk].hua;
	//加入手牌
	if(!bZiMo)//放炮需要加入别人打的牌
	{
		if (GetBase()->IsLegal(uCard))
		{
			GetBase()->AddCard(GetBase()->m_CardInfo[bDesk].hand, uCard);
		}
	}
	GetBase()->TidyMJ(GetBase()->m_CardInfo[bDesk].hand);
	hand = GetBase()->m_CardInfo[bDesk].hand;
	for (eatdata = eat.begin();eatdata!=eat.end();++eatdata)
	{
		if (GetBase()->m_UserResultInfo[bDesk].uEatNum < MESS_CHI_NUM)
		{
			CopyMemory(GetBase()->m_UserResultInfo[bDesk].uEat[GetBase()->m_UserResultInfo[bDesk].uEatNum++], 
				eatdata->second.udata, sizeof(BYTE)*3);
		}

		for (int i=0;i<3;++i)
		{
			if (handnum < NUM_MAX_HAND*2)
			{//手牌也要加进去
				GetBase()->m_UserResultInfo[bDesk].uHand[handnum++] = eatdata->second.udata[i];
			}
		}
	}
	for (pengdata = peng.begin();pengdata!=peng.end();++pengdata)
	{
		if (GetBase()->m_UserResultInfo[bDesk].uPengNum < MESS_PENG_NUM)
		{
			GetBase()->m_UserResultInfo[bDesk].uPeng[GetBase()->m_UserResultInfo[bDesk].uPengNum++] = pengdata->second.udata;
		}

		for (int i=0;i<3;++i)
		{
			if (handnum < NUM_MAX_HAND*2)
			{//手牌也要加进去
				GetBase()->m_UserResultInfo[bDesk].uHand[handnum++] = pengdata->second.udata;
			}
		}
	}
	for (gangdata = gang.begin();gangdata!=gang.end();++gangdata)
	{
		if (GetBase()->m_UserResultInfo[bDesk].uGangNum < MESS_GANG_NUM)
		{
			CopyMemory(&GetBase()->m_UserResultInfo[bDesk].uGang[GetBase()->m_UserResultInfo[bDesk].uGangNum++], 
				&gangdata->second, sizeof(MJ_GANG));
		}

		for (int i=0;i<4;++i)
		{
			if (handnum < NUM_MAX_HAND*2)
			{//手牌也要加进去
				GetBase()->m_UserResultInfo[bDesk].uHand[handnum++] = gangdata->second.udata;
			}
		}
	}
	for (huadata = hua.begin();huadata!=hua.end();++huadata)
	{
		if (GetBase()->m_UserResultInfo[bDesk].uHuaNum < MESS_HUA_NUM)
		{
			GetBase()->m_UserResultInfo[bDesk].uHua[GetBase()->m_UserResultInfo[bDesk].uHuaNum++] = huadata->second;
		}
	}
	
	//补充手牌
	//将所有此人吃碰杠的数据全部加进来
	for (handdata = hand.begin();handdata!=hand.end();++handdata)
	{
		if (handnum < NUM_MAX_HAND*2)
		{
			GetBase()->m_UserResultInfo[bDesk].uHand[handnum++] = handdata->second;
		}
	}
	GetBase()->m_UserResultInfo[bDesk].uHandNum = handnum;
	GetBase()->m_iWiner = bDesk;//谁赢了
	return true;
}

//能否出牌
bool CBaseShowCardLogic::CanOut(BYTE bDesk, BYTE uCard)
{
	if (!GetBase()->IsTrueDesk(bDesk))
	{
		return false;
	}

	//存在该手牌
	if (GetBase()->GetSameCardNum(GetBase()->m_CardInfo[bDesk].hand, uCard) > 0)
	{
		return true;
	}

	return false;
}
//出牌
bool CBaseShowCardLogic::Out(BYTE bDesk, BYTE uCard)
{
	if (!CanOut(bDesk,uCard))
	{
		return false;
	}

	UDT_MAP_MJ_DATA::iterator Getdata;
	for (Getdata = GetBase()->m_CardInfo[bDesk].hand.begin(); Getdata != GetBase()->m_CardInfo[bDesk].hand.end(); ++Getdata)
	{
		if (Getdata->second == uCard)//存在
		{
			GetBase()->AddCard(GetBase()->m_CardInfo[bDesk].out, uCard);
			GetBase()->DeleteCard(GetBase()->m_CardInfo[bDesk].hand, Getdata->second);
			return true;
		}
	}

	return false;
}
//能否摸牌
bool CBaseShowCardLogic::CanTake(BYTE bDesk, BYTE ukind)
{
	if (!GetBase()->IsTrueDesk(bDesk))
	{
		return false;
	}

	//计算总共所有人面前牌墙有多少张
	BYTE uall = 0;
	BYTE ualluser[PLAY_COUNT] = {0};
	for (int i=0; i<PLAY_COUNT;++i)
	{
		uall += GetBase()->GetCardNum(GetBase()->m_CardInfo[i].wall);
	}

	if (uall == 0)
	{
		return false;
	}

	return true;
}

//摸牌
bool CBaseShowCardLogic::Take(BYTE bDesk, BYTE ukind, BYTE &uCard)
{
	if (!CanTake(bDesk, ukind))
	{
		OutputDebugString("lwlog::CanTake=false了啊");
		return false;
	}
	SetNextTakePortInfo(ukind);
	//先设置好准备摸的牌在哪
	/*if (!SetNextTakePortInfo(ukind))
	{
		OutputDebugString("lwlog::SetNextTakePortInfo=false了啊");
		return false;
	}*/
	BYTE uPosition = GetBase()->m_MJ_TakeCardPortInfo.uDesk;//当前抓牌的牌墙方位
	BYTE uPort = GetBase()->m_MJ_TakeCardPortInfo.uPort;//当前抓的是牌墙上第几张牌
	BYTE midd = 0;
	UDT_MAP_MJ_DATA::iterator Getdata;
	Getdata = GetBase()->m_CardInfo[uPosition].wall.find(uPort);
	if (Getdata != GetBase()->m_CardInfo[uPosition].wall.end())
	{
		midd = Getdata->second;
		Getdata->second = 0;
	}
	uCard = midd;
	return true;
}

//设置下一个摸牌牌墙位置
bool CBaseShowCardLogic::SetNextTakePortInfo(BYTE ukind)
{
	BYTE uResPosition = 0;
	BYTE uPort = 0;
	if (ukind == Eie_TAKE_BUHUA || ukind == Eie_TAKE_GANG || ukind == Eie_TAKE_WILD)//补花摸牌//杠牌摸牌//万能牌摸牌
	{
		uPort = GetBase()->m_MyDealLogic.GetNextPort(GetBase()->m_MJ_TakeCardPortInfo.uDesk, uResPosition, GetBase()->m_MJ_TakeCardPortInfo.uPort, GetBase()->m_DrawOrder);
		if (uPort != 255)
		{
			SetNowTakePortInfo(uResPosition, uPort);
			return true;
		}
	}
	else if (ukind == Eie_TAKE_NORMAL)//正常摸牌
	{
		uPort = GetBase()->m_MyDealLogic.GetNextPort(GetBase()->m_MJ_TakeCardPortInfo.uDesk, uResPosition, GetBase()->m_MJ_TakeCardPortInfo.uPort, !GetBase()->m_DrawOrder);
		if (uPort != 255)
		{
			SetNowTakePortInfo(uResPosition, uPort);
			return true;
		}
	}
	
	return false;
}

//设置当前摸牌牌墙位置
bool CBaseShowCardLogic::SetNowTakePortInfo(BYTE uDesk, BYTE uPort)
{
	//还是要尽量验证数据的合法性
	if (!GetBase()->IsTrueDesk(uDesk))
	{
		return false;
	}

	GetBase()->m_MJ_TakeCardPortInfo.uDesk = uDesk;
	GetBase()->m_MJ_TakeCardPortInfo.uPort = uPort;
	return true;
}

//摸取万能牌(定什么牌是万能牌)
bool CBaseShowCardLogic::TakeWildCard(BYTE uDesk)
{
	return true;
}

//删除一张在一组碰牌中的指定的牌
bool CBaseShowCardLogic::DeletePeng(map<BYTE, MJ_PENG> &peng, BYTE uCard)
{
	bool tag = false;
	map<BYTE, MJ_PENG>::iterator Getdata;
	for (BYTE i=0;i<peng.size();++i)
	{
		Getdata = peng.find(i);
		if (Getdata != peng.end())
		{
			if (Getdata->second.udata != 0 && Getdata->second.udata == uCard)
			{
				Getdata->second.udata=0;
				tag = true;
				break;
			}
		} 
	}
	if (tag)
	{
		//再整理
		map<BYTE, MJ_PENG> hand;
		hand.clear();
		BYTE index = 0;
		for (BYTE i=0; i<peng.size();++i)
		{
			Getdata = peng.find(i);
			if (Getdata != peng.end())
			{
				if (Getdata->second.udata != 0)
				{
					hand.insert(map<BYTE, MJ_PENG>::value_type(index++, Getdata->second));
				}
			}
		}

		peng = hand;
	}

	return tag;
}
/*
@brief:判断三金倒胡牌
@param：byDeskStation:当前检测的位置;byFetchCard:的牌
@return:胡牌真否
*/
bool CBaseShowCardLogic::CanSanJinHuPai(BYTE byDeskStation,BYTE byFetchCard)
{
	BYTE byHandleCard[20];
	int index=0;
	memset(byHandleCard,255,sizeof(byHandleCard));
	UDT_MAP_MJ_DATA card;
	card = GetBase()->m_CardInfo[byDeskStation].hand;
	UDT_MAP_MJ_DATA::iterator ite=card.begin();
	for(;ite!=card.end();ite++)
	{
		byHandleCard[index]=ite->second;
		index++;
	}
	if(255!=byFetchCard)
	{
		byHandleCard[index]=byFetchCard;
	}
	int iJinNums=0;//金牌数量
	for(int i=0;i<20;i++)
	{
		if(255==byHandleCard[i])
		{
			continue;
		}
		if(GetBase()->m_WildCardInfo.uCard==byHandleCard[i])
		{
			iJinNums++;
		}
	}
	if(iJinNums>=3)
	{
		return true;
	}
	return false;
}
/*
@brief:判断胡牌
@param：byDeskStation:当前检测的位置;byOutCard:别家出的牌;bZiMo:是自摸还是放炮
@return:胡牌真否
*/
bool CBaseShowCardLogic::CanHu(BYTE byDeskStation,BYTE byOutCard,bool bZiMo)
{
	memset(m_byArHandPai,255,sizeof(m_byArHandPai));//检测数组初始化
	memset(m_byJiangCard,255,sizeof(m_byJiangCard));
	BYTE byNoCaiShenPai[20];//不带财神的牌,白板替换的财神牌值不算
	int iBaiBanNums=0;//手中白板个数
	memset(byNoCaiShenPai,255,sizeof(byNoCaiShenPai));
	//添加byDeskStation位置玩家的手牌到检测数组中
	int index=0;
	UDT_MAP_MJ_DATA card;
	card = GetBase()->m_CardInfo[byDeskStation].hand;
	UDT_MAP_MJ_DATA::iterator ite=card.begin();
	for(;ite!=card.end();ite++)
	{
		m_byArHandPai[index]=ite->second;
		index++;
	}	
	//将别人出的牌添加到检测数组中
	m_byArHandPai[index]=byOutCard;
	//test
	/*BYTE g_byHandPai[17] = {1,2,3,5,6,7,27,34,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff};
	memcpy(m_byArHandPai,g_byHandPai,sizeof(g_byHandPai));
	GetBase()->m_WildCardInfo.uCard=27;
	GetBase()->m_WildCardInfo.bWildCard=true;*/
	
	//白板：白板代替了金牌所在的牌面，俗称假金。如金牌翻出来为3条，则白板作为3条，可以和4条5条组成顺子
	int iTemp=0;
	for(int i=0;i<20;i++)
	{
		if(GetBase()->m_WildCardInfo.uCard==m_byArHandPai[i] || 255==m_byArHandPai[i])
		{
			continue;
		}
		if(Eie_BASE_JIAN_BAI==m_byArHandPai[i])//白板替换成财神牌值
		{
			m_byArHandPai[i]=GetBase()->m_WildCardInfo.uCard;
			iBaiBanNums++;
		}
		byNoCaiShenPai[iTemp]=m_byArHandPai[i];
		iTemp++;
	}
	//手牌从小到大排序
	MaoPaoSort(m_byArHandPai,20,false);
	MaoPaoSort(byNoCaiShenPai,20,false);
	//牌数据转换
	ChangeHandPaiData(m_byArHandPai,20,byNoCaiShenPai,20,iBaiBanNums);	
	/*if(CheckQiDui())
	{
		return true;
	}*/
	if(CheckPingHu())
	{
		return true;
	}
	return false;
}
///平糊检测
bool CBaseShowCardLogic::CheckPingHu()
{
	//循环遍历
	m_TempHupaistruct.Init();
	m_hupaistruct.Init();
	BYTE pai[4]={255};
	bool caishen[4]={0};
	m_iMaxFan=0;
	
	//1个财神的检测,一个财神和一张牌做将
	if(m_byJingNum>=1 && m_byJingNum!=255)
	{
		if(m_byJingNum >= 0x02)
		{
			m_hupaistruct.Init();//初始化糊牌结构

			memset(caishen,0,sizeof(caishen));
			memset(pai,255,sizeof(pai));
			pai[0] = pai[1] = GetBase()->m_WildCardInfo.uCard;
			caishen[0] = true;
			caishen[1] = true;
			m_hupaistruct.AddData(TYPE_JIANG_PAI,pai,caishen);
			MakePingHu(m_NoJingHuTempData,m_hupaistruct,m_byJingNum-2);//胡牌检测递归函数：在递归中完成全部检测

		}

		for(int i=0;i<m_NoJingHuTempData.conut;i++)
		{
			if(m_NoJingHuTempData.data[i][1] <1 || m_NoJingHuTempData.data[i][0] == 255)
			{
				continue;
			}
			if(m_NoJingHuTempData.data[i][1]>=2)
			{
				m_hupaistruct.Init();//初始化糊牌结构
				m_NoJingHuTempData.data[i][1]-=2;
				memset(caishen,0,sizeof(caishen));
				memset(pai,255,sizeof(pai));
				pai[0] = pai[1] = m_NoJingHuTempData.data[i][0];
				caishen[0] = true;
				m_hupaistruct.AddData(TYPE_JIANG_PAI,pai,caishen);
				MakePingHu(m_NoJingHuTempData,m_hupaistruct,m_byJingNum);//胡牌检测递归函数：在递归中完成全部检测
				m_NoJingHuTempData.data[i][1]+=2;
			}
			else
			{
				m_hupaistruct.Init();//初始化糊牌结构
				m_NoJingHuTempData.data[i][1]-=1;
				memset(caishen,0,sizeof(caishen));
				memset(pai,255,sizeof(pai));
				pai[0] = pai[1] = m_NoJingHuTempData.data[i][0];
				caishen[0] = true;
				m_hupaistruct.AddData(TYPE_JIANG_PAI,pai,caishen);
				MakePingHu(m_NoJingHuTempData,m_hupaistruct,m_byJingNum-1);//胡牌检测递归函数：在递归中完成全部检测
				m_NoJingHuTempData.data[i][1]+=1;
			}
		}
	}
	else
	{
		//正常牌做的检测,(无精的麻将HuTempData 与 NoJingHuTempData是一样的数据)
		for(int i=0;i<m_NoJingHuTempData.conut;i++)
		{
			if(m_NoJingHuTempData.data[i][1]<2 || m_NoJingHuTempData.data[i][0]==255)
				continue;
			m_hupaistruct.Init();//初始化糊牌结构
			m_NoJingHuTempData.data[i][1]-=2;
			memset(caishen,0,sizeof(caishen));
			memset(pai,255,sizeof(pai));
			pai[0] = pai[1] = m_NoJingHuTempData.data[i][0];
			m_hupaistruct.AddData(TYPE_JIANG_PAI,pai,caishen);
			MakePingHu(m_NoJingHuTempData,m_hupaistruct,m_byJingNum);//胡牌检测递归函数：在递归中完成全部检测
			m_NoJingHuTempData.data[i][1]+=2;
		}
	}
	if(m_iMaxFan>0)
	{
		//test
		//记录胡牌数据
		CString cs="";
		CString cs1="";
		for(int i=0;i<20;i++)
		{
			if(255!=m_byArHandPai[i])
			{
				cs1.Format(" %d ",m_byArHandPai[i]);
				cs.Append(cs1);
			}
		}
		GetBase()->DebugPrintf("胡牌玩家手里的牌=%s",cs);
		//m_hupaistruct = m_TempHupaistruct;
		return true;
	}
	return false;
}
//平糊组牌
bool CBaseShowCardLogic::MakePingHu(CheckHuStruct &PaiData,PingHuStruct &hupaistruct,int csnum)
{
	if(PaiData.GetAllPaiCount()<=0)//已经糊牌
	{
		for(int i=0;i<6;i++)
		{
			if(TYPE_JIANG_PAI==m_hupaistruct.byType[i] && m_hupaistruct.isjing[i][0])//是将牌且是财神替代
			{
				m_byJiangCard[0]=m_hupaistruct.data[i][0];
				m_byJiangCard[1]=m_hupaistruct.data[i][1];
				break;
			}
		}
		////检测牌型，记录最大牌型
		m_iMaxFan=1;
		//return GetMaxCombination(m_bZimo,true,m_byLastPai);
		return true;
	}
	//取牌
	BYTE pai = 0xff;
	int count = 0x00;
	int ijing = csnum;
	for(int i = 0x00; PaiData.conut;i ++)
	{
		if(PaiData.data[i][1] == 0xff || PaiData.data[i][1] == 0x00)
			continue;
		pai = PaiData.data[i][0];
		count = PaiData.data[i][1];
		break;
	}
	BYTE data[4];
	bool caishen[4];
	memset(data,0xff,sizeof(data));
	memset(caishen,0x00,sizeof(caishen));
	data[0] = pai;
	data[1] = pai;
	data[2] = pai;
	if(count >= 0x03)
	{
		memset(caishen,0x00,sizeof(caishen));
		hupaistruct.AddData(TYPE_AN_KE,data,caishen);
		PaiData.SetPaiCount(pai,count - 3);
		MakePingHu(PaiData,hupaistruct,csnum);
		PaiData.SetPaiCount(pai,count);
		hupaistruct.DeleteData(TYPE_AN_KE,data,caishen);
	}
	if(pai<30)//东南西北红中发财不能做顺子
	{
		if(pai % 10 < 0x08 && PaiData.GetPaiCount(pai + 1) >= 0x01 && PaiData.GetPaiCount(pai + 2) >= 0x01)	//组成 pai + (pai + 1) + (pai + 2) 的顺子
		{
			memset(caishen,0x00,sizeof(caishen));
			data[0] = pai;
			data[1] = pai + 1;
			data[2] = pai + 2;
			hupaistruct.AddData(TYPE_SHUN_ZI,data,caishen);
			PaiData.SetPaiCount(pai,count - 1);
			PaiData.SetPaiCount(pai + 1,PaiData.GetPaiCount(pai + 1) - 1);
			PaiData.SetPaiCount(pai + 2,PaiData.GetPaiCount(pai + 2) - 1);
			MakePingHu(PaiData,hupaistruct,csnum);
			PaiData.SetPaiCount(pai,count);		//检测完毕,恢复牌
			PaiData.SetPaiCount(pai + 1,PaiData.GetPaiCount(pai + 1) + 1);
			PaiData.SetPaiCount(pai + 2,PaiData.GetPaiCount(pai + 2) + 1);
			hupaistruct.DeleteData(TYPE_SHUN_ZI,data,caishen);
		}
	}
	
	//if(pai >= 10)//字牌，只考虑刻子 10只是测试用
	//{
	//	return false;
	//}
	if(csnum >= 1 && count>=2)
	{
		memset(caishen,0x00,sizeof(caishen));
		caishen[0] = true;
		hupaistruct.AddData(TYPE_AN_KE,data,caishen);
		PaiData.SetPaiCount(pai,count - 2);
		MakePingHu(PaiData,hupaistruct,csnum - 1);
		PaiData.SetPaiCount(pai,count);
		hupaistruct.DeleteData(TYPE_AN_KE,data,caishen);
	}
	if(csnum >= 0x01 && pai % 10 >= 0x02 && pai % 10 < 0x09 && PaiData.GetPaiCount(pai + 1) >= 0x01)	//组成 x + pai + (pai + 1) 的顺子(x是财神)
	{
		memset(caishen,0x00,sizeof(caishen));
		caishen[0] = true;
		data[0] = pai - 1;
		data[1] = pai;
		data[2] = pai + 1;
		hupaistruct.AddData(TYPE_SHUN_ZI,data,caishen);
		PaiData.SetPaiCount(pai,count - 1);
		PaiData.SetPaiCount(pai + 1,PaiData.GetPaiCount(pai + 1) - 1);
		MakePingHu(PaiData,hupaistruct,csnum - 1);
		PaiData.SetPaiCount(pai,count);
		PaiData.SetPaiCount(pai + 1,PaiData.GetPaiCount(pai +1) + 1);
		hupaistruct.DeleteData(TYPE_SHUN_ZI,data,caishen);
	}
	if(csnum>=1 && pai%10<8 &&  PaiData.GetPaiCount(pai+1)<1 && PaiData.GetPaiCount(pai+2)>=1)//组成 pai+(x)+(pai+2) 的顺子
	{
		memset(caishen,0,sizeof(caishen));
		caishen[1] = true;
		data[0] = pai;
		data[1] = pai+1;
		data[2] = pai+2;
		hupaistruct.AddData(TYPE_SHUN_ZI,data,caishen);//添加组牌：顺子
		PaiData.SetPaiCount(pai,count-1);//设置牌
		PaiData.SetPaiCount(pai+2,PaiData.GetPaiCount(pai+2)-1);//设置牌
		MakePingHu(PaiData,hupaistruct,csnum-1);
		PaiData.SetPaiCount(pai,count);//检测完毕恢复牌
		PaiData.SetPaiCount(pai+2,PaiData.GetPaiCount(pai+2)+1);//设置牌
		hupaistruct.DeleteData(TYPE_SHUN_ZI,data,caishen);//恢复组牌	
	}
	if(csnum>=1 && pai%10<8 && PaiData.GetPaiCount(pai+1)>=1 && PaiData.GetPaiCount(pai+2)<1)//组成 pai+(pai+1)+(x) 的顺子
	{
		memset(caishen,0,sizeof(caishen));
		caishen[2] = true;
		data[0] = pai;
		data[1] = pai+1;
		data[2] = pai+2;
		hupaistruct.AddData(TYPE_SHUN_ZI,data,caishen);//添加组牌：顺子
		PaiData.SetPaiCount(pai,count-1);//设置牌
		PaiData.SetPaiCount(pai+1,PaiData.GetPaiCount(pai+1)-1);//设置牌
		MakePingHu(PaiData,hupaistruct,csnum-1);
		PaiData.SetPaiCount(pai,count);//检测完毕恢复牌
		PaiData.SetPaiCount(pai+1,PaiData.GetPaiCount(pai+1)+1);//设置牌
		hupaistruct.DeleteData(TYPE_SHUN_ZI,data,caishen);//恢复组牌	
	}
	if(csnum > 0x02)
	{
		caishen[0] = true;
		caishen[1] = true;
		hupaistruct.AddData(TYPE_AN_KE,data,caishen); 
		PaiData.SetPaiCount(pai,count-1);				//设置牌
		MakePingHu(PaiData,hupaistruct,csnum-2);		//递归
		PaiData.SetPaiCount(pai,count);					//检测完毕恢复牌
		hupaistruct.DeleteData(TYPE_AN_KE,data,caishen);//恢复组牌
	}
	//万筒条,考虑顺子
	if((csnum >= 0x02) &&(pai %10 >= 0x03))		//x + x + pai 的顺子(x是财神)
	{
		memset(caishen,0x00,sizeof(caishen));
		caishen[0] = true;
		caishen[1] = true;
		data[0] = pai - 0x02;
		data[1] = pai - 0x01;
		data[2] = pai;
		hupaistruct.AddData(TYPE_SHUN_ZI,data,caishen);
		PaiData.SetPaiCount(pai,count - 1);
		MakePingHu(PaiData,hupaistruct,csnum - 2);
		PaiData.SetPaiCount(pai,count);
		hupaistruct.DeleteData(TYPE_SHUN_ZI,data,caishen);
	}
	if(csnum >= 0x02 && pai % 10 >= 0x02 && pai % 10 < 0x09)	//组成 x + pai + x 的顺子(x是财神)
	{
		memset(caishen,0x00,sizeof(caishen));
		caishen[0] = true;
		caishen[2] = true;
		data[0] = pai - 1;
		data[1] = pai;
		data[2] = pai + 1;
		hupaistruct.AddData(TYPE_SHUN_ZI,data,caishen);	//添加组牌:顺子
		PaiData.SetPaiCount(pai,count - 1);
		MakePingHu(PaiData,hupaistruct,csnum - 2);
		PaiData.SetPaiCount(pai,count);
		hupaistruct.DeleteData(TYPE_SHUN_ZI,data,caishen);	//恢复组牌
	}
	if(csnum>=2 && pai%10<8 )//组成 pai+(x)+(x) 的顺子
	{
		memset(caishen,0,sizeof(caishen));
		caishen[1] = true;
		caishen[2] = true;
		data[0] = pai;
		data[1] = pai+1;
		data[2] = pai+2;
		hupaistruct.AddData(TYPE_SHUN_ZI,data,caishen);//添加组牌：顺子
		PaiData.SetPaiCount(pai,count-1);//设置牌
		MakePingHu(PaiData,hupaistruct,csnum-2);
		PaiData.SetPaiCount(pai,count);//检测完毕恢复牌
		hupaistruct.DeleteData(TYPE_SHUN_ZI,data,caishen);//恢复组牌	
	}
	return false;	
}
//七对：由7个对子组成和牌。不计不求人、单钓  (非平糊类型)
bool CBaseShowCardLogic::CheckQiDui()
{
	int iDoubleNums = 0;
	for(int i=0;i<m_HuTempData.conut;++i)
	{
		if(0==m_HuTempData.data[i][1]%2)
		{
			iDoubleNums++;
		}
	}
	if(7==iDoubleNums)
	{
		return true;
	}
	else
	{
		return false;
	}
}
///名称：ChangeHandPaiData
///描述：转换手牌数据的存储方式，为了方便糊牌检测
///@param handpai[] 手牌数组, pai 别人打出的牌
///@return 
void CBaseShowCardLogic::ChangeHandPaiData(BYTE byArHandPai[],int iCount,BYTE byNoCaiShenPai[],int NoCaiShenCount,int iBaiBanNums)
{			
	m_HuTempData.Init();
	m_NoJingHuTempData.Init();///糊牌检测临时数据(无财神)	
	m_byJingNum = 0;
	for(int i=0;i<iCount;i++)
	{
		if(byArHandPai[i] != 255 )
		{
			if(GetBase()->m_WildCardInfo.uCard==byArHandPai[i])//财神牌
			{
				m_byJingNum++;
			}
			m_HuTempData.Add(byArHandPai[i]);
		}
	}
	for(int i=0;i<NoCaiShenCount;i++)
	{
		if(byNoCaiShenPai[i] != 255 )
		{
			m_NoJingHuTempData.Add(byNoCaiShenPai[i]);//添加无财神的牌
		}
	}
	m_byJingNum-=iBaiBanNums;
}
/*
@brief:冒泡排序
@param：a:排序数组;count:牌数量;big:排序顺序:false:从小到大，true：从大到小
*/
void CBaseShowCardLogic::MaoPaoSort(BYTE a[], int count,bool big)
{
	if(a == NULL)
		return ;
	for (int i =0; i < count - 1; ++i )
	{
		for(int j = i+1; j < count; ++j)
		{
			if(big)
			{
				if(a[j] > a[i])
				{
					BYTE iTempBig = a[j];
					a[j] = a[i];
					a[i] = iTempBig;		
				}
			}
			else
			{
				if(a[j] < a[i])
				{
					BYTE iTempBig = a[j];
					a[j] = a[i];
					a[i] = iTempBig;		
				}
			}
		}	
	}
}