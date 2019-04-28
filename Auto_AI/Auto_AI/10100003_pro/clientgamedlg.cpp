#include "StdAfx.h"

#include ".\clientgamedlg.h"
#include "gameframedlg.h"
#include "Cfg.h"
#include "Cfg.cpp"
#include "CardData.h"
#include "CardData.CPP"
#include "def.h"
#include "GameStation.h"
#include "GameStation.cpp"
#include "UpGradeLogic.h"
#include "ExtensionObject.cpp"
#include "Common.h"
#include "Common.cpp"
#include "./Card/CardArrayBase.h"
#include "./AIAlgorithm/IAIAlgorithm.h"

#include "./Card/CardArrayBase.cpp"
#include "./Card/CardBase.cpp"
#include "./AIAlgorithm/AIAlgorithmManage.cpp"
#include "./AIAlgorithm/HandsNumberAndWeight.cpp"
#include "./util/log/LogModule.cpp"
#include "./DataManage.cpp"

bool g_bFirstTimeNtOut = false;


BEGIN_MESSAGE_MAP(CClientGameDlg, CLoveSendClass)
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
END_MESSAGE_MAP()

CClientGameDlg::CClientGameDlg()
	: CLoveSendClass(IDD_GAME_FRAME)
	//, m_pExt(new ExtensionObject())
	, m_pExt(NULL)
{
	m_pExt = HN::CSmartPointer<ExtensionObject>(new ExtensionObject());
	m_bHaveRobOrCall = false;
	for(int i = 0; i < PLAY_COUNT; ++i)
	{
		vChuCard[i].clear();
	}
	CString s = CINIFile::GetAppPath ();/////本地路径
	CString nid;
	nid.Format("%d",NAME_ID);
	CINIFile f( s +nid +"_c.ini");

	CString key = "game";

	m_iDefaultTime = f.GetKeyVal(key,"DefaultTime",5);
	m_iHandCount = f.GetKeyVal(key,"HandCount",17);

	HN_AI::SSysConfig tSysConfig;
	tSysConfig.bIsWBomb_Separate = f.GetKeyVal(key, "BombCompareType", 1);
	tSysConfig.bIs3W1_One = f.GetKeyVal(key, "BombCompareType", 1);
	tSysConfig.bIs3W1_Double = f.GetKeyVal(key, "BombCompareType", 1);
	tSysConfig.bIs4W2_One = f.GetKeyVal(key, "BombCompareType", 1);
	tSysConfig.bIs4W2_Double = f.GetKeyVal(key, "BombCompareType", 1);
	tSysConfig.bIsLongBomb = f.GetKeyVal(key, "BombCompareType", 0);
	tSysConfig.iBombCompareType = f.GetKeyVal(key, "BombCompareType", 1);
	_dataMange.SetCardRulesConfig(tSysConfig);
}

BOOL CClientGameDlg::OnInitDialog()
{
	CLoveSendClass::OnInitDialog();

	// TODO:  在此添加额外的初始化
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CClientGameDlg::KillAllTimer()
{
	for( int i = TID_BEGIN; TID_MAX > i; i++ )
		KillGameTimer(i);
}

void CClientGameDlg::ResetExt()
{
	OBJ_GET_EXT(m_pExt, CGameStation, gs);
	OBJ_GET_EXT(m_pExt, CPlayerCardMgr, cardMgr);
	OBJ_GET_EXT(m_pExt, CCfg, cfg);

	cfg->Clear();
	cfg->SetValue(CFG_PLAYER_COUNT, PLAY_COUNT);

	cardMgr->Clear();
	cardMgr->Init();

	gs->Clear();

	for(int i = 0; i < PLAY_COUNT; ++i)
	{
		vChuCard[i].clear();
	}
	m_bHaveRobOrCall = false;
}

bool CClientGameDlg::SetGameStation(void * pStationData, UINT uDataSize)
{
	bool bResult = true;

	//////////////////////////////////////////////////////////////////////////
	// 清理数据
	ResetExt();
	///

	OBJ_GET_EXT(m_pExt, CCfg, cfg);
	OBJ_GET_EXT(m_pExt, CPlayerCardMgr, cardMgr);

	switch( GetStationParameter() )
	{
	case GS_WAIT_SETGAME:
	case GS_WAIT_ARGEE:
		{
			GameStation_2 * pGs = (GameStation_2 *)pStationData;

			cfg->SetValue(CFG_BEGIN_TIME, pGs->iBeginTime);
			cfg->SetValue(CFG_THINK_TIME, pGs->iThinkTime);
			cfg->SetValue(CFG_ROB_NT_TIME, pGs->iRobNTTime);
			cfg->SetValue(CFG_CALL_SCORE_TIME, pGs->iCallScoreTime);
			cfg->SetValue(CFG_ADD_DOUBLE_TIME, pGs->iAddDoubleTime);
			cfg->SetValue(CFG_ROOM_BASE_POINT, pGs->iRoomBasePoint);
			cfg->SetValue(CFG_RUN_PUBLISH, pGs->iRunPublish);
			cfg->SetValue(CFG_CARD_SHAPE, pGs->iCardShape);
			cfg->SetValue(CFG_ROOM_RULE, m_pGameInfo->dwRoomRule);
			cfg->SetValue(CFG_CARD_SHAPE, pGs->iCardShape);

			cardMgr->Init();
			if( !IsQueueGame() && !IsContestGame() )
				StartupGameBeginTimer();
		}
		break;
	case GS_SEND_CARD:
	case GS_WAIT_BACK:
		{
			GameStation_3 * pGs = (GameStation_3 *)pStationData;
			cfg->SetValue(CFG_BEGIN_TIME, pGs->iBeginTime);
			cfg->SetValue(CFG_THINK_TIME, pGs->iThinkTime);
			cfg->SetValue(CFG_ROB_NT_TIME, pGs->iRobNTTime);
			cfg->SetValue(CFG_CALL_SCORE_TIME, pGs->iCallScoreTime);
			cfg->SetValue(CFG_ADD_DOUBLE_TIME, pGs->iAddDoubleTime);
			cfg->SetValue(CFG_ROOM_BASE_POINT, pGs->iRoomBasePoint);
			cfg->SetValue(CFG_RUN_PUBLISH, pGs->iRunPublish);
			cfg->SetValue(CFG_CARD_SHAPE, pGs->iCardShape);
			cfg->SetValue(CFG_ROOM_RULE, m_pGameInfo->dwRoomRule);
			cfg->SetValue(CFG_CARD_SHAPE, pGs->iCardShape);

			int nPos = 0;
			for( int i = 0; PLAY_COUNT > i; i++ )
			{
				cardMgr->SetHandCard(i, &pGs->iUserCardList[nPos], pGs->iUserCardCount[i] );
				nPos += pGs->iUserCardCount[i];
			}
			BYTE byMyStation = GetMeUserInfo()->bDeskStation;
			if(pGs->iRobNT[byMyStation] == 3 || pGs->iRobNT[byMyStation] == 1)
			{
				m_bHaveRobOrCall = true;
			}
			else if(pGs->iCallScore[byMyStation] != 0)
			{
				m_bHaveRobOrCall = true;
			}
			
			switch (pGs->iGameFlag)
			{
			case  GS_FLAG_ROB_NT:
				if(byMyStation == pGs->iCurOperator)
				{
					SetMyGameTimer(byMyStation, cfg->GetValue(CFG_ROB_NT_TIME, 5), TID_ROB_NT);
				}
				break;
			case  GS_FLAG_ADD_DOUBLE:
				{
					if( 0 == pGs->iUserDoubleValue[byMyStation] )
					{
						SetMyGameTimer(GetMeUserInfo()->bDeskStation, cfg->GetValue(CFG_ADD_DOUBLE_TIME, 5), TID_ADD_DOUBLE);
					}
				}
				break;
			case  GS_FLAG_SHOW_CARD:
				{
					//SetMyGameTimer(byMyStation, cfg->GetValue(CFG_ADD_DOUBLE_TIME, 5), TID_SHOW_CARD);
				}
				break;
			}
		}
		break;
		//mark
	case GS_PLAY_GAME:	//游戏进行中
		{
			GameStation_4 * pGs = (GameStation_4 *)pStationData;
			cfg->SetValue(CFG_BEGIN_TIME, pGs->iBeginTime);
			cfg->SetValue(CFG_THINK_TIME, pGs->iThinkTime);
			cfg->SetValue(CFG_ROB_NT_TIME, pGs->iRobNTTime);
			cfg->SetValue(CFG_CALL_SCORE_TIME, pGs->iCallScoreTime);
			cfg->SetValue(CFG_ADD_DOUBLE_TIME, pGs->iAddDoubleTime);
			cfg->SetValue(CFG_ROOM_BASE_POINT, pGs->iRoomBasePoint);
			cfg->SetValue(CFG_RUN_PUBLISH, pGs->iRunPublish);
			cfg->SetValue(CFG_CARD_SHAPE, pGs->iCardShape);
			cfg->SetValue(CFG_ROOM_RULE, m_pGameInfo->dwRoomRule);
			cfg->SetValue(CFG_CARD_SHAPE, pGs->iCardShape);

			int nPos = 0;
			for( int i = 0; PLAY_COUNT > i; i++ )
			{
				cardMgr->SetHandCard(i, &pGs->iUserCardList[nPos], pGs->iUserCardCount[i] );
				nPos += pGs->iUserCardCount[i];
			}

			for( int i = 0; PLAY_COUNT > i; i++ )
				cardMgr->SetDeskCard(i, pGs->iDeskCardList[i], pGs->iDeskCardCount[i]);

			OBJ_GET_EXT(m_pExt, CGameStation, gs);
			gs->SetOutCardPeople( pGs->iOutCardPeople );
			gs->SetNtPeople(pGs->iUpGradePeople);
			gs->SetBigOutPeople(pGs->iBigOutPeople);


			BYTE byMyStation = GetMeUserInfo()->bDeskStation;
			if(gs->GetOutCardPeople() == byMyStation)
			{
				SetMyGameTimer(byMyStation, cfg->GetValue(CFG_THINK_TIME, 5), TID_OUT_CARD);
			}
		}
		break;
	case GS_WAIT_NEXT:		//等待下一盘开始
		{
			GameStation_5 * pGs = (GameStation_5 *)pStationData;
			cfg->SetValue(CFG_BEGIN_TIME, pGs->iBeginTime);
			cfg->SetValue(CFG_THINK_TIME, pGs->iThinkTime);
			cfg->SetValue(CFG_ROB_NT_TIME, pGs->iRobNTTime);
			cfg->SetValue(CFG_CALL_SCORE_TIME, pGs->iCallScoreTime);
			cfg->SetValue(CFG_ADD_DOUBLE_TIME, pGs->iAddDoubleTime);
			cfg->SetValue(CFG_ROOM_BASE_POINT, pGs->iRoomBasePoint);
			cfg->SetValue(CFG_RUN_PUBLISH, pGs->iRunPublish);
			cfg->SetValue(CFG_CARD_SHAPE, pGs->iCardShape);
			cfg->SetValue(CFG_ROOM_RULE, m_pGameInfo->dwRoomRule);
			cfg->SetValue(CFG_CARD_SHAPE, pGs->iCardShape);

			if( !IsQueueGame() && !IsContestGame() )
				StartupGameBeginTimer();
			break;
		}
	default:
		bResult = false;
	}

	return bResult;
}

bool CClientGameDlg::HandleGameMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	if( MDM_GM_GAME_NOTIFY != pNetHead->bMainID )
		return __super::HandleGameMessage( pNetHead, pNetData, uDataSize, pClientSocket );

	OBJ_GET_EXT( m_pExt, CCfg, cfg );

	switch( pNetHead->bAssistantID )
	{
	case ASS_GAME_BEGIN:		//游戏开始
		{
			if (uDataSize != sizeof(GameBeginStruct)) return false;
			GameBeginStruct * pBegin=(GameBeginStruct *)pNetData;	
			OBJ_GET_EXT(m_pExt, CCfg, cfg);
			cfg->SetValue(CFG_CARD_SHAPE, pBegin->iCardShape);
			SetStationParameter( GS_SEND_CARD );
		}
		return true;
	case ASS_SEND_FINISH:
		{
			SetStationParameter( GS_WAIT_BACK );
			return true;
		}
	case ASS_SEND_CARD:			
		{
			if (uDataSize!=sizeof(SendCardStruct)) return false;
			SendCardStruct * pSendCard=(SendCardStruct *)pNetData;
			OBJ_GET_EXT(m_pExt, CPlayerCardMgr, cardMgr);
			cardMgr->SetHandCard( pSendCard->bDeskStation, &pSendCard->bCard, 1 );
		}
		return true;
	case ASS_SEND_ALL_CARD://发完牌了,存入到自己的手牌中
		{
			if (uDataSize!=sizeof(SendAllStruct)) return false;
			SendAllStruct * pSendAll=(SendAllStruct *)pNetData;
			OBJ_GET_EXT(m_pExt, CPlayerCardMgr, cardMgr);
			int nPos = 0;
			for( int i = 0; PLAY_COUNT > i; i++ )
			{
				cardMgr->SetHandCard( i, &pSendAll->iUserCardList[nPos], pSendAll->iUserCardCount[i]);
				nPos += pSendAll->iUserCardCount[i];
			}
		}
		return true;
	case ASS_CALL_SCORE:			//叫分//计算牌权值的地方
		{	
			SetStationParameter(GS_WAIT_BACK);
			if (sizeof(CallScoreStruct)!= uDataSize) return false;
			CallScoreStruct *pScore=(CallScoreStruct *)pNetData;	

			if( GetMeUserInfo()->bDeskStation == pScore->bDeskStation )
				SetMyGameTimer(pScore->bDeskStation, cfg->GetValue(CFG_CALL_SCORE_TIME, 5), TID_CALL_SCORE);
		}
		return true;
	case ASS_ROB_NT:	//抢地主
		{
			if (uDataSize != sizeof(RobNTStruct)) return false;
				
			RobNTStruct * pRobNT = (RobNTStruct *)pNetData;
			if( GetMeUserInfo()->bDeskStation == pRobNT->byDeskStation )
				SetMyGameTimer(pRobNT->byDeskStation, cfg->GetValue(CFG_ROB_NT_TIME, 5), TID_ROB_NT);
		}
		return true;
	case ASS_BACK_CARD_EX:		//扩展底牌数据
		{
			if (uDataSize!=sizeof(BackCardExStruct)) 
				return FALSE;
			BackCardExStruct * pBackCard=(BackCardExStruct *)pNetData;
			OBJ_GET_EXT(m_pExt, CGameStation, gs);
			OBJ_GET_EXT(m_pExt, CPlayerCardMgr, cardMgr);
			gs->SetNtPeople( pBackCard->iGiveBackPeople );
			cardMgr->SetBackCard(pBackCard->iBackCard, pBackCard->iBackCardCount);
			cardMgr->PushHandCard(pBackCard->iGiveBackPeople, pBackCard->iBackCard, pBackCard->iBackCardCount);
			
			LOG( "ASS_BACK_CARD_EX %d - %d - %d", GetMeUserInfo()->bDeskStation, GetMeUserInfo()->dwUserID, cardMgr->GetHandCardCount(GetMeUserInfo()->bDeskStation) );
		}
		return true;
	case ASS_ADD_DOUBLE://加棒
		{
			if (uDataSize!=sizeof(AddDoubleStruct)) return false;
			AddDoubleStruct * pAddDouble=(AddDoubleStruct *)pNetData;
			//if(GetMeUserInfo()->bDeskStation != pAddDouble->bDeskStation )
				SetMyGameTimer(GetMeUserInfo()->bDeskStation, cfg->GetValue(CFG_ADD_DOUBLE_TIME, 5), TID_ADD_DOUBLE);
		}
		return true;
	case ASS_SHOW_CARD://亮牌
		{
			if (uDataSize!=sizeof(ShowCardStruct)) return false;
			ShowCardStruct  * pShow = (ShowCardStruct*)pNetData;
			if( GetMeUserInfo()->bDeskStation == pShow->bDeskStation )
				SetMyGameTimer(pShow->bDeskStation, cfg->GetValue(CFG_ADD_DOUBLE_TIME, 5), TID_SHOW_CARD);
		}
		return true;
	case ASS_GAME_PLAY:		//开始游戏
		{
			if (uDataSize!=sizeof(BeginPlayStruct)) return false;

			BeginPlayStruct * pBeginInfo=(BeginPlayStruct *)pNetData;
			OBJ_GET_EXT( m_pExt, CGameStation, gs );

			SetStationParameter(GS_PLAY_GAME);
			gs->SetBigOutPeople( pBeginInfo->iOutDeskStation );
			gs->SetOutCardPeople( pBeginInfo->iOutDeskStation );
			if( INVALID_INDEX == gs->GetBigOutPeople() )
				throw(-1);

			if (GetMeUserInfo()->bDeskStation == pBeginInfo->iOutDeskStation)
				SetMyGameTimer(pBeginInfo->iOutDeskStation, cfg->GetValue(CFG_THINK_TIME, 5), TID_OUT_CARD);
		}
		return true;
	case ASS_OUT_CARD_RESULT:
		{
			if (uDataSize != sizeof(OutCardMsg2AI))return false;
			OutCardMsg2AI * pOutCardInfo=(OutCardMsg2AI *)pNetData;
			OBJ_GET_EXT( m_pExt, CPlayerCardMgr, cardMgr );
			OBJ_GET_EXT( m_pExt, CGameStation, gs );

			cardMgr->SetDeskCard( pOutCardInfo->outCard.bDeskStation, pOutCardInfo->outCard.iCardList, pOutCardInfo->outCard.iCardCount );
			cardMgr->SetHandCard(  pOutCardInfo->outCard.bDeskStation,pOutCardInfo->bHandCard,pOutCardInfo->iHandCardCount);
			if( 0 < pOutCardInfo->outCard.iCardCount )
			{
				LOG( "ASS_OUT_CARD_RESULT %d - %d - %d - %d", GetMeUserInfo()->bDeskStation, 
					pOutCardInfo->outCard.bDeskStation, pOutCardInfo->outCard.iCardCount, 
					cardMgr->GetHandCardCount(pOutCardInfo->outCard.bDeskStation) );
				gs->SetBigOutPeople( pOutCardInfo->outCard.bDeskStation );
				outCardResult(pOutCardInfo);
			}
		}
		return true;
	case ASS_OUT_CARD:		//用户出牌//mark
		{
			if (uDataSize != sizeof(OutCardMsg)) return false;
			
			OutCardMsg * pOutCardInfo=(OutCardMsg *)pNetData;
			OBJ_GET_EXT(m_pExt, CGameStation, gs);
			gs->SetOutCardPeople( pOutCardInfo->iNextDeskStation );

			if( pOutCardInfo->iNextDeskStation == GetMeUserInfo()->bDeskStation )
				SetMyGameTimer( pOutCardInfo->iNextDeskStation, cfg->GetValue(CFG_THINK_TIME, 5), TID_OUT_CARD );
		}
		return true;
	case ASS_NEW_TURN:		//新一轮出牌
		{
			if(uDataSize!=sizeof(NewTurnStruct)) return false;
			NewTurnStruct * pTurn = (NewTurnStruct *)pNetData; 
			if (pTurn == NULL)
			{
				return false;
			}

			OBJ_GET_EXT(m_pExt, CGameStation, gs);
			gs->SetBigOutPeople( pTurn->bDeskStation );
			gs->SetOutCardPeople( pTurn->bDeskStation );

			if( INVALID_INDEX == gs->GetBigOutPeople() )
				throw(-1);

			//删除扑克信息
			OBJ_GET_EXT( m_pExt, CPlayerCardMgr, cardMgr );
			cardMgr->ClearDeskCard();

			if ( GetMeUserInfo()->bDeskStation == pTurn->bDeskStation )
				SetMyGameTimer( pTurn->bDeskStation, cfg->GetValue(CFG_THINK_TIME, 5), TID_OUT_CARD );
		}
		return true;
	case ASS_NO_CALL_SCORE_END://无人叫分直接进入下一局
		{
			KillGameTimer(0);
			SetStationParameter(GS_WAIT_NEXT);
			ResetGameStation(0);
		}
		return true;
	case ASS_SAFE_END:	//游戏安全结束
	case ASS_CUT_END:	//用户强行离开
	case ASS_TERMINATE_END://意外结束
	case ASS_NO_CONTINUE_END://游戏结束
	case ASS_CONTINUE_END:	//游戏结束
	case ASS_AHEAD_END:
		{
			KillGameTimer(0);
			SetStationParameter(GS_WAIT_NEXT);
			ResetGameStation(0);

			if( !IsContestGame() )
				StartupGameBeginTimer();
		}
		return true;

	case ASS_CALL_SCORE_RESULT:			//叫分结果
		{
			if(uDataSize!=sizeof(RobNTStruct)) return false;
			RobNTStruct * pTurn = (RobNTStruct *)pNetData; 
			if (pTurn == NULL)
			{
				return false;
			}
			if(pTurn->byDeskStation == GetMeUserInfo()->bDeskStation)
			{
				m_bHaveRobOrCall = true;
			}
		}
		return true;
	case ASS_ROB_NT_RESULT://抢地主结果
		{
			if(uDataSize!=sizeof(CallScoreStruct)) return false;
			CallScoreStruct * pTurn = (CallScoreStruct *)pNetData; 
			if (pTurn == NULL) return false;
			if(pTurn->bDeskStation == GetMeUserInfo()->bDeskStation)
			{
				m_bHaveRobOrCall = true;
			}
		}
		return true;
	case ASS_SEND_CARD_MSG://发牌消息
	case ASS_CALL_SCORE_FINISH:
	case ASS_GAME_MULTIPLE:
	case ASS_ROB_NT_FINISH:		//抢地主结束
	case ASS_ADD_DOUBLE_RESULT://加棒结果
	case ASS_ADD_DOUBLE_FINISH:
	case ASS_SHOW_CARD_RESULT:
	case ASS_SHOW_CARD_FINISH:
	case ASS_AWARD_POINT://将分
	case ASS_AI_STATION:
	case ASS_ONE_TURN_OVER:
	case ASS_HAVE_THING:	//用户请求离开
	case ASS_LEFT_RESULT:	//请求离开结果 
	case ASS_AUTO:				//托管
	case ASS_MESSAGE:
		return true;
	}

	return __super::HandleGameMessage( pNetHead, pNetData, uDataSize, pClientSocket );
}

/// 出牌的处理，主要用于记录出牌的信息,和更正猜牌的信息
/// pOutCardInfo为所出的牌
void CClientGameDlg::outCardResult(OutCardMsg2AI * pOutCardInfo)
{
	OutCardStruct temp;
	temp.iCardCount = pOutCardInfo->outCard.iCardCount;
	memcpy(temp.iCardList,pOutCardInfo->outCard.iCardList,sizeof(temp.iCardList));
	vChuCard[pOutCardInfo->outCard.bDeskStation].push_back(temp);
	//if(pOutCardInfo->bDeskStation != GetMeUserInfo()->bDeskStation)
	//{
    //   deleteGuessCard(temp.iCardList,temp.iCardCount);
	//}

}

bool CClientGameDlg::OnControlHitBegin()
{
	if (((GetStationParameter()==GS_WAIT_SETGAME)
		||(GetStationParameter()==GS_WAIT_NEXT)
		||(GetStationParameter()==GS_WAIT_ARGEE))
		)
	{
		SendGameData( MDM_GM_GAME_NOTIFY, ASS_GM_AGREE_GAME, 0 );
	}

	KillGameTimer(0);
	ResetGameStation(0);
	return true;
}

bool CClientGameDlg::OnGameTimer(BYTE bDeskStation, UINT uTimeID, UINT uTimeCount)
{
	int iChoice = Choice_Rand;
	PreDealing(uTimeID, iChoice);

	switch( uTimeID )
	{
	case TID_BEGIN:
		{
			KillGameTimer(TID_BEGIN);
			if( (GRR_CONTEST & m_pGameInfo->dwRoomRule) || (GRR_QUEUE_GAME & m_pGameInfo->dwRoomRule) )
			{
				//OutputDebugString("dwjrobot::比赛场或者排队机 不用准备计时器-直接返回");
			}
			else
				OnHitBegin(0,0);
		}
		break;
	case TID_CALL_SCORE:
		{
			KillGameTimer( TID_CALL_SCORE );
			int iCallType = 0;
			if(iChoice == Choice_OK)
			{
				iCallType = 3;
			}
			else if(iChoice == Choice_Not)
			{
				iCallType = 0;
			}
			else
			{
				iCallType = rand() % 4;
			}
			OnCallScore( iCallType, 0 );
		}
		break;
	case TID_ROB_NT:
		{
			KillGameTimer( TID_ROB_NT );
			int iCallType = 0;
			if(iChoice == Choice_OK)
			{
				iCallType = 1;
			}
			else if(iChoice == Choice_Not)
			{
				iCallType = 0;
			}
			else
			{
				iCallType = rand() % 2;
			}
			OnRobNT( iCallType, 0 );
		}
		break;
	case TID_ADD_DOUBLE:
		{
			KillGameTimer( TID_ADD_DOUBLE );
			int iCallType = 0;
			if(iChoice == Choice_OK)
			{
				iCallType = 1;
			}
			else if(iChoice == Choice_Not)
			{
				iCallType = 0;
			}
			else
			{
				if(m_bHaveRobOrCall)
				{
					iCallType = rand() % 2;
				}
				else
				{
					iCallType = 0;
				}
				
			}
			OnAddDouble( iCallType, 1 );
		}
		break;
	case TID_SHOW_CARD:
		{
			KillGameTimer( TID_SHOW_CARD );
			OnShowCard( 0, 0 ); // 永远不亮张
		}
		break;
	case TID_OUT_CARD:
		{
			KillGameTimer( TID_OUT_CARD );
			UserOutCard();
		}
		break;
	}
	return true;
}

void CClientGameDlg::ResetGameStation(int iGameStation)
{
	ResetExt();
}

LRESULT CClientGameDlg::OnCallScore(WPARAM wparam, LPARAM lparam)
{
	KillGameTimer(TID_CALL_SCORE);
	CallScoreStruct score;
	memset(&score,0,sizeof(score));
	score.iValue = (int)wparam;								
	score.bDeskStation = GetMeUserInfo()->bDeskStation;			
	score.bCallScoreflag = false;
	SendGameData(&score,sizeof(score),MDM_GM_GAME_NOTIFY,ASS_CALL_SCORE,0);
	return 0;
}

LRESULT CClientGameDlg::OnRobNT(WPARAM wparam, LPARAM lparam)
{
	RobNTStruct robnt;
	memset(&robnt,0,sizeof(robnt));
	robnt.byDeskStation = GetMeUserInfo()->bDeskStation;		
	robnt.iValue = (int)wparam;	
	SendGameData(&robnt,sizeof(robnt),MDM_GM_GAME_NOTIFY,ASS_ROB_NT,0);
	return 0;
}

LRESULT CClientGameDlg::OnAddDouble(WPARAM wparam, LPARAM lparam)
{
	AddDoubleStruct AddDouble;
	memset(&AddDouble,0,sizeof(AddDouble));
	AddDouble.bDeskStation = GetMeUserInfo()->bDeskStation;		
	AddDouble.iValue  = (int)wparam;	
	SendGameData(&AddDouble,sizeof(AddDouble),MDM_GM_GAME_NOTIFY,ASS_ADD_DOUBLE,0);
	return 0;
}

LRESULT CClientGameDlg::OnShowCard(WPARAM wparam, LPARAM lparam)
{
	ShowCardStruct show;
	memset(&show,0,sizeof(show));
	show.bDeskStation = GetMeUserInfo()->bDeskStation;		
	show.iValue  = (int)wparam;	
	SendGameData(&show,sizeof(show),MDM_GM_GAME_NOTIFY,ASS_SHOW_CARD,0);
	return 0;
}

LRESULT	CClientGameDlg::OnHitBegin(WPARAM wparam, LPARAM lparam)
{
	OnControlHitBegin();
	return 0;
}

void CClientGameDlg::StartupGameBeginTimer()
{
	OBJ_GET_EXT( m_pExt, CCfg, cfg );
	SetMyGameTimer( GetMeUserInfo()->bDeskStation, cfg->GetValue(CFG_BEGIN_TIME, 5), TID_BEGIN);
}

void CClientGameDlg::OnControlHitPass(void)
{
	KillAllTimer();
	//设置数据
	OBJ_GET_EXT(m_pExt, CGameStation, gs);
	gs->SetOutCardPeople( INVALID_INDEX );
	//发送数据
	OutCardStruct OutCard;
	memset(OutCard.iCardList, 0, sizeof(OutCard.iCardList));
	OutCard.iCardCount=0;
	SendGameData(&OutCard,sizeof(OutCard),MDM_GM_GAME_NOTIFY,ASS_OUT_CARD,0);
}

//牌的列表赋值到T_C2S_PLAY_CARD_REQ上
void CClientGameDlg::transformPlayCard(const Card *pCard, int nCardCount, T_C2S_PLAY_CARD_REQ& tLastCard)
{
	
	OBJ_GET_EXT(m_pExt, CUpGradeGameLogic, logic);
	//获取上一手牌的类型
	BYTE nShape = logic->GetCardShape( pCard, nCardCount );
	//赋值到tLastCard上//转换函数
	ShapeAdapter(nShape, tLastCard.eArrayType);
	
	for(int i = 0; i < nCardCount; ++i)
	{
		tLastCard.uCards[i] = pCard[i];
		tLastCard.uActualCards[i] = pCard[i];
	}
	tLastCard.iCardCount = nCardCount;
}

void CClientGameDlg::GetOutCardCount(int* iUserCardCounts)
{
	for(int i = 0; i < PLAY_COUNT; ++i)
	{
		iUserCardCounts[i] = 0;
		for(int j = 0; j < (int)vChuCard[i].size(); j++)
		{
			iUserCardCounts[i] += vChuCard[i][j].iCardCount;
		}
	}
}

//对应的转换函数
void CClientGameDlg::ShapeAdapter(BYTE &byShape, EArrayType &eArrayType)
{
	switch(byShape)
	{
	case UG_ONLY_ONE:
		eArrayType = ARRAY_TYPE_SINGLE;
		break;
	case UG_DOUBLE:
		eArrayType = ARRAY_TYPE_DOUBLE;
		break;
	case UG_THREE:
		eArrayType = ARRAY_TYPE_3W_;
		break;
	case UG_THREE_ONE:
	case UG_THREE_TWO:
		eArrayType = ARRAY_TYPE_3W1_ONE;
		break;
	case UG_THREE_DOUBLE:
		eArrayType = ARRAY_TYPE_3W1_DOUBLE;
		break;
	case UG_THREE_SEQUENCE:
		eArrayType = ARRAY_TYPE_PLANE_;
		break;
	case UG_THREE_ONE_SEQUENCE:
		eArrayType = ARRAY_TYPE_PLANE_ONE;
		break;
	case UG_THREE_TWO_SEQUENCE:
	case UG_THREE_DOUBLE_SEQUENCE:
		eArrayType = ARRAY_TYPE_PLANE_DOUBLE;
		break;
	case UG_STRAIGHT:
		eArrayType = ARRAY_TYPE_STRAIGHT_ONE;
		break;
	case UG_DOUBLE_SEQUENCE:
		eArrayType = ARRAY_TYPE_STRAIGHT_DOUBLE;
		break;
	case UG_FOUR_ONE:
	case UG_FOUR_TWO:
		eArrayType = ARRAY_TYPE_4w2_ONE;
		break;
	case UG_FOUR_TWO_DOUBLE:
	case UG_FOUR_ONE_DOUBLE:
		eArrayType = ARRAY_TYPE_4w2_DOUBLE;
		break;
	/*case UG_ONLY_ONE:
		eArrayType = ARRAY_TYPE_SBOMB;*/
	case UG_BOMB:
		eArrayType = ARRAY_TYPE_HBOMB;
		break;
	case UG_KING_BOMB:
		eArrayType = ARRAY_TYPE_WBOMB;
		break;
		/*case UG_ONLY_ONE:
		eArrayType = ARRAY_TYPE_4L;
		break;
		case UG_ONLY_ONE:
		eArrayType = ARRAY_TYPE_LBOMB_S;
		break;
		case UG_ONLY_ONE:
		eArrayType = ARRAY_TYPE_LBOMB_L;*/
		break;
	default:
		eArrayType = ARRAY_TYPE_ERROR;
		break;
	}
}

//玩家出牌//mark
void CClientGameDlg::UserOutCard()
{
	OutCardStruct outCard;
	memset( &outCard, 0, sizeof(outCard) );

	OBJ_GET_EXT(m_pExt, CGameStation, gs);
	OBJ_GET_EXT(m_pExt, CPlayerCardMgr, cardMgr);
	OBJ_GET_EXT(m_pExt, CUpGradeGameLogic, logic);
	const int nMyStation = GetMeUserInfo()->bDeskStation;
	const int nBigOutStation = gs->GetBigOutPeople();
	const int nNtStation = gs->GetNtPeople();
	bool bFirstOut = (nMyStation == nBigOutStation);

	//获取手牌
	int nHandCardCount;
	const Card * pHandCard = cardMgr->GetHandCard( nMyStation, &nHandCardCount );

	//记录上一手出的牌
	int nDeskCardCount;
	const Card * pDeskCard = cardMgr->GetDeskCard( nBigOutStation, &nDeskCardCount );

	//20190427
	set<int> playerHandCard;
	set<int> deskHandCard;

	//手牌转换
	for (int i = 0; i < nHandCardCount; ++i)
	{
		playerHandCard.insert(this->bytetoint(pHandCard[i]));
	}
	for (int i = 0; i < nDeskCardCount; ++i)
	{
		deskHandCard.insert(this->bytetoint(pDeskCard[i]));
	}
	/*
	FILE * fp = fopen("log.txt", "a");
	fprintf(fp, "2:%d 小王:%d 大王:%d\n", playerHandCard[2], playerHandCard[14], playerHandCard[15]);
	fclose(fp);
	*/

	//是否是庄家第一次出牌（首出）
	if(nMyStation == nNtStation && bFirstOut && nHandCardCount == ONE_HAND_CARD_COUNT)
	{
		g_bFirstTimeNtOut = true;
	}
	else
	{
		g_bFirstTimeNtOut = false;
	}

	HN::CardArrayBase HandCard;
	for(int i = 0; i < nHandCardCount; ++i)
	{
		BYTE byCard = pHandCard[i];
		//获取手牌的值(转换)
		logic->CardAdapter(byCard, true);
		HandCard.push_back(byCard);
	}

	T_C2S_PLAY_CARD_REQ tLastCard ;
	//将pDeskCard赋值到tLastCard上(封装)
	transformPlayCard(pDeskCard, nDeskCardCount, tLastCard);
	for(int i = 0; i < tLastCard.iCardCount; ++i)
	{
		logic->CardAdapter(tLastCard.uCards[i], true);
		logic->CardAdapter(tLastCard.uActualCards[i], true);
	}

	std::vector<T_C2S_PLAY_CARD_REQ>  tPlayCardList; 

	if (bFirstOut) HandcardtoList(playerHandCard);
	else {};
	//是否主动出牌
	if(bFirstOut)
	{
		//主动出牌不需要管桌上的牌
		_dataMange.getPickUpOutAll(HandCard,tPlayCardList);
	}
	else
	{
		//跟牌
		_dataMange.getPickUpFollowAll(HandCard, tLastCard, tPlayCardList);
	}
	
	/// 结果赋值//可出牌 牌堆
	T_S2C_PROMPT_CARD_RES m_sPlayCard;
	for(int i = 0;i < ONE_HAND_CARD_COUNT && i < tPlayCardList.size();i++ )
	{
		m_sPlayCard.sCards[i] = tPlayCardList[i];
		m_sPlayCard.iCardCount = i+1;
	}
	
	if(m_sPlayCard.iCardCount == 0)										/// 过
	{
		OnControlHitPass();
	}
	else
	{
		T_C2S_PLAY_CARD_REQ res;
		SGetPlayCardparam tParam;
		BYTE byShape = logic->GetCardShape(pDeskCard, nDeskCardCount);
		ShapeAdapter(byShape, tParam.iDeskShape);
		tParam.bOutCard = bFirstOut;
		tParam.iBanker = nNtStation;
		tParam.iLastOutCardUser = nBigOutStation;
		tParam.iMybSeatNO = nMyStation;
		int outCardCount[PLAY_COUNT] = {0};
		GetOutCardCount(outCardCount);
		for(int i = 0; i < PLAY_COUNT; ++i)
		{
			tParam.pHandCard[i] = cardMgr->GetHandCard( nMyStation, &tParam.iUserCardCounts[i] );
		}
		/// 挑选出最优的出牌
		logic->GetOptimalPlayCard(tParam, m_sPlayCard, res);
		for(int i = 0; i < res.iCardCount; ++i)
		{
			logic->CardAdapter(res.uCards[i], false);
			logic->CardAdapter(res.uActualCards[i], false);
		}
		if( res.iCardCount == 0)
		{
			OnControlHitPass();
		}
		else
		{
			OutCardStruct outCard;
			memset( &outCard, 0, sizeof(outCard) );
			outCard.iCardCount = res.iCardCount;
			memcpy(outCard.iCardList, res.uCards, outCard.iCardCount);
			SendGameData( &outCard, sizeof(outCard), MDM_GM_GAME_NOTIFY, ASS_OUT_CARD, 0 );
			gs->SetOutCardPeople( INVALID_INDEX );
		}
	}
}

int CClientGameDlg::AI_DaTongHuo(OutCardStruct & outCard)
{
	OBJ_GET_EXT(m_pExt, CGameStation, gs);
	OBJ_GET_EXT(m_pExt, CPlayerCardMgr, cardMgr);
	OBJ_GET_EXT(m_pExt, CUpGradeGameLogic, logic);

	const int nMyStation = GetMeUserInfo()->bDeskStation;
	const int nBigOutStation = gs->GetBigOutPeople();
	const int nNtStation = gs->GetNtPeople();
	bool bFirstOut = (nMyStation == nBigOutStation);

	int nHandCardCount;
	const Card * pHandCard = cardMgr->GetHandCard( nMyStation, &nHandCardCount );
	int nDeskCardCount;
	const Card * pDeskCard = cardMgr->GetDeskCard( nBigOutStation, &nDeskCardCount );

	if( NULL == pHandCard || 0 >= nHandCardCount )
	{
		LOG(_T("用户数据出问题了"));
		throw( -1 );
	}

	if( NULL == pDeskCard || 0 >= nDeskCardCount )
		return 0;

	outCard.iCardCount = 0;

	Card pTempCardList[MAX_CARD_BUFFER];
	int nTempCardCount;
	
	int nShape = logic->GetCardShape( pDeskCard, nDeskCardCount );
	Card bySplit = 0;
	int nSplitCount = 0;
	if( UG_ONLY_ONE == nShape )
	{
		bySplit = 10;
		nSplitCount = 1;
	}
	else if( UG_DOUBLE == nShape )
	{
		bySplit = 7;
		nSplitCount = 2;
	}

	if( 0 != nSplitCount && bySplit > logic->GetCardBulk(*pDeskCard) )
	{
		memcpy_s( pTempCardList, MAX_CARD_BUFFER * sizeof(Card), pHandCard, nHandCardCount );
		nTempCardCount = nHandCardCount;
		do {
			logic->AutoOutCard( pTempCardList, nTempCardCount, pDeskCard, nDeskCardCount
				, outCard.iCardList, outCard.iCardCount, bFirstOut );
			if( 0 < outCard.iCardCount )
			{
				nTempCardCount -= logic->RemoveCard( outCard.iCardList, outCard.iCardCount, 
					pTempCardList, nTempCardCount );
				if( 1 == outCard.iCardCount && 15 > logic->GetCardBulk(*outCard.iCardList)
					&& bySplit <= logic->GetCardBulk(*outCard.iCardList) )
					break;
				if( 0 == nTempCardCount )
				{
					logic->AutoOutCard( pHandCard, nHandCardCount, pDeskCard, nDeskCardCount
						, outCard.iCardList, outCard.iCardCount, bFirstOut );
					if( nSplitCount != outCard.iCardCount || 15 <= logic->GetCardBulk(*outCard.iCardList) )
						outCard.iCardCount = 0;
					break;
				}
			}
			else if( 0 == outCard.iCardCount )
			{
				logic->AutoOutCard( pHandCard, nHandCardCount, pDeskCard, nDeskCardCount
					, outCard.iCardList, outCard.iCardCount, bFirstOut );
				if( nSplitCount != outCard.iCardCount || 15 <= logic->GetCardBulk(*outCard.iCardList) )
					outCard.iCardCount = 0;
				break;
			}
		}while( true );
	}

	return outCard.iCardCount;
}

void CClientGameDlg::PreDealing(UINT uTimeID, int& iChoice)
{
	OBJ_GET_EXT(m_pExt, CUpGradeGameLogic, logic);
	OBJ_GET_EXT(m_pExt, CPlayerCardMgr, cardMgr);
	const int nMyStation = GetMeUserInfo()->bDeskStation;

	int nHandCardCount;
	const Card * pHandCard = cardMgr->GetHandCard( nMyStation, &nHandCardCount );
	std::vector<T_C2S_PLAY_CARD_REQ>  tPlayCardList; 
	HN::CardArrayBase HandCard;
	for(int i = 0; i < nHandCardCount; ++i)
	{
		BYTE byCard = pHandCard[i];
		logic->CardAdapter(byCard, true);
		HandCard.push_back(byCard);
	}


	switch( uTimeID )
	{
	case TID_CALL_SCORE:
	case TID_ROB_NT:
	case TID_ADD_DOUBLE:
		if(logic->HaveKingBomb(pHandCard, nHandCardCount))									iChoice = Choice_OK;
		if(logic->GetOneCardCount(pHandCard, nHandCardCount, Value_A) >= 4)					iChoice = Choice_OK;
		if(logic->GetOneCardCount(pHandCard, nHandCardCount, Value_2) >= 3)					iChoice = Choice_OK;
		if(logic->GetSpecifyCardCount(pHandCard, nHandCardCount, 4) >= 2)					iChoice = Choice_OK;
		if( (logic->HaveKing(pHandCard, nHandCardCount, true) || logic->HaveKing(pHandCard, nHandCardCount, false))
			&& ( logic->GetOneCardCount(pHandCard, nHandCardCount, Value_A) + logic->GetOneCardCount(pHandCard, nHandCardCount, Value_2) >= 4 )
			)																				iChoice = Choice_OK;


		if(Choice_OK != iChoice)
		{
			if(_dataMange.CheckType(HandCard, SanZhangCheck) || _dataMange.CheckType(HandCard, NoSanPaiCheck))
			{
				if(TID_CALL_SCORE == uTimeID || TID_ROB_NT == uTimeID)
				{
					iChoice = Choice_OK;
				}
			}

			if(_dataMange.CheckType(HandCard, SanZhang_Double) ||_dataMange.CheckType(HandCard, A2King_Double) || _dataMange.CheckType(HandCard, DanZhang_Double))
			{
				if(TID_ADD_DOUBLE == uTimeID)
				{
					iChoice = Choice_OK;
				}
			}
		}

		if(Choice_OK != iChoice)
		{
			if(!_dataMange.CheckType(HandCard, SanPai_Not))
			{
				iChoice = Choice_Not;
			}
		}
		break;
	}
}

bool CClientGameDlg::SetMyGameTimer(BYTE bDeskStation,UINT uTimeCount,UINT uTimeID)
{
	if(uTimeCount >= 5)
	{
		uTimeCount = (rand() + bDeskStation) % m_iDefaultTime + 1;
	}
	else if(uTimeCount >= 1)
	{
		uTimeCount = (rand() + bDeskStation) % uTimeCount + 1;
	}
	else
	{
		uTimeCount = (rand() + bDeskStation) % m_iDefaultTime + 1;
	}
	return SetGameTimer(bDeskStation, uTimeCount, uTimeID);
}


int CClientGameDlg::bytetoint(BYTE cardValue)
{
	//参考
	//0x01, 0x02 ,0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,		//方块 2 - A
	//0x11, 0x12 ,0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,		//梅花 2 - A
	//0x21, 0x22 ,0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D,		//红桃 2 - A
	//0x31, 0x32 ,0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D,		//黑桃 2 - A
	//0x4E, 0x4F	//小鬼，大鬼

	/*
	//不算花色
	if (huase == false)
	{
		if (cardValue == 0x4E) return 16;
		if (cardValue == 0x4F) return 17;

		if (cardValue & 0x0F == 1) return 14;
		if (cardValue & 0x0F == 2) return 15;
		return  cardValue & 0x0F;
	}
	//算花色
	else
	{

		//if (byCard % 16 == 1) byCard = byCard / 16 * 16 + 13;
		//else byCard -= 1;
		int value = cardValue & 0x0f - 1;
		int hua = (cardValue & 0xf0)>>4;

		if (value == 0) value = 13;
		return hua * 13 + value;
	}

	*/
	if (cardValue == 0x4E) return 52;
	if (cardValue == 0x4F) return 53;
	int value = cardValue & 0x0f;
	int hua = (cardValue & 0xf0) >> 4;
	switch (value)
	{
	case 1:return 48 + hua;
	case 2:return 0 + hua;
	case 3:return 4 + hua;
	case 4:return 8 + hua;
	case 5:return 12 + hua;
	case 6:return 16 + hua;
	case 7:return 20 + hua;
	case 8:return 24 + hua;
	case 9:return 28 + hua;
	case 10:return 32 + hua;
	case 11:return 36 + hua;
	case 12:return 40 + hua;
	case 13:return 44 + hua;
	default:return -1;
	}
}

void CClientGameDlg::freezoro(map<int, int> &m)
{
	bool notcomplete = true;
	while (notcomplete) {
		notcomplete = false;
		auto b = m.begin();
		for (; b != m.end(); ++b) {
			if (b->second == 0) {
				m.erase(b);
				notcomplete = true;
				break;
			}
		}
	}
}

int CClientGameDlg::ValueToNum(set<int> &cardscopy, int value)
{
	if (value < 3 || value>17 || cardscopy.empty())
		throw runtime_error("Value not in set!");

	if (value == 16 && cardscopy.find(52) != cardscopy.end()) {
		cardscopy.erase(52);
		return 52;
	}
	else if (value == 17 && cardscopy.find(53) != cardscopy.end()) {
		cardscopy.erase(53);
		return 53;
	}
	else {
		for (int i = (value - 3) * 4, j = 0; j < 4; ++j) {
			if (cardscopy.find(i + j) != cardscopy.end()) {
				cardscopy.erase(i + j);
				return i + j;
			}

		}
		throw runtime_error("Value not in set!");
	}
}


bool CClientGameDlg::HandcardtoList(const set<int> Handcard)
{
	bool result = false;
	if (Handcard.empty()) return result;

	set<int> cardscopy(Handcard);//手牌副本
	map<int, int> needanalyse;

	for(auto it = cardscopy.begin();it!= cardscopy.end();++it)
		++needanalyse[Ecardgroup::Translate(*it)];//根据手牌构造待分析集合
	
	//王炸
	if (needanalyse.find(14) != needanalyse.end() &&
		needanalyse.find(15) != needanalyse.end())
	{
		//todo需要找 结构体存放
		Ecardgroup *c =new Ecardgroup(cardgroupisBomb, 17);
		for (int i = 14; i < 16; ++i)
		{
			c->AddNumber(ValueToNum(cardscopy, i));
			needanalyse.erase(i);
		}
		analyse.push_back(c);
	}

	//炸弹
	for (auto mem = needanalyse.begin();mem!= needanalyse.end();mem++)
	{
		if (mem->second == 4) 
		{	

			Ecardgroup *c = new Ecardgroup(cardgroupisBomb, mem->first);
			for (int i = 0; i < 4; ++i) 
			{
				c->AddNumber(ValueToNum(cardscopy, mem->first));
			}
			analyse.push_back(c);

			//标记
			needanalyse[mem->first] = 0;
		}
	}
	
	//todo 清除标记为0的
	freezoro(needanalyse);

	//处理2
	if (needanalyse.find(2) != needanalyse.end())
	{
		Ecardgroup *c = new Ecardgroup(cardgroupisUnkown, 15);
		int n = needanalyse[2];
		switch (n) {
		case 3:
			c->type = cardgroupisThree;
			break;
		case 2:
			c->type = cardgroupisDouble;
			break;
		case 1:
			c->type = cardgroupisSingle;
			break;
		}
		for (int i = 0; i < n; ++i)
			;
		c->AddNumber(ValueToNum(cardscopy, 15));
		needanalyse.erase(2);
		analyse.push_back(c);
	}

	//单顺
	int begin, n;
	bool exist = true;
	while (exist && needanalyse.size())
	{
		begin = n = 0;
		for (auto b = needanalyse.begin(); b != needanalyse.end(); ++b)
		{
			if (b->second > 0)
			{//跳过为零的元素
				if (!begin)
					begin = b->first;
				if (begin == b->first)
					++n;
				++begin;//这个是标记
			}
			if (n == 5)
			{//满足组成单顺的数量
				auto p = b;
				int first = p->first - 4;//单顺的第一个//最小的数
				Ecardgroup *c = new Ecardgroup(cardgroupisSingleSeq, p->first);
				for (first; first <= p->first; ++first) {
					c->AddNumber(ValueToNum(cardscopy, first));
					--needanalyse[first];//减一
				}
				analyse.push_back(c);
				exist = true;
				break;//从开始重新查找
			}
			//连续牌面数量小于五个，重新计数；或者已到集合最后数量仍不满足
			auto end = needanalyse.end();
			if (begin - 1 != b->first || b == --end)
			{
				if (b->second > 0)
				{
					begin = b->first;
					++begin;
					n = 1;
				}
				else
					begin = n = 0;
				exist = false;
			}

		}
	}

	//todo 清标记为0的牌
	freezoro(needanalyse);

	//找更大的单顺.5以上
	for (auto mem = analyse.begin();mem!=analyse.end();++mem)
	{
		if ((*mem)->type == cardgroupisSingleSeq)
		{//针对每个单顺
			for (auto m = needanalyse.begin();m!= needanalyse.end();++m)
			{
				if (m->second > 0 && m->first == (*mem)->value + 1)
				{//剩余牌中还有比单顺最大大一的牌
					(*mem)->AddNumber(ValueToNum(cardscopy, m->first));
					++(*mem)->value;
					--needanalyse[m->first];

				}
			}
		}
	}

	freezoro(needanalyse);

	
	//有两个相同的单顺组合一起
		//如现有单顺中有可以对接成更长的单顺；或两个单顺元素相同，组合成双顺
	for (auto mem1 = analyse.begin();mem1!=analyse.end();++mem1)
	{
		if ((*mem1)->type == cardgroupisSingleSeq)
		{//单顺1
			for (auto mem2 = analyse.begin();mem2 != analyse.end();++mem2)
			{
				if ((*mem2)->type == cardgroupisSingleSeq && (*mem1) != (*mem2))
				{//单顺2，且和单顺1不是同一个
					if ((*mem1)->value < (*mem2)->value)
					{//mem1在前
						if ((*mem1)->value == (*mem2)->value - (*mem2)->count)
						{//可以拼接
							for (auto m = (*mem2)->cards.begin();m!=(*mem2)->cards.end();++(*mem2))
								(*mem1)->AddNumber(*m);
							(*mem1)->value = (*mem2)->value;
							(*mem2)->type = cardgroupisUnkown;
						}
					}
					else if ((*mem1)->value > (*mem2)->value) {//mem1在后
						if ((*mem2)->value == (*mem1)->value - (*mem1)->count)
						{
							for (auto m = (*mem1)->cards.begin();m!= (*mem1)->cards.end();++m)
								(*mem2)->AddNumber(*m);
							(*mem2)->value = (*mem1)->value;
							(*mem1)->type = cardgroupisUnkown;
						}
					}
					else {//测试是否完全一样，可以合并成双顺
						if ((*mem1)->count == (*mem2)->count)
						{
							for (auto m = (*mem2)->cards.begin();m!= (*mem2)->cards.end();++m)
								(*mem1)->AddNumber(*m);
							(*mem1)->type = cardgroupisDoubleSeq;
							(*mem2)->type = cardgroupisUnkown;
						}
					}
				}
			}
		}
	}

	//检查一波手牌是否还需要继续

	//查找双顺
	//双顺，只查找数量大于等于2的连续牌，并且3个以上相连
	begin = n = 0;
	auto last = --needanalyse.end();
	for (auto b = needanalyse.begin(); b != needanalyse.end(); ++b)
	{
		if (b->second >= 2)
		{
			if (!begin)
				begin = b->first;
			if (begin == b->first)
				++n;
			++begin;
		}
		if (begin && begin - 1 != b->first || b == last) {//出现与之前不连续的,或已到集合最后
			if (n >= 3)
			{
				auto p = b;
				if (begin - 1 != b->first)
					--p;
				Ecardgroup *c = new Ecardgroup(cardgroupisDoubleSeq, p->first);
				for (int i = n; i > 0; --i, --p) 
				{
					for (int j = 0; j < 2; ++j) 
					{
						c->AddNumber(ValueToNum(cardscopy, p->first));
						--p->second;
					}
				}
				analyse.push_back(c);
			}
			if (b->second >= 2)
			{
				n = 1;//当前分析牌是两张以上的
				begin = b->first;
				++begin;
			}
			else 
			{
				n = 0;
				begin = 0;
			}
		}
	}

	freezoro(needanalyse);

	//三顺
	//查找是否有重合的单顺和双顺组合成三顺
	for (auto mem1 = analyse.begin();mem1!=analyse.end();mem1++)
	{
		if ((*mem1)->type == cardgroupisSingleSeq)
		{
			for (auto mem2 = analyse.begin();mem2 != analyse.end();mem2++)
			{
				if ((*mem2)->type == cardgroupisDoubleSeq)
				{
					if ((*mem1)->value == (*mem2)->value && (*mem1)->count * 2 == (*mem2)->count)
					{
						for (auto m = (*mem1)->cards.begin();m!=(*mem1)->cards.end();++m)
							(*mem2)->AddNumber(*m);
						(*mem2)->type = cardgroupisThreeSeq;
						(*mem1)->type = cardgroupisUnkown;
					}
				}
			}
		}
	}

	//检查手牌是否还需要继续


	begin = n = 0;
	last = --needanalyse.end();
	for (auto b = needanalyse.begin(); b != needanalyse.end(); ++b)
	{
		if (b->second == 3)
		{
			if (!begin)
				begin = b->first;
			if (begin == b->first)
				++n;
			++begin;
		}
		if (begin && begin - 1 != b->first || b == last) 
		{//出现与之前不连续的,或已到集合最后
			if (n >= 2)
			{//存在2组及以上
				auto p = b;
				if (begin - 1 != b->first)
					--p;
				Ecardgroup *c = new Ecardgroup(cardgroupisThreeSeq, p->first);
				for (int i = n; i > 0; --i, --p)
				{
					for (int j = 0; j < 3; ++j)
					{
						c->AddNumber(ValueToNum(cardscopy, p->first));
						--p->second;
					}
				}
				analyse.push_back(c);
				if (b->second == 3)
				{//当前分析牌为3张，
					n = 1;
					begin = b->first;
					++begin;
				}
				else
				{
					n = 0;
					begin = 0;
				}
			}
		}
	}

	//三条
	for (auto mem = needanalyse.begin();mem!= needanalyse.end();++mem)
	{
		if (mem->second == 3) 
		{
			Ecardgroup *c = new Ecardgroup(cardgroupisThree, mem->first);
			for (int i = 0; i < 3; ++i)
				c->AddNumber(ValueToNum(cardscopy, mem->first));
			needanalyse[mem->first] = 0;
			analyse.push_back(c);
		}
	}

	//对子
	for (auto mem = needanalyse.begin();mem!= needanalyse.end();++mem)
	{
		if (mem->second == 2) 
		{
			Ecardgroup *c = new Ecardgroup(cardgroupisDouble, mem->first);
			for (int i = 0; i < 2; ++i)
				c->AddNumber(ValueToNum(cardscopy, mem->first));
			needanalyse[mem->first] = 0;
			analyse.push_back(c);
		}
	}

	freezoro(needanalyse);

	//单牌
	for (auto mem = needanalyse.begin();mem!= needanalyse.end();++mem)
	{
		if (mem->second != 1)
			throw runtime_error("Still has singleness card");
		Ecardgroup *c = new Ecardgroup(cardgroupisSingle, mem->first);
		c->AddNumber(ValueToNum(cardscopy, mem->first));
		needanalyse[mem->first] = 0;
		analyse.push_back(c);
	}


	freezoro(needanalyse);

	DeleteUnkown();
	//fixme 
	sort(analyse.begin(), analyse.end());
	//sort(analyse.begin(), analyse.end(), MyCompare);
}


void CClientGameDlg::DeleteUnkown(void)
{
	auto b = analyse.begin();
	while (b != analyse.end()) {
		if ((*b)->type == cardgroupisUnkown) {
			delete *b;
			b = analyse.erase(b);
		}
		else
			++b;
	}
}

bool CClientGameDlg::MyCompare(Ecardgroup *c1, Ecardgroup *c2)
{
	if (c1->type != c2->type)
		return c1->type < c2->type;
	else
		return c1->value < c2->value;
}

void CClientGameDlg::sandaiXorfeijidaiX()
{
	int n,
		doublecount = 0,//统计对子的数量，方便下面的整合
		singlecount = 0;//统计单张数量

	for (auto mem = analyse.begin(); mem != analyse.end();++mem)
	{
		if ((*mem)->type == cardgroupisSingle)
			++singlecount;
		else if ((*mem)->type == cardgroupisDouble)
			++doublecount;
	}

	for (auto mem = analyse.begin();mem!= analyse.end();++mem)
	{//完善飞机
		if ((*mem)->type == cardgroupisThreeSeq)
		{
			n = (*mem)->count / 3;
			if (singlecount >= n)
			{
				for (auto temp = analyse.begin();temp!= analyse.end();++temp)
				{
					if ((*temp)->type == Single)
					{
						for (auto m = (*temp)->cards.begin();m!= (*temp)->cards.end();++m)
							(*mem)->AddNumber(*m);
						(*temp)->type = cardgroupisUnkown;
						--singlecount;
						--n;
					}
					if (!n)
					{
						(*mem)->type = cardgroupisAirplane;
						break;
					}
				}
			}
			else if (doublecount >= n)
			{
				for (auto temp = analyse.begin();temp!=analyse.end();++temp)
				{
					if ((*temp)->type == cardgroupisDouble)
					{
						for (auto m = (*temp)->cards.begin();m!= (*temp)->cards.end();++m)
							(*mem)->AddNumber(*m);
						(*temp)->type = cardgroupisUnkown;
						--doublecount;
						--n;
					}
					if (!n)
					{
						(*mem)->type = cardgroupisAirplane;
						break;
					}
				}
			}
		}
	}
	for (auto mem = analyse.begin();mem!= analyse.end();++mem)
	{//完善三带一
		if ((*mem)->type == cardgroupisThree)
		{
			if (singlecount)
			{
				for (auto temp = analyse.begin();temp!= analyse.end();temp++)
				{
					if ((*temp)->type == Single)
					{
						for (auto m = (*temp)->cards.begin();m!= (*temp)->cards.end();++m)
							(*mem)->AddNumber(*m);
						(*temp)->type = cardgroupisUnkown;
						--singlecount;
						(*mem)->type = cardgroupisThreePlus;
						break;
					}
				}
			}
			else if (doublecount)
			{
				for (auto temp = analyse.begin();temp!= analyse.end();++temp)
				{
					if ((*temp)->type == cardgroupisDouble)
					{
						for (auto m = (*temp)->cards.begin();m!=(*temp)->cards.end();++m)
							(*mem)->AddNumber(*m);
						(*temp)->type = cardgroupisUnkown;
						--doublecount;
						(*mem)->type = cardgroupisThreePlus;
						break;
					}
				}
			}
		}
	}
}

void CClientGameDlg::BankeroutCard(bool isnormal)
{
	this->canoutCard.Clear();
	if (isnormal)
	{

	}
	else
	{

	}
}

//都是主动出牌
void CClientGameDlg::frameroutCard(int nexthandcard, bool isfriend)
{
	this->canoutCard.Clear();
	if (analyse.size == 0) return;
	if (isfriend)
	{
		if (nexthandcard>2)
		{
			for (auto it = analyse.begin(); it != analyse.end(); ++it)
			{
				//飞机
				if ((*it)->type == cardgroupisAirplane)
				{
					canoutCard = (*(*it));
					return;
				}
				//三顺
				else if ((*it)->type == cardgroupisThreeSeq)
				{
					canoutCard = (*(*it));
					return;
				}
				//双顺
				else if ((*it)->type == cardgroupisDoubleSeq)
				{
					canoutCard = (*(*it));
					return;
				}
				//三带X
				else if ((*it)->type == cardgroupisThreePlus)
				{
					canoutCard = (*(*it));
					return;
				}
				//单顺
				else if ((*it)->type == cardgroupisSingleSeq)
				{
					canoutCard = (*(*it));
					return;
				}
				//三条
				else if ((*it)->type == cardgroupisThree)
				{
					canoutCard = (*(*it));
					return;
				}
				//对子
				else if ((*it)->type == cardgroupisDouble)
				{
					canoutCard = (*(*it));
					return;
				}
				//单子
				else if ((*it)->type == cardgroupisSingle)
				{
					canoutCard = (*(*it));
					return;
				}
				
			}
		}
		//队友的牌小于等于2
		else
		{
			for (auto it = analyse.begin(); it != analyse.end(); ++it)
			{
				if ((*it)->count > nexthandcard)
				{
					canoutCard = (*(*it));
					return;
				}
			}
		}
	}
	else
	{
		if (nexthandcard>2)
		{
			for (auto it = analyse.begin(); it != analyse.end(); ++it)
			{
				//飞机
				if ((*it)->type == cardgroupisAirplane)
				{
					canoutCard = (*(*it));
					return;
				}
				//三顺
				else if ((*it)->type == cardgroupisThreeSeq)
				{
					canoutCard = (*(*it));
					return;
				}
				//双顺
				else if ((*it)->type == cardgroupisDoubleSeq)
				{
					canoutCard = (*(*it));
					return;
				}
				//三带X
				else if ((*it)->type == cardgroupisThreePlus)
				{
					canoutCard = (*(*it));
					return;
				}
				//单顺
				else if ((*it)->type == cardgroupisSingleSeq)
				{
					canoutCard = (*(*it));
					return;
				}
				//三条
				else if ((*it)->type == cardgroupisThree)
				{
					canoutCard = (*(*it));
					return;
				}
				//对子
				else if ((*it)->type == cardgroupisDouble)
				{
					canoutCard = (*(*it));
					return;
				}
				//单子
				else if ((*it)->type == cardgroupisSingle)
				{
					canoutCard = (*(*it));
					return;
				}
			}
		}
		else
		{
			for (auto it = analyse.begin(); it != analyse.end(); ++it)
			{
				if ((*it)->count > nexthandcard)
				{
					canoutCard = (*(*it));
					return;
				}
				//todo牌数小于或等于2
				//拆牌出大的
			}
		}
	}
}

void CClientGameDlg::ClearAnalyse()
{
	if (analyse.empty())
		return;

	for (auto mem = analyse.begin();mem!= analyse.end();++mem)
		delete *mem;
	analyse.clear();

	return;
}






