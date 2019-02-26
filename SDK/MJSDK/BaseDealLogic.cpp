#include "stdafx.h"
#include "BaseDealLogic.h"
#include "BaseMJLogic.h"


//构造函数		
CBaseDealLogic::CBaseDealLogic(void)
{

}
//析构函数
CBaseDealLogic::~CBaseDealLogic()
{

}

//发牌，设置玩家手牌(同时是会影响到牌墙的)//先扔色子再摸牌
bool CBaseDealLogic::SetUserHandCard()
{
	//通过色子信息确定从哪个方位开始抓，并且是第几屯开始
	//GetBase()->m_uFirstHandCard;

	return true;
}

//发牌，设置玩家手牌(同时是会影响到牌墙的)//先摸牌再扔色子
bool CBaseDealLogic::SetUserHandCardEx()
{
	//先摸牌再扔色子 最简单，先将每个人面前的牌墙对应取出起手手牌数目的牌给自己即可
	BYTE midd;
	UDT_MAP_MJ_DATA::iterator Getdata;

	//获取抓牌方位和第几屯
	BYTE uResPosition = 0;
	BYTE uPosition = GetFirstPosition(0,true);
	BYTE uFirstPort = GetFirstPort(uPosition, uResPosition, 1, true);

	if (uFirstPort > 0)
	{
		uFirstPort = (uFirstPort-1) * GetBase()->m_uNumForTun/* - 1*/;//一屯可能不止一张,现在转换为第几张牌
	}
	if (uFirstPort == 255)
	{
		uFirstPort = 0;
	}

	midd = 0;
	//开始摸牌
	for (Getdata=GetBase()->m_CardInfo[uResPosition].wall.begin();Getdata!=GetBase()->m_CardInfo[uResPosition].wall.end();++Getdata)
	{
		if (Getdata->second != 0)//存在牌
		{
			if (uFirstPort == 1)//摸的牌前一张停下来
			{
				//摸牌位置记录到出牌状态里，这里就是开始摸牌的位置了
				GetBase()->m_MyShowCardLogic.SetNowTakePortInfo(uResPosition, Getdata->first);
				BYTE middPosition=0;
				GetBase()->m_first_PostInfo.uFirstPort = GetBase()->m_MyDealLogic.GetNextPort(GetBase()->m_MJ_TakeCardPortInfo.uDesk, middPosition, GetBase()->m_MJ_TakeCardPortInfo.uPort, !GetBase()->m_DrawOrder);
				GetBase()->m_first_PostInfo.uPosition = middPosition;

				break;
			}
			else
			{
				uFirstPort--;
			}
		}	
	}
	
	//每人摸指定的牌
	for (BYTE i=0;i<PLAY_COUNT;++i)
	{
		for (BYTE j=0;j<GetBase()->GetFirstHandCard();++j)
		{
			GetBase()->m_MyShowCardLogic.Take(i, Eie_TAKE_NORMAL, midd);
			GetBase()->AddCard(GetBase()->m_CardInfo[i].hand, midd);
		}
	}
	//庄家多摸一张
	BYTE BankerID = GetBase()->m_iBankerID[0];

	GetBase()->m_MyShowCardLogic.Take(BankerID, Eie_TAKE_NORMAL, midd);
	GetBase()->AddCard(GetBase()->m_CardInfo[BankerID].hand, midd);
	GetBase()->m_CardInfo[BankerID].bTakeCard = midd;

	//记录摸到哪了
	GetBase()->m_first_PostInfo.uEndPosition = GetBase()->m_MJ_TakeCardPortInfo.uDesk;
	GetBase()->m_first_PostInfo.uEndPort = GetBase()->m_MJ_TakeCardPortInfo.uPort;

	return true;
}

//根据色子得出的开始抓牌方位(从扔色子的位置开始顺时针计算)
BYTE CBaseDealLogic::GetFirstPosition(BYTE bDesk, BYTE uTime, bool clockwise)
{
	if (uTime >= GetBase()->m_MJ_ShaiZi_Database.utime)
	{
		return 0;
	}
	//点数
	BYTE uport = GetBase()->m_MyFixBankerLogic.GetShaiZiResult(uTime);

	for(BYTE i = bDesk ;;i = GetBase()->GetNext(i, PLAY_COUNT, clockwise))
	{
		uport--;
		if(uport == 0)
			return i;
	}
}

//根据色子得出的开始抓牌方位(属于系统扔色子)
BYTE CBaseDealLogic::GetFirstPosition(BYTE uTime, bool clockwise)
{
	return 0;
}

//根据色子得出抓某一方位抓第几屯（几个为1屯）
BYTE CBaseDealLogic::GetFirstPort(BYTE uStarPosition, BYTE &uResPosition, BYTE uTime, bool clockwise)
{
	BYTE uError = GetBase()->m_MJ_ShaiZi_Database.uAllNum;//错误就设定为2，因为N个色子最小点数和为N*1
	uResPosition = uStarPosition;
	//已经知道是哪个方位了
	if (uStarPosition >= PLAY_COUNT)
	{
		return uError;
	}

	if (uTime >= GetBase()->m_MJ_ShaiZi_Database.utime)
	{
		return uError;
	}
	//点数
	BYTE uport = GetBase()->m_MyFixBankerLogic.GetShaiZiResult(uTime);//两个色子范围则是2-12
	UDT_MAP_MJ_DATA::iterator Getdata;

	//计算总共所有人面前牌墙有多少屯
	BYTE uall = 0;
	BYTE ualluser[PLAY_COUNT] = {0};
	for (int i=0; i<PLAY_COUNT;++i)
	{
		for (int j=0;j<GetBase()->m_CardInfo[i].wall.size();++j)
		{
			Getdata = GetBase()->m_CardInfo[i].wall.find(j);
			if (Getdata != GetBase()->m_CardInfo[i].wall.end())
			{
				if (Getdata->second != 0)
				{
					uall++;
					ualluser[i]++;
				}
			} 
		}
	}
	uall = uall / GetBase()->m_uNumForTun;//变成屯数
	for (int i=0; i<PLAY_COUNT;++i)
	{
		ualluser[i] = ualluser[i] / GetBase()->m_uNumForTun;
	}

	if (uport > uall)//超过了所有人牌墙屯数之和，就得处理下，把范围缩小到一圈里面
	{
		uport = uport % uall;
	}

	for(BYTE i = uStarPosition ;;i = GetBase()->GetNext(i, PLAY_COUNT, clockwise))
	{
		if (uport > ualluser[i])
		{
			uport = uport - ualluser[i];
		}
		else
		{
			uResPosition = i;
			return uport;
		}
	}

	return uError;
}

//获取下一个牌墙节点(摸一张牌)//牌墙是个环
BYTE CBaseDealLogic::GetNextPort(BYTE uStarPosition, BYTE &uResPosition, BYTE uPort, bool clockwise)
{
	BYTE uError = 255;
	uResPosition = uStarPosition;
	//已经知道是哪个方位了
	if (uStarPosition >= PLAY_COUNT)
	{
		return uError;
	}

	//点数
	//BYTE uport = 1;
	UDT_MAP_MJ_DATA::iterator Getdata;
	UDT_MAP_MJ_DATA::reverse_iterator Getrdata;

	//计算总共所有人面前牌墙有多少张
	BYTE uall = 0;
	BYTE ualluser[PLAY_COUNT] = {0};
	for (int i=0; i<PLAY_COUNT;++i)
	{
		for (int j=0;j<GetBase()->m_CardInfo[i].wall.size();++j)
		{
			Getdata = GetBase()->m_CardInfo[i].wall.find(j);
			if (Getdata != GetBase()->m_CardInfo[i].wall.end())
			{
				if (Getdata->second != 0)
				{
					ualluser[i]++;
					uall++;
				}
			} 
		}
	}

	//没有牌了，返回
	if (uall == 0)
	{
		return uError;
	}

	BYTE bleng = PLAY_COUNT;//只用执行一个轮回即可
	for(BYTE i = uStarPosition ;;i = GetBase()->GetNext(i, PLAY_COUNT, clockwise))
	{
		if (clockwise)//顺时针就是递增
		{
			if (uStarPosition != i)
			{
				uPort = 0;
			}
			for (Getdata=GetBase()->m_CardInfo[i].wall.begin();Getdata!=GetBase()->m_CardInfo[i].wall.end();++Getdata)
			{
				if (uPort >= Getdata->first && uStarPosition == i)
				{
					continue;
				}

				if (uPort > Getdata->first && uStarPosition != i)
				{
					continue;
				}


				if (Getdata->second != 0)//存在牌
				{
					uResPosition = i;
					return Getdata->first;
				}
			}
		}
		else
		{
			if (uStarPosition != i)
			{
				uPort = GetBase()->m_CardInfo[i].wall.size()-1;//设置为最大那个位置也许为0也无所谓
			}
			for (Getrdata=GetBase()->m_CardInfo[i].wall.rbegin();Getrdata!=GetBase()->m_CardInfo[i].wall.rend();++Getrdata)
			{
				if (uPort <= Getrdata->first && uStarPosition == i)
				{
					continue;
				}

				if (uPort < Getrdata->first && uStarPosition != i)
				{
					continue;
				}

				if (Getrdata->second != 0)//存在牌
				{
					uResPosition = i;
					return Getrdata->first;
				}
			}
		}
		--bleng;
		if (bleng == 0)//还是没有找到出错了
		{
			return uError;
		}
	}


	//for(BYTE i = uStarPosition ;;i = GetBase()->GetNext(i, PLAY_COUNT, clockwise))
	//{
	//	if (uPort < GetBase()->m_CardInfo[i].wall.size())
	//	{
	//		if (clockwise)//顺时针就是递增
	//		{
	//			for (Getdata=GetBase()->m_CardInfo[i].wall.begin();Getdata!=GetBase()->m_CardInfo[i].wall.end();++Getdata)
	//			{
	//				if (Getdata->first < uPort)//前面的跳过
	//				{
	//					continue;
	//				}

	//				if (Getdata->second != 0)//存在牌
	//				{
	//					uResPosition = i;
	//					return Getdata->first;
	//				}
	//			}
	//		}
	//		else
	//		{
	//			for (Getrdata=GetBase()->m_CardInfo[i].wall.rbegin();Getrdata!=GetBase()->m_CardInfo[i].wall.rend();++Getrdata)
	//			{
	//				if (Getrdata->first > uPort)//前面的跳过
	//				{
	//					continue;
	//				}

	//				if (Getrdata->second != 0)//存在牌
	//				{
	//					uResPosition = i;
	//					return Getrdata->first;
	//				}
	//			}
	//		}
	//	}
	//	else
	//	{
	//		uPort -= GetBase()->m_CardInfo[i].wall.size();
	//		continue;
	//	}
	//}


	//for (int i=0; i<PLAY_COUNT;++i)
	//{
	//	ualluser[i] = ualluser[i] / GetBase()->m_uNumForTun;
	//}

	//for(BYTE i = uStarPosition ;;i = GetBase()->GetNext(i, PLAY_COUNT, clockwise))
	//{
	//	if (uport > ualluser[i])
	//	{
	//		uport = uport - ualluser[i];
	//	}
	//	else
	//	{
	//		uResPosition = i;
	//		break;
	//	}
	//}

	////存在就帮他找出该牌的数据key值
	//if (clockwise)//顺时针
	//{
	//	for (Getdata=GetBase()->m_CardInfo[uResPosition].wall.begin();Getdata!=GetBase()->m_CardInfo[uResPosition].wall.end();++Getdata)
	//	{
	//		if (Getdata->second != 0)//存在牌
	//		{
	//			return Getdata->first;
	//		}
	//	}
	//}
	//else
	//{
	//	for (Getrdata=GetBase()->m_CardInfo[uResPosition].wall.rbegin();Getrdata!=GetBase()->m_CardInfo[uResPosition].wall.rend();++Getrdata)
	//	{
	//		if (Getrdata->second != 0)//存在牌
	//		{
	//			return Getrdata->first;
	//		}
	//	}
	//}


	return uError;
}