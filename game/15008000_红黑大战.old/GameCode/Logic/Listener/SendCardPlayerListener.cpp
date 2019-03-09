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
	m_State = GS_OPEN_CARD;
}

SendCardPlayerListener::~SendCardPlayerListener()
{
}

void SendCardPlayerListener::OnAdd()
{
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
		CountUesrWinMoney();
	}
	else
	{
		__int64 i64Money = 0;
		for(int i=0; i<PLAY_COUNT; i++)
		{
			if(!exDataMgr->getUserInfo(i,userinf))continue;
			if (userinf.bIsVirtual) continue;

			//if(exDataMgr->byWinQuYu==3)  //和
			//{
			//	i64Money += userinf.i64UserXiaZhuData[2]*8-userinf.i64UserXiaZhuData[2];
			//}
			//else
			if(exDataMgr->byWinQuYu==1)
			{
				//红
				i64Money += userinf.i64UserXiaZhuData[0]*2-userinf.i64UserXiaZhuData[1]-userinf.i64UserXiaZhuData[2]-userinf.i64UserXiaZhuData[0];
				if (exDataMgr->WinUserShape > UG_DAN_ZHANG)
				{
					i64Money += userinf.i64UserXiaZhuData[2] * exDataMgr->getbeilv(exDataMgr->WinUserShape);
				}
			}
			else if(exDataMgr->byWinQuYu==2)
			{
				//黑
				i64Money += userinf.i64UserXiaZhuData[1]*2-userinf.i64UserXiaZhuData[0]-userinf.i64UserXiaZhuData[2]-userinf.i64UserXiaZhuData[1];
				if (exDataMgr->WinUserShape > UG_DAN_ZHANG)
				{
					i64Money += userinf.i64UserXiaZhuData[2] * exDataMgr->getbeilv(exDataMgr->WinUserShape);
				}
			}
		}
		if ((exDataMgr->m_i64AIHaveWinMoney-i64Money) <= 0)
		{
			CountUesrWinMoney();
		}
	}
	return true;
}

bool SendCardPlayerListener::CountUesrWinMoney()
{
	//return true;
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	DataManage::sGameUserInf userinf;
	__int64 i64MoneyHong=0;
	__int64 i64MoneyHei=0;
	__int64 i64MoneyTeShu=0;
	for(int i=0; i<PLAY_COUNT; i++)
	{
		if(!exDataMgr->getUserInfo(i,userinf))continue;
		if (userinf.bIsVirtual) continue;
		//特殊牌型
		i64MoneyTeShu += userinf.i64UserXiaZhuData[2]*8-userinf.i64UserXiaZhuData[2];

		//红
		i64MoneyHong += userinf.i64UserXiaZhuData[0]*2-userinf.i64UserXiaZhuData[1]-userinf.i64UserXiaZhuData[2]-userinf.i64UserXiaZhuData[0];

		//黑
		i64MoneyHei += userinf.i64UserXiaZhuData[1]*2-userinf.i64UserXiaZhuData[0]-userinf.i64UserXiaZhuData[2]-userinf.i64UserXiaZhuData[1];
	}

	__int64 tempMin=min(i64MoneyTeShu, i64MoneyHong);
	tempMin=min(tempMin, i64MoneyHei);
	BYTE tWinQuYu = 0;
	bool flagteshu = false;
	if (tempMin== i64MoneyTeShu)
	{
		flagteshu = true;//特殊牌型赢
	}
	if (tempMin== i64MoneyHong)
	{
		tWinQuYu = 1;
	}
	else 
	{
		tWinQuYu = 2;
	}

	if (flagteshu)
	{
		if (exDataMgr->WinUserShape == UG_DAN_ZHANG)
		{
			exDataMgr->gailvcontrol();
		}
	}
	else
	{
		//特殊牌型不能赢，所以控制两方为单张
		if (exDataMgr->WinUserShape != UG_DAN_ZHANG)
		{
			exDataMgr->gailvcontrol(true, tWinQuYu);
		}
	}
	//if (tWinQuYu != exDataMgr->byWinQuYu)
	//{
	//	if(exDataMgr->byWinQuYu == 3)//和局
	//	{
	//		if(exDataMgr->vSendCard[1].getRealVal()==14)
	//		{
	//			if(tWinQuYu == 1)
	//			{
	//				exDataMgr->vSendCard[0].setData(CardBase(D_10));
	//			}
	//			else if(tWinQuYu == 2)
	//			{
	//				exDataMgr->vSendCard[1].setData(CardBase(C_10));
	//			}
	//		}
	//		else
	//		{
	//			if(tWinQuYu == 1)
	//			{
	//				exDataMgr->vSendCard[1].setData(CardBase(H_A));
	//			}
	//			else if(tWinQuYu == 2)
	//			{
	//				exDataMgr->vSendCard[0].setData(CardBase(C_A));
	//			}
	//		}
	//	}
	//	else if(3 == tWinQuYu)
	//	{
	//		uchar type=exDataMgr->vSendCard[0].getType();
	//		if (type < TYPE_SPADE )
	//		{
	//			type+=1;
	//		}
	//		else 
	//		{
	//			type = 0;
	//		}
	//		uchar val=exDataMgr->vSendCard[0].getVal();
	//		exDataMgr->vSendCard[1].setData(CardBase((type<<4)+val));
	//	}
	//	else
	//	{
	//		std::swap(exDataMgr->vSendCard[0],exDataMgr->vSendCard[1]);
	//	}
	//	exDataMgr->byWinQuYu = tWinQuYu;
	//}
	return true;
}

void SendCardPlayerListener::DelaySendCardOver(void **data, int dataCnt)
{
	SendCardPlayerListener *p = (SendCardPlayerListener *)data[0];
	if(nullptr == p) return ;
	if(nullptr == p->m_Context->GetGameDesk()) return;
	OBJ_GET_EXT(p->GetContext(),DataManage,exDataMgr);
	S_C_ShowWinAreas Noti;
	/*if(exDataMgr->byWinQuYu==3)
	{
		exDataMgr->byRunHeCount++;
	}
	else*/ 
	if(exDataMgr->byWinQuYu==1)
	{
		exDataMgr->byRunHongCount++;
	}
	else if(exDataMgr->byWinQuYu==2)
	{
		exDataMgr->byRunHeiCount++;
	}
	exDataMgr->byRunSeq[exDataMgr->byGameBeen]=exDataMgr->byWinQuYu;
	exDataMgr->byRunCardShape[exDataMgr->byGameBeen] = exDataMgr->WinUserShape;
//	Noti.byRunHeCount = exDataMgr->byRunHeCount;
	Noti.byWinQuYu = exDataMgr->byWinQuYu;
	Noti.bSpecialType = (exDataMgr->WinUserShape > UG_DAN_ZHANG) ? true : false;
	Noti.byRunHongCount = exDataMgr->byRunHongCount;
	Noti.byRunHeiCount = exDataMgr->byRunHeiCount;
	Noti.iGameBeen = exDataMgr->byGameBeen;
	memcpy_s(Noti.byRunSeq,sizeof(Noti.byRunSeq),exDataMgr->byRunSeq,sizeof(Noti.byRunSeq));
	memcpy_s(Noti.byRunCardShape, sizeof(Noti.byRunCardShape), exDataMgr->byRunCardShape, sizeof(Noti.byRunCardShape));
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
 	if(m_Context->GetGameDesk()->getGameStation() != GS_OPEN_CARD) return;
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	GameStation_OpenCard TGameStation;
	TGameStation.iXiaZhuTime=Config::iXiaZhuTime;
	TGameStation.iKaiPaiTime=Config::iKaiPaiTime;
	TGameStation.iFreeTime = Config::iFreeTime;
	TGameStation.iGameBeen = exDataMgr->byGameBeen;
	memcpy(TGameStation.i64ChouMa, Config::chouma, sizeof(TGameStation.i64ChouMa));
//	TGameStation.byRunHeCount = exDataMgr->byRunHeCount;
	TGameStation.byRunHeiCount = exDataMgr->byRunHeiCount;
	TGameStation.byRunHongCount = exDataMgr->byRunHongCount;
	TGameStation.RemainTimeKP = Config::iKaiPaiTime-exDataMgr->m_haveTimeSs;
	DataManage::sGameUserInf usr;
	if(!exDataMgr->getUserInfo(playerPos,usr))
	{
		LOGGER_ERROR_FILE(m_Context,"有玩家消失了强制结束游戏");
		return ;
	}
	TGameStation.i64MyMoney=usr.iMoney-usr.iXiaZhuMoney;
	memcpy_s(TGameStation.byRunSeq,sizeof(TGameStation.byRunSeq),exDataMgr->byRunSeq,sizeof(TGameStation.byRunSeq));
	memcpy_s(TGameStation.byRunCardShape, sizeof(TGameStation.byRunCardShape), exDataMgr->byRunCardShape, sizeof(TGameStation.byRunCardShape));
	memcpy(TGameStation.CardShape, exDataMgr->UserShape, sizeof(TGameStation.CardShape));
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

	memcpy(TGameStation.byShowCard, exDataMgr->UserCard, sizeof(TGameStation.byShowCard));
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
	S_C_OpenCard Noti;
	memcpy(Noti.byShowCard, exDataMgr->UserCard, sizeof(Noti.byShowCard));
	memcpy(Noti.CardShape, exDataMgr->UserShape, sizeof(Noti.CardShape));
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if(exDataMgr->isNomalUser(i))
		{
			m_Context->GetGameDesk()->send2(i,(char*)&Noti,sizeof(Noti),S_C_SEND_CARD);
		}
	}	
	return true;
}

bool SendCardPlayerListener::checkKingFine(uchar playerPos)
{

	return false;
}
