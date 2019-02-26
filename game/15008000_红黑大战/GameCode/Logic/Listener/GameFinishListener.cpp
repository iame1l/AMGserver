#include "stdafx.h"
#include "ServerManage.h"
#include "GameFinishListener.h"
#include "ExtensionLogger.h"
#include "ExtensionTimer.h"
#include "Algorithm.h"
#include "DataManage.h"

GameFinishListener::GameFinishListener()
{
	m_State = GS_SEND_RESULT;
}


void GameFinishListener::OnAdd()
{
	__super::OnAdd();

	void *pointers[] = {this};
	OBJ_GET_EXT(m_Context, ExtensionTimer, extTimer);
	m_Context->GetGameDesk()->SetGameStation(m_State);
	countScore();

	extTimer->Add(2.0f,TimerData(pointers,ARRAY_LEN(pointers),DelayResultOver));

	OBJ_GET_EXT(m_Context,ExtensionListenerController,exListenerController);
	
}

bool GameFinishListener::countScore()
{
 	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
 	DataManage::sGameUserInf userinf;
 	__int64 i_ChangePoint[PLAY_COUNT] = {0};
 	for(int i=0; i<PLAY_COUNT; i++)
 	{
 		if(!exDataMgr->getUserInfo(i,userinf))continue;
		/*__int64 tempscore = 0;
 		if (exDataMgr->m_bSpecialCardWin)
 		{
			tempscore = userinf.i64UserXiaZhuData[2]* exDataMgr->getbeilv(exDataMgr->WinUserShape)-userinf.i64UserXiaZhuData[2];
 			if(userinf.iScore > 0)
 			{
 				userinf.iWinCount+=1;
 				if(userinf.iWinCount > userinf.iRecordWinCount)
 				{
 					userinf.iRecordWinCount = userinf.iWinCount;
 				}
 			}
 			else
 			{
 				userinf.iWinCount = 0;
 			}				
 		}
*/
 		if(exDataMgr->byWinQuYu == 1)		//红
 		{
 			userinf.iScore = userinf.i64UserXiaZhuData[0]*2-userinf.i64UserXiaZhuData[1]-userinf.i64UserXiaZhuData[2]-userinf.i64UserXiaZhuData[0];
			if (exDataMgr->WinUserShape > UG_DAN_ZHANG)
			{
				userinf.iScore += userinf.i64UserXiaZhuData[2] * exDataMgr->getbeilv(exDataMgr->WinUserShape);
			}
			if (userinf.i64UserXiaZhuData[1] == 0)
 			{
 				if(userinf.i64UserXiaZhuData[0] > 0)
 				{
 					userinf.iWinCount+=1;
 					if(userinf.iWinCount > userinf.iRecordWinCount)
 					{
 						userinf.iRecordWinCount = userinf.iWinCount;
 					}
 				}
 				else
 				{
 					userinf.iWinCount = 0;
 				}				
 			}
 		}
 		else 
		if(exDataMgr->byWinQuYu == 2)//黑
 		{
 			userinf.iScore = userinf.i64UserXiaZhuData[1]*2-userinf.i64UserXiaZhuData[0]-userinf.i64UserXiaZhuData[2]-userinf.i64UserXiaZhuData[1];
			if (exDataMgr->WinUserShape > UG_DAN_ZHANG)
			{
				userinf.iScore += userinf.i64UserXiaZhuData[2] * exDataMgr->getbeilv(exDataMgr->WinUserShape);
			}
			if (userinf.i64UserXiaZhuData[0] == 0)		//区域0代表红
 			{
 				if(userinf.i64UserXiaZhuData[1] > 0)
 				{
 					userinf.iWinCount+=1;
 					if(userinf.iWinCount > userinf.iRecordWinCount)
 					{
 						userinf.iRecordWinCount = userinf.iWinCount;
 					}
 				}
 				else
 				{
 					userinf.iWinCount = 0;
 				}				
 			}
 		}

 		exDataMgr->alterUserInfo(i, userinf);
 	}
 	S_C_GameResult Noti;
 
 	for(int i=0; i<PLAY_COUNT; i++)
 	{
 		if(!exDataMgr->getUserInfo(i,userinf))continue;
 		i_ChangePoint[i] = userinf.iScore;
 		LOGGER_FILE(m_Context,"玩家"<<i<<","<<userinf.userID<<"结算"<<i_ChangePoint[i]<<\
 			", 分数"<<userinf.iScore<<" 总分数"<<userinf.iTotalScore); 
 	}
 	// 服务器写入数据库
 	bool temp_cut[PLAY_COUNT] = {0};
 	m_Context->GetGameDesk()->ChangeUserPointint64(i_ChangePoint, temp_cut);
 	exDataMgr->updateUserMoney();
 	exDataMgr->updateDataMoney();
 	exDataMgr->ProcessData(Noti.userMaxMoney);
 	exDataMgr->byGameBeen++;
 	if (exDataMgr->byGameBeen >= MAXCOUNT)
 	{
 		exDataMgr->byGameBeen = 0;
 		//exDataMgr->byRunHeCount = 0;
 		exDataMgr->byRunHongCount = 0;
 		exDataMgr->byRunHeiCount = 0;
 		memset(exDataMgr->byRunSeq,0,sizeof(exDataMgr->byRunSeq));
		memset(exDataMgr->byRunCardShape, 0, sizeof(exDataMgr->byRunCardShape));
 	}
 	for (int i=0;i < USER_MAX_MONEY_NUM;i++)
 	{
 		if(!exDataMgr->getUserInfo(Noti.userMaxMoney[i],userinf))continue;
 		Noti.i64MaxUserWin[i] = userinf.iScore;
 	}
 	for(int i=0; i<PLAY_COUNT; i++)
 	{
 		if(!exDataMgr->getUserInfo(i,userinf))continue;
 		Noti.byDeskStation = i;
 		Noti.i64UserMoney = userinf.iMoney;
 		Noti.i64UserWin = userinf.iScore;
 		m_Context->GetGameDesk()->send2(i,(char*)&Noti,sizeof(Noti),S_C_GAME_RESULT);
 	}
	exDataMgr->notiDataMoney();
	//奖池记录
	if (exDataMgr->m_bAIWinAndLostAutoCtrl)
	{
		exDataMgr->RecordAiWinMoney();
	}

	return true;
}

void GameFinishListener::OnReconnect( uchar playerPos, uint socketID, bool isWatchUser )
{
	if(m_Context->GetGameDesk()->getGameStation() != GS_SEND_RESULT) return;
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	GameStation_Result TGameStation;
	TGameStation.iXiaZhuTime=Config::iXiaZhuTime;
	TGameStation.iKaiPaiTime=Config::iKaiPaiTime;
	TGameStation.iFreeTime = Config::iFreeTime;
	TGameStation.iGameBeen = exDataMgr->byGameBeen;
	memcpy(TGameStation.i64ChouMa, Config::chouma, sizeof(TGameStation.i64ChouMa));
	TGameStation.byRunHongCount = exDataMgr->byRunHongCount;
	TGameStation.byRunHeiCount = exDataMgr->byRunHeiCount;
	TGameStation.byDeskStation = playerPos;
	memcpy(TGameStation.byShowCard, exDataMgr->UserCard, sizeof(TGameStation.byShowCard));
	DataManage::sGameUserInf usr;
	if(!exDataMgr->getUserInfo(playerPos,usr))
	{
		LOGGER_ERROR_FILE(m_Context,"有玩家消失了强制结束游戏");
		return ;
	}
	TGameStation.i64MyMoney=usr.iMoney-usr.iXiaZhuMoney;
	TGameStation.i64UserWin = usr.iScore;
	memcpy_s(TGameStation.byRunSeq,sizeof(TGameStation.byRunSeq),exDataMgr->byRunSeq,sizeof(TGameStation.byRunSeq));
	memcpy_s(TGameStation.byRunCardShape, sizeof(TGameStation.byRunCardShape), exDataMgr->byRunCardShape, sizeof(TGameStation.byRunCardShape));
	memcpy(TGameStation.CardShape, exDataMgr->UserShape, sizeof(TGameStation.CardShape));
	memset(TGameStation.userMaxMoney,255,sizeof(TGameStation.userMaxMoney));
	exDataMgr->updateUserMoney();
	exDataMgr->updateDataMoney();
	exDataMgr->ProcessData(TGameStation.userMaxMoney);
	for (int i=0;i < USER_MAX_MONEY_NUM;i++)
	{
		if(!exDataMgr->getUserInfo(TGameStation.userMaxMoney[i],usr))continue;
		TGameStation.i64MaxUserWin[i] = usr.iScore;
	}
	m_Context->GetGameDesk()->sendState(playerPos,(char*)&TGameStation,sizeof(TGameStation),socketID,isWatchUser);
}

void GameFinishListener::Record( uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode )
{
}


void GameFinishListener::DelayResultOver(void **data, int dataCnt)
{
	GameFinishListener *p = (GameFinishListener *)data[0];
	if(nullptr == p) return;
	if(nullptr == p->m_Context->GetGameDesk()) return ;
	OBJ_GET_EXT(p->GetContext(),ExtensionListenerController,exListenerControl);
	exListenerControl->OnListenEnd(LT_GameFinish);
	
}