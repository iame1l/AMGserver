/****************************************************************************
Copyright (c) 2014-2017 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2017 ShenZhen Redbird Network Polytron Technologies Inc.
 
http://www.hotniao.com

All of the content of the software, including code, pictures, 
resources, are original. For unauthorized users, the company 
reserves the right to pursue its legal liability.
****************************************************************************/
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             #include "StdAfx.h"
#include "ServerManage.h"
#include "GameContextCreator.h"
#include "ExtensionTimer.h"
#include "ExtensionLogger.h"
#include "PlayerActionDelegate.h"



//构造函数
CServerGameDesk::CServerGameDesk(void):CGameDesk(ALL_ARGEE),
	m_GameContext(GameContextCreator::Create()),
	m_TimerStarted(false),
	m_Dismissed(false),
	m_LatestUpdateTime(0)
{

	m_GameContext->SetGameDesk(this);
	m_GameStarted= false;
	OBJ_GET_EXT(m_GameContext,DataManage,exDataMgr);
	exDataMgr->SetContext(m_GameContext);
	exDataMgr->init(this);
	memset(m_SuperSetData, 0, sizeof(m_SuperSetData));
	OBJ_GET_EXT(m_GameContext,ExtensionListenerController,exListenerController);
	exListenerController->SetContext(m_GameContext);
	exListenerController->InitListener();
	exListenerController->Add(LT_WaitSet);
	
}

//析构函数
CServerGameDesk::~CServerGameDesk(void)
{
	GameContextCreator::Destroy(m_GameContext);
}

//拦截玩家强退消息
bool CServerGameDesk::HandleFrameMessage(uchar bDeskStation, NetMessageHead * pNetHead, void * pData, uint uSize, uint uSocketID, bool bWatchUser)
{
	if (bDeskStation>=PLAY_COUNT)
	{
		return true;
	}
	ASSERT(pNetHead->bMainID==MDM_GM_GAME_FRAME);

	switch (pNetHead->bAssistantID)
	{
	case ASS_GM_FORCE_QUIT:
		{
			bool  bForceQuitAsAuto = true ; 
			///查找上层路径
			CString szAppPath = CINIFile::GetAppPath();

			CString strBCFFile  ; 
			CString strKeyName  ; 
			strKeyName.Format("%d" ,NAME_ID) ; 
			strBCFFile.Format("%s\\SpecialRule.bcf",szAppPath) ; 

			CBcfFile File(strBCFFile) ;

			if (File.IsFileExist(strBCFFile))
			{
				bForceQuitAsAuto = File.GetKeyVal("ForceQuitAsAuto",strKeyName ,0)>0 ; 
			}
			if(bForceQuitAsAuto == true || (m_pDataManage->m_InitData.dwRoomRule & GRR_NOT_COST_POINT))
			{
				return true ; 
			}
		}
		break;
	default:
		break;
	}
	return __super::HandleFrameMessage(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
}

//游戏数据包处理函数
bool CServerGameDesk::HandleNotifyMessage(uchar bDeskStation, 
										  NetMessageHead * pNetHead, 
										  void * pData, 
										  uint uSize, 
										  uint uSocketID, 
										  bool bWatchUser)
{
	OBJ_GET_EXT(m_GameContext,DataManage,exDataMgr);

	DataManage::sGameUserInf userinf;

	if (pNetHead->bAssistantID ==  ASS_SUPER_USER_SET)
	{
		if( bWatchUser )
		{
			return true;
		}
		SuperUserSetData * pSuperSet= ( SuperUserSetData* )pData;
		if( NULL == pSuperSet ) 
		{
			return true;
		}
		if (pSuperSet->byDeskStation < 0 || pSuperSet->byDeskStation >= PLAY_COUNT)
		{
			return true;
		}
		if (IsSuperUser(bDeskStation))
		{
			m_SuperSetData[pSuperSet->byDeskStation].bSetSuccess = true;
			m_SuperSetData[pSuperSet->byDeskStation].byDeskStation = pSuperSet->byDeskStation;
			m_SuperSetData[pSuperSet->byDeskStation].byWinQuYu = pSuperSet->byWinQuYu;
			send2(bDeskStation,(char*)&m_SuperSetData[pSuperSet->byDeskStation],sizeof(SuperUserSetData),ASS_SUPER_USER_SET_RESULT);
		}
		return true;
	}

	if (MDM_GM_GAME_NOTIFY ==  pNetHead->bMainID)
	{
		OBJ_GET_EXT(m_GameContext,ExtensionListenerController,exListenerController);
		exListenerController->Listen(bDeskStation, pNetHead->bAssistantID, pData,uSize);
	}


	return true;
}

//定时器消息
bool CServerGameDesk::OnTimer(uint uTimerID)
{
	switch(uTimerID)
	{
	case TIME_UPDATE:
		{
			m_LatestUpdateTime += Config::s_TickInterval;
			OBJ_GET_EXT(m_GameContext, ExtensionTimer, extTimer);
			extTimer->Update((float)Config::s_TickInterval);
			break;
		}
	case ID_TIMER_GAMEBEGIN:
		{	//开始
			GameBegin(ALL_ARGEE);
			KillTimer(ID_TIMER_GAMEBEGIN);
		}
	case ID_TIMER_GAME_NEW_GAME:
		{
			GameBegin(ALL_ARGEE);
			KillTimer(ID_TIMER_GAME_NEW_GAME);
		}
	}

	return __super::OnTimer(uTimerID);
}

bool CServerGameDesk::IsSuperUser(BYTE byDeskStation)
{
	if (m_pUserInfo[byDeskStation] != NULL)
	{
		for(int i=0; i< Config::s_SuperID.size(); i++)
		{
			if (m_pUserInfo[byDeskStation]->m_UserData.dwUserID == Config::s_SuperID.at(i))
			{
				return true;
			}
		}
	}
	return false;
}

//验证是否超端
void CServerGameDesk::SpuerExamine(BYTE byDeskStation)
{
	//是超端用户 那么就发送超端消息过去
	if (IsSuperUser(byDeskStation))
	{
		S_C_SuperUser TSuperUser;
		TSuperUser.bIsSuper = true;
		TSuperUser.byDeskStation = byDeskStation;	
		
		send2(byDeskStation,(char*)&TSuperUser,sizeof(TSuperUser),S_C_IS_SUPER_USER);
	}
}

//获取游戏状态信息
bool CServerGameDesk::OnGetGameStation(uchar bDeskStation, uint uSocketID, bool bWatchUser)
{	
	if(Config::s_PrintLog)
	{
		InitLogFile();
	}

	OBJ_GET_EXT(m_GameContext,DataManage,exDataMgr);

	//memcpy_s(&exDataMgr->sRoomRule,sizeof(exDataMgr->sRoomRule),m_szDeskConfig,sizeof(exDataMgr->sRoomRule));

	OBJ_GET_EXT(m_GameContext,ExtensionListenerController,exListenerController);
	exListenerController->Reconnect(bDeskStation, uSocketID, bWatchUser);

	//超端验证
	SpuerExamine(bDeskStation);

	return true;
}

void CServerGameDesk::SetGameStation( BYTE station )
{
	m_bGameStation = station;
	LOGGER_FILE(m_GameContext,"GameStation  "<<(int) station);
}

//重置游戏状态
bool CServerGameDesk::ReSetGameState(uchar bLastStation)
{
	return TRUE;
}

/*---------------------------------------------------------------------------------*/
//游戏开始
bool	CServerGameDesk::GameBegin(uchar bBeginFlag)
{
	LOGGER_FILE(m_GameContext,"---------------GameBegin ("<<m_iRunGameCount<<"/"<<m_iVipGameCount<<")---------------");
	for(int i=0;i<PLAY_COUNT;i++)
	{
		if(NULL == m_pUserInfo[i])//玩家不存在就
		{
			continue;
		}
		m_pUserInfo[i]->m_UserData.bUserState = USER_ARGEE;
	}

	if (__super::GameBegin(bBeginFlag)==false)
	{
		GameFinish(0,GF_SAFE);
		return false;
	}

	OBJ_GET_EXT(m_GameContext,DataManage,exDataMgr);

	Config::Init();
	exDataMgr->GetAIContrlSetFromIni();

	GameInit();
	
	// 开始一局游戏
	m_GameContext->GameStart();

	m_Dismissed = false;

	OBJ_GET_EXT(m_GameContext,ExtensionListenerController,exListenerController);

	exListenerController->Remove(LT_WaitSet);
	exListenerController->Remove(LT_WaitNext);
	exListenerController->Add(LT_GameBegin);

	return TRUE;
}

bool CServerGameDesk::GameFinish(uchar bDeskStation, uchar bCloseFlag)
{
	LOGGER_FILE(m_GameContext,"GameFinish " << (int)bCloseFlag);
	switch (bCloseFlag)
	{
	case GF_NORMAL:		//游戏正常结束
		{
			GameClear();
			StartUpdateLoop(false);
			SetTimer(ID_TIMER_GAME_NEW_GAME, (Config::iFreeTime+3.5)*1000);
			break;
		}
	case GFF_SAFE_FINISH:
		{
			m_Dismissed = true;
			break;
		}
	case GFF_FORCE_FINISH:
		{
			return true;
		}
	case GF_SAFE:			//游戏安全结束
		{
			m_Dismissed = true;
			break;
		}
	case GFF_DISSMISS_FINISH:
		{
			m_Dismissed = true;

			OBJ_GET_EXT(m_GameContext,ExtensionListenerController,exListenerController);
			exListenerController->Add(LT_DismissRoom);
			break;
		}
	}
	if(m_Dismissed)
	{
		GameClear();
		GameReset();
	}

	//重置数据
	ReSetGameState(bCloseFlag);
	__super::GameFinish(bDeskStation,bCloseFlag);
	return true;
}

void CServerGameDesk::InitLogFile()
{
	OBJ_GET_EXT(m_GameContext, ExtensionLogger, extLogger);
	extLogger->SetEnable(Config::s_PrintLog);
	if(!extLogger->IsEnable()) return;

	CString strGame;
	strGame.Format("%d\\",NAME_ID);
	CString strPath = CINIFile::GetAppPath() +"GameLog\\"+strGame;
	SHCreateDirectoryEx(NULL, strPath, NULL);

	time_t nowTs = time(NULL);
	tm time_data;
	localtime_s(&time_data, &nowTs);
	char timeStrBuf[256];
	sprintf_s(timeStrBuf, "%d-%02d-%02d-%02d", 1900 + time_data.tm_year,
		time_data.tm_mon + 1,
		time_data.tm_mday,
		time_data.tm_hour);

	
	char buf[256];
	if(!isCreatedRoom())
		sprintf(buf, "%s/[%d-%d]_%s.log",strPath.GetBuffer(strPath.GetLength()),GetRoomID(), GetDeskID(), timeStrBuf);
	else
		sprintf(buf, "%s/[%d-%s]_%s.log",strPath.GetBuffer(strPath.GetLength()),GetRoomID(), m_szDeskPassWord, timeStrBuf);
	strPath.ReleaseBuffer(strPath.GetLength());
	extLogger->Open(buf);
}

//判断是否正在游戏
bool CServerGameDesk::IsPlayGame(uchar bDeskStation)
{
	OBJ_GET_EXT(m_GameContext,DataManage,exDataMgr);
	DataManage::sGameUserInf userinf;
	if (!exDataMgr->getUserInfo(bDeskStation, userinf))
	{
		return false;
	}
	if (userinf.iXiaZhuMoney >0 )
	{
		return true;
	}
	return false;
}

uchar CServerGameDesk::UserSitDesk(MSG_GR_S_UserSit * pUserSit, CGameUserInfo * pUserInfo)
{
	OBJ_GET_EXT(m_GameContext,DataManage,exDataMgr);
	uchar bFlag = __super::UserSitDesk(pUserSit,pUserInfo);
	exDataMgr->addUser(pUserSit->bDeskStation);

	OBJ_GET_EXT(m_GameContext,ExtensionPlayerActionDelegate,extActionDelegate);
	extActionDelegate->RemoveAction(pUserSit->bDeskStation,ASS_GM_AGREE_GAME);
	DataManage::sGameUserInf userinf;
	if (exDataMgr->getUserInfo(pUserSit->bDeskStation, userinf))
		LOGGER_FILE(m_GameContext,"DeskNo: "<<(int)pUserSit->bDeskStation<<" UserID: "<<userinf.userID<<" sit ");
	if (!m_GameStarted)
	{
		//GameBegin(ALL_ARGEE);
		m_GameStarted = true;
		SetTimer(ID_TIMER_GAMEBEGIN,3000);
	}
	return bFlag;
}

//用户离开游戏桌
uchar CServerGameDesk::UserLeftDesk(uchar bDeskStation, CGameUserInfo * pUserInfo)
{
	OBJ_GET_EXT(m_GameContext,DataManage,exDataMgr);
	DataManage::sGameUserInf userinf;
	if (exDataMgr->getUserInfo(bDeskStation, userinf))
		LOGGER_FILE(m_GameContext,"DeskNo: "<<(int)bDeskStation<<" UserID: "<<userinf.userID<<" leave ");
	OBJ_GET_EXT(m_GameContext,ExtensionPlayerActionDelegate,extActionDelegate);
	extActionDelegate->RemoveAction(bDeskStation,ASS_GM_AGREE_GAME);
	exDataMgr->eraseUser(bDeskStation);
	return __super::UserLeftDesk(bDeskStation,pUserInfo);
}
///用户断线离开
bool CServerGameDesk::UserNetCut(uchar bDeskStation, CGameUserInfo * pLostUserInfo)
{
	OBJ_GET_EXT(m_GameContext,DataManage,exDataMgr);
	DataManage::sGameUserInf userinf;
	if(exDataMgr->getUserInfo(bDeskStation,userinf))
		LOGGER_FILE(m_GameContext,"DeskNo: "<<(int)bDeskStation<<" UserID: "<<userinf.userID<<" netcut ");
	userinf.bOffline = true;
	exDataMgr->alterUserInfo(bDeskStation,userinf);

	bool flag =   __super::UserNetCut(bDeskStation,pLostUserInfo);

// 	size_t offlineCnt = 0;
// 	for (size_t i = 0;i < exDataMgr->UserCount();i++)
// 	{
// 		DataManage::sGameUserInf usr;
// 		if(exDataMgr->getUserInfo(i,usr))
// 		{
// 			if(usr.bOffline)offlineCnt++;
// 		}
// 	}

// 	if(offlineCnt == exDataMgr->UserCount())
// 	{
// 		OBJ_GET_EXT(m_GameContext,ExtensionListenerController,exListenerController);
// 		exListenerController->Add(LT_GameFinish);
// 	}

	return flag;
}

bool CServerGameDesk::UserReCome(uchar bDeskStation, CGameUserInfo * pNewUserInfo)
{
	OBJ_GET_EXT(m_GameContext,DataManage,exDataMgr);
	DataManage::sGameUserInf userinf;
	exDataMgr->getUserInfo(bDeskStation,userinf);
	userinf.bOffline = false;
	exDataMgr->alterUserInfo(bDeskStation,userinf);
	LOGGER_FILE(m_GameContext,"DeskNo: "<<(int)bDeskStation<<" UserID: "<<userinf.userID<<" recome ");
	return  __super::UserReCome(bDeskStation,pNewUserInfo);
}

//用来改变用户断线条件的函数
bool CServerGameDesk::CanNetCut(uchar bDeskStation)
{
	return true;
}
//初始化
bool CServerGameDesk::InitDeskGameStation()
{
	Config::Init();
	Config::Init(m_pDataManage->m_InitData.uRoomID);
	OBJ_GET_EXT(m_GameContext,DataManage,exDataMgr);
	exDataMgr->GetAIContrlSetFromIni();
	return true;
}


uchar  CServerGameDesk::getGameStation()
{
	return m_bGameStation;
}

void  CServerGameDesk::setGameStation(uchar bGameState)
{
	m_bGameStation = bGameState;
}


void  CServerGameDesk::ReleaseRoomBefore()
{
//	if (nullptr != _pLogic)
	{
		//_pLogic->saveDataBeforeDeskInterrupted();
		//_pLogic->reset();
	}
}

void CServerGameDesk::SetReturnGameInfo(int UserID,void* szUserGameInfo,int iSize,int iCount)
{
	//if (nullptr != _pLogic)
	{
		//_pLogic->recoverDataAfterDeskRebuild(UserID,szUserGameInfo,iSize,iCount);
	}
}

bool CServerGameDesk::isCreatedRoom()
{
	return m_bIsBuy;
}

bool CServerGameDesk::isCreatedRoomBegin()
{
	//if (nullptr != _pLogic)
	{
		//return _pLogic->createRoomBegin();
	}
	return false;
}

bool CServerGameDesk::isCreatedRoomEnd()
{
	if(!isCreatedRoom()) return false;
	return (m_iRunGameCount >= m_iVipGameCount);
}

int  CServerGameDesk::getRoomDeskFlag()
{
	if(isCreatedRoom())
		return atoi(m_szDeskPassWord);
	return m_bDeskIndex;
}


bool CServerGameDesk::send2all(char *pData,size_t size,uint assID)
{
	OBJ_GET_EXT(m_GameContext,DataManage,exDataMgr);
	for(int i=0; i<PLAY_COUNT; i++)
	{
		if(exDataMgr->isNomalUser(i))
		{
			if(nullptr == pData || 0 == size)
			{
				SendGameData(i,MDM_GM_GAME_NOTIFY,assID,0);
				SendWatchData(i,MDM_GM_GAME_NOTIFY,assID,0);
			}
			else
			{
				SendGameData(i,pData,size,MDM_GM_GAME_NOTIFY,assID,0);
				SendWatchData(i,pData,size,MDM_GM_GAME_NOTIFY,assID,0);
			}
		}
	}
	LOGGER_FILE(m_GameContext,"broad >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> message id:"<<assID<< " size:"<<size);
	return true;
}

bool CServerGameDesk::send2(uchar bSeatNO, char *pData,size_t size,uint assID)
{
	OBJ_GET_EXT(m_GameContext,DataManage,exDataMgr);
	if(exDataMgr->isNomalUser(bSeatNO))
	{
		SendGameData(bSeatNO,pData,size,MDM_GM_GAME_NOTIFY,assID,0);
		SendWatchData(bSeatNO,pData,size,MDM_GM_GAME_NOTIFY,assID,0);
		LOGGER_FILE(m_GameContext,"send >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> to"<<(int)bSeatNO<<" message id: "<<assID<< " size: "<<size);
		return true;
	}
	return false;
}

bool CServerGameDesk::sendState(uchar bSeatNO,char *pData,size_t size,uint uSocketID, bool bWatcher)
{
	LOGGER_FILE(m_GameContext,"sendstate >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> "<<(int)bSeatNO<<" state: "<<(int)getGameStation());
	SendGameStation(bSeatNO,uSocketID,bWatcher,pData,size);
	return true;
}

void CServerGameDesk::StartUpdateLoop(bool b)
{
	if(b)
	{
		SetTimer(TIME_UPDATE, (int)(Config::s_TickInterval));
	}
	else
	{
		KillTimer(TIME_UPDATE);
	}
}

void CServerGameDesk::GameInit()
{	
	StartUpdateLoop(true);
	m_LatestUpdateTime = 0;
}

void CServerGameDesk::GameClear()
{
	m_GameContext->GameFinish();

	OBJ_GET_EXT(m_GameContext, ExtensionTimer, extTimer);
	extTimer->Clear();

	OBJ_GET_EXT(m_GameContext,ExtensionListenerController,exListenerController);
	exListenerController->Clear();

	OBJ_GET_EXT(m_GameContext,ExtensionPlayerActionDelegate,extActionDelegate);
	extActionDelegate->RemoveAll();

	LOGGER_FILE(m_GameContext, "clean game"<<std::endl<<std::endl<<std::endl<<std::endl<<std::endl);

}

// 重置游戏
void CServerGameDesk::GameReset()
{
	OBJ_GET_EXT(m_GameContext,DataManage,exDataMgr);
	m_GameStarted = false;
	m_Dismissed = false;

	m_GameContext->Reset();

	Config::Init();
	Config::Init(m_pDataManage->m_InitData.uRoomID);

	exDataMgr->GetAIContrlSetFromIni();

	exDataMgr->init(this);

	OBJ_GET_EXT(m_GameContext,ExtensionListenerController,exListenerController);
	exListenerController->Add(LT_WaitSet);

	LOGGER_FILE(m_GameContext, "reset game");
}