//Listener.cpp
//各个阶段监听器的父类
#include "stdafx.h"
#include "ServerManage.h"
#include "Listener.h"
#include "ExtensionLogger.h"

Listener::Listener()
{
	m_Flag = false;
}


void Listener::Listen( uchar playerPos, uint msgID, void* msgData ,uint uSize)
{
}

void Listener::OnAdd()
{
	LOGGER_FILE(m_Context,"");
	m_Flag = true;
	m_startTime = GetTickCount();
}

void Listener::OnRemove()
{
	LOGGER_FILE(m_Context,"");
	m_Flag = false;
}

void Listener::OnTimer( uint timerID )
{
}

bool Listener::AssureMsg(uchar playerPos, uint msgID, void* msgData)
{
	return true;
}

void Listener::OnReconnect( uchar bDeskStation, uint uSocketID, bool bWatchUser )
{
}

void Listener::Record( uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode )
{
}

bool Listener::Flag()
{
	return m_Flag;
}

void Listener::sendBaseState(uchar playerPos, uint socketID, bool isWatchUser)
{
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	GameStation TGameStation;
	TGameStation.iXiaZhuTime=Config::iXiaZhuTime;
	TGameStation.iKaiPaiTime=Config::iKaiPaiTime;
	TGameStation.iFreeTime = Config::iKaiPaiTime;
	TGameStation.iGameBeen = exDataMgr->byGameBeen;
//	TGameStation.byRunHeCount = exDataMgr->byRunHeCount;
	TGameStation.byRunHeiCount = exDataMgr->byRunHeiCount;
	TGameStation.byRunHongCount = exDataMgr->byRunHongCount;
	memcpy_s(TGameStation.byRunSeq,sizeof(TGameStation.byRunSeq),exDataMgr->byRunSeq,sizeof(TGameStation.byRunSeq));
	memcpy_s(TGameStation.byRunCardShape, sizeof(TGameStation.byRunCardShape), exDataMgr->byRunCardShape, sizeof(TGameStation.byRunCardShape));
	memset(TGameStation.userMaxMoney,255,sizeof(TGameStation.userMaxMoney));
	memcpy(TGameStation.i64ChouMa, Config::chouma, sizeof(TGameStation.i64ChouMa));
	TGameStation.i64MyMoney = m_Context->GetGameDesk()->m_pUserInfo[playerPos]->m_UserData.i64Money;
	exDataMgr->updateDataMoney();
	exDataMgr->ProcessData(TGameStation.userMaxMoney);
	m_Context->GetGameDesk()->sendState(playerPos,(char*)&TGameStation,sizeof(TGameStation),socketID,isWatchUser);
}

int  Listener::getRunTime()
{
	return GetTickCount() - m_startTime;
}