#include "StdAfx.h"
#include "ServerManage.h"
#include "Shlwapi.h"

//Moved by QiWang 20180226, form .h
static __int64					G_iAIHaveWinMoney;	//机器人赢钱数20121126	dwj 这里声明为静态全局变量，因为多个桌子需要及时修改该变量
static __int64					G_iReSetAIHaveWinMoney;	/**<机器人输赢控制：重置机器人已经赢钱的数目  该变量也要声明为静态全局变量 否则每个桌子都会去重置*/

//构造函数
CServerGameDesk::CServerGameDesk(void):CGameDesk(ALL_ARGEE)
{
	m_bGameStation=GS_WAIT_ARGEE;//游戏状态

	m_iAllCardCount		= 52;
	m_byUpGradePeople	= 255;	
	m_iFirstJiao		= 255;
	m_byCurrOperateUser = 255;
	m_iLimit			= 0;
	m_iNTBase			= 1;
	memset(m_iTotalCard,0,sizeof(m_iTotalCard));
	m_SuperSetData.Init();
	memset(m_i64UserNoteLimite,0,sizeof(m_i64UserNoteLimite));
	memset(m_iUserCard,0,sizeof(m_iUserCard));
	memset(m_byOpenUnderCard,0,sizeof(m_byOpenUnderCard));
	memset(m_byOpenUpCard,0,sizeof(m_byOpenUpCard));
	memset(m_bBullCard,0,sizeof(m_bBullCard));
	
	for(int i=0; i<PLAY_COUNT; i++)
	{
		m_bUserReady[i]			= false;
		m_iPerJuTotalNote[i]	= 0;
		m_iLimitNote[i]			= 0;
		m_byUserStation[i]		= STATE_NULL;
		m_iUserCardCount[i]		= 0;
		m_iCallScore[i]			= -1;
		m_iOpenShape[i]			= UG_NO_POINT;
		m_byOpenUnderCount[i]	= 0;
		m_iUserBase[i]			= 0; // Changed by QiWang 20171011, 初始化为0，便于客户端显示
	}

	m_bKickPerson = false;
	m_bTimerCheckSet = false;
	memset(m_byNoReadyTimeArr, 0, sizeof(m_byNoReadyTimeArr));

	//Added by QiWang 20180226
	m_iBasePoint = 1;
}
//初始化
bool CServerGameDesk::InitDeskGameStation()
{
	//加载配置文件
	LoadIni();
	//重新加载配置文件里面的
	LoadExtIni(m_pDataManage->m_InitData.uRoomID);
	memset(m_TCalculateBoard,0,sizeof(m_TCalculateBoard));

	return true;
}
//加载ini配置文件
BOOL	CServerGameDesk::LoadIni()
{
	CString s = CINIFile::GetAppPath ();/////本地路径
	CINIFile f( s +SKIN_FOLDER +"_s.ini");
	CString key = TEXT("game");
	m_iBeginTime	= f.GetKeyVal(key,"begintime",30);
	if(m_iBeginTime<10)
	{
		m_iBeginTime=10;
	}
	m_iThinkTime	= f.GetKeyVal(key,"thinktime",20);
	if(m_iThinkTime<10)
	{
		m_iThinkTime=10;
	}
	m_iCallScoreTime= f.GetKeyVal(key,"CallScoretime",15);
	if(m_iCallScoreTime<10)
	{
		m_iCallScoreTime=10;
	}
	m_iXiaZhuTime	= f.GetKeyVal(key,"XiaZhutime",15);
	if(m_iXiaZhuTime<10)
	{
		m_iXiaZhuTime=10;
	}

	//Added by QiWang 20180226,由游戏设置底分，DB中改为1
	m_iBasePoint   = f.GetKeyVal(key,"iBasePoint",1);
	if(m_iBasePoint<1)
	{
		m_iBasePoint=1;
	}

	//Added by QiWang 20171106, T人相关
	m_bKickPerson = f.GetKeyVal(key,"bKickPerson",0);
	m_iRobNTTime    = f.GetKeyVal(key,"RobNTtime",5);
	if(m_iRobNTTime<5)
	{
		m_iRobNTTime=5;
	}
	m_iDoubleTime   = f.GetKeyVal(key,"Doubletime",5);
	if(m_iDoubleTime<5)
	{
		m_iDoubleTime=5;
	}
	m_iGoodCard		= f.GetKeyVal(key,"iGoodCard",5);				//mark 是否好牌
	m_bHaveKing		= f.GetKeyVal(key,"HaveKing",0);				//是否有王
	m_iLimit		= f.GetKeyVal(key,"LimitNote",0);				//最大注，配为小于或=0是就自动以玩家身上钱有注
	m_iSendCardTime = f.GetKeyVal(key,"Sendcardtime",120);
	m_bTurnRule		= f.GetKeyVal(key,"TurnRule",0);
	m_iCardShape &=0x00000000;
	m_iCardShape |= (f.GetKeyVal(key,"Yn",0)&0xFFFFFFFF);//银牛
	m_iCardShape |= ((f.GetKeyVal(key,"Jn",0)<<1)&0xFFFFFFFF);//金牛
	m_iCardShape |= ((f.GetKeyVal(key,"Bomb",0)<<2)&0xFFFFFFFF);//炸弹
	m_iCardShape |= ((f.GetKeyVal(key,"Five",0)<<3)&0xFFFFFFFF);//五小
	m_Logic.SetCardShape(m_iCardShape);

	//牌型赔率配置
	CString keyName;
	int base = 0;
	for (int i=0; i<MAX_SHAPE_COUNT; i++)
	{
		keyName.Format("CardShape%d",i);
		if (i < 6)
		{
			base = 1;
		}
		else
		{
			base = i - 5;
		}
		m_bCardShapeBase[i] = f.GetKeyVal(key,keyName,base);
		if(m_bCardShapeBase[i]<1)
		{
			m_bCardShapeBase[i]=1;
		}
	}

	//超端配置
	key = TEXT("SuperSet");
	int iSuperCount = f.GetKeyVal(key, "SuperUserCount", 0);
	long  lUserID = 0;
	CString strText;
	m_vlSuperID.clear();
	for (int j=0; j<iSuperCount; j++)
	{
		strText.Format("SuperUserID[%d]", j);
		lUserID = f.GetKeyVal(key, strText, 0);
		//将所有超端ID 保存到超端容器中
		m_vlSuperID.push_back(lUserID);
	}
	return true;
}
//根据房间ID加载配置文件
BOOL CServerGameDesk::LoadExtIni(int iRoomID)
{

	//TCHAR szKey[20];
	//wsprintf(szKey,"%s_%d",SKIN_FOLDER,iRoomID);
	//CString s = CINIFile::GetAppPath ();/////本地路径
	//CINIFile f( s +SKIN_FOLDER +"_s.ini");
	//CString key = szKey;
	//Eil @ 20190325 统一房间的读取数值
	CString s = CINIFile::GetAppPath ();/////本地路径
	CINIFile f( s +SKIN_FOLDER +"_s.ini");
	CString key = TEXT("game");
	m_iBeginTime	= f.GetKeyVal(key,"begintime",m_iBeginTime);
	if(m_iBeginTime<10)
	{
		m_iBeginTime=10;
	}
	m_iThinkTime	= f.GetKeyVal(key,"thinktime",m_iThinkTime);
	if(m_iThinkTime<10)
	{
		m_iThinkTime=10;
	}
	m_iCallScoreTime= f.GetKeyVal(key,"CallScoretime",m_iCallScoreTime);
	if(m_iCallScoreTime<10)
	{
		m_iCallScoreTime=10;
	}
	m_iXiaZhuTime	= f.GetKeyVal(key,"XiaZhutime",m_iXiaZhuTime);
	if(m_iXiaZhuTime<10)
	{
		m_iXiaZhuTime=10;
	}
	m_iRobNTTime    = f.GetKeyVal(key,"RobNTtime",m_iRobNTTime);
	if(m_iRobNTTime<5)
	{
		m_iRobNTTime=5;
	}
	m_iDoubleTime   = f.GetKeyVal(key,"Doubletime",m_iDoubleTime);
	if(m_iDoubleTime<5)
	{
		m_iDoubleTime=5;
	}
	m_iGoodCard        = f.GetKeyVal(key,"iGoodCard",m_iGoodCard);//mark
	m_bHaveKing        = f.GetKeyVal(key,"HaveKing",m_bHaveKing);//是否有王
	m_iLimit           = f.GetKeyVal(key,"LimitNote",m_iLimit);//最大注，配为小于或=0是就自动以玩家身上钱有注
	//牌型赔率配置
	CString keyName;
	int base = 0;
	for (int i=0; i<MAX_SHAPE_COUNT; i++)
	{
		keyName.Format("CardShape%d",i);
		if (i < 6)
		{
			base = 1;
		}
		else
		{
			base = i - 5;
		}
		base = f.GetKeyVal(key,keyName,m_bCardShapeBase[i]);
		m_bCardShapeBase[i] = base;
		if(m_bCardShapeBase[i]<1)
		{
			m_bCardShapeBase[i]=1;
		}
	}
	//奖池配置
	m_iAIWantWinMoneyA1	= f.GetKeyVal(key,"AIWantWinMoneyA1 ",__int64(500000));		/**<机器人赢钱区域1  */
	m_iAIWantWinMoneyA2	= f.GetKeyVal(key,"AIWantWinMoneyA2 ",__int64(5000000));	/**<机器人赢钱区域2  */
	m_iAIWantWinMoneyA3	= f.GetKeyVal(key,"AIWantWinMoneyA3 ",__int64(50000000));	/**<机器人赢钱区域3  */
	m_iAIWinLuckyAtA1	= f.GetKeyVal(key,"AIWinLuckyAtA1 ",60);				/**<机器人在区域1赢钱的概率  */
	m_iAIWinLuckyAtA2	= f.GetKeyVal(key,"AIWinLuckyAtA2 ",60);				/**<机器人输赢控制：机器人在区域2赢钱的概率  */
	m_iAIWinLuckyAtA3	= f.GetKeyVal(key,"AIWinLuckyAtA3 ",60);				/**<机器人输赢控制：机器人在区域3赢钱的概率  */
	m_iAIWinLuckyAtA4	= f.GetKeyVal(key,"AIWinLuckyAtA4 ",60);				/**<机器人输赢控制：机器人在区域4赢钱的概率  */
	G_iAIHaveWinMoney	= f.GetKeyVal(key,"AIHaveWinMoney ",__int64(0));	/**<机器人输赢控制：直接从配置文件中读取机器人已经赢钱的数目  */
	m_bAIWinAndLostAutoCtrl = f.GetKeyVal(key,"AIWinAndLostAutoCtrl",0);		//机器人输赢控制20121122dwj
	G_iReSetAIHaveWinMoney	= f.GetKeyVal(key,"ReSetAIHaveWinMoney ",__int64(0)); //记录重置机器人赢钱数，如果游戏过程中改变了就要改变机器人赢钱数
	
	return TRUE;
}

//-------------------------------add-by-DWJ-20121113-----------------------------------------------------------------------------------------
//从配置文件中读取机器人已经赢了多少钱，因为在每盘结算的时候都会将机器人赢钱结算写到配置文件中。 
//这里在每盘开始的时候又读取下，因为如果玩家修改了那么就机器人吞钱了.
void CServerGameDesk::GetAIContrlSetFromIni(int iRoomID)
{
	/*
	CString s = CINIFile::GetAppPath ();/////本地路径
	CINIFile f(s +SKIN_FOLDER  + _T("_s.ini"));
	CString szSec = TEXT("game");
	szSec.Format("%s_%d",SKIN_FOLDER,iRoomID);
	*/
	//Eil @ 20190325 统一房间的读取数值
	CString s = CINIFile::GetAppPath ();/////本地路径
	CINIFile f( s +SKIN_FOLDER +"_s.ini");
	CString key = TEXT("game");
	m_iAIWantWinMoneyA1	= f.GetKeyVal(key,"AIWantWinMoneyA1 ",m_iAIWantWinMoneyA1);		/**<机器人赢钱区域1  */
	m_iAIWantWinMoneyA2	= f.GetKeyVal(key,"AIWantWinMoneyA2 ",m_iAIWantWinMoneyA2);		/**<机器人赢钱区域2  */
	m_iAIWantWinMoneyA3	= f.GetKeyVal(key,"AIWantWinMoneyA3 ",m_iAIWantWinMoneyA3);		/**<机器人赢钱区域3  */
	m_iAIWinLuckyAtA1	= f.GetKeyVal(key,"AIWinLuckyAtA1 ",m_iAIWinLuckyAtA1);			/**<机器人在区域1赢钱的概率  */
	m_iAIWinLuckyAtA2	= f.GetKeyVal(key,"AIWinLuckyAtA2 ",m_iAIWinLuckyAtA2);			/**<机器人输赢控制：机器人在区域2赢钱的概率  */
	m_iAIWinLuckyAtA3	= f.GetKeyVal(key,"AIWinLuckyAtA3 ",m_iAIWinLuckyAtA3);			/**<机器人输赢控制：机器人在区域3赢钱的概率  */
	m_iAIWinLuckyAtA4	= f.GetKeyVal(key,"AIWinLuckyAtA4 ",m_iAIWinLuckyAtA4);			/**<机器人输赢控制：机器人在区域4赢钱的概率  */

	m_bAIWinAndLostAutoCtrl = f.GetKeyVal(key,"AIWinAndLostAutoCtrl",m_bAIWinAndLostAutoCtrl);//机器人输赢控制20121122dwj
	G_iAIHaveWinMoney	= f.GetKeyVal(key,"AIHaveWinMoney ",G_iAIHaveWinMoney);
}

//-------------------------------------------------------------------------------------------------------------------------------------------


//析构函数
CServerGameDesk::~CServerGameDesk(void)
{
}




//定时器消息
bool CServerGameDesk::OnTimer(UINT uTimerID)
{
	switch(uTimerID)
	{
	case TIME_GAME_BEGIN:
		{
			KillTimer(TIME_GAME_BEGIN);
			SendCard();
		}
	case TIME_CALL_ROBNT:	//抢庄
		{
			KillTimer(TIME_CALL_ROBNT);
			if(!m_bIsBuy)
			{
				if (m_bGameStation == GS_ROB_NT)
				{
					for (int i=0; i<PLAY_COUNT; i++)
					{
						if (m_pUserInfo[i] != NULL && m_byUserStation[i] == STATE_PLAY_GAME)
						{
							UserRobNT(i, 0);
						}
					}
				}
			}
			return TRUE;
		}
		//mark
	case TIME_DOUBLE:	//加倍计时器
		{
			KillTimer(TIME_DOUBLE);
			if(!m_bIsBuy)
			{
				if (m_bGameStation == GS_DOUBLE)
				{
					for (int i=0; i<PLAY_COUNT; i++)
					{
						if (m_pUserInfo[i] != NULL && m_byUserStation[i] == STATE_HAVE_ROBNT && i!= m_byUpGradePeople)
						{
							tagUserProcess userBet;
							userBet.iNoteType = 0;
							userBet.iVrebType = TYPE_NOTE;				
							//下注类型
							UserDoubleResult(i,userBet.iVrebType,userBet.iNoteType);
						}
					}
				}
				SetTimer(TIME_DOUBLE_FINISH,100);
			}
			return TRUE;
		}
	case TIME_DOUBLE_FINISH:	//加倍完成定时器
		{
			KillTimer(TIME_DOUBLE_FINISH);
			if (m_bGameStation==GS_DOUBLE) 
			{
				BeginOpenCard();
			}
			return TRUE;
		}
	case TIME_SEND_CARD_FINISH:	//发牌定时器
		{
			KillTimer(TIME_SEND_CARD_FINISH);
			if (m_bGameStation == GS_SEND_CARD)
			{
				SendCardFinish();
			}
			return TRUE;
		}
	case TIME_AUTO_BULL:	//自动摆牛计时器//mark
		{
			KillTimer(TIME_AUTO_BULL);
			// 摆牛前断线（如庄家在其他玩家没下注前断线），系统自动帮其摆牛，
			if(!m_bIsBuy)
			{
				for (BYTE i=0; i<PLAY_COUNT; i++)
				{
					if (NULL!=m_pUserInfo[i] && m_byUserStation[i] != STATE_NULL && m_byUserStation[i] != STATE_OPEN_CARD)
					{
						GetBullCard(i);

						UserTanPai TUserTanPai;
						TUserTanPai.byUnderCount = 3;
						memcpy(TUserTanPai.byUnderCard,m_bBullCard[i],sizeof(TUserTanPai.byUnderCard));

						UserOpenCard(i,&TUserTanPai,false);
					}
				}
			}
			return TRUE;
		}
	case TIME_GAME_FINISH:	//结束定时器
		{
			KillTimer(TIME_GAME_FINISH);
			if (m_bGameStation==GS_OPEN_CARD ) 
			{
				GameFinish(0,GF_NORMAL);
			}
			return TRUE;
		}
	case TIME_CHECK_READY://普通场T人相关, Added by QiWang 20171106
		{
			if (!m_bIsBuy && m_bKickPerson 
				&& (GS_WAIT_NEXT == m_bGameStation|| GS_WAIT_ARGEE == m_bGameStation || GS_WAIT_SETGAME == m_bGameStation)
				&& !m_pDataManage->IsQueueGameRoom()) 
			{
				bool bKillTimer = true;
				for (int i=0; i<PLAY_COUNT; i++)
				{
					if (NULL == m_pUserInfo[i] || m_bUserReady[i])
					{
						m_byNoReadyTimeArr[i] = 0;
						continue;
					}

					if ((m_byNoReadyTimeArr[i]++) >= m_iBeginTime)//多加1S，是因为计时器是公共计时器
					{
						m_byNoReadyTimeArr[i] = 0;
						UserLeftDesk(i,m_pUserInfo[i]); // T掉
					} else
					{
						bKillTimer = false;
					}
				}

				if (bKillTimer)
				{
					KillTimer(TIME_CHECK_READY);
					m_bTimerCheckSet = false;
					memset(m_byNoReadyTimeArr, 0, sizeof(m_byNoReadyTimeArr));
				}
			} else 
			{
				KillTimer(TIME_CHECK_READY);
				m_bTimerCheckSet = false;
				memset(m_byNoReadyTimeArr, 0, sizeof(m_byNoReadyTimeArr));
			}
			return TRUE;
		}
	}
	return __super::OnTimer(uTimerID);
}

//框架消息处理函数
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
				iResult = fsr.GetKeyVal (_T("ForceQuitAsAuto"), cKey, 0);
			}

			if (iResult == 0)
			{
				return __super::HandleFrameMessage( bDeskStation,  pNetHead,  pData,  uSize,  uSocketID,  bWatchUser); 
			}
			return true;
		}
	}
	OutputDebugString("err::HandleFrameMessage(1)");
	return __super::HandleFrameMessage( bDeskStation,  pNetHead,  pData,  uSize,  uSocketID,  bWatchUser);
}

//游戏数据包处理函数
bool CServerGameDesk::HandleNotifyMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	if (m_pUserInfo[bDeskStation] == NULL)
	{
		return true;
	}
	switch (pNetHead->bAssistantID)
	{
		case ASS_SUPER_USER_SET:	//超端玩家设置消息
			{
				if(bWatchUser)
				{
					return true;
				}
				SuperUserSetData * pSuperSet= (SuperUserSetData*)pData;
				if(NULL==pSuperSet)
				{
					return true;
				}

				if (IsSuperUser(bDeskStation))
				{
					memcpy(&m_SuperSetData, pSuperSet, sizeof(m_SuperSetData));
					m_SuperSetData.bSetSuccese = true;

					//安全判断
					if (m_SuperSetData.byMaxDesk >=0 && m_SuperSetData.byMaxDesk < PLAY_COUNT )
					{
						if (NULL == m_pUserInfo[m_SuperSetData.byMaxDesk])
						{
							m_SuperSetData.bSetSuccese = false;
						}
					}
					if (m_SuperSetData.byMinDesk >=0 && m_SuperSetData.byMinDesk < PLAY_COUNT )
					{
						if (NULL == m_pUserInfo[m_SuperSetData.byMinDesk])
						{
							m_SuperSetData.bSetSuccese = false;
						}
					}

					SendGameData(bDeskStation,&m_SuperSetData,sizeof(m_SuperSetData),MDM_GM_GAME_NOTIFY,ASS_SUPER_USER_SET_RESULT,0);
				}
				return true;
			}

		case ASS_GM_AGREE_GAME:		//用户同意游戏
			{
				if (bWatchUser)
					return true;
				
				if (GS_WAIT_SETGAME != m_bGameStation
					&& GS_WAIT_ARGEE != m_bGameStation
					&& GS_WAIT_NEXT != m_bGameStation)
				{
					return true;
				}

				if(m_bUserReady[bDeskStation])
				{
					return true; 
				}

				m_bUserReady[bDeskStation] = true ; 

				__super::HandleNotifyMessage(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
				return true;
			}
		case ASS_USER_OPEN:		//用户摆牛
			{
				if(bWatchUser||(uSize!=sizeof(UserTanPai)))
				{
					return true;
				}

				if (m_bGameStation != GS_OPEN_CARD)
				{
					return true;
				}

				if (m_byUserStation[bDeskStation] == STATE_NULL
					|| STATE_OPEN_CARD == m_byUserStation[bDeskStation])
				{
					return true;
				}

				UserOpenCard(bDeskStation,pData,true);
				return true;
			}
		case ASS_USER_ROBNT:	//玩家抢庄
			{
				if ((bWatchUser)||(uSize!=sizeof(RobNTStruct))) 
					return true;
				if (m_bGameStation != GS_ROB_NT)
				{
					return true;
				}
				if (m_byUserStation[bDeskStation] == STATE_NULL)
				{
					return true;
				}
				RobNTStruct * pRobNT=(RobNTStruct *)pData;
				if (NULL == pRobNT)
				{
					return true;
				}
				
				UserRobNT(bDeskStation,pRobNT->iValue);
				return true;
			}
		case ASS_USER_DOUBLE:	//玩家加倍
			{
				OutputDebugString("err::ASS_USER_DOUBLE(0)");
				if(bWatchUser||(uSize!=sizeof(tagUserProcess)))
					return true;

				if (m_bGameStation != GS_DOUBLE)
				{
					return true;
				}

				if (m_byUpGradePeople == bDeskStation
					|| m_byUserStation[bDeskStation] == STATE_HAVE_DOUBLE
					|| m_byUserStation[bDeskStation] == STATE_NULL)
				{
					return true;
				}
				tagUserProcess *pVerb = (tagUserProcess *) pData;
				if (NULL == pVerb)
				{
					return true;
				}
				OutputDebugString("err::ASS_USER_DOUBLE(1)");
				return UserDoubleResult(bDeskStation,pVerb->iVrebType,pVerb->iNoteType);

				return true;
			}
	}
	__super::HandleNotifyMessage(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
	return true;
}

//获取游戏状态信息
bool CServerGameDesk::OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser)
{
	if(m_bIsBuy)
	{
		int iLeaveJuShu = m_iVipGameCount - m_iRunGameCount;
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (m_pUserInfo[i] == NULL)
			{
				continue;
			}
			SendGameData(i,&iLeaveJuShu,sizeof(int),MDM_GM_GAME_NOTIFY,S_C_UPDATE_REMAIN_JUSHU_SIG,0);
		}
	}
	time_t CurTime = time(NULL);
	// 验证超级用户权限
	SuperUserExamine(bDeskStation);
	CString llog;
	llog.Format("lbtest::::::::::::::::::::bDeskStation = %d:::::: iWinCount = %d",bDeskStation,m_TCalculateBoard[bDeskStation].iWinCount);
	OutputDebugString(llog);
	switch (m_bGameStation)
	{
	case GS_WAIT_SETGAME:		//游戏没有开始状态
	case GS_WAIT_ARGEE:			//等待玩家开始状态
	case GS_WAIT_NEXT:		//等待下一盘游戏开始
		{
			int iUserCount = 0;
			for (int i = 0; i < PLAY_COUNT; i++ )
			{
				iUserCount += (m_pUserInfo[i] != NULL);
			}
			if (iUserCount == 1)
			{
				ReSetGameState(GFF_FORCE_FINISH);
			}

			GameStation_WaiteAgree TGameStation;
			::memset(&TGameStation,0,sizeof(TGameStation));
			//游戏版本核对
			TGameStation.iVersion=DEV_HEIGHT_VERSION;			//游戏版本号
			TGameStation.iVersion2=DEV_LOW_VERSION;
	
			//游戏基础信息
			TGameStation.byGameStation	= m_bGameStation;		//游戏状态
			TGameStation.iThinkTime		= m_iThinkTime;			//摊牌时间
			TGameStation.iBeginTime		= m_iBeginTime;			//准备时间
			TGameStation.iSendCardTime	= m_iSendCardTime;		//发牌时间
			TGameStation.iCallScoreTime = m_iCallScoreTime;		//叫庄时间
			TGameStation.iXiaZhuTime	= m_iXiaZhuTime;		//下注时间
			TGameStation.iAllCardCount	= m_iAllCardCount;		//扑克数目
			TGameStation.iRobNTTime     = m_iRobNTTime;         //抢庄时间
			TGameStation.iDoubleTime    = m_iDoubleTime;		//加倍时间
			TGameStation.iRoomBasePoint = GetRoomBasePoint();	//房间倍数
			TGameStation.iBaseNote		= m_iBasePoint;			//底分
			TGameStation.iCardShape		= m_iCardShape;			//牌型设置

			//玩家是否已准备
			memcpy(TGameStation.bUserReady ,m_bUserReady , sizeof(TGameStation.bUserReady )) ;
			//发送数据
			SendGameStation(bDeskStation,uSocketID,bWatchUser,&TGameStation,sizeof(TGameStation));

			if (!m_bIsBuy && m_bKickPerson)
			{
				if (!m_bTimerCheckSet)
				{
					m_bTimerCheckSet = true;
					SetTimer(TIME_CHECK_READY,NUM_ONE_SECOND_MS);
				}

				if (bDeskStation < PLAY_COUNT)
				{
					m_byNoReadyTimeArr[bDeskStation] = 0;
				}
			}

			return TRUE;
		}
	case GS_ROB_NT:		//抢庄状态
		{
			GameStation_RobNt TGameStation;
			::memset(&TGameStation,0,sizeof(TGameStation));
			//游戏版本核对
			TGameStation.iVersion=DEV_HEIGHT_VERSION;			//游戏版本号
			TGameStation.iVersion2=DEV_LOW_VERSION;

			//游戏基础信息
			TGameStation.byGameStation	= m_bGameStation;		//游戏状态
			TGameStation.iThinkTime		= m_iThinkTime;			//摊牌时间
			TGameStation.iBeginTime		= m_iBeginTime;			//准备时间
			TGameStation.iSendCardTime	= m_iSendCardTime;		//发牌时间
			TGameStation.iCallScoreTime = m_iCallScoreTime;		//叫庄时间
			TGameStation.iXiaZhuTime	= m_iXiaZhuTime;		//下注时间
			TGameStation.iAllCardCount	= m_iAllCardCount;		//扑克数目
			TGameStation.iRobNTTime     = m_iRobNTTime;         //抢庄时间
			TGameStation.iDoubleTime    = m_iDoubleTime;		//加倍时间
			TGameStation.iRoomBasePoint = GetRoomBasePoint();	//房间倍数
			TGameStation.iBaseNote		= m_iBasePoint;			//底分
			TGameStation.iCardShape		= m_iCardShape;			//牌型设置

			
			TGameStation.byCurrentRobNT	= 255;					//当前抢庄者	 

			TGameStation.iStartPos		= m_iFirstJiao;			//发牌位置

			memcpy(TGameStation.iUserCard,m_iUserCard,sizeof(TGameStation.iUserCard));

			for (int i=0; i<PLAY_COUNT; i++)
			{
				//用户手上扑克数目
				TGameStation.iUserCardCount[i]		= m_iUserCardCount[i];			
				//各玩家抢庄情况	255-表示还没操作 0-表示不抢 1-表示已经抢了
				TGameStation.byUserRobNT[i] = m_iCallScore[i];
				//个玩家的状态 标记是否中途加入的
				TGameStation.iUserStation[i] = m_byUserStation[i];
			}

			TGameStation.byRemainTime = ((CurTime - tRobNTTime) <= m_iRobNTTime 
										? m_iRobNTTime - (CurTime - tRobNTTime) : 0);	//剩余时间

			//发送数据
			SendGameStation(bDeskStation,uSocketID,bWatchUser,&TGameStation,sizeof(TGameStation));
			return TRUE;
		}
	case GS_NOTE:		//下注状态
		{

			GameStation_Note TGameStation;
			//游戏版本核对
			TGameStation.iVersion=DEV_HEIGHT_VERSION;			//游戏版本号
			TGameStation.iVersion2=DEV_LOW_VERSION;

			//游戏基础信息
			TGameStation.byGameStation	= m_bGameStation;		//游戏状态
			TGameStation.iThinkTime		= m_iThinkTime;			//摊牌时间
			TGameStation.iBeginTime		= m_iBeginTime;			//准备时间
			TGameStation.iSendCardTime	= m_iSendCardTime;		//发牌时间
			TGameStation.iCallScoreTime = m_iCallScoreTime;		//叫庄时间
			TGameStation.iXiaZhuTime	= m_iXiaZhuTime;		//下注时间
			TGameStation.iAllCardCount	= m_iAllCardCount;		//扑克数目
			TGameStation.iRobNTTime     = m_iRobNTTime;         //抢庄时间
			TGameStation.iDoubleTime    = m_iDoubleTime;		//加倍时间
			TGameStation.iRoomBasePoint = GetRoomBasePoint();	//房间倍数
			TGameStation.iBaseNote		= m_iBasePoint;			//底分
			TGameStation.iCardShape		= m_iCardShape;			//牌型设置
			
			TGameStation.byNtStation	= m_byUpGradePeople;	//庄家位置

	
			memcpy(TGameStation.i64UserNoteLimite,m_i64UserNoteLimite,sizeof(TGameStation.i64UserNoteLimite));
			for(int i=0;i<PLAY_COUNT;i++)
			{
				TGameStation.iUserStation[i]		= m_byUserStation[i];			//各玩家状态
				TGameStation.i64PerJuTotalNote[i]	= m_iPerJuTotalNote[i];			//用户每下注情况
				TGameStation.iLimitNote[i]			= m_iLimitNote[i];				//最大注数
			}
			//发送数据
			SendGameStation(bDeskStation,uSocketID,bWatchUser,&TGameStation,sizeof(TGameStation));

			return TRUE;
		}
	case GS_DOUBLE:		//加倍状态
		{

			GameStation_Double TGameStation;
			//游戏版本核对
			TGameStation.iVersion=DEV_HEIGHT_VERSION;			//游戏版本号
			TGameStation.iVersion2=DEV_LOW_VERSION;

			//游戏基础信息
			TGameStation.byGameStation	= m_bGameStation;		//游戏状态
			TGameStation.iThinkTime		= m_iThinkTime;			//摊牌时间
			TGameStation.iBeginTime		= m_iBeginTime;			//准备时间
			TGameStation.iSendCardTime	= m_iSendCardTime;		//发牌时间
			TGameStation.iCallScoreTime = m_iCallScoreTime;		//叫庄时间
			TGameStation.iXiaZhuTime	= m_iXiaZhuTime;		//下注时间
			TGameStation.iAllCardCount	= m_iAllCardCount;		//扑克数目
			TGameStation.iRobNTTime     = m_iRobNTTime;         //抢庄时间
			TGameStation.iDoubleTime    = m_iDoubleTime;		//加倍时间
			TGameStation.iRoomBasePoint = GetRoomBasePoint();	//房间倍数
			TGameStation.iBaseNote		= m_iBasePoint;			//底分
			TGameStation.iCardShape		= m_iCardShape;			//牌型设置

			TGameStation.iStartPos		= m_byUpGradePeople;	//庄家位置


			//用户手上的扑克
			memcpy(TGameStation.iUserCard,m_iUserCard,sizeof(TGameStation.iUserCard));

			for(int i=0;i<PLAY_COUNT;i++)
			{
				TGameStation.iUserCardCount[i]		= m_iUserCardCount[i];			//用户手上扑克数目
				TGameStation.iUserStation[i]		= m_byUserStation[i];			//各玩家状态
				TGameStation.iUserBase[i]			= m_iUserBase[i];				//用户加倍情况
			}

			if (m_byUpGradePeople < PLAY_COUNT)
			{
				TGameStation.iUserBase[m_byUpGradePeople] = m_iNTBase;				//庄家倍率
			}

			TGameStation.byRemainTime = ((CurTime - tDoubleTime) <= m_iDoubleTime
										? m_iDoubleTime - (CurTime - tDoubleTime) : 0);	//剩余时间

			//发送数据
			SendGameStation(bDeskStation,uSocketID,bWatchUser,&TGameStation,sizeof(TGameStation));

			return TRUE;
		}
	case GS_SEND_CARD:	
		{

			GameStation_SendCard TGameStation;
			//游戏版本核对
			TGameStation.iVersion=DEV_HEIGHT_VERSION;			//游戏版本号
			TGameStation.iVersion2=DEV_LOW_VERSION;

			//游戏基础信息
			TGameStation.byGameStation	= m_bGameStation;		//游戏状态
			TGameStation.iThinkTime		= m_iThinkTime;			//摊牌时间
			TGameStation.iBeginTime		= m_iBeginTime;			//准备时间
			TGameStation.iSendCardTime	= m_iSendCardTime;		//发牌时间
			TGameStation.iCallScoreTime = m_iCallScoreTime;		//叫庄时间
			TGameStation.iXiaZhuTime	= m_iXiaZhuTime;		//下注时间
			TGameStation.iAllCardCount	= m_iAllCardCount;		//扑克数目
			TGameStation.iRobNTTime     = m_iRobNTTime;         //抢庄时间
			TGameStation.iDoubleTime    = m_iDoubleTime;		//加倍时间
			TGameStation.iRoomBasePoint = GetRoomBasePoint();	//房间倍数
			TGameStation.iBaseNote		= m_iBasePoint;			//底分
			TGameStation.iCardShape		= m_iCardShape;			//牌型设置

			TGameStation.iStartPos		= m_iFirstJiao;			//发牌位置

			//用户手上的扑克
			memcpy(TGameStation.iUserCard,m_iUserCard,sizeof(TGameStation.iUserCard));

			for(int i=0; i<PLAY_COUNT; i++)
			{
				TGameStation.iUserCardCount[i]		= m_iUserCardCount[i];			//用户手上扑克数目
				TGameStation.i64PerJuTotalNote[i]	= m_iPerJuTotalNote[i];			//用户每下注情况
				//每个玩家的状态 标记是否中途加入的
				TGameStation.iUserStation[i] = m_byUserStation[i];
			}
	
			//发送数据
			SendGameStation(bDeskStation,uSocketID,bWatchUser,&TGameStation,sizeof(TGameStation));
			return TRUE;
		}
	case GS_OPEN_CARD:	//游戏中状态
		{

			GameStation_OpenCard TGameStation;
			//游戏版本核对
			TGameStation.iVersion=DEV_HEIGHT_VERSION;			//游戏版本号
			TGameStation.iVersion2=DEV_LOW_VERSION;

			//游戏基础信息
			TGameStation.byGameStation	= m_bGameStation;		//游戏状态
			TGameStation.iThinkTime		= m_iThinkTime;			//摊牌时间
			TGameStation.iBeginTime		= m_iBeginTime;			//准备时间
			TGameStation.iSendCardTime	= m_iSendCardTime;		//发牌时间
			TGameStation.iCallScoreTime = m_iCallScoreTime;		//叫庄时间
			TGameStation.iXiaZhuTime	= m_iXiaZhuTime;		//下注时间
			TGameStation.iAllCardCount	= m_iAllCardCount;		//扑克数目
			TGameStation.iRobNTTime     = m_iRobNTTime;         //抢庄时间
			TGameStation.iDoubleTime    = m_iDoubleTime;		//加倍时间
			TGameStation.iRoomBasePoint = GetRoomBasePoint();	//房间倍数
			TGameStation.iBaseNote		= m_iBasePoint;			//底分
			TGameStation.iCardShape		= m_iCardShape;			//牌型设置

			TGameStation.byNtStation	= m_byUpGradePeople;	//庄家位置
			

			//用户手中牌
			::memcpy(TGameStation.iUserCardCount,m_iUserCardCount,sizeof(TGameStation.iUserCardCount));
			::memcpy(TGameStation.iUserCard,m_iUserCard,sizeof(TGameStation.iUserCard));
			//当前各用户下注总数
			::memcpy(TGameStation.i64PerJuTotalNote,m_iPerJuTotalNote,sizeof(m_iPerJuTotalNote));
			for(int i=0; i<PLAY_COUNT; i++)
			{
				//每个玩家的状态 标记是否中途加入的
				TGameStation.iUserStation[i] = m_byUserStation[i];
				TGameStation.iUserBase[i] = m_iUserBase[i];				//用户加倍情况
			}
			//memcpy(TGameStation.iUserStation, m_byUserStation, sizeof(TGameStation.iUserStation));
			memcpy(TGameStation.byOpenUnderCount, m_byOpenUnderCount, sizeof(TGameStation.byOpenUnderCount));
			memcpy(TGameStation.iOpenShape, m_iOpenShape, sizeof(TGameStation.iOpenShape));
			memcpy(TGameStation.byOpenUnderCard,m_byOpenUnderCard,sizeof(TGameStation.byOpenUnderCard));
			memcpy(TGameStation.byOpenUpCard,m_byOpenUpCard,sizeof(TGameStation.byOpenUpCard));

			if (m_byUpGradePeople < PLAY_COUNT)
			{
				TGameStation.iUserBase[m_byUpGradePeople] = m_iNTBase;				//庄家倍率
			}
			
			TGameStation.byRemainTime = ((CurTime - tShowCardTime) <= m_iThinkTime
										? m_iThinkTime - (CurTime - tShowCardTime) : 0);//剩余时间

			//发送数据
			SendGameStation(bDeskStation,uSocketID,bWatchUser,&TGameStation,sizeof(TGameStation));

			return TRUE;
		}
	}
	return false;
}

//重置游戏状态
bool CServerGameDesk::ReSetGameState(BYTE bLastStation)
{
	KillAllTimer();
	m_byUpGradePeople	= 255;	
	m_iFirstJiao		= 255;
	m_byCurrOperateUser = 255;
	m_iLimit            = 0;
	m_iNTBase			= 1;
	memset(m_iTotalCard,0,sizeof(m_iTotalCard));
	memset(m_i64UserNoteLimite,0,sizeof(m_i64UserNoteLimite));
	memset(m_iUserCard,0,sizeof(m_iUserCard));
	memset(m_byOpenUnderCard,0,sizeof(m_byOpenUnderCard));
	memset(m_byOpenUpCard,0,sizeof(m_byOpenUpCard));
	memset(m_bBullCard,0,sizeof(m_bBullCard));

	for(int i=0; i<PLAY_COUNT; i++)
	{
		m_bUserReady[i]			= false;
		m_iPerJuTotalNote[i]	= 0;
		m_iLimitNote[i]			= 0;
		m_byUserStation[i]		= STATE_NULL;
		m_iUserCardCount[i]		= 0;
		m_iCallScore[i]			= -1;
		m_iOpenShape[i]			= UG_NO_POINT;
		m_byOpenUnderCount[i]	= 0;
		m_iUserBase[i]			= 0; //Changed by QiWang 20171011, 初始为0，便于客户端显示
	}
	return TRUE;
}

/*---------------------------------------------------------------------------------*/
//游戏开始
bool	CServerGameDesk::GameBegin(BYTE bBeginFlag)
{
	if (__super::GameBegin(bBeginFlag)==false) 
	{
		GameFinish(0,GF_SALE);
		return false;
	}

	//Added by QiWang 20171106, T人相关
	if (!m_bIsBuy && m_bKickPerson)
	{
		KillTimer(TIME_CHECK_READY);
		m_bTimerCheckSet = false;
		memset(m_byNoReadyTimeArr, 0, sizeof(m_byNoReadyTimeArr));
	}
	//游戏状态
	m_bGameStation = GS_SEND_CARD;				
	if (0 == m_iRunGameCount)
	{
		for(int i = 0; i < PLAY_COUNT; i ++)
		{
			m_vecWinInfo[i].RemoveAll();
		}
		memset(m_TCalculateBoard,0,sizeof(m_TCalculateBoard));
	}
	//memset(m_bUserReady,0,sizeof(m_bUserReady));
	//这里每盘根据房间ID读取下配置文件中的ReSetAIHaveWinMoney 参数 设定下机器人赢了多少钱
	GetAIContrlSetFromIni(m_pDataManage->m_InitData.uRoomID);

	memset(m_bUserReady,0,sizeof(m_bUserReady));
	memset(m_iUserCardCount,0,sizeof(m_iUserCardCount));		//用户手中牌数
	for (int i = 0; i < PLAY_COUNT; i++) 
	{
		m_byUserStation[i] = STATE_NULL;
		m_iPerJuTotalNote[i] = 0;
		if (m_pUserInfo[i])
		{
			m_byUserStation[i] = STATE_PLAY_GAME;
		}
	}
	//分发扑克
	m_Logic.RandCard(m_iTotalCard,m_iAllCardCount,m_bHaveKing);


	int temp = rand()%100;
	//洗到好牌
	if (temp < m_iGoodCard)
	{
		for (int i=0; i<20; i++)
		{
			if (GetMaxCardShape(m_iTotalCard,m_iAllCardCount) > UG_BULL_SEVEN)
			{
				break;
			}
			m_Logic.RandCard(m_iTotalCard,m_iAllCardCount,m_bHaveKing);
		}
	}
	
	srand(GetTickCount());
	if (m_iFirstJiao != 255)
	{
		m_iFirstJiao = GetNextDeskStation(m_iFirstJiao);
	}
	else
	{
		m_iFirstJiao = GetNextDeskStation(rand()%PLAY_COUNT);
	}
	
	for (int i=0;i<PLAY_COUNT;i++) 
	{
		if (NULL != m_pUserInfo[i])
		{
			SendGameData(i,NULL,0,MDM_GM_GAME_NOTIFY,ASS_GAME_BEGIN,0);
		}
	}
	
	SetTimer(TIME_GAME_BEGIN,3000);

	return true;
}
/*---------------------------------------------------------------------------------*/
//通知用户下注
BOOL	CServerGameDesk::NoticeUserNote()
{
	m_bGameStation = GS_NOTE;

	//清空相关数据
	memset(m_i64UserNoteLimite,0,sizeof(m_i64UserNoteLimite));
	for (int i=0; i<PLAY_COUNT; i++)
	{
		if (i==m_byUpGradePeople || !m_pUserInfo[i])
		{
			continue;
		}
		m_iLimitNote[i] = GetLimit(i);

		//四个下注值
		for (int m=0; m<4; m++)
		{
			m_i64UserNoteLimite[i][m] = m_iLimitNote[i]/(m+1);
			if(m_i64UserNoteLimite[i][m] <= 1)
			{
				m_i64UserNoteLimite[i][m] = 1;
			}
		}
	}


	BeginUpgradeStruct TBeginMessage;
	memset(&TBeginMessage,0,sizeof(TBeginMessage));
	TBeginMessage.iNt			= m_byUpGradePeople;	
	TBeginMessage.AllCardCount	= m_iAllCardCount;
	::CopyMemory(TBeginMessage.iLimitNote,m_iLimitNote,sizeof(m_iLimitNote));
	memcpy(TBeginMessage.i64UserNoteLimite,m_i64UserNoteLimite,sizeof(TBeginMessage.i64UserNoteLimite));
	 
	for (int i=0;i<PLAY_COUNT;i++) 
	{
		if (NULL == m_pUserInfo[i] || m_byUserStation[i] ==STATE_NULL)
		{
			continue;
		}
		SendGameData(i,&TBeginMessage,sizeof(TBeginMessage),MDM_GM_GAME_NOTIFY,ASS_CALL_NOTE,0);	
	}
	SendWatchData(m_bMaxPeople,&TBeginMessage,sizeof(TBeginMessage),MDM_GM_GAME_NOTIFY,ASS_CALL_NOTE,0);


	SetTimer(TIME_NOTE,(m_iXiaZhuTime+2)*NUM_ONE_SECOND_MS);
	

	return TRUE;
}
/*---------------------------------------------------------------------------------*/
//通知用户加倍
BOOL	CServerGameDesk::NoticeUserDouble()
{
	m_bGameStation = GS_DOUBLE;

	UserDoubleStruct TUserDouble;
	memset(&TUserDouble,0,sizeof(TUserDouble));
	TUserDouble.iNt				= m_byUpGradePeople;	
	TUserDouble.AllCardCount	= m_iAllCardCount;

	for (int i=0;i<PLAY_COUNT;i++) 
	{
		if (NULL == m_pUserInfo[i] || m_byUserStation[i] ==STATE_NULL)
		{
			continue;
		}
		SendGameData(i,&TUserDouble,sizeof(TUserDouble),MDM_GM_GAME_NOTIFY,ASS_CALL_DOUBLE,0);	
	}
	SendWatchData(m_bMaxPeople,&TUserDouble,sizeof(TUserDouble),MDM_GM_GAME_NOTIFY,ASS_CALL_DOUBLE,0);


	SetTimer(TIME_DOUBLE,(m_iDoubleTime+2)*NUM_ONE_SECOND_MS);
	tDoubleTime = time(NULL);

	return TRUE;
}
/*---------------------------------------------------------------------------------*/
//mark
//用户下注
BOOL	CServerGameDesk::UserNoteResult(BYTE bDeskStation, BYTE iVerbType,int iNoteType)
{
	if (m_iPerJuTotalNote[bDeskStation]!=0 || m_byUserStation[bDeskStation] != STATE_PLAY_GAME)
	{
		return true;
	}
	if (iNoteType <0 || iNoteType >3)
	{
		return true ;
	}
	
	m_iPerJuTotalNote[bDeskStation] = m_i64UserNoteLimite[bDeskStation][iNoteType];
	//标记该玩家已经下注了
	m_byUserStation[bDeskStation] = STATE_HAVE_NOTE;

	NoteResult Tnoteresult;					
	Tnoteresult.bAddStyle	= iVerbType;					//下注类型
	Tnoteresult.iOutPeople	= bDeskStation;					//本次下注者
	Tnoteresult.iCurNote		= m_i64UserNoteLimite[bDeskStation][iNoteType];
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if(m_pUserInfo[i])
		{
			SendGameData(i,&Tnoteresult,sizeof(Tnoteresult),MDM_GM_GAME_NOTIFY,ASS_CALL_NOTE_RESULT,0);
		}		
	}
	SendWatchData(m_bMaxPeople,&Tnoteresult,sizeof(Tnoteresult),MDM_GM_GAME_NOTIFY,ASS_CALL_NOTE_RESULT,0);

	//发牌消息
	if (CountNoNotePeople() == 1)
	{
		KillTimer(TIME_NOTE);
		//20121122 dwj 机器人控制输赢了且 没有超端控制;
		if (m_bAIWinAndLostAutoCtrl && !m_SuperSetData.bSetSuccese)
		{
			IAWinAutoCtrl();
		}

		SetTimer(TIME_NOTE_FINISH,500);
		return TRUE;
	}
	return TRUE;

}
/*---------------------------------------------------------------------------------*/
//用户加倍
BOOL	CServerGameDesk::UserDoubleResult(BYTE bDeskStation, BYTE iVerbType,int iNoteType)
{
	if (m_byUserStation[bDeskStation] != STATE_HAVE_ROBNT)
	{
		return true;
	}
	if (iNoteType <1 || iNoteType >3)
	{
		iNoteType = 1;
	}

	m_iUserBase[bDeskStation]		= iNoteType;
	//标记该玩家已经加倍了
	m_byUserStation[bDeskStation] = STATE_HAVE_DOUBLE;

	DoubleResult TDoubleresult;					
	TDoubleresult.bDoubleStyle  	= iVerbType;			//加倍类型
	TDoubleresult.iDoublePeople	= bDeskStation;				//本次加倍者
	TDoubleresult.iCurBase	    = iNoteType;				//加倍数
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if(m_pUserInfo[i])
		{
			SendGameData(i,&TDoubleresult,sizeof(TDoubleresult),MDM_GM_GAME_NOTIFY,ASS_CALL_DOUBLE_RESULT,0);
		}		
	}
	SendWatchData(m_bMaxPeople,&TDoubleresult,sizeof(TDoubleresult),MDM_GM_GAME_NOTIFY,ASS_CALL_DOUBLE_RESULT,0);

	//发牌消息
	if (CountNoNotePeople() == 1)
	{
		KillTimer(TIME_DOUBLE);
		//20121122 dwj 机器人控制输赢了且 没有超端控制;
		if (m_bAIWinAndLostAutoCtrl && !m_SuperSetData.bSetSuccese)
		{
			IAWinAutoCtrl();
		}

		SetTimer(TIME_DOUBLE_FINISH,500);
		return TRUE;
	}
	return TRUE;
}
/*---------------------------------------------------------------------------------*/
//发送扑克给用户
BOOL	CServerGameDesk::SendCard()
{
	m_bGameStation = GS_SEND_CARD;


	//发扑克的地方
	//分发扑克牌
	for(int j = 0 ; j<SH_USER_CARD ;j++)
	{
		for(int i = 0 ;i<PLAY_COUNT;i++)
		{
			if( NULL == m_pUserInfo[i] || m_byUserStation[i] == STATE_NULL)
			{
				m_iUserCardCount[i] = 0;
				memset(&m_iUserCard[i],0,sizeof(m_iUserCard[i]));
			}
			else
			{
				m_iUserCardCount[i] = SH_USER_CARD;
				m_iUserCard[i][j] =  m_iTotalCard[i*SH_USER_CARD+j];	
			}			
		}
	}

	//todo 将决定发牌权放在这里
	//if(this->m_bAIWinAndLostAutoCtrl)
	//{
	//this->IAWinAutoCtrl_pro();
	//}



	//

	//如果超端设置了 就要换牌
	if (m_SuperSetData.bSetSuccese)
	{
		//最大的
		if (m_SuperSetData.byMaxDesk != 255)
		{
			if (STATE_NULL != m_byUserStation[m_SuperSetData.byMaxDesk] && m_iUserCardCount[m_SuperSetData.byMaxDesk] == SH_USER_CARD)
			{
				for(int i=0; i<PLAY_COUNT; i++)
				{
					if (i != m_SuperSetData.byMaxDesk && STATE_NULL != m_byUserStation[i] && m_iUserCardCount[i] == SH_USER_CARD)
					{
						//判断大小
						if( m_Logic.CompareCard(m_iUserCard[i], SH_USER_CARD, m_iUserCard[m_SuperSetData.byMaxDesk],SH_USER_CARD) == 1)
						{
							ChangeTwoUserCard(i,m_SuperSetData.byMaxDesk);
						}
					}
				}
			}
		}

		//最小的
		if (m_SuperSetData.byMinDesk != 255)
		{
			if (STATE_NULL != m_byUserStation[m_SuperSetData.byMinDesk] && m_iUserCardCount[m_SuperSetData.byMinDesk] == SH_USER_CARD)
			{
				for(int i=0; i<PLAY_COUNT; i++)
				{
					if (i != m_SuperSetData.byMinDesk && STATE_NULL != m_byUserStation[i] && m_iUserCardCount[i] == SH_USER_CARD)
					{
						//判断大小
						if( m_Logic.CompareCard(m_iUserCard[i], SH_USER_CARD, m_iUserCard[m_SuperSetData.byMinDesk],SH_USER_CARD) != 1)
						{
							ChangeCard(i,m_SuperSetData.byMaxDesk);
						}
					}
				}
			}
		}
	}
	//设置成功后只生效一局
	m_SuperSetData.Init();

	SendAllCardStruct TSendAllCard;
	TSendAllCard.iStartPos = m_iFirstJiao;
	memcpy(TSendAllCard.iUserCardCount,m_iUserCardCount,sizeof(TSendAllCard.iUserCardCount));
	memcpy(TSendAllCard.iUserCard,m_iUserCard,sizeof(TSendAllCard.iUserCard));	


	for(int i=0;i<PLAY_COUNT;i++)
	{
		if(m_pUserInfo[i])
		{
			SendGameData(i,&TSendAllCard,sizeof(TSendAllCard),MDM_GM_GAME_NOTIFY,ASS_CALL_SEND_CARD,0);
		}
	}	
	SendWatchData(m_bMaxPeople,&TSendAllCard,sizeof(TSendAllCard),MDM_GM_GAME_NOTIFY,ASS_CALL_SEND_CARD,0);

	///发牌时间
	SetTimer(TIME_SEND_CARD_FINISH,(CountPlayer()*(m_iSendCardTime*SH_USER_CARD))+NUM_ONE_SECOND_MS);

	return TRUE;
}
/*---------------------------------------------------------------------------------*/
//发牌结束
BOOL	CServerGameDesk::SendCardFinish()
{
	for (int i=0;i<PLAY_COUNT;i++) 
	{
		if(m_pUserInfo[i])
		{
			SendGameData(i,MDM_GM_GAME_NOTIFY,ASS_CALL_SEND_FINISH,0);
		}
	}
	SendWatchData(m_bMaxPeople,MDM_GM_GAME_NOTIFY,ASS_CALL_SEND_FINISH,0);

	SendRobNT(m_iFirstJiao);
	return true;
}
/*---------------------------------------------------------------------------------*/
//游戏开始
BOOL	 CServerGameDesk::BeginOpenCard()
{
	//设置游戏状态
	m_bGameStation=GS_OPEN_CARD;

	//发送游戏开始消息
	BeginPlayStruct TBegin;
	for (int i=0;i<PLAY_COUNT;i++)
	{
		SendGameData(i,&TBegin,sizeof(TBegin),MDM_GM_GAME_NOTIFY,ASS_CALL_OPEN,0);
	}
	SendWatchData(m_bMaxPeople,&TBegin,sizeof(TBegin),MDM_GM_GAME_NOTIFY,ASS_CALL_OPEN,0);

	// 倒计时摆牛
	SetTimer(TIME_AUTO_BULL, (m_iThinkTime+2)*NUM_ONE_SECOND_MS);
	tShowCardTime=time(NULL);
	return TRUE;
}
/*---------------------------------------------------------------------------------*/
//玩家摆牛
//bClient, set m_bBullCard if bClient = true
BOOL	CServerGameDesk::UserOpenCard(BYTE byDeskStation,void * pData,bool bClient)
{
	UserTanPai *pUserTanPai = (UserTanPai *) pData;
	if (NULL == pUserTanPai || byDeskStation >= PLAY_COUNT || STATE_OPEN_CARD == m_byUserStation[byDeskStation])
	{
		return TRUE;
	}

	//计算升起来的两张牌
	BYTE byUpCard[2];
	memset(byUpCard,255,sizeof(byUpCard));

	//	GetShape, then check Recv Message
	//	GetShape

	bool bFlagGetBullInfo = false;
	int iShape = UG_NO_POINT;

	m_byUserStation[byDeskStation] = STATE_OPEN_CARD;

	//	GetBullInfo
	//Set Bull Card
	if (bClient)
	{
		GetBullCard(byDeskStation);
	}

	//Set Up Card
	{
		BYTE byTmpCard[SH_USER_CARD];
		memcpy(byTmpCard,m_iUserCard[byDeskStation],sizeof(byTmpCard));

		int iCountSameUnderCard = 0;// Added by QiWang 20170821, 检查三张牛牌是否均在手牌中
		for (int i=0; i<SH_USER_CARD; i++)
		{
			for(int j=0;j<3;j++)
			{
				if (m_bBullCard[byDeskStation][j] == byTmpCard[i])
				{
					byTmpCard[i] = 255;
					iCountSameUnderCard ++;
					break;
				}
			}
		}

		int iIndex = 0;
		for (int i=0; i<SH_USER_CARD; i++)
		{
			if (255 != byTmpCard[i])
			{
				byUpCard[iIndex] = byTmpCard[i];
				iIndex++;
				if (iIndex >=2 )
				{
					break;
				}
			}
		}
	}

	iShape = m_Logic.GetShape(m_iUserCard[byDeskStation],SH_USER_CARD,m_bBullCard[byDeskStation]);
	//	check Recv Message
	if(bClient)
	{
		const int iShapeClient = pUserTanPai->iShape;
		if (iShapeClient == iShape && 3 == pUserTanPai->byUnderCount)
		{
			//	Check UnderCard&UpCard
			BYTE byTmpCard[SH_USER_CARD];
			memcpy(byTmpCard,m_iUserCard[byDeskStation],sizeof(byTmpCard));

			int iCountSameUnderCard = 0;// Added by QiWang 20170822, 检查三张底牌是否均在手牌中
			int iCountSameUpCard = 0;// Added by QiWang 20170822, 检查两张UpCard是否均在手牌中
			bool bFlagBreak = false;
			for (int i=0; i<SH_USER_CARD; i++)
			{
				bFlagBreak = false;
				if (iCountSameUnderCard < 3)
				{
					for(int j=0;j<3;j++)
					{
						if (pUserTanPai->byUnderCard[j] == byTmpCard[i])
						{
							byTmpCard[i] = 0xFF;
							iCountSameUnderCard ++;
							bFlagBreak = true;
							break;
						}
					}
				}

				if (bFlagBreak)
				{
					continue;// no check up card
				}

				if (iCountSameUpCard < 2)
				{
					for(int j=0;j<2;j++)
					{
						if (pUserTanPai->byUpCard[j] == byTmpCard[i])
						{
							byTmpCard[i] = 0xFF;
							iCountSameUpCard ++;
							break;
						}
					}
				}
			}

			if (SH_USER_CARD == iCountSameUnderCard + iCountSameUpCard)
			{
				const int iShapeFunctionFromClient = m_Logic.GetShape(m_iUserCard[byDeskStation],SH_USER_CARD,pUserTanPai->byUnderCard); 
				if (iShapeFunctionFromClient == iShapeClient)
				{
					//copy undercard & upCard
					memcpy(m_bBullCard[byDeskStation],pUserTanPai->byUnderCard,sizeof(m_bBullCard[byDeskStation]));
					memcpy(byUpCard,pUserTanPai->byUpCard,sizeof(pUserTanPai->byUpCard));
				}
			}
		}
	}

	UserTanPai TUserTanPaiResult;

	TUserTanPaiResult.iShape = iShape;
	TUserTanPaiResult.byDeskStation = byDeskStation;

	//升起的两张牌
	memcpy(TUserTanPaiResult.byUpCard,byUpCard,sizeof(TUserTanPaiResult.byUpCard));

	//底牌牛牌张数和数据
	TUserTanPaiResult.byUnderCount = pUserTanPai->byUnderCount;
	memcpy(TUserTanPaiResult.byUnderCard,pUserTanPai->byUnderCard,sizeof(TUserTanPaiResult.byUnderCard));

	//标识该玩家已经摆牛了
	for(int i = 0;i < PLAY_COUNT; ++i)
	{
		TUserTanPaiResult.bReadyOpenCard[i] = (m_byUserStation[byDeskStation] == STATE_OPEN_CARD?true:false);
	}
	



	//这里记录玩家摆牛信息(断线重连的时候需要用到这些数据)
	//摆牛牌型
	m_iOpenShape[byDeskStation] = iShape;
	//底牌张数
	m_byOpenUnderCount[byDeskStation] = TUserTanPaiResult.byUnderCount;		
	//底牌的三张牌
	memcpy(m_byOpenUnderCard[byDeskStation],TUserTanPaiResult.byUnderCard,sizeof(m_byOpenUnderCard[byDeskStation]));
	//升起来的2张牌
	memcpy(m_byOpenUpCard[byDeskStation],TUserTanPaiResult.byUpCard,sizeof(m_byOpenUpCard[byDeskStation]));

	for(int i = 0;i < PLAY_COUNT; ++i)
	{
		if(m_pUserInfo[i])
		{
			SendGameData(i,&TUserTanPaiResult,sizeof(TUserTanPaiResult),MDM_GM_GAME_NOTIFY,ASS_CALL_OPEN_RESULT,0);
		}
	}
	SendWatchData(m_bMaxPeople,&TUserTanPaiResult,sizeof(TUserTanPaiResult),MDM_GM_GAME_NOTIFY,ASS_CALL_OPEN_RESULT,0);

	//检查是否都摆牛了
	if (CheckFinish())
	{
		KillTimer(TIME_AUTO_BULL);
		//结束
		SetTimer(TIME_GAME_FINISH,NUM_ONE_SECOND_MS);
	}
	return TRUE;
}
/*---------------------------------------------------------------------------------*/
//检测是否结束
BOOL CServerGameDesk::CheckFinish()
{
	int iOpenCount = 0;
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if(!m_pUserInfo[i])
		{
			continue;
		}
		if (m_byUserStation[i] == STATE_OPEN_CARD)
		{
			iOpenCount++;
		}
	}

	if(iOpenCount == CountPlayer())			
	{
		return TRUE;
	}
	return FALSE;
}
/*---------------------------------------------------------------------------------*/



//统计玩游戏玩家
BYTE CServerGameDesk::CountPlayer()
{
	BYTE count=0;
	for(int i=0;i<m_bMaxPeople;i++)
	{
		if(m_pUserInfo[i] && m_byUserStation[i] != STATE_NULL)
		{
			count++;
		}
	}
	return count;
}




BYTE CServerGameDesk::CountNoNotePeople()
{
	int iCount=0;
	for(int i=0;i<PLAY_COUNT;i++)
	{
		if(!m_pUserInfo[i])		
		{
			continue;
		}
		if(m_byUserStation[i] == STATE_HAVE_ROBNT)
		{
			iCount++;
		}
	}
	return iCount;
}

//Added by QiWang 20171106, 普通场T人相关
//Notice: 只在GameFinish中调用
void CServerGameDesk::SetTimerCheckReadyInFinish()
{
	if (!m_bIsBuy && m_bKickPerson)
	{
		m_bTimerCheckSet = true;
		SetTimer(TIME_CHECK_READY,NUM_ONE_SECOND_MS);
		memset(m_byNoReadyTimeArr, 0, sizeof(m_byNoReadyTimeArr));
	}
}

//Added by QiWang 20180302, 获得牛底牌
void CServerGameDesk::GetBullCard(BYTE byDeskStation)
{
	BYTE bResult[3];
	memset(bResult, 0, sizeof(bResult));
	if (m_Logic.GetBull(m_iUserCard[byDeskStation], SH_USER_CARD, bResult))
	{
		memcpy(m_bBullCard[byDeskStation], bResult, sizeof(bResult));
	}
	else
	{
		memset(m_bBullCard[byDeskStation], 0, sizeof(m_bBullCard[byDeskStation]));
		m_bBullCard[byDeskStation][0] = m_iUserCard[byDeskStation][0];
		m_bBullCard[byDeskStation][1] = m_iUserCard[byDeskStation][1];
		m_bBullCard[byDeskStation][2] = m_iUserCard[byDeskStation][2];
	}
}

//游戏结束
bool CServerGameDesk::GameFinish(BYTE bDeskStation, BYTE bCloseFlag)
{
	//得到最小数的金币
	long m_dwMaxPoint=0;
	bool bSetMaxMoney=false;
	KillTimer(TIME_AUTO_BULL);
	KillAllTimer();

	switch (bCloseFlag)
	{
	case GF_NORMAL:		//游戏正常结束
		{
			//设置数据 
			m_bGameStation=GS_WAIT_NEXT;

			//游戏结束
			GameEndStruct TGameEnd;
			::memset(&TGameEnd,0,sizeof(TGameEnd));
			TGameEnd.iUpGradeStation=m_byUpGradePeople;
			::CopyMemory(TGameEnd.iUpBullCard,&m_bBullCard,sizeof(m_bBullCard));
			::CopyMemory(TGameEnd.iCardList,&m_iUserCard,sizeof(m_iUserCard));
			//传给客户端当前牌型
			for (int i = 0;i < PLAY_COUNT;i++)
			{
				if (m_pUserInfo[i]==NULL || m_byUserStation[i] == STATE_NULL)//Changed by QiWang 20170922
					continue;
				TGameEnd.iUserState[i] = m_byUserStation[i];
				//牌型处理
				if(m_byUserStation[i]==STATE_OPEN_CARD)
				{
					TGameEnd.iCardShape[i] = m_iOpenShape[i];//m_Logic.GetShape(m_iUserCard[i],m_iUserCardCount[i],m_bBullCard[i]);
				}
			}
			__int64 iNtPoint = 0; //庄家输的钱（算上赢部分闲家）
			double  iNtLoseMoney = 0;//庄家输的钱。（不算赢部分闲家的）
			for (int i = 0;i < PLAY_COUNT;i++)
			{
				if (m_pUserInfo[i]==NULL || m_byUserStation[i] == STATE_NULL)//Changed by QiWang 20170922
					continue;
				if(i != m_byUpGradePeople)//得到各闲家的得分点
				{
					/*if(!m_bIsBuy)
					{
						TGameEnd.iTurePoint[i] = ComputePoint(i)*m_iPerJuTotalNote[i];
					}*/
					TGameEnd.iTurePoint[i] = m_iBasePoint*ComputePoint(i)*m_iUserBase[i]*m_iNTBase;
					//20100802 zht 同喜乐客户修改了输赢结算闲家不够钱
					if (!m_bIsBuy && TGameEnd.iTurePoint[i] + m_pUserInfo[i]->m_UserData.i64Money < 0)
					{
                         TGameEnd.iTurePoint[i] = -m_pUserInfo[i]->m_UserData.i64Money;    
					}
					if (TGameEnd.iTurePoint[i] > 0)
					{
						iNtLoseMoney+=TGameEnd.iTurePoint[i];
					}
					iNtPoint += TGameEnd.iTurePoint[i];
				}
			}
			//20100802 zht 同喜乐客户修改了输赢结算
			if (m_pUserInfo[m_byUpGradePeople] != NULL)
			{

				__int64 dwNtMoney = m_pUserInfo[m_byUpGradePeople]->m_UserData.i64Money;
				if (!m_bIsBuy && iNtPoint > dwNtMoney)//不够钱赔
				{

					for (int i = 0;i < PLAY_COUNT;i++)
					{
						if (m_pUserInfo[i]==NULL || m_byUserStation[i] == STATE_NULL) //Changed by QiWang 20170922
							continue;
						if(i != m_byUpGradePeople && TGameEnd.iTurePoint[i] > 0)//得到各闲家的得分点
						{
							TGameEnd.iTurePoint[i] = (dwNtMoney+(iNtLoseMoney-iNtPoint)) *(TGameEnd.iTurePoint[i]/iNtLoseMoney);
						}
					}
					TGameEnd.iTurePoint[m_byUpGradePeople] = -m_pUserInfo[m_byUpGradePeople]->m_UserData.i64Money; 

				}
				else
				{
					TGameEnd.iTurePoint[m_byUpGradePeople] = -iNtPoint;
				}
			}

			//写入数据库				
			int temp_point[PLAY_COUNT];
			bool temp_cut[PLAY_COUNT];
			memset(temp_cut,0,sizeof(temp_cut));
			for (int i=0;i<PLAY_COUNT;i++)
			{
				temp_point[i]=TGameEnd.iTurePoint[i];//+m_iAwardPoint[i];
				TGameEnd.iChangeMoney[i]=TGameEnd.iTurePoint[i];
				CString str;
			
			}
			//20121203dwj 记录机器人输赢值 在扣税之前记录 否则会出现机器人输赢不平衡的情况;
			RecordAiHaveWinMoney(&TGameEnd);

			ChangeUserPointint64(TGameEnd.iTurePoint, temp_cut);
			
			//全是机器人就不记录
			bool bHaveUser = false;
			for(int i=0; i<PLAY_COUNT; i++)
			{
				if (NULL == m_pUserInfo[i])
				{
					continue;
				}

				if (0 == m_pUserInfo[i]->m_UserData.isVirtual)
				{
					bHaveUser = true;
				}
			}

			if (bHaveUser)
			{
				__super::RecoderGameInfo(TGameEnd.iChangeMoney);
			}
			

			bool bNotCostPoint = (m_pDataManage->m_InitData.dwRoomRule & GRR_NOT_COST_POINT) > 0; /// 金币场不扣积分    add by wys 2010-11-1
			if (true == bNotCostPoint)
			{
				memset(TGameEnd.iTurePoint,0,sizeof(TGameEnd.iTurePoint));
			}
			//发送数据

			for (int i=0; i < PLAY_COUNT; i++) 
			{
				if (m_pUserInfo[i]!=NULL) 
				{
					SendGameData(i,&TGameEnd,sizeof(TGameEnd),MDM_GM_GAME_NOTIFY,ASS_CONTINUE_END,0);
				}
			}
			SendWatchData(m_bMaxPeople,&TGameEnd,sizeof(TGameEnd),MDM_GM_GAME_NOTIFY,ASS_CONTINUE_END,0);
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				m_TCalculateBoard[i].i64WinMoney += TGameEnd.iTurePoint[i];
			}
			CString llog;
			llog.Format("lbtest::::::::::::m_bIsBuy = %d",m_bIsBuy);
			OutputDebugString(llog);



			if (m_bIsBuy)
			{
				for (int i = 0; i < PLAY_COUNT; i++)
				{
					//最多赢钱数目
					if(m_TCalculateBoard[i].iMaxWinMoney <= TGameEnd.iChangeMoney[i])
					{
						m_TCalculateBoard[i].iMaxWinMoney = TGameEnd.iChangeMoney[i];
					}
					//胜利局数
					if(TGameEnd.iTurePoint[i] > 0)
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
				UpdateCalculateBoard();
			}
			ReSetGameState(bCloseFlag);
			__super::GameFinish(bDeskStation,bCloseFlag);

			int iLeaveJuShu = m_iVipGameCount - m_iRunGameCount;
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (m_pUserInfo[i] == NULL)
				{
					continue;
				}
				SendGameData(i,&iLeaveJuShu,sizeof(int),MDM_GM_GAME_NOTIFY,S_C_UPDATE_REMAIN_JUSHU_SIG,0);
			}

			SetTimerCheckReadyInFinish();

			return true;
		}
	case GF_SALE:			//游戏安全结束
	case GFF_SAFE_FINISH:
		{
			m_bGameStation = GS_WAIT_ARGEE;//GS_WAIT_SETGAME;
			/*for (int i = 0; i < m_bMaxPeople; i++)
			{
				if (m_pUserInfo[i] != NULL) 
					m_pUserInfo[i]->m_UserData.bUserState = USER_SITTING;
			}*/
			GameCutStruct CutEnd;
			::memset(&CutEnd,0,sizeof(CutEnd));
			CutEnd.bDeskStation=bDeskStation;
			for (int i = 0; i < m_bMaxPeople; i++)
				SendGameData(i,&CutEnd,sizeof(CutEnd),MDM_GM_GAME_NOTIFY,ASS_SALE_END,0);
			SendWatchData(m_bMaxPeople,&CutEnd,sizeof(CutEnd),MDM_GM_GAME_NOTIFY,ASS_SALE_END,0);
			bCloseFlag = GFF_SAFE_FINISH;

		
			ReSetGameState(bCloseFlag);
			__super::GameFinish(bDeskStation,bCloseFlag);
			OutputDebugString("err::GameFinish(1)");

			SetTimerCheckReadyInFinish();

			return true;

		}
	case GFF_FORCE_FINISH:		//用户断线离开
		{
			//设置数据
			m_bGameStation=GS_WAIT_ARGEE;//GS_WAIT_SETGAME;

			GameCutStruct CutEnd;
			memset(&CutEnd,0,sizeof(CutEnd));
			CutEnd.bDeskStation=bDeskStation;
			int total = 0;
			for (int i=0; i<PLAY_COUNT; i++)
			{
				total+=m_iPerJuTotalNote[i];
			}

			if (bDeskStation == m_byUpGradePeople)
			{
				CutEnd.iTurePoint[bDeskStation]= -total*5;;
			}
			else
			{
				CutEnd.iTurePoint[bDeskStation]= -m_iPerJuTotalNote[bDeskStation]*5;;//m_iTotalNote;			//扣除强退玩家全部下注
			}

			bool temp_cut[PLAY_COUNT];
			memset(temp_cut,0,sizeof(temp_cut));
			int i=0;
			for (i=0; i<PLAY_COUNT; ++i) 
			{ 
				temp_cut[i] = (bDeskStation == i)?true:false; 
			} 


			ChangeUserPointint64(CutEnd.iTurePoint, temp_cut);
			//__super::RecoderGameInfo(CutEnd.iChangeMoney);

			//全是机器人就不记录
			bool bHaveUser = false;
			for(int i=0; i<PLAY_COUNT; i++)
			{
				if (NULL == m_pUserInfo[i])
				{
					continue;
				}

				if (0 == m_pUserInfo[i]->m_UserData.isVirtual)
				{
					bHaveUser = true;
					break;
				}
			}

			if (bHaveUser)
			{
				__super::RecoderGameInfo(CutEnd.iChangeMoney);
			}
		
			bool bNotCostPoint = (m_pDataManage->m_InitData.dwRoomRule & GRR_NOT_COST_POINT) > 0; /// 金币场不扣积分    add by wys 2010-11-1
			if (true == bNotCostPoint)
			{
				memset(CutEnd.iTurePoint,0,sizeof(CutEnd.iTurePoint));
			}

			for (int i=0;i<m_bMaxPeople;i++)
				SendGameData(i,&CutEnd,sizeof(CutEnd),MDM_GM_GAME_NOTIFY,ASS_CUT_END,0);
			SendWatchData(m_bMaxPeople,&CutEnd,sizeof(CutEnd),MDM_GM_GAME_NOTIFY,ASS_CUT_END,0);

			bCloseFlag=GFF_FORCE_FINISH;


			ReSetGameState(bCloseFlag);
			__super::GameFinish(bDeskStation,bCloseFlag);
			OutputDebugString("err::GameFinish(1)");

			SetTimerCheckReadyInFinish();

			return true;
		}
	case GFF_DISSMISS_FINISH:
		{
			if (IsNeedSendBoard())
			{
				DissMissRoomBefore();
			}
			m_bGameStation = GS_WAIT_SETGAME;
			for(int i = 0; i < PLAY_COUNT; i ++)
			{
				m_vecWinInfo[i].RemoveAll();
			}
			memset(m_TCalculateBoard,0,sizeof(m_TCalculateBoard));
			CString llog;
			llog.Format("lbtest::::::::解散房间::::::::::::bDeskStation = %d:::::: iWinCount = %d",bDeskStation,m_TCalculateBoard[bDeskStation].iWinCount);
			OutputDebugString(llog);

			ReSetGameState(bCloseFlag);
			__super::GameFinish(bDeskStation,bCloseFlag);

			SetTimerCheckReadyInFinish();

			return true;

		}
	}

	//重置数据
	ReSetGameState(bCloseFlag);
	__super::GameFinish(bDeskStation,bCloseFlag);
	OutputDebugString("err::GameFinish(1)");

	SetTimerCheckReadyInFinish();

	return true;
}

void CServerGameDesk::UpdateCalculateBoard()
{
	CString llog;
	llog.Format("lbtest::::::::::::m_iVipGameCount = %d, m_iRunGameCount = %d",m_iVipGameCount , m_iRunGameCount);
	OutputDebugString(llog);
	if(m_iVipGameCount <= m_iRunGameCount)
	{
		int iBigWinnerStation = 0;	
		vector<int> iContinueWinCount;
		vector<int> temp;


		for (int i = 0; i < PLAY_COUNT; i++)
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
		if (m_TCalculateBoard[iBigWinnerStation].i64WinMoney > 0)
		{	
			m_TCalculateBoard[iBigWinnerStation].bWinner = true;
		}
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (m_pUserInfo[i] == NULL)
			{
				continue;
			}
			SendGameData(i,m_TCalculateBoard,sizeof(m_TCalculateBoard),MDM_GM_GAME_NOTIFY,S_C_UPDATE_CALCULATE_BOARD_SIG,0);
		}
	}

}

bool CServerGameDesk::UserNetCut(BYTE bDeskStation, CGameUserInfo *pLostUserInfo)
{
	if (bDeskStation < PLAY_COUNT)
	{
		m_bUserReady[bDeskStation] = false;
	}

	return __super:: UserNetCut(bDeskStation, pLostUserInfo);
}

//用户断线重来
bool CServerGameDesk::UserReCome(BYTE bDeskStation, CGameUserInfo * pNewUserInfo)
{
	
	return __super:: UserReCome(bDeskStation, pNewUserInfo);
}

void CServerGameDesk::DissMissRoomBefore()
{
	CString llog;
	llog.Format("lbtest::::::::::::m_iVipGameCount = %d, m_iRunGameCount = %d",m_iVipGameCount , m_iRunGameCount);
	OutputDebugString(llog);

	int iBigWinnerStation = 0;	
	vector<int> iContinueWinCount;
	vector<int> temp;


	for (int i = 0; i < PLAY_COUNT; i++)
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
	if (m_TCalculateBoard[iBigWinnerStation].i64WinMoney > 0)
	{	
		m_TCalculateBoard[iBigWinnerStation].bWinner = true;
	}
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] == NULL)
		{
			continue;
		}
		SendGameData(i,m_TCalculateBoard,sizeof(m_TCalculateBoard),MDM_GM_GAME_NOTIFY,S_C_UPDATE_CALCULATE_BOARD_SIG,0);
	}

}

void CServerGameDesk::ReleaseRoomBefore()
{
	GameInfForReplay inf[PLAY_COUNT];
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (!m_pUserInfo[i])
		{
			continue;
		}

		memcpy(&inf[i].TCalculateBoard, &m_TCalculateBoard[i], sizeof(m_TCalculateBoard[i]));
		for(int j = 0; j < m_vecWinInfo[i].GetCount() && j < MAX_GAME_TURNS;j++)
		{
			inf[i].byWinInfo[j]  = m_vecWinInfo[i].GetAt(j);
		}

		ReleaseRoom(m_pUserInfo[i]->m_UserData.dwUserID,&inf[i],sizeof(inf[i]));
	}
}

void CServerGameDesk::SetReturnGameInfo(int UserID,void* szUserGameInfo,int iSize,int iCount)
{
	if (NULL == szUserGameInfo)
	{
		return;
	}

	GameInfForReplay *pGameInf = (GameInfForReplay*)szUserGameInfo;

	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (m_pReturnInfo[i].iUserID == UserID)
		{
			memcpy(&m_TCalculateBoard[i], &pGameInf->TCalculateBoard, sizeof(m_TCalculateBoard[i]));

			for(int j = 0; j < MAX_GAME_TURNS;j++)
			{
				if (255 == pGameInf->byWinInfo[j])
				{
					break;
				}
				if (j < m_vecWinInfo[i].GetCount())
				{
					m_vecWinInfo[i].SetAt(j,pGameInf->byWinInfo[j]);
				} else 
				{
					m_vecWinInfo[i].Add(pGameInf->byWinInfo[j]);
				}
			}

			break;
		}	
	}
}

//判断是否正在游戏
bool CServerGameDesk::IsPlayGame(BYTE bDeskStation)
{	
	if (m_byUserStation[bDeskStation] == STATE_NULL)
	{
		return false;
	}
	if(GS_ROB_NT <= m_bGameStation && m_bGameStation <= GS_DOUBLE && GS_WAIT_NEXT != m_bGameStation)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//判断需要发送大结算与否
bool CServerGameDesk::IsNeedSendBoard()
{
	if (!m_bIsBuy)
	{
		return false;
	}

	if (m_iRunGameCount > 0)
	{
		return true;
	}

	if(GS_ROB_NT <= m_bGameStation && m_bGameStation <= GS_DOUBLE && GS_WAIT_NEXT != m_bGameStation)
	{
		return true;
	} else
	{
		return false;
	}
}

//用户离开游戏桌
BYTE CServerGameDesk::UserLeftDesk(BYTE bDeskStation, CGameUserInfo * pUserInfo)
{
	for(int i = 0; i < PLAY_COUNT; i ++)
	{
		m_vecWinInfo[i].RemoveAll();
	}
	memset(m_TCalculateBoard,0,sizeof(m_TCalculateBoard));

	CString llog;
	llog.Format("lbtest::::::::UserLeftDesk::::::::::::bDeskStation = %d:::::: iWinCount = %d",bDeskStation,m_TCalculateBoard[bDeskStation].iWinCount);
	OutputDebugString(llog);
	m_bUserReady[bDeskStation] = false ; 
	return __super::UserLeftDesk(bDeskStation,pUserInfo);
}

//游戲基礎倍數
int CServerGameDesk::GetRoomBasePoint()
{
	OutputDebugString("err::GetRoomBasePoint(0)");
	int iBasePoint = m_pDataManage->m_InitData.uBasePoint;//倍数
	OutputDebugString("err::GetRoomBasePoint(1)");
	return (iBasePoint>0?iBasePoint:1);
}

//计算各家分数
int CServerGameDesk::ComputePoint(BYTE DeskStation)
{
	OutputDebugString("err::ComputePoint(0)");
	int Shape;
	int Point = 0;
	int winer = -1;
	if( m_Logic.CompareCard(m_iUserCard[DeskStation], m_iUserCardCount[DeskStation], m_bBullCard[DeskStation],
		m_iUserCard[m_byUpGradePeople],m_iUserCardCount[m_byUpGradePeople],m_bBullCard[m_byUpGradePeople]) == 1)
	{
		winer = DeskStation;
		Point = 1;
	}
	else
	{
		winer = m_byUpGradePeople;
		Point = -1;
	}
	Shape = m_Logic.GetShape(m_iUserCard[winer],m_iUserCardCount[winer],m_bBullCard[winer]);
	Point = Point * m_bCardShapeBase[Shape];
	
	OutputDebugString("err::ComputePoint(1)");
	return Point;
}
//得到最大牌和最小牌,win值为1时得到最大,其它为最小
BYTE CServerGameDesk::GetIndex(int win)
{
	OutputDebugString("err::GetIndex(0)");
	BYTE Max = 0, Mix = 0;
	for (int i=1; i<PLAY_COUNT; i++)
	{
		if (m_Logic.CompareCard(&m_iTotalCard[Max*SH_USER_CARD],SH_USER_CARD,
			&m_iTotalCard[i*SH_USER_CARD],SH_USER_CARD)==-1)
			Max = i;
		if (m_Logic.CompareCard(&m_iTotalCard[Mix*SH_USER_CARD],SH_USER_CARD,
			&m_iTotalCard[i*SH_USER_CARD],SH_USER_CARD)==1)
			Mix = i;
	}
	if (win == 1)
	{
		OutputDebugString("err::GetIndex(1)");
		return Max;
	}
	else 
	{
		OutputDebugString("err::GetIndex(1)");
		return Mix;
	}

}
//mark
//换牌
BOOL CServerGameDesk::ChangeCard(BYTE bDestStation,BYTE bWinStation)
{
	OutputDebugString("err::ChangeCard(0)");
	for(int i = 0; i < SH_USER_CARD; i ++)
	{
		BYTE bTemp = m_iTotalCard[bDestStation * SH_USER_CARD + i];
		m_iTotalCard[bDestStation * SH_USER_CARD + i ] = m_iTotalCard[bWinStation * SH_USER_CARD + i];
		m_iTotalCard[bWinStation * SH_USER_CARD + i] = bTemp;
	}
	OutputDebugString("err::ChangeCard(1)");
	return TRUE;
}

//交换指定两个玩家的手牌
void	CServerGameDesk::ChangeTwoUserCard(BYTE byFirstDesk,BYTE bySecondDesk)
{
	BYTE byTmpCard[SH_USER_CARD];
	ZeroMemory(byTmpCard,sizeof(byTmpCard));

	memcpy(byTmpCard,m_iUserCard[byFirstDesk],sizeof(byTmpCard));
	memcpy(m_iUserCard[byFirstDesk],m_iUserCard[bySecondDesk],sizeof(m_iUserCard[byFirstDesk]));
	memcpy(m_iUserCard[bySecondDesk],byTmpCard,sizeof(m_iUserCard[bySecondDesk]));
}

//获取下一个玩家位置
BYTE CServerGameDesk::GetNextDeskStation(BYTE bDeskStation)
{
	if(!m_bTurnRule)//顺时针
	{
		int i=1;
		for (;i<PLAY_COUNT; i++)
		{
			if (m_pUserInfo[(bDeskStation + i ) % PLAY_COUNT] && m_byUserStation[(bDeskStation + i ) % PLAY_COUNT] != STATE_NULL)
				break;
		}
		return (bDeskStation + i ) % PLAY_COUNT;
	}
	//逆时针
	int i=PLAY_COUNT-1;
	for (;i>=1; i--)
	{
		if (m_pUserInfo[(bDeskStation + i ) % PLAY_COUNT] &&  m_byUserStation[(bDeskStation + i ) % PLAY_COUNT] != STATE_NULL)
			break;
	}
	return 	(bDeskStation + i) % PLAY_COUNT;
}
/*---------------------------------------------------------------------------------*/
//发送给第一个叫分
BOOL	CServerGameDesk::SendCallScore(BYTE bDeskStation)
{																
	CallScoreStruct TCallScore;
	//TCallScore.iValue = (m_iPrepareNT == 255) ? -1 : m_iCallScore[m_iPrepareNT];
	TCallScore.bDeskStation = bDeskStation;						
	TCallScore.bCallScoreflag = true;	
	//玩家的状态
	for(int i=0; i<PLAY_COUNT; i++)
	{
		TCallScore.byUserState[i] = m_byUserStation[i];
	}

	//记录当前操作的玩家是谁
	m_byCurrOperateUser = bDeskStation ;

	for(int i = 0; i < PLAY_COUNT; i ++)
	{
		TCallScore.bPlayer = (m_byUserStation[i] == STATE_NULL?false:true);
		SendGameData(i,&TCallScore,sizeof(TCallScore),MDM_GM_GAME_NOTIFY,ASS_CALL_SCORE,0);
	}
	SendWatchData(m_bMaxPeople,&TCallScore,sizeof(TCallScore),MDM_GM_GAME_NOTIFY,ASS_CALL_SCORE,0);
	SetTimer(TIME_CALL_NT,(m_iCallScoreTime+2)*NUM_ONE_SECOND_MS);
	
	return TRUE;
}
/*---------------------------------------------------------------------------------*/
//发送抢庄
BOOL	CServerGameDesk::SendRobNT(BYTE bDeskStation)
{																
	m_bGameStation = GS_ROB_NT;   //游戏状态
	RobNTStruct TRobNT;
	TRobNT.bDeskStation = bDeskStation;						
	TRobNT.bRobNTflag = true;	
	//玩家的状态
	for(int i=0; i<PLAY_COUNT; i++)
	{
		TRobNT.byUserState[i] = m_byUserStation[i];
	}

	for(int i = 0; i < PLAY_COUNT; i ++)
	{
		TRobNT.bPlayer = (m_byUserStation[i] == STATE_NULL?false:true);
		TRobNT.bDeskStation = i;
		SendGameData(i,&TRobNT,sizeof(TRobNT),MDM_GM_GAME_NOTIFY,ASS_CALL_ROBNT,0);
	}
	SendWatchData(m_bMaxPeople,&TRobNT,sizeof(TRobNT),MDM_GM_GAME_NOTIFY,ASS_CALL_ROBNT,0);
	SetTimer(TIME_CALL_ROBNT,(m_iRobNTTime+2)*NUM_ONE_SECOND_MS);
	tRobNTTime = time(NULL);//记录开始抢庄时间

	return TRUE;
}
/*---------------------------------------------------------------------------------*/
//叫庄
BOOL	CServerGameDesk::UserCallScore(BYTE bDeskStation, int iVal)
{
	if(bDeskStation != m_byCurrOperateUser)
		return true;
	KillTimer(TIME_CALL_NT);
	if(iVal == 0)
	{	
		m_iCallScore[bDeskStation] = 0;		//====某位置不叫分
	}
	else
	{
		m_iCallScore[bDeskStation] = 1;
		m_byUpGradePeople = bDeskStation;	//抢庄了就是庄家
		goto NT;
	}

	CallScoreStruct Tcallscore;
	Tcallscore.bDeskStation		= bDeskStation;
	Tcallscore.bCallScoreflag	= FALSE;					
	Tcallscore.iValue			= m_iCallScore[bDeskStation];		//当前叫分类型保存

	for(int i = 0; i < PLAY_COUNT; i ++)
	{
		SendGameData(i,&Tcallscore,sizeof(Tcallscore),MDM_GM_GAME_NOTIFY,ASS_CALL_SCORE_RESULT,0);		//将叫分情况发给其他用户
	}
	SendWatchData(m_bMaxPeople,&Tcallscore,sizeof(Tcallscore),MDM_GM_GAME_NOTIFY,ASS_CALL_SCORE_RESULT,0);

	//获取下一个位置的玩家
	int iNextPeople = GetNextDeskStation(bDeskStation);
	//如果下一个位置玩家已经操作过了 说明已经全部轮到了
	if(m_iCallScore[iNextPeople] == 0)
	{
		//第一个抢庄的玩家作为庄家
		m_byUpGradePeople=m_iFirstJiao;
		m_iCallScore[m_byUpGradePeople]=1;
		goto NT;
	}
	else if(m_iCallScore[iNextPeople] == -1)
	{
		SendCallScore(iNextPeople);
		return TRUE;
	}

NT:		
	CallScoreFinish();
	return true;
}

/*---------------------------------------------------------------------------------*/
//是否抢庄结束
BOOL CServerGameDesk::IsRobNTFinish()
{
	int playnum = 0;
	int robNTFinishNum = 0;
	for(int i = 0; i < PLAY_COUNT; i ++)
	{
		if(NULL == m_pUserInfo[i])
		{
			continue;
		}
		if(m_byUserStation[i] == STATE_HAVE_ROBNT || m_byUserStation[i] == STATE_PLAY_GAME)
		{
			++playnum;
		}
		
		if(m_byUserStation[i] == STATE_HAVE_ROBNT)
		{
			++robNTFinishNum;
		}
	}
	return playnum == robNTFinishNum;
}


/*---------------------------------------------------------------------------------*/
//抢庄
BOOL	CServerGameDesk::UserRobNT(BYTE bDeskStation, int iVal)
{
	if(m_byUserStation[bDeskStation] != STATE_PLAY_GAME)
		return true;

	if(iVal >= 0 && iVal <= 4)
	{
		m_iCallScore[bDeskStation] = iVal;
	}else
	{
		m_iCallScore[bDeskStation] = 0;
	}

	m_byUserStation[bDeskStation] = STATE_HAVE_ROBNT;

	RobNTStruct TRobNT;
	TRobNT.bDeskStation		= bDeskStation;
	TRobNT.bRobNTflag	    = FALSE;					
	TRobNT.iValue			= m_iCallScore[bDeskStation];		//当前叫分类型保存

	for(int i = 0; i < PLAY_COUNT; i ++)
	{
		SendGameData(i,&TRobNT,sizeof(TRobNT),MDM_GM_GAME_NOTIFY,ASS_CALL_ROBNT_RESULT,0);		//将叫分情况发给其他用户
	}
	SendWatchData(m_bMaxPeople,&TRobNT,sizeof(TRobNT),MDM_GM_GAME_NOTIFY,ASS_CALL_ROBNT_RESULT,0);

	//获取下一个位置的玩家
	/*int iNextPeople = GetNextDeskStation(bDeskStation);*/
	//如果下一个位置玩家已经操作过了 说明已经全部轮到了
	if(IsRobNTFinish())
	{
		KillTimer(TIME_CALL_ROBNT);
		BYTE TopCallScore[PLAY_COUNT];
		memset(TopCallScore,255,sizeof(TopCallScore));
		TopCallScore[0] = 0;
		int count = 1;
		for(int i = 1; i < 4; ++i)
		{
			if(m_iCallScore[i] > m_iCallScore[TopCallScore[0]])
			{
				memset(TopCallScore,255,sizeof(TopCallScore));

				TopCallScore[0] = i;
				count = 1;
			}else if(m_iCallScore[i] == m_iCallScore[TopCallScore[0]])
			{
				TopCallScore[count] = i;
				count++;
			}
		}
		srand(unsigned(time(0)));    
		if(m_iCallScore[TopCallScore[0]] == 0)
		{
			m_byUpGradePeople = m_iFirstJiao;
			m_iCallScore[m_byUpGradePeople] = 1;
			m_iNTBase = 1;
		}
		else
		{
			m_byUpGradePeople = TopCallScore[rand()%count];
			m_iNTBase = m_iCallScore[m_byUpGradePeople];
		}
		RobNTFinish();
	}	
	return true;
}
/*---------------------------------------------------------------------------------*/
//叫分结束
BOOL	CServerGameDesk::CallScoreFinish()
{

	CallScoreStruct Tscoreresult;
	Tscoreresult.iValue = m_iCallScore[m_byUpGradePeople];
	Tscoreresult.bDeskStation  = m_byUpGradePeople;
	Tscoreresult.bCallScoreflag = false;

	for(int i = 0; i < PLAY_COUNT; i ++)
	{
		SendGameData(i,&Tscoreresult,sizeof(Tscoreresult),MDM_GM_GAME_NOTIFY,ASS_CALL_SCORE_FINISH,0);		//将叫分情况发给其他用户
	}
	SendWatchData(m_bMaxPeople,&Tscoreresult,sizeof(Tscoreresult),MDM_GM_GAME_NOTIFY,ASS_CALL_SCORE_FINISH,0);

	NoticeUserNote();
	return TRUE;
}
/*---------------------------------------------------------------------------------*/
//抢庄结束
BOOL	CServerGameDesk::RobNTFinish()
{

	RobNTStruct TRobNTresult;
	TRobNTresult.iValue = m_iCallScore[m_byUpGradePeople];
	TRobNTresult.bDeskStation  = m_byUpGradePeople;
	TRobNTresult.bRobNTflag = false;

	for(int i = 0; i < PLAY_COUNT; i ++)
	{
		SendGameData(i,&TRobNTresult,sizeof(TRobNTresult),MDM_GM_GAME_NOTIFY,ASS_CALL_ROBNT_FINISH,0);		//将抢庄情况发给其他用户
	}
	SendWatchData(m_bMaxPeople,&TRobNTresult,sizeof(TRobNTresult),MDM_GM_GAME_NOTIFY,ASS_CALL_ROBNT_FINISH,0);

	NoticeUserDouble();
	return TRUE;
}
/*---------------------------------------------------------------------------------*/
//得到玩家的下注上限
__int64 CServerGameDesk::GetLimit(int bDeskStation)
{
	OutputDebugString("err::GetLimit(0)");

	int iCardShapeCount = MAX_SHAPE_COUNT;

	OutputDebugString("err::GetLimit(0-1)");
	int MaxBase = 1; //游戏最大倍率
	for (int i=0; i<iCardShapeCount; i++)
	{
		if (m_bCardShapeBase[i] > MaxBase)
		{
			MaxBase = m_bCardShapeBase[i];
		}
	}
	OutputDebugString("err::GetLimit(0-2)");
	__int64 i64Limite = 0;

	//庄家身上的钱
	__int64 i64NtNote = 0;
	if (NULL != m_pUserInfo[m_byUpGradePeople])
	{
		i64NtNote = m_pUserInfo[m_byUpGradePeople]->m_UserData.i64Money;
	}
	
	//自己身上的钱
	__int64 i64StationNote = 0;
	if (NULL != m_pUserInfo[bDeskStation])
	{
		i64StationNote = m_pUserInfo[bDeskStation]->m_UserData.i64Money;
	}
	
	OutputDebugString("err::GetLimit(0-4)");
	//真是玩家的人数
	int iPlayerCount = CountPlayer();
	OutputDebugString("err::GetLimit(0-5)");
	if (iPlayerCount>1)
	{
		//求出庄家平分后的分数
		i64Limite = i64NtNote/(iPlayerCount-1);

		if (m_iLimit > 0)
		{
			if (i64Limite > i64StationNote)
			{
				i64Limite = i64StationNote;
			}

			if (i64Limite > m_iLimit)
			{
				i64Limite = m_iLimit;
			}
			return i64Limite/MaxBase;
		}
		else if (m_pUserInfo[m_byUpGradePeople]!= NULL && m_pUserInfo[bDeskStation]!=NULL)
		{
			if (i64Limite > i64StationNote)
			{
				i64Limite = i64StationNote;
			}
			return i64Limite/MaxBase;
		}
	}
	OutputDebugString("err::GetLimit(1)");
	return 0;
}


///清理所有计时器
void CServerGameDesk::KillAllTimer()
{
	KillTimer(TIME_CALL_NT			);
	KillTimer(TIME_NOTE             );
	KillTimer(TIME_NOTE_FINISH		);
	KillTimer(TIME_SEND_CARD_FINISH	);
	KillTimer(TIME_AUTO_BULL		);
	KillTimer(TIME_GAME_FINISH		);
	KillTimer(TIME_CALL_ROBNT		);
	KillTimer(TIME_DOUBLE			);
	KillTimer(TIME_DOUBLE_FINISH	);
	KillTimer(TIME_GAME_BEGIN	    );

	//	Added by QiWang 20171106, 非房卡模式Timer
	if (!m_bIsBuy && m_bKickPerson)
	{
		KillTimer(TIME_CHECK_READY);
		m_bTimerCheckSet = false;
	}
}

int CServerGameDesk::GetMaxCardShape(BYTE iCard[], int iCardCount)
{
	if (iCardCount < SH_USER_CARD*PLAY_COUNT)
	{
		return 0;
	}
	int shape = 0;
	for(int i = 0; i < PLAY_COUNT; i ++)
	{
		if (m_pUserInfo[i] && m_byUserStation[i]!=STATE_NULL)
		{
			int temp = m_Logic.GetShape(&m_iTotalCard[i*SH_USER_CARD],SH_USER_CARD);
			if (shape < temp)
			{
				shape = temp;
			}
		}
		
	}
	return shape;
}
//-----------------------------------------------------------
/*机器人控制输赢 
add by dwj 20121122
*/
void CServerGameDesk::IAWinAutoCtrl()
{
	if(!m_bAIWinAndLostAutoCtrl)
	{
		return;
	}

	bool bAIWin = false;
	srand((unsigned)GetTickCount());
	int iResult = rand()%100;

	if (0 >=G_iAIHaveWinMoney )
	{//机器人赢的钱少于0 机器人必赢
		bAIWin = true;		
	}
	else if((0 < G_iAIHaveWinMoney) && (G_iAIHaveWinMoney < m_iAIWantWinMoneyA1))
	{// 机器人的赢钱在0-A1区域 并且概率符合m_iAIWinLuckyAtA1 机器人要赢
		if (iResult <=m_iAIWinLuckyAtA1)
			bAIWin = true;								
	}
	else if ((m_iAIWantWinMoneyA1 <= G_iAIHaveWinMoney)  && (G_iAIHaveWinMoney <m_iAIWantWinMoneyA2))
	{// 机器人的赢钱在A1-A2区域 并且概率符合m_iAIWinLuckyAtA2 机器人要赢
		if (iResult <=m_iAIWinLuckyAtA2)
			bAIWin = true;			
	}
	else if ((m_iAIWantWinMoneyA2 <= G_iAIHaveWinMoney)  && (G_iAIHaveWinMoney <m_iAIWantWinMoneyA3))
	{// 机器人的赢钱在A2-A3区域 并且概率符合m_iAIWinLuckyAtA3 机器人要赢
		if (iResult <=m_iAIWinLuckyAtA3)
			bAIWin = true;
	}
	else
	{// 机器人的赢钱超过A3区域 并且概率符合m_iAIWinLuckyAtA4 机器人要赢
		if (iResult <=m_iAIWinLuckyAtA4)
			bAIWin = true;
	}
	//CString strInfo;
	if (bAIWin)
	{//机器人要赢钱
		if (CountAIWinMoney() < 0)//机器人输,就换牌
		{
			for (int i=0;i<PLAY_COUNT;i++)
			{		
				//todo
				//为什么 一定是 0?
				ChangeCard(i+1,0);
				if(CountAIWinMoney()>=0)
				{
					break;
				}
			}	
		}		
	}
	else
	{//机器人要输钱
		if (CountAIWinMoney() > 0 || (G_iAIHaveWinMoney +CountAIWinMoney())<0)//机器人赢了人的话
		{
			for (int i=0;i<PLAY_COUNT;i++)
			{			
				ChangeCard(i+1,0);
				if (CountAIWinMoney()<= 0)
				{
					if ((G_iAIHaveWinMoney +CountAIWinMoney())<0)
					{
						//如果机器人这盘输钱了 导致机器人总赢钱数为负数，那么重新换牌;
						if (i != (PLAY_COUNT-1))
						{
							continue;
						}
						// 牌都换完了还是不能满足条件，那么只能重新换牌去让机器人赢了;
						for (int j = 0; j < PLAY_COUNT;j++)
						{				
							//todo 换好一点
							ChangeCard(j+1,0);
							if (CountAIWinMoney() >=0)
							{
								break;
							}
						}		
					}
					break;
				}
			}
		}
	}
}

	
//------------------------------------------------------------------------------------
void CServerGameDesk::RecordAiHaveWinMoney(GameEndStruct *GameEnd)
{
	//20121122dwj 结算机器人赢钱的情况;// 20121203 这里需要在结算扣税之前计算，不然会出现机器人输赢不平衡的情况
	if (/*!bAllAi && */m_bAIWinAndLostAutoCtrl)
	{//桌子上不全是机器人， 并且机器人控制了
		try
		{
			//这里异常处理，因为二人、四人牛牛涉及到好到房间对同一个配置文件读写操作，如果出错了；就不往配置文件中写数据;
			__int64 iReSetAIHaveWinMoney;
			CString sTemp = CINIFile::GetAppPath ();/////本地路径；
			CINIFile f(sTemp +SKIN_FOLDER  + _T("_s.ini"));
			CString szSec = TEXT("game");
			//Eil @ 20190325 
			//TCHAR szSec[_MAX_PATH] = TEXT("game");
			//_stprintf_s(szSec, sizeof(szSec), _T("%s_%d"), SKIN_FOLDER,m_pDataManage->m_InitData.uRoomID);
			iReSetAIHaveWinMoney = f.GetKeyVal(szSec,"ReSetAIHaveWinMoney ",(__int64)0);
			if (G_iReSetAIHaveWinMoney != iReSetAIHaveWinMoney)
			{//如果不相等，说明手动修改了配置文件中的值（让机器人吞钱了），相等就去累加机器人赢得钱;
				G_iAIHaveWinMoney = iReSetAIHaveWinMoney;
				G_iReSetAIHaveWinMoney = iReSetAIHaveWinMoney;
			}
			else
			{
				for(int i=0;i<PLAY_COUNT;i++)
				{
					//m_iUserFen[i] = m_iUserMoney[i];
					if(NULL != m_pUserInfo[i] && m_pUserInfo[i]->m_UserData.isVirtual == 0)///机器人输赢控制：是否开启机器人输赢自动控制
					{
						//只算机器人输赢
						//m_iAIHaveWinMoney += GameEnd.iTurePoint[i];	
						G_iAIHaveWinMoney -= GameEnd->iChangeMoney[i];///机器人输赢控制：机器人已经赢了多少钱iChangeMoney
				
					}
				}
			}
			//将当前机器人赢的钱写入配置文件当中
			sTemp.Format("%I64d",G_iAIHaveWinMoney);
			f.SetKeyValString(szSec,"AIHaveWinMoney ",sTemp);
			
		}
		catch (...)
		{
			//这里只是做个防错措施，什么也不处理DWJ
			OutputDebugString("dwjlog::写配置文件出错了!");
		}			
	}
}
//-------------------------------------------------------------------------------
int CServerGameDesk::CountAIWinMoney()
{
	int money = 0;
	BYTE iUserCard[PLAY_COUNT][10];	//临时保存各玩家的牌;
	memset(&iUserCard, 0, sizeof(iUserCard));
	BYTE iShape[PLAY_COUNT];
	memset(&iShape, 0, sizeof(iShape));
	for (int i =0;i<PLAY_COUNT;i++)
	{
		for (int j =0;j<SH_USER_CARD;j++)
		{
			iUserCard[i][j] =  m_iTotalCard[i * SH_USER_CARD+j];
		}
		//保存各玩家的牌型用于比较大小；
		iShape[i] = m_Logic.GetShape(iUserCard[i],5);
	} 
	
	if (m_byUpGradePeople!=255 && m_pUserInfo[m_byUpGradePeople])
	{
		int itmpmoney = 0;//机器人的钱统计
		if (m_pUserInfo[m_byUpGradePeople]->m_UserData.isVirtual)
		{///如果机器人是庄家,对比真是玩家的牌型
			for(int i=0;i<PLAY_COUNT; i++)
			{
				if (NULL != m_pUserInfo[i] )
				{
					if (!m_pUserInfo[i]->m_UserData.isVirtual)
					{//玩家不是机器人ddwwjj;
						//20121126dwj 比较机器人与庄家的牌大小;
						if( m_Logic.CompareCard(iUserCard[i], SH_USER_CARD, iUserCard[m_byUpGradePeople],SH_USER_CARD) == 1)
						{//玩家的牌型大于庄家机器人，那么就是机器人输钱;
							itmpmoney -= m_bCardShapeBase[iShape[i]]*m_iPerJuTotalNote[i];
							//itmpmoney += m_bCardShapeBase[iShape[m_iUpGradePeople]]*m_iPerJuTotalNote[i];
						}
						else
						{
							//itmpmoney -= m_bCardShapeBase[iShape[i]]*m_iPerJuTotalNote[i];
							itmpmoney += m_bCardShapeBase[iShape[m_byUpGradePeople]]*m_iPerJuTotalNote[i];
						}					
					}	
				}			
			}
			money = itmpmoney;
		}
		else
		{///如果机器人不是庄家,计算机器人的钱;
			for(int i=0;i<PLAY_COUNT; i++)
			{//遍历把机器人的钱加起来
				if (NULL != m_pUserInfo[i] )
				{
					if (m_pUserInfo[i]->m_UserData.isVirtual)
					{
						//20121126dwj 比较机器人庄家与玩家的牌大小;(第一个比第二个大就返回1,否侧返回-1)
						if(m_Logic.CompareCard(iUserCard[i], SH_USER_CARD, iUserCard[m_byUpGradePeople],SH_USER_CARD) == 1)
						{//机器人的牌型大于庄家的牌，机器人赢钱
							itmpmoney += m_bCardShapeBase[iShape[i]]*m_iPerJuTotalNote[i];
							//itmpmoney -= m_bCardShapeBase[iShape[m_iUpGradePeople]]*m_iPerJuTotalNote[i];
						}
						else
						{
							itmpmoney -= m_bCardShapeBase[iShape[m_byUpGradePeople]]*m_iPerJuTotalNote[i];
							//itmpmoney += m_bCardShapeBase[iShape[i]]*m_iPerJuTotalNote[i];
						}					
					}
				}			
			}
			money = itmpmoney;
		}
	}

	return money;
}
//------------------------------------------------------------------------------
//超级用户验证
void	CServerGameDesk::SuperUserExamine(BYTE bDeskStation)
{
	SUPERSTATE TSuperUser;
	if (NULL == m_pUserInfo[bDeskStation])
	{
		return ;
	}
	if (IsSuperUser(bDeskStation))
	{
		// 该用户为超级用户, 通知客户端
		TSuperUser.byDeskStation = bDeskStation;
		// 超级客户端
		TSuperUser.bEnable = true;
		SendGameData(bDeskStation, &TSuperUser, sizeof(TSuperUser), MDM_GM_GAME_NOTIFY, ASS_SUPER_USER, 0);
	}
}
//---------------------------------------------------------------------------------------------------
//是否超端用户 超端用户要发送结果给他看到
bool	CServerGameDesk::IsSuperUser(BYTE byDeskStation)
{
	if (NULL == m_pUserInfo[byDeskStation])
	{
		return false;
	}
#ifdef	SUPER_VERSION
	for(int i=0; i<m_vlSuperID.size(); i++)
	{
		if (NULL != m_pUserInfo[byDeskStation])
		{
			//找打了超端玩家
			if (m_pUserInfo[byDeskStation]->m_UserData.dwUserID == m_vlSuperID.at(i))
			{
				return true;
			}
		}
	}
#endif
	return false;
}
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
	return false;
}
//GRM函数
//更新奖池数据(游戏->平台)
/// @return 机器人赢钱数
void CServerGameDesk::UpDataRoomPond(__int64 iAIHaveWinMoney)
{

}