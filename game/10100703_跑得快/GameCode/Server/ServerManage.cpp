#include "StdAfx.h"
#include "ServerManage.h"
#include "writelog.h"
#include <math.h>
#include "../../GameCode/CfgCard/CfgCard.h"
//游戏配置

bool CServerGameDesk:: m_bHaveKing;
bool CServerGameDesk:: m_bRobnt;
bool CServerGameDesk:: m_bAdddouble;
bool CServerGameDesk:: m_bShowcard;
UINT CServerGameDesk:: m_iPlayCard;
UINT CServerGameDesk:: m_iBackCount;
UINT CServerGameDesk:: m_iUserCount;


BOOL CServerGameDesk:: m_bKingCanReplace;
DWORD CServerGameDesk::m_iCardShape;
//游戏扩展配置
BYTE CServerGameDesk:: m_iThinkTime;
BYTE CServerGameDesk:: m_iBeginTime;
BYTE CServerGameDesk:: m_iSendCardTime ; 
BOOL CServerGameDesk:: m_bTurnRule;		//顺时针还是逆时针
BOOL CServerGameDesk:: m_bIsAutoOutCard;
BYTE CServerGameDesk::m_FirstNecessaryCard;
int CServerGameDesk::m_AutoOutCardTime;
BYTE CServerGameDesk::m_MonkeyCard;

BYTE CServerGameDesk::m_bHaveMonkey;

int CServerGameDesk:: m_iOutTimeAutoCount;
int CServerGameDesk::m_ZhaDanDiFen;
int CServerGameDesk::m_iBasePoint;

void DebugPrintf(int iRoomID,const char *p, ...)
{
	char szFilename[256];
	sprintf( szFilename, "%s\\Log\\%d_%dServer.txt", CINIFile::GetAppPath (),NAME_ID,iRoomID);

	FILE *fp = fopen( szFilename, "a" );
	if (NULL == fp)
	{
		return;
	}
	va_list arg;
	va_start( arg, p );
	vfprintf(fp, p, arg );
	fprintf(fp,"\n");
	fclose(fp);
}
//构造函数
CServerGameDesk::CServerGameDesk(void):CGameDesk(FULL_BEGIN)
{
	//c++ test
	::ZeroMemory(m_iUserCardCount,sizeof(m_iUserCardCount));
	::ZeroMemory(m_iUserCard,sizeof(m_iUserCard));
	::ZeroMemory(m_iBackCard,sizeof(m_iBackCard));
	::ZeroMemory(m_iDeskCard,sizeof(m_iDeskCard));
	m_gameCountLimit=0; 
	m_gameMoney=0;//进入
	//已初始化
	for(int i = 0; i < PLAY_COUNT; i ++)
	{
		m_iLastCardCount[i] = 0;
		m_iAwardPoint[i] = 0;
		m_iWinPoint[i] = 0;

		m_bUserNetCut[i] = false;
	}
	::ZeroMemory(m_iLastOutCard, 0, PLAY_COUNT*CARD_COUNT);
	m_iRecvMsg = 0;
	m_iGameFlag = GS_FLAG_NORMAL;
	m_iLeaveArgee = 0;
	m_iNtFirstCount = 0;		//第一首牌清零
	//	m_iNowCallScorePeople = -1;		//
	m_iBaseOutCount = 0;
	m_iWinPeople= -1;
	m_iDealPeople = -1;
	m_iFirstOutPeople = -1;
	m_iNowBigPeople = -1;
	m_iOutCardPeople = -1;
	m_bGameStation = GS_WAIT_ARGEE;

	::memset(m_bIsBombWin,0,sizeof(m_bIsBombWin));
	m_dThinkLeftTime= GetTickCount();

	::memset(m_iDeskCardCount,0,sizeof(m_iDeskCardCount));	
	::memset(m_iAIStation,0,sizeof(m_iAIStation));		//智能托管
	::memset(m_bAuto,0,sizeof(m_bAuto));				//托管
	::memset(m_iUserOutTime,0,sizeof(m_iUserOutTime));	//用户托管次数
	::memset(m_iCallScore,-1,sizeof(m_iCallScore));		//叫分
	::memset(m_bCanleave,1,sizeof(m_bCanleave));
	::memset(m_bUserReady , 0 , sizeof(m_bUserReady)); 

	::memset(m_byPass , 0 ,sizeof(m_byPass)) ; 
	::memset(m_byLastTurnPass , 0 , sizeof(m_byLastTurnPass)) ; 

	m_bySendFinishCount = 0 ; 

	m_iFirstCutPeople = 255;
	m_icountleave = 0;

	m_iDeskID = 255;

	LoadIni();

	InitThisGame();
	m_iOutBombUser =-1;
	m_vlSuperID.clear();	//清空超端玩家ID
}

//析构函数
CServerGameDesk::~CServerGameDesk(void)
{
}

//加载ini配置文件
BOOL CServerGameDesk::LoadIni()
{
	CString nid;
	nid.Format("%d",NAME_ID);
	CString s = CINIFile::GetAppPath ();/////本地路径
	CINIFile f( s +nid +"_s.ini");
	CString key = TEXT("game");

	m_FirstNecessaryCard = f.GetKeyVal(key, "FirstNecessaryCard", 0x32);
	m_AutoOutCardTime = f.GetKeyVal(key, "AutoOutCardTime", 20);
	m_MonkeyCard = f.GetKeyVal(key, "MonkeyCard", 0x28);

    m_bHaveMonkey = f.GetKeyVal(key,"Monkey",1);

	m_bHaveKing = f.GetKeyVal(key,"haveking",1);
	m_iPlayCard = f.GetKeyVal(key,"card",1);
	m_iBackCount = f.GetKeyVal(key,"backcard",0);
	m_iUserCount = f.GetKeyVal(key,"usercount",16);


	m_bKingCanReplace = f.GetKeyVal(key,"kingcanreplace",0);


	key=TEXT("other");
	m_iBeginTime = f.GetKeyVal(key,"begintime",15);
	m_iThinkTime = f.GetKeyVal(key,"thinktime",20);

	m_iSendCardTime = f.GetKeyVal(key,"SendCardTime",6);

	key = TEXT("cardshape");
	m_iCardShape &=0x00000000;
	m_iCardShape |= (f.GetKeyVal(key,"one",1)&0xFFFFFFFF);//单张
	m_iCardShape |= ((f.GetKeyVal(key,"two",1)<<1)&0xFFFFFFFF);//对
	m_iCardShape |= ((f.GetKeyVal(key,"three",1)<<2)&0xFFFFFFFF);//三条
	m_iCardShape |= ((f.GetKeyVal(key,"threeandone",1)<<3)&0xFFFFFFFF);//三带单
	m_iCardShape |= ((f.GetKeyVal(key,"threeandtwo",1)<<4)&0xFFFFFFFF);//三带二单
	m_iCardShape |= ((f.GetKeyVal(key,"threeanddouble",1)<<5)&0xFFFFFFFF);//三带对

	m_iCardShape |= ((f.GetKeyVal(key,"traight",1)<<6)&0xFFFFFFFF);//单顺
	m_iCardShape |= ((f.GetKeyVal(key,"traightflush",0)<<7)&0xFFFFFFFF);//同花顺
	m_iCardShape |= ((f.GetKeyVal(key,"twosequence",1)<<8)&0xFFFFFFFF);//双顺
	m_iCardShape |= ((f.GetKeyVal(key,"threesequence",1)<<9)&0xFFFFFFFF);//三顺
	m_iCardShape |= ((f.GetKeyVal(key,"threeandonesequence",1)<<10)&0xFFFFFFFF);//三带带单
	m_iCardShape |= ((f.GetKeyVal(key,"threeandtwosequence",1)<<11)&0xFFFFFFFF);//三带二单顺
	m_iCardShape |= ((f.GetKeyVal(key,"threeanddublesequence",1)<<12)&0xFFFFFFFF);//三带对顺

	m_iCardShape |= ((f.GetKeyVal(key,"fourandone",1)<<13)&0xFFFFFFFF);//四带一
	m_iCardShape |= ((f.GetKeyVal(key,"fourandtwo",1)<<14)&0xFFFFFFFF);//四带二单
	m_iCardShape |= ((f.GetKeyVal(key,"fourandonedouble",1)<<15)&0xFFFFFFFF);//四带一对
	//m_iCardShape |= ((f.GetKeyVal(key,"fourandtwodouble",0)<<16)&0xFFFFFFFF);//四带二对
	m_iCardShape |= ((f.GetKeyVal(key,"fourandThree",1)<<16)&0xFFFFFFFF);//四带三张


	m_iCardShape |= ((f.GetKeyVal(key,"510k",0)<<17)&0xFFFFFFFF);			//510k
	m_iCardShape |= ((f.GetKeyVal(key,"master510k",0)<<18)&0xFFFFFFFF);		//同花510k
	m_iCardShape |= ((f.GetKeyVal(key,"bomb",1)<<19)&0xFFFFFFFF);			//炸弹
	m_iCardShape |= ((f.GetKeyVal(key,"kingbomb",0)<<20)&0xFFFFFFFF);		//王炸
	return true;
}

//加载局数限制
BOOL CServerGameDesk::LoadExtIni()
{
	CString s = CINIFile::GetAppPath ();/////本地路径
	CString nid;
	nid.Format("%d",NAME_ID);
	CINIFile f( s +nid +"_s.ini");

	CString key = "game";
 
 	m_bTurnRule = f.GetKeyVal(key,"TurnRule",1);
 	m_iLimitPlayGame = f.GetKeyVal(key,"gameCountLimit",1);
	m_gameMoney = f.GetKeyVal(key,"gameMoneyLimit",1);
	m_bIsAutoOutCard = f.GetKeyVal(key,"IsAutoOutCard",1);
	m_iOutTimeAutoCount = f.GetKeyVal(key,"OutTimeAutoCount",1);
	m_ZhaDanDiFen = f.GetKeyVal(key, "ZhaDanDiFen", 1); //炸弹底分（还会剩以倍率）
	if (m_ZhaDanDiFen < 1)
	{
		m_ZhaDanDiFen = 1;
	}

	m_iBasePoint = f.GetKeyVal( key, "BasePoint", 1);

	//读取超端数据 
	key = "SuperSet";
	// 判断超级用户在服务器中是否配置
	int iCount = f.GetKeyVal(key, "SuperUserCount", 0);
	CString strText;
	for (int j = 1; j <= iCount; j++)
	{
		strText.Format("SuperUserID%d", j);
		int  lUserID = f.GetKeyVal(key, strText, 0);
		m_vlSuperID.push_back(lUserID);
	}
	return TRUE;
}

//根据房间ID加载配置文件
BOOL CServerGameDesk::LoadExtIni(int iRoomID)
{
 	TCHAR szKey[20];
 	CString nid;
 	nid.Format("%d",NAME_ID);
 	wsprintf(szKey,"%s_%d",nid,iRoomID);
 	CString key = szKey;
 	CString s = CINIFile::GetAppPath ();/////本地路径
 	CINIFile f( s +nid +"_s.ini");
  
 	m_iLimitPlayGame = f.GetKeyVal(key,"gameCountLimit",m_iLimitPlayGame);
	m_iLimitPlayGame=5;//m_pDataManage->m_InitData.iGameCount;
	if (m_iLimitPlayGame<1)
	{
		m_iLimitPlayGame=1;
	}
 	m_bTurnRule = f.GetKeyVal(key,"TurnRule",m_bTurnRule);
	m_bIsAutoOutCard = f.GetKeyVal(key,"IsAutoOutCard",m_bIsAutoOutCard);
	m_iOutTimeAutoCount = f.GetKeyVal(key,"OutTimeAutoCount",m_iOutTimeAutoCount);

	return TRUE;
}
//設置游戲狀態
BOOL CServerGameDesk::InitThisGame()
{
	m_Logic.SetKingCanReplace(m_bKingCanReplace);
	m_Logic.SetCardShape(m_iCardShape);
	return true;
}
bool CServerGameDesk::InitDeskGameStation()
{
	LoadExtIni();
	LoadExtIni(m_pDataManage->m_InitData.uRoomID);

	return true;
}
//拦截玩家强退消息
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
			ResetFirstSendRule();
			m_bGameStation = GS_WAIT_ARGEE;

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
				bForceQuitAsAuto = File.GetKeyVal("ForceQuitAsAuto",strKeyName ,0) ; 
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

//机器人托管
BOOL CServerGameDesk::UseAI(BYTE bDeskStation)
{
	m_iAIStation[bDeskStation] = 1;

	int count = 0;
	for(int i = 0; i < PLAY_COUNT ; i ++)
	{
		if(m_iAIStation[i])
		{
			count ++;
		}
	}

	if(count >= PLAY_COUNT)
	{
		GameFinish(255,GFF_FORCE_FINISH);
		return true;
	}
	SetAIMachine(bDeskStation,TRUE);
	return true;
}

//游戏数据包处理函数
bool CServerGameDesk::HandleNotifyMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	switch (pNetHead->bAssistantID)
	{
	case ASS_GM_AGREE_GAME:		//用户同意游戏
		{
			if (bWatchUser) 
			{
				return true;
			}

			if(m_bGameStation >= GS_SEND_CARD && m_bGameStation <= GS_PLAY_GAME)
			{
				return true;
			}
			if(m_bGameStation != GS_WAIT_NEXT)
			{
				m_bGameStation = GS_WAIT_ARGEE;
			}
			
			if(m_bUserReady[bDeskStation])
			{
				return true; 
			}
			m_bUserReady[bDeskStation] = true ; 
			/// 删除准备定时器
			KillTimer(TIMER_WAIT_READY+bDeskStation);
			return __super::HandleNotifyMessage(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);

		}
	case ASS_SEND_FINISH:
		{
			m_bySendFinishCount ++ ; 

			if(m_bySendFinishCount >= PLAY_COUNT)
			{
				KillTimer(TIME_SEND_CARD_ANI) ; 
				KillTimer(TIME_SEND_CARD_FINISH) ;
				SetTimer(TIME_SEND_CARD_FINISH , 300) ; 
			}
			return true ; 
		}
	case ASS_OUT_CARD:		//用户出牌
		{
			
			if(bWatchUser)
			{
				return true;
			}
			if(uSize != sizeof(C_S_OutCardStruct))
			{
				return true ; 
			}

			if(m_bGameStation != GS_PLAY_GAME)
			{
				return true;
			}

			C_S_OutCardStruct * pOutCard=(C_S_OutCardStruct *)pData;
			if(NULL == pOutCard)
			{
				return true;
			}

			if(pOutCard->iCardCount <= 0x00 || m_iUserCardCount[bDeskStation] <= 0x00)						//客户端不能发送过了,牌完了就不能再出了
			{
				return true;
			}

			if(m_bLastHandFlag)						//最后一手牌将数系统帮其处理,不接受客户端发过来的数据
			{
				return true;
			}
		
			UserOutCard(bDeskStation,pOutCard->iCardList,pOutCard->iCardCount);

			return true;
		}
	case ASS_AUTO:			//托管
		{
			if (bWatchUser)
			{
				return true ; 
			}
			if(uSize!=sizeof(AutoStruct))
			{
				return true ; 
			}
	
			AutoStruct * pAuto = (AutoStruct*)pData;
			UserSetAuto(bDeskStation,pAuto->bAuto);
			if( bDeskStation ==  m_iOutCardPeople )
			{
				UserAutoOutCard(m_iOutCardPeople);
			}
			return true;
		}
	case ASS_HAVE_THING:	//有事请求离开
		{
			if(bWatchUser)
			{
				return true;
			}
			if(uSize !=sizeof(HaveThingStruct))
			{
				return true ; 
			}

			HaveThingStruct * pThing=(HaveThingStruct *)pData;
			UserHaveThing(bDeskStation,pThing->szMessage);
			return true;
		}
	case ASS_LEFT_RESULT: //同意用户离开
		{
			if(bWatchUser)
			{
				return true;
			}
			if(uSize != sizeof(LeaveResultStruct))
			{
				return true ; 
			}

			LeaveResultStruct * pLeave=(LeaveResultStruct *)pData;
			ArgeeUserLeft(bDeskStation,pLeave->bArgeeLeave);
			return true;
		}
	case ASS_REQ_SET_CARD:
		{
			if(uSize != sizeof(SuperSetCardMsg))
			{
				return true;
			}
			if (!IsSuperUser(bDeskStation))
			{
				return true;
			}

			SuperSetCardMsg *pSet =  (SuperSetCardMsg*)pData;
			_isSuperMake = pSet->isMake;
			_superUserDesk = bDeskStation;

			return true;
		}
	default: 
		{
			break; 
		}
	}
	return __super::HandleNotifyMessage(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
}


//定时器消息
bool CServerGameDesk::OnTimer(UINT uTimerID)
{
	//JAdd-20090113-//取消1M倒计时退出
	if ((uTimerID>=IDT_USER_CUT)&&(uTimerID<(UINT)(IDT_USER_CUT+m_bMaxPeople)))
	{
		if (m_bGameStation>=20&&m_bGameStation<23)
		{
			return true;
		}
	}
	switch(uTimerID)
	{
	case TIME_SEND_ALL_CARD:	//准备发牌计时器
		{
			if (m_bGameStation == GS_SEND_CARD)
			{
				SendAllCard();
			}
			else 
			{
				KillTimer(TIME_SEND_ALL_CARD);
			}
			break;
		}
	case TIME_SEND_CARD_ANI:		///客户端发牌定时器	
		{
			KillTimer(TIME_SEND_CARD_ANI) ; 
			SetTimer(TIME_SEND_CARD_FINISH , 300) ; 
			
			break;
		}
	case  TIME_SEND_CARD_FINISH:	//发牌结束计时器
		{
			KillTimer(TIME_SEND_CARD_FINISH) ; 
			SendCardFinish();
			break;
		}
	case TIME_WAIT_NEWTURN:				//新一輪
		{
			KillTimer(TIME_WAIT_NEWTURN);
			if (m_bGameStation == GS_PLAY_GAME)
			{
				NewPlayTurn(m_iNowBigPeople);
			}
			break;
		}	

	case TIME_OUT_CARD:
		{
			KillTimer(TIME_OUT_CARD);
			if(m_bGameStation == GS_PLAY_GAME) 
			{
				KillTimer(TIME_AUTO_OUT_CARD);
				AddBomScroe();
				BYTE bCardList[CARD_COUNT];
				int iCardCount = 0;
				memcpy(bCardList,m_iUserCard[m_iOutCardPeople],m_iUserCardCount[m_iOutCardPeople]);
				iCardCount = m_iUserCardCount[m_iOutCardPeople];
				UserOutCard(m_iOutCardPeople,bCardList,iCardCount);
			}
			break;
		}
	case TIME_AUTO_OUT_CARD:
		KillTimer(TIME_AUTO_OUT_CARD);

		if (m_bIsAutoOutCard && !m_bAuto[m_iOutCardPeople])
		{
			m_iUserOutTime[m_iOutCardPeople] ++;

			if (m_iUserOutTime[m_iOutCardPeople] >= m_iOutTimeAutoCount)
			{
				m_iUserOutTime[m_iOutCardPeople] = 0;
				UserSetAuto(m_iOutCardPeople, true);
			}
		}

		UserAutoOutCard(m_iOutCardPeople);
		break;
	case TIME_GAME_FINISH:				//結束
		{
			KillTimer(TIME_GAME_FINISH);
			if (m_bGameStation == GS_PLAY_GAME) 
			{
				GameFinish(0,GF_NORMAL);
			}
			break;
		}
	case TIME_GAME_PASS:
		{
			KillTimer(TIME_GAME_PASS);
           /* S_C_CannotOutCard tCannotOutCard;
            tCannotOutCard.byDeskStation = m_iOutCardPeople;
            for (int i = 0; i < PLAY_COUNT; i++)
            {
                SendGameData(i, &tCannotOutCard, sizeof(S_C_CannotOutCard), MDM_GM_GAME_NOTIFY, S_C_CANNT_COUT_CARD, 0);
            }*/
            OutputDebugStringA("PDKT::0002");
			////AddBomScroe();
            BYTE bCardList[CARD_COUNT];
            memset(bCardList, 0, sizeof(BYTE) * CARD_COUNT);
            int iCardCount = 0;
            UserOutCard(m_iOutCardPeople, bCardList, iCardCount);

			break;
		}
	}
	/// 踢未准备的玩家
	if(uTimerID >= TIMER_WAIT_READY && 
		uTimerID <= TIMER_WAIT_READY_MAX)
	{
		KillTimer(uTimerID);
		MakeUserOffLine(uTimerID - TIMER_WAIT_READY);
	}
	return __super::OnTimer(uTimerID);
}

//获取游戏状态信息
bool CServerGameDesk::OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser)
{
	/// 玩家重连，表示不在托管状态，因此在这里设置为不托管
	if (!bWatchUser)
	{
		m_bAuto[bDeskStation] = false;
	}
	m_bUserReady[bDeskStation] = false;
	//超端验证
	SpuerExamine(bDeskStation);

	//保存游戏桌子号 用于写日志
	if(m_iDeskID == 255)
	{
		if(m_pUserInfo[bDeskStation] != NULL)
		{
			m_iDeskID = m_pUserInfo[bDeskStation]->m_UserData.bDeskNO;
		}
	}

	if(m_pUserInfo[bDeskStation]->m_UserData.isVirtual)
	{
		// 机器人自动出牌
		m_bAuto[bDeskStation] = true;
	}
	
	switch (m_bGameStation)
	{
	case GS_WAIT_SETGAME:		//游戏没有开始状态
	case GS_WAIT_ARGEE:			//等待玩家开始状态
		{
			S_C_GameStation_Wait GameStation;
			::memset(&GameStation,0,sizeof(GameStation));
			//游戏版本核对
			GameStation.iVersion = DEV_HEIGHT_VERSION;			//游戏高版本
			GameStation.iVersion2 = DEV_LOW_VERSION;			//低版本
			//游戏内容设置
			GameStation.iCardShape = m_iCardShape;						//牌型设置
			//辅助时间
			GameStation.iBeginTime = m_iBeginTime;				//游戏开始时间设置
			GameStation.iThinkTime = m_iThinkTime;				//游戏思考时间
			//房间倍数
			GameStation.iDeskBasePoint = GetDeskBasePoint();	//桌子倍数
			GameStation.iRoomBasePoint = GetRoomMul();		    //房间倍数
			GameStation.iRunPublish = GetRunPublish();			//逃跑扣分
			//GameStation.gameMoneyLimit=m_gameMoney;
//			GameStation.iZamaNum = m_iZamaNum;					//创建玩法
			GameStation.bMastOutBlackThree = m_bFirstUserOut;	//是否是黑桃3先出
			memcpy(GameStation.bUserReady ,m_bUserReady , sizeof(GameStation.bUserReady ));
			SendGameStation(bDeskStation,uSocketID,bWatchUser,&GameStation,sizeof(GameStation));
			return TRUE;
		}
	case GS_SEND_CARD:		//发牌状态
	case GS_PLAY_GAME:	//游戏中状态
		{
			S_C_GameStation_Play GameStation;
			::memset(&GameStation,0,sizeof(GameStation));
			//游戏版本核对
			GameStation.iVersion = DEV_HEIGHT_VERSION;			//游戏高版本
			GameStation.iVersion2 = DEV_LOW_VERSION;			//低版本
			//游戏内容设置
			GameStation.iCardShape = m_iCardShape;						//牌型设置
			//辅助时间
			GameStation.iBeginTime = m_iBeginTime;				//游戏开始时间设置
			GameStation.iThinkTime = m_iThinkTime;				//游戏思考时间
 			GameStation.iLeftTime = m_iThinkTime - (GetTickCount()- m_dThinkLeftTime)/1000;
			//房间倍数
			GameStation.iDeskBasePoint = GetDeskBasePoint();	//桌子倍数
			GameStation.iRoomBasePoint = GetRoomMul();	//房间倍数
			GameStation.iRunPublish = GetRunPublish();			//逃跑扣分
			//GameStation.gameMoneyLimit=m_gameMoney;
			GameStation.iOutCardPeople = m_iOutCardPeople;	
			GameStation.iFirstOutPeople = m_iFirstOutPeople;	
			GameStation.iBigOutPeople = m_iNowBigPeople;	
			//游戏倍数	
			memcpy(GameStation.bCanleave, m_bCanleave, sizeof(m_bCanleave));
			memcpy(GameStation.bAuto , m_bAuto , sizeof(m_bAuto)) ;

			memcpy(GameStation.bPass , m_byPass , sizeof(GameStation.bPass)) ; 
			memcpy(GameStation.bLastTurnPass ,m_byLastTurnPass , sizeof(GameStation.bLastTurnPass)) ;
//			GameStation.iZamaNum = m_iZamaNum;					//创建玩法
			GameStation.bMastOutBlackThree = m_bFirstUserOut;	//是否是黑桃3先出
			//设置各家手中牌
			int iPos = 0;
			ZeroMemory(&GameStation.iUserCardList, sizeof(GameStation.iUserCardList));
			for (int i = 0; i < PLAY_COUNT; i ++)
			{
				//设置用户手中牌
				GameStation.iUserCardCount[i] = m_iUserCardCount[i];

				if (i == bDeskStation)
				{
					::CopyMemory(&GameStation.iUserCardList[iPos],m_iUserCard[i],sizeof(BYTE)*m_iUserCardCount[i]);
				}

				iPos += m_iUserCardCount[i];
			}

			//设置用户桌面牌牌
			GameStation.bIsLastCard = m_bIsLastCard;

			GameStation.iBaseOutCount = m_iBaseOutCount;	
			//GameStation.gameMoneyLimit=m_gameMoney;
			::CopyMemory(&GameStation.iBaseCardList , m_iDeskCard[m_iNowBigPeople] , sizeof(BYTE)*m_iBaseOutCount ) ; 
			::CopyMemory(&GameStation.iDeskCardCount, m_iDeskCardCount, sizeof(GameStation.iDeskCardCount));
			::CopyMemory(GameStation.iDeskCardList, m_iDeskCard, sizeof(GameStation.iDeskCardList));

			::CopyMemory(&GameStation.iLastCardCount, m_iLastCardCount, sizeof(GameStation.iLastCardCount));
			::CopyMemory(GameStation.iLastOutCard, m_iLastOutCard, sizeof(GameStation.iLastOutCard));
	
			//是否不出
			GameStation.bIsPass = m_byteHitPass;				
			
			SendGameStation(bDeskStation,uSocketID,bWatchUser,&GameStation,sizeof(GameStation));

			return TRUE;
		}
	case GS_WAIT_NEXT:		//等待下一盘游戏开始
		{
			//发送取消机器人托管
			//SetAIMachine(bDeskStation,false);
			S_C_GameStation_Next GameStation;
			::memset(&GameStation,0,sizeof(GameStation));
			//游戏版本核对
			GameStation.iVersion = DEV_HEIGHT_VERSION;			//游戏高版本
			GameStation.iVersion2 = DEV_LOW_VERSION;			//低版本
			//辅助时间
			GameStation.iBeginTime = m_iBeginTime;				//游戏开始时间设置
			GameStation.iThinkTime = m_iThinkTime;				//游戏思考时间
			GameStation.iCardShape = m_iCardShape;						//牌型设置

			//房间倍数
			GameStation.iDeskBasePoint = GetDeskBasePoint();	//桌子倍数
			GameStation.iRoomBasePoint = GetRoomMul();	//房间倍数
			GameStation.iRunPublish = GetRunPublish();			//逃跑扣分
			//GameStation.gameMoneyLimit=m_gameMoney;
			memcpy(GameStation.bUserReady ,m_bUserReady , sizeof(GameStation.bUserReady )) ;
//			GameStation.iZamaNum = m_iZamaNum;					//创建玩法
			GameStation.bMastOutBlackThree = m_bFirstUserOut;	//是否是黑桃3先出
			SendGameStation(bDeskStation,uSocketID,bWatchUser,&GameStation,sizeof(GameStation));
			return TRUE;
		}
	}
	return false;
}

//清所有计时器
void CServerGameDesk::KillAllTimer()
{
	OutputDebugStringA("LHX::TIME_SEND_CARD_ANI关 03");
	KillTimer(TIME_SEND_ALL_CARD);
	KillTimer(TIME_SEND_CARD_ANI);
	KillTimer(TIME_GAME_FINISH);
	KillTimer(TIME_WAIT_NEWTURN);
	KillTimer(TIME_OUT_CARD);
	KillTimer(TIME_AUTO_OUT_CARD);
}

//重置游戏状态
bool CServerGameDesk::ReSetGameState(BYTE bLastStation)
{
	KillAllTimer();
	if((bLastStation == GFF_FORCE_FINISH)||( bLastStation== GF_SAFE) || (bLastStation== GFF_DISSMISS_FINISH))
	{
		::memset(m_iWinPoint,0,sizeof(m_iWinPoint));
		::memset(m_iAIStation,0,sizeof(m_iAIStation));
		m_iDealPeople=-1;
		m_iWinPeople= -1;
	}

	m_dThinkLeftTime= GetTickCount();
	m_iFirstCutPeople = 255;

	for(int i = 0;i < PLAY_COUNT; i ++)
	{
		m_iAwardPoint[i]=0;
	}
	m_iGameFlag = GS_FLAG_NORMAL;
	m_iNtFirstCount=0;		//庄家出的第一手牌数量
	m_iLeaveArgee=0;
	m_iBaseOutCount=0;
	m_iNowBigPeople=-1;
	m_iOutCardPeople=-1;
	m_iSendCardPos=0;
	::memset(m_bIsBombWin,0,sizeof(m_bIsBombWin));
	m_iOutBombUser =-1;
	::memset(m_iDeskCardCount,0,sizeof(m_iDeskCardCount));
	::memset(m_iUserCardCount,0,sizeof(m_iUserCardCount));
	::memset(m_iAIStation,0,sizeof(m_iAIStation));		//智能托管
	::memset(m_bAuto,0,sizeof(m_bAuto));				//托管
	::memset(m_iUserOutTime,0,sizeof(m_iUserOutTime));	//用户超时次数，用于托管
	::memset(m_iCallScore,-1,sizeof(m_iCallScore));		//叫分

	return TRUE;
}


/*---------------------------------------------------------------------------------*/
//游戏开始
bool	CServerGameDesk::GameBegin(BYTE bBeginFlag)
{

	if (__super::GameBegin(bBeginFlag)==false)
	{
		GameFinish(0,GF_SAFE);
		return false;
	}
	/// 清理所有的准备定时器
	for (size_t i = TIMER_WAIT_READY;i <= TIMER_WAIT_READY_MAX;i++)
	{
		KillTimer(i);
	}
	if(IsBuyDesk() && GetRunGameCount() == 1)
	{
		//VIP房第一局重置一下
		ResetFirstSendRule();
	}

	if(!IsBuyDesk() || GetRunGameCount() == 1)
	{
		// 普通房或VIP房第一局清空统计数据
		m_tSettlementList.Clrea();
	}

// 	CString str;
// 	str.Format("PDKT::当前玩法选择 %d %d",m_iUserCount,m_iZamaNum);
// 	OutputDebugStringA(str);

	//当前玩法是几张玩法
// 	if(m_iZamaNum == 1 || m_iZamaNum == 2)
// 	{
		m_iUserCount = 16;
// 	}
// 	else
// 	{
// 		m_iUserCount = 15;
// 	}

	::ZeroMemory(m_iAIStation,sizeof(m_iAIStation));//清空代打情况；20090218 By Shao

	m_icountleave = 0;
	m_iSendCardPos = 0;
	m_bySendFinishCount = 0 ;  
	::memset(m_bIsBombWin,0,sizeof(m_bIsBombWin));
	m_bGameStation = GS_SEND_CARD;

	::memset(m_iDeskCardCount,0,sizeof(m_iDeskCardCount));
	::memset(m_iUserCardCount,0,sizeof(m_iUserCardCount));
	::memset(m_bCanleave,1,sizeof(m_bCanleave));
	::memset(m_bUserReady,0,sizeof(m_bUserReady));
	::memset(m_iBomMultiple,0,sizeof(m_iBomMultiple));
	
	::memset(m_iLastCardCount,0,sizeof(m_iLastCardCount));
	::memset(m_bUserNetCut,false,sizeof(m_bUserNetCut));
	::memset(m_byBomOutNumber,0,sizeof(m_byBomOutNumber));
	::ZeroMemory(m_iLastOutCard, 0, PLAY_COUNT*CARD_COUNT);
	::memset(m_bCoseSpring,false,sizeof(m_bCoseSpring));
	::memset(m_iUserCard,0xff,sizeof(m_iUserCard));
	m_bIsLastCard = false;
	m_byteHitPass = 0;
	m_bAlreadSendSettlementList = false;
	m_bLastHandFlag = false;

	//发送游戏开始消息
	S_C_GameBeginStruct TGameBegin;
	TGameBegin.iBeenPlayGame = GetRunGameCount();
	TGameBegin.iPlayLimit = m_iVipGameCount;
	TGameBegin.iCardShape = m_iCardShape;
//	TGameBegin.iZamaNum = m_iZamaNum;

	for (int i = 0; i < PLAY_COUNT; i ++)
	{
		SendGameData(i,&TGameBegin,sizeof(TGameBegin),MDM_GM_GAME_NOTIFY,ASS_GAME_BEGIN,0);
	}
	SendWatchData(m_bMaxPeople,&TGameBegin,sizeof(TGameBegin),MDM_GM_GAME_NOTIFY,ASS_GAME_BEGIN,0);

	//服务端分发所有玩家的扑克
	BYTE iCardArray[162];

	//16张玩法的牌
	if(16 == m_iUserCount)
	{
		m_Logic.RandCard(iCardArray,48,m_pDataManage->m_InitData.uRoomID,m_bHaveKing);
	}
	else
	{
		m_Logic.RandCard15(iCardArray,45,m_pDataManage->m_InitData.uRoomID,m_bHaveKing);
	}
	

	//拷贝玩家牌数据
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		m_iMonkeyPlayMultiple[i] = 1;
		::CopyMemory(m_iUserCard[i],&iCardArray[m_iUserCount*i],sizeof(BYTE)*m_iUserCount);
	}

	IniTestSetHandPai(); //读取配牌
	OutputDebugStringA("PDK::======================================================================A");
	for(int i = 0;i < PLAY_COUNT;i ++)
	{
		for(int j = 0;j < 16;j ++)
		{
			CString str;
			str.Format("PDK::当前牌值 i = %d CardValue = %x %d",i,m_iUserCard[i][j],m_iUserCount);
			OutputDebugStringA(str);
		}
	}
	OutputDebugStringA("PDK::======================================================================B");

	//manager设置牌
	if (setCard())
	{
		_isSuperMake = false;
	}

	for(int i = 0; i < PLAY_COUNT; ++i)
	{
		// 机器人自动出牌
		if(m_pUserInfo[i] && m_pUserInfo[i]->m_UserData.isVirtual)
		{
			m_bAuto[i] = true;
		}
	}

	//当前是不是猴子打法
 	if(1 == m_bHaveMonkey)
 	{
		for(int i = 0;i < PLAY_COUNT;i ++)
		{
			for(int j = 0;j < m_iUserCount;j ++)
			{
				if(m_iUserCard[i][j] == m_MonkeyCard)
				{
					m_iMonkeyPlayMultiple[i] = 2;				//猴子在那家,那家的分数要番倍
					break;
				}
			}
		}
	}

	SetTimer(TIME_SEND_ALL_CARD,200);

	return TRUE;
}
/*---------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------*/
///一次发所有牌
/// 为防止外挂看牌器存在，只发自己的手牌，他人的手牌清空。
/// 旁观玩家不发任何牌，以免作弊现象

BOOL	CServerGameDesk::SendAllCard()
{
	S_C_SendAllStruct TSendAll;

	for(int i = 0; i < PLAY_COUNT; i ++)
	{
		m_iUserCardCount[i] = m_iUserCount;		
		TSendAll.iUserCardCount[i] = m_iUserCardCount[i];
	}
	//发送数据
	int iPos = 0;
	int iTempPos = 0 ;
	for(int i = 0;i < PLAY_COUNT; i ++)
	{
		int iTempPos = 0 ;
		for(int  j  = 0  ; j < PLAY_COUNT  ;j++)
		{
			if(i == j )
			{
				::CopyMemory(&TSendAll.iUserCardList[iTempPos],m_iUserCard[j],sizeof(BYTE)*m_iUserCardCount[j]);

				m_Logic.SortCard(&TSendAll.iUserCardList[iTempPos], NULL , m_iUserCardCount[j]) ; 
			}

			iTempPos += m_iUserCardCount[j] ; 
		}
	
		iPos += m_iUserCardCount[i];
		SendGameData(i,&TSendAll,sizeof(TSendAll),MDM_GM_GAME_NOTIFY,ASS_SEND_ALL_CARD,0);
		SendWatchData(i,&TSendAll,sizeof(TSendAll),MDM_GM_GAME_NOTIFY,ASS_SEND_ALL_CARD,0);
		memset(&TSendAll.iUserCardList,255, sizeof(TSendAll.iUserCardList));
	}

	m_iSendCardPos ++;

	if(m_iSendCardPos == 1)
	{
		KillTimer(TIME_SEND_ALL_CARD);
		SetTimer(TIME_SEND_CARD_ANI , m_iSendCardTime *1000) ; ///给玩家发牌定时器
		return TRUE;
	}

	return FALSE;
}
/*------------------------------------------------------------------------------*/
//发牌结束
BOOL	CServerGameDesk::SendCardFinish()
{
	S_C_SendCardFinishStruct Tsendcardfinish;

	//发送发牌结束
	for(int i = 0; i < PLAY_COUNT; i ++)
	{
		memset(&Tsendcardfinish, 0, sizeof(Tsendcardfinish));
		//如果是超端 就把所有的牌数据发送过去
		if (IsSuperUser(i))
		{
			//所有玩家的牌数据
			memcpy(&Tsendcardfinish.byUserCard, m_iUserCard, sizeof(Tsendcardfinish.byUserCard));
			memcpy(&Tsendcardfinish.byUserCardCount, m_iUserCardCount, sizeof(Tsendcardfinish.byUserCardCount));
		}

		SendGameData(i,&Tsendcardfinish,sizeof(Tsendcardfinish),MDM_GM_GAME_NOTIFY,ASS_SEND_FINISH,0);
	}
	SendWatchData(m_bMaxPeople,&Tsendcardfinish,sizeof(Tsendcardfinish),MDM_GM_GAME_NOTIFY,ASS_SEND_FINISH,0);

	m_bGameStation = GS_PLAY_GAME;

	BeginPlay();

	return TRUE;
}
/*------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------*/
//获取下一个玩家位置

BYTE CServerGameDesk::GetNextDeskStation(BYTE bDeskStation)
{
	if(!m_bTurnRule)//顺时针
	{
		return (bDeskStation + 1 ) % PLAY_COUNT;
	}
	//逆时针
	return 	(bDeskStation + (PLAY_COUNT - 1)) % PLAY_COUNT;
}

/*------------------------------------------------------------------------------*/
//游戏开始
BOOL	CServerGameDesk::BeginPlay()
{
	KillAllTimer() ; 
	m_bFirstUserOut = false;
	//设置数据
	m_bGameStation = GS_PLAY_GAME;
	m_iBaseOutCount = 0;

	m_iFirstOutPeople = m_iWinPeople;
	m_bFirstUserOut = false;
	//开局要找红桃3先出牌 
	if(-1 == m_iWinPeople)
	{
		for (int i = 0; i < PLAY_COUNT; i ++) 
		{
			BYTE bCardBuf[8];
			int ht3cnt=0;
			int count=m_Logic.TackOutBySpecifyCard(m_iUserCard[i],m_iUserCardCount[i],bCardBuf,ht3cnt,m_FirstNecessaryCard);
			if(count)
			{
				m_iFirstOutPeople =i;
				m_bFirstUserOut = true;
				break;
			}
		}
	}

	m_iNowBigPeople = m_iOutCardPeople = m_iFirstOutPeople;

	::memset(m_iDeskCardCount,0,sizeof(m_iDeskCardCount));

	//排列扑克
	for (int i = 0; i < PLAY_COUNT; i ++) 
	{
		m_Logic.SortCard(m_iUserCard[i],NULL,m_iUserCardCount[i]);
	}

	//发送游戏开始消息
	S_C_BeginPlayStruct Begin;
	Begin.iOutDeskStation = m_iOutCardPeople;
	Begin.bMastOutBlackThree = m_bFirstUserOut;
	for (int i = 0;i < PLAY_COUNT; i ++)
	{
		SendGameData(i,&Begin,sizeof(Begin),MDM_GM_GAME_NOTIFY,ASS_GAME_PLAY,0);
	}
	SendWatchData(m_bMaxPeople,&Begin,sizeof(Begin),MDM_GM_GAME_NOTIFY,ASS_GAME_PLAY,0);

	m_dThinkLeftTime= GetTickCount();

	SetTimer(TIME_AUTO_OUT_CARD, (m_bAuto[m_iOutCardPeople] ? 1 : m_AutoOutCardTime) * 1000); // 自动出卡倒计时
	
	return TRUE;
}

/*------------------------------------------------------------------------------*/
//用户出牌
BOOL	CServerGameDesk::UserOutCard(BYTE bDeskStation,  BYTE iOutCard[],  int iCardCount)
{
	//--------------------------------------------------------------------------------------------------------------------------------------------------
	S_C_OutCardError tOutCardError;

	if (bDeskStation != m_iOutCardPeople) 
	{
		tOutCardError.iError = 1;
		SendGameData(bDeskStation,&tOutCardError,sizeof(S_C_OutCardError),MDM_GM_GAME_NOTIFY,S_C_OUTCARD_ERROR,0);
		return true;
	}
	else 
	{
		//当前最大出牌的玩家不能不出
		if (0 == iCardCount  && bDeskStation == m_iNowBigPeople)
		{
			return true;
		}
	}

	BYTE byCardShap = m_Logic.GetCardShape(iOutCard, iCardCount);

	if(UG_ERROR_KIND == byCardShap && iCardCount > 0x00)
	{
		tOutCardError.iError = 2;
		SendGameData(bDeskStation,&tOutCardError,sizeof(S_C_OutCardError),MDM_GM_GAME_NOTIFY,S_C_OUTCARD_ERROR,0);
		return true;
	}

	//检测第一手牌要出黑桃3
	if(iCardCount > 0x00 && m_bFirstUserOut && !m_Logic.CheckHaveCard(iOutCard, iCardCount, m_FirstNecessaryCard))
	{
		tOutCardError.iError = 3;
		SendGameData(bDeskStation,&tOutCardError,sizeof(S_C_OutCardError),MDM_GM_GAME_NOTIFY,S_C_OUTCARD_ERROR,0);
		return true;
	}

	//如果玩家当前出的是三带一，又不是最后一手牌,则不能出牌
	if(UG_THREE_ONE == byCardShap && m_iUserCardCount[bDeskStation] != 4 && iCardCount > 0)
	{
		tOutCardError.iError = 4;
		SendGameData(bDeskStation,&tOutCardError,sizeof(S_C_OutCardError),MDM_GM_GAME_NOTIFY,S_C_OUTCARD_ERROR,0);
		return true;
	}

	//炸弹是不能拆滴
	if(UG_BOMB != byCardShap && iCardCount > 0)
	{
		if(m_Logic.CheckOutCardAndHandCardComBo(iOutCard,iCardCount,m_iUserCard[bDeskStation],m_iUserCardCount[bDeskStation]))
		{
			tOutCardError.iError = 5;
			SendGameData(bDeskStation,&tOutCardError,sizeof(S_C_OutCardError),MDM_GM_GAME_NOTIFY,S_C_OUTCARD_ERROR,0);
			return true;
		}
	}
	if(iCardCount > 0 && byCardShap == UG_THREE_SEQUENCE_EX && m_iUserCardCount[bDeskStation] != iCardCount)
	{
		tOutCardError.iError = 6;
		SendGameData(bDeskStation,&tOutCardError,sizeof(S_C_OutCardError),MDM_GM_GAME_NOTIFY,S_C_OUTCARD_ERROR,0);
		return true;
	}

	//检测如果只出一张，那么下家只能出最大的一张
	if(0x01 == iCardCount)
	{
		int iNextUser = GetNextDeskStation(bDeskStation);
		if(1 == m_iUserCardCount[iNextUser])
		{
			BYTE iResultCard[45];
			int iResultCardCount = 0;
			BOOL bMaxCard = m_Logic.TackOutCardMoreThanLast(m_iUserCard[bDeskStation],m_iUserCardCount[bDeskStation],iOutCard,iCardCount,iResultCard,iResultCardCount);	
			if(iResultCardCount > 0)
			{
				return true;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------------------
	//处理服务器牌
	if(iCardCount > 0x00)					
	{
	
		if(0 == m_iNtFirstCount)		//第一手牌
		{
			m_bFirstUserOut = false;
			m_iNtFirstCount = iCardCount;
		}

		if (!m_Logic.CanOutCard(iOutCard,iCardCount,m_iDeskCard[m_iNowBigPeople],m_iDeskCardCount[m_iNowBigPeople],
								m_iUserCard[bDeskStation],m_iUserCardCount[bDeskStation],bDeskStation == m_iNowBigPeople))
		{
			return true;
		}


		m_iNowBigPeople = bDeskStation;
		m_iBaseOutCount = iCardCount;

		//如果不是最后一手牌,那就检测是否是拦牌
		if(iCardCount != m_iUserCardCount[bDeskStation])
		{
			CheckCloseSpring(bDeskStation,iOutCard,iCardCount);
		}
		
		BYTE iBaseMaxCard = m_Logic.GetBulkBySepcifyMinOrMax(m_iUserCard[bDeskStation],m_iUserCardCount[bDeskStation], 255);

		if (m_Logic.RemoveCard(iOutCard,iCardCount,m_iUserCard[bDeskStation],m_iUserCardCount[bDeskStation]) == 0)
		{
			if(bDeskStation != m_iNowBigPeople)
			{
				iCardCount = 0;
				memset(iOutCard,0,sizeof(iOutCard));
			}
			else
			{
				return FALSE;
			}
		}

		m_iUserCardCount[bDeskStation] -= iCardCount;

		//记录出牌信息
		m_iDeskCardCount[bDeskStation] = iCardCount;
		::CopyMemory(m_iDeskCard[bDeskStation],iOutCard,sizeof(BYTE)*iCardCount);

		m_byPass[bDeskStation] = false ; 

		//------炸弹赢5倍基数分奖励
		if(m_Logic.IsBomb(iOutCard,iCardCount))
		{
			m_iOutBombUser = bDeskStation;
		}
	}
	else
	{
		//某一位置什麼牌也不出
		m_iDeskCardCount[bDeskStation] = iCardCount;

		m_byteHitPass |= (1<<bDeskStation);	

		m_byPass[bDeskStation] = true ; 

	}
    
    KillTimer(TIME_OUT_CARD); 
    if (m_byPass[bDeskStation])
    {
        S_C_CannotOutCard tCannotOutCard;
        tCannotOutCard.byDeskStation = m_iOutCardPeople;
        for (int i = 0; i < PLAY_COUNT; i++)
        {
            SendGameData(i, &tCannotOutCard, sizeof(S_C_CannotOutCard), MDM_GM_GAME_NOTIFY, S_C_CANNT_COUT_CARD, 0);
        }
    }
    else
    {
        //发送玩家出牌结果
        S_C_OutCardMsg UserOutResult;
        UserOutResult.bDeskStation = bDeskStation;
        UserOutResult.iCardCount = iCardCount;
        ::CopyMemory(UserOutResult.iCardList, iOutCard, sizeof(BYTE)*iCardCount);

        for (int i = 0; i < PLAY_COUNT; i++)
        {
            SendGameData(i, &UserOutResult, sizeof(S_C_OutCardMsg), MDM_GM_GAME_NOTIFY, ASS_OUT_CARD_RESULT, 0);
        }
        SendWatchData(m_bMaxPeople, &UserOutResult, sizeof(S_C_OutCardMsg), MDM_GM_GAME_NOTIFY, ASS_OUT_CARD_RESULT, 0);
    }
	
																					    
	//====判断是否这个玩家是否出完牌
	if (m_iUserCardCount[bDeskStation] <= 0)
	{
		OutputDebugStringA("PDKT::0003");
		//如果有炸弹,那就计算最大的那个赢得分数
		AddBomScroe();

		m_Logic.SetLastCardData(iOutCard , iCardCount) ; 

		m_iWinPeople = bDeskStation;
		//出完牌游戏结束
		SetTimer(TIME_GAME_FINISH,2000);
		return true;
	}

	//计算下一轮出牌者
	m_iOutCardPeople = GetNextDeskStation(bDeskStation);

	for(int i = m_iOutCardPeople ;;i = GetNextDeskStation(i))
	{
		m_iOutCardPeople = i;				//当前出牌者
		if(IsNewTurn())
		{
			OutputDebugStringA("PDKT::0004");
			//如果有炸弹,那就计算最大的那个赢得分数
			AddBomScroe();
			SetTimer(TIME_AUTO_OUT_CARD, (m_bAuto[m_iOutCardPeople] ? 1 : m_AutoOutCardTime) * 1000);
			return true;
		}
		//当前出牌者有牌未出
		if(m_iUserCardCount[i] > 0)
		{
			break;
		}
	}

	m_dThinkLeftTime= GetTickCount();

	int bNextUserCanOut = true;

	BYTE bCardList[CARD_COUNT];
	memset(bCardList,0,sizeof(BYTE) * CARD_COUNT);
	//int iCardCount = 0;

	m_Logic.AutoOutCard(m_iUserCard[m_iOutCardPeople],m_iUserCardCount[m_iOutCardPeople],m_iDeskCard[m_iNowBigPeople],m_iDeskCardCount[m_iNowBigPeople],bCardList,iCardCount,false);
	if(iCardCount <= 0)		//不能出牌
	{
		bNextUserCanOut = false;
		SetTimer(TIME_GAME_PASS, 1000);
	}
	else
	{
		//是最后一手牌,而且可以压,那就帮出牌
		if(iCardCount == m_iUserCardCount[m_iOutCardPeople])
		{
			OutputDebugStringA("PDKT::0005");
			//如果有炸弹,那就计算最大的那个赢得分数(如果之前是空炸弹,那么就在这里增加一个炸弹)
			AddBomScroe();
			SetTimer(TIME_OUT_CARD, 1000);
			m_bLastHandFlag = true;
			bNextUserCanOut = false;
		}
		else
		{
			SetTimer(TIME_AUTO_OUT_CARD, (m_bAuto[m_iOutCardPeople] ? 1 : m_AutoOutCardTime) * 1000);
		}
	}

	//下一个出牌的人
	S_C_OutCardMsg UserOut;
	UserOut.iNextDeskStation = m_iOutCardPeople;
	UserOut.iCardCount = 0;
	UserOut.bShowNotic = bNextUserCanOut;
	for (int i = 0;i<m_bMaxPeople;i++) 
	{
		SendGameData(i,&UserOut,sizeof(S_C_OutCardMsg),MDM_GM_GAME_NOTIFY,ASS_OUT_CARD,0);
	}
	SendWatchData(m_bMaxPeople,&UserOut,sizeof(S_C_OutCardMsg),MDM_GM_GAME_NOTIFY,ASS_OUT_CARD,0);

	return TRUE;
}

//增加炸弹分
void CServerGameDesk::AddBomScroe()
{
	if(-1 != m_iOutBombUser)
	{
			int iFirst = (m_iOutBombUser + 1) % PLAY_COUNT;
			int iSecond = (m_iOutBombUser + 2) % PLAY_COUNT;

			m_iBomMultiple[iFirst] -= m_iBasePoint * m_ZhaDanDiFen * m_iMonkeyPlayMultiple[m_iOutBombUser] * m_iMonkeyPlayMultiple[iFirst];
			m_iBomMultiple[iSecond] -= m_iBasePoint * m_ZhaDanDiFen * m_iMonkeyPlayMultiple[m_iOutBombUser] * m_iMonkeyPlayMultiple[iSecond];
			m_iBomMultiple[m_iOutBombUser] +=(m_iBasePoint * m_ZhaDanDiFen * m_iMonkeyPlayMultiple[m_iOutBombUser] * (m_iMonkeyPlayMultiple[iFirst] + m_iMonkeyPlayMultiple[iSecond]));

			
			m_tSettlementList.iOutBomNumber[m_iOutBombUser]++;			//出炸弹的记录
			m_byBomOutNumber[m_iOutBombUser] ++;

			m_iOutBombUser = -1;
			CString str;
			str.Format("PDKT::增加的炸弹数 %d %d %d %d %d %d ",m_iBomMultiple[0],m_iBomMultiple[1],m_iBomMultiple[2],iFirst,iSecond,m_iOutBombUser);
			OutputDebugStringA(str);
	}
}

/*------------------------------------------------------------------------------*/
//是否为新一轮

BOOL CServerGameDesk::IsNewTurn()
{
	if(m_iOutCardPeople == m_iFirstOutPeople)
	{
		for(int i = 0; i < PLAY_COUNT; i ++)
		{
			SendGameData(i,MDM_GM_GAME_NOTIFY,ASS_ONE_TURN_OVER,0);
		}

		SendWatchData(m_bMaxPeople,MDM_GM_GAME_NOTIFY,ASS_ONE_TURN_OVER,0);
	}	

	if (m_iOutCardPeople == m_iNowBigPeople)			//最先出牌者
	{

		if (m_bGameStation == GS_PLAY_GAME)
		{
			NewPlayTurn(m_iNowBigPeople);
		}	

		return true;
	}
	return false;
}
//新一轮开始
BOOL CServerGameDesk::NewPlayTurn(BYTE bDeskStation)
{
	m_iBaseOutCount = 0;
	m_iOutCardPeople = m_iFirstOutPeople = m_iNowBigPeople = bDeskStation;

	//保存上一轮出牌情况
	for(int i = 0; i < PLAY_COUNT; i ++)
	{
		m_bIsLastCard = true;
		m_byteHitPass = 0;
		m_iLastCardCount[i] = m_iDeskCardCount[i];
		::CopyMemory(m_iLastOutCard[i],m_iDeskCard[i],sizeof(BYTE)*m_iLastCardCount[i]);
		::CopyMemory(m_byLastTurnPass , m_byPass , sizeof(m_byLastTurnPass)) ; 
	}

	::memset(m_iDeskCardCount,0,sizeof(m_iDeskCardCount));
	::memset(m_byPass , 0 ,sizeof(m_byPass)); 

	NewTurnStruct turn;
	turn.bDeskStation = bDeskStation ;
	
	bool bEnableNotic = true;

	//最后一手牌系统帮出了
	BYTE byCardShap = m_Logic.GetCardShape(m_iUserCard[bDeskStation],m_iUserCardCount[bDeskStation]);

	if(UG_ERROR_KIND != byCardShap)
	{
		bEnableNotic = false;
	}

	for (int i = 0; i < PLAY_COUNT; i ++) 
	{
		if(i == bDeskStation)
		{
			turn.bEnableNotic = bEnableNotic;
		}
		else
		{
			turn.bEnableNotic = false;
		}
		SendGameData(i,&turn,sizeof(turn),MDM_GM_GAME_NOTIFY,ASS_NEW_TURN,0);
	}

	SendWatchData(m_bMaxPeople,&turn,sizeof(turn),MDM_GM_GAME_NOTIFY,ASS_NEW_TURN,0);


	if(UG_ERROR_KIND != byCardShap)
	{
		m_bLastHandFlag = true;

		BYTE bCardList[CARD_COUNT];
		int iCardCount = 0;

		memcpy(bCardList,m_iUserCard[bDeskStation],m_iUserCardCount[bDeskStation]);
		iCardCount = m_iUserCardCount[bDeskStation];
		OutputDebugStringA("PDKT::0006");
		AddBomScroe();
		UserOutCard(bDeskStation,bCardList,iCardCount);
	}


	return TRUE;
}

//所出牌中讨赏设置
BOOL CServerGameDesk::IsAwardPoin(BYTE iOutCard[],int iCardCount,BYTE bDeskStation)
{
	if(m_Logic.IsKingBomb(iOutCard,iCardCount) || m_Logic.IsBomb(iOutCard,iCardCount))
	{
		
		m_iAwardPoint[bDeskStation] += 1;							

		AwardPointStruct award;
		award.iAwardPoint = m_iAwardPoint[bDeskStation];
		award.bDeskStation = bDeskStation;
		
		//发送奖分情况
		for (int i = 0;i < PLAY_COUNT; i++) 
			SendGameData(i,&award,sizeof(award),MDM_GM_GAME_NOTIFY,ASS_AWARD_POINT,0);
		SendWatchData(m_bMaxPeople,&award,sizeof(award),MDM_GM_GAME_NOTIFY,ASS_AWARD_POINT,0);
	}
	return true;
}

//游戏结束
bool CServerGameDesk::GameFinish(BYTE bDeskStation, BYTE bCloseFlag)
{
	CString strInfo ; 
	strInfo.Format("LHX:: bDeskStation ==%d , bCloseFlag==%d" , bDeskStation , bCloseFlag) ; 
	OutputDebugString(strInfo) ; 
		//编写代码
	switch (bCloseFlag)
	{
	case GF_NORMAL:		//游戏正常结束
		{
			CString str;
			str.Format("PDKT::炸弹分A = %d %d %d",m_iBomMultiple[0],m_iBomMultiple[1],m_iBomMultiple[2]);
			OutputDebugString(str);
			//设置数据 
			m_bGameStation = GS_WAIT_ARGEE;
 			for (int i = 0; i < PLAY_COUNT; i ++)
 			{
 				if (m_pUserInfo[i] != NULL) 
 					m_pUserInfo[i]->m_UserData.bUserState = USER_SITTING;
 			}

 			//游戏结束
 			S_C_GameEndStruct GameEnd;
 			::memset(&GameEnd,0,sizeof(GameEnd));
 			::memset(GameEnd.iTurePoint,0,sizeof(GameEnd.iTurePoint));
			::memset(GameEnd.iBomb,0,sizeof(GameEnd.iBomb));
			ZeroMemory(GameEnd.szName,sizeof(GameEnd.szName));
			GameEnd.iBaopeiUser = -1;

 			for(int i = 0 ; i <PLAY_COUNT ; i ++)
 			{
 				GameEnd.iUserCardCount[i] = m_iUserCardCount[i] ; 
 				memcpy(GameEnd.iUserCard[i] , m_iUserCard[i] , sizeof(BYTE) *m_iUserCardCount[i]) ; 

				strcpy(GameEnd.szName[i],m_pUserInfo[i]->m_UserData.nickName);
 			}
 
			str.Format("PDKT::炸弹分B = %d %d %d",m_iBomMultiple[0],m_iBomMultiple[1],m_iBomMultiple[2]);
			OutputDebugString(str);

  			memcpy(&GameEnd.iUserCard,&m_iUserCard,sizeof(GameEnd.iUserCard));
  
  			memcpy(&GameEnd.iUserCardCount,&m_iUserCardCount,sizeof(m_iUserCardCount));
  
  			///一下计算玩家的得分情况，如果是金币场的话需要判断是否超过玩家身上的钱，如果是积分场则不需判断

			//平台会剩以数据库配置的赔率
			
			bool bFanBeiFlag[PLAY_COUNT];

			memset(bFanBeiFlag,false,sizeof(bFanBeiFlag));

			//int iCountSpring = 0;

			for(int i = 0;i < PLAY_COUNT;i ++)
			{
				if(m_iUserCount == m_iUserCardCount[i])
				{
					bFanBeiFlag[i] = true;
					//iCountSpring ++;
				}
			}

// 			if(iCountSpring != 2)
// 			{
// 				for(int i = 0;i < PLAY_COUNT;i ++)
// 				{
// 					if(bFanBeiFlag[i])					//当前玩家是春天了
// 					{
// 						if(m_bCoseSpring[i])			//如果被压了
// 						{
// 							if((i + 1)%PLAY_COUNT == m_iWinPeople)	//胜利的是下一家,那就不能算春天
// 							{
// 								bFanBeiFlag[i] = false;
// 							}
// 						}
// 					}
// 				}
// 			}
			str.Format("PDKT::炸弹分C = %d %d %d",m_iBomMultiple[0],m_iBomMultiple[1],m_iBomMultiple[2]);
			OutputDebugString(str);
			///统计分数
			LONGLONG lScore = 0;
			int iFeiBei = 1;
			GameEnd.iBaopeiUser = -1;

			for(int i = 0 ; i< PLAY_COUNT ; i++)
			{
				if(m_iUserCardCount[i] <= 1)				//当前牌只有一张,那就不算分了
				{
					continue;
				}

				if(i != m_iWinPeople)
				{	
					int iScore = 0;
					int ibombCount = 0 ;
					if(bFanBeiFlag[i])			//是否是春天
					{
						 iScore = 30;
					}
					else
					{
						iScore = m_iUserCardCount[i];
					}
											//基本分 * 输分 * 手上是否有猴子牌(输家) * 手上是否有猴子(赢牌玩家)
					GameEnd.iTurePoint[i] -= m_iBasePoint*iScore*m_iMonkeyPlayMultiple[i] * m_iMonkeyPlayMultiple[m_iWinPeople];	
					GameEnd.iTurePoint[m_iWinPeople] += m_iBasePoint*iScore*m_iMonkeyPlayMultiple[i] * m_iMonkeyPlayMultiple[m_iWinPeople];
				}	
			}
			str.Format("PDKT::炸弹分D = %d %d %d",m_iBomMultiple[0],m_iBomMultiple[1],m_iBomMultiple[2]);
			OutputDebugString(str);

			m_tSettlementList.iWinNumber[m_iWinPeople] ++;							//胜的次数
			m_tSettlementList.iLostNumber[(m_iWinPeople + 1) % PLAY_COUNT] ++;		//输的次数
			m_tSettlementList.iLostNumber[(m_iWinPeople + 2) % PLAY_COUNT] ++;		//输的次数

			//
			for(int i = 0;i < PLAY_COUNT;i ++)
			{
				GameEnd.iTurePoint[i] += m_iBomMultiple[i];
			}

			//出的炸弹个数
			memcpy(GameEnd.iBomb,m_byBomOutNumber,sizeof(m_byBomOutNumber));

  			bool temp_cut[PLAY_COUNT];
  			memset(temp_cut,0,sizeof(temp_cut));
  
  			ChangeUserPointint64(GameEnd.iTurePoint,temp_cut);
  			__super::RecoderGameInfo(GameEnd.iChangeMoney);

			//不用手游修改，这边把服务端计算金币和平台处理的金币交换下。不影响后面大结算的数据统计
			__int64	iTurePointTmp[3]={0};	//玩家金币
			memcpy(iTurePointTmp, GameEnd.iTurePoint, sizeof(iTurePointTmp));
			memcpy(GameEnd.iTurePoint, GameEnd.iChangeMoney, sizeof(GameEnd.iTurePoint));
			memcpy(GameEnd.iChangeMoney, iTurePointTmp, sizeof(GameEnd.iChangeMoney));

			CString lhx;
			lhx.Format("PDKT::玩家积分 %I64d %I64d %I64d",GameEnd.iTurePoint[0],GameEnd.iTurePoint[1],GameEnd.iTurePoint[2]);
			OutputDebugString(lhx);
			lhx.Format("PDKT::玩家金币 %I64d %I64d %I64d",GameEnd.iChangeMoney[0],GameEnd.iChangeMoney[1],GameEnd.iChangeMoney[2]);
			OutputDebugString(lhx);
			//单局最高分
// 			if(m_tSettlementList.iSingleMaxScore[m_iWinPeople] < GameEnd.iTurePoint[m_iWinPeople])
// 			{
// 				m_tSettlementList.iSingleMaxScore[m_iWinPeople] = GameEnd.iTurePoint[m_iWinPeople];
// 			}
  
			//单局最高分和历史总分
			for(int i = 0;i < PLAY_COUNT;i ++)
			{
				if (m_iRunGameCount == 1) //第一局
				{
					m_tSettlementList.iSingleMaxScore[i] = GameEnd.iTurePoint[i];
				}
				else if(m_tSettlementList.iSingleMaxScore[i] < GameEnd.iTurePoint[i])
				{
					m_tSettlementList.iSingleMaxScore[i] = GameEnd.iTurePoint[i];
				}

				m_tSettlementList.llScore[i] += GameEnd.iTurePoint[i];
			}


  			{
  				for (int i = 0; i < PLAY_COUNT; i ++) 
				{
  					SendGameData(i,&GameEnd,sizeof(GameEnd),MDM_GM_GAME_NOTIFY,ASS_CONTINUE_END,0);
				}
				SendWatchData(m_bMaxPeople,&GameEnd,sizeof(GameEnd),MDM_GM_GAME_NOTIFY,ASS_CONTINUE_END,0);
  				//设置数据 
  				m_bGameStation = GS_WAIT_NEXT;
  			}
 
 			ReSetGameState(bCloseFlag);
			/// 金币场开启为准备踢人
			if(IsBuyDesk() == false)
			{
				for (size_t i = TIMER_WAIT_READY;i <= TIMER_WAIT_READY_MAX;i++)
				{
					SetTimer(i,1000*m_iBeginTime);
				}
			}
			//发送结算榜
			if (IsBuyDesk() && GetSurplusCount() <= 0)
			{
				CString str;
				str.Format("%s -- 正常大结算", __FUNCTION__);
				WriteLog(str);

				m_bAlreadSendSettlementList = true;
				m_iWinPeople= -1;
				for (int i = 0; i < PLAY_COUNT; i ++) 
				{
					SendGameData(i,&m_tSettlementList,sizeof(S_C_SettlementList),MDM_GM_GAME_NOTIFY,S_C_SETTLEMENTLIST,0);
				}
				SendWatchData(m_bMaxPeople,&m_tSettlementList,sizeof(S_C_SettlementList),MDM_GM_GAME_NOTIFY,S_C_SETTLEMENTLIST,0);
			}

 			__super::GameFinish(bDeskStation,bCloseFlag);
			return true;
		}
	case GFF_SAFE_FINISH:
	case GF_SAFE:			//游戏安全结束
		{
			//设置数据
			m_bGameStation = GS_WAIT_ARGEE;//GS_WAIT_SETGAME;

			for (int i = 0; i < PLAY_COUNT; i ++)
			{
				if (m_pUserInfo[i] != NULL) 
				{
					m_pUserInfo[i]->m_UserData.bUserState = USER_SITTING;
				}
			}
			S_C_GameCutStruct CutEnd;
			::memset(&CutEnd,0,sizeof(CutEnd));
			CutEnd.bDeskStation = -1 ; 

			for (int i = 0; i < PLAY_COUNT; i ++) 
			{
				SendGameData(i,&CutEnd,sizeof(CutEnd),MDM_GM_GAME_NOTIFY,ASS_SAFE_END,0);
			}
			SendWatchData(m_bMaxPeople,&CutEnd,sizeof(CutEnd),MDM_GM_GAME_NOTIFY,ASS_SAFE_END,0);

			bCloseFlag = GF_SAFE;

			ReSetGameState(bCloseFlag);
			__super::GameFinish(bDeskStation,bCloseFlag);
			return true;
		}
	case GFF_FORCE_FINISH:		//用户断线离开
		{
			m_bGameStation = GS_WAIT_ARGEE;
			for (int i = 0; i < PLAY_COUNT; i ++)
			{
				if (m_pUserInfo[i] != NULL) 
				{
					m_pUserInfo[i]->m_UserData.bUserState = USER_SITTING;
				}
			}
			S_C_GameCutStruct CutEnd;
			::memset(&CutEnd,0,sizeof(CutEnd));

			CutEnd.bDeskStation = bDeskStation;
			CutEnd.iTurePoint[bDeskStation] = - GetRunPublish(bDeskStation)-m_DeskBasePoint*2;
			CutEnd.iTurePoint[(bDeskStation + 1)%PLAY_COUNT] = GetRunAwayOtherGetPoint((bDeskStation + 1)%PLAY_COUNT,bDeskStation);
			CutEnd.iTurePoint[(bDeskStation + 2)%PLAY_COUNT] = GetRunAwayOtherGetPoint((bDeskStation + 2)%PLAY_COUNT,bDeskStation);

			bool temp_cut[PLAY_COUNT];
			memset(temp_cut,0,sizeof(temp_cut));

			for (int i = 0; i < PLAY_COUNT; i ++)
			{
				temp_cut[i] = (bDeskStation == i && CutEnd.iTurePoint[i] < 0);
			}

			ChangeUserPointint64(CutEnd.iTurePoint, temp_cut);
			__super::RecoderGameInfo(CutEnd.iChangeMoney);

			bool bNotCostPoint = (m_pDataManage->m_InitData.dwRoomRule & GRR_NOT_COST_POINT) > 0; /// 金币场不扣积分  add by wys 2010-11-1
			if (true == bNotCostPoint)
			{
				memset(CutEnd.iTurePoint,0,sizeof(CutEnd.iTurePoint));
			}

			for (int i = 0; i < PLAY_COUNT; i ++)
			{
				SendGameData(i,&CutEnd,sizeof(CutEnd),MDM_GM_GAME_NOTIFY,ASS_CUT_END,0);
			}
			SendWatchData(m_bMaxPeople,&CutEnd,sizeof(CutEnd),MDM_GM_GAME_NOTIFY,ASS_CUT_END,0);

			bCloseFlag = GFF_FORCE_FINISH;
			 
			ReSetGameState(bCloseFlag);
			__super::GameFinish(bDeskStation,bCloseFlag);

			return TRUE;
		}
	case GF_AHEAD_END://提前结束
		{
			//设置数据 
			m_bGameStation = GS_WAIT_ARGEE;

			for (int i = 0; i < PLAY_COUNT; i ++)
			{
				if (m_pUserInfo[i] != NULL) 
				{
					m_pUserInfo[i]->m_UserData.bUserState = USER_SITTING;
				}
			}
			S_C_GameCutStruct CutEnd;
			::memset(&CutEnd,0,sizeof(CutEnd));
			for (int i = 0; i < PLAY_COUNT; i ++)
				SendGameData(i,&CutEnd,sizeof(CutEnd),MDM_GM_GAME_NOTIFY,ASS_AHEAD_END,0);
			SendWatchData(m_bMaxPeople,&CutEnd,sizeof(CutEnd),MDM_GM_GAME_NOTIFY,ASS_AHEAD_END,0);

			bCloseFlag = GF_AHEAD_END;

			ReSetGameState(bCloseFlag);
			__super::GameFinish(0,bCloseFlag);
			return true;
		}
	case GFF_DISSMISS_FINISH:
		{
			//发送结算榜
			CString str;
			str.Format("LHX::当前游戏局数 %d %d ",m_iRunGameCount + 1,m_iVipGameCount);
			OutputDebugStringA(str);
			if(!m_bAlreadSendSettlementList)			//申请解散房间时,也要把结算榜结发送了
			{
				CString str;
				str.Format("%s -- 中途解散发大结算", __FUNCTION__);
				WriteLog(str);

				OutputDebugStringA("LHX::房间解散了 哈哈A");
				for (int i = 0; i < PLAY_COUNT; i ++) 
				{
					if(NULL != m_pUserInfo[i])
					{
						SendGameData(i,&m_tSettlementList,sizeof(S_C_SettlementList),MDM_GM_GAME_NOTIFY,S_C_SETTLEMENTLIST,0);
					}
				}
				SendWatchData(m_bMaxPeople,&m_tSettlementList,sizeof(S_C_SettlementList),MDM_GM_GAME_NOTIFY,S_C_SETTLEMENTLIST,0);
			}
			m_bGameStation = GS_WAIT_ARGEE;
			OutputDebugStringA("LHX::房间解散了 哈哈B");
			::memset(m_bUserReady ,false,sizeof(m_bUserReady));
			ReSetGameState(GFF_DISSMISS_FINISH);
			break;
		}
	}
	//重置数据
	ReSetGameState(bCloseFlag);
	__super::GameFinish(bDeskStation,bCloseFlag);

	return true;
}

//玩家逃跑其他玩家得分
int CServerGameDesk::GetRunAwayOtherGetPoint(BYTE bDeskStation,BYTE bRunDeskStation )
{
	return 0;
}

//逃跑扣分
int CServerGameDesk::GetRunPublish()
{
	return m_pDataManage->m_InitData.uRunPublish;
}
//逃跑扣分
int CServerGameDesk::GetRunPublish(BYTE bDeskStation)
{
	BYTE iResultCardList[CARD_COUNT] ; 
	memset(iResultCardList , 0 , sizeof(iResultCardList)) ; 

	int  iResultCardcount = 0 ; 
	int  ibombCount = 0; 
	for(int i = 0 ; i < PLAY_COUNT ; i++)
	{
		if(NULL != m_pUserInfo[i])
		{
			ibombCount +=m_Logic.GetBombCount(m_iUserCard[i] ,m_iUserCardCount[i],4) ; 
		}
		if(m_Logic.TackOutKingBomb(m_iUserCard[i] ,m_iUserCardCount[i] ,iResultCardList ,iResultCardcount))
		{
			ibombCount ++ ; 
		}
	}



	//if(bDeskStation == m_iUpGradePeople)
	//{
	//	if(iMutiple <10)
	//	{
	//		iMutiple = 20 ; 
	//	}
	//	else
	//	{
	//		iMutiple *= 4; 
	//	}
	//}
	//else
	//{
	//	if(iMutiple <10)
	//	{
	//		iMutiple = 10 ; 
	//	}
	//	else
	//	{
	//		iMutiple *= 2 ; 
	//	}
	//}

	return  9;//(iMutiple *m_GameMutiple.sBaseMutiple) ;
	
}

//游戲基礎倍數
int CServerGameDesk::GetRoomMul()
{
	return m_pDataManage->m_InitData.uBasePoint; // 倍数
}

//桌子倍数
int CServerGameDesk::GetDeskBasePoint()
{
	return 1;
	//return GetTableBasePoint();
}

//获取真实分(每一个玩家真实分
int CServerGameDesk::GetTruePoint(BYTE bDeskStation)
{
	return 0;
}

//判断是否正在游戏
bool CServerGameDesk::IsPlayGame(BYTE bDeskStation)
{
	if((m_bGameStation >= GS_SEND_CARD && m_bGameStation < GS_WAIT_NEXT))
	{
		return true;
	}
	return false;
}

//判断是否正在游戏
bool CServerGameDesk::IsPlayGameEx(BYTE bDeskStation)
{
	return true;
}

//用户坐下
BYTE CServerGameDesk::UserSitDesk(MSG_GR_S_UserSit * pUserSit, CGameUserInfo * pUserInfo)
{
	if( IsBuyDesk() == false )
	{
		if( IsPlayGame(pUserSit->bDeskStation) == false )
		{
			SetTimer(TIMER_WAIT_READY+pUserSit->bDeskStation,1000*m_iBeginTime) ; 
		}
	}
	return __super::UserSitDesk(pUserSit,pUserInfo);
}

//用户离开游戏桌
BYTE CServerGameDesk::UserLeftDesk(BYTE bDeskStation, CGameUserInfo * pUserInfo)
{
	ResetFirstSendRule();
	m_bUserReady[bDeskStation] = false ; 
	KillTimer(TIMER_WAIT_READY+bDeskStation);   /// 清理准备定时器
	return __super::UserLeftDesk(bDeskStation,pUserInfo);
}

//用户设置游戏
BOOL CServerGameDesk::UserSetGame(UserArgeeGame * pGameSet)
{
	return TRUE;
}

//用来改变用户断线条件的函数
bool CServerGameDesk::CanNetCut(BYTE bDeskStation)
{
	return true;
}

//用户请求离开
BOOL CServerGameDesk::UserHaveThing(BYTE bDeskStation, char * szMessage)
{
	// 已经点过退出按钮(一局只能单击一次)
	if (!m_bCanleave[bDeskStation])
		return TRUE;

	if (m_bGameStation < GS_SEND_CARD) 
		return TRUE;

	if (m_iHaveThingPeople != bDeskStation)
	{
		m_iLeaveArgee = 0;
	}

	m_icountleave = 0;
	m_iHaveThingPeople = bDeskStation;
	m_iLeaveArgee|= 1<<bDeskStation;
	m_bCanleave[bDeskStation]=false;

	if (m_iLeaveArgee != 7)
	{
		HaveThingStruct HaveThing;
		HaveThing.pos = bDeskStation;
		::CopyMemory(HaveThing.szMessage,szMessage,60*sizeof(char));

		for (int i = 0; i < PLAY_COUNT; i ++)
			if (i != bDeskStation)
				SendGameData(i,&HaveThing,sizeof(HaveThing),MDM_GM_GAME_NOTIFY,ASS_HAVE_THING,0);
	}
	else 
		GameFinish(255,GF_SAFE);
	return TRUE;
}

//同意用户离开
BOOL CServerGameDesk::ArgeeUserLeft(BYTE bDeskStation, BOOL bArgee)
{
	m_icountleave++;
	if (bArgee) m_iLeaveArgee|= 1<<bDeskStation;
	else 
		m_iLeaveArgee = 0;
	if (m_iLeaveArgee != 7)				//3个人游戏
	{
		LeaveResultStruct Leave;
		Leave.bDeskStation = bDeskStation;
		Leave.bArgeeLeave = bArgee;
		for (int i = 0; i < PLAY_COUNT; i ++) 
			if (i != bDeskStation) 
				SendGameData(i,&Leave,sizeof(Leave),MDM_GM_GAME_NOTIFY,ASS_LEFT_RESULT,0);
	}
	else
	{
		GameFinish(255,GF_SAFE);
		LeaveResultStruct Leave1;
		Leave1.bDeskStation = m_iHaveThingPeople;
		Leave1.bArgeeLeave = true;
		SendGameData(m_iHaveThingPeople,&Leave1,sizeof(Leave1),MDM_GM_GAME_NOTIFY,ASS_LEFT_RESULT,0);
	}
	if (m_icountleave>=2)
	{
		m_iLeaveArgee=0;
	}

	return TRUE;
}

//托管设置
bool CServerGameDesk::UserSetAuto(BYTE bDeskStation,bool bAutoCard)
{
	if( bDeskStation < 0 || bDeskStation >= PLAY_COUNT) return false;

	m_bAuto[bDeskStation] = bAutoCard;
	AutoStruct autoset;
	autoset.bAuto = bAutoCard;
	autoset.bDeskStation = bDeskStation;

	for(int i = 0; i < PLAY_COUNT; i ++)
		SendGameData(i,&autoset,sizeof(autoset),MDM_GM_GAME_NOTIFY,ASS_AUTO,0);
	SendWatchData(m_bMaxPeople,&autoset,sizeof(autoset),MDM_GM_GAME_NOTIFY,ASS_AUTO,0);
	return true;
}

//自动出牌
BOOL CServerGameDesk::UserAutoOutCard(BYTE bDeskStation)
{
	if (bDeskStation >= PLAY_COUNT || !m_bIsAutoOutCard)
	{
		return FALSE;
	}

	BYTE bCardList[CARD_COUNT];
	memset(bCardList,0,sizeof(BYTE) * CARD_COUNT);
	int iCardCount = 0;
	//if(bDeskStation == m_iFirstOutPeople)   // duanxiaohui alter 20100319
	BYTE nextPlayer = GetNextDeskStation(bDeskStation);
	//下家手上只剩一张牌的时候
	if (m_iUserCardCount[nextPlayer]==1)
	{
		//首出
       if (bDeskStation == m_iNowBigPeople)
       {
			   //抽手上最大的一张牌出去
		 m_Logic.TackOutCardBySpecifyCardNum(m_iUserCard[bDeskStation],m_iUserCardCount[bDeskStation], bCardList, iCardCount, m_iUserCard[bDeskStation][0]);
		 iCardCount=1;

       }
	   else if(m_iDeskCardCount[m_iNowBigPeople]==1)
	   {
		      //有炸弹就出炸弹
		   BYTE iCount = m_Logic.TackOutBySepcifyCardNumCount(m_iUserCard[bDeskStation],m_iUserCardCount[bDeskStation], bCardList, 4);
		
		   if(iCount >= 4)
		   {
			   iCardCount=4;
		   }
		   else
		   {
			   //抽手上最大的一张牌出去
			   m_Logic.TackOutCardBySpecifyCardNum(m_iUserCard[bDeskStation],m_iUserCardCount[bDeskStation], bCardList, iCardCount, m_iUserCard[bDeskStation][0]);
			    iCardCount=1;
			   if(!m_Logic.CompareOnlyOne(m_iDeskCard[m_iNowBigPeople][0], bCardList[0]))
			   {
				  iCardCount=0;
			   }
		   }
		
	   }
	   else
	   {
		   m_Logic.AutoOutCard(m_iUserCard[bDeskStation],m_iUserCardCount[bDeskStation],
			   m_iDeskCard[m_iNowBigPeople],m_iDeskCardCount[m_iNowBigPeople],
			   bCardList,iCardCount,bDeskStation == m_iNowBigPeople);
	   }
      
	}
	else{
		OutputDebugString("LHX::超时自动出牌=================A");
		CString str;
		str.Format("LHX::当前牌数据 %x %x %x %x %x",m_iDeskCard[m_iNowBigPeople][0],m_iDeskCard[m_iNowBigPeople][1],m_iDeskCard[m_iNowBigPeople][2],m_iDeskCard[m_iNowBigPeople][3],m_iDeskCard[m_iNowBigPeople][4]);
		OutputDebugString(str);


		m_Logic.AutoOutCard(m_iUserCard[bDeskStation],m_iUserCardCount[bDeskStation],
							m_iDeskCard[m_iNowBigPeople],m_iDeskCardCount[m_iNowBigPeople],
							bCardList,iCardCount,bDeskStation == m_iNowBigPeople);
		OutputDebugString("LHX::超时自动出牌=================B");
	}
	//else
	//{
	//	iCardCount = 0;
	//}
	return UserOutCard(bDeskStation,bCardList,iCardCount);
}

//===================================================================================================================================================
//当前是否是头春天了
//
bool CServerGameDesk::CheckCloseSpring(BYTE bDeskStation,BYTE byOutCard[],int iOutCardCount)
{
	if(m_iUserCardCount[bDeskStation] == iOutCardCount || iOutCardCount == 0x00)			//如果是最后一手牌,那就不用检测了
	{
		return true;
	}
	
	BYTE byNextDesk = GetNextDeskStation(bDeskStation);			//下一个出牌玩家

	//清除当前轮是否被拦牌了
	m_bCoseSpring[byNextDesk] = false;

	BYTE byCardList[CARD_COUNT];
	
	memset(byCardList,0,sizeof(BYTE) * CARD_COUNT);
	
	int iCardCount = 0;

	//检测下一玩家胡否出牌
	m_Logic.AutoOutCard(m_iUserCard[byNextDesk],m_iUserCardCount[byNextDesk],byOutCard,iOutCardCount,byCardList,iCardCount,false);

	if(iCardCount > 0x00)		//下一玩家有牌可以压
	{
		return true;
	}

	BYTE byCardListTemp[CARD_COUNT];

	memset(byCardListTemp,0,sizeof(BYTE) * CARD_COUNT);

	int iCardCountTemp = 0;

	//检测查找上一家打出一个最小的并且能出的牌的
	m_Logic.AutoOutCard(m_iUserCard[bDeskStation],m_iUserCardCount[bDeskStation],m_iDeskCard[m_iNowBigPeople],m_iDeskCardCount[m_iNowBigPeople],byCardListTemp,iCardCountTemp,bDeskStation == m_iNowBigPeople);

	//检测下家是否存在这类牌型,可以把这牌给压了
	m_Logic.AutoOutCard(m_iUserCard[byNextDesk],m_iUserCardCount[byNextDesk],byCardListTemp,iCardCountTemp,byCardList,iCardCount,false);

	if(iCardCount > 0x00)		//表明下家被拦牌了不能出牌了
	{
		m_bCoseSpring[byNextDesk] = true;
	}

	return true;
}

//发送取消机器人托管
BOOL CServerGameDesk::SetAIMachine(BYTE bDeskStation,BOOL bState)
{
	m_iAIStation[bDeskStation] = bState;

	UseAIStation ai;
	ai.bDeskStation = bDeskStation;
	ai.bState = bState;
	for(int i = 0; i < PLAY_COUNT ;i++)
	{
		if(i != bDeskStation &&m_pUserInfo[i] != NULL)		
		{
			SendGameData(i,&ai,sizeof(ai),MDM_GM_GAME_NOTIFY,ASS_AI_STATION,0);
		}
	}
	SendWatchData(m_bMaxPeople,&ai,sizeof(ai),MDM_GM_GAME_NOTIFY,ASS_AI_STATION,0);

	return TRUE;
}


//用户断线离开
bool CServerGameDesk::UserNetCut(BYTE bDeskStation, CGameUserInfo * pLostUserInfo)
{
	__super::UserNetCut(bDeskStation,  pLostUserInfo);
	m_bUserNetCut[bDeskStation] = true;
	UserleftDesk  bagUserleft;
	bagUserleft.bDeskStation = bDeskStation;
	for (int i = 0;i<PLAY_COUNT;i++)
	{
		if (NULL == m_pUserInfo[i])
		{
			continue;
		}
		SendGameData(i,&bagUserleft,sizeof(bagUserleft),MDM_GM_GAME_NOTIFY,ASS_USER_LEFTDESK,0);
	}
	return true;
}

//------------------------------------------------------------------------------
bool	CServerGameDesk::IsSuperUser(BYTE byDeskStation)
{
	if (m_pUserInfo[byDeskStation] != NULL)
	{
		for(int i=0; i< m_vlSuperID.size(); i++)
		{
			if (m_pUserInfo[byDeskStation]->m_UserData.dwUserID == m_vlSuperID.at(i))
			{
				return true;
			}
		}
	}
	return false;
}
//------------------------------------------------------------------------------
//验证是否超端
void	CServerGameDesk::SpuerExamine(BYTE byDeskStation)
{
	//是超端用户 那么就发送超端消息过去
	if (IsSuperUser(byDeskStation))
	{
		SuperUserMsg TSuperUser;
		TSuperUser.bIsSuper = true;
		TSuperUser.byDeskStation = byDeskStation;
		SendGameData(byDeskStation,&TSuperUser,sizeof(TSuperUser),MDM_GM_GAME_NOTIFY,ASS_SUPER_USER,0);
	}
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
	return FALSE;
}
//GRM函数
//更新奖池数据(游戏->平台)
/// @return 机器人赢钱数
void CServerGameDesk::UpDataRoomPond(__int64 iAIHaveWinMoney)
{

}

//解散房间弹出结算框
void CServerGameDesk::DissMissRoomBefore( void )
{
	if (IsBuyDesk() && m_iRunGameCount != 0)
	{
		//m_bGameStation = GS_WAIT_ARGEE;

		for (int i = 0; i < PLAY_COUNT; i ++) 
		{
			if(NULL != m_pUserInfo[i])
			{
				SendGameData(i,&m_tSettlementList,sizeof(S_C_SettlementList),MDM_GM_GAME_NOTIFY,S_C_SETTLEMENTLIST,0);
			}
		}
		SendWatchData(m_bMaxPeople,&m_tSettlementList,sizeof(S_C_SettlementList),MDM_GM_GAME_NOTIFY,S_C_SETTLEMENTLIST,0);

		::memset(m_bUserReady ,false,sizeof(m_bUserReady));
		ReSetGameState( GFF_DISSMISS_FINISH );
	}
}

void CServerGameDesk::ResetFirstSendRule()
{
	if(!m_bIsBuy)
	{
		ReSetGameState(GFF_DISSMISS_FINISH);
	}
}


//=============================================================================================================================================
//测试牌用
void CServerGameDesk::IniTestSetHandPai()
{
	for(int i=0; i<PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] == NULL)continue;
		for(int j = 0;j < m_iUserCount;j ++)
		{
			GetSpecifiedCard(m_pUserInfo[i]->m_UserData.dwUserID,m_iUserCard[i],j);
		}
	}
}

bool CServerGameDesk::setCard()
{
	if (!_isSuperMake)
	{
		return false;
	}

	if (_superUserDesk >= PLAY_COUNT)
	{
		return false;
	}

	HN::vCard SuperCard;
	if (!m_Cfg.makeCard(SuperCard,0))
	{
		return false;
	}

	HN::vCard card[2];

	if (!m_Cfg.nxtSel(card[0],HN::HAND_CARD_NUM))
	{
		return false;
	}

	if (!m_Cfg.nxtSel(card[1],HN::HAND_CARD_NUM))
	{
		return false;
	}


	for (int i = 0,j = 0;i < PLAY_COUNT;i++)
	{
		if (i == _superUserDesk)
		{
			m_Cfg.reassign(SuperCard,m_iUserCard[i],HN::HAND_CARD_NUM);
			continue;
		}
		m_Cfg.reassign(card[j],m_iUserCard[i],HN::HAND_CARD_NUM);
		j++;
	}

	return true;
}

size_t CServerGameDesk::GetDeskId()
{
	size_t id = 0;
	if (m_bIsBuy)
	{
		id = (size_t)(stoul(m_szDeskPassWord));
	}
	return id;
}

//写日志文件
void  CServerGameDesk::WriteLog(CString csMessage) 
{ 
	try 
	{ 
		//获取路径 可通过ini文件里的节点名 指定输出目录
		char vBuffer[1024]={0};
		GetModuleFileName(GetModuleHandle(NULL),   vBuffer,   sizeof(vBuffer));
		CString mFileName = CString(vBuffer);
		CString ROOTPATH = mFileName.Left(mFileName.ReverseFind('\\'));
		SYSTEMTIME sys_time; 
		CString stemp;
		GetLocalTime(&sys_time);
		CString csLogPath = ROOTPATH + "\\Log";
		CString csLocalPath = CINIFile::GetAppPath ();

		char ch[15]={0};
		sprintf(ch, "%d", NAME_ID);

		CINIFile fINIFile( csLocalPath +ch +"_s.ini");		
		CString key = TEXT("game");
		CString csServiceName=fINIFile.GetKeyVal(key,"ServiceName","ServiceName");
		csLogPath += csServiceName;
		if(!PathFileExists(csLogPath))
		{
			CreateDirectory(csLogPath,NULL);
		}

		//写log
		stemp.Format("%d%02d%02d.txt",sys_time.wYear,sys_time.wMonth,sys_time.wDay);
		CString csLogFileName = csLogPath + "\\"+ stemp;		
		CStdioFile f;
		if(!f.Open(csLogFileName,CStdioFile::modeCreate|CStdioFile::modeNoTruncate|CStdioFile::modeWrite|CStdioFile::typeText))
		{
			return ;
		}
		f.SeekToEnd();
		stemp.Format("%02d:%02d:%02d  DeskId[%u] -> %s\n",sys_time.wHour,sys_time.wMinute,sys_time.wSecond,GetDeskId(),csMessage);
		f.WriteString(stemp);
		f.Flush();
		f.Close();

	} 
	catch(CException *e) 
	{  
		e->Delete(); 
		return ;  
	} 
} 