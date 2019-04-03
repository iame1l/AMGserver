#include "StdAfx.h"
#include "ServerManage.h"
#include "Shlwapi.h"
#include <random>

using namespace std;
/*利用梅森素数产生随机数介于[_Min,_Max]
VS版本必须大于2008
*/
int rand_Mersense(const int _Min, int _Max)
{
	if (_Min > _Max) { return 0; }
	if (_MSC_VER < 1500) { return 0; }
	random_device rd;
	mt19937 mt(rd());
	uniform_int_distribution<> dis(_Min, _Max);
	return dis(mt);
}


void DebugPrintf(const char *p, ...)
{
	return ;
	char szFilename[256];
	sprintf( szFilename, "%dServer.txt", NAME_ID);
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
void MsgPrint(const char *lpszFormat, ...)
{
	va_list pArg;
	va_start(pArg, lpszFormat);
	char szMessage[1024] = { 0 };
#ifdef WIN32
	_vsnprintf(szMessage, 1023, lpszFormat, pArg);
#else
	vsnprintf(szMessage, 1023, lpszFormat, pArg);
#endif
	va_end(pArg);
	OutputDebugString(szMessage);

}

__int64 my_atoi(const char *str)
{
	__int64 result = 0;
	int signal = 1;
	/* 默认为正数 */
	if((*str>='0'&&*str<='9')||*str=='-'||*str=='+')
	{
		if(*str=='-'||*str=='+')
		{
			if(*str=='-')
				signal = -1; /* 输入负数 */
			str++;
		}
	}
	else return 0;/* 开始转换 */
	while(*str>='0'&&*str<='9')
		result = result*10+(*str++ -'0');
	return signal*result;
}
///名称：CServerGameDesk
///描述：服务器构造函数
///@param ALL_ARGEE 游戏开始模式（所有玩家同意即可开始）
///@return 
CServerGameDesk::CServerGameDesk(void):CGameDesk(ALL_ARGEE)
{
	m_bGameStation=GS_WAIT_SETGAME;  // 初始为等待开始状态

	m_bAIWinAndLostAutoCtrl		= false;///机器人输赢控制：是否开启机器人输赢自动控制
	m_i64AIWantWinMoneyA1	= 100000;		/**<机器人输赢控制：机器人赢钱区域1  */
	m_i64AIWantWinMoneyA2	= 1000000;		/**<机器人输赢控制：机器人赢钱区域2  */
	m_i64AIWantWinMoneyA3	= 10000000;		/**<机器人输赢控制：机器人赢钱区域3  */
	m_iAIWinLuckyAtA1	= 90;			/**<机器人输赢控制：机器人在区域1赢钱的概率  */
	m_iAIWinLuckyAtA2	= 70;			/**<机器人输赢控制：机器人在区域2赢钱的概率  */
	m_iAIWinLuckyAtA3	= 50;			/**<机器人输赢控制：机器人在区域3赢钱的概率  */
	m_iAIWinLuckyAtA4	= 30;			/**<机器人输赢控制：机器人在区域4赢钱的概率  */
	m_i64AIHaveWinMoney	= 0;			///机器人输赢控制：机器人已经赢了多少钱

	m_iXiaZhuTime	= 15;				/**< 下注时间	*/		
	m_iKaiPaiTime	= 10;     			/**< 开牌时间*/
	m_iShowResult	= 5;				//显示结算框时间
	m_iFreeTime		= 15;				/**< 空闲时间*/

	m_iRobotSZCount	= 5;            /**< 允许机器人上庄数量 */
	m_iMaxZhuang	= 5;				/**< 庄家一次最多做多少局，默认30局 */
	m_iNtTax		= 0;					///庄家抽水控制.0-只抽庄家的税 1-抽所有赢家的税收/收取所有玩家的台费
	m_i64ShangZhuangLimit = 10000; 		/**< 上庄需要的最少金币*/
	m_i64MaxNote	= 0;				///最大能下的总注
	m_i64UserMaxNote= 0;				///每局玩家最大下注数

	m_iNowNtStation	= -1;						/**< 当前庄家位置*/
	m_iZhuangBaShu	= 0;							/**< 庄家进行了几把*/
	m_i64NtMoney	= 0;							//庄家的金币
	m_i64NtWin		= 0;            					/**< 当前庄家赢的金币*/
	m_bXiaZhuang	= false;							/**< 当前庄家申请下庄*/	
	memset(m_i64UserWin,0,sizeof(m_i64UserWin));				//玩家的输赢情况
	m_qZhuangList.clear();
	m_DqLuziData.clear();

	m_dqUserPlayResult.clear();
	//初始化部分数据 游戏一开始需要初始化的数据
	ServerInit();
	memset(&m_iRobotRectNoteMax, 0, sizeof(m_iRobotRectNoteMax));
}
///名称：~CServerGameDesk
///描述：服务器析造函数
///@param 
///@return 
CServerGameDesk::~CServerGameDesk(void)
{	
}
///名称：InitDeskGameStation
///描述：初始化桌子函数，一般在这里加载配置文件
///@param 
///@return 
bool CServerGameDesk::InitDeskGameStation()
{
	//加载配置文件
	LoadIni();
	//重新加载配置文件里面的
	//统一房间
	//LoadExtIni(m_pDataManage->m_InitData.uRoomID);
	return true;
}


//初始化所有数据
void	CServerGameDesk::InitAllData()
{
	m_bGameStation=GS_WAIT_SETGAME;  // 初始为等待开始状态

	m_iNowNtStation	= -1;						/**< 当前庄家位置*/
	m_iZhuangBaShu	= 0;							/**< 庄家进行了几把*/
	m_i64NtMoney	= 0;							//庄家的金币
	m_i64NtWin		= 0;            					/**< 当前庄家赢的金币*/
	m_bXiaZhuang	= false;							/**< 当前庄家申请下庄*/		
	m_qZhuangList.clear();
	memset(m_i64UserWin,0,sizeof(m_i64UserWin)); //玩家的输赢情况
	m_DqLuziData.clear();
	m_dqUserPlayResult.clear();
	//初始化部分数据 游戏一开始需要初始化的数据
	ServerInit();
}

///名称：LoadExtIni
///描述：加载服务器端ini配置文件，发牌时间，发牌帧数
///@param 
///@return 
BOOL CServerGameDesk::LoadIni()
{
	CString s = CBcfFile::GetAppPath ();/////本地路径
	CINIFile f(s +SKIN_FOLDER  + _T("_s.ini"));
	TCHAR szSec[_MAX_PATH] = TEXT("game");
	CString strChouMa;
	for (int i = 0; i < CHOUMANUM; ++i)
	{
		strChouMa.Format("ChouMaValue[%d]", i);
		m_i64ChouMaValue[i] = f.GetKeyVal(szSec, strChouMa, 1000);
	}
	///获取空闲时间
	m_iFreeTime = f.GetKeyVal(szSec,"FreeTime",10);		
	if(m_iFreeTime<10)
	{
		m_iFreeTime=10;
	}
	///获取下注时间
	m_iXiaZhuTime = f.GetKeyVal(szSec,"XiazhuTime",15);	
	if(m_iXiaZhuTime<10)
	{
		m_iXiaZhuTime=10;
	}
	///获取开牌时间
	m_iKaiPaiTime = f.GetKeyVal(szSec,"KaipaiTime",15);	
	if(m_iKaiPaiTime<10)
	{
		m_iKaiPaiTime=10;
	}
	//结算框显示时间
	m_iShowResult = f.GetKeyVal(szSec,"ShowResultTime",5);
	if (m_iShowResult <1)
	{
		m_iShowResult = 5;
	}
	
	///获取上庄需要的最少金币
	m_i64ShangZhuangLimit = f.GetKeyVal(szSec,"ShangZhuangMoney",1000);
	///庄家一次最多做多少局，默认5局
	m_iMaxZhuang = f.GetKeyVal(szSec,"MaxZhuang",5);
	///庄家抽水控制
	m_iNtTax = f.GetKeyVal(szSec,"NtTax",0);	
	///当前房间最大能下的总注
	m_i64MaxNote = f.GetKeyVal(szSec,"MaxNote",0);
	//单个玩家最大下注限额
	m_i64UserMaxNote = f.GetKeyVal(szSec,"UserMaxNote",0);
	// 读取机器人上庄玩家数量
	m_iRobotSZCount = f.GetKeyVal(szSec, "RobotSZCount", 15);

	//机器人下注区域限制
	CString strName ;
	for(int i = 0 ; i< BET_ARES ; i++)
	{
		strName.Format("RobotRectNoteMax[%d]",i) ; 
		m_iRobotRectNoteMax[i] = f.GetKeyVal(szSec , strName ,0) ; 
		if(m_iRobotRectNoteMax[i]<0)
		{
			m_iRobotRectNoteMax[i]=0;
		}
	}

	//读取超端数据 
	CString key = "SuperSet";
	// 判断超级用户在服务器中是否配置
	int iCount = f.GetKeyVal(key, "SuperUserCount", 0);
	CString strText;
	for (int j = 1; j <= iCount; j++)
	{
		strText.Format("SuperUserID%d", j);
		long int  lUserID = f.GetKeyVal(key, strText, 0);
		m_vtSuperUserID.push_back(lUserID);
	}
	return TRUE;
}

///名称：LoadExtIni
///描述：根据房间ID加载配置文件
///@param iRoomID 房间号
///@return 
BOOL CServerGameDesk::LoadExtIni(int iRoomID)
{
	CString s = CBcfFile::GetAppPath ();/////本地路径
	CINIFile f(s +SKIN_FOLDER  + _T("_s.INI"));
	TCHAR szSec[_MAX_PATH] = TEXT("game");
	_stprintf_s(szSec, sizeof(szSec), _T("%s_%d"), SKIN_FOLDER,iRoomID);

	
	///获取空闲时间
	m_iFreeTime = f.GetKeyVal(szSec,"FreeTime",m_iFreeTime);		
	if(m_iFreeTime<10)
	{
		m_iFreeTime=10;
	}
	///获取下注时间
	m_iXiaZhuTime = f.GetKeyVal(szSec,"XiazhuTime",m_iXiaZhuTime);	
	if(m_iXiaZhuTime<10)
	{
		m_iXiaZhuTime=10;
	}
	///获取开牌时间
	m_iKaiPaiTime = f.GetKeyVal(szSec,"KaipaiTime",m_iKaiPaiTime);	
	if(m_iKaiPaiTime<10)
	{
		m_iKaiPaiTime=10;
	}
	//结算框显示时间
	m_iShowResult = f.GetKeyVal(szSec,"ShowResultTime",m_iShowResult);
	if (m_iShowResult <1)
	{
		m_iShowResult = 5;
	}

	///获取上庄需要的最少金币
	m_i64ShangZhuangLimit = f.GetKeyVal(szSec,"ShangZhuangMoney",m_i64ShangZhuangLimit);
	///庄家一次最多做多少局，默认5局
	m_iMaxZhuang = f.GetKeyVal(szSec,"MaxZhuang",m_iMaxZhuang);
	///庄家抽水控制
	m_iNtTax = f.GetKeyVal(szSec,"NtTax",m_iNtTax);	
	///当前房间最大能下的总注
	m_i64MaxNote = f.GetKeyVal(szSec,"MaxNote",m_i64MaxNote);
	//单个玩家最大下注限额
	m_i64UserMaxNote = f.GetKeyVal(szSec,"UserMaxNote",m_i64UserMaxNote);
	// 读取机器人上庄玩家数量
	m_iRobotSZCount = f.GetKeyVal(szSec, "RobotSZCount", m_iRobotSZCount);

	//下注区域限制
	CString strName ;
	for(int i = 0 ; i< BET_ARES ; i++)
	{
		strName.Format("RobotRectNoteMax[%d]",i) ; 
		m_iRobotRectNoteMax[i] = f.GetKeyVal(szSec , strName ,m_iRobotRectNoteMax[i]) ; 
		if(m_iRobotRectNoteMax[i]<0)
		{
			m_iRobotRectNoteMax[i]=0;
		}
	}

	return TRUE;
}


/************************************************************************/
///描述：游戏开始
bool	CServerGameDesk::GameBegin(BYTE bBeginFlag)
{
	//所有玩家置为同意状态
	for(int i=0;i<PLAY_COUNT;i++)
	{
		if(NULL == m_pUserInfo[i])//玩家不存在就
		{
			m_i64UserWin[i] = 0;
			continue;
		}
		m_pUserInfo[i]->m_UserData.bUserState = USER_ARGEE;
	}
	
	// 调用系统初始化游戏开始状态
	if (!__super::GameBegin(bBeginFlag)) 
	{
		GameFinish(0,GF_SAFE);
		return false;
	}
	ServerInit();
	// 游戏一开始就进入下注状态。
	m_bGameStation = GS_NOTE_STATE;         
	
	//这里每盘根据房间ID读取下配置文件中的ReSetAIHaveWinMoney 参数 设定下机器人赢了多少钱
	GetAIContrlSetFromIni(m_pDataManage->m_InitData.uRoomID);
	//庄家坐庄把数+1
	m_iZhuangBaShu ++;
	//是否能下注
	m_bCanXiaZhu = true;		

	//计算本局最大下注数字
	//到了这里铁定有庄家的
	m_i64PerMaxNotes = m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money;

	if (m_i64MaxNote >0 && m_i64PerMaxNotes > m_i64MaxNote)
	{
		m_i64PerMaxNotes = m_i64MaxNote;
	}

	//获取各个区域最大下注 
	GetQuYuCanNote();
	//发送游戏开始消息 通知开始下注
	S_C_GameBegin TGameBegin;
	TGameBegin.iNowNtStation	= m_iNowNtStation;
	TGameBegin.i64NtMoney		= m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money;
	TGameBegin.i64NtWinMoney	= m_i64NtWin;		
	TGameBegin.iNtPlayCount		= m_iZhuangBaShu;
	TGameBegin.i64MaxXiaZhu		= m_i64PerMaxNotes;

	for(int i=0;i<BET_ARES;i++)
	{
		TGameBegin.i64AreaMaxZhu[i] = m_i64AreasMaxZhu[i];
	}
	
	for (auto itr = m_qZhuangList.begin();itr != m_qZhuangList.end();itr++)
	{
		TGameBegin.byZhuangList[itr - m_qZhuangList.begin()] = *itr;
	}

	for (int i=0;i<PLAY_COUNT; ++i)
	{
		if (NULL == m_pUserInfo[i])
		{
			continue;
		}	
		TGameBegin.i64UserMoney = m_pUserInfo[i]->m_UserData.i64Money;
		SendGameData(i,&TGameBegin,sizeof(TGameBegin),MDM_GM_GAME_NOTIFY, S_C_NOTICE_XIA_ZHU,0);
		SetTimer(ID_TIMER_REMAINTIME_GAME,1000);
	}

	//启动下注完成计时器
	SetTimer(ID_TIMER_SEND_CARD,m_iXiaZhuTime*1000);
	return true;
}
/*----------------------------------------------------------------------------*/
///名称：GameFinish
///描述：游戏结束
///@param byDeskStation 结束位置，bCloseFlag 结束方式
bool CServerGameDesk::GameFinish(BYTE byDeskStation, BYTE bCloseFlag)
{
	
	switch (bCloseFlag)
	{
	case GF_NORMAL:
		{
			KillAllTimer();
			m_bGameStation = GS_WAIT_NEXT;
			//结算分数
			CountFen();
			//清空数据
			ServerInit();
			//启动下一轮开始计时器
			SetTimer(ID_TIMER_GAME_NEW_GAME,(m_iFreeTime+5)*1000);
			break;
		}
	case GF_SAFE:
		{
			//初始化所有数据
			InitAllData();
			NoNtWait();
			break;
		}
	case GFF_FORCE_FINISH:		//用户断线离开
		{
			__int64 i64XiaZhuSum = 0;
			for (int i=0; i<BET_ARES; i++)
			{
				i64XiaZhuSum += m_i64UserXiaZhuData[byDeskStation][i];
			}
			if ((byDeskStation != m_iNowNtStation)	&& i64XiaZhuSum == 0)
			{
				MakeUserOffLine(byDeskStation);
			}
			return true;
		}
	default:
		{
			break;
		}
	}

	return __super::GameFinish(byDeskStation,bCloseFlag);
}

/*-----------------------------------------------------------------------------------------*/
/**
* 框架游戏数据包处理函数
*
* @param [in] bDeskStation 玩家位置
*
* @param [in] pNetHead 消息头
*
* @param [in] pData 消息体
*
* @param [in] uSize 消息包的大小
*
* @param [in] uSoketID Socket ID
*
* @param [in] bWatchUser 旁观
*
* @return 返回布尔类型
*/
bool CServerGameDesk::HandleFrameMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	switch(pNetHead->bAssistantID)
	{
	case ASS_GM_FORCE_QUIT:	 //强行退出
		{
			return true;
		}
	}
	return __super::HandleFrameMessage( bDeskStation,  pNetHead,  pData,  uSize,  uSocketID,  bWatchUser);
}
/*-----------------------------------------------------------------------------------------*/
///名称：HandleNotifyMessage
///描述：游戏数据包处理函数 
///@param byDeskStation 位置 pNetHead 消息头 pData 消息体 uSize 消息包的大小 uSocketID Socket ID  bWatchUser 旁观
///@return 
bool CServerGameDesk::HandleNotifyMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	if (bWatchUser)
	{
		return true;
	}
	switch (pNetHead->bAssistantID)
	{
	case C_S_APPLY_ZHUANG:
		{
			if (sizeof(C_S_ApplyShangZhuang) != uSize)
			{
				return false;
			}
			C_S_ApplyShangZhuang *pdata = (C_S_ApplyShangZhuang *)pData;
			if (NULL == pData)
			{
				return false;
			}
			//收到申请上下庄的消息
			OnHandleShangZhuang(bDeskStation,pdata);
			return true;
		}
	case C_S_XIA_ZHU:	//玩家下注
		{
			if (sizeof(C_S_UserNote) != uSize)
			{
				return false;
			}

			C_S_UserNote *pUserNoTe = (C_S_UserNote *)pData;
			if (NULL == pUserNoTe)
			{
				return false;
			}
			OnHandleXiaZhu(bDeskStation,pUserNoTe);
			return true;
		}
	case C_S_SUPER_SET:	//超端设置
		{
			if (sizeof(SuperUserSetData) != uSize)
			{
				return false;
			}

			SuperUserSetData *pSuperUserSet = (SuperUserSetData *)pData;
			if (NULL == pSuperUserSet)
			{
				return false;
			}
			OnHandleSuperSet(bDeskStation,pSuperUserSet);
			return true;
		}

	default:
		break;
	}
	return __super::HandleNotifyMessage(bDeskStation, pNetHead, pData, uSize, uSocketID, bWatchUser);
}
/*-----------------------------------------------------------------------------------------*/
//收到申请上下庄的消息
void	CServerGameDesk::OnHandleShangZhuang(BYTE bDeskStation,C_S_ApplyShangZhuang *pdata)
{
	if (m_pUserInfo[bDeskStation] == NULL)
	{
		return;
	}
	
	if(pdata->bShangZhuang)//上庄申请
	{
		//判断机器人是否超过指定的数量
		if (m_pUserInfo[bDeskStation]->m_UserData.isVirtual > 0)
		{
			if (CountRobortNum() >= m_iRobotSZCount)
			{
				return;
			}
		}
		//检测是否在游戏列表中
		if (bDeskStation == m_iNowNtStation || CheckInZhuangList(bDeskStation))
		{
			return;
		}
		if(m_pUserInfo[bDeskStation]->m_UserData.i64Money>= m_i64ShangZhuangLimit && m_pUserInfo[bDeskStation]->m_UserData.i64Money<150000000000000000)
		{
			//如果当前没有庄家 ， 就直接开始游戏
			//庄家下庄或者离开的时候，应该立马换庄家，如果有人申请上庄了，而没有庄家，铁定庄家列表里面没有人，而且铁定是五庄等待状态
			if (-1 == m_iNowNtStation)
			{
				m_iNowNtStation = bDeskStation;
				StartGame();
			}
			else
			{

				m_qZhuangList.push_back(bDeskStation);
				banker.onUserApplyBanker(bDeskStation,isRobot(bDeskStation));

				S_C_ApplyZhuangResult	TApplyZhuangResult;
				for (auto itr = m_qZhuangList.begin();itr != m_qZhuangList.end();itr++)
				{
					TApplyZhuangResult.byZhuangList[itr - m_qZhuangList.begin()] = *itr;
				}

				TApplyZhuangResult.iNowNtStation= m_iNowNtStation;
				TApplyZhuangResult.iZhuangBaShu = m_iZhuangBaShu;
				TApplyZhuangResult.i64NtWin		= m_i64NtWin;
				if (NULL != m_pUserInfo[m_iNowNtStation])
				{
					TApplyZhuangResult.i64NtMoney = m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money;
				}
				else
				{
					TApplyZhuangResult.i64NtMoney = 0;
				}

				for(int i=0; i<PLAY_COUNT; i++)
				{
					if (NULL == m_pUserInfo[i])
					{
						continue;
					}
					SendGameData(i,&TApplyZhuangResult,sizeof(TApplyZhuangResult),MDM_GM_GAME_NOTIFY, S_C_APPLY_ZHUANG_RESULT,0);//发送下注消息
				}
			}
		}
	}
	else
	{
		auto itr = find(m_qZhuangList.begin(),m_qZhuangList.end(),bDeskStation);
		//如果是庄家 
		if (bDeskStation == m_iNowNtStation)
		{
			////游戏中 那么就标记庄家申请了下庄
			m_bXiaZhuang = true;
		}
		else if (itr != m_qZhuangList.end())	//在庄家列表中删除
		{
			banker.onUserLeaveBanker(*itr,isRobot(*itr));
			m_qZhuangList.erase(itr);
			S_C_ApplyZhuangResult	TApplyZhuangResult;
			TApplyZhuangResult.iNowNtStation= m_iNowNtStation;
			TApplyZhuangResult.iZhuangBaShu = m_iZhuangBaShu;
			TApplyZhuangResult.i64NtWin		= m_i64NtWin;
			TApplyZhuangResult.bXiaZhuang	= m_bXiaZhuang;
			TApplyZhuangResult.byDeskStation= bDeskStation;
			for (auto itr = m_qZhuangList.begin();itr != m_qZhuangList.end();itr++)
			{
				TApplyZhuangResult.byZhuangList[itr - m_qZhuangList.begin()] = *itr;
			}
			if (NULL != m_pUserInfo[m_iNowNtStation])
			{
				TApplyZhuangResult.i64NtMoney = m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money;
			}
			else
			{
				TApplyZhuangResult.i64NtMoney = 0;
			}

			for(int i=0; i<PLAY_COUNT; i++)
			{
				if (NULL == m_pUserInfo[i])
				{
					continue;
				}
				SendGameData(i,&TApplyZhuangResult,sizeof(TApplyZhuangResult),MDM_GM_GAME_NOTIFY, S_C_APPLY_ZHUANG_RESULT,0);
			}
		}
	}
}
/*-----------------------------------------------------------------------------------------*/
//是否在上庄列表中
bool	CServerGameDesk::CheckInZhuangList(BYTE bDeskStation)
{
	return m_qZhuangList.end()!= find(m_qZhuangList.begin(),m_qZhuangList.end(),bDeskStation);
}

//统计机器人上庄数量
int		CServerGameDesk::CountRobortNum()
{
	int iCount = 0;
	for (auto itr = m_qZhuangList.begin();itr != m_qZhuangList.end();itr++)
	{
		if(*itr >= PLAY_COUNT) continue;
		if(nullptr == m_pUserInfo[*itr])continue;
		if (m_pUserInfo[*itr]->m_UserData.isVirtual>0)
		{
			iCount++;
		}
	}
	return iCount;	
}
/*-----------------------------------------------------------------------------------------*/
//收到玩家下注消息
void	CServerGameDesk::OnHandleXiaZhu(BYTE bDeskStation,C_S_UserNote *pUserNoTe)
{
	if (m_bGameStation != GS_NOTE_STATE)
	{
		return;
	}
	//不能下注
	if (false == m_bCanXiaZhu)
	{
		return;
	}
	//无庄不能下注
	if (-1 == m_iNowNtStation || NULL == m_pUserInfo[m_iNowNtStation])
	{
		return;
	}
	if (bDeskStation == m_iNowNtStation)
	{
		return;
	}
	if (pUserNoTe->iChouMaType < 0 || pUserNoTe->iChouMaType > 6 || pUserNoTe->iNoteArea < 0 || pUserNoTe->iNoteArea >= BET_ARES)
	{
		return;
	}


	//机器人区域下注总数限制
	if(m_pUserInfo[bDeskStation]->m_UserData.isVirtual)
	{
		if(m_iRobotRectNoteMax[pUserNoTe->iNoteArea] >0)
		{
			if(m_i64QuYuZhu[pUserNoTe->iNoteArea] - m_i64QuYuZhuTrue[pUserNoTe->iNoteArea] + \
			   m_i64ChouMaValue[pUserNoTe->iChouMaType] >
				m_iRobotRectNoteMax[pUserNoTe->iNoteArea])
			{
				return;
			}
		}
	}

	__int64	i64SumXiaZhu = 0;
	for(int i=0; i<BET_ARES; i++)
	{
		i64SumXiaZhu += m_i64UserXiaZhuData[bDeskStation][i];
	}
	//是否超过了个人总注
	if (m_i64UserMaxNote > 0 && (i64SumXiaZhu + m_i64ChouMaValue[pUserNoTe->iChouMaType]) > m_i64UserMaxNote)
	{
		SendGameData(bDeskStation,MDM_GM_GAME_NOTIFY, S_C_PLAYER_OUTRT,0);
		return;
	}

	if ( (i64SumXiaZhu + m_i64ChouMaValue[pUserNoTe->iChouMaType]) > m_pUserInfo[bDeskStation]->m_UserData.i64Money)
	{
		SendGameData(bDeskStation,MDM_GM_GAME_NOTIFY, S_C_PLAYERMONEY_LOSER,0);
		return;
	}

	////区域下注超过了
	//if (G_i64ChouMaValues[pUserNoTe->iChouMaType] > m_i64AreasMaxZhu[pUserNoTe->iNoteArea])
	//{
	//	return;
	//}

	//这里计算各区域还可以下注多少并且判断是否可以下注成功
	if (false == CountAreasMaxNote(pUserNoTe->iNoteArea,pUserNoTe->iChouMaType))
	{
		SendGameData(bDeskStation,MDM_GM_GAME_NOTIFY, S_C_ALLMONEY_OUTRT,0);
		return;
	}


	m_i64ZhongZhu += m_i64ChouMaValue[pUserNoTe->iChouMaType];
	m_i64QuYuZhu[pUserNoTe->iNoteArea]	+= m_i64ChouMaValue[pUserNoTe->iChouMaType];
	//0庄  3闲
	m_i64UserXiaZhuData[bDeskStation][pUserNoTe->iNoteArea] += m_i64ChouMaValue[pUserNoTe->iChouMaType];

	//真是玩家下注
	if (0 == m_pUserInfo[bDeskStation]->m_UserData.isVirtual)
	{
		m_i64QuYuZhuTrue[pUserNoTe->iNoteArea]	+= m_i64ChouMaValue[pUserNoTe->iChouMaType];
	}

	C_S_UserNoteResult	TUserNoteResult;
	TUserNoteResult.byDeskStation = bDeskStation;
	TUserNoteResult.iArea		= pUserNoTe->iNoteArea;
	TUserNoteResult.i64Money	= m_i64ChouMaValue[pUserNoTe->iChouMaType];
	TUserNoteResult.i64ZhongZhu	= m_i64ZhongZhu;
	memcpy(TUserNoteResult.i64QuYuZhu,m_i64QuYuZhu,sizeof(TUserNoteResult.i64QuYuZhu));
	memcpy(TUserNoteResult.i64QuYuZhuTrue,m_i64QuYuZhuTrue,sizeof(TUserNoteResult.i64QuYuZhuTrue));
	//每个区域还能下多少注
	memcpy(TUserNoteResult.i64AreaMaxZhu,m_i64AreasMaxZhu,sizeof(TUserNoteResult.i64AreaMaxZhu));
	for(int i=0; i<PLAY_COUNT;i++)
	{
		if (NULL == m_pUserInfo[i])
		{
			continue;
		}
		//个人区域下注信息
		memset(TUserNoteResult.i64UserXiaZhuData,0,sizeof(TUserNoteResult.i64UserXiaZhuData));
		memcpy(TUserNoteResult.i64UserXiaZhuData,m_i64UserXiaZhuData[i],sizeof(TUserNoteResult.i64UserXiaZhuData));

		SendGameData(i,&TUserNoteResult,sizeof(TUserNoteResult),MDM_GM_GAME_NOTIFY, S_C_XIA_ZHU_RESULT,0);
	}
}

//计算各区域还可以下注多少
bool	CServerGameDesk::CountAreasMaxNote(int	iNoteArea,int iChouMaType)
{
	__int64			iTmp64QuYuZhu[BET_ARES]; 				
	memcpy(iTmp64QuYuZhu,m_i64QuYuZhu,sizeof(iTmp64QuYuZhu));
	iTmp64QuYuZhu[iNoteArea] += m_i64ChouMaValue[iChouMaType];

	/**< 本把每个区域最大能下的注额*/
	__int64	iTmp64AreasMaxZhu[BET_ARES];  		
	memset(iTmp64AreasMaxZhu,0,sizeof(iTmp64AreasMaxZhu));
	//m_i64PerMaxNotes 就是庄家身上的金币数 游戏开始的时候就已经算好了
	__int64	i64NtMoney = m_i64PerMaxNotes;

	__int64 i64TmpMoney = abs((long)(iTmp64QuYuZhu[GF_XIAN_AREA] - iTmp64QuYuZhu[GF_ZHUANG_AREA]));//庄和闲位之差
	//中间变量
	__int64 i64MaxMoney=0;

	//计算闲家还可以下注多少
	i64MaxMoney = i64NtMoney - iTmp64QuYuZhu[GF_XIAN_AREA] - iTmp64QuYuZhu[GF_X_TIANWANG_AREA]*2 - iTmp64QuYuZhu[GF_X_DUIZI_AREA]*11;
	i64MaxMoney -= (iTmp64QuYuZhu[GF_Z_TIANWANG_AREA]*2 + iTmp64QuYuZhu[GF_Z_DUIZI_AREA]*11 - iTmp64QuYuZhu[GF_ZHUANG_AREA]);
	iTmp64AreasMaxZhu[GF_XIAN_AREA] = i64MaxMoney;
	//闲天王
	i64MaxMoney  = i64NtMoney-i64TmpMoney-iTmp64QuYuZhu[GF_X_TIANWANG_AREA]*2-iTmp64QuYuZhu[GF_X_DUIZI_AREA]*11;
	i64MaxMoney -= (iTmp64QuYuZhu[GF_Z_TIANWANG_AREA]*2 + iTmp64QuYuZhu[GF_Z_DUIZI_AREA]*11 + iTmp64QuYuZhu[GF_HE_AREA]*8 + iTmp64QuYuZhu[GF_T_HE_AREA]*32);
	iTmp64AreasMaxZhu[GF_X_TIANWANG_AREA] = i64MaxMoney;
	//闲对子
	i64MaxMoney  = i64NtMoney - i64TmpMoney-iTmp64QuYuZhu[GF_X_TIANWANG_AREA]*2-iTmp64QuYuZhu[GF_X_DUIZI_AREA]*11;
	i64MaxMoney -= (iTmp64QuYuZhu[GF_Z_TIANWANG_AREA]*2 + iTmp64QuYuZhu[GF_Z_DUIZI_AREA]*11 + iTmp64QuYuZhu[GF_HE_AREA]*8 + iTmp64QuYuZhu[GF_T_HE_AREA]*32);		
	iTmp64AreasMaxZhu[GF_X_DUIZI_AREA] = i64MaxMoney/11;

	//庄家还可以下注多少
	i64MaxMoney  = i64NtMoney - iTmp64QuYuZhu[GF_ZHUANG_AREA] - iTmp64QuYuZhu[GF_Z_TIANWANG_AREA]*2 - iTmp64QuYuZhu[GF_Z_DUIZI_AREA]*11;
	i64MaxMoney -= (iTmp64QuYuZhu[GF_X_TIANWANG_AREA]*2 + iTmp64QuYuZhu[GF_X_DUIZI_AREA]*11 - iTmp64QuYuZhu[GF_XIAN_AREA]);
	iTmp64AreasMaxZhu[GF_ZHUANG_AREA] = i64MaxMoney;
	//庄天王
	i64MaxMoney  = i64NtMoney-i64TmpMoney-iTmp64QuYuZhu[GF_Z_TIANWANG_AREA]*2-iTmp64QuYuZhu[GF_Z_DUIZI_AREA]*11;
	i64MaxMoney -= (iTmp64QuYuZhu[GF_X_TIANWANG_AREA]*2 + iTmp64QuYuZhu[GF_X_DUIZI_AREA]*11 + iTmp64QuYuZhu[GF_HE_AREA]*8 + iTmp64QuYuZhu[GF_T_HE_AREA]*32);
	iTmp64AreasMaxZhu[GF_Z_TIANWANG_AREA] = i64MaxMoney;
	//庄对子
	i64MaxMoney  = i64NtMoney - i64TmpMoney-iTmp64QuYuZhu[GF_Z_TIANWANG_AREA]*2-iTmp64QuYuZhu[GF_Z_DUIZI_AREA]*11;
	i64MaxMoney -= (iTmp64QuYuZhu[GF_X_TIANWANG_AREA]*2 + iTmp64QuYuZhu[GF_X_DUIZI_AREA]*11 + iTmp64QuYuZhu[GF_HE_AREA]*8 + iTmp64QuYuZhu[GF_T_HE_AREA]*32);		
	iTmp64AreasMaxZhu[GF_Z_DUIZI_AREA] = i64MaxMoney/11;
	// 和(返还庄家和闲家的下注数)
	i64MaxMoney  = i64NtMoney - iTmp64QuYuZhu[GF_HE_AREA]*8 - iTmp64QuYuZhu[GF_T_HE_AREA]*32 ; 
	i64MaxMoney -= (iTmp64QuYuZhu[GF_Z_TIANWANG_AREA]*2 + iTmp64QuYuZhu[GF_Z_DUIZI_AREA]*11  +iTmp64QuYuZhu[GF_X_TIANWANG_AREA]*2+ iTmp64QuYuZhu[GF_X_DUIZI_AREA]*11);//加上其他区域的下注
	iTmp64AreasMaxZhu[GF_HE_AREA] = i64MaxMoney/8;
	// 同点和
	i64MaxMoney  = i64NtMoney - iTmp64QuYuZhu[GF_HE_AREA]*8 - iTmp64QuYuZhu[GF_T_HE_AREA]*32;    
	i64MaxMoney -= (iTmp64QuYuZhu[GF_Z_TIANWANG_AREA]*2 + iTmp64QuYuZhu[GF_Z_DUIZI_AREA]*11 + iTmp64QuYuZhu[GF_X_TIANWANG_AREA]*2 + iTmp64QuYuZhu[GF_X_DUIZI_AREA]*11);//加上其他区域的下注
	iTmp64AreasMaxZhu[GF_T_HE_AREA] = i64MaxMoney/32;

	//判断是否有负数的情况
	bool bSuccese = true;
	for(int i=0; i<BET_ARES; i++)
	{
		if (iTmp64AreasMaxZhu[i] < 0  && iNoteArea == i)
		{
			bSuccese = false;
			break;
		}
	}

	//下注成功
	if (bSuccese)
	{
		memcpy(m_i64AreasMaxZhu,iTmp64AreasMaxZhu,sizeof(m_i64AreasMaxZhu));
		return true;
	}

	return false;
}

//超端设置消息
void	CServerGameDesk::OnHandleSuperSet(BYTE bDeskStation,SuperUserSetData *pSuperUserSet)
{
	if (!IsSuperUser(bDeskStation))
	{
		return;
	}
	if (m_bGameStation != GS_NOTE_STATE)
	{
		return;
	}

	memcpy(m_SuperSetData.iSetResult,pSuperUserSet->iSetResult,sizeof(m_SuperSetData.iSetResult));
	m_SuperSetData.bSetSuccese = true;
	

	//发送设置结果消息
	SendGameData(bDeskStation,&m_SuperSetData,sizeof(m_SuperSetData),MDM_GM_GAME_NOTIFY,S_C_SUPER_SET_RESULT,0);
}




///名称：IsPlayGame
///描述：判断是否正在游戏
///@param byDeskStation 位置
///@return true 正在游戏。false 不在游戏
bool CServerGameDesk::IsPlayGame(BYTE bDeskStation)
{
	__int64 llUserNoteAll  = 0l; 

	for (int i = 0 ; i < BET_ARES; i++)
	{
		llUserNoteAll += m_i64UserXiaZhuData[bDeskStation][i]; 
	}

	if (0 == llUserNoteAll && bDeskStation != m_iNowNtStation) //玩家中途进入的
	{
		return false; 
	}
	else
	{
		return 	m_bGameStation >= GS_NOTE_STATE && m_bGameStation < GS_WAIT_NEXT;
	}
}
///名称：UserLeftDesk
///描述：用户离开游戏桌 
///@param byDeskStation 位置 pUserInfo 数据
///@return 
BYTE CServerGameDesk::UserLeftDesk(BYTE bDeskStation, CGameUserInfo * pUserInfo)
{
	m_i64UserWin[bDeskStation] = 0;
	OutputDebugString("dwjtest5::UserLeftDesk-0");
	//清空下注记录和输赢记录
	for(int i=0; i<m_dqUserPlayResult.size(); i++)
	{
		m_dqUserPlayResult.at(i).bHaveXiaZhu[bDeskStation]	= false;
		m_dqUserPlayResult.at(i).iWinMoney[bDeskStation]	= 0;
	}
	OutputDebugString("dwjtest5::UserLeftDesk-1");
	//如果在庄家列表里面 就要更新上庄列表
	if (m_iNowNtStation == bDeskStation || CheckInZhuangList(bDeskStation))
	{
		C_S_ApplyShangZhuang TApplyShangZhuang;
		TApplyShangZhuang.bShangZhuang = false;
		OnHandleShangZhuang(bDeskStation,&TApplyShangZhuang);
	}

	banker.onUserLeaveBanker(bDeskStation,isRobot(bDeskStation));
	banker.onUserLeave(bDeskStation,isRobot(bDeskStation));

	return __super::UserLeftDesk(bDeskStation, pUserInfo);
}


//用户断线离开
bool CServerGameDesk::UserNetCut(BYTE bDeskStation, CGameUserInfo * pLostUserInfo)
{
	//只要一掉线就自动下庄
	C_S_ApplyShangZhuang TApplyShangZhuang;
	TApplyShangZhuang.bShangZhuang = false;
	OnHandleShangZhuang(bDeskStation,&TApplyShangZhuang);
	return __super::UserNetCut(bDeskStation,  pLostUserInfo);
}

BYTE CServerGameDesk::UserSitDesk(MSG_GR_S_UserSit * pUserSit, CGameUserInfo * pUserInfo)
{
	BYTE bFlag =  __super::UserSitDesk(pUserSit, pUserInfo);
	banker.onUserSit(pUserSit->bDeskStation,isRobot(pUserSit->bDeskStation));
	static bool bFirstSitFlag = false;
	if(!bFirstSitFlag)
	{
		bFirstSitFlag = true;
		SetTimer(TIMER_UPDATE_AI_NT,1000);
	}
	return bFlag;
}

bool CServerGameDesk::isRobot(BYTE bDeakNO)
{
	if (bDeakNO >= PLAY_COUNT || NULL == m_pUserInfo[bDeakNO])
	{
		return false;
	}
	return m_pUserInfo[bDeakNO]->m_UserData.isVirtual;
}

//用来改变用户断线条件的函数
//bool	CServerGameDesk::CanNetCut(BYTE bDeskStation)
//{
//	return true;
//}
///用户断线离开
//bool  CServerGameDesk::UserNetCut(BYTE bDeskStation, CGameUserInfo * pLostUserInfo)
//{
//	return __super::UserNetCut(bDeskStation, pLostUserInfo);
//}


///名称：OnGetGameStation
///描述：获取游戏状态 
///@param byDeskStation 位置 uSocketID socket id  bWatchUser 是否允许旁观
///@return 
bool CServerGameDesk::OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser)
{
	if (m_pUserInfo[bDeskStation] == NULL)
	{
		return false;
	}

	if (m_bGameStation >= GS_NOTE_STATE && m_bGameStation <= GS_SHOW_WIN)
	{
		m_pUserInfo[bDeskStation]->m_UserData.bUserState = USER_PLAY_GAME;
	}	
	// 验证超级用户权限
	AuthPermissions(bDeskStation);

	switch (m_bGameStation)
	{
	case GS_WAIT_SETGAME:		//无庄等待状态
		{
			//设置数据发送到客户端
			GameStationBase TGameStation;
			memcpy(TGameStation.i64JettonVal, m_i64ChouMaValue, sizeof(m_i64ChouMaValue));
			TGameStation.iFreeTime		= m_iFreeTime;
			TGameStation.iKaiPaiTime	= m_iKaiPaiTime;
			TGameStation.iXiaZhuTime	= m_iXiaZhuTime;
			TGameStation.iShowWinTime	= m_iShowResult;
			TGameStation.i64ShangZhuangLimit = m_i64ShangZhuangLimit;
			TGameStation.i64UserMaxNote	= m_i64UserMaxNote;

			TGameStation.iNtStation		= m_iNowNtStation;	
			TGameStation.iNtPlayCount	= m_iZhuangBaShu;
			TGameStation.i64NtWinMoney	= m_i64NtWin;
			if (-1 != m_iNowNtStation && NULL != m_pUserInfo[m_iNowNtStation])
			{
				TGameStation.i64NtMoney	= m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money;
			}
			else
			{
				TGameStation.i64NtMoney	= 0;
			}
			//自己的输赢情况
			TGameStation.i64UserWin	= m_i64UserWin[bDeskStation];
			//自己的金币数
			TGameStation.i64MyMoney = m_pUserInfo[bDeskStation]->m_UserData.i64Money;

			//上庄列表
			for (auto itr = m_qZhuangList.begin();itr != m_qZhuangList.end();itr++)
			{
				TGameStation.byZhuangList[itr - m_qZhuangList.begin()] = *itr;
			}

			//路子信息
			for(int i=0;i<m_DqLuziData.size()&&i<MAXCOUNT;i++)
			{
				TGameStation.TLuziData[i] = m_DqLuziData.at(i);
			}

			//发送数据
			SendGameStation(bDeskStation,uSocketID,bWatchUser,&TGameStation,sizeof(TGameStation));
			return true;
		}
	case GS_WAIT_AGREE:			//等待玩家开始状态e
	case GS_WAIT_NEXT:			//等待下一盘游戏开始
		{
			//设置数据发送到客户端
			GameStationBase TGameStation;
			memcpy(TGameStation.i64JettonVal, m_i64ChouMaValue, sizeof(m_i64ChouMaValue));
			TGameStation.iFreeTime		= m_iFreeTime;
			TGameStation.iKaiPaiTime	= m_iKaiPaiTime;
			TGameStation.iXiaZhuTime	= m_iXiaZhuTime;
			TGameStation.iShowWinTime	= m_iShowResult;
			TGameStation.i64ShangZhuangLimit = m_i64ShangZhuangLimit;
			TGameStation.i64UserMaxNote	= m_i64UserMaxNote;

			TGameStation.iNtStation		= m_iNowNtStation;	
			TGameStation.iNtPlayCount	= m_iZhuangBaShu;
			TGameStation.i64NtWinMoney	= m_i64NtWin;
			if (-1 != m_iNowNtStation && NULL != m_pUserInfo[m_iNowNtStation])
			{
				TGameStation.i64NtMoney	= m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money;
			}
			else
			{
				TGameStation.i64NtMoney	= 0;
			}
			//自己的输赢情况
			TGameStation.i64UserWin	= m_i64UserWin[bDeskStation];
			//自己的金币数
			TGameStation.i64MyMoney = m_pUserInfo[bDeskStation]->m_UserData.i64Money;

			//上庄列表
			for (auto itr = m_qZhuangList.begin();itr != m_qZhuangList.end();itr++)
			{
				TGameStation.byZhuangList[itr - m_qZhuangList.begin()] = *itr;
			}
			//路子信息
			for(int i=0;i<m_DqLuziData.size()&&i<MAXCOUNT;i++)
			{
				TGameStation.TLuziData[i] = m_DqLuziData.at(i);
			}

			//发送数据
			SendGameStation(bDeskStation,uSocketID,bWatchUser,&TGameStation,sizeof(TGameStation));
			return true;
		}
	case GS_NOTE_STATE:			//下注状态
		{
			//设置数据发送到客户端
			GameStation_Bet TGameStation;
			memcpy(TGameStation.i64JettonVal, m_i64ChouMaValue, sizeof(m_i64ChouMaValue));
			TGameStation.iFreeTime		= m_iFreeTime;
			TGameStation.iKaiPaiTime	= m_iKaiPaiTime;
			TGameStation.iXiaZhuTime	= m_iXiaZhuTime;
			TGameStation.iShowWinTime	= m_iShowResult;
			TGameStation.iRemainTimeXZ  = m_iXiaZhuTime-m_haveTimeSs>0?m_iXiaZhuTime-m_haveTimeSs:0;
			TGameStation.i64ShangZhuangLimit = m_i64ShangZhuangLimit;
			TGameStation.i64UserMaxNote	= m_i64UserMaxNote;

			TGameStation.iNtStation		= m_iNowNtStation;	
			TGameStation.iNtPlayCount	= m_iZhuangBaShu;
			TGameStation.i64NtWinMoney	= m_i64NtWin;
			TGameStation.i64NtMoney	= m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money;

			//自己的输赢情况
			TGameStation.i64UserWin	= m_i64UserWin[bDeskStation];
			//自己的金币数
			TGameStation.i64MyMoney = m_pUserInfo[bDeskStation]->m_UserData.i64Money;

			//上庄列表
			for (auto itr = m_qZhuangList.begin();itr != m_qZhuangList.end();itr++)
			{
				TGameStation.byZhuangList[itr - m_qZhuangList.begin()] = *itr;
			}
			//路子信息
			for(int i=0;i<m_DqLuziData.size()&&i<MAXCOUNT;i++)
			{
				TGameStation.TLuziData[i] = m_DqLuziData.at(i);
			}
			/**< 本把当前总注额*/
			TGameStation.i64ZhongZhu	= m_i64ZhongZhu;   									
			/**< 本把每个区域下的注额*/
			memcpy(TGameStation.i64QuYuZhu,m_i64QuYuZhu,sizeof(TGameStation.i64QuYuZhu));	
			///真实玩家的下注值
			memcpy(TGameStation.i64QuYuZhuTrue,m_i64QuYuZhuTrue,sizeof(TGameStation.i64QuYuZhuTrue));	
			//玩家区域下注信息
			memcpy(TGameStation.i64UserXiaZhuData,m_i64UserXiaZhuData[bDeskStation],sizeof(TGameStation.i64UserXiaZhuData));	
			//每个区域还能下多少注
			memcpy(TGameStation.i64AreaMaxZhu,m_i64AreasMaxZhu,sizeof(TGameStation.i64AreaMaxZhu));

			//发送数据
			SendGameStation(bDeskStation,uSocketID,bWatchUser,&TGameStation,sizeof(TGameStation));
			return true;
		}
	case GS_SEND_CARD:			//发牌状态
		{
			//设置数据发送到客户端
			GameStation_SendCard TGameStation;
			memcpy(TGameStation.i64JettonVal, m_i64ChouMaValue, sizeof(m_i64ChouMaValue));
			TGameStation.iFreeTime		= m_iFreeTime;
			TGameStation.iKaiPaiTime	= m_iKaiPaiTime;
			TGameStation.iXiaZhuTime	= m_iXiaZhuTime;
			TGameStation.iShowWinTime	= m_iShowResult;
			TGameStation.i64ShangZhuangLimit = m_i64ShangZhuangLimit;
			TGameStation.i64UserMaxNote	= m_i64UserMaxNote;
			TGameStation.RemainTimeKP	=  m_iKaiPaiTime-m_haveTimeSs>0?m_iKaiPaiTime-m_haveTimeSs:0;

			TGameStation.iNtStation		= m_iNowNtStation;	
			TGameStation.iNtPlayCount	= m_iZhuangBaShu;
			TGameStation.i64NtWinMoney	= m_i64NtWin;
			TGameStation.i64NtMoney	= m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money;

			//自己的输赢情况
			TGameStation.i64UserWin	= m_i64UserWin[bDeskStation];
			//自己的金币数
			TGameStation.i64MyMoney = m_pUserInfo[bDeskStation]->m_UserData.i64Money;
			
			//上庄列表
			for (auto itr = m_qZhuangList.begin();itr != m_qZhuangList.end();itr++)
			{
				TGameStation.byZhuangList[itr - m_qZhuangList.begin()] = *itr;
			}
			//路子信息
			for(int i=0;i<m_DqLuziData.size()&&i<MAXCOUNT;i++)
			{
				TGameStation.TLuziData[i] = m_DqLuziData.at(i);
			}
			/**< 本把当前总注额*/
			TGameStation.i64ZhongZhu	= m_i64ZhongZhu;   									
			/**< 本把每个区域下的注额*/
			memcpy(TGameStation.i64QuYuZhu,m_i64QuYuZhu,sizeof(TGameStation.i64QuYuZhu));	
			///真实玩家的下注值
			memcpy(TGameStation.i64QuYuZhuTrue,m_i64QuYuZhuTrue,sizeof(TGameStation.i64QuYuZhuTrue));	
			//玩家区域下注信息
			memcpy(TGameStation.i64UserXiaZhuData,m_i64UserXiaZhuData[bDeskStation],sizeof(TGameStation.i64UserXiaZhuData));	
			//每个区域还能下多少注
			memcpy(TGameStation.i64AreaMaxZhu,m_i64AreasMaxZhu,sizeof(TGameStation.i64AreaMaxZhu));

			//庄闲的牌，0为庄，1为闲
			memcpy(TGameStation.byUserCard,m_byUserCard,sizeof(TGameStation.byUserCard));
			//庄家牌型,元素0前两张牌的值，元素1总牌值，元素2天王，元素3对子，元素4和
			memcpy(TGameStation.iZPaiXing,m_iZPaiXing,sizeof(TGameStation.iZPaiXing));
			//闲家牌型,元素0前两张牌的值，元素1总牌值，元素2天王，元素3对子，元素4和
			memcpy(TGameStation.iXPaiXing,m_iXPaiXing,sizeof(TGameStation.iXPaiXing));
	
			//发送数据
			SendGameStation(bDeskStation,uSocketID,bWatchUser,&TGameStation,sizeof(TGameStation));
			return true;
		}
	case GS_SHOW_WIN:			//显示中奖区域
		{
			//设置数据发送到客户端
			GameStation_ShowWin TGameStation;
			memcpy(TGameStation.i64JettonVal, m_i64ChouMaValue, sizeof(m_i64ChouMaValue));
			TGameStation.iFreeTime		= m_iFreeTime;
			TGameStation.iKaiPaiTime	= m_iKaiPaiTime;
			TGameStation.iXiaZhuTime	= m_iXiaZhuTime;
			TGameStation.iShowWinTime	= m_iShowResult;
			TGameStation.i64ShangZhuangLimit = m_i64ShangZhuangLimit;
			TGameStation.i64UserMaxNote	= m_i64UserMaxNote;

			TGameStation.iNtStation		= m_iNowNtStation;	
			TGameStation.iNtPlayCount	= m_iZhuangBaShu;
			TGameStation.i64NtWinMoney	= m_i64NtWin;
			TGameStation.i64NtMoney	= m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money;

			TGameStation.RemainTimeZJ = m_iShowResult-m_haveTimeSs>0?m_iShowResult-m_haveTimeSs:0;;
			//自己的输赢情况
			TGameStation.i64UserWin	= m_i64UserWin[bDeskStation];
			//自己的金币数
			TGameStation.i64MyMoney = m_pUserInfo[bDeskStation]->m_UserData.i64Money;

			//上庄列表
			for (auto itr = m_qZhuangList.begin();itr != m_qZhuangList.end();itr++)
			{
				TGameStation.byZhuangList[itr - m_qZhuangList.begin()] = *itr;
			}
			//路子信息
			for(int i=0;i<m_DqLuziData.size()&&i<MAXCOUNT;i++)
			{
				TGameStation.TLuziData[i] = m_DqLuziData.at(i);
			}
			/**< 本把当前总注额*/
			TGameStation.i64ZhongZhu	= m_i64ZhongZhu;   									
			/**< 本把每个区域下的注额*/
			memcpy(TGameStation.i64QuYuZhu,m_i64QuYuZhu,sizeof(TGameStation.i64QuYuZhu));	
			///真实玩家的下注值
			memcpy(TGameStation.i64QuYuZhuTrue,m_i64QuYuZhuTrue,sizeof(TGameStation.i64QuYuZhuTrue));	
			//玩家区域下注信息
			memcpy(TGameStation.i64UserXiaZhuData,m_i64UserXiaZhuData[bDeskStation],sizeof(TGameStation.i64UserXiaZhuData));	
			//每个区域还能下多少注
			memcpy(TGameStation.i64AreaMaxZhu,m_i64AreasMaxZhu,sizeof(TGameStation.i64AreaMaxZhu));

			//游戏的赢钱区域 0闲，1闲天王，2闲对子，3庄，4庄天王，5庄对子，6和，7同点和	
			memcpy(TGameStation.iWinQuYu,m_iWinQuYu,sizeof(TGameStation.iWinQuYu));

			//发送数据
			SendGameStation(bDeskStation,uSocketID,bWatchUser,&TGameStation,sizeof(TGameStation));
			return true;
		}
	default:
		break;
	}

	return true;
}

///名称：ReSetGameState
///描述：重置游戏状态 
///@param bLastStation 状态
///@return 
bool CServerGameDesk::ReSetGameState(BYTE bLastStation)
{
	return true;
}




/// 清除所有定时器
void  CServerGameDesk::KillAllTimer(void)
{

}

///名称：OnTimer
///描述：定时器消息 
///@param uTimerID 定时器id
///@return 
bool CServerGameDesk::OnTimer(UINT uTimerID)
{
	switch (uTimerID)
	{
	case ID_TIMER_SEND_CARD:	//下注时间到
		{
			KillTimer(ID_TIMER_SEND_CARD);
			KillTimer(ID_TIMER_REMAINTIME_GAME);
			m_haveTimeSs = 0;
			//下注结束 开始发牌
			OnSendAllCard();
			return true;
		}
	case ID_TIMER_SEND_CARD_FINISH:	//发牌结束
		{
			KillTimer(ID_TIMER_SEND_CARD_FINISH);
			KillTimer(ID_TIMER_REMAINTIME_GAME);
			m_haveTimeSs = 0;
			//显示中奖区域
			OnSendWinAreas();
			return true;
		}
	case ID_TIMER_SHOW_WIN_FINISH:	//中奖区域显示结束
		{
			KillTimer(ID_TIMER_SHOW_WIN_FINISH);
			KillTimer(ID_TIMER_REMAINTIME_GAME);
			m_haveTimeSs = 0;
			//游戏结束
			GameFinish(0,GF_NORMAL);
			return true;
		}
	case ID_TIMER_GAME_NEW_GAME:	//新一轮
		{
			KillTimer(ID_TIMER_GAME_NEW_GAME);
			KillTimer(ID_TIMER_REMAINTIME_GAME);
			m_haveTimeSs = 0;
			StartGame();
			return true;
		}
	case ID_TIMER_REMAINTIME_GAME:	//倒计时定时器
		{
			m_haveTimeSs ++;
			return true;
		}
	case TIMER_UPDATE_AI_NT:
		{
			NTDecision();
			return true;
		}
	}
	return __super::OnTimer(uTimerID);	
}








	









/*-----------------------------------------------------------------------------------------*/
///名称：StartGame
///描述：通过全部举手的方式开始游戏
///@param 
///@return 
bool	CServerGameDesk::StartGame()
{
	ServerInit();
	//整理庄家列表
	CleanZhuangList();

	//庄家不存在 换庄家
	if (m_iNowNtStation != -1 && NULL == m_pUserInfo[m_iNowNtStation])
	{
		m_iNowNtStation = -1;
		m_iZhuangBaShu	= 0;		//初始化庄家的局数
		m_i64NtWin		= 0;		//初始化庄家赢的金币
		m_bXiaZhuang	= false;	//当前庄家申请下庄
	}
	
	//庄家坐庄次数到了该换庄了
	if(m_iMaxZhuang == m_iZhuangBaShu && m_iMaxZhuang > 0)
	{
		m_iNowNtStation	=-1;
		m_iZhuangBaShu	= 0;		//初始化庄家的局数
		m_i64NtWin		= 0;		//初始化庄家赢的金币
		m_bXiaZhuang	= false;	//当前庄家申请下庄
	}
	//金币不足换庄家
	if(-1!=m_iNowNtStation && m_pUserInfo[m_iNowNtStation])
	{
		if((m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money)<m_i64ShangZhuangLimit)
		{
			m_iNowNtStation	=-1;
			m_iZhuangBaShu	= 0;		//初始化庄家的局数
			m_i64NtWin		= 0;		//初始化庄家赢的金币
			m_bXiaZhuang	= false;	//当前庄家申请下庄
		}
	}
	//如果庄家申请了下庄
	if (m_bXiaZhuang)
	{
		m_iNowNtStation	=-1;
		m_iZhuangBaShu	= 0;		//初始化庄家的局数
		m_i64NtWin		= 0;		//初始化庄家赢的金币
		m_bXiaZhuang	= false;	//当前庄家申请下庄
	}


	//轮换庄家
	if (-1 == m_iNowNtStation)
	{
		ChangeNT();
	}

	//庄家存在 才开始游戏
	if(-1 != m_iNowNtStation && m_pUserInfo[m_iNowNtStation])
	{
		GameBegin(ALL_ARGEE);//游戏开始
	}
	else
	{
		NoNtWait();
	}

	return true;
}

void	CServerGameDesk::NoNtWait()
{
	m_bGameStation = GS_WAIT_SETGAME;
	m_qZhuangList.clear();

	banker.m_PreBankerReal.clear();
	banker.m_PreBankerRobots.clear();

	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if(m_pUserInfo[i] && isRobot(i))
		{
			banker.m_NomalPlayerRobots.insert(i);
		}
	}


	m_iNowNtStation = -1;
	m_iZhuangBaShu	= 0;		//初始化庄家的局数
	m_i64NtWin		= 0;		//初始化庄家赢的金币
	m_bXiaZhuang	= false;	//当前庄家申请下庄
	//发送消息给客户端 告诉客户端 无庄等待
	S_C_NoNtStation	TNoNtStation;
	TNoNtStation.iNowNtStation	= m_iNowNtStation;
	TNoNtStation.i64NtMoney		= 0;
	TNoNtStation.i64NtWinMoney	= m_i64NtWin;		//庄家的成绩
	TNoNtStation.iNtPlayCount	= m_iZhuangBaShu;

	for (int i=0;i<PLAY_COUNT; ++i)
	{
		if (NULL == m_pUserInfo[i])
		{
			continue;
		}	
		SendGameData(i,&TNoNtStation,sizeof(TNoNtStation),MDM_GM_GAME_NOTIFY, S_C_NO_NT_WAITE,0);
	}	
}
///名称：ServerInit
///描述：初始化服务器数据数据
///@param 
///@return 
void CServerGameDesk::ServerInit()
{			
	memset(m_i64Max,0,sizeof(m_i64Max));/**< 本把每个区域控制的最大下注额*/
	m_i64PerMaxNotes = 0;			/// 本局最大下注总注
	m_i64ZhongZhu	 = 0;   					/**< 本把当前总注额*/
	memset(m_i64QuYuZhu,0,sizeof(m_i64QuYuZhu));/**< 本把每个区域下的注额*/
	memset(m_i64QuYuZhuTrue,0,sizeof(m_i64QuYuZhuTrue));///真实玩家的下注值
	memset(m_i64UserXiaZhuData,0,sizeof(m_i64UserXiaZhuData));
	memset(m_i64UserAreasFen,0,sizeof(m_i64UserAreasFen));/// 各个玩家本局的得分
	memset(m_i64UserFen,0,sizeof(m_i64UserFen));
	memset(m_i64UserMoney,0,sizeof(m_i64UserMoney));
	m_bCanXiaZhu = false;				//是否能下注

	m_bThreeCard	=false;						/**< 庄或闲拿了3张牌*/
	memset(m_iTotalCard,255,sizeof(m_iTotalCard));	//总的牌
	memset(m_byUserCard,0,sizeof(m_byUserCard));	//庄闲的牌数据0-闲家 1-庄家
	memset(m_iZPaiXing,0,sizeof(m_iZPaiXing));	/**< 庄家牌型,元素0前两张牌的值，元素1总牌值，元素2天王，元素3对子，元素4和*/
	memset(m_iXPaiXing,0,sizeof(m_iXPaiXing));	/**< 闲家牌型*/
	memset(m_iWinQuYu,0,sizeof(m_iWinQuYu));	/**< 游戏的赢钱区域 0闲，1闲天王，2闲对子，3庄，4庄天王，5庄对子，6和，7同点和*/
	memset(m_i64AreasMaxZhu,0,sizeof(m_i64AreasMaxZhu));	/**< 本把每个区域最大能下的注额*/

	memset(&m_SuperSetData,0,sizeof(m_SuperSetData));
	m_SuperSetData.bSetSuccese = false;
}


/*------------------------------------------------------------------------*/
///庄家下庄后获取下一个庄家列表中的庄家
bool	CServerGameDesk::ChangeNT()
{
	for (auto itr = m_qZhuangList.begin();itr != m_qZhuangList.end();itr++)
	{
		if(*itr >= PLAY_COUNT)continue;
		if(nullptr == m_pUserInfo[*itr])continue;
		m_iNowNtStation = *itr;
		m_iZhuangBaShu	= 0;		//初始化庄家的局数
		m_i64NtWin		= 0;		//初始化庄家赢的金币
		m_bXiaZhuang	= false;	//当前庄家申请下庄
		banker.onUserLeaveBanker(*itr,isRobot(*itr));
		m_qZhuangList.erase(itr);
		
		break;
	}

	return false;
}


//------------------------------------------------------------------------------------------------------------------------
//从配置文件中读取机器人已经赢了多少钱，因为在每盘结算的时候都会将机器人赢钱结算写到配置文件中。 
//这里在每盘开始的时候又读取下，因为如果玩家修改了那么就机器人吞钱了.
void	CServerGameDesk::GetAIContrlSetFromIni(int iRoomID)
{
	CString s = CINIFile::GetAppPath ();/////本地路径
	CINIFile f(s +SKIN_FOLDER  + _T("_s.ini"));
	CString szSec = TEXT("game");
	//统一房间
	//szSec.Format("%s_%d",SKIN_FOLDER,iRoomID);
	///机器人输赢自动控制
	m_bAIWinAndLostAutoCtrl = f.GetKeyVal(szSec,"AIWinAndLostAutoCtrl",1);		///是否开启机器人输赢自动控制
	m_i64AIWantWinMoneyA1	= f.GetKeyVal(szSec,"AIWantWinMoneyA1 ",100000);		/**<机器人赢钱区域1  */
	m_i64AIWantWinMoneyA2	= f.GetKeyVal(szSec,"AIWantWinMoneyA2 ",1000000);		/**<机器人赢钱区域2  */
	m_i64AIWantWinMoneyA3	= f.GetKeyVal(szSec,"AIWantWinMoneyA3 ",10000000);		/**<机器人赢钱区域3  */
	m_iAIWinLuckyAtA1	= f.GetKeyVal(szSec,"AIWinLuckyAtA1 ",90);				/**<机器人在区域1赢钱的概率  */
	m_iAIWinLuckyAtA2	= f.GetKeyVal(szSec,"AIWinLuckyAtA2 ",70);				/**<机器人输赢控制：机器人在区域2赢钱的概率  */
	m_iAIWinLuckyAtA3	= f.GetKeyVal(szSec,"AIWinLuckyAtA3 ",50);				/**<机器人输赢控制：机器人在区域3赢钱的概率  */
	m_iAIWinLuckyAtA4	= f.GetKeyVal(szSec,"AIWinLuckyAtA4 ",30);				/**<机器人输赢控制：机器人在区域4赢钱的概率  */
	m_i64AIHaveWinMoney	= f.GetKeyVal(szSec,"ReSetAIHaveWinMoney ",0);			/**<机器人输赢控制：直接从配置文件中读取机器人已经赢钱的数目  */
}

///描述：获取每个区域最大下注
void	CServerGameDesk::GetQuYuCanNote()
{
	__int64 money = 0;
	for(int i=0;i<8;i++)
	{
		money += m_i64AreasMaxZhu[i];  	/**< 本把每个区域最大能下的注额 */
	}
	if(m_iNowNtStation>=0 && m_iNowNtStation <PLAY_COUNT && NULL != m_pUserInfo[m_iNowNtStation]  && money<=0)
	{
		m_i64AreasMaxZhu[0]=m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money;			//本把每个区域最大能下的注额
		m_i64AreasMaxZhu[1]=m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money/2;		//本把每个区域最大能下的注额
		m_i64AreasMaxZhu[2]=m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money/11;		//本把每个区域最大能下的注额
		m_i64AreasMaxZhu[3]=m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money;			//本把每个区域最大能下的注额
		m_i64AreasMaxZhu[4]=m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money/2;		//本把每个区域最大能下的注额
		m_i64AreasMaxZhu[5]=m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money/11;		//本把每个区域最大能下的注额
		m_i64AreasMaxZhu[6]=m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money/8;		//本把每个区域最大能下的注额
		m_i64AreasMaxZhu[7]=m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money/32;		//本把每个区域最大能下的注额
	}
}


/*----------------------------------------------------------------------------*/
//下注结束 开始发牌
void CServerGameDesk::OnSendAllCard()
{
	//发送开牌消息			
	m_bGameStation = GS_SEND_CARD;    
	//不能下注了
	m_bCanXiaZhu = false;
	//发牌
	SendCard();	
	//超端是否有设置  
	if (m_SuperSetData.bSetSuccese)
	{
		//超端设置去换牌
		OnSuperSetChange();
	}
	else if (m_bAIWinAndLostAutoCtrl)	//奖池是否有控制
	{
		AiWinAutoCtrl();
	}


	S_C_SendCard TSendCard;
	//牌数据
	TSendCard.byUserCard[0][0] = m_byUserCard[0][0];
	TSendCard.byUserCard[0][1] = m_byUserCard[0][1];
	TSendCard.byUserCard[0][2] = m_byUserCard[0][2];

	TSendCard.byUserCard[1][0] = m_byUserCard[1][0];
	TSendCard.byUserCard[1][1] = m_byUserCard[1][1];
	TSendCard.byUserCard[1][2] = m_byUserCard[1][2];


	/**< 庄家牌型,元素0前两张牌的值，元素1总牌值，元素2天王，元素3对子，元素4和*/
	for(int i=0;i<5;i++)
	{
		TSendCard.iZPaiXing[i] =  m_iZPaiXing[i];
		TSendCard.iXPaiXing[i] =  m_iXPaiXing[i];
	}
	//获取赢牌区域
	for(int i=0;i<BET_ARES;i++)
	{
		//游戏的赢钱区域 0闲，1闲天王，2闲对子，3庄，4庄天王，5庄对子，6和，7同点和
		TSendCard.iWinQuYu[i] = m_iWinQuYu[i];			
	}
	for (int i=0;i<PLAY_COUNT; i++)
	{
		if (NULL != m_pUserInfo[i])
		{
			SendGameData(i, &TSendCard, sizeof(TSendCard), MDM_GM_GAME_NOTIFY, S_C_SEND_CARD, 0);//发送开牌消息
			SetTimer(ID_TIMER_REMAINTIME_GAME,1000);
		}
	}

	//发牌结束计时器
	SetTimer(ID_TIMER_SEND_CARD_FINISH,(m_iKaiPaiTime)*1000);
}

///名称：SendCard
///描述：发牌
///@param 
///@return 
void CServerGameDesk::SendCard(int iIndex)
{
	memset(m_iWinQuYu,0,sizeof(m_iWinQuYu));
	ZeroMemory(m_iZPaiXing,sizeof(m_iZPaiXing));
	ZeroMemory(m_iXPaiXing,sizeof(m_iXPaiXing));
	m_bThreeCard = false;
	//分发扑克
	memset(m_byUserCard,0,sizeof(m_byUserCard));
	memset(m_iTotalCard,255,sizeof(m_iTotalCard));
	m_Logic.RandCard(m_iTotalCard,52,iIndex);


	for (int i=0; i<2; i++)
	{
		for (int j=0; j<2;j++)
		{
			GetACardValue(m_byUserCard[i][j]);
		}		
	}

	//m_byUserCard[0][0] = 0x08;
	//m_byUserCard[0][1] = 0x06;
	//m_byUserCard[1][0] = 0x13;
	//m_byUserCard[1][1] = 0x11;

	//计算牌值
	CountPaiZhi(2);
	if(GetTheThird())//获取第三张牌
	{

// 		m_byUserCard[0][2] = 0x09;
// 		m_byUserCard[1][2] = 0x19;
		//有第三张牌就重新计算牌值				
		m_bThreeCard=true;
		CountPaiZhi(3);//计算总牌值		
	}

	//获取中奖的区域
	GetWinAres();
}

//获取一张牌
void	CServerGameDesk::GetACardValue(BYTE &byCard)
{
	for(int i=0; i<52; i++)
	{
		if (m_iTotalCard[i] != 255)
		{
			byCard = m_iTotalCard[i];
			m_iTotalCard[i] = 255;
			break;
		}
	}
}


///名称：CountPaiZhi
///描述：计算牌值,前conut张的总牌值
///@param count 前几张的牌值 2 前两张，3 就是总牌值
///@return 
void	CServerGameDesk::CountPaiZhi(int count)
{	
	int zhuang=0;//庄牌点
	int xian=0;  //闲牌点

	if(count==2)//前两张牌的值
	{
		int value=0,value1=0;
		//第一张牌
		value = m_Logic.GetCardNum(m_byUserCard[1][0]);//庄
		value1= m_Logic.GetCardNum(m_byUserCard[0][0]);//闲
		if(value>=10)
		{
			value=0;
		}
		zhuang = value;
		if(value1>=10)
		{
			value1=0;
		}
		xian = value1;

		//第二张牌
		value = m_Logic.GetCardNum(m_byUserCard[1][1]);//庄
		value1= m_Logic.GetCardNum(m_byUserCard[0][1]);//闲
		if(value>=10)
		{
			value=0;
		}
		zhuang += value;
		if(value1>=10)
		{
			value1=0;
		}
		xian += value1;

		if(zhuang>=10)
		{
			zhuang%=10;
		}
		if(xian>=10)
		{
			xian%=10;
		}
		m_iZPaiXing[0]=zhuang; //前两张牌的牌点
		m_iXPaiXing[0]=xian;   //

	}
	else if(count==3)//三张牌的总牌点
	{
		int value=0,value1=0;
		value = m_Logic.GetCardNum(m_byUserCard[1][2]);//庄
		value1= m_Logic.GetCardNum(m_byUserCard[0][2]);//闲

		if(value>=10)
		{
			value=0;
		}
		if(value1>=10)
		{
			value1=0;
		}

		zhuang = value + m_iZPaiXing[0];	//庄牌点
		xian = value1 + m_iXPaiXing[0];		//闲牌点

		if(zhuang>=10)
		{
			zhuang%=10;
		}
		if(xian>=10)
		{
			xian%=10;
		}
		m_iZPaiXing[1]=zhuang;
		m_iXPaiXing[1]=xian;
	}

	if(zhuang>=8)//庄天王
	{
		m_iZPaiXing[2]=zhuang;
		if(zhuang==xian)//天王同点
		{
			m_iZPaiXing[4]=zhuang;
		}
	}

	if(xian>=8)//闲天王
	{
		m_iXPaiXing[2]=xian;
		if(zhuang==xian)//天王同点
		{
			m_iXPaiXing[4]=xian;
		}
	}

	if(m_Logic.GetCardNum(m_byUserCard[1][0])==m_Logic.GetCardNum(m_byUserCard[1][1]))//庄对子
	{
		m_iZPaiXing[3]=zhuang;
		if (zhuang == 0)		//主要是用于控制当总牌值为0时，前两张牌相同仍然是对子
		{
			m_iZPaiXing[3] = 1;
		}
	}
	if(m_Logic.GetCardNum(m_byUserCard[0][0])==m_Logic.GetCardNum(m_byUserCard[0][1]))//闲对子
	{
		m_iXPaiXing[3]=xian;
		if (xian == 0)			//主要是用于控制当总牌值为0时，前两张牌相同仍然是对子
		{
			m_iXPaiXing[3] = 1;
		}
	}
}



///名称：GetTheThird
///描述：获取第三张牌
///@param 
///@return true有第三张牌  false 没有第三张牌
bool	CServerGameDesk::GetTheThird()
{
	if(m_iZPaiXing[2] > 0 || m_iXPaiXing[2] > 0)//庄闲任意一家天王都不再抓牌
		return false;

	if(m_iXPaiXing[0] >= 6)//闲家不用抓牌
	{
		if(m_iZPaiXing[0] <= 5)//庄家5点以下要拿牌
		{
			GetACardValue(m_byUserCard[1][2]);

			return true;
		}
	}
	else//闲家要拿一张牌
	{
		GetACardValue(m_byUserCard[0][2]);

		if(IsGetTheThird())//根据闲家的第三张牌值确定庄家是否要拿牌
		{
			GetACardValue(m_byUserCard[1][2]);	
		}
		return true;
	}
	return false;
}

///名称：IsGetTheThird
///描述：闲家拿了第三张牌情况下，庄家是要拿牌
///@param 
///@return true拿  false 不拿
bool	CServerGameDesk::IsGetTheThird()
{
	int num=0;
	num=m_Logic.GetCardNum(m_byUserCard[0][2]);//闲家的第三张牌面值	
	if(num>=10)
	{
		num=0;
	}
	switch(num)
	{
	case 0:
	case 1:
		{
			if(m_iZPaiXing[0]<=3)//庄家牌在3点以下，拿牌
				return true;
		}
		break;
	case 2:
	case 3:
		{
			if(m_iZPaiXing[0]<=4)//庄家牌在4点以下，拿牌
				return true;
		}
		break;
	case 4:
	case 5:
		{
			if(m_iZPaiXing[0]<=5)//庄家牌在5点以下，拿牌
				return true;
		}
		break;
	case 6:
	case 7:
		{
			if(m_iZPaiXing[0]<=6)//庄家牌在6点以下，拿牌
				return true;
		}
		break;
	case 8:
		{
			if(m_iZPaiXing[0]<=2)//庄家牌在2点以下，拿牌
				return true;
		}
		break;
	case 9:
		{
			if(m_iZPaiXing[0]<=3)//庄家牌在3点以下，拿牌
				return true;
		}
		break;
	default:
		break;
	}
	return false;
}


/*--------------------------------------------------------------------------*/
///获取赢钱的区域
void	CServerGameDesk::GetWinAres()
{
	memset(&m_iWinQuYu, 0, sizeof(m_iWinQuYu));	//游戏的赢钱区域 0闲，1闲天王，2闲对子，3庄，4庄天王，5庄对子，6和，7同点和
	if(!m_bThreeCard)//都只拿了两张牌,前两张牌值就是总牌点值
	{
		m_iZPaiXing[1] = m_iZPaiXing[0];
		m_iXPaiXing[1] = m_iXPaiXing[0];
	}

	if (m_iXPaiXing[1] == m_iZPaiXing[1])
	{
		m_iWinQuYu[6] = 9;	//和
		m_iWinQuYu[3] = 0;
		m_iWinQuYu[0] = 0;
		if (m_iZPaiXing[2]>0)	
		{
			m_iWinQuYu[4] = 3;	
		}
		if (m_iZPaiXing[3]>0)
		{
			m_iWinQuYu[5] = 12;	
		}

		if (m_iXPaiXing[2]>0)	
		{
			m_iWinQuYu[1] = 3;	
		}
		if (m_iXPaiXing[3]>0)
		{
			m_iWinQuYu[2] = 12;	
		}
		//检测是否同点和
		if (CheckTongDian(3))
		{
			m_iWinQuYu[7] = 33;
		}		
	}
	else if (m_iZPaiXing[1] > m_iXPaiXing[1])
	{
		m_iWinQuYu[3] = 2;
		if (m_iZPaiXing[2]>0)	
		{
			m_iWinQuYu[4] = 3;	
		}
		if (m_iZPaiXing[3]>0)
		{
			m_iWinQuYu[5] = 12;	
		}

		if (m_iXPaiXing[2]>0)	
		{
			m_iWinQuYu[1] = 3;	
		}
		if (m_iXPaiXing[3]>0)
		{
			m_iWinQuYu[2] = 12;	
		}
	}
	else if (m_iZPaiXing[1] < m_iXPaiXing[1])
	{
		m_iWinQuYu[0] = 2;
		if (m_iZPaiXing[2]>0)	
		{
			m_iWinQuYu[4] = 3;	
		}
		if (m_iZPaiXing[3]>0)
		{
			m_iWinQuYu[5] = 12;	
		}

		if (m_iXPaiXing[2]>0)	
		{
			m_iWinQuYu[1] = 3;	
		}
		if (m_iXPaiXing[3]>0)
		{
			m_iWinQuYu[2] = 12;	
		}
	}
}


/*---------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------*/
///名称：CheckTongDian
///描述：同点检测
///@param num,检测前几张牌
///@return true 同点，false 不是同点
bool CServerGameDesk::CheckTongDian(int num)
{
	int zhuang[3];
	int xian[3];
	int temp=0;
	for(int i=0;i<3;i++)
	{
		zhuang[i]= m_Logic.GetCardNum(m_byUserCard[1][i]);
		xian[i]  = m_Logic.GetCardNum(m_byUserCard[0][i]);
	}
	for(int i=0;i<2;i++)//排序
	{
		for(int j=i+1;j<3;j++)
		{
			if(zhuang[i]<zhuang[j])
			{
				temp = zhuang[i];
				zhuang[i]=zhuang[j];
				zhuang[j]=temp;
			}
			if(xian[i]<xian[j])
			{
				temp = xian[i];
				xian[i]=xian[j];
				xian[j]=temp;
			}
		}
	}

	for(int i=0;i<num;i++)
	{
		if(zhuang[i] != xian[i])
		{
			return false;
		}
	}
	return true;
}

//超端设置去换牌
void	CServerGameDesk::OnSuperSetChange()
{
	//先判断原本的牌是否符合超端设定
	bool bNeedReSendCard = false;		//是否需要重新发牌
	for (int i=0; i<BET_ARES; i++)
	{
		////如果设定的结果=0 而开奖区域不等于0 说明 与超端设定的不符
		//if (0 == m_SuperSetData.iSetResult[i] && 0 != m_iWinQuYu[i] )
		//{
		//	bNeedReSendCard = true;
		//	break;
		//}
		//如果设定的结果＞0 而开奖结果=0 说明与超端设定的不符
		if (m_SuperSetData.iSetResult[i] > 0 && 0 == m_iWinQuYu[i])
		{
			bNeedReSendCard = true;
			break;
		}
	}

	int iRandIndex = 0;
	while (bNeedReSendCard)
	{
		iRandIndex++;
		//发牌
		SendCard(iRandIndex);
		//判断是否需要重新发牌
		bNeedReSendCard = false;

		for (int i=0; i<BET_ARES; i++)
		{
			////如果设定的结果=0 而开奖区域不等于0 说明 与超端设定的不符
			//if (0 == m_SuperSetData.iSetResult[i] && 0 != m_iWinQuYu[i] )
			//{
			//	bNeedReSendCard = true;
			//	break;
			//}
			//如果设定的结果＞0 而开奖结果=0 说明与超端设定的不符
			if (m_SuperSetData.iSetResult[i] > 0 && 0 == m_iWinQuYu[i])
			{
				bNeedReSendCard = true;
				break;
			}
		}
	}
}
//-------------------------------------------------------------------------------------------------------------------
///名称：IAWinAutoCtrl
///描述：机器人输赢控制：机器人根据设定的区域控制一定的输赢概率，让机器人输赢区域稳定
void	 CServerGameDesk::AiWinAutoCtrl()
{
	///如果没有超级客户端控制并且开启了自动控制，则进行机器人输赢自动控制
	if(!m_bAIWinAndLostAutoCtrl)
	{
		return;
	}
	//是否全是机器人
	bool bHaveUser = false;
	for (int i=0; i<PLAY_COUNT; i++)
	{
		if (NULL == m_pUserInfo[i])
		{
			continue;
		}
		if (0 == m_pUserInfo[i]->m_UserData.isVirtual )
		{
			bHaveUser = true;
			break;
		}
	}
	//全是机器人就不去控制了
	if (!bHaveUser)
	{
		return;
	}
	bool bAIWin = false;


	srand((unsigned)GetTickCount());
	int iResult = rand()%100;

	if (0 >=m_i64AIHaveWinMoney )
	{
		//机器人赢的钱少于0 机器人必赢
		bAIWin = true;		
	}
	else if((0 < m_i64AIHaveWinMoney) && (m_i64AIHaveWinMoney < m_i64AIWantWinMoneyA1))
	{
		// 机器人的赢钱在0-A1区域 并且概率符合m_iAIWinLuckyAtA1 机器人要赢
		if (iResult <=m_iAIWinLuckyAtA1)
		{
			bAIWin = true;		
		}
	}
	else if ((m_i64AIWantWinMoneyA1 <= m_i64AIHaveWinMoney)  && (m_i64AIHaveWinMoney <m_i64AIWantWinMoneyA2))
	{
		// 机器人的赢钱在A1-A2区域 并且概率符合m_iAIWinLuckyAtA2 机器人要赢
		if (iResult <=m_iAIWinLuckyAtA2)
		{
			bAIWin = true;			
		}
	}
	else if ((m_i64AIWantWinMoneyA2 <= m_i64AIHaveWinMoney)  && (m_i64AIHaveWinMoney <m_i64AIWantWinMoneyA3))
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
		if (iResult <=m_iAIWinLuckyAtA4)
		{
			bAIWin = true;
		}
	}

	if(bAIWin)
	{
		OutputDebugStringA("chenlog-----------机器人赢钱控制----------0");
		//机器人要赢钱
		if(CountAIWinMoney()<0)
		{
			for(int i=0; i<5000; i++)
			{
				SendCard(i);
				if(CountAIWinMoney()>=0)
				{
					OutputDebugStringA("chenlog-----------机器人赢钱控制----------1");
					break;
				}
			}
		}
	}
	else
	{
		CString str;
		OutputDebugStringA("chenlog-----------机器人输钱控制---------0");
		///机器人要输钱//本来开奖赢钱大于0或者导致机器人赢钱为负数，就要重新发牌
		if(CountAIWinMoney()>0 || (m_i64AIHaveWinMoney +CountAIWinMoney())<0)
		{
			str.Format("chenlog-------机器人赢钱 %d  总计 %d", CountAIWinMoney(), m_i64AIHaveWinMoney + CountAIWinMoney());
			OutputDebugStringA(str);
			for(int i=0; i<5000; i++)
			{
				SendCard();
				str.Format("chenlog-------机器人赢钱 %d  总计 %d", CountAIWinMoney(), m_i64AIHaveWinMoney + CountAIWinMoney());
				OutputDebugStringA(str);
				if(CountAIWinMoney()<=0)
				{
					if ((m_i64AIHaveWinMoney +CountAIWinMoney())<0)
					{
						//如果开奖导致机器人总赢钱为负数，那么此次重开，直到让机器人赢。
						//DWJ机器人输钱，并且机器人赢钱减去此盘输的钱不能为负数,如果重发了5000次还不行，只能说明RP有问题或者。。。
						if (4999 != i)
						{
							continue;
						}
						//重新发牌发了49次还是没有符合要求的，只能让机器人赢了
						for (int j=0; j<5000; j++)
						{
							SendCard();
							if (CountAIWinMoney()<=0 && (m_i64AIHaveWinMoney + CountAIWinMoney())> 0)
							{
								break;
							}
						}
					}
					else
					{
						OutputDebugStringA("chenlog-----------机器人输钱控制---------1");
						//找到了符合条件的数据就跳出
						break;
					}
				}
			}
		}
	}
}
//-------------------------------------------------------------------------------------------------------------------
///名称：CountAIWinMoney
///描述：机器人输赢控制：计算当前牌机器人的输赢钱
///@param 
///@return 机器人赢的钱，负数为输钱
int		CServerGameDesk::CountAIWinMoney()
{
	__int64 i64Money = 0;

	if(NULL != m_pUserInfo[m_iNowNtStation] && m_pUserInfo[m_iNowNtStation]->m_UserData.isVirtual >= 1)
	{
		///如果机器人是庄家,计算真实玩家的钱
		for(int i=0;i<PLAY_COUNT;i++)
		{
			if(NULL == m_pUserInfo[i] || m_pUserInfo[i]->m_UserData.isVirtual >=1 )//机器人不用计算 || 玩家不存在就不用计算 || 玩家没有下注就不用计算
			{
				continue;
			}
			for(int j=0;j<BET_ARES;j++)
			{			
				if (0 == m_i64UserXiaZhuData[i][j])
				{
					continue;
				}
				i64Money += m_i64UserXiaZhuData[i][j];
				i64Money -= (m_i64UserXiaZhuData[i][j]*m_iWinQuYu[j]);
			}
		}
	}
	else
	{
		///如果机器人不是庄家,计算机器人的钱
		for(int i=0;i<PLAY_COUNT;i++)
		{
			if(NULL == m_pUserInfo[i] || m_pUserInfo[i]->m_UserData.isVirtual <= 0)
			{
				continue;
			}
			for(int j=0;j<8;j++)
			{		
				if (0 == m_i64UserXiaZhuData[i][j])
				{
					continue;
				}
				i64Money -= m_i64UserXiaZhuData[i][j];
				i64Money += (m_i64UserXiaZhuData[i][j]*m_iWinQuYu[j]);
			}
		}
	}
	return i64Money;
}



/*----------------------------------------------------------------------------*/
//显示中奖区域
void	CServerGameDesk::OnSendWinAreas()
{
	m_bGameStation = GS_SHOW_WIN;

	S_C_ShowWinAreas	TShowWinAreas;
	memcpy(TShowWinAreas.iWinQuYu,m_iWinQuYu,sizeof(TShowWinAreas.iWinQuYu));
	for (int i=0;i<PLAY_COUNT; i++)
	{
		if (NULL == m_pUserInfo[i])
		{
			continue;
		}
		SendGameData(i,&TShowWinAreas,sizeof(TShowWinAreas),MDM_GM_GAME_NOTIFY, S_C_SHOW_WINAREA,0);//发送开牌消息
		SetTimer(ID_TIMER_REMAINTIME_GAME,1000);
	}

	//中奖区域显示结束计时器
	SetTimer(ID_TIMER_SHOW_WIN_FINISH,m_iShowResult*1000);
}


/*----------------------------------------------------------------------------*/
///名称：CountFen
///描述：本局算分
///@param 
///@return 
void	CServerGameDesk::CountFen()
{
	memset(m_i64UserFen,0,sizeof(m_i64UserFen));
	memset(m_i64UserAreasFen,0,sizeof(m_i64UserAreasFen));

	

	//统计每个玩家各区域的输赢情况
	for(int i=0; i<PLAY_COUNT; i++)
	{
		if(i == m_iNowNtStation)//庄家不用算分
		{
			continue;
		}
		if(NULL == m_pUserInfo[i])//玩家不存在就不用计算
		{
			continue;
		}
		//没下注的玩家也不结算
		__int64 i64Sum = 0;
		for(int j=0; j<BET_ARES; j++)
		{
			i64Sum += m_i64UserXiaZhuData[i][j];
		}
		if (0 == i64Sum)
		{
			continue;
		}

		// 是否开和 如果开奖区域是 和 全额返回 庄闲下注玩家的下注
		bool isFlat = (m_iWinQuYu[6] > 0);

		//计算玩家各区域的赢钱数目
		for(int j=0; j<BET_ARES; j++)
		{
			if (m_i64UserXiaZhuData[i][j] > 0)
			{
				if (isFlat && (j == 0 || j == 3))    continue;
				if (m_iWinQuYu[j] > 0)
				{
					m_i64UserAreasFen[i][j] = m_i64UserXiaZhuData[i][j]*(m_iWinQuYu[j]-1);
					//庄家得分
					m_i64UserAreasFen[m_iNowNtStation][j]-= m_i64UserAreasFen[i][j];
				}
				else
				{
					m_i64UserAreasFen[i][j] = -m_i64UserXiaZhuData[i][j];
					//庄家得分
					m_i64UserAreasFen[m_iNowNtStation][j] += m_i64UserXiaZhuData[i][j];
				}
			}
			else
			{
				m_i64UserAreasFen[i][j] = 0;
			}
		}
	}

	//记录玩家的输赢和下注情况
	UserPlayResult	TUserPlayResult;
	memset(&TUserPlayResult,0,sizeof(TUserPlayResult));
	
	//标记
	bool flag[PLAY_COUNT];
	memset(flag, 0, sizeof(flag));

	//统计玩家总输赢情况
	for(int i=0; i<PLAY_COUNT; i++)
	{
		if(NULL == m_pUserInfo[i])//玩家不存在就不用计算
		{
			continue;
		}
		//没下注的玩家也不结算
		__int64 i64Sum = 0;
		
		for(int j=0; j<BET_ARES; j++)
		{
			i64Sum += m_i64UserXiaZhuData[i][j];
		}

		//mark
		//有下注就标记(包括庄家)
		if (i64Sum != 0 || i==m_iNowNtStation)
		{
			flag[i] = true;
		}
		
		if (0 == i64Sum && i != m_iNowNtStation)
		{
			continue;
		}
		for(int j=0; j<BET_ARES; j++)
		{
			m_i64UserFen[i] += m_i64UserAreasFen[i][j];
		}

		if (i != m_iNowNtStation)
		{
			//这里统计下玩家压住的把数 用于统计命中率
			TUserPlayResult.bHaveXiaZhu[i]	= true;
			TUserPlayResult.iWinMoney[i]	= m_i64UserFen[i];
		}
	}
	//这里记录放到容器里面保存
	if (m_dqUserPlayResult.size()<MAXCOUNT)
	{
		m_dqUserPlayResult.push_back(TUserPlayResult);
	}
	else
	{
		m_dqUserPlayResult.pop_front();
		m_dqUserPlayResult.push_back(TUserPlayResult);
	}

	//开始结算
	bool temp_cut[PLAY_COUNT];
	memset(&temp_cut, 0, sizeof(temp_cut)); //庄家列表总的庄家位置
	//有下注就交费
	//Eil 庄没结算
	ChangeUserPointint64_IsJoin(m_i64UserFen, temp_cut, flag);
	
	/*
	if (m_iNtTax == 0)				//是否只扣庄家的税
	{
		if(-1 != m_iNowNtStation)
		{
			//ChangeUserPointint64(m_i64UserFen, temp_cut, m_iNowNtStation);
			ChangeUserPointint64_IsJoin(m_i64UserFen, temp_cut, flag, m_iNowNtStation);
		}
	}
	else
	{
		//ChangeUserPointint64(m_i64UserFen, temp_cut);
		ChangeUserPointint64_IsJoin(m_i64UserFen, temp_cut, flag);
	}
	*/


	
	__super::RecoderGameInfo(m_i64UserMoney);


	//记录玩家输赢情况
	for(int i=0;i<PLAY_COUNT;i++)
	{
		if(NULL == m_pUserInfo[i])
		{
			m_i64UserWin[i] = 0;
			continue;
		}
		//记录玩家输赢情况
		m_i64UserWin[i] += m_i64UserMoney[i];
	}
	//更新庄家的总成绩
	m_i64NtWin	+= m_i64UserMoney[m_iNowNtStation];

	//路子更新
	RecordLuziData();
	//清理上庄列表
	CleanZhuangList();
	//奖池记录
	if (m_bAIWinAndLostAutoCtrl)
	{
		RecordAiWinMoney();
	}
	


	//发送消息
	S_C_GameResult	TGameResult;
	TGameResult.i64NtWin	= m_i64NtWin;
	TGameResult.i64NtMoney	= m_pUserInfo[m_iNowNtStation]->m_UserData.i64Money;		//庄家的金币数量

	//路子信息
	for(int i=0;i<m_DqLuziData.size()&&i<MAXCOUNT;i++)
	{
		TGameResult.TLuziData[i] = m_DqLuziData.at(i);
	}
	//上庄列表
	for (auto itr = m_qZhuangList.begin();itr != m_qZhuangList.end();itr++)
	{
		TGameResult.byZhuangList[itr - m_qZhuangList.begin()] = *itr;
	}

	TGameResult.i64NtScoreSum	= m_i64UserMoney[m_iNowNtStation];

	for (int i=0; i<PLAY_COUNT; i++)
	{
		if (NULL == m_pUserInfo[i])
		{
			continue;
		}
		//玩家各区域得分情况
		for(int j=0; j<BET_ARES; j++)
		{
			TGameResult.i64UserAreaScore[j] = m_i64UserAreasFen[i][j];
		}
		
		TGameResult.i64UserScoreSum = m_i64UserMoney[i];		//玩家本局得分
		TGameResult.i64UserMoney	= m_pUserInfo[i]->m_UserData.i64Money;		//玩家自身的金币
		TGameResult.i64UserWin		= m_i64UserWin[i];				//玩家输赢成绩
		//计算玩家命中率
		float fSumCount = 0.0f;	//总下注次数
		float fWinCount	= 0.0f;	//赢的总盘数 
		for(int m=0; m<m_dqUserPlayResult.size(); m++)
		{
			if (m_dqUserPlayResult.at(m).bHaveXiaZhu[i])
			{
				fSumCount++;
			}

			if (m_dqUserPlayResult.at(m).iWinMoney[i] >0 )
			{
				fWinCount++;
			}
		}
		if (fSumCount == 0 || 0== fWinCount)
		{
			TGameResult.fUserPercent = 0.00f;
		}
		else
		{
			TGameResult.fUserPercent = fWinCount/fSumCount;
		}
		
		SendGameData(i,&TGameResult,sizeof(TGameResult),MDM_GM_GAME_NOTIFY, S_C_GAME_END,0);
	}
}

/*------------------------------------------------------------------------*/
///记录本局的成绩数据
void	CServerGameDesk::RecordLuziData()
{
	//记录路子
	LuziData TLuziTmp;
	//记录庄闲各点数
	TLuziTmp.byZPoint = m_iZPaiXing[1];
	TLuziTmp.byXpoint = m_iXPaiXing[1];
	//记录赢得区域 
	if (m_iWinQuYu[6] > 0)	//和了
	{
		TLuziTmp.byWinResult = 2;
	}
	else if (m_iWinQuYu[0] > 0)	// 闲家赢了
	{
		TLuziTmp.byWinResult = 0;
	}
	else if (m_iWinQuYu[3]>0)	//庄家赢了
	{
		TLuziTmp.byWinResult = 1;
	}
	 

	if (m_DqLuziData.size()<MAXCOUNT)
	{
		m_DqLuziData.push_back(TLuziTmp);
	}
	else
	{
		m_DqLuziData.pop_front();
		m_DqLuziData.push_back(TLuziTmp);
	}
}


//清理上庄列表
void	CServerGameDesk::CleanZhuangList()
{
	for (auto itr = m_qZhuangList.begin();itr != m_qZhuangList.end();)
	{
		bool bflag = true;
		if(*itr >= PLAY_COUNT) bflag = false;
		else if(nullptr == m_pUserInfo[*itr]) bflag = false;
		else
		{
			if(m_pUserInfo[*itr]->m_UserData.i64Money < m_i64ShangZhuangLimit) bflag = false;
		}

		if(!bflag) 
		{
			banker.onUserLeaveBanker(*itr,isRobot(*itr));
			itr = m_qZhuangList.erase(itr);
		}
		else itr++;
	}
}


//奖池记录
void	CServerGameDesk::RecordAiWinMoney()
{
	//统计真人的输赢
	__int64 i64Money=0;
	for(int i=0;i<PLAY_COUNT;i++)
	{
		if(m_pUserInfo[i] && (0 == m_pUserInfo[i]->m_UserData.isVirtual))
		{
			i64Money+=m_i64UserMoney[i];
		}
	}

	CString s = CINIFile::GetAppPath ();/////本地路径
	CINIFile f(s +SKIN_FOLDER  + _T("_s.ini"));
	TCHAR szSec[_MAX_PATH] = TEXT("game");
	//写到game上
	//_stprintf_s(szSec, sizeof(szSec), _T("%s_%d"), SKIN_FOLDER,m_pDataManage->m_InitData.uRoomID);

	int i64Tmp = 0;
	i64Tmp = f.GetKeyVal(szSec,"ReSetAIHaveWinMoney ",(__int64)0);

	//不相等 说明手动修改了
	if (m_i64AIHaveWinMoney != i64Tmp)
	{
		m_i64AIHaveWinMoney = i64Tmp;
	}
	else
	{
		//将真人的赢钱数减掉
		m_i64AIHaveWinMoney -= i64Money;
	}

	CString sTemp="";
	sTemp.Format("%I64d",m_i64AIHaveWinMoney);
	f.SetKeyValString(szSec,"ReSetAIHaveWinMoney ",sTemp);
}



///名称：AuthPermissions
///描述：验证超级用户权限：玩家是否超级客户端，给超级客户端玩家发送信息
///@param 
///@return 
void CServerGameDesk::AuthPermissions(BYTE bDeskStation)
{
	if (IsSuperUser(bDeskStation))
	{
		S_C_IsSuperUser TIsSuperUser;
		// 该用户为超级用户, 通知客户端
		TIsSuperUser.byDeskStation = bDeskStation;
		// 超级客户端
		TIsSuperUser.bEnable = true;
		SendGameData(bDeskStation, &TIsSuperUser, sizeof(TIsSuperUser), MDM_GM_GAME_NOTIFY, S_C_IS_SUPER_USER, 0);

	}
}

//判断是否超端玩家
bool CServerGameDesk::IsSuperUser(BYTE byDeskStation)
{
	if (NULL == m_pUserInfo[byDeskStation])
	{
		return false;
	}

	for(int i=0; i<m_vtSuperUserID.size();i++)
	{
		if (m_pUserInfo[byDeskStation]->m_UserData.dwUserID == m_vtSuperUserID.at(i))
		{
			return true;
		}
	}
	return	false;
}


enum NT_STATE
{
	E_EMPTY = 0,
	E_ROBOT,
	E_REALMAN,
};

void CServerGameDesk::NTDecision()
{
	NT_STATE ntState = E_EMPTY;
	if(m_iNowNtStation != -1 && m_pUserInfo[m_iNowNtStation]){
		ntState = m_pUserInfo[m_iNowNtStation]->m_UserData.isVirtual?E_ROBOT:E_REALMAN;
	}

	if(ntState == E_ROBOT && !m_bXiaZhuang && m_iZhuangBaShu > 1)
	{
		bool bLeftFlag =   (1 == rand_Mersense(1,3));
		if(bLeftFlag)
		{
			CString log;
			log.Format("hnlog : 庄家是机器人，庄申请下庄");
			OutputDebugString(log);

			C_S_ApplyShangZhuang cmd;
			cmd.bShangZhuang = false;
			OnHandleShangZhuang(m_iNowNtStation,&cmd);
		}
	}

	//当玩家上庄时，上庄列表还有多个机器人，机器人慢慢下庄不要一次全部下庄。
	if(!banker.m_PreBankerReal.empty())
	{
		if(!banker.m_PreBankerRobots.empty())
		{
			bool bLeftFlag =   (1 == rand_Mersense(1,m_iFreeTime + m_iXiaZhuTime + m_iKaiPaiTime ));

			CString log;
			log.Format("hnlog : 上庄列表真人数量 %d  庄家是 %s, 上庄列表机器人数量 %d,是否随机下庄 %d",
				banker.m_PreBankerReal.size(),
				ntState == E_REALMAN?"真人":"不是真人",
				banker.m_PreBankerRobots.size(),
				bLeftFlag);
			OutputDebugString(log);


			if(bLeftFlag)
			{
				C_S_ApplyShangZhuang cmd;
				BYTE player = -1;
				banker.set_rand(banker.m_PreBankerRobots,player);
				if(player != -1)
				{
					cmd.bShangZhuang = false;
					OnHandleShangZhuang(player,&cmd);
				}

			}
		}
	}

	//没有玩家上庄机器人上庄。
	if(banker.m_PreBankerReal.empty() && banker.m_PreBankerRobots.empty())
	{
		if(!banker.m_NomalPlayerRobots.empty())
		{
			C_S_ApplyShangZhuang cmd;
			BYTE player = -1;
			banker.set_rand(banker.m_NomalPlayerRobots,player);

			CString log;
			log.Format("hnlog : 没有玩家上庄机器人上庄");
			OutputDebugString(log);

			if(player != -1)
			{
				cmd.bShangZhuang = true;
				OnHandleShangZhuang(player,&cmd);
			}
		}
	}

	if(banker.m_PreBankerReal.empty()||banker.m_PreBankerReal.size() + banker.m_PreBankerRobots.size() < rand_Mersense(2,4))
	{
		if(!banker.m_NomalPlayerRobots.empty())
		{
			bool bLeftFlag =   rand_Mersense(1,7) == 1;
			if(bLeftFlag)
			{
				CString log;
				log.Format("hnlog : 上庄列表没有真人，随机上一些机器人");
				OutputDebugString(log);

				C_S_ApplyShangZhuang cmd;
				BYTE player = -1;
				banker.set_rand(banker.m_NomalPlayerRobots,player);
				if(player != 255)
				{
					cmd.bShangZhuang = true;
					OnHandleShangZhuang(player,&cmd);
				}
			}
		}
	}
}












