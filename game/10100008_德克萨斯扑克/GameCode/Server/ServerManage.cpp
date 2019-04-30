#include "StdAfx.h"
#include "ServerManage.h"
#include "../GameMessage/GameAlgorithm.h"
#include "../../../../sdk/include/Common/writelog.h"
#include <math.h>

#include <time.h>
#include <sys/timeb.h>

#if MODE_LOG_DEBUG
#include "LogModule.h"
#endif // MODE_LOG_DEBUG

//CString g_outString;

#if MODE_LOG_SEND
#define LOG_MSG_SEND(AssistID, ...)	(m_bLogOutput ? LogMsgSend(AssistID, __VA_ARGS__) : 0)
#else
#define LOG_MSG_SEND(AssistID, ...)
#endif

#if MODE_LOG_RECV
#define LOG_MSG_RECV(bDeskStation, AssistID, ...)	(m_bLogOutput ? LogMsgRecv(bDeskStation, AssistID, __VA_ARGS__) : 0)
#else
#define LOG_MSG_RECV(bDeskStation, AssistID, ...)
#endif

#if MODE_LOG_TIMER
#define LOG_ON_TIMER(uTimerID)	(m_bLogOutput ? LogOnTimer(uTimerID) : 0)
#else
#define LOG_ON_TIMER(uTimerID)
#endif

#if MODE_LOG_REMIDN
#define LOG_REMIDN(str,...)	(m_bLogOutput ? LogMsgRemind(str, __VA_ARGS__) : 0)
#else	
#define LOG_REMIDN(str,...)
#endif

#ifndef RC_NUM_ALLIN_LIMIT
	#if MODE_ALLIN_LIMIT_DEBUG
	#define RC_NUM_ALLIN_LIMIT	(m_GameData.m_tRCOption.iBaseTakeInMoney)
	#else
	#define RC_NUM_ALLIN_LIMIT	(m_GameData.m_tRCOption.iAllInMoneyLimit)
	#endif //MODE_ALLIN_LIMIT_DEBUG
#endif //RC_NUM_ALLIN_LIMIT


TCHAR szNumber[][10] = {"","2","3","4","5","6","7","8","9","10","J","Q","K","A"};
TCHAR szShape[][10] = {"黑桃", "红桃", "梅花" ,"方块"};
TCHAR szName[][20] = {"", "散牌", "对子", "两对", "三条", "最小顺子", "顺子", "同花", "葫芦", "四条", "最小同花顺", "同花顺", "皇家同花顺"};

#include <math.h>
#include <windows.h> 
#include <DbgHelp.h>  
#include <stdlib.h>  
#pragma comment(lib, "dbghelp.lib")  
#pragma comment(lib, "dbghelp.lib")  

#ifndef _M_IX86  
#error "The following code only works for x86!"  
#endif

//Moved from .h
static __int64 G_i64AIHaveWinMoney = 0;
static __int64 G_i64ReSetAIHaveWinMoney = 0;

static BYTE G_szDynamicInfo[128] = {0};
static bool G_bReSetDynamicInfo = false;
static TRCOptionLimit G_tRCOptionLimit;

static bool G_bTestSetActionProb = false; //测试概率开关
static bool	G_bTestSetActionProbInit = false; //测试概率开关已经初始化与否
static bool G_bUseSetActionProb = false; //使用设定行为概率与否
static bool	G_bUseSetActionProbInit = false; //使用设定行为概率已经初始化与否

inline BOOL IsDataSectionNeeded(const WCHAR* pModuleName)  
{  
	if(pModuleName == 0)  
	{  
		return FALSE;  
	}  

	WCHAR szFileName[_MAX_FNAME] = L"";  
	_wsplitpath(pModuleName, NULL, NULL, szFileName, NULL);  

	if(wcsicmp(szFileName, L"ntdll") == 0)  
		return TRUE;  

	return FALSE;  
}  

inline BOOL CALLBACK MiniDumpCallback(PVOID                            pParam,  
	const PMINIDUMP_CALLBACK_INPUT   pInput,  
	PMINIDUMP_CALLBACK_OUTPUT        pOutput)  
{  
	if(pInput == 0 || pOutput == 0)  
		return FALSE;  

	switch(pInput->CallbackType)  
	{  
	case ModuleCallback:  
		if(pOutput->ModuleWriteFlags & ModuleWriteDataSeg)  
			if(!IsDataSectionNeeded(pInput->Module.FullPath))  
				pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);  
	case IncludeModuleCallback:  
	case IncludeThreadCallback:  
	case ThreadCallback:  
	case ThreadExCallback:  
		return TRUE;  
	default:;  
	}  

	return FALSE;  
}  

inline void CreateMiniDump(PEXCEPTION_POINTERS pep, LPCTSTR strFileName)  
{  
	HANDLE hFile = CreateFile(strFileName, GENERIC_READ | GENERIC_WRITE,  
		FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);  

	if((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))  
	{  
		MINIDUMP_EXCEPTION_INFORMATION mdei;  
		mdei.ThreadId           = GetCurrentThreadId();  
		mdei.ExceptionPointers  = pep;  
		mdei.ClientPointers     = NULL;  

		MINIDUMP_CALLBACK_INFORMATION mci;  
		mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;  
		mci.CallbackParam       = 0;  

		::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, (pep != 0) ? &mdei : 0, NULL, &mci);  

		CloseHandle(hFile);  
	}  
}  

LONG __stdcall MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)  
{  
	CString strPath;
	char szPath[MAX_PATH];
	memset(szPath, 0, MAX_PATH);
	GetCurrentDirectory(MAX_PATH, szPath);
	strPath.ReleaseBuffer();
	strPath.Format("%s\\D-Error\\", szPath);
	CreateDirectory(strPath, NULL);
	CString strFile;
	CTime tm = CTime::GetCurrentTime();
	strFile.Format(_T("%sError-game-%d-%d-%d&%d-%d-%d.dmp"), strPath,tm.GetYear(),tm.GetMonth(),tm.GetDay(),tm.GetHour(),tm.GetMinute(),tm.GetSecond());
	CreateMiniDump(pExceptionInfo, strFile);  
	AfxMessageBox(strPath);
	return EXCEPTION_EXECUTE_HANDLER;  
}  

// 此函数一旦成功调用，之后对 SetUnhandledExceptionFilter 的调用将无效  
void DisableSetUnhandledExceptionFilter()  
{  
	void* addr = (void*)GetProcAddress(LoadLibrary("kernel32.dll"),  
		"SetUnhandledExceptionFilter");  

	if (addr)  
	{  
		unsigned char code[16];  
		int size = 0;  

		code[size++] = 0x33;  
		code[size++] = 0xC0;  
		code[size++] = 0xC2;  
		code[size++] = 0x04;  
		code[size++] = 0x00;  

		DWORD dwOldFlag, dwTempFlag;  
		VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);  
		WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);  
		VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);  
	}  
}  

void InitMinDump()  
{  
	//注册异常处理函数  
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);  

	//使SetUnhandledExceptionFilter  
	DisableSetUnhandledExceptionFilter();  
} 

/*
* 打印日志文件
*/
void DebugPrintf(int uRoomID,const char *p, ...)
{
	//return;	// 发布时不做任何事	By Zxd 20090727
	char szFilename[1024];
	CString strPath = CINIFile::GetAppPath() +"\\GameLog\\";

	SHCreateDirectoryEx(NULL, strPath, NULL);

	CTime time = CTime::GetCurrentTime();

	wsprintf(szFilename, "%s%d_%d_%d%d%d_Server.txt",strPath ,  NAME_ID,uRoomID ,time.GetYear(), time.GetMonth(), time.GetDay());
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

//游戏配置

//游戏扩展配置

//构造函数
CServerGameDesk::CServerGameDesk(void):CGameDesk(ALL_ARGEE)
{
	//for (int i = 0; i < PLAY_COUNT; i++)
	//{
	//	m_emUserAgree[i] = US_UNKNOWN;
	//}
	m_bGameFinished = true;

    m_bGameStation = GS_WAIT_SETGAME;
	m_bFaiPaiFinish = false;
	memset(m_i64UserMoney,0,sizeof(m_i64UserMoney));
	InitMinDump();

#if MODE_LOG_DEBUG
	m_bLogOutput = false;
#endif // MODE_LOG_DEBUG

#if MODE_SET_CARD_DEBUG
	m_bSetCard = false;
#endif // MODE_SET_CARD_DEBUG

	m_bKickPerson = false;
	m_bTimerCheckSet = false;
	memset(m_byNoReadyTimeArr, 0, sizeof(m_byNoReadyTimeArr));
	m_bInitKickPersonFlag = false;

	//最新超端需求 20180411
	m_bSuperSetCardEffectNext = false;
	m_bSuperSetCardEffectCur = false;

	m_bReleaseRoom = false;

	m_bGameStartAtleastOnce = false;

	m_tGameBeginInfo.Clear();
}

//析构函数
CServerGameDesk::~CServerGameDesk(void)
{
}


/*-----------------------------------------------------------------------------------------------------------
一系列平台提供的需要重载来驱动游戏服务的函数实现，已适用大多数游戏的开发，但每个函数的实现因游戏而异。
----------------------------------------------------------------------------------------------------------*/

//读取配置文件
bool CServerGameDesk::InitDeskGameStation()
{
	//LoadDeskCfg();
	LoadDeskCfg(m_pDataManage->m_InitData.uRoomID, false);
	//LoadExtIni(m_pDataManage->m_InitData.uRoomID);

	LoadRobotActionProb();

	SetDynamicInfo(G_szDynamicInfo);

	ClearStatData();

	return true;
}

//游戏开始//game start
bool CServerGameDesk::GameBegin(BYTE bBeginFlag)
{
	m_GameData.m_iPlayCount = 0;
	if (GetPlayerCount() < 2)
	{
		OutputDebugString("dxh: GameBegin, 人数不足");
		return false;
	}

	AutoBuyInIfLTAllInLimit();

	m_tGameBeginInfo.Clear();

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL)
		{
			if (false == 
				(m_bIsBuy ? m_GameData.m_UserData[i].m_bRCReady : m_GameData.m_UserData[i].m_bIn))
			{
				OutputDebugString("dxh: GameBegin, 有玩家未代入金币");
				m_tGameBeginInfo.Clear();
				return false;
			} else 
			{
				m_i64UserMoney[i] = m_GameData.m_UserMoney[i];
				m_tGameBeginInfo.bPlaying[i] = true;
			}
		}
		else
		{
			m_i64UserMoney[i] = 0;
		}
	}

	OutputDebugString("dxh: GameBegin");
	if (__super::GameBegin(bBeginFlag) == false) 
	{
		GameFinish(0, GF_SALE);
		return false;
	}

	//Added by QiWang 20180323, T人相关
	if (!m_bIsBuy && m_bKickPerson)
	{
		KillTimer(TIME_CHECK_READY);
		m_bTimerCheckSet = false;
		memset(m_byNoReadyTimeArr, 0, sizeof(m_byNoReadyTimeArr));
	}

#if MODE_SUPER_DEBUG
	TestSuperSetCard();
#endif //MODE_SUPER_DEBUG
	//最新超端需求 20180411
	m_bSuperSetCardEffectCur = m_bSuperSetCardEffectNext;
	m_tSuperSetCardReqCur = m_tSuperSetCardReqNext;
	m_bSuperSetCardEffectNext = false;
	m_tSuperSetCardReqNext.Clear();


	m_bGameStation = GS_PLAY_GAME;

	m_bGameFinished = false;

	// Added by QiWang 20171012
	if (m_bIsBuy && 0 == m_iRunGameCount)
	{
		//ClearStatData(); 
	}

	m_bGameStartAtleastOnce = true;


	//启动MyTimer，意味游戏的状态流程随之启动
	SetTimer(IDT_MYTIMER,MY_ELAPSE);

	//关闭事件游戏保护
	CloseEventProtection();

	//初始分组表
	InitGroupTable();

	//初始服务端游戏数据
	m_GameData.Init();

	// 游戏数据对象的半初始化
	m_GameData.HalfInit();

	for (int i = 0; i < PLAY_COUNT; i++) 
	{
		m_iUserStation[i] = 255;
		if (m_pUserInfo[i] != NULL)
		{
			bool isReady = m_bIsBuy ? m_GameData.m_UserData[i].m_bRCReady : m_GameData.m_UserData[i].m_bIn;
			if ((m_pUserInfo[i]->m_UserData.bUserState == USER_PLAY_GAME || m_pUserInfo[i]->m_UserData.bUserState == USER_CUT_GAME) && isReady)
			{
				m_iUserStation[i] = TYPE_PLAY_GAME;
			}
			m_GameData.m_bGiveUp[i] = false;
		}
	}

	CString log,temp;
	log.Format("Server: send user state :");
	msgUserState msgResponse;
	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		if (NULL != m_pUserInfo[i])
		{
			msgResponse.byUserState[i] = m_pUserInfo[i]->m_UserData.bUserState;
			temp.Format(" %d ", msgResponse.byUserState[i]);
			log += temp;
		}
	}
	OutputDebugString(log);

	//下发游戏开始信号，通知客户端开启游戏状态机
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL)
		{
			SendGameData(i, &m_tGameBeginInfo, sizeof(m_tGameBeginInfo), MDM_GM_GAME_NOTIFY,ASS_GAME_BEGIN,0);
			SendGameData(i, &msgResponse, sizeof(msgResponse), MDM_GM_GAME_NOTIFY, S_C_S_USER_STATE, 0);
		}
	}
	SendWatchData(m_bMaxPeople,MDM_GM_GAME_NOTIFY,ASS_GAME_BEGIN,0);


	LOG_MSG_SEND(ASS_GAME_BEGIN);

	//SendRCCurCanBuyInRsp();

	//启动系统设置桌面配置流程，主要完成（时间配置，规则配置的任务）
	ActionRedirect(IDEV_SYS_CFG_DESK, 1);

	SendRemainingGameCount();

	if (!m_bIsBuy)
	{
		if (G_bUseSetActionProb && G_bTestSetActionProb)
		{
			LoadRobotActionProb();
		}
		
		SendRobotActionProb();
	}	

	OutputDebugString("DZPKServer:OnUserSubMoney2");
	return true;
}

// 游戏结束
bool CServerGameDesk::GameFinish(BYTE bDeskStation, BYTE bCloseFlag)
{
	CString str;
	str.Format("Server: GameFinish() 游戏结束标志: %d",bCloseFlag);
	OutputDebugString(str);

	KillMyTimer(IDT_MYTIMER);

	switch (bCloseFlag)
	{
	case GF_NORMAL:
		{
			TResult tagResult;

			memset(&tagResult,0,sizeof(tagResult));

			int i, j, k;

			CString str;

			CByteArray arrayResult;

			for (i = 0; i < PLAY_COUNT; i++)
			{
				if (m_pUserInfo[i] != NULL)
				{
					m_pUserInfo[i]->m_UserData.bUserState = USER_SITTING;
					tagResult.nbyUserID[i] = m_pUserInfo[i]->m_UserData.dwUserID;
					//memcpy(tagResult.szName[i],m_pUserInfo[i]->m_UserData.nickName,sizeof(m_pUserInfo[i]->m_UserData.nickName));
					tagResult.bOrder[i] = i;
				}
			}

			int nWinCount = 0;
			__int64 nScore[PLAY_COUNT] = {0};
			__int64 nRecoverMoney[PLAY_COUNT][8] = {0}; // 回收下注的金币

			if (!m_GameData.m_bGiveUpOver)
			{
				OutputDebugString("Server: 比牌结束");
				
				// 整个游戏8个下注池, 需要检测每个下注池胜利玩家
				for (i = 0; i <= m_GameData.m_nBetPool && i < 8; i++)
				{
					// 计算每一个下注池赢家 // 赢的玩家与相等玩家
					CalculateWin(i, arrayResult);
					
					// 获取数组大小
					nWinCount = arrayResult.GetSize();

					str.Format("Server: 当前赢家数量: %d", nWinCount);
					OutputDebugString(str);

					if (nWinCount > 0)
					{
						bool bIsEqual = true;
						for (j = 0; j < PLAY_COUNT; j++)
						{
							if (m_pUserInfo[j] == NULL || m_GameData.m_nBetMoney[j][i] <= 0)
							{
								continue;
							}

							bool bIsWin = false;

							for (k = 0; k < nWinCount; k++)
							{
								CString llog;
								llog.Format("lbdzpk::::::::1111111111赢家 = %d",arrayResult.GetAt(k));
								OutputDebugString(llog);
								if (j == arrayResult.GetAt(k))
								{
									bIsWin = true;
									break;
								}
							}

							if (bIsWin)
							{
								// 胜利玩家跳过
								continue;
							}

							str.Format("Server: 第%d个下注池, 玩家ID: %d, 位置:%d, 输: %I64d", i, m_pUserInfo[j]->m_UserData.dwUserID, j, m_GameData.m_nBetMoney[j][i]);
							OutputDebugString(str);

							// 输家扣金币
							nScore[j] -= m_GameData.m_nBetMoney[j][i];

							bIsEqual = false;

							for (k = 0; k < nWinCount; k++)
							{
								int nStation = arrayResult.GetAt(k);  // 赢的玩家

								str.Format("Server: 赢家人数:%d, 赢家位置: %d", nWinCount, nStation);
								OutputDebugString(str);

								// 胜利玩家加金币
								nScore[nStation] += m_GameData.m_nBetMoney[j][i] / nWinCount;

								// 记录玩家赢的下注池金币
								tagResult.nWinPoolsMoney[nStation][i] += m_GameData.m_nBetMoney[j][i] / nWinCount;

								// 记录玩家赢的下注池
								tagResult.bWinBetPools[nStation][i] = true;

								// 回收自己下注的金币
								if (nRecoverMoney[nStation][i] <= 0)
								{
									nRecoverMoney[nStation][i] = m_GameData.m_nBetMoney[nStation][i];
									CString str;
									str.Format("Server: 比牌结束1---玩家:%d 回收自己下注金币:%I64d, 第%d个下注池", m_pUserInfo[nStation]->m_UserData.dwUserID,nRecoverMoney[nStation][i], i);
									OutputDebugString(str);
								}

								str.Format("Server: 第%d个下注池, 玩家ID: %d, 位置:%d, 赢: %I64d", i, m_pUserInfo[nStation]->m_UserData.dwUserID, nStation, m_GameData.m_nBetMoney[j][i]);
								OutputDebugString(str);
							}
						}

						if (bIsEqual)
						{
							// 玩家牌型相等，没有输家，需要把自己下注的金币回收
							for (k = 0; k < nWinCount; k++)
							{
								int nStation = arrayResult.GetAt(k);  // 赢的玩家

								// 回收自己下注的金币
								if (nRecoverMoney[nStation][i] <= 0)
								{
									tagResult.bWinBetPools[nStation][i] = true;
									nRecoverMoney[nStation][i] = m_GameData.m_nBetMoney[nStation][i];
									CString str;
									str.Format("Server: 比牌结束2---玩家:%d 回收自己下注金币:%I64d, 第%d个下注池", m_pUserInfo[nStation]->m_UserData.dwUserID,nRecoverMoney[nStation][i], i);
									OutputDebugString(str);
								}
							}
						}
					}
					else
					{
						str.Format("Server: 第%d个下注池没有找到胜利玩家", i);
						OutputDebugString(str);
					}
				}
			}
			else
			{
				// 弃牌结束, 只有一个玩家赢
				OutputDebugString("Server: 弃牌结束");
				BYTE bWinStation = -1;
				for (int i = 0; i < PLAY_COUNT; i++)
				{
					if (m_pUserInfo[i] == NULL)
					{
						continue;
					}

					// 先查找到最后一个未弃牌的玩家
					if (!m_GameData.m_bGiveUp[i])
					{
						bWinStation = i;

						str.Format("Server: bWinStation=%d未弃牌", bWinStation);
						OutputDebugString(str);

						for (int j = 0; j <= m_GameData.m_nBetPool && j < 8; j++)
						{
							// 记录玩家赢的下注池
							tagResult.bWinBetPools[bWinStation][j] = true;

							// 回收自己下注的金币
							nRecoverMoney[bWinStation][j] = m_GameData.m_nBetMoney[bWinStation][j];

							CString str;
							str.Format("Server: 弃牌结束---玩家:%d 回收自己下注金币:%I64d, 第%d个下注池", m_pUserInfo[bWinStation]->m_UserData.dwUserID,nRecoverMoney[bWinStation][j], j);
							OutputDebugString(str);
						}

						break;
					}
				}

				for (int i = 0; i < PLAY_COUNT; i++)
				{
					if (m_pUserInfo[i] == NULL || bWinStation == i)
					{
						continue;
					}

					// 输家扣金币
					nScore[i] -= m_GameData.GetBetMoney(i);

					str.Format("Server: 玩家: %d 输金币: %I64d", i, nScore[i]);
					OutputDebugString(str);

					// 赢家金币
					nScore[bWinStation] += _abs64(nScore[i]);

					for (int j = 0; j <= m_GameData.m_nBetPool && j < 8; j++)
					{
						// 记录玩家赢的下注池金币
						tagResult.nWinPoolsMoney[bWinStation][j] = _abs64(nScore[i]);
					}


					str.Format("Server: 玩家: %d 赢金币: %I64d", bWinStation, nScore[bWinStation]);
					OutputDebugString(str);
				}
			}

			OutputDebugString("Server: 处理玩家积分");

			for (i = 0; i < PLAY_COUNT; i++)
			{
				if (m_pUserInfo[i] == NULL)
				{
					continue;
				}

				// 玩家输赢积分
				tagResult.nScore[i] = nScore[i];

				if (nScore[i] > 0)
				{
					tagResult.bWin[i] = true;
				}

				for (j = 0; j <= m_GameData.m_nBetPool && j < 8; j++)
				{
					// 玩家每个下注池数据
					tagResult.nUserBet[i][j] = m_GameData.m_nBetMoney[i][j];
				}

				str.Format("Server: 位置: %d，输赢积分: %I64d", i, tagResult.nScore[i]);
				OutputDebugString(str);
			}

			for (i = 0; i <= m_GameData.m_nBetPool && i < 8; i++) //Changed by QiWang 20171113, j < 8 改为 i < 8
			{
				// 把每个下注池的金币
				tagResult.nBetPools[i] = m_GameData.GetBetMoney(i, false);
			}

			// 写入数据库				
			bool bNetCutUser[PLAY_COUNT];
			memset(bNetCutUser, 0, sizeof(bNetCutUser));

			// 进行金币扣除
			__super::ChangeUserPointint64(tagResult.nScore,bNetCutUser);

			if (m_bIsBuy) // Changed by QiWang 20171014,只有房卡需要大结算信息相关信息,并且考虑多个赢家和平局
			{
				for(int i = 0; i < PLAY_COUNT; ++i)
				{
					m_TotalMoney[i] += tagResult.nScore[i];
					if(tagResult.nScore[i] > 0)
					{
						m_bLatestWinner[i] = true;

						++m_WinCount[i];
						if (m_bLatestWinner[i])
						{
							++m_ContinuouslyWin[i];
						} else 
						{
							m_ContinuouslyWin[i] = 1;
						}
						
						if(m_ContinuouslyWin[i] > m_MaxContinuouslyWin[i]) 
						{
							m_MaxContinuouslyWin[i] = m_ContinuouslyWin[i];
						}

						if(tagResult.nScore[i] > m_MaxWinningMoney[i])
						{
							m_MaxWinningMoney[i] = tagResult.nScore[i];
						}
					} else 
					{
						m_bLatestWinner[i] = false;
						m_ContinuouslyWin[i] = 0;

						continue;
					}
				}
			}

			memset(tagResult.nMoney,0,sizeof(tagResult.nMoney));
			/*if (m_bIsBuy)
			{
				memcpy(tagResult.iTotalMoney, m_TotalMoney, sizeof(tagResult.iTotalMoney));
			}*/

			// 获取扣税后金币
			__super::RecoderGameInfo(tagResult.nMoney);
			
			if (m_bAIWinAndLostAutoCtrl)
			{
				RecordAiHaveWinMoney(tagResult.nScore);
			}

			// 自己下注金币回收处理
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (m_pUserInfo[i] == NULL)
				{
					m_i64UserMoney[i] = 0;
					continue;
				}

				// 加减相应的代入金币, 输的不需要再处理，赢的时候需要把赢的金币加入到代入金币数组中
				if (tagResult.nMoney[i] > 0)
				{
					//m_GameData.m_UserMoney[i] += tagResult.nMoney[i];
				}
				else
				{
					// 下注的钱 + 输了的钱 = 本局游戏返还的钱 2012-10-11 duanxiaohui alter 
					//m_GameData.m_UserMoney[i] += (m_GameData.GetBetMoney(i) + tagResult.nMoney[i]);

					if (m_GameData.m_UserMoney[i] < 0)
					{
						//m_GameData.m_UserMoney[i] = 0;
					}
				}

                // 客户端自身金币更新
                tagResult.nSelfMoney[i] = m_pUserInfo[i]->m_UserData.i64Money;

				tagResult.nMoneyEx[i] = tagResult.nMoney[i];

				str.Format("Server: nScore[%d]=%I64d,  nMoneyEx[%d]=%I64d,  nMoney[%d]=%I64d ", i, tagResult.nScore[i], i, tagResult.nMoneyEx[i], i, tagResult.nMoney[i]);
				OutputDebugString(str);

				for (int j = 0; j < 8; j++)
				{
					if (tagResult.bWinBetPools[i][j])
					{
						CString str;
						str.Format("Server: 玩家:%d 赢金币:%I64d, 回收自己下注金币:%I64d, 第%d个下注池", m_pUserInfo[i]->m_UserData.dwUserID,tagResult.nMoney[i], nRecoverMoney[i][j], j);
						OutputDebugString(str);

						// 把自己下注的金币回收
						//m_GameData.m_UserMoney[i] += nRecoverMoney[i][j];

						// 赢的金币数据也加上自己下注的金币
						tagResult.nWinPoolsMoney[i][j] += nRecoverMoney[i][j];

						if (tagResult.nMoneyEx[i] < 0)
						{
							// 赢的金币数据也加上自己下注的金币
							//tagResult.nMoneyEx[i] = nRecoverMoney[i][j];
							// 2012-10-11 duanxiaohui alter
							tagResult.nMoneyEx[i] = (m_GameData.GetBetMoney(i) + tagResult.nMoney[i]);
						}
						else
						{
							// 把自己下注的金币回收 2012-10-11 duanxiaohui alter
							//m_GameData.m_UserMoney[i] += nRecoverMoney[i][j];

							// 赢的金币数据也加上自己下注的金币
							tagResult.nMoneyEx[i] += nRecoverMoney[i][j];
						}
					}
				}


				m_i64UserMoney[i] = tagResult.nScore[i] + m_i64UserMoney[i];
				m_GameData.m_UserMoney[i] = m_i64UserMoney[i];
			}
            // 更新客户端代入金币
            memcpy(tagResult.nSubMoney, m_GameData.m_UserMoney, sizeof(tagResult.nSubMoney));

			BYTE temp = 0;
			// 金币排序(赢最多的玩家排前名, 依次排名)
			for (i = 0; i < PLAY_COUNT; i++)
			{
				for (j = 0  ; j < PLAY_COUNT - i - 1; j++)
				{
					if (m_pUserInfo[i] == NULL)
					{
						continue;
					}

					if (tagResult.nMoney[j] > tagResult.nMoney[j + 1])
					{
						temp = tagResult.bOrder[j];
						tagResult.bOrder[j] = tagResult.bOrder[j+1];
						tagResult.bOrder[j + 1] = temp;
					}
				}
			}

			for (int i = 0; i < PLAY_COUNT; i ++)
			{
				if (m_pUserInfo[i] == NULL)
				{
					tagResult.bOrder[i] = 255;
					tagResult.nMoney[i] = 0;
				}
			}

			CString strbOrder;

			bool dismiss = (m_iVipGameCount - m_iRunGameCount <= 0);

			//Changed by QiWang 20170915,[- 1 -> - 0],因为房卡模式调用ChangeUserPointint64时m_iRunGameCount已更新
			tagResult.mRemainingGameCount = m_bIsBuy ? (m_iVipGameCount - m_iRunGameCount - 0) : 0;

			// 通知客户端
			for (i = 0; i < PLAY_COUNT; i++)
			{
 				m_GameData.m_UserData[i].m_bIn = false;

				if (m_pUserInfo[i] != NULL)
 				{
					strbOrder.Format("Server:bOrder[%d] == %d",i,tagResult.bOrder[i]);
					OutputDebugString(strbOrder);

 					SendGameData(i, &tagResult, sizeof(TResult), MDM_GM_GAME_NOTIFY, ASS_RESULT, 0);

					str.Format("Server:游戏结算数据-Server:玩家ID:%d,玩家昵称:%s,玩家代入金额:%I64d,玩家真实金额1:%I64d,玩家真实金额2:%I64d,玩家实际输赢金额:%I64d,玩家输赢金额:%I64d",
						m_pUserInfo[i]->m_UserData.dwUserID,m_pUserInfo[i]->m_UserData.szName,tagResult.nSubMoney[i],
						m_pUserInfo[i]->m_UserData.i64Money,tagResult.nSelfMoney[i],tagResult.nMoney[i],tagResult.nMoneyEx[i]);

				   	OutputDebugString(str);
 				}
			}

			SendWatchData(m_bMaxPeople, &tagResult, sizeof(TResult), MDM_GM_GAME_NOTIFY, ASS_RESULT, 0);
			LOG_MSG_SEND(ASS_RESULT);

			m_bGameStation = GS_WAIT_NEXT_ROUND;
			if (m_bIsBuy)
			{
				SendWinTotalInfo();

				if(false)//房卡改成时间场
				//if(dismiss)
				{
					SendScoreBoard(false);
					ClearStatData();
				} else 
				{
					//AutoBuyInAskMoney();
					//AutoMakeUserOffLine();
					SendRCCurMoney();
					//同步玩家金币
					SyncMoney2Store();
				}
			}
			
			// 清除断线玩家信息 duanxiaohui
			//房卡场不清除信息
			if (!m_bIsBuy)
			{
				for (i = 0; i < PLAY_COUNT; i++)
				{
					if (m_pUserInfo[i] != NULL)
					{
						if (m_GameData.m_UserData[i].m_bNetCut)
						{
							CString str;
							str.Format("dxh: 清空断线玩家:%d", i);
							OutputDebugString(str);

							//MakeUserOffLine(i);   // 强退清空该玩家信息
							OnUserMoneyLack(i);   // 通知客户端删除该玩家信息
						}
					}
				}
			}
			
			// 断流，等待每个玩家确认继续游戏消息(ASS_NEXT_ROUND_REQ)，在此需要完善断流风险管理机制			
			//OpenEventProtection(IDEV_NEXT_ROUND_BEGIN, /*m_GameData.m_tagDeskCfg.Time.byNextRound +*/ 30 * 1000);)

			// 游戏结束
			//GameFinish(bDeskStation,bCloseFlag);
			//return true;
			SetTimerCheckReadyInFinish();
			
		}
		break;
	case GFF_SAFE_FINISH:
	case GF_SALE:			//游戏安全结束
		{
			//设置数据
			m_bGameStation = GS_WAIT_ARGEE;//GS_WAIT_SETGAME;

			for (int i = 0; i < PLAY_COUNT; i ++)
			{
				if (m_pUserInfo[i] != NULL) 
					m_pUserInfo[i]->m_UserData.bUserState = USER_SITTING;
			}
			/*GameCutStruct CutEnd;
			::memset(&CutEnd,0,sizeof(CutEnd));

			for (int i = 0; i < PLAY_COUNT; i ++)
			{
				if (m_pUserInfo[i] != NULL)
				{
					SendGameData(i,&CutEnd,sizeof(CutEnd),MDM_GM_GAME_NOTIFY,ASS_SAFE_END,0);
				}
			}
			SendWatchData(m_bMaxPeople,&CutEnd,sizeof(CutEnd),MDM_GM_GAME_NOTIFY,ASS_SAFE_END,0);
			LOG_MSG_SEND(ASS_SAFE_END);*/

			bCloseFlag = GFF_SAFE_FINISH;

			ReSetGameState(bCloseFlag);
			__super::GameFinish(bDeskStation,bCloseFlag);

			SetTimerCheckReadyInFinish();
			return true;
		}
		break;
	case GFF_DISSMISS_FINISH:
		{
			const string strRemind = "解散房间";
			LOG_REMIDN(strRemind);

			DissMissRoomBefore(); // Changed by QiWang 20171012
			m_bGameStation = GS_WAIT_SETGAME;
			ReSetGameState(bCloseFlag);
			__super::GameFinish(bDeskStation,bCloseFlag);
			SetTimerCheckReadyInFinish();
			return true;
		}
	default:
		break;
	}

	// 重置数据
	ReSetGameState(bCloseFlag);

	SetTimerCheckReadyInFinish();

	return __super::GameFinish(bDeskStation,bCloseFlag);
}

//判断是否正在游戏
bool CServerGameDesk::IsPlayGame(BYTE bDeskStation)
{
	if (m_iUserStation[bDeskStation] != TYPE_PLAY_GAME ||m_GameData.m_bGiveUp[bDeskStation] == true)
	{
		return false;
	}
	else
	{
		return m_bGameStation >= GS_PLAY_GAME && m_bGameStation < GS_WAIT_NEXT_ROUND;
	}
	
	//return __super::IsPlayGame(bDeskStation);//m_bGameStation >= GS_SEND_CARD && m_bGameStation < GS_WAIT_NEXT;
}

//根据游戏判断是否在游戏中
BOOL CServerGameDesk::IsPlayingByGameStation() 
{
	return m_bGameStation >= GS_PLAY_GAME && m_bGameStation < GS_WAIT_NEXT_ROUND;
	//return 	__super::IsPlayingByGameStation();//m_bGameStation >= GS_SEND_CARD && m_bGameStation < GS_WAIT_NEXT;
}

bool CServerGameDesk::HandleFrameMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	switch (pNetHead->bAssistantID)
	{
	case ASS_GM_FORCE_QUIT:
		{
			bool bForceQuitAsAuto = true; 
			///查找上层路径
			CString szAppPath = CINIFile::GetAppPath();

			CString strBCFFile; 
			CString strKeyName; 
			strKeyName.Format("%d", NAME_ID); 
			strBCFFile.Format("%s\\SpecialRule.bcf", szAppPath); 

			CBcfFile File(strBCFFile);

			if (File.IsFileExist(strBCFFile))
			{
				bForceQuitAsAuto = File.GetKeyVal("ForceQuitAsAuto", strKeyName, 0); 
			}

			if (bForceQuitAsAuto == true)
			{
				return true; 
			}
			else
			{
				if (m_GameData.m_byTokenUser != bDeskStation 
					&& m_pUserInfo[bDeskStation] != NULL
					&&!m_GameData.m_bGiveUp[bDeskStation] 
				&&m_iUserStation[bDeskStation] == TYPE_PLAY_GAME)
				{
					m_GameData.m_bGiveUp[bDeskStation] = true;

					if (m_GameData.m_byNTUser == bDeskStation)
					{
						m_GameData.m_byNTUser = 255;
					}
					else if (m_GameData.m_bSmallBlind == bDeskStation)
					{
						// 小盲注玩家弃牌, 把小盲注至为255
						m_GameData.m_bSmallBlind = 255;
					}
					else if (m_GameData.m_bBigBlind == bDeskStation)
					{
						// 大盲注玩家弃牌, 把大盲注至为255
						m_GameData.m_bBigBlind = 255;
					}
				}
				else
				{
					TBet tagVerb;
					tagVerb.nType = ET_FOLD;
					OnUserBet(bDeskStation, &tagVerb, true);
				}
			}

			int iCount = 0; 
			for(int  i = 0; i < PLAY_COUNT; i++)
			{
				if(NULL != m_pUserInfo[i] && m_iUserStation[i] == TYPE_PLAY_GAME && m_GameData.m_bGiveUp[i] == false)
				{
					iCount ++ ;
				}
			}

			if(iCount < 2)
			{
				m_GameData.m_bGiveUpOver = true;
				GameFinish(0, GF_NORMAL);
			}

			return true; 
		}
	default:
		{
			break; 
		}
	}

	return __super::HandleFrameMessage(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
}

//游戏数据包处理函数
bool CServerGameDesk::HandleNotifyMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	switch (pNetHead->bAssistantID)
	{

	case ASS_GM_AGREE_GAME:		
		{
			if (IsPlayingByGameStation())
			{
				return true;
			}

			//房卡场货币检测
			if (m_bIsBuy && !m_GameData.CheckCanPlay(bDeskStation))
			{
				return true;
			}

			LOG_MSG_RECV(bDeskStation, pNetHead->bAssistantID);
			//m_GameData.m_UserData[bDeskStation].m_bIn = true;
			m_GameData.m_UserData[bDeskStation].m_bRCReady = true;

			//用户同意游戏
			OnUserAgreeGame(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);


			if (CheckCanGameBegin())
			{
				// 游戏开始
				GameBegin(ALL_ARGEE);
			}

			return true;
		}
	case C_S_SUPER_SET_SIG:
		{
			if (uSize != sizeof(SuperSet))
			{
				return true;
			}
			SuperSet *pSuperSet = (SuperSet *)pData;
			if (NULL == pSuperSet)
			{
				return true;
			}
			m_GameData.m_byWinUserID = pSuperSet->byWinUserID;
			LOG_MSG_RECV(bDeskStation, pNetHead->bAssistantID);
			SendGameData(bDeskStation,MDM_GM_GAME_NOTIFY,S_C_SUPER_SET_RESULT_SIG,0);
			LOG_MSG_SEND(S_C_SUPER_SET_RESULT_SIG, bDeskStation);
			return true;
		}
	case ASS_SUPER_SET_CARD_REQ:
		{
			if (uSize != sizeof(TSuperSetCardReq))
			{
				return true;
			}
			TSuperSetCardReq *pSuperSetCardReq = (TSuperSetCardReq *)pData;
			if (NULL == pSuperSetCardReq)
			{
				return true;
			}

			if (!ExamSuperUser(bDeskStation))
			{
				return true;
			}

			LOG_MSG_RECV(bDeskStation, pNetHead->bAssistantID);
			OnSuperSetCard(bDeskStation, *pSuperSetCardReq);
			//LOG_MSG_SEND(S_C_SUPER_SET_RESULT_SIG, bDeskStation);
			return true;
		}
		break;
	case ASS_CFG_DESK:
		{
			//代入金币限制
			if (bWatchUser)
			{
				return true;
			}
			//处理玩家主设置底注
			LOG_MSG_RECV(bDeskStation, pNetHead->bAssistantID);
			OnUserCfgDesk(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
			return true;
		}
		break;

	case ASS_CFG_DESK_TIMEOUT:
		{
			//代入金币限制
			if (bWatchUser)
			{
				return true;
			}
			//设置底注超时
			LOG_MSG_RECV(bDeskStation, pNetHead->bAssistantID);
			OnUserCfgDeskTimeout(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
			return true;
		}
		break;

	case ASS_AGREE_DESK_CFG:
		{
			//代入金币限制
			if (bWatchUser)
			{
				return true;
			}
			//处理玩家主同意底注
			LOG_MSG_RECV(bDeskStation, pNetHead->bAssistantID);
			OnUserAgreeDeskCfg(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
			return true;
		}
		break;

	case ASS_SUB_MENOY:
		{
			//代入金币限制
			if (bWatchUser)
			{
				return true;
			}

			if (uSize != sizeof(TSubMoney))
			{
				return true;
			}

			if (GS_PLAY_GAME == m_bGameStation)
			{
				return true;
			}

			//房卡场不再处理该消息
			if (m_bIsBuy)
			{
				return true;
			}

			// 玩家代入金币设置
			LOG_MSG_RECV(bDeskStation, pNetHead->bAssistantID);
			OnUserSubMoney(bDeskStation, pData);
			return true;
		}
		break;
		
	case ASS_BET:
		{
			if (bWatchUser)
				return true;
			if(NULL == pData || uSize != sizeof(TBet))
				return true;

			//处理玩家主不同意底注
			LOG_MSG_RECV(bDeskStation, pNetHead->bAssistantID, pData, uSize);
			OnUserBet(bDeskStation, pData, false, true);
			return true;
		}
		break;

	case ASS_SEND_CARD_FINISH:
		{
			//代入金币限制
			if (bWatchUser)
			{
				return true;
			}
			CString llog;
			if (m_pUserInfo[bDeskStation])
			{
				llog.Format("llog::::%s,发牌结束",m_pUserInfo[bDeskStation]->m_UserData.nickName);
				OutputDebugString(llog);
			}
			//处理玩家发牌完成
			LOG_MSG_RECV(bDeskStation, pNetHead->bAssistantID);
			OnUserSendCardFinish(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
			return true;
		}
		break;

	case ASS_NEXT_ROUND_REQ:
		{
			//代入金币限制
			if (bWatchUser)
			{
				return true;
			}
			//处理玩家对下一回合准备的信号
			LOG_MSG_RECV(bDeskStation, pNetHead->bAssistantID);
			OnUserNextRoundReq(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
			return true;
		}
		break;
	case ASS_MONEY_LACK:
		{
			if (bWatchUser)
			{
				return true;
			}

			// 处理玩家本身金币不足于最少带入金额
			LOG_MSG_RECV(bDeskStation, pNetHead->bAssistantID);
			OnUserMoneyLack(bDeskStation);
			return true;
		}
		break;
	case ASS_RC_CUR_CAN_BUYIN_REQ:
		{
			return true;//不处理该消息
			if (bWatchUser)
			{
				return true;
			}

			if (!m_bIsBuy)
			{
				return true;
			}

			//当前可买入回复
			LOG_MSG_RECV(bDeskStation, pNetHead->bAssistantID);
			SendRCCurCanBuyInRsp(false,bDeskStation);
			return true;
		}
		break;
	case ASS_RC_CUR_BUYIN_REQ:
		{
			return true;//不处理该消息
			if (bWatchUser)
			{
				return true;
			}

			if (!m_bIsBuy)
			{
				return true;
			}

			if(uSize != sizeof(TRCCurBuyInReq) || NULL == pData)
				return true;

			//当前买入处理
			LOG_MSG_RECV(bDeskStation, pNetHead->bAssistantID);
			OnUserCurBuyInReq(bDeskStation, pData);
			return true;
		}
		break;

#ifdef CLOSE_CARD
	case ASS_LOOK_CARD:		//玩家看牌
		{
			if(bWatchUser)
			{
				return true;
			}

			UserLookCardStruct s_userlookcard;
			s_userlookcard.bDeskStation = bDeskStation;

			LOG_MSG_RECV(bDeskStation, pNetHead->bAssistantID);
			for(int i = 0; i < PLAY_COUNT; ++i)
			{
				if(m_pUserInfo[i] == NULL || i == bDeskStation)
					continue;

				SendGameData(i,&s_userlookcard,sizeof(UserLookCardStruct),MDM_GM_GAME_NOTIFY,ASS_LOOK_CARD,0);
			}
			LOG_MSG_SEND(ASS_LOOK_CARD);

			return true;
		}
	case ASS_SHOW_CARD:	//展示牌消息
		{
			if(bWatchUser)
			{
				return true;
			}

			//展示牌数据
			ShowCardStruct s_showcard;
			s_showcard.byDeskStation = bDeskStation;
			s_showcard.iCardCount = m_GameData.m_UserData[bDeskStation].m_iCardsNum;
			::CopyMemory(s_showcard.byCardList,m_GameData.m_UserData[bDeskStation].m_byMyCards,sizeof(BYTE)*s_showcard.iCardCount);

			LOG_MSG_RECV(bDeskStation, pNetHead->bAssistantID);
			for(int i = 0; i < PLAY_COUNT; ++i)
			{
				if(m_pUserInfo[i] == NULL)
					continue;

				SendGameData(i,&s_showcard,sizeof(ShowCardStruct),MDM_GM_GAME_NOTIFY,ASS_SHOW_CARD,0);
			}
			LOG_MSG_SEND(ASS_SHOW_CARD);
			return true;
		}
#endif
		//游戏调试的消息处理
#ifdef CD_CAN_SETPAI

	case ASS_FINISH_ROUND_REQ:
		{
			//结束本轮游戏
			LOG_MSG_RECV(bDeskStation, pNetHead->bAssistantID);
			OnTestFinishRoundRequest(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
			return true;
		}
		break;
#endif

	}

	__super::HandleNotifyMessage(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
	return true;
}

//玩家坐下
BYTE CServerGameDesk::UserSitDesk(MSG_GR_S_UserSit * pUserSit, CGameUserInfo * pUserInfo)
{
	//如果玩家桌桌后，桌面没有其他人，则获取桌面的配置
	int iSeatCount = 0;

	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		if (m_pUserInfo[i])
		{
			iSeatCount++;
		}
	}

	if (iSeatCount == 0)
	{
		//此处需要初始化，
		if (CheckCanInitRCOption())
		{
			InitRCOption();
		}

		//清除原来的注额信息
		m_GameData.m_tagDeskCfg.Clear();

		LoadDeskCfg(m_pDataManage->m_InitData.uRoomID);
	}

	return __super::UserSitDesk(pUserSit,pUserInfo);
}

//用户离开游戏桌
BYTE CServerGameDesk::UserLeftDesk(BYTE bDeskStation, CGameUserInfo * pUserInfo)
{
	/*CString str;
	str.Format("Server:-Server: 玩家%d, 离开游戏", bDeskStation);
	OutputDebugString(str);*/

	const string strRemind = "用户退出";
	LOG_REMIDN(strRemind,bDeskStation);

	if (!IsPlayingByGameStation())
	{
		// 清除玩家
		m_GameData.m_UserMoney[bDeskStation] = 0;
		m_GameData.m_UserData[bDeskStation].m_bIn = false;
	}
	
	bool bShow = false;
	int nCount = 0;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		// 判断存在的玩家并且不是当前退出的玩家
		if (m_pUserInfo[i] != NULL && i != bDeskStation)
		{
			nCount++;
		}
		else
		{
			bShow = true;
		}
	}

	if (nCount == 0 && IsPlayingByGameStation())
	{
		m_GameData.m_iDealPeople = 255;
		// 当前玩家退出后, 没有玩家了, 解散本桌
		OutputDebugString("Server: 当前玩家退出后, 没有玩家了, 解散本桌");
		GameFinish(bDeskStation,GF_SALE);
	}


	UserLeftDeskStruct  UserLeft;
	UserLeft.bDeskStation = bDeskStation ; 

	UserLeft.bClearLogo = (m_iUserStation[bDeskStation] == TYPE_PLAY_GAME /*&& m_bGameStation == GS_PLAY_GAME || m_bGameStation == GS_WAIT_ARGEE*/ && bShow) ; 

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL && i != bDeskStation)
		{
			SendGameData(i,&UserLeft,sizeof(UserLeft),MDM_GM_GAME_NOTIFY ,ASS_USER_LEFT_DESK,0);
		}
	}
	LOG_MSG_SEND(ASS_USER_LEFT_DESK);

	 __super::UserLeftDesk(bDeskStation,pUserInfo);

	 if (CheckCanGameBegin())
	 {
		 // 游戏开始
		 GameBegin(ALL_ARGEE);
	 }

	 return true;
}

///名称：UserReCome
///描述：用来截取玩家断线回来信息
bool CServerGameDesk::UserReCome(BYTE bDeskStation, CGameUserInfo * pNewUserInfo)
{
	CString str;
	str.Format("Server: 玩家%d, 玩家断线重回", bDeskStation);
	OutputDebugString(str);

	m_GameData.m_UserData[bDeskStation].m_bNetCut = false;

	{
		string str = "断线重连";
		LOG_REMIDN(str,bDeskStation);
	}

	return __super::UserReCome(bDeskStation, pNewUserInfo);
}

//名称：UserNetCut
///描述：用来截取玩家断线信息
bool CServerGameDesk::UserNetCut(BYTE bDeskStation, CGameUserInfo * pLostUserInfo)
{
	m_GameData.m_UserData[bDeskStation].m_bNetCut = true;
	m_GameData.m_UserData[bDeskStation].m_bRCReady = false;

	{
		string str = "断线离开";
		LOG_REMIDN(str,bDeskStation);
	}

	bool bRet = __super::UserNetCut(bDeskStation, pLostUserInfo);


	CString str;
	str.Format("Server: 玩家%d, 玩家断线", bDeskStation);
	OutputDebugString(str);

	int iPlayerCount = 0;
	int iNetCutCount = 0;
	
	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		if (m_pUserInfo[i])
		{
			iPlayerCount++;
		}
	}

	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		if (m_pUserInfo[i])
		{
			if (m_GameData.m_UserData[i].m_bNetCut)
			{
				iNetCutCount++;
			}
		}
	}

	if (iNetCutCount >= iPlayerCount)
	{
		m_GameData.m_iDealPeople = 255;
		OutputDebugString("Server:重置数据");
		// 重置数据
		ReSetGameState(GF_SALE);
		__super::GameFinish(bDeskStation,GF_SALE);
	}
	
	//switch(m_bGameStation)
	//{
	//case GS_PLAY_GAME:
	//	{
	//		
	//		;
	//	}
	//	break;
	//case GS_WAIT_NEXT_ROUND:
	//	{
	//		TNextRoundReq NextRoundReq;
	//		NextRoundReq.nType = TNextRoundReq::EXIT;
	//		OnUserNextRoundReq(bDeskStation,NULL,&NextRoundReq,sizeof(TNextRoundReq),0,false);
	//	}
	//	break;
	//default:
	//	break;
	//}

	return bRet;
}

///名称：CanNetCut
///描述：用来改变用户断线条件的函数
///@param bDeskStation 要断线玩家的位置
///@return 
bool CServerGameDesk::CanNetCut(BYTE bDeskStation)
{
	return true;
}


void CServerGameDesk::ReleaseRoomBefore()
{
	GameInfForReplay inf[PLAY_COUNT];
	__int64	iCurMoneyTemp = 0; //当前货币[包含此次买入]
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (!m_pUserInfo[i])
		{
			continue;
		}

		inf[i].bLatestWinner = m_bLatestWinner[i];
		inf[i].iTotalMoney = m_TotalMoney[i];
		inf[i].iContinuouslyWin = m_ContinuouslyWin[i];
		inf[i].iMaxContinuouslyWin = m_MaxContinuouslyWin[i];
		inf[i].iMaxWinningMoney = m_MaxWinningMoney[i];
		inf[i].iWinCount = m_WinCount[i];

		//房卡场(时间场)新需求
		if (m_GameData.GetMapCurMoney(m_pUserInfo[i]->m_UserData.dwUserID, iCurMoneyTemp))
		{
			inf[i].iCurMoney = iCurMoneyTemp;
		} else 
		{
			inf[i].iCurMoney = iCurMoneyTemp = 0;
		}
		
		ReleaseRoom(m_pUserInfo[i]->m_UserData.dwUserID,&inf[i],sizeof(inf[i]));
	}

	//清空所有的玩家数据
	ClearStatData();
	const string strRemind = "释放房间";
	LOG_REMIDN(strRemind);
}

void CServerGameDesk::SetReturnGameInfo(int UserID,void* szUserGameInfo,int iSize,int iCount)
{
	m_bReleaseRoom = true;

	if (NULL == szUserGameInfo)
	{
		return;
	}

	GameInfForReplay *pGameInf = (GameInfForReplay*)szUserGameInfo;

	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (m_pReturnInfo[i].iUserID == UserID)
		{
			m_bLatestWinner[i] = pGameInf->bLatestWinner;
			m_TotalMoney[i] = pGameInf->iTotalMoney;
			m_ContinuouslyWin[i] = pGameInf->iContinuouslyWin;
			m_MaxContinuouslyWin[i] = pGameInf->iMaxContinuouslyWin;
			m_MaxWinningMoney[i] = pGameInf->iMaxWinningMoney;
			m_WinCount[i] = pGameInf->iWinCount;
			
			m_GameData.SetMapCurMoney(UserID, pGameInf->iCurMoney);

			break;
		}	
	}
}


//获取游戏状态信息
bool CServerGameDesk::OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser)
{
	//基本状态
	TGSBase tagGSBase;

	//获得游戏的基本代码
	GetGSBase(&tagGSBase,bDeskStation);

	CString str;
	str.Format("lbdzpk: 玩家%d, 获取游戏状态信息", m_bGameStation);
	OutputDebugString(str);

	const string strRemind = "用户进入";
	LOG_REMIDN(strRemind,bDeskStation);

	//for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (bDeskStation < PLAY_COUNT && ExamSuperUser(bDeskStation))
		{
			SendGameData(bDeskStation,MDM_GM_GAME_NOTIFY,S_C_IS_SUPER_SIG,0);
			LOG_MSG_SEND(S_C_IS_SUPER_SIG, bDeskStation);
		}
	}

	switch (m_bGameStation)
	{
	case GS_WAIT_SETGAME:		
	case GS_WAIT_ARGEE:
	case GS_WAIT_NEXT_ROUND:
		{
			//此处等待修改 房卡场
			if (!m_bIsBuy)
			{
				m_GameData.m_UserMoney[bDeskStation] = 0;
				m_GameData.m_UserData[bDeskStation].m_bIn = false;
			} else 
			{
				//初始化房卡选项
				if (CheckCanInitRCOption() || m_bReleaseRoom)
				{
					m_bReleaseRoom = true;
					InitRCOption();
				}

				//初始化当前金币，初始化买入相关
				InitPlayerMoney(bDeskStation);
				m_GameData.m_UserData[bDeskStation].m_bRCReady = false;
			}		
			

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

			//等待玩家开始状态
			OnGetGSWaitAgree(bDeskStation,uSocketID,bWatchUser,&tagGSBase);
			OnGetRCInfoAppend(bDeskStation);

			/*if (!m_GameData.CheckCanPlay(bDeskStation))
			{
			AutoMakeUserOffLine(bDeskStation);
			}*/

			return true;
		}
		break;
	case GS_PLAY_GAME:
		{
			if (m_bIsBuy)
			{
				//初始化当前金币，初始化买入相关
				InitPlayerMoney(bDeskStation);
			}

			//游戏进行的状态中
			OnGetGSPlaying(bDeskStation,uSocketID,bWatchUser,&tagGSBase);
			OnGetRCInfoAppend(bDeskStation);
			SendRobotActionProb(bDeskStation);
			return true;
		}
		break;
	//case GS_WAIT_NEXT_ROUND:
	//	{
	//		m_GameData.m_UserMoney[bDeskStation] = 0;
	//		m_GameData.m_UserData[bDeskStation].m_bIn = false;

	//		//等待下一轮游戏的开始
	//		return OnGetGSWaitNextRound(bDeskStation,uSocketID,bWatchUser,&tagGSBase);
	//	}
	//	break;
	}

	return false;
}

//重置游戏状态
bool CServerGameDesk::ReSetGameState(BYTE bLastStation)
{
	//KillAllTimer();

	//记得把定时器（MyTimer）完全清理掉，否则会严重影响这桌下一次游戏！
	KillTimer(IDT_MYTIMER);
	memset(m_arTimer,0,sizeof(m_arTimer));

	//事件流保护也需要关掉
	CloseEventProtection();

	m_GameData.Init();
	m_bGameStation = GS_WAIT_SETGAME;

	if (m_bIsBuy)
	{

	}

	return TRUE;
}

//定时器消息
bool CServerGameDesk::OnTimer(UINT uTimerID)
{
	//用MyTimer托管MFC的Timer

	switch(uTimerID)
	{
	case IDT_MYTIMER:
		{
			for (int i = 0; i < TIMER_COUNT; ++i)
			{
				if (m_arTimer[i].uiID == 0)
				{
					continue;
				}

				m_arTimer[i].nTickCount++;

				if (m_arTimer[i].nTickCount >= m_arTimer[i].nTriggerCount)
				{
					m_arTimer[i].nTickCount = 0;
					OnMyTimer(m_arTimer[i].uiID);
				}
			}
		}
		return TRUE;
	case TIME_CHECK_READY://普通场T人相关, Added by QiWang 20171106
		{
			if (!m_bIsBuy && m_bKickPerson 
				&& (GS_WAIT_NEXT_ROUND == m_bGameStation|| GS_WAIT_ARGEE == m_bGameStation || GS_WAIT_SETGAME == m_bGameStation)
				&& !m_pDataManage->IsQueueGameRoom()) 
			{
				bool bKillTimer = true;
				bool bHadKickPerson = false;
				for (int i=0; i<PLAY_COUNT; i++)
				{
					if (NULL == m_pUserInfo[i] || m_GameData.m_UserData[i].m_bIn)
					{
						m_byNoReadyTimeArr[i] = 0;
						continue;
					}

					if ((m_byNoReadyTimeArr[i]++) >= m_GameData.m_tagDeskCfg.dz.iUserInMoney)//多加1S，是因为计时器是公共计时器
					{
						m_byNoReadyTimeArr[i] = 0;
						UserLeftDesk(i,m_pUserInfo[i]); // T掉
						bHadKickPerson = true;
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

				if (bHadKickPerson && CheckCanGameBegin())
				{
					// 游戏开始
					GameBegin(ALL_ARGEE);
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

void CServerGameDesk::DissMissRoomBefore()
{
	if (m_bGameStartAtleastOnce) //房卡场一局未开始不发送大结算
	{
		SendScoreBoard(true);
	}
	
	//SendRCScoreBoard();
	ClearStatData(); // Changed by QiWang 20171012
}


/*-----------------------------------------------------------------------------------------------------------
一系列桌子特有机制函数实现，已适用大多数游戏的开发，但每个函数的实现因游戏而异。
函括配置机制、计时机制、动作流机制、游戏状态的管理
----------------------------------------------------------------------------------------------------------*/

//加载限注
BOOL CServerGameDesk::LoadDeskCfg()
{
	TCHAR szIniName[MAX_PATH];
	GET_SERVER_INI_NAME(szIniName);

	CINIFile f(szIniName);

	CString strSeg = "game";
	CString strKey;

	TDeskCfg* pDeskCfg = &m_GameData.m_tagDeskCfg;

	//时间相关////////////////////////////////////////////////////////////////////////////

	//玩家设置玩家操作时间(秒)
	strKey = _TEXT("OperateTime");
	pDeskCfg->Time.byOperate = f.GetKeyVal(strSeg,strKey,10);
	if(pDeskCfg->Time.byOperate<10)
	{
		pDeskCfg->Time.byOperate=10;
	}

#if MODE_LOG_DEBUG
	m_bLogOutput = (bool)f.GetKeyVal(strSeg,"LogOutput",0);
#endif // MODE_LOG_DEBUG

	//Added by QiWang 20180323, T人相关
	if (!m_bInitKickPersonFlag)
	{
		m_bInitKickPersonFlag = true;
		m_bKickPerson = f.GetKeyVal(strSeg,"bKickPerson",0);
	}

	if (!G_bUseSetActionProbInit)
	{
		G_bUseSetActionProbInit = true;
		G_bUseSetActionProb = f.GetKeyVal(strSeg, "UseSetActionProb", 0);
		//LoadRobotActionProb();
	}

	if (!G_bTestSetActionProbInit)
	{
		G_bTestSetActionProbInit = true;
		G_bTestSetActionProb = f.GetKeyVal(strSeg, "TestSetActionProb", 0);
	}

	//奖池配置
	m_bAIWinAndLostAutoCtrl = f.GetKeyVal(strSeg,"AIWinAndLostAutoCtrl",0);				//机器人输赢控制20121122dwj
	m_iAIWantWinMoneyA1		= f.GetKeyVal(strSeg,"AIWantWinMoneyA1 ",__int64(500000));		/**<机器人赢钱区域1  */
	m_iAIWantWinMoneyA2		= f.GetKeyVal(strSeg,"AIWantWinMoneyA2 ",__int64(5000000));	/**<机器人赢钱区域2  */
	m_iAIWantWinMoneyA3		= f.GetKeyVal(strSeg,"AIWantWinMoneyA3 ",__int64(50000000));	/**<机器人赢钱区域3  */
	m_iAIWinLuckyAtA1		= f.GetKeyVal(strSeg,"AIWinLuckyAtA1 ",90);					/**<机器人在区域1赢钱的概率  */
	m_iAIWinLuckyAtA2		= f.GetKeyVal(strSeg,"AIWinLuckyAtA2 ",70);					/**<机器人输赢控制：机器人在区域2赢钱的概率  */
	m_iAIWinLuckyAtA3		= f.GetKeyVal(strSeg,"AIWinLuckyAtA3 ",50);					/**<机器人输赢控制：机器人在区域3赢钱的概率  */
	m_iAIWinLuckyAtA4		= f.GetKeyVal(strSeg,"AIWinLuckyAtA4 ",30);					/**<机器人输赢控制：机器人在区域4赢钱的概率  */
	G_i64AIHaveWinMoney		= f.GetKeyVal(strSeg,"AIHaveWinMoney ",__int64(0));			/**<机器人输赢控制：直接从配置文件中读取机器人已经赢钱的数目  */
	G_i64ReSetAIHaveWinMoney	= f.GetKeyVal(strSeg,"ReSetAIHaveWinMoney ",__int64(0));		//记录重置机器人赢钱数，如果游戏过程中改变了就要改变机器人赢钱数

	if (!G_bReSetDynamicInfo)
	{
		G_bReSetDynamicInfo = true;
		memset(G_szDynamicInfo,0,sizeof(G_szDynamicInfo));
		G_tRCOptionLimit.Clear();
		
		G_tRCOptionLimit.iSmallBlindNoteMax = f.GetKeyVal(strSeg,"SmallBlindNoteMax",__int64(500));
		if (G_tRCOptionLimit.iSmallBlindNoteMax < 1)
		{
			G_tRCOptionLimit.iSmallBlindNoteMax = 1;
		}
		G_tRCOptionLimit.iAllInMoneySmallRate = f.GetKeyVal(strSeg,"AllInMoneySmallRate",__int64(200));
		if (G_tRCOptionLimit.iAllInMoneySmallRate < 1)
		{
			G_tRCOptionLimit.iAllInMoneySmallRate = 1;
		}
		G_tRCOptionLimit.iAllInMoneyBigRate = f.GetKeyVal(strSeg,"AllInMoneyBigRate",__int64(1000));
		if (G_tRCOptionLimit.iAllInMoneyBigRate < 1)
		{
			G_tRCOptionLimit.iAllInMoneyBigRate = 1;
		}

		if (G_tRCOptionLimit.iAllInMoneyBigRate < G_tRCOptionLimit.iAllInMoneySmallRate * 2)
		{
			G_tRCOptionLimit.iAllInMoneyBigRate = G_tRCOptionLimit.iAllInMoneySmallRate * 2;
		}
		memcpy(G_szDynamicInfo, &G_tRCOptionLimit, sizeof(G_tRCOptionLimit));
	}


	

	return TRUE;
}

//根据房间ID加载配置文件
BOOL CServerGameDesk::LoadDeskCfg(int iRoomID, bool bConfigRC)
{
	// 加载通用配置
	LoadDeskCfg();

	TCHAR szIniName[MAX_PATH];
	GET_SERVER_INI_NAME(szIniName);

	CINIFile f(szIniName);

	CString strSeg;
	strSeg.Format("%d_%d",NAME_ID,iRoomID);

	CString strKey;

	TDeskCfg* pDeskCfg = &m_GameData.m_tagDeskCfg;

	//底注相关////////////////////////////////////////////////////////////////////////////
	__int64 iLowers[5] = {2, 5, 10, 20, 50};
	::memcpy(pDeskCfg->dz.iLowers,iLowers,sizeof(iLowers));

	pDeskCfg->dz.iRoomMultiple = f.GetKeyVal(strSeg, "RoomMultiple", 1); // 房间倍数
	if(pDeskCfg->dz.iRoomMultiple<1)
	{
		pDeskCfg->dz.iRoomMultiple=1;
	}
	pDeskCfg->dz.iLower = f.GetKeyVal(strSeg, "MinLimit", 100);			 // 游戏最低额
	pDeskCfg->dz.iUpper = f.GetKeyVal(strSeg, "MaxLimit", 200000000);	 // 游戏最高额
	
	//修改

	pDeskCfg->dz.iBigBlindNote = pDeskCfg->dz.iLower;		             // 大盲注底注
	pDeskCfg->dz.iSmallBlindNote = pDeskCfg->dz.iLower / 2;		         // 小盲注底注
	//pDeskCfg->dz.iBigBlindNote = 20000;
	//pDeskCfg->dz.iSmallBlindNote = 10000;

	pDeskCfg->dz.bIsRoomSubMoney = f.GetKeyVal(strSeg, "RoomSubMoney", 0);  // 房间代入金币  0: 此房间不需要代入金币  1: 此房间需要代入金币
	pDeskCfg->dz.iSubMinMoney = f.GetKeyVal(strSeg, "SubMinMoney",  1000);  // 房间最低代入金币
	pDeskCfg->dz.iSubMaxMoney = f.GetKeyVal(strSeg, "SubMaxMoney", 10000);  // 房间最高代入金币
	pDeskCfg->dz.iSubPlanMoney = f.GetKeyVal(strSeg, "SubPlanMoney",5000);  // 当前准备代入金币

	pDeskCfg->dz.iMinusMoney  = f.GetKeyVal(strSeg, "MinusMoney",100);		// 修改注额加减金币
	pDeskCfg->dz.iMinMoney   = f.GetKeyVal(strSeg, "MinMoney",100);			// 最小下注金币 //和机器人加注有关
	pDeskCfg->dz.iUntenMoney   = f.GetKeyVal(strSeg, "UntenMoney ",0);		// 少于多少金额时带入

	pDeskCfg->dz.iUserClose = f.GetKeyVal(strSeg, "UserClose", true);		// 一局后10秒内将玩家强退
	pDeskCfg->dz.iUserCloseTime = f.GetKeyVal(strSeg, "UserCloseTime", 10); // 关闭客户端时间
	pDeskCfg->dz.iUserInMoney = f.GetKeyVal(strSeg, "iUserInMoney", 10);	// 游戏金额带入时间

	if (pDeskCfg->dz.iUserInMoney < 10)
	{
		pDeskCfg->dz.iUserInMoney = 10;
	}

	//时间相关////////////////////////////////////////////////////////////////////////////

	//玩家设置桌子时间(秒)
	strKey = _TEXT("OperateTime");
	pDeskCfg->Time.byOperate = f.GetKeyVal(strSeg, strKey, pDeskCfg->Time.byOperate);
	
	//规则相关////////////////////////////////////////////////////////////////////////////

	if (bConfigRC && m_bIsBuy)
	{
		//根据房卡选项配置注额相关配置
		//ToDo
		//m_GameData.m_tRCOption.iBigBlindNote = 10;

		pDeskCfg->dz.iLower = m_GameData.m_tRCOption.iBigBlindNote;	// 游戏最低额
		pDeskCfg->dz.iBigBlindNote = pDeskCfg->dz.iLower;		    // 大盲注底注
		pDeskCfg->dz.iSmallBlindNote = pDeskCfg->dz.iLower / 2;		// 小盲注底注

		pDeskCfg->dz.bIsRoomSubMoney = false;						// 房间代入金币  0: 此房间不需要代入金币  1: 此房间需要代入金币

		pDeskCfg->dz.iMinusMoney = pDeskCfg->dz.iLower;				// 修改注额加减金币
		pDeskCfg->dz.iMinMoney = pDeskCfg->dz.iLower;				// 最小下注金币

		for (int i = 0; i < 5; ++i)
		{
			pDeskCfg->dz.iLowers[i] *= pDeskCfg->dz.iLower;			// 房间倍数失效
		}
	} else 
	{
		for (int i = 0; i < 5; ++i)
		{
			pDeskCfg->dz.iLowers[i] *= pDeskCfg->dz.iLower * pDeskCfg->dz.iRoomMultiple;	// 房间倍数不失效
		}
	}

	return TRUE;
}

/// 读取机器人行为概率
bool CServerGameDesk::LoadRobotActionProb()
{
	//TCHAR skin[MAX_NAME_INFO];
	//CString s = CINIFile::GetAppPath ();/////本地路径
	//CString temp = _T("");
	//temp.Format("%s%s_s.ini",s,GET_SKIN_FOLDER(skin));
	//CINIFile f( temp);
	//CString key = TEXT("game");

	TCHAR szIniName[MAX_PATH];
	GET_SERVER_INI_NAME(szIniName);
	CINIFile f(szIniName);
	CString key = "game";

	SetActionProb tSetActionProbTemp;
	CString StrTemp = "";
	int iSumProb = 0;
	for(int i = 0; i < ENUM_SET_ACTION_STATUS_Max; ++i)
	{
		iSumProb = 0;
		StrTemp.Format("ProbFold[%d]",i);
		iSumProb += tSetActionProbTemp.byProbFold[i] = (BYTE)f.GetKeyVal(key,StrTemp,0);
		StrTemp.Format("ProbFollow[%d]",i);
		iSumProb += tSetActionProbTemp.byProbFollow[i] = (BYTE)f.GetKeyVal(key,StrTemp,0);
		StrTemp.Format("ProbRaise[%d]",i);
		iSumProb += tSetActionProbTemp.byProbRaise[i] = (BYTE)f.GetKeyVal(key,StrTemp,0);
		StrTemp.Format("ProbAllIn[%d]",i);
		iSumProb += tSetActionProbTemp.byProbAllIn[i] = (BYTE)f.GetKeyVal(key,StrTemp,0);

		if (iSumProb != 100)
		{
			tSetActionProbTemp.ReSet((ENUM_SET_ACTION_STATUS)i);
		}
	}

	m_tSetActionProb = tSetActionProbTemp;

	return true;
}

/// 发送机器人行为概率
bool CServerGameDesk::SendRobotActionProb(BYTE bDeskStation)
{
	//如果不使用就不发送
	if (!G_bUseSetActionProb)
	{
		return true;
	}

	//房卡场不发送
	if (m_bIsBuy)
	{
		return true;
	}

	//不是游戏状态不发送
	if (!IsPlayingByGameStation())
	{
		return true;
	}

	if (bDeskStation >= PLAY_COUNT)
	{
		for (int i = 0; i < PLAY_COUNT; i ++) 
		{
			//只给Robot发送
			if(NULL == m_pUserInfo[i] || !m_pUserInfo[i]->m_UserData.isVirtual)
				continue;
			SendGameData(i,&m_tSetActionProb,sizeof(m_tSetActionProb),MDM_GM_GAME_NOTIFY,ASS_SET_ACTION_PROB,0);
		}
		LOG_MSG_SEND(ASS_SET_ACTION_PROB);
	} else 
	{
		if(NULL == m_pUserInfo[bDeskStation] || !m_pUserInfo[bDeskStation]->m_UserData.isVirtual)
		{
			return true;
		}
		SendGameData(bDeskStation,&m_tSetActionProb,sizeof(m_tSetActionProb),MDM_GM_GAME_NOTIFY,ASS_SET_ACTION_PROB,0);
		LOG_MSG_SEND(ASS_SET_ACTION_PROB, bDeskStation);
	}

	return true;
}

BOOL CServerGameDesk::LoadChangCard(void)
{
	return true;
}

//初始分组表
void CServerGameDesk::InitGroupTable(void)
{
	return;
}

//创建游戏内部计时器
int CServerGameDesk::SetMyTimer(UINT nTimeID, int uElapse)
{
	int nElapse = uElapse / MY_ELAPSE;
	if (nElapse < 1)
	{
		nElapse = 1;
	}
	int nIdx = nTimeID - IDMYT_BASE;
	if (nIdx < 0 || nIdx > TIMER_COUNT - 1)
	{
		CString strCNOutput;
		strCNOutput.Format("SJServer：定时器[%d]创建不成功.",nTimeID);
		OutputDebugString(strCNOutput);
		return -1;
	}
	m_arTimer[nIdx].uiID = nTimeID;
	m_arTimer[nIdx].nTriggerCount = nElapse;
	return 0;
}


//删除游戏内部计时器
void CServerGameDesk::KillMyTimer(UINT nTimeID)
{
	int nIdx = nTimeID - IDMYT_BASE;
	if (nIdx < 0 || nIdx > TIMER_COUNT-1)
	{
		CString strCNOutput;
		strCNOutput.Format("SJServer：删除定时器[%d]创建不成功.",nTimeID);
		OutputDebugString(strCNOutput);
		return;
	}
	memset(&m_arTimer[nIdx],0,sizeof(TMyTimerNode));
}

//定时器消息
bool CServerGameDesk::OnMyTimer(UINT uTimerID)
{
	if (IDMYT_EVENT_PROTECT != uTimerID)
	{
		LOG_ON_TIMER(uTimerID);
	}
	switch(uTimerID)
	{

		//处理事件流机制/////////////////////////////////////////////
	case IDEV_SYS_CFG_DESK: //系统设定桌面配置
		{
			KillMyTimer(IDEV_SYS_CFG_DESK);
			return OnEventSysCfgDesk();
		}
		break;

	case IDEV_USER_CFG_DESK: //玩家设定桌面配置
		{
			KillMyTimer(IDEV_USER_CFG_DESK);
			return OnEventUserCfgDesk();
		}
		break;

	case IDEV_RAND_CARD:	//洗牌
		{
			KillMyTimer(IDEV_RAND_CARD);
			return OnEventRandCard();
		}
		break;

	case IDEV_RANDOM:	//随机一个庄
		{
			KillMyTimer(IDEV_RANDOM);
			return OnEventRandom();
		}
		break;

	case IDEV_SENDACARD:	 //下发2底牌
		{
			KillMyTimer(IDEV_SENDACARD);
			return OnEventSendACard();
		}
		break;

	case IDEV_SEND_3_CARD: //下发3张公共底牌
		{
			KillMyTimer(IDEV_SEND_3_CARD);
			return OnEventSend3Card();
		}
		break;

	case IDEV_SEND_4_5_CARD: //下发3张公共底牌
		{
			KillMyTimer(IDEV_SEND_4_5_CARD);
			return OnEventSend1Card();
		}
		break;

	case IDEV_BET_START:	//首次下注事件
		{
			KillMyTimer(IDEV_BET_START);
			return OnEventBetStart();
		}
		break;

	case IDEV_BET_BLINDS_AUTO:	//大小盲注同时下注事件
		{
			KillMyTimer(IDEV_BET_BLINDS_AUTO);
			return OnEventBetBlindsAuto();
		}
		break;

	case IDEV_BET_NEXT:	//下位玩家下注事件
		{
			KillMyTimer(IDEV_BET_NEXT);
			return OnEventBetNext();
		}
		break;

	case IDEV_SMALL_BLIND:   //小肓注下注
		{
			KillMyTimer(IDEV_SMALL_BLIND);
		}
		break;

	case IDEV_BIGBLIND:   //大肓注下注
		{
			KillMyTimer(IDEV_BIGBLIND);

		}
		break;

	case IDEV_ROUND_FINISH: //回合结束
		{
			KillMyTimer(IDEV_ROUND_FINISH);
			//return OnEventRoundFinish();
		}
		break;
	case IDEV_COMPARE_CARD: //游戏比牌
		{
			KillMyTimer(IDEV_COMPARE_CARD);
			return OnCompareCard();
		}
		break;
	case IDEV_RESULT: //游戏结算
		{
			KillMyTimer(IDEV_RESULT);
			//return OnEventResult();
			return GameFinish(0, GF_NORMAL);
		}
		break;

	case IDEV_NEXT_ROUND_BEGIN: //下一回合开始
		{
			KillMyTimer(IDEV_NEXT_ROUND_BEGIN);
			return OnEventNextRoundBegin();
		}
		break;

    case IDEV_AUTO_OPERATE:   // 自动操作
        {
            KillMyTimer(IDEV_AUTO_OPERATE);
            return OnEventAutoOperate();
        }
        break;

		//事件流保护相关/////////////////////////////////////////////

	case IDMYT_EVENT_PROTECT:  //事件流保护定时器
		{
			return EventGoing();
		}
		break;
	}

	//return __super::OnTimer(uTimerID);//屏蔽避免出错

	return false;
}


//动作流控制
bool CServerGameDesk::ActionRedirect(UINT uiActID, int iDelay)
{
	switch(uiActID)
	{
	case IDEV_SYS_CFG_DESK:
		{
			//系统设定桌面配置
			OutputDebugString("DZPKServer：动作转向[IDEV_SYS_CFG_DESK.系统设定桌面配置].");
		}
		break;

	case IDEV_USER_CFG_DESK:
		{
			//玩家设定桌面配置
			OutputDebugString("DZPKServer：动作转向[IDEV_USER_CFG_DESK.玩家设定底注额].");
		}
		break;
		
	case IDEV_RANDOM:
		{
			//随机一个庄
			OutputDebugString("DZPKServer：动作转向[IDEV_RANDOM.随机一个庄].");
		}
		break;

	case IDEV_RAND_CARD:
		{
			//洗牌
			OutputDebugString("DZPKServer：动作转向[IDEV_RAND_CARD.洗牌].");
		}
		break;

	case IDEV_SENDACARD:
		{
			//发牌
			OutputDebugString("DZPKServer：动作转向[IDEV_SEND_CARD.发牌].");
		}
		break;

	case IDEV_SEND_3_CARD: 
		{
			//发3张公共牌
			OutputDebugString("DZPKServer：动作转向[IDEV_SEND_3_CARD.发3张公共牌].");
		}
		break;

	case IDEV_BET_START:
		{
			//首次下注
			OutputDebugString("DZPKServer：动作转向[IDEV_BET_START.新的一轮下注].");
		}
		break;

	case IDEV_BET_BLINDS_AUTO:
		{
			//大小盲注同时下注事件
			OutputDebugString("DZPKServer：动作转向[IDEV_BET_BLINDS_AUTO.大小盲注同时下注事件].");
		}
		break;

	case IDEV_BET_NEXT:
		{
			//下位玩家下注事件
			OutputDebugString("DZPKServer：动作转向[IDEV_BET_NEXT.下位玩家下注事件].");
		}
		break;

	case IDEV_ROUND_FINISH:
		{
			//一回合结束
			OutputDebugString("DZPKServer：动作转向[IDEV_ROUND_FINISH.回合结束].");
		}
		break;

	case IDEV_RESULT:
		{
			//回合结算
			OutputDebugString("DZPKServer：动作转向[IDEV_RESULT.回合结算].");
		}
		break;

	case IDEV_NEXT_ROUND_BEGIN:
		{
			//开始下一回合游戏
			OutputDebugString("DZPKServer：动作转向[IDEV_NEXT_ROUND_BEGIN.开始下一回合游戏].");
		}
	}

	SetMyTimer(uiActID,iDelay);

	return true;
}

//打开动作流保护程序
bool CServerGameDesk::OpenEventProtection(UINT uiEvID,int iLifeTime)
{
	if (m_EventProtecter.uiEvID > 0)
	{
		return false;
	}

	m_EventProtecter.uiEvID = uiEvID;
	m_EventProtecter.iLiftTime = iLifeTime;
	m_EventProtecter.iPassTime = 0;

	SetMyTimer(IDMYT_EVENT_PROTECT,1000);

	return true;
}

//处理预算已发牌数量的定时器
bool CServerGameDesk::EventGoing()
{
	if (m_EventProtecter.uiEvID == 0)
	{
		return true;
	}

	m_EventProtecter.iPassTime++;

	if (m_EventProtecter.iPassTime < m_EventProtecter.iLiftTime) 
	{
		return true;
	}

	//事件将要毁灭，根据m_epDescr.uiEvID提供必要的保护

	switch(m_EventProtecter.uiEvID)
	{
	case IDEV_SENDACARD:
		{
			//代管玩家发牌完成
			OnAgentSendCardsFinish();
		}
		break;
	case IDEV_NEXT_ROUND_BEGIN:
		{
			//下一局开始事件流代理
			OnAgentNextRoundBegin();
		}
		break;
	case IDEV_BET_NEXT:
		{
			// 自动操作
			OnEventAutoOperate();
		}
		break;
	}

	//自然毁灭事件
	CloseEventProtection();

	return false;
}

//消灭事件
bool CServerGameDesk::CloseEventProtection()
{
	//uiEvID事件保护利用完毕，销毁事件保护定时器
	KillMyTimer(IDMYT_EVENT_PROTECT);

	//清空事件保护上下文，以便为一下个事件保护提供资源
	::memset(&m_EventProtecter,0,sizeof(TEvProtectDescr));

	return false;
}

//获得游戏的基本的状态
void CServerGameDesk::GetGSBase(TGSBase* pGSBase,BYTE byDeskstation)
{
	//GSBase基本属性
	//当前事件已消耗的时间
	pGSBase->iEvPassTime = m_EventProtecter.iPassTime + 2; //考虑网络延时，加2秒

	//桌面基本配置
	::memcpy(&pGSBase->tagDeskCfg,&m_GameData.m_tagDeskCfg,sizeof(TDeskCfg));

	// 将游戏状态发至客户端
	pGSBase->bGameStation = m_bGameStation;

	CString strUserInfo;
	//获取各座位是否存在玩家
	for (int i = 0; i < PLAY_COUNT; ++i)
	{	
		pGSBase->bHaveUser[i] = (m_pUserInfo[i] != NULL);
	}
	return;
}

//获得游戏的基本的状态
void CServerGameDesk::CopyGSBase(TGSBase* pTarget,const TGSBase* pSource)
{
	if (!pTarget || !pSource)
	{
		return;
	}

	::memcpy(pTarget,pSource,sizeof(TGSBase));

	return;
}

//检查是否可用牌
bool CServerGameDesk::CheckIsAvailableCard(BYTE byCard)
{
	if((0x01 <= byCard &&  byCard <= 0x0D)
		|| (0x11 <= byCard &&  byCard <= 0x1D)
		|| (0x21 <= byCard &&  byCard <= 0x2D)
		|| (0x31 <= byCard &&  byCard <= 0x3D))
	{
		return true;
	}

	return false;
}

//检查牌位于数组哪那个位置
bool CServerGameDesk::CheckCardPosInCardArr(BYTE byCard, int& iCardPos)
{
	if (!CheckIsAvailableCard(byCard))
	{
		return false;
	}

	switch (byCard&0xF0)
	{
	case 0x00:
		iCardPos = (byCard - 0x01);
		break;
	case 0x10:
		iCardPos = ((byCard&0x0F) - 0x01) + 13 * 1;
		break;
	case 0x20:
		iCardPos = ((byCard&0x0F) - 0x01) + 13 * 2;
		break;
	case 0x30:
		iCardPos = ((byCard&0x0F) - 0x01) + 13 * 3;
		break;
	default:
		return false;
		break;
	}

	return true;
}

//检查是否可以开始游戏
bool CServerGameDesk::CheckCanGameBegin()
{
	bool bRet = false;

	if (IsPlayingByGameStation())
	{
		return bRet;
	}

	int nCount = 0;

	bool bReadyTemp = false;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL)
		{
			nCount++;

			//房卡场准备判定修改
			if (m_bIsBuy)
			{
				bReadyTemp = m_GameData.m_UserData[i].m_bRCReady;
			} else 
			{
				bReadyTemp = m_GameData.m_UserData[i].m_bIn;
			}

			if (false == bReadyTemp)
			{
				OutputDebugString("Server: 有玩家未代入金币");
				return false;
			}
		}	
	}

	if (nCount >= 2)
	{
		bRet = true;
	}

	return bRet;
}

//检查是否可以过牌
bool CServerGameDesk::CheckCanSelectCheck(BYTE byDeskstation)
{
	if (byDeskstation >= PLAY_COUNT)
	{
		byDeskstation = m_GameData.m_byTokenUser;
	}

	__int64 nAllBetMoney = m_GameData.GetBetMoney(byDeskstation);
	if (m_GameData.m_nMaxBetMoney <= nAllBetMoney)
	{
		return true;
	}

	return false;
}

//检查是否可以加注
bool CServerGameDesk::CheckCanSelectAdd(BYTE byDeskstation)
{
	if (byDeskstation >= PLAY_COUNT)
	{
		return false;
	}

	__int64 nAllBetMoney = m_GameData.GetBetMoney(byDeskstation);
	if (m_GameData.m_nMaxBetMoney + m_GameData.GetMinBetOrRaiseMoney() 
		<= (m_bIsBuy ? RC_NUM_ALLIN_LIMIT : (nAllBetMoney + m_GameData.m_UserMoney[byDeskstation])))
	{
		return true;
	}

	return false;
}

//检查是否可以跟注
bool CServerGameDesk::CheckCanSelectCall(BYTE byDeskstation)
{
	if (byDeskstation >= PLAY_COUNT)
	{
		return false;
	}

	__int64 nAllBetMoney = m_GameData.GetBetMoney(byDeskstation);
	if (m_GameData.m_nMaxBetMoney <= nAllBetMoney + m_GameData.m_UserMoney[byDeskstation])
	{
		return true;
	}

	return false;
}

//检查是否可以初始化房卡选项
bool CServerGameDesk::CheckCanInitRCOption()
{
	bool bRet = false;

	if (!m_bIsBuy)
	{
		return bRet;
	}

	if (m_GameData.m_tMapCurMoney.size() <= 0)
	{
		bRet = true;
	}

	return bRet;
}

//初始化房卡选项
bool CServerGameDesk::InitRCOption()
{
	bool bRet = false;

	if (!m_bIsBuy)
	{
		return bRet;
	}

	memcpy(&m_GameData.m_tRCOption, m_szDeskConfig, sizeof(m_GameData.m_tRCOption));

	//检查数据是否有错
	if (m_GameData.m_tRCOption.iPlayerCount != 2 
		&& m_GameData.m_tRCOption.iPlayerCount != 6 
		&& m_GameData.m_tRCOption.iPlayerCount != 8 )
	{
		m_GameData.m_tRCOption.iPlayerCount = 8;
	}

	if (m_GameData.m_tRCOption.iSmallBlindNote <= 0)
	{
		m_GameData.m_tRCOption.iSmallBlindNote = 1;
	} else if (m_GameData.m_tRCOption.iSmallBlindNote >= G_tRCOptionLimit.iSmallBlindNoteMax)
	{
		m_GameData.m_tRCOption.iSmallBlindNote = G_tRCOptionLimit.iSmallBlindNoteMax;
	}
	m_GameData.m_tRCOption.iBigBlindNote = m_GameData.m_tRCOption.iSmallBlindNote * 2;

	m_GameData.m_tRCOption.iSmallTakeInMoney = m_GameData.m_tRCOption.iBigBlindNote * 100;
	m_GameData.m_tRCOption.iBigTakeInMoney = m_GameData.m_tRCOption.iSmallBlindNote * 1000;

	//确定记分带入
	if (m_GameData.m_tRCOption.iBaseTakeInMoney < m_GameData.m_tRCOption.iSmallTakeInMoney)
	{
		m_GameData.m_tRCOption.iBaseTakeInMoney = m_GameData.m_tRCOption.iSmallTakeInMoney;
	} else if (m_GameData.m_tRCOption.iBaseTakeInMoney > m_GameData.m_tRCOption.iBigTakeInMoney)
	{
		m_GameData.m_tRCOption.iBaseTakeInMoney = m_GameData.m_tRCOption.iBigTakeInMoney;
	}

	__int64	iAllInMoneySmallLimit = m_GameData.m_tRCOption.iBigBlindNote * G_tRCOptionLimit.iAllInMoneySmallRate;	//AllIn金额下限
	__int64	iAllInMoneyBigLimit = m_GameData.m_tRCOption.iSmallBlindNote * G_tRCOptionLimit.iAllInMoneyBigRate;	//AllIn金额上限
	if (m_GameData.m_tRCOption.iAllInMoneyLimit < iAllInMoneySmallLimit)
	{
		m_GameData.m_tRCOption.iAllInMoneyLimit = iAllInMoneySmallLimit;
	} else if (m_GameData.m_tRCOption.iAllInMoneyLimit > iAllInMoneyBigLimit)
	{
		m_GameData.m_tRCOption.iAllInMoneyLimit = iAllInMoneyBigLimit;
	}

	return true;
}

//初始化玩家金币
bool CServerGameDesk::InitPlayerMoney(BYTE byDeskstation)
{
	if (!m_bIsBuy || byDeskstation >= PLAY_COUNT || NULL == m_pUserInfo[byDeskstation])
	{
		return false;
	}

	const int iPlayerID = m_pUserInfo[byDeskstation]->m_UserData.dwUserID;
	MAP_INT_INT64::iterator itr = m_GameData.m_tMapCurMoney.find(iPlayerID);

	__int64 & iUserMoneyAlias = m_GameData.m_UserMoney[byDeskstation];
	TRCCurBuyInInfo & tRCCurBuyInInfoAlias = m_GameData.m_tRCCurBuyInInfo[byDeskstation];
	if (itr != m_GameData.m_tMapCurMoney.end())
	{
		//玩家已有存储数据，从已有拷贝
		//(金币)
		if (!IsPlayingByGameStation()) //游戏中途不允许修改货币
		{
			m_GameData.GetMapCurMoney(iPlayerID,iUserMoneyAlias);
		}
		
		////(买入)
		//m_GameData.GetMapCurBuyInInfo(iPlayerID,tRCCurBuyInInfoAlias);

		////(大结算) Pause

		//if (iUserMoneyAlias <= 0)
		//{
		//	if (m_GameData.CheckCanBuyIn(byDeskstation))
		//	{
		//		//不能买入
		//	} else
		//	{
		//		//可以买入
		//		//自动买入API
		//		AutoBuyIn(byDeskstation);
		//	}
		//}
	} else 
	{
		//玩家无存储数据，建立
		//初始化数据
		//(金币)
		iUserMoneyAlias = RC_NUM_ALLIN_LIMIT;
		m_GameData.SetMapCurMoney(iPlayerID,iUserMoneyAlias);

		////(买入)
		//tRCCurBuyInInfoAlias.Clear();
		//tRCCurBuyInInfoAlias.iLimitBuyInMoney = (m_GameData.m_tRCOption.iBigTakeInMoney - iUserMoneyAlias);
		//tRCCurBuyInInfoAlias.tRCCurCanBuyIn.iHadBuyInMoney = iUserMoneyAlias;
		//tRCCurBuyInInfoAlias.tRCCurCanBuyIn.iSmallBuyInMoney = m_GameData.m_tRCOption.iSmallTakeInMoney;
		//tRCCurBuyInInfoAlias.tRCCurCanBuyIn.iBigBuyInMoney = tRCCurBuyInInfoAlias.iLimitBuyInMoney;
		//	//检查数值正确性
		//m_GameData.CheckCurBuyInInfo(byDeskstation);
		//m_GameData.SetMapCurBuyInInfo(iPlayerID,tRCCurBuyInInfoAlias);

		////(大结算) Pause // 玩家累计输赢
		//itr->second;
	}

	return true;
}

//同步金币值存储
bool CServerGameDesk::SyncMoney2Store(BYTE byDeskstation)
{
	bool bAll = (byDeskstation >= PLAY_COUNT);

	if (bAll)
	{
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (NULL != m_pUserInfo[i])
			{
				m_GameData.SetMapCurMoney(m_pUserInfo[i]->m_UserData.dwUserID, m_GameData.m_UserMoney[i]);
			}
		}
	} else 
	{
		if (NULL == m_pUserInfo[byDeskstation])
		{
			return false;
		}
		m_GameData.SetMapCurMoney(m_pUserInfo[byDeskstation]->m_UserData.dwUserID, m_GameData.m_UserMoney[byDeskstation]);
	}

	return true;
}

//自动买入
bool CServerGameDesk::AutoBuyIn(BYTE byDeskstation, bool bIsDefault)
{
	if (!m_bIsBuy || byDeskstation >= PLAY_COUNT || NULL == m_pUserInfo[byDeskstation])
	{
		return false;
	}

	TRCCurBuyInReq tCurBuyInReq;
	if (bIsDefault)
	{
		tCurBuyInReq.iBuyInMoney = m_GameData.m_tRCOption.iBaseTakeInMoney;
	} else 
	{
		tCurBuyInReq.iBuyInMoney = m_GameData.m_tRCCurBuyInInfo[byDeskstation].iNextRoundBuyInMoney;
	}
	OnUserCurBuyInReq(byDeskstation, &tCurBuyInReq);

	return true;
}

//自动买入
bool CServerGameDesk::AutoBuyInAskMoney()
{
	if (!m_bIsBuy)
	{
		return false;
	}

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		//修改无金币时的自动买入 ToDo
		AutoBuyIn(i, false);
		m_GameData.m_tRCCurBuyInInfo[i].iNextRoundBuyInMoney = 0;
	}

	return true;
}

//自动买入（玩家金币小于AllIn限制）
bool CServerGameDesk::AutoBuyInIfLTAllInLimit()
{
	//非房卡场退出
	if (!m_bIsBuy)
	{
		return false;
	}

	bool bGotChange = false;
	//买入
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (NULL != m_pUserInfo[i])
		{
			if (m_GameData.m_UserMoney[i] < RC_NUM_ALLIN_LIMIT)
			{
				m_GameData.m_UserMoney[i] = RC_NUM_ALLIN_LIMIT;
				bGotChange = true;
			}
		}
	}

	if (bGotChange)
	{
		//如果有修改就同步
		SyncMoney2Store();

		//如果有修改就发送新金币消息
		SendRCCurMoney();
	}
	
	return true;
}

//自动踢人
bool CServerGameDesk::AutoMakeUserOffLine()
{
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i])
		{
			AutoMakeUserOffLine(i);
		}
	}

	return true;
}

//自动踢人
bool CServerGameDesk::AutoMakeUserOffLine(BYTE byDeskstation)
{
	if (!m_bIsBuy || byDeskstation >= PLAY_COUNT || NULL == m_pUserInfo[byDeskstation])
	{
		return false;
	}

	//检测是否不能玩耍
	if (!m_GameData.CheckCanPlay(byDeskstation))
	{
		//缺乏货币不能玩耍
		SendGameData(byDeskstation, MDM_GM_GAME_NOTIFY, ASS_RC_CANNOT_PLAY_LACK_MONEY, 0);

		//大结算
		SendRCScoreBoard(false, byDeskstation);

		//T人
		//MakeUserOffLine(byDeskstation);
	}

	return true;
}

//自动设置下一个位置动作标记
bool CServerGameDesk::AutoMakeNextPosVerbs(BYTE bNextStation)
{
	if (bNextStation >= PLAY_COUNT)
	{
		return false;
	}

	const BYTE byPreValidRaisePos = m_GameData.GetValidRaisePos(); //上一个有效加注位置

	//每轮开始可以下注、(跟注、加注、)弃牌、过牌、全下
	if (!CheckCanSelectCheck(bNextStation))
	{
		BYTE byFlag = 0;
		
		if (byPreValidRaisePos < PLAY_COUNT
			&& ((bNextStation == byPreValidRaisePos) //有效加注位置不允许再加注
			|| m_GameData.GetBetMoney(bNextStation) >= m_GameData.GetValidRaisePosEffectbet())) //已下筹码大于等于有效加注位置筹码的也不能再加注
		{
			byFlag = UD_VF_CALL|UD_VF_FOLD;
			if (!CheckCanSelectCall(bNextStation))
			{
				byFlag = UD_VF_ALLIN|UD_VF_FOLD;
			}
			//m_GameData.SetValidRaisePosForceNextTurn(true);
		} else 
		{
			byFlag = UD_VF_FOLD|UD_VF_ALLIN;
			//检测货币，是否可以加注
			if (CheckCanSelectAdd(bNextStation))
			{
				byFlag |= (UD_VF_CALL|UD_VF_ADD);
			} else if (CheckCanSelectCall(bNextStation))//检测货币，是否可以跟注
			{
				byFlag |= (UD_VF_CALL);
			}
			//m_GameData.SetValidRaisePosForceNextTurn();
		}

		m_GameData.MakeVerbs(byFlag);
	}
	else
	{
		//OutputDebugString("Server:第一圈大盲注操作时, 则过片按钮、加注按钮、弃牌按钮可显示");
		// 第一圈大盲注操作时, 则过片按钮、加注按钮、弃牌按钮可显示
		//m_GameData.MakeVerbs(UD_VF_CHECK|UD_VF_ADD|UD_VF_FOLD|UD_VF_ALLIN);
		// 还没有人下注: 下注按钮、弃牌按钮、过牌按钮、全下按钮可显示
		// m_GameData.MakeVerbs(UD_VF_BET|UD_VF_FOLD|UD_VF_CHECK|UD_VF_ALLIN);

		BYTE byFlag = (/*UD_VF_ADD|*/UD_VF_FOLD|UD_VF_CHECK|UD_VF_ALLIN);
		if (CheckCanSelectAdd(bNextStation))
		{
			byFlag |= UD_VF_ADD;
		}

		//特殊情况检测,无人加注，即可以下注
		if (m_GameData.m_bRaiseStation >= PLAY_COUNT && byPreValidRaisePos >= PLAY_COUNT)
		{
			byFlag = (/*UD_VF_BET|*/UD_VF_FOLD|UD_VF_CHECK|UD_VF_ALLIN);
			if (CheckCanSelectAdd(bNextStation))
			{
				byFlag |= UD_VF_BET;
			}
		}
		m_GameData.MakeVerbs(byFlag);
		//m_GameData.SetValidRaisePosForceNextTurn();
	}

	return true;
}

/*-----------------------------------------------------------------------------------------------------------
游戏事件响应函数实现，功能函数实现，不同游戏差异较大
----------------------------------------------------------------------------------------------------------*/

//系统设定桌面的事件处理（主要设定时间、游戏规则等）
bool CServerGameDesk::OnEventSysCfgDesk()
{
	//清除原来的注额信息
	m_GameData.m_tagDeskCfg.Clear();

	LoadDeskCfg(m_pDataManage->m_InitData.uRoomID);

	// 广播设定底注的消息
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL)
		{
			SendGameData(i,&m_GameData.m_tagDeskCfg,sizeof(TDeskCfg),MDM_GM_GAME_NOTIFY,ASS_SYS_DESK_CFG,0);
		}
	}

	SendWatchData(m_bMaxPeople,&m_GameData.m_tagDeskCfg,sizeof(TDeskCfg),MDM_GM_GAME_NOTIFY,ASS_SYS_DESK_CFG,0);
	LOG_MSG_SEND(ASS_SYS_DESK_CFG);

	//启动用户设置桌面配置流程，主要完成（时间配置，规则配置的任务）

	ActionRedirect(IDEV_RAND_CARD, 1);

	return true;
}

//注额设定
bool CServerGameDesk::OnEventUserCfgDesk()
{

	//随机选择一个玩家，让TA设置底注，其余玩家等待设置

	int iPlayerNums = 0;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL)
		{
			iPlayerNums++;
		}
	}

	if (iPlayerNums >= 2)
	{
		//::srand((unsigned)::time(NULL)); 
		struct timeb timeSeed;
		ftime(&timeSeed);
		srand((UINT)(timeSeed.time * NUM_ONE_SECOND_MS + timeSeed.millitm));  // milli time
		int iUserSel = rand()%iPlayerNums;

		iPlayerNums = 0;
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (m_pUserInfo[i] != NULL)
			{
				if (iPlayerNums++ == iUserSel)
				{
					iUserSel = i;
					break;
				}
			}
		}

		//记录是那一个玩家需要设置底注
		//m_GameData.m_tagDeskCfg.dz.byUser = iUserSel;


		//可选择性的判断：由于四个玩家中有可能会有玩家的金币数小于顶注，那么这些顶注对应的底注不能被选择
		bool bBreak = false;
		for (int i = 0; i < 5; ++i)
		{
			for (int j = 0; j < PLAY_COUNT; ++j)
			{
				if (m_pUserInfo[j] != NULL)
				{
					if (m_pUserInfo[j]->m_UserData.i64Money < m_GameData.m_tagDeskCfg.dz.iUppers[i])
					{
						bBreak = true;
						break;
					}
				}
			}

			if (bBreak)
			{
				break;
			}

			//m_GameData.m_tagDeskCfg.dz.bSelects[i] = true;
		}


		//广播下注提示
		TCfgDesk tagCfgDesk;
		//复制可选的注值
		::memcpy(tagCfgDesk.dz.iLowers,m_GameData.m_tagDeskCfg.dz.iLowers,sizeof(tagCfgDesk.dz.iLowers));
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (m_pUserInfo[i] != NULL)
			{
				tagCfgDesk.dz.bCanSet = (i == iUserSel);
				SendGameData(i,&tagCfgDesk,sizeof(TCfgDesk),MDM_GM_GAME_NOTIFY,ASS_CFG_DESK,0);
			}
		}
		
		tagCfgDesk.dz.bCanSet = false;
		SendWatchData(m_bMaxPeople,&tagCfgDesk,sizeof(TCfgDesk),MDM_GM_GAME_NOTIFY,ASS_CFG_DESK,0);
		LOG_MSG_SEND(ASS_CFG_DESK);

		//广播下注后，标志着游戏进入GS_CONFIG_NOTE状态（设置底注状态）
		//m_bGameStation = GS_CONFIG_NOTE;
	}

	return true;
}

//随机一个庄事件
bool CServerGameDesk::OnEventRandom()
{	
	//::srand((unsigned)::time(NULL)); 
	struct timeb timeSeed;
	ftime(&timeSeed);
	srand((UINT)(timeSeed.time * NUM_ONE_SECOND_MS + timeSeed.millitm));  // milli time

	if (m_GameData.m_iDealPeople == 255)
	{
		// 第一次随机庄
		m_GameData.m_iDealPeople = rand() % PLAY_COUNT;
		
		if (m_pUserInfo[m_GameData.m_iDealPeople] == NULL)
		{
			m_GameData.m_iDealPeople = GetNextUserStation(m_GameData.m_iDealPeople);
		}
	}
	else
	{
		// 轮流做庄
		m_GameData.m_iDealPeople = GetNextUserStation(m_GameData.m_iDealPeople);
	}

	m_GameData.m_byNTUser = m_GameData.m_iDealPeople;

	////////////////////////////////////////////////////

	if (GetPlayerCount() == 2)
	{
		m_GameData.m_bSmallBlind = m_GameData.m_byNTUser;
		m_GameData.m_bBigBlind = GetNextUserStation(m_GameData.m_byNTUser);		// 获取大盲注位置
	}
	else
	{
		m_GameData.m_bSmallBlind = GetNextUserStation(m_GameData.m_byNTUser);	// 获取小盲注位置
		m_GameData.m_bBigBlind = GetNextUserStation(m_GameData.m_bSmallBlind);	// 获取大盲注位置
	}

	// 广播新回合开始包
	TNextRoundBeginInfo tagNextRoundBeginInfo;
	tagNextRoundBeginInfo.byNTUser = m_GameData.m_byNTUser;
	tagNextRoundBeginInfo.bSmallBlind = m_GameData.m_bSmallBlind;
	tagNextRoundBeginInfo.bBigBlind = m_GameData.m_bBigBlind;

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		// 通知游戏开始
		if (m_pUserInfo[i] != NULL)
		{
			SendGameData(i,&tagNextRoundBeginInfo,sizeof(TNextRoundBeginInfo),MDM_GM_GAME_NOTIFY,ASS_NEW_ROUND_BEGIN,0);
		}
	}

	SendWatchData(m_bMaxPeople,&tagNextRoundBeginInfo,sizeof(TNextRoundBeginInfo),MDM_GM_GAME_NOTIFY,ASS_NEW_ROUND_BEGIN,0);
	LOG_MSG_SEND(ASS_NEW_ROUND_BEGIN);

	//启动洗牌流程
	ActionRedirect(IDEV_SENDACARD, 1);

	return true;
}

//洗牌事件
bool CServerGameDesk::OnEventRandCard()
{
	//int iCardsNum = sizeof(m_GameData.m_byCards);
	GAlg::RandCard(m_GameData.m_byCards,m_GameData.m_tagDeskCfg.Rule.nPokeNum,m_bDeskIndex/*,m_GameData.m_byCardOriginalIndex*/);

	//BYTE bCard[] = {0x03, 0x04, 0x03, 0x24, 0x05, 0x06, 0x3D, 0x33, 0x37, 0x34, 0x14, 0x1B, 0x07, 0x21, 0x22};
	//memcpy(m_GameData.m_byCards, bCard, sizeof(bCard));

	//启动发牌流程
	ActionRedirect(IDEV_RANDOM, 1);
	
	return true;
}

void CServerGameDesk::AiWinAutoCtrl()
{
	bool bAIWin = false;
	//srand((unsigned)GetTickCount());
	struct timeb timeSeed;
	ftime(&timeSeed);
	srand((UINT)(timeSeed.time * NUM_ONE_SECOND_MS + timeSeed.millitm));  // milli time
	int iResult = rand()%100;

	if (0 >=G_i64AIHaveWinMoney)
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
	CString llog;
	llog.Format("lbdzpk :bAIWin = %d",bAIWin);
	OutputDebugString(llog);

	BYTE byResultCard[8][7];
	memset(byResultCard, 0, sizeof(byResultCard));
	BYTE byDeskCard[5] = {0};	//桌面牌

	::CopyMemory(byDeskCard,&m_GameData.m_byCards[m_GameData.m_iGetCardPos],sizeof(BYTE)*5);	//获取低牌
	// 分析牌型
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		m_Logic.AnalysisCard(m_GameData.m_UserData[i].m_byMyCards, 2, byDeskCard, 5, byResultCard[i]);
		memcpy(m_GameData.m_bResultCards[i],byResultCard[i], sizeof(BYTE) * 5);  // 五张最大的牌
	}
	//找出最大牌的玩家
	CByteArray arrayResult;
	CalculateWin(0,arrayResult,true);
	bool bTraveled[PLAY_COUNT] = {0};

	if (bAIWin)
	{
		for (int j = 0;j < arrayResult.GetSize();j++)
		{								
			CString llog;
			llog.Format("lbdzpk::::::::00000000000赢家 = %d",arrayResult.GetAt(j));
			OutputDebugString(llog);
			for (int i = 0;i < PLAY_COUNT;i++)
			{
				if (!m_pUserInfo[i]) continue;
				if (!m_pUserInfo[i]->m_UserData.isVirtual) continue; //不是机器人
				if (!m_GameData.m_UserData[i].m_bIn) continue; //没带入金币
				if (bTraveled[i]) continue;

				// 比牌(-1:输, 0:和, 1:赢)
				//if (m_Logic.CompareCard(m_GameData.m_bResultCards[i], 5, m_GameData.m_bResultCards[arrayResult.GetAt(j)], 5) > 0)
				{
					llog.Format("lbdzpk::::::::00000000000赢家 = %s ,最大牌者  = %s",m_pUserInfo[arrayResult.GetAt(j)]->m_UserData.nickName,m_pUserInfo[i]->m_UserData.nickName);
					OutputDebugString(llog);
					Change2UserCard(i,arrayResult.GetAt(j));
					bTraveled[i] = true;
					break;
				}
			}
		}
	}
	else
	{
		for (int j = 0;j < arrayResult.GetSize();j++)
		{
			for (int i = 0;i < PLAY_COUNT;i++)
			{
				if (!m_pUserInfo[i]) continue;
				if (m_pUserInfo[i]->m_UserData.isVirtual) continue; //是机器人
				if (!m_GameData.m_UserData[i].m_bIn) continue; //没带入金币
				if (bTraveled[i]) continue;
				llog.Format("lbdzpk::::::::111111赢家 = %s ,最大牌者  = %s",m_pUserInfo[arrayResult.GetAt(j)]->m_UserData.nickName,m_pUserInfo[i]->m_UserData.nickName);

				OutputDebugString(llog);
				Change2UserCard(i,arrayResult.GetAt(j));
				bTraveled[i] = true;
				break;
			}
		}
	}
}

void CServerGameDesk::GotHandCards()
{
	//初始化一些数据
	m_GameData.m_iGetCardPos = 0;
	m_GameData.m_iPlayCount = 0;
	CUserDataSvrObject* pUserData = NULL;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		pUserData = &m_GameData.m_UserData[i];
		pUserData->m_iCardsNum = 0;
	}
	
	//获得手牌
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL && TYPE_PLAY_GAME == m_iUserStation[i]) //合法玩家
		{
			//为玩家i取2张牌
			m_GameData.DealCardsTo(i, MAX_DEAL_CARDS);
			m_GameData.m_iPlayCount++;
		}
	}
}

bool CServerGameDesk::SuperUserSetCard()
{
	if (!m_bSuperSetCardEffectCur)
	{
		return false;
	}

	//return false;

	//!!!任何部分出错，即返回false

	static bool bCardSetArr[NUM_CARDS_NUM]={false};
	BYTE byCardArray[NUM_CARDS_NUM]={0x00}; //改动的牌数组
	memcpy(byCardArray, m_GameData.m_byCards, sizeof(byCardArray));

	//设定公共牌
	const int iPlayerCountTotal = m_GameData.m_iPlayCount;
	int iCardPos = 0;
	BYTE byCardToSet = 0;
	for (int i=0; i < NUM_GET_ARR(m_tSuperSetCardReqCur.byComCards); i++)
	{
		//配单张牌
		byCardToSet = m_tSuperSetCardReqCur.byComCards[i];
		if (!CheckIsAvailableCard(byCardToSet))
		{
			continue;
		}

		if (!SetCardInArr(byCardToSet, byCardArray, iPlayerCountTotal * MAX_DEAL_CARDS + i, bCardSetArr))
		{
			return false;
		}
	}

	//设定指定UserID玩家的手牌
	int iFindIndex = 0;
	BYTE iStartIndexUserCard[PLAY_COUNT]={0};
	BYTE iPlayerCountTemp = 0;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL && TYPE_PLAY_GAME == m_iUserStation[i]) //合法玩家
		{
			iStartIndexUserCard[i] = 0 + iPlayerCountTemp * MAX_DEAL_CARDS;
			iPlayerCountTemp ++;
			if (FindUserID(m_pUserInfo[i]->m_UserData.dwUserID, m_tSuperSetCardReqCur, iFindIndex))
			{
				//预防错误
				if (iFindIndex < 0 || iFindIndex >= PLAY_COUNT)
				{
					return false;
				}

				//配手牌
				THandCards tHandCardsAlias = m_tSuperSetCardReqCur.tPlayerHandCards[iFindIndex];
				for (int j=0; j < NUM_GET_ARR(tHandCardsAlias.byCard); j++)
				{
					byCardToSet = tHandCardsAlias.byCard[j];
					if (!CheckIsAvailableCard(byCardToSet))
					{
						continue;
					}

					if (!SetCardInArr(byCardToSet, byCardArray, iStartIndexUserCard[i] + j, bCardSetArr))
					{
						return false;
					}
				}
			}
		}
	}

	//随机生成未设置的公共牌
	for (int i=0; i < NUM_GET_ARR(m_tSuperSetCardReqCur.byComCards); i++)
	{
		byCardToSet = m_tSuperSetCardReqCur.byComCards[i];
		if (CheckIsAvailableCard(byCardToSet))
		{
			continue;
		}

		if (!RandomSetCardInArr(byCardArray, iPlayerCountTotal * MAX_DEAL_CARDS + i, bCardSetArr))
		{
			return false;
		}
	}


	//随机生成未设置的手牌
	iPlayerCountTemp = 0;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL && TYPE_PLAY_GAME == m_iUserStation[i]) //合法玩家
		{
			iStartIndexUserCard[i] = 0 + iPlayerCountTemp * MAX_DEAL_CARDS;
			iPlayerCountTemp ++;
			if (!FindUserID(m_pUserInfo[i]->m_UserData.dwUserID, m_tSuperSetCardReqCur, iFindIndex))
			{
				//随机生成手牌
				for (int j=0; j < MAX_DEAL_CARDS; j++)
				{
					if (!RandomSetCardInArr(byCardArray, iStartIndexUserCard[i] + j, bCardSetArr))
					{
						return false;
					}
				}
			} else
			{
				//预防错误
				if (iFindIndex < 0 || iFindIndex >= PLAY_COUNT)
				{
					return false;
				}

				//配手牌
				THandCards tHandCardsAlias = m_tSuperSetCardReqCur.tPlayerHandCards[iFindIndex];
				for (int j=0; j < NUM_GET_ARR(tHandCardsAlias.byCard); j++)
				{
					byCardToSet = tHandCardsAlias.byCard[j];
					if (CheckIsAvailableCard(byCardToSet))
					{
						continue;
					}

					if (!RandomSetCardInArr(byCardArray, iStartIndexUserCard[i] + j, bCardSetArr))
					{
						return false;
					}
				}
			}
		}
	}

	//拷贝回去
	memcpy(m_GameData.m_byCards, byCardArray, sizeof(byCardArray));

	//重新获得玩家的手牌
	GotHandCards();

	return true;
}

bool CServerGameDesk::SetCardInArr(BYTE byCard, BYTE* byCardArr, int iIndex, bool * bSetCardArr)
{
	if (NULL == byCardArr || NULL == bSetCardArr)
	{
		return false;
	}

	if (iIndex < 0 || iIndex >= NUM_CARDS_NUM)
	{
		return false;
	}

	bool bSetCardSucceed = false; 
	//换牌
	for(int i=0; i < NUM_CARDS_NUM; i++)
	{
		if (iIndex == i || byCard == byCardArr[i])
		{
			byCardArr[i] = byCard;
			//记录已经换牌的数组
			bSetCardArr[i] = true;
			bSetCardSucceed = true;
		}
	}

	return bSetCardSucceed;
}


//随机设定数组对应位置的牌
bool CServerGameDesk::RandomSetCardInArr(BYTE* byCardArr, int iIndex, bool * bSetCardArr)
{
	if (NULL == byCardArr || NULL == bSetCardArr)
	{
		return false;
	}

	if (iIndex < 0 || iIndex >= NUM_CARDS_NUM)
	{
		return false;
	}

	bool bSetCardSucceed = false;

	//找到还未设定牌的位置
	int bySelectCardIndex = 0;
	for(int i=0; i < NUM_CARDS_NUM; i++)
	{
		if (!bSetCardArr[i])
		{
			bSetCardSucceed = true;
			bySelectCardIndex = i;
			
			break;
		}
	}

	//换牌
	BYTE byCardTemp = 0;
	if (bSetCardSucceed)
	{
		byCardTemp = byCardArr[bySelectCardIndex];
		byCardArr[bySelectCardIndex] = byCardArr[iIndex];
		byCardArr[iIndex] = byCardTemp;

		//记录已经换牌的数组
		bSetCardArr[bySelectCardIndex] = true;
		bSetCardArr[iIndex] = true;
	}

	return bSetCardSucceed;
}

bool CServerGameDesk::FindUserID(const int iUserID, const TSuperSetCardReq& tSetCard, int& iFindIndex)
{
	for (int i = 0; i < NUM_GET_ARR(tSetCard.tPlayerHandCards); i++)
	{
		if(tSetCard.tPlayerHandCards[i].bSetHandCard
		   && iUserID == tSetCard.tPlayerHandCards[i].iUserID)
		{
			iFindIndex = i;
			return true;
		}
	}

	return false;
}

/*---------------------------------------------------------------------*/
//交换两个人的牌
void	CServerGameDesk::Change2UserCard(BYTE byFirstDesk,BYTE bySecondDesk)
{
	BYTE byTmpCard[MAX_BACKCARDS_NUM];
	memcpy(byTmpCard,	m_GameData.m_UserData[byFirstDesk].m_byMyCards,	sizeof(byTmpCard));
	memcpy(m_GameData.m_UserData[byFirstDesk].m_byMyCards,	m_GameData.m_UserData[bySecondDesk].m_byMyCards,	sizeof(m_GameData.m_UserData[byFirstDesk].m_byMyCards));
	memcpy(m_GameData.m_UserData[bySecondDesk].m_byMyCards,byTmpCard,sizeof(m_GameData.m_UserData[bySecondDesk].m_byMyCards));
}

/*-----------------------------------------------------------------------------------*/
void CServerGameDesk::RecordAiHaveWinMoney(__int64 i64UserScore[])
{
	try
	{
		TCHAR szIniName[MAX_PATH];
		GET_SERVER_INI_NAME(szIniName);

		CINIFile f(szIniName);

		CString szSec("Game");


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
			for(int i=0;i<PLAY_COUNT;i++)
			{
				if(NULL != m_pUserInfo[i] && m_pUserInfo[i]->m_UserData.isVirtual == 0)
				{
					G_i64AIHaveWinMoney -= i64UserScore[i];
				}
			}
		}
		CString sTemp;
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


//为每个玩家下发2张底牌的事件
bool CServerGameDesk::OnEventSendACard()
{
	GotHandCards();
	TCards tagCards;

#ifdef MODE_SUPER
	if (m_bSuperSetCardEffectCur && SuperUserSetCard())
	{
		//Do Nothing
	} else if (m_GameData.m_byWinUserID != 0)
	{
		for (int i = 0; i < PLAY_COUNT; ++i)
		{
			if (m_pUserInfo[i] && m_pUserInfo[i]->m_UserData.dwUserID == m_GameData.m_byWinUserID)
			{
				m_GameData.GetGoodCard(i);
				m_GameData.m_byWinUserID = 0;
			}
		}
	} else if ((ExistRealUser() != 0 && ExistRealUser() != GetPlayerCount(true)) && m_bAIWinAndLostAutoCtrl)
	{
		AiWinAutoCtrl();
	}

#endif
	
	tagCards.iCardsNum = MAX_DEAL_CARDS;
	tagCards.nTypeCard = SEND_A_CAND;

	//广播发牌数据
	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		if (m_pUserInfo[i] != NULL && TYPE_PLAY_GAME == m_iUserStation[i]) //合法玩家
		{
			tagCards.byUser = i;

			memset(tagCards.byCards,0,sizeof(tagCards.byCards));
			memset(tagCards.byPubCards,0,sizeof(tagCards.byPubCards));
			//得到玩家i的手牌
			m_GameData.m_UserData[i].GetHandCards(tagCards.byCards[i]);
			int nextPos = GetNextUserStation(i);
			CString cstr = NULL;
			CString cstr1 = NULL;
			cstr.Format("llog::::i = %d ,自己 card[%d] = %d   ， %d",i,i,tagCards.byCards[i][0],tagCards.byCards[i][1]);
			bool bSuper = ExamSuperUser(i);
			if (bSuper || m_pUserInfo[i]->m_UserData.isVirtual)
			{
				memcpy(tagCards.byPubCards,  &m_GameData.m_byCards[m_GameData.m_iGetCardPos], MAX_THREE_CARDS + MAX_FIRST_CARDS + MAX_FIRST_CARDS);
			}
			while (nextPos != i && (bSuper || m_pUserInfo[i]->m_UserData.isVirtual))
			{
				m_GameData.m_UserData[nextPos].GetHandCards(tagCards.byCards[nextPos]);
				cstr1.Format("::::i = %d ,别人 card[%d] = %d   ， %d",nextPos,nextPos,tagCards.byCards[nextPos][0],tagCards.byCards[nextPos][1]);
				cstr += cstr1;
				OutputDebugString(cstr);
				nextPos = GetNextUserStation(nextPos);
			}
			SendGameData(i, &tagCards,sizeof(TCards), MDM_GM_GAME_NOTIFY, ASS_SEND_A_CARD, 0);
		}
	}

	memset(tagCards.byCards,0,sizeof(tagCards.byCards));
	SendWatchData(m_bMaxPeople, &tagCards,sizeof(TCards),MDM_GM_GAME_NOTIFY, ASS_SEND_A_CARD, 0);
	LOG_MSG_SEND(ASS_SEND_A_CARD);

	OutputDebugString("dxh: 为每个玩家下发2张底牌的事件");
	//ActionRedirect(IDEV_BET_START, 1000);
	m_bFaiPaiFinish = false;

	OpenEventProtection(IDEV_SENDACARD, GetPlayerCount() * 2);

	return true;

}

//为发公牌的事件
bool CServerGameDesk::OnEventSend3Card()
{
	TCards tagCards;

	CString str;
	str.Format("dxh : 当前发牌位置: %d", m_GameData.m_iGetCardPos);
	OutputDebugString(str);
	memcpy(m_GameData.m_iCards, &m_GameData.m_byCards[m_GameData.m_iGetCardPos], MAX_THREE_CARDS);
	m_GameData.m_iGetCardPos += MAX_THREE_CARDS;
	m_GameData.m_iCardsCount += MAX_THREE_CARDS;

	memcpy(tagCards.byPubCards, m_GameData.m_iCards, sizeof(BYTE) * m_GameData.m_iCardsCount);
	tagCards.iCardsNum += m_GameData.m_iCardsCount;

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		/*bool bSuper = ExamSuperUser(i);
		if (bSuper)
		{
			continue;
		}*/
		
		if (m_pUserInfo[i] != NULL)
		{
			SendGameData(i, &tagCards, sizeof(tagCards), MDM_GM_GAME_NOTIFY, ASS_SEND_3_CARD, 0);
		}
	}	
	
	SendWatchData(m_bMaxPeople,&tagCards, sizeof(tagCards), MDM_GM_GAME_NOTIFY, ASS_SEND_3_CARD, 0);
	LOG_MSG_SEND(ASS_SEND_3_CARD);

	OutputDebugString("dxh: 为发公牌的事件");


	if (m_GameData.m_bAutoSendCard)
	{
		m_GameData.m_nTurnNums++;

		// 发一张公共牌
		ActionRedirect(IDEV_SEND_4_5_CARD, 1200);
	}
	else
	{
		ActionRedirect(IDEV_BET_START, 500);
	}

	return true;
}

//为发1张公共底牌的事件
bool CServerGameDesk::OnEventSend1Card()
{
	CString str;
	str.Format("dxh : 当前发牌位置: %d", m_GameData.m_iGetCardPos);
	OutputDebugString(str);

	if (m_GameData.m_iCardsCount >= 5)
	{
		OutputDebugString("Server: 公共牌最多5张牌");
		return true;
	}

	TCards tagCards;
	memcpy(&m_GameData.m_iCards[m_GameData.m_iCardsCount], &m_GameData.m_byCards[m_GameData.m_iGetCardPos], MAX_THREE_CARDS);
	m_GameData.m_iGetCardPos += MAX_FIRST_CARDS;
	m_GameData.m_iCardsCount += MAX_FIRST_CARDS;

	memcpy(tagCards.byPubCards, m_GameData.m_iCards, sizeof(BYTE) * m_GameData.m_iCardsCount);
	tagCards.iCardsNum = m_GameData.m_iCardsCount;

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		/*bool bSuper = ExamSuperUser(i);
		if (bSuper)
		{
		continue;
		}*/

		if (m_pUserInfo[i] != NULL)
		{
			SendGameData(i, &tagCards, sizeof(tagCards), MDM_GM_GAME_NOTIFY, ASS_SEND_4_5_CARD, 0);
		}
	}	

	SendWatchData(m_bMaxPeople,&tagCards, sizeof(tagCards), MDM_GM_GAME_NOTIFY, ASS_SEND_4_5_CARD, 0);
	LOG_MSG_SEND(ASS_SEND_4_5_CARD);

	if (m_GameData.m_bAutoSendCard)
	{
		if (m_GameData.m_nTurnNums < 3)
		{
			m_GameData.m_nTurnNums++;

			// 发一张公共牌
			ActionRedirect(IDEV_SEND_4_5_CARD, 1200);
		}
		else 
		{
			// 游戏结束, 进行结算比牌
			OutputDebugString("dxh:  OnEventSend1Card中, 游戏结束, 进行结算比牌");
			ActionRedirect(IDEV_COMPARE_CARD, 500);
			return true;
		}
	}
	else
	{
		ActionRedirect(IDEV_BET_START, 500);
	}

	return true;
}

//首次下注事件
bool CServerGameDesk::OnEventBetStart()
{
	OutputDebugString("Server:首次下注事件");
	//首位玩家下注意味当前游戏的活动轮数加1
	m_GameData.m_nTurnNums++;
	for (int i = 0; i < m_GameData.m_nTurnNums; i++)
	{
		if (m_pUserInfo[i] != NULL)
		{
			SendGameData(i, MDM_GM_GAME_NOTIFY, ASS_SORT_OUT, 0);
		}
	}

	SendWatchData(m_bMaxPeople, MDM_GM_GAME_NOTIFY, ASS_SORT_OUT, 0);
	LOG_MSG_SEND(ASS_SORT_OUT);

	// 进行小肓注下注
	//如果当前是第一轮下注，转向大小盲注自动下注流程\
	从第二轮开始则转向下位玩家下注流程(也就从小盲注开始,庄的下位开始)
	//m_GameData.m_byTokenUser = GetLastUserStation(m_GameData.m_bSmallBlind);
	m_GameData.m_byTokenUser = m_GameData.m_byNTUser; //移交令牌至庄位

	CString str;
	str.Format("YY:m_GameData.m_byTokenUser == %d",m_GameData.m_byTokenUser);
	OutputDebugString(str);

	m_GameData.m_iBetCounts = 0;
	m_GameData.m_iCallCounts = 0;
	m_GameData.m_iRaiseCounts = 0;
	//m_GameData.m_iAllinUser = 0;
	m_GameData.m_iCheckCount = 0;
	//m_GameData.m_iFoldCount = 0;
	m_GameData.m_bFirstStation = 255;

	OutputDebugString("Server: 首次下注事件");

    // 新的一轮开始
    m_GameData.m_bNewTurn = true;
	// 新的一轮开始
	m_GameData.m_bNewTurns = true;
	

	if(m_GameData.m_nTurnNums == 1)
	{
		// 第一轮只可以跟注、加注、全下、弃牌
		m_GameData.MakeVerbs(UD_VF_CALL|UD_VF_ADD|UD_VF_FOLD|UD_VF_ALLIN);

		// 进行大小盲注下注
		ActionRedirect(IDEV_BET_BLINDS_AUTO, 500);
	}
	else
	{
		// 第一轮之后每轮开始可以下注、弃牌、过牌、全下
		m_GameData.MakeVerbs(UD_VF_BET|UD_VF_FOLD|UD_VF_CHECK|UD_VF_ALLIN);

		// 新的一轮开始
		ActionRedirect(IDEV_BET_NEXT, 500);
	}

	return true;
}

//大小盲注强制同时下注事件
bool CServerGameDesk::OnEventBetBlindsAuto()
{
	m_GameData.SetMinBetOrRaiseMoney(m_GameData.m_tagDeskCfg.dz.iBigBlindNote);

	TBet tagBet;
	tagBet.nType = ET_AUTO;

	//小盲注强制下注
	m_GameData.m_byTokenUser = m_GameData.m_bSmallBlind;

	//小盲注强制下注
	tagBet.byUser = m_GameData.m_byTokenUser;
	tagBet.nMoney = m_GameData.m_tagDeskCfg.dz.iSmallBlindNote;

	// 模拟下注消息
	OnUserBet(m_GameData.m_bSmallBlind, &tagBet);
	CString str;
	str.Format("dxh: ------------- 下注玩家: %d", m_GameData.m_byTokenUser);
	OutputDebugString(str);

	//大盲注强制下注
	m_GameData.m_byTokenUser = m_GameData.m_bBigBlind;

	//大盲注强制下注
	tagBet.byUser = m_GameData.m_byTokenUser;
	tagBet.nMoney = m_GameData.m_tagDeskCfg.dz.iBigBlindNote;

	// 模拟下注消息
	OnUserBet(m_GameData.m_bBigBlind, &tagBet);

	str.Format("dxh: ------------- 下注玩家: %d", m_GameData.m_byTokenUser);
	OutputDebugString(str);

	// 通知大盲注下一个玩家开始下注
	ActionRedirect(IDEV_BET_NEXT, 500);

	AutoMakeNextPosVerbs(GetNextUserStation(m_GameData.m_byTokenUser));

	return true;
}

//下一位玩家下注事件
bool CServerGameDesk::OnEventBetNext()
{
	// 移交令牌到下一个玩家
	m_GameData.m_byTokenUser = GetNextUserStation(m_GameData.m_byTokenUser);

	const __int64 iTokenUserBetMoney = m_GameData.GetBetMoney(m_GameData.m_byTokenUser);

	//广播令牌
	TToken tagToken;
	tagToken.byUser = m_GameData.m_byTokenUser;
	tagToken.byVerbFlag = m_GameData.m_byVerbFlag;
    tagToken.bNewTurn = m_GameData.m_bNewTurn;
	tagToken.bNewTurns = m_GameData.m_bNewTurns;
	tagToken.nTurnNums = m_GameData.m_nTurnNums;
	tagToken.nCallMoney = m_GameData.m_nMaxBetMoney - iTokenUserBetMoney;
	tagToken.iMinBetOrRaiseMoney = m_GameData.GetMinBetOrRaiseMoney();
	if (m_bIsBuy)
	{
		tagToken.iMaxBetOrRaiseMoney = RC_NUM_ALLIN_LIMIT - m_GameData.m_nMaxBetMoney;
	} else 
	{
		if (m_GameData.m_byTokenUser >= 0 && m_GameData.m_byTokenUser < PLAY_COUNT)
		{
			tagToken.iMaxBetOrRaiseMoney = (iTokenUserBetMoney + m_GameData.m_UserMoney[m_GameData.m_byTokenUser]) - m_GameData.m_nMaxBetMoney;
		} else 
		{
			tagToken.iMaxBetOrRaiseMoney = 0;
		}
	}

	if (tagToken.iMaxBetOrRaiseMoney < tagToken.iMinBetOrRaiseMoney)
	{
		tagToken.iMaxBetOrRaiseMoney = tagToken.iMinBetOrRaiseMoney;
	}

	m_GameData.SetMaxBetOrRaiseMoney(tagToken.iMaxBetOrRaiseMoney);
	
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL)
		{
			SendGameData(i,&tagToken,sizeof(TToken),MDM_GM_GAME_NOTIFY,ASS_TOKEN,0);
		}
	}	

	SendWatchData(m_bMaxPeople,&tagToken,sizeof(TToken),MDM_GM_GAME_NOTIFY,ASS_TOKEN,0);
	LOG_MSG_SEND(ASS_TOKEN, &tagToken, sizeof(tagToken));

    // 开启自动操作流程, 客户端在规则的时间内未操作, 则服务器自动处理操作
	OpenEventProtection(IDEV_BET_NEXT,m_GameData.m_tagDeskCfg.Time.byOperate + 2);
    ActionRedirect(IDEV_AUTO_OPERATE, (m_GameData.m_tagDeskCfg.Time.byOperate + 2) * 1000);
	return true;
}

//开始下一回合游戏事件处理
bool CServerGameDesk::OnEventNextRoundBegin()
{
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL)
		{
			if (!m_GameData.m_UserData[i].m_bIn)
			{
				return true;
			}

			m_pUserInfo[i]->m_UserData.bUserState = USER_ARGEE;
		}
	}

	OutputDebugString("dxh: OnEventNextRoundBegin");
	GameBegin(ALL_ARGEE);
	return true;
}

// 比牌
bool CServerGameDesk::OnCompareCard()
{
	OutputDebugString("dxh: 进行比牌流程");

	// 进行比牌流程
	TCompareCard cmd;
	memset(&cmd, 0, sizeof(TCompareCard));

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] == NULL || m_GameData.m_bGiveUp[i])
		{
			continue;
		}

		// 得到玩家手牌
		memcpy(cmd.bHandCards[i], m_GameData.m_UserData[i].m_byMyCards, sizeof(cmd.bHandCards[i]));
		cmd.nHandCardCount[i] = 2;

		CString str,str1;
		str.Format("jan: 玩家位置: %d, 用户ID:%d, 手牌: ", i, m_pUserInfo[i]->m_UserData.dwUserID);

		for (int j = 0; j < 2; j++)
		{
			BYTE bNumber = m_GameData.m_UserData[i].m_byMyCards[j] & 0x0F;
			BYTE bShape = (m_GameData.m_UserData[i].m_byMyCards[j] & 0xF0) >> 4;
			str1.Format("%s%s(0x%x) ", szShape[bShape], szNumber[bNumber],  m_GameData.m_UserData[i].m_byMyCards[j]);
			str += str1;
		}

		str += "  公共牌:";
		for (int j = 0; j < 5; j++)
		{
			BYTE bNumber = m_GameData.m_iCards[j] & 0x0F;
			BYTE bShape = (m_GameData.m_iCards[j] & 0xF0)>>4;
			str1.Format("%s%s(0x%x) ", szShape[bShape], szNumber[bNumber], m_GameData.m_iCards[j]);
			str += str1;
		}

		OutputDebugString(str);

		BYTE byResultCard[7];
		memset(byResultCard, 0, sizeof(byResultCard));

		// 分析牌型
		cmd.nCardKind[i] = m_Logic.AnalysisCard(m_GameData.m_UserData[i].m_byMyCards, 2, m_GameData.m_iCards, 5, byResultCard);
		cmd.nCardCount = 5;
		memcpy(m_GameData.m_bResultCards[i],byResultCard, sizeof(BYTE) * cmd.nCardCount);  // 五张最大的牌
		memcpy(cmd.bCardsEx[i], &byResultCard[cmd.nCardCount], sizeof(BYTE) * 2);           // 剩余二张没用的牌

		memcpy(cmd.bCards[i], m_GameData.m_bResultCards[i], sizeof(BYTE) * cmd.nCardCount);
		
		
		str.Format("jan: 玩家位置: %d, 用户昵称:%s,  牌型: ID:%d  名称:%s, 牌值: ", i, m_pUserInfo[i]->m_UserData.nickName,  cmd.nCardKind[i], szName[cmd.nCardKind[i]]);
		OutputDebugString(str);

		for (int j = 0; j < 5; j++)
		{
			BYTE bNumber = cmd.bCards[i][j] & 0x0F;
			BYTE bShape = (cmd.bCards[i][j] & 0xF0) >> 4;
			str1.Format("%s%s(0x%x) ", szShape[bShape], szNumber[bNumber], cmd.bCards[i][j]);
			str += str1;
		}

		OutputDebugString(str);
	}

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		// 通知客户端
		if (m_pUserInfo[i] != NULL)
		{
			SendGameData(i, &cmd, sizeof(cmd), MDM_GM_GAME_NOTIFY, ASS_COMPARE_CARD, 0);
		}
	}
	
	SendWatchData(m_bMaxPeople,&cmd, sizeof(cmd), MDM_GM_GAME_NOTIFY, ASS_COMPARE_CARD, 0);
	LOG_MSG_SEND(ASS_COMPARE_CARD);



	OutputDebugString("Server: 进入结算流程");

	// 进入结算流程
	ActionRedirect(IDEV_RESULT, 1500);
	return true;
}

///名称：OnUserAgreeGame
///描述：处理玩家同意游戏
///@param bDeskStation 位置, pNetHead 消息头,pData 消息数据,uSize 消息包长度,uSocketID id,bWatchUser 是否旁观
///@return 
bool CServerGameDesk::OnUserAgreeGame(BYTE bDeskStation,NetMessageHead* pNetHead,void* pData,UINT uSize,UINT uSocketID,bool bWatchUser)
{
	//只要有玩家同意游戏，意味着游戏进入等待其它玩家同意的状态
	//m_bGameStation = GS_WAIT_ARGEE;
	OutputDebugString("dxh: OnUserAgreeGame");
	return __super::HandleNotifyMessage(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
}


///名称：OnUserSetDiZhu
///描述：处理玩家主设置底注
///@param bDeskStation 位置, pNetHead 消息头,pData 消息数据,uSize 消息包长度,uSocketID id,bWatchUser 是否旁观
///@return 
bool CServerGameDesk::OnUserCfgDesk(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	TCfgDesk* pCfgDesk = (TCfgDesk*)pData;

	//这时该玩家应被认定同意游戏
	m_GameData.SetAgreeStateTo(bDeskStation);

	//记录底注值，通过底注计算顶注
	m_GameData.m_tagDeskCfg.dz.iLower = m_GameData.m_tagDeskCfg.dz.iLowers[pCfgDesk->dz.bySelect];
	m_GameData.m_tagDeskCfg.dz.iUpper = m_GameData.m_tagDeskCfg.dz.iUppers[pCfgDesk->dz.bySelect];

	//广播设定底注的消息
	for (int i  = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL)
		{
			SendGameData(i,&m_GameData.m_tagDeskCfg,sizeof(TDeskCfg),MDM_GM_GAME_NOTIFY,ASS_DESK_CFG,0);
		}
	}

	SendWatchData(m_bMaxPeople,&m_GameData.m_tagDeskCfg,sizeof(TDeskCfg),MDM_GM_GAME_NOTIFY,ASS_DESK_CFG,0);
	LOG_MSG_SEND(ASS_DESK_CFG);

	//广播下注后，标志着游戏进入GS_AGREE_NOTE状态（同意底注状态）
	//m_bGameStation = GS_AGREE_NOTE;

	return true;
}

///名称：OnUserSetNoteTimeout
///描述：处理玩家主设置底注超时
///@param bDeskStation 位置, pNetHead 消息头,pData 消息数据,uSize 消息包长度,uSocketID id,bWatchUser 是否旁观
///@return 
bool CServerGameDesk::OnUserCfgDeskTimeout(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	//这时该玩家应被认定不同意游戏，修改用户对象，设用户状态
	m_GameData.SetDisAgreeStateTo(bDeskStation);

	//下发定注超时消息，并解散桌子
	for (int i  = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL && i != bDeskStation)
		{
			SendGameData(i,MDM_GM_GAME_NOTIFY,ASS_CFG_DESK_TIMEOUT,0);
		}
	}
	LOG_MSG_SEND(ASS_CFG_DESK_TIMEOUT);

	GameFinish(0,GF_SALE);

	return true;
}

///名称：OnUserAgreeDiZhu
///描述：处理玩家主同意底注
///@param bDeskStation 位置, pNetHead 消息头,pData 消息数据,uSize 消息包长度,uSocketID id,bWatchUser 是否旁观
///@return 
bool CServerGameDesk::OnUserAgreeDeskCfg(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{

	//if (m_pUserInfo[bDeskStation] == NULL)
	//{
	//	return false;
	//}

	////修改用户对象，设用户状态
	//m_GameData.SetAgreeStateTo(bDeskStation);

	////广播玩家bDeskStation同意消息

	//TAgree tagAgree;
	//tagAgree.byUser = bDeskStation;
	//tagAgree.bAgree = true;

	////是否所有人都同意游戏
	//tagAgree.bAllAgree = (m_GameData.GetAgreeNums() == PLAY_COUNT);

	//for (int i = 0; i < PLAY_COUNT; ++i)
	//{
	//	if (m_pUserInfo[i])
	//	{
	//		SendGameData(i,&tagAgree,sizeof(TAgree),MDM_GM_GAME_NOTIFY,ASS_AGREE,0);
	//	}
	//}
	//LOG_MSG_SEND(ASS_AGREE);

	////如果所有玩家都同意底注的设定，则启动发牌流程
	//if (tagAgree.bAllAgree)
	//{
	//	//广播所有玩家准备好（tagAgree.bAllAgree = true）的通知后，标志着游戏进入GS_PLAY_GAME状态（游戏过程状态）\
	//	  也标志着游戏的进入第一回合，回合计数要加1
	//	
	//	m_bGameStation = GS_PLAY_GAME;

	//	//启动洗牌流程
	//	ActionRedirect(IDEV_RAND_CARD,500);
	//}

	return true;
}

// 玩家代入金币
bool CServerGameDesk::OnUserSubMoney(BYTE bDeskStation, void* pData)
{
	TSubMoney *pCmd = (TSubMoney*)pData;

	if (pCmd == NULL)
	{
		return false;
	}

	if (m_pUserInfo[bDeskStation] == NULL)
	{
		return false;
	}

	if (pCmd->nMoney[bDeskStation] <= m_GameData.m_tagDeskCfg.dz.iUntenMoney
	|| (!m_bIsBuy && m_pUserInfo[bDeskStation]->m_UserData.i64Money < pCmd->nMoney[bDeskStation])
	|| (!m_bIsBuy && m_pUserInfo[bDeskStation]->m_UserData.i64Money < m_GameData.m_tagDeskCfg.dz.iSubMinMoney))
	{
		// 不合法设置
		return true;
	}

	if (pCmd->nMoney[bDeskStation] < m_GameData.m_tagDeskCfg.dz.iSubMinMoney) 
	{
		// 玩家当前带入金额少于最低带入值时，将最低带入值赋予当前带前带入金额
		pCmd->nMoney[bDeskStation] = m_GameData.m_tagDeskCfg.dz.iSubMinMoney;
	}
	else if (pCmd->nMoney[bDeskStation] > m_GameData.m_tagDeskCfg.dz.iSubMaxMoney)
	{
		// 玩家当前带入金额大于最高带入值时，将最高带入值赋予当前带前带入金额
		pCmd->nMoney[bDeskStation] = m_GameData.m_tagDeskCfg.dz.iSubMaxMoney;
	}

	if (!m_bIsBuy && pCmd->nMoney[bDeskStation] > m_pUserInfo[bDeskStation]->m_UserData.i64Money)
	{
		pCmd->nMoney[bDeskStation] = m_pUserInfo[bDeskStation]->m_UserData.i64Money;
	}

	m_pUserInfo[bDeskStation]->m_UserData.bUserState = USER_ARGEE;

	m_GameData.m_UserData[bDeskStation].m_bIn = true;

	// 存入代入金额
	m_GameData.m_UserMoney[bDeskStation] = pCmd->nMoney[pCmd->bDeskStation];
	
	m_i64UserMoney[bDeskStation] = m_GameData.m_UserMoney[bDeskStation];
	// 设置当前可用金币
	TSubMoney pBag;
	pBag.bDeskStation = bDeskStation;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL)
		{
			pBag.bIn[i] = m_GameData.m_UserData[i].m_bIn;
			
			CString str,str1;

			str.Format("Server游戏代入数据-Server:游戏桌号:%d,玩家ID:%d,玩家昵称:%s,玩家代入金额:%I64d,玩家真实金额:%I64d",
				m_bDeskIndex,m_pUserInfo[i]->m_UserData.dwUserID,m_pUserInfo[i]->m_UserData.szName,pCmd->nMoney[i],
				m_pUserInfo[i]->m_UserData.i64Money);

			
			str1.Format("Server: 玩家:%d(位置:%d), 代入金币: %I64d, m_bIn: %d", m_pUserInfo[i]->m_UserData.dwUserID, i, m_GameData.m_UserMoney[i],  m_GameData.m_UserData[i].m_bIn);
			OutputDebugString(str1);
		}
	}

	memcpy(pBag.nMoney,m_GameData.m_UserMoney,sizeof(m_GameData.m_UserMoney));
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL)
		{
			SendGameData(i, &pBag,sizeof(pBag),  MDM_GM_GAME_NOTIFY, ASS_SHOW_IN_MONEY, 0);
		}
	}
	
	SendWatchData(m_bMaxPeople,&pBag,sizeof(pBag),  MDM_GM_GAME_NOTIFY, ASS_SHOW_IN_MONEY, 0);
	LOG_MSG_SEND(ASS_SHOW_IN_MONEY);

	if (CheckCanGameBegin())
	{
		// 游戏开始
		GameBegin(ALL_ARGEE);
	}

	return true;
}

//处理超端设置牌请求
bool CServerGameDesk::OnSuperSetCard(BYTE bDeskStation, const TSuperSetCardReq & tReqAlias)
{
	if (bDeskStation >= PLAY_COUNT)
	{
		return true;
	}

	TSuperSetCardRsp tSuperSetCardRspTemp;
	tSuperSetCardRspTemp.bSuccess = true;

	static const BYTE m_CardArray[NUM_CARDS_NUM]={
		/*2     3     4     5     6    7      8     9     10    J    Q      K     A*/
		0x01, 0x02 ,0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D,		//方块 2 - A
		0x11, 0x12 ,0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,		//梅花 2 - A
		0x21, 0x22 ,0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D,		//红桃 2 - A
		0x31, 0x32 ,0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D,		//黑桃 2 - A
		/*0x4E, 0x4F*/};
	bool m_bCardSetArr[NUM_CARDS_NUM]={false};
	
	//检查公共牌
	int iCardPos = 0;
	bool bGotAtleastOnCard = false;
	for (int i=0; i < NUM_GET_ARR(tReqAlias.byComCards); i++)
	{
		if (CheckCardPosInCardArr(tReqAlias.byComCards[i], iCardPos))
		{
			bGotAtleastOnCard = true;
			if (iCardPos < 0 || iCardPos >= NUM_GET_ARR(m_bCardSetArr))
			{
				tSuperSetCardRspTemp.bSuccess = false;
				tSuperSetCardRspTemp.iErrorReason = ENUM_SuperSetCard_ErrorReason_ErrorComCard;
				break;
			}

			if (m_bCardSetArr[iCardPos])
			{
				tSuperSetCardRspTemp.bSuccess = false;
				tSuperSetCardRspTemp.iErrorReason = ENUM_SuperSetCard_ErrorReason_RepeatedCard;
				break;
			}

			m_bCardSetArr[iCardPos] = true;
		} else 
		{
			/*tSuperSetCardRspTemp.bSuccess = false;
			tSuperSetCardRspTemp.iErrorReason = ENUM_SuperSetCard_ErrorReason_ErrorComCard;
			break;*/
		}
	}

	//检查手牌
	if (tSuperSetCardRspTemp.bSuccess)
	{
		for (int i=0; i < NUM_GET_ARR(tReqAlias.tPlayerHandCards); i++)
		{
			if (!tReqAlias.tPlayerHandCards[i].bSetHandCard)
			{
				continue;
			}

			bool bGotAtleastOneHandCard = false;
			for (int j=0; j < NUM_GET_ARR(tReqAlias.tPlayerHandCards[i].byCard); j++)
			{
				if (CheckCardPosInCardArr(tReqAlias.tPlayerHandCards[i].byCard[j], iCardPos))
				{
					bGotAtleastOnCard = true;
					bGotAtleastOneHandCard = true;
					if (iCardPos < 0 || iCardPos >= NUM_GET_ARR(m_bCardSetArr))
					{
						tSuperSetCardRspTemp.bSuccess = false;
						tSuperSetCardRspTemp.iErrorReason = ENUM_SuperSetCard_ErrorReason_ErrorHandCard;
						break;
					}

					if (m_bCardSetArr[iCardPos])
					{
						tSuperSetCardRspTemp.bSuccess = false;
						tSuperSetCardRspTemp.iErrorReason = ENUM_SuperSetCard_ErrorReason_RepeatedCard;
						break;
					}

					m_bCardSetArr[iCardPos] = true;
				} else 
				{
					/*tSuperSetCardRspTemp.bSuccess = false;
					tSuperSetCardRspTemp.iErrorReason = ENUM_SuperSetCard_ErrorReason_ErrorHandCard;
					break;*/
				}
			}

			if (!bGotAtleastOneHandCard)
			{
				tSuperSetCardRspTemp.bSuccess = false;
				tSuperSetCardRspTemp.iErrorReason = ENUM_SuperSetCard_ErrorReason_ErrorHandCard;
				bGotAtleastOnCard = true;
				break;
			}
		}
	}

	if (!bGotAtleastOnCard)
	{
		tSuperSetCardRspTemp.bSuccess = false;
		tSuperSetCardRspTemp.iErrorReason = ENUM_SuperSetCard_ErrorReason_Max;
	}

	//copy 设定牌
	if (tSuperSetCardRspTemp.bSuccess)
	{
		m_bSuperSetCardEffectNext = tSuperSetCardRspTemp.bSuccess;
		m_tSuperSetCardReqNext = tReqAlias;
	}

	SendGameData(bDeskStation, &tSuperSetCardRspTemp, sizeof(tSuperSetCardRspTemp), MDM_GM_GAME_NOTIFY, ASS_SUPER_SET_CARD_RSP, 0);

	return true;
}

//玩家下注选择正确与否检测
bool CServerGameDesk::CheckOptionIsRightOnUserBet(emType nType)
{
	bool bCheckSuccess = false;

	switch (nType)
	{
	case ET_BET:
		if (m_GameData.m_byVerbFlag & UD_VF_BET)
		{
			bCheckSuccess = true;
		}
		break;
	case ET_CALL:
		if (m_GameData.m_byVerbFlag & UD_VF_CALL)
		{
			bCheckSuccess = true;
		}
		break;
	case ET_ADDNOTE:
		if (m_GameData.m_byVerbFlag & UD_VF_ADD)
		{
			bCheckSuccess = true;
		}
		break;
	case ET_CHECK:
		if (m_GameData.m_byVerbFlag & UD_VF_CHECK)
		{
			bCheckSuccess = true;
		}
		break;
	case ET_FOLD:
		if (m_GameData.m_byVerbFlag & UD_VF_FOLD)
		{
			bCheckSuccess = true;
		}
		break;
	case ET_ALLIN:
		if (m_GameData.m_byVerbFlag & UD_VF_ALLIN)
		{
			bCheckSuccess = true;
		}
		break;
	default:
		break;
	}

	return bCheckSuccess;
}

//更新有效加注信息
bool CServerGameDesk::UpdateValidRaiseOnUserBet(const BYTE& bDeskStation, const __int64& nAllBetMoney, const __int64& nRaiseMoneyTemp, const __int64& nAllBetMoneyValidRaisePos,  bool bForceUpdate)
{
	if (bDeskStation >= PLAY_COUNT)
	{
		return false;
	}

	//判断是否有效加注
	const BYTE byValidRaisePos = m_GameData.GetValidRaisePos();
	const __int64 iMinBetOrRaiseMoney = m_GameData.GetMinBetOrRaiseMoney();
	const __int64 nDeskStationAllBetMoney = m_GameData.GetBetMoney(bDeskStation);
	if (byValidRaisePos < PLAY_COUNT && !bForceUpdate)
	{
		if (nAllBetMoney - nAllBetMoneyValidRaisePos >= iMinBetOrRaiseMoney)
		{
			m_GameData.SetMinBetOrRaiseMoney(nAllBetMoney - nAllBetMoneyValidRaisePos);
			m_GameData.SetValidRaisePos(bDeskStation);
			m_GameData.SetValidRaisePosEffectbet(nDeskStationAllBetMoney);
		}
	} else 
	{
		m_GameData.SetMinBetOrRaiseMoney(nRaiseMoneyTemp);
		m_GameData.SetValidRaisePos(bDeskStation);
		m_GameData.SetValidRaisePosEffectbet(nDeskStationAllBetMoney);
	}

	return true;
}

///名称：OnUserBet
///描述：处理玩家下注信息
///@param bDeskStation 位置, pData 消息数据
///@return 
bool CServerGameDesk::OnUserBet(const BYTE bDeskStation, void* pData, bool bUserforceExit, const bool bCheckOption)
{
	OutputDebugString("Server: 处理玩家下注信息");
	
	// 不是当前操作玩家
	if (m_GameData.m_byTokenUser != bDeskStation)
	{
		OutputDebugString("Server: 不是当前操作玩家");
		return false;
	}
	// 玩家不存在 或 玩家已弃牌 或 不可下注玩家 或 中途进入玩家
	if (m_pUserInfo[bDeskStation] == NULL || m_GameData.m_bGiveUp[bDeskStation] || m_GameData.m_bNoBet[bDeskStation])
	{
		OutputDebugString("Server: 玩家不存在或玩家已弃牌");
		return true;
	}

	if (m_GameData.m_bAutoSendCard)
	{
		OutputDebugString("Server: 已经发所有牌, 所有玩家都不可进行操作, 等待游戏结束操作");
		return true;
	}

	CString str;

	TBet * pCmd = (TBet *)pData;

	if (NULL == pCmd)
	{
		return true;
	}

	//检查行为选择是否正确
	if (bCheckOption)
	{
		if (!CheckOptionIsRightOnUserBet(pCmd->nType))
		{
			return false;
		}
	}

	// 停止自动定时操作
	KillMyTimer(IDEV_AUTO_OPERATE);

	const __int64 iMinBetOrRaiseMoney = m_GameData.GetMinBetOrRaiseMoney();
	const BYTE byPreValidRaisePos = m_GameData.GetValidRaisePos(); //上一个有效加注位置
	const emType nOriginalType = pCmd->nType;				// 下注类型
	__int64 nAllBetMoneyValidRaisePos = 0;
	if (byPreValidRaisePos < PLAY_COUNT)
	{
		nAllBetMoneyValidRaisePos = m_GameData.GetBetMoney(byPreValidRaisePos); //必须在更新下注额度之前获取之前有效加注位置下注的信息
	}
	bool bFlagRCAllInTemp = false; //房卡AllIn标记(只用于下注计算过程)

	const __int64 i64CallMoney = m_GameData.m_nMaxBetMoney - m_GameData.GetBetMoney(bDeskStation);
	switch (pCmd->nType)
	{
	case ET_BET:     // 下注
	case ET_AUTO:
		{
			//判断最小额度下注
			if (ET_AUTO != pCmd->nType)
			{
				if (pCmd->nMoney < iMinBetOrRaiseMoney)
				{
					pCmd->nMoney = iMinBetOrRaiseMoney;
				}
			}

			if (pCmd->nMoney < 0 || pCmd->nMoney > m_GameData.m_UserMoney[bDeskStation])
			{
				// 下注额超过本人自身金币, 则把自己自身金币全下
				pCmd->nMoney = m_GameData.m_UserMoney[bDeskStation];
				str.Format("Server:下注额超过本人自身金币, 则把自己自身金币全下:%I64d",pCmd->nMoney);
				OutputDebugString(str);
			}

			m_GameData.m_UserMoney[bDeskStation] -= pCmd->nMoney;
			m_GameData.m_nBetMoney[bDeskStation][m_GameData.m_nBetPool] += pCmd->nMoney;

			AllInLimitRCEffectOnUserBet(bDeskStation, bFlagRCAllInTemp);

			if (bFlagRCAllInTemp || m_GameData.m_UserMoney[bDeskStation] == 0)
			{
				// 当作全下处理
				m_GameData.m_bNoBet[bDeskStation] = true;
				pCmd->nType = ET_ALLIN;
				m_GameData.m_bAllBetStation = bDeskStation;
				m_GameData.m_iAllinUser++;
				m_GameData.m_byAllPools[m_GameData.m_nBetPool]++;
				m_GameData.m_bAllBet[bDeskStation][m_GameData.m_nBetPool] = true;
			}

			__int64 nAllBetMoney = m_GameData.GetBetMoney(bDeskStation);
			if (m_GameData.m_nMaxBetMoney < nAllBetMoney)
			{
				const __int64 nRaiseMoneyTemp = nAllBetMoney - m_GameData.m_nMaxBetMoney;
				m_GameData.m_nMaxBetMoney = nAllBetMoney;  // 记录最多的金币

				UpdateValidRaiseOnUserBet(bDeskStation, nAllBetMoney, nRaiseMoneyTemp, nAllBetMoneyValidRaisePos,
										(ET_AUTO == nOriginalType ? true : false)); //ET_AUTO 时强制更新

				if (ET_AUTO != pCmd->nType)
				{
					m_GameData.m_bRaiseStation = bDeskStation;
				}
			}
			
			m_GameData.m_bBetStation = bDeskStation;
			m_GameData.m_iBetCounts++;
			
			CString str;
			str.Format("Server:m_GameData.m_bBetStation == %d,bDeskStation == %d",m_GameData.m_bBetStation,bDeskStation);
			OutputDebugString(str);
		}
		break;
	case ET_CALL:    // 跟注
		{
			// 2012-10-11 duanxiaohui add 防止客户端乱跟金币, 保证跟注为一轮中最大的下注
			__int64 nAllMoeny = m_GameData.GetBetMoney(bDeskStation);
			//if ((nAllMoeny + pCmd->nMoney) < m_GameData.m_nMaxBetMoney)
			//{
			//	pCmd->nMoney = m_GameData.m_nMaxBetMoney - nAllMoeny;   // 跟注异常了, 这里计算出应该要跟的金币
			//}		
			//// end duanxiaohui
			//自动计算跟注的金币
			if (m_GameData.m_nMaxBetMoney >= nAllMoeny)
			{
				pCmd->nMoney = m_GameData.m_nMaxBetMoney - nAllMoeny;
			}

			if (pCmd->nMoney < 0 || pCmd->nMoney > m_GameData.m_UserMoney[bDeskStation])
			{
				// 跟注额超过本人自身金币, 则把自己自身金币全跟
				pCmd->nMoney = m_GameData.m_UserMoney[bDeskStation];
			}

			m_GameData.m_UserMoney[bDeskStation] -= pCmd->nMoney;
			m_GameData.m_nBetMoney[bDeskStation][m_GameData.m_nBetPool] += pCmd->nMoney;
			AllInLimitRCEffectOnUserBet(bDeskStation, bFlagRCAllInTemp);

			if (bFlagRCAllInTemp || m_GameData.m_UserMoney[bDeskStation] == 0)
			{
				// 当作全下处理
				m_GameData.m_bNoBet[bDeskStation] = true;
				pCmd->nType = ET_ALLIN;
				m_GameData.m_bAllBetStation = bDeskStation;
				m_GameData.m_iAllinUser++;
				m_GameData.m_byAllPools[m_GameData.m_nBetPool]++;
				m_GameData.m_bAllBet[bDeskStation][m_GameData.m_nBetPool] = true;
			}
			else
			{
				m_GameData.m_iCallCounts++;
			}

			__int64 nAllBetMoney = m_GameData.GetBetMoney(bDeskStation);
			if (m_GameData.m_nMaxBetMoney < nAllBetMoney)
			{
				const __int64 nRaiseMoneyTemp = nAllBetMoney - m_GameData.m_nMaxBetMoney;
				m_GameData.m_nMaxBetMoney = nAllBetMoney;  // 记录最多的金币

				UpdateValidRaiseOnUserBet(bDeskStation, nAllBetMoney, nRaiseMoneyTemp, nAllBetMoneyValidRaisePos);

				m_GameData.m_bRaiseStation = bDeskStation;
			}
		}
		break;
	case ET_ADDNOTE: // 加注    
		{ 
			//判断最小额度加注
			if (pCmd->nMoney < i64CallMoney + iMinBetOrRaiseMoney)
			{
				pCmd->nMoney = i64CallMoney + iMinBetOrRaiseMoney;
			}

			if (pCmd->nMoney < 0 || pCmd->nMoney > m_GameData.m_UserMoney[bDeskStation])
			{
				// 加注额超过本人自身金币, 则把自己自身金币全加
				pCmd->nMoney = m_GameData.m_UserMoney[bDeskStation];
			}

			m_GameData.m_UserMoney[bDeskStation] -= pCmd->nMoney;
			m_GameData.m_nBetMoney[bDeskStation][m_GameData.m_nBetPool] += pCmd->nMoney;
			AllInLimitRCEffectOnUserBet(bDeskStation, bFlagRCAllInTemp);

			if (bFlagRCAllInTemp || m_GameData.m_UserMoney[bDeskStation] == 0)
			{
				// 当作全下处理
				m_GameData.m_bNoBet[bDeskStation] = true;
				pCmd->nType = ET_ALLIN;
				m_GameData.m_bAllBetStation = bDeskStation;
				m_GameData.m_iAllinUser++;
				m_GameData.m_byAllPools[m_GameData.m_nBetPool]++;
				m_GameData.m_bAllBet[bDeskStation][m_GameData.m_nBetPool] = true;
			}
			else
			{
				m_GameData.m_iRaiseCounts++;
			}

			__int64 nAllBetMoney = m_GameData.GetBetMoney(bDeskStation);
			if (m_GameData.m_nMaxBetMoney < nAllBetMoney)
			{
				const __int64 nRaiseMoneyTemp = nAllBetMoney - m_GameData.m_nMaxBetMoney;
				m_GameData.m_nMaxBetMoney = nAllBetMoney;  // 记录最多的金币

				UpdateValidRaiseOnUserBet(bDeskStation, nAllBetMoney, nRaiseMoneyTemp, nAllBetMoneyValidRaisePos);

				m_GameData.m_bRaiseStation = bDeskStation;
			}

			CString str;
			str.Format("Server:m_GameData.m_bRaiseStation == %d,bDeskStation == %d",m_GameData.m_bRaiseStation,bDeskStation);
			OutputDebugString(str);
		}
		break;
	case ET_ALLIN:   // 全下
		{
			pCmd->nMoney = m_GameData.m_UserMoney[bDeskStation];
			
			str.Format("Server:m_UserMoney = %I64d",m_GameData.m_UserMoney[bDeskStation]);
			OutputDebugString(str);

			m_GameData.m_bNoBet[bDeskStation] = true;
			m_GameData.m_bAllBetStation = bDeskStation;

			m_GameData.m_UserMoney[bDeskStation] -= pCmd->nMoney;
			m_GameData.m_nBetMoney[bDeskStation][m_GameData.m_nBetPool] += pCmd->nMoney; 
			AllInLimitRCEffectOnUserBet(bDeskStation, bFlagRCAllInTemp);

			m_GameData.m_iAllinUser++;
			m_GameData.m_byAllPools[m_GameData.m_nBetPool]++;
			m_GameData.m_bAllBet[bDeskStation][m_GameData.m_nBetPool] = true;

			__int64 nAllBetMoney = m_GameData.GetBetMoney(bDeskStation);
			if (m_GameData.m_nMaxBetMoney < nAllBetMoney)
			{
				const __int64 nRaiseMoneyTemp = nAllBetMoney - m_GameData.m_nMaxBetMoney;
				m_GameData.m_nMaxBetMoney = nAllBetMoney;  // 记录最多的金币

				UpdateValidRaiseOnUserBet(bDeskStation, nAllBetMoney, nRaiseMoneyTemp, nAllBetMoneyValidRaisePos);
				
				m_GameData.m_bRaiseStation = bDeskStation;
			}
		}
		break; 
	case ET_CHECK:   // 过牌
		{
			if (m_GameData.m_bFirstStation == 255)
			{
				m_GameData.m_bFirstStation = bDeskStation;  // 记录第一个过牌玩家
			}

			m_GameData.m_iCheckCount++;
		}
		break;
	case ET_FOLD:    // 弃牌
		{
			m_GameData.m_bGiveUp[bDeskStation] = true;

			if(!bUserforceExit)
			{
				m_GameData.m_iFoldCount++;
			}

			if (m_GameData.m_byNTUser == bDeskStation)
			{
				m_GameData.m_byNTUser = 255;
			}
			else if (m_GameData.m_bSmallBlind == bDeskStation)
			{
				// 小盲注玩家弃牌, 把小盲注至为255
				m_GameData.m_bSmallBlind = 255;
			}
			else if (m_GameData.m_bBigBlind == bDeskStation)
			{
				// 大盲注玩家弃牌, 把大盲注至为255
				m_GameData.m_bBigBlind = 255;
			}
			
		}
		break;
	default:
		break;
	}

	str.Format("Server: 玩家:%d, 下注:%I64d, 第%I64d个下注池, 剩余金币:%I64d",bDeskStation, pCmd->nMoney,m_GameData.m_nBetPool+1, m_GameData.m_UserMoney[bDeskStation]);
	OutputDebugString(str);

	// 将下注数据发送到客户端显示
	TBet tagAnteVerb;
	tagAnteVerb.byUser = bDeskStation;
	tagAnteVerb.nType = pCmd->nType;
	tagAnteVerb.nMoney = pCmd->nMoney;
	//memcpy(tagAnteVerb.bUserman,m_GameData.m_bGiveUp,sizeof(m_GameData.m_bGiveUp));
	tagAnteVerb.nBetMoney = m_GameData.m_nMaxBetMoney;

	str.Format("Server:下注金额：%I64d",tagAnteVerb.nBetMoney);
	OutputDebugString(str);

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL)
		{
			SendGameData(i, &tagAnteVerb, sizeof(tagAnteVerb), MDM_GM_GAME_NOTIFY, ASS_BET_INFO,0);
		}
	}

	SendWatchData(m_bMaxPeople,&tagAnteVerb, sizeof(tagAnteVerb), MDM_GM_GAME_NOTIFY, ASS_BET_INFO,0);
	LOG_MSG_SEND(ASS_BET_INFO, &tagAnteVerb, sizeof(tagAnteVerb));
	CloseEventProtection();

	// 判断下一步操作
	if (pCmd->nType != ET_AUTO)
	{
		if (m_GameData.m_iFoldCount >= GetPlayerCount(true) - 1)
		{
			OutputDebugString("Server: 所有玩家弃牌, 游戏结束");
			m_GameData.m_bGiveUpOver = true;

#ifdef CLOSE_CARD
			for(int i = 0; i < PLAY_COUNT; ++i)
			{
				//玩家在游戏中 并且没有 放弃的玩家 发送能展示牌的消息
				if(NULL != m_pUserInfo[i] && !m_GameData.m_bGiveUp[i])
				{
					SendGameData(i,NULL,0,MDM_GM_GAME_NOTIFY,ASS_CAN_SHOW_CARD,0);
				}
			}
			LOG_MSG_SEND(ASS_CAN_SHOW_CARD);

			// 所有玩家弃牌, 游戏结束
			ActionRedirect(IDEV_RESULT, 4000);
#else
			// 所有玩家弃牌, 游戏结束
			ActionRedirect(IDEV_RESULT, 1000);
#endif
			return true;
		}

		bool bEqual = false;

		//if (m_GameData.m_bBigBlind == bDeskStation)		// 玩家位置等于大盲注
		{
			/*CString str;
			str.Format("Server:玩家位置等于大盲注__玩家%d全下%I64d",m_GameData.m_nBetMoney[bDeskStation][m_GameData.m_nBetPool]);
			OutputDebugString(str);*/

			bEqual = true;
			__int64 nMoney = m_GameData.m_nBetMoney[bDeskStation][m_GameData.m_nBetPool];
			__int64 nCurNoAllInMoney = 0;
			bool bNoAllInMoneyInitFlag = false;
			//检测未全下人下注是否相同
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (m_pUserInfo[i] == NULL || m_GameData.m_bGiveUp[i] || m_GameData.m_bNoBet[i])
				{
					continue;
				}
			
				if (!bNoAllInMoneyInitFlag)
				{
					bNoAllInMoneyInitFlag = true;
					nCurNoAllInMoney = m_GameData.m_nBetMoney[i][m_GameData.m_nBetPool];
				} else if (nCurNoAllInMoney != m_GameData.m_nBetMoney[i][m_GameData.m_nBetPool])
				{
					bEqual = false;
					break;
				}
			}

			//检测未全下人下注是否大于等于全下值
			if (bEqual && bNoAllInMoneyInitFlag)
			{
				for (int i = 0; i < PLAY_COUNT; i++)
				{
					if (m_pUserInfo[i] == NULL || m_GameData.m_bGiveUp[i])
					{
						continue;
					}

					//获取全下最大值
					if (m_GameData.m_bNoBet[i])
					{
						if (nCurNoAllInMoney < m_GameData.m_nBetMoney[i][m_GameData.m_nBetPool])
						{
							bEqual = false;
							break;
						}
					}
				}
			}

			CString str;
			if (!bEqual)
			{
				int nCount = 0;
				BYTE byStation = -1;
				for (int i = 0; i < PLAY_COUNT; i++)
				{
					if (m_pUserInfo[i] == NULL || m_GameData.m_bGiveUp[i] || m_GameData.m_bNoBet[i])
					{
						continue;
					}

					nCount++;
					byStation = i;
				}

				// 如果只剩下一个人, 判断是否还需要继续下注操作
				if (nCount == 1)
				{
					bEqual = true;
					nMoney = m_GameData.m_nBetMoney[byStation][m_GameData.m_nBetPool];

					for (int i = 0; i < PLAY_COUNT; i++)
					{
						if (m_pUserInfo[i] == NULL || m_GameData.m_bGiveUp[i] || (m_GameData.m_bNoBet[i] && !m_GameData.m_bAllBet[i][m_GameData.m_nBetPool]))
						{
							continue;
						}

						// 判断最后一个玩家下注的金币是否大于全下的玩家下注金币, 大于则直接结算，小于需要继续下注
						if (nMoney < m_GameData.m_nBetMoney[i][m_GameData.m_nBetPool])
						{
							bEqual = false;
							break;
						}	
					}
				}
			
				str.Format("Server: ============nCount =  %d", nCount);
				OutputDebugString(str);
			}

			str.Format("Server: ============bEqual   %d", bEqual);
			OutputDebugString(str);
		}

		// 新的一轮标志置为false
		m_GameData.m_bNewTurn = false;
		
		// 加注玩家的上家结束, 开始新的一轮下注或轮到大盲注时(前面没有人加注), 开始新的一轮下注
		BYTE bNextStation = GetNextUserStation(bDeskStation);
		//BYTE bNextStation = GetNextUserStation(bDeskStation);

		str.Format("Server: m_GameData.m_bBetStation=%d,m_GameData.m_bRaiseStation=%d, 下注人: %d, 大: %d, bEqual=%d, m_GameData.m_iBetCounts: %d, 当前下注第%d轮,第一个过牌玩家%d,GetNextUserStation(bDeskStation) == %d,bNextStation == %d",
			m_GameData.m_bBetStation,m_GameData.m_bRaiseStation, bDeskStation, m_GameData.m_bBigBlind, bEqual, m_GameData.m_iBetCounts, m_GameData.m_nTurnNums, m_GameData.m_bFirstStation,GetNextUserStation(bDeskStation),bNextStation);
		OutputDebugString(str);

		bool temp = false;
		if (m_GameData.m_nTurnNums == 1)
		{
			if (m_GameData.m_bRaiseStation < PLAY_COUNT && m_GameData.m_bNoBet[m_GameData.m_bRaiseStation])//最后加注位置已经AllIn即可以检测结束
			{
				temp = true;
			} else if (m_GameData.m_bNoBet[m_GameData.m_bBigBlind])//大盲位已经AllIn即可以检测结束
			{
				temp = true;
			} else if (m_GameData.m_bBigBlind == bDeskStation)//大盲位说话时可以检测结束
			{
				temp = true;
			}
			else if (m_GameData.m_bBigBlind != bNextStation)//下一个非大盲位可以检测结束
			{
				temp = true;
			} else//其他情况
			{
				temp = false;
			}
		}
		else
		{
			if (m_GameData.m_bRaiseStation < PLAY_COUNT && m_GameData.m_bNoBet[m_GameData.m_bRaiseStation])//最后加注位置已经AllIn即可以检测结束
			{
				temp = true;
			} if (m_GameData.m_iBetCounts > 1)//有加注检测结束
			{
				temp = true;
			} else if ((m_GameData.m_bFirstStation == bNextStation
				|| m_GameData.m_bRaiseStation == bNextStation
				|| m_GameData.m_bBetStation == bNextStation))//下一个位置是第一个过、下注或最后一个实际加注位置时检测结束
			{
				temp = true;
			}
			else//其他情况
			{
				temp = false;
			}
		}

		if ((m_GameData.m_bRaiseStation != 255 && bNextStation == m_GameData.m_bRaiseStation)//最后一个加注的玩家说话
			|| (bEqual && temp) //下注额度相同时与特殊位置的检测
			|| (m_GameData.m_iCheckCount > 1 && m_GameData.m_bFirstStation == bNextStation && bEqual) // 全过
			|| bNextStation == 255) // (找不到下一个位置) 全下
		{
			m_GameData.m_iCheckCount = 0;

			m_GameData.ReSetMinBetOrRaiseMoney();
			m_GameData.ReSetMaxBetOrRaiseMoney();
			m_GameData.ReSetValidRaisePos();
			m_GameData.ReSetValidRaisePosEffectbet();
			//m_GameData.SetValidRaisePosForceNextTurn();

			// 新的一轮标志置为false
			m_GameData.m_bNewTurns = false;
			OutputDebugString("Server:加注玩家的上家结束, 开始新的一轮下注或轮到大盲注时(前面没有人加注), 开始新的一轮下注");
			// 有人全下之后, 需要计算边池
			if (m_GameData.m_byAllPools[m_GameData.m_nBetPool] > 0)
			{
				// 进行边池计算
				str.Format("Server: 第%I64d个边池有人全下人数:%d, 需要进行边池产生处理", m_GameData.m_nBetPool+1, m_GameData.m_byAllPools[m_GameData.m_nBetPool]);
				OutputDebugString(str);

				int i, j;
				bool bIsNewPool = false;
				__int64 nBetPool = m_GameData.m_nBetPool;
				for (i = 0; i < m_GameData.m_byAllPools[m_GameData.m_nBetPool]; i++)
				{
					// 查找池中最小下注金币, 做为下注池上限
					__int64 nMinMoney = /*2147483647*/LLONG_MAX;//9223372036854775807;
					bIsNewPool = false;

					for (j = 0; j < PLAY_COUNT; j++)
					{
						if (m_pUserInfo[j] == NULL || m_GameData.m_bGiveUp[j] || m_GameData.m_nBetMoney[j][nBetPool] <= 0/* || m_GameData.m_bNoBet[j]*/)
						{
							continue;
						}

						if (m_GameData.m_nBetMoney[j][nBetPool] < nMinMoney)
						{
							nMinMoney = m_GameData.m_nBetMoney[j][nBetPool];
						}
					}

					CString str;
					str.Format("Server: 当前%d边池, 最按最低金币:%I64d 进行分池", nBetPool, nMinMoney);
					OutputDebugString(str);

					for (j = 0; j < PLAY_COUNT; j++)
					{
						if (m_pUserInfo[j] == NULL || m_GameData.m_bGiveUp[j] || m_GameData.m_nBetMoney[j][nBetPool] <= 0/* || m_GameData.m_bNoBet[j]*/)
						{
							continue;
						}

						str.Format("Server: 玩家:%d, 当前下注金币:%I64d", m_pUserInfo[j]->m_UserData.dwUserID, m_GameData.m_nBetMoney[j][nBetPool]);
						OutputDebugString(str);

						// 判断该玩家下注的金币是否大于全压的玩家金币
						if (m_GameData.m_nBetMoney[j][nBetPool] > nMinMoney)
						{
							// 把多出的金币转到边池去
							__int64 nPoolMoney = m_GameData.m_nBetMoney[j][nBetPool] - nMinMoney;
							m_GameData.m_nBetMoney[j][nBetPool] -= nPoolMoney;
							m_GameData.m_nBetMoney[j][nBetPool+1] = nPoolMoney;

							str.Format("Server: 玩家:%d, 分池后: %I64d, %I64d", m_pUserInfo[j]->m_UserData.dwUserID, m_GameData.m_nBetMoney[j][nBetPool], nPoolMoney);
							OutputDebugString(str);

							str.Format("Server: 分池金币: %I64d", nPoolMoney);
							OutputDebugString(str);
							
							bIsNewPool = true;
						} else if (m_GameData.m_nBetMoney[j][nBetPool] == nMinMoney) //有AllIn就要算边池
						{
							bIsNewPool = true;
						}
					}

					if (!bIsNewPool)
					{
						// 没有新边池产生, 则跳出
						break;
					}

					// 边池位置移到下一个边池
					nBetPool++;
				}

				m_GameData.m_nBetPool = nBetPool;
			}

			//一圈结束后发送边池信息
			TBetPool tagTbetPool;

			// 更新下池信息到客户端
			for (int i = 0; i <= m_GameData.m_nBetPool; i++)
			{
				// 把8个下注池的金币发到客户端
				tagTbetPool.iBetPools[i] = m_GameData.GetBetMoney(i, false);
				CString str;
				str.Format("Server: 第%d个边池金币: %I64d", i, tagTbetPool.iBetPools[i]);
				OutputDebugString(str);
			}

			for (int i = 0; i < PLAY_COUNT; i++)
			{
				// 发送
				if (m_pUserInfo[i] != NULL)
				{
					SendGameData(i, &tagTbetPool, sizeof(tagTbetPool), MDM_GM_GAME_NOTIFY, ASS_BETPOOL_UP, 0);
				}
			}

			SendWatchData(m_bMaxPeople,&tagTbetPool, sizeof(tagTbetPool), MDM_GM_GAME_NOTIFY, ASS_BETPOOL_UP, 0);
			LOG_MSG_SEND(ASS_BETPOOL_UP);

			OutputDebugString("Server: 进入下一轮发牌操作");

			int nCount = 0;
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (m_pUserInfo[i] == NULL || m_GameData.m_bGiveUp[i] || m_GameData.m_bNoBet[i]/* || i == bDeskStation*/)
				{
					continue;
				}

				nCount++;
			}

			if (nCount <= 1)
			{
				// 全下之后，没人可以下注了，需要把剩余的牌全发了
				OutputDebugString("Server: 全下之后，没人可以下注了，需要把剩余的牌全发了");
				m_GameData.m_bAutoSendCard = true;
			}

			// 没有加注, 下一轮开始
			if (m_GameData.m_nTurnNums == 1)
			{
				// 发三张公共牌
				ActionRedirect(IDEV_SEND_3_CARD, 500);
			}
			else if (m_GameData.m_nTurnNums < 4)
			{
				// 发一张公共牌
				ActionRedirect(IDEV_SEND_4_5_CARD, 500);
			}
			else 
			{
				// 游戏结束, 进行结算比牌
				OutputDebugString("Server:  游戏结束, 进行结算比牌");
				ActionRedirect(IDEV_COMPARE_CARD, 500);
				return true;
			}

			// 重置下注位置
			m_GameData.m_bRaiseStation = -1;
			m_GameData.m_bBetStation = 255;
		}
		else
		{
			OutputDebugString("Server: 下一个玩家操作");

			int nCount = 0;
			for (int i = 0; i < PLAY_COUNT; i++)
			{
				if (m_pUserInfo[i] == NULL || m_GameData.m_bGiveUp[i] || m_GameData.m_bNoBet[i] || i == bDeskStation)
				{
					continue;
				}

				nCount++;
			}

			if (nCount <= 0)
			{
				OutputDebugString("Server:  下一个玩家操作后, 其他玩家放弃或全下(没有可操作玩家了), 游戏结束");
				m_GameData.m_bAutoSendCard = true;

				// 没有加注, 下一轮开始
				if (m_GameData.m_nTurnNums == 1)
				{
					// 发三张公共牌
					ActionRedirect(IDEV_SEND_3_CARD, 500);
				}
				else if (m_GameData.m_nTurnNums < 4)
				{
					// 发一张公共牌
					ActionRedirect(IDEV_SEND_4_5_CARD, 500);
				}
				else 
				{
					// 游戏结束, 进行结算比牌
					OutputDebugString("Server:  游戏结束, 进行结算比牌");
					ActionRedirect(IDEV_COMPARE_CARD, 500);
				}

				return true;
			}

			//bool temps = true;
			////bool bCheckSpecialMustAdd = false;
			//if (m_GameData.m_nTurnNums == 1)
			//{
			//	OutputDebugString("Server:m_GameData.m_nTurnNums == 1");
			//	if (m_GameData.m_iBetCounts > 2)
			//	{
			//		temps = true;
			//	}
			//	else if (m_GameData.m_bBigBlind == bNextStation)
			//	{
			//		temps = false;
			//		//bCheckSpecialMustAdd = true;
			//	}
			//}
			//else
			//{
			//	if (m_GameData.m_iBetCounts > 1)
			//	{
			//		temps = true;
			//	}
			//	else if (m_GameData.m_bBigBlind == bNextStation)
			//	{
			//		temps = false;
			//	}
			//}

			//if (/*m_GameData.m_iBetCounts > 0*/temps)
			AutoMakeNextPosVerbs(bNextStation);

			// 继续下一个玩家下注
			ActionRedirect(IDEV_BET_NEXT, 500);
		}
	}

	return true;
}

//玩家下注信息
bool CServerGameDesk::AllInLimitRCEffectOnUserBet(const BYTE bDeskStation, bool &bFlagRCAllInTemp)
{
	if (bDeskStation >= PLAY_COUNT)
	{
		return false;
	}

	if (m_bIsBuy)
	{
		__int64 nAllBetMoneyTemp = m_GameData.GetBetMoney(bDeskStation);
		if (nAllBetMoneyTemp >= RC_NUM_ALLIN_LIMIT)
		{
			bFlagRCAllInTemp = true;
			__int64 iMoneyGetBack = nAllBetMoneyTemp - RC_NUM_ALLIN_LIMIT;

			m_GameData.m_UserMoney[bDeskStation] += iMoneyGetBack;
			m_GameData.m_nBetMoney[bDeskStation][m_GameData.m_nBetPool] -= iMoneyGetBack;
		}
	}

	return true;
}


//处理玩家发牌完成
bool CServerGameDesk::OnUserSendCardFinish(BYTE bDeskStation,NetMessageHead* pNetHead,void* pData,UINT uSize,UINT uSocketID,bool bWatchUser)
{
	// 不需要客户端同步
	//return true;

	////为第bDeskStation个玩家置发玩牌的标记
	m_GameData.m_UserData[bDeskStation].m_bSendCardFinish = true;

	////如果还有玩家未发完牌，则不能继续流程
	if (!m_GameData.IsAllPlayerSendCardFinish())
	{
		return true;
	}

	////发牌完成开始首次下注事件处理
	OutputDebugString("Server: 处理玩家发牌完成");
	
	if (m_bFaiPaiFinish == false)
	{
		ActionRedirect(IDEV_BET_START,1);
	}
	m_bFaiPaiFinish = true;

	return true;
}

//处理玩家对下一回合准备的信号
bool CServerGameDesk::OnUserNextRoundReq(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	TNextRoundReq* pNextRoundReq = (TNextRoundReq*)pData;

	if (m_pUserInfo[bDeskStation] == NULL)
	{
		return false;
	}

	TAgree tagAgree;
	tagAgree.byUser = bDeskStation;

	if (pNextRoundReq->nType == TNextRoundReq::READY)
	{
		//m_emUserAgree[bDeskStation] = US_AGREE;
		tagAgree.bAgree = true;
	}
	else if (pNextRoundReq->nType == TNextRoundReq::EXIT)
	{
		//m_emUserAgree[bDeskStation] = US_DISAGREE;
		tagAgree.bAgree = false;
	}
	else
	{
		return true;
	}

	//广播该玩家是否同意继续游戏的消息
	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		if (m_pUserInfo[i] != NULL)
		{
			SendGameData(i,&tagAgree,sizeof(TAgree),MDM_GM_GAME_NOTIFY,ASS_AGREE,0);
		}
	}

	SendWatchData(m_bMaxPeople,&tagAgree,sizeof(TAgree),MDM_GM_GAME_NOTIFY,ASS_AGREE,0);
	LOG_MSG_SEND(ASS_AGREE);

	//只要有一个人退出游戏，则无法进行游戏，通知在仍在桌上的玩家有玩家退出，并结束游戏

	if (pNextRoundReq->nType == TNextRoundReq::EXIT)
	{
		TNoPlayer tagNoPlayer;
		tagNoPlayer.bGameFinished = false;
		for (int i = 0; i < PLAY_COUNT; ++i)
		{
			if (m_pUserInfo[i])
			{
				SendGameData(i,&tagNoPlayer,sizeof(TNoPlayer),MDM_GM_GAME_NOTIFY,ASS_NO_PLAYER,0);
			}
		}

		SendWatchData(m_bMaxPeople,&tagNoPlayer,sizeof(TNoPlayer),MDM_GM_GAME_NOTIFY,ASS_NO_PLAYER,0);
		LOG_MSG_SEND(ASS_NO_PLAYER);

		GameFinish(bDeskStation,GF_SALE);
		//__super::GameFinish(0,GF_SALE);//这儿使用GameFinish后，客户无法调用踢人接口，故屏蔽

		return true;
	}

	if (GetPlayerCount() >= 2)
	{
		//释放当前正在运行的事件
		CloseEventProtection();

		//开始下一回合游戏事件处理
		//ActionRedirect(IDEV_NEXT_ROUND_BEGIN,500);
		GameBegin(ALL_ARGEE);
	}

	return true;
}

//处理玩家本身金币不足于最少带入金额
bool CServerGameDesk::OnUserMoneyLack(BYTE bDeskStation)
{
	TMoneyLack  cmd;
	cmd.nbyUser = bDeskStation;

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL)
		{
			SendGameData(i,&cmd, sizeof(cmd), MDM_GM_GAME_NOTIFY, ASS_NOT_ENOUGH_MONEY, 0);
		}
	}

	SendWatchData(m_bMaxPeople,&cmd, sizeof(cmd), MDM_GM_GAME_NOTIFY, ASS_NOT_ENOUGH_MONEY, 0);
	LOG_MSG_SEND(ASS_NOT_ENOUGH_MONEY);
	
	if (m_pUserInfo[bDeskStation] != NULL)
	{
		// 踢除断线玩家
		//KickUserByID(m_pUserInfo[bDeskStation]->m_UserData.dwUserID);
	}

	// 清空玩家坐位上的信息，以免下一个玩家再次使用
	m_GameData.m_UserMoney[bDeskStation] = 0;
	m_GameData.m_UserData[bDeskStation].m_bIn = false;

	return true;
}

//处理玩家当前买入请求
bool CServerGameDesk::OnUserCurBuyInReq(BYTE byDeskStation, void* pData, bool bForceEffect)
{
	if (!m_bIsBuy
		|| byDeskStation >= PLAY_COUNT 
		|| NULL == m_pUserInfo[byDeskStation]
		|| NULL == pData)
	{
		return true;
	}
	
	TRCCurBuyInReq* tCurBuyInReq = (TRCCurBuyInReq*)pData;

	//可以买入与否
	const bool bCanbuyIn = m_GameData.CheckCanBuyIn(byDeskStation);
	if (bCanbuyIn)
	{
		return true;
	}

	__int64 & iUserMoneyAlias = m_GameData.m_UserMoney[byDeskStation];
	TRCCurBuyInInfo & tRCCurBuyInInfoAlias = m_GameData.m_tRCCurBuyInInfo[byDeskStation];
	//判断买入是否合法值[合法值就行，不进行其他限制]
	if (tCurBuyInReq->iBuyInMoney < tRCCurBuyInInfoAlias.tRCCurCanBuyIn.iSmallBuyInMoney)
	{
		tCurBuyInReq->iBuyInMoney = tRCCurBuyInInfoAlias.tRCCurCanBuyIn.iSmallBuyInMoney;
	} else if (tCurBuyInReq->iBuyInMoney > tRCCurBuyInInfoAlias.tRCCurCanBuyIn.iBigBuyInMoney)
	{
		tCurBuyInReq->iBuyInMoney = tRCCurBuyInInfoAlias.tRCCurCanBuyIn.iBigBuyInMoney;
	}

	//买入之后相关修改
	bool bEffect = bForceEffect;
	if (!bEffect)
	{
		//检查游戏状态
		switch (m_bGameStation)
		{
		case GS_WAIT_SETGAME:		
		case GS_WAIT_ARGEE:
		case GS_WAIT_NEXT_ROUND:
			bEffect = true;
			break;
		default:
			break;
		}
	}

	if (bEffect)
	{
		//立即生效:修改金币
		iUserMoneyAlias += tCurBuyInReq->iBuyInMoney;
		tRCCurBuyInInfoAlias.tRCCurCanBuyIn.iSmallBuyInMoney -= tCurBuyInReq->iBuyInMoney;
		tRCCurBuyInInfoAlias.tRCCurCanBuyIn.iBigBuyInMoney -= tCurBuyInReq->iBuyInMoney;
		tRCCurBuyInInfoAlias.tRCCurCanBuyIn.iHadBuyInMoney += tCurBuyInReq->iBuyInMoney;
		tRCCurBuyInInfoAlias.iLimitBuyInMoney -= tCurBuyInReq->iBuyInMoney;

		//同步数据 Pause
		const int iPlayerID = m_pUserInfo[byDeskStation]->m_UserData.dwUserID;
		m_GameData.SetMapCurMoney(iPlayerID,iUserMoneyAlias);
		m_GameData.SetMapCurBuyInInfo(iPlayerID,tRCCurBuyInInfoAlias);
	} else//否则只改准备买入，本局买入最大值、最小值
	{
		tRCCurBuyInInfoAlias.tRCCurCanBuyIn.iSmallBuyInMoney -= tCurBuyInReq->iBuyInMoney;
		tRCCurBuyInInfoAlias.tRCCurCanBuyIn.iBigBuyInMoney -= tCurBuyInReq->iBuyInMoney;
		tRCCurBuyInInfoAlias.iNextRoundBuyInMoney += tCurBuyInReq->iBuyInMoney;
	}

	TRCCurBuyInRsp tRCCurBuyInRsp;
	tRCCurBuyInRsp.nbyUser = byDeskStation;
	tRCCurBuyInRsp.bBuyInSuccess = true;
	tRCCurBuyInRsp.iBuyInMoney = tCurBuyInReq->iBuyInMoney;
	tRCCurBuyInRsp.bEffectImediately = bEffect;
	if (bEffect)
	{
		tRCCurBuyInRsp.iCurMoneyBuyInUser = iUserMoneyAlias;
	}
	
	SendRCCurBuyInRsp(tRCCurBuyInRsp, bEffect, byDeskStation);
	if (true || bEffect)
	{
		SendRCCurCanBuyInRsp(false, byDeskStation);
	}

	return true;
}


//处理玩家线束本轮游戏的请求
bool CServerGameDesk::OnTestFinishRoundRequest(BYTE bDeskStation,NetMessageHead* pNetHead,void* pData,UINT uSize,UINT uSocketID,bool bWatchUser)
{
	// 	if (m_bGameStation != GS_PLAY_GAME)
	// 	{
	// 		return true;
	// 	}

	//释放当前正在运行的事件
	CloseEventProtection();

	//启动回合结算流程
	ActionRedirect(IDEV_ROUND_FINISH,500);

	return true;
}

//代管玩家发牌完成
bool CServerGameDesk::OnAgentSendCardsFinish()
{
	OutputDebugString("Server:代管玩家发牌完成");
	//查看有没有玩家掉线
	if (m_bFaiPaiFinish == false)	
	{
		for (int i = 0; i < PLAY_COUNT; ++i)
		{
			//if (m_GameData.m_UserData[i].m_bNetCut)

			{
				OnUserSendCardFinish(i,NULL,NULL,0,0,false);
			}
		}
	}
	else
	{
		OutputDebugString("llog::::所有的发牌结束消息都收到了");
	}
	return true;
}

//保护出牌事件流
bool CServerGameDesk::OnAgentOutCard()
{
	return true;
}

//下一局开始事件流代理
bool CServerGameDesk::OnAgentNextRoundBegin()
{
	//查看有没有玩家掉线
	BYTE byNetCutUser = 255;
	for (int i = 0; i < PLAY_COUNT; ++i)
	{
		if (m_GameData.m_UserData[i].m_bNetCut)
		{
			byNetCutUser = i;
			break;
		}
	}

	if (byNetCutUser != 255)
	{
		//掉线的玩家请求退出
		TNextRoundReq NextRoundReq;
		NextRoundReq.nType = TNextRoundReq::EXIT;

		OnUserNextRoundReq(byNetCutUser,NULL,&NextRoundReq,sizeof(TNextRoundReq),0,false);
	}

	return true;
}

//处理要得到游戏等待开始的请求
bool CServerGameDesk::OnGetGSWaitAgree(BYTE bDeskStation,UINT uSocketID,bool bWatchUser,TGSBase* pGSBase)
{
	TGSWaitAgree GSWaitAgree;
	CopyGSBase(dynamic_cast<TGSWaitAgree*>(&GSWaitAgree),pGSBase);

	//具体的数据获取代码写下此
    memcpy(GSWaitAgree.nSubMoney, m_GameData.m_UserMoney, sizeof(m_GameData.m_UserMoney));
	/*if (m_bIsBuy)
	{
		memcpy(GSWaitAgree.iTotalMoney, m_TotalMoney, sizeof(GSWaitAgree.iTotalMoney));
	}*/

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] != NULL)
		{
			GSWaitAgree.bReady[i] = (m_bIsBuy ? m_GameData.m_UserData[i].m_bRCReady : m_GameData.m_UserData[i].m_bIn);
		}
	}

	return SendGameStation(bDeskStation,uSocketID,bWatchUser,&GSWaitAgree,sizeof(TGSWaitAgree));
}

//处理要得到游戏中状态的请求
bool CServerGameDesk::OnGetGSPlaying(BYTE bDeskStation,UINT uSocketID,bool bWatchUser,TGSBase* pGSBase)
{
	TGSPlaying GSPlaying;
	CopyGSBase(dynamic_cast<TGSPlaying*>(&GSPlaying),pGSBase);

	//具体的数据获取代码写下此
	GSPlaying.tPlayInfo = m_tGameBeginInfo;

	// 获得得到令牌的玩家
	GSPlaying.byTokenUser = m_GameData.m_byTokenUser;

    // 当前可用金币
    memcpy(GSPlaying.nSubMoney, m_GameData.m_UserMoney, sizeof(m_GameData.m_UserMoney));
	/*if (m_bIsBuy)
	{
	memcpy(GSPlaying.iTotalMoney, m_TotalMoney, sizeof(GSPlaying.iTotalMoney));
	}*/

    // 庄家
    GSPlaying.byNTUser = m_GameData.m_byNTUser;

    // 大小肓家
    GSPlaying.bySmallBlind = m_GameData.m_bSmallBlind;
    GSPlaying.byBigBlind = m_GameData.m_bBigBlind;

    GSPlaying.byVerbFlag = m_GameData.m_byVerbFlag;
    GSPlaying.bNewTurn = m_GameData.m_bNewTurn;
	GSPlaying.bNewTurns = m_GameData.m_bNewTurns;

	// 跟注金币
	GSPlaying.nCallMoney = m_GameData.m_nMaxBetMoney - m_GameData.GetBetMoney(m_GameData.m_byTokenUser);
	//最小下注/加注的金币
	GSPlaying.iMinBetOrRaiseMoney = m_GameData.m_iMinBetOrRaiseMoney;
	//最大下注/加注的金币
	GSPlaying.iMaxBetOrRaiseMoney = m_GameData.m_iMaxBetOrRaiseMoney;

    // 获取玩家手上的扑克
    GSPlaying.nHandCardNums = m_GameData.m_UserData[bDeskStation].m_iCardsNum;
	memset(GSPlaying.byHandCard,0,sizeof(GSPlaying.byHandCard));

	memcpy(GSPlaying.byHandCard[bDeskStation], m_GameData.m_UserData[bDeskStation].m_byMyCards,  GSPlaying.nHandCardNums * sizeof(BYTE));

	bool bIsSuper = ExamSuperUser(bDeskStation);
	bool bIsVirtual = m_pUserInfo[bDeskStation]->m_UserData.isVirtual;
	for (int i = 0;  (bIsSuper || bIsVirtual) && i < PLAY_COUNT;i++)
	{
		memcpy(GSPlaying.byHandCard[i], m_GameData.m_UserData[i].m_byMyCards,  GSPlaying.nHandCardNums * sizeof(BYTE));
	}

	if (!bIsSuper && !bIsVirtual)
	{
		// 获取公共牌
		GSPlaying.nCardsCount = m_GameData.m_iCardsCount;
		memcpy(GSPlaying.byCards, m_GameData.m_iCards, GSPlaying.nCardsCount * sizeof(BYTE));
	}
	else
	{
		// 获取公共牌
		GSPlaying.nCardsCount = 5;
		memcpy(GSPlaying.byCards, &m_GameData.m_byCards[m_GameData.m_iPlayCount * MAX_DEAL_CARDS], GSPlaying.nCardsCount * sizeof(BYTE));
	}
	// 获取公共牌(供手游使用)
	GSPlaying.nCardsCountToMobile = m_GameData.m_iCardsCount;
    // 获取下注金币
    memcpy(GSPlaying.nBetMoney, m_GameData.m_nBetMoney, sizeof(m_GameData.m_nBetMoney));

	/*CString strMoney;

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] == NULL)
		{
			continue;
		}
		for (int j = 0; j < PLAY_COUNT; j++)
		{
			strMoney.Format("Server:玩家%d,获取下注金币:%I64d",GSPlaying.nBetMoney[i][j]);
			OutputDebugString(strMoney);
		}
	}*/

    // 获取不能下注玩家
    memcpy(GSPlaying.bNoBet, m_GameData.m_bNoBet, sizeof(m_GameData.m_bNoBet));

    // 获取弃牌玩家
    memcpy(GSPlaying.bGiveUp, m_GameData.m_bGiveUp, sizeof(m_GameData.m_bGiveUp));

	CString str;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		str.Format("giwo:获取弃牌玩家：%d",m_GameData.m_bGiveUp[i]);
		OutputDebugString(str);
	}

	// 中途进入玩家
	GSPlaying.iUserStation = m_iUserStation[bDeskStation] ; 
		
    // 所有边池数据
    for (int i = 0; i <= m_GameData.m_nBetPool; i++)
    {
        GSPlaying.nBetPools[i] = m_GameData.GetBetMoney(i, false);
    }

	SendGameStation(bDeskStation,uSocketID,bWatchUser,&GSPlaying,sizeof(TGSPlaying));

	SendRemainingGameCount();

	return true;
}

//处理要得到游戏回合结束的请求
bool CServerGameDesk::OnGetGSWaitNextRound(BYTE bDeskStation,UINT uSocketID,bool bWatchUser,TGSBase* pGSBase)
{
	TGSWaitNextRound GSWaitNextRound;
	//CopyGSBase(dynamic_cast<TGSWaitNextRound*>(&GSWaitNextRound),pGSBase);

	//具体的数据获取代码写下此
    //memcpy(GSWaitNextRound.nSubMoney, m_GameData.m_UserMoney, sizeof(m_GameData.m_UserMoney));

	return SendGameStation(bDeskStation,uSocketID,bWatchUser,&GSWaitNextRound,sizeof(TGSWaitNextRound));
}

//房卡场新追加信息
bool CServerGameDesk::OnGetRCInfoAppend(BYTE bDeskStation)
{
	if (!m_bIsBuy)
	{
		return true;	
	}

	SendRCOption(false,bDeskStation);
	SendRCCurMoney();
	//SendRCCurCanBuyInRsp(false,bDeskStation);
	SendWinTotalInfo(false, bDeskStation);

	return true;
}

//顺时针顺序得到下家位置
BYTE CServerGameDesk::GetNextUserStation(BYTE byStation, BOOL bIsAll)
{
	int nCount = 0;
	int iNextStation = (byStation + PLAY_COUNT + 1) % PLAY_COUNT;

	// 绕过无效玩家与已放弃的玩家 或 已经全下的玩家 或 中途进入游戏玩家
	while (m_pUserInfo[iNextStation] == NULL 
	|| m_GameData.m_bGiveUp[iNextStation] 
	|| (bIsAll && m_GameData.m_bNoBet[iNextStation])
	|| m_iUserStation[iNextStation] != TYPE_PLAY_GAME)
	{
		iNextStation = (iNextStation + PLAY_COUNT + 1) % PLAY_COUNT;
		nCount++;

		if (nCount >= PLAY_COUNT)
		{
			iNextStation = -1;
			break;
		}
	}

	return iNextStation;
}

//逆时针顺序得到上家位置
BYTE CServerGameDesk::GetLastUserStation(BYTE byStation, BOOL bIsAll)
{
	int nCount = 0;
	int iLastStation = (byStation + PLAY_COUNT - 1) % PLAY_COUNT;

	// 绕过无效玩家与已放弃的玩家 或 已经全下的玩家 或 中途进入游戏玩家
	while (m_pUserInfo[iLastStation] == NULL
	|| m_GameData.m_bGiveUp[iLastStation]
	|| (bIsAll && m_GameData.m_bNoBet[iLastStation])
	|| m_iUserStation[iLastStation] != TYPE_PLAY_GAME)
	{
		iLastStation = (iLastStation + PLAY_COUNT - 1) % PLAY_COUNT;

		nCount++;

		if (nCount >= PLAY_COUNT)
		{
			iLastStation = -1;
			break;
		}
	}

	return iLastStation;
}

///名称：GetPlayerCount
///描述：获取游戏玩家数量
///@return 
int CServerGameDesk::GetPlayerCount(bool bFold)
{
	int nCount = 0;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (bFold)
		{
			if (m_pUserInfo[i] != NULL && m_iUserStation[i] == TYPE_PLAY_GAME)
			{
				nCount++;
			}
		}
		else
		{
			if (m_pUserInfo[i] != NULL)
			{
				nCount++;
			}
		}
		
	}

	return nCount;
}

// 计算个下注池胜利玩家
void CServerGameDesk::CalculateWin(BYTE nSectionCount, CByteArray &arrayResult,bool bForce)
{
	arrayResult.RemoveAll();

	// 找出牌型最大的牌
	// 查找一个可用的玩家开始比牌
	BYTE bMaxStation = -1;

	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] == NULL || m_GameData.m_bGiveUp[i] || (m_GameData.m_nBetMoney[i][nSectionCount] <= 0 && !bForce))
		{
			continue ;
		}

		bMaxStation = i;
		break;
	}

	CString str;
	str.Format("lbdzpk: 第%d个下注池, 第一个比牌玩家: %d", nSectionCount, bMaxStation);
	OutputDebugString(str);

	if (bMaxStation == 255)
	{
		return ;
	}

	// 目前为赢家
	arrayResult.Add(bMaxStation);

	// 找出比牌后胜利玩家
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i] == NULL || m_GameData.m_bGiveUp[i] || (m_GameData.m_nBetMoney[i][nSectionCount] <= 0 && !bForce) || i == bMaxStation)
		{
			continue ;
		}

		int nRet = m_Logic.CompareCard(m_GameData.m_bResultCards[i], 5, m_GameData.m_bResultCards[bMaxStation], 5);

		if (nRet > 0)
		{
			arrayResult.RemoveAll();

			// 赢
			bMaxStation = i;
			arrayResult.Add(i);
		}
		else if (nRet == 0)
		{			
			// 相等
			arrayResult.Add(i);
		}
	}
}

// 判断玩家是否属于配好牌玩家
bool CServerGameDesk::ExamSuperUser(BYTE byStation)
{
	if (byStation >= PLAY_COUNT)
	{
		return false;
	}

	if(!m_pUserInfo[byStation])
	{
		return false;
	}

	long int UserID = m_pUserInfo[byStation]->m_UserData.dwUserID;

	TCHAR szIniName[MAX_PATH];
	GET_SERVER_INI_NAME(szIniName);

	CINIFile f(szIniName);

	CString key = TEXT("superUserConfig");

	int	iSuperUserCount = 0;
	iSuperUserCount = f.GetKeyVal(key,"SuperUserCount",0);	//超端用户人数

	if(iSuperUserCount <= 0)
	{
		return false;
	}

	long int iTempUserID = -1;
	CString sTemp = "";
	for(int i = 1; i <= iSuperUserCount; ++i)
	{
		sTemp.Format("SuperUserID%d",i);
		iTempUserID = f.GetKeyVal(key,sTemp,0);		//读取超端用户ID

		//与配置文件中的ID相符 则为超端用户
		if(UserID == iTempUserID)
		{
			return true;
		}
	}
	return false;
	
}

//自动操作游戏事件处理
bool CServerGameDesk::OnEventAutoOperate()
{
    // 自动放弃操作
    TBet tagBet;
    tagBet.nType = ET_FOLD;
    tagBet.byUser = m_GameData.m_byTokenUser;
    tagBet.nMoney = 0;

	//添加功能，可以Check的时候不放弃
	bool bCheckOption = false;
	if (CheckCanSelectCheck())
	{
		tagBet.nType = ET_CHECK;
		bCheckOption = true;
	}

    // 模拟下注消息
    OnUserBet(m_GameData.m_byTokenUser, &tagBet, false, bCheckOption);

    return true;
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

int CServerGameDesk::ExistRealUser()
{
	int iRealCount = 0;
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		if (!m_pUserInfo[i]) continue;
		if (!m_pUserInfo[i]->m_UserData.isVirtual) continue; //不是机器人
		if (!m_GameData.m_UserData[i].m_bIn) continue; //没带入金币
		iRealCount++;
	}
	return iRealCount;
}

void CServerGameDesk::SendScoreBoard(bool dismiss)
{
	CalculateBoardData data[PLAY_COUNT];
	for(int i = 0; i < PLAY_COUNT; ++i)
	{
		data[i] = Statistic(i);
		//data[i].gameFinish = !dismiss;
	}

	for(int i = 0; i < PLAY_COUNT; ++i)
	{
		if (m_pUserInfo[i] != NULL)
		{
			SendGameData(i, &data, sizeof(data), MDM_GM_GAME_NOTIFY, ASS_CALCULATE_BOARD, 0);
		}
	}

	LOG_MSG_SEND(ASS_CALCULATE_BOARD);
}

CalculateBoardData CServerGameDesk::Statistic(int playerPos)
{
	CalculateBoardData data;
	data.Clear();
	int maxWinPlayer = 0;
	for(int i = 0; i < PLAY_COUNT; ++i)
	{
		if(m_TotalMoney[i] > m_TotalMoney[maxWinPlayer])
		{
			maxWinPlayer = i;
		}
	}

	if (playerPos >= 0 && playerPos < PLAY_COUNT && m_pUserInfo[playerPos])
	{
		data.iUserID = m_pUserInfo[playerPos]->m_UserData.dwUserID;
	} else 
	{
		data.iUserID = INT_MAX;
	}

	data.bWinner = (0 != m_TotalMoney[maxWinPlayer] && m_TotalMoney[maxWinPlayer] == m_TotalMoney[playerPos]); // Changed by QiWang 20171012, 预防多个大赢家
	data.i64WinMoney = m_TotalMoney[playerPos];
	data.iMaxContinueCount = m_MaxContinuouslyWin[playerPos];
	data.iWinCount = m_WinCount[playerPos];
	data.iMaxWinMoney = m_MaxWinningMoney[playerPos];
	return data;
}

void CServerGameDesk::ClearStatData()
{
	for(int i = 0; i < PLAY_COUNT; ++i)
	{
		m_bLatestWinner[i] = false;
		m_TotalMoney[i] = 0;
		m_ContinuouslyWin[i] = 0;
		m_MaxContinuouslyWin[i] = 0; // Added by QiWang 20171112
		m_WinCount[i] = 0;
		m_MaxWinningMoney[i] = 0;
	}

	//新增房卡金币相关数据Clear
	m_GameData.ClearRCMapData();

	//最新超端需求 20180411
	m_bSuperSetCardEffectNext = false;
	m_tSuperSetCardReqNext.Clear();

	m_bGameStartAtleastOnce = false;
}

void CServerGameDesk::SendRemainingGameCount()
{
	if(m_bIsBuy)
	{
		RemainingGameCount data;
		data.gameCount = m_iVipGameCount - m_iRunGameCount - 1;
		for(int i = 0; i < PLAY_COUNT; ++i)
		{
			if (m_pUserInfo[i] != NULL)
			{
				SendGameData(i, &data, sizeof(data), MDM_GM_GAME_NOTIFY, ASS_REMAINING_GAME_COUNT, 0);
			}
		}
		LOG_MSG_SEND(ASS_REMAINING_GAME_COUNT);
	}
}

//发送房卡配置
void CServerGameDesk::SendRCOption(bool bAll, BYTE byDeskStation)
{
	if (!bAll && byDeskStation >= PLAY_COUNT)
	{
		return;
	}

	TRCOption tRCOption;
	memcpy(&tRCOption,&m_GameData.m_tRCOption,sizeof(tRCOption));
	if (bAll)
	{
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (m_pUserInfo[i])
			{
				SendGameData(i, &tRCOption, sizeof(tRCOption), MDM_GM_GAME_NOTIFY, ASS_RC_OPTION, 0);
			}
		}
		LOG_MSG_SEND(ASS_RC_OPTION);
	} else 
	{
		if (m_pUserInfo[byDeskStation])
		{
			SendGameData(byDeskStation, &tRCOption, sizeof(tRCOption), MDM_GM_GAME_NOTIFY, ASS_RC_OPTION, 0);
			LOG_MSG_SEND(ASS_RC_OPTION,byDeskStation);
		}
	}
}

//发送当前货币
void CServerGameDesk::SendRCCurMoney(bool bAll, BYTE byDeskStation)
{
	if (!bAll && byDeskStation >= PLAY_COUNT)
	{
		return;
	}

	TRCCurMoney tRCCurMoney;
	for (int i = 0; i < PLAY_COUNT; i++)
	{
		if (m_pUserInfo[i])
		{
			//copy 数据
			tRCCurMoney.tCurMoneyInfoArr[i].iCurMoney = m_GameData.m_UserMoney[i];
			//tRCCurMoney.tCurMoneyInfoArr[i].iHadBuyInMoney = m_GameData.m_tRCCurBuyInInfo[i].tRCCurCanBuyIn.iHadBuyInMoney;
			//tRCCurMoney.tCurMoneyInfoArr[i].iNextRoundBuyInMoney = m_GameData.m_tRCCurBuyInInfo[i].iNextRoundBuyInMoney;
			//此次买入Pause
		}
	}


	if (bAll)
	{
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (m_pUserInfo[i])
			{
				SendGameData(i, &tRCCurMoney, sizeof(tRCCurMoney), MDM_GM_GAME_NOTIFY, ASS_RC_CUR_MONEY, 0);
			}
		}
		LOG_MSG_SEND(ASS_RC_CUR_MONEY, &tRCCurMoney, sizeof(tRCCurMoney));
	} else 
	{
		if (m_pUserInfo[byDeskStation])
		{
			SendGameData(byDeskStation, &tRCCurMoney, sizeof(tRCCurMoney), MDM_GM_GAME_NOTIFY, ASS_RC_CUR_MONEY, 0);
			LOG_MSG_SEND(ASS_RC_CUR_MONEY, byDeskStation, &tRCCurMoney, sizeof(tRCCurMoney));
		}
	}
}

//发送输赢统计信息
void CServerGameDesk::SendWinTotalInfo(bool bAll, BYTE byDeskStation)
{
	if (!bAll && byDeskStation >= PLAY_COUNT)
	{
		return;
	}

	TUersWinTotalInfo tUersWinTotalInfo;

	memcpy(tUersWinTotalInfo.iTotalMoney, m_TotalMoney, sizeof(tUersWinTotalInfo.iTotalMoney));
	
	if (bAll)
	{
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (m_pUserInfo[i])
			{
				SendGameData(i, &tUersWinTotalInfo, sizeof(tUersWinTotalInfo), MDM_GM_GAME_NOTIFY, ASS_USERS_WIN_TOTAL_INFO, 0);
			}
		}
		LOG_MSG_SEND(ASS_USERS_WIN_TOTAL_INFO);
	} else 
	{
		if (m_pUserInfo[byDeskStation])
		{
			SendGameData(byDeskStation, &tUersWinTotalInfo, sizeof(tUersWinTotalInfo), MDM_GM_GAME_NOTIFY, ASS_USERS_WIN_TOTAL_INFO, 0);
			LOG_MSG_SEND(ASS_USERS_WIN_TOTAL_INFO, byDeskStation);
		}
	}
}

//发送当前可买入应答
void CServerGameDesk::SendRCCurCanBuyInRsp(bool bAll, BYTE byDeskStation)
{
	if (!bAll && byDeskStation >= PLAY_COUNT)
	{
		return;
	}

	TRCCurCanBuyInRsp tRCCurCanBuyInRsp;
	if (bAll)
	{
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (m_pUserInfo[i])
			{
				memcpy(&tRCCurCanBuyInRsp,&m_GameData.m_tRCCurBuyInInfo[i].tRCCurCanBuyIn,sizeof(tRCCurCanBuyInRsp));
				SendGameData(i, &tRCCurCanBuyInRsp, sizeof(tRCCurCanBuyInRsp), MDM_GM_GAME_NOTIFY, ASS_RC_CUR_CAN_BUYIN_RSP, 0);
			}
		}
		LOG_MSG_SEND(ASS_RC_CUR_CAN_BUYIN_RSP);
	} else 
	{
		if (m_pUserInfo[byDeskStation])
		{
			memcpy(&tRCCurCanBuyInRsp,&m_GameData.m_tRCCurBuyInInfo[byDeskStation].tRCCurCanBuyIn,sizeof(tRCCurCanBuyInRsp));
			SendGameData(byDeskStation, &tRCCurCanBuyInRsp, sizeof(tRCCurCanBuyInRsp), MDM_GM_GAME_NOTIFY, ASS_RC_CUR_CAN_BUYIN_RSP, 0);
			LOG_MSG_SEND(ASS_RC_CUR_CAN_BUYIN_RSP,byDeskStation);
		}
	}
}

//发送当前可买入应答
void CServerGameDesk::SendRCCurBuyInRsp(TRCCurBuyInRsp& tRCCurBuyInRsp, bool bAll, BYTE byDeskStation)
{
	if (!bAll && byDeskStation >= PLAY_COUNT)
	{
		return;
	}

	TRCCurBuyInRsp* p_tRCCurBuyInRsp = &tRCCurBuyInRsp;
	if (bAll)
	{
		for (int i = 0; i < PLAY_COUNT; i++)
		{
			if (m_pUserInfo[i])
			{
				SendGameData(i, (void*)p_tRCCurBuyInRsp, sizeof(tRCCurBuyInRsp), MDM_GM_GAME_NOTIFY, ASS_RC_CUR_BUYIN_RSP, 0);
			}
		}
		LOG_MSG_SEND(ASS_RC_CUR_BUYIN_RSP);
	} else 
	{
		if (m_pUserInfo[byDeskStation])
		{
			SendGameData(byDeskStation, (void*)p_tRCCurBuyInRsp, sizeof(tRCCurBuyInRsp), MDM_GM_GAME_NOTIFY, ASS_RC_CUR_BUYIN_RSP, 0);
			LOG_MSG_SEND(ASS_RC_CUR_BUYIN_RSP,byDeskStation);
		}
	}
}

//发送大结算
void CServerGameDesk::SendRCScoreBoard(bool bAll, BYTE byDeskStation)
{
	if (!bAll && byDeskStation >= PLAY_COUNT)
	{
		return;
	}

	if (bAll)
	{
		for(int i = 0; i < PLAY_COUNT; ++i)
		{
			byDeskStation = i;
			SendRCScoreBoard(byDeskStation);
		}
		LOG_MSG_SEND(ASS_CALCULATE_BOARD);
	} else 
	{
		SendRCScoreBoard(byDeskStation);
		LOG_MSG_SEND(ASS_CALCULATE_BOARD,byDeskStation);
	}
}

//发送大结算
void CServerGameDesk::SendRCScoreBoard(BYTE byDeskStation)
{
	if (byDeskStation >= PLAY_COUNT)
	{
		return;
	}

	if (NULL == m_pUserInfo[byDeskStation])
	{
		return;
	}

	CalculateBoardData data[PLAY_COUNT];
	SendGameData(byDeskStation, &data, sizeof(data), MDM_GM_GAME_NOTIFY, ASS_CALCULATE_BOARD, 0);
	//LOG_MSG_SEND(ASS_CALCULATE_BOARD,byDeskStation);
}

//Added by QiWang 20180323, 普通场T人相关
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

//测试新超端
#if MODE_SUPER_DEBUG
void CServerGameDesk::TestSuperSetCard()
{
	m_bSuperSetCardEffectNext = true;
	m_tSuperSetCardReqNext.Clear();

	//测试数据
	BYTE byComCards[NUM_COMMUNITY_CARDS_NUM] = {0x02, 0x00, 0x18, 0x00, 0x36}; //设定公共牌
	THandCards tHandCardsTemp;
	tHandCardsTemp.bSetHandCard = true;
	tHandCardsTemp.iUserID = 11932;
	tHandCardsTemp.byCard[0] = 0x08;
	tHandCardsTemp.byCard[1] = 0x00;
	
	//copy
	memcpy(m_tSuperSetCardReqNext.byComCards, byComCards, sizeof(byComCards));
	m_tSuperSetCardReqNext.tPlayerHandCards[0] = tHandCardsTemp;
}
#endif //MODE_SUPER_DEBUG


//以下Log相关函数
#if MODE_LOG_DEBUG
string  CServerGameDesk::GetTimerNameByID(UINT uTimerID)
{
	struct STimerNode
	{
		UINT uId;
		string sName;
	};    
	static STimerNode NameList[] = {
		//定时器 ID
		{IDT_MYTIMER			, "公用定时器"},
		{IDMYT_EVENT_PROTECT	, "事件流保护定时器"},
		
		{IDEV_SYS_CFG_DESK		, "系统设定桌面配置定时器/事件"},
		{IDEV_USER_CFG_DESK		, "玩家设定桌面配置定时器/事件"},
		{IDEV_RAND_CARD			, "洗牌定时器/事件"},
		
		{IDEV_SENDACARD			, "下发牌定时器/事件"},
		{IDEV_SEND_3_CARD		, "发3张牌定时器/事件"},
		{IDEV_SEND_4_5_CARD		, "发第4,5张牌定时器/事件"},
		{IDEV_BURN_CARD			, "烧牌定时器/事件"},
		
		{IDEV_BET_START			, "首次下注定时器/事件"},
		{IDEV_BET_BLINDS_AUTO	, "大小盲注同时下注定时器/事件(系统强制)"},
		{IDEV_BET_NEXT			, "下位玩家下注定时器/事件"},
		
		{IDEV_BIGBLIND          , "下大肓注定时器"},
		{IDEV_SMALL_BLIND	    , "下小肓注定时器"},
		
		{IDEV_ROUND_FINISH		, "一回合完成/事件"},
		{IDEV_RESULT			, "游戏结算定时器/事件"},
		{IDEV_NEXT_ROUND_BEGIN	, "下一回合开始的定时器/事件"},		
		{IDEV_COMPARE_CARD      , "比牌"},
		{IDEV_RANDOM			, "随机一个庄与大小/事件"},
		
		{IDEV_AUTO_OPERATE      , "自动操作定时器"},
	};	
	///////////// 获取 ////////////////////////////////////////
	string  cName = "";
	int len = sizeof(NameList) / sizeof(NameList[0]);
	for(int i=0;i<len;i++)
	{
		if(NameList[i].uId == uTimerID)
		{
			cName = NameList[i].sName;
			break;
		}
	}
	return cName;
}

string  CServerGameDesk::GetCHNameByAssID(UINT bAssistantID)
{
	/////////////// 初始化 ///////////////////////////////
	////  alt + R 可以局部选中 
	struct SNode
	{
		UINT uId;
		string sName;
	};    
	static SNode NameList[] = {
		{ASS_GM_AGREE_GAME				,"同意游戏"},
		{ASS_UG_USER_SET				,"用户设置游戏"},
		{ASS_UG_USER_SET_CHANGE			,"玩家设置更改"},
		
		{ASS_GAME_BEGIN					,"游戏开始"},
		{ASS_SYS_DESK_CFG				,"系统设定桌面配置"},
		{ASS_CFG_DESK					,"玩家设置底注"},
		{ASS_DESK_CFG					,"玩家设定桌面配置"},
		{ASS_AGREE_DESK_CFG				,"同意桌面配置"},
		{ASS_BET						,"玩家下注"},
		{ASS_SEND_A_CARD				,"发底牌2张"},
		{ASS_SEND_3_CARD				,"发翻牌3张"},
		{ASS_SEND_4_5_CARD				,"发转牌或者河牌1张"},
		{ASS_SEND_CARD_FINISH			,"发牌结束"},
		{ASS_BETPOOL_UP					,"边池更新"},
		{ASS_BET_REQ					,"下注请求"},
		{ASS_BET_INFO					,"下注消息"},
		{ASS_NEW_USER					,"下一个玩家操作"},
		{ASS_PASS_USER					,"弃牌的玩家操作"},
		{ASS_SUB_MENOY                  ,"代入金币限制"},
		{ASS_COMPARE_CARD               ,"比牌信息"},
		
		{ASS_CALLMANDATE				,"跟注托管"},
		{ASS_CHECKMANDATE               ,"过牌托管"},
		{ASS_CALLANYMANDATE             ,"跟任何注托管"},
		{ASS_PASSABANDONMANDATE			,"过牌或弃牌托管"},

		{ASS_USER_LEFT_DESK             ,"用户离开"},
		
		{ASS_SORT_OUT					,"清理图片"},
	
		{ASS_TOKEN						,"令牌信息"},
		{ASS_REMAINING_GAME_COUNT		,"剩余局数"},
		{ASS_CALCULATE_BOARD			,"大结算"},
		{ASS_RESULT						,"回合算分"},
		{ASS_SHOW_IN_MONEY				,"显示代入金额"},
		{ASS_LOOK_CARD					,"看牌"},
		{ASS_CAN_SHOW_CARD				,"能展示牌的消息"},
		{ASS_SHOW_CARD					,"展示牌的消息"},

		//房卡场新增消息
		{ASS_RC_OPTION					,"房卡选项"},
		{ASS_RC_CUR_MONEY				,"当前货币"},
		{ASS_RC_CUR_CAN_BUYIN_REQ		,"当前可买入请求"},
		{ASS_RC_CUR_CAN_BUYIN_RSP		,"当前可买入应答"},
		{ASS_RC_CUR_BUYIN_REQ			,"当前买入请求"},
		{ASS_RC_CUR_BUYIN_RSP			,"当前买入应答"},
		{ASS_RC_CANNOT_PLAY_LACK_MONEY	,"缺乏货币不能玩耍"},

		//新增新超端消息
		{ASS_SUPER_SET_CARD_REQ			,"超端设置牌请求"},
		{ASS_SUPER_SET_CARD_RSP			,"超端设置牌应答"},
		
		{ASS_USERS_WIN_TOTAL_INFO		,"玩家输赢统计信息"},

		{ASS_SET_ACTION_PROB			,"设定行为概率"},
		
		{ASS_NO_PLAYER					,"没有玩家进行游戏"},
		{ASS_AGREE						,"玩家是否同意的消息"},
		{ASS_CFG_DESK_TIMEOUT			,"设置底注超时"},
		{ASS_NOT_ENOUGH_MONEY			,"玩家金币不足的清除玩家信息"},
		{ASS_MONEY_LACK					,"玩家金币不足的提示"},
		
		{ASS_NEXT_ROUND_REQ				,"玩家下一回合请求"},
		{ASS_NEW_ROUND_BEGIN			,"下一回合开始"},
		
		{ASS_FINISH_ROUND_REQ			,"结束本轮游戏"},
		
		{S_C_IS_SUPER_SIG				,"是否是超端"},
		{S_C_SUPER_SET_RESULT_SIG		,"超端设置结果"},
		
		{C_S_SUPER_SET_SIG				,"设置下盘赢得位置"},
	};	
	///////////// 获取 ////////////////////////////////////////
	string  cName = "";
	int len = NUM_GET_ARR(NameList);

	for(int i=0;i<len;i++)
	{
		if(NameList[i].uId == bAssistantID)
		{
			cName = NameList[i].sName;
			break;
		}
	}
	return cName;
}

//Log Message Send
void CServerGameDesk::LogMsgSend(UINT AssistID)
{
	CTime time = CTime::GetCurrentTime();
	LOGMODULEINSTANCE.WriteLog(
		"%02d,%02d,%02d,房间%02d,密码%s,%08x,%02d,%02d,游戏状态,%d,服务器,%s\n", 
		time.GetHour(), time.GetMinute(), time.GetSecond(), m_bDeskIndex, m_szDeskPassWord, 0, m_iVipGameCount, m_iRunGameCount, 
		m_bGameStation, GetCHNameByAssID(AssistID).c_str());
}

void CServerGameDesk::LogMsgSend(UINT AssistID, BYTE bDeskStation)
{
	CTime time = CTime::GetCurrentTime();
	LOGMODULEINSTANCE.WriteLog(
		"%02d,%02d,%02d,房间%02d,密码%s,%08x,%02d,%02d,游戏状态,%d,服务器,%s,发送给%d\n", 
		time.GetHour(), time.GetMinute(), time.GetSecond(), m_bDeskIndex, m_szDeskPassWord, 0, m_iVipGameCount, m_iRunGameCount, 
		m_bGameStation, GetCHNameByAssID(AssistID).c_str(),bDeskStation);
}

void CServerGameDesk::LogMsgSend(UINT AssistID, void *data, int dataSize)
{
	if (NULL != data && dataSize > 0)
	{
		switch(AssistID)
		{
		case ASS_TOKEN:
			if (dataSize >= sizeof(TToken))
			{
				LogMsgSend(AssistID, *(TToken*)data);
				return;
			}
			break;
		case ASS_BET_INFO:
			if (dataSize >= sizeof(TBet))
			{
				LogMsgSend(AssistID, *(TBet*)data);
				return;
			}
			break;
		case ASS_RC_CUR_MONEY:
			if (dataSize >= sizeof(TRCCurMoney))
			{
				LogMsgSend(AssistID, *(TRCCurMoney*)data);
				return;
			}
			break;
		default:
			break;
		}
	}

	LogMsgSend(AssistID);
}

void CServerGameDesk::LogMsgSend(UINT AssistID, BYTE bDeskStation, void *data, int dataSize)
{
	if (NULL != data && dataSize > 0)
	{
		switch(AssistID)
		{
		case ASS_RC_CUR_MONEY:
			if (dataSize >= sizeof(TRCCurMoney))
			{
				LogMsgSend(AssistID, bDeskStation, *(TRCCurMoney*)data);
				return;
			}
			break;
		default:
			break;
		}
	}

	LogMsgSend(AssistID, bDeskStation);
}

void CServerGameDesk::LogMsgSend(UINT AssistID, const TToken& msg)
{
	if (ASS_TOKEN == AssistID)
	{
		CTime time = CTime::GetCurrentTime();
		LOGMODULEINSTANCE.WriteLog(
			"%02d,%02d,%02d,房间%02d,密码%s,%08x,%02d,%02d,游戏状态,%d,服务器,%s", 
			time.GetHour(), time.GetMinute(), time.GetSecond(), m_bDeskIndex, m_szDeskPassWord, 0, m_iVipGameCount, m_iRunGameCount, 
			m_bGameStation, GetCHNameByAssID(AssistID).c_str());

		//msg Log
		LOGMODULEINSTANCE.WriteLog(",byUser=%d", msg.byUser);
		LOGMODULEINSTANCE.WriteLog(",byVerbFlag=%d", msg.byVerbFlag);

		LOGMODULEINSTANCE.WriteLog("\n");
	} else 
	{
		LogMsgSend(AssistID);
	}
}

void CServerGameDesk::LogMsgSend(UINT AssistID, const TBet& msg)
{
	if (ASS_BET_INFO == AssistID)
	{
		CTime time = CTime::GetCurrentTime();
		LOGMODULEINSTANCE.WriteLog(
			"%02d,%02d,%02d,房间%02d,密码%s,%08x,%02d,%02d,游戏状态,%d,服务器,%s", 
			time.GetHour(), time.GetMinute(), time.GetSecond(), m_bDeskIndex, m_szDeskPassWord, 0, m_iVipGameCount, m_iRunGameCount, 
			m_bGameStation, GetCHNameByAssID(AssistID).c_str());

		//msg Log
		LOGMODULEINSTANCE.WriteLog(",nType=%d", msg.nType);
		LOGMODULEINSTANCE.WriteLog(",byUser=%d", msg.byUser);
		LOGMODULEINSTANCE.WriteLog(",nMoney=%ld", msg.nMoney);
		LOGMODULEINSTANCE.WriteLog(",nBetMoney=%ld", msg.nBetMoney);

		LOGMODULEINSTANCE.WriteLog("\n");
	} else 
	{
		LogMsgSend(AssistID);
	}
}

void CServerGameDesk::LogMsgSend(UINT AssistID, const TRCCurMoney& msg)
{
	if (ASS_RC_CUR_MONEY == AssistID)
	{
		CTime time = CTime::GetCurrentTime();
		LOGMODULEINSTANCE.WriteLog(
			"%02d,%02d,%02d,房间%02d,密码%s,%08x,%02d,%02d,游戏状态,%d,服务器,%s", 
			time.GetHour(), time.GetMinute(), time.GetSecond(), m_bDeskIndex, m_szDeskPassWord, 0, m_iVipGameCount, m_iRunGameCount, 
			m_bGameStation, GetCHNameByAssID(AssistID).c_str());

		//msg Log
		LOGMODULEINSTANCE.WriteLog(",msg=%d", msg);
		LOGMODULEINSTANCE.WriteLog(",tCurMoneyInfoArr{");
		for (int i = 0; i < NUM_GET_ARR(msg.tCurMoneyInfoArr); i++)
		{
			if (NULL == m_pUserInfo[i])
			{
				continue;
			}
			LOGMODULEINSTANCE.WriteLog("[%d]iCurMoney=%ld_iHadBuyInMoney=%ld_iTheBuyInMoney=%ld_iNextRoundBuyInMoney=%ld_,",
										i, 
										msg.tCurMoneyInfoArr[i].iCurMoney,
										msg.tCurMoneyInfoArr[i].iHadBuyInMoney,
										msg.tCurMoneyInfoArr[i].iTheBuyInMoney,
										msg.tCurMoneyInfoArr[i].iNextRoundBuyInMoney);
		}

		LOGMODULEINSTANCE.WriteLog("\n");
	} else 
	{
		LogMsgSend(AssistID);
	}
}

void CServerGameDesk::LogMsgSend(UINT AssistID, BYTE bDeskStation, const TRCCurMoney& msg)
{
	if (ASS_RC_CUR_MONEY == AssistID)
	{
		CTime time = CTime::GetCurrentTime();
		LOGMODULEINSTANCE.WriteLog(
			"%02d,%02d,%02d,房间%02d,密码%s,%08x,%02d,%02d,游戏状态,%d,服务器,%s,发送给%d", 
			time.GetHour(), time.GetMinute(), time.GetSecond(), m_bDeskIndex, m_szDeskPassWord, 0, m_iVipGameCount, m_iRunGameCount, 
			m_bGameStation, GetCHNameByAssID(AssistID).c_str(), bDeskStation);

		//msg Log
		LOGMODULEINSTANCE.WriteLog(",msg=%d", msg);
		LOGMODULEINSTANCE.WriteLog(",tCurMoneyInfoArr{");
		for (int i = 0; i < NUM_GET_ARR(msg.tCurMoneyInfoArr); i++)
		{
			if (NULL == m_pUserInfo[i])
			{
				continue;
			}
			LOGMODULEINSTANCE.WriteLog("[%d]iCurMoney=%ld_iHadBuyInMoney=%ld_iTheBuyInMoney=%ld_iNextRoundBuyInMoney=%ld_,",
										i, 
										msg.tCurMoneyInfoArr[i].iCurMoney,
										msg.tCurMoneyInfoArr[i].iHadBuyInMoney,
										msg.tCurMoneyInfoArr[i].iTheBuyInMoney,
										msg.tCurMoneyInfoArr[i].iNextRoundBuyInMoney);
		}

		LOGMODULEINSTANCE.WriteLog("\n");
	} else 
	{
		LogMsgSend(AssistID);
	}
}

void CServerGameDesk::LogMsgSend(UINT AssistID, const TResult& msg)
{

}

//Log Message Recv
void CServerGameDesk::LogMsgRecv(BYTE bDeskStation, UINT AssistID)
{
	CTime time = CTime::GetCurrentTime();
	LOGMODULEINSTANCE.WriteLog(
		"%02d,%02d,%02d,房间%02d,密码%s,%08x,%02d,%02d,\
		游戏状态,%d,客户端,%2d,%s\n", 
		time.GetHour(), time.GetMinute(), time.GetSecond(), m_bDeskIndex, m_szDeskPassWord, 0, m_iVipGameCount, m_iRunGameCount, 
		m_bGameStation,bDeskStation, GetCHNameByAssID(AssistID).c_str());
}

void CServerGameDesk::LogMsgRecv(BYTE bDeskStation, UINT AssistID, void *data, int dataSize)
{
	if (NULL != data && dataSize > 0)
	{
		switch(AssistID)
		{
		case ASS_BET:
			if (dataSize >= sizeof(TBet))
			{
				LogMsgRecv(bDeskStation, AssistID, *(TBet*)data);
				return;
			}
			break;
		default:
			break;
		}
	}

	LogMsgRecv(bDeskStation, AssistID);
}

void CServerGameDesk::LogMsgRecv(BYTE bDeskStation, UINT AssistID, const TBet& msg)
{
	if (ASS_BET == AssistID)
	{
		CTime time = CTime::GetCurrentTime();
		LOGMODULEINSTANCE.WriteLog(
			"%02d,%02d,%02d,房间%02d,密码%s,%08x,%02d,%02d,\
			游戏状态,%d,客户端,%2d,%s", 
			time.GetHour(), time.GetMinute(), time.GetSecond(), m_bDeskIndex, m_szDeskPassWord, 0, m_iVipGameCount, m_iRunGameCount, 
			m_bGameStation,bDeskStation, GetCHNameByAssID(AssistID).c_str());

		//msg Log
		LOGMODULEINSTANCE.WriteLog(",nType=%d", msg.nType);
		LOGMODULEINSTANCE.WriteLog(",byUser=%d", msg.byUser);
		LOGMODULEINSTANCE.WriteLog(",nMoney=%ld", msg.nMoney);

		LOGMODULEINSTANCE.WriteLog("\n");
	} else 
	{
		LogMsgRecv(bDeskStation, AssistID);
	}

}

//Log On Timer
void CServerGameDesk::LogOnTimer(UINT uTimerID)
{
	CTime time = CTime::GetCurrentTime();
	LOGMODULEINSTANCE.WriteLog(
		"%02d,%02d,%02d,房间%02d,密码%s,%08x,%02d,%02d,\
		游戏状态,%d,定时器,%2d,%s\n", 
		time.GetHour(), time.GetMinute(), time.GetSecond(), m_bDeskIndex, m_szDeskPassWord, 0, m_iVipGameCount, m_iRunGameCount, 
		m_bGameStation, uTimerID, GetTimerNameByID(uTimerID).c_str());
}

//Log On Remind
void CServerGameDesk::LogMsgRemind(const string& str)
{
	CTime time = CTime::GetCurrentTime();
	LOGMODULEINSTANCE.WriteLog(
		"%02d,%02d,%02d,房间%02d,密码%s,%08x,%02d,%02d,\
		游戏状态,%d,提示信息,%s\n", 
		time.GetHour(), time.GetMinute(), time.GetSecond(), m_bDeskIndex, m_szDeskPassWord, 0, m_iVipGameCount, m_iRunGameCount, 
		m_bGameStation, str.c_str());
}

void CServerGameDesk::LogMsgRemind(const string& str, BYTE bDeskStation)
{
	if (true)
	{
		CTime time = CTime::GetCurrentTime();
		LOGMODULEINSTANCE.WriteLog(
			"%02d,%02d,%02d,房间%02d,密码%s,%08x,%02d,%02d,\
			游戏状态,%d,提示信息,%s,%d",
			time.GetHour(), time.GetMinute(), time.GetSecond(), m_bDeskIndex, m_szDeskPassWord, 0, m_iVipGameCount, m_iRunGameCount, 
			m_bGameStation,str.c_str(),bDeskStation);

		LOGMODULEINSTANCE.WriteLog("\n");
	} else
	{
		LogMsgRemind(str);
	}
}

#endif // MODE_LOG_DEBUG