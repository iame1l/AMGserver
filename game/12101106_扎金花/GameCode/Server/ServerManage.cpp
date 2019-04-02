#include "StdAfx.h"
#include "ServerManage.h"
#include "../../../../sdk/include/Common/writelog.h"
#include <math.h>




/*
* 打印日志文件
*/
void DebugPrintf(int iRoomID, const char *p, ...)
{
	return;
	CTime time = CTime::GetCurrentTime();
	char szFilename[256];
	sprintf(szFilename, "%d_%d-%d%d%d-log.txt", NAME_ID, iRoomID, time.GetYear(), time.GetMonth(), time.GetDay());
	FILE *fp = fopen(szFilename, "a");
	if (NULL == fp)
	{
		return;
	}
	va_list arg;
	va_start(arg, p);
	vfprintf(fp, p, arg);
	fprintf(fp,"\n");

	fclose(fp);
}
//构造函数
CServerGameDesk::CServerGameDesk(void):CGameDesk(ALL_ARGEE)
{
	m_TGameData.InitAllData();

	ZeroMemory(m_iTotalCard,sizeof(m_iTotalCard));		//总的牌	
	m_iThisDiZhu			= 0;			//本局底注      
	m_iThisGuoDi			= 0;			//本局锅底值	
	m_i64ThisDingZhu		= 0;			//本局总下注上限	
	//m_i64ThisShangXian		= 0;			//本局个人下注上限  暗注的，但显示为明注的
	m_vclSuperUserID.clear();

	//断线重连需要用到的时间数据
	 m_iLastTimerTime = 1000;
	m_dwLastGameStartTime = 0 ;

	//加载配置文件
	LoadIni();
}

/*--------------------------------------------------------------------------*/
//加载ini配置文件
void	CServerGameDesk::LoadIni()
{
	TCHAR skin[MAX_PATH];
	CString s = CINIFile::GetAppPath ();/////本地路径
	CString temp = _T("");
	temp.Format("%s%s_s.ini",s,GET_SKIN_FOLDER(skin));
	if (!CINIFile::IsFileExist(temp))
	{
		return ;
	}

	CINIFile f(temp);
	CString key = TEXT("Game");

	m_TGameData.m_byBeginTime	= f.GetKeyVal(key,"BeginTime",20);
	m_TGameData.m_byThinkTime	= f.GetKeyVal(key,"ThinkTime",20);

	m_TGameData.m_iDiZhu		= f.GetKeyVal(key,"DiZhu",10);				//最小单注下注
	m_TGameData.m_i64DingZhu	= f.GetKeyVal(key,"DingZhu",1000000000);		//最大总注//mark
	m_TGameData.m_iGuoDi		= f.GetKeyVal(key,"GuoDi",10);				//底注
	//m_TGameData.m_i64ShangXian	= f.GetKeyVal(key,"ShangXian",10000);		//一轮最大下注
	m_TGameData.m_NoteKinds[0]  = f.GetKeyVal(key, "NoteKind1", 50);
	m_TGameData.m_NoteKinds[1]  = f.GetKeyVal(key, "NoteKind2", 100);
	m_TGameData.m_NoteKinds[2]  = f.GetKeyVal(key, "NoteKind3", 200);
	m_TGameData.m_NoteKinds[3]  = f.GetKeyVal(key, "NoteKind4", 500);

    

	for (int i = 0; i < 4; ++i)
	{
		G_iChouMaMoney[i] = m_TGameData.m_NoteKinds[i];
	}

	//todo AI获胜
	m_bAIWinAndLostAutoCtrl = f.GetKeyVal(key,"AIWinAndLostAutoCtrl",1);				//机器人输赢控制20121122dwj
	m_iAIWantWinMoneyA1		= f.GetKeyVal(key,"AIWantWinMoneyA1 ",__int64(500000));		/**<机器人赢钱区域1  */
	m_iAIWantWinMoneyA2		= f.GetKeyVal(key,"AIWantWinMoneyA2 ",__int64(5000000));	/**<机器人赢钱区域2  */
	m_iAIWantWinMoneyA3		= f.GetKeyVal(key,"AIWantWinMoneyA3 ",__int64(50000000));	/**<机器人赢钱区域3  */
	m_iAIWinLuckyAtA1		= f.GetKeyVal(key,"AIWinLuckyAtA1 ",95);					/**<机器人在区域1赢钱的概率  */
	m_iAIWinLuckyAtA2		= f.GetKeyVal(key,"AIWinLuckyAtA2 ",95);					/**<机器人输赢控制：机器人在区域2赢钱的概率  */
	m_iAIWinLuckyAtA3		= f.GetKeyVal(key,"AIWinLuckyAtA3 ",95);					/**<机器人输赢控制：机器人在区域3赢钱的概率  */
	m_iAIWinLuckyAtA4		= f.GetKeyVal(key,"AIWinLuckyAtA4 ",95);					/**<机器人输赢控制：机器人在区域4赢钱的概率  */
	G_i64AIHaveWinMoney		= f.GetKeyVal(key,"AIHaveWinMoney ",__int64(0));			/**<机器人输赢控制：直接从配置文件中读取机器人已经赢钱的数目  */
	G_i64ReSetAIHaveWinMoney	= f.GetKeyVal(key,"ReSetAIHaveWinMoney ",__int64(0));		//记录重置机器人赢钱数，如果游戏过程中改变了就要改变机器人赢钱数
	//

	//超端
	m_vclSuperUserID.clear();
	key = TEXT("SuperSet");
	int iCount = f.GetKeyVal(key,"SuperCount",0);	//超端个数
	CString sText;
	for (int i=0; i<iCount; i++)
	{
		sText.Format("SuperUserID[%d]",i);
		long lID = f.GetKeyVal(key,sText,0);
		m_vclSuperUserID.push_back(lID);
	}
	return;
}
/*--------------------------------------------------------------------------*/
bool CServerGameDesk::InitDeskGameStation()
{
	LoadExtIni(m_pDataManage->m_InitData.uRoomID);

	memset(m_TCalculateBoard,0,sizeof(m_TCalculateBoard));

	return true;
}
/*--------------------------------------------------------------------------*/
//根据房间ID加载配置文件
BOOL	CServerGameDesk::LoadExtIni(int iRoomID)
{
	CString s = CINIFile::GetAppPath ();/////本地路径
	TCHAR skin[MAX_PATH];
	CString temp = _T("");
	temp.Format("%s%s_s.ini",s,GET_SKIN_FOLDER(skin));

	CINIFile f(temp);
	CString szSec("Game");
	//去掉房间的设置
	//szSec.Format("%d_%d",NAME_ID,iRoomID);

	m_TGameData.m_iDiZhu		= f.GetKeyVal(szSec,"DiZhu",m_TGameData.m_iDiZhu); //最小单注下注
	m_TGameData.m_i64DingZhu	= f.GetKeyVal(szSec,"DingZhu",m_TGameData.m_i64DingZhu); //最大总注
	m_TGameData.m_iGuoDi		= f.GetKeyVal(szSec,"GuoDi",m_TGameData.m_iGuoDi); //底注
	//m_TGameData.m_i64ShangXian	= f.GetKeyVal(szSec,"ShangXian",m_TGameData.m_i64ShangXian); //一轮最大下注

	//奖池配置
	m_bAIWinAndLostAutoCtrl = f.GetKeyVal(szSec,"AIWinAndLostAutoCtrl",0);				//机器人输赢控制20121122dwj
	m_iAIWantWinMoneyA1		= f.GetKeyVal(szSec,"AIWantWinMoneyA1 ",__int64(500000));		/**<机器人赢钱区域1  */
	m_iAIWantWinMoneyA2		= f.GetKeyVal(szSec,"AIWantWinMoneyA2 ",__int64(5000000));	/**<机器人赢钱区域2  */
	m_iAIWantWinMoneyA3		= f.GetKeyVal(szSec,"AIWantWinMoneyA3 ",__int64(50000000));	/**<机器人赢钱区域3  */
	m_iAIWinLuckyAtA1		= f.GetKeyVal(szSec,"AIWinLuckyAtA1 ",90);					/**<机器人在区域1赢钱的概率  */
	m_iAIWinLuckyAtA2		= f.GetKeyVal(szSec,"AIWinLuckyAtA2 ",70);					/**<机器人输赢控制：机器人在区域2赢钱的概率  */
	m_iAIWinLuckyAtA3		= f.GetKeyVal(szSec,"AIWinLuckyAtA3 ",50);					/**<机器人输赢控制：机器人在区域3赢钱的概率  */
	m_iAIWinLuckyAtA4		= f.GetKeyVal(szSec,"AIWinLuckyAtA4 ",30);					/**<机器人输赢控制：机器人在区域4赢钱的概率  */
	G_i64AIHaveWinMoney		= f.GetKeyVal(szSec,"AIHaveWinMoney ",__int64(0));			/**<机器人输赢控制：直接从配置文件中读取机器人已经赢钱的数目  */
	G_i64ReSetAIHaveWinMoney	= f.GetKeyVal(szSec,"ReSetAIHaveWinMoney ",__int64(0));		//记录重置机器人赢钱数，如果游戏过程中改变了就要改变机器人赢钱数


	return TRUE;
}
/*--------------------------------------------------------------------------*/
//析构函数
CServerGameDesk::~CServerGameDesk(void)
{

}

/*-------------------------------------------------------------------*/
bool CServerGameDesk::OnTimer(UINT uTimerID)
{
	switch(uTimerID)
	{
	case TIME_SEND_CARD:
		{
			KillTimer(TIME_SEND_CARD);
			if (m_bGameStation == GS_SEND_CARD) 
			{
				//开始发牌
				SendCard();
			}
			return true;
		}
	case TIME_BEGIN_PLAY:	//发牌完成 开始下注计时器
		{
			KillTimer(TIME_BEGIN_PLAY);
			if (m_bGameStation == GS_SEND_CARD)
			{
				BeginPlayUpGrade();
			}
			return true;
		}
	case TIME_WAIT_ACTION:	//操作时间到 自动弃牌
		{
			KillTimer(TIME_WAIT_ACTION);
			if (m_bGameStation == GS_PLAY_GAME)
			{
				//自动弃牌
				AutoGiveUp();
			}
			return true;
		}
	case TIME_NOTICE_ACTION:	//通知下注
		{	//发放4种行为的入口
			KillTimer(TIME_NOTICE_ACTION);
			if (m_bGameStation == GS_PLAY_GAME)
			{
				NoticeUserAction();
			}

			//记录时间段，用于断线重连所是需要的剩余时间段
			m_iLastTimerTime = ((m_TGameData.m_byThinkTime + 1) * 1000);
			m_dwLastGameStartTime = GetTickCount();

			return true;
		}
	case TIME_GAME_FINISH:	//游戏结束
		{
			KillTimer(TIME_GAME_FINISH);
			if (m_bGameStation == GS_PLAY_GAME)
			{
				GameFinish(0,GF_NORMAL);
			}
			return true;
		}
    case TIME_READY_:
    case TIME_READY_ + 1:
    case TIME_READY_ + 2:
    case TIME_READY_ + 3:
    case TIME_READY_ + 4:
    case TIME_READY_ + 5:
        {
            OutputDebugString("WBJ::Agree2");
            KillTimer(uTimerID);
            if(!m_bIsBuy)
                UserLeftDesk(uTimerID - TIME_READY_ , m_pUserInfo[uTimerID - TIME_READY_]);
        }
	}
	return __super::OnTimer(uTimerID);
}
/*-------------------------------------------------------------------*/
//自动弃牌
void	CServerGameDesk::AutoGiveUp()
{
	C_S_UserGiveUp TUserGiveUp;
	TUserGiveUp.byDeskStation = m_TGameData.m_byCurrHandleDesk;
	OnHandleUserGiveUp(m_TGameData.m_byCurrHandleDesk,&TUserGiveUp);
}
/*-------------------------------------------------------------------*/
//检测是否结束
bool	CServerGameDesk::CheckFinish()
{
	int iRemaindCount = 0;
	for (int i=0; i<PlayerCount(); i++)
	{
		if (STATE_ERR != m_TGameData.m_iUserState[i] && STATE_GIVE_UP != m_TGameData.m_iUserState[i] && STATE_LOOK_GIVE_UP != m_TGameData.m_iUserState[i]
		    && STATE_LOST != m_TGameData.m_iUserState[i] && STATE_LOOK_LOST != m_TGameData.m_iUserState[i] )
		{
			iRemaindCount++;
		}
	}
	//剩下最后一个人了 结束
	if (iRemaindCount <= 1)
	{
		return true;
	}

	//是否所有人都处于等待开牌状态
	bool bAllWaiteOpen = true;
	//全部都下注达到了顶住 下注都封顶了 等待开牌 
	for (int i = 0;i < PlayerCount();i++)
	{
		if (STATE_NORMAL == m_TGameData.m_iUserState[i] || STATE_LOOK == m_TGameData.m_iUserState[i])
		{
			bAllWaiteOpen = false;
		}
	}
	if (bAllWaiteOpen)
	{
		return true;
	}

	return false;  //没有结束
}
/*-------------------------------------------------------------------*/
//获取最终胜利的玩家
BYTE	CServerGameDesk::GetFinalWiner()
{
	//判断还有几个未弃牌的玩家
	int iCount = 0;
	BYTE byWiner = 255;
	for (int i=0; i<PlayerCount(); i++)
	{
		if (STATE_ERR != m_TGameData.m_iUserState[i] && STATE_GIVE_UP != m_TGameData.m_iUserState[i] && STATE_LOOK_GIVE_UP != m_TGameData.m_iUserState[i]
		    && STATE_LOST != m_TGameData.m_iUserState[i] && STATE_LOOK_LOST != m_TGameData.m_iUserState[i] )
		{
			iCount++;
			if (255 == byWiner)
			{
				byWiner = i;
			}
		}
	}

	//只有一个正常玩家 那么这个玩家就是赢家了
	if (1 == iCount)
	{
		return byWiner;
	}

	//多人 就要比谁的牌大了
	if (iCount > 1)
	{
		for (int i=0; i<PlayerCount(); i++)
		{
			if (i == byWiner)
			{
				continue;
			}
			if (STATE_ERR != m_TGameData.m_iUserState[i] && STATE_GIVE_UP != m_TGameData.m_iUserState[i] && STATE_LOOK_GIVE_UP != m_TGameData.m_iUserState[i]
			    && STATE_LOST != m_TGameData.m_iUserState[i] && STATE_LOOK_LOST != m_TGameData.m_iUserState[i] )
			{
				int IsWin = m_Logic.CompareCard(m_TGameData.m_byUserCard[byWiner],m_TGameData.m_byUserCardCount[byWiner],m_TGameData.m_byUserCard[i],m_TGameData.m_byUserCardCount[i]);
				//被比较着的牌大
				if (-1 == IsWin)
				{
					byWiner = i;
				}
			}
		}
	}

	return byWiner;

}
/*-------------------------------------------------------------------*/

bool CServerGameDesk::HandleFrameMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	switch(pNetHead->bAssistantID)
	{
	case ASS_GM_FORCE_QUIT:		//强行退出//安全退出
		{
			char cKey[10];
			CString sPath=CBcfFile::GetAppPath();
			int iResult = 1;
			if (CBcfFile::IsFileExist(sPath + "SpecialRule.bcf"))
			{
				CBcfFile fsr( sPath + "SpecialRule.bcf");
				sprintf(cKey, "%d", NAME_ID);
				iResult = fsr.GetKeyVal (_T("ForceQuitAsAuto"), cKey, 1);
			}

			if (iResult)
			{
				return true;
			}
		}
	}
	return __super::HandleFrameMessage( bDeskStation,  pNetHead,  pData,  uSize,  uSocketID,  bWatchUser);
}

//游戏数据包处理函数
bool CServerGameDesk::HandleNotifyMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	if(bWatchUser)
	{
		return TRUE;
	}

	switch (pNetHead->bAssistantID)
	{
	case ASS_GM_AGREE_GAME:		//用户同意游戏
		{
			if (m_bGameStation>=GS_SEND_CARD&&m_bGameStation<GS_WAIT_NEXT)
			{
				return TRUE;
			}
            KillTimer(TIME_READY_ + bDeskStation);
			//GameBegin(ALL_ARGEE);

			return __super::HandleNotifyMessage(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
		}
	case C_S_SUPER_SET://超级客户端设定
		{
			if(uSize!=sizeof(C_S_SuperUserSet))
			{
                OutputDebugString("WBJDZB::Size");
				return false;
			}

			C_S_SuperUserSet *pSuperSet = (C_S_SuperUserSet *) pData;
			if (NULL == pSuperSet)
			{
				return false;
			}
			//响应超端设定
			OnHandleSuperSet(bDeskStation,pSuperSet);
			return true;
		}
	case C_S_LOOK_CARD:		//玩家看牌
		{
			if(m_bGameStation == GS_WAIT_NEXT || m_bGameStation==GS_WAIT_ARGEE || m_bGameStation==GS_WAIT_SETGAME)
			{
				return true;
			}
		
			if(uSize!=sizeof(C_S_LookCard))
			{
				return false;
			}
			
			OnHandleUserLookCard(bDeskStation,pData);
			return true;
		}
	case C_S_GEN_ZHU:		//玩家跟注
		{
			if(m_bGameStation==GS_WAIT_NEXT || m_bGameStation==GS_WAIT_ARGEE || m_bGameStation==GS_WAIT_SETGAME)
			{
				return true;
			}

			if(uSize!=sizeof(C_S_UserGenZhu))
			{
				return false;
			}
			OnHandleUserFollow(bDeskStation,pData);
			return true;
		}
	case C_S_JIA_ZHU:		//玩家加注
		{
			if(m_bGameStation==GS_WAIT_NEXT || m_bGameStation==GS_WAIT_ARGEE || m_bGameStation==GS_WAIT_SETGAME)
			{
				return true;
			}

			if(uSize!=sizeof(C_S_JiaZhu))
			{
				return false;
			}
			OnHandleUserAddNote(bDeskStation,pData);
			return true;
		}
	case C_S_GIVE_UP:	//玩家弃牌
		{
			if(m_bGameStation==GS_WAIT_NEXT || m_bGameStation==GS_WAIT_ARGEE || m_bGameStation==GS_WAIT_SETGAME)
			{
				return true;
			}

			if(uSize!=sizeof(C_S_UserGiveUp))
			{
				return false;
			}
			OnHandleUserGiveUp(bDeskStation,pData);
			return true;
		}
	case C_S_BI_PAI:	//玩家比牌
		{
			if(m_bGameStation==GS_WAIT_NEXT || m_bGameStation==GS_WAIT_ARGEE || m_bGameStation==GS_WAIT_SETGAME)
			{
				return true;
			}

			if(uSize!=sizeof(C_S_UserBiPai))
			{
				return false;
			}
			OnHandleUserOpenCard(bDeskStation,pData);
			return true;
		}
	}
	return __super::HandleNotifyMessage(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
}
/*------------------------------------------------------------*/
void	CServerGameDesk::OnHandleSuperSet(BYTE byDeskStation,C_S_SuperUserSet *pSuperUserSet)
{
    CString tem;
	if (NULL == pSuperUserSet)
	{
        OutputDebugString("WBJDZB::1");
		return;
	}
	if (pSuperUserSet->byMaxDesk == pSuperUserSet->byMinDesk)
	{
        OutputDebugString("WBJDZB::2");
		return;
	}
	//是否超端
	if (!IsSuperUser(byDeskStation))
	{
        OutputDebugString("WBJDZB::3");
        tem.Format("WBJDZB::3--desk=%d",byDeskStation);
        OutputDebugString(tem);
		return;
	}

	//是否已经有人看牌了
	for(int i=0; i<PlayerCount();i++)
	{
		if (m_TGameData.m_iUserState[i] >= STATE_LOOK)
		{
            tem.Format("WBJDZB:: 4--%d,state=%d",i,m_TGameData.m_iUserState[i]);
            OutputDebugString(tem);
			return ;
		}
	}
	
	//设定的玩家是否正常状态
	if (m_TGameData.m_iUserState[byDeskStation] == STATE_ERR)
	{
        OutputDebugString("WBJDZB::5");
		return;
	}

	//设定的玩家状态判断
	if ((255 != pSuperUserSet->byMaxDesk&&m_TGameData.m_iUserState[pSuperUserSet->byMaxDesk] == STATE_ERR)  || (255 != pSuperUserSet->byMinDesk&&m_TGameData.m_iUserState[pSuperUserSet->byMinDesk] == STATE_ERR) )
	{
        tem.Format("WBJDZB::6--maxdesk=%d,mindesk=%d,state=%d,%d",pSuperUserSet->byMaxDesk,pSuperUserSet->byMinDesk,m_TGameData.m_iUserState[pSuperUserSet->byMaxDesk],m_TGameData.m_iUserState[pSuperUserSet->byMinDesk]);
        OutputDebugString(tem);
		return ;
	}
	
	//开始设置最大
	if (pSuperUserSet->byMaxDesk >=0 && pSuperUserSet->byMaxDesk < PlayerCount())
	{
		BYTE byTmpMax = 255;
		for (int i=0; i<PlayerCount(); i++)
		{
			if (STATE_ERR != m_TGameData.m_iUserState[i])
			{
				if (255 == byTmpMax)
				{
					byTmpMax = i;
					break;
				}
			}
		}
		if (byTmpMax != 255)
		{
			for (int i=0; i<PlayerCount(); i++)
			{
				if (i == byTmpMax)
				{
					continue;
				}
				if (STATE_ERR != m_TGameData.m_iUserState[i])
				{
					int IsWin = m_Logic.CompareCard(m_TGameData.m_byUserCard[byTmpMax],MAX_CARD_COUNT,m_TGameData.m_byUserCard[i],MAX_CARD_COUNT);
					//被比较着的牌大
					if (-1 == IsWin)
					{
						byTmpMax = i;
					}
				}
			}
			//最大的玩家!=设定的玩家 准备换牌
			if (byTmpMax != pSuperUserSet->byMaxDesk)
			{
				Change2UserCard(byTmpMax,pSuperUserSet->byMaxDesk);
			}
		}
	}

	//开始设置最小
	if (pSuperUserSet->byMinDesk >=0 && pSuperUserSet->byMinDesk < PlayerCount())
	{
		BYTE byTmpMin = 255;
		for (int i=0; i<PlayerCount(); i++)
		{
			if (STATE_ERR != m_TGameData.m_iUserState[i])
			{
				if (255 == byTmpMin)
				{
					byTmpMin = i;
					break;
				}
			}
		}
		if (byTmpMin != 255)
		{
			for (int i=0; i<PlayerCount(); i++)
			{
				if (i == byTmpMin)
				{
					continue;
				}
				if (STATE_ERR != m_TGameData.m_iUserState[i])
				{
					int IsWin = m_Logic.CompareCard(m_TGameData.m_byUserCard[byTmpMin],MAX_CARD_COUNT,m_TGameData.m_byUserCard[i],MAX_CARD_COUNT);
					//byTmpMin的牌大
					if (1 == IsWin)
					{
						byTmpMin = i;
					}
				}
			}
			//最小的玩家!=设定的玩家 准备换牌
			if (byTmpMin != pSuperUserSet->byMinDesk)
			{
				Change2UserCard(byTmpMin,pSuperUserSet->byMinDesk);
			}
		}
	}

	S_C_SuperUserSetResult TSuperUserSetResult;
	TSuperUserSetResult.bSuccess = true;
	TSuperUserSetResult.byDeskStation = byDeskStation;
	TSuperUserSetResult.byMaxDesk	 = pSuperUserSet->byMaxDesk;
	TSuperUserSetResult.byMinDesk	 = pSuperUserSet->byMinDesk;
	if(m_pUserInfo[byDeskStation])
	{
        OutputDebugString("WBJDZB::7");
		SendGameData(byDeskStation,&TSuperUserSetResult,sizeof(TSuperUserSetResult),MDM_GM_GAME_NOTIFY,S_C_SUPER_SET_RESULT,0);
	}
}
/*------------------------------------------------------------*/
void	CServerGameDesk::OnHandleUserLookCard(BYTE byDeskStation,void * pData)
{
	C_S_LookCard * pLookCard = (C_S_LookCard *)pData;
	if (NULL == pLookCard)
	{
		return;
	}
	
	// 检测当前操作玩家是否合法
	if (byDeskStation != m_TGameData.m_byCurrHandleDesk || m_TGameData.m_iUserState[byDeskStation] != STATE_NORMAL 
		|| m_TGameData.m_iUserState[byDeskStation] == STATE_GIVE_UP || m_TGameData.m_iUserState[byDeskStation] == STATE_LOOK_GIVE_UP
		||  STATE_LOST == m_TGameData.m_iUserState[byDeskStation]
	    ||  STATE_LOOK_LOST == m_TGameData.m_iUserState[byDeskStation] )
	{
		return ;
	}



	//记录玩家的状态
	m_TGameData.m_iUserState[byDeskStation] = STATE_LOOK;

	S_C_LookCardResult TLookCardResult;
	TLookCardResult.byDeskStation	= pLookCard->byDeskStation;
	TLookCardResult.iUserState		= m_TGameData.m_iUserState[byDeskStation];
	TLookCardResult.byUserCardCount = m_TGameData.m_byUserCardCount[byDeskStation];

	for (int i = 0;i < PlayerCount();i++)
	{
		memset(TLookCardResult.byUserCard,0,sizeof(TLookCardResult.byUserCard));
		if (i == pLookCard->byDeskStation)
		{
			memcpy(TLookCardResult.byUserCard,m_TGameData.m_byUserCard[i],sizeof(TLookCardResult.byUserCard));
		}
		if(m_pUserInfo[i])
		{
			SendGameData(i,&TLookCardResult,sizeof(TLookCardResult),MDM_GM_GAME_NOTIFY,S_C_LOOK_CARD_RESULT,0);
		}
	}
	SendWatchData(m_bMaxPeople,&TLookCardResult,sizeof(TLookCardResult),MDM_GM_GAME_NOTIFY,S_C_LOOK_CARD_RESULT,0);	
}


/*------------------------------------------------------------*/
void	CServerGameDesk::OnHandleUserFollow(BYTE byDeskStation,void * pData)
{
	C_S_UserGenZhu * pUserFollow = (C_S_UserGenZhu *)pData;
	if (NULL == pUserFollow)
	{
		return;
	}

	// 检测当前操作玩家是否合法
	if (pUserFollow->byDeskStation != m_TGameData.m_byCurrHandleDesk || m_TGameData.m_iUserState[byDeskStation] == STATE_GIVE_UP 
		|| m_TGameData.m_iUserState[byDeskStation] == STATE_LOOK_GIVE_UP
		|| m_TGameData.m_iUserState[byDeskStation] == STATE_ERR 
		|| STATE_LOST == m_TGameData.m_iUserState[byDeskStation] 
	    || STATE_LOOK_LOST == m_TGameData.m_iUserState[byDeskStation] )
	{
		return ;
	}

	// 防止第一把跟操作, 前家押注不能小于等于0
	if (m_TGameData.m_i64CurrZhuBase <= 0)
	{
		return ;
	}

	__int64	i64XiaZhuNum = 0;
	//如果该玩家已经看牌了就要下注双倍
	if (m_TGameData.m_iUserState[byDeskStation] == STATE_LOOK)
	{
		i64XiaZhuNum = m_TGameData.m_i64CurrZhuBase* 2;
	}
	else
	{
		i64XiaZhuNum = m_TGameData.m_i64CurrZhuBase;
	}
  
	//mark
	//如果当前下注达到了封顶 那么就要限制下  因为最高下注不能超过封顶
	if ((m_TGameData.m_i64XiaZhuData[byDeskStation] + i64XiaZhuNum) > GetUserMoney()/*m_i64ThisDingZhu*/)
	{
		AutoGiveUp();//下注金币不足，则默认五弃牌状态
		return ;
	}
	m_OperateCount[byDeskStation]++;

	//杀掉等待动作计时器
	KillTimer(TIME_WAIT_ACTION);
    for(int i = 0; i < E_CHOUMA_COUNT; ++i) 
    {
        if(G_iChouMaMoney[i] == m_TGameData.m_i64CurrZhuBase)
        {
            if(m_TGameData.m_iUserState[byDeskStation] == STATE_LOOK)
                ++m_TGameData.m_iNoteCount[i][1];
            else
                ++m_TGameData.m_iNoteCount[i][0];
            break;
        }
        
    }
	//如果有人封顶，余下的也要接着封顶
// 	if ( bNeedFengDing())
// 	{
// 		//标记玩家的状态
// 		m_TGameData.m_iUserState[byDeskStation] = STATE_WAITE_OPEN;
// 	}

	//统计个人下注
	m_TGameData.m_i64XiaZhuData[byDeskStation] += i64XiaZhuNum;
	//累加下注总数
	m_TGameData.m_i64ZongXiaZhu += i64XiaZhuNum;

	//不是第一次下注
	m_TGameData.m_bFirstNote = false;
	m_TGameData.m_iAddNoteTime++;

	//获取下个操作的玩家位置
// 	BYTE byNextStation = GetNextNoteStation(byDeskStation);
// 
// 	for (int i=0; i<PLAY_COUNT; i++)
// 	{
// 		if (m_TGameData.m_iUserState[byNextStation] == STATE_LOOK)
// 		{
// 			i64XiaZhuNum = m_TGameData.m_i64CurrZhuBase* 2;
// 		}
// 		else
// 		{
// 			i64XiaZhuNum = m_TGameData.m_i64CurrZhuBase;
// 		}
// 		//检测下个玩家的下注操作是否超过顶住,如果超过,则设置为等待开牌状态,并继续寻找下个玩家位置
// 		if ((m_TGameData.m_i64XiaZhuData[byNextStation] + i64XiaZhuNum) > m_i64ThisDingZhu)
// 		{
// 			m_TGameData.m_iUserState[byNextStation] = STATE_WAITE_OPEN;
// 			byNextStation = GetNextNoteStation(byNextStation);
// 			if (byNextStation == byDeskStation)
// 			{
// 				break;
// 			}
// 		}
// 		else
// 		{
// 			break;
// 		}
// 	}

	S_C_UserGenZhuResult TUserGenZhuResult;
	TUserGenZhuResult.byDeskStation		= byDeskStation;
	TUserGenZhuResult.iUserState		= m_TGameData.m_iUserState[byDeskStation];
	TUserGenZhuResult.i64FollowNum		= i64XiaZhuNum;
	TUserGenZhuResult.i64UserZongZhu	= m_TGameData.m_i64XiaZhuData[byDeskStation];	
	TUserGenZhuResult.i64AllZongZhu		= m_TGameData.m_i64ZongXiaZhu;	

	for (int i = 0;i < PlayerCount();i++)
	{
		if(m_pUserInfo[i])
		{
			SendGameData(i,&TUserGenZhuResult,sizeof(TUserGenZhuResult),MDM_GM_GAME_NOTIFY,S_C_GEN_ZHU_RESULT,0);
		}
	}
	SendWatchData(m_bMaxPeople,&TUserGenZhuResult,sizeof(TUserGenZhuResult),MDM_GM_GAME_NOTIFY,S_C_GEN_ZHU_RESULT,0);	

	//开始判断是否可以结束了
	if (CheckFinish())
	{
		SetTimer(TIME_GAME_FINISH,1*1000);
	}
	else
	{
		//启动计时器准备通知玩家操作
		SetTimer(TIME_NOTICE_ACTION,(1)*1000);
	}
}

/*------------------------------------------------------------*/
void	CServerGameDesk::OnHandleUserAddNote(BYTE byDeskStation,void * pData)
{
	C_S_JiaZhu * pUserJiaZhu = (C_S_JiaZhu *)pData;
	if (NULL == pUserJiaZhu)
	{
		return;
	}
	// 检测当前操作玩家是否合法
	if (pUserJiaZhu->byDeskStation != m_TGameData.m_byCurrHandleDesk || m_TGameData.m_iUserState[byDeskStation] == STATE_GIVE_UP 
		|| m_TGameData.m_iUserState[byDeskStation] == STATE_LOOK_GIVE_UP 
		|| m_TGameData.m_iUserState[byDeskStation] == STATE_ERR 
		|| STATE_LOST == m_TGameData.m_iUserState[byDeskStation]
		|| STATE_LOOK_LOST == m_TGameData.m_iUserState[byDeskStation] )
	{
		return ;
	}
	// 下注不合法
	if (pUserJiaZhu->iAddType < 0 || pUserJiaZhu->iAddType > 3/*E_CHOUMA_COUNT*/)
	{
		return ;
	}

	__int64	i64XiaZhuNum = 0;
	//如果该玩家已经看牌了就要下注双倍
	if (m_TGameData.m_iUserState[byDeskStation] == STATE_LOOK)
	{
		i64XiaZhuNum = G_iChouMaMoney[pUserJiaZhu->iAddType]* 2;
        ++m_TGameData.m_iNoteCount[pUserJiaZhu->iAddType][1];
	}
	else
	{
		i64XiaZhuNum = G_iChouMaMoney[pUserJiaZhu->iAddType];
        ++m_TGameData.m_iNoteCount[pUserJiaZhu->iAddType][0];
	}

	int iTempUserMoney = i64XiaZhuNum;	//个人下注

	//如果当前下注达到了封顶 那么就要限制下  因为最高下注不能超过封顶
	if ((m_TGameData.m_i64XiaZhuData[byDeskStation] + i64XiaZhuNum) > GetUserMoney()/*m_i64ThisDingZhu*/)
	{
		AutoGiveUp();	//下注金币不足，默认为弃牌状态
		return ;
	}

    ;
	m_OperateCount[byDeskStation]++;
	//杀掉等待动作计时器
	KillTimer(TIME_WAIT_ACTION);

	//如果有人封顶，余下的也要接着封顶
// 	if ( bNeedFengDing())
// 	{
// 		//标记玩家的状态
// 		m_TGameData.m_iUserState[byDeskStation] = STATE_WAITE_OPEN;
// 	}

	//统计个人下注
	m_TGameData.m_i64XiaZhuData[byDeskStation] += i64XiaZhuNum;
	//累加下注总数
	m_TGameData.m_i64ZongXiaZhu += i64XiaZhuNum;

	//当前的蒙注累加
	m_TGameData.m_i64CurrZhuBase = G_iChouMaMoney[pUserJiaZhu->iAddType];

	//不是第一次下注了
	m_TGameData.m_bFirstNote = false;

	m_TGameData.m_iAddNoteTime++;


// 	BYTE byNextStation = GetNextNoteStation(byDeskStation);
// 
// 	for (int i=0; i<PLAY_COUNT; i++)
// 	{
// 		if (m_TGameData.m_iUserState[byNextStation] == STATE_LOOK)
// 		{
// 			i64XiaZhuNum = m_TGameData.m_i64CurrZhuBase* 2;
// 		}
// 		else
// 		{
// 			i64XiaZhuNum = m_TGameData.m_i64CurrZhuBase;
// 		}
// 		//检测下个玩家的下注操作是否超过顶住,如果超过,则设置为等待开牌状态,并继续寻找下个玩家位置
// 		if ((m_TGameData.m_i64XiaZhuData[byNextStation] + i64XiaZhuNum) > m_i64ThisDingZhu)
// 		{
// 			m_TGameData.m_iUserState[byNextStation] = STATE_WAITE_OPEN;
// 			byNextStation = GetNextNoteStation(byNextStation);
// 			if (byNextStation == byDeskStation)
// 			{
// 				break;
// 			}
// 		}
// 		else
// 		{
// 			break;
// 		}
// 	}

	S_C_JiaZhuResult TUserAddNoteResult;
	TUserAddNoteResult.byDeskStation	= byDeskStation;
	TUserAddNoteResult.iUserState		= m_TGameData.m_iUserState[byDeskStation];
	TUserAddNoteResult.i64AddNum		= iTempUserMoney/*i64XiaZhuNum*/;
	TUserAddNoteResult.i64UserZongZhu	= m_TGameData.m_i64XiaZhuData[byDeskStation];	
	TUserAddNoteResult.i64AllZongZhu	= m_TGameData.m_i64ZongXiaZhu;	

	for (int i = 0;i < PlayerCount();i++)
	{
		if(m_pUserInfo[i])
		{
			SendGameData(i,&TUserAddNoteResult,sizeof(TUserAddNoteResult),MDM_GM_GAME_NOTIFY,S_C_JIA_ZHU_RESULT,0);
		}
	}
	SendWatchData(m_bMaxPeople,&TUserAddNoteResult,sizeof(TUserAddNoteResult),MDM_GM_GAME_NOTIFY,S_C_JIA_ZHU_RESULT,0);

	//开始判断是否可以结束了
	if (CheckFinish())
	{
		SetTimer(TIME_GAME_FINISH,1*1000);
	}
	else
	{
		//启动计时器准备通知玩家操作
		SetTimer(TIME_NOTICE_ACTION,(1)*1000);
	}
}
/*------------------------------------------------------------*/
void	CServerGameDesk::OnHandleUserGiveUp(BYTE byDeskStation,void * pData)
{
	C_S_UserGiveUp * pUserGiveUp = (C_S_UserGiveUp *)pData;
	if (NULL == pUserGiveUp)
	{
		return;
	}
	// 检测当前操作玩家是否合法
	if (byDeskStation != m_TGameData.m_byCurrHandleDesk)
	{
		return ;
	}

	KillTimer(TIME_WAIT_ACTION);
												
	//记录玩家状态
	if( m_TGameData.m_iUserState[byDeskStation] == STATE_LOOK )
	{
		m_TGameData.m_iUserState[byDeskStation] = STATE_LOOK_GIVE_UP;
	}
	else
	{
		m_TGameData.m_iUserState[byDeskStation] = STATE_GIVE_UP;
	}
	m_TGameData.m_iAddNoteTime++;


// 	__int64 i64XiaZhuNum = 0;
// 	BYTE byNextStation = GetNextNoteStation(byDeskStation);
// 	for (int i=0; i<PLAY_COUNT; i++)
// 	{
// 		if (m_TGameData.m_iUserState[byNextStation] == STATE_LOOK)
// 		{
// 			i64XiaZhuNum = m_TGameData.m_i64CurrZhuBase* 2;
// 		}
// 		else
// 		{
// 			i64XiaZhuNum = m_TGameData.m_i64CurrZhuBase;
// 		}
// 		//检测下个玩家的下注操作是否超过顶住,如果超过,则设置为等待开牌状态,并继续寻找下个玩家位置
// 		if ((m_TGameData.m_i64XiaZhuData[byNextStation] + i64XiaZhuNum) > m_i64ThisDingZhu)
// 		{
// 			m_TGameData.m_iUserState[byNextStation] = STATE_WAITE_OPEN;
// 			byNextStation = GetNextNoteStation(byNextStation);
// 			if (byNextStation == byDeskStation)
// 			{
// 				break;
// 			}
// 		}
// 		else
// 		{
// 			break;
// 		}
// 	}

	S_C_UserGiveUpResult	TUserGiveUpResult;
	TUserGiveUpResult.byDeskStation = byDeskStation;
	TUserGiveUpResult.iUserState	= m_TGameData.m_iUserState[byDeskStation];
	for (int i = 0;i < PlayerCount();i++)
	{
		if(m_pUserInfo[i])
		{
			SendGameData(i,&TUserGiveUpResult,sizeof(TUserGiveUpResult),MDM_GM_GAME_NOTIFY,S_C_GIVE_UP_RESULT,0);
		}
	}
	SendWatchData(m_bMaxPeople,&TUserGiveUpResult,sizeof(TUserGiveUpResult),MDM_GM_GAME_NOTIFY,S_C_GIVE_UP_RESULT,0);	


	//开始判断是否可以结束了
	if (CheckFinish())
	{
		SetTimer(TIME_GAME_FINISH,1*1000);
	}
	else
	{
		//启动计时器准备通知玩家操作
		SetTimer(TIME_NOTICE_ACTION,(1)*1000);
	}
}

/*------------------------------------------------------------*/
//收到玩家比牌消息
void	CServerGameDesk::OnHandleUserOpenCard(BYTE byDeskStation,void * pData)
{
	C_S_UserBiPai * pUserBiPai = (C_S_UserBiPai *)pData;
	if (NULL == pUserBiPai)
	{
		return;
	}
	// 检测当前操作玩家是否合法
	if (byDeskStation != m_TGameData.m_byCurrHandleDesk)
	{
		return ;
	}

	if (m_TGameData.m_iUserState[byDeskStation] == STATE_GIVE_UP 
		|| m_TGameData.m_iUserState[byDeskStation] == STATE_LOOK_GIVE_UP 
		||m_TGameData.m_iUserState[byDeskStation] == STATE_ERR 
		|| STATE_LOST == m_TGameData.m_iUserState[byDeskStation]
	    || STATE_LOOK_LOST == m_TGameData.m_iUserState[byDeskStation]
		)
	{
		return;
	}
	//被比牌的玩家状态
	if (m_TGameData.m_iUserState[pUserBiPai->byBeDeskStation] == STATE_GIVE_UP 
		|| m_TGameData.m_iUserState[pUserBiPai->byBeDeskStation] == STATE_LOOK_GIVE_UP
		|| m_TGameData.m_iUserState[pUserBiPai->byBeDeskStation] == STATE_ERR 
		|| STATE_LOST == m_TGameData.m_iUserState[pUserBiPai->byBeDeskStation]
	    || STATE_LOOK_LOST == m_TGameData.m_iUserState[pUserBiPai->byBeDeskStation]
		)
	{
		return;
	}
	// 一圈下注未结束，不能比牌
	if (m_TGameData.m_iAddNoteTime < CountPlayer())
	{
		return ;
	}
	__int64	i64XiaZhuNum = 0;
	//如果该玩家已经看牌了就要下注双倍
	if (m_TGameData.m_iUserState[byDeskStation] == STATE_LOOK)
	{
		i64XiaZhuNum = m_TGameData.m_i64CurrZhuBase* 2;
	}
	else
	{
		i64XiaZhuNum = m_TGameData.m_i64CurrZhuBase;
	}

	//如果当前下注达到了封顶 那么就要限制下  因为最高下注不能超过封顶
	if ((m_TGameData.m_i64XiaZhuData[byDeskStation] + i64XiaZhuNum) > GetUserMoney()/*m_i64ThisDingZhu*/)
	{
		return ;
	}

	m_OperateCount[byDeskStation]++;
	//杀掉等待动作计时器
	KillTimer(TIME_WAIT_ACTION);

	//如果有人封顶，余下的也要接着封顶
// 	if ( bNeedFengDing())
// 	{
// 		//标记玩家的状态
// 		m_TGameData.m_iUserState[byDeskStation] = STATE_WAITE_OPEN;
// 	}

	m_bOpenRecord[byDeskStation][pUserBiPai->byBeDeskStation] = true;
	m_bOpenRecord[pUserBiPai->byBeDeskStation][byDeskStation] = true;

	//统计个人下注
	m_TGameData.m_i64XiaZhuData[byDeskStation] += i64XiaZhuNum;
	//累加下注总数
	m_TGameData.m_i64ZongXiaZhu += i64XiaZhuNum;

	m_TGameData.m_iAddNoteTime++;


	S_C_UserBiPaiResult	TUserBiPaiResult;
	TUserBiPaiResult.byDeskStation		= byDeskStation;				//主动比牌的玩家
	TUserBiPaiResult.byBeDeskStation	= pUserBiPai->byBeDeskStation;	//被动比牌玩家
	

	int IsWin = m_Logic.CompareCard(m_TGameData.m_byUserCard[byDeskStation],MAX_CARD_COUNT,m_TGameData.m_byUserCard[pUserBiPai->byBeDeskStation],MAX_CARD_COUNT);
	if (IsWin == -1)  //被比较着的牌大
	{
		TUserBiPaiResult.byWinDesk	= pUserBiPai->byBeDeskStation;
		TUserBiPaiResult.byLostDesk	= byDeskStation;
		if( m_TGameData.m_iUserState[byDeskStation] == STATE_LOOK )
		{
			m_TGameData.m_iUserState[byDeskStation] = STATE_LOOK_LOST;
		}
		else
		{
		    m_TGameData.m_iUserState[byDeskStation] = STATE_LOST;
		}
	}
	else   //比牌者比较大
	{
		TUserBiPaiResult.byWinDesk	= byDeskStation;
		TUserBiPaiResult.byLostDesk	= pUserBiPai->byBeDeskStation;
		
		if( m_TGameData.m_iUserState[pUserBiPai->byBeDeskStation] == STATE_LOOK )
		{
		    m_TGameData.m_iUserState[pUserBiPai->byBeDeskStation] = STATE_LOOK_LOST;
		}
		else
		{
			m_TGameData.m_iUserState[pUserBiPai->byBeDeskStation] = STATE_LOST;
		}
	}

	//比牌下注情况
	TUserBiPaiResult.i64AddNum	= i64XiaZhuNum;		
	//玩家的总下注
	TUserBiPaiResult.i64UserZongZhu = m_TGameData.m_i64XiaZhuData[byDeskStation];	
	//所有总下注
	TUserBiPaiResult.i64AllZongZhu	 = m_TGameData.m_i64ZongXiaZhu;	


// 	BYTE byNextStation = GetNextNoteStation(byDeskStation);
// 	for (int i=0; i<PLAY_COUNT; i++)
// 	{
// 		if (m_TGameData.m_iUserState[byNextStation] == STATE_LOOK)
// 		{
// 			i64XiaZhuNum = m_TGameData.m_i64CurrZhuBase* 2;
// 		}
// 		else
// 		{
// 			i64XiaZhuNum = m_TGameData.m_i64CurrZhuBase;
// 		}
// 		//检测下个玩家的下注操作是否超过顶住,如果超过,则设置为等待开牌状态,并继续寻找下个玩家位置
// 		if ((m_TGameData.m_i64XiaZhuData[byNextStation] + i64XiaZhuNum) > m_i64ThisDingZhu)
// 		{
// 			m_TGameData.m_iUserState[byNextStation] = STATE_WAITE_OPEN;
// 			byNextStation = GetNextNoteStation(byNextStation);
// 			if (byNextStation == byDeskStation)
// 			{
// 				break;
// 			}
// 		}
// 		else
// 		{
// 			break;
// 		}
// 	}

	for (int i=0; i<PlayerCount(); i++)
	{
		TUserBiPaiResult.iUserStation[i] = m_TGameData.m_iUserState[i];
	}
	
	for (int i = 0;i < PlayerCount();i++)
	{
		if(m_pUserInfo[i])
		{
			SendGameData(i,&TUserBiPaiResult,sizeof(TUserBiPaiResult),MDM_GM_GAME_NOTIFY,S_C_BI_PAI_RESULT,0);
		}
	}
	SendWatchData(m_bMaxPeople,&TUserBiPaiResult,sizeof(TUserBiPaiResult),MDM_GM_GAME_NOTIFY,S_C_BI_PAI_RESULT,0);	

	//开始判断是否可以结束了
	if (CheckFinish())
	{
		SetTimer(TIME_GAME_FINISH,4*1000);
	}
	else
	{
		//启动计时器准备通知玩家操作
		SetTimer(TIME_NOTICE_ACTION,(4)*1000);
	}
}



/*------------------------------------------------------------*/
//获取游戏状态信息
bool	CServerGameDesk::OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser)
{

	//发送剩余局数
	int iLeaveJuShu = m_iVipGameCount - m_iRunGameCount;
	for (int i = 0; i < PlayerCount(); i++)
	{
		if (m_pUserInfo[i] == NULL)
		{
			continue;
		}
		SendGameData(i,&iLeaveJuShu,sizeof(int),MDM_GM_GAME_NOTIFY,S_C_UPDATE_REMAIN_JUSHU_SIG,0);
	}
    if(!m_bIsBuy && m_bGameStation != GS_SEND_CARD && m_bGameStation != GS_PLAY_GAME)
    {
        CString tem;
        tem.Format("WBJL::Begintime = %d",m_TGameData.m_byBeginTime);
        OutputDebugString(tem);
        SetTimer(TIME_READY_+bDeskStation,m_TGameData.m_byBeginTime * 1000);
    }
        
	//超端验证
	SpuerProof(bDeskStation);
	switch (m_bGameStation)
	{
	case GS_WAIT_SETGAME:		//游戏没有开始状态
	case GS_WAIT_ARGEE:			//等待玩家开始状态
	case GS_WAIT_NEXT:			//等待下一盘游戏开始
		{
			S_C_GameStation_WaiteAgree TGameStation;

			TGameStation.byThinkTime	= m_TGameData.m_byThinkTime;				//游戏思考时间
			TGameStation.byBeginTime	= m_TGameData.m_byBeginTime;				//游戏开始时间
		
			TGameStation.iGuoDi			= m_TGameData.m_iGuoDi;			//锅底值	
			TGameStation.iDiZhu			= m_TGameData.m_iDiZhu;			//底注      
			
			//TGameStation.i64ShangXian	= m_i64ThisShangXian;	//总下注上限		
			TGameStation.i64DingZhu		= m_TGameData.m_i64DingZhu;		//个人下注上限  暗注的，但显示为明注的
            memcpy(TGameStation.iNoteCount,m_TGameData.m_iNoteCount,sizeof(TGameStation.iNoteCount));
			for (int i = 0; i < 4; ++i)
			{
				TGameStation.iNoteKinds[i] = m_TGameData.m_NoteKinds[i];
			}

			//各玩家的准备状态
			for(int i=0; i<PlayerCount(); i++)
			{
				if (m_pUserInfo[i])
				{
					if (m_pUserInfo[i]->m_UserData.bUserState == USER_ARGEE)
					{
						TGameStation.bAgree[i] = true;
					}
				}
			}

			SendGameStation(bDeskStation,uSocketID,bWatchUser,&TGameStation,sizeof(TGameStation));
			return TRUE;
		}
	case GS_SEND_CARD:		//发牌状态
		{
			S_C_GameStation_SendCard	TGameStation;

			TGameStation.byThinkTime	= m_TGameData.m_byThinkTime;				//游戏思考时间
			TGameStation.byBeginTime	= m_TGameData.m_byBeginTime;				//游戏开始时间

			TGameStation.iGuoDi			= m_TGameData.m_iGuoDi;			//锅底值	
			TGameStation.iDiZhu			= m_TGameData.m_iDiZhu;			//底注      

			//TGameStation.i64ShangXian	= m_i64ThisShangXian;	//总下注上限		
			TGameStation.i64DingZhu		= m_i64ThisDingZhu;		//个人下注上限  暗注的，但显示为明注的

			TGameStation.byNtStation	= m_TGameData.m_byNtPeople;	
			TGameStation.i64AllZongZhu	= m_TGameData.m_i64ZongXiaZhu;

            memcpy(TGameStation.iNoteCount,m_TGameData.m_iNoteCount,sizeof(TGameStation.iNoteCount));

			for (int i = 0; i < 4; ++i)
			{
				TGameStation.iNoteKinds[i] = m_TGameData.m_NoteKinds[i];
			}

			for (int i = 0;i<PlayerCount();i++)
			{
				TGameStation.iUserState[i]		= m_TGameData.m_iUserState[i];
				TGameStation.byUserCardCount[i]	= m_TGameData.m_byUserCardCount[i];
				TGameStation.i64XiaZhuData[i]	= m_TGameData.m_i64XiaZhuData[i];
			}

			SendGameStation(bDeskStation,uSocketID,bWatchUser,&TGameStation,sizeof(TGameStation));
			return TRUE;
		}
	case GS_PLAY_GAME:	//游戏中状态
		{
			S_C_GameStation_PlayGame	TGameStation;

			TGameStation.byThinkTime	= m_TGameData.m_byThinkTime;				//游戏思考时间
			TGameStation.byBeginTime	= m_TGameData.m_byBeginTime;				//游戏开始时间

			TGameStation.iGuoDi			= m_TGameData.m_iGuoDi;			//锅底值	
			TGameStation.iDiZhu			=  m_TGameData.m_iDiZhu;			//底注    

			TGameStation.iRemainderTime	= GetGameRemainTime()/1000 ;

			//TGameStation.i64ShangXian	= m_i64ThisShangXian;	//总下注上限		
			TGameStation.i64DingZhu		= m_i64ThisDingZhu;		//个人下注上限  暗注的，但显示为明注的

			TGameStation.byNtStation	= m_TGameData.m_byNtPeople;	
			TGameStation.i64AllZongZhu	= m_TGameData.m_i64ZongXiaZhu;

            memcpy(TGameStation.iNoteCount,m_TGameData.m_iNoteCount,sizeof(TGameStation.iNoteCount));

			for (int i = 0; i < 4; ++i)
			{
				TGameStation.iNoteKinds[i] = m_TGameData.m_NoteKinds[i];
			}

			for (int i = 0;i<PlayerCount();i++)
			{
				TGameStation.iUserState[i]		= m_TGameData.m_iUserState[i];
				TGameStation.byUserCardCount[i]	= m_TGameData.m_byUserCardCount[i];
				TGameStation.i64XiaZhuData[i]	= m_TGameData.m_i64XiaZhuData[i];
			}
			//当前操作的玩家
			TGameStation.byCurrHandleDesk= m_TGameData.m_byCurrHandleDesk;	
			//用户手上的牌
			memcpy(TGameStation.byUserCard,m_TGameData.m_byUserCard,sizeof(TGameStation.byUserCard));

			if(m_TGameData.m_byCurrHandleDesk == bDeskStation)
			{
				TGameStation.bCanLook		= m_TGameData.m_bCanLook;	//可否看牌
		    	TGameStation.bCanFollow		= m_TGameData.m_bCanFollow;	//可否跟注
			    //TGameStation.bCanAdd		= m_TGameData.m_bCanAdd;	//可否加注
			    memcpy(TGameStation.bCanAdd,m_TGameData.m_bCanAdd,sizeof(m_TGameData.m_bCanAdd));//可否加注
			    TGameStation.bCanOpen		= m_TGameData.m_bCanOpen;	//可否比牌
			    TGameStation.bCanGiveUp		= m_TGameData.m_bCanGiveUp;	//可否弃牌
			}
			
			SendGameStation(bDeskStation,uSocketID,bWatchUser,&TGameStation,sizeof(TGameStation));
			return TRUE;
		}
	}
	return false;
}
/*------------------------------------------------------------------*/
//重置游戏状态
bool	CServerGameDesk::ReSetGameState(BYTE bLastStation)
{
	KillTimer(TIME_SEND_CARD);
	KillTimer(TIME_GAME_FINISH);
	//游戏状态
	m_bGameStation=GS_WAIT_NEXT;
	ReSetGameData();
	return TRUE;
}
/*------------------------------------------------------------------*/
void	CServerGameDesk::ReSetGameData()
{
	m_TGameData.InitSomeData();
	ZeroMemory(m_iTotalCard,sizeof(m_iTotalCard));		//总的牌	
	m_iThisDiZhu			= 0;			//本局底注      
	m_iThisGuoDi			= 0;			//本局锅底值	
	m_i64ThisDingZhu		= 0;			//本局总下注上限	
	//m_i64ThisShangXian		= 0;			//本局个人下注上限  暗注的，但显示为明注的	
	memset(m_OperateCount,0,sizeof(m_OperateCount));

	for (int i=0; i<PlayerCount();i++)
	{
		memset(m_bOpenRecord[i],false,sizeof(bool)*6);
	}
}
/*------------------------------------------------------------------*/
//游戏开始
bool CServerGameDesk::GameBegin(BYTE bBeginFlag)
{
	if (__super::GameBegin(bBeginFlag)==false) 
	{
		return false;
	}

	if (0 == m_iRunGameCount)
	{
		for(int i = 0; i < PlayerCount(); i ++)
		{
			m_vecWinInfo[i].RemoveAll();
		}
		memset(m_TCalculateBoard,0,sizeof(m_TCalculateBoard));
	}


	KillTimer(TIME_GAME_FINISH);
    for(int i=0 ; i<PLAY_COUNT;++i)
    {
        KillTimer(TIME_READY_ + i);
    }
	//重置部分游戏数据
	ReSetGameData();
	//游戏状态
	m_bGameStation = GS_SEND_CARD;	

	//保存玩家的状态
	for (int i = 0;i < PlayerCount();i++) 
	{
		if (m_pUserInfo[i])
		{
			m_TGameData.m_iUserState[i] = STATE_NORMAL;
		}
		else
		{
			m_TGameData.m_iUserState[i] = STATE_ERR;
		}
	}
	__int64 i64MinMoney = GetMinMoney();
	//计算顶注（顶注不能超过最小金币数）
	if (i64MinMoney < m_TGameData.m_i64DingZhu)
	{
		m_i64ThisDingZhu = i64MinMoney;
	}
	else
	{
		m_i64ThisDingZhu = m_TGameData.m_i64DingZhu;
	}

	m_iThisDiZhu = m_TGameData.m_iDiZhu;

	m_iThisGuoDi = m_TGameData.m_iGuoDi;

	//m_i64ThisShangXian = m_TGameData.m_i64ShangXian;

	//各家目前押注情况,游戏开始之前每位玩家先下锅底
	for(int i=0;i<PlayerCount();i++)									
	{
		m_TGameData.m_i64XiaZhuData[i] = 0;
		if(m_pUserInfo[i] != NULL && m_TGameData.m_iUserState[i] == STATE_NORMAL)
		{
			m_TGameData.m_i64XiaZhuData[i]	+= m_iThisGuoDi;
			m_TGameData.m_i64ZongXiaZhu		+= m_iThisGuoDi;
		}
	}

	BYTE bPlayCount = CountPlayer();

	if (m_TGameData.m_byNtPeople == 255)
	{
		// 创建桌子的人做庄
		for(int i = 0; i < PlayerCount(); ++i)
		{
			if(m_pUserInfo[i] && m_pUserInfo[i]->m_UserData.dwUserID == m_iDeskMaster)
			{
				m_TGameData.m_byNtPeople = i;
				break;
			}
		}

		if(m_TGameData.m_byNtPeople == 255)
		{
			//随机生成庄家
			m_TGameData.m_byNtPeople = rand() % bPlayCount;
		}
	}

	if(	m_pUserInfo[m_TGameData.m_byNtPeople] == NULL)
	{
		for(int i=0; i<PlayerCount(); ++i)
		{
			if(m_pUserInfo[i] == NULL)
			{
				continue;
			}
			m_TGameData.m_byNtPeople = i;
			break;
		}
	}

	S_C_GameBegin TGameBegin;
	memset(&TGameBegin,0,sizeof(TGameBegin));
	TGameBegin.byNtStation		= m_TGameData.m_byNtPeople;
	TGameBegin.iGuoDi			= m_iThisGuoDi;							
	TGameBegin.iDiZhu			= m_iThisDiZhu;
	//TGameBegin.i64ShangXian		= m_i64ThisShangXian;
	TGameBegin.i64DingZhu		= m_i64ThisDingZhu;
	TGameBegin.i64ZongXiaZhu	= m_TGameData.m_i64ZongXiaZhu;

	

	//下注情况
	memcpy(TGameBegin.i64XiaZhuData,m_TGameData.m_i64XiaZhuData,sizeof(TGameBegin.i64XiaZhuData));
	//各玩家的状态
	memcpy(TGameBegin.iUserState, m_TGameData.m_iUserState,sizeof(TGameBegin.iUserState));
	for (int i=0;i<PlayerCount();i++) 
	{
		if(m_pUserInfo[i])
		{
			SendGameData(i,&TGameBegin,sizeof(TGameBegin),MDM_GM_GAME_NOTIFY,S_C_GAME_BEGIN,0);
		}
	}
	SendWatchData(m_bMaxPeople,&TGameBegin,sizeof(TGameBegin),MDM_GM_GAME_NOTIFY,S_C_GAME_BEGIN,0);


	//启动发牌计时器
	SetTimer(TIME_SEND_CARD,(2)*1000);

	return TRUE;
}
/*------------------------------------------------------------------*/
//发送扑克给用户
BOOL CServerGameDesk::SendCard()
{
	KillTimer(TIME_SEND_CARD);
    
   TCHAR skin[MAX_PATH];
   CString s = CINIFile::GetAppPath ();/////本地路径
   CString temp = _T("");
   temp.Format("%s%s_s.ini",s,GET_SKIN_FOLDER(skin));
   if (!CINIFile::IsFileExist(temp))
   {
        return false;
   }

   CINIFile f(temp);
   CString key = TEXT("Game");
   CString cardName;
   //分发扑克
   m_Logic.RandCard(m_iTotalCard,52,m_bDeskIndex);
   m_TGameData.m_byCanSet = f.GetKeyVal(key,"CanSet",0);
   cardName.Format("Card::CanSet = %d",m_TGameData.m_byCanSet);
   OutputDebugString(cardName);
   if(m_TGameData.m_byCanSet)
   {
        for (int i=0; i<PlayerCount(); i++)
        {
            if (m_TGameData.m_iUserState[i] == STATE_NORMAL && m_pUserInfo[i])
            {
                for (int j=0; j<MAX_CARD_COUNT;j++)
                {
                    cardName.Format("Card%d[%d]",m_pUserInfo[i]->m_UserData.dwUserID,j);
                    OutputDebugString(cardName);
                    BYTE byCard =  f.GetKeyVal(key, cardName, m_iTotalCard[i * 3+j]);
                    m_iTotalCard[i * 3 + j] = byCard;
                    m_TGameData.m_byUserCard[i][j] = byCard;
                }
                m_TGameData.m_byUserCardCount[i] = MAX_CARD_COUNT;
            }
            else
            {
                m_TGameData.m_byUserCardCount[i] = 0;
                memset(m_TGameData.m_byUserCard[i],0,sizeof(m_TGameData.m_byUserCard[i]));
            }
        }
    }
    else 
    {
        //分发扑克
       // m_Logic.RandCard(m_iTotalCard,52,m_bDeskIndex);
        for (int i=0; i<PlayerCount(); i++)
        {
            if (m_TGameData.m_iUserState[i] == STATE_NORMAL && m_pUserInfo[i])
            {
                for (int j=0; j<MAX_CARD_COUNT;j++)
                {
                    BYTE byCard = m_iTotalCard[i * 3+j];
                    m_TGameData.m_byUserCard[i][j] = byCard;
                    
                }
                m_TGameData.m_byUserCardCount[i] = MAX_CARD_COUNT;
            }
            else
            {
                m_TGameData.m_byUserCardCount[i] = 0;
                memset(m_TGameData.m_byUserCard[i],0,sizeof(m_TGameData.m_byUserCard[i]));
            }
        }
    }
	

	//机器人奖池控制//dealer
	if (m_bAIWinAndLostAutoCtrl)
	{
		AiWinAutoCtrl();
	}

	S_C_SendCard TSendCard;
	//下发所有玩家牌的数据(不考虑外挂)
	memcpy(TSendCard.byCardCount,m_TGameData.m_byUserCardCount,sizeof(TSendCard.byCardCount));
	memcpy(TSendCard.byCard,m_TGameData.m_byUserCard,sizeof(TSendCard.byCard));
	//发牌顺序  从庄家开始发牌
	int iIndex = 0;
	for(int m=0; m<MAX_CARD_COUNT; m++)
	{
		for(int n=0; n<PlayerCount(); n++)
		{
			if (m_TGameData.m_iUserState[(m_TGameData.m_byNtPeople+n)%PlayerCount()] == STATE_ERR)
			{
				TSendCard.bySendCardTurn[iIndex] = 255;
			}
			else
			{
				BYTE bDesk = (m_TGameData.m_byNtPeople+n)%PlayerCount();
				TSendCard.bySendCardTurn[iIndex] = bDesk;
			}
			iIndex++;
		}
	}

	for(int i=0;i<PlayerCount();i++)
	{
		if (m_pUserInfo[i])
		{
			SendGameData(i,&TSendCard,sizeof(TSendCard),MDM_GM_GAME_NOTIFY,S_C_SEND_CARD,0);
		}
	}
	SendWatchData(m_bMaxPeople,&TSendCard,sizeof(TSendCard),MDM_GM_GAME_NOTIFY,S_C_SEND_CARD,0);

	//启动开始计时器
	SetTimer(TIME_BEGIN_PLAY,CountPlayer() * 1000);

	return TRUE;
}
/*------------------------------------------------------------------*/
//游戏开始-开始下注(第一轮的)
BOOL CServerGameDesk::BeginPlayUpGrade()
{
	KillTimer(TIME_BEGIN_PLAY);
	//设置数据
	m_bGameStation=GS_PLAY_GAME;

	//当前的下注基数
	m_TGameData.m_i64CurrZhuBase	= 0;
	//是第一次下注
	m_TGameData.m_bFirstNote = true;
	//庄家的下家第一个开始下注
	m_TGameData.m_byCurrHandleDesk = GetNextNoteStation(m_TGameData.m_byNtPeople);
	//当前有哪些操作权限
	m_TGameData.m_bCanLook		= true;
	m_TGameData.m_bCanFollow	= false;
	m_TGameData.m_bCanOpen		= false;
	m_TGameData.m_bCanGiveUp	= true;

	__int64 i64XiaZhuNum = 0;
	memset(m_TGameData.m_bCanAdd,0,sizeof(m_TGameData.m_bCanAdd));
	for(int i = 0; i < E_CHOUMA_COUNT;i++)
	{
		//如果该玩家已经看牌了就要下注双倍
		if (m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] == STATE_LOOK)
		{
			i64XiaZhuNum = G_iChouMaMoney[i]* 2;
		}
		else
		{
			i64XiaZhuNum = G_iChouMaMoney[i];
		}

		
		if ((m_TGameData.m_i64XiaZhuData[m_TGameData.m_byCurrHandleDesk] + i64XiaZhuNum) <= GetUserMoney()/*m_i64ThisDingZhu*/)
		{
			m_TGameData.m_bCanAdd[i] = true;
		}
		else
		{
			//m_TGameData.m_bCanAdd[i] = false;
			break;
		}
	}




	S_C_BeginPlay TBeginPlay;
	TBeginPlay.byCurrHandleStation	= m_TGameData.m_byCurrHandleDesk;
	TBeginPlay.bLook	= m_TGameData.m_bCanLook;		//是否可看牌
	TBeginPlay.bFollow	= m_TGameData.m_bCanFollow;		//是否可跟注
	memcpy(TBeginPlay.bAdd,m_TGameData.m_bCanAdd,sizeof(TBeginPlay.bAdd));		//是否可加注)
	TBeginPlay.bOpen	= m_TGameData.m_bCanOpen;		//是否可比牌
	TBeginPlay.bGiveUp	= m_TGameData.m_bCanGiveUp;		//是否可弃牌


	for (int i=0;i<PLAY_COUNT;i++)
	{
		if (m_pUserInfo[i])
		{
			SendGameData(i,&TBeginPlay,sizeof(TBeginPlay),MDM_GM_GAME_NOTIFY,S_C_GAME_PLAY,0);
		}
	}	
	SendWatchData(m_bMaxPeople,&TBeginPlay,sizeof(TBeginPlay),MDM_GM_GAME_NOTIFY,S_C_GAME_PLAY,0);

	m_iLastTimerTime = ((m_TGameData.m_byThinkTime + 1) * 1000);
	m_dwLastGameStartTime = GetTickCount();

    if(!m_bIsBuy)
    {
        //启动玩家操作计时器
        SetTimer(TIME_WAIT_ACTION,(m_TGameData.m_byThinkTime + 1) * 1000);
    }
	
	return TRUE;
}
/*------------------------------------------------------------------*/
//通知玩家下注
void	CServerGameDesk::NoticeUserAction()
{
	KillTimer(TIME_NOTICE_ACTION);


	//设定当前操作的玩家
	m_TGameData.m_byCurrHandleDesk = GetNextNoteStation(m_TGameData.m_byCurrHandleDesk);

	//是否可以看牌
	if (m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] != STATE_ERR && m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] != STATE_LOOK)
	{
		m_TGameData.m_bCanLook = true;
	}
	else
	{
		m_TGameData.m_bCanLook = false;
	}

	__int64	i64XiaZhuNum = 0;

	
	//是否可以跟注	//fixme? 跟注需要判断钱是否足够
	m_TGameData.m_bCanFollow	=   (!m_TGameData.m_bFirstNote);

	memset(m_TGameData.m_bCanAdd,0,sizeof(m_TGameData.m_bCanAdd));
	for(int i = 0; i < E_CHOUMA_COUNT;i++)
	{
		//加注要比当前大//基准是上一家的筹码.既然是加注不能比上一家低
		if (G_iChouMaMoney[i] <= m_TGameData.m_i64CurrZhuBase)
		{
			continue;
		}
		//如果该玩家已经看牌了就要下注双倍
		if (m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] == STATE_LOOK)
		{
			i64XiaZhuNum = G_iChouMaMoney[i]* 2;
		}
		else
		{
			i64XiaZhuNum = G_iChouMaMoney[i];
		}

		//
		if ((m_TGameData.m_i64XiaZhuData[m_TGameData.m_byCurrHandleDesk] + i64XiaZhuNum) <= GetUserMoney()/*m_i64ThisDingZhu*/)
		{
			m_TGameData.m_bCanAdd[i] = true;  
		}
		else
		{			 
			break;
		}
	}
	
	//mark
	//查询当前是否有足够的钱下注,如果不够,则默认为放弃状态	
	if (!CanXiZhu())
	{
		AutoGiveUp();
		return;
	}
	

	//是否可以比牌								
	m_TGameData.m_bCanOpen		= (m_TGameData.m_iAddNoteTime >= CountPlayer())?true:false;	
	//封顶了 也不能比牌了 只能跟注
// 	if (bNeedFengDing())
// 	{
// 		m_TGameData.m_bCanOpen	= false;
// 	}
// 	if (CanXiZhu())
// 	{
// 		m_TGameData.m_bCanOpen	= true;
// 	}
// 	else
// 	{
// 		m_TGameData.m_bCanOpen	= false;
// 	}


	//是否可以弃牌	
	if (m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] != STATE_ERR && m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] != STATE_GIVE_UP
		&& m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] != STATE_LOOK_GIVE_UP
		&& STATE_LOST != m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] 
	    && STATE_LOOK_LOST != m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk]
		)
	{
		m_TGameData.m_bCanGiveUp = true;	
	}
	else
	{
		m_TGameData.m_bCanGiveUp = false;
	}

	//通知玩家下注
	S_C_NoticeAction TNoticeAction;
	TNoticeAction.byCurrHandleStation	= m_TGameData.m_byCurrHandleDesk;
	TNoticeAction.bCanLook		= m_TGameData.m_bCanLook;
	TNoticeAction.bCanFollow	= m_TGameData.m_bCanFollow;
	TNoticeAction.bCanOpen		= m_TGameData.m_bCanOpen;
	TNoticeAction.bCanGiveUp	= m_TGameData.m_bCanGiveUp;
	memcpy(TNoticeAction.bCanAdd,m_TGameData.m_bCanAdd,sizeof(TNoticeAction.bCanAdd));
	for (int i = 0;i < PlayerCount();i++)
	{
		if(m_pUserInfo[i])		
		{
			SendGameData(i,&TNoticeAction,sizeof(TNoticeAction),MDM_GM_GAME_NOTIFY,S_C_CALL_ACTION,0);
		}
	}	
	SendWatchData(m_bMaxPeople,&TNoticeAction,sizeof(TNoticeAction),MDM_GM_GAME_NOTIFY,S_C_CALL_ACTION,0);
    if(!m_bIsBuy)
    {
        //启动玩家操作计时器
        SetTimer(TIME_WAIT_ACTION,(m_TGameData.m_byThinkTime + 1) * 1000);
    }
	
}
/*------------------------------------------------------------------*/
//统计玩游戏玩家
BYTE	CServerGameDesk::CountPlayer()
{
	BYTE count=0;
	for(int i=0;i<m_bMaxPeople;i++)
	{
		if(m_pUserInfo[i] && m_TGameData.m_iUserState[i] != STATE_ERR)
		{
			count++;
		}
	}
	return count;
}
/*---------------------------------------------------------------------*/
//游戏结束
bool	CServerGameDesk::GameFinish(BYTE bDeskStation, BYTE bCloseFlag)
{
	switch (bCloseFlag)
	{
	case GF_NORMAL:		//游戏正常结束
		{	

			//设置游戏状态
			m_bGameStation=GS_WAIT_ARGEE;

			//获取最终胜利的玩家
			int iWiner	= GetFinalWiner();
			//下一盘赢家是庄家
			m_TGameData.m_byNtPeople	= iWiner;

			//游戏结束
			S_C_GameEnd TGameEnd;
			TGameEnd.byWiner = iWiner;

		
			memcpy(TGameEnd.iOperateCount,m_OperateCount,sizeof(TGameEnd.iOperateCount));

			//传给客户端当前牌型
			for (int i = 0;i < PlayerCount();i++)
			{
				if (m_pUserInfo[i]==NULL || m_TGameData.m_iUserState[i] == STATE_ERR) 
				{
					continue;
				}
				//牌型处理
				TGameEnd.byCardShape[i] = m_Logic.GetCardShape(m_TGameData.m_byUserCard[i],MAX_CARD_COUNT);
			}

			for (int i = 0;i < PlayerCount();i++)
			{
				if (m_pUserInfo[i]==NULL || m_TGameData.m_iUserState[i] == STATE_ERR) 
				{
					continue;
				}
				if(i==iWiner)
				{
					TGameEnd.i64UserScore[i] = m_TGameData.m_i64ZongXiaZhu - m_TGameData.m_i64XiaZhuData[i]; 
				}
				else
				{
					//输掉所有押的注
					TGameEnd.i64UserScore[i] = -m_TGameData.m_i64XiaZhuData[i];				
				}
			}

			//写入数据库						
			bool temp_cut[PLAY_COUNT];
			memset(temp_cut,0,sizeof(temp_cut));
		
			ChangeUserPointint64(TGameEnd.i64UserScore, temp_cut);
			__super::RecoderGameInfo(TGameEnd.i64ChangeMoney);

			//奖池记录
			if (m_bAIWinAndLostAutoCtrl)
			{
				RecordAiHaveWinMoney(TGameEnd.i64UserScore);
			}

			//发送数据
			for (int i=0;i<PlayerCount();i++) 
			{
				if (m_pUserInfo[i]!=NULL) 
				{
					memset(TGameEnd.byCard,0,sizeof(TGameEnd.byCard));
					for (int k = 0;k < PlayerCount();k++)
					{
						memcpy(&TGameEnd.byCard[k],&m_TGameData.m_byUserCard[k],sizeof(BYTE)*3);
					}
					//自己的牌
					memcpy(&TGameEnd.byCard[i],&m_TGameData.m_byUserCard[i],sizeof(BYTE)*3);
					//和赢家的牌
					memcpy(&TGameEnd.byCard[iWiner],&m_TGameData.m_byUserCard[iWiner],sizeof(BYTE)*3);
					SendGameData(i,&TGameEnd,sizeof(TGameEnd),MDM_GM_GAME_NOTIFY,S_C_GAME_END,0);
				}
			}
			SendWatchData(m_bMaxPeople,&TGameEnd,sizeof(TGameEnd),MDM_GM_GAME_NOTIFY,S_C_GAME_END,0);


			//玩家输赢的钱累计
			for (int i = 0; i < PlayerCount(); i++)
			{
				m_TCalculateBoard[i].i64WinMoney += TGameEnd.i64UserScore[i];	
			}

			//判断是否购买房间
			if(m_bIsBuy)
			{
				for (int i = 0; i < PlayerCount(); i++)
				{
					//最多赢钱数目
					if(m_TCalculateBoard[i].iMaxWinMoney <= m_TCalculateBoard[i].i64WinMoney)
					{
						m_TCalculateBoard[i].iMaxWinMoney = m_TCalculateBoard[i].i64WinMoney;
					}
					//胜利局数
					if(TGameEnd.i64UserScore[i] > 0)
					{
						//保存输赢记录
						m_vecWinInfo[i].Add(true);
						m_TCalculateBoard[i].iWinCount++;
					}
					else
					{
						//保存输赢记录
						m_vecWinInfo[i].Add(false);
					}
				}

				//更新记录
				UpdateCalculateBoard(false);
			}

			ReSetGameState(bCloseFlag);
			__super::GameFinish(bDeskStation,bCloseFlag);

			//发送游戏局数
			int iLeaveJuShu = m_iVipGameCount - m_iRunGameCount;
			for (int i = 0; i < PlayerCount(); i++)
			{
				if (m_pUserInfo[i] == NULL)
				{
					continue;
				}
                if(!m_bIsBuy)
                    SetTimer( TIME_READY_ + i,m_TGameData.m_byBeginTime * 1000);
				SendGameData(i,&iLeaveJuShu,sizeof(int),MDM_GM_GAME_NOTIFY,S_C_UPDATE_REMAIN_JUSHU_SIG,0);
			}

			return true;
		}
	case GF_SALE:			//游戏安全结束	//只有所有人都断线了 才会安全结束
	case GFF_FORCE_FINISH:		//用户断线离开
		{
			m_bGameStation=GS_WAIT_ARGEE;
			bCloseFlag	= GF_SALE;
			ReSetGameState(bCloseFlag);
			//m_TGameData.InitAllData();
            m_TGameData.m_byNtPeople = 255;
			__super::GameFinish(bDeskStation,bCloseFlag);
			return true;

		}
	case GFF_DISSMISS_FINISH:
		{
			DismissRoom();
			m_bGameStation = GS_WAIT_SETGAME;
			ReSetGameState(bCloseFlag);
			__super::GameFinish(bDeskStation,bCloseFlag);
			return true;
		}
	}

	//重置数据
	ReSetGameState(bCloseFlag);
	__super::GameFinish(bDeskStation,bCloseFlag);

	return true;
}
/*---------------------------------------------------------------------*/
//判断是否正在游戏
bool CServerGameDesk::IsPlayGame(BYTE bDeskStation)
{
	if(m_bGameStation>=GS_SEND_CARD&&m_bGameStation<GS_WAIT_NEXT && m_TGameData.m_iUserState[bDeskStation] != STATE_ERR)
	{
		return true;
	}
	return false;
}
/*---------------------------------------------------------------------*/
//用户离开游戏桌
BYTE	CServerGameDesk::UserLeftDesk(BYTE bDeskStation, CGameUserInfo * pUserInfo)
{
    CString tem;
    tem.Format("WBJL::UserLeft = %d",bDeskStation);
    OutputDebugString(tem);
	if (m_TGameData.m_byNtPeople == bDeskStation)
	{
		m_TGameData.m_byNtPeople = 255;
	}

	memset(m_TCalculateBoard,0,sizeof(m_TCalculateBoard));

	return __super::UserLeftDesk(bDeskStation,pUserInfo);
}
/*---------------------------------------------------------------------*/
//用户断线
bool	CServerGameDesk::UserNetCut(BYTE bDeskStation, CGameUserInfo * pUserInfo)
{	
	CString tem;
	tem.Format("WBJL::UserNetCut = %d",bDeskStation);
	OutputDebugString(tem);
	return __super::UserNetCut(bDeskStation,pUserInfo);
}

/*---------------------------------------------------------------------*/
//逃跑扣分
int		CServerGameDesk::GetRunPublish()
{
	return m_pDataManage->m_InitData.uRunPublish;
}
/*---------------------------------------------------------------------*/

BYTE CServerGameDesk::GetNextNoteStation(BYTE bCurDeskStation)
{
	BYTE bNextStation = bCurDeskStation;

	int kkk = PlayerCount() - 1;   //逆时针寻找下一个操作对象(过滤掉弃牌，状态错误，等待开牌的状态)
	for (int i = 0;i < PlayerCount();i++)
	{
		bNextStation = (bNextStation + kkk) % PlayerCount();
		if(!m_pUserInfo[bNextStation])
		{
			continue;
		}
		if (m_TGameData.m_iUserState[bNextStation] != STATE_GIVE_UP && m_TGameData.m_iUserState[bNextStation] != STATE_LOOK_GIVE_UP 
			&& m_TGameData.m_iUserState[bNextStation] != STATE_ERR 
			&& m_TGameData.m_iUserState[bNextStation] != STATE_WAITE_OPEN && STATE_LOST != m_TGameData.m_iUserState[bNextStation]
		    && STATE_LOOK_LOST != m_TGameData.m_iUserState[bNextStation]
			)
		{
			break;
		}

		if (bNextStation == bCurDeskStation)
		{
			break;
		}
	}

	return bNextStation;

}
/*---------------------------------------------------------------------*/
/*
* @return 返回最小金币玩家的金币,如果配置的输赢上限小于最少金币玩家的金币，则是返回输赢上限
*/
__int64		CServerGameDesk::GetMinMoney()
{
	__int64 Min = -1;
	for ( int i=0; i<PlayerCount(); i++ )
	{
		if ( NULL == m_pUserInfo[i] || m_TGameData.m_iUserState[i] == STATE_ERR)
		{
			continue;
		}
		if (Min == -1)
		{
			Min = m_pUserInfo[i]->m_UserData.i64Money;
		}
		else if (m_pUserInfo[i]->m_UserData.i64Money < Min)
		{
			Min = m_pUserInfo[i]->m_UserData.i64Money;
		}
	}

	if(m_bIsBuy)
	{
		//房卡场不根据玩家金币限制操作
		Min = LONG_MAX;
	}
	return Min;
}
/*---------------------------------------------------------------------*/
//是否需要封顶
bool	CServerGameDesk::bNeedFengDing()
{
	bool bFengDing = false;
	for(int i=0; i<PlayerCount(); i++)
	{
		if (m_TGameData.m_iUserState[i] == STATE_WAITE_OPEN)
		{
			bFengDing = true;
			break;
		}
	}
	return bFengDing;
}

/*---------------------------------------------------------------------*/
//验证是否超端
void	CServerGameDesk::SpuerProof(BYTE byDeskStation)
{
	//是超端用户 那么就发送超端消息过去
	if (IsSuperUser(byDeskStation))
	{
		S_C_SuperUserProof TSuperUser;
		TSuperUser.bIsSuper = true;
		TSuperUser.byDeskStation = byDeskStation;
		SendGameData(byDeskStation,&TSuperUser,sizeof(TSuperUser),MDM_GM_GAME_NOTIFY,S_C_SUPER_PROOF,0);
	}

}
/*---------------------------------------------------------------------*/
bool	CServerGameDesk::IsSuperUser(BYTE byDeskStation)
{
	if (m_pUserInfo[byDeskStation] != NULL)
	{
		for(int i=0; i< m_vclSuperUserID.size(); i++)
		{
			if (m_pUserInfo[byDeskStation]->m_UserData.dwUserID == m_vclSuperUserID.at(i))
			{
				return true;
			}
		}
	}
	return false;
}
/*---------------------------------------------------------------------*/
//交换两个人的牌
void	CServerGameDesk::Change2UserCard(BYTE byFirstDesk,BYTE bySecondDesk)
{
	BYTE byTmpCard[MAX_CARD_COUNT];
	memcpy(byTmpCard,m_TGameData.m_byUserCard[byFirstDesk],sizeof(byTmpCard));
	memcpy(m_TGameData.m_byUserCard[byFirstDesk],m_TGameData.m_byUserCard[bySecondDesk],sizeof(m_TGameData.m_byUserCard[byFirstDesk]));
	memcpy(m_TGameData.m_byUserCard[bySecondDesk],byTmpCard,sizeof(m_TGameData.m_byUserCard[bySecondDesk]));
}

/*-----------------------------------------------------------------------------------*/
///机器人输赢自动控制,
void	CServerGameDesk::AiWinAutoCtrl()
{
	//首先去判断 这一桌是否既有机器人又有真人 
	bool bHaveRobot = false;
	bool bHaveUser	= false;
	BYTE byRobotDesk = 255;		//随便找一个机器人的位置
	BYTE byUserDesk	= 255;		//随便找一个真实玩家的位置
	for(int i=0; i<PlayerCount(); i++)
	{
		if (NULL == m_pUserInfo[i])
		{
			continue;
		}

		if (STATE_ERR == m_TGameData.m_iUserState[i] && m_pUserInfo[i]->m_UserData.isVirtual > 0)
		{
			bHaveRobot = true;
			if (255 == byRobotDesk)
			{
				byRobotDesk = i;
			}
		}
		else if (STATE_ERR == m_TGameData.m_iUserState[i] && 0 == m_pUserInfo[i]->m_UserData.isVirtual)
		{
			bHaveUser = true;
			if (255 == byUserDesk)
			{
				byUserDesk = i;
			}
		}
	}

	//既有机器人又有真人  才去进行控制
	if ( bHaveUser && bHaveRobot)
	{
		bool bAIWin = false;
		srand((unsigned)GetTickCount());
		int iResult = rand()%100;

		if (0 >=G_i64AIHaveWinMoney )
		{
			//机器人赢的钱少于0 机器人必赢
			bAIWin = true;		
		}
		else if((0 < G_i64AIHaveWinMoney) && (G_i64AIHaveWinMoney < m_iAIWantWinMoneyA1))
		{
			// 机器人的赢钱在0-A1区域 并且概率符合m_iAIWinLuckyAtA1 机器人要赢
			if (iResult <=m_iAIWinLuckyAtA1)
			{
				bAIWin = true;		
			}
		}
		else if ((m_iAIWantWinMoneyA1 <= G_i64AIHaveWinMoney)  && (G_i64AIHaveWinMoney <m_iAIWantWinMoneyA2))
		{
			// 机器人的赢钱在A1-A2区域 并且概率符合m_iAIWinLuckyAtA2 机器人要赢
			if (iResult <=m_iAIWinLuckyAtA2)
			{
				bAIWin = true;	
			}
		}
		else if ((m_iAIWantWinMoneyA2 <= G_i64AIHaveWinMoney)  && (G_i64AIHaveWinMoney <m_iAIWantWinMoneyA3))
		{
			// 机器人的赢钱在A2-A3区域 并且概率符合m_iAIWinLuckyAtA3 机器人要赢
			if (iResult <=m_iAIWinLuckyAtA3)
			{
				bAIWin = true;
			}
		}
		else
		{
			// 机器人的赢钱超过A3区域 并且概率符合m_iAIWinLuckyAtA4 机器人要赢
			if (iResult <=G_i64AIHaveWinMoney)
			{
				bAIWin = true;
			}
		}


		BYTE byTmpMax = 255;
		//先找到最大牌的玩家 
		for (int i=0; i<PlayerCount(); i++)
		{
			if (STATE_ERR != m_TGameData.m_iUserState[i])
			{
				if (255 == byTmpMax)
				{
					byTmpMax = i;
					break;
				}
			}
		}
		if (byTmpMax != 255)
		{
			for (int i=0; i<PlayerCount(); i++)
			{
				if (i == byTmpMax)
				{
					continue;
				}
				if (STATE_ERR != m_TGameData.m_iUserState[i])
				{
					int IsWin = m_Logic.CompareCard(m_TGameData.m_byUserCard[byTmpMax],MAX_CARD_COUNT,m_TGameData.m_byUserCard[i],MAX_CARD_COUNT);
					//被比较着的牌大
					if (-1 == IsWin)
					{
						byTmpMax = i;
					}
				}
			}
		}

		//机器人要赢钱 那么就把最大的牌发给机器人 
		if (bAIWin)
		{
			OutputDebugString("hlsz::机器人要赢钱");
			if (255 != byTmpMax)
			{
				//如果最大牌是真人 就要换牌给机器人
				if ( 0 == m_pUserInfo[byTmpMax]->m_UserData.isVirtual )
				{
					OutputDebugString("hlsz::最大的牌在真人手上 所以换牌");
					Change2UserCard(byTmpMax,byRobotDesk);
				}
				else
				{
					OutputDebugString("hlsz::最大的牌本来就在机器人手上 所以不用换牌");
				}
			}
		}
		else
		{
			OutputDebugString("hlsz::机器人要输钱");
			//机器人要输钱
			if (255 != byTmpMax)
			{
				//如果最大牌是机器人 就要换牌给真人
				if (m_pUserInfo[byTmpMax]->m_UserData.isVirtual > 0 )
				{
					OutputDebugString("hlsz::最大的牌本在机器人手上 所以要换牌");
					Change2UserCard(byTmpMax,byUserDesk);
				}
				else
				{
					OutputDebugString("hlsz::最大的牌本来就在真人手上 所以不用换牌");
				}
			}
		}
	}
}

/*-----------------------------------------------------------------------------------*/
void CServerGameDesk::RecordAiHaveWinMoney(__int64 i64UserScore[])
{
	try
	{
		CString s = CINIFile::GetAppPath ();/////本地路径
		TCHAR skin[MAX_PATH];
		CString sTemp = _T("");
		sTemp.Format("%s%s_s.ini",s,GET_SKIN_FOLDER(skin));

		CINIFile f(sTemp);
		CString szSec("Game");
		//统一房间
		//szSec.Format("%d_%d",NAME_ID,m_pDataManage->m_InitData.uRoomID);

		__int64 iReSetAIHaveWinMoney;
		iReSetAIHaveWinMoney = f.GetKeyVal(szSec,"ReSetAIHaveWinMoney ",(__int64)0);
		if (G_i64ReSetAIHaveWinMoney != iReSetAIHaveWinMoney)
		{
			//如果不相等，说明手动修改了配置文件中的值（让机器人吞钱了），相等就去累加机器人赢得钱;
			G_i64AIHaveWinMoney = iReSetAIHaveWinMoney;
			G_i64ReSetAIHaveWinMoney = iReSetAIHaveWinMoney;
		}
		else
		{
			for(int i=0;i<PlayerCount();i++)
			{
				if(NULL != m_pUserInfo[i] && m_pUserInfo[i]->m_UserData.isVirtual == 0)
				{
					G_i64AIHaveWinMoney -= i64UserScore[i];
				}
			}
		}
		//将当前机器人赢的钱写入配置文件当中
		sTemp.Format("%I64d",G_i64AIHaveWinMoney);
		f.SetKeyValString(szSec,"AIHaveWinMoney ",sTemp);

	}
	catch (...)
	{
		//这里只是做个防错措施，什么也不处理
		OutputDebugString("dwjlog::写配置文件出错了!");
	}			

}

// 获得当前正在运行的游戏定时器的剩余时间
int CServerGameDesk::GetGameRemainTime()
{
	long lTime = m_iLastTimerTime - (GetTickCount() - m_dwLastGameStartTime);
	if (lTime < 0)
	{
		return false;
	}
	return ((int)lTime);
}

//mark
//能否下注
bool CServerGameDesk::CanXiZhu()
{
	int i64XiaZhuNum = 0;
	//查询当前是否有足够的钱下注,如果不够,则默认为放弃状态
	if (m_TGameData.m_iUserState[m_TGameData.m_byCurrHandleDesk] == STATE_LOOK)
	{
		//钱够了就翻倍
		i64XiaZhuNum = m_TGameData.m_i64CurrZhuBase*2;
	}
	else
	{
		i64XiaZhuNum = m_TGameData.m_i64CurrZhuBase;
	}

	__int64 iUserMoney = m_pUserInfo[m_TGameData.m_byCurrHandleDesk]->m_UserData.i64Money;//获取玩家当前的金币
	if(m_bIsBuy)
	{
		//房卡场不根据玩家金币限制操作
		iUserMoney = LONG_MAX;
	}
	__int64 iXiaZhuMoney = m_TGameData.m_i64XiaZhuData[m_TGameData.m_byCurrHandleDesk] + i64XiaZhuNum;//获取玩家当前至少下注多少

	if (iUserMoney < iXiaZhuMoney)
	{
		return false;
	}
	else
	{
		return true;
	}
}

//获取玩家当前的金币
__int64 CServerGameDesk::GetUserMoney()
{
	if(!m_bIsBuy)
	{
		return m_pUserInfo[m_TGameData.m_byCurrHandleDesk]->m_UserData.i64Money;
	}
	else
	{
		//房卡场不根据玩家金币限制操作
		return LONG_MAX;
	}
}

void CServerGameDesk::UpdateCalculateBoard(bool dismiss)
{
	if (m_bIsBuy)
	{
		if(m_iVipGameCount <= m_iRunGameCount || (dismiss && m_iRunGameCount > 0))
		{
			int iBigWinnerStation = 0;	
			vector<int> iContinueWinCount;
			vector<int> temp;


			for (int i = 0; i < PlayerCount(); i++)
			{
				temp.clear();
				for(int j = 0; j < m_vecWinInfo[i].GetCount();j++)
				{
					temp.push_back(m_vecWinInfo[i].GetAt(j));
				}

				//最高连胜
				iContinueWinCount.clear();
				for(auto ite = temp.begin(); ite != temp.end(); ite++)
				{
					//第一个赢，插入1
					if(ite == temp.begin())
					{
						if(*ite == true)
						{
							iContinueWinCount.push_back(1);
						}									
					}
					//有两连续的，说明是连胜，连胜加1
					else if(*(ite - 1) == true && *ite == true)
					{
						iContinueWinCount.at(iContinueWinCount.size() - 1) += 1;
					}
					//前一个不赢这个赢，新插入一个1
					else if(*ite == true)
					{
						iContinueWinCount.push_back(1);
					}
				}

				for(auto ite = iContinueWinCount.begin(); ite != iContinueWinCount.end(); ite++)
				{
					if(m_TCalculateBoard[i].iMaxContinueCount < *ite)
					{
						m_TCalculateBoard[i].iMaxContinueCount = *ite;
					}
				}
				//大赢家
				m_TCalculateBoard[i].bWinner = false;
				if(m_TCalculateBoard[i].i64WinMoney >= m_TCalculateBoard[iBigWinnerStation].i64WinMoney)
				{
					iBigWinnerStation = i;
				}
			}
			m_TCalculateBoard[iBigWinnerStation].bWinner = true;

			for (int i = 0; i < PlayerCount(); i++)
			{
				if (m_pUserInfo[i] == NULL)
				{
					continue;
				}
				SendGameData(i,m_TCalculateBoard,sizeof(m_TCalculateBoard),MDM_GM_GAME_NOTIFY,S_C_UPDATE_CALCULATE_BOARD_SIG,0);
			}

			//m_TGameData.InitAllData();
            m_TGameData.m_byNtPeople = 255;
            m_TGameData.InitSomeData();
		}
	}
}

int CServerGameDesk::PlayerCount() const 
{
	int num = PLAY_COUNT;
// 	if(m_bIsBuy)
// 	{
// 		num = m_PeopleNum;
// 	}
	return num;
}

int CServerGameDesk::DismissRoom()
{
	UpdateCalculateBoard(true);
	m_bGameStation=GS_WAIT_ARGEE;
	ReSetGameState(GF_SALE);
	//m_TGameData.InitAllData();
    m_TGameData.m_byNtPeople = 255;
    m_TGameData.InitSomeData();
	return 0;
}
