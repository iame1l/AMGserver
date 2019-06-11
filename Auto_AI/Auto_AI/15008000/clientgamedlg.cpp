#include "StdAfx.h"
#include "ClientGameDlg.h"
#include <random>

using namespace std;

int rand_Mersenne(const int _Min,const int _Max)
{
	if (_Min > _Max){ return 0;}
	if (_MSC_VER < 1500 ){ return 0;}
	random_device rd;
	mt19937 mt(rd());
	uniform_int_distribution<> dis(_Min,_Max);
	return dis(mt);
}

//CEvent g_Event,g_EventPlay;

// 消息映射
BEGIN_MESSAGE_MAP(CClientGameDlg,/*CLoveSendClassInExe*/ CLoveSendClass)
	ON_WM_TIMER()
	//ON_WM_TIMER()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



CClientGameDlg::CClientGameDlg(void):CLoveSendClass(IDD_GAME_FRAME) //CLoveSendClass(&m_PlayView)
{

	//加载配置文件
	LoadIniConfig();
}

CClientGameDlg::~CClientGameDlg(void)
{
}

bool CClientGameDlg::OnNotifyUpdateClientView(void * pNetData, UINT uDataSize)
{
	return true;
}
// 玩家强退结束游戏
bool CClientGameDlg::OnNotifyCutEnd(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize)
{
	return TRUE;
}

void CClientGameDlg::OnChangeTrustee(BYTE byDeskStation, bool bIsTrustee)
{
}
//
BOOL CClientGameDlg::OnInitDialog()
{

	// 游戏房间类型
	CLoveSendClass::OnInitDialog();
	srand((unsigned)GetTickCount());
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CClientGameDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	OutputDebugString("");
	switch(nIDEvent)
	{

	default:
		break;
	}
	//CLoveSendClass::OnTimer((UINT)nIDEvent);
	CLoveSendClass::OnTimer(nIDEvent);
}


//================================================================================================================
//设置游戏状态
//================================================================================================================
bool CClientGameDlg::SetGameStation(void * pStationData, UINT uDataSize)
{
	switch (GetStationParameter())
	{         
	case GS_WAIT_SETGAME:		//游戏没有开始状态
	case GS_WAIT_NEXT:
		{
			if (sizeof(GameStation) != uDataSize)
			{
				return true;
			}
			GameStation * pGameStation=(GameStation *)pStationData;
			if (NULL == pGameStation)
			{
				return true;
			}

			SetStationParameter(GS_WAIT_NEXT);
			m_iXiaZhuTime	= pGameStation->iXiaZhuTime;		/// 下注时间			
			m_iKaiPaiTime	= pGameStation->iKaiPaiTime;     	/// 开牌时间    
			m_iFreeTime		= pGameStation->iFreeTime;			/// 空闲时间	
			m_i64UserMoney = pGameStation->i64MyMoney;
			break;
		}
	case GS_NOTE_STATE:			//下注状态
		{
			if (sizeof(GameStation_PlayGame) != uDataSize)
			{
				return true;
			}
			GameStation_PlayGame * pGameStation=(GameStation_PlayGame *)pStationData;
			if (NULL == pGameStation)
			{
				return true;
			}
			SetStationParameter(GS_NOTE_STATE);
			m_iXiaZhuTime	= pGameStation->iXiaZhuTime;		/// 下注时间			
			m_iKaiPaiTime	= pGameStation->iKaiPaiTime;     	/// 开牌时间    
			m_iFreeTime		= pGameStation->iFreeTime;			/// 空闲时间	
			m_i64UserMoney = pGameStation->i64MyMoney;
			m_iRemainTimeXZ	= pGameStation->iRemainTimeXZ;		/// 剩余下注时间
			m_Clock =GetTickCount()+m_iRemainTimeXZ*1000;
			break;
		}
	case GS_SEND_CARD:			//发牌状态
		{
			if (sizeof(GameStation_SendCard) != uDataSize)
			{
				return true;
			}
			GameStation_SendCard * pGameStation=(GameStation_SendCard *)pStationData;
			if (NULL == pGameStation)
			{
				return true;
			}

			SetStationParameter(GS_SEND_CARD);
			m_iXiaZhuTime	= pGameStation->iXiaZhuTime;		/// 下注时间			
			m_iKaiPaiTime	= pGameStation->iKaiPaiTime;     	/// 开牌时间    
			m_iFreeTime		= pGameStation->iFreeTime;			/// 空闲时间	
			m_i64UserMoney = pGameStation->i64MyMoney;
			break;
		}
	case GS_SEND_RESULT:			//结算状态
		{
			if (sizeof(GameStation_Result) != uDataSize)
			{
				return true;
			}
			GameStation_Result * pGameStation=(GameStation_Result *)pStationData;
			if (NULL == pGameStation)
			{
				return true;
			}

			SetStationParameter(GS_SEND_RESULT);
			m_iXiaZhuTime	= pGameStation->iXiaZhuTime;		/// 下注时间			
			m_iKaiPaiTime	= pGameStation->iKaiPaiTime;     	/// 开牌时间    
			m_iFreeTime		= pGameStation->iFreeTime;			/// 空闲时间	
			m_i64UserMoney = pGameStation->i64MyMoney;
			break;
		}
	default:
		break;
	}
	return true;
}
//是否使用微妙为单位启动下注定时器
bool CClientGameDlg::SetGameTimer(BYTE bDeskStation,UINT uTimeCount,UINT uTimeID,bool bUseMillSecond)
{
	if(m_pGameRoom != NULL)
	{
		m_pGameRoom->SendMessage(IDM_SET_GAME_TIME,(WPARAM)uTimeID,(LPARAM)uTimeCount);
	}
	return true;
}

//定时器消息
bool CClientGameDlg::OnGameTimer(BYTE byDeskStation, UINT uTimeID, UINT uTimeCount)
{
	switch (uTimeID)
	{
	case TIME_XIA_ZHU:	//继续游戏定时器
		{		
			KillGameTimer(TIME_XIA_ZHU);
			OnXiaZhu();
			return true;
		}
	}
	return true;
}

// 上下庄申请按钮函数
void	CClientGameDlg::OnShangZhuang()
{
	if (CheckMeInZhuang())
	{
		return;
	}

}

// 清除所有定时器
void CClientGameDlg::KillAllTimer(void)
{
	KillGameTimer(TIME_XIA_ZHU);

}



   
//========================================================================================
bool	CClientGameDlg::CheckMeInZhuang(void)
{

	return false;
}
//========================================================================================
//机器人下注
void	CClientGameDlg::OnXiaZhu()
{
	if(GetMeUserInfo()->bDeskStation == 255)
	{
		return ;
	}
	srand((unsigned)GetTickCount());

	int iArea = rand_Mersenne(1,BET_ARES);
	if (-1 == iArea)
	{
		return;
	}
	else if (iArea == 3)
	{
		int Percent = rand_Mersenne(1,10);
		if (Percent >=3 && Percent <=6)
		{
			iArea = 1;
		}
		else if (Percent >6)
		{
			iArea = 2;
		}
	}

	int iMoneyType = rand_Mersenne(0,CHOUMA_NUM-1);

	if(G_i64ChouMaValues[iMoneyType] > m_i64UserMoney)		//检测当前自已的钱还够不够下
	{
		return ;
	}	
	
	C_S_UserNote	TUserNoTe;
	TUserNoTe.byDeskStation = GetMeUserInfo()->bDeskStation;
	TUserNoTe.iChouMaValues	= G_i64ChouMaValues[iMoneyType];
	TUserNoTe.byQuYu		= iArea;
	SendGameData(&TUserNoTe,sizeof(TUserNoTe),MDM_GM_GAME_NOTIFY,C_S_XIA_ZHU,0);

 	if (GetMeUserInfo()->bDeskStation != 255 && GetTickCount() < m_Clock)
 	{
 		SetGameTimer(GetMeUserInfo()->bDeskStation, rand_Mersenne(m_iNoteTimeMin,m_iNoteTimeMax), TIME_XIA_ZHU);	
 	}
}

//获取下注的区域
int		CClientGameDlg::GetXiaZhuAreas(int iAreasRandNum)
{
	int iAreaType = -1;
	int iSum = 0;
	for (int i=0; i<BET_ARES; i++)
	{
		iSum += m_iAreasPercent[i];
		if (iAreasRandNum < iSum)
		{
			iAreaType = i;
			break;
		}
	}
	return iAreaType;
}

//获取下注筹码类型
int		CClientGameDlg::GetXiaZhuTpye(int iRandNum)
{
	int iChouMaType = -1;
	int iSum = 0;
	for (int i=0; i<7; i++)
	{
		iSum += m_iChouMaPercent[i];
		if (iRandNum < iSum)
		{
			iChouMaType = i;
			break;
		}
	}
	return iChouMaType;
}

int  CClientGameDlg::rand_pos(int *perent,int size)
{
	int iSum = 0;
	for (int i = 0; i < size; i++)
	{
		iSum += perent[i];
	}

	int iRandNum  = rand_Mersenne(0,iSum-1);

	int iRandPos = -1;
	int iPlus = 0;
	for (int i = 0; i < size; i++)
	{
		iPlus += perent[i];
		if (iRandNum < iPlus)
		{
			iRandPos = i;
			break;
		}
	}
	return iRandPos;
}


//游戏消息处理函数
bool CClientGameDlg::HandleGameMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	
	if (MDM_GM_GAME_NOTIFY == pNetHead->bMainID)
	{
		switch (pNetHead->bAssistantID)
		{
		case S_C_NOTICE_XIA_ZHU:		//游戏开始开始下注
			{
				if (uDataSize != sizeof(S_C_GameBegin))
				{
					return true;
				}
				S_C_GameBegin *pBegin = (S_C_GameBegin *)pNetData;
				if (NULL == pBegin)
				{
					return true;
				}
				m_i64UserMoney= pBegin->i64UserMoney;
			
				//启动下注
				SetGameTimer(GetMeUserInfo()->bDeskStation, rand_Mersenne(m_iNoteTimeMin,m_iNoteTimeMax), TIME_XIA_ZHU);	
				m_Clock =GetTickCount()+m_iXiaZhuTime*1000;
				break;
			}
		case S_C_XIA_ZHU_RESULT:	//下注结果
			{
				if (uDataSize != sizeof(C_S_UserNoteResult))
				{
					return true;
				}
				C_S_UserNoteResult *pUserNoteResult = (C_S_UserNoteResult *)pNetData;
				m_i64UserMoney = pUserNoteResult->i64MyMoney;
				if (NULL == pUserNoteResult)
				{
					return true;
				}
				break;
			}
		case S_C_SEND_CARD:		//开牌
			{
				KillAllTimer();
				break;
			}
		case S_C_IS_SUPER_USER:			//超端消息
		case ASS_SUPER_USER_SET_RESULT:		//超端设置结果
			{
				return true;
			}
		case S_C_SHOW_WINAREA:	//显示中奖区域
			{
				return true;
			}
		default:
			{
				break;
			}
		}
	}
	return __super::HandleGameMessage(pNetHead,pNetData,uDataSize,pClientSocket);
}



//加载配置文件
void CClientGameDlg::LoadIniConfig()
{
	//CINIFile iniFile(CINIFile::GetAppPath()+"BRXJRobotSet.ini");
	CINIFile iniFile(CINIFile::GetAppPath()+"LHDRobotSet.ini");
	
	m_iXiaZhuTime = iniFile.GetKeyVal("RobortSet","XiaZhuTime",15);//最小下注频率;	
	if(m_iXiaZhuTime <=0)
	{
		m_iXiaZhuTime = 15;
	}
	m_iKaiPaiTime = iniFile.GetKeyVal("RobortSet","KaiPaiTime",5);	
	if(m_iKaiPaiTime <0)
	{
		m_iKaiPaiTime = 5;
	}
	m_iFreeTime = iniFile.GetKeyVal("RobortSet","FreeTime",3);	
	if(m_iFreeTime <0)
	{
		m_iFreeTime = 3;
	}

	m_iNoteTimeMin = iniFile.GetKeyVal("RobortSet","NoteTimeMin",200);
	m_iNoteTimeMax = iniFile.GetKeyVal("RobortSet","NoteTimeMax",1000);


}
