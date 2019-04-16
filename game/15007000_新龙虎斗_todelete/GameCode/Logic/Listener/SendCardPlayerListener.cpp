//SendCardPlayerListener.cpp
//发牌阶段监听器[只发每个人自己的牌]
#include "stdafx.h"
#include "../../Server/ServerManage.h"
#include "SendCardPlayerListener.h"
#include "ExtensionLogger.h"
#include "ExtensionTimer.h"
#include "Algorithm.h"
SendCardPlayerListener::SendCardPlayerListener()
{
	m_State = GS_SEND_CARD;
}

SendCardPlayerListener::~SendCardPlayerListener()
{
}

void SendCardPlayerListener::OnAdd()
{
	//mark
	__super::OnAdd();
	m_Context->GetGameDesk()->SetGameStation(m_State);

	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	
	exDataMgr->m_haveTimeSs=0;


	void *pointers[] = {this};
	OBJ_GET_EXT(m_Context, ExtensionTimer, extTimer);
	extTimer->Add(2.0f,TimerData(pointers,ARRAY_LEN(pointers),DelayWait));
	if (!exDataMgr->m_isSupSet && exDataMgr->m_bAIWinAndLostAutoCtrl)
	{
		countScore();
	}
}

bool SendCardPlayerListener::countScore()
{
	//是否全是机器人
	bool bHaveUser = false;
	//宏
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	DataManage::sGameUserInf userinf;
	
	for (int i=0; i<PLAY_COUNT; i++)
	{
		if (NULL == m_Context->GetGameDesk()->m_pUserInfo[i])
		{
			continue;
		}
		if(!exDataMgr->getUserInfo(i,userinf))continue;
		if (!userinf.bIsVirtual && (userinf.i64UserXiaZhuData[0] > 0 || userinf.i64UserXiaZhuData[1] > 0 || userinf.i64UserXiaZhuData[2] > 0))
		{
			bHaveUser = true;
			break;
		}
	}
	//全是机器人就不去控制了或玩家没下注
	if(!bHaveUser)
	{
		return true;
	}

	srand((unsigned)GetTickCount());
	bool bAIWin = false;
	int iResult = Algor::rand_Mersenne(0,99);

	if (0 >=exDataMgr->m_i64AIHaveWinMoney )
	{
		//机器人赢的钱少于0 机器人必赢
		bAIWin = true;		
	}
	else if((0 < exDataMgr->m_i64AIHaveWinMoney) && (exDataMgr->m_i64AIHaveWinMoney < exDataMgr->m_i64AIWantWinMoneyA1))
	{
		// 机器人的赢钱在0-A1区域 并且概率符合m_iAIWinLuckyAtA1 机器人要赢
		if (iResult <=exDataMgr->m_iAIWinLuckyAtA1)
		{
			bAIWin = true;		
		}
	}
	else if ((exDataMgr->m_i64AIWantWinMoneyA1 <= exDataMgr->m_i64AIHaveWinMoney)  && (exDataMgr->m_i64AIHaveWinMoney <exDataMgr->m_i64AIWantWinMoneyA2))
	{
		// 机器人的赢钱在A1-A2区域 并且概率符合m_iAIWinLuckyAtA2 机器人要赢
		if (iResult <=exDataMgr->m_iAIWinLuckyAtA2)
		{
			bAIWin = true;			
		}
	}
	else if ((exDataMgr->m_i64AIWantWinMoneyA2 <= exDataMgr->m_i64AIHaveWinMoney)  && (exDataMgr->m_i64AIHaveWinMoney <exDataMgr->m_i64AIWantWinMoneyA3))
	{
		// 机器人的赢钱在A2-A3区域 并且概率符合m_iAIWinLuckyAtA3 机器人要赢
		if (iResult <=exDataMgr->m_iAIWinLuckyAtA3)
		{
			bAIWin = true;
		}
	}
	else
	{
		// 机器人的赢钱超过A3区域 并且概率符合m_iAIWinLuckyAtA4 机器人要赢
		if (iResult <=exDataMgr->m_iAIWinLuckyAtA4)
		{
			bAIWin = true;
		}
	}

	
	if(bAIWin)
	{
		//todo 加入平局概念
		if (rand() % 100 <= exDataMgr->m_pingjuPro)
			pingjuFunction();

		
		//直接翻牌
		else   CountUesrWinMoney();
	}
	else
	{
		__int64 i64Money = 0;
		for(int i=0; i<PLAY_COUNT; i++)
		{
			if(!exDataMgr->getUserInfo(i,userinf))continue;
			if (userinf.bIsVirtual) continue;

			// 规则改变
			if(exDataMgr->byWinQuYu==3)  //和
			{
				//i64Money += userinf.i64UserXiaZhuData[2]*8-userinf.i64UserXiaZhuData[2];
				//龙虎区 各返还一半
				i64Money += userinf.i64UserXiaZhuData[2]*8-userinf.i64UserXiaZhuData[2]-userinf.i64UserXiaZhuData[0]/2-userinf.i64UserXiaZhuData[1]/2;
			}
			else if(exDataMgr->byWinQuYu==1)
			{
				//龙
				i64Money += userinf.i64UserXiaZhuData[0]*2-userinf.i64UserXiaZhuData[1]-userinf.i64UserXiaZhuData[2]-userinf.i64UserXiaZhuData[0];
			}
			else if(exDataMgr->byWinQuYu==2)
			{
				//虎
				i64Money += userinf.i64UserXiaZhuData[1]*2-userinf.i64UserXiaZhuData[0]-userinf.i64UserXiaZhuData[2]-userinf.i64UserXiaZhuData[1];
			}
		}

		if(i64Money > 0)
		{
			//真人如果赢了，但赢太多还是让真人输
			if ((exDataMgr->m_i64AIHaveWinMoney-i64Money) <= 0)
			{
				CountUesrWinMoney();
			}
		}
		else
		{
			//真人如果输了，让真人赢
			CountUesrLoseMoney();
		}
	}
	return true;
}

//系统lose
bool SendCardPlayerListener::CountUesrLoseMoney()
{
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	DataManage::sGameUserInf userinf;

	//找一个真人赢并且不会导致奖池为负的中奖区域。找不到的话就还是让机器人赢

	__int64 i64MoneyLong=0;
	__int64 i64MoneyHu=0;
	__int64 i64MoneyHe=0;

	for(int i=0; i<PLAY_COUNT; i++)
	{
		if(!exDataMgr->getUserInfo(i,userinf))continue;
		if (userinf.bIsVirtual) continue;

		//i64MoneyHe += userinf.i64UserXiaZhuData[2]*8-userinf.i64UserXiaZhuData[2];
		//Eil @ 20190313 龙虎区返一半,所以算法扣龙虎区各一半
		//和
		i64MoneyHe += userinf.i64UserXiaZhuData[2]*8-userinf.i64UserXiaZhuData[2]-userinf.i64UserXiaZhuData[0]/2-userinf.i64UserXiaZhuData[1]/2;
		
		//龙
		i64MoneyLong += userinf.i64UserXiaZhuData[0]*2-userinf.i64UserXiaZhuData[1]-userinf.i64UserXiaZhuData[2]-userinf.i64UserXiaZhuData[0];

		//虎
		i64MoneyHu += userinf.i64UserXiaZhuData[1]*2-userinf.i64UserXiaZhuData[0]-userinf.i64UserXiaZhuData[2]-userinf.i64UserXiaZhuData[1];

		
	}

	//Eil @ 20190315
	//没人投和的时候给玩家赢
	emWinAreaType byLoseQuYu = Area_Invalid;
	if(i64MoneyHe <= 0)
	{
		int tempmax=max(i64MoneyLong,i64MoneyHu);

		if(tempmax==i64MoneyLong && exDataMgr->m_i64AIHaveWinMoney - i64MoneyLong >0 && tempmax > 0)
			byLoseQuYu = Area_Long;

		else if(tempmax == i64MoneyHu && exDataMgr->m_i64AIHaveWinMoney - i64MoneyHu >0 && tempmax > 0)
			byLoseQuYu = Area_Hu;
	}
	//有人投和的时候需要注意.压低开和的概率
	else 
	{
		
		if(rand()%100 > 85) byLoseQuYu = Area_He;

		else if(i64MoneyLong == i64MoneyHu) 
			byLoseQuYu=rand()%2?Area_Long:Area_Hu;

		else 
			byLoseQuYu=i64MoneyLong>i64MoneyHu?Area_Long:Area_Hu;
	}
	//
	/*
	if(i64MoneyLong > 0 && exDataMgr->m_i64AIHaveWinMoney - i64MoneyLong >0)
	{
		byLoseQuYu = Area_Long;
	}
	else if(i64MoneyHu > 0 && exDataMgr->m_i64AIHaveWinMoney - i64MoneyHu >0)
	{
		byLoseQuYu = Area_Hu;
	}
	else if(i64MoneyHe > 0 && exDataMgr->m_i64AIHaveWinMoney - i64MoneyHe > 0)
	{
		//Eil @ todo 将"和"的概率再压下去 
		
		if(rand()%100 > 90)  byLoseQuYu = Area_He; 		
	}
	*/
	

	if(Area_Invalid == byLoseQuYu)
	{
		//找不到满足条件的话还是让机器人赢
		CountUesrWinMoney();
	}
	else
	{
		//开一个byLoseQuyu对应的牌型
		GenerateByType(byLoseQuYu);
		exDataMgr->byWinQuYu = byLoseQuYu;
	}
	return true;
}

//根据区域类型生成牌型
void SendCardPlayerListener::GenerateByType(emWinAreaType emType)
{
	if(emType == Area_Invalid) return;
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	vector<int> vAllCard;
	const int iMaxSize = 13;
	for(int i = 1; i <= iMaxSize; ++i)
		vAllCard.push_back(i);

	switch(emType)
	{
	case Area_Long:
		{
			int iLong = rand() % 12 + 1;		//1到12
			int iHu = iLong + (rand() % (iMaxSize - iLong) + 1);		// iLong到后面的值
			CardBase::GetRandOneCard( iLong, exDataMgr->vSendCard[1] );		//虎来接收龙的牌
			CardBase::GetRandOneCard( iHu, exDataMgr->vSendCard[0] );
		}
		break;
	case Area_Hu:
		//龙随机前12,虎随机龙右边的数值
		{
			int iLong = rand() % 12 + 1;		//1到12
			int iHu = iLong + (rand() % (iMaxSize - iLong) + 1);		// iLong到后面的值
			CardBase::GetRandOneCard( iLong, exDataMgr->vSendCard[0] );
			CardBase::GetRandOneCard( iHu, exDataMgr->vSendCard[1] );
		}
		break;
	case Area_He:
		{
			CardBase::GetRandTwoCard(rand() % iMaxSize + 1, exDataMgr->vSendCard);
		}
		break;
	default:
		break;
	}
}

//系统win
bool SendCardPlayerListener::CountUesrWinMoney()
{
	srand((unsigned int)time(NULL));
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	DataManage::sGameUserInf userinf;
	__int64 i64MoneyLong=0;
	__int64 i64MoneyHu=0;
	__int64 i64MoneyHe=0;
	for(int i=0; i<PLAY_COUNT; i++)
	{
		if(!exDataMgr->getUserInfo(i,userinf))continue;
		if (userinf.bIsVirtual) continue;
		//统计和的钱
		//和
		//i64MoneyHe += userinf.i64UserXiaZhuData[2]*8-userinf.i64UserXiaZhuData[2];
		i64MoneyHe += userinf.i64UserXiaZhuData[2]*8-userinf.i64UserXiaZhuData[2]-userinf.i64UserXiaZhuData[0]/2-userinf.i64UserXiaZhuData[1]/2;

		//龙
		i64MoneyLong += userinf.i64UserXiaZhuData[0]*2-userinf.i64UserXiaZhuData[1]-userinf.i64UserXiaZhuData[2]-userinf.i64UserXiaZhuData[0];

		//虎
		i64MoneyHu += userinf.i64UserXiaZhuData[1]*2-userinf.i64UserXiaZhuData[0]-userinf.i64UserXiaZhuData[2]-userinf.i64UserXiaZhuData[1];
	}

	//真人最少的赢钱区域，让真人赢
	__int64 tempMin=min(i64MoneyHe,i64MoneyLong);
	tempMin=min(tempMin,i64MoneyHu);
	emWinAreaType tWinQuYu = Area_Invalid;

	//
	if(i64MoneyLong == i64MoneyHu && i64MoneyHe > 0 && i64MoneyHu < i64MoneyHe)
		tWinQuYu=rand()%2?Area_Long:Area_Hu;
	
	//
	else if (tempMin==i64MoneyHe)
	{
		tWinQuYu = Area_He;
	}
	else if (tempMin==i64MoneyLong)
	{
		tWinQuYu = Area_Long;
	}
	else 
	{
		tWinQuYu = Area_Hu;
	}
	if (tWinQuYu != exDataMgr->byWinQuYu)		//赢钱最多区域和实际开奖区域不一致
	{
		GenerateByType(tWinQuYu);
		exDataMgr->byWinQuYu = tWinQuYu;
		LOGGER_FILE(m_Context,"机器人1控制：："<< (int)exDataMgr->byWinQuYu); 
	}
	return true;
}



//mark
void SendCardPlayerListener::DelaySendCardOver(void **data, int dataCnt)
{
	SendCardPlayerListener *p = (SendCardPlayerListener *)data[0];
	if(nullptr == p) return ;
	if(nullptr == p->m_Context->GetGameDesk()) return;
	OBJ_GET_EXT(p->GetContext(),DataManage,exDataMgr);
	S_C_ShowWinAreas Noti;
	if(exDataMgr->byWinQuYu==3)
	{
		exDataMgr->byRunHeCount++;
	}
	else if(exDataMgr->byWinQuYu==1)
	{
		exDataMgr->byRunLongCount++;
	}
	else if(exDataMgr->byWinQuYu==2)
	{
		exDataMgr->byRunHuCount++;
	}

	exDataMgr->byRunSeq[exDataMgr->byGameBeen]=exDataMgr->byWinQuYu;
	Noti.byRunHeCount = exDataMgr->byRunHeCount;
	Noti.byWinQuYu = exDataMgr->byWinQuYu;
	Noti.byRunLongCount = exDataMgr->byRunLongCount;
	Noti.byRunHuCount = exDataMgr->byRunHuCount;
	Noti.iGameBeen = exDataMgr->byGameBeen;
	memcpy_s(Noti.byRunSeq,sizeof(Noti.byRunSeq),exDataMgr->byRunSeq,sizeof(Noti.byRunSeq));
	p->m_Context->GetGameDesk()->send2all((char*)&Noti,sizeof(Noti),S_C_SHOW_WINAREA);
	void *pointers[] = {p};
	OBJ_GET_EXT(p->GetContext(), ExtensionTimer, extTimer);
	extTimer->Add((float)2.7f,TimerData(pointers,ARRAY_LEN(pointers),DelayListen));
}

void SendCardPlayerListener::DelayWait(void **data, int dataCnt)
{
	SendCardPlayerListener *p = (SendCardPlayerListener *)data[0];
	if(nullptr == p) return ;
	if(nullptr == p->m_Context->GetGameDesk()) return;
	p->sendCard();
	void *pointers[] = {p};
	OBJ_GET_EXT(p->GetContext(), ExtensionTimer, extTimer);
	extTimer->Add((float)Config::iKaiPaiTime,TimerData(pointers,ARRAY_LEN(pointers),DelaySendCardOver));
	extTimer->Add(1.0f,TimerData(pointers,ARRAY_LEN(pointers),DelayTiming));
}

void SendCardPlayerListener::DelayListen(void **data, int dataCnt)
{
	SendCardPlayerListener *p = (SendCardPlayerListener *)data[0];
	if(nullptr == p) return ;
	if(nullptr == p->m_Context->GetGameDesk()) return;
	OBJ_GET_EXT(p->GetContext(),ExtensionListenerController,exListenerControl);
	exListenerControl->OnListenEnd(LT_SendCard_Player);
}

void SendCardPlayerListener::DelayTiming(void **data, int dataCnt)
{
	SendCardPlayerListener *p = (SendCardPlayerListener *)data[0];
	if(nullptr == p) return ;
	if(nullptr == p->m_Context->GetGameDesk()) return;
	OBJ_GET_EXT(p->GetContext(),DataManage,exDataMgr);
	exDataMgr->m_haveTimeSs++;
	if (exDataMgr->m_haveTimeSs <= Config::iKaiPaiTime-1)
	{
		void *pointers[] = {p};
		OBJ_GET_EXT(p->GetContext(), ExtensionTimer, extTimer);
		extTimer->Add((float)1.0f,TimerData(pointers,ARRAY_LEN(pointers),DelayTiming));
	}
}

void SendCardPlayerListener::OnRemove()
{
	__super::OnRemove();
}

void SendCardPlayerListener::OnTimer( uint timerID )
{

}

void SendCardPlayerListener::OnReconnect( uchar playerPos, uint socketID, bool isWatchUser )
{
 	if(m_Context->GetGameDesk()->getGameStation() != GS_SEND_CARD) return;
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	GameStation_SendCard TGameStation;
	TGameStation.iXiaZhuTime=Config::iXiaZhuTime;
	TGameStation.iKaiPaiTime=Config::iKaiPaiTime;
	TGameStation.iFreeTime = Config::iFreeTime;
	TGameStation.iGameBeen = exDataMgr->byGameBeen;
	TGameStation.byRunHeCount = exDataMgr->byRunHeCount;
	TGameStation.byRunHuCount = exDataMgr->byRunHuCount;
	TGameStation.byRunLongCount = exDataMgr->byRunLongCount;
	TGameStation.RemainTimeKP = Config::iKaiPaiTime-exDataMgr->m_haveTimeSs;
	DataManage::sGameUserInf usr;
	if(!exDataMgr->getUserInfo(playerPos,usr))
	{
		LOGGER_ERROR_FILE(m_Context,"有玩家消失了强制结束游戏");
		return ;
	}
	TGameStation.i64MyMoney=usr.iMoney-usr.iXiaZhuMoney;
	memcpy_s(TGameStation.byRunSeq,sizeof(TGameStation.byRunSeq),exDataMgr->byRunSeq,sizeof(TGameStation.byRunSeq));
	memcpy_s(TGameStation.i64QuYuZhu,sizeof(TGameStation.i64QuYuZhu),exDataMgr->i64QuYuZhu,sizeof(TGameStation.i64QuYuZhu));
	memcpy_s(TGameStation.i64UserXiaZhuData,sizeof(TGameStation.i64UserXiaZhuData),usr.i64UserXiaZhuData,sizeof(TGameStation.i64UserXiaZhuData));
	memcpy_s(TGameStation.rAresData,sizeof(TGameStation.rAresData),exDataMgr->rAresData,sizeof(TGameStation.rAresData));
	memset(TGameStation.userMaxMoney,255,sizeof(TGameStation.userMaxMoney));
	exDataMgr->updateDataMoney();
	exDataMgr->ProcessData(TGameStation.userMaxMoney);
	if(TGameStation.userMaxMoney[1]!= 255)
	{
		if(!exDataMgr->getUserInfo(TGameStation.userMaxMoney[1],usr))
		{
			LOGGER_ERROR_FILE(m_Context,"有玩家消失了强制结束游戏");
			return ;
		}
		if (usr.i64UserXiaZhuData[0] > 0)
		{
			TGameStation.bIsShenSuanBet[0]= true;
		}
		if (usr.i64UserXiaZhuData[1] > 0)
		{
			TGameStation.bIsShenSuanBet[1]= true;
		}
		if (usr.i64UserXiaZhuData[2] > 0)
		{
			TGameStation.bIsShenSuanBet[2]= true;
		}
	}
	TGameStation.byShowCard[0]=exDataMgr->vSendCard[0].getData();
	TGameStation.byShowCard[1]=exDataMgr->vSendCard[1].getData();
	m_Context->GetGameDesk()->sendState(playerPos,(char*)&TGameStation,sizeof(TGameStation),socketID,isWatchUser);
}

void SendCardPlayerListener::Record( uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode )
{

}

void SendCardPlayerListener::Listen( uchar playerPos, uint msgID, void* msgData ,uint uSize)
{

}

void SendCardPlayerListener::setFirstPlayer()
{

}

bool SendCardPlayerListener::sendCard()
{
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	S_C_SendCard Noti;
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if(exDataMgr->isNomalUser(i))
		{
			Noti.byShowCard[0]=exDataMgr->vSendCard[0].getData();
			Noti.byShowCard[1]=exDataMgr->vSendCard[1].getData();
			m_Context->GetGameDesk()->send2(i,(char*)&Noti,sizeof(Noti),S_C_SEND_CARD);
		}
	}	
	return true;
}

bool SendCardPlayerListener::checkKingFine(uchar playerPos)
{

	return false;
}

//todelete
bool SendCardPlayerListener::pingjuFunction()
{
	srand((unsigned int)time(NULL));
	OBJ_GET_EXT(m_Context, DataManage, exDataMgr);
	DataManage::sGameUserInf userinf;
	__int64 i64MoneyLong = 0;
	__int64 i64MoneyHu = 0;
	__int64 i64MoneyHe = 0;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (!exDataMgr->getUserInfo(i, userinf))continue;
		if (userinf.bIsVirtual) continue;
		//和
		//i64MoneyHe += userinf.i64UserXiaZhuData[2]*8-userinf.i64UserXiaZhuData[2];
		i64MoneyHe += userinf.i64UserXiaZhuData[2] * 8 ;

		//龙
		i64MoneyLong += userinf.i64UserXiaZhuData[0] * 2 ;

		//虎
		i64MoneyHu += userinf.i64UserXiaZhuData[1] * 2 ;
	}

	//找到个第二区域胜利的区域
	__int64 tempMax = max(i64MoneyHe, i64MoneyLong);
	tempMax = max(tempMax, i64MoneyHu);
	emWinAreaType tWinQuYu = Area_Invalid;


	//
	if (tempMax == i64MoneyHe)
	{
		__int64 tmp = max(i64MoneyHu, i64MoneyLong);
		tWinQuYu = (tmp == i64MoneyHu ? Area_Hu : Area_Long);
	}
	else if (tempMax == i64MoneyLong)
	{
		__int64 tmp = max(i64MoneyHe, i64MoneyHu);
		tWinQuYu = (tmp == i64MoneyHu ? Area_Hu : Area_He);
	}
	else
	{
		__int64 tmp = max(i64MoneyHe, i64MoneyLong);
		tWinQuYu = (tmp == i64MoneyLong ? Area_Long : Area_He);
	}

	if (i64MoneyLong == i64MoneyHu && tempMax == i64MoneyHu)
		tWinQuYu = rand() % 2 ? Area_Hu : Area_Long;

	if (tWinQuYu != exDataMgr->byWinQuYu)		//赢钱最多区域和实际开奖区域不一致
	{
	
		GenerateByType(tWinQuYu);
		exDataMgr->byWinQuYu = tWinQuYu;
		LOGGER_FILE(m_Context, "机器人1控制：：" << (int)exDataMgr->byWinQuYu);
	}
	return true;
}




