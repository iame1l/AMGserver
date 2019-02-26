//DetermineNtListener.cpp
//定庄监听器
#pragma once
#include "stdafx.h"
#include "DetermineNtListener.h"
#include "ServerManage.h"
#include "Algorithm.h"
#include <random>
#include "ExtensionTimer.h"
#include "ExtensionLogger.h"
using namespace std;

DetermineNtListener::DetermineNtListener( CServerGameDesk *desk, DataManage *dataMgr )
	: Listener(desk, dataMgr)
{
 	m_State = e_gs_robot_banker;
}

void DetermineNtListener::DelayNotiBanker(void **data, int dataCnt)
{
	DetermineNtListener *p = (DetermineNtListener *)data[0];
	if(nullptr == p) return;
	p->NoticeBanker();
}

void DetermineNtListener::DelayDeterminBankerOver(void **data, int dataCnt)
{
	DetermineNtListener *p = (DetermineNtListener *)data[0];
	if(nullptr == p) return;
	p->m_pDesk->OnListenEnd(LT_DetermineNt);
}

//通知定庄
void DetermineNtListener::OnAdd()
{
	if(nullptr == m_pDesk) return ;
	if(nullptr == m_pDataMgr) return ;

	__super::OnAdd();
	m_pDesk->SetGameStation(m_State);

	//赢家坐庄
	if (Config::s_GameMode.iRuleSelect & e_mode_winner_is_banker)
	{
		vector<uchar> winner =  m_pDataMgr->getWinner();
		if(winner.empty()) m_pDataMgr->setBanker(Algor::rand_Mersenne(0,m_pDataMgr->UserCount() - 1));
		else	m_pDataMgr->setBanker(winner.at(Algor::rand_Mersenne(0,winner.size() - 1)));
		NoticeBanker();
	}
	//固定庄家
	else if (Config::s_GameMode.iRuleSelect & e_mode_banker_is_const)
	{
		uchar master = m_pDataMgr->getMasterDeskNO();
		if(master != BYTE_ERROR) m_pDataMgr->setBanker(master);
		else m_pDataMgr->setBanker(0);
		NoticeBanker();
	}
	//轮流上庄
	else if (Config::s_GameMode.iRuleSelect & e_mode_turning_banker)
	{
		uchar banker = BYTE_ERROR;
		if(m_pDataMgr->isRoomFirstBegin())
		{
			uchar master = m_pDataMgr->getMasterDeskNO();
			if(master != BYTE_ERROR)banker = master;
			else	banker = 0;
		}
		else
		{
			uchar curBanker = m_pDataMgr->getBanker();
			DataManage::sPJUserInf curBankerInf;
			if(curBankerInf.bLnkBankerCnt >= Config::s_GameMode.iLnkBankerCnt)
				banker = m_pDataMgr->prevUser(curBanker);
			else
				banker = curBanker;
		}
		m_pDataMgr->setBanker(banker);
		m_pDataMgr->addLnkBanker(banker);
		NoticeBanker();
	}
	//骰子定庄
	else if(Config::s_GameMode.iRuleSelect & e_mode_dice_select_banker)
	{
		vector<uchar> winner =  m_pDataMgr->getWinner();
		if(winner.empty()) 
		{
			m_pDataMgr->setBanker(Algor::rand_Mersenne(0,m_pDataMgr->UserCount() - 1));
		}
		else
		{
			m_pDataMgr->starter = winner.at(Algor::rand_Mersenne(0,winner.size() - 1));
			m_pDataMgr->dice[0] = Algor::rand_Mersenne(1,6);
			m_pDataMgr->dice[1] = Algor::rand_Mersenne(1,6);
			uchar turns = m_pDataMgr->dice[0] + m_pDataMgr->dice[1];
			uchar banker = m_pDataMgr->starter;
			do 
			{
				banker = m_pDataMgr->nextActiveUser(banker);
				turns--;
			} while (turns);
			m_pDataMgr->setBanker(banker);

			NoticeDice();

			void *pointers[] = {this};
			OBJ_GET_EXT(m_pDesk->m_GameContext, ExtensionTimer, extTimer);
			extTimer->Add(2,TimerData(pointers,ARRAY_LEN(pointers),DelayNotiBanker));
		}
	}
	//抢庄
	else if(Config::s_GameMode.iRuleSelect & e_mode_robot_banker)
	{
		NoticeRob();
	}
}

//结束抢庄
void DetermineNtListener::OnRemove()
{
	__super::OnRemove();
}

//监听抢庄消息
void DetermineNtListener::Listen( uchar playerPos, uint msgID, void* msgData ,uint uSize)
{
	if(msgID == e_ass_c2s_robot_banker)
	{
		T_C2S_Rob_Banker_Req *data = (T_C2S_Rob_Banker_Req*)msgData;
		m_pDataMgr->setRobBankerState(playerPos,data->bRob?DataManage::E_ROB_BANKER_DO:DataManage::E_ROB_BANKER_PASS);

		T_S2C_Rob_Banker_Res res;
		res.bSeatNO = playerPos;
		res.bRob = data->bRob;
		m_pDesk->send2all((char*)&res,sizeof(res),e_ass_s2c_robot_banker_res);

		bool robBankerOver = true;
		vector<uchar> vRob;
		vector<uchar> vUnRob;
		for (size_t i = 0;i < m_pDataMgr->UserCount();i++)
		{
			DataManage::sPJUserInf user;
			if(!m_pDataMgr->getUserInfo(i,user))continue;
			if(user.eRobBankerState == DataManage::E_ROB_BANKER_NIL) robBankerOver = false;
			if(user.eRobBankerState == DataManage::E_ROB_BANKER_DO) vRob.push_back(i);
			else vUnRob.push_back(i);
		}

		if(robBankerOver)
		{
			uchar banker = BYTE_ERROR;
			if(!vRob.empty()) 
			{
				banker = vRob.at(Algor::rand_Mersenne(0,vRob.size() - 1));
			}
			else if(!vUnRob.empty()) 
			{
				banker = vUnRob.at(Algor::rand_Mersenne(0,vUnRob.size() - 1));	
			}
			else  
			{
				LOGGER_FILE(m_pDesk->m_GameContext,"用户信息错误");
			}

			m_pDataMgr->setBanker(banker);
			NoticeBanker();
		}
	}
}

void DetermineNtListener::Record( uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode )
{
}

void DetermineNtListener::OnTimer( uint timerID )
{
}

void DetermineNtListener::OnReconnect( uchar playerPos, uint socketID, bool isWatchUser )
{
	state_info_rob_banker data;
	fillBase(&data.baseinfo);
	data.iLastTime = Config::s_RobTime - (GetTickCount() - m_startTime)/1000;
	m_pDesk->sendState(playerPos,(char*)&data,sizeof(data),socketID,isWatchUser);

	NotiRound();

	if(Config::s_GameMode.iRuleSelect & e_mode_robot_banker)
	{
		NotiRobInfo();
	}
}

bool DetermineNtListener::AssureMsg( uchar playerPos, uint msgID, void* msgData )
{
	T_C2S_Rob_Banker_Req *data = (T_C2S_Rob_Banker_Req*)msgData;
 	if(NULL == data)
 		return false;

 	return true;
}


void DetermineNtListener::NoticeRob()
{
	m_pDesk->send2all(nullptr,0,e_ass_s2c_begin_rob_banker_noti);
}

void DetermineNtListener::NotiRobInfo()
{
	T_S2C_Rob_Banker_Info data;
	for (size_t i = 0;i < m_pDataMgr->UserCount();i++)
	{
		DataManage::sPJUserInf user;
		if(!m_pDataMgr->getUserInfo(i,user))continue;
		data.bRob[i] = user.eRobBankerState;
	}
	m_pDesk->send2all((char*)&data,sizeof(data),e_ass_s2c_begin_rob_banker_noti);
}

void DetermineNtListener::NoticeBanker()
{
	T_S2C_Banker Noti;
	Noti.bBanker = m_pDataMgr->getBanker();
	DataManage::sPJUserInf bankerInf;
	if(m_pDataMgr->getUserInfo(m_pDataMgr->getBanker(),bankerInf))
	{
		LOGGER_FILE(m_pDesk->m_GameContext,"通知 庄家 "<<m_pDataMgr->getBanker()<<"坐庄次数 "<<bankerInf.bLnkBankerCnt);
	}

	m_pDesk->send2all((char*)&Noti,sizeof(Noti),e_ass_s2c_banker_noti);

	void *pointers[] = {this};
	OBJ_GET_EXT(m_pDesk->m_GameContext, ExtensionTimer, extTimer);
	extTimer->Add(2,TimerData(pointers,ARRAY_LEN(pointers),DelayDeterminBankerOver));
}

void DetermineNtListener::NoticeDice()
{
	T_S2C_Dice Noti;
	Noti.bStart = m_pDataMgr->starter;
	Noti.bDice[0] = m_pDataMgr->dice[0];
	Noti.bDice[1] = m_pDataMgr->dice[1];
	m_pDesk->send2all((char*)&Noti,sizeof(Noti),e_ass_s2c_dice_noti);
}




DetermineNtListener::~DetermineNtListener()
{
}
