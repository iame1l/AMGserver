#include "StdAfx.h"
#include "ClientGamedlg.h"

BEGIN_MESSAGE_MAP(CClientGameDlg, CLoveSendClass)
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
END_MESSAGE_MAP()

const int GAME_RAND_TIME = 5;
//构造函数
CClientGameDlg::CClientGameDlg() : CLoveSendClass(IDD_GAME_FRAME)
{
	InitGameData();
} 

//析构函数
CClientGameDlg::~CClientGameDlg()
{

}

//---------------------------------------------------------------------------------------
///初始化部分数据
void	CClientGameDlg::ResetGameData()
{
	KillAllTimer();
}
//初始化游戏数据
void	CClientGameDlg::InitGameData()
{
	LoadGameIni();
}
//加载配置文件
void CClientGameDlg::LoadGameIni()
{
	CString nid;
	nid.Format("%d",NAME_ID);
	CString s = CINIFile::GetAppPath ();///本地路径
	CINIFile f( s +nid +"RobotSet.ini");
	CString key = TEXT("robot");

}
//初始化函数
BOOL CClientGameDlg::OnInitDialog()
{
	CLoveSendClass::OnInitDialog();
	return TRUE;
}
//------------------------------------------------------------------------------------------------------------------
//设置游戏状态
bool CClientGameDlg::SetGameStation(void * pStationData, UINT uDataSize)
{
	GameStation_base *pGameStation = (GameStation_base *) pStationData;
	//设置游戏状态
	SetStationParameter(GS_WAIT_SETGAME);
	m_iMyDeskStation = GetMeUserInfo()->bDeskStation;
	switch(GS_WAIT_SETGAME)
	{
	case GS_WAIT_SETGAME:	//游戏没有开始状态
	case GS_WAIT_ARGEE:		//等待玩家开始状态
	case GS_WAIT_NEXT:		//等待下一盘游戏开始
		{
			//保存基础数据
			if(!(m_pGameInfo->dwRoomRule &GRR_CONTEST))
			{
				SetGameTimer(m_iMyDeskStation,(rand() % GAME_RAND_TIME)+1,TIME_BEGIN);
			}
			break;
		}
	case GS_PLAY_GAME:	//游戏中状态
		{
			break;
		}
	default:
		break;
	}

	return true;
}
//------------------------------------------------------------------------------------------------------------------
//游戏消息处理函数
bool CClientGameDlg::HandleGameMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	if (pNetHead->bMainID == MDM_GM_GAME_NOTIFY)
	{
		switch (pNetHead->bAssistantID)
		{
		case ASS_GM_GAME_STATION:						///<设置状态(平台调用的)
			{
				SetGameStation(pNetData, uDataSize);
				break;       
			}
		case ASS_GAME_BEGIN:                           //  游戏开始
			{
				break;
			}
		case ASS_SEND_ALL_CARD:
			{
			    break;
			}
		case ASS_NEW_TURN:
			{
			    break;
			}
		case ASS_OUT_CARD:
			{
			    break;
			}
		case ASS_OUT_CARD_RESULT:
			{
				break;
			}
		case ASS_CONTINUE_END:
			{
				//保存基础数据
				if(!(m_pGameInfo->dwRoomRule &GRR_CONTEST))
				{
					SetGameTimer(m_iMyDeskStation,(rand() % GAME_RAND_TIME)+1,TIME_BEGIN);
				}
			    break;
			}
		default:break;
		}
		
	}
	return __super::HandleGameMessage(pNetHead,pNetData,uDataSize,pClientSocket);
}
/*-----------------------------------------------------------------------------*/



//重新设置界面
void CClientGameDlg::ResetGameFrame()
{
	return __super::ResetGameFrame();
}


bool CClientGameDlg::OnControlHitBegin()
{
	return true;
}


//计时器
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

///原先的秒为单位不足以持续下注的速度，故重写改为秒为单位
bool CClientGameDlg::SetGameTimer(BYTE bDeskStation,UINT uTimeCount,UINT uTimeID,bool bUserMilliSecond)
{
	int iTimeType = 1000 ; 

	if(m_pGameRoom != NULL)
	{
		m_pGameRoom->SendMessage(IDM_SET_GAME_TIME,(WPARAM)uTimeID,(LPARAM)uTimeCount*iTimeType);
	}
	return true;
}

//清除所有定时器
void	CClientGameDlg::KillAllTimer()
{
	KillGameTimer(TIME_BEGIN);
	KillGameTimer(TIME_NOTIC_OUTPAI);

}
//------------------------------------------------------------------------------------------------------------------
//定时器消息
bool CClientGameDlg::OnGameTimer(BYTE bDeskStation, UINT uTimeID, UINT uTimeCount)
{
	switch (uTimeID)
	{
	case TIME_BEGIN:
		{	
			KillGameTimer(TIME_BEGIN);
			if((m_pGameInfo->dwRoomRule &GRR_CONTEST) || m_pGameInfo->dwRoomRule &GRR_QUEUE_GAME)
			{
				return true;
			}
			SendGameData(MDM_GM_GAME_NOTIFY,ASS_GM_AGREE_GAME,0);
			break;
		}
	case TIME_NOTIC_OUTPAI:
		{
			KillGameTimer(TIME_NOTIC_OUTPAI);
			
			break;
		}
	default:break;
	}
	return true;
}