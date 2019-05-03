/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "StdAfx.h"
#include "GameDesk.h"
#include "GameRoomLogonDT.h"
#include <valarray>
#include "GameRoomMessage.h"
#include"..\common\WriteLog.h"
#include "commonuse.h"
#include "MD5.h"
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
//#include "AFCClass.h"
//宏定义 平台定时器范围51~59
#define IDT_CHECK_DESK				51L										//检测是否有人定时器 ID
#define IDT_DISMISS_TIMEOUT			52L										//申请解散超时，自动解散
#define IDT_CONTEST_SIT_TIMEOUT		53L										//排队坐下等待超时
#define IDT_DESKRUNOUT_TIMER        54L                                     //时效房间解散定时器

#define TIME_PLAY					30L										//默认等待时间（秒）
#define NET_CUT_TIMES				3										//断线次数

#define TIME_DISMISS_TIMEOUT		2*60*1000L								//申请解散超时时间（毫秒）
//构造函数
CGameDesk::CGameDesk(BYTE bBeginMode) : m_bBeginMode(bBeginMode)
{
    m_iBuyMinutes = 0;
	m_bLock=false;
	m_bPlayGame=false;
	m_bMaxPeople=0;
	m_bDeskIndex=0;
	m_szLockPass[0]=0;
	m_dwOwnerUserID=0L;
	m_dwZhuangjiaUserID = 0L;
	m_DeskBasePoint=0;
	m_bGameStation=0;	
	m_bEnableWatch=0;
	m_pDataManage=NULL;
	m_bWaitTime=TIME_PLAY;
	m_dwBeginTime=(long int)time(NULL);
	m_dwTax=0;
	m_bReturnDesk = false;
	memset(m_bIsMidEnter,0,sizeof(m_bIsMidEnter));
	memset(m_bCutGame,0,sizeof(m_bCutGame));
	memset(m_uCutTime,0,sizeof(m_uCutTime));
	memset(m_bConnect,0,sizeof(m_bConnect));
	memset(m_pUserInfo,0,sizeof(m_pUserInfo));
	memset(m_dwScrPoint,0,sizeof(m_dwScrPoint));
	memset(m_pMatchInfo,0,sizeof(m_pMatchInfo));
	memset(m_bBreakCount,0,sizeof(m_bBreakCount));
	memset(m_dwGameUserID,0,sizeof(m_dwGameUserID));
	memset(m_GameUserInfo,0,sizeof(m_GameUserInfo));
	memset(m_dwChangePoint,0,sizeof(m_dwChangePoint));
	memset(m_dwChangeMoney,0,sizeof(m_dwChangeMoney));
	memset(m_dwTaxCom,0,sizeof(m_dwTaxCom));
	memset(m_GRM_Key,0,sizeof(m_GRM_Key));
	memset(m_GRM_Key_win,0,sizeof(m_GRM_Key_win));
	memset(m_GRM_Key_los,0,sizeof(m_GRM_Key_los));
	m_pReturnInfo.clear();
	m_pDistance.clear();
	m_byDoublePointEffect = 0;

	m_bInDissmissing = false;
	m_iDissmissUserID=0;
	m_iDissmissAgreePeople=0;
	m_VecAgreeUserID.clear();

	m_bhavevideo = false;
	m_videoip = "127.0.0.1";
	m_videoport = 6677;
	m_audioport = 7766;

	m_bIsVirtualLock = false; 

	m_iVipGameCount=0;		//购买桌子局数
	m_bIsBuy=false;			//是否被购买
	m_iDeskMaster=0;
	m_iRunGameCount=0;		//游戏运行的局数
	m_iDissmissAgreePeople=0;

	M_bMidEnter=0;

	m_bPositionCheck = false;
	m_bIPCheck = false;

	m_bAllAgreeLocation = false;
	m_bInLocation = false;

	m_bIsPlay = false;
	m_iClubID = 0;

	m_bQueueReset = true;

	memset(m_szDeskPassWord,0,sizeof(m_szDeskPassWord));
	memset(m_szDeskConfig,0,sizeof(m_szDeskConfig));
	memset(m_GameSN,0,sizeof(m_GameSN));
	CString sPath=CBcfFile::GetAppPath();
	CBcfFile fsr(sPath + "SpecialRule.bcf");

	m_bLeave = true;		//默认离开

	for(int i = 0;i< 10;i++)
	{
		CString cs;
		cs.Format("Fish_%d",i+1);
		m_dwFishGamesNameID[i] = fsr.GetKeyVal(_T("FishType"),cs,0);
	}
	fsr.CloseFile();

	//初始化锁
	::InitializeCriticalSection(&m_csLock_);//百家乐
}

//析构函数
CGameDesk::~CGameDesk(void)
{
	::DeleteCriticalSection(&m_csLock_);//百家乐
}
int CGameDesk::CanSitDesk(MSG_GR_S_UserSit * pUserSit, CGameUserInfo * pUserInfo)
{
	//效验数据 
	ASSERT(pUserInfo!=NULL);

	CGameUserInfo * pUser = dynamic_cast<CGameUserInfo *>(m_pUserInfo[pUserSit->bDeskStation]);
	//效验状态
	if (pUser !=NULL && DESK_TYPE_BJL!=m_uDeskType)
	{
		return ERR_GR_ALREAD_USER;
	}

	//百家乐
	if (DESK_TYPE_BJL==m_uDeskType)
	{
		pUserSit->bDeskStation = FindStation(pUserInfo);

		if (m_bMaxPeople <= pUserSit->bDeskStation)
		{
			return ERR_GR_DESK_FULL;
		}
	}
	else
	{
		bool _flag = true;
		for (BYTE i=0;i<m_bMaxPeople;i++)
		{
			if(m_pUserInfo[i]==NULL) 
			{
				_flag = false;
				break;
			}
		}

	//	if(IsPlayingByGameStation() && _flag) return ERR_GR_BEGIN_GAME;

		if (m_pUserInfo[pUserSit->bDeskStation]!=NULL)
			return ERR_GR_ALREAD_USER;

		if (m_pDataManage->m_InitData.dwRoomRule & GRR_LIMIT_SAME_IP)
		{
			for (BYTE i = 0; i < m_bMaxPeople; ++i)
			{
				if (m_pUserInfo[i] != NULL)
				{
					if (m_pUserInfo[i]->m_UserData.dwUserIP == pUserInfo->m_UserData.dwUserIP)
					{
						return ERR_GR_IP_SAME;
					}
				}
			}
		}
		

		//恢复房间判断做下位置是否正确
		if (m_pDataManage->m_InitData.dwRoomRule & GRR_GAME_BUY)
		{
			if (1==m_pDataManage->m_InitData.iLockType && m_GameUserInfo[pUserSit->bDeskStation] != pUserInfo->m_UserData.dwUserID && (m_iRunGameCount>0 || IsPlayGame(0) || m_GameUserInfo[pUserSit->bDeskStation]!=0))
				return ERR_GR_NOT_FIX_STATION;
			if (2==m_pDataManage->m_InitData.iLockType && (m_GameUserInfo[pUserSit->bDeskStation]!=pUserInfo->m_UserData.dwUserID && m_GameUserInfo[pUserSit->bDeskStation]!=0))
				return ERR_GR_NOT_FIX_STATION;
			if (M_bMidEnter  && (m_GameUserInfo[pUserSit->bDeskStation]!=pUserInfo->m_UserData.dwUserID && m_GameUserInfo[pUserSit->bDeskStation]!=0))
				return ERR_GR_NOT_FIX_STATION;
			if (!M_bMidEnter && m_GameUserInfo[pUserSit->bDeskStation] != pUserInfo->m_UserData.dwUserID && (m_iRunGameCount>0 || IsPlayGame(0) || m_GameUserInfo[pUserSit->bDeskStation]!=0))
				return ERR_GR_NOT_FIX_STATION;
			if (2==m_iPayType && 1==pUserInfo->m_UserData.bUserState && m_iAANeedJewels > pUserInfo->m_UserData.iJewels)	// 中途加入判断钻石
				return ERR_GR_JEWEL_LIMIT;
		}
		//end

		int peopleCnt = 0; ///< 查看一张桌子有多少个人，如果没有人则不需要密码
		//不于IP相同或不于IP前几位的人游戏
		for (BYTE i=0;i<m_bMaxPeople;i++)
		{
			ULONG dwUserIP=pUserInfo->m_UserData.dwUserIP;
			if(m_pUserInfo[i]==NULL)
				continue;
			peopleCnt++;
			if (pUserInfo->m_Rule.bIPLimite||m_pUserInfo[i]->m_Rule.bIPLimite)
			{
				//不于IP前3位相同的人游戏（放在这里可以减少循环次数）
				if((pUserInfo->m_Rule.bIPLimite>=3 || m_pUserInfo[i]->m_Rule.bIPLimite>=3) &&
					FOURTH_IPADDRESS(m_pUserInfo[i]->m_UserData.dwUserIP)==FOURTH_IPADDRESS(dwUserIP))
					return ERR_GR_IP_SAME_3;
				//不于IP前4位相同的人游戏（放在这里可以减少循环次数）
				//记录第4位IP
				CString szBuffer;
				int sz1,sz2;
				szBuffer.Format(TEXT("%d"),THIRD_IPADDRESS(dwUserIP));
				sz1 = szBuffer.GetAt(1);
				if(pUserInfo->m_Rule.bIPLimite==4||m_pUserInfo[i]->m_Rule.bIPLimite==4)
				{
					szBuffer.Format(TEXT("%d"),THIRD_IPADDRESS(m_pUserInfo[i]->m_UserData.dwUserIP));
					sz2 = szBuffer.GetAt(1);
					if(sz1==sz2 && 
						FOURTH_IPADDRESS(m_pUserInfo[i]->m_UserData.dwUserIP)==FOURTH_IPADDRESS(dwUserIP))
						return ERR_GR_IP_SAME_4;
				}
			}
		}

		//判断是否限制位置
		if (m_pDataManage->m_InitData.uComType!=TY_MATCH_GAME && !(m_pDataManage->m_InitData.dwRoomRule & GRR_CONTEST || m_pDataManage->m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
		{
			//金币场分数效验
			if ((m_pDataManage->m_InitData.uComType==TY_MONEY_GAME)
				&& !(m_pDataManage->m_InitData.dwRoomRule&GRR_GAME_BUY)&&
				(m_pDataManage->m_InitData.uLessPoint!=0)&&
				(pUserInfo->m_UserData.i64Money<m_pDataManage->m_InitData.uLessPoint))
			{
				return ERR_GR_POINT_LIMIT;
			}

			//密码效验
			if (m_bLock)
			{
				if (lstrcmp(m_szLockPass,pUserSit->szPassword) != 0)
					return ERR_GR_PASS_ERROR;
			}

			//判断用户设置
			BYTE bErrorCode=0;
			for (BYTE i=0;i<m_bMaxPeople;i++)
			{
				if ((m_pUserInfo[i]!=NULL)&&(m_pUserInfo[i]->IsFixRule(&pUserInfo->m_Rule,
					&pUserInfo->m_UserData,bErrorCode)==false))
					return bErrorCode;
			}

			//标志玩家为中途加入
			if (IsPlayGame(0))	
				m_bIsMidEnter[pUserSit->bDeskStation]=true;
		}
	}
	return ERR_GR_SIT_SUCCESS;
}


//用户坐到游戏桌
BYTE CGameDesk::UserSitDesk(MSG_GR_S_UserSit * pUserSit, CGameUserInfo * pUserInfo)
{
	//效验数据 
	ASSERT(pUserInfo!=NULL);

	CGameUserInfo * pUser = dynamic_cast<CGameUserInfo *>(m_pUserInfo[pUserSit->bDeskStation]);
	//效验状态
	if (pUser !=NULL && DESK_TYPE_BJL!=m_uDeskType)
	{
		return ERR_GR_ALREAD_USER;
	}

	//百家乐
	if (DESK_TYPE_BJL==m_uDeskType)
	{
		pUserSit->bDeskStation = FindStation(pUserInfo);

		if (m_bMaxPeople <= pUserSit->bDeskStation)
		{
			return ERR_GR_DESK_FULL;
		}

		if ((m_pDataManage->m_InitData.uComType==TY_MONEY_GAME)
			&& !(m_pDataManage->m_InitData.dwRoomRule&GRR_GAME_BUY) &&
			(m_pDataManage->m_InitData.uLessPoint!=0)&&
			(pUserInfo->m_UserData.i64Money<m_pDataManage->m_InitData.uLessPoint))
			return ERR_GR_POINT_LIMIT;
	}//end of 百家乐
	else
	{
		if ((m_pDataManage->m_InitData.dwRoomRule & GRR_GAME_BUY))
		{
			if (1==m_pDataManage->m_InitData.iLockType && m_GameUserInfo[pUserSit->bDeskStation] != pUserInfo->m_UserData.dwUserID  && (m_iRunGameCount>0 || IsPlayGame(0) || m_GameUserInfo[pUserSit->bDeskStation]!=0))
				return ERR_GR_NOT_FIX_STATION;
			if (2==m_pDataManage->m_InitData.iLockType && (m_GameUserInfo[pUserSit->bDeskStation]!=pUserInfo->m_UserData.dwUserID && m_GameUserInfo[pUserSit->bDeskStation]!=0))
				return ERR_GR_NOT_FIX_STATION;
			if ((1==m_pDataManage->m_InitData.bLockMaster || 0 == m_bMasterState) && pUserInfo->m_UserData.dwUserID!=m_iDeskMaster)
			{
				bool isMasterExist = false;//房主是否在桌子
				int  iDeskPeople = 0;
				for (int i=0;i<m_bMaxPeople;i++)
				{
					if(m_pUserInfo[i]!=NULL)
					{
						iDeskPeople++;
						if (m_pUserInfo[i]->m_UserData.dwUserID == m_iDeskMaster)
						{
							isMasterExist = true;
							break;
						}
					}
				}
				if (!isMasterExist && iDeskPeople>=(m_bMaxPeople-1))
				{
					return ERR_GR_DESK_FULL;
				}
			}
			if (m_bIsBuy && m_bPositionCheck && (pUserInfo->m_UserData.flnt<0.000001 && pUserInfo->m_UserData.flnt>-0.000001) && (pUserInfo->m_UserData.flat<0.000001 && pUserInfo->m_UserData.flat>-0.000001))
			{
				return ERR_GR_POSITION_FAIL;
			}
		}
		bool _flag = true;
		for (BYTE i=0;i<m_bMaxPeople;i++)
		{
			if(m_pUserInfo[i]==NULL) 
			{
				_flag = false;
				break;
			}
		}

		/// 百人游戏不需要等所有玩家都准备好后才开始游戏，因此只在这里判断是否正在游戏中
		//考虑捕鱼玩家是在游戏进行中进来的
		CString ctext;
		ctext.Format("%d",m_pDataManage->m_DllInfo.uNameID);

		if (ctext.Left(3) == "300" && ctext.GetAt(4) == '0')
		{
			if(_flag) 
				return ERR_GR_BEGIN_GAME;
		}
		else
		{
			if(IsPlayingByGameStation() && _flag) 
				return ERR_GR_BEGIN_GAME;
		}

		//效验状态
		if (m_pUserInfo[pUserSit->bDeskStation]!=NULL)
			return ERR_GR_ALREAD_USER;
		
		//如果是限制IP房间，与桌上各玩家比对IP,IP与其中一个玩家相同,则返回ERR_GR_IP_SAME
		if (m_pDataManage->m_InitData.dwRoomRule & GRR_LIMIT_SAME_IP)
		{
			for (BYTE i = 0; i < m_bMaxPeople; ++i)
			{
				if (m_pUserInfo[i] != NULL)
				{
					if (m_pUserInfo[i]->m_UserData.dwUserIP == pUserInfo->m_UserData.dwUserIP)
					{
						return ERR_GR_IP_SAME;
					}
				}
			}
		}
		//End Add


		int peopleCnt = 0; ///< 查看一张桌子有多少个人，如果没有人则不需要密码
		//不于IP相同或不于IP前几位的人游戏
		for (BYTE i=0;i<m_bMaxPeople;i++)
		{
			ULONG dwUserIP=pUserInfo->m_UserData.dwUserIP;
			if(m_pUserInfo[i]==NULL)
				continue;
			peopleCnt++;
			if (pUserInfo->m_Rule.bIPLimite||m_pUserInfo[i]->m_Rule.bIPLimite)
			{
				//不于IP前3位相同的人游戏（放在这里可以减少循环次数）
				if((pUserInfo->m_Rule.bIPLimite>=3 || m_pUserInfo[i]->m_Rule.bIPLimite>=3) &&
					FOURTH_IPADDRESS(m_pUserInfo[i]->m_UserData.dwUserIP)==FOURTH_IPADDRESS(dwUserIP))
					return ERR_GR_IP_SAME_3;
				//不于IP前4位相同的人游戏（放在这里可以减少循环次数）
				//记录第4位IP
				CString szBuffer;
				int sz1,sz2;
				szBuffer.Format(TEXT("%d"),THIRD_IPADDRESS(dwUserIP));
				sz1 = szBuffer.GetAt(1);
				if(pUserInfo->m_Rule.bIPLimite==4||m_pUserInfo[i]->m_Rule.bIPLimite==4)
				{
					szBuffer.Format(TEXT("%d"),THIRD_IPADDRESS(m_pUserInfo[i]->m_UserData.dwUserIP));
					sz2 = szBuffer.GetAt(1);
					if(sz1==sz2 && 
						FOURTH_IPADDRESS(m_pUserInfo[i]->m_UserData.dwUserIP)==FOURTH_IPADDRESS(dwUserIP))
						return ERR_GR_IP_SAME_4;
				}
			}
		}
		/////////////////////////////////////////////////////////////////////
		if (0 == peopleCnt)
			m_bLock = false;

		//判断是否限制位置
		if (m_pDataManage->m_InitData.uComType!=TY_MATCH_GAME&& !(m_pDataManage->m_InitData.dwRoomRule & GRR_CONTEST || m_pDataManage->m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
		{
			//金币场分数效验
			if ((m_pDataManage->m_InitData.uComType==TY_MONEY_GAME)
				&& !(m_pDataManage->m_InitData.dwRoomRule&GRR_GAME_BUY) &&
				(m_pDataManage->m_InitData.uLessPoint!=0)&&
				(pUserInfo->m_UserData.i64Money<m_pDataManage->m_InitData.uLessPoint))
				return ERR_GR_POINT_LIMIT;

			//密码效验
			if (m_bLock==true)
			{
				if (lstrcmp(m_szLockPass,pUserSit->szPassword)!=0)//20081205
					return ERR_GR_PASS_ERROR;
			}

			//判断用户设置
			BYTE bErrorCode=0;
			for (BYTE i=0;i<m_bMaxPeople;i++)
			{
				if ((m_pUserInfo[i]!=NULL)&&(m_pUserInfo[i]->IsFixRule(&pUserInfo->m_Rule,
					&pUserInfo->m_UserData,bErrorCode)==false)) return bErrorCode;
			}
		}

		//更新规则
		UINT uSitCount=0;
		for (BYTE i=0;i<m_bMaxPeople;i++)
		{
			if(m_pUserInfo[i]!=NULL) uSitCount++;
		}


		if (uSitCount==0)
		{
			if (m_pDataManage->m_InitData.uComType==TY_MATCH_GAME)
			{
				m_bLock=false;
				m_dwOwnerUserID=0L;
			}
			else
			{
				m_bLock=(pUserInfo->m_Rule.bPass==TRUE);
				m_dwOwnerUserID=pUserInfo->m_UserData.dwUserID;
			}
			if (m_bLock==true)
			{
				CopyMemory(m_szLockPass,pUserInfo->m_Rule.szPass,sizeof(m_szLockPass));
				m_szLockPass[sizeof(m_szLockPass)/sizeof(m_szLockPass[0])-1]=0;
			}
		}
	}

	//设置用户数据
	m_bCutGame[pUserSit->bDeskStation] = false;
	m_uCutTime[pUserSit->bDeskStation]=0L;
	m_bConnect[pUserSit->bDeskStation]=false;
	m_pUserInfo[pUserSit->bDeskStation]=pUserInfo;
	m_bEnableWatch&=~(1<<pUserSit->bDeskStation);
	pUserInfo->m_UserData.bDeskNO=m_bDeskIndex;
	pUserInfo->m_UserData.bDeskStation=pUserSit->bDeskStation;
	pUserInfo->m_UserData.bUserState=USER_SITTING;
	
	if (pUserInfo->m_UserData.bLogonbyphone && !m_pDataManage->IsQueueGameRoom())
	{
		SendDeskUserInfo(pUserInfo->GetSocketIndex());
	}

	if (m_bIsBuy)
	{
		SendReturnInfo(pUserInfo->GetSocketIndex());
	}
	
	//发送房间消息
	MSG_GR_R_UserSit UserSit;
	UserSit.bLock=m_bLock;
	UserSit.bDeskIndex=m_bDeskIndex;
	UserSit.bDeskStation=pUserSit->bDeskStation;
	UserSit.bUserState=USER_SITTING;
	UserSit.dwUserID=pUserInfo->m_UserData.dwUserID;
	UserSit.bIsDeskOwner=(m_dwOwnerUserID==pUserInfo->m_UserData.dwUserID);//是台主
	UserSit.bDeskMaster=(m_bIsBuy&&m_iDeskMaster==pUserInfo->m_UserData.dwUserID);//房主

	if (!m_pDataManage->IsQueueGameRoom())
		m_pDataManage->m_TCPSocket.SendDataBatch(&UserSit,sizeof(UserSit),MDM_GR_USER_ACTION,ASS_GR_USER_SIT,ERR_GR_SIT_SUCCESS);

	KillTimer(IDT_CHECK_DESK);

	if (m_bIsBuy &&m_bInDissmissing && m_iDissmissUserID!=0 && DESK_TYPE_BJL!=m_uDeskType)
	{
		vector<int>::iterator result = find( m_VecAgreeUserID.begin( ), m_VecAgreeUserID.end( ), pUserInfo->m_UserData.dwUserID ); 
		if ( result == m_VecAgreeUserID.end( ) ) 
		{

		}
		else 
		{
			m_iDissmissAgreePeople++;
		}
	}
	
	if (m_bIsBuy)
	{
		if (pUserInfo->m_UserData.dwUserID == m_iDeskMaster)
		{
			DL_GR_I_MasterLeave MasterLeave;
			MasterLeave.bLeave = false;
			MasterLeave.bMaster = true;
			MasterLeave.iDeskID = m_bDeskIndex;
			MasterLeave.iUserID = pUserInfo->m_UserData.dwUserID;

			m_pDataManage->m_SQLDataManage.PushLine(&MasterLeave.DataBaseHead, sizeof(MasterLeave), DTK_GR_MASTER_LEAVE, 0, 0);	

			m_bMasterState = 0;
		}
		else
		{
			DL_GR_I_MasterLeave MasterLeave;
			MasterLeave.bLeave = false;
			MasterLeave.bMaster = false;
			MasterLeave.iDeskID = m_bDeskIndex;
			MasterLeave.iUserID = pUserInfo->m_UserData.dwUserID;

			m_pDataManage->m_SQLDataManage.PushLine(&MasterLeave.DataBaseHead, sizeof(MasterLeave), DTK_GR_MASTER_LEAVE, 0, 0);	
		}
	}

	return ERR_GR_SIT_SUCCESS;
}


//百家乐
//desc:给玩家安排位置
//return:玩家的位置号(0,1,2...)
BYTE CGameDesk::FindStation(CGameUserInfo * pUserInfo)
{
	BYTE i;

	::EnterCriticalSection(&m_csLock_);
	try
	{
		for (i=0;i<m_bMaxPeople;i++)
		{
			if(m_pUserInfo[i]!=NULL && m_pUserInfo[i]->m_UserData.bDeskStation == pUserInfo->m_UserData.bDeskStation)
			{
				return i;
			}
		}

		for (i=0;i<m_bMaxPeople;i++)
		{
			if(m_pUserInfo[i]==NULL)
			{
				m_pUserInfo[i]=pUserInfo;
				break;
			}
		}
	}
	catch (...)
	{
		::LeaveCriticalSection(&m_csLock_);
		throw; 
	}

	::LeaveCriticalSection(&m_csLock_);

	return i;
}

//---------------------------------------以下为获取视频是否启动的加密码-----------------------
/// 根据全局变量中的加密方式，取得加密后的密码字符串是MD5
/// 函数里不对指针的合法性作判断
/// @param szMD5Pass 加密后的字符串
/// @param szSrcPass 源字符串
/// @return 加密后的字符串指针
TCHAR *GetCryptedPasswd(TCHAR *szMD5Pass, TCHAR *szSrcPass)
{
	unsigned char szMDTemp[16];
	TCHAR myKey = 'G';
	MD5_CTX Md5;
	Md5.MD5Update((unsigned char *)szSrcPass,lstrlen(szSrcPass));
	Md5.MD5Final(szMDTemp);
	for (int i = 0; i < 16; i ++) 
		wsprintf(&szMD5Pass[i * 2], "%02x", szMDTemp[i]^myKey);
	return szMD5Pass;
}
CString innerGetMac()
{
	CString retMac="";
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter=NULL;
	DWORD dwRetVal=0;
	pAdapterInfo=(IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
	ULONG ulOutBufLen=sizeof(IP_ADAPTER_INFO);
	if(GetAdaptersInfo(pAdapterInfo,&ulOutBufLen)!=ERROR_SUCCESS)
	{
		free(pAdapterInfo);
		pAdapterInfo=(IP_ADAPTER_INFO*)malloc(ulOutBufLen);
	}
	if((dwRetVal=GetAdaptersInfo(pAdapterInfo,&ulOutBufLen))==NO_ERROR)
	{
		pAdapter=pAdapterInfo;
		CString temp;
		while(pAdapter)
		{
			if(pAdapter->Type==6)//pAdapter->Description中包含"PCI"为：物理网卡//pAdapter->Type是71为：无线网卡
			{
				for(UINT i=0;i<pAdapter->AddressLength;i++)
				{
					temp.Format("%02X",pAdapter->Address[i]);
					retMac+=temp;
					//	printf("%02X%c",pAdapter->Address[i],i==pAdapter->AddressLength-1?’n’:’-’);
				}
				break;
			}
			pAdapter=pAdapter->Next;
		}
	}
	if(pAdapterInfo)
		free(pAdapterInfo);
	return retMac;
}

//获取机器码相关函数
CString coreGetCode()
{
	CString strRet="";
	CString str=innerGetMac(),s;
	if(str=="")
	{		
		return _T("");
	}

	int r=0;
	int l=str.GetLength();
	for(int i=0;i<l;i+=2)
	{
		int r1=0,r2=0;
		s=str.Mid(i,1);
		if(s>="0" && s<="9")
			r1=atoi(s);
		if(s=="A" || s=="a")	r1=10;
		if(s=="B" || s=="b")	r1=11;
		if(s=="C" || s=="c")	r1=12;
		if(s=="D" || s=="d")	r1=13;
		if(s=="E" || s=="e")	r1=14;
		if(s=="F" || s=="f")	r1=15;
		s=str.Mid(i+1,1);
		if(s>="0" && s<="9")
			r2=atoi(s);
		if(s=="A" || s=="a")	r2=10;
		if(s=="B" || s=="b")	r2=11;
		if(s=="C" || s=="c")	r2=12;
		if(s=="D" || s=="d")	r2=13;
		if(s=="E" || s=="e")	r2=14;
		if(s=="F" || s=="f")	r2=15;

		CString t;
		r+=r1*16+r2;
		srand(r);
		t.Format("%s%04X%s-",str.Mid(l-i-1,1),rand(),str.Mid(l-i-2,1));
		strRet+=t;
	}
	if(strRet.Right(1)=="-")strRet=strRet.Left(strRet.GetLength()-1);
	return strRet;
}
//---------------------------------------以上为获取视频是否启动的加密码-----------------------

//初始化函数
bool CGameDesk::Init(BYTE bDeskIndex, BYTE bMaxPeople, CGameMainManage * pDataManage, UINT	uDeskType)//百家乐lym
{
	m_bDeskIndex=bDeskIndex;
	m_bMaxPeople=bMaxPeople;
	m_pDataManage=pDataManage;
	if (pDataManage->m_InitData.dwRoomRule&GRR_ENABLE_WATCH) m_bEnableWatch=0xFF;
	if (pDataManage->m_InitData.dwRoomRule&GRR_UNENABLE_WATCH) m_bEnableWatch=0x00;
	SetTableBasePoint(pDataManage->m_InitData.uBasePoint);
	m_uDeskType = uDeskType;
	InitDeskGameStation();

	//初始化视频服务器信息
	CString s = CINIFile::GetAppPath ();/////本地路径
	CString nid;
	nid.Format("%d", m_pDataManage->m_KernelData.uNameID);
	CINIFile f( s +nid +"_s.ini");
	CString strPswd = f.GetKeyVal("videosrv","videocode","");
	CString strSrcCode;//原始码
	strSrcCode.Format(_T("%s%d"), coreGetCode(), pDataManage->m_KernelData.uNameID);
	TCHAR szMD5Pass[64];	memset(szMD5Pass, 0, 64);
	m_bhavevideo = !strcmp(strPswd, GetCryptedPasswd(szMD5Pass, strSrcCode.GetBuffer(strSrcCode.GetLength())));
	m_bhavevideo &= !m_pDataManage->IsQueueGameRoom();//防作弊房间和比赛房不开启视频功能
	m_videoip = f.GetKeyVal("videosrv","ip",m_videoip.c_str()).GetBuffer();	
	m_videoport = f.GetKeyVal("videosrv","videoport",m_videoport);
	m_audioport = f.GetKeyVal("videosrv","audioport",m_audioport);	

	return true;
}

//将视频服务器信息发送到客户端
void CGameDesk::send_video_ip(BYTE bDeskStation)
{
	if (!m_bhavevideo) return;
	SetVideoSrvStruct msg;
	strcpy(msg.szIP, m_videoip.c_str());
	msg.videoport = m_videoport;
	msg.audioport = m_audioport;
	SendGameData(bDeskStation,&msg,sizeof(msg),MDM_GM_GAME_FRAME,ASS_GM_SET_VIDEOADDR,0);
}

//是否允许旁观
bool CGameDesk::IsEnableWatch(BYTE bDeskStation)
{
	return ((m_bEnableWatch&(1<<bDeskStation))!=0);
}

//设置定时器
bool CGameDesk::SetTimer(UINT uTimerID, int uElapse)
{
	if (uTimerID>=TIME_SPACE) return false;
	return m_pDataManage->SetTimer(m_bDeskIndex*TIME_SPACE+uTimerID+TIME_START_ID,uElapse);
}

//删除定时器
bool CGameDesk::KillTimer(UINT uTimerID)
{
	if (uTimerID>=TIME_SPACE) return false;
	return m_pDataManage->KillTimer(m_bDeskIndex*TIME_SPACE+TIME_START_ID+uTimerID);
}

//获取游戏时间
long int CGameDesk::GetPlayTimeCount()
{
	if (m_bPlayGame==false) return 0L;
	return (long int)time(NULL)-m_dwBeginTime;
}

//用户断线离开
bool CGameDesk::UserNetCut(BYTE bDeskStation, CGameUserInfo * pLostUserInfo)
{
	//效验数据
	ASSERT(m_bCutGame[bDeskStation]==false);
	ASSERT(m_pUserInfo[bDeskStation]!=NULL);
	ASSERT(m_pUserInfo[bDeskStation]->m_UserData.dwUserID==pLostUserInfo->m_UserData.dwUserID);

	//设置数据
	m_bCutGame[bDeskStation]=true;
	m_bConnect[bDeskStation]=false;
	m_bBreakCount[bDeskStation]++;
	m_pUserInfo[bDeskStation]=pLostUserInfo;
	//发送用户断线消息
	MSG_GR_R_UserCut UserCut;
	UserCut.bDeskNO=m_bDeskIndex;
	UserCut.bDeskStation=bDeskStation;
	UserCut.dwUserID=pLostUserInfo->m_UserData.dwUserID;
	m_pDataManage->m_TCPSocket.SendDataBatch(&UserCut,sizeof(UserCut),MDM_GR_USER_ACTION,ASS_GR_USER_CUT,0);
	if((m_pDataManage->m_InitData.dwRoomRule & GRR_GAME_BUY))
	{
		int iCutPeople=0;
		for(int i=0;i<m_bMaxPeople;i++)
		{
			if(!m_pUserInfo[i] || !m_bCutGame[i])
			{
				continue;
			}
			iCutPeople++;
		}
		if(iCutPeople>=GetDeskPlayerNum())
		{
			KillTimer(IDT_CHECK_DESK);
			SetTimer(IDT_CHECK_DESK,3*60*1000);
		}
	}
	//设定定时器
	if (DESK_TYPE_BJL==m_uDeskType)	// 如果是百家乐类型的游戏，则直接令玩家结束，否则会产生定时器ID混乱
	{
		if ((m_pUserInfo[bDeskStation]!=NULL)&&(m_bCutGame[bDeskStation]==true))
		{
			GameFinish(bDeskStation,GFF_FORCE_FINISH);
		}
	}
	
	if(m_bIsBuy && m_bInDissmissing)
	{
		vector<int>::iterator result = find( m_VecAgreeUserID.begin( ), m_VecAgreeUserID.end( ), pLostUserInfo->m_UserData.dwUserID ); 
		if ( result == m_VecAgreeUserID.end( ) ) 
		{
			if(m_iDissmissAgreePeople>=GetOnlineDeskPlayerNum())
			{
				GameFinish(0,GFF_DISSMISS_FINISH);
			}
			else
			{
				m_VecAgreeUserID.push_back(pLostUserInfo->m_UserData.dwUserID);
				VipDeskDismissAgreeRes tAgreeData;
				tAgreeData.bAgree=true;
				tAgreeData.iUserID=pLostUserInfo->m_UserData.dwUserID;
				for(int i=0;i<m_bMaxPeople;i++)
				{
					if(!m_pUserInfo[i])
					{
						continue;
					}
					m_pDataManage->m_TCPSocket.SendData(m_pUserInfo[i]->m_uSocketIndex,&tAgreeData,sizeof(tAgreeData),MDM_GR_DESKRUNOUT,ASS_GR_DISSMISS_AGREE,0,0);
				}
			}
		}
		else 
		{
			m_iDissmissAgreePeople--;

		}
	}

	return true;
}

//用户断线重来
bool CGameDesk::UserReCome(BYTE bDeskStation, CGameUserInfo * pNewUserInfo)
{
	//效验数据
	ASSERT(m_bCutGame[bDeskStation]==true);
	ASSERT(m_pUserInfo[bDeskStation]!=NULL);
	ASSERT(m_pUserInfo[bDeskStation]->m_UserData.dwUserID==pNewUserInfo->m_UserData.dwUserID);
	//设置数据
	m_bCutGame[bDeskStation]=false;
	m_pUserInfo[bDeskStation]=pNewUserInfo;
	m_pUserInfo[bDeskStation]->m_UserData.bUserState = pNewUserInfo->m_UserData.bUserState; // 还原之前的状态


	//{用户断线重连后也需要些W记录
	DL_GR_I_UserRecome userRecome;
	memset(&userRecome, 0, sizeof(userRecome));
	userRecome.lUserID = pNewUserInfo->m_UserData.dwUserID;
	m_pDataManage->m_SQLDataManage.PushLine(&userRecome.DataBaseHead,sizeof(userRecome),DTK_GR_USER_RECOME,0,0);
	//添加结束}

	if (pNewUserInfo->m_UserData.bLogonbyphone /*&& !m_pDataManage->IsQueueGameRoom()*/)
	{
		SendDeskUserInfo(pNewUserInfo->GetSocketIndex());
	}

	if (m_bIsBuy)
	{
		SendReturnInfo(pNewUserInfo->GetSocketIndex());
	}

	//发送房间消息
	MSG_GR_R_UserSit UserSit;
	UserSit.bLock=m_bLock;
	UserSit.bDeskIndex=m_bDeskIndex;
	UserSit.bDeskStation=bDeskStation;
	UserSit.bUserState=m_pUserInfo[bDeskStation]->m_UserData.bUserState;
	UserSit.dwUserID=pNewUserInfo->m_UserData.dwUserID;
	UserSit.bDeskMaster=(m_bIsBuy&&m_iDeskMaster==pNewUserInfo->m_UserData.dwUserID);//房主
	m_pDataManage->m_TCPSocket.SendDataBatch(&UserSit,sizeof(UserSit),MDM_GR_USER_ACTION,ASS_GR_USER_SIT,ERR_GR_SIT_SUCCESS);

	if (m_bIsBuy &&m_bInDissmissing && m_iDissmissUserID!=0 && DESK_TYPE_BJL!=m_uDeskType)
	{
		vector<int>::iterator result = find( m_VecAgreeUserID.begin( ), m_VecAgreeUserID.end( ), pNewUserInfo->m_UserData.dwUserID ); 
		if ( result == m_VecAgreeUserID.end( ) ) 
		{
			
		}
		else 
		{
			m_iDissmissAgreePeople++;
		}

	}

	KillTimer(IDT_CHECK_DESK);

	return true;
}

//设置比赛信息
bool CGameDesk::SetMatchInfo(BYTE bDeskStation, MatchInfoStruct * pMatchInfo, bool bNotify)
{
	ASSERT(bDeskStation<m_bMaxPeople);
	m_pMatchInfo[bDeskStation]=pMatchInfo;
	return true;
}

//清理断线资料
bool CGameDesk::CleanCutGameInfo()
{
	for (BYTE i=0;i<m_bMaxPeople;i++)
	{
		if ((m_pUserInfo[i]!=NULL)&&(m_bCutGame[i]==true))
		{
			//用户离开
			CGameUserInfo * pUserInfo = m_pUserInfo[i];
			//清除断线标志
			DL_GR_I_ClearOnlineFlag dtClearOnlineFlag;
			memset(&dtClearOnlineFlag, 0, sizeof(dtClearOnlineFlag));
			dtClearOnlineFlag.lUserID = pUserInfo->m_UserData.dwUserID;
			m_pDataManage->m_SQLDataManage.PushLine(&dtClearOnlineFlag.DataBaseHead, sizeof(dtClearOnlineFlag), DTK_GR_CLEAR_ONLINE_FLAG, 0, 0);
			MakeUserOffLine(i);
		}
		m_bCutGame[i]=false;
		if (m_pDataManage->m_InitData.dwRoomRule & GRR_CONTEST || m_pDataManage->m_InitData.dwRoomRule & GRR_TIMINGCONTEST)
		{
			if (m_pUserInfo[i] == NULL)
				continue;
			m_pUserInfo[i]=NULL;
		}
	}

	return true;
}
//比赛场清理断线资料
bool CGameDesk::CleanCutGameInfoContest()
{
	if (!(m_pDataManage->m_InitData.dwRoomRule & GRR_CONTEST) && !(m_pDataManage->m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
	{
		return true;
	}

	for (BYTE i=0;i<m_bMaxPeople;i++)
	{
		if ((m_pUserInfo[i]!=NULL)&&(m_bCutGame[i]==true))
		{
			//清理数据
			//用户离开
			CGameUserInfo * pUserInfo = m_pUserInfo[i];
			MakeUserOffLine(i);
			//清除断线标志
			DL_GR_I_ClearOnlineFlag dtClearOnlineFlag;
			memset(&dtClearOnlineFlag, 0, sizeof(dtClearOnlineFlag));
			dtClearOnlineFlag.lUserID = pUserInfo->m_UserData.dwUserID;
			m_pDataManage->m_SQLDataManage.PushLine(&dtClearOnlineFlag.DataBaseHead, sizeof(dtClearOnlineFlag), DTK_GR_CLEAR_ONLINE_FLAG, 0, 0);
		}
		m_bCutGame[i]=false;
		m_pUserInfo[i]=NULL;
	}

	return true;
}
//清理断线指定断线玩家资料
bool CGameDesk::CleanCutGameInfo(BYTE bDeskStation)
{
	if ((m_pUserInfo[bDeskStation]!=NULL)&&(m_bCutGame[bDeskStation]==true))
	{
		//用户离开
		CGameUserInfo * pUserInfo=m_pUserInfo[bDeskStation];
		MakeUserOffLine(bDeskStation);
		//清理用户资料
		//m_pDataManage->CleanUserInfo(pUserInfo);
		//m_pDataManage->m_UserManage.FreeUser(pUserInfo,false);
	}
	m_bCutGame[bDeskStation]=false;

	return true;
}

//记录游戏信息
bool CGameDesk::RecoderGameInfo(__int64 *ChangeMoney)
{
	if ((m_pDataManage->m_InitData.dwRoomRule&GRR_RECORD_GAME)!=0L/* && !m_bAllRobot*/)
	{
		memset(ChangeMoney,0,sizeof(ChangeMoney));
		//定义数据
		DL_GR_I_GameRecord GameRecord;
		memset(&GameRecord,0,sizeof(GameRecord));

		//写入数据
		GameRecord.dwTax=m_dwTax;//每局所有玩家总实缴税收
		GameRecord.bDeskIndex=m_bDeskIndex;
		GameRecord.uRoomID=m_pDataManage->m_InitData.uRoomID;
		GameRecord.dwBeginTime=m_dwBeginTime;

		int iCount =0;
		for (BYTE i=0;i<m_bMaxPeople;i++)
		{
			if(m_pUserInfo[i]== NULL)
				continue;

			ChangeMoney[i]=m_dwChangeMoney[i];

			{
				GameRecord.dwUserID[i]=m_pUserInfo[i]->m_UserData.dwUserID;
				GameRecord.dwScrPoint[i]=m_dwScrPoint[i]+m_dwChangePoint[i];
				GameRecord.dwTaxCom[i]=m_dwTaxCom[i];
				GameRecord.dwChangePoint[i]=m_dwChangePoint[i];
				GameRecord.dwChangeMoney[i]=m_dwChangeMoney[i];

				GameRecord.i64ScrMoney[i]=m_pUserInfo[i]->m_UserData.i64Money;


				iCount++;
			}
		}
		if (iCount > 0)
		{
			return m_pDataManage->m_SQLDataManage.PushLine(&GameRecord.DataBaseHead,sizeof(GameRecord),DTK_GR_RECORD_GAME,0,0);
		}
	}
	return true;
}

/// 赠送游戏币,20把
/// param void
/// return void
void CGameDesk::PresentCoin()
{
	return;
}

/// 把一个__int64转化为int
int I64ToInt(__int64 i64Value)
{
	__int64 flag = i64Value & 0x8000000000000000;
	if (flag != 0)
	{
		/// 是负数
		if (i64Value<0xFFFFFFFF80000001)
		{
			return 0x80000001;
		}
	}
	else
	{
		if (i64Value>0x7FFFFFFF)
		{
			return 0x7FFFFFFF;
		}
	}
	return i64Value & 0xFFFFFFFF;
}

bool CGameDesk::ChangeUserPointContest(__int64 *arPoint, bool *bCut, int nTaxIndex)
{
	__int64 arLoseMoney[MAX_PEOPLE];
	ZeroMemory(arLoseMoney, sizeof(arLoseMoney));

	MSG_GR_ContestChange _tmp;
	DL_GR_I_UserContestData _p;	
	bool bSomeOneNotEnoughMoney = false;
	// 输的总分和金币
	__int64 i64TotalLosePoint = 0;
	__int64 i64TotalLoseMoney = 0;

	for (int i=0; i<m_bMaxPeople; ++i)
	{
		if (NULL==m_pUserInfo[i])
		{
			continue;
		}
		//计算总共该输的钱
		if (arPoint[i] < 0)
		{			
			arLoseMoney[i] = arPoint[i]* m_pDataManage->m_InitData.uBasePoint;

			if (m_pUserInfo[i]->m_UserData.i64ContestScore < 0-arLoseMoney[i])
			{
				bSomeOneNotEnoughMoney = true;
				if (0 >= m_pUserInfo[i]->m_UserData.i64ContestScore)
				{
					//玩家身上没钱
					arLoseMoney[i] = 0;					
				}
				else
				{
					//将玩家身上的钱扣光
					arLoseMoney[i] = 0 - m_pUserInfo[i]->m_UserData.i64ContestScore;
				}
			}
			m_pUserInfo[i]->m_UserData.i64ContestScore += arLoseMoney[i];
			m_pUserInfo[i]->m_UserData.iContestCount++;
			m_dwChangeMoney[i] = arLoseMoney[i];
			i64TotalLosePoint -= arPoint[i];
			i64TotalLoseMoney -= arLoseMoney[i];

			_p.iUserID = m_pUserInfo[i]->m_UserData.dwUserID;
			_p.iLowChip = m_pDataManage->m_InitData.i64LowChip;
			_p.iCheckedScore = m_pUserInfo[i]->m_UserData.i64ContestScore;
			_p.ChangePoint = arPoint[i];
			_p.ChangeScore = arLoseMoney[i];
			_p.iContestID = m_pDataManage->m_InitData.iContestID;
			_p.iDeskNo = m_pUserInfo[i]->m_UserData.bDeskNO;
			_p.iSitNo = m_pUserInfo[i]->m_UserData.bDeskStation;
			_p.iRoomID = m_pDataManage->m_InitData.uRoomID;
			_p.iMatchID = m_pDataManage->m_InitData.iMatchID;
			_p.iGameID = m_pDataManage->m_InitData.iGameID;
			m_pDataManage->m_SQLDataManage.PushLine(&_p.DataBaseHead, sizeof(DL_GR_I_UserContestData), DTK_GR_UPDATE_CONTEST_RESULT, 0, 0);
		}
	}
	for (int i=0; i<m_bMaxPeople; ++i)
	{
		if (NULL==m_pUserInfo[i])
		{
			continue;
		}
		if (arPoint[i] > 0)
		{
			arLoseMoney[i] = arPoint[i]* m_pDataManage->m_InitData.uBasePoint;
			if (bSomeOneNotEnoughMoney)
			{
				if (i64TotalLosePoint != 0)
					arLoseMoney[i] = i64TotalLoseMoney*arPoint[i]/i64TotalLosePoint;
				else
					arLoseMoney[i] = 0;
			}
			m_pUserInfo[i]->m_UserData.i64ContestScore += arLoseMoney[i];
			m_pUserInfo[i]->m_UserData.iContestCount++;
			m_dwChangeMoney[i] = arLoseMoney[i];

			_p.iUserID = m_pUserInfo[i]->m_UserData.dwUserID;
			_p.iLowChip = m_pDataManage->m_InitData.i64LowChip;
			_p.iCheckedScore = m_pUserInfo[i]->m_UserData.i64ContestScore;
			_p.ChangePoint = arPoint[i];
			_p.ChangeScore = arLoseMoney[i];
			_p.iContestID = m_pDataManage->m_InitData.iContestID;
			_p.iDeskNo = m_pUserInfo[i]->m_UserData.bDeskNO;
			_p.iSitNo = m_pUserInfo[i]->m_UserData.bDeskStation;
			_p.iRoomID = m_pDataManage->m_InitData.uRoomID;
			_p.iMatchID = m_pDataManage->m_InitData.iMatchID;
			_p.iGameID = m_pDataManage->m_InitData.iGameID;
			m_pDataManage->m_SQLDataManage.PushLine(&_p.DataBaseHead, sizeof(DL_GR_I_UserContestData), DTK_GR_UPDATE_CONTEST_RESULT, 0, 0);
		}
	}

	DL_GR_I_UserContestRank _p_rank;
	_p_rank.pUserIDs = NULL;
	_p_rank.iIDNums = 0;
	_p_rank.iDeskID = m_bDeskIndex;
	m_pDataManage->m_SQLDataManage.PushLine(&_p_rank.DataBaseHead, sizeof(DL_GR_I_UserContestRank), DTK_GR_GET_CONTEST_RESULT, 0, 0);
	return true;
}

bool CGameDesk::ChangeUserPointContest(int *arPoint, bool *bCut, int nTaxIndex)
{
	int arLoseMoney[MAX_PEOPLE];
	ZeroMemory(arLoseMoney, sizeof(arLoseMoney));

	MSG_GR_ContestChange _tmp;
	DL_GR_I_UserContestData _p;
	DL_GR_I_UserContestRank _p_rank;
	_p_rank.pUserIDs = new int[m_bMaxPeople];
	_p_rank.iIDNums = 0;
	if (_p_rank.pUserIDs)
	{
		::memset(_p_rank.pUserIDs,0,sizeof(int)*m_bMaxPeople);
		_p_rank.iIDNums = m_bMaxPeople;
	}

	for (int i=0; i<m_bMaxPeople; ++i)
	{
		if (NULL==m_pUserInfo[i])
		{
			continue;
		}

		_p_rank.pUserIDs[i] = m_pUserInfo[i]->m_UserData.dwUserID;

		/// 计算应输者金币是否够
		arLoseMoney[i] = arPoint[i]*m_DeskBasePoint;

		m_pUserInfo[i]->m_UserData.i64ContestScore += arLoseMoney[i];
		m_pUserInfo[i]->m_UserData.iContestCount++;
		m_dwChangeMoney[i] = arLoseMoney[i];


		_p.ChangePoint = arPoint[i];
		_p.ChangeScore = arLoseMoney[i];
		_p.iContestID = m_pDataManage->m_InitData.iContestID;
		_p.iDeskNo = m_pUserInfo[i]->m_UserData.bDeskNO;
		_p.iSitNo = m_pUserInfo[i]->m_UserData.bDeskStation;
		_p.iSitNo = m_pDataManage->m_InitData.uRoomID;
		m_pDataManage->m_SQLDataManage.PushLine(&_p.DataBaseHead, sizeof(DL_GR_I_UserContestData), DTK_GR_UPDATE_CONTEST_RESULT, 0, 0);
	}

	if (_p_rank.iIDNums > 0)
	{
		_p_rank.iDeskID = m_bDeskIndex;
		m_pDataManage->m_SQLDataManage.PushLine(&_p_rank.DataBaseHead, sizeof(DL_GR_I_UserContestRank), DTK_GR_GET_CONTEST_RESULT, 0, 0);
	}

	return true;
}

bool CGameDesk::ChangeUserPointint64(__int64 *arPoint, bool *bCut, int nTaxIndex,int iCount)
{
	/// 计算本局游戏赢家所赢金币的总和
	/// 计算本局游戏输家所输金币的总和
	/// 判断每个输家身上的金币是否足够输
	/// 若有输家金币不够支付，则赢家要相应减少所得
	/// 更新m_pInfo里各玩家数据，供游戏中读取到正确的值
	/// 统一把所有玩家的结算结果存入数据库
	/// 发送消息到客户端，表示有人输钱或赢钱了

	if (m_pDataManage->m_InitData.dwRoomRule & GRR_CONTEST || m_pDataManage->m_InitData.dwRoomRule & GRR_TIMINGCONTEST)
	{
		return ChangeUserPointContest(arPoint, bCut, nTaxIndex);
	}

	m_tEndTime = CTime::GetCurrentTime();
	
	/// 取当前时间
	CString stime;
	stime.Format("%d",CTime::GetCurrentTime());
	int curtime=atoi(stime);

	/// 是否为大家都要交台费的房间规则，一般都为赢家交台费
	bool bIsAllNeedTax = (m_pDataManage->m_InitData.dwRoomRule & GRR_ALL_NEED_TAX) > 0;
	bool bTaxMain = false; ///< 是否只是庄家扣税。

	bool bNotCostPoint = (m_pDataManage->m_InitData.dwRoomRule & GRR_NOT_COST_POINT) > 0; /// 金币场不扣积分

	BYTE i=0;	///< 循环变量

	/// 输的总分和金币
	__int64 i64TotalLosePoint = 0;
	__int64 i64TotalWinPoint = 0;
	__int64 i64TotalLoseMoney = 0;
	__int64 i64TotalWinMoney = 0;

	/// 实际玩家有多少
	int nTotalPlayerCount = 0;
	/// 断线玩家是哪个
	BYTE bCutDeskStation = 255;

	for (i=0; i<m_bMaxPeople; ++i)
	{
		if (m_pUserInfo[i] == NULL)
			continue;
		++nTotalPlayerCount;
		if (bCut[i])
		{
			bCutDeskStation = i;
		}

		if (!bTaxMain && nTaxIndex>=0)
		{
			if (i == nTaxIndex) ///< 只扣该玩家的税。
			{
				bTaxMain = true;
			}
		}

	}

	__int64 arLoseMoney[MAX_PEOPLE];
	__int64 arTaxCount[MAX_PEOPLE];
	ZeroMemory(arLoseMoney, sizeof(arLoseMoney));
	ZeroMemory(arTaxCount, sizeof(arTaxCount));
	/// 用户ID数组
	long arUserID[MAX_PEOPLE];
	ZeroMemory(arUserID, sizeof(arUserID));
	long arLogonTime[MAX_PEOPLE];
	ZeroMemory(arLogonTime, sizeof(arLogonTime));
	/// 是否有人不够钱
	bool bSomeOneNoEnoughMoney = false;

	__int64 i64Tmp = 1;	/// 为了防止上溢，引入一个int64类型的数，免得写好多个强制类型转换

	long int dwNowTime=(long int)time(NULL);

	if (m_pDataManage->m_nRate < 100)
	{
		m_pDataManage->m_nRate = 100;
	}

	/// 金币场才需要计算，
	/// 根据应输积分，计算应输玩家应输的钱，同时考虑身上的钱是否够支付
	if(m_pDataManage->m_InitData.uComType == TY_MONEY_GAME )
	{
		for (i=0; i<m_bMaxPeople; ++i)
		{
			if (NULL==m_pUserInfo[i] || (m_bIsMidEnter[i]==true)&&m_pDataManage->m_InitData.uDeskType!=DESK_TYPE_BJL)
			{
				continue;
			}
            WriteLog("UserID=%d,arPoint[%d]=%I64d",m_pUserInfo[i]->m_UserData.dwUserID,i,arPoint[i]);
			/// 计算应输者金币是否够
			if (arPoint[i]<=0)
			{	
				arLoseMoney[i] = arPoint[i]*m_DeskBasePoint;	///< 该数为负数
				/// 是不是大家都要扣税，无论输赢
				if (bIsAllNeedTax && (m_pDataManage->m_InitData.uTax>0) && (nTaxIndex < 0 || nTaxIndex == i))
				{
					arTaxCount[i] = i64Tmp * m_pDataManage->m_InitData.uTax;
					arLoseMoney[i] -= arTaxCount[i];
				}

				/// 检查该玩家的钱是否已经不够了
				if (m_pUserInfo[i]->m_UserData.i64Money < (0-arLoseMoney[i])&& !(m_pDataManage->m_InitData.dwRoomRule&GRR_GAME_BUY)) ///<钱不够输了
				{
					bSomeOneNoEnoughMoney = true;
					if (m_pUserInfo[i]->m_UserData.i64Money <= 0) ///<这种情况应该不会出现
					{
						arLoseMoney[i] = 0;	///< 一分钱都没有了
					}
					else
					{
						arLoseMoney[i] = 0-m_pUserInfo[i]->m_UserData.i64Money; ///< 把玩家身上的钱都扣光
					}
				}
				/// 总共输的分数和钱数，如果有人不够钱，用来计算赢家分配钱的比例
				i64TotalLosePoint -= arPoint[i];
				i64TotalLoseMoney -= arLoseMoney[i];
			}
		}
	}

	/// 所有输家该输的金币都已经确定，可以开始计算赢家的收入了
	for (i=0; i<m_bMaxPeople; ++i)
	{
		if (NULL==m_pUserInfo[i])
		{
			continue;
		}
		arUserID[i] = m_pUserInfo[i]->m_UserData.dwUserID; ///<把有效玩家的dwUsreID放到数组中
		arLogonTime[i] = m_pUserInfo[i]->m_dwLogonTime; ///<把有效玩家的登录时间放到数组中
		/// 赢钱的需要重新计算，输家已经计算过了
		if ((m_pDataManage->m_InitData.uComType == TY_MONEY_GAME) && (arPoint[i]>0) && !bTaxMain)
		{
			/// 判断是否有玩家不够钱，若有，则赢不了这么多钱
			if (bSomeOneNoEnoughMoney)
			{

				if (i64TotalLosePoint != 0)
					arLoseMoney[i] = i64TotalLoseMoney*arPoint[i]/i64TotalLosePoint;
				else
					arLoseMoney[i] = 0;
			}
			else
			{
				arLoseMoney[i] = arPoint[i]*m_DeskBasePoint;
			}
			i64Tmp = 1;
			/// 计算好该赢金额后，扣税
			/// 大家都交
			if (bIsAllNeedTax)
			{
				if (m_pDataManage->m_InitData.uTax>0)
				{
					arTaxCount[i] = i64Tmp * m_pDataManage->m_InitData.uTax;
					arLoseMoney[i] -= arTaxCount[i];
			
				}
			}
			else
			{
				if ((m_pDataManage->m_InitData.uTax>0) && (arLoseMoney[i]>=2))
				{	
					arTaxCount[i] = i64Tmp * arLoseMoney[i] * m_pDataManage->m_InitData.uTax/m_pDataManage->m_nRate;
					arLoseMoney[i] -= arTaxCount[i];
				}
			}
		}
		else if ((m_pDataManage->m_InitData.uComType == TY_MONEY_GAME) && (arPoint[i]>0) && bTaxMain)///< 只扣该玩家的税
		{

			/// 判断是否有玩家不够钱，若有，则赢不了这么多钱
			if (bSomeOneNoEnoughMoney)
			{

				if (i64TotalLosePoint != 0)
					arLoseMoney[i] = i64TotalLoseMoney*arPoint[i]/i64TotalLosePoint;
				else
					arLoseMoney[i] = 0;

			}
			
			else
			{
				arLoseMoney[i] = arPoint[i]*m_DeskBasePoint;
			}
			i64Tmp = 1;
            if (bIsAllNeedTax && i == nTaxIndex)
            {
                if (m_pDataManage->m_InitData.uTax>0)
                {
                    arTaxCount[i] = i64Tmp * m_pDataManage->m_InitData.uTax;

                    arLoseMoney[i] -= arTaxCount[i];

                }
            }
            else
            {

			    if (i == nTaxIndex && m_pDataManage->m_InitData.uTax>0) ///< 只扣该玩家的税。
			    {
				    if (arLoseMoney[i] >= 2)
				    {
					    arTaxCount[i] = i64Tmp * arLoseMoney[i] * m_pDataManage->m_InitData.uTax/m_pDataManage->m_nRate;

					    arLoseMoney[i] -= arTaxCount[i];
				    }
			    }
            }
		}
		/// 给成员变量赋值
		m_dwTaxCom[i] = arTaxCount[i];

		if(bNotCostPoint)		//金币场不扣积分
		{
			m_dwChangePoint[i] = 0;
		}
		else
		{
			m_dwChangePoint[i] = arPoint[i];
		}

		/// 更新m_pUserInfo里的数据
		m_pUserInfo[i]->ChangePoint(m_dwChangePoint[i], m_dwTaxCom[i], arPoint[i]>0, arPoint[i]<0,arPoint[i]==0, bCut[i], dwNowTime-m_dwBeginTime, arLoseMoney[i],m_bIsBuy);

		/// 经过再次判断后，再赋值给结算
		m_dwChangeMoney[i] = arLoseMoney[i];
	}
	/// 统一把所有玩家的结算结果存入数据库

	/// 在上面的循环中已经统计过玩家人数nTotalPlayerCount
	UpdateUserInfo_t *pUpdateUserInfo = NULL;
	int nIndex=0;
	

	pUpdateUserInfo = new UpdateUserInfo_t[nTotalPlayerCount];
	for (i=0; i<m_bMaxPeople; ++i)
	{
		if (NULL == m_pUserInfo[i])
		{
			continue;
		}
		//结算之后，金币不足房间下限，自动赠送 
		__int64 i64Money = m_pUserInfo[i]->m_UserData.i64Money + m_pUserInfo[i]->m_UserData.i64Bank;
		if(m_pDataManage->m_InitData.bSendAlms && (i64Money <= m_pDataManage->m_InitData.iAlmsMinMoney)
			&& (m_pDataManage->m_InitData.uComType == TY_MONEY_GAME)
			&& !(m_pDataManage->m_InitData.dwRoomRule&GRR_EXPERCISE_ROOM))
		{
			pUpdateUserInfo[nIndex].bISAutoSendMoney = true;
		}

		/// 按次序赋值
		pUpdateUserInfo[nIndex].dwUserID			= m_pUserInfo[i]->m_UserData.dwUserID;
		pUpdateUserInfo[nIndex].dwOnLineTimeCount	= dwNowTime-m_pUserInfo[i]->m_dwLogonTime;
		__int64 iPoint = arPoint[i];
		
		if (bNotCostPoint)//金币场不扣积分
		{
			pUpdateUserInfo[nIndex].dwChangePoint = 0;
		}
		else 
		{
			pUpdateUserInfo[nIndex].dwChangePoint = iPoint;
		}

		pUpdateUserInfo[nIndex].dwChangeMoney = arLoseMoney[i];
		pUpdateUserInfo[nIndex].dwChangeTaxCom = m_dwTaxCom[i];

		if (arPoint[i]==0)
		{
			pUpdateUserInfo[nIndex].uMidCount = 1;
		}
		else if (arPoint[i]>0)
		{
			pUpdateUserInfo[nIndex].uWinCount = 1;
		}
		else
		{
			pUpdateUserInfo[nIndex].uLostCount = 1;
		}

		pUpdateUserInfo[nIndex].uCutCount = bCut[i]? 1 : 0;
		++nIndex;
	}


	/// 批量更新用户数据
	UpdateAllUserInfo(pUpdateUserInfo, nTotalPlayerCount, m_tEndTime.GetTime() - m_tBeginTime.GetTime());

	
	m_dwTax = 0;
	/// 发送用户更改数据到客户端

	for (i=0; i<m_bMaxPeople; ++i)
	{
		if (NULL == m_pUserInfo[i])
		{
			continue;
		}
		
		m_dwTax += m_dwTaxCom[i];

		m_pUserInfo[i]->m_dwLogonTime=(long int)time(NULL);
		//发送消息
		MSG_GR_R_UserPoint UserPoint;
		ZeroMemory(&UserPoint, sizeof(UserPoint));
		UserPoint.dwUserID = m_pUserInfo[i]->m_UserData.dwUserID;
		UserPoint.dwPoint = arPoint[i];

		if (bNotCostPoint)
		{
			UserPoint.dwPoint = 0;
		}
		
		UserPoint.dwMoney = arLoseMoney[i];


		if (arPoint[i] > 0)
			UserPoint.bWinCount = 1;

		if (arPoint[i] < 0)
		{
			UserPoint.bLostCount = 1;
		}

		if (arPoint[i] == 0) 
			UserPoint.bMidCount = 1;

		if (bCut[i] == true) 
			UserPoint.bCutCount = 1;

		UserPoint.dwSend = -1;

		m_pDataManage->m_TCPSocket.SendDataBatch(&UserPoint,sizeof(UserPoint),MDM_GR_ROOM,ASS_GR_USER_POINT,0);
	}

	//战绩
	if ((m_pDataManage->m_InitData.dwRoomRule & GRR_GAME_BUY) && m_bIsBuy)
	{
		m_iRunGameCount++;
	}
	//保存战绩
	UpdateGameRecord(arPoint);

	return true;
}
bool CGameDesk::ChangeUserPointint64_IsJoin(__int64 *arPoint, bool *bCut,bool *IsJoin, int nTaxIndex,int iCount)
{
    /// 计算本局游戏赢家所赢金币的总和
    /// 计算本局游戏输家所输金币的总和
    /// 判断每个输家身上的金币是否足够输
    /// 若有输家金币不够支付，则赢家要相应减少所得
    /// 更新m_pInfo里各玩家数据，供游戏中读取到正确的值
    /// 统一把所有玩家的结算结果存入数据库
    /// 发送消息到客户端，表示有人输钱或赢钱了

    if (m_pDataManage->m_InitData.dwRoomRule & GRR_CONTEST || m_pDataManage->m_InitData.dwRoomRule & GRR_TIMINGCONTEST)
    {
        return ChangeUserPointContest(arPoint, bCut, nTaxIndex);
    }

    m_tEndTime = CTime::GetCurrentTime();

    /// 取当前时间
    CString stime;
    stime.Format("%d",CTime::GetCurrentTime());
    int curtime=atoi(stime);

    /// 是否为大家都要交台费的房间规则，一般都为赢家交台费
    bool bIsAllNeedTax = (m_pDataManage->m_InitData.dwRoomRule & GRR_ALL_NEED_TAX) > 0;
    bool bTaxMain = false; ///< 是否只是庄家扣税。

    bool bNotCostPoint = (m_pDataManage->m_InitData.dwRoomRule & GRR_NOT_COST_POINT) > 0; /// 金币场不扣积分

    BYTE i=0;	///< 循环变量

    /// 输的总分和金币
    __int64 i64TotalLosePoint = 0;
    __int64 i64TotalWinPoint = 0;
    __int64 i64TotalLoseMoney = 0;
    __int64 i64TotalWinMoney = 0;

    /// 实际玩家有多少
    int nTotalPlayerCount = 0;
    /// 断线玩家是哪个
    BYTE bCutDeskStation = 255;

    for (i=0; i<m_bMaxPeople; ++i)
    {
        if (m_pUserInfo[i] == NULL)
            continue;
        ++nTotalPlayerCount;
        if (bCut[i])
        {
            bCutDeskStation = i;
        }

        if (!bTaxMain && nTaxIndex>=0)
        {
            if (i == nTaxIndex) ///< 只扣该玩家的税。
            {
                bTaxMain = true;
            }
        }

    }

    __int64 arLoseMoney[MAX_PEOPLE];
    __int64 arTaxCount[MAX_PEOPLE];
    ZeroMemory(arLoseMoney, sizeof(arLoseMoney));
    ZeroMemory(arTaxCount, sizeof(arTaxCount));
    /// 用户ID数组
    long arUserID[MAX_PEOPLE];
    ZeroMemory(arUserID, sizeof(arUserID));
    long arLogonTime[MAX_PEOPLE];
    ZeroMemory(arLogonTime, sizeof(arLogonTime));
    /// 是否有人不够钱
    bool bSomeOneNoEnoughMoney = false;

    __int64 i64Tmp = 1;	/// 为了防止上溢，引入一个int64类型的数，免得写好多个强制类型转换

    long int dwNowTime=(long int)time(NULL);

    if (m_pDataManage->m_nRate < 100)
    {
        m_pDataManage->m_nRate = 100;
    }

    /// 金币场才需要计算，
    /// 根据应输积分，计算应输玩家应输的钱，同时考虑身上的钱是否够支付
    if(m_pDataManage->m_InitData.uComType == TY_MONEY_GAME )
    {
        for (i=0; i<m_bMaxPeople; ++i)
        {
            if ((NULL==m_pUserInfo[i] || (m_bIsMidEnter[i]==true)&&m_pDataManage->m_InitData.uDeskType!=DESK_TYPE_BJL)||IsJoin[i]==false)
            {
                continue;
            }
            WriteLog("UserID=%d,arPoint[%d]=%I64d",m_pUserInfo[i]->m_UserData.dwUserID,i,arPoint[i]);
            /// 计算应输者金币是否够
            if (arPoint[i]<=0)
            {	
                arLoseMoney[i] = arPoint[i]*m_DeskBasePoint;	///< 该数为负数
                /// 是不是大家都要扣税，无论输赢
                if (bIsAllNeedTax && (m_pDataManage->m_InitData.uTax>0) && (nTaxIndex < 0 || nTaxIndex == i))
                {
                    arTaxCount[i] = i64Tmp * m_pDataManage->m_InitData.uTax;
                    arLoseMoney[i] -= arTaxCount[i];
                }

                /// 检查该玩家的钱是否已经不够了
                if (m_pUserInfo[i]->m_UserData.i64Money < (0-arLoseMoney[i])&& !(m_pDataManage->m_InitData.dwRoomRule&GRR_GAME_BUY)) ///<钱不够输了
                {
                    bSomeOneNoEnoughMoney = true;
                    if (m_pUserInfo[i]->m_UserData.i64Money <= 0) ///<这种情况应该不会出现
                    {
                        arLoseMoney[i] = 0;	///< 一分钱都没有了
                    }
                    else
                    {
                        arLoseMoney[i] = 0-m_pUserInfo[i]->m_UserData.i64Money; ///< 把玩家身上的钱都扣光
                    }
                }
                /// 总共输的分数和钱数，如果有人不够钱，用来计算赢家分配钱的比例
                i64TotalLosePoint -= arPoint[i];
                i64TotalLoseMoney -= arLoseMoney[i];
            }
        }
    }

    /// 所有输家该输的金币都已经确定，可以开始计算赢家的收入了
    for (i=0; i<m_bMaxPeople; ++i)
    {
        if (NULL==m_pUserInfo[i]||IsJoin[i]==false)
        {
            continue;
        }
        arUserID[i] = m_pUserInfo[i]->m_UserData.dwUserID; ///<把有效玩家的dwUsreID放到数组中
        arLogonTime[i] = m_pUserInfo[i]->m_dwLogonTime; ///<把有效玩家的登录时间放到数组中
        /// 赢钱的需要重新计算，输家已经计算过了
        if ((m_pDataManage->m_InitData.uComType == TY_MONEY_GAME) && (arPoint[i]>0) && !bTaxMain)
        {
            /// 判断是否有玩家不够钱，若有，则赢不了这么多钱
            if (bSomeOneNoEnoughMoney)
            {

                if (i64TotalLosePoint != 0)
                    arLoseMoney[i] = i64TotalLoseMoney*arPoint[i]/i64TotalLosePoint;
                else
                    arLoseMoney[i] = 0;
            }
            else
            {
                arLoseMoney[i] = arPoint[i]*m_DeskBasePoint;
            }
            i64Tmp = 1;
            /// 计算好该赢金额后，扣税
            /// 大家都交
            if (bIsAllNeedTax)
            {
                if (m_pDataManage->m_InitData.uTax>0)
                {
                    arTaxCount[i] = i64Tmp * m_pDataManage->m_InitData.uTax;
                    arLoseMoney[i] -= arTaxCount[i];

                }
            }
            else
            {
                if ((m_pDataManage->m_InitData.uTax>0) && (arLoseMoney[i]>=2))
                {	
                    arTaxCount[i] = i64Tmp * arLoseMoney[i] * m_pDataManage->m_InitData.uTax/m_pDataManage->m_nRate;
                    arLoseMoney[i] -= arTaxCount[i];
                }
            }
        }
        else if ((m_pDataManage->m_InitData.uComType == TY_MONEY_GAME) && (arPoint[i]>0) && bTaxMain)///< 只扣该玩家的税
        {

            /// 判断是否有玩家不够钱，若有，则赢不了这么多钱
            if (bSomeOneNoEnoughMoney)
            {

                if (i64TotalLosePoint != 0)
                    arLoseMoney[i] = i64TotalLoseMoney*arPoint[i]/i64TotalLosePoint;
                else
                    arLoseMoney[i] = 0;

            }

            else
            {
                arLoseMoney[i] = arPoint[i]*m_DeskBasePoint;
            }
            i64Tmp = 1;
            if (bIsAllNeedTax && i == nTaxIndex)
            {
                if (m_pDataManage->m_InitData.uTax>0)
                {
                    arTaxCount[i] = i64Tmp * m_pDataManage->m_InitData.uTax;

                    arLoseMoney[i] -= arTaxCount[i];

                }
            }
            else
            {

                if (i == nTaxIndex && m_pDataManage->m_InitData.uTax>0) ///< 只扣该玩家的税。
                {
                    if (arLoseMoney[i] >= 2)
                    {
                        arTaxCount[i] = i64Tmp * arLoseMoney[i] * m_pDataManage->m_InitData.uTax/m_pDataManage->m_nRate;

                        arLoseMoney[i] -= arTaxCount[i];
                    }
                }
            }
        }
        /// 给成员变量赋值
        m_dwTaxCom[i] = arTaxCount[i];

        if(bNotCostPoint)		//金币场不扣积分
        {
            m_dwChangePoint[i] = 0;
        }
        else
        {
            m_dwChangePoint[i] = arPoint[i];
        }

        /// 更新m_pUserInfo里的数据
        m_pUserInfo[i]->ChangePoint(m_dwChangePoint[i], m_dwTaxCom[i], arPoint[i]>0, arPoint[i]<0,arPoint[i]==0, bCut[i], dwNowTime-m_dwBeginTime, arLoseMoney[i],m_bIsBuy);

        /// 经过再次判断后，再赋值给结算
        m_dwChangeMoney[i] = arLoseMoney[i];
    }
    /// 统一把所有玩家的结算结果存入数据库

    /// 在上面的循环中已经统计过玩家人数nTotalPlayerCount
    UpdateUserInfo_t *pUpdateUserInfo = NULL;
    int nIndex=0;


    pUpdateUserInfo = new UpdateUserInfo_t[nTotalPlayerCount];
    for (i=0; i<m_bMaxPeople; ++i)
    {
        if (NULL == m_pUserInfo[i]||IsJoin[i]==false)
        {
            continue;
        }
        //结算之后，金币不足房间下限，自动赠送 
        __int64 i64Money = m_pUserInfo[i]->m_UserData.i64Money + m_pUserInfo[i]->m_UserData.i64Bank;
        if(m_pDataManage->m_InitData.bSendAlms && (i64Money <= m_pDataManage->m_InitData.iAlmsMinMoney)
            && (m_pDataManage->m_InitData.uComType == TY_MONEY_GAME)
            && !(m_pDataManage->m_InitData.dwRoomRule&GRR_EXPERCISE_ROOM))
        {
            pUpdateUserInfo[nIndex].bISAutoSendMoney = true;
        }

        /// 按次序赋值
        pUpdateUserInfo[nIndex].dwUserID			= m_pUserInfo[i]->m_UserData.dwUserID;
        pUpdateUserInfo[nIndex].dwOnLineTimeCount	= dwNowTime-m_pUserInfo[i]->m_dwLogonTime;
        __int64 iPoint = arPoint[i];

        if (bNotCostPoint)//金币场不扣积分
        {
            pUpdateUserInfo[nIndex].dwChangePoint = 0;
        }
        else 
        {
			//写入积分的点
            pUpdateUserInfo[nIndex].dwChangePoint = iPoint;
        }

        pUpdateUserInfo[nIndex].dwChangeMoney = arLoseMoney[i];
        pUpdateUserInfo[nIndex].dwChangeTaxCom = m_dwTaxCom[i];

        if (arPoint[i]==0)
        {
            pUpdateUserInfo[nIndex].uMidCount = 1;
        }
        else if (arPoint[i]>0)
        {
            pUpdateUserInfo[nIndex].uWinCount = 1;
        }
        else
        {
            pUpdateUserInfo[nIndex].uLostCount = 1;
        }

        pUpdateUserInfo[nIndex].uCutCount = bCut[i]? 1 : 0;
        ++nIndex;
    }


    /// 批量更新用户数据
    UpdateAllUserInfo(pUpdateUserInfo, nTotalPlayerCount, m_tEndTime.GetTime() - m_tBeginTime.GetTime());


    m_dwTax = 0;
    /// 发送用户更改数据到客户端

    for (i=0; i<m_bMaxPeople; ++i)
    {
        if (NULL == m_pUserInfo[i]||IsJoin[i]==false)
        {
            continue;
        }

        m_dwTax += m_dwTaxCom[i];

        m_pUserInfo[i]->m_dwLogonTime=(long int)time(NULL);
        //发送消息
        MSG_GR_R_UserPoint UserPoint;
        ZeroMemory(&UserPoint, sizeof(UserPoint));
        UserPoint.dwUserID = m_pUserInfo[i]->m_UserData.dwUserID;
        UserPoint.dwPoint = arPoint[i];

        if (bNotCostPoint)
        {
            UserPoint.dwPoint = 0;
        }

        UserPoint.dwMoney = arLoseMoney[i];


        if (arPoint[i] > 0)
            UserPoint.bWinCount = 1;

        if (arPoint[i] < 0)
        {
            UserPoint.bLostCount = 1;
        }

        if (arPoint[i] == 0) 
            UserPoint.bMidCount = 1;

        if (bCut[i] == true) 
            UserPoint.bCutCount = 1;

        UserPoint.dwSend = -1;

        m_pDataManage->m_TCPSocket.SendDataBatch(&UserPoint,sizeof(UserPoint),MDM_GR_ROOM,ASS_GR_USER_POINT,0);
    }

    //战绩
    if ((m_pDataManage->m_InitData.dwRoomRule & GRR_GAME_BUY) && m_bIsBuy)
    {
        m_iRunGameCount++;
    }
    //保存战绩
    UpdateGameRecord(arPoint);

    return true;
}
//保存战绩
void CGameDesk::UpdateGameRecord(__int64* arPoint)
{
	if (DESK_TYPE_BJL!=m_uDeskType && !m_bAllRobot)
	{
		DL_GR_I_BUYDESK_GameRecord _indata;
		strcpy_s(_indata.szDeskPassWord,m_szDeskPassWord);
		_indata.iUserCount = m_bMaxPeople;
		_indata.iGameCount = m_iRunGameCount;

		for (BYTE i=0;i<m_bMaxPeople;i++)
		{
			if (NULL == m_pUserInfo[i])
			{
				continue;
			}

            if (20 <= i)
            {
                _indata.iUserCount = 20;
                break;
            }

			_indata.pGameUserRecord[i].iScore = arPoint[i];
			_indata.pGameUserRecord[i].iUserID = m_pUserInfo[i]->m_UserData.dwUserID;
		}

		strcpy_s(_indata.szGameSN,m_GameSN);
		if((m_pDataManage->m_InitData.dwRoomRule & GRR_GAME_BUY))
			_indata.iGameType = 1;
		else if(m_pDataManage->m_InitData.dwRoomRule & GRR_CONTEST || m_pDataManage->m_InitData.dwRoomRule & GRR_TIMINGCONTEST)
			_indata.iGameType = 2;
		else 
			_indata.iGameType = 0;

		m_pDataManage->m_SQLDataManage.PushLine(&_indata.DataBaseHead, sizeof(_indata), DTK_GR_GAMERECORD, 0, 0);
	}
}
/// 修改用户分数和钱币
/// 该函数需要完成的功能
/// 根据输入参数，计算出用户实际输赢的金币数，并将结果保存在m_dwChangeMoney成员中，同时写入数据库
/// @param[in] dwPoint 用户积分数组，数组大小为m_bMaxPeople，本桌支持的最大人数，由调用者保证数组不越界
/// @param[in] bCut 用户是否断线数组，数组大小为m_bMaxPeople，本桌支持的最大人数，由调用者保证数组不越界
/// @param[in] nTaxIndex -1表示扣所以玩家的税，否则只扣该用户的钱
/// 该函数会修改m_dwChangeMoney[]数组成员的值
/// 游戏服务器程序调用完此函数的，可通过RecordGameInfo函数获取该值
bool CGameDesk::ChangeUserPoint(int *arPoint, bool *bCut,int nTaxIndex)
{
	/// 计算本局游戏赢家所赢金币的总和
	/// 计算本局游戏输家所输金币的总和
	/// 判断每个输家身上的金币是否足够输
	/// 若有输家金币不够支付，则赢家要相应减少所得
	/// 更新m_pInfo里各玩家数据，供游戏中读取到正确的值
	/// 统一把所有玩家的结算结果存入数据库
	/// 发送消息到客户端，表示有人输钱或赢钱了
	if (m_pDataManage->m_InitData.dwRoomRule & GRR_CONTEST || m_pDataManage->m_InitData.dwRoomRule & GRR_TIMINGCONTEST)
	{
		return ChangeUserPointContest(arPoint, bCut, nTaxIndex);
	}

	/// 取当前时间
	CString stime;
	stime.Format("%d",CTime::GetCurrentTime());
	int curtime=atoi(stime);

	/// 是否为大家都要交台费的房间规则，一般都为赢家交台费
	bool bIsAllNeedTax = (m_pDataManage->m_InitData.dwRoomRule & GRR_ALL_NEED_TAX) > 0;
	bool bTaxMain = false; ///< 是否只是庄家扣税。
	
	bool bNotCostPoint = (m_pDataManage->m_InitData.dwRoomRule & GRR_NOT_COST_POINT) > 0; /// 金币场不扣积分  add by lxl 2010-10-26

	BYTE i=0;	///< 循环变量
	
	/// 输的总分和金币
	__int64 i64TotalLosePoint = 0;
	__int64 i64TotalWinPoint = 0;
	__int64 i64TotalLoseMoney = 0;
	__int64 i64TotalWinMoney = 0;

	/// 实际玩家有多少
	int nTotalPlayerCount = 0;
	/// 断线玩家是哪个
	BYTE bCutDeskStation = 255;

	for (i=0; i<m_bMaxPeople; ++i)
	{
		if (m_pUserInfo[i] == NULL)
			continue;
		++nTotalPlayerCount;
		if (bCut[i])
		{
			bCutDeskStation = i;
		}

		if (!bTaxMain && nTaxIndex>=0)
		{
			if (i == nTaxIndex) ///< 只扣该玩家的税。
			{
				bTaxMain = true;
			}
			if(nTaxIndex > 1000000) ///如果是炒锅麻将规则，扣需要扣税的玩家 add by lxl 2010-12-14
			{
				bTaxMain = true;
			}
		}

	}

	__int64 arLoseMoney[MAX_PEOPLE];
	__int64 arTaxCount[MAX_PEOPLE];
	ZeroMemory(arLoseMoney, sizeof(arLoseMoney));
	ZeroMemory(arTaxCount, sizeof(arTaxCount));
	/// 用户ID数组
	long arUserID[MAX_PEOPLE];
	ZeroMemory(arUserID, sizeof(arUserID));
	long arLogonTime[MAX_PEOPLE];
	ZeroMemory(arLogonTime, sizeof(arLogonTime));
	/// 是否有人不够钱
	bool bSomeOneNoEnoughMoney = false;

	__int64 i64Tmp = 1;	/// 为了防止上溢，引入一个int64类型的数，免得写好多个强制类型转换

	long int dwNowTime=(long int)time(NULL);

	if (m_pDataManage->m_nRate < 100)
		m_pDataManage->m_nRate = 100;


	/// 金币场才需要计算，
	/// 根据应输积分，计算应输玩家应输的钱，同时考虑身上的钱是否够支付
	if(m_pDataManage->m_InitData.uComType == TY_MONEY_GAME)
	{
		for (i=0; i<m_bMaxPeople; ++i)
		{
			if (NULL==m_pUserInfo[i])
			{
				continue;
			}
			/// 计算应输者金币是否够
			if (arPoint[i]<=0)
			{
				if(nTaxIndex > 1000000) //炒锅规则金币==积分
					arLoseMoney[i] = arPoint[i];
				else
					arLoseMoney[i] = arPoint[i]*m_DeskBasePoint;	///< 该数为负数
				/// 是不是大家都要扣税，无论输赢
				if (bIsAllNeedTax && (m_pDataManage->m_InitData.uTax>0))
				{
					if(bCutDeskStation == 255)
					{
						arTaxCount[i] = i64Tmp * m_pDataManage->m_InitData.uTax;
						arLoseMoney[i] -= arTaxCount[i];
					}
				}

				/// 检查该玩家的钱是否已经不够了
				if (m_pUserInfo[i]->m_UserData.i64Money < (0-arLoseMoney[i])) ///<钱不够输了
				{
					bSomeOneNoEnoughMoney = true;
					if (m_pUserInfo[i]->m_UserData.i64Money <= 0) ///<这种情况应该不会出现
					{
						arLoseMoney[i] = 0;	///< 一分钱都没有了
					}
					else
					{
						arLoseMoney[i] = 0-m_pUserInfo[i]->m_UserData.i64Money; ///< 把玩家身上的钱都扣光
					}
				}

				/// 总共输的分数和钱数，如果有人不够钱，用来计算赢家分配钱的比例
				i64TotalLosePoint -= arPoint[i];
				i64TotalLoseMoney -= arLoseMoney[i];
			}
		}
	}

	//炒锅麻将扣税 add by lxl 2010-12-14
	int nCGTax = 0;
	//炒锅麻将扣税预处理  
	if(nTaxIndex > 1000000)
	{
		int nBasePoint; //炒锅点数
		int nPointRate; //点数和金币的比率 
		int nTax;	    //税率
		//如nTaxIndex = 1000100，则比率为100，即1点 = 100个金币
		nPointRate = nTaxIndex - 1000000;
		nBasePoint = 50;
		
	    int nCount = 0; //要扣税的人数
		//统计要扣税的人数,arPoint[8]数组前4个为玩家积分数值，后4个表示为是否扣税（1为要扣税）
		for(int i = 4; i < 8; i++)
		{
			if(arPoint[i] == 1)
			  nCount++;
		}

		//扣税计算公式：扣税金币 =  房间倍数 * 点数 * 比率(点数:金币) * 税率
		if(nCount > 0)
		{
			nCGTax = (i64Tmp  * m_DeskBasePoint * nBasePoint * nPointRate * m_pDataManage->m_InitData.uTax / nCount ) / m_pDataManage->m_nRate;
		}
	}

	/// 所有输家该输的金币都已经确定，可以开始计算赢家的收入了
	for (i=0; i<m_bMaxPeople; ++i)
	{
		if (NULL==m_pUserInfo[i])
		{
			continue;
		}
		arUserID[i] = m_pUserInfo[i]->m_UserData.dwUserID; ///<把有效玩家的dwUsreID放到数组中
		arLogonTime[i] = m_pUserInfo[i]->m_dwLogonTime; ///<把有效玩家的登录时间放到数组中
		/// 赢钱的需要重新计算，输家已经计算过了
		if ((m_pDataManage->m_InitData.uComType == TY_MONEY_GAME) && (arPoint[i]>0) && !bTaxMain)
		{
			/// 判断是否有玩家不够钱，若有，则赢不了这么多钱
			if (bSomeOneNoEnoughMoney)
			{
				if (i64TotalLosePoint != 0 && arPoint[i] != 0)
					arLoseMoney[i] = i64TotalLoseMoney/i64TotalLosePoint*arPoint[i];
				else
					arLoseMoney[i] = 0;
			}
			else
			{
				if(nTaxIndex > 1000000) //炒锅规则金币==积分
					arLoseMoney[i] = arPoint[i];
				else
					arLoseMoney[i] = arPoint[i]*m_DeskBasePoint;
			}
			i64Tmp = 1;
			/// 计算好该赢金额后，扣税
			/// 大家都交
			if (bIsAllNeedTax)
			{
				if (m_pDataManage->m_InitData.uTax>0)
				{
					//if (i == bCutDeskStation) ///赢家为强退的人。要扣除所有人的台费，这种情况在游戏未处理好时会出现
					//{
					//	arTaxCount[i] = m_pDataManage->m_InitData.uTax * nTotalPlayerCount;
					//}
					//else
					//{
					//	arTaxCount[i] = m_pDataManage->m_InitData.uTax;
					//}
					//arLoseMoney[i] -= arTaxCount[i];   //台费值也需计算在内
					if(bCutDeskStation == 255)
					{
						arTaxCount[i] = i64Tmp * m_pDataManage->m_InitData.uTax;
						arLoseMoney[i] -= arTaxCount[i];
					}
					//如果有人强退则不扣台费

				}
			}
			else
			{
				if ((m_pDataManage->m_InitData.uTax>0) && (arLoseMoney[i]>=2))
				{	
					arTaxCount[i] = i64Tmp * arLoseMoney[i] * m_pDataManage->m_InitData.uTax/m_pDataManage->m_nRate;
			//		if (arTaxCount[i] == 0)	/// 最少要抽1个金币
			//		{
			//			arTaxCount[i] = 1;
			//		} 2010 5 14 抽水不够一个金币的时候将不抽水
					arLoseMoney[i] -= arTaxCount[i];
				}
			}
		}
		else if ((m_pDataManage->m_InitData.uComType == TY_MONEY_GAME) && (arPoint[i]>0) && bTaxMain)///< 只扣该玩家的税
		{
			/// 判断是否有玩家不够钱，若有，则赢不了这么多钱
			if (bSomeOneNoEnoughMoney)
			{
				if (i64TotalLosePoint != 0 && arPoint[i] != 0)
					arLoseMoney[i] = i64TotalLoseMoney/i64TotalLosePoint*arPoint[i];
				else
					arLoseMoney[i] = 0;
			}
			else
			{
				if(nTaxIndex > 1000000) //炒锅规则金币==积分
					arLoseMoney[i] = arPoint[i];
				else
					arLoseMoney[i] = arPoint[i]*m_DeskBasePoint;
			}
			i64Tmp = 1;

			if (i == nTaxIndex && m_pDataManage->m_InitData.uTax>0) ///< 只扣该玩家的税。
			{
				if (arLoseMoney[i] >= 2)
				{
					arTaxCount[i] = i64Tmp * arLoseMoney[i] * m_pDataManage->m_InitData.uTax/m_pDataManage->m_nRate;
					//if (arTaxCount[i] == 0)	/// 最少要抽1个金币
					//{
					//	arTaxCount[i] = 1;
					//}
					arLoseMoney[i] -= arTaxCount[i];
				}
			}
			//炒锅麻将扣税规则  add by lxl 2010-12-14
			if(nTaxIndex > 1000000 && m_pDataManage->m_InitData.uTax>0)
			{
				if (arPoint[i+4] == 1 && arLoseMoney[i] >= 2) //判断是否要扣税
				{
					arTaxCount[i] = nCGTax;
					arLoseMoney[i] -= arTaxCount[i];
				}
			}
		}


		/// 给成员变量赋值
		m_dwTaxCom[i] = arTaxCount[i];

		if(bNotCostPoint)		//金币场不扣积分 add by lxl 2010-10-26
			m_dwChangePoint[i] = 0;
		else
			m_dwChangePoint[i] = arPoint[i];


		__int64 _sendmoney = 0;
		/// 更新m_pUserInfo里的数据
		m_pUserInfo[i]->ChangePoint(arPoint[i], m_dwTaxCom[i], arPoint[i]>0, arPoint[i]<0,arPoint[i]==0, bCut[i], dwNowTime-m_dwBeginTime, arLoseMoney[i]);
		m_pUserInfo[i]->m_UserData.i64Money += _sendmoney;

		/// 经过再次判断后，再赋值给结算
		m_dwChangeMoney[i] = arLoseMoney[i];
	}
	/// 统一把所有玩家的结算结果存入数据库

	/// 在上面的循环中已经统计过玩家人数nTotalPlayerCount
	UpdateUserInfo_t *pUpdateUserInfo = new UpdateUserInfo_t[nTotalPlayerCount];
	int nIndex=0;
	for (i=0; i<m_bMaxPeople; ++i)
	{
		if (NULL == m_pUserInfo[i])
		{
			continue;
		}
		/// 按次序赋值
		pUpdateUserInfo[nIndex].dwUserID			= m_pUserInfo[i]->m_UserData.dwUserID;
		pUpdateUserInfo[nIndex].dwOnLineTimeCount	= dwNowTime-m_pUserInfo[i]->m_dwLogonTime;
		int iPoint = arPoint[i];
		
		if (bNotCostPoint)//金币场不扣积分
		{
			pUpdateUserInfo[nIndex].dwChangePoint = 0;
		}
		else 
		{
			pUpdateUserInfo[nIndex].dwChangePoint = iPoint;
		}

		pUpdateUserInfo[nIndex].dwChangeMoney = arLoseMoney[i];
		pUpdateUserInfo[nIndex].dwChangeTaxCom = m_dwTaxCom[i];
		
		if (arPoint[i]==0)
		{
			pUpdateUserInfo[nIndex].uMidCount = 1;
		}
		else if (arPoint[i]>0)
		{
			pUpdateUserInfo[nIndex].uWinCount = 1;
		}
		else
		{
			pUpdateUserInfo[nIndex].uLostCount = 1;
		}
	
		pUpdateUserInfo[nIndex].uCutCount = bCut[i]? 1 : 0;
		++nIndex;
	}
		
//	if (TY_MONEY_GAME == m_pDataManage->m_InitData.uComType && m_pDataManage->m_bPresentCoin) ///< 金币场才有赠送金币
//	{
//		PresentCoin();
//	}

	/// 批量更新用户数据
	UpdateAllUserInfo(pUpdateUserInfo, nTotalPlayerCount, m_tEndTime.GetTime() - m_tBeginTime.GetTime());
	m_dwTax = 0;
	/// 发送用户更改数据到客户端

	for (i=0; i<m_bMaxPeople; ++i)
	{
		if (NULL == m_pUserInfo[i])
		{
			continue;
		}
		m_dwTax += m_dwTaxCom[i];

		m_pUserInfo[i]->m_dwLogonTime=(long int)time(NULL);
		//发送消息
		MSG_GR_R_UserPoint UserPoint;
		ZeroMemory(&UserPoint, sizeof(UserPoint));
		UserPoint.dwUserID = m_pUserInfo[i]->m_UserData.dwUserID;
		UserPoint.dwPoint = arPoint[i];

		if(bNotCostPoint)
		{
			UserPoint.dwPoint = 0;
		}
		
		UserPoint.dwMoney = arLoseMoney[i];
		if (arPoint[i] > 0)
			UserPoint.bWinCount = 1;

		if (arPoint[i] < 0)
		{
			UserPoint.bLostCount = 1;
		}

		if (arPoint[i] == 0) 
			UserPoint.bMidCount = 1;

		if (bCut[i] == true) 
			UserPoint.bCutCount = 1;

		UserPoint.dwSend = -1;
		
		m_pDataManage->m_TCPSocket.SendDataBatch(&UserPoint,sizeof(UserPoint),MDM_GR_ROOM,ASS_GR_USER_POINT,0);
	}

	return true;
}
//修改用户分数
/*
这个函数在每局游戏结束的时候被调用，用来修改用户重进入房间起到此局结束其分数、金币、税收、输赢局数等的改变差量，这些改变值
将在用户退出房间时（OnUserLeft函数）被使用，用来修改数据库中用户信息（被加在MathchTable和Users表中）。
这个函数被分成两个模式，由IsCheckInMoney参数来控制，第一个模式是直接模式，由游戏桌子（dll）的开发者在gamefinish函数中直接指定用户
金币改变值。第二个模式由此函数根据分数值、房间倍数和游戏桌的倍数算出金币的改变值。
参数：
int *dwPoint: （输入）一个指针，指向大小为8的一维整型数组，记录了此桌用户分数（经验）的改变情况，如果不足8个人则取有效人数，其他值默认
为0。
bool *bCut:（输入） 一个指针，指向大小为8的一维bool型数组，记录了用户是否掉线，如果不足8个人则取有效人数，其他值默认false。
int *dwMoneyInput: 模式1下（输入）指向大小为8的一维整型数组，记录了此桌用户金币（经验）的改变情况，如果不足8个人则取有效人数，其他值
默认为0。
bool IsCheckInMoney: 模式选择,true为模式1,false 为模式2(默认)。

*/
bool CGameDesk::ChangeUserPoint_old(int *dwPoint, bool *bCut,int *dwMoneyInput,bool IsCheckInMoney)//, bool bWin, bool bLost, bool bMid)
{
	return true;
}
//得到经验等级
int CGameDesk::GetOrderClass(int dwPoint)
{
	static int uBasePoint[19] =
	{
		400,		800,		1600,		3200,
		6400,		12800,		25600,		51200,
		102400,		204800,		409600,		819200,
		1638400,	3276800,	6553600,	13107200,
		26214400,	52428800,	104857600
	};

	for (int i = 0; i < 19; i ++)	
	{
		if (dwPoint < uBasePoint[i]) 
			return i+1;
	}
	return 20;
}

/// 更新所有玩家的数据库资料
/// @param [in] pUpdateUserInfo，多个用户数据指针
/// @param [in] nCount，用户数据个数
/// @return 无
void CGameDesk::UpdateAllUserInfo(UpdateUserInfo_t * pUpdateUserInfo, int nCount, int iroundtime)
{
	DL_GR_I_AllUserData_t allUserData;
	UINT dwNowTime=(UINT)time(NULL);
	allUserData.nUserCount = nCount;
	allUserData.nPlayTime = dwNowTime - m_dwBeginTime;
	allUserData.iRoundPlayTime = iroundtime;
	allUserData.pUpdateUserInfo = pUpdateUserInfo;
	m_pDataManage->m_SQLDataManage.PushLine(&allUserData.DataBaseHead,sizeof(allUserData),DTK_GR_UPDATE_GAME_RESULT_ALLUSER,0,0);
}
void CGameDesk::UpdateUserPoint(BYTE *pbDeskStation,__int64 *arPoint,int nCount)
{
	UpdateUserInfo_t * pUpdateUserInfo = new UpdateUserInfo_t[nCount];    //此处的内存会在，数据库处理线程删除
    
	MSG_GR_R_UserPoint UserPoint;
	ZeroMemory(&UserPoint, sizeof(UserPoint));
    for(int i=0;i<nCount;i++)
	{
		pUpdateUserInfo[i].dwUserID			 = m_pUserInfo[pbDeskStation[i]]->m_UserData.dwUserID;
		pUpdateUserInfo[i].dwOnLineTimeCount = 0;                                  //此处也不记录
		pUpdateUserInfo[i].dwChangePoint     = 0;
		pUpdateUserInfo[i].dwChangeMoney     = arPoint[i];
		pUpdateUserInfo[i].dwChangeTaxCom    = 0;                                  //此处不收费

		UserPoint.dwUserID                   = m_pUserInfo[pbDeskStation[i]]->m_UserData.dwUserID;
		UserPoint.dwMoney                    = arPoint[i];
        UserPoint.dwPoint                    = 0;
		m_pDataManage->m_TCPSocket.SendDataBatch(&UserPoint,sizeof(UserPoint),MDM_GR_ROOM,ASS_GR_USER_POINT,0);
	}

	DL_GR_I_AllUserData_t allUserData;
	allUserData.nUserCount = nCount;
	allUserData.nPlayTime = 0;
	allUserData.pUpdateUserInfo = pUpdateUserInfo;
	m_pDataManage->m_SQLDataManage.PushLine(&allUserData.DataBaseHead,sizeof(allUserData),DTK_GR_UPDATE_GAME_RESULT_ALLUSER,0,0);	
	
	
}
//记录用户分数
void CGameDesk::UpdateUserInfo(long dwUserID,int iPoint,int Money, int taxCom,bool bcut,long playTime,long logonTime)
{//将Point改为iPoint，因为Point是MFC内部的一个类，虽然重载名称可以使用，但易引起歧义
 //调整了整个函数的位置，是因为不仅仅记录了积分，还根据分数对输赢进行判断
	//处理vip功能
	DL_GR_I_UserLeft m_UserInfo;
	memset(&m_UserInfo,0,sizeof(m_UserInfo));
	m_UserInfo.uWinCount = (iPoint > 0 ? 1:0);
	m_UserInfo.uLostCount =(iPoint < 0 ? 1:0);
	m_UserInfo.uMidCount = (iPoint == 0 ? 1:0);
	m_UserInfo.dwUserID = dwUserID;
	m_UserInfo.dwChangeMoney = Money;
	m_UserInfo.dwChangeTaxCom = taxCom;
	m_UserInfo.uCutCount = bcut;
	m_UserInfo.dwPlayTimeCount = playTime;
	m_UserInfo.dwOnLineTimeCount = logonTime;

	DL_GR_I_SendMonet_By_Times sSendMoneyByTimes;//当前用户为要送的用户
	memset(&sSendMoneyByTimes,0,sizeof(sSendMoneyByTimes));
	sSendMoneyByTimes.dwUserID =dwUserID;
	sSendMoneyByTimes.dwGameFinishCount+=1;

	for(int i = 0; i < MAX_PEOPLE; i ++)
	{
		if(m_pUserInfo[i] == NULL)
			continue;

		CString stime;
		stime.Format("%d",CTime::GetCurrentTime());        //当前的时间，String格式
		int curtime=atoi(stime);                           //转换为int

		if(dwUserID == m_pUserInfo[i]->m_UserData.dwUserID)
		{
			m_UserInfo.dwChangePoint = iPoint;

			sSendMoneyByTimes.i64SrcGameMoney = m_pUserInfo[i]->m_UserData.i64Money;//钱
			sSendMoneyByTimes.i64SrcMoneyInBank = m_pUserInfo[i]->m_UserData.i64Bank;//银行
			
			
			break;
		}
	}
	m_pDataManage->m_SQLDataManage.PushLine(&m_UserInfo.DataBaseHead,sizeof(m_UserInfo),DTK_GR_UPDATE_GAME_RESULT,0,0);

	//按次送金币
	
	m_pDataManage->m_SQLDataManage.PushLine(&sSendMoneyByTimes.DataBaseHead,sizeof(sSendMoneyByTimes),DTK_GR_SENDMONEY_TIMES,0,0);
	return ;
}
//发送
bool CGameDesk::SendGameData(BYTE bDeskStation, BYTE bMainID, BYTE bAssID, BYTE bHandleCode)
{
	ASSERT(bDeskStation <= m_bMaxPeople);
	if (bDeskStation < m_bMaxPeople)
	{
		if ((m_pUserInfo[bDeskStation] != NULL))
		{
			UINT uSocketIndex = m_pUserInfo[bDeskStation]->m_uSocketIndex;
			m_pDataManage->m_TCPSocket.SendData(uSocketIndex,bMainID,bAssID,bHandleCode,m_pUserInfo[bDeskStation]->m_dwHandleID);
		}
	}
	else 
	{
		for (BYTE i = 0; i < m_bMaxPeople; i ++)
		{
			if (m_pUserInfo[i] != NULL)
			{
				UINT uSocketIndex=m_pUserInfo[i]->m_uSocketIndex;
				m_pDataManage->m_TCPSocket.SendData(uSocketIndex,bMainID,bAssID,bHandleCode,m_pUserInfo[i]->m_dwHandleID);
			}
		}
	}
	return true;
}

//发送旁观数据 （发送消息给旁观者）
bool CGameDesk::SendWatchData(BYTE bDeskStation, BYTE bMainID, BYTE bAssID, BYTE bHandleCode)
{
	ASSERT(bDeskStation <= m_bMaxPeople);
	for (INT_PTR i = 0; i< m_WatchUserPtr.GetCount(); i ++)
	{
		CGameUserInfo * pUserInfo = (CGameUserInfo *)m_WatchUserPtr.GetAt(i);
		if (((pUserInfo != NULL)&&(pUserInfo->m_UserData.bUserState != USER_CUT_GAME))
			&&((pUserInfo->m_UserData.bDeskStation == bDeskStation)||(bDeskStation == m_bMaxPeople)))
		{
			m_pDataManage->m_TCPSocket.SendData(pUserInfo->m_uSocketIndex,bMainID,bAssID,bHandleCode,pUserInfo->m_dwHandleID);
		}
	}
	return true;
}

//发送数据函数 （发送消息给游戏者）
bool CGameDesk::SendGameData(BYTE bDeskStation, void * pData, UINT uSize, BYTE bMainID, BYTE bAssID, BYTE bHandleCode)
{
	ASSERT(bDeskStation <= m_bMaxPeople);
	if (bMainID == MDM_GM_MESSAGE && bAssID == ASS_GM_SYSTEM_MESSAGE_EX)
	{

		GameNoticeMessage* p = (GameNoticeMessage*)pData;
		if (p != NULL)
		{
			m_pDataManage->m_TCPSocket.SendDataBatch(pData, uSize, MDM_GM_GAME_NOTIFY, ASS_GM_MESSAGE_EX,0);

			//发送数据
			//	定义数据 
			MSG_GA_S_Message Message;
			memset(&Message,0,sizeof(Message));
			Message.bFontSize=0;
			Message.bCloseFace=false;
			Message.bShowStation=SHS_TALK_MESSAGE;
			sprintf(Message.szMessage, p->szMessage);
			m_pDataManage->m_TCPSocket.SendDataBatch(&Message,sizeof(Message),MDM_GR_MESSAGE,ASS_GR_SYSTEM_MESSAGE,0);
			return true;
		}
	}
	if (bDeskStation < m_bMaxPeople)
	{
		if ((m_pUserInfo[bDeskStation] != NULL)&&(m_pUserInfo[bDeskStation]->m_UserData.bUserState != USER_CUT_GAME))
		{
			UINT uSocketIndex = m_pUserInfo[bDeskStation]->m_uSocketIndex;
			m_pDataManage->m_TCPSocket.SendData(uSocketIndex,pData,uSize,bMainID,bAssID,bHandleCode,m_pUserInfo[bDeskStation]->m_dwHandleID);
		}
	}
	else 
	{
		for (BYTE i = 0; i < m_bMaxPeople; i ++)
		{
			if ((m_pUserInfo[i] != NULL)&&(m_pUserInfo[i]->m_UserData.bUserState != USER_CUT_GAME))
			{
				m_pDataManage->m_TCPSocket.SendData(m_pUserInfo[i]->m_uSocketIndex,pData,uSize,bMainID,bAssID,bHandleCode,m_pUserInfo[i]->m_dwHandleID);
			}
		}
	}
	return true;
}

//发送旁观数据 （发送消息给旁观者）
bool CGameDesk::SendWatchData(BYTE bDeskStation, void * pData, UINT uSize, BYTE bMainID, BYTE bAssID, BYTE bHandleCode)
{
	ASSERT(bDeskStation <= m_bMaxPeople);
	for (INT_PTR i = 0; i < m_WatchUserPtr.GetCount(); i ++)
	{
		CGameUserInfo * pUserInfo = (CGameUserInfo *)m_WatchUserPtr.GetAt(i);
		if (((pUserInfo!=NULL)&&(pUserInfo->m_UserData.bUserState != USER_CUT_GAME))&&
			((pUserInfo->m_UserData.bDeskStation == bDeskStation)||(bDeskStation == m_bMaxPeople)))
		{
			m_pDataManage->m_TCPSocket.SendData(pUserInfo->m_uSocketIndex,pData,uSize,bMainID,bAssID,bHandleCode,pUserInfo->m_dwHandleID);
		}
	}
	return true;
}

//发送旁观数据 （发送消息给旁观者）
bool CGameDesk::SendWatchDataByID(UINT uSendSocketID, void * pData, UINT uSize, BYTE bMainID, BYTE bAssID, BYTE bHandleCode)
{
	for (INT_PTR i = 0; i < m_WatchUserPtr.GetCount(); i ++)
	{
		CGameUserInfo * pUserInfo = (CGameUserInfo *)m_WatchUserPtr.GetAt(i);
		if ((pUserInfo != NULL)&&(pUserInfo->m_uSocketIndex == uSendSocketID)
			&&(pUserInfo->m_UserData.bUserState != USER_CUT_GAME))
		{
			m_pDataManage->m_TCPSocket.SendData(uSendSocketID,pData,uSize,bMainID,bAssID,bHandleCode,pUserInfo->m_dwHandleID);
			return true;
		}
	}
	return false;
}

//是否可以断线
bool CGameDesk::CanNetCut(BYTE bDeskStation)
{
	return m_bBreakCount[bDeskStation] < NET_CUT_TIMES;
}

/// 获得桌子中有多少人,为排队机添加
int CGameDesk::GetDeskPlayerNum()
{
	int iPeopoleCount = 0;
	for (BYTE i = 0 ; i < m_bMaxPeople; i ++) 
	{
		if (m_pUserInfo[i] != NULL)
		{
			iPeopoleCount++;
		}
	}

	return iPeopoleCount;
}

int CGameDesk::GetOnlineDeskPlayerNum()
{
	int iPeopoleCount = 0;
	for (BYTE i = 0 ; i < m_bMaxPeople; i ++) 
	{
		if (m_pUserInfo[i] != NULL && !m_bCutGame[i])
		{
			iPeopoleCount++;
		}
	}

	return iPeopoleCount;
}

/// 获得桌子中第一个没有人的座位索引
///@return 桌子满人了，返回-1;否则，返回第一个空座位索引
int CGameDesk::GetDeskNoPlayerStation()
{
	int iDeskStation = 0;
	for (BYTE i = 0 ; i < m_bMaxPeople; i ++) 
	{
		if (m_pUserInfo[i] != NULL)
		{
			iDeskStation++;
		}
		else
		{
			break;
		}
	}

	if (iDeskStation < m_bMaxPeople)
	{
		return iDeskStation;
	}
	else
	{
		return -1;
	}	
}

/// 判断此桌中玩家的ip是否有相同的,为排队机添加
///@return:有ip相同的玩家返回true，否则，返回false
bool CGameDesk::IsDeskIpSame(CGameUserInfo * pUserInfo)
{
	return false;
	
	if (NULL == pUserInfo)
	{
		return false;
	}

	ULONG dwUserIP=pUserInfo->m_UserData.dwUserIP;
	for (BYTE i=0;i<m_bMaxPeople;i++)
	{
		if(m_pUserInfo[i]==NULL)
			continue;

		//是否IP相同
		if ((m_pUserInfo[i]->m_UserData.dwUserIP==dwUserIP)
			/*&& pUserInfo->m_UserData.bGameMaster==0*/) 
		{
			return true;
		}
	}

	for (BYTE i=0;i<m_bMaxPeople-1;i++)
	{
		if (m_pUserInfo[i]==NULL) 
			continue;

		for (BYTE j=i+1;j<m_bMaxPeople;j++)
		{
			if ((m_pUserInfo[j]!=NULL)
				&& (m_pUserInfo[i]->m_UserData.dwUserIP==m_pUserInfo[j]->m_UserData.dwUserIP)
				/*&& pUserInfo->m_UserData.bGameMaster==0*/) 
			{
				return true;
			}
		}
	}

	/////////////////////////////////////////////////////////////////////
	return false;
}

/// 设置桌子中的玩家信息, 让玩家坐到指定的空座位上，或清空指定座位的玩家信息
///@param para1:用户信息的指针
///@param para2:座位索引
void CGameDesk::SetDeskUserInfo(CGameUserInfo * pUserInfo, int iDeskStation)
{
	if (iDeskStation < m_bMaxPeople)
	{
		if (NULL != pUserInfo && NULL == m_pUserInfo[iDeskStation])
		{
			m_pUserInfo[iDeskStation] = pUserInfo;
		}

		if (NULL == pUserInfo)
		{
			m_pUserInfo[iDeskStation] = NULL;
		}
	}
	
	return;
}

//判断是否可以开始游戏
bool CGameDesk::CanBeginGame()
{
    if(m_bFinishCondition == 3 && m_bIsPlay)
    {// 时效房间小于3秒不准在开始游戏
        COleDateTime nowTime = COleDateTime::GetCurrentTime();
        int iDay  = m_iBuyMinutes / (60*24);
        int iHour = (m_iBuyMinutes - iDay * 60 * 24) / 60;
        int iMinutes = (m_iBuyMinutes - iDay * 60 * 24 - iHour * 60);
        //int iSecond = iMinutes * 60;
        COleDateTimeSpan timespan( iDay, iHour, iMinutes, 0 );
        COleDateTimeSpan tmDiff = (m_iBuyMinutesDeskBeginTime + timespan - nowTime);

        if((long)tmDiff.GetTotalSeconds() < 3)
        {
            return false;
        }
    }
	//判断所有人是否同意
	BYTE bPeopoleCount = 0;
	for (BYTE i = 0 ; i < m_bMaxPeople; i ++) 
	{
		if (m_pUserInfo[i] != NULL)
		{
			bPeopoleCount ++;
			if (m_pUserInfo[i]->m_UserData.bUserState != USER_ARGEE)
				return false;
		}
	}

	//判断适合开始条件
	if (bPeopoleCount > 1L)
	{
		if (m_bBeginMode == SYMMETRY_ARGEE)
		{
			if ((bPeopoleCount % 2) != 0) 
				return false;

			if (bPeopoleCount == m_bMaxPeople)
				return true;
			BYTE bHalfCount = m_bMaxPeople/2;

			for (BYTE i = 0; i < bHalfCount; i ++)
			{
				if ((m_pUserInfo[i] == NULL)&&(m_pUserInfo[i + bHalfCount] != NULL)) 
					return false;

				if ((m_pUserInfo[i] != NULL)&&(m_pUserInfo[i + bHalfCount] == NULL)) 
					return false;
			}
			return true;
		}
		else 
		{
			if (m_bBeginMode == FULL_BEGIN) 
				return (bPeopoleCount == m_bMaxPeople);
			else if (m_bBeginMode == ALL_ARGEE && bPeopoleCount >= m_pDataManage->m_KernelData.uMinDeskPeople) 
			{
				if (m_bIsBuy && m_bPlayerNum!=0 && m_iRunGameCount==0)
				{
					if (bPeopoleCount == m_bPlayerNum)
					{
						return true;
					}
					return false;
				}
				return true;
			}
			//-----------6个人中有4个人开始---------------------
			else if (m_bBeginMode==FOUR_SIX_ARGEE)
			{
				if(bPeopoleCount >=4)
					return true;
				if (bPeopoleCount == m_bMaxPeople)
					return true;
				return false;
			}
		   //-----------6个人中有4个人开始---------------------
		}
	}

	return false;
}
/// 强制玩家离线函数
/// 无论是否在游戏中，都强制玩家离线，调用者须自行处理该玩家离线后的游戏进程
/// 清空本房间中的该玩家信息，调用结束后，该玩家离开本房间，同时m_pUserInfo[bDeskStation]会被清成NULL
/// 本函数与UserLeftDesk的区别在于，本函数是服务器根据玩家身上的金币数和其它在服务器端判断的条件，主动令玩家退出房间。而
/// UserLeftDesk函数则是由客户端发出离桌消息到服务器后，再由平台调用的函数，具体游戏根据自身的规则，作相应处理，根据游戏
/// 进行状态，不一定会令玩家退出本局游戏或房间
/// 
/// @param bDeskStation 玩家在桌子中的位置号
/// @return 如果bDeskStation大于本房间一桌最大人数，或m_pUserInfo[bDeskStation]已经是空值NULL，则返回false，否则返回true
bool CGameDesk::MakeUserOffLine(BYTE bDeskStation,bool bSendUserUp /*= true*/)
{
	/// 如果没找到这个玩家，则返回 false
	if (bDeskStation>=m_bMaxPeople)
	{
		return false;
	}
	if(m_pUserInfo[bDeskStation] == NULL) 
	{
		return false;
	}

	//广播用户离开
	MSG_GR_R_UserSit UserSit;
	UserSit.bLock=m_bLock;
	UserSit.bDeskIndex=m_bDeskIndex;
	UserSit.bDeskStation=bDeskStation;
	UserSit.bUserState=USER_LOOK_STATE;
	UserSit.dwUserID=m_pUserInfo[bDeskStation]->m_UserData.dwUserID;
	UserSit.bIsDeskOwner=false;
	UserSit.bLeave = bSendUserUp;
	UserSit.bDeskMaster=(m_bIsBuy&&m_iDeskMaster==m_pUserInfo[bDeskStation]->m_UserData.dwUserID);//房主
	m_pDataManage->m_TCPSocket.SendDataBatch(&UserSit,sizeof(UserSit),MDM_GR_USER_ACTION,ASS_GR_USER_UP,ERR_GR_SIT_SUCCESS);

	m_pDataManage->AutoAllotDeskDeleteUser(m_pUserInfo[bDeskStation]);

	CGameUserInfo *pUserInfo = m_pDataManage->m_UserManage.FindOnLineUser(m_pUserInfo[bDeskStation]->m_UserData.dwUserID);
	if (pUserInfo)
	{
		pUserInfo->m_UserData.bDeskNO=255;
		pUserInfo->m_UserData.bDeskStation=255;
		pUserInfo->m_UserData.bUserState=USER_LOOK_STATE;

		if (bSendUserUp)
		{
			m_pDataManage->m_TCPSocket.SendData(m_pUserInfo[bDeskStation]->GetSocketIndex(),MDM_GR_USER_ACTION,ASS_GR_FORCE_QUITE,0,0);
		}
	}
	else
	{
		pUserInfo = m_pDataManage->m_UserManage.FindNetCutUser(m_pUserInfo[bDeskStation]->m_UserData.dwUserID);
		if (pUserInfo)
		{
			pUserInfo->m_UserData.bDeskNO=255;
			pUserInfo->m_UserData.bDeskStation=255;
			pUserInfo->m_UserData.bUserState=USER_LOOK_STATE;

			m_pDataManage->CleanUserInfo(pUserInfo);
			m_pDataManage->m_UserManage.FreeUser(pUserInfo,false);
			m_pUserInfo[bDeskStation]=NULL;
		}
	}

	m_uCutTime[bDeskStation]=0L;
	m_bConnect[bDeskStation]=false;
	m_pUserInfo[bDeskStation]=NULL;
	m_bEnableWatch&=~(1<<bDeskStation);
	
	if (m_pDataManage->IsQueueGameRoom())
	{
		for (int i = 0; i < m_bMaxPeople; i++)
		{
			if (m_pUserInfo[i] != NULL)
			{
				CGameUserInfo *p = m_pDataManage->m_UserManage.FindOnLineUser(m_pUserInfo[i]->m_UserData.dwUserID);

				if (p && p != pUserInfo)
				{
					UINT uSocketIndex = p->m_uSocketIndex;
					m_pDataManage->m_TCPSocket.SendData(uSocketIndex,&p->m_UserData.dwUserID,sizeof(int),MDM_GR_ROOM,ASS_GR_ROOM_QUEUE_READY, 0, 0);

					m_bLeave = false;
					UserLeftDesk(p->m_UserData.bDeskStation,p);
					m_bLeave = true;

					if((m_pDataManage->m_InitData.dwRoomRule & GRR_CONTEST) || (m_pDataManage->m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
					{
						//重置该桌子的状态
						/*m_pUserInfo[i]->m_UserData.bDeskNO=255;
						m_pUserInfo[i]->m_UserData.bDeskStation=255;
						m_pUserInfo[i]->m_UserData.bUserState=USER_LOOK_STATE;

						m_uCutTime[i]=0L;
						m_bConnect[i]=false;
						m_pUserInfo[i]=NULL;*/
					}
					else
					{
						m_pDataManage->m_pIAutoAllotDesk->Add(p);
					}
				}
			}
		}
		return true;
	}

	return true;
}

//用户离开游戏桌
BYTE CGameDesk::UserLeftDesk(BYTE bDeskStation, CGameUserInfo * pUserInfo)
{
	if(m_pDataManage->m_InitData.dwRoomRule & GRR_GAME_BUY)
	{
		if(GetOnlineDeskPlayerNum()<=1)
		{
			KillTimer(IDT_CHECK_DESK);
			SetTimer(IDT_CHECK_DESK,3*60*1000);
		}
	}
	//防止由于数据访问越界造成崩溃，在27王游戏中出现过
	//检查传进来的数据的是否有效
	if (bDeskStation < 0 || bDeskStation >= m_bMaxPeople)
	{
		return ERR_GR_SIT_SUCCESS;
	}
	if (! pUserInfo )
	{
		return ERR_GR_SIT_SUCCESS;
	}
	///////////

	if(m_pUserInfo[bDeskStation] == NULL) 
		return ERR_GR_SIT_SUCCESS;
	//效验数据 
	ASSERT(m_pUserInfo[bDeskStation] == pUserInfo);
    DWORD dwUserID = pUserInfo->m_UserData.dwUserID;
	//设置数据
	m_bConnect[bDeskStation] = false;
	if (IsPlayGame(bDeskStation))
		GameFinish(bDeskStation,GFF_FORCE_FINISH);


	//更新规则
	UINT uSitCount = 0L;
	for (BYTE i = 0; i < m_bMaxPeople; i ++)
	{
		if (m_pUserInfo[i] != NULL) 
			uSitCount ++;
	}
	if (uSitCount == 1)
	{
		m_bLock=false;
		m_szLockPass[0]=0;
	}

	//发送房间消息
	MSG_GR_R_UserSit UserSit;
	UserSit.bLock=m_bLock;
	UserSit.bDeskIndex=m_bDeskIndex;
	UserSit.bDeskStation=bDeskStation;
	UserSit.bUserState=USER_LOOK_STATE;
	UserSit.dwUserID=dwUserID;
	UserSit.bIsDeskOwner=(m_dwOwnerUserID==dwUserID);
	UserSit.bLeave = m_bLeave;
	UserSit.bDeskMaster=(m_bIsBuy&&m_iDeskMaster==dwUserID);//房主
	m_pDataManage->m_TCPSocket.SendDataBatch(&UserSit,sizeof(UserSit),MDM_GR_USER_ACTION,ASS_GR_USER_UP,ERR_GR_SIT_SUCCESS);

	//更新台主
	if (m_dwOwnerUserID==dwUserID)
	{
		m_dwOwnerUserID=0L;
		//m_DeskBasePoint=0;
		for(int i=0; i<MAX_PEOPLE; i++)
		{
			if(i == bDeskStation)
				continue;
			// 查找下一个台主
			if(m_pUserInfo[i])
			{
				m_dwOwnerUserID = m_pUserInfo[i]->m_UserData.dwUserID;
				break;
			}
		}
	}

	//设置数据
	m_uCutTime[bDeskStation]=0L;
	m_bConnect[bDeskStation]=false;
	m_pUserInfo[bDeskStation]=NULL;
	m_bEnableWatch&=~(1<<bDeskStation);
    if (NULL != pUserInfo)
    {
        pUserInfo->m_UserData.bDeskNO = 255;
        pUserInfo->m_UserData.bDeskStation = 255;
        pUserInfo->m_UserData.bUserState = USER_LOOK_STATE;
    }

	//判断是否可以开始
	if (CanBeginGame()==true) GameBegin(0);

	if(m_bIsBuy && m_bInDissmissing)
	{
		vector<int>::iterator result = find( m_VecAgreeUserID.begin( ), m_VecAgreeUserID.end( ), pUserInfo->m_UserData.dwUserID ); 
		if ( result == m_VecAgreeUserID.end( ) ) 
		{
			if(m_iDissmissAgreePeople>=GetOnlineDeskPlayerNum())
			{
				GameFinish(0,GFF_DISSMISS_FINISH);
			}
			else
			{
				m_VecAgreeUserID.push_back(pUserInfo->m_UserData.dwUserID);
				VipDeskDismissAgreeRes tAgreeData;
				tAgreeData.bAgree=true;
				tAgreeData.iUserID=pUserInfo->m_UserData.dwUserID;
				for(int i=0;i<m_bMaxPeople;i++)
				{
					if(!m_pUserInfo[i])
					{
						continue;
					}
					m_pDataManage->m_TCPSocket.SendData(m_pUserInfo[i]->m_uSocketIndex,&tAgreeData,sizeof(tAgreeData),MDM_GR_DESKRUNOUT,ASS_GR_DISSMISS_AGREE,0,0);
				}
			}
		}
		else 
		{
			m_iDissmissAgreePeople--;
		}
	}

	if (m_bIsBuy && pUserInfo->m_UserData.dwUserID != m_iDeskMaster)
	{
		DL_GR_I_MasterLeave MasterLeave;
		MasterLeave.bLeave = true;
		MasterLeave.bMaster = false;
		MasterLeave.iDeskID = m_bDeskIndex;
		MasterLeave.iUserID = pUserInfo->m_UserData.dwUserID;

		m_pDataManage->m_SQLDataManage.PushLine(&MasterLeave.DataBaseHead, sizeof(MasterLeave), DTK_GR_MASTER_LEAVE, 0, 0);	
	}

	if (m_pDataManage->IsQueueGameRoom() && m_bQueueReset)
	{
		m_bQueueReset = false;
		for (int i = 0; i < m_bMaxPeople; i++)
		{
			if (m_pUserInfo[i] != NULL)
			{
				CGameUserInfo *p = m_pDataManage->m_UserManage.FindOnLineUser(m_pUserInfo[i]->m_UserData.dwUserID);

				if (p && p != pUserInfo)
				{
					UINT uSocketIndex = p->m_uSocketIndex;
					m_pDataManage->m_TCPSocket.SendData(uSocketIndex,&p->m_UserData.dwUserID,sizeof(int),MDM_GR_ROOM,ASS_GR_ROOM_QUEUE_READY, 0, 0);

					m_bLeave = false;
					UserLeftDesk(p->m_UserData.bDeskStation,p);
					m_bLeave = true;

					if((m_pDataManage->m_InitData.dwRoomRule & GRR_CONTEST) || (m_pDataManage->m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
					{
						//重置该桌子的状态
						/*m_pUserInfo[i]->m_UserData.bDeskNO=255;
						m_pUserInfo[i]->m_UserData.bDeskStation=255;
						m_pUserInfo[i]->m_UserData.bUserState=USER_LOOK_STATE;

						m_uCutTime[i]=0L;
						m_bConnect[i]=false;
						m_pUserInfo[i]=NULL;*/
					}
					else
					{
						m_pDataManage->m_pIAutoAllotDesk->Add(p);
					}
				}
			}
		}
		m_bQueueReset = true;
	}

	return ERR_GR_SIT_SUCCESS;
}

//旁观用户坐到游戏桌
BYTE CGameDesk::WatchUserSitDesk(MSG_GR_S_UserSit* pUserSit, CGameUserInfo * pUserInfo)
{
	//效验数据 
	if (pUserInfo==NULL) return ERR_GR_ERROR_UNKNOW;
	//不允许旁观
	if(m_pDataManage->m_InitData.dwRoomRule&GRR_UNENABLE_WATCH)
		return ERR_GR_UNENABLE_WATCH;

	//寻找旁观位置
	UINT dwWatchStation;
	for (dwWatchStation=0;dwWatchStation<(UINT)m_WatchUserPtr.GetCount();dwWatchStation++)
	{
		if (m_WatchUserPtr.GetAt(dwWatchStation)==NULL)	break;
	}

	//密码效验
	if (m_bLock==true)
	{
		if (lstrcmp(m_szLockPass,pUserSit->szPassword)!=0)//20081205
			return ERR_GR_PASS_ERROR;
	}

	//设置数据
	pUserInfo->m_UserData.bDeskNO=m_bDeskIndex;
	pUserInfo->m_UserData.bDeskStation=pUserSit->bDeskStation;
	pUserInfo->m_UserData.bUserState=USER_WATCH_GAME;
	m_WatchUserPtr.SetAtGrow(dwWatchStation,pUserInfo);

	//发送大厅消息
	MSG_GR_R_UserSit UserWatchSit;
	UserWatchSit.bLock=m_bLock;
	UserWatchSit.bDeskIndex=m_bDeskIndex;
	UserWatchSit.bDeskStation=pUserSit->bDeskStation;
	UserWatchSit.bUserState=USER_WATCH_GAME;
	UserWatchSit.dwUserID=pUserInfo->m_UserData.dwUserID;
	UserWatchSit.bDeskMaster=false;//房主
	m_pDataManage->m_TCPSocket.SendDataBatch(&UserWatchSit,sizeof(UserWatchSit),MDM_GR_USER_ACTION,ASS_GR_WATCH_SIT,ERR_GR_SIT_SUCCESS);

	return ERR_GR_SIT_SUCCESS;
}

//旁观用户离开游戏桌
BYTE CGameDesk::WatchUserLeftDesk(BYTE bDeskStation, CGameUserInfo * pUserInfo)
{
	//效验数据 
	if (pUserInfo==NULL) return ERR_GR_ERROR_UNKNOW;
	UINT dwWatchStation;
	//寻找旁观位置
	for (dwWatchStation=0;dwWatchStation<(UINT)m_WatchUserPtr.GetCount();dwWatchStation++)
	{
		CGameUserInfo * pWatchUserInfo=(CGameUserInfo *)m_WatchUserPtr.GetAt(dwWatchStation);
		if (pWatchUserInfo==pUserInfo) break;
	}
	if (dwWatchStation==m_WatchUserPtr.GetCount()) return ERR_GR_ERROR_UNKNOW;

	//发送大厅消息
	MSG_GR_R_UserSit UserWatchUp;
	UserWatchUp.bLock=m_bLock;
	UserWatchUp.bDeskIndex=m_bDeskIndex;
	UserWatchUp.bDeskStation=bDeskStation;
	UserWatchUp.bUserState=USER_LOOK_STATE;
	UserWatchUp.dwUserID=pUserInfo->m_UserData.dwUserID;
	UserWatchUp.bDeskMaster=false;//房主
	m_pDataManage->m_TCPSocket.SendDataBatch(&UserWatchUp,sizeof(UserWatchUp),MDM_GR_USER_ACTION,ASS_GR_WATCH_UP,ERR_GR_SIT_SUCCESS);

	//设置数据
	pUserInfo->m_UserData.bDeskNO=255;
	pUserInfo->m_UserData.bDeskStation=255;
	pUserInfo->m_UserData.bUserState=USER_LOOK_STATE;
	m_WatchUserPtr.SetAt(dwWatchStation,NULL);

	return ERR_GR_SIT_SUCCESS;
}

//用户同意开始
bool CGameDesk::UserAgreeGame(BYTE bDeskStation)
{
	if (m_pUserInfo[bDeskStation]!=NULL
		&&m_pUserInfo[bDeskStation]->m_UserData.i64Money < m_pDataManage->m_InitData.uLessPoint
		&&m_pDataManage->m_InitData.uComType == TY_MONEY_GAME
		&& !(m_pDataManage->m_InitData.dwRoomRule&GRR_GAME_BUY) && !(m_pDataManage->m_InitData.dwRoomRule & GRR_CONTEST || m_pDataManage->m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
	{
		UserLeftDesk(bDeskStation,m_pUserInfo[bDeskStation]);
		return true;
	}
	HNLOG_G(55,"ASS_GM_AGREE_GAME DeskPass:%s UserID:%d",m_szDeskPassWord,m_pUserInfo[bDeskStation]?m_pUserInfo[bDeskStation]->m_UserData.dwUserID:0);
    //排队机场人数不够时须重新参与排队
    if(m_pDataManage->IsQueueGameRoom())
    {
		char cKey[10];
		CString sPath=CBcfFile::GetAppPath();
		CBcfFile fsr(sPath + "SpecialRule.bcf");
		sprintf(cKey, "%d", m_pDataManage->m_KernelData.uNameID);
		int HaveZhuang = fsr.GetKeyVal (_T("IsHaveZhuang"), cKey, 0);
		if(
			///非连庄游戏在游戏中收到同意消息，解散此桌
			(m_pUserInfo[bDeskStation] != NULL && (HaveZhuang == 0) && 
			(m_pUserInfo[bDeskStation]->m_UserData.bUserState == USER_PLAY_GAME ||
			m_pUserInfo[bDeskStation]->m_UserData.bUserState == USER_SITTING))  || 
			(GetDeskPlayerNum() <= m_bMaxPeople && m_pUserInfo[bDeskStation] != NULL)
			)
        {

			//设置数据
			m_pUserInfo[bDeskStation]->m_UserData.bUserState=USER_ARGEE;

			//先发送游戏同意消息, 配合排队机自动进入排队

			MSG_GR_R_UserAgree UserAgree;
			UserAgree.bAgreeGame=TRUE;
			UserAgree.bDeskNO=m_bDeskIndex;
			UserAgree.bDeskStation=bDeskStation;
			UINT uSocketIndex = m_pUserInfo[bDeskStation]->m_uSocketIndex;

			//判断是否可以开始
			if (CanBeginGame()==true) 
			{
				GameBegin(0);
				return true;
			}

			m_pDataManage->m_TCPSocket.SendDataBatch(&UserAgree,sizeof(UserAgree),MDM_GR_ROOM,ASS_GR_USER_AGREE,0);

            return true;
        }
    }

	if (m_pUserInfo[bDeskStation]!=NULL)
	{
		//设置数据
		m_pUserInfo[bDeskStation]->m_UserData.bUserState=USER_ARGEE;

		//发送游戏同意消息
		MSG_GR_R_UserAgree UserAgree;
		UserAgree.bAgreeGame=TRUE;
		UserAgree.bDeskNO=m_bDeskIndex;
		UserAgree.bDeskStation=bDeskStation;

		m_pDataManage->m_TCPSocket.SendDataBatch(&UserAgree,sizeof(UserAgree),MDM_GR_ROOM,ASS_GR_USER_AGREE,0);

		if (m_pDataManage->m_InitData.dwRoomRule&GRR_GAME_BUY)
		{
			if ((m_bPositionCheck || m_bIPCheck) && !IsBuyDeskPlaying() && CanBeginGame()==true)
			{
				onCheckPrevent();
				return true;
			}
			else
			{
				if (!m_pDataManage->IsQueueGameRoom() && CanBeginGame()==true) 
				{
					GameBegin(0);
					return true;
				}
			}
			return true;
		}
		else
		{
			//判断是否可以开始
			if (!m_pDataManage->IsQueueGameRoom() && CanBeginGame()==true) 
			{
				GameBegin(0);
				return true;
			}

			return true;
		}
	}

	return false;
}

//发送游戏状态
bool CGameDesk::SendGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser, void * pStationData, UINT uSize)
{
	if (NULL == m_pDataManage)
	{
		return true;
	}

	//获取游戏信息
	UINT uSendSize=0;
	MSG_GM_S_GameInfo GameInfo;
	GameInfo.bGameStation=m_bGameStation;///< 游戏状态
	GameInfo.bWatchOther=((m_bEnableWatch&(1<<bDeskStation))!=0);
	GameInfo.bWaitTime=m_bWaitTime;
	GameInfo.bReserve=FALSE;
	if (m_pDataManage->m_szMsgGameLogon[0]!=0)
	{
		lstrcpy(GameInfo.szMessage,m_pDataManage->m_szMsgGameLogon);
		uSendSize=sizeof(GameInfo)-sizeof(GameInfo.szMessage)+(lstrlen(m_pDataManage->m_szMsgGameLogon)+1)*sizeof(char);
	}
	else 
	{
		GameInfo.szMessage[0]=0;
		uSendSize=sizeof(GameInfo)-sizeof(GameInfo.szMessage);
	}

	//发送游戏信息

	if (bWatchUser==false) SendGameData(bDeskStation,&GameInfo,uSendSize,MDM_GM_GAME_FRAME,ASS_GM_GAME_INFO,0);
	else SendWatchDataByID(uSocketID,&GameInfo,uSendSize,MDM_GM_GAME_FRAME,ASS_GM_GAME_INFO,0);

	//发送游戏状态
	if (bWatchUser==false) SendGameData(bDeskStation,pStationData,uSize,MDM_GM_GAME_FRAME,ASS_GM_GAME_STATION,0);
	else SendWatchDataByID(uSocketID,pStationData,uSize,MDM_GM_GAME_FRAME,ASS_GM_GAME_STATION,0);

	/// 排队机游戏是否可以开始
	if (m_pDataManage->IsQueueGameRoom()/* && CanBeginGame()*/)
	{
		if (!IsPlayGame(0))
			UserAgreeGame(bDeskStation);


		if (!(m_pDataManage->m_InitData.dwRoomRule & GRR_CONTEST) && !(m_pDataManage->m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
		{
			for (BYTE i=0; i<m_bMaxPeople; i++)
			{
				if (m_pUserInfo[i]!=NULL)
				{
					if (!m_bConnect[m_pUserInfo[i]->m_UserData.bDeskStation])
					{
						return true;
					}
				}
			}
		}
	}

	return true;
}

//游戏开始
bool CGameDesk::GameBegin(BYTE bBeginFlag)
{
	if ((m_pDataManage->m_InitData.uLessPoint!=0)&&(m_pDataManage->m_InitData.uComType==TY_MONEY_GAME))
	{
		bool quit=false;
		for (BYTE i=0;i<m_bMaxPeople;i++)
		{
			if (m_pDataManage->m_InitData.dwRoomRule & GRR_CONTEST || m_pDataManage->m_InitData.dwRoomRule & GRR_TIMINGCONTEST || m_pDataManage->m_InitData.dwRoomRule&GRR_GAME_BUY)
				break;
			if (m_pUserInfo[i]!=NULL
				&&((m_pUserInfo[i]->m_UserData.i64Money < (int)m_pDataManage->m_InitData.uLessPoint
				&&m_pDataManage->m_InitData.uComType == TY_MONEY_GAME)
				||m_pDataManage->m_InitData.uComType != TY_MONEY_GAME))
			{
				UINT SocketIndex = m_pUserInfo[i]->m_uSocketIndex;
				MakeUserOffLine(i);
				m_pDataManage->m_TCPSocket.SendData(SocketIndex, MDM_GR_ROOM, ASS_GR_MONEY_NOTENOUGH, 0, 0);
				quit=true;
			}
		}
		if(quit) return false;
	}

	//设置数据
	m_bPlayGame = true;
	m_dwBeginTime=(long int)time(NULL);
	m_dwTax=0;
	memset(m_bCutGame,0,sizeof(m_bCutGame));
	memset(m_uCutTime,0,sizeof(m_uCutTime));
	memset(m_bBreakCount,0,sizeof(m_bBreakCount));
	memset(m_dwChangePoint,0,sizeof(m_dwChangePoint));
	memset(m_dwChangeMoney,0,sizeof(m_dwChangeMoney));
	memset(m_dwTaxCom,0,sizeof(m_dwTaxCom));

	m_byDoublePointEffect = 0;
	m_bAllRobot = true;

	//记录游戏
	for (BYTE i=0;i<m_bMaxPeople;i++)
	{
		if (m_pUserInfo[i]!=NULL)
		{
			CString str;
			m_dwGameUserID[i]=m_pUserInfo[i]->m_UserData.dwUserID;
//			m_dwScrPoint[i]=m_pUserInfo[i]->m_UserData.dwPoint;
			m_pUserInfo[i]->m_UserData.bUserState=USER_PLAY_GAME;
			if (m_pDataManage->m_InitData.dwRoomRule & GRR_GAME_BUY)
				m_GameUserInfo[i]=m_pUserInfo[i]->m_UserData.dwUserID;

			if (m_pUserInfo[i]->m_UserData.isVirtual == false)
				m_bAllRobot = false;
		}
		else
		{
			m_dwScrPoint[i]=0L;
			m_dwGameUserID[i]=0L;
		}
	}

	m_tBeginTime = CTime::GetCurrentTime();
	//发送大厅消息
	m_pDataManage->m_TCPSocket.SendDataBatch(NULL,0,MDM_GR_ROOM,ASS_GR_GAME_BEGIN,m_bDeskIndex);

	//重置恢复房间标识
	m_bReturnDesk = false;
	//记录断线重连数据
	if (m_pDataManage->m_InitData.dwRoomRule & GRR_GAME_BUY)
	{
		//记录游戏状态
		DL_GR_I_CutNetRecordStart input_net;
		input_net.bStart=true;
		input_net.byDeskIndex=m_bDeskIndex;
		input_net.iRoomID=m_pDataManage->m_InitData.uRoomID;
		input_net.iUserID=m_iDeskMaster;
		m_pDataManage->m_SQLDataManage.PushLine(&input_net.DataBaseHead, sizeof(input_net), DTK_GR_CUTNETRECORDSTART, 0, 0);

		if (DESK_TYPE_BJL!=m_uDeskType)
		{
			DL_GR_I_CutNetRecord _data;
			strcpy_s(_data.szDeskPass,m_szDeskPassWord);
			_data.iPeopleCount = m_bMaxPeople;
			for (int i=0;i<m_bMaxPeople;i++)
			{
				if (m_pUserInfo[i]!=NULL)
				{
					_data.iUserID[i]=m_pUserInfo[i]->m_UserData.dwUserID;
				}
			}
			m_pDataManage->m_SQLDataManage.PushLine(&_data.DataBaseHead, sizeof(_data), DTK_GR_CUTNETRECORD, 0, 0);
		}

        if (m_iClubID != 0 && m_iRunGameCount==0)		// 俱乐部房间通知俱乐部玩家
        {
            MSG_GP_O_Club_RoomChange  _outdata;
            _outdata.bCreate = false;
            _outdata.iClub = m_iClubID;
            strcpy_s(_outdata._data.szDeskPass,m_szDeskPassWord);

            _outdata._data.bIsPlay = true;
            if(M_bMidEnter==0)		// 俱乐部房间通知俱乐部玩家
            {//  不允许加入
                _outdata._data.bAllowEnter = false;
            }
            else
            { 
                if(GetDeskPlayerNum() >= m_bPlayerNum && m_bPlayerNum != 0)
                {
                    _outdata._data.bAllowEnter = false;
                }
                else if(GetDeskPlayerNum() >= m_bMaxPeople && m_bPlayerNum == 0)
                {
                    _outdata._data.bAllowEnter = false;
                }
                else
                {
                    _outdata._data.bAllowEnter = true;
                }
            }

            m_pDataManage->m_pMainServerSocket->SendData(&_outdata,sizeof(_outdata),MDM_MG_CLUB,ASS_MG_CLUB_CLEARDESK,0);
        }

        if(m_iRunGameCount == 0)
        {
            m_iBuyMinutesDeskBeginTime = COleDateTime::GetCurrentTime();
            DL_GR_I_UpdateBeginTime _data;
            _data.iRoomID = m_pDataManage->m_InitData.uRoomID;
            _data.iDeskID = m_bDeskIndex;
            memcpy(_data.szDeskPass, m_szDeskPassWord, sizeof(_data.szDeskPass));
            m_pDataManage->m_SQLDataManage.PushLine(&_data.DataBaseHead, sizeof(_data), DTK_GR_UPDATEBEGINTIME, 0, 0);
            if(m_bFinishCondition == 3)
            {// 时效房间
                SetTimer(IDT_DESKRUNOUT_TIMER, m_iBuyMinutes * 60 * 1000);
            }
        }

		VipDeskInfoResult outdata;
		outdata.byDeskIndex = m_bDeskIndex;
		outdata.iMasterID = m_iDeskMaster;
		outdata.iPlayCount = m_iVipGameCount;
		outdata.iNowCount = m_iRunGameCount + 1;
        outdata.iType = m_bFinishCondition;
		strcpy_s(outdata.szPassWord, m_szDeskPassWord);
		strcpy_s(outdata.szGameName, m_pDataManage->m_szGameName);
        

        COleDateTime nowTime = COleDateTime::GetCurrentTime();
        int iDay  = m_iBuyMinutes / (60*24);
        int iHour = (m_iBuyMinutes - iDay * 60 * 24) / 60;
        int iMinutes = (m_iBuyMinutes - iDay * 60 * 24 - iHour * 60);
        COleDateTimeSpan timespan( iDay, iHour, iMinutes, 0 );
        COleDateTimeSpan tmDiff = (m_iBuyMinutesDeskBeginTime + timespan - nowTime);

        outdata.iSeconds = (long)tmDiff.GetTotalSeconds();
        if(outdata.iSeconds < 0) outdata.iSeconds = 1;
        outdata.bTimeKeeper = 1;

		for (int i = 0; i < m_bMaxPeople; i++)
		{
			if (!m_pUserInfo[i])
			{
				continue;
			}
			m_pDataManage->m_TCPSocket.SendData(m_pUserInfo[i]->m_uSocketIndex, &outdata, sizeof(outdata), MDM_GR_DESKRUNOUT, ASS_GR_GETDESKMASTER, 0, 0);
		}

		if (m_bIsBuy && m_iPayType != 1 && m_iRunGameCount == 0)//游戏开始准备扣费
		{
			if (2 == m_iPayType)
			{
				for (int i = 0; i < m_bMaxPeople; i++)
				{
					if (!m_pUserInfo[i])
					{
						continue;
					}
					if (m_pUserInfo[i]->m_UserData.iJewels < m_iAANeedJewels)
					{
						return false;
					}
				}
			}
			DL_GR_I_BuyRoom_CostFee _indata;
			strcpy_s(_indata.szDeskPassWord, m_szDeskPassWord);
			_indata.iType = 0;
			m_pDataManage->m_SQLDataManage.PushLine(&_indata.DataBaseHead, sizeof(_indata), DTK_GR_BUYROOM_COSTFEE, 0, 0);

			if (m_pDataManage->m_InitData.bLockMaster == 0)
			{
				bool isMasterExist = false;//房主是否在桌子
				for (int i=0;i<m_bMaxPeople;i++)
				{
					if(m_pUserInfo[i]!=NULL)
					{
						if (m_pUserInfo[i]->m_UserData.dwUserID == m_iDeskMaster)
						{
							isMasterExist = true;
							break;
						}
					}
				}
				if (!isMasterExist)
				{
					DL_GR_I_MasterLeave MasterLeave;
					MasterLeave.bLeave = true;
					MasterLeave.bMaster = true;
					MasterLeave.iDeskID = m_bDeskIndex;
					MasterLeave.iUserID = m_iDeskMaster;
					m_pDataManage->m_SQLDataManage.PushLine(&MasterLeave.DataBaseHead, sizeof(MasterLeave), DTK_GR_MASTER_LEAVE, 0, 0);
					m_bMasterState = 1;
				}
			}
			m_bIsPlay = true;
		}
	}
	if (!m_bAllRobot)
	{
		memset(m_GameSN,0,sizeof(m_GameSN));
		CreateGameSN();
	}
	
	return true;
}
  
//游戏结束
bool CGameDesk::GameFinish(BYTE bDeskStation, BYTE bCloseFlag)
{
	if (m_pDataManage->m_InitData.dwRoomRule & GRR_CONTEST || m_pDataManage->m_InitData.dwRoomRule & GRR_TIMINGCONTEST)
	{
		return GameFinishContest(bDeskStation, bCloseFlag);
	}
	
	//设置数据
	m_bPlayGame=false;
	m_dwBeginTime=(long int)time(NULL);
	//m_iDissmissAgreePeople=0;
	if((m_pDataManage->m_InitData.dwRoomRule & GRR_GAME_BUY) && m_bIsBuy)
	{
		if((m_iRunGameCount>=m_iVipGameCount && 0 == m_bFinishCondition) || GFF_DISSMISS_FINISH == bCloseFlag)//购买桌子已到期
		{
			m_pDataManage->OnClearDesk(m_bDeskIndex);
		}
	}
	//发送结束消息
	for (BYTE i=0;i<m_bMaxPeople;i++)
	{
		if (m_pUserInfo[i] == NULL)
		{
			continue;
		}
		if (m_pDataManage->IsLeaseTimeOver())
		{
			m_pDataManage->m_TCPSocket.SendData(m_pUserInfo[i]->m_uSocketIndex, MDM_GR_USER_ACTION, ASS_GR_LEASE_TIMEOVER, 0, 0);
			MakeUserOffLine(i);
		}
		// 如果玩家已经没钱了，则马上离开桌子
		if ((m_pUserInfo[i]->m_UserData.i64Money < (int)m_pDataManage->m_InitData.uLessPoint)
			&&(m_pDataManage->m_InitData.uComType == TY_MONEY_GAME)
			&& !(m_pDataManage->m_InitData.dwRoomRule&GRR_GAME_BUY))
		{
			if (!m_pDataManage->IsQueueGameRoom())
			{
				UINT SocketIndex = m_pUserInfo[i]->m_uSocketIndex;
				MakeUserOffLine(i);
				m_pDataManage->m_TCPSocket.SendData(SocketIndex, MDM_GR_ROOM, ASS_GR_MONEY_NOTENOUGH, 0, 0);
			}
			continue;
		}
		if ((m_pUserInfo[i] != NULL) && (m_pUserInfo[i]->m_UserData.bUserState != USER_CUT_GAME))
		{
			m_pUserInfo[i]->m_UserData.bUserState = USER_SITTING;
		}
	}	
	//发送大厅消息
	m_pDataManage->m_TCPSocket.SendDataBatch(NULL,0,MDM_GR_ROOM,ASS_GR_GAME_END,m_bDeskIndex);
	//清理断线资料
	if (!m_bIsBuy || GFF_DISSMISS_FINISH == bCloseFlag)
		CleanCutGameInfo();
	//清楚中途加入标志
	memset(m_bIsMidEnter,0,sizeof(m_bIsMidEnter));
	return true;
}


bool CGameDesk::GameFinishContest(BYTE bDeskStation, BYTE bCloseFlag)
{
	//设置数据
	m_bPlayGame=false;
	m_dwBeginTime=(long int)time(NULL);
	m_pDataManage->m_TCPSocket.SendDataBatch(NULL,0,MDM_GR_ROOM,ASS_GR_GAME_END,m_bDeskIndex);
	for (BYTE i=0;i<m_bMaxPeople;i++)
	{
		if (m_pUserInfo[i] == NULL)
		{
			continue;
		}
		if (m_pUserInfo[i]->m_UserData.i64ContestScore < m_pDataManage->m_InitData.i64LowChip)
		{//先清除要被淘汰的用户
			m_pDataManage->AutoAllotDeskDeleteUser(m_pUserInfo[i]);
		}
	}
	
	//根据目前在线人数判断比赛是否结束
	bool _bfinish = m_pDataManage->IsContestGameOver();

	//发送结束消息
	for (BYTE i=0;i<m_bMaxPeople;i++)
	{
		if (m_pUserInfo[i] == NULL)
		{
			continue;
		}
		if (m_pUserInfo[i]->m_UserData.i64ContestScore < (int)m_pDataManage->m_InitData.i64LowChip && !_bfinish)
		{
			m_pDataManage->m_TCPSocket.SendData(m_pUserInfo[i]->m_uSocketIndex, MDM_GR_ROOM, ASS_GR_CONTEST_KICK, m_bDeskIndex, 0);	
		}
		else if (!_bfinish)
		{
			MSG_GR_R_UserSit UserSit;
			UserSit.bLock=m_bLock;
			UserSit.bDeskIndex=m_bDeskIndex;
			UserSit.bDeskStation=bDeskStation;
			UserSit.bUserState=USER_LOOK_STATE;
			UserSit.dwUserID=m_pUserInfo[i]->m_UserData.dwUserID;
			UserSit.bIsDeskOwner=(m_dwOwnerUserID==m_pUserInfo[i]->m_UserData.dwUserID);
			UserSit.bLeave = false;
			UserSit.bDeskMaster=(m_bIsBuy&&m_iDeskMaster==m_pUserInfo[i]->m_UserData.dwUserID);//房主
			m_pDataManage->m_TCPSocket.SendDataBatch(&UserSit,sizeof(UserSit),MDM_GR_USER_ACTION,ASS_GR_USER_UP,ERR_GR_SIT_SUCCESS);
		}
		else if (_bfinish)
		{
			//比赛已经结束，但是有用户还没有结束这一局
			m_pDataManage->m_TCPSocket.SendData(m_pUserInfo[i]->m_uSocketIndex, MDM_GR_ROOM, ASS_GR_CONTEST_WAIT_GAMEOVER, m_bDeskIndex, 0);
		}
		//重置该桌子的状态
		m_pUserInfo[i]->m_UserData.bDeskNO=255;
		m_pUserInfo[i]->m_UserData.bDeskStation=255;
		m_pUserInfo[i]->m_UserData.bUserState=USER_LOOK_STATE;

		m_uCutTime[i]=0L;
		m_bConnect[i]=false;
		m_pUserInfo[i]=NULL;


	}
	//清理断线资料
	CleanCutGameInfoContest();

	return true;
}

//定时器消息
bool CGameDesk::OnTimer(UINT uTimerID)
{
	if (IDT_CHECK_DESK == uTimerID)
	{
		KillTimer(IDT_CHECK_DESK);
		m_pDataManage->OnReleaseDesk(m_bDeskIndex);
		//if(IsPlayingByGameStation())
		{
			GameFinish(0,GFF_FORCE_FINISH);
		}
		return true;
	}
	else if (IDT_DISMISS_TIMEOUT == uTimerID)
	{
		KillTimer(IDT_DISMISS_TIMEOUT);
		
		GameFinish(0,GFF_DISSMISS_FINISH);
		return true;
	}
	else if (IDT_CONTEST_SIT_TIMEOUT == uTimerID)
	{
		KillTimer(IDT_CONTEST_SIT_TIMEOUT);

		if (!(m_pDataManage->m_InitData.dwRoomRule & GRR_CONTEST) || (m_pDataManage->m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
		{
			return true;
		}
		return true;
	}
    else if(IDT_DESKRUNOUT_TIMER == uTimerID)
    {
		KillTimer(IDT_DESKRUNOUT_TIMER);
        if(m_iRunGameCount == 0 && !m_bPlayGame)
        {
            return true;
        }
        if(!m_bPlayGame)
        {
            GameFinish(0, GFF_DISSMISS_FINISH);
        }
        else
        {
            SetTimer(IDT_DESKRUNOUT_TIMER, 1000);
        }
        return true;
    }

	return false;
}

//游戏数据包处理函数
bool CGameDesk::HandleFrameMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	if (bDeskStation>=m_bMaxPeople|| m_pUserInfo[bDeskStation] == NULL)
	{
		return true;
	}
	ASSERT(pNetHead->bMainID==MDM_GM_GAME_FRAME);
	switch (pNetHead->bAssistantID)
	{
	case ASS_GM_GAME_INFO:		//游戏状态
		{
			//效验数据
			if (uSize!=sizeof(MSG_GM_S_ClientInfo))
			{
				HNLOG_G(m_pDataManage->m_InitData.uRoomID,"[%d-%d]CGameDesk::HandleFrameMessage(ASS_GM_GAME_INFO)中数据长度不正确[%d行]",
					this->m_pDataManage->m_KernelData.uNameID,
					this->m_pDataManage->m_InitData.uRoomID,
					__LINE__);
				return false;
			}
			MSG_GM_S_ClientInfo * pClientInfo=(MSG_GM_S_ClientInfo *)pData;
			if ((!bWatchUser) && (m_bConnect[bDeskStation]))
			{
				/// 当快速换桌的时候，会出现这种情况，为避免总断线的情况，返回true　
				HNLOG_G(m_pDataManage->m_InitData.uRoomID,"[%d-%d]CGameDesk::HandleFrameMessage(ASS_GM_GAME_INFO)中，连接过了再次连接[%d行]",
					this->m_pDataManage->m_KernelData.uNameID,
					this->m_pDataManage->m_InitData.uRoomID,
					__LINE__);
				return true;
			}

			//处理数据
			if (bWatchUser==false)
			{
				m_bConnect[bDeskStation]=true;
				if (((m_pDataManage->m_InitData.dwRoomRule&GRR_ENABLE_WATCH)==0)&&
					((m_pDataManage->m_InitData.dwRoomRule&GRR_UNENABLE_WATCH)==0))
				{
					if (pClientInfo->bEnableWatch) m_bEnableWatch|=(1<<bDeskStation);
					else m_bEnableWatch&=~(1<<bDeskStation);
				}
			}
			//向客户端发送视频服务器地址
			send_video_ip(bDeskStation);
			//发送用户游戏状态
			bool bRet = OnGetGameStation(bDeskStation,uSocketID,bWatchUser);

			//捕鱼比较特殊  暂时这样处理  
			for (int i = 0;i< 10;++i)
			{
				if (m_pDataManage->m_uNameID == m_dwFishGamesNameID[i])
				{
					m_dwBeginTime=(long int)time(NULL);
					break;
				}
			}
			if (!bRet)
			{
				HNLOG_G(m_pDataManage->m_InitData.uRoomID,"[%d-%d]CGameDesk::HandleFrameMessage(ASS_GM_GAME_INFO)中OnGetGameStation()返回false[%d行]",
					this->m_pDataManage->m_KernelData.uNameID,
					this->m_pDataManage->m_InitData.uRoomID,
					__LINE__);
			}
			return bRet;
		}
	case ASS_GM_FORCE_QUIT:		//强行退出//安全退出
		{
			if (bWatchUser==true) return true;
			if(pNetHead->bHandleCode==0)//强行退出
			{
				if (IsPlayGame(bDeskStation)==true) GameFinish(bDeskStation,GFF_FORCE_FINISH);
			}
			else if(pNetHead->bHandleCode==1)//安全退出
			{
				if (IsPlayGame(bDeskStation)==true) GameFinish(bDeskStation,GFF_SAFE_FINISH);
			}
			return true;
		}
	case ASS_GM_NORMAL_TALK:	//普通聊天
		{
			//不允许普通聊天
			if(m_pDataManage->m_InitData.dwRoomRule & GRR_FORBID_GAME_TALK)
				return false;
			//效验数据 
			MSG_GR_RS_NormalTalk * pNormalTalk=(MSG_GR_RS_NormalTalk *)pData;

			//处理数据
			CGameUserInfo * pUserInfo=m_pDataManage->m_UserManage.GetOnLineUserInfo(uSocketID);
			if (pUserInfo==NULL) return false;

			/// 如果禁言时间有效，则不发送  
			CString strTime;
			strTime.Format("%d",CTime::GetCurrentTime());
			int curTime=atoi(strTime);
//			if (pUserInfo->m_UserData.userInfoEx2 > curTime)
			{
//				return true;
			}

			//群发数据
			pNormalTalk->dwSendID=pUserInfo->m_UserData.dwUserID;

			string str_temp(pNormalTalk->szMessage);
			m_pDataManage->m_HNFilter.censor(str_temp,false);
			strcpy(pNormalTalk->szMessage,str_temp.c_str());
			pNormalTalk->iLength = strlen(pNormalTalk->szMessage) + 1;
			pNormalTalk->szMessage[pNormalTalk->iLength]=0;

			SendGameData(m_bMaxPeople,pNormalTalk,sizeof(MSG_GR_RS_NormalTalk),MDM_GM_GAME_FRAME,ASS_GM_NORMAL_TALK,0);
			SendWatchData(m_bMaxPeople,pNormalTalk,sizeof(MSG_GR_RS_NormalTalk),MDM_GM_GAME_FRAME,ASS_GM_NORMAL_TALK,0);

			return true;
		}
	case ASS_GM_WATCH_SET:		//旁观设置
		{
			//效验数据
			if (uSize!=sizeof(MSG_GM_WatchSet)) return false;
			MSG_GM_WatchSet * pWatchSet=(MSG_GM_WatchSet *)pData;
			if (bWatchUser==true) return false;

			//处理数据
			if (((m_pDataManage->m_InitData.dwRoomRule&GRR_ENABLE_WATCH)==0)&&
				((m_pDataManage->m_InitData.dwRoomRule&GRR_UNENABLE_WATCH)==0))
			{
				if (pWatchSet->dwUserID!=0L)
				{
					//单一用户
					for (INT_PTR i=0;i<m_WatchUserPtr.GetCount();i++)
					{
						CGameUserInfo * pUserInfo=(CGameUserInfo *)m_WatchUserPtr.GetAt(i);
						if (((pUserInfo!=NULL)&&(pUserInfo->m_UserData.dwUserID==pWatchSet->dwUserID))&&(pUserInfo->m_UserData.bDeskStation==bDeskStation))
						{
							m_pDataManage->m_TCPSocket.SendData(pUserInfo->m_uSocketIndex,MDM_GM_GAME_FRAME,ASS_GM_WATCH_SET,
								pNetHead->bHandleCode,pUserInfo->m_dwHandleID);
							break;
						}
					}
				}
				else 
				{
					//群发用户
					if (pNetHead->bHandleCode==TRUE) m_bEnableWatch|=(1<<bDeskStation);
					else m_bEnableWatch&=~(1<<bDeskStation);
					SendWatchData(bDeskStation,MDM_GM_GAME_FRAME,ASS_GM_WATCH_SET,pNetHead->bHandleCode);
				}

				//通知变化
				OnWatchSetChange(bDeskStation,pWatchSet->dwUserID,pNetHead->bHandleCode==TRUE);
			}

			return true;
		}
    case ASS_GM_USE_KICK_PROP:      // 使用踢人卡
        {
            // 效验数据 
            if(uSize != sizeof(MSG_GR_RS_KickProp)) return false;
            MSG_GR_RS_KickProp* pKickProp = (MSG_GR_RS_KickProp* )pData;

            CGameUserInfo * pUserInfo=m_pDataManage->m_UserManage.GetOnLineUserInfo(uSocketID);
            if (pUserInfo==NULL) return false;

            try
            {
                DL_GR_I_UseKickProp DU_KickProp;
                memset(&DU_KickProp, 0, sizeof(DU_KickProp));
                DU_KickProp.dwUserID = pKickProp->dwUserID;
                DU_KickProp.iPropID = pKickProp->iPropID;

                m_pDataManage->m_SQLDataManage.PushLine(&DU_KickProp.DataBaseHead,sizeof(DU_KickProp),DTK_GR_USE_KICK_PROP,
                                                        pUserInfo->m_uSocketIndex, pUserInfo->m_dwHandleID);
            }
            catch(...)
            {TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
            }

            return true;
        }
    case ASS_GM_USE_ANTI_KICK_PROP: // 使用防踢卡
        {
            // 效验数据 
            if(uSize != sizeof(MSG_GR_RS_KickProp)) return false;
            MSG_GR_RS_KickProp* pKickProp = (MSG_GR_RS_KickProp* )pData;

            CGameUserInfo * pUserInfo=m_pDataManage->m_UserManage.GetOnLineUserInfo(uSocketID);
            if (pUserInfo==NULL) return false;

            try
            {
                DL_GR_I_UseKickProp DU_KickProp;
                memset(&DU_KickProp, 0, sizeof(DU_KickProp));
                DU_KickProp.dwUserID = pKickProp->dwUserID;
                DU_KickProp.iPropID = pKickProp->iPropID;

                m_pDataManage->m_SQLDataManage.PushLine(&DU_KickProp.DataBaseHead,sizeof(DU_KickProp),DTK_GR_USE_ANTI_KICK_PROP,
                                                        pUserInfo->m_uSocketIndex, pUserInfo->m_dwHandleID);
            }
            catch(...)
            {TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
            }

            return true;
        }
	}
	return false;
}

//游戏数据包处理函数
bool CGameDesk::HandleNotifyMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	ASSERT(pNetHead->bMainID==MDM_GM_GAME_NOTIFY);
	switch (pNetHead->bAssistantID)
	{
	case ASS_GM_AGREE_GAME:		//用户同意
		{
			//效验数据
			if (bWatchUser==true)
				return false;
			return UserAgreeGame(bDeskStation);					
		}
	}
	return false;
}



//mark
//即时更新数据库
BOOL CGameDesk::UpdateUserDB(LONG dwUserID,int dwChangeMoney,int dwChangePoint)
{
	if(dwChangeMoney == 0 && dwChangePoint == 0)
		return TRUE;

	CGameUserInfo* UserInfo=m_pDataManage->m_UserManage.FindOnLineUser(dwUserID);

	if(UserInfo==NULL)
		UserInfo = m_pDataManage->m_UserManage.FindNetCutUser(dwUserID);

	if(UserInfo!=NULL)
	{
		//return TRUE;	
//		UserInfo->m_UserData.dwPoint += dwChangePoint ;
		//处理负金币情况
		if(UserInfo->m_UserData.i64Money + dwChangeMoney < 0)
		{
			dwChangeMoney = 0;
			UserInfo->m_UserData.i64Money = 0;
		}
		else
			UserInfo->m_UserData.i64Money += dwChangeMoney; 
	}
	//更新数据库
	try
	{
		DL_GR_Update_InstantMoney DT_InstantMoney;
		memset(&DT_InstantMoney,0,sizeof(DT_InstantMoney));
		DT_InstantMoney.dwChangeMoney = dwChangeMoney;
		DT_InstantMoney.dwChangePoint =dwChangePoint;
		DT_InstantMoney.dwUserID = dwUserID;
		DT_InstantMoney.bCutValue = 0;
		m_pDataManage->m_SQLDataManage.PushLine(&DT_InstantMoney.DataBaseHead,sizeof(DT_InstantMoney),DTK_GR_UPDATE_INSTANT_MONEY,0,0);
	}
	catch(...)
	{TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}
	//通知客户端
	MSG_GR_R_InstantUpdate InstantUpdate;
	InstantUpdate.dwUserID = dwUserID;
	InstantUpdate.dwMoney = dwChangeMoney;
	InstantUpdate.dwPoint = dwChangePoint;
	m_pDataManage->m_TCPSocket.SendDataBatch(&InstantUpdate,sizeof(InstantUpdate),MDM_GR_ROOM,ASS_GR_INSTANT_UPDATE,0);
	return true;
}

//道俱
bool CGameDesk::UserUseProp(BYTE bsendDeskStation,BYTE brecvDeskStation,_TAG_PROP_MOIVE_USE * propMoive)
{
	if(!m_pUserInfo[bsendDeskStation])
		return true;

	if(!m_pUserInfo[brecvDeskStation])
		return true;
	
	for(int i = 0; i < m_bMaxPeople; i ++)
		SendGameData(i,propMoive,sizeof(_TAG_PROP_MOIVE_USE),MDM_GM_GAME_NOTIFY,ASS_PROP,0);
	SendWatchData(m_bMaxPeople,propMoive,sizeof(_TAG_PROP_MOIVE_USE),MDM_GM_GAME_NOTIFY,ASS_PROP,0);
	return true;
}

//rongqiufen
//修改指定用户的道具信息
int CGameDesk::PropChange(const TPropChange& propChange)
{
	TpropChange_I _p;
	_p.info = propChange;

	m_pDataManage->m_SQLDataManage.PushLine(&_p.DataBaseHead, sizeof(TpropChange_I), DTK_GR_PROP_CHANGE, 0, 0);

	return 0;
}

//更新奖池数据(游戏->平台)
/// @param iAIHaveWinMoney 机器人赢钱数
/// @return void
void CGameDesk::UpDataRoomPond(__int64 iAIHaveWinMoney)
{
	m_pDataManage->SendRoomPondUpData(iAIHaveWinMoney);
	return;
}

//获取 读取配置文件时所用的关键字
void CGameDesk::GetGRM_SET_KEY(TCHAR key[])
{
	if (key == NULL)
	{
		return;
	}

	CopyMemory(m_GRM_Key,key,sizeof(m_GRM_Key));
	return;
}

//获取 读取配置文件时所用的关键字(胜率)
void CGameDesk::GetGRM_SET_KEY_Win(TCHAR key[])
{
	if (key == NULL)
	{
		return;
	}

	CopyMemory(m_GRM_Key_win,key,sizeof(m_GRM_Key_win));
	return;
}

//获取 读取配置文件时所用的关键字(输率)
void CGameDesk::GetGRM_SET_KEY_Los(TCHAR key[])
{
	if (key == NULL)
	{
		return;
	}

	CopyMemory(m_GRM_Key_los,key,sizeof(m_GRM_Key_los));
	return;
}

//发送解散申请消息
void CGameDesk::SendDissmissingData(int SocketID)
{
	if (m_bInDissmissing && m_iDissmissUserID!=0 && DESK_TYPE_BJL!=m_uDeskType)
	{
		CGameUserInfo * pUserInfo=m_pDataManage->m_UserManage.GetOnLineUserInfo(SocketID);
		auto ite = find(m_VecAgreeUserID.begin(),m_VecAgreeUserID.end(),pUserInfo->m_UserData.dwUserID);
		if (ite == m_VecAgreeUserID.end() && 1==M_bMidEnter)	//中途加入的玩家没有同意或拒绝解散的权限
		{
			m_iDissmissAgreePeople++;
		}

		NetCutDismissNotify desk_notify;
		desk_notify.iUserID=m_iDissmissUserID;
		desk_notify.iAgreeNum = m_iDissmissAgreePeople;

		int i = 0;
		for(vector<int>::iterator it = m_VecAgreeUserID.begin();
			it != m_VecAgreeUserID.end() && i<20;
			it++)
		{
			desk_notify.iUserAgreeID[i] = *it;
			i++;
		}

		COleDateTime nowTime = COleDateTime::GetCurrentTime();
		COleDateTimeSpan tmDiff = (nowTime - m_DismissTime);
		long lsec = (long)tmDiff.GetTotalSeconds();
		desk_notify.iTimeCount = (TIME_DISMISS_TIMEOUT/1000 - lsec);

		m_pDataManage->m_TCPSocket.SendData(SocketID,&desk_notify,sizeof(desk_notify),MDM_GR_DESKRUNOUT,ASS_GR_DISSMISS_NETCUT,0,0);
	}
	
	return;
}

void CGameDesk::CreateGameSN()
{
	DL_GR_I_CREATE_GAMESN _p;
	_p.iDeskID = m_bDeskIndex;
	_p.iRoomID=m_pDataManage->m_InitData.uRoomID;

	m_pDataManage->m_SQLDataManage.PushLine(&_p.DataBaseHead, sizeof(DL_GR_I_CREATE_GAMESN), DTK_GR_CREATE_GAMESN, 0, 0);

	return;
}

void CGameDesk::ReleaseRoom(int UserID,void* szUserGameInfo,int iSize)
{
	DL_GR_I_RECORD_GAMEINFO _p;
	_p.iUserID=UserID;
	_p.iSize = iSize;
	memcpy_s(_p.GameInfo,sizeof(_p.GameInfo),szUserGameInfo,iSize);
	memcpy_s(_p.szPass,sizeof(_p.szPass),m_szDeskPassWord,sizeof(m_szDeskPassWord));

	m_pDataManage->m_SQLDataManage.PushLine(&_p.DataBaseHead, sizeof(DL_GR_I_RECORD_GAMEINFO), DTK_GR_RELEASERECORD, 0, 0);

	return;
}
void CGameDesk::SendReturnInfo(UINT uIndex)
{
	if ((m_pDataManage->m_InitData.dwRoomRule & GRR_GAME_BUY) && m_bReturnDesk&&3!=m_pDataManage->m_InitData.iLockType)    //发送房间数据
	{
		vector<MSG_GM_S_ReturnGameInfo>::iterator iterReturnInfo = m_pReturnInfo.begin();
		for (iterReturnInfo; iterReturnInfo != m_pReturnInfo.end(); iterReturnInfo++)
		{
			if (iterReturnInfo->iUserID != 0)
			{
				CGameUserInfo * pGameUserInfo=m_pDataManage->m_UserManage.FindOnLineUser(iterReturnInfo->iUserID);
				if (pGameUserInfo!=NULL)
				{
					continue;
				}
				else
				{
					pGameUserInfo=m_pDataManage->m_UserManage.FindNetCutUser(iterReturnInfo->iUserID);
					if (pGameUserInfo!=NULL)
					{
						iterReturnInfo->iUserState = 2;
						if(m_pDataManage->m_InitData.uComType==TY_MONEY_GAME && (m_pDataManage->m_InitData.dwRoomRule & GRR_EXPERCISE_ROOM))
						{
							pGameUserInfo->m_UserData.i64Money = iterReturnInfo->i64Score;
						}
					}
					else
					{
						iterReturnInfo->iUserState = 3;
					}
				}
				iterReturnInfo->bDeskID = m_bDeskIndex;
				m_pDataManage->m_TCPSocket.SendData(uIndex,&(*iterReturnInfo),sizeof(MSG_GM_S_ReturnGameInfo),MDM_GR_RETURNDESK,ASS_GR_RETURNDESK,0,0);
			}
		}
	}

}

bool CGameDesk::IsBuyDeskPlaying()
{
	return (m_iRunGameCount > 0 || IsPlayGame(0));
}

void CGameDesk::DismissDeskbyGame()
{
	if (m_bFinishCondition == 0)
	{
		return;
	}

	m_pDataManage->OnClearDesk(m_bDeskIndex);

}

int CGameDesk::SavePlayBackFile()
{
	char ExistFile[256];
	char NewFile[256];

	for (int i=0;i<m_bMaxPeople;i++)
	{
		sprintf_s(ExistFile,"%s%s%02d.txt",m_pDataManage->m_InitData.szPlayBackTemp,m_GameSN,i);
		sprintf_s(NewFile,"%s%s%02d.txt",m_pDataManage->m_szPlayBackURL,m_GameSN,i);

		if (!MoveFileA(ExistFile, NewFile))  
		{  
			DWORD getlasterror;  
			getlasterror=GetLastError();
		}  
	}

	return 1;
}

void CGameDesk::UpdateRunCount(int iPlayCount) 
{ 
	m_iRunGameCount = iPlayCount;
}

void CGameDesk::onCheckPrevent()
{
	onCheckIP_Location();
	if ((m_bPositionCheck||m_bIPCheck) && m_pDistance.size()>0)
	{
		m_bInLocation = true;
		m_iLocationAgreeNum = 0;
		m_bAllAgreeLocation = false;
		char bBuffer[MAX_SEND_SIZE];
		memset(bBuffer,0,sizeof(bBuffer));
		int count = 0;
		while(count<m_pDistance.size())
		{
			if ((count+1)*sizeof(MSG_GR_S_Position_Notice)>MAX_SEND_SIZE)
			{
				break;
			}
			memcpy_s(bBuffer+count*sizeof(MSG_GR_S_Position_Notice),sizeof(MSG_GR_S_Position_Notice),&m_pDistance[count],sizeof(MSG_GR_S_Position_Notice));
			count++;
		}

		for (int i=0;i<m_bMaxPeople;i++)
		{
			if (m_pUserInfo[i] == nullptr) continue;

			m_pDataManage->m_TCPSocket.SendData(m_pUserInfo[i]->GetSocketIndex(), bBuffer, count*sizeof(MSG_GR_S_Position_Notice),MDM_GR_DESKRUNOUT,ASS_GR_GETDISTANCEINFO, 0, 0);
		}
	}
	else
	{
		m_bInLocation = false;
		m_iLocationAgreeNum = 0;
		m_bAllAgreeLocation = true;
	}

	if (m_bAllAgreeLocation)
	{
		if (!m_pDataManage->IsQueueGameRoom() && CanBeginGame()==true) 
		{
			GameBegin(0);
			return;
		}
	}
}

void CGameDesk::onCheckGameBegin()
{
	if (m_bAllAgreeLocation)
	{
		for(int i=0;i<m_bMaxPeople;i++)
		{
			if(!m_pUserInfo[i])
			{
				continue;
			}
			m_pDataManage->m_TCPSocket.SendData(m_pUserInfo[i]->m_uSocketIndex,MDM_GR_DESKRUNOUT,ASS_GR_DISTANCE_ALLAGREE,0,0);
		}
		
		if (!m_pDataManage->IsQueueGameRoom() && CanBeginGame()==true) 
		{
			GameBegin(0);
			return;
		}
	}
}

void CGameDesk::onCheckIP_Location()
{
	if (m_bIsBuy && (m_bPositionCheck || m_bIPCheck))	
	{
		int temp_distance = 0;
		m_pDistance.clear();
		for (BYTE i=0;i<m_bMaxPeople-1;i++)
		{
			if (m_pUserInfo[i]==NULL) continue;
			for (BYTE j=(i+1);j<m_bMaxPeople;j++)
			{
				if (m_pUserInfo[j]==NULL) continue;
				if (m_pUserInfo[i]->m_UserData.dwUserID == m_pUserInfo[j]->m_UserData.dwUserID) continue;

				MSG_GR_S_Position_Notice DistanceInfo;
				DistanceInfo.iUserIDA = m_pUserInfo[i]->m_UserData.dwUserID;
				DistanceInfo.iUserIDB = m_pUserInfo[j]->m_UserData.dwUserID;

				if (m_bPositionCheck)	//加入过近玩家信息
				{
					temp_distance = m_pUserInfo[i]->GetDistance(m_pUserInfo[j]->m_UserData.flat,m_pUserInfo[j]->m_UserData.flnt);
					if (temp_distance < m_pDataManage->m_InitData.iPositionLimit)
					{
						DistanceInfo.iDistance = temp_distance;
						DistanceInfo.bDistance = true;
					}
					else
					{
						DistanceInfo.iDistance = 0;
						DistanceInfo.bDistance = false;
					}
				}
				else
				{
					DistanceInfo.iDistance = 0;
					DistanceInfo.bDistance = false;
				}

				if (m_bIPCheck)		//加入同IP玩家信息
				{
					if (m_pUserInfo[i]->m_UserData.dwUserIP == m_pUserInfo[j]->m_UserData.dwUserIP)
					{
						DistanceInfo.bSameIP = true;
					}
					else
					{
						DistanceInfo.bSameIP = false;
					}
				}
				else
				{
					DistanceInfo.bSameIP = false;
				}

				if(DistanceInfo.bSameIP || DistanceInfo.bDistance)
				{
					m_pDistance.push_back(DistanceInfo);
				}
			}
		}
	}
}

void CGameDesk::UserNotAgreePrevent(int UserID)
{
	if (m_bIsBuy && m_bInLocation)
	{
		m_bInLocation = false;
		m_iLocationAgreeNum=0;
		m_bAllAgreeLocation = false;

		VipDeskDismissAgreeRes tAgreeData;
		tAgreeData.bAgree=false;
		tAgreeData.iUserID=UserID;
		for(int i=0;i<m_bMaxPeople;i++)
		{
			if(!m_pUserInfo[i])
			{
				continue;
			}
			m_pDataManage->m_TCPSocket.SendData(m_pUserInfo[i]->m_uSocketIndex,&tAgreeData,sizeof(tAgreeData),MDM_GR_DESKRUNOUT,ASS_GR_DISTANCE_AGREE,0,0);
		}
	}
}


void CGameDesk::SendDeskUserInfo(int iSocketID)
{
	char bBuffer[MAX_SEND_SIZE];
	int count = 0;
	int iMaxCount = MAX_SEND_SIZE/sizeof(UserInfoStruct);

	for (BYTE i=0;i<m_bMaxPeople;i++)
	{
		if (!m_pUserInfo[i])
		{
			continue;
		}

		if (count >= iMaxCount)
		{
			m_pDataManage->m_TCPSocket.SendData(iSocketID,bBuffer,count*sizeof(UserInfoStruct),MDM_GR_USER_LIST,ASS_GR_ONLINE_USER,ERR_GR_LIST_PART,0);
			count = 0;
			memset(bBuffer,0,sizeof(bBuffer));
		}

		if (m_bCutGame[i])
		{
			m_pUserInfo[i]->m_UserData.bUserState = USER_CUT_GAME;
		}

		memcpy_s(bBuffer+count*sizeof(UserInfoStruct),sizeof(UserInfoStruct),&m_pUserInfo[i]->m_UserData,sizeof(UserInfoStruct));

		count++;
	}
	m_pDataManage->m_TCPSocket.SendData(iSocketID,bBuffer,count*sizeof(UserInfoStruct),MDM_GR_USER_LIST,ASS_GR_ONLINE_USER,ERR_GR_LIST_FINISH,0);
}

void CGameDesk::PlayerNetCutContest(BYTE bDeskStation)
{
	if (!IsPlayGame(0))
		UserAgreeGame(bDeskStation);

	return ;
}


bool CGameDesk::ChangeStation(BYTE bDeskStationA,BYTE bDeskStationB)
{
	if (!m_pUserInfo[bDeskStationA] || !m_pUserInfo[bDeskStationB])
	{
		return false;
	}

	CGameUserInfo *pUserInfoTemp = m_pUserInfo[bDeskStationA];		//交换AB玩家桌子上的信息
	m_pUserInfo[bDeskStationA] = m_pUserInfo[bDeskStationB];
	m_pUserInfo[bDeskStationB] = pUserInfoTemp;

	bool bCutGame = m_bCutGame[bDeskStationA];						//交换断线信息
	m_bCutGame[bDeskStationA] = m_bCutGame[bDeskStationB];
	m_bCutGame[bDeskStationB] = bCutGame;

	long uCutTime = m_uCutTime[bDeskStationA];						//交换断线时间
	m_uCutTime[bDeskStationA] = m_uCutTime[bDeskStationB];
	m_uCutTime[bDeskStationB] = uCutTime;

	bool bConnect = m_bConnect[bDeskStationA];						//交换连接信息
	m_bConnect[bDeskStationA] = m_bConnect[bDeskStationB];
	m_bConnect[bDeskStationB] = bConnect;

	BYTE bBreakCount = m_bBreakCount[bDeskStationA];
	m_bBreakCount[bDeskStationA] = m_bBreakCount[bDeskStationB];
	m_bBreakCount[bDeskStationB] = bBreakCount;

	m_GameUserInfo[bDeskStationA] = m_pUserInfo[bDeskStationA]->m_UserData.dwUserID;
	m_GameUserInfo[bDeskStationB] = m_pUserInfo[bDeskStationB]->m_UserData.dwUserID;

	m_dwGameUserID[bDeskStationA] = m_pUserInfo[bDeskStationA]->m_UserData.dwUserID;
	m_dwGameUserID[bDeskStationB] = m_pUserInfo[bDeskStationB]->m_UserData.dwUserID;

	CGameUserInfo *pUserInfo = m_pDataManage->m_UserManage.FindOnLineUser(m_pUserInfo[bDeskStationA]->m_UserData.dwUserID);	//修改A的座位号
	if (pUserInfo)
	{
		pUserInfo->m_UserData.bDeskStation=bDeskStationA;
	}
	else
	{
		pUserInfo = m_pDataManage->m_UserManage.FindNetCutUser(m_pUserInfo[bDeskStationA]->m_UserData.dwUserID);
		if (pUserInfo)
		{
			pUserInfo->m_UserData.bDeskStation=bDeskStationA;
		}
	}

	pUserInfo = nullptr;
	pUserInfo = m_pDataManage->m_UserManage.FindOnLineUser(m_pUserInfo[bDeskStationB]->m_UserData.dwUserID);  //修改B的座位号
	if (pUserInfo)
	{
		pUserInfo->m_UserData.bDeskStation=bDeskStationB;
	}
	else
	{
		pUserInfo = m_pDataManage->m_UserManage.FindNetCutUser(m_pUserInfo[bDeskStationB]->m_UserData.dwUserID);
		if (pUserInfo)
		{
			pUserInfo->m_UserData.bDeskStation=bDeskStationB;
		}
	}

	MSG_GR_O_ChangeUserStation   _out;
	_out.bDeskID = m_bDeskIndex;
	_out.bUserANewStation = bDeskStationA;
	_out.iUserIDA = m_pUserInfo[bDeskStationA]->m_UserData.dwUserID;
	_out.bUserBNewStation = bDeskStationB;
	_out.iUserIDB = m_pUserInfo[bDeskStationB]->m_UserData.dwUserID;

	m_pDataManage->m_TCPSocket.SendDataBatch(&_out,sizeof(MSG_GR_O_ChangeUserStation),MDM_GR_ROOM,ASS_GR_CHANGE_USERSTATION,0);

	//
	if (m_bIsBuy)
	{
		DL_GR_I_ChangeUserStation _p;
		strcpy_s(_p.szDeskPass,m_szDeskPassWord);
		memcpy_s(&_p._data,sizeof(_p._data),&_out,sizeof(_out));

		m_pDataManage->m_SQLDataManage.PushLine(&_p.DataBaseHead, sizeof(DL_GR_I_ChangeUserStation), DTK_GR_CHANGE_USERSTATION, 0, 0);
	}
	
	return true;
}

void CGameDesk::SetDynamicInfo(BYTE szDynamicInfo1[128])
{
    if(!m_pDataManage->m_bSetDynamiced)
    {
        m_pDataManage->m_bSetDynamiced = true;
        memcpy(m_pDataManage->m_szDynamicPlayingMethod1, szDynamicInfo1, sizeof(m_pDataManage->m_szDynamicPlayingMethod1));
    }
}

void CGameDesk::SetSPGameInfo(BYTE bDeskInfo[128])
{
    if(!m_pDataManage->m_bSetRoomDynamiced)
    {
        m_pDataManage->m_bSetRoomDynamiced = true;
        memcpy(m_pDataManage->m_szDynaminRoomInfo, bDeskInfo, sizeof(m_pDataManage->m_szDynaminRoomInfo));
    }
}


void CGameDesk::setEffectivebet()
{
	test.Connect();

	test.setString("linux", "fuckVS2010");
	test.disConnect();

}