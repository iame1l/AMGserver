#include "StdAfx.h"
#include "ClientGamedlg.h"

BEGIN_MESSAGE_MAP(CClientGameDlg, CLoveSendClass)
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
END_MESSAGE_MAP()

//构造函数
CClientGameDlg::CClientGameDlg() : CLoveSendClass(IDD_GAME_FRAME)
{
	m_bUserAgree = false;//玩家是否举手了
} 

//析构函数
CClientGameDlg::~CClientGameDlg()
{

}

int CClientGameDlg::GetGameStatus()
{
	if (this)
	{
		return GetStationParameter();
	}
	else
	{
		return 0;
	}    

}

//初始化函数
BOOL CClientGameDlg::OnInitDialog()
{
	CLoveSendClass::OnInitDialog();
	return TRUE;
}

//设置游戏状态
bool CClientGameDlg::SetGameStation(void * pStationData, UINT uDataSize)
{
	GameStation_Base *pGameStation = (GameStation_Base *) pStationData;
	SetStationParameter(pGameStation->byGameStation);
	m_byMyStation = GetMeUserInfo()->bDeskStation;

	CString cs;
	cs.Format("llj:jqr::GetStationParameter = %d",GetStationParameter());
	OutputDebugString(cs);
	m_GameData.m_mjRule.byBlockTime		= pGameStation->byBlockTime;	//拦牌思考时间

	switch(GetStationParameter())
	{
	case GS_WAIT_SETGAME:	//游戏没有开始状态
	case GS_WAIT_ARGEE:		//等待玩家开始状态
	case GS_WAIT_NEXT:		//等待下一盘游戏开始
		{
			//m_GameData.m_mjRule.byBeginTime		= pGameStation->byBeginTime;	//开始等待时间
			//m_GameData.m_mjRule.byOutTime		= pGameStation->byOutTime;		//出牌时间
			//m_GameData.m_mjRule.bForceTuoGuan	= pGameStation->bForceTuoGuan;	//强退是否托管
			SetGameTimer(m_byMyStation,rand()%5 + 1,TIME_BEGIN);
			break;
		}
	}
	return true;
}

//游戏消息处理函数
bool CClientGameDlg::HandleGameMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	switch(pNetHead->bAssistantID)
	{
	case THING_2SEZI_NT:
		{
			break;
		}
	case THING_2SEZI_DIR:
		{
			break;
		}
	case THING_2SEZI_GP:
		{
			break;
		}
	case THING_SEND_PAI:
		{
			OnNotifyDeal(pNetData,uDataSize);
			break;
		}
	case THING_BEGIN_OUT:
		{
			OnBeginOutCard( pNetData,  uDataSize); 
			m_GameData.m_byThingDoing = THING_BEGIN_OUT;//正在执行的事件
			break;
		}
	case THING_ALL_BUHUA:		//	所有玩家补花事件	DWJ
		{
			m_GameData.m_byThingDoing = THING_ALL_BUHUA;
			break;
		}
	case THING_OUT_PAI:
		{
			KillAllTimer();
			if(uDataSize != sizeof(tagOutPaiEx))
			{
				return false;
			}

			tagOutPaiEx *pOutPaiResult = (tagOutPaiEx *)pNetData;
			if(NULL == pOutPaiResult)
			{
				return false;
			}
			m_GameData.m_byThingDoing = THING_OUT_PAI;

			//拷贝出牌数据
			m_GameData.T_OutPai.Clear();
			memcpy(&m_GameData.T_OutPai, pOutPaiResult, sizeof(m_GameData.T_OutPai));
			break;
		}
	case THING_ZHUA_PAI:
		{
			OnNotifyZhuaPai(pNetData, uDataSize);
			m_GameData.m_byThingDoing = THING_ZHUA_PAI;//正在执行的事件

			break;
		}
	case THING_CPG_NOTIFY:
		{
			//OnNotifyBlock(pNetData, uDataSize);
			break;
		}
	case THING_HAVE_ACTION:
		{
			OnNotifyBlock(pNetData, uDataSize);
			break;
		}
	case THING_ONE_BUHUA:		//	单个玩家补花
		{
			m_GameData.m_byThingDoing = THING_ONE_BUHUA;

			break;
		}
	case THING_GUO_PAI:			//	放弃拦牌	
		{

			break;
		}
	case THING_CHI_PAI:			//	吃牌结果消息
		{
			m_GameData.m_byThingDoing = THING_CHI_PAI;

			break;
		}
	case THING_PENG_PAI:		//	碰牌结果消息
		{
			m_GameData.m_byThingDoing = THING_PENG_PAI;

			break;
		}
	case THING_GANG_PAI:		//	杠牌结果消息
		{
			m_GameData.m_byThingDoing = THING_GANG_PAI;

			break;
		}
	case THING_HU_PAI:			 //	胡牌结果消息
		{
			//OnNotifyHuPai( buffer,  nLen);            
			break;
		}
	case THING_ZHONG_NIAO:
		{
			//OnNotifyZhongNiao(buffer,nLen);
			break;
		}
	case THING_ENG_HANDLE:
		{
			m_GameData.m_byThingDoing = THING_ENG_HANDLE;

			SetGameTimer(m_byMyStation,rand()%5 + 1,TIME_BEGIN);
			break;
		}
	default:
		{
			break;
		}
	}
	return true;
}

//重新设置界面
void CClientGameDlg::ResetGameFrame()
{
	return __super::ResetGameFrame();
}


// 控件开始按钮被点击
bool CClientGameDlg::OnControlHitBegin()
{
	OutputDebugString("llj:jqr：OnControlHitBegin ");
	
	SendGameData(MDM_GM_GAME_NOTIFY,ASS_GM_AGREE_GAME,0);

	KillAllTimer();
    return true;
}

//清除所有定时器
void CClientGameDlg::KillAllTimer()
{
	KillGameTimer(TIME_BEGIN);
	KillGameTimer(TIME_OUT_CARD);
	return;
}

void CClientGameDlg::OnTimer(UINT nIDEvent)
{   
	CLoveSendClass::OnTimer(nIDEvent);

}

//能否离开桌子,如果能,就在客户端收到消息后马上离开.
BOOL CClientGameDlg::CanLeftDesk()
{
	return TRUE;
}

//退出
void CClientGameDlg::OnCancel()
{
	KillAllTimer();
	AFCCloseFrame();
	__super::OnCancel();
}


//桌号换算到视图位置
BYTE CClientGameDlg::ViewStation(BYTE bDeskStation)
{
	return __super::ViewStation(bDeskStation);
}

BOOL CClientGameDlg::PreTranslateMessage(MSG* pMsg)
{
	return CLoveSendClass::PreTranslateMessage(pMsg);
}

//定时器消息
bool CClientGameDlg::OnGameTimer(BYTE bDeskStation, UINT uTimeID, UINT uTimeCount)
{
    switch (uTimeID)
	{
	case TIME_BEGIN:      // 自动开始
		{
			KillGameTimer(uTimeID);
			OnControlHitBegin();
		}break;
	case TIME_OUT_CARD: //出牌
		{
			KillGameTimer(uTimeID);		
			AutoOutCard();
		}break;
	case TIME_BLOCK_ME://自己拦牌 ：胡牌，吃听吃牌后
		{
			KillGameTimer(uTimeID);	
			//AutoBlockMe();
		}break;
	case TIME_BLOCK_CARD://拦别人的牌：
		{
			KillGameTimer(uTimeID);	
			//AutoBlockCard();
		}break;
	case TIME_WAIT_BLOCK_ME://等待拦牌
		{
			KillGameTimer(uTimeID);	
			//超时没有反应，自动出牌
			//AutoBlockCard();
		}break;
	case TIME_WAIT_BLOCK_CARD: //等待拦别人的牌
		{
			KillGameTimer(uTimeID);	
			//超时没有反应，自动放弃
			//TCMD_REQUEST_BLOCK cmd;
			//cmd.ucFlag = ACTION_EMPTY;
			//SendGameData((void*)&cmd,sizeof(cmd),MDM_GM_GAME_NOTIFY,0,0);//
		}break;
	case TIME_OPERATE_HU:
		{
			KillGameTimer(uTimeID);	
			tagHuPaiEx hu;
			hu.Clear();
			hu.byUser	= m_byMyStation;
			SendGameData(&hu,sizeof(hu),MDM_GM_GAME_NOTIFY,THING_HU_PAI,0);
			break;
		}
	case TIME_OPERATE_GANG:
		{
			KillGameTimer(uTimeID);	
			//如果是出牌的话 那么杠牌就是只杠玩家出的牌
			if(m_GameData.m_byThingDoing == THING_OUT_PAI)	//明杠
			{
				if(m_GameData.T_OutPai.byPs == 255)
				{
					return  false;   
				}

				tagGangPaiEx gang;
				gang.Clear();
				gang.byPs		= m_GameData.T_OutPai.byPs;
				gang.byUser		= m_byMyStation;
				gang.byBeGang	= m_GameData.T_OutPai.byUser;
				for(int i=0; i<4; i++)
				{
					gang.byGangPai[i] = m_GameData.T_OutPai.byPs;
				}
				SendGameData(&gang,sizeof(gang),MDM_GM_GAME_NOTIFY,THING_GANG_PAI,0);

			}
			else if(m_GameData.m_byThingDoing == THING_ZHUA_PAI || m_GameData.m_byThingDoing == THING_ONE_BUHUA || m_GameData.m_byThingDoing == THING_BEGIN_OUT)	//当前是抓牌 ||补花||开始出牌
			{
				//手上是否有多个杠
				int iGangCount = 0;
				for(int i=0; i<4; i++)
				{
					if(m_GameData.T_CPGNotify[m_byMyStation].m_iGangData[i][0] != 255 && m_GameData.T_CPGNotify[m_byMyStation].m_iGangData[i][1] != 255)
					{
						iGangCount++;
					}
				}

				if(iGangCount > 1)
				{
					//设置手牌选择杠牌状态
					//SendGameData(iGangCount,true);
					SendGameData(NULL,0,MDM_GM_GAME_NOTIFY,THING_GUO_PAI,0);
				}
				else	//只有一个暗杠
				{
					tagGangPaiEx gang;
					gang.Clear();
					gang.byPs		= m_GameData.T_CPGNotify[m_byMyStation].m_iGangData[0][1];
					gang.byUser		= m_byMyStation;
					gang.byBeGang	= m_byMyStation;
					for(int i=0; i<4; i++)
					{
						gang.byGangPai[i] = m_GameData.T_OutPai.byPs;
					}
					SendGameData(&gang,sizeof(gang),MDM_GM_GAME_NOTIFY,THING_GANG_PAI,0);
				}
			}
			break;
		}
	case TIME_OPERATE_PENG:
		{
			KillGameTimer(uTimeID);	
			tagPengPaiEx peng;
			peng.Clear();
			peng.byUser	= m_byMyStation;
			peng.byPs	  = m_GameData.T_OutPai.byPs;
			peng.byBePeng = m_GameData.T_OutPai.byUser;
			SendGameData(&peng,sizeof(peng),MDM_GM_GAME_NOTIFY,THING_PENG_PAI,0);
			break;
		}
	case TIME_OPERATE_CHI:
		{
			KillGameTimer(uTimeID);	
			//tagChiPaiEx hu;
			//hu.Clear();
			//hu.byUser	= m_byMyStation;
			//SendGameData(&hu,sizeof(hu),MDM_GM_GAME_NOTIFY,THING_CHI_PAI,0);
			break;
		}
		
    default:
        break;
	}
	return true;
}

 //  @brief 房间基础信息
void  CClientGameDlg::OnNotifyBaseInfo(void * buffer, int nLen)
{
	OutputDebugString("麻将机器人：OnNotifyBaseInfo ");


}


//  @brief 处理设置连局信息(游戏开始发送的的第一条游戏消息)
void  CClientGameDlg::OnNotifyRunNbr(void * buffer, int nLen)
{
	OutputDebugString("麻将机器人：OnNotifyRunNbr ");
	//m_byGameState = MJ_STATE_DEAL;
}



//  @brief 接受到开局发牌消息后
void  CClientGameDlg::OnNotifyDeal(void * buffer, int nLen)
{
	if(nLen != sizeof(tagSendPaiEx))
	{
		return;
	}
	tagSendPaiEx *pSendPai = (tagSendPaiEx*)buffer;
	if(NULL == pSendPai)
	{
		return;
	}

	OutputDebugString("麻将机器人：OnNotifyDeal ");
	// 发牌状态
	//m_byGameState = MJ_STATE_DEAL;//设置状态为发牌状态
	// 此状态提供给框架使用
	SetStationParameter(GS_SEND_CARD);
	//拷贝发牌数据
	memcpy(&m_GameData.T_SendPai, pSendPai, sizeof(m_GameData.T_SendPai));
	// 发牌完成, 需要通知服务端
	for(int i=0;i<PLAY_COUNT;++i)
	{
		//保存玩家的手牌数据
		m_UserData.SetHandPaiData(i,pSendPai->m_byArHandPai[i],pSendPai->m_byArHandPaiCount[i]);
		//手牌张数
		//m_UserData.m_byArHandPaiCount[i] = pSendPai->m_byArHandPaiCount[i];
		//保存玩家的门牌数据
		m_UserData.SetMenPaiData(i,pSendPai->byMenPai[i]);
	}
}

//  @brief 接受到结束消息
void  CClientGameDlg::OnNotifyFinish(void * buffer, int nLen)
{	
	ReSetGameData();
	SetGameTimer(m_byMyStation, rand()%6+3,TIME_BEGIN);//启动开始计时器										
}
void  CClientGameDlg::ReSetGameData()
{
	m_GameData.InitData();
	m_UserData.InitData();
}
//  @brief 接受到修改旁观状态消息
void  CClientGameDlg::OnNotifyChangeLookOn(void * buffer, int nLen)
{
	OutputDebugString("麻将机器人：OnNotifyChangeLookOn ");
}

//  @brief 接受到刷新手牌消息
void CClientGameDlg::OnNotifyUpdateHandTiles(void * buffer, int nLen)
{
	/*OutputDebugString("麻将机器人：OnNotifyUpdateHandTiles ");
	TCMD_NOTIFY_UPDATEHANDTILES * pCmd = (TCMD_NOTIFY_UPDATEHANDTILES *)buffer;
	if (pCmd == NULL)
	{
	return ;
	}
	OutputDebugString("麻将机器人：OnNotifyUpdateHandTiles 1");
	if (pCmd->ucChair == GetMeUserInfo()->bDeskStation)
	{
	memset(m_UserData.m_byArHandPai[GetMeUserInfo()->bDeskStation],255,sizeof(m_UserData.m_byArHandPai[GetMeUserInfo()->bDeskStation]));
	for (int i = 0; i < pCmd->ucHandCount; i++)
	{
	if (pCmd->ucHandTiles[i] != TILE_BEGIN)
	{
	m_UserData.AddToHandPai(GetMeUserInfo()->bDeskStation,pCmd->ucHandTiles[i]);
	}
	}
	m_UserData.SortHandPai(GetMeUserInfo()->bDeskStation,false);

	}*/
}

// 接收到GM命令执行结果消息---
void CClientGameDlg::OnNotifyGMCmd(void * buffer, int nLen)
{

}

// 接收提示信息---
void CClientGameDlg::OnNotifyShowDlg(void * buffer, int nLen)
{

}

//  接收到在线玩家的信息---
void CClientGameDlg::serverOnlineUsersInfoNotify(void *buffer, int nLen)
{

}

// 接收到精牌的信息---
void CClientGameDlg::serverJpInfoNotify(void *buffer, int nLen)
{
	//if (buffer == NULL) return ;
	//if (nLen != sizeof(SERVER_TCMD_NOFITY_JP)) return ;

	//SERVER_TCMD_NOFITY_JP *pTempJp = (SERVER_TCMD_NOFITY_JP *)buffer;
	//BYTE jp_card_ = pTempJp->jp_card_;

	//if (jp_card_ >= 1 && jp_card_ <= 37) 
	//{
	//	m_UserData.m_StrJing.Init();
	//	m_UserData.m_StrJing.AddJing(jp_card_);
	//}

}

// 接收到服务端买顶底的信息---
void CClientGameDlg::serverMddInfoNofity(void *buffer, int nLen)
{

}

// 接收到服务端买顶底的信息----
void CClientGameDlg::serverMddTimeoutNotify()
{

}

// 庄家买底的通知---
void CClientGameDlg::serverZmInfoNotify(void *buffer, int nLen)
{

}

//获取连庄的消息---
void CClientGameDlg::serverLianInfoNotify(void *buffer, int nLen)
{

}

//@brief 自定义初使化数据
void CClientGameDlg::InitData()
{
	m_UserData.InitData();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//机器人自动出牌
void CClientGameDlg::AutoOutCard()
{
	BYTE byData = m_UserData.GetLastHandPai(m_byMyStation);
	if (m_UserData.IsOutPaiPeople(m_byMyStation))
	{
		tagOutPaiEx TOutPai;
		TOutPai.Clear();
		TOutPai.byPs = byData;
		TOutPai.byUser = m_byMyStation;
		TOutPai.bTing = false;//听牌
		SendGameData(&TOutPai, sizeof(TOutPai), MDM_GM_GAME_NOTIFY, THING_OUT_PAI,0);
	}
}

//获取最无用的牌
BYTE CClientGameDlg::FindUselessPai()
{
	char buff[500];
	m_UserData.SortHandPai(GetMeUserInfo()->bDeskStation,false);//手牌排序
	//正常出牌：查找最无用的牌
	int index = 0;
	BYTE outpai = m_UserData.GetLastHandPai(GetMeUserInfo()->bDeskStation);//获取最后抓到的牌
	BYTE handpai[17];
	int paiValue[17];	//对应每张手牌的牌值
	memset(handpai,255,sizeof(handpai));	
	memset(paiValue,-1,sizeof(paiValue));	
	for(int i=0;i<17;++i)
	{//获取手牌数据
		handpai[i] = m_UserData.m_byArHandPai[GetMeUserInfo()->bDeskStation][i];
	}
	for(int i=0;i<17;++i)
	{
		if(handpai[i] == 255)
			continue;
		paiValue[i] = GetPaiValue(handpai[i]);//获取牌值
	}
	for(int i=0;i<17;++i)
	{
		if(/*paiValue[i]<0 ||*/ handpai[i] == 255)
			continue;
		//不可或缺的牌增加牌值
		if(IsImport(handpai[i]))//确定不能打的重要牌
		{
			paiValue[i] = 10000000;
		}
	}
	//查找值最小的牌打出
	for(int i=0;i<17;++i)
	{
		if(paiValue[i]<0 || handpai[i] == 255)
			continue;
		if(paiValue[i] < paiValue[index])
			index = i;
	}
	if(handpai[index] != 255)
	{
		outpai = handpai[index];
	}
	return outpai;
}

//获取牌值
int CClientGameDlg::GetPaiValue(BYTE pai)
{
	if(pai == 255 || pai%10 == 0)
		return -1;
	int value = 0;
	//重复牌的值
	int count = m_UserData.GetAHandPaiCount(GetMeUserInfo()->bDeskStation,pai);
	value = count*10;
	if(pai%10 == 1)
	{
		//存在大一相连
		if(m_UserData.IsHaveAHandPai(GetMeUserInfo()->bDeskStation,pai+1))
		{
			value += 2;
		}
		//存在大二相连
		if(m_UserData.IsHaveAHandPai(GetMeUserInfo()->bDeskStation,pai+2))
		{
			value += 1;
		}
	}
	else if(pai%10 == 2)
	{
		//存在小一相连
		if(m_UserData.IsHaveAHandPai(GetMeUserInfo()->bDeskStation,pai-1))
		{
			value += 2;
		}
		//存在大一相连
		if(m_UserData.IsHaveAHandPai(GetMeUserInfo()->bDeskStation,pai+1))
		{
			value += 2;
		}
		//存在大二相连
		if(m_UserData.IsHaveAHandPai(GetMeUserInfo()->bDeskStation,pai+2))
		{
			value += 1;
		}
	}
	else if(pai%10 == 8)
	{
		//存在小二相连
		if(m_UserData.IsHaveAHandPai(GetMeUserInfo()->bDeskStation,pai-2))
		{
			value += 1;
		}
		//存在小一相连
		if(m_UserData.IsHaveAHandPai(GetMeUserInfo()->bDeskStation,pai-1))
		{
			value += 2;
		}
		//存在大一相连
		if(m_UserData.IsHaveAHandPai(GetMeUserInfo()->bDeskStation,pai+1))
		{
			value += 2;
		}
	}
	else if(pai%10 == 9)
	{
		//存在小二相连
		if(m_UserData.IsHaveAHandPai(GetMeUserInfo()->bDeskStation,pai-2))
		{
			value += 1;
		}
		//存在小一相连
		if(m_UserData.IsHaveAHandPai(GetMeUserInfo()->bDeskStation,pai-1))
		{
			value += 2;
		}
	}
	else //3—7之间的数字
	{
		//存在小二相连
		if(m_UserData.IsHaveAHandPai(GetMeUserInfo()->bDeskStation,pai-2))
		{
			value += 1;
		}
		//存在小一相连
		if(m_UserData.IsHaveAHandPai(GetMeUserInfo()->bDeskStation,pai-1))
		{
			value += 2;
		}
		//存在大一相连
		if(m_UserData.IsHaveAHandPai(GetMeUserInfo()->bDeskStation,pai+1))
		{
			value += 2;
		}
		//存在大二相连
		if(m_UserData.IsHaveAHandPai(GetMeUserInfo()->bDeskStation,pai+2))
		{
			value += 1;
		}
	}
	////幺九牌，加大牌值
	//if(m_UserData.IsYaoJiu(pai))
	//{
	//	value += 2;
	//}

	return value;
}

//是非常重要的牌，或者不可或缺的，比如幺九牌。增加牌值，减少被打出的机会
bool CClientGameDlg::IsImport(BYTE pai)
{
	//财神不打出去
	if(m_UserData.m_StrJing.CheckIsJing(pai))
		return true;
	////唯一的幺九和刻子顺子不打，
	////幺九
	//if(m_UserData.IsOnlyYaoJiu(m_byMyStation,pai))
	//{
	//	return true;
	//}
	////刻子
	//if(m_UserData.GetAHandPaiCount(m_byMyStation,pai) == 3)
	//{
	//	if(m_UserData.GetKeziNum(m_byMyStation)<=1)
	//		return true;
	//}
	////花色
	//if(m_UserData.IsOnlyNeedSe(m_byMyStation,pai))
	//{
	//	return true;
	//}
	return false;
}



void CClientGameDlg::OnBeginOutCard(void *buffer, int nLen)
{
	if(nLen != sizeof(tagBeginOutPaiEx))
	{
		return;
	}
	tagBeginOutPaiEx *pBeginOutPai = (tagBeginOutPaiEx*)buffer;
	if(NULL == pBeginOutPai)
	{
		return;
	}
	m_GameData.m_byThingDoing = THING_BEGIN_OUT;//正在执行的事件
	SetStationParameter(GS_PLAY_GAME);

	m_GameData.T_BeginOutPai.Clear();
	memcpy(&m_GameData.T_BeginOutPai, pBeginOutPai, sizeof(m_GameData.T_BeginOutPai));
	for(int i=0;i<PLAY_COUNT;++i)
	{
		//保存玩家的手牌数据
		m_UserData.SetHandPaiData(i,pBeginOutPai->m_byArHandPai[i],pBeginOutPai->m_byArHandPaiCount[i]);
		//手牌张数
		m_UserData.m_byArHandPaiCount[i] = pBeginOutPai->m_byArHandPaiCount[i];
		//保存玩家的门牌数据
		m_UserData.SetMenPaiData(i,pBeginOutPai->byMenPai[i]);	
	}
	if(m_byMyStation == pBeginOutPai->byUser)
		SetGameTimer(m_byMyStation,rand()%3+2,TIME_OUT_CARD);
}

void CClientGameDlg::OnNotifyZhuaPai(void *buffer, int nLen)
{
	if(nLen != sizeof(tagZhuaPaiEx))
	{
		return;
	}
	tagZhuaPaiEx *pZhuaPai = (tagZhuaPaiEx *)buffer;
	if(NULL == pZhuaPai)
	{
		return;
	}

	//先清空抓牌数据
	m_GameData.T_ZhuaPai.Clear();
	//拷贝抓牌数据
	memcpy(&m_GameData.T_ZhuaPai, pZhuaPai, sizeof(m_GameData.T_ZhuaPai));

	SetStationParameter(GS_PLAY_GAME);
	m_GameData.m_byThingDoing = THING_ZHUA_PAI;//正在执行的事件

	m_UserData.m_byNowOutStation = pZhuaPai->byUser; //当前出牌位置

	for(int i=0;i<PLAY_COUNT;++i)
	{
		//保存玩家的门牌数据
		m_UserData.SetMenPaiData(i,pZhuaPai->byMenPai[i]);	
	}

	m_UserData.SetHandPaiData(pZhuaPai->byUser,pZhuaPai->m_byArHandPai[pZhuaPai->byUser],pZhuaPai->m_byArHandPaiCount[pZhuaPai->byUser]);
	
	m_UserData.m_byLastZhuaPai[pZhuaPai->byUser] = pZhuaPai->byPs;

	if(pZhuaPai->byUser == m_byMyStation)
		SetGameTimer(m_byMyStation,rand()%3 + 2,TIME_OUT_CARD);
}

void CClientGameDlg::OnNotifyBlock(void *buffer, int nLen)
{
	//因为服务端是以数组形式发过来的数据包 所以这里长度判断要*PLAY_COUNT
	if(nLen != sizeof(HaveAction))
	{
		return;
	}

	HaveAction *pCPGNotify = (HaveAction *)buffer;
	if(NULL == pCPGNotify)
	{
		return;
	}
	KillGameTimer(TIME_OUT_CARD);
	//拷贝拦牌数据
	memcpy(&m_GameData.T_CPGNotify, pCPGNotify->T_CPGNotify, sizeof(m_GameData.T_CPGNotify));
	if(m_GameData.T_CPGNotify[m_byMyStation].bHu)
	{
		////请求胡牌
		//tagHuPaiEx hu;
		//hu.Clear();
		//hu.byUser	= m_byMyStation;
		//SendGameData(&hu,sizeof(hu),MDM_GM_GAME_NOTIFY,THING_HU_PAI,0);
		OutputDebugString("lbtestjqr:::::::启动胡牌定时器");
		SetGameTimer(0, rand() % 3 + 2,TIME_OPERATE_HU);
	}
	bool bCanOperate = true;
	for(int i=0; i<PLAY_COUNT; i++)
	{
		if(i == m_byMyStation)
		{
			continue;
		}

		if(m_GameData.T_CPGNotify[i].bHu)
		{
			bCanOperate = false;
			break;
		}
	}
	if(!bCanOperate)
	{
		SendGameData(NULL,0,MDM_GM_GAME_NOTIFY,THING_GUO_PAI,0);
	}
	else if(m_GameData.T_CPGNotify[m_byMyStation].bGang)
	{
		OutputDebugString("lbtestjqr:::::::启动干牌定时器");
		SetGameTimer(0,rand()%3+2,TIME_OPERATE_GANG);
	}
	else if(m_GameData.T_CPGNotify[m_byMyStation].bPeng)
	{
		OutputDebugString("lbtestjqr:::::::启动碰牌定时器");
		SetGameTimer(0, rand() % 3 + 2,TIME_OPERATE_PENG);
	}
	else if(m_GameData.T_CPGNotify[m_byMyStation].bChi)
	{
		OutputDebugString("lbtestjqr:::::::启动吃牌定时器");
		SetGameTimer(0, rand() % 3 + 2,TIME_OPERATE_CHI);
	}
	OutputDebugString("lbtestjqr:::::::不能操作，不管他");
}

/****************************************************************************/

UserData::UserData(void)
{
	memset(m_byArHandPai,255,sizeof(m_byArHandPai));	/// 手牌不超过20张，255为结束标志
	memset(m_byArOutPai,255,sizeof(m_byArOutPai));		/// 出牌不超过160张，255为结束标志
	memset(m_byArHuaPai,255,sizeof(m_byArHuaPai));		/// 花牌不超过10张，255为结束标志
	memset(m_iArMenPai,255,sizeof(m_iArMenPai));		/// 门牌不超过40张，255为结束标志
	memset(m_byGuanXi,0,sizeof(m_byGuanXi));			/// 吃碰杠关系数据
	memset(m_bEnableCard,1,sizeof(m_bEnableCard));		/// 是否可用的牌
	memset(m_bIsBoy,0,sizeof(m_bIsBoy));				/// 控件变量	
	memset(m_byCanOutCard,255,sizeof(m_byCanOutCard));	/// 停牌或其他操作后能打出的牌
	memset(m_byMenFeng,255,sizeof(m_byMenFeng));		/// 门风	
	memset(m_bFoceLeave,0,sizeof(m_bFoceLeave));		///	玩家是否已经断线
	memset(m_bTing,0,sizeof(m_bTing));					///	玩家是否听牌
	memset(m_bTuoGuan,0,sizeof(m_bTuoGuan));			///	玩家是否托管
	memset(m_bWinner,0,sizeof(m_bWinner));				///	玩家是否赢家
	memset(m_byLastZhuaPai,255,sizeof(m_byLastZhuaPai));///	最后抓到的牌
	memset(m_bySetMoPai,255,sizeof(m_bySetMoPai));//抓牌强制指定值（测试用）
	memset(m_bSiziFinish,0,sizeof(m_bSiziFinish));			///	玩家是否返回了色子结束消息
	memset(m_bHaveOut,0,sizeof(m_bHaveOut));			///	记录玩家是否出过牌了

	m_bWaitSezi = false;//是否处理等待色子动画结束状态

	m_StrJing.Init();
	for(int i=0;i<4;++i)
	{//换牌数据结构
		m_SetData[i].Init();
	}
	//初始化牌
	m_MenPai.Init();

	m_byFoceLeavCount=0;		///玩家离开的数量

	m_byStation = 255;			///当前玩家的逻辑位置（客户端使用）
	byPlayCount = 0;			///游戏已经进行的局数（强退，安全退出，解散等重新开始计算）
	m_byQuanFeng = 255;			///圈风
	m_bIsDuoXiang = false;		///是否一炮多响


	m_bMeOutPai = false;		//是否我出牌
	m_bySelectIndex = 255;		///当前选中排索引
	m_bisTing = 0;				///当前玩家是否听牌
	m_byLastOutPai = 255;		///最后打出的牌
	m_byMeZhuaPai = 255;		///当前玩家最后抓拍
	m_byNowOutStation = 255;	///当前出牌位置
	m_byLastOutStation = 255;	///上次出牌位置
	m_byBaoPaiUser = 255;		///包牌玩家
	m_byOtherOutPai = 255;		///当前其他玩家打出的牌
	///当前抓牌方向，true 顺时针，false 逆时针
	m_bZhuaPaiDir = true;
	//同意玩家离开的人数
	m_byAgreeLeaveNum = 0;

	m_byApplyMaxAction = 0;//玩家当前申请的最大事件

	m_byCanDoMaxAction = 0;//本次检查玩家能做的最大动作
}

UserData::~UserData(void)
{

}


///名称：IsHaveAHandPai
///描述：检查是否存在指定的手牌
///@param pai 要检测的牌
///@return true 有 false 无
bool UserData::IsHaveAHandPai(BYTE station,BYTE pai)
{
	if(station <0 || station>=PLAY_COUNT)
		return false;
	for(int i=0;i<HAND_CARD_NUM;i++)
	{
		if(m_byArHandPai[station][i] == pai)
			return true;
	}
	return false;
}

///名称：GetAHandPaiCount
///描述：检查手牌中某张牌的个数
///@param pai 要检测的牌
///@return 牌数量
int UserData::GetAHandPaiCount(BYTE station,BYTE pai)
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	int count = 0;
	for(int i=0;i<HAND_CARD_NUM;i++)
	{
		if(m_byArHandPai[station][i] == pai)
			count++;
	}
	return count;
}

///名称：GetHandPaiCount
///描述：检查手牌个数
///@param 
///@return 玩家手牌的总数
int UserData::GetHandPaiCount(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	int count = 0;
	for(int i=0;i<HAND_CARD_NUM;i++)
	{
		if(m_byArHandPai[station][i] != 255)
			count++;
	}
	return count;
}

///名称：CopyHandPai
///描述：拷贝玩家的手牌
///@param pai[] 牌数据, station 位置, show 是否拷贝明牌
///@return 玩家手牌总数
int UserData::CopyHandPai(BYTE pai[][HAND_CARD_NUM] ,BYTE station,bool show)
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	memset(pai,255,sizeof(pai));
	for(int i=0;i<4;++i)
	{
		int count = GetHandPaiCount(station);
		if(i == station)//自己的牌
		{
			for(int j=0;j<HAND_CARD_NUM;++j)
			{
				pai[i][j] = m_byArHandPai[station][j];
			}
		}
		else 
		{
			if(!show)//显示牌背
			{
				for(int j=0;j<HAND_CARD_NUM;++j)
				{
					if( m_byArHandPai[i][j] != 255)
					{	
						pai[i][j] = 0;
					}
					else 
					{
						pai[i][j] = m_byArHandPai[i][j];
					}
				}
			}
			else
			{
				for(int j=0;j<HAND_CARD_NUM;++j)
				{
					pai[i][j] = m_byArHandPai[i][j];
				}
			}
		}
	}
	return 0;
}

///名称：CopyOneUserHandPai
///描述：拷贝某个玩家的手牌
///@param pai[] 牌数据, station 位置
///@return 玩家手牌总数
int UserData::CopyOneUserHandPai(BYTE pai[HAND_CARD_NUM] ,BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	memset(pai,255,sizeof(pai));
	for(int j=0;j<HAND_CARD_NUM;++j)
	{
		pai[j] = m_byArHandPai[station][j];
	}
	return GetHandPaiCount(station);
}

///名称：SortHandPai
///描述：手牌排序，small是否从小到大排
///@param big 是否从到到小排序。默认从小到大排序
void UserData::SortHandPai(BYTE station,bool big)
{
	if(station <0 || station>=PLAY_COUNT)
		return ;
	///排序
	MaoPaoSort(m_byArHandPai[station],HAND_CARD_NUM,big);
	BYTE HandPai[HAND_CARD_NUM];
	for(int j=0;j<HAND_CARD_NUM;++j)
	{
		HandPai[j] = m_byArHandPai[station][j];
	}
	int index=0;
	for(int j=0;j<HAND_CARD_NUM;++j)
	{
		if(m_StrJing.CheckIsJing(HandPai[j]))
		{
			m_byArHandPai[station][index] = HandPai[j];
			index++;
		}
	}
	for(int j=0;j<HAND_CARD_NUM;++j)
	{
		if(!m_StrJing.CheckIsJing(HandPai[j]) && index<HAND_CARD_NUM)
		{
			m_byArHandPai[station][index] = HandPai[j];
			index++;
		}
	}
}

///名称：AddToHandPai
///描述：添加一张牌到手牌中
///@param pai 添加的手牌值
///@return 
void UserData::AddToHandPai(BYTE station,BYTE pai)
{
	if(station <0 || station>=PLAY_COUNT)
		return ;
	if((GetHandPaiCount(station)+3*GetGCPCount(station))>=MAX_HAND_PAI || pai==255)
	{
		return;
	}
	for(int i=0;i<HAND_CARD_NUM;i++)
	{
		if(m_byArHandPai[station][i] == 255)
		{
			m_byArHandPai[station][i] = pai;
			break;
		}
	}
}

///名称：SetHandPaiData
///描述：设置手牌数据
///@param  pai[] 传入的手牌数组
void UserData::SetHandPaiData(BYTE station,BYTE pai[],BYTE byCount)
{
	if(station <0 || station>=PLAY_COUNT)
	{
		return ;
	}
	for(int i=0;i<HAND_CARD_NUM;i++)
	{
		m_byArHandPai[station][i] = pai[i];
	}
	m_byArHandPaiCount[station] = byCount;
}

///名称：DelleteAHandPai
///描述：删除一张指定的手牌
///@param pai 要删除的牌
void UserData::DelleteAHandPai(BYTE station,BYTE pai)
{
	if(station <0 || station>=PLAY_COUNT)
		return ;
	for(int i=0;i<HAND_CARD_NUM;i++)
	{
		if(m_byArHandPai[station][i] == pai)
		{
			m_byArHandPai[station][i] = 255;
			break;
		}
	}
	MaoPaoSort(m_byArHandPai[station],HAND_CARD_NUM,false);

	BYTE HandPai[HAND_CARD_NUM];
	for(int j=0;j<HAND_CARD_NUM;++j)
	{
		HandPai[j] = m_byArHandPai[station][j];
	}
	int index=0;
	for(int j=0;j<HAND_CARD_NUM;++j)
	{
		if(m_StrJing.CheckIsJing(HandPai[j]))
		{
			m_byArHandPai[station][index] = HandPai[j];
			index++;
		}
	}
	for(int j=0;j<HAND_CARD_NUM;++j)
	{
		if(!m_StrJing.CheckIsJing(HandPai[j]) && index<HAND_CARD_NUM)
		{
			m_byArHandPai[station][index] = HandPai[j];
			index++;
		}
	}
}

///名称：SetHandPaiBack
///描述：设置手牌牌背
///@param num设置的数量
///@return 
void UserData::SetHandPaiBack(BYTE station,BYTE num)
{
	if(station <0 || station>=PLAY_COUNT)
		return ;
	memset(m_byArHandPai[station],255,sizeof(m_byArHandPai[station]));
	for(int i=0;i<num;i++)
	{
		m_byArHandPai[station][i] = 0;
	}
}

///名称：GetLastHandPai
///描述：获得最后一张手牌
///@param 
BYTE UserData::GetLastHandPai(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return 255;
	for(int i=HAND_CARD_NUM-1;i>=0;i--)
	{
		if(m_byArHandPai[station][i] != 255 && m_bEnableCard[station][i] && m_byArHandPai[station][i] != 35)
		{
			return m_byArHandPai[station][i];
		}
	}
	return 255;
}

///名称：GetSelectHandPai
///描述：获得选中的一张手牌
///@param 
///@return 
BYTE UserData::GetSelectHandPai(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return 255;
	if(m_bySelectIndex<HAND_CARD_NUM && m_bySelectIndex!=255)
		return m_byArHandPai[station][m_bySelectIndex];
	return 255;
}

///名称：IsOutPaiPeople
///描述：是否出牌玩家
///@param 
///@return  true 是 ,false 不是
bool UserData::IsOutPaiPeople(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return false;
	if(GetHandPaiCount(station)%3==2)
	{
		return true;
	}
	return false;
}

///名称：IsHuaPaiInHand
///描述：检测手牌中是否存在花牌
///@param 
///@return  true 是 ,false 不是
bool UserData::IsHuaPaiInHand(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return false;
	for(int i=0;i<HAND_CARD_NUM;i++)
	{
		if(CheckIsHuaPai(m_byArHandPai[station][i]))
		{
			return true;
		}
	}
	return false;
}

///名称：MoveHuaPaiFormHand
///描述：将手牌中的花牌移到花牌数组中
///@param 
///@return  花牌的数量
int UserData::MoveHuaPaiFormHand(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	int hua = 0;
	for(int i=0;i<HAND_CARD_NUM;i++)
	{
		if(m_byArHandPai[station][i] == 255)
		{
			break;
		}
		if(CheckIsHuaPai(m_byArHandPai[station][i]))
		{
			hua++;
			AddToHuaPai(station,m_byArHandPai[station][i]);
			DelleteAHandPai(station,m_byArHandPai[station][i]);
		}
	}
	return hua;
}

///通过索引获取一张手牌
BYTE UserData::GetHandPaiFromIndex(BYTE station,int index)
{
	if(station <0 || station>=PLAY_COUNT)
		return 255;
	if(station <0 || station>3)
		return 255;
	return m_byArHandPai[station][index];
}

///通过索引设置一张手牌
bool UserData::SetHandPaiFromIndex(BYTE station,int index,BYTE pai)
{
	if(station <0 || station>3 || !CMjRef::IsValidateMj(pai))
		return false;
	if(m_byArHandPai[station][index] == 255)
		return false;
	m_byArHandPai[station][index] = pai;
	return true;
}

///索引是否有牌
bool UserData::IsHaveHandPaiFromIndex(BYTE station,int index)
{
	if(station <0 || station>3 || index < 0 || index >= HAND_CARD_NUM)
	{
		OutputDebugString(TEXT("二人麻将： IsHaveHandPaiFromIndex 数据错误"));
		return false;
	}
	if(m_byArHandPai[station][index] == 255)
	{
		OutputDebugString(TEXT("二人麻将：IsHaveHandPaiFromIndex 无牌"));
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///名称：GetUserGangNum
///描述：获取玩家杠牌的个数(暗杠，明杠，补杠)
///@param &an 暗杠数量, &ming 明杠数量 ,  &bu 补杠数量
///@return 杠牌的总个数
int UserData::GetUserGangNum(BYTE station,BYTE &an,BYTE &ming , BYTE &bu)
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	int all = 0;
	an = 0;
	ming = 0;
	bu = 0;
	for(int i=0;i<5;i++)
	{
		switch(m_UserGCPData[station][i].byType)
		{
		case ACTION_AN_GANG://暗杠
			an++;
			break;
		case ACTION_BU_GANG://补杠
			bu++;
			break;
		case ACTION_MING_GANG://明杠
			ming++;
			break;
		}
	}
	all = ming + an + bu;
	return all;
}

///名称：GetUserChiNum
///描述：玩家吃牌次数
///@param station 玩家位置
///@return 吃牌的总个数
int UserData::GetUserChiNum(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	int num = 0;
	for(int i=0;i<5;i++)
	{
		switch(m_UserGCPData[station][i].byType)
		{
		case ACTION_CHI://暗杠
			num++;
			break;
		}
	}
	return num;
}

///名称：GetUserPengNum
///描述：玩家杠牌次数
///@param station 玩家位置
///@return 碰牌的总个数
int UserData::GetUserPengNum(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	int num = 0;
	for(int i=0;i<5;i++)
	{
		switch(m_UserGCPData[station][i].byType)
		{
		case ACTION_PENG://暗杠
			num++;
			break;
		}
	}
	return num;
}

///名称：IsUserHaveGangPai
///描述：玩家杠了某个牌
///@param pai 牌, &type 类型
///@return  true 是 ,false 不是
bool UserData::IsUserHaveGangPai(BYTE station,BYTE pai,BYTE &type)
{	
	if(station <0 || station>=PLAY_COUNT)
		return false;
	for(int i=0;i<5;i++)
	{
		if(m_UserGCPData[station][i].byType>=ACTION_AN_GANG && m_UserGCPData[station][i].byType<=ACTION_MING_GANG && m_UserGCPData[station][i].byData[0] == pai)
		{
			type = m_UserGCPData[station][i].byType;
			return true;
		}
	}
	return false;
}
///
///名称：IsUserHavePengPai
///描述：玩家是否碰了某张牌
///@param pai 牌
///@return  true 是 ,false 不是
bool UserData::IsUserHavePengPai(BYTE station,BYTE pai)
{
	if(station <0 || station>=PLAY_COUNT)
		return false;
	for(int i=0;i<5;i++)
	{
		if(m_UserGCPData[station][i].byType == 255)
			continue;
		if(m_UserGCPData[station][i].byType == ACTION_PENG &&  m_UserGCPData[station][i].byData[0] == pai)
		{
			return true;
		}
	}
	return false;
}

///名称：IsUserHaveChiPai
///描述：玩家是否吃过某张牌
///@param pai 牌
///@return  true 是 ,false 不是
bool UserData::IsUserHaveChiPai(BYTE station,BYTE pai)
{
	if(station <0 || station>=PLAY_COUNT)
		return false;
	for(int i=0;i<5;i++)
	{
		if(m_UserGCPData[station][i].byType==ACTION_CHI &&  m_UserGCPData[station][i].iOutpai == pai)
		{
			return true;
		}
	}
	return false;
}

///名称：AddToGCP
///描述：添加一组数据到杠吃碰数组中
///@param gcpData 要添加的吃碰杠数据
void UserData::AddToGCP(BYTE station,GCPStructEx *gcpData)
{
	if(station <0 || station>=PLAY_COUNT)
		return ;
	for(int i=0;i<5;i++)
	{
		if(m_UserGCPData[station][i].byType==255)
		{
			m_UserGCPData[station][i].byType = gcpData->byType;
			m_UserGCPData[station][i].iBeStation = gcpData->iBeStation;
			m_UserGCPData[station][i].iOutpai = gcpData->iOutpai;
			m_UserGCPData[station][i].iStation = gcpData->iStation;
			for(int j=0;j<4;j++)
			{
				m_UserGCPData[station][i].byData[j] = gcpData->byData[j];
			}
			break;
		}
	}
}

///名称：DelleteAGCPData
///描述：删除杠吃碰数组中指定的数据组
///@param type 类型, pai 牌
void UserData::DelleteAGCPData(BYTE station,BYTE type ,BYTE pai)
{			
	if(station <0 || station>=PLAY_COUNT)
		return ;
	for(int i=0;i<5;i++)
	{
		if(m_UserGCPData[station][i].byType == type && (m_UserGCPData[station][i].byData[0]==pai ||m_UserGCPData[station][i].byData[1]==pai || m_UserGCPData[station][i].byData[2]==pai ))
		{
			m_UserGCPData[station][i].Init();
			break;
		}
	}
	ShortGCP(station);//排序杠吃碰数组
}	

///名称：CopyGCPData
///描述：拷贝杠吃碰数组中的数据组
///@param gcpData 吃碰杠数组
///@return 吃碰杠个数
int UserData::CopyGCPData(BYTE station,GCPStructEx gcpData[])
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	//memcpy(gcpData,m_UserGCPData,sizeof(m_UserGCPData));
	for(int i=0;i<5;i++)
	{
		gcpData[i] = m_UserGCPData[station][i];
	}
	return GetGCPCount(station);
}

///名称：SetGCPData
///描述：设置杠吃碰数组中的数据组
///@param gcpData 吃碰杠数组
///@return 
void UserData::SetGCPData(BYTE station,GCPStructEx gcpData[])
{
	if(station <0 || station>=PLAY_COUNT)
		return ;
	for(int i=0;i<5;i++)
	{
		m_UserGCPData[station][i] = gcpData[i];
	}
}

///名称：GetGCPCount
///描述：获取杠吃碰的节点个数
///@param 
///@return 获得吃碰杠总个数
int UserData::GetGCPCount(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	int count =0;
	for(int i=0;i<5;i++)
	{
		if(m_UserGCPData[station][i].byType!=255)
		{
			count++;
		}
	}
	return count;
}
///名称：GetOnePaiGCPCount
///描述：获取杠吃碰的中某种牌的数量
///@param 
///@return 某种牌的数量
int UserData::GetOnePaiGCPCount(BYTE station,BYTE pai)
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	int count =0;
	for(int i=0;i<5;i++)
	{
		if(m_UserGCPData[station][i].byType==255)
			continue;
		for(int j=0;j<4;j++)
		{
			if(m_UserGCPData[station][i].byData[j] == pai && pai!=255)
			{
				count++;
			}
		}
	}
	return count;
}
///名称：ShortGCP
///描述：排序杠吃碰数组
///@param 
void UserData::ShortGCP(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return ;
	int index=0;
	GCPStructEx m_TempGCPData[5];
	for(int i=0;i<5;i++)
	{
		m_TempGCPData[i] = m_UserGCPData[station][i];
	}
	memset(m_UserGCPData[station],255,sizeof(m_UserGCPData[station]));
	for(int i=0;i<5;i++)
	{
		if(m_TempGCPData[i].byType != 255)
		{
			m_UserGCPData[station][index] = m_TempGCPData[i];
			index++;
		}
	}
}
///名称：InitGCP
///描述：初始化杠吃碰数组
///@param 
void UserData::InitGCP(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return ;
	for(int i=0;i<5;i++)
	{
		m_UserGCPData[station][i].Init();
	}
}
//////////////////出牌操作///////////////////////////////////////////////////////////////////////////////

///名称：AddToOutPai
///描述：添加一张牌到出牌数组中
///@param pai 牌
void UserData::AddToOutPai(BYTE station,BYTE pai)
{
	if(station <0 || station>=PLAY_COUNT)
		return ;
	for(int i=0;i<OUT_CARD_NUM;i++)
	{
		if(m_byArOutPai[station][i] == 255)
		{
			m_byArOutPai[station][i] = pai;
			break;
		}
	}
}

///名称：DelleteLastOutPai
///描述：删除最后一张出牌
///@param 
void UserData::DelleteLastOutPai(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return ;
	for(int i=0;i<OUT_CARD_NUM;i++)
	{
		if(m_byArOutPai[station][i] == 255 && i>0)
		{
			m_byArOutPai[station][i-1] = 255;
			break;
		}
	}
}

///名称：SetOutPaiData
///描述：设置出牌数据
///@param pai[] 出牌数组
///@return 
void UserData::SetOutPaiData(BYTE station,BYTE pai[],BYTE byCount)
{
	if(station <0 || station>=PLAY_COUNT)
	{
		return ;
	}
	//memcpy(m_byArOutPai[station],pai,sizeof(m_byArOutPai[station]));
	for(int i=0;i<OUT_CARD_NUM;i++)
	{
		m_byArOutPai[station][i] = pai[i] ;
	}
	m_byArOutPaiCount[station] = byCount;
}

///名称：CopyOutPai
///描述：拷贝玩家的出牌
///@param pai[] 出牌数组
///@return 出牌的张数
int UserData::CopyOutPai(BYTE station,BYTE pai[])
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	//memcpy(pai,m_byArOutPai,sizeof(m_byArOutPai));
	for(int i=0;i<OUT_CARD_NUM;i++)
	{
		pai[i] = m_byArOutPai[station][i];
	}
	return GetPaiNum(station,m_byArOutPai[station],OUT_CARD_NUM);
}

///名称：GetOutPaiCount
///描述：获取玩家的出牌数量
///@param 
///@return 出牌的总数
int UserData::GetOutPaiCount(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	int num = 0;
	for(int i=0;i<OUT_CARD_NUM;i++)
	{
		if(m_byArOutPai[station][i] != 255)
		{
			num++;
		}
	}
	return num;
}
///名称：GetOutPaiCount
///描述：获取玩家某张牌的出牌数量
///@param 
///@return 某张牌出牌的数量
int UserData::GetOneOutPaiCount(BYTE station,BYTE pai)
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	int num = 0;
	for(int i=0;i<OUT_CARD_NUM;i++)
	{
		if(m_byArOutPai[station][i] != 255 && m_byArOutPai[station][i] == pai)
		{
			num++;
		}
	}
	return num;
}
/////////////////////花牌操作////////////////////////////////////////////////////////

///名称：AddToHuaPai
///描述：添加一张花牌到花牌数组中
///@param pai 花牌
void UserData::AddToHuaPai(BYTE station,BYTE pai)
{
	if(station <0 || station>=PLAY_COUNT)
		return ;
	for(int i=0;i<HUA_CARD_NUM;i++)
	{
		if(m_byArHuaPai[station][i] == 255)
		{
			m_byArHuaPai[station][i] = pai;
			break;
		}
	}
}

///名称：SetHuaPaiData
///描述：设置花牌数据
///@param pai[] 花牌数组
///@return 
void UserData:: SetHuaPaiData(BYTE station,BYTE pai[])
{
	if(station <0 || station>=PLAY_COUNT)
		return ;
	memcpy(m_byArHuaPai[station],pai,sizeof(m_byArHuaPai[station]));
}

///名称：CopyHuaPai
///描述：拷贝玩家的花牌
///@param pai[] 花牌数组
///@return 
int UserData::CopyHuaPai(BYTE station,BYTE pai[])
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	int count =0;
	for(int i=0;i<HUA_CARD_NUM;i++)
	{
		pai[i] = m_byArHuaPai[station][i];
		if(m_byArHuaPai[station][i]!=255)
		{
			count++;
		}
	}
	return count;
}

///检测是否花牌
bool UserData::CheckIsHuaPai(BYTE pai)
{
	if(pai>=CMjEnum::MJ_TYPE_FCHUN && pai != 255)
		return true;
	return false;
}
///////////////////////门牌操作//////////////////////////////////////////////////////////////////

///名称：SetMenPaiData
///描述：设置门牌数据
///@param  pai[] 门牌数组
///@return 
void UserData::SetMenPaiData(BYTE station,BYTE pai[])
{
	if(station <0 || station>=PLAY_COUNT)
		return ;
	memcpy(m_iArMenPai[station],pai,sizeof(m_iArMenPai[station]));
}

///名称：CopyMenPai
///描述：拷贝玩家的门牌
///@param  pai[] 门牌数组
///@return 玩家门牌总数
int UserData::CopyMenPai(BYTE station,BYTE pai[])
{
	if(station <0 || station>=PLAY_COUNT)
		return 0;
	for(int i=0;i<MEN_CARD_NUM;i++)
	{
		pai[i] = m_iArMenPai[station][i];
	}
	return GetPaiNum(station,m_iArMenPai[station],MEN_CARD_NUM);
}

///名称：ChangeMenPai
///描述：门牌转换
///@param  pai[] 门牌数组
///@return 玩家门牌总数
void UserData::ChangeMenPai(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return ;
	BYTE index = 0;
	bool shang = false;
	for(int i=0;i<MEN_CARD_NUM;i++)
	{
		if(m_iArMenPai[station][i]!=255 && i%2==0)
		{
			index = i/2;
			shang =true;
		}
		else if(m_iArMenPai[station][i]!=255)
		{
			index = i/2;
			shang = false;
		}
		if(i%2==0)//上面一张
		{	
			m_byMenPaiData[station][i/2][0] = m_iArMenPai[station][i];    //转换后的门牌数据
		}
		else//下面一张
		{
			m_byMenPaiData[station][i/2][1] = m_iArMenPai[station][i];    //转换后的门牌数据
		}
	}

	if(shang)//最后一张在上方，要强制绘制在下方
	{
		m_byMenPaiData[station][index][1] = m_byMenPaiData[station][index][0] ;
		m_byMenPaiData[station][index][0] = 255;
	}

}
///描述：获取某玩家的门牌数量
int UserData::GetMenPaiCount(BYTE station)
{
	if(station <0 || station>=PLAY_COUNT)
		return false;
	int count = 0;
	for(int i=0;i<MEN_CARD_NUM;++i)
	{
		if(m_iArMenPai[station][i] != 255)
		{
			count++;
		}
	}
	return count;
}

///名称：ChangeAllHandPai
///换所有手牌
///@param  station 位置, pai[] 牌, count牌数量
void UserData::ChangeAllHandPai(BYTE station,BYTE pai[],BYTE count)
{
	if(station <0 || station>=PLAY_COUNT)
		return ;
	for(int i=0;i<count;i++)
	{
		if(!CMjRef::IsValidateMj(pai[i]))
			continue;
		if(m_byArHandPai[station][i] == 255)
			break;
		m_byArHandPai[station][i] = pai[i];
	}
}

///名称：ChangeAllHandPai
///换指定手牌
///@param  station 位置, pai[] 牌, handpai[]要换掉的手牌, count牌数量
void UserData::ChangeSomeHandPai(BYTE station,BYTE pai[],BYTE handpai[],BYTE count)
{
	if(station <0 || station>=PLAY_COUNT)
		return ;
	for(int i=0;i<count;i++)
	{
		if(!CMjRef::IsValidateMj(pai[i]) || !CMjRef::IsValidateMj(handpai[i]))
			continue;
		if(IsHaveAHandPai(station,handpai[i]))
		{
			DelleteAHandPai(station,handpai[i]);
			AddToHandPai(station,pai[i]);
		}
	}
}

///名称：SetMeNextPai
///设置自己的下一张牌
///@param  station 位置,pai 牌
void UserData::SetMeNextPai(BYTE station,BYTE pai)
{

}

///名称：SetGameNextPai
///设置牌墙的下一张牌
///@param  pai 牌
void UserData::SetGameNextPai(BYTE pai)
{
	if(m_MenPai.byStartIndex == 255 )
		return;
	if(m_MenPai.byMenPai[m_MenPai.byStartIndex] == 255)
		return;
	m_MenPai.byMenPai[m_MenPai.byStartIndex]  = pai;
}


///名称：GetPaiNum
///描述：获取牌数组的有效牌张数
///@param  pai[] 要检测的牌数组, count 有效牌的张数
///@return 
int UserData::GetPaiNum(BYTE station,BYTE pai[],BYTE count)
{
	int num=0;
	for(int i=0;i<count;i++)
	{
		if(pai[i]!=255)
		{
			num++;
		}
	}
	return num;
}


///名称：InitData
///描述：初始化数据
///@param 
///@return 
void UserData::InitData()
{
	memset(m_byArHandPai,255,sizeof(m_byArHandPai));			//手牌不超过20张，255为结束标志
	memset(m_byArHandPaiCount,0,sizeof(m_byArHandPaiCount));	//手牌张数
	memset(m_byArOutPai,255,sizeof(m_byArOutPai));				//出牌不超过160张，255为结束标志
	memset(m_byArOutPaiCount,0,sizeof(m_byArOutPaiCount));		//出牌张数
	memset(m_byArHuaPai,255,sizeof(m_byArHuaPai));				//花牌不超过10张，255为结束标志
	memset(m_iArMenPai,255,sizeof(m_iArMenPai));				//门牌不超过40张，255为结束标志



	memset(m_byMenPaiData,255,sizeof(m_byMenPaiData));	//转换后的门牌数据

	memset(m_byGuanXi,0,sizeof(m_byGuanXi));			//吃碰杠关系数据
	memset(m_bEnableCard,1,sizeof(m_bEnableCard));		//是否可用的牌
	memset(m_bySetMoPai,255,sizeof(m_bySetMoPai));		//抓牌强制指定值（测试用）

	m_StrJing.Init();//初始化财神
	m_StrJing.AddJing(35);
	m_MenPai.Init();//初始化牌
	for(int i=0;i<PLAY_COUNT;++i)
	{//换牌数据结构
		m_SetData[i].Init();
		m_bHaveOut[i] = false;
	}
	for(int i=0;i<5;i++)
	{
		m_UserGCPData[0][i].Init();
		m_UserGCPData[1][i].Init();
		m_UserGCPData[2][i].Init();
		m_UserGCPData[3][i].Init();
	}	

	memset(m_bTing,0,sizeof(m_bTing));				//玩家是否听牌
	memset(m_byTingType,0,sizeof(m_byTingType));	//玩家听牌类型
	memset(m_bWinner,0,sizeof(m_bWinner));			//玩家是否赢家
	memset(m_bTuoGuan,0,sizeof(m_bTuoGuan));		//玩家是否托管
	memset(m_bFoceLeave,0,sizeof(m_bFoceLeave));	//玩家是否已经断线

	memset(m_byCanOutCard,255,sizeof(m_byCanOutCard));///停牌或其他操作后能打出的牌
	memset(m_byMenFeng,255,sizeof(m_byMenFeng));	///门风	
	memset(m_bFoceLeave,0,sizeof(m_bFoceLeave));	///	玩家是否已经断线
	memset(m_bTing,0,sizeof(m_bTing));				///	玩家是否听牌
	memset(m_bTuoGuan,0,sizeof(m_bTuoGuan));		///	玩家是否托管
	memset(m_bWinner,0,sizeof(m_bWinner));			///	玩家是否赢家
	memset(m_byLastZhuaPai,255,sizeof(m_byLastZhuaPai));///	最后抓到的牌
	memset(m_bHaveOut,0,sizeof(m_bHaveOut));			///	记录玩家是否出过牌了

	m_byFoceLeavCount=0;				////玩家离开的数量
	m_bMeOutPai = false;				///是否我出牌
	m_bySelectIndex = 255;				///当前选中排索引
	m_bisTing = 0;						///当前玩家是否听牌
	m_byLastOutPai = 255;				///最后打出的牌
	m_byMeZhuaPai = 255;				///当前玩家最后抓拍
	m_byNowOutStation = 255;			///当前出牌位置
	m_byLastOutStation = 255;			///上次出牌位置
	m_byBaoPaiUser = 255;				///包牌玩家
	m_byOtherOutPai = 255;				///当前其他玩家打出的牌
	memset(m_bSiziFinish,0,sizeof(m_bSiziFinish));			///	玩家是否返回了色子结束消息
	m_bWaitSezi = false;//是否处理等待色子动画结束状态
	///当前抓牌方向，true 顺时针，false 逆时针
	m_bZhuaPaiDir = true;
	//同意玩家离开的人数
	m_byAgreeLeaveNum = 0;
	m_byApplyMaxAction = 0;//玩家当前申请的最大事件
	m_byCanDoMaxAction = 0;//本次检查玩家能做的最大动作
} 

/***************************************************************************************/
UserDataEx::UserDataEx(void)
{
	UserData::UserData();
	m_byCaiPiaoStation = 255;//财飘位置，该值不为255是不允许吃碰杠和放炮
	memset(m_byGuanXi,0,sizeof(m_byGuanXi));//吃碰杠关系

	memset(m_bGangKai,0,sizeof(m_bGangKai));//杠开状态玩家
	memset(m_bOutJing,0,sizeof(m_bOutJing));//玩家是否打出了财神牌

	m_byLianZhuang = 0;//连庄数
	m_bTianHu = true;		//庄家天糊状态
	m_bChanPai = false;		//玩家是否铲牌

	memset(m_bLookBao,0,sizeof(m_bLookBao));//看宝状态
	memset(m_byFirstHandPai,255,sizeof(m_byFirstHandPai));//第一手牌，出牌，吃碰杠后抓到的不算
	memset(m_byDingQue, 255, sizeof(m_byDingQue));
	memset(m_byGFXY, 0, sizeof(m_byGFXY));
	memset(m_bIsHu, false, sizeof(m_bIsHu));
	memset(m_byGengCount, 0, sizeof(m_byGengCount));

	memset(m_bIsGangPao, 0, sizeof(m_bIsGangPao));//正在杠后炮的状态
	memset(m_bQiHu, 0, sizeof(m_bQiHu));//玩家是否弃糊状态
	memset(m_NoFenGang, 255, sizeof(m_NoFenGang));////记录无分杠（补杠，手中有4张牌先碰后杠无分，抓牌后能补杠但是不在当前圈杠无分）


	m_byBuGangPai = 255;//补杠得分的牌
	//杠分
	for(int i=0;i<PLAY_COUNT;++i)
	{
		m_stGangFen[i].Init();
	}
}

UserDataEx::~UserDataEx(void)
{
	UserData::~UserData();
}


///名称：IsHaveAHandPai
///描述：检查是否存在指定的手牌
///@param pai 要检测的牌
///@return true 有 false 无
bool UserDataEx::IsHaveAHandPai(BYTE station,BYTE pai)
{
	return UserData::IsHaveAHandPai(station,pai);
}

///名称：GetAHandPaiCount
///描述：检查手牌中某张牌的个数
///@param pai 要检测的牌
///@return 牌数量
int UserDataEx::GetAHandPaiCount(BYTE station,BYTE pai)
{
	return UserData::GetAHandPaiCount(station,pai);
}

///得到某种花色牌（0万 1筒 2条)
int UserDataEx::GetAKindPai(BYTE station,BYTE kind)
{
	kind %= 3;

	int count = 0;
	for(int i=0;i<HAND_CARD_NUM;i++)
	{
		if(m_byArHandPai[station][i]/10 == kind)
			return m_byArHandPai[station][i];
	}
	return 0;
}

///检查某种花色牌的个数（0万 1筒 2条)
int UserDataEx::GetAKindPaiCount(BYTE station,BYTE kind)
{
	kind %= 3;

	int count = 0;
	for(int i=0;i<HAND_CARD_NUM;i++)
	{
		if(m_byArHandPai[station][i] == 255)
			continue;
		if(m_byArHandPai[station][i]/10 == kind)
			count++;
	}
	return count;
}

///名称：GetHandPaiCount
///描述：检查手牌个数
///@param 
///@return 玩家手牌的总数
int UserDataEx::GetHandPaiCount(BYTE station)
{
	return UserData::GetHandPaiCount(station);
}

///名称：CopyHandPai
///描述：拷贝玩家的手牌
///@param pai[] 牌数据, station 位置, show 是否拷贝明牌
///@return 玩家手牌总数
int UserDataEx::CopyHandPai(BYTE pai[][HAND_CARD_NUM] ,BYTE station,bool show)
{
	memset(pai,255,sizeof(pai));
	for(int i=0;i<PLAY_COUNT;++i)
	{
		int count = GetHandPaiCount(station);
		if(i == station)//自己的牌
		{
			for(int j=0;j<HAND_CARD_NUM;++j)
			{
				pai[i][j] = m_byArHandPai[station][j];
			}
		}
		else 
		{
			if(m_bIsHu[i] || show)//胡牌了发送明牌
			{
				for(int j=0;j<HAND_CARD_NUM;++j)
				{
					pai[i][j] = m_byArHandPai[i][j];
				}
			}
			else//发送牌背
			{
				for(int j=0;j<HAND_CARD_NUM;++j)
				{
					if( m_byArHandPai[i][j] != 255)
					{	
						pai[i][j] = 0;
					}
					else 
					{
						pai[i][j] = m_byArHandPai[i][j];
					}
				}
			}
		}
	}
	return 0;
}

///名称：CopyOneUserHandPai
///描述：拷贝某个玩家的手牌
///@param pai[] 牌数据, station 位置
///@return 玩家手牌总数
int UserDataEx::CopyOneUserHandPai(BYTE pai[HAND_CARD_NUM] ,BYTE station)
{
	return UserData::CopyOneUserHandPai( pai,station);
}

///名称：SortHandPai
///描述：手牌排序，small是否从小到大排
///@param big 是否从到到小排序。默认从小到大排序
///@return 
void UserDataEx::SortHandPai(BYTE station,bool big)
{
	UserData::SortHandPai( station, big);
}

///名称：AddToHandPai
///描述：添加一张牌到手牌中
///@param pai 添加的手牌值
///@return 
void UserDataEx::AddToHandPai(BYTE station,BYTE pai)
{
	UserData::AddToHandPai( station, pai);
}

///名称：SetHandPaiData
///描述：设置手牌数据
///@param  pai[] 传入的手牌数组
///@return 
void UserDataEx::SetHandPaiData(BYTE station,BYTE pai[],BYTE byCount)
{
	UserData::SetHandPaiData( station, pai,byCount);
}

///名称：DelleteAHandPai
///描述：删除一张指定的手牌
///@param pai 要删除的牌
///@return 
void UserDataEx::DelleteAHandPai(BYTE station,BYTE pai)
{
	UserData::DelleteAHandPai(station,pai);
}

///名称：SetHandPaiBack
///描述：设置手牌牌背
///@param num设置的数量
///@return 
void UserDataEx::SetHandPaiBack(BYTE station,BYTE num)
{
	UserData::SetHandPaiBack(station,num);
}

///名称：GetLastHandPai
///描述：获得最后一张手牌
///@param 
///@return 
BYTE UserDataEx::GetLastHandPai(BYTE station)
{
	return UserData::GetLastHandPai(station);
}

///名称：GetSelectHandPai
///描述：获得选中的一张手牌
///@param 
///@return 
BYTE UserDataEx::GetSelectHandPai(BYTE station)
{
	return UserData::GetSelectHandPai(station);
}

///名称：IsOutPaiPeople
///描述：是否出牌玩家
///@param 
///@return  true 是 ,false 不是
bool UserDataEx::IsOutPaiPeople(BYTE station)
{
	return UserData::IsOutPaiPeople(station);
}

///名称：IsHuaPaiInHand
///描述：检测手牌中是否存在花牌
///@param 
///@return  true 是 ,false 不是
bool UserDataEx::IsHuaPaiInHand(BYTE station)
{
	return UserData::IsHuaPaiInHand(station);
}

///名称：MoveHuaPaiFormHand
///描述：将手牌中的花牌移到花牌数组中
///@param 
///@return  花牌的数量
int UserDataEx::MoveHuaPaiFormHand(BYTE station)
{
	return UserData::MoveHuaPaiFormHand(station);
}

///获取手牌的牌色数
int UserDataEx::GetPaiSeCount(BYTE station)
{
	int count = 0;
	bool se[3];
	memset(se,0,sizeof(se));
	for(int i=0;i<HAND_CARD_NUM;i++)
	{
		if(m_byArHandPai[station][i] < 0 || m_byArHandPai[station][i] > CMjEnum::MJ_TYPE_B9)
			continue;
		se[m_byArHandPai[station][i]/10] = true;
	}
	for(int i=0;i<3;i++)
	{
		if(se[i])
			count++;
	}
	return count;
}
///是否还有缺门的牌
bool UserDataEx::IsHaveQueMen(BYTE station,BYTE type)
{
	if(type >2 || type<0)
		return false;
	for(int i=0;i<HAND_CARD_NUM;i++)
	{
		if(m_byArHandPai[station][i] < 0 || m_byArHandPai[station][i] > CMjEnum::MJ_TYPE_B9)
			continue;
		if(m_byArHandPai[station][i]/10 == type)
			return true;
	}
	return false;
}
//获取一张data中没有的牌，用来换牌
BYTE UserDataEx::GetChangePai(BYTE station,BYTE data[])
{
	BYTE pai = 255;
	for(int i=0;i<HAND_CARD_NUM;i++)
	{
		bool have = false;
		for(int j=0;j<HAND_CARD_NUM;++j)
		{
			if(data[j] == m_byArHandPai[station][i])
				have = true;//换牌数组中存在该牌，不能拿去替换
		}
		if(!have)
		{
			return m_byArHandPai[station][i];
		}
	}
	return pai;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///名称：GetUserGangNum
///描述：获取玩家杠牌的个数(暗杠，明杠，补杠)
///@param &an 暗杠数量, &ming 明杠数量 ,  &bu 补杠数量
///@return 杠牌的总个数
int UserDataEx::GetUserGangNum(BYTE station,BYTE &an,BYTE &ming , BYTE &bu)
{
	return UserData::GetUserGangNum(station,an,ming ,bu);
}

///名称：GetUserChiNum
///描述：玩家吃牌次数
///@param station 玩家位置
///@return 吃牌的总个数
int UserDataEx::GetUserChiNum(BYTE station)
{
	return UserData::GetUserChiNum(station);
}

///名称：GetUserPengNum
///描述：玩家杠牌次数
///@param station 玩家位置
///@return 碰牌的总个数
int UserDataEx::GetUserPengNum(BYTE station)
{
	return UserData::GetUserPengNum(station);
}
///名称：IsUserHaveGangPai
///描述：玩家杠了某个牌
///@param pai 牌, &type 类型
///@return  true 是 ,false 不是
bool UserDataEx::IsUserHaveGangPai(BYTE station,BYTE pai,BYTE &type)
{	
	return UserData::IsUserHaveGangPai(station, pai, type);
}
///
///名称：IsUserHavePengPai
///描述：玩家是否碰了某张牌
///@param pai 牌
///@return  true 是 ,false 不是
bool UserDataEx::IsUserHavePengPai(BYTE station,BYTE pai)
{
	return UserData::IsUserHavePengPai(station, pai);
}

///名称：IsUserHaveChiPai
///描述：玩家是否吃过某张牌
///@param pai 牌
///@return  true 是 ,false 不是
bool UserDataEx::IsUserHaveChiPai(BYTE station,BYTE pai)
{
	return UserData::IsUserHaveChiPai(station,pai);
}

///名称：AddToGCP
///描述：添加一组数据到杠吃碰数组中
///@param gcpData 要添加的吃碰杠数据
///@return 
void UserDataEx::AddToGCP(BYTE station,GCPStructEx *gcpData)
{
	//UserData::AddToGCP(station,gcpData);
	for(int i=0;i<5;i++)
	{
		if(m_UserGCPData[station][i].byType==255)
		{
			m_UserGCPData[station][i].byType = gcpData->byType;
			m_UserGCPData[station][i].iBeStation = gcpData->iBeStation;
			m_UserGCPData[station][i].iOutpai = gcpData->iOutpai;
			m_UserGCPData[station][i].iStation = gcpData->iStation;
			m_UserGCPData[station][i].bIsBigGang = gcpData->bIsBigGang;
			for(int j=0;j<4;j++)
			{
				m_UserGCPData[station][i].byData[j] = gcpData->byData[j];
			}
			for(int j=0;j<4;j++)
			{
				m_UserGCPData[station][i].byBigGang[j][0] = gcpData->byBigGang[j][0];
				m_UserGCPData[station][i].byBigGang[j][1] = gcpData->byBigGang[j][1];
			}
			break;
		}
	}
}

///名称：DelleteAGCPData
///描述：删除杠吃碰数组中指定的数据组
///@param type 类型, pai 牌
///@return 
void UserDataEx::DelleteAGCPData(BYTE station,BYTE type ,BYTE pai)
{
	UserData::DelleteAGCPData(station, type , pai);
}	

///名称：CopyGCPData
///描述：拷贝杠吃碰数组中的数据组
///@param gcpData 吃碰杠数组
///@return 吃碰杠个数
int UserDataEx::CopyGCPData(BYTE station,GCPStructEx gcpData[])
{
	return UserData::CopyGCPData(station,gcpData);
}

///名称：SetGCPData
///描述：设置杠吃碰数组中的数据组
///@param gcpData 吃碰杠数组
///@return 
void UserDataEx::SetGCPData(BYTE station,GCPStructEx gcpData[])
{
	UserData::SetGCPData(station,gcpData);
}

///名称：GetGCPCount
///描述：获取杠吃碰的节点个数
///@param 
///@return 获得吃碰杠总个数
int UserDataEx::GetGCPCount(BYTE station)
{
	return UserData::GetGCPCount(station);
}
///名称：GetOnePaiGCPCount
///描述：获取杠吃碰的中某种牌的数量
///@param 
///@return 某种牌的数量
int UserDataEx::GetOnePaiGCPCount(BYTE station,BYTE pai)
{
	return UserData::GetOnePaiGCPCount(station,pai);
}
///名称：ShortGCP
///描述：排序杠吃碰数组
///@param 
///@return 
void UserDataEx::ShortGCP(BYTE station)
{
	UserData::ShortGCP(station);
}
///名称：InitGCP
///描述：初始化杠吃碰数组
///@param 
///@return 
void UserDataEx::InitGCP(BYTE station)
{
	UserData::InitGCP(station);
}
//////////////////出牌操作///////////////////////////////////////////////////////////////////////////////

///名称：AddToOutPai
///描述：添加一张牌到出牌数组中
///@param pai 牌
///@return 
void UserDataEx::AddToOutPai(BYTE station,BYTE pai)
{
	UserData::AddToOutPai(station, pai);
}

///名称：DelleteLastOutPai
///描述：删除最后一张出牌
///@param 
///@return 
void UserDataEx::DelleteLastOutPai(BYTE station)
{
	UserData::DelleteLastOutPai(station);
}

///名称：SetOutPaiData
///描述：设置出牌数据
///@param pai[] 出牌数组
///@return 
void UserDataEx::SetOutPaiData(BYTE station,BYTE pai[],BYTE byCount)
{
	UserData::SetOutPaiData(station, pai, byCount);
}

///名称：CopyOutPai
///描述：拷贝玩家的出牌
///@param pai[] 出牌数组
///@return 出牌的张数
int UserDataEx::CopyOutPai(BYTE station,BYTE pai[])
{
	return UserData::CopyOutPai(station, pai);
}

///名称：GetOutPaiCount
///描述：获取玩家的出牌数量
///@param 
///@return 出牌的总数
int UserDataEx::GetOutPaiCount(BYTE station)
{
	return UserData::GetOutPaiCount(station);
}
///名称：GetOutPaiCount
///描述：获取玩家某张牌的出牌数量
///@param 
///@return 某张牌出牌的数量
int UserDataEx::GetOneOutPaiCount(BYTE station,BYTE pai)
{
	return UserData::GetOneOutPaiCount(station, pai);
}
/////////////////////花牌操作////////////////////////////////////////////////////////

///名称：AddToHuaPai
///描述：添加一张花牌到花牌数组中
///@param pai 花牌
///@return 
void UserDataEx::AddToHuaPai(BYTE station,BYTE pai)
{
	UserData::AddToHuaPai(station, pai);
}

///名称：SetHuaPaiData
///描述：设置花牌数据
///@param pai[] 花牌数组
///@return 
void UserDataEx:: SetHuaPaiData(BYTE station,BYTE pai[])
{
	UserData::SetHuaPaiData( station, pai);
}

///名称：CopyHuaPai
///描述：拷贝玩家的花牌
///@param pai[] 花牌数组
///@return 
int UserDataEx::CopyHuaPai(BYTE station,BYTE pai[])
{
	return UserData::CopyHuaPai(station, pai);
}

///检测是否花牌
bool UserDataEx::CheckIsHuaPai(BYTE pai)
{
	return UserData::CheckIsHuaPai(pai);
}
///////////////////////门牌操作//////////////////////////////////////////////////////////////////

///名称：SetMenPaiData
///描述：设置门牌数据
///@param  pai[] 门牌数组
///@return 
void UserDataEx::SetMenPaiData(BYTE station,BYTE pai[])
{
	UserData::SetMenPaiData(station, pai);
}

///名称：CopyMenPai
///描述：拷贝玩家的门牌
///@param  pai[] 门牌数组
///@return 玩家门牌总数
int UserDataEx::CopyMenPai(BYTE station,BYTE pai[])
{
	return UserData::CopyMenPai(station,pai);
}

///名称：GetPaiNum
///描述：获取牌数组的有效牌张数
///@param  pai[] 要检测的牌数组, count 有效牌的张数
///@return 
int UserDataEx::GetPaiNum(BYTE station,BYTE pai[],BYTE count)
{
	return UserData::GetPaiNum(station, pai,count);
}
///名称：InitData
///描述：初始化数据
///@param 
///@return 
void UserDataEx::InitData()
{
	UserData::InitData();

	m_byCaiPiaoStation = 255;//财飘位置，该值不为255是不允许吃碰杠和放炮
	memset(m_byGuanXi,0,sizeof(m_byGuanXi));//吃碰杠关系
	memset(m_bGangKai,0,sizeof(m_bGangKai));//杠开状态玩家
	memset(m_bOutJing,0,sizeof(m_bOutJing));//玩家是否打出了财神牌
	m_bTianHu = true;;//庄家天糊状态
	m_bChanPai = false;		//玩家是否铲牌

	memset(m_bLookBao,0,sizeof(m_bLookBao));//看宝状态

	memset(m_byFirstHandPai,255,sizeof(m_byFirstHandPai));//第一手牌，出牌，吃碰杠后抓到的不算
	memset(m_byDingQue, 255, sizeof(m_byDingQue));
	memset(m_byGFXY, 0, sizeof(m_byGFXY));
	memset(m_bIsHu, false, sizeof(m_bIsHu));
	memset(m_byGengCount, 0, sizeof(m_byGengCount));

	memset(m_bIsGangPao, 0, sizeof(m_bIsGangPao));//正在杠后炮的状态
	memset(m_bQiHu, 0, sizeof(m_bQiHu));//玩家是否弃糊状态
	memset(m_NoFenGang, 255, sizeof(m_NoFenGang));////记录无分杠（补杠，手中有4张牌先碰后杠无分，抓牌后能补杠但是不在当前圈杠无分）
	m_byBuGangPai = 255;//补杠得分的牌
	//杠分
	for(int i=0;i<PLAY_COUNT;++i)
	{
		m_stGangFen[i].Init();
	}

}

///名称：IsUserHaveBigGangPai
///描述：玩家是否存在某种牌的大杠
///@param pai 牌
///@return  true 是 ,false 不是
bool UserDataEx::IsUserHaveBigGangPai(BYTE station,BYTE pai)
{
	if(pai == 255)
		return false;
	for(int i=0;i<5;i++)
	{
		if(!m_UserGCPData[station][i].bIsBigGang || m_UserGCPData[station][i].byType<ACTION_AN_GANG || m_UserGCPData[station][i].byType>ACTION_MING_GANG )
			continue;
		for(int j=0;j<4;j++)
		{
			if(m_UserGCPData[station][i].byBigGang[j][0] == pai)
				return true;
		}
	}
	return false;
}

///描述：将抓牌索引转换成抓牌玩家的门牌索引
bool UserDataEx::ZhuaPaiIndex(BYTE &index,BYTE &dir)
{
	dir = 255;
	if(index == 255)
		return false;
	//int num = 0;
	//int count = 0;
	//num = 27;
	//for(int i=0;i<4;i++)
	//{
	//	if(i == 0 || i == 2)
	//	{
	//		num += 1;
	//	}
	//	else 
	//	{
	//		num -= 1;
	//	}
	//	for(int j=count;j<(count + num) ;++j)
	//	{
	//		if(j == index)
	//		{
	//			dir = i;
	//			index = j-count;
	//			return true;
	//		}
	//	}		
	//	count += num;
	//}

	if(index<28)
	{
		dir = 0;
		index = index;
	}
	else if(index<54)
	{
		dir = 1;
		index = index%28;
	}
	else if(index<82)
	{
		dir = 2;
		index = index%54;
	}
	else if(index<108)
	{
		dir = 3;
		index = index%82;
	}
	return true;
}
/************************************************************************************/
GameData::GameData(void)
{

	m_iHuangZhuangCount = 0;

	///房间的倍率
	basepoint = 0;	
	///要申请的事件id
	ApplyThingID = 255;
	//是否显示台费
	m_bShowTax = 0;

	memset(m_byThingNext,255,sizeof(m_byThingNext));

	InitData();
	LoadIni();
}

GameData::~GameData(void)
{

}


///设置所有事件的后接事件
void GameData::SetThingNext()
{
	//游戏开始事件
	/*	T_Begin.byNext = m_byThingNext[THING_BEGIN];*/
	//首局以东为庄事件	
	//	T_DongNt.byNext = m_byThingNext[THING_DONG_NT];
	//首局掷2颗色子的点数和为庄事件	
	T_TwoSeziNt.byNext = m_byThingNext[THING_2SEZI_NT];
	//首局掷2颗色子定庄家和起牌点事件	
	T_TwoSeziNtAndGetPai.byNext = m_byThingNext[THING_2SEZI_NT_GP];
	//首局轮流掷2颗色子定庄家事件	
	T_TurnSeziNt.byNext = m_byThingNext[THING_TURN_2SEZI_NT];
	//掷2颗色子事件定起牌方向事件
	//	T_TwoSeziDir.byNext = m_byThingNext[THING_2SEZI_DIR];
	//掷2颗色子事件定起牌方向和起牌点事件
	//	T_TwoSeziDirAndGetPai.byNext = m_byThingNext[THING_2SEZI_DIR_GP];
	//掷2颗色子事件定起牌点事件	
	T_TwoSeziGetPai.byNext = m_byThingNext[THING_2SEZI_GP];
	//出牌前事件
	//发牌事件	
	//	T_SendPai.byNext = m_byThingNext[THING_SEND_PAI];
	//跳牌事件	
	T_TiaoPai.byNext = m_byThingNext[THING_TIAO_PAI];
	//掷2颗色子定精牌事件
	T_TwoSeziJing.byNext = m_byThingNext[THING_2SEZI_JING];
	//掷1颗色子定金牌事件
	T_OneSeziJin.byNext = m_byThingNext[THING_1SEZI_JIN];
	//为所有玩家补花事件
	T_AllBuHua.byNext = m_byThingNext[THING_ALL_BUHUA];
	//单个玩家补花事件	
	T_OneBuHua.byNext = m_byThingNext[THING_ONE_BUHUA];
	//正常动作事件
	//庄家开始发牌通知事件
	//	T_BeginOutPai.byNext = m_byThingNext[THING_BEGIN_OUT];
	//出牌事件
	//	T_OutPai.byNext = m_byThingNext[THING_OUT_PAI];
	//抓牌事件
	T_ZhuaPai.byNext = m_byThingNext[THING_ZHUA_PAI];
	//吃牌事件
	T_ChiPai.byNext = m_byThingNext[THING_CHI_PAI];
	//碰牌事件
	T_PengPai.byNext = m_byThingNext[THING_PENG_PAI];

	//杠牌事件
	T_GangPai.byNext = m_byThingNext[THING_GANG_PAI];
	//听牌事件
	T_TingPai.byNext = m_byThingNext[THING_TING_PAI];
	//糊牌及糊牌后事件
	//糊事件
	//	T_HuPai.byNext = m_byThingNext[THING_HU_PAI];
	//算分事件	
	//	T_CountFen.byNext = m_byThingNext[THING_COUNT_FEN];
	//结束处理事件
	T_EndHandle.byNext = m_byThingNext[THING_ENG_HANDLE];
	//非正常结束处理事件
	T_UnnormalEndHandle.byNext = m_byThingNext[THING_ENG_UN_NORMAL];
}

///设置某个事件的后接事件
void GameData::SetThingNext(BYTE id)
{
	//	switch(id)
	//	{
	////发牌前事件
	//	case THING_BEGIN:			//游戏开始事件
	//		T_Begin.byNext = m_byThingNext[id];
	//		break;
	//	case THING_DONG_NT:			//首局以东为庄事件	
	//		T_DongNt.byNext = m_byThingNext[id];
	//		break;
	//	case THING_2SEZI_NT:		//首局掷2颗色子的点数和为庄事件	
	//		T_TwoSeziNt.byNext = m_byThingNext[id];
	//		break;
	//	case THING_2SEZI_NT_GP:		//首局掷2颗色子定庄家和起牌点事件	
	//		T_TwoSeziNtAndGetPai.byNext = m_byThingNext[id];
	//		break;
	//	case THING_TURN_2SEZI_NT:	//首局轮流掷2颗色子定庄家事件	
	//		T_TurnSeziNt.byNext = m_byThingNext[id];
	//		break;
	//	case THING_2SEZI_DIR:		//掷2颗色子事件定起牌方向事件
	//		T_TwoSeziDir.byNext = m_byThingNext[id];
	//		break;
	//	case THING_2SEZI_DIR_GP:	//掷2颗色子事件定起牌方向和起牌点事件
	//		T_TwoSeziDirAndGetPai.byNext = m_byThingNext[id];
	//		break;
	//	case THING_2SEZI_GP:		//掷2颗色子事件定起牌点事件	
	//		T_TwoSeziGetPai.byNext = m_byThingNext[id];
	//		break;
	////出牌前事件
	//	case THING_SEND_PAI:		//发牌事件	
	//		T_SendPai.byNext = m_byThingNext[id];
	//		break;
	//	case THING_TIAO_PAI:		//跳牌事件	
	//		T_TiaoPai.byNext = m_byThingNext[id];
	//		break;
	//	case THING_2SEZI_JING:		//掷2颗色子定精牌事件
	//		T_TwoSeziJing.byNext = m_byThingNext[id];
	//		break;
	//	case THING_1SEZI_JIN:		//掷1颗色子定金牌事件
	//		T_OneSeziJin.byNext = m_byThingNext[id];
	//		break;
	//	case THING_ALL_BUHUA:		//为所有玩家补花事件
	//		T_AllBuHua.byNext = m_byThingNext[id];
	//		break;
	//	case THING_ONE_BUHUA:		//单个玩家补花事件	
	//		T_OneBuHua.byNext = m_byThingNext[id];
	//		break;
	////正常动作事件
	//	case THING_BEGIN_OUT:		//庄家开始发牌通知事件
	//		T_BeginOutPai.byNext = m_byThingNext[id];
	//		break;
	//	case THING_OUT_PAI:			//出牌事件
	//		T_OutPai.byNext =		m_byThingNext[id];
	//		break;
	//	case THING_ZHUA_PAI:		//抓牌事件
	//		T_ZhuaPai.byNext = m_byThingNext[id];
	//		break;
	//	case THING_CPG_NOTIFY:		//杠吃碰通知事件事件
	//		break;
	//	case THING_CHI_PAI:			//吃牌事件
	//		T_ChiPai.byNext = m_byThingNext[id];
	//		break;
	//	case THING_PENG_PAI:		//碰牌事件
	//		T_PengPai.byNext = m_byThingNext[id];
	//		break;

	//		break;
	//	case THING_SAO_HU:			//扫虎事件
	//		T_SaoHu.byNext = m_byThingNext[id];
	//		break;
	//	case THING_GANG_PAI:		//杠牌事件
	//		T_GangPai.byNext = m_byThingNext[id];
	//		break;
	//	case THING_TING_PAI:		//听牌事件
	//		T_TingPai.byNext = m_byThingNext[id];
	//		break;
	////其他动作事件
	////糊牌及糊牌后事件
	//	case THING_HU_PAI:			//糊事件
	//		T_HuPai.byNext = m_byThingNext[id];
	//		break;
	//	case THING_COUNT_FEN:		//算分事件	
	//		T_CountFen.byNext = m_byThingNext[id];
	//		break;
	//	case THING_ENG_HANDLE:		//结束处理事件
	//		T_EndHandle.byNext = m_byThingNext[id];
	//		break;
	//	case THING_ENG_UN_NORMAL:	//非正常结束处理事件
	//		T_UnnormalEndHandle.byNext = m_byThingNext[id];
	//		break;
	//	}

}

///初始化数据
void GameData::InitData()
{

	memset(m_byThingRecord,255,sizeof(m_byThingRecord));
	memset(m_bGangState,0,sizeof(m_bGangState));//记录杠牌状态，拥于杠开和杠后炮

	m_byNtStation	= 255;	//庄家位置

	ApplyThingID	= 255;	///要申请的事件	DWJ

	m_byThingDoing	= 255;	///正在发生的事件号

	m_byWatingThing	= 255;	//正在等待响应的事件

	m_iRemaindTime = 0;



	///待执行事件
	m_byNextAction = ACTION_NO;



	///吃牌牌事件,临时数据
	temp_ChiPai.Clear();
	///碰牌牌事件,临时数据
	temp_PengPai.Clear();
	///杠牌牌事件,临时数据
	temp_GangPai.Clear();

	///游戏开始事件
	T_Begin.Clear();

	///以东为庄事件
	T_DongNt.Clear();

	///掷2颗色子的点数和为庄事件
	T_TwoSeziNt.Clear();

	///掷2颗色子确定庄家和起牌点位置事件
	T_TwoSeziNtAndGetPai.Clear();

	///轮流掷2颗色子确定庄家
	T_TurnSeziNt.Clear();

	///掷2颗色子确定起牌位置事件
	T_TwoSeziDir.Clear();

	///掷2颗色子确定起牌位置（点数和）和起牌点（最小点）事件
	T_TwoSeziDirAndGetPai.Clear();

	///掷2颗色子确定起牌敦数事件
	T_TwoSeziGetPai.Clear();

	///发牌事件
	T_SendPai.Clear();

	///跳牌事件
	T_TiaoPai.Clear();

	///掷色子2颗色子定精牌事件
	T_TwoSeziJing.Clear();

	///掷色子1颗色子定金牌事件
	T_OneSeziJin.Clear();

	///所有玩家补花事件
	T_AllBuHua.Clear();

	///单个玩家补花事件
	T_OneBuHua.Clear();

	///开始出牌通知事件
	T_BeginOutPai.Clear();

	///出牌事件
	T_OutPai.Clear();

	///抓牌牌事件
	T_ZhuaPai.Clear();

	/////吃碰杠糊牌通知事件牌事件
	T_CPGNotify[PLAY_COUNT].Clear();

	///吃牌牌事件
	T_ChiPai.Clear();

	///碰牌牌事件
	T_PengPai.Clear();




	///杠牌牌事件
	T_GangPai.Clear();

	///听牌牌事件
	T_TingPai.Clear();

	///糊牌牌事件
	T_HuPai.Clear();

	///算分事件
	T_CountFen.Clear();

	///游戏结束处理事件
	T_EndHandle.Clear();

	///非正常结束处理事件
	T_UnnormalEndHandle.Clear();

	SetThingNext();

}
/*--------------------------------------------------------------------------*/
///在人配置文件
void GameData::LoadIni()
{

	m_mjRule.byAllMjCount   = 112;			//麻将的数量 


	m_mjRule.bHaveWan		= 1;			//是否有万
	m_mjRule.bHaveTiao		= 1;			//是否有条
	m_mjRule.bHaveBing		= 1;			//是是有柄
	m_mjRule.bHaveHongZhong	= 1;			//是否有红中
	m_mjRule.bHaveFaCai		= 0;			//是否有发财
	m_mjRule.bHaveBaiBan	= 0;			//是否有白板
	m_mjRule.bHaveFengPai	= 0;			//是否有东南西北
	m_mjRule.bHaveFlower	= 0;			//是否有花牌

	m_mjRule.byHandCardNum	= 13;			//手牌张数
	m_mjRule.byGamePassNum	= 0;			//流局牌数
	m_mjRule.byGamePassType	= 0;			//流局留牌类型
	m_mjRule.byNextNtType	= 2;			//下局庄家的确定类型
	m_mjRule.bHaveJing		= 1;			//有无财神

	m_mjRule.byMenPaiNum[0]	= 28;			//各家门牌数
	m_mjRule.byMenPaiNum[1]	= 28;			//各家门牌数
	m_mjRule.byMenPaiNum[2]	= 28;			//各家门牌数
	m_mjRule.byMenPaiNum[3]	= 28;			//各家门牌数

	m_mjAction.bChi				= 0;		//吃
	m_mjAction.bChiFeng			= 0;		//吃风牌
	m_mjAction.bChiJian			= 0;		//吃中发白
	m_mjAction.bPeng			= 1;		//碰
	m_mjAction.bKan				= 0;		//坎
	m_mjAction.bSaoHu			= 0;		//扫虎
	m_mjAction.bGang			= 1;		//杠
	m_mjAction.bBuGang			= 1;		//补杠
	m_mjAction.bTing			= 0;		//听
	m_mjAction.bQiangChi		= 0;		//抢吃(吃后听牌)


#pragma warning(push)
#pragma warning(disable:4800)
	CString s = CINIFile::GetAppPath ();/////本地路径
	CINIFile pz(s + SKIN_FOLDER  + _T("_s.ini"));
	CString key = TEXT("game");// 麻将常规属性

	m_mjRule.byAutoOutTime	= pz.GetKeyVal(key,TEXT("byAutoOutTime"),1);		//自动出牌时间	
	m_mjRule.byOutTime		= pz.GetKeyVal(key,TEXT("byOutTime"),15);			//出牌时间		
	m_mjRule.byBlockTime	= pz.GetKeyVal(key,TEXT("byBlockTime"),15);			//拦牌思考时间	
	m_mjRule.byBeginTime	= pz.GetKeyVal(key,TEXT("byBeginTime"),15);			//开始等待时间	
	m_mjRule.bAutoBegin		= pz.GetKeyVal(key,TEXT("bAutoBegin"),0);			//时间到了是否自动开始

	m_mjRule.bNetCutTuoGuan	= pz.GetKeyVal(key,TEXT("bNetCutTuoGuan"),1);		//是否断线托管
	m_mjRule.bForceTuoGuan	= pz.GetKeyVal(key,TEXT("bForceTuoGuan"),1);		//是否强退托管
	m_mjRule.byGamePassNum	= pz.GetKeyVal(key,TEXT("GamePassNum"),0);;			//流局牌数

#pragma warning(pop)
}

/*********************************************************************************/
GameDataEx::GameDataEx(void)
{
	GameData::GameData();

	LoadIni();

}

GameDataEx::~GameDataEx(void)
{
	GameData::~GameData();
}


///设置所有事件的后接事件
void GameDataEx::SetThingNext()
{
	GameData::SetThingNext();

}

///设置某个事件的后接事件
void GameDataEx::SetThingNext(BYTE id)
{
	GameData::SetThingNext(id);
}

//virtual 初始化数据
void GameDataEx::InitData()
{
	memset(m_byThingRecord,255,sizeof(m_byThingRecord));
	///待执行事件
	m_byNextAction = ACTION_NO;

	///正在发生的事件号
	m_byThingDoing = 255;  

	///吃牌牌事件,临时数据
	temp_ChiPai.Clear();
	///碰牌牌事件,临时数据
	temp_PengPai.Clear();
	///杠牌牌事件,临时数据
	temp_GangPai.Clear();

	///游戏开始事件
	T_Begin.Clear();

	///以东为庄事件
	T_DongNt.Clear();

	///掷2颗色子的点数和为庄事件
	T_TwoSeziNt.Clear();

	///掷2颗色子确定庄家和起牌点位置事件
	T_TwoSeziNtAndGetPai.Clear();

	///轮流掷2颗色子确定庄家
	T_TurnSeziNt.Clear();

	///掷2颗色子确定起牌位置事件
	T_TwoSeziDir.Clear();

	///掷2颗色子确定起牌位置（点数和）和起牌点（最小点）事件
	T_TwoSeziDirAndGetPai.Clear();

	///掷2颗色子确定起牌敦数事件
	T_TwoSeziGetPai.Clear();

	///发牌事件
	T_SendPai.Clear();

	///跳牌事件
	T_TiaoPai.Clear();

	///掷色子2颗色子定精牌事件
	T_TwoSeziJing.Clear();

	///掷色子1颗色子定金牌事件
	T_OneSeziJin.Clear();

	///所有玩家补花事件
	T_AllBuHua.Clear();

	///单个玩家补花事件
	T_OneBuHua.Clear();

	///开始出牌通知事件
	T_BeginOutPai.Clear();

	///出牌事件
	T_OutPai.Clear();

	///抓牌牌事件
	T_ZhuaPai.Clear();

	/////吃碰杠糊牌通知事件牌事件
	//T_CPGNotify[4];

	///吃牌牌事件
	T_ChiPai.Clear();

	///碰牌牌事件
	T_PengPai.Clear();





	///杠牌牌事件
	T_GangPai.Clear();

	///听牌牌事件
	T_TingPai.Clear();

	///糊牌牌事件
	T_HuPai.Clear();

	///算分事件
	T_CountFen.Clear();

	///游戏结束处理事件
	T_EndHandle.Clear();

	///非正常结束处理事件
	T_UnnormalEndHandle.Clear();

	m_ZhongNiao.clear();
}

///初始化数据
void GameDataEx::LoadIni()
{
	GameData::LoadIni();
}

