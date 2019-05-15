#include "StdAfx.h"
#include "ServerManage.h"
#include <shlobj.h>
#include "GameContextCreator.h"
#include "ExtensionLogger.h"
int CServerGameDesk::m_iBaseMoney = 0;


/*------------------------------------------------------------------------------*/
/**
* 构造函数	DWJ
*/

CServerGameDesk::CServerGameDesk(void):CGameDesk(FULL_BEGIN),m_GameContext(GameContextCreator::Create())
{	
	m_uSocketID		= 0;
	m_bGameStation	= GS_WAIT_ARGEE;
	m_byAgreeLeaveNum = 0;//同意结束游戏的玩家数量
	m_RoomId = 0;

	sCPGAction.SetDeskPoint(this);	//吃碰杠检测方法(获取父类指针)
	sCheckHuPai.SetDeskPoint(this);	//糊牌检测方法(获取父类指针)
	m_sThingHandle.SetDeskPoint(this);//事件处理(获取父类指针)

	m_ZhaMa = 1;   
	m_ucDiFen = 1; 
}
/*------------------------------------------------------------------------------*/
/**
* 析构函数
*/
CServerGameDesk::~CServerGameDesk(void)
{
}
/*------------------------------------------------------------------------------*/
/**
* 定时器消息
* @param [in] uTimerID 定时器ID
* @return 返回布尔类型
*/
bool CServerGameDesk::OnTimer(UINT uTimerID)
{
	if (m_bGameStation >= GS_SEND_CARD && m_bGameStation < GS_WAIT_NEXT)
	{
		m_sThingHandle.OnGameTimer(uTimerID);
	}
	return __super::OnTimer(uTimerID);
}
/*------------------------------------------------------------------------------*/
/**
* 获取游戏状态信息
* @param [in] bDeskStation 玩家位置
* @param [in] uSoketID Socket ID
* @param [in] bWatchUser 旁观
* @return 返回布尔类型
*/
bool CServerGameDesk::OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser)
{

	InitLogFile();

	//保存断线重连时的socket
	m_uSocketID = uSocketID;
	//房间
	m_RoomId = m_pDataManage->m_InitData.uRoomID;
	
	
	m_sThingHandle.GetGameStaton(bDeskStation, uSocketID, bWatchUser, m_bGameStation);	

	if (IsBuyDesk())
	{
		SendRemaindedGameCount();
	}

	if(m_sThingHandle.IsSuperUser(bDeskStation))
	{
		SendGameDataEx(bDeskStation, NULL, 0, MDM_GM_GAME_NOTIFY, ASS_IM_SUPERMAN, 0);
	}

// 	if(GetRunGameCount() == 0 && (m_bGameStation == GS_WAIT_SETGAME || m_bGameStation == GS_WAIT_ARGEE || m_bGameStation == GS_WAIT_NEXT))
// 	{
// 		UserAgreeGame(bDeskStation);
// 	}

	return true;
}
/*-------------------------------------------------------------------------------------------------*/
/**
* 框架游戏数据包处理函数
* @param [in] bDeskStation 玩家位置
* @param [in] pNetHead 消息头
* @param [in] pData 消息体
* @param [in] uSize 消息包的大小
* @param [in] uSoketID Socket ID
* @param [in] bWatchUser 旁观
* @return 返回布尔类型
*/
bool CServerGameDesk::HandleFrameMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
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
			//不是强退托管 那么就是强退结束游戏
			if(sGameData.m_mjRule.bForceTuoGuan)
			{
				//如果已经有三个玩家被托管，则第四个玩家不再托管，直接退出游戏。
				int IsTuoGuan = 0;
				for(int i = 0;i<PLAY_COUNT;i++)
				{
					if(sUserData.m_bTuoGuan[i])
					{
						IsTuoGuan++;
					}
				}
				if(3==IsTuoGuan)
				{
					for(int i=0;i<PLAY_COUNT;i++)
					{
						GameFinish(i,GFF_FORCE_FINISH);
					}
					break;
				}

				sUserData.m_bTuoGuan[bDeskStation] = true;
				return true ; 
			}
			else
			{
				GameFinish(bDeskStation,GFF_FORCE_FINISH);//强退结束游戏
			}
			break;
		}
	default:
		{
			break;
		}
	}
	return __super::HandleFrameMessage(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
}
/*-------------------------------------------------------------------------------------------------*/
/**
* 游戏数据包处理函数
* @param [in] bDeskStation 玩家位置
* @param [in] pNetHead 消息头
* @param [in] pData 消息体
* @param [in] uSize 消息包的大小
* @param [in] uSoketID Socket ID
* @param [in] bWatchUser 旁观
* @return 返回布尔类型
*/
bool CServerGameDesk::HandleNotifyMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	KillTimer(TIME_CHECK_GAME_MSG);
	SetTimer(TIME_CHECK_GAME_MSG,TIME_DEFAULT_HANDLE);//重新设置超时计时器

	switch (pNetHead->bAssistantID)
	{
	case ASS_GM_AGREE_GAME:		//用户同意游戏
		{
			if (bWatchUser)
			{
				return FALSE;
			}
			if(NULL != m_pUserInfo[bDeskStation])
			{
				m_pUserInfo[bDeskStation]->m_UserData.bUserState = USER_ARGEE;  //玩家置为同意状态
			}
			return __super::HandleNotifyMessage(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
		}
	}
	if(pNetHead->bMainID == MDM_GM_GAME_NOTIFY)//接收游戏消息
	{
		m_sThingHandle.HandleNotifyMessage(bDeskStation,pNetHead->bAssistantID,(BYTE*)pData,uSize,bWatchUser);
	}
	if(pNetHead->bMainID == 180)
	{
		return true;
	}
	return __super::HandleNotifyMessage(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
}

/**
* 重置游戏状态
* @param [in] bLastStation 游戏结束标志
* @return 返回布尔类型
*/
bool	CServerGameDesk::ReSetGameState(BYTE bLastStation)
{
	memset(m_userTingPai, 255, sizeof(m_userTingPai));
	return TRUE;
}
/*-------------------------------------------------------------------------------------------------*/
/**
* 初始化游戏
* @return 返回布尔类型
*/
bool	CServerGameDesk::InitDeskGameStation()
{
	LoadIni();
	memset(sUserData.m_TCalculateBoard,0,sizeof(sUserData.m_TCalculateBoard));
	return true;
}
/*-------------------------------------------------------------------------------------------------*/
/**
* 游戏开始
* @param [in] bBeginFlasg 开始模式标志
*/
bool	CServerGameDesk::GameBegin(BYTE bBeginFlag)
{
	if (__super::GameBegin(bBeginFlag) == false) 
	{
		GameFinish(0,GF_SALE);
		return false;
	}
	
	LoadIni();

	if(IsBuyDesk() && 0 == GetRunGameCount())
	{
		memset(sUserData.m_TCalculateBoard,0,sizeof(sUserData.m_TCalculateBoard));
		
	}
	int iNowGameCount =0;

	if(IsBuyDesk() )
	{
		iNowGameCount = GetRunGameCount()+1;
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (m_pUserInfo[i]==nullptr) continue;
			SendGameData(i, &iNowGameCount, sizeof(int), MDM_GM_GAME_NOTIFY, S_C_UPDATE_REMAIN_JUSHU_SIG, 0);
		}
	}
	

	
	
	m_bGameStation = GS_SEND_CARD;
	sGameData.ApplyThingID = THING_GAME_BEGIN;

	//请求事件
	m_sThingHandle.ApplyAction(THING_GAME_BEGIN,200);
	//char temp[MAX_PATH]={0};
	
	return true;
}
void CServerGameDesk::SetParamaterStation(BYTE byStation)
{
	m_bGameStation = byStation;
}
/*-------------------------------------------------------------------------------------------------*/
/**
* 游戏结束
* @param [in] bDeskStation 玩家位置
* @param [in] bCloseFlag 游戏结束标志
* @return 返回布尔类型
*/
bool CServerGameDesk::GameFinish(BYTE bDeskStation, BYTE bCloseFlag)
{
	/*char str[800];
	sprintf(str,"HZMJEX8:------666666666666--GFF_DISSMISS_FINISH--------%d",bCloseFlag);
	OutputDebugString(str);*/

	switch (bCloseFlag)
	{
	case GFF_DISSMISS_FINISH:
	{
		DissMissRoomBefore();
		m_bGameStation = GS_WAIT_ARGEE;
		KillTimer(TIME_WAIT_MSG);
		KillTimer(10);//
		KillTimer(TIME_CHECK_GAME_MSG);

		for (int i=0; i<m_bMaxPeople; i++)
		{
			if (m_pUserInfo[i] != NULL) 
				m_pUserInfo[i]->m_UserData.bUserState = USER_LOOK_STATE;
		}				
		bCloseFlag	= GFF_DISSMISS_FINISH;
		ReSetGameState(bCloseFlag);
		LOGGER_FILE(m_GameContext,"游戏解散结束");
		__super::GameFinish(bDeskStation,bCloseFlag);
		return true;
	}
	case GF_NORMAL:		//游戏正常结束
		{
			m_bGameStation = GS_WAIT_NEXT;	
			if (!IsBuyDesk())   m_bGameStation = GS_WAIT_ARGEE;

			//计算胜负分数
			for (int i = 0;i < PLAY_COUNT; i ++)
			{
				if (m_pUserInfo[i] != NULL) 
				{
					m_pUserInfo[i]->m_UserData.bUserState = USER_SITTING;
				}
			}

			__int64   i_ChangePoint[PLAY_COUNT];     //积分
			__int64   i_ChangeMoney[PLAY_COUNT];     //金币
			__int64   i_ReturnMoney[PLAY_COUNT];     //返回金币			

			memset(i_ChangePoint, 0, sizeof(i_ChangePoint));
			memset(i_ChangeMoney, 0, sizeof(i_ChangeMoney));
			memset(i_ReturnMoney, 0, sizeof(i_ReturnMoney));

			// 服务器写入数据库
			bool temp_cut[PLAY_COUNT];
			memset(temp_cut, 0, sizeof(temp_cut));
	
			if(m_bIsBuy && m_ucDiFen >=1 && m_ucDiFen <= 100)
			{
				for(int i=0;i<PLAY_COUNT;++i)
				{
					i_ChangePoint[i] = sGameData.T_CountFen.iZongFen[i] * m_ucDiFen;
					sUserData.m_TCalculateBoard[i].i64WinMoney += i_ChangePoint[i];
					sGameData.T_CountFen.iZongFen[i] *= m_ucDiFen;
				}
			}
			else
			{
				for(int i=0;i<PLAY_COUNT;++i)
				{
					i_ChangePoint[i] = sGameData.T_CountFen.iZongFen[i];
				}
			}

			// 扣所有赢家税
			ChangeUserPointint64(i_ChangePoint, temp_cut);
			//__super::RecoderGameInfo(i_ChangeMoney);

			//20190513 有效投注修改m_iBaseMoney
			__super::RecoderGameInfo_Effectivebet(i_ChangeMoney, m_iBaseMoney);

			//扣税后获得的金币
			for(int i=0;i<PLAY_COUNT;++i)
			{
				sGameData.T_CountFen.i64Money[i] = i_ChangeMoney[i];
			}

			sGameData.T_CountFen.bIsShowTax = sGameData.m_bShowTax;//是否显示服务费
			sGameData.T_CountFen.iBasePoint = m_pDataManage->m_InitData.uBasePoint;//倍数
			if (sGameData.T_CountFen.bQiangGang)
			{
				for(int i=0; i<PLAY_COUNT; i++)
				{
					for (int j=0; j<HAND_CARD_NUM; j++)
					{
						LOGGER_FILE(m_GameContext,"抢杠胡：玩家"<<i<<"的手牌"<<j<<"="<<sGameData.T_CountFen.m_byArHandPai[i][j]);
					}
				}
				LOGGER_FILE(m_GameContext,"------------------");
			}
			

			KillTimer(TIME_WAIT_MSG);
			KillTimer(10);//
			KillTimer(TIME_CHECK_GAME_MSG);
				
			sGameData.T_CountFen.byCloseFlag = bCloseFlag;

			
			//发送数据
			for (int i=0; i<PLAY_COUNT; i++)     
			{
				SendGameData(i, &sGameData.T_CountFen, sizeof(sGameData.T_CountFen), MDM_GM_GAME_NOTIFY, THING_ENG_HANDLE, 0);   
				SendWatchDataEx(i, &sGameData.T_CountFen, sizeof(sGameData.T_CountFen), MDM_GM_GAME_NOTIFY, THING_ENG_HANDLE, 0); 
			}

			if (IsBuyDesk() && GetSurplusCount() <= 0)
			{
				LOGGER_FILE(m_GameContext,"游戏正常结束，发送大结算");
				UpdateCalculateBoard();
			}

			bCloseFlag = GF_NORMAL;
			ReSetGameState(bCloseFlag);
			

			//清除有关算分的信息
			sGameData.T_HuPai.Clear();
			sGameData.T_CountFen.Clear();
			for (int i=0; i< PLAY_COUNT; i++)
			{
				for (int j=0; j<5; j++)
				{
					sUserData.m_UserGCPData[i][j].Init();
				}
			}
			LOGGER_FILE(m_GameContext,"第"<<GetRunGameCount()<<"局正常结束");
			__super::GameFinish(bDeskStation,bCloseFlag);
			
			if (IsBuyDesk())
			{
				SendRemaindedGameCount();
			}

			return true;
		}
	case GFF_SAFE_FINISH:
		{
			if(bDeskStation==255)
			{
				bDeskStation=0;
			}
			KillTimer(TIME_WAIT_MSG);
			KillTimer(10);//
			KillTimer(TIME_CHECK_GAME_MSG);

			int iBasePoint = m_pDataManage->m_InitData.uBasePoint;//倍数
			for (int i=0; i<m_bMaxPeople; i++)
			{
				if (m_pUserInfo[i] != NULL) 
					m_pUserInfo[i]->m_UserData.bUserState = USER_SITTING;
			}				
			bCloseFlag	= GFF_FORCE_FINISH;

			//写入数据库				
			__int64 i_ChangePoint[PLAY_COUNT];
			__int64 iWinMoney[8] ={0};
			bool temp_cut[PLAY_COUNT];
			memset(i_ChangePoint,0,sizeof(i_ChangePoint));
			memset(temp_cut,0,sizeof(temp_cut));
	
			ChangeUserPointint64(i_ChangePoint, temp_cut);
			__super::RecoderGameInfo(iWinMoney);
			for (int i=0;i<PLAY_COUNT;i++)
			{
				sGameData.T_CountFen.i64Money[i] = iWinMoney[i]; 
			}
			sGameData.T_CountFen.bIsShowTax = sGameData.m_bShowTax;//是否显示服务费
			
			
			sUserData.CopyHandPai(sGameData.T_CountFen.m_byArHandPai,0,true);
			
			
			
			
			sGameData.T_CountFen.byCloseFlag = bCloseFlag;
			///发送数据
			for (int i=0; i<PLAY_COUNT; i++)     
			{
				SendGameData(i, &sGameData.T_CountFen, sizeof(sGameData.T_CountFen), MDM_GM_GAME_NOTIFY, THING_ENG_UN_NORMAL, 0);   
				SendWatchDataEx(i, &sGameData.T_CountFen, sizeof(sGameData.T_CountFen), MDM_GM_GAME_NOTIFY, THING_ENG_UN_NORMAL, 0);
			}
			//DebugPrintf("游戏安全结束");
			ReSetGameState(bCloseFlag);
			__super::GameFinish(bDeskStation,bCloseFlag);
			return true;
		}
	case GFF_FORCE_FINISH:
		{
			m_bGameStation = GS_WAIT_ARGEE;//GS_WAIT_SETGAME;
			for (int i = 0; i < PLAY_COUNT; i ++)
			{
				if (m_pUserInfo[i] != NULL) 
					m_pUserInfo[i]->m_UserData.bUserState = USER_SITTING;
			}

			KillTimer(TIME_WAIT_MSG);
			KillTimer(10);//
			KillTimer(TIME_CHECK_GAME_MSG);
			bCloseFlag	= GFF_FORCE_FINISH;
			//int iBasePoint = m_pDataManage->m_InitData.uBasePoint;//倍数

			//写入数据库				
			__int64 i_ChangePoint[PLAY_COUNT]={0};
			__int64 iWinMoney[8] ={0};
			bool temp_cut[PLAY_COUNT] = {0};
			//乘以基础分
			i_ChangePoint[bDeskStation] = -(__int64)m_pDataManage->m_InitData.uRunPublish * m_ucDiFen;
			temp_cut[bDeskStation] = true;

			if (!m_bIsBuy)
			{
				ChangeUserPointint64(i_ChangePoint, temp_cut);
				__super::RecoderGameInfo(iWinMoney);
			}
			
			for (int i=0;i<PLAY_COUNT;i++)
			{
				sGameData.T_CountFen.i64Money[i] = iWinMoney[i]; 
			}
			sGameData.T_CountFen.bIsShowTax = sGameData.m_bShowTax;//是否显示服务费

			///拷贝手牌数据
			sUserData.CopyHandPai(sGameData.T_CountFen.m_byArHandPai,0,true);
			sGameData.T_CountFen.byCloseFlag = bCloseFlag;

			///发送数据
			for (int i=0; i<PLAY_COUNT; i++)     
			{
				SendGameData(i, &sGameData.T_CountFen, sizeof(sGameData.T_CountFen), MDM_GM_GAME_NOTIFY, THING_ENG_UN_NORMAL, 0);   
				SendWatchDataEx(i, &sGameData.T_CountFen, sizeof(sGameData.T_CountFen), MDM_GM_GAME_NOTIFY, THING_ENG_UN_NORMAL, 0);
			}
			ReSetGameState(bCloseFlag);
			//DebugPrintf("游戏强制结束");
			__super::GameFinish(bDeskStation,bCloseFlag);
			return true;
		}
	default:
		{
			break;
		}
	}
	// 重置数据
	ReSetGameState(bCloseFlag);
	return __super::GameFinish(bDeskStation,bCloseFlag);
}
/*-----------------------------------------------------------------------------------*/
/**
* 判断是否正在游戏
* @param [in] bDeskStation 玩家位置
* @return 返回当前游戏状态
*/
bool	CServerGameDesk::IsPlayGame(BYTE bDeskStation)
{
	if(m_bGameStation >= GS_SEND_CARD && m_bGameStation < GS_WAIT_NEXT)
	{
		return true;
	}
	return false;
}
/*-----------------------------------------------------------------------------------*/
/**
* 用户离开游戏桌
* @return 返回用户离开状态
*/
BYTE	CServerGameDesk::UserLeftDesk(BYTE bDeskStation, CGameUserInfo * pUserInfo)
{
	m_sThingHandle.UserLeftDesk(bDeskStation);
	memset(sUserData.m_TCalculateBoard,0,sizeof(sUserData.m_TCalculateBoard));
	return __super::UserLeftDesk(bDeskStation,pUserInfo);
}
/*-----------------------------------------------------------------------------------*/
//用来改变用户断线条件的函数
bool	CServerGameDesk::CanNetCut(BYTE bDeskStation)
{
	return true;
}
/*-----------------------------------------------------------------------------------*/
//用户断线离开
bool	CServerGameDesk::UserNetCut(BYTE bDeskStation, CGameUserInfo * pLostUserInfo)
{
	//在游戏中，有断线托管就进行托管
	if (IsPlayGame(bDeskStation))  
	{

		//sUserData.m_bTuoGuan[bDeskStation] = true;
		//m_sThingHandle.UserNetCut(bDeskStation);

		//sUserData.m_byFoceLeavCount = 0;
		//for(int i=0;i<4;++i)
		//{
		//	if(sUserData.m_bFoceLeave[i])//强退
		//	{
		//		sUserData.m_byFoceLeavCount++;
		//	}
		//}
  //      if (sUserData.m_byFoceLeavCount >= PLAY_COUNT && sUserData.m_byFoceLeavCount != 255) //四家都强行离开了
  //      {
  //          GameFinish(0,GF_NORMAL); //用户强行离开
  //          return true;
  //      }
	}
	//记录断线玩家（在重连前部给该玩家发送消息）
	return __super::UserNetCut(bDeskStation, pLostUserInfo);
}

bool CServerGameDesk::UserReCome(BYTE bDeskStation, CGameUserInfo * pNewUserInfo)
{
	LOGGER_FILE(m_GameContext,"玩家 "<<bDeskStation<<","<<pNewUserInfo->m_UserData.nickName<<"的托管状态="<<sUserData.m_bTuoGuan[bDeskStation]);
	return __super::UserReCome(bDeskStation, pNewUserInfo);
}
/*-----------------------------------------------------------------------------------*/
/**
* 玩家坐下
* @return 返回用户坐下状态
*/
BYTE	CServerGameDesk::UserSitDesk(MSG_GR_S_UserSit * pUserSit, CGameUserInfo * pUserInfo)
{
	BYTE result = __super::UserSitDesk(pUserSit, pUserInfo);
	return result;
}
/*-----------------------------------------------------------------------------------*/
///设置游戏定时器
void CServerGameDesk::SetGameTimer(UINT iId,int iTime)
{
	SetTimer(iId,iTime);
}
/*-----------------------------------------------------------------------------------*/
//删除指定计时器
void CServerGameDesk::KillGameTimer(UINT iId)
{
	KillTimer(iId);
}
/*-----------------------------------------------------------------------------------*/
///发送游戏状态
void	CServerGameDesk::SendGameStationEx(BYTE nDeskStation, bool bWatchUser, void * pStationData, UINT uSize)
{
	// 发送数据
	SendGameStation(nDeskStation,m_uSocketID,bWatchUser,pStationData,uSize);
}
///发送数据函数 （发送消息给游戏者）
void	CServerGameDesk::SendGameDataEx(BYTE nDeskStation, BYTE bMainID, BYTE bAssID, BYTE bHandleCode)
{
	SendGameData(nDeskStation,MDM_GM_GAME_NOTIFY,bAssID,0) ; 
}
///发送旁观数据 （发送消息给旁观者）
void	CServerGameDesk::SendWatchDataEx(BYTE nDeskStation, BYTE bMainID, BYTE bAssID, BYTE bHandleCode)
{
	SendWatchData(nDeskStation,MDM_GM_GAME_NOTIFY,bAssID,0) ; 
}
///发送数据函数 （发送消息给游戏者）
void	CServerGameDesk::SendGameDataEx(BYTE nDeskStation, void * pData, UINT uSize, BYTE bMainID, BYTE bAssID, BYTE bHandleCode)
{
	SendGameData(nDeskStation, pData, uSize, MDM_GM_GAME_NOTIFY, bAssID, 0);
}
///发送旁观数据 （发送消息给旁观者）
void	CServerGameDesk::SendWatchDataEx(BYTE nDeskStation, void * pData, UINT uSize, BYTE bMainID, BYTE bAssID, BYTE bHandleCode)
{
	SendWatchData(nDeskStation, pData, uSize, MDM_GM_GAME_NOTIFY, bAssID, 0);
}
/*-----------------------------------------------------------------------------------*/
//加载房间设置//mark
void	CServerGameDesk::LoadiniByRoom(int iRoomId)
{
	char key[MAX_PATH];
	CString s = CINIFile::GetAppPath ();/////本地路径    
	CINIFile f(s +_T("\\")+SKIN_FOLDER  + _T("_s.ini"));
	sprintf(key,"%d_%d",NAME_ID,iRoomId);

	sGameData.m_mjRule.byAutoOutTime	= f.GetKeyVal(key,TEXT("byAutoOutTime"),sGameData.m_mjRule.byAutoOutTime);	//自动出牌时间	DWJ
	sGameData.m_mjRule.byOutTime		= f.GetKeyVal(key,TEXT("byOutTime"),sGameData.m_mjRule.byOutTime);			//出牌时间		DWJ
	sGameData.m_mjRule.byBlockTime		= f.GetKeyVal(key,TEXT("byBlockTime"),sGameData.m_mjRule.byBlockTime);		//拦牌思考时间	DWJ
	sGameData.m_mjRule.byBeginTime		= f.GetKeyVal(key,TEXT("byBeginTime"),sGameData.m_mjRule.byBeginTime);		//开始等待时间	DWJ
	sGameData.m_mjRule.bAutoBegin		= f.GetKeyVal(key,TEXT("bAutoBegin"),sGameData.m_mjRule.bAutoBegin);		//时间到了是否自动开始
	sGameData.m_mjRule.byZhongTime = f.GetKeyVal(key,TEXT("byBeginTime"),(int)sGameData.m_mjRule.byZhongTime);
	sGameData.m_mjRule.byTuoGuanTime  = f.GetKeyVal(key,TEXT("byTuoGuanTime"), sGameData.m_mjRule.byTuoGuanTime);		//托管时间
	sGameData.m_mjRule.bNetCutTuoGuan	= f.GetKeyVal(key,TEXT("bNetCutTuoGuan"),sGameData.m_mjRule.bNetCutTuoGuan);	//是否断线托管
	sGameData.m_mjRule.byGamePassNum	= f.GetKeyVal(key,TEXT("GamePassNum"),sGameData.m_mjRule.byGamePassNum);;		//流局牌数
	sGameData.m_mjRule.byZamaNum = f.GetKeyVal(key,TEXT("ZamaNum"),sGameData.m_mjRule.byZamaNum);

	sGameData.m_mjRule.bPeiPai = f.GetKeyVal(key,TEXT("PeiPai"), sGameData.m_mjRule.bPeiPai);				//是否配牌

	if (sGameData.m_mjRule.bPeiPai)
	{
		CString sText;
		for(int i=0; i<PLAY_COUNT; i++)
		{
			sText.Format("PeiPai%d",i);
			sGameData.m_bUserPeiPai[i] = f.GetKeyVal(key, sText, sGameData.m_bUserPeiPai[i]);
		}

		BYTE card = 1;
		int index = 0;
		int num = 0;

		for(int i=0; i<PLAY_COUNT; i++)
		{
			num = (i==0 ? 14:13);

			for (int j=0; j<num; j++)
			{
				sText.Format("Card%d",index);
				if (card%10 == 0)
				{
					card += 1;
				}

				sGameData.byAllPeiPai[index] = sGameData.byPeiPai[i][j] = (sGameData.m_bUserPeiPai[i] ? f.GetKeyVal(key, sText,card) : 255);

				card++;
				index++;
			}
		}
	}
	//sGameData.m_mjRule.byGamePassNum = sGameData.m_mjRule.byZamaNum;
	//sprintf(key,"game");
	//m_iBaseMoney = f.GetKeyVal(key,TEXT("BaseMoney"),10); //底分设置
	//
	//sprintf(key,"SuperSet");
	//int iCount = f.GetKeyVal(key,TEXT("SuperCount"),0);
	//long int iSuperId = 0;
	//CString sText;

	//for(int i=0; i<iCount; i++)
	//{
	//	sText.Format("SuperID_%d",i);
	//	iSuperId = f.GetKeyVal(key,sText,0);
	//	m_vlSuperUserID.push_back(iSuperId);
	//}

	///查找上层路径
	bool  bForceQuitAsAuto = true ; 
	CString szAppPath = CINIFile::GetAppPath();

	CString strBCFFile  ; 
	CString strKeyName  ; 
	strKeyName.Format("%d" ,NAME_ID) ; 
	strBCFFile.Format("%s\\SpecialRule.bcf",szAppPath) ; 

	CBcfFile File(strBCFFile) ;

	if (File.IsFileExist(strBCFFile))
	{
		bForceQuitAsAuto = File.GetKeyVal("ForceQuitAsAuto",strKeyName ,1) ; 
	}
	sGameData.m_mjRule.bForceTuoGuan	= bForceQuitAsAuto;		//是否强退托管
}
/*-----------------------------------------------------------------------------*/
//---------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//修改奖池数据1(平台->游戏)
/// @param bAIWinAndLostAutoCtrl 机器人控制输赢开关
/// @return 是否成功
bool CServerGameDesk::SetRoomPond(bool	bAIWinAndLostAutoCtrl)
{
	return false;
}

//修改奖池数据2(平台->游戏)
/// @param m_iAIWantWinMoney 机器人输赢控制：机器人赢钱区域1,2,3 4所用断点
/// @param m_iAIWinLuckyAt 机器人输赢控制：机器人在区域1,2,3,4赢钱的概率
/// @return 是否成功
bool CServerGameDesk::SetRoomPondEx(__int64	iAIWantWinMoney[], int	iAIWinLuckyAt[])
{
	return false;
}

//修改奖池数据3(平台->游戏)
/// @param iReSetAIHaveWinMoney 机器人输赢控制：重置机器人已经赢钱的数目
/// @return 是否成功
bool CServerGameDesk::SetRoomPondEx(__int64	iReSetAIHaveWinMoney)
{
	return false;
}

//判断
BOOL CServerGameDesk::Judge()
{
	if(JudgeWiner())		//先处理胜者
	{
		return TRUE;
	}

	if (JudgeLoser())		//处理输者
	{
		return TRUE;
	}		

	return FALSE;
}
//================================================
//判断胜者
//================================================
BOOL CServerGameDesk::JudgeLoser()
{
	return FALSE;
}
//================================================
//判断胜者
//================================================
BOOL CServerGameDesk::JudgeWiner()
{
	return FALSE;
}
//GRM函数
//更新奖池数据(游戏->平台)
/// @return 机器人赢钱数
void CServerGameDesk::UpDataRoomPond(__int64 iAIHaveWinMoney)
{

}

void CServerGameDesk::ReleaseRoom(int UserID,void* szUserGameInfo,int iSize)
{
	_putRoomInfo();	
}

void CServerGameDesk::SetReturnGameInfo(int UserID,void* szUserGameInfo,int iSize,int iCount)
{
	if (nullptr == szUserGameInfo)
	{
		return;
	}

	GameInfForReplay *pGameInf =  (GameInfForReplay*)szUserGameInfo;

	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (m_pReturnInfo[i].iUserID == UserID)
		{
			sUserData.m_TCalculateBoard[i].iZimo = pGameInf->iZimo;
			sUserData.m_TCalculateBoard[i].iJiePao = pGameInf->iJiePao;
			sUserData.m_TCalculateBoard[i].iDianPao = pGameInf->iDianPao;
			sUserData.m_TCalculateBoard[i].iAnGan = pGameInf->iAnGan;
			sUserData.m_TCalculateBoard[i].iMingGan = pGameInf->iMingGan;
			sUserData.m_TCalculateBoard[i].iZhongNiaoNum = pGameInf->iZhongNiaoNum;
			sUserData.m_TCalculateBoard[i].i64WinMoney = pGameInf->i64WinMoney;
		}	
	}
}

void CServerGameDesk::_putRoomInfo()
{
	GameInfForReplay inf[PLAY_COUNT];
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (!m_pUserInfo[i])
		{
			continue;
		}
		inf[i].iZimo = sUserData.m_TCalculateBoard[i].iZimo;
		inf[i].iJiePao = sUserData.m_TCalculateBoard[i].iJiePao;
		inf[i].iDianPao = sUserData.m_TCalculateBoard[i].iDianPao;
		inf[i].iAnGan = sUserData.m_TCalculateBoard[i].iAnGan;
		inf[i].iMingGan = sUserData.m_TCalculateBoard[i].iMingGan;
		inf[i].iZhongNiaoNum = sUserData.m_TCalculateBoard[i].iZhongNiaoNum;
		inf[i].i64WinMoney = sUserData.m_TCalculateBoard[i].i64WinMoney;


		char szUserInf[512] = {0};
		memcpy_s(szUserInf,sizeof(szUserInf),&inf[i],sizeof(GameInfForReplay));

		__super::ReleaseRoom(m_pUserInfo[i]->m_UserData.dwUserID,szUserInf,sizeof(GameInfForReplay));
	}
}

//解散房间弹出结算框
void CServerGameDesk::DissMissRoomBefore( void )
{
	if (IsBuyDesk())
	{
		BYTE nextid = 255, time =255;
		m_sThingHandle.ExecuteCountFen(nextid, time);
		LOGGER_FILE(m_GameContext,"玩了"<<GetRunGameCount()<<"局, 中途解散房间");
		KillTimer(TIME_WAIT_MSG);
		KillTimer(10);
		KillTimer(TIME_CHECK_GAME_MSG);
		for (int i=0; i<m_bMaxPeople; i++)
		{
			if (m_pUserInfo[i] != NULL) 
				m_pUserInfo[i]->m_UserData.bUserState = USER_LOOK_STATE;
		}		

		if ((m_bGameStation>=GS_SEND_CARD && m_bGameStation<GS_WAIT_NEXT) ||  m_iRunGameCount>0)
			UpdateCalculateBoard();

		sUserData.CopyHandPai(sGameData.T_CountFen.m_byArHandPai,0,true);
		sGameData.T_CountFen.byCloseFlag = GFF_DISSMISS_FINISH;
		ReSetGameState(GFF_DISSMISS_FINISH);
	}
}

//发送大结算
void CServerGameDesk::UpdateCalculateBoard()
{
	int iMaxDianPaoStation = 0;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if(sUserData.m_TCalculateBoard[i].iDianPao >= sUserData.m_TCalculateBoard[iMaxDianPaoStation].iDianPao)
		{
			iMaxDianPaoStation = i;
		}
	}

	int iBigWinnerStation = 0;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if(sUserData.m_TCalculateBoard[i].i64WinMoney >= sUserData.m_TCalculateBoard[iBigWinnerStation].i64WinMoney)
		{
			iBigWinnerStation = i;
		}
	}

	sUserData.m_TCalculateBoard[iBigWinnerStation].bWinner = (sUserData.m_TCalculateBoard[iBigWinnerStation].i64WinMoney>0);
	sUserData.m_TCalculateBoard[iMaxDianPaoStation].bBestFire = (sUserData.m_TCalculateBoard[iMaxDianPaoStation].bBestFire>0);

	for(int i = 0; i < PLAY_COUNT; i++)
	{
		sUserData.m_TCalculateBoard[i].bClear = false;
	}

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i])
		{
			LOGGER_FILE(m_GameContext,"玩家"<<i<<","<<m_pUserInfo[i]->m_UserData.nickName<<\
				"的自摸次数="<<sUserData.m_TCalculateBoard[i].iZimo<<\
				"明杠="<<sUserData.m_TCalculateBoard[i].iMingGan<<\
				",暗杠="<<sUserData.m_TCalculateBoard[i].iAnGan<<\
				",中码="<<sUserData.m_TCalculateBoard[i].iZhongNiaoNum<<\
				",点炮="<<sUserData.m_TCalculateBoard[i].iDianPao<<\
				",总成绩="<<sUserData.m_TCalculateBoard[i].i64WinMoney);
			SendGameData(i,sUserData.m_TCalculateBoard,sizeof(sUserData.m_TCalculateBoard),MDM_GM_GAME_NOTIFY,S_C_UPDATE_CALCULATE_BOARD_SIG,0);
		}
	}

	memset(sUserData.m_TCalculateBoard, 0, sizeof(sUserData.m_TCalculateBoard));
}

//发送剩余局数
void CServerGameDesk::SendRemaindedGameCount()
{
	int iNowGameCount = 0;
	if(m_bGameStation == GS_WAIT_SETGAME || m_bGameStation == GS_WAIT_ARGEE || m_bGameStation == GS_WAIT_NEXT)
	{
		iNowGameCount = GetRunGameCount();
	}
	else
	{
	    iNowGameCount = GetRunGameCount() + 1;
	}

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i])
		{
			LOGGER_FILE(m_GameContext,"发送玩家"<<i<<","<<m_pUserInfo[i]->m_UserData.nickName<<",iNowGameCount="<<iNowGameCount);
			SendGameData(i, &iNowGameCount, sizeof(int), MDM_GM_GAME_NOTIFY, S_C_UPDATE_REMAIN_JUSHU_SIG, 0);
		}
	}
}

void CServerGameDesk::LoadIni()
{
	//加载房间设置

	CString s = CINIFile::GetAppPath ();/////本地路径
	CINIFile f(s + SKIN_FOLDER  + _T("_s.ini"));

	CString key;
	key = TEXT("game");
	m_iBaseMoney = f.GetKeyVal(key,"BaseMoney",10); //底分设置
	if(m_iBaseMoney<1 || m_bIsBuy)
	{
		m_iBaseMoney=1;
	}

	m_ZhaMaCount = f.GetKeyVal(key, "ZhaMaCount", 4); // 扎马数量
	char temp[MAX_PATH]={0};
	sprintf(temp,"lx the m_ZhaMaCount %d",m_ZhaMaCount);
	OutputDebugString(temp);
	//m_ZhaMa = f.GetKeyVal(key, "ZhaMaCount", 4); // 扎马数量
	m_ZhaMaFanShu = f.GetKeyVal(key, "ZhaMaFanShu", 2);

	m_AutoActionWhenTooManyTimeout = f.GetKeyVal(key, "AutoActionWhenTimeout", 0);

	key = TEXT("SuperSet");
	m_SuperSetCard = f.GetKeyVal(key, "SetCard", 0) > 0;
	int iCount = f.GetKeyVal(key,"SuperCount",0);
	long int iSuperId = 0;
	CString sText;

	m_vlSuperUserID.clear();
	for(int i=0; i < iCount; i++)
	{
		sText.Format("SuperID_%d",i);
		iSuperId = f.GetKeyVal(key,sText,0);
		m_vlSuperUserID.push_back(iSuperId);
	}

	LoadiniByRoom(m_pDataManage->m_InitData.uRoomID);
}

void CServerGameDesk::InitLogFile()
{
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

	OBJ_GET_EXT(m_GameContext, ExtensionLogger, extLogger);
	char buf[256];
	if(!IsBuyDesk())
		sprintf(buf, "%s/[%d-%d]_%s.log",strPath.GetBuffer(strPath.GetLength()),m_pDataManage->m_InitData.uRoomID, m_bDeskIndex, timeStrBuf);
	else
		sprintf(buf, "%s/[%d-%s]_%s.log",strPath.GetBuffer(strPath.GetLength()),m_pDataManage->m_InitData.uRoomID, m_szDeskPassWord, timeStrBuf);
	strPath.ReleaseBuffer(strPath.GetLength());
	extLogger->Open(buf);
}
