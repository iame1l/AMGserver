//CallListener.cpp
//游戏组牌阶段监听器
#include "stdafx.h"
#include "ServerManage.h"
#include "PlayCardListener.h"
#include "ExtensionLogger.h"
#include "ExtensionTimer.h"
#include "Algorithm.h"
#include "CardPickUp.h"
#include "GameDefine.h"
#include "PlayerActionDelegate.h"
#include "DataManage.h"
#include "Config.h"
#include "CardDefine.h"

//int pos[] = { 0,1,2,3 };

PlayCardListener::PlayCardListener()
{
	m_State = GS_NOTE_STATE;
}



void PlayCardListener::OnAdd()
{
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	__super::OnAdd();
	
	m_Context->GetGameDesk()->SetGameStation(GS_SEND_CARD_BEFORE);
	notiBeginPlay();
	void *pointers[] = {this};
	OBJ_GET_EXT(m_Context, ExtensionTimer, extTimer);

	extTimer->Add((float)1.0f, TimerData(pointers, ARRAY_LEN(pointers), NoticeXiaZhu));
	
	extTimer->Add((float)Config::iXiaZhuTime+2.f,TimerData(pointers,ARRAY_LEN(pointers),DelayPlayCardOver));

	extTimer->Add((float)2.0f,TimerData(pointers,ARRAY_LEN(pointers),DelayTiming));
}

void PlayCardListener::OnTimer( uint timerID )//
{

}

void PlayCardListener::OnReconnect(uchar playerPos, uint socketID, bool isWatchUser )
{
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	if(m_Context->GetGameDesk()->getGameStation() != GS_NOTE_STATE && m_Context->GetGameDesk()->getGameStation() != GS_SEND_CARD_BEFORE) return;
	if (m_Context->GetGameDesk()->getGameStation() == GS_NOTE_STATE)
	{
		GameStation_PlayGame TGameStation;
		TGameStation.iXiaZhuTime = Config::iXiaZhuTime;
		TGameStation.iKaiPaiTime = Config::iKaiPaiTime;
		TGameStation.iFreeTime = Config::iFreeTime;
		TGameStation.iGameBeen = exDataMgr->byGameBeen;
		memcpy(TGameStation.i64ChouMa, Config::chouma, sizeof(TGameStation.i64ChouMa));
		//TGameStation.byRunHeiCount = exDataMgr->byRunHeCount;
		TGameStation.byRunHeiCount = exDataMgr->byRunHeiCount;
		TGameStation.byRunHongCount = exDataMgr->byRunHongCount;
		TGameStation.iRemainTimeXZ = Config::iXiaZhuTime - exDataMgr->m_haveTimeSs;
		DataManage::sGameUserInf usr;
		if (!exDataMgr->getUserInfo(playerPos, usr))
		{
			LOGGER_ERROR_FILE(m_Context, "有玩家消失了强制结束游戏");
			return;
		}
		TGameStation.i64MyMoney = usr.iMoney - usr.iXiaZhuMoney;
		memcpy_s(TGameStation.byRunSeq, sizeof(TGameStation.byRunSeq), exDataMgr->byRunSeq, sizeof(TGameStation.byRunSeq));
		memcpy_s(TGameStation.byRunCardShape, sizeof(TGameStation.byRunCardShape), exDataMgr->byRunCardShape, sizeof(TGameStation.byRunCardShape));
		memcpy_s(TGameStation.i64QuYuZhu, sizeof(TGameStation.i64QuYuZhu), exDataMgr->i64QuYuZhu, sizeof(TGameStation.i64QuYuZhu));
		memcpy_s(TGameStation.i64UserXiaZhuData, sizeof(TGameStation.i64UserXiaZhuData), usr.i64UserXiaZhuData, sizeof(TGameStation.i64UserXiaZhuData));
		memset(TGameStation.rAresData, 0, sizeof(TGameStation.rAresData));
		memcpy_s(TGameStation.rAresData, sizeof(TGameStation.rAresData), exDataMgr->rAresData, sizeof(TGameStation.rAresData));
		memset(TGameStation.userMaxMoney, 255, sizeof(TGameStation.userMaxMoney));
		exDataMgr->updateDataMoney();
		exDataMgr->ProcessData(TGameStation.userMaxMoney);
		if (TGameStation.userMaxMoney[1] != 255)
		{
			if (!exDataMgr->getUserInfo(TGameStation.userMaxMoney[1], usr))
			{
				LOGGER_ERROR_FILE(m_Context, "有玩家消失了强制结束游戏");
				return;
			}
			if (usr.i64UserXiaZhuData[0] > 0)
			{
				TGameStation.bIsShenSuanBet[0] = true;
			}
			if (usr.i64UserXiaZhuData[1] > 0)
			{
				TGameStation.bIsShenSuanBet[1] = true;
			}
			if (usr.i64UserXiaZhuData[2] > 0)
			{
				TGameStation.bIsShenSuanBet[2] = true;
			}
		}

		if (m_Context != NULL)
		{
			m_Context->GetGameDesk()->sendState(playerPos, (char*)&TGameStation, sizeof(TGameStation), socketID, isWatchUser);
		}
	}
	else
	{
		GameStation TGameStation;
		TGameStation.iXiaZhuTime = Config::iXiaZhuTime;
		TGameStation.iKaiPaiTime = Config::iKaiPaiTime;
		TGameStation.iFreeTime = Config::iFreeTime;
		TGameStation.iGameBeen = exDataMgr->byGameBeen;
		memcpy(TGameStation.i64ChouMa, Config::chouma, sizeof(TGameStation.i64ChouMa));
		TGameStation.byRunHeiCount = exDataMgr->byRunHeiCount;
		TGameStation.byRunHongCount = exDataMgr->byRunHongCount;
		memcpy_s(TGameStation.byRunSeq, sizeof(TGameStation.byRunSeq), exDataMgr->byRunSeq, sizeof(TGameStation.byRunSeq));
		memcpy_s(TGameStation.byRunCardShape, sizeof(TGameStation.byRunCardShape), exDataMgr->byRunCardShape, sizeof(TGameStation.byRunCardShape));
		memset(TGameStation.userMaxMoney, 255, sizeof(TGameStation.userMaxMoney));
		exDataMgr->updateDataMoney();
		exDataMgr->ProcessData(TGameStation.userMaxMoney);

		DataManage::sGameUserInf usr;
		if (!exDataMgr->getUserInfo(playerPos, usr))
		{
			LOGGER_ERROR_FILE(m_Context, "有玩家消失了强制结束游戏");
			return;
		}
		TGameStation.i64MyMoney = usr.iMoney - usr.iXiaZhuMoney;



		if (m_Context != NULL)
		{
			m_Context->GetGameDesk()->sendState(playerPos, (char*)&TGameStation, sizeof(TGameStation), socketID, isWatchUser);
		}
	}
	
}

void PlayCardListener::Record( uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode )
{
}

void PlayCardListener::Listen( uchar playerPos, uint msgID, void* msgData ,uint uSize)
{
	if(m_Context->GetGameDesk()->getGameStation() != GS_NOTE_STATE) return;
	if(C_S_XIA_ZHU == msgID)
	{
		notiXiaZhu(playerPos,(C_S_UserNote*)msgData);
	}
}

PlayCardListener::~PlayCardListener()
{
}

void PlayCardListener::notiXiaZhu(uchar playerPos,C_S_UserNote *message)
{
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	DataManage::sGameUserInf user;
	if(!exDataMgr->getUserInfo(playerPos,user)) return ;
	if (message->iChouMaValues > 0 && message->byQuYu >0 && message->byQuYu <= BET_ARES && (user.iMoney - user.iXiaZhuMoney) > (message->iChouMaValues))
	{
		user.i64UserXiaZhuData[message->byQuYu-1] += message->iChouMaValues;
		user.iXiaZhuMoney += message->iChouMaValues;
		exDataMgr->i64QuYuZhu[message->byQuYu-1] += message->iChouMaValues;
		C_S_UserNoteResult Noti;
		if (message->iChouMaValues == Config::chouma[0])
		{
			exDataMgr->rAresData[message->byQuYu-1][0] += 1;
		}
		else if(message->iChouMaValues == Config::chouma[1])
		{
			exDataMgr->rAresData[message->byQuYu-1][1] += 1;
		}										   
		else if(message->iChouMaValues == Config::chouma[2])
		{										   
			exDataMgr->rAresData[message->byQuYu-1][2] += 1;
		}										   
		else if(message->iChouMaValues == Config::chouma[3])
		{										   
			exDataMgr->rAresData[message->byQuYu-1][3] += 1;
		}										  
		else if(message->iChouMaValues == Config::chouma[4])
		{										  
			exDataMgr->rAresData[message->byQuYu-1][4] += 1;
		}										   
		else if(message->iChouMaValues == Config::chouma[5])
		{										   
			exDataMgr->rAresData[message->byQuYu-1][5] += 1;
		}
		Noti.byDeskStation = playerPos;
		Noti.byQuYu = message->byQuYu;
		Noti.i64MyMoney = user.iMoney-user.iXiaZhuMoney;
		Noti.iChouMaValues = message->iChouMaValues;
		memset(Noti.i64UserMaxMoney, 0, sizeof(Noti.i64UserMaxMoney));
		BYTE tempmaxuser[USER_MAX_MONEY_NUM];
		memset(tempmaxuser, 255, sizeof(tempmaxuser));
		exDataMgr->updateUserMoney();
		exDataMgr->updateDataMoney();
		exDataMgr->ProcessData(tempmaxuser);
		DataManage::sGameUserInf tempuser;
		for (int i = 0; i < USER_MAX_MONEY_NUM; i++)
		{
			if (playerPos == tempmaxuser[i])
			{
				Noti.i64UserMaxMoney[i] = Noti.i64MyMoney;
				break;
			}
			else
			{
				if (!exDataMgr->getUserInfo(tempmaxuser[i], tempuser))continue;
				Noti.i64UserMaxMoney[i] = tempuser.iMoney;
			}
		}
		exDataMgr->alterUserInfo(playerPos, user);
		memcpy_s(Noti.i64QuYuZhu,sizeof(Noti.i64QuYuZhu),exDataMgr->i64QuYuZhu,sizeof(Noti.i64QuYuZhu));
	
		m_Context->GetGameDesk()->send2all((char*)&Noti,sizeof(Noti),S_C_XIA_ZHU_RESULT);
	}
}

void PlayCardListener::notiBeginPlay()
{
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	S_C_GameBegin Noti;
	Noti.iGameBeen=exDataMgr->byGameBeen;
	memcpy_s(Noti.byRunSeq,sizeof(Noti.byRunSeq),exDataMgr->byRunSeq,sizeof(Noti.byRunSeq));
	memcpy_s(Noti.byRunCardShape, sizeof(Noti.byRunCardShape), exDataMgr->byRunCardShape, sizeof(Noti.byRunCardShape));
	exDataMgr->updateDataMoney();
	memset(Noti.userMaxMoney,255,sizeof(Noti.userMaxMoney));
	exDataMgr->ProcessData(Noti.userMaxMoney);
	for (int i=0;i<PLAY_COUNT; ++i)
	{
		if(exDataMgr->isNomalUser(i))
		{
			DataManage::sGameUserInf usr;
			if(!exDataMgr->getUserInfo(i,usr))
			{
				continue;
			}
			Noti.i64UserMoney=usr.iMoney;
			m_Context->GetGameDesk()->send2(i,(char*)&Noti,sizeof(Noti), S_C_SEND_CARD_XIA_ZHU);
		}	
	}
	m_startTime = 0;
}

void PlayCardListener::notiCardScore()
{

}

bool PlayCardListener::checkPlayCardFinish(uchar playPos)
{
	
 	return false;
}

void PlayCardListener::DelayPlayCardOver(void **data, int dataCnt)
{
	PlayCardListener *p = (PlayCardListener *)data[0];
	if(nullptr == p) return ;
	if ( p->m_Context==NULL)
	{
		return;
	}
	if(nullptr == p->m_Context->GetGameDesk()) return;
	OBJ_GET_EXT(p->GetContext(),ExtensionListenerController,exListenerControl);
	exListenerControl->OnListenEnd(LT_PlayCard);
}

void PlayCardListener::DelayTiming(void **data, int dataCnt)
{
	PlayCardListener *p = (PlayCardListener *)data[0];
	if(nullptr == p) return ;
	if ( p->m_Context==NULL)
	{
		return;
	}
	if(nullptr == p->m_Context->GetGameDesk()) return;
	OBJ_GET_EXT(p->GetContext(),DataManage,exDataMgr);
	exDataMgr->m_haveTimeSs++;
	if (exDataMgr->m_haveTimeSs <= Config::iXiaZhuTime-1)
	{
		void *pointers[] = {p};
		OBJ_GET_EXT(p->GetContext(), ExtensionTimer, extTimer);
		extTimer->Add((float)1.0f,TimerData(pointers,ARRAY_LEN(pointers),DelayTiming));
	}
}

void PlayCardListener::NoticeXiaZhu(void **data, int dataCnt)
{
	PlayCardListener *p = (PlayCardListener *)data[0];
	if (nullptr == p) return;
	if (p->m_Context == NULL)
	{
		return;
	}
	if (nullptr == p->m_Context->GetGameDesk()) return;
	OBJ_GET_EXT(p->GetContext(), DataManage, exDataMgr);
	p->m_Context->GetGameDesk()->SetGameStation(GS_NOTE_STATE);
	for (int i = 0; i<PLAY_COUNT; ++i)
	{
		if (exDataMgr->isNomalUser(i))
		{
			DataManage::sGameUserInf usr;
			if (!exDataMgr->getUserInfo(i, usr))
			{
				continue;
			}
			p->m_Context->GetGameDesk()->send2(i, NULL, 0, S_C_NOTICE_XIA_ZHU);
		}
	}
}

void PlayCardListener::AutoPlayCard(GameContext *context, int type,uchar playerPos,void *data)//
{
	PlayCardListener *p = (PlayCardListener *)data;
	if (nullptr == p) return;
	OBJ_GET_EXT(p->GetContext(),DataManage,exDataMgr);
	
}

bool PlayCardListener::autoPickUpFirst(uchar bSeatNO,uchar *bCards,size_t& sLen)
{

	return true;
}

bool PlayCardListener::autoPickUpFollow(uchar bSeatNO,uchar *bCards,size_t& sLen)
{

	return false;
}