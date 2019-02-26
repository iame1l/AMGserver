#include "StdAfx.h"
#include "ClientGameDlg.h"
#include ".\clientgamedlg.h"


BEGIN_MESSAGE_MAP(CClientGameDlg, CLoveSendClass)
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	//	ON_WM_CLOSE()
END_MESSAGE_MAP()

//构造函数
CClientGameDlg::CClientGameDlg() : CLoveSendClass(IDD_GAME_FRAME)
{
	m_TCGameData.InitAllData();
	m_iNoteCount = 0;
	m_iMyCardShape = UG_ERROR_KIND;

}

//析构函数
CClientGameDlg::~CClientGameDlg()
{

}

//数据绑定函数
void CClientGameDlg::DoDataExchange(CDataExchange * pDX)
{
	CLoveSendClass::DoDataExchange(pDX);
//	DoDataExchangeWebVirFace(pDX);
}

//初始化函数
BOOL CClientGameDlg::OnInitDialog()
{
	CLoveSendClass::OnInitDialog();

	return TRUE;
}

/*--------------------------------------------------------------------------*/
//设置游戏状态
bool CClientGameDlg::SetGameStation(void * pBuffer, UINT nLen)
{
	switch(GetStationParameter())
	{
	case GS_WAIT_SETGAME:
	case GS_WAIT_ARGEE:
	case GS_WAIT_NEXT:
		{
			if (nLen != sizeof(S_C_GameStation_WaiteAgree))
			{
				return true;
			}
			S_C_GameStation_WaiteAgree *pGameStation = (S_C_GameStation_WaiteAgree *)pBuffer;
			if (NULL == pGameStation)
			{
				return true;
			}

			m_TCGameData.m_byThinkTime	= pGameStation->byThinkTime;		//游戏思考时间
			m_TCGameData.m_byBeginTime	= pGameStation->byBeginTime;		//游戏开始时间

			m_TCGameData.m_iGuoDi		= pGameStation->iGuoDi;			//锅底值	
			m_TCGameData.m_iDiZhu		= pGameStation->iDiZhu;			//底注      

			//m_TCGameData.m_i64ShangXian	= pGameStation->i64ShangXian;	//总下注上限		
			m_TCGameData.m_i64DingZhu	= pGameStation->i64DingZhu;	//个人下注上限  暗注的，但显示为明注的


			//如果是排队场就不用去点击准备了
			int iTime=1;
			if ((m_TCGameData.m_byBeginTime ) > 10)
			{
				iTime = ((rand()+GetMeUserInfo()->bDeskStation)%(5))+1;
			}
			else if (m_TCGameData.m_byBeginTime > 0)
			{
				iTime = ((rand()+GetMeUserInfo()->bDeskStation)%(m_TCGameData.m_byBeginTime))+1;
			}

			if (!(m_pGameInfo->dwRoomRule & GRR_QUEUE_GAME))
			{
				SetGameTimer(GetMeUserInfo()->bDeskStation,(iTime+1),ID_BEGIN_TIME);
			}

			break;
		}
	case GS_SEND_CARD:
		{
			if (nLen != sizeof(S_C_GameStation_SendCard))
			{
				return true;
			}
			S_C_GameStation_SendCard *pGameStation = (S_C_GameStation_SendCard *)pBuffer;
			if (NULL == pGameStation)
			{
				return true;
			}

			m_TCGameData.m_byThinkTime	= pGameStation->byThinkTime;	//游戏思考时间
			m_TCGameData.m_byBeginTime	= pGameStation->byBeginTime;	//游戏开始时间

			m_TCGameData.m_iGuoDi		= pGameStation->iGuoDi;			//锅底值	
			m_TCGameData.m_iDiZhu		= pGameStation->iDiZhu;			//底注      

			//m_TCGameData.m_i64ShangXian	= pGameStation->i64ShangXian;	//总下注上限		
			m_TCGameData.m_i64DingZhu	= pGameStation->i64DingZhu;	//个人下注上限  暗注的，但显示为明注的

			m_TCGameData.m_byNtPeople	= pGameStation->byNtStation;	//庄家位置
			m_TCGameData.m_i64ZongXiaZhu= pGameStation->i64AllZongZhu;	//总下注

			for (int i = 0;i<PLAY_COUNT;i++)
			{
				m_TCGameData.m_iUserState[i]	= pGameStation->iUserState[i];
				m_TCGameData.m_byUserCardCount[i] = pGameStation->byUserCardCount[i];
				m_TCGameData.m_i64XiaZhuData[i]	= pGameStation->i64XiaZhuData[i];
			}


			break;
		}		
	case GS_PLAY_GAME:
		{
			if (nLen != sizeof(S_C_GameStation_PlayGame))
			{
				return true;
			}
			S_C_GameStation_PlayGame *pGameStation = (S_C_GameStation_PlayGame *)pBuffer;
			if (NULL == pGameStation)
			{
				return true;
			}

			m_TCGameData.m_byThinkTime	= pGameStation->byThinkTime;	//游戏思考时间
			m_TCGameData.m_byBeginTime	= pGameStation->byBeginTime;	//游戏开始时间

			m_TCGameData.m_iGuoDi		= pGameStation->iGuoDi;			//锅底值	
			m_TCGameData.m_iDiZhu		= pGameStation->iDiZhu;			//底注      

			//m_TCGameData.m_i64ShangXian	= pGameStation->i64ShangXian;	//总下注上限		
			m_TCGameData.m_i64DingZhu	= pGameStation->i64ShangXian;	//个人下注上限  暗注的，但显示为明注的

			m_TCGameData.m_byNtPeople	= pGameStation->byNtStation;	//庄家位置
			m_TCGameData.m_i64ZongXiaZhu= pGameStation->i64AllZongZhu;	//总下注

			for (int i = 0;i<PLAY_COUNT;i++)
			{
				m_TCGameData.m_iUserState[i]	= pGameStation->iUserState[i];
				m_TCGameData.m_byUserCardCount[i] = pGameStation->byUserCardCount[i];
				m_TCGameData.m_i64XiaZhuData[i]	= pGameStation->i64XiaZhuData[i];
			}

			//当前操作的玩家
			m_TCGameData.m_byCurrHandleDesk	= pGameStation->byCurrHandleDesk;
			//用户手上的牌
			memcpy(m_TCGameData.m_byUserCard,pGameStation->byUserCard,sizeof(m_TCGameData.m_byUserCard));

			break;
		}
	}
	return true;
}
/*--------------------------------------------------------------------------*/
//游戏消息处理函数
bool CClientGameDlg::HandleGameMessage(NetMessageHead * pNetHead, void * buffer, UINT nLen, CTCPClientSocket * pClientSocket)
{

	CString log;
	log.Format("hnai: handle message %d",pNetHead->bAssistantID);
	OutputDebugString(log);

	if (pNetHead->bMainID==MDM_GM_GAME_NOTIFY)
	{
		switch (pNetHead->bAssistantID)
		{
		case ASS_GM_AGREE_GAME:
		case S_C_SUPER_PROOF:	//超端验证
		case S_C_SUPER_SET_RESULT:	//超端设置结果
			{
				return TRUE;
			}	
		case S_C_GAME_BEGIN:		//游戏开始
			{
				OnHandleGameBegin(buffer,nLen);
				return true;
			}
		case S_C_SEND_CARD:			//服务器发牌	
			{
				OnHandleSendCard(buffer,nLen);
				return true;
			}
		case S_C_GAME_PLAY:		//开始游戏
			{
				OnHandleBeginPlay(buffer,nLen);
				return true;
			}
		case S_C_CALL_ACTION:	//接收到操作的消息
			{
				OnHandleNoticeAction(buffer,nLen);
				return true;
			}
		case S_C_LOOK_CARD_RESULT:	//玩家看牌
			{
				OnHandleLookCardResult(buffer,nLen);
				return true;
			}
		case S_C_GEN_ZHU_RESULT:	//玩家跟注
			{
				OnHandleFollowResult(buffer,nLen);
				return true;
			}	
		case S_C_JIA_ZHU_RESULT:	//加注结果
			{
				OnHandleAddResult(buffer,nLen);
				return true;
			}
		case S_C_BI_PAI_RESULT:		//比牌结果
			{
				OnHandleBiPaiResult(buffer,nLen);
				return true;
			}
		case S_C_GIVE_UP_RESULT:	//弃牌结果
			{
				OnHandleGiveUpResult(buffer,nLen);
				return true;
			}
		case S_C_GAME_END:	//游戏正常结束
			{
				OnHandleGameFinish(buffer,nLen);
				return true;
			}

		}
	}

	return __super::HandleGameMessage(pNetHead,buffer,nLen,pClientSocket);
}
//收到游戏开始消息
void	CClientGameDlg::OnHandleGameBegin(void * buffer,int nLen)
{
	if (sizeof(S_C_GameBegin) != nLen)
	{
		return ;
	}
	S_C_GameBegin *pGameBegin = (S_C_GameBegin *)buffer;
	if (NULL == pGameBegin)
	{
		return ;
	}
	ResetGameStation(0);
	SetStationParameter(GS_SEND_CARD);
	memcpy(m_TCGameData.m_iUserState,pGameBegin->iUserState,sizeof(m_TCGameData.m_iUserState));
}
//收到开始发牌消息
void	CClientGameDlg::OnHandleSendCard(void * buffer,int nLen)
{
	if (sizeof(S_C_SendCard) != nLen)
	{
		return ;
	}
	S_C_SendCard *pSendCard = (S_C_SendCard *)buffer;
	if (NULL == pSendCard)
	{
		return ;
	}
	//拷贝发牌数据
	memcpy(m_TCGameData.m_byUserCard,pSendCard->byCard,sizeof(m_TCGameData.m_byUserCard));
	memcpy(m_TCGameData.m_byUserCardCount,pSendCard->byCardCount,sizeof(m_TCGameData.m_byUserCardCount));

	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if(nullptr == m_pUserInfo[i])continue;
		CString log;
		log.Format("hnlog %d,%s,牌（%d,%d,%d）牌型 %d ",i,m_pUserInfo[i]->GameUserInfo.nickName,
			m_TCGameData.m_byUserCard[i][0],
			m_TCGameData.m_byUserCard[i][1],
			m_TCGameData.m_byUserCard[i][2],
			m_Logic.GetCardShape(m_TCGameData.m_byUserCard[i],m_TCGameData.m_byUserCardCount[i]));
		OutputDebugString(log);
	}
	//判断自己的牌型
	//m_Logic.GetCardShape(m_iUserCard[GetMeUserInfo()->bDeskStation],m_iUserCardCount[GetMeUserInfo()->bDeskStation]);
}
//第一个玩家开始进行下注
void	CClientGameDlg::OnHandleBeginPlay(void * buffer,int nLen)
{
	if (sizeof(S_C_BeginPlay) != nLen)
	{
		return ;
	}
	S_C_BeginPlay *pBeginPlay = (S_C_BeginPlay *)buffer;
	if (NULL == pBeginPlay)
	{
		return ;
	}

	CString log;
	log.Format("hnlog  %s:%d",__FUNCTION__,__LINE__);
	OutputDebugString(log);

	SetStationParameter(GS_PLAY_GAME);
	m_TCGameData.m_byCurrHandleDesk = pBeginPlay->byCurrHandleStation;
	m_TCGameData.m_bCanLook		= true;
	m_TCGameData.m_bCanFollow	= false;
	memcpy(m_TCGameData.m_bCanAdd,pBeginPlay->bAdd,sizeof(m_TCGameData.m_bCanAdd));
	m_TCGameData.m_bCanOpen		= false;
	m_TCGameData.m_bCanGiveUp	= true;

	if(m_TCGameData.m_byCurrHandleDesk == GetMeUserInfo()->bDeskStation)
	{
		//如果是自己 那么就启动计时器  1/3的概率看牌
		int iRand = (rand()+GetMeUserInfo()->bDeskStation)%3;
		int iTime=1;
		//看牌
		if (0 == iRand && m_TCGameData.m_bCanLook)
		{
			iTime = ((rand()%2)+1);
			SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_LOOK_CARD);
		}
		else
		{
			if ((m_TCGameData.m_byThinkTime ) > 5)
			{
				iTime = ((rand()+GetMeUserInfo()->bDeskStation)%(5))+1;
			}
			else if (m_TCGameData.m_byThinkTime > 0)
			{
				iTime = ((rand()+GetMeUserInfo()->bDeskStation)%(m_TCGameData.m_byThinkTime))+1;
			} 
			SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_ADD_NOTE);
		}
	}
}

/*-----------------------------------------------------------------------------------*/
//接收到通知操作的消息
void	CClientGameDlg::OnHandleNoticeAction(void * buffer,int nLen)
{
	CString log;
	log.Format("hnlog  %s:%d",__FUNCTION__,__LINE__);
	OutputDebugString(log);

	if (sizeof(S_C_NoticeAction) != nLen)
	{
		return ; 
	}
	S_C_NoticeAction *pNoticeAction = (S_C_NoticeAction *)buffer;
	if (NULL == pNoticeAction)
	{
		return ; 
	}


	m_TCGameData.m_byCurrHandleDesk = pNoticeAction->byCurrHandleStation;
	m_TCGameData.m_bCanLook		= pNoticeAction->bCanLook;
	m_TCGameData.m_bCanFollow	= pNoticeAction->bCanFollow;
	memcpy(m_TCGameData.m_bCanAdd,pNoticeAction->bCanAdd,sizeof(m_TCGameData.m_bCanAdd));
	m_TCGameData.m_bCanOpen		= pNoticeAction->bCanOpen;
	m_TCGameData.m_bCanGiveUp	= pNoticeAction->bCanGiveUp;

	CString zrrTest;
	//检测是否有下注按钮
	bool bTempCanAdd = false;
	for (int i=0; i < E_CHOUMA_COUNT; i++)
	{
		bool kkk = m_TCGameData.m_bCanAdd[i];
		zrrTest.Format("zrrTestR::m_TCGameData.m_bCanAdd[%d],bTempCanAdd=%d,kkk=%d",m_TCGameData.m_bCanAdd[i],bTempCanAdd,kkk);
		OutputDebugString(zrrTest);
		if (kkk)
		{
			bTempCanAdd = true;
			break;
		}
	}

	bool bHasOpenCardObject = true;
	vector<BYTE> vCanOpenUsers;
	for (BYTE i=0; i<PLAY_COUNT; i++)
	{
		if(i ==GetMeUserInfo()->bDeskStation)continue;
		if (m_TCGameData.m_iUserState[i] == STATE_LOOK ||
			m_TCGameData.m_iUserState[i] == STATE_NORMAL ||
			m_TCGameData.m_iUserState[i] == STATE_WAITE_OPEN)
		{
			vCanOpenUsers.push_back(i);
		}
	}

	if(vCanOpenUsers.empty())
	{
		bHasOpenCardObject = false;
	}

	//最大牌玩家
	BYTE bMaxUser = GetCurMaxUser();
	//当前最大牌玩家是否是AI
	bool bMaxIsAi = false;
	if(bMaxUser != 255 && nullptr != m_pUserInfo[bMaxUser])
	{
		if(m_pUserInfo[bMaxUser]->GameUserInfo.isVirtual)
			bMaxIsAi = true;

		CString log;
		log.Format("hnlog  我 %d, 当前最大牌玩家%d %s AI",GetMeUserInfo()->bDeskStation,bMaxUser,bMaxIsAi?"是":"不是");
		OutputDebugString(log);
	}

	if(m_TCGameData.m_byCurrHandleDesk == GetMeUserInfo()->bDeskStation)
	{
		int iRand = (rand()+GetMeUserInfo()->bDeskStation)%5;
		int iTime=1;
		//如果还没看牌
		if (m_TCGameData.m_iUserState[GetMeUserInfo()->bDeskStation] == STATE_NORMAL)
		{
			//跟注或者加注了两次就看牌或者比牌
			bool LookEarlier = false;
			if(!bMaxIsAi)
			{
				LookEarlier = rand()%5 >=1;
			}

			if (m_iNoteCount >= 2 || LookEarlier)
			{
// 				int iRand = rand()%4;
// 				if (m_TCGameData.m_bCanOpen && 0==iRand && bHasOpenCardObject)
// 				{
// 					if ((m_TCGameData.m_byThinkTime ) > 5)
// 					{
// 						iTime = ((rand()+GetMeUserInfo()->bDeskStation)%(5))+1;
// 					}
// 					else if (m_TCGameData.m_byThinkTime > 0)
// 					{
// 						iTime = ((rand()+GetMeUserInfo()->bDeskStation)%(m_TCGameData.m_byThinkTime))+1;
// 					} 
// 					SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_OPEN_CARD);
// 				}
// 				else
				{
					iTime = ((rand()%2)+1);
					SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_LOOK_CARD);
				}
			}
			else
			{
				if ((m_TCGameData.m_byThinkTime ) > 5)
				{
					iTime = ((rand()+GetMeUserInfo()->bDeskStation)%(5))+1;
				}
				else if (m_TCGameData.m_byThinkTime > 0)
				{
					iTime = ((rand()+GetMeUserInfo()->bDeskStation)%(m_TCGameData.m_byThinkTime))+1;
				} 

				//如果可以跟注
				iRand = (rand()+m_iMyCardShape)%4;
				if (2 > iRand && m_TCGameData.m_bCanFollow)	//跟注
				{
					SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_FOLLOW_NOTE);
				}
				else
				{
					//iRand = rand()%4;
					//if (m_TCGameData.m_bCanOpen && 0==iRand && bHasOpenCardObject)
					//{
					//	SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_OPEN_CARD);
					//}
					if (bTempCanAdd/*m_TCGameData.m_bCanAdd*/)
					{
						SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_ADD_NOTE);
					}
					else
					{
						SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_FOLLOW_NOTE);
					}
				}
			}
		}
		else if (m_TCGameData.m_iUserState[GetMeUserInfo()->bDeskStation] == STATE_LOOK)	//已经看牌了
		{
			bool bGiveUp = false;
			//是否A以上
			bool bHaveA = false;
			for(int i=0; i<MAX_CARD_COUNT; i++)
			{
				if (	m_TCGameData.m_byUserCard[GetMeUserInfo()->bDeskStation][i] == 0x0D 
					||	m_TCGameData.m_byUserCard[GetMeUserInfo()->bDeskStation][i] == 0x1D
					||	m_TCGameData.m_byUserCard[GetMeUserInfo()->bDeskStation][i] == 0x2D
					||	m_TCGameData.m_byUserCard[GetMeUserInfo()->bDeskStation][i] == 0x3D)
				{
					bHaveA = true;
					break;
				}
			}

			if(bMaxIsAi)
			{
				if(bMaxUser == GetMeUserInfo()->bDeskStation)
				{
					bGiveUp = false;
				}
				else
				{
					if(!bHaveA && m_iMyCardShape == SH_OTHER) bGiveUp = (0 == rand()%2);
					if(bHaveA && m_iMyCardShape == SH_OTHER) bGiveUp = (0 == rand()%3);
					if(m_iMyCardShape >= SH_DOUBLE) bGiveUp = false;
				}
			}
			else
			{		
				if(m_iMyCardShape >= SH_DOUBLE) bGiveUp = false;
			}


			if ((m_TCGameData.m_byThinkTime ) > 5)
			{
				iTime = ((rand()+GetMeUserInfo()->bDeskStation)%(5))+1;
			}
			else if (m_TCGameData.m_byThinkTime > 0)
			{
				iTime = ((rand()+GetMeUserInfo()->bDeskStation)%(m_TCGameData.m_byThinkTime))+1;
			} 

			if(bGiveUp)
			{
				SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_GIVE_UP);
			}

			else
			{
				if(!bMaxIsAi && m_TCGameData.m_bCanOpen && bHasOpenCardObject && rand()%7 <= m_iMyCardShape)
				{
					SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_OPEN_CARD);
				}
				else
				{
					//如果可以跟注
					iRand = (rand()+m_iMyCardShape)%5;
					if (m_TCGameData.m_bCanOpen && (0==iRand || bGiveUp) && bHasOpenCardObject)
					{
						SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_OPEN_CARD);
					}
					else if (m_TCGameData.m_bCanFollow && iRand>=1 && iRand <= 2)
					{
						SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_FOLLOW_NOTE);
					}
					else if (bTempCanAdd/*m_TCGameData.m_bCanAdd*/)
					{
						SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_ADD_NOTE);
					}
					else
					{
						SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_FOLLOW_NOTE);
					}
				}
				
			}

		
		}
	}
}


/*-----------------------------------------------------------------------------------*/
//收到看牌结果消息
void	CClientGameDlg::OnHandleLookCardResult(void * buffer,int nLen)
{
	if (sizeof(S_C_LookCardResult) != nLen)
	{
		return; 
	}
	S_C_LookCardResult *pLookCardResult = (S_C_LookCardResult *)buffer;
	if (NULL == pLookCardResult)
	{
		return;
	}

	//玩家的状态
	m_TCGameData.m_iUserState[pLookCardResult->byDeskStation]		= pLookCardResult->iUserState;
	m_TCGameData.m_byUserCardCount[pLookCardResult->byDeskStation]	= pLookCardResult->byUserCardCount;
	//memcpy(m_TCGameData.m_byUserCard[pLookCardResult->byDeskStation],pLookCardResult->byUserCard, sizeof(m_TCGameData.m_byUserCard[pLookCardResult->byDeskStation]));

	//检测是否下注按钮可用
	bool bTempCanAdd = false;
	for (int i=0; i<E_CHOUMA_COUNT; i++)
	{
		bool kkk = m_TCGameData.m_bCanAdd[i];
		if ( kkk == true)
		{
			bTempCanAdd = true;
			break;
		}
	}



	//是否有可比牌对象
	bool bHasOpenCardObject = true;
	vector<BYTE> vCanOpenUsers;
	for (BYTE i=0; i<PLAY_COUNT; i++)
	{
		if(i ==GetMeUserInfo()->bDeskStation)continue;
		if (m_TCGameData.m_iUserState[i] == STATE_LOOK ||
			m_TCGameData.m_iUserState[i] == STATE_NORMAL ||
			m_TCGameData.m_iUserState[i] == STATE_WAITE_OPEN)
		{
			vCanOpenUsers.push_back(i);
		}
	}

	if(m_TCGameData.m_byCurrHandleDesk == GetMeUserInfo()->bDeskStation)
	{
		//获取牌型
		m_iMyCardShape = m_Logic.GetCardShape(m_TCGameData.m_byUserCard[pLookCardResult->byDeskStation],m_TCGameData.m_byUserCardCount[pLookCardResult->byDeskStation]);
		int iTime=1;
		iTime = ((rand()+GetMeUserInfo()->bDeskStation)%(5))+1;	
		/*
		int iTime=1;
		iTime = ((rand()+GetMeUserInfo()->bDeskStation)%(5))+1;
		if (m_iMyCardShape >= SH_DOUBLE)	//对子以上
		{
			//如果可以跟注
			int iRand = (rand()+m_iMyCardShape)%5;
			if (0 == iRand && m_TCGameData.m_bCanFollow)	//跟注
			{
				SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_FOLLOW_NOTE);
			}
			else if (1 == iRand && m_TCGameData.m_bCanOpen && bHasOpenCardObject)
			{
				SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_OPEN_CARD);
			}
			else if (bTempCanAdd && m_TCGameData.m_bCanAdd)	// 是否能够加注
			{
				SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_ADD_NOTE);
			}
			else
			{
				SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_FOLLOW_NOTE);
			}
		}
		else	//弃牌
	*/
		{
			//是否A以上
			bool bHaveA = false;
			for(int i=0; i<MAX_CARD_COUNT; i++)
			{
				if (	m_TCGameData.m_byUserCard[GetMeUserInfo()->bDeskStation][i] == 0x0D 
					||	m_TCGameData.m_byUserCard[GetMeUserInfo()->bDeskStation][i] == 0x1D
					||	m_TCGameData.m_byUserCard[GetMeUserInfo()->bDeskStation][i] == 0x2D
					||	m_TCGameData.m_byUserCard[GetMeUserInfo()->bDeskStation][i] == 0x3D)
				{
					bHaveA = true;
					break;
				}
			}

			//最大牌玩家
			BYTE bMaxUser = GetCurMaxUser();
			//当前最大牌玩家是否是AI
			bool bMaxIsAi = false;
			if(bMaxUser != 255 && nullptr != m_pUserInfo[bMaxUser])
			{
				if(m_pUserInfo[bMaxUser]->GameUserInfo.isVirtual)
					bMaxIsAi = true;
				CString log;
				log.Format("hnlog  我 %d, 当前最大牌玩家 %d %s AI",GetMeUserInfo()->bDeskStation,bMaxUser,bMaxIsAi?"是":"不是");
				OutputDebugString(log);
			}
			bool bGiveUp = true;
			if(bMaxIsAi)
			{
				if(bMaxUser == GetMeUserInfo()->bDeskStation)
				{
					bGiveUp = false;
				}
				else
				{
					if(!bHaveA && m_iMyCardShape == SH_OTHER) bGiveUp = (0 == rand()%2);
					if(bHaveA && m_iMyCardShape == SH_OTHER) bGiveUp = (0 == rand()%3);
					if(m_iMyCardShape >= SH_DOUBLE) bGiveUp = false;
				}
			}
			else
			{		
				if(m_iMyCardShape >= SH_DOUBLE) bGiveUp = false;
			}

			if (!bGiveUp)
			{
				if(!bMaxIsAi && m_TCGameData.m_bCanOpen && bHasOpenCardObject && rand()%7 <= m_iMyCardShape)
				{
					SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_OPEN_CARD);
				}
				else
				{
					int iRand = (rand()+m_iMyCardShape)%7;
					if (iRand >= 0 && iRand<=2 && m_TCGameData.m_bCanFollow)	//跟注
					{
						SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_FOLLOW_NOTE);
					}
					else if (iRand >= 3 && iRand <= 5 && m_TCGameData.m_bCanOpen && bHasOpenCardObject)
					{
						SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_OPEN_CARD);
					}
					else if (bTempCanAdd/* m_TCGameData.m_bCanAdd*/)
					{
						SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_ADD_NOTE);
					}
					else
					{
						SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,TIME_FOLLOW_NOTE);
					}
				}
			}
			else
			{
				SetGameTimer(GetMeUserInfo()->bDeskStation,iTime+1,TIME_GIVE_UP);
			}
		}
	}
}

//收到跟注消息
void	CClientGameDlg::OnHandleFollowResult(void * buffer,int nLen)
{
	if (sizeof(S_C_UserGenZhuResult) != nLen)
	{
		return;
	}
	S_C_UserGenZhuResult *pGenZhuResult = (S_C_UserGenZhuResult *)buffer;
	if (NULL == pGenZhuResult)
	{
		return;
	}
	//玩家的状态--判断玩家是否下注已满状态
	m_TCGameData.m_iUserState[pGenZhuResult->byDeskStation] = pGenZhuResult->iUserState;
	//没看牌跟注次数+1
	if (pGenZhuResult->byDeskStation == GetMeUserInfo()->bDeskStation && m_TCGameData.m_iUserState[GetMeUserInfo()->bDeskStation] == STATE_NORMAL)
	{
		m_iNoteCount++;
	}
}
//收到玩家加注消息
void	CClientGameDlg::OnHandleAddResult(void * buffer,int nLen)
{
	if (sizeof(S_C_JiaZhuResult) != nLen)
	{
		return ;
	}
	S_C_JiaZhuResult *pAddResult = (S_C_JiaZhuResult *)buffer;
	if (NULL == pAddResult)
	{
		return ;
	}
	//玩家的状态--判断玩家是否下注已满状态
	m_TCGameData.m_iUserState[pAddResult->byDeskStation] = pAddResult->iUserState;
	//没看牌跟注次数+1
	if (pAddResult->byDeskStation == GetMeUserInfo()->bDeskStation && m_TCGameData.m_iUserState[GetMeUserInfo()->bDeskStation] == STATE_NORMAL)
	{
		m_iNoteCount++;
	}
}

//收到比牌结果消息
void	CClientGameDlg::OnHandleBiPaiResult(void * buffer,int nLen)
{
	if (sizeof(S_C_UserBiPaiResult) != nLen)
	{
		return;
	}
	S_C_UserBiPaiResult *pBiPaiResult = (S_C_UserBiPaiResult *)buffer;
	if (NULL == pBiPaiResult)
	{
		return;
	}
	//玩家状态
	for(int i=0; i<PLAY_COUNT; i++)
	{
		m_TCGameData.m_iUserState[i] = pBiPaiResult->iUserStation[i];
	}
}
//收到弃牌消息
void	CClientGameDlg::OnHandleGiveUpResult(void * buffer,int nLen)
{
	if (sizeof(S_C_UserGiveUpResult) != nLen)
	{
		return; 
	}
	S_C_UserGiveUpResult *pGiveUpResult = (S_C_UserGiveUpResult *)buffer;
	if (NULL == pGiveUpResult)
	{
		return;
	}
	//玩家状态
	m_TCGameData.m_iUserState[pGiveUpResult->byDeskStation] = pGiveUpResult->iUserState;
}
//收到游戏正常结束消息
void	CClientGameDlg::OnHandleGameFinish(void * buffer,int nLen)
{
	if (sizeof(S_C_GameEnd) != nLen)
	{
		return; 
	}
	S_C_GameEnd *pGameEnd = (S_C_GameEnd *)buffer;
	if (NULL == pGameEnd)
	{
		return;
	}
	SetStationParameter(GS_WAIT_NEXT);

	ResetGameStation(0);
	//函数调用
	int iTime=1;
	if ((m_TCGameData.m_byBeginTime ) > 10)
	{
		iTime = ((rand()+GetMeUserInfo()->bDeskStation)%(10))+1;
	}
	else if (m_TCGameData.m_byBeginTime > 0)
	{
		iTime = ((rand()+GetMeUserInfo()->bDeskStation)%(3))+1;
	}

	//如果是排队场就不用去点击准备了
	if (!(m_pGameInfo->dwRoomRule & GRR_QUEUE_GAME))
	{

		SetGameTimer(GetMeUserInfo()->bDeskStation,iTime,ID_BEGIN_TIME);
	}
}

//重新设置界面
void CClientGameDlg::ResetGameFrame()
{
	return __super::ResetGameFrame();
}

//开始按钮函数
LRESULT	CClientGameDlg::OnHitBegin(WPARAM wparam, LPARAM lparam)
{
	OnControlHitBegin();
	return 0;
}

bool CClientGameDlg::OnControlHitBegin()
{
	if (((GetStationParameter()==GS_WAIT_SETGAME)||(GetStationParameter()==GS_WAIT_NEXT)||(GetStationParameter()==GS_WAIT_ARGEE))&& (!(m_pGameInfo->dwRoomRule & GRR_QUEUE_GAME)))
	{
		SendGameData(MDM_GM_GAME_NOTIFY,ASS_GM_AGREE_GAME,0);
	}

	KillGameTimer(0);
	ResetGameStation(0);
	return true;;
}

//清除所有定时器
void CClientGameDlg::KillAllTimer()
{
	KillTimer(ID_BEGIN_TIME);
	KillTimer(TIME_LOOK_CARD);
	KillTimer(TIME_ADD_NOTE);
	KillTimer(TIME_FOLLOW_NOTE);
	KillTimer(TIME_OPEN_CARD);
	KillTimer(TIME_GIVE_UP);

	return;
}

//定时器消息
bool CClientGameDlg::OnGameTimer(BYTE bDeskStation, UINT uTimeID, UINT uTimeCount)
{
	switch (uTimeID)
	{
	case ID_BEGIN_TIME:	//继续游戏定时器
		{
			KillAllTimer();
			if(GetStationParameter() == GS_WAIT_SETGAME || GetStationParameter() == GS_WAIT_ARGEE || GetStationParameter() == GS_WAIT_NEXT)
			{
				OnHitBegin(0,0);
			}			
			return true;
		}
	case TIME_LOOK_CARD:
		{
			KillTimer(TIME_LOOK_CARD);
			if(!m_TCGameData.m_bCanLook)
			{
				return true;
			}
			
			C_S_LookCard TLookCard;
			TLookCard.byDeskStation = GetMeUserInfo()->bDeskStation;
			SendGameData(&TLookCard,sizeof(TLookCard),MDM_GM_GAME_NOTIFY,C_S_LOOK_CARD,0);	

			return true;
		}	
	case TIME_ADD_NOTE:
		{
			KillTimer(TIME_ADD_NOTE);

			int iTempType = -1;

			for (int i = 0; i < E_CHOUMA_COUNT; i++)
			{
				bool kkk = m_TCGameData.m_bCanAdd[i];
				if (kkk)
				{
					iTempType = i;
					break;
				}
			}

			C_S_JiaZhu	TJiaZhu;
			TJiaZhu.iAddType = iTempType;
			TJiaZhu.byDeskStation	= GetMeUserInfo()->bDeskStation;
			SendGameData(&TJiaZhu,sizeof(TJiaZhu),MDM_GM_GAME_NOTIFY,C_S_JIA_ZHU,0);

			CString zrrTest;
			zrrTest.Format("zrrTestR::iTempType=%d,m_TCGameData.m_bCanAdd[0]=%d,m_TCGameData.m_bCanAdd[1]=%d,m_TCGameData.m_bCanAdd[2]=%d,m_TCGameData.m_bCanAdd[3]=%d",iTempType,m_TCGameData.m_bCanAdd[0],m_TCGameData.m_bCanAdd[1],m_TCGameData.m_bCanAdd[2],m_TCGameData.m_bCanAdd[3]);
			OutputDebugString(zrrTest);

			return true;
		}
	case TIME_FOLLOW_NOTE:
		{
			KillTimer(TIME_ADD_NOTE);
			/*if(!m_TCGameData.m_bCanFollow)
			{
			return true;
			}
			*/
			C_S_UserGenZhu TUserGenZhu;
			TUserGenZhu.byDeskStation = GetMeUserInfo()->bDeskStation;;
			SendGameData(&TUserGenZhu,sizeof(TUserGenZhu),MDM_GM_GAME_NOTIFY,C_S_GEN_ZHU,0);

			return true;
		}
	case TIME_OPEN_CARD:	//比牌
		{
			KillTimer(TIME_OPEN_CARD);
			if (!m_TCGameData.m_bCanOpen)
			{
				return true;
			}
	
			int iIndex =0;

			vector<BYTE> vCanOpenUsers;
			for (BYTE i=0; i<PLAY_COUNT; i++)
			{
				if(i ==GetMeUserInfo()->bDeskStation)continue;
				if (m_TCGameData.m_iUserState[i] == STATE_LOOK ||
					m_TCGameData.m_iUserState[i] == STATE_NORMAL ||
					m_TCGameData.m_iUserState[i] == STATE_WAITE_OPEN)
				{
					vCanOpenUsers.push_back(i);
				}
			}

			if(vCanOpenUsers.empty()) return true;
			random_shuffle(vCanOpenUsers.begin(),vCanOpenUsers.end());		

			C_S_UserBiPai	TUserBiPai;
			TUserBiPai.byDeskStation = GetMeUserInfo()->bDeskStation;
			TUserBiPai.byBeDeskStation	= vCanOpenUsers.front();	
			SendGameData(&TUserBiPai,sizeof(TUserBiPai), MDM_GM_GAME_NOTIFY, C_S_BI_PAI, 0);
			
			return true;
		}
	case TIME_GIVE_UP:	         //弃牌计时器
		{
			KillTimer(TIME_GIVE_UP);
		
			C_S_UserGiveUp TUserGiveUp;
			TUserGiveUp.byDeskStation = GetMeUserInfo()->bDeskStation;

			SendGameData(&TUserGiveUp,sizeof(TUserGiveUp),MDM_GM_GAME_NOTIFY,C_S_GIVE_UP,0);
			return true;
		}
	}
	return true;
}




//重置游戏
void CClientGameDlg::ResetGameStation(int iGameStation)
{
	m_TCGameData.InitSomeData();
	m_iNoteCount = 0;
	m_iMyCardShape = UG_ERROR_KIND;
	return;
}

void CClientGameDlg::OnGameSetting()
{
	AfxSetResourceHandle(GetModuleHandle(NULL));
	return;
}

void CClientGameDlg::OnWatchSetChange(void)
{

}

//用户离开
bool CClientGameDlg::GameUserLeft(BYTE bDeskStation, UserItemStruct * pUserItem, bool bWatchUser)
{
	
	if (GetStationParameter()>GS_WAIT_ARGEE)
	{
		for (int i=0;i<PLAY_COUNT;i++)
		{
			if (m_pUserInfo[i]!=NULL) 
			{
				m_pUserInfo[i]->GameUserInfo.bUserState=USER_SITTING;
			}
		}
	}

	//当前玩家离开看此玩家牌的玩家也随之离开房间
	if(GetMeUserInfo()->bDeskStation==bDeskStation&&pUserItem!=NULL&&!bWatchUser==TRUE)
	{
		OnCancel();
	}
	return true;

}

void CClientGameDlg::OnTimer(UINT nIDEvent)
{
	CLoveSendClass::OnTimer(nIDEvent);
}


void CClientGameDlg::OnCancel()
{
	if(GetStationParameter()>=GS_SEND_CARD&&GetStationParameter() < GS_WAIT_NEXT)		//查询状态
	{
		return;
		SendGameData(MDM_GM_GAME_FRAME,ASS_GM_FORCE_QUIT,0);
	}

	KillAllTimer();

	AFCCloseFrame();

	SendGameData(MDM_GR_USER_ACTION,ASS_GR_USER_UP,0);		//发送起立消息

	__super::OnCancel();
}

BYTE	CClientGameDlg::GetCurMaxUser()
{
	BYTE byTmpMax = 255;
	//先找到最大牌的玩家 
	for (BYTE i = 0; i < PLAY_COUNT; i++)
	{
		if(nullptr == m_pUserInfo[i])continue;
		if (STATE_ERR !=m_TCGameData.m_iUserState[i])
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
		for (BYTE i = 0; i < PLAY_COUNT; i++)
		{
			if(nullptr == m_pUserInfo[i])continue;
			if (i == byTmpMax)continue;
			if (STATE_ERR != m_TCGameData.m_iUserState[i])
			{
				int IsWin = m_Logic.CompareCard(m_TCGameData.m_byUserCard[byTmpMax],MAX_CARD_COUNT,m_TCGameData.m_byUserCard[i],MAX_CARD_COUNT);
				//被比较着的牌大
				if (-1 == IsWin)
				{
					byTmpMax = i;
				}
			}
		}
	}

	return byTmpMax;
}