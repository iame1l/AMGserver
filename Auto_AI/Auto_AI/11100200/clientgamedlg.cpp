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

	m_iXiaZhuTime = 15;			//下注时间			
	m_iKaiPaiTime = 10;         //开牌时间
	m_iFreeTime = 20;			//空闲时间
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

//void CClientGameDlg::DoDataExchange(CDataExchange* pDX)
//{
//	// TODO: 在此添加专用代码和/或调用基类
//	CLoveSendClass::DoDataExchange(pDX);
//	//CLoveSendClass::DoDataExchange(pDX);
//	DoDataExchangeWebVirFace(pDX);
//}


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
		{
			if (sizeof(GameStationBase) != uDataSize)
			{
				return true;
			}
			GameStationBase * pGameStation=(GameStationBase *)pStationData;
			if (NULL == pGameStation)
			{
				return true;
			}

			SetStationParameter(GS_WAIT_SETGAME);
			m_iXiaZhuTime	= pGameStation->iXiaZhuTime;		/// 下注时间			
			m_iKaiPaiTime	= pGameStation->iKaiPaiTime;     	/// 开牌时间    
			m_iFreeTime		= pGameStation->iFreeTime;			/// 空闲时间	
			m_iShowResult	= pGameStation->iShowWinTime;		/// 显示中奖时间
			m_iNowNtStation	= pGameStation->iNtStation;			///当前庄家的位置
			m_iNtPlayCount	= pGameStation->iNtPlayCount;		//庄家坐庄次数
			m_i64NtMoney	= pGameStation->i64NtMoney;			//庄家金币
			m_i64NtWinMoney	= pGameStation->i64NtWinMoney;		//庄家输赢情况
			m_i64LimitMoney	= pGameStation->i64ShangZhuangLimit;
			m_i64UserMaxNote= pGameStation->i64UserMaxNote;		//玩家最大下注数
			//上庄列表
			memcpy(m_byZhuangList,pGameStation->byZhuangList,sizeof(m_byZhuangList));

		
			if (m_iNowNtStation != GetMeUserInfo()->bDeskStation)
			{
				//等待计时器，如果10秒时间到了还没有庄家就主动上庄
				SetGameTimer(GetMeUserInfo()->bDeskStation, (rand()%10 + 1)*1000, TIME_SHANG_ZHUANG);		

			}

			break;
		}
	case GS_WAIT_AGREE:		//等待同意状态
	case GS_WAIT_NEXT:		//23等待下一盘开始
		{
			if (sizeof(GameStationBase) != uDataSize)
			{
				return true;
			}
			GameStationBase * pGameStation=(GameStationBase *)pStationData;
			if (NULL == pGameStation)
			{
				return true;
			}

			SetStationParameter(GS_WAIT_AGREE);
			m_iXiaZhuTime	= pGameStation->iXiaZhuTime;		/// 下注时间			
			m_iKaiPaiTime	= pGameStation->iKaiPaiTime;     	/// 开牌时间    
			m_iFreeTime		= pGameStation->iFreeTime;			/// 空闲时间	
			m_iShowResult	= pGameStation->iShowWinTime;		/// 显示中奖时间
			m_iNowNtStation	= pGameStation->iNtStation;			///当前庄家的位置
			m_iNtPlayCount	= pGameStation->iNtPlayCount;		//庄家坐庄次数
			m_i64NtMoney	= pGameStation->i64NtMoney;			//庄家金币
			m_i64NtWinMoney	= pGameStation->i64NtWinMoney;		//庄家输赢情况
			m_i64LimitMoney	= pGameStation->i64ShangZhuangLimit;
			m_i64UserMaxNote= pGameStation->i64UserMaxNote;		//玩家最大下注数
	
			//上庄列表
			memcpy(m_byZhuangList,pGameStation->byZhuangList,sizeof(m_byZhuangList));

			
			break;
		}
	case GS_NOTE_STATE:			//下注状态
		{
			if (sizeof(GameStation_Bet) != uDataSize)
			{
				return true;
			}
			GameStation_Bet * pGameStation=(GameStation_Bet *)pStationData;
			if (NULL == pGameStation)
			{
				return true;
			}

			SetStationParameter(GS_NOTE_STATE);
			m_iXiaZhuTime	= pGameStation->iXiaZhuTime;		/// 下注时间			
			m_iKaiPaiTime	= pGameStation->iKaiPaiTime;     	/// 开牌时间    
			m_iFreeTime		= pGameStation->iFreeTime;			/// 空闲时间	
			m_iShowResult	= pGameStation->iShowWinTime;		/// 显示中奖时间
			m_iNowNtStation	= pGameStation->iNtStation;			///当前庄家的位置
			m_iNtPlayCount	= pGameStation->iNtPlayCount;		//庄家坐庄次数
			m_i64NtMoney	= pGameStation->i64NtMoney;			//庄家金币
			m_i64NtWinMoney	= pGameStation->i64NtWinMoney;		//庄家输赢情况
			m_i64LimitMoney	= pGameStation->i64ShangZhuangLimit;
			m_i64UserMaxNote= pGameStation->i64UserMaxNote;		//玩家最大下注数
	
			//上庄列表
			memcpy(m_byZhuangList,pGameStation->byZhuangList,sizeof(m_byZhuangList));
			
			m_i64ZhongZhu	= pGameStation->i64ZhongZhu;
			memcpy(m_i64MyZhu,pGameStation->i64UserXiaZhuData,sizeof(m_i64MyZhu));
			memcpy(m_i64QuYuZhu,pGameStation->i64QuYuZhu,sizeof(m_i64QuYuZhu));
			//八个区域可下注数
			memcpy(m_i64AreasMaxZhu,pGameStation->i64AreaMaxZhu,sizeof(m_i64AreasMaxZhu));


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
			m_iShowResult	= pGameStation->iShowWinTime;		/// 显示中奖时间
			m_iNowNtStation	= pGameStation->iNtStation;			///当前庄家的位置
			m_iNtPlayCount	= pGameStation->iNtPlayCount;		//庄家坐庄次数
			m_i64NtMoney	= pGameStation->i64NtMoney;			//庄家金币
			m_i64NtWinMoney	= pGameStation->i64NtWinMoney;		//庄家输赢情况
			m_i64LimitMoney	= pGameStation->i64ShangZhuangLimit;
			m_i64UserMaxNote= pGameStation->i64UserMaxNote;		//玩家最大下注数
		
			//上庄列表
			memcpy(m_byZhuangList,pGameStation->byZhuangList,sizeof(m_byZhuangList));
			break;
		}
	case GS_SHOW_WIN:			//显示中奖区域
		{
			if (sizeof(GameStation_ShowWin) != uDataSize)
			{
				return true;
			}
			GameStation_ShowWin * pGameStation=(GameStation_ShowWin *)pStationData;
			if (NULL == pGameStation)
			{
				return true;
			}

			SetStationParameter(GS_SHOW_WIN);
			m_iXiaZhuTime	= pGameStation->iXiaZhuTime;		/// 下注时间			
			m_iKaiPaiTime	= pGameStation->iKaiPaiTime;     	/// 开牌时间    
			m_iFreeTime		= pGameStation->iFreeTime;			/// 空闲时间	
			m_iShowResult	= pGameStation->iShowWinTime;		/// 显示中奖时间
			m_iNowNtStation	= pGameStation->iNtStation;			///当前庄家的位置
			m_iNtPlayCount	= pGameStation->iNtPlayCount;		//庄家坐庄次数
			m_i64NtMoney	= pGameStation->i64NtMoney;			//庄家金币
			m_i64NtWinMoney	= pGameStation->i64NtWinMoney;		//庄家输赢情况
			m_i64LimitMoney	= pGameStation->i64ShangZhuangLimit;
			m_i64UserMaxNote= pGameStation->i64UserMaxNote;		//玩家最大下注数
		
			//上庄列表
			memcpy(m_byZhuangList,pGameStation->byZhuangList,sizeof(m_byZhuangList));
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
	case TIME_SHANG_ZHUANG:	//继续游戏定时器
		{			
			KillGameTimer(TIME_SHANG_ZHUANG);
			//OnShangZhuang();
			return true;
		}
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
	if (GetMeUserInfo()->i64Money >= m_i64LimitMoney)
	{
		C_S_ApplyShangZhuang TApplyShangZhuang;
		TApplyShangZhuang.bShangZhuang = true;
		SendGameData(&TApplyShangZhuang,sizeof(TApplyShangZhuang),MDM_GM_GAME_NOTIFY,C_S_APPLY_ZHUANG,0);
	}
}

// 清除所有定时器
void CClientGameDlg::KillAllTimer(void)
{
	KillGameTimer(TIME_XIA_ZHU);
	KillGameTimer(TIME_SHANG_ZHUANG);
}









   
//========================================================================================
bool	CClientGameDlg::CheckMeInZhuang(void)
{
	if(GetMeUserInfo()->bDeskStation == m_iNowNtStation)
	{
		return true;
	}
	for(int i=0;i<PLAY_COUNT;i++)
	{
		if(GetMeUserInfo()->bDeskStation == m_byZhuangList[i])//我在庄家列表
		{
			return true; 
		}
	}
	return false;
}
//========================================================================================
//机器人下注
void	CClientGameDlg::OnXiaZhu()
{
	if(GetMeUserInfo()->bDeskStation == m_iNowNtStation)
	{
		return ;
	}
	srand((unsigned)GetTickCount());
	//下注区域
	int iAreasSum = 0;
	for (int i=0; i<BET_ARES; i++)
	{
		iAreasSum += m_iAreasPercent[i];
	}
	//下注区域
	int iArea = GetXiaZhuAreas(rand_Mersenne(0,iAreasSum-1));
	if (-1 == iArea)
	{
		return;
	}

	//下注金额
	int iMoneySum = 0;
	for (int i=0; i<7; i++)
	{
		iMoneySum += m_iChouMaPercent[i];
	}

	int tChouMaPercent[7] = {0};
	for (int i = 0;i < 7;i++)
	{
		tChouMaPercent[i] = m_iChouMaPercent[i];
		if(iArea != 0 && iArea != 3)
		{
			if(i == 5 || i == 6 || i == 7)
			{
				tChouMaPercent[i] == 0;
			}
		}
	}

	int iMoneyType = rand_pos(tChouMaPercent,7);
	if (-1 == iMoneyType)
	{
		return;
	}


	if(G_i64ChouMaValues[iMoneyType]>GetMeUserInfo()->i64Money)		//检测当前自已的钱还够不够下
	{
		return ;
	}	
	
	C_S_UserNote	TUserNoTe;
	TUserNoTe.byDeskStation = GetMeUserInfo()->bDeskStation;
	TUserNoTe.iChouMaType	= iMoneyType;
	TUserNoTe.iNoteArea		= iArea;
	SendGameData(&TUserNoTe,sizeof(TUserNoTe),MDM_GM_GAME_NOTIFY,C_S_XIA_ZHU,0);


	if (GetMeUserInfo()->bDeskStation != m_iNowNtStation)
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
		case S_C_APPLY_ZHUANG_RESULT:	//申请上庄结果
			{
				if (uDataSize != sizeof(S_C_ApplyZhuangResult))
				{
					return true;
				}
				S_C_ApplyZhuangResult *pApplyZhuangResult = (S_C_ApplyZhuangResult *)pNetData;
				if (NULL == pApplyZhuangResult)
				{
					return true;
				}
				memcpy(m_byZhuangList,pApplyZhuangResult->byZhuangList,sizeof(m_byZhuangList));
				break;
			}
		case S_C_NO_NT_WAITE:	//无庄等待
			{
				if (uDataSize != sizeof(S_C_NoNtStation))
				{
					return true;
				}
				S_C_NoNtStation *pNoNtStation = (S_C_NoNtStation *)pNetData;
				if (NULL == pNoNtStation)
				{
					return true;
				}
				m_iNowNtStation = pNoNtStation->iNowNtStation;	///当前庄家的位置
				m_i64NtMoney	= pNoNtStation->i64NtMoney;		//庄家金币

				//等待计时器，如果10秒时间到了还没有庄家就主动上庄
				SetGameTimer(GetMeUserInfo()->bDeskStation, (rand()%10 + 1)*1000, TIME_SHANG_ZHUANG);
				break;
			}
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
				m_iNowNtStation = pBegin->iNowNtStation;	///当前庄家的位置
				m_i64NtMoney	= pBegin->i64NtMoney;		//庄家金币

				if (GetMeUserInfo()->bDeskStation != m_iNowNtStation)
				{
					//启动下注
					SetGameTimer(GetMeUserInfo()->bDeskStation, rand_Mersenne(m_iNoteTimeMin,m_iNoteTimeMax), TIME_XIA_ZHU);	
				}
				break;
			}
		case S_C_XIA_ZHU_RESULT:	//下注结果
			{
				if (uDataSize != sizeof(C_S_UserNoteResult))
				{
					return true;
				}
				C_S_UserNoteResult *pUserNoteResult = (C_S_UserNoteResult *)pNetData;
				if (NULL == pUserNoteResult)
				{
					return true;
				}

				memcpy(m_i64MyZhu,pUserNoteResult->i64UserXiaZhuData,sizeof(m_i64MyZhu));
				memcpy(m_i64QuYuZhu,pUserNoteResult->i64QuYuZhu,sizeof(m_i64QuYuZhu));
				break;
			}
		case S_C_GAME_END:		//游戏结束结算消息
			{
				//等待计时器，如果10秒时间到了还没有庄家就主动上庄
				SetGameTimer(GetMeUserInfo()->bDeskStation, (rand()%10 + 1)*1000, TIME_SHANG_ZHUANG);	
				break;
			}
		case S_C_IS_SUPER_USER:			//超端消息
		case S_C_SUPER_SET_RESULT:		//超端设置结果
			{
				return true;
			}
		case S_C_SEND_CARD:		//开始发牌
		case S_C_SHOW_WINAREA:	//显示中奖区域
			{
				//KillAllTimer();
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
	CINIFile iniFile(CINIFile::GetAppPath()+"BJLRobotSet.ini");
	//最小下注频率
	m_iNoteTimeMin = iniFile.GetKeyVal("RobortSet","NoteTimeMin",200);
	m_iNoteTimeMax = iniFile.GetKeyVal("RobortSet","NoteTimeMin",1000);
	//下注筹码的概率
	m_iChouMaPercent[0] = iniFile.GetKeyVal("RobortSet","ChouMaPercent[1]",10000);
	//下注筹码的概率
	m_iChouMaPercent[1] = iniFile.GetKeyVal("RobortSet","ChouMaPercent[2]",5000);
	//下注筹码的概率
	m_iChouMaPercent[2] = iniFile.GetKeyVal("RobortSet","ChouMaPercent[3]",1000);
	//下注筹码的概率
	m_iChouMaPercent[3] = iniFile.GetKeyVal("RobortSet","ChouMaPercent[4]",500);
	//下注筹码的概率
	m_iChouMaPercent[4] = iniFile.GetKeyVal("RobortSet","ChouMaPercent[5]",100);
	//下注筹码的概率
	m_iChouMaPercent[5] = iniFile.GetKeyVal("RobortSet","ChouMaPercent[6]",10);
	//下注筹码的概率
	m_iChouMaPercent[6] = iniFile.GetKeyVal("RobortSet","ChouMaPercent[7]",5);

	m_iAreasPercent[0] = iniFile.GetKeyVal("RobortSet","AreasPercent[0]",198);	//闲的概率
	m_iAreasPercent[1] = iniFile.GetKeyVal("RobortSet","AreasPercent[1]",132);	//闲天王的概率
	m_iAreasPercent[2] = iniFile.GetKeyVal("RobortSet","AreasPercent[2]",33);	//闲对子的概率
	m_iAreasPercent[3] = iniFile.GetKeyVal("RobortSet","AreasPercent[3]",198);	//庄的概率
	m_iAreasPercent[4] = iniFile.GetKeyVal("RobortSet","AreasPercent[4]",132);	//庄天王的概率
	m_iAreasPercent[5] = iniFile.GetKeyVal("RobortSet","AreasPercent[5]",33);	//庄对子的概率
	m_iAreasPercent[6] = iniFile.GetKeyVal("RobortSet","AreasPercent[6]",44);	//和的概率
	m_iAreasPercent[7] = iniFile.GetKeyVal("RobortSet","AreasPercent[7]",12);	//同点和的概率

}
