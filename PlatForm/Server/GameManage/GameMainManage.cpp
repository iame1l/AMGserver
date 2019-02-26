/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "StdAfx.h"
#include "GameMainManage.h"
#include "GameRoomMessage.h"
#include "AFCException.h"
#include "AutoAllotDesk.h"

//宏定义

//GameDesk定时器
#define IDT_CHECK_DESK				51L										//检测是否有人定时器 ID
#define IDT_DISMISS_TIMEOUT			52L										//申请解散超时，自动解散
#define IDT_CONTEST_SIT_TIMEOUT		53L										//排队坐下等待超时
#define IDT_DESKRUNOUT_TIMER        54L                                     //时效房间解散定时器
#define TIME_DISMISS_TIMEOUT		2*60*1000L								//申请解散超时时间（毫秒）

#define IDT_WRITE_SERVER_INFO						7					//更新服务器信息定时器
#define IDT_CHECK_DATA_CONNECT						2					//检测数据连接定时器
#define IDT_UPDATE_ROOM_PEOPLE						3					//强制更新房间用户在线人数

#define IDT_QUEUE_GAME								4					/// 排队机定时器
#define IDT_CONTEST_GAMEOVER						5
#define IDT_LEASE_TIMEOVER							6					///租赁房间

#define IDT_CONNECT_OTHERSERVER_SOCKET				8					//维护与其他服务器连接的计时器
#define IDT_SET_DYNAMIC                             9
#define IDT_SET_ROOMINFO                            10

#define IDT_TIMING_CONTEST_BEGIN					20					///定时赛 开始ID
#define IDT_TIMING_CONTEST_END						21					///定时赛 结束ID
#define IDT_TIMING_CONTEST_INFO						22					///定时赛，开赛前通知定时器
#define IDT_TIMING_CONTEST_INFO_END					30					///

#define IDT_CONTEST_KICK							1000

#define VIP_CHECK_TIME								60000				//1分钟检测一次
#define EXPERCISE_MONEY						5000000						//金币训练场给钱数

///用于比赛场标志是否重新设置定时配桌，作为全局变量是避免游戏重新编译
static int	m_iResetTime = 0;

/*******************************************************************************************************/

//注册用户登陆
//构造函数
CGameMainManage::CGameMainManage(void)
{
    m_bSetRoomDynamiced = false;
    m_bSetDynamiced = false;
    memset(m_szDynamicPlayingMethod1, 0 ,sizeof(m_szDynamicPlayingMethod1));
    memset(m_szDynaminRoomInfo, 0 ,sizeof(m_szDynaminRoomInfo));
	m_bStopLogon=false;
	m_pDesk=NULL;
	m_uDeskCount=0;
	m_uMatchUserCount=0;
	m_pMatchUser=NULL;
	m_pMatchInfo=NULL;
	m_pDeskArray=NULL;
	m_szMsgRoomLogon[0]=0;
	m_szMsgGameLogon[0]=0;
//	m_ItemDate = NULL;
	m_PropService=NULL;
	m_pBankService = NULL;
	m_pHandleMSMessage = NULL;
	m_lVIPID = -1;  ///< 设置房间密码的ID
	m_uVIPIndex = 0;
	memset(m_szVIPPassWord,0,sizeof(m_szVIPPassWord));
	memset(m_szGameName,0,sizeof(m_szGameName));
	
	CBcfFile f(CBcfFile::GetAppPath() + "Function.bcf");
	CString strValue = f.GetKeyVal("TaxRate","Rate","100");
	m_nRate = atoi(strValue);
	strValue = f.GetKeyVal("PresentCoinPerDay","Available","0");
	m_bPresentCoin = (atoi(strValue)!=0);

	m_pIAutoAllotDesk = NULL;
	m_iVIP_Rate = 1.0;
	m_uNameID = 0;

	m_ContestTimeover = false;
}

//析构函数	
CGameMainManage::~CGameMainManage(void)
{
	m_uDeskCount=0;
	SafeDeleteArray(m_pDesk);
	SafeDeleteArray(m_pMatchUser);
	SafeDeleteArray(m_pMatchInfo);
	SafeDeleteArray(m_pDeskArray);
	//if(m_ItemDate) 
	//	delete m_ItemDate;
	if(m_PropService)
		delete m_PropService;
	if (m_pBankService)
		delete m_pBankService;
	if (m_pHandleMSMessage)
		delete m_pHandleMSMessage;
	m_pGameUserInfoList.clear();
	m_GRMUserID.clear();
	m_GRMRoomID.clear();
}

//数据管理模块初始化
bool CGameMainManage::OnInit(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData)
{
	try
	{
		//初始化用户和游戏桌子
		m_uMatchUserCount=0;
		m_szMsgRoomLogon[0]=0;
		m_szMsgGameLogon[0]=0;
		m_RoomLimited.RemoveAll();
		m_GameLimited.RemoveAll();
		lstrcpy(m_szMsgRoomLogon,TEXT("欢迎你，祝你能在这里玩得开心，快乐从这里开始！"));
		lstrcpy(m_szMsgGameLogon,TEXT("欢迎你，祝你能在这里玩得开心，快乐从这里开始！"));

		m_pMainServerSocket = new CMainServerSocket(this);

		//m_ItemDate=new CAFCItemData((CWnd*)this);		
		m_PropService=new CPropService((CWnd*)this);
		m_pBankService = new CBankService((CWnd*)this);
		m_pHandleMSMessage = new CHandleMSMessage((CWnd*)this);
		m_uNameID = pKernelData->uNameID;
		if (m_UserManage.Init(pInitData->uMaxPeople,pInitData->uMaxPeople,pInitData->uVipReseverCount)==false) return false;
		if (InitGameDesk(pInitData->uDeskCount, pInitData->uDeskType)==false) return false;//百家乐lym

		if (m_KernelData.uDeskPeople != 0)
		{
			for(int i=0; m_InitData.uVirtualUser,i<(m_InitData.uVirtualUser/m_KernelData.uDeskPeople); i++) ///< 启动W服务器后让虚拟玩家坐下桌子。
			{
				CGameDesk * pDesk = *(m_pDesk + i);
				pDesk->VirtualLockDesk();
			}
		}

		
		//pKernelData->uMinDeskPeople = pKernelData->uDeskPeople;
		
		if (m_InitData.uMinDeskPeople > 0)
		{
			pKernelData->uMinDeskPeople = m_InitData.uMinDeskPeople;
		}
		else
		{
			pKernelData->uMinDeskPeople = pKernelData->uDeskPeople;
		}
		
		CBcfFile f(CBcfFile::GetAppPath()+"SpecialRule.bcf");
		m_iQueueTime = f.GetKeyVal("Queuer","QueuerTime", 10);

		////先查找是否存在该房间，存在则跳过，否则添加
		/*if (!IsExistGRMRoom(pKernelData->uNameID))
		{
			CBcfFile f(CBcfFile::GetAppPath()+"GRMInfo.bcf");
			bool bRoomGRM = f.GetKeyVal("ManageGameID",_NameID, 0);
			if (bRoomGRM)//只有确认有效才添加
			{
				m_GRMRoomID.push_back(pKernelData->uNameID);
			}
		}*/

		if (IsQueueGameRoom())
		{
			m_pIAutoAllotDesk = CreateAutoAllotDesk(m_KernelData.uDeskPeople, m_InitData.uDeskCount);
			if (m_InitData.dwRoomRule &GRR_CONTEST || m_InitData.dwRoomRule &GRR_TIMINGCONTEST)
			{//少于此人数时，无需排队
				m_pIAutoAllotDesk->SetMinQueuePeople(m_InitData.iLeasePeople);
			}
			m_pUserQueueBuffer = new MSG_GR_S_UserQueueInfo[m_KernelData.uDeskPeople];
		}

		return true;
	}
	catch (CAFCException * pException)	{ pException->Delete();TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);	}
	catch (...) {TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);}
	return false;
}

//数据管理模块卸载
bool CGameMainManage::OnUnInit()
{
		/// 如果是自动撮桌的房间，额外创建的对象需要释放
	if (IsQueueGameRoom())
	{
		SafeDeleteArray(m_pUserQueueBuffer);
		DestroyAutoAllotDesk(m_pIAutoAllotDesk);
	}

	for(int i=0;i<m_UserManage.GetOnLineUserCount();i++)
	{
		CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(i);
		if (pUserInfo!=NULL) 
		{
			CleanUserInfo(pUserInfo);
		}

	}

	if (m_pMainServerSocket)
	{
		delete m_pMainServerSocket;
		m_pMainServerSocket = nullptr;
		HNLOG_G(m_InitData.uRoomID,"OnUnInit :: %d",m_InitData.uRoomID);
	}

	//设置数据
	m_uDeskCount=0;
	m_uMatchUserCount=0;
	m_szMsgRoomLogon[0]=0;
	m_szMsgGameLogon[0]=0;
	SafeDeleteArray(m_pDesk);
	SafeDeleteArray(m_pMatchInfo);
	SafeDeleteArray(m_pDeskArray);
	SafeDeleteArray(m_pMatchUser);
	m_UserManage.UnInit();
	m_RoomLimited.RemoveAll();
	m_GameLimited.RemoveAll();

	return true;
}

//数据管理模块启动
bool CGameMainManage::OnStart()
{
	SetTimer(IDT_WRITE_SERVER_INFO,60000L);
	SetTimer(IDT_CHECK_DATA_CONNECT,60000L);
	SetTimer(IDT_CONNECT_OTHERSERVER_SOCKET, 5000L);

	//是排队机房间，设置排队机定时器
	if (IsQueueGameRoom() && !(m_InitData.dwRoomRule & GRR_CONTEST) && !(m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
	{
		SetTimer(IDT_QUEUE_GAME, m_iQueueTime * 1000);
		m_InitData.iRoomState = 1;
	}

	if (m_InitData.iLeaseID != 0)
	{
		SetTimer(IDT_LEASE_TIMEOVER, 10000);
	}
	
	m_SQLDataManage.m_pEngineSink->closeRecord();

	//获取房间信息
	DataBaseLineHead GetRoomInfo;
	m_SQLDataManage.PushLine(&GetRoomInfo,sizeof(GetRoomInfo),DTK_GR_GET_ROOMINFO,0,0);

	//获取随机IP列表
	DataBaseLineHead GetRandIPList;
	m_SQLDataManage.PushLine(&GetRandIPList, sizeof(GetRandIPList), DTK_ROBOT_RAND_IP_LIST, 0,0);

	//获取比赛信息
	if (m_InitData.dwRoomRule & GRR_TIMINGCONTEST || m_InitData.dwRoomRule & GRR_CONTEST)
	{
		DataBaseLineHead GetRoomInfo;
		m_SQLDataManage.PushLine(&GetRoomInfo,sizeof(GetRoomInfo),DTK_GR_GET_CONTESTINFO,0,0);
	}

	//更新转账返利数据
	DL_GR_I_RebateUpDateStruct Rebate;
	m_SQLDataManage.PushLine(&Rebate.DataBaseHead,sizeof(Rebate),DTK_GR_BANK_UPDATE_REBATE,0,0);

	DataBaseLineHead _Emptey;
	m_SQLDataManage.PushLine(&_Emptey,sizeof(_Emptey),DTK_GR_CLEAR_DATA,0,0);

	if (m_InitData.dwRoomRule & GRR_GAME_BUY)
	{
		UpdateDeskInfo();
	}
	
	m_InitData.iPositionLimit = 100;

	CBcfFile f(CBcfFile::GetAppPath() + "HNGameGate.bcf");
	CString PlayBackTemp = f.GetKeyVal("GateServer","PlayBackTemp","");
	CString PlayBackURL = f.GetKeyVal("GateServer","PlayBackDownload","");

	strcpy_s(m_InitData.szPlayBackTemp,PlayBackTemp.GetBuffer());
	strcpy_s(m_szPlayBackURL,PlayBackURL.GetBuffer());

	PlayBackTemp.ReleaseBuffer();
	PlayBackURL.ReleaseBuffer();

    SetTimer(IDT_SET_DYNAMIC, 2000);
    SetTimer(IDT_SET_ROOMINFO, 2000);
	return true;
}

bool CGameMainManage::StopTimer()
{
	//删除定时器
	KillTimer(IDT_WRITE_SERVER_INFO);
	KillTimer(IDT_CHECK_DATA_CONNECT);
	KillTimer(IDT_QUEUE_GAME);
	KillTimer(IDT_CONNECT_OTHERSERVER_SOCKET);

	return true;
}

//数据管理模块关闭
bool CGameMainManage::OnStop()
{
	//删除定时器
	KillTimer(IDT_WRITE_SERVER_INFO);
	KillTimer(IDT_CHECK_DATA_CONNECT);
	KillTimer(IDT_QUEUE_GAME);
	KillTimer(IDT_CONNECT_OTHERSERVER_SOCKET);
	HNLOG_G(m_InitData.uRoomID,"OnStop :: %d",m_InitData.uRoomID);

	for (UINT i=0;i<m_uDeskCount;i++)
	{
		if (*(m_pDesk+i))
		{
			(*(m_pDesk+i))->GameFinish(0,GFF_SAFE_FINISH);
		}
	}

	return true;
}

void CGameMainManage::UpdateDeskInfo()
{
	DL_GR_I_FindDeskBuyStation _p;
	_p.iRoomID=m_InitData.uRoomID;

	m_SQLDataManage.PushLine(&_p.DataBaseHead, sizeof(DL_GR_I_FindDeskBuyStation), DTK_GR_FINDDESKBUYSTATION, 0, 0);

	return;
}

/// GM消息处理模块
bool CGameMainManage::OnGMSocketMessage(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	if (pNetHead->bAssistantID == ASS_GM_MESSAGE_PRINTLOG) //打印玩家日志
	{
		GM_MSG_PrintLog_t* pPrintLogCmd = (GM_MSG_PrintLog_t*)pData;
		if (m_InitData.uRoomID == pPrintLogCmd->uRoomID)
		{
			PrintUsersLog();
		}
		return true;
	}

	if(pNetHead->bAssistantID != ASS_GM_MESSAGE_OPERATE_ACT)
	{
		return true;
	}
	//效验数据
	if ((NULL == pData)||(uSize!=sizeof(GM_OperateByID_t))) return false;///验证结构是否一致

	/// 验证操作者权限
	CGameUserInfo * pOperator=m_UserManage.GetOnLineUserInfo(uIndex);
	if ((NULL == pOperator) /*|| (pOperator->m_UserData.bGameMaster==0)*/)
	{
		return false;
	}

	//处理
	GM_OperateByID_t * pOperation=(GM_OperateByID_t *)pData;

	/// 找到用户
	CGameUserInfo *pUserInfo = this->m_UserManage.FindOnLineUser(pOperation->iUserID);
	if (NULL == pUserInfo)
	{
		pUserInfo = this->m_UserManage.FindNetCutUser(pOperation->iUserID);
		if (NULL == pUserInfo)
		{
			return true;
		}
	}
	/// 解除操作在此直接返回
	if (pOperation->iWorkMinutes == 0)
	{
//		pUserInfo->m_UserData.userInfoEx2 = 0;
		return true;
	}
	switch(pOperation->iOperation)
	{
	case GM_ACT_AWORD:		//1	///< 奖励，奖励金额保存到银行中，不实时写入用户钱包里，本条消息发送到Z服务器
		break;
	case GM_ACT_NO_TALK:	//2	///< 禁言，被操作的玩若当前在某房间中，GM管理工具客户端须同时向该玩家所在的W服务器发本条消息
		{
			/// 执行禁言
			CTime tm = CTime::GetCurrentTime();
			tm += pOperation->iWorkMinutes*60;
			CString strTime;
			strTime.Format("%d", tm);
//			pUserInfo->m_UserData.userInfoEx2 = atoi(strTime);
			break;
		}
	case GM_ACT_KICK_OUT:	//3	///< 踢出房间，被操作的玩若当前在某房间中，GM管理工具客户端须同时向该玩家所在的W服务器发本条消息
	case GM_ACT_BANDON_ID:	//4	///< 禁止登录，即封号，被操作的玩若当前在某房间中，GM管理工具客户端同时向该玩家所在的W服务器发本条消息
		{
			bool bAlreadyCut = false;
			CGameUserInfo *pUserInfo = this->m_UserManage.FindOnLineUser(pOperation->iUserID);
			if (NULL == pUserInfo)
			{
				bAlreadyCut = true;
				pUserInfo = this->m_UserManage.FindNetCutUser(pOperation->iUserID);
				if (NULL == pUserInfo)
				{
					break;
				}
			}
			//m_TCPSocket.SendData(pUserInfo->m_uSocketIndex, MDM_GP_PLAYER_2_GM, ASS_GR_MSG_BE_BANDID, 0, pUserInfo->m_dwHandleID);
			/// 一般不可能是解除操作，因为被踢出或封号的，不可能登录到W服务器
			if (pOperation->iWorkMinutes>0)
			{
				/// 判断是否已经在游戏桌
				if (pUserInfo->m_UserData.bDeskNO != 255 && pUserInfo->m_UserData.bDeskNO<m_uDeskCount)
				{
					CGameDesk *pDesk = *(m_pDesk+pUserInfo->m_UserData.bDeskNO);
					// 不添加新字段，而在标志位的最高位置1，因时间值不会大于这么多分钟
//					pUserInfo->m_UserData.userInfoEx2 |= 0x80000000;
					//if ((pUserInfo->m_UserData.bUserState == USER_PLAY_GAME) || (pUserInfo->m_UserData.bUserState == USER_CUT_GAME))
					if(pDesk->IsPlayingByGameStation())
					{
						/// 如果玩家正在游戏，或已经断线，则令玩家断线，等游戏结束后自动清除玩家数据
						if ((pUserInfo->m_UserData.bUserState == USER_PLAY_GAME) || (pUserInfo->m_UserData.bUserState == USER_CUT_GAME))
						{
							if (!bAlreadyCut)
							{
								m_UserManage.RegNetCutUser(pUserInfo);
							}
							pDesk->UserNetCut(pUserInfo->m_UserData.bDeskStation, pUserInfo);
						}
						else
						{
							pDesk->WatchUserLeftDesk(pUserInfo->m_UserData.bDeskStation,pUserInfo);
							//清理用户资料
							CleanUserInfo(pUserInfo);
							m_UserManage.FreeUser(pUserInfo, false);
							m_TCPSocket.CloseSocket(pUserInfo->m_uSocketIndex, 0);
						}
					}
					else
					{
						pDesk->m_bLeave = true;
						pDesk->UserLeftDesk(pUserInfo->m_UserData.bDeskStation,pUserInfo);
						//清理用户资料
						CleanUserInfo(pUserInfo);
						m_UserManage.FreeUser(pUserInfo, false);
						m_TCPSocket.CloseSocket(pUserInfo->m_uSocketIndex, 0);
					}
				}
				else
				{
					//清理用户资料
					CleanUserInfo(pUserInfo);
					m_UserManage.FreeUser(pUserInfo, false);
					m_TCPSocket.CloseSocket(pUserInfo->m_uSocketIndex, 0);
				}
			}
			break;
		}
	default:
		break;
	}
	return true;
}

//SOCKET 数据读取
bool CGameMainManage::OnSocketRead(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	// GM消息处理过程
	if (MDM_GAMEMASTER_MESSAGE == pNetHead->bMainID)
	{
		return OnGMSocketMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
	}
	try
	{
	switch (pNetHead->bMainID)
	{
		//信号检测灯
	case MDM_GR_NETSIGNAL:
		{
			m_TCPSocket.SendData(uIndex,pData,uSize,MDM_GR_NETSIGNAL,0,0,dwHandleID);
			return true;
		}break;
	case MDM_GR_LOGON:			//房间登陆
		{
			return OnUserLogonMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}break;
	case MDM_GR_USER_ACTION:	//用户动作
		{
			return OnUserActionMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}break;
	case MDM_GR_ROOM:			//房间信息
		{
			return OnRoomMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}break;
	case MDM_GR_MANAGE:			//管理消息
		{
			return OnManageMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}break;
	case MDM_GR_MONEY:	//钱柜划账
		{
			return OnAboutMoney(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}break;
	case MDM_GM_GAME_FRAME:		//框架消息
	case MDM_GM_GAME_NOTIFY:	//游戏消息
		{
			//效验用户
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL)
			{
				HNLOG_G(m_InitData.uRoomID,"CGameMainManage::OnSocketRead()未找到在线玩家[uIndex=%d][%d行]", uIndex, __LINE__);
				return false;
			}
			if ((pUserInfo->m_UserData.bDeskNO>=m_uDeskCount)||(pUserInfo->m_UserData.bDeskStation>=m_KernelData.uDeskPeople))
			{
				HNLOG_G(m_InitData.uRoomID,"CGameMainManage::OnSocketRead()桌号[%d]或座位号[%d]非法[%d行]",
					pUserInfo->m_UserData.bDeskNO,
					pUserInfo->m_UserData.bDeskStation,
					__LINE__);
				return true;
			}

			//处理数据 
			bool bWatchUser=(pUserInfo->m_UserData.bUserState==USER_WATCH_GAME);
			CGameDesk * pGameDesk=*(m_pDesk+pUserInfo->m_UserData.bDeskNO);
			bool bRet = false;
			TCHAR szFunctionName[16];
			if (pNetHead->bMainID==MDM_GM_GAME_NOTIFY)
			{
				{
					_tcscpy(szFunctionName, "Notify");
					bRet = pGameDesk->HandleNotifyMessage(pUserInfo->m_UserData.bDeskStation,pNetHead,pData,uSize,uIndex,bWatchUser);
				}
			}
			else if (pNetHead->bMainID==MDM_GM_GAME_FRAME)
			{
				_tcscpy(szFunctionName, "Frame");
				bRet = pGameDesk->HandleFrameMessage(pUserInfo->m_UserData.bDeskStation,pNetHead,pData,uSize,uIndex,bWatchUser);
			}
			if (!bRet)
			{
				HNLOG_G(m_InitData.uRoomID,"CGameMainManage::OnSocketRead()中pGameDesk->Handle%sMessage()返回值为false[bMainID=%d, bAssID=%d][uIndex=%d][%d行]",
					szFunctionName, pNetHead->bMainID, pNetHead->bAssistantID, uIndex, __LINE__);
			}
			return bRet;		
		}break;
	case MDM_GR_MESSAGE:	//大厅系统信息
		{
			MSG_GR_RS_NormalTalk * pNormalTalk=(MSG_GR_RS_NormalTalk *)pData;
			if ((uSize<=(sizeof(MSG_GR_RS_NormalTalk)-sizeof(pNormalTalk->szMessage)))||(uSize>sizeof(MSG_GR_RS_NormalTalk))) return false;
			if ((pNormalTalk->iLength+1)!=(uSize+sizeof(pNormalTalk->szMessage)-sizeof(MSG_GR_RS_NormalTalk))) return false;

			MSG_GA_S_Message Message;
			memset(&Message,0,sizeof(Message));

			//发送数据
			Message.bFontSize=0;
			Message.bCloseFace=0;
			Message.bShowStation=pNormalTalk->szMessage[0]==64?2:1;
			
			string str_temp(pNormalTalk->szMessage);
			m_HNFilter.censor(str_temp,false);
			strcpy(pNormalTalk->szMessage,str_temp.c_str());
			pNormalTalk->iLength = strlen(pNormalTalk->szMessage) + 1;
			pNormalTalk->szMessage[pNormalTalk->iLength + 1] = 0;

			if (pNetHead->bAssistantID==ASS_GR_SYSTEM_MESSAGE)
				m_TCPSocket.SendDataBatch(&Message,sizeof(Message),MDM_GR_MESSAGE,ASS_GR_SYSTEM_MESSAGE,0);
			return true;
		}break;
	case MDM_GM_MESSAGE:	//房间系统信息
		{
			MSG_GR_RS_NormalTalk * pNormalTalk=(MSG_GR_RS_NormalTalk *)pData;
			if ((uSize<=(sizeof(MSG_GR_RS_NormalTalk)-sizeof(pNormalTalk->szMessage)))||(uSize>sizeof(MSG_GR_RS_NormalTalk))) return false;
			if ((pNormalTalk->iLength+1)!=(uSize+sizeof(pNormalTalk->szMessage)-sizeof(MSG_GR_RS_NormalTalk))) return false;

			MSG_GA_S_Message Message;
			memset(&Message,0,sizeof(Message));

			//发送数据
			Message.bFontSize=0;
			Message.bCloseFace=0;
			Message.bShowStation=1;
			
			string str_temp(pNormalTalk->szMessage);
			m_HNFilter.censor(str_temp,false);
			strcpy(pNormalTalk->szMessage,str_temp.c_str());
			pNormalTalk->iLength = strlen(pNormalTalk->szMessage) + 1;
			pNormalTalk->szMessage[pNormalTalk->iLength + 1] = 0;

			if (pNetHead->bAssistantID==ASS_GM_SYSTEM_MESSAGE)
				m_TCPSocket.SendDataBatch(&Message,sizeof(Message),MDM_GM_MESSAGE,ASS_GM_SYSTEM_MESSAGE,0);

			return true;
		}break;
	case MDM_GR_POINT://经验盒?
		{
			return OnPointMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}break;
	case MDM_GR_FRIEND_MANAGE://好友
		{
			return OnFriendManageMsg(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}break;
	case MDM_GR_QUEUE_MSG://排队
		{
			return OnQueueMsg(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}break;
	case MDM_GR_PROP://vip踢人也放入PROP中
		{
            // 踢人卡踢人功能
            if(ASS_PROP_NEW_KICKUSER == pNetHead->bAssistantID)
            {
                return OnNewKickUserProp(pNetHead, pData, uSize, uAccessIP, uIndex, dwHandleID);
            }

			if(m_PropService)
			{				
				return m_PropService->OnNetMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
			}
			return true;
		}break;
	case MDM_BANK:
		{
			if (m_pBankService)
			{
				return m_pBankService->OnNetMessage(pNetHead, pData, uSize, uAccessIP, uIndex, dwHandleID);
			}
		}break;
	case MDM_GR_DESKRUNOUT://桌子解散
		{
			return OnDeskDissmiss(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}break;
	case MDM_GR_VOIEC://语音消息
		{
			if (ASS_GR_VOIEC == pNetHead->bAssistantID)
			{
				return OnVoiceInfo(pNetHead, pData, uSize, uAccessIP, uIndex, dwHandleID);
			}
		}break;
	}
	}
	catch (...)
	{
		HNLOG_G(m_InitData.uRoomID,"GameMainManage OnSocketRead异常[bMainID = %d][%d行]", pNetHead->bMainID, __LINE__);
		return false;
	}
	return false;
}



//大厅登陆处理（登陆每个房间）

//用户动作处理
bool CGameMainManage::OnUserActionMessage(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	TRACE("USER ACTION MESSAGE:%d with %d\n",pNetHead->bMainID,pNetHead->bAssistantID);
	if(pNetHead->bMainID!=MDM_GR_USER_ACTION)
		return false;
	
	switch (pNetHead->bAssistantID)
	{
	case ASS_GR_USER_HIT_BEGIN:
		{
			MSG_GM_WatchSet UserAgreeID;
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if(pUserInfo!=NULL)
			{
				pUserInfo->m_UserData.bUserState=USER_DESK_AGREE;
				UserAgreeID.dwUserID = pUserInfo->m_UserData.dwUserID;
				m_TCPSocket.SendDataBatch(&UserAgreeID,sizeof(MSG_GM_WatchSet),MDM_GR_USER_ACTION,ASS_GR_USER_HIT_BEGIN,0);
				return true;
			}
			else
				return false;
		}
	case ASS_GR_SET_TABLE_BASEPOINT:
		{
			if(uSize!=sizeof(MSG_GR_S_SetDeskBasePoint)) return false;
			MSG_GR_S_SetDeskBasePoint* setBasePoint=(MSG_GR_S_SetDeskBasePoint*)pData;

			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if(pUserInfo->m_UserData.bDeskNO!=255 && pUserInfo->m_UserData.bDeskNO<m_uDeskCount)
			{
				CGameDesk * pNewDesk=*(m_pDesk+pUserInfo->m_UserData.bDeskNO);
				if(pNewDesk->GetTableOwnerID()==setBasePoint->dwUserID)//桌主
				{
					setBasePoint->bDeskOwner=1;
					pNewDesk->SetTableBasePoint(setBasePoint->iDeskBasePoint);
				}
			}else 
				setBasePoint->bDeskOwner=0;
			if(pUserInfo==NULL)
			{
				//发送失败信息
				m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_SET_TABLE_BASEPOINT,0,dwHandleID);
				return false;
			}
			else 
			{
				//pUserInfo->m_UserData.uDeskBasePoint=setBasePoint->iDeskBasePoint;
				//m_TCPSocket.SendDataBatch(setBasePoint,uSize,MDM_GR_USER_ACTION,ASS_GR_SET_TABLE_BASEPOINT,1);
			}
			return true;
		}
	case ASS_GR_MASTER_LEAVE:
		{
			if (uSize!=0) return false;
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL) return false;

			if (pUserInfo->m_UserData.bDeskNO!=255 && pUserInfo->m_UserData.bDeskNO<m_uDeskCount)
			{
				if (!(m_InitData.dwRoomRule & GRR_GAME_BUY))
				{
					m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_MASTER_LEAVE,ERR_GR_NOT_BUY_ROOM,dwHandleID);
					return true;
				}
				else if (IsUserPlayGame(pUserInfo))
				{
					m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_MASTER_LEAVE,ERR_GR_IN_GAME,dwHandleID);
					return true;
				}
				else if (m_InitData.bLockMaster != 0)
				{
					m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_MASTER_LEAVE,ERR_GR_CANNOT_LEAVE,dwHandleID);
					return true;
				}

				CGameDesk *pDesk = *(m_pDesk+pUserInfo->m_UserData.bDeskNO);
				if (pDesk && pDesk->m_iDeskMaster != pUserInfo->m_UserData.dwUserID)
				{
					m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_MASTER_LEAVE,ERR_GR_NOT_MASTER,dwHandleID);
					return true;
				}

				if (pDesk->m_iClubID != 0)
				{
					m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_MASTER_LEAVE,ERR_GR_IS_CLUBROOM,dwHandleID);
					return true;
				}

				DL_GR_I_MasterLeave MasterLeave; 
				MasterLeave.bLeave = true;
				MasterLeave.bMaster = true;
				MasterLeave.iDeskID = pUserInfo->m_UserData.bDeskNO;
				MasterLeave.iUserID = pUserInfo->m_UserData.dwUserID;
				m_SQLDataManage.PushLine(&MasterLeave.DataBaseHead, sizeof(MasterLeave), DTK_GR_MASTER_LEAVE, uIndex, dwHandleID);
				pDesk->m_bMasterState = 1;

				m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_MASTER_LEAVE,ERR_GR_LEAVE_SUCCESS,dwHandleID);
				UserLeftDesk(pUserInfo);
			}
			else
			{
				m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_MASTER_LEAVE,ERR_GR_NOT_IN_DESK,dwHandleID);
				return true;
			}
			return true;
		}
	case ASS_GR_USER_UP:		//用户起来
	case ASS_GR_WATCH_UP:		//旁观起来
		{
			//效验数据合法性
			if (uSize!=0) return false;
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL) return false;
			//判断是否可以离开
			if (pUserInfo->m_UserData.bDeskNO!=255)
			{
				if (IsUserPlayGame(pUserInfo)==true) 
				{
					m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_SIT_ERROR,ERR_GR_CAN_NOT_LEFT,dwHandleID);
					return true;
				}
				
				UserLeftDesk(pUserInfo);

			}
			return true;
		}
	case ASS_GR_USER_SIT:		//用户坐下游戏
	case ASS_GR_WATCH_SIT:		//用户旁观游戏
		{
			//效验数据
			if (uSize!=sizeof(MSG_GR_S_UserSit)) return false;
			MSG_GR_S_UserSit * pUserSitInfo=(MSG_GR_S_UserSit *)pData;
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL) return false;
			if ((pUserSitInfo->bDeskIndex>=m_uDeskCount)||(pUserSitInfo->bDeskStation>=m_KernelData.uDeskPeople)) return false;

			//过滤是否连续发信息
			if ((pUserInfo->m_UserData.bDeskNO==pUserSitInfo->bDeskIndex)&&
				(pUserInfo->m_UserData.bDeskStation==pUserSitInfo->bDeskStation)) 
			{
				m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_SIT_CUTRESIT,ERR_GR_ALREAD_USER,dwHandleID);
				return true;
			}

			//设置用户离开原位置
			if (pUserInfo->m_UserData.bDeskNO!=255)
			{
				if (IsUserPlayGame(pUserInfo)==true)
				{
					if(m_InitData.uComType==TY_MATCH_GAME) return true;
					m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_SIT_ERROR,ERR_GR_CAN_NOT_LEFT,dwHandleID);
					return true;
				}
				UserLeftDesk(pUserInfo);
			}

			//设置用户到新位置
			BYTE bErrorCode=ERR_GR_ERROR_UNKNOW;
			CGameDesk * pNewDesk=*(m_pDesk+pUserSitInfo->bDeskIndex);
			///  如果被虚拟锁桌了，则不能坐下
			if (pNewDesk->IsVirtualLock())
			{
				m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_SIT_ERROR,ERR_GR_ALREAD_USER,dwHandleID);
				return true;
			}
			/// 创建房间检查密码
			if (pNetHead->bAssistantID==ASS_GR_USER_SIT) 
			{
				if (m_InitData.dwRoomRule & GRR_GAME_BUY)
				{
					return true;
				}
				else
				{
					bErrorCode=pNewDesk->UserSitDesk(pUserSitInfo,pUserInfo);
				}
			}
			else 
			{
				bErrorCode=pNewDesk->WatchUserSitDesk(pUserSitInfo,pUserInfo);
			}

			if (bErrorCode!=ERR_GR_SIT_SUCCESS)	
			{
				m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_SIT_ERROR,bErrorCode,dwHandleID);
			}
			return true;
		}
	case ASS_GR_JOIN_QUEUE:	///<有用户加入排队机
		{
			///不是排队机房间
			if (!IsQueueGameRoom())
			{
				return true;
			}

			if (uSize!=0) 
			{
				return false;
			}

			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL) 
			{
				return false;
			}

			if ((m_InitData.dwRoomRule & GRR_CONTEST) && 1 != m_InitData.iRoomState)
			{
				//设置比赛开始标志
				m_InitData.iRoomState = 1;
				//从数据库获取比赛ID，并通知比赛开始
				DL_GR_I_ContestBegin DL_ContestBegin;
				DL_ContestBegin.uNameID = m_InitData.iGameID;
				DL_ContestBegin.uRoomID = m_InitData.uRoomID;
				DL_ContestBegin.iContestID = m_InitData.iContestID;
				m_SQLDataManage.PushLine(&DL_ContestBegin.DataBaseHead, sizeof(DL_ContestBegin), DTK_GR_CONTEST_BEGIN, uIndex, dwHandleID);					
			}
			if (!(m_InitData.dwRoomRule & GRR_CONTEST) && !(m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
			{
				if (m_InitData.uComType==TY_MONEY_GAME && pUserInfo->m_UserData.i64Money < m_InitData.uLessPoint)	//排队机钱不足
				{
					m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_JOIN_QUEUE,1,dwHandleID);
					return true;
				}
				m_pGameUserInfoList.remove(pUserInfo);		//删除正在排队的玩家，防止同一用户排队多次
				m_pGameUserInfoList.push_back(pUserInfo);
				AutoAllotDeskAddUser(pUserInfo);
			}
			return true;
		}
	case ASS_GR_QUIT_QUEUE:///<有人离开排队机
		{
			///不是排队机房间
			if (!IsQueueGameRoom())
			{
				return true;
			}

			if (uSize!=0) 
			{
				return false;
			}

			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL)
			{
				return false;
			}

			//int dwUserID = pUserInfo->m_UserData.dwUserID;
			m_pGameUserInfoList.remove(pUserInfo);	//删除正在排队的玩家

			AutoAllotDeskDeleteUser(pUserInfo);

			return true;
		}
	case ASS_GR_CONTEST_APPLY:
		{
			if (!(m_InitData.dwRoomRule & GRR_CONTEST) && !(m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
			{
				return false;
			}
			
			if (sizeof(MSG_GR_I_ContestApply) != uSize && 0 != uSize)
			{
				return false;
			}
			DL_GR_I_ContestApply DL_ContestApply;
			if (0 == uSize)
			{
				CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
				if (pUserInfo==NULL) 
				{
					return false;
				}
				DL_ContestApply.iUserID = pUserInfo->m_UserData.dwUserID;
				DL_ContestApply.iTypeID = 0;//默认为报名
			}
			else
			{
				MSG_GR_I_ContestApply *_in = (MSG_GR_I_ContestApply*)pData;
				
				DL_ContestApply.iUserID = _in->iUserID;
				DL_ContestApply.iTypeID = _in->iTypeID;
			}

			m_SQLDataManage.PushLine(&DL_ContestApply.DataBaseHead, sizeof(DL_ContestApply), DTK_GR_CONTEST_APPLY, uIndex, dwHandleID);
			return true;
		}
	case ASS_GR_FAST_JOIN_IN://快速坐桌
		{
			return OnUserAction_FastSit(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}
	case ASS_GR_CHANGE_DESK:
		{
			return OnUserAction_ChangeDesk(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}break;
	case ASS_GR_GET_USERINFO:
		{
			return OnGetDeskUserInfo(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}break;
	}
	return false;
}

//钱柜划账处理
bool CGameMainManage::OnAboutMoney(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	ASSERT(pNetHead->bMainID==MDM_GR_MONEY);
	switch (pNetHead->bAssistantID)
	{
	case ASS_GR_PAY_MONEY:
		{
			//扣钱，不直接重数据库里扣，扣身上的钱

			//效验数据
			if(uSize!=sizeof(MSG_GR_S_Pay)) return false;
			MSG_GR_S_Pay* pPayMoney=(MSG_GR_S_Pay*)pData;
			//处理数据
			CGameUserInfo* pUserInfo = m_UserManage.FindOnLineUser(pPayMoney->UserID);
			if(pUserInfo==NULL)
				return false;

			if(pPayMoney->uCount + pUserInfo->m_UserData.i64Money < 0)//身上的钱不够
			{
				m_TCPSocket.SendData(uIndex,MDM_GR_MONEY,ASS_GR_PAY_MONEY,0,dwHandleID);//
				return true;
			}

			pUserInfo->m_ChangePoint.dwMoney +=  pPayMoney->uCount;
			pUserInfo->m_UserData.i64Money +=  pPayMoney->uCount;
			//嘟嘟扣钱
			DL_GR_I_UserLeft m_UserInfo;
			memset(&m_UserInfo,0,sizeof(m_UserInfo));
			m_UserInfo.dwUserID = pPayMoney->UserID;
			m_UserInfo.dwChangeMoney = pPayMoney->uCount;
			m_SQLDataManage.PushLine(&m_UserInfo.DataBaseHead,sizeof(m_UserInfo),DTK_GR_UPDATE_GAME_RESULT,0,0);

			m_TCPSocket.SendDataBatch(pPayMoney,uSize,MDM_GR_MONEY,ASS_GR_PAY_MONEY,1);//成功
			return true;
		}
	case ASS_GR_REFLASH_MONEY:
		{
			//效验数据
			if(uSize!=sizeof(MSG_GR_S_RefalshMoney)) return false;
			MSG_GR_S_RefalshMoney* pReflashMoney=(MSG_GR_S_RefalshMoney*)pData;
			if(pNetHead->bHandleCode==0)//同步金币
			{
				if(m_InitData.uComType!=TY_MONEY_GAME) return false;

				//处理数据
				CGameUserInfo * pUserInfo=m_UserManage.FindOnLineUser(pReflashMoney->dwUserID);
				if (pUserInfo==NULL) return false;
				else
				{
					pUserInfo->m_UserData.i64Money = pReflashMoney->i64Money;
					if(pUserInfo->m_UserData.i64Money<0) 
					{
						pUserInfo->m_UserData.i64Money=0; 
						pReflashMoney->i64Money=0;
					}

				}
				m_TCPSocket.SendDataBatch(pReflashMoney,uSize,MDM_GR_ROOM,ASS_GR_USER_POINT,10);
				return true;
			}
		}
	case ASS_GR_OPEN_WALLET_INGAME:		///< 游戏中打开银行界面 
		{
			//效验数据
			MSG_GR_OpenBank * pOpenBank=(MSG_GR_OpenBank *)pData;

			//处理数据
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL) return false;

			//读取数据库
			DL_GR_I_UnRegisterUser OpenWallet;
			memset(&OpenWallet,0,sizeof(OpenWallet));
			OpenWallet.dwUserID = pUserInfo->m_UserData.dwUserID;
			
			m_SQLDataManage.PushLine(&OpenWallet.DataBaseHead,sizeof(OpenWallet),DTK_GR_DEMAND_MONEY_IN_GAME,uIndex,dwHandleID);
			return true;
		}
	case ASS_GR_OPEN_WALLET:			///< 在大厅打开钱柜
		{
			MSG_GR_OpenBank * pOpenBank=(MSG_GR_OpenBank *)pData;

			//处理数据
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL) return false;

			//读取数据库
			DL_GR_I_UnRegisterUser OpenWallet;
			memset(&OpenWallet,0,sizeof(OpenWallet));
			OpenWallet.dwUserID = pUserInfo->m_UserData.dwUserID;
		
			m_SQLDataManage.PushLine(&OpenWallet.DataBaseHead,sizeof(OpenWallet),DTK_GR_DEMAND_MONEY,uIndex,dwHandleID);
			return true;
		}
	case ASS_GR_CHECK_OUT:			//取
		{
			//效验数据
			if (uSize!=sizeof(MSG_GR_S_CheckMoneyWithPwd_t)) return false;
			MSG_GR_S_CheckMoneyWithPwd_t* pCheckOut=(MSG_GR_S_CheckMoneyWithPwd_t *)pData;

			//处理数据
			CGameUserInfo* pTargetUserInfo=m_UserManage.FindOnLineUser(pCheckOut->UserID);

			//安全验证，由于机器人共用一个uIndex，所以机器人不参与安全验证
			if (pTargetUserInfo == NULL)
			{
				return false;
			}

			if (!pTargetUserInfo->m_UserData.isVirtual)
			{
				CGameUserInfo* pUserInfo = m_UserManage.GetOnLineUserInfo(uIndex);
				if (pUserInfo == NULL)
				{
					return false;
				}

				if (pCheckOut->UserID != pUserInfo->m_UserData.dwUserID)
				{
					return false;
				}
			}

			//end add

			/// 如果玩家正在游戏中，非百人类游戏或庄家(可配)，则不让取钱
			if (pTargetUserInfo->m_UserData.bUserState == USER_PLAY_GAME 
				|| pTargetUserInfo->m_UserData.bUserState == USER_CUT_GAME
				|| ((m_InitData.dwRoomRule & GRR_GAME_U3D) && IsUserPlayGame(pTargetUserInfo))
				)
			{
				if(pTargetUserInfo->m_UserData.bDeskNO != 255 && pTargetUserInfo->m_UserData.bDeskNO<m_uDeskCount)
				{
					CGameDesk *pDesk = *(m_pDesk+pTargetUserInfo->m_UserData.bDeskNO);

					if(!pDesk->IsBJLGameType())
					{		
						m_TCPSocket.SendData(uIndex, MDM_GR_MONEY, ASS_GR_CHECK_OUT, ASS_GR_CHECKOUT_ERROR_PLAYING, dwHandleID);
						return true;
					}

					CBcfFile f(CBcfFile::GetAppPath() + "Function.bcf");
					int drawMoneyInGame = f.GetKeyVal("DrawMoneyInGame","Available",0);
					if(drawMoneyInGame)
					{
						if(pDesk->IsZhuangjia(pTargetUserInfo->m_UserData.dwUserID))
						{
							m_TCPSocket.SendData(uIndex, MDM_GR_MONEY, ASS_GR_CHECK_OUT, ASS_GR_CHECKOUT_ERROR_PLAYING, dwHandleID);
							return true;
						}
					}

				}
			}


			//写入数据库
			DL_GR_I_CheckOutMoneyWithPwd_t CheckOut;
			memset(&CheckOut,0,sizeof(CheckOut));
			CheckOut.dwUserID=pCheckOut->UserID;
			CheckOut.i64Count=pCheckOut->i64Count;
			CopyMemory(CheckOut.szMD5Pass, pCheckOut->szMD5Pass, sizeof(CheckOut.szMD5Pass));
			m_SQLDataManage.PushLine(&CheckOut.DataBaseHead,sizeof(CheckOut),DTK_GR_CHECKOUT_MONEY,uIndex,dwHandleID);
			return true;
		}
	case ASS_GR_CHECK_OUT_INGAME:	// 游戏中取
		{
			//效验数据
			if (uSize!=sizeof(MSG_GR_S_CheckMoneyWithPwd_t)) return false;
			MSG_GR_S_CheckMoneyWithPwd_t * pCheckOut=(MSG_GR_S_CheckMoneyWithPwd_t *)pData;

			//处理数据
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL) return false;
			CGameUserInfo * pTargetUserInfo=m_UserManage.FindOnLineUser(pCheckOut->UserID);
			if (pTargetUserInfo==NULL||pCheckOut->UserID!=pUserInfo->m_UserData.dwUserID) return false;

			/// 如果玩家正在游戏中，且是非百人类游戏，则不让取钱
			if (pUserInfo->m_UserData.bUserState == USER_PLAY_GAME 
				|| pUserInfo->m_UserData.bUserState == USER_CUT_GAME
				|| ((m_InitData.dwRoomRule & GRR_GAME_U3D) && IsUserPlayGame(pTargetUserInfo))
				)
			{
				if(pUserInfo->m_UserData.bDeskNO != 255 && pUserInfo->m_UserData.bDeskNO<m_uDeskCount)
				{
					CGameDesk *pDesk = *(m_pDesk+pUserInfo->m_UserData.bDeskNO);
					if(!pDesk->IsBJLGameType())
					{		
						m_TCPSocket.SendData(uIndex, MDM_GR_MONEY, ASS_GR_CHECK_OUT_INGAME, ASS_GR_CHECKOUT_ERROR_PLAYING, dwHandleID);
						return true;
					}

					CBcfFile f(CBcfFile::GetAppPath() + "Function.bcf");
					int drawMoneyInGame = f.GetKeyVal("DrawMoneyInGame","Available",0);
					if(drawMoneyInGame)
					{
						if(pDesk->IsZhuangjia(pUserInfo->m_UserData.dwUserID))
						{
							m_TCPSocket.SendData(uIndex, MDM_GR_MONEY, ASS_GR_CHECK_OUT_INGAME, ASS_GR_CHECKOUT_ERROR_PLAYING, dwHandleID);
							return true;
						}
					}
				}
			}


			//写入数据库
			DL_GR_I_CheckOutMoneyWithPwd_t CheckOut;
			memset(&CheckOut,0,sizeof(CheckOut));
			CheckOut.dwUserID=pCheckOut->UserID;
			CheckOut.i64Count=pCheckOut->i64Count;
			CopyMemory(CheckOut.szMD5Pass, pCheckOut->szMD5Pass, sizeof(CheckOut.szMD5Pass));
			m_SQLDataManage.PushLine(&CheckOut.DataBaseHead,sizeof(CheckOut),DTK_GR_CHECKOUT_MONEY_INGAME,uIndex,dwHandleID);
			return true;

		}
	case ASS_GR_CHECK_IN:			//存钱
		{
			//效验数据
			if (uSize!=sizeof(MSG_GR_S_CheckMoney)) return false;
			MSG_GR_S_CheckMoney * pCheckIn=(MSG_GR_S_CheckMoney *)pData;

			//处理数据
			CGameUserInfo* pTargetUserInfo=m_UserManage.FindOnLineUser(pCheckIn->UserID);

			//安全验证，由于机器人共用一个uIndex，所以机器人不参与安全验证
			if (pTargetUserInfo == NULL)
			{
				return false;
			}
			
			if (!pTargetUserInfo->m_UserData.isVirtual)
			{
				CGameUserInfo* pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
				if (pUserInfo == NULL)
				{
					return false;
				}
				if (pCheckIn->UserID != pUserInfo->m_UserData.dwUserID)
				{
					return false;
				}
			}
			//end add

			/// 如果玩家正在游戏中，则不让存钱

			if (pTargetUserInfo->m_UserData.bUserState == USER_PLAY_GAME
				|| pTargetUserInfo->m_UserData.bUserState == USER_CUT_GAME
				|| ((m_InitData.dwRoomRule & GRR_GAME_U3D) && IsUserPlayGame(pTargetUserInfo))
				)
			{
				m_TCPSocket.SendData(uIndex, MDM_GR_MONEY, ASS_GR_CHECK_IN, ASS_GR_CHECKIN_ERROR_PLAYING, dwHandleID);
				return true;
			}

			/// 百家乐类游戏，如果玩家是庄家，则不让存钱
			//判断是否在游戏桌子，只有在游戏中才有可能是庄家
			if(pTargetUserInfo->m_UserData.bDeskNO != 255 && pTargetUserInfo->m_UserData.bDeskNO<m_uDeskCount)
			{
				CGameDesk* pDesk = *(m_pDesk + pTargetUserInfo->m_UserData.bDeskNO);
				if(pDesk->IsZhuangjia(pTargetUserInfo->m_UserData.dwUserID))
				{
					m_TCPSocket.SendData(uIndex, MDM_GR_MONEY, ASS_GR_CHECK_IN_INGAME, ASS_GR_CHECKIN_ZHUANGJIA, dwHandleID);
					return true;
				}
			}

			//写入数据库
			DL_GR_I_CheckOutMoney CheckIn;
			memset(&CheckIn,0,sizeof(CheckIn));
			CheckIn.dwUserID=pCheckIn->UserID;
			CheckIn.i64Count= pCheckIn->i64Count;
			m_SQLDataManage.PushLine(&CheckIn.DataBaseHead,sizeof(CheckIn),DTK_GR_CHECKIN_MONEY,uIndex,dwHandleID);
			return true;

		}
	case ASS_GR_CHECK_IN_INGAME:	//游戏中存钱
		{
			//效验数据
			if (uSize!=sizeof(MSG_GR_S_CheckMoney)) return false;
			MSG_GR_S_CheckMoney * pCheckIn=(MSG_GR_S_CheckMoney *)pData;

			//处理数据
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL) return false;
			CGameUserInfo * pTargetUserInfo=m_UserManage.FindOnLineUser(pCheckIn->UserID);
			if (pTargetUserInfo==NULL||pCheckIn->UserID!=pUserInfo->m_UserData.dwUserID) return false;
			/// 如果玩家正在游戏中，则不让存钱
			/// todo
			if (pUserInfo->m_UserData.bUserState == USER_PLAY_GAME
				|| pUserInfo->m_UserData.bUserState == USER_CUT_GAME
				|| ((m_InitData.dwRoomRule & GRR_GAME_U3D) && IsUserPlayGame(pTargetUserInfo))
				)
			{
				m_TCPSocket.SendData(uIndex, MDM_GR_MONEY, ASS_GR_CHECK_IN_INGAME, ASS_GR_CHECKIN_ERROR_PLAYING, dwHandleID);
				return true;
			}

			/// 百家乐类游戏，如果玩家是庄家，则不让存钱
			//判断是否在游戏桌子，只有在游戏中才有可能是庄家
			if(pUserInfo->m_UserData.bDeskNO != 255 && pUserInfo->m_UserData.bDeskNO<m_uDeskCount)
			{
				CGameDesk *pDesk = *(m_pDesk+pUserInfo->m_UserData.bDeskNO);
				if(pDesk->IsZhuangjia(pUserInfo->m_UserData.dwUserID))
				{
					m_TCPSocket.SendData(uIndex, MDM_GR_MONEY, ASS_GR_CHECK_IN_INGAME, ASS_GR_CHECKIN_ZHUANGJIA, dwHandleID);
					return true;
				}
			}

			//写入数据库
			DL_GR_I_CheckOutMoney CheckIn;
			memset(&CheckIn,0,sizeof(CheckIn));
			CheckIn.dwUserID=pCheckIn->UserID;
			CheckIn.i64Count=pCheckIn->i64Count;
			m_SQLDataManage.PushLine(&CheckIn.DataBaseHead,sizeof(CheckIn),DTK_GR_CHECKIN_MONEY_INGAME,uIndex,dwHandleID);
			return true;

		}
	case ASS_GR_TRANSFER_SET_REBATE_RES:
		{
			return OnUserSetRebateRequest(pData,uSize,uIndex,dwHandleID);
		}
	case ASS_GR_TRANSFER_MONEY:			//转帐
		{
			//效验数据
			if (uSize!=sizeof(MSG_GR_S_TransferMoney)) return false;
			MSG_GR_S_TransferMoney * pTransferMoney=(MSG_GR_S_TransferMoney *)pData;

			//处理数据
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL)
				return false;
			if(pTransferMoney->UserID != pUserInfo->m_UserData.dwUserID)
				return false;

			/// 如果自己转给自己
			if (pTransferMoney->UserID == pTransferMoney->destUserID)
			{
				return false;
			}

			//写入数据库
			DL_GR_I_TransferMoney TransferMoney;
			memset(&TransferMoney,0,sizeof(TransferMoney));
			::memcpy(&TransferMoney.stTransfer,pTransferMoney,sizeof(MSG_GR_S_TransferMoney));
			//应交的税
			CString stime;
			stime.Format("%d",CTime::GetCurrentTime());        //当前的时间，String格式
			int curtime=atoi(stime);                           //转换为int

			TransferMoney.stTransfer.bTranTax = 1; ///< 默认情况下对每笔转账扣税

			CBcfFile fConfig(CBcfFile::GetAppPath()+"Function.bcf");
			CString strBuffer = fConfig.GetKeyVal("SpecificID","Available","0");
			if (atoi(strBuffer)) ///< 指派账号ID
			{
				strBuffer = fConfig.GetKeyVal("SpecificID","NormalID","60000000,69999999");
				int reValue = TransferIDCompare(TransferMoney.stTransfer.UserID,TransferMoney.stTransfer.destUserID,
					atol(strBuffer.Left(strBuffer.Find(",")+1)),
					atol(strBuffer.Right(strBuffer.GetLength()-strBuffer.Find(",")-1)));

				switch (reValue)
				{
				case 0:
					return false; ///< 不能转账

				case 1:
					///< 可以转账，扣税
					TransferMoney.stTransfer.bTranTax = 1;
					break;

				case 2:
					///< 可以转账，不扣税
					TransferMoney.stTransfer.bTranTax = 2;
					break;

				case 3:
					///< 散户间转账
					TransferMoney.stTransfer.bTranTax = 3;
					break;

				default :
					break;
				}
			}

            // 普通转账还是全转功能，1=全转
            TransferMoney.stTransfer.uHandleCode = pNetHead->bHandleCode;

			m_SQLDataManage.PushLine(&TransferMoney.DataBaseHead,sizeof(TransferMoney),DTK_GR_TRANSFER_MONEY,uIndex,dwHandleID);
			return true;
		}
	case ASS_GR_TRANSFER_MONEY_INGAME:	//游戏中转帐
		{
			//效验数据
			if (uSize!=sizeof(MSG_GR_S_TransferMoney)) return false;
			MSG_GR_S_TransferMoney * pTransferMoney=(MSG_GR_S_TransferMoney *)pData;

			//处理数据
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL)
				return false;
			if(pTransferMoney->UserID != pUserInfo->m_UserData.dwUserID)
				return false;

			/// 如果自己转给自己
			if (pTransferMoney->UserID == pTransferMoney->destUserID)
			{
				return false;
			}

			//写入数据库
			DL_GR_I_TransferMoney TransferMoney;
			memset(&TransferMoney,0,sizeof(TransferMoney));
			::memcpy(&TransferMoney.stTransfer,pTransferMoney,sizeof(MSG_GR_S_TransferMoney));
			//应交的税
			CString stime;
			stime.Format("%d",CTime::GetCurrentTime());        //当前的时间，String格式
			int curtime=atoi(stime);                           //转换为int

			TransferMoney.stTransfer.bTranTax = 1; ///< 默认情况下对每笔转账扣税

			CBcfFile fConfig(CBcfFile::GetAppPath()+"Function.bcf");
			CString strBuffer = fConfig.GetKeyVal("SpecificID","Available","0");
			if (atoi(strBuffer)) ///< 指派账号ID
			{
				strBuffer = fConfig.GetKeyVal("SpecificID","NormalID","60000000,69999999");
				int reValue = TransferIDCompare(TransferMoney.stTransfer.UserID,TransferMoney.stTransfer.destUserID,
					atol(strBuffer.Left(strBuffer.Find(",")+1)),
					atol(strBuffer.Right(strBuffer.GetLength()-strBuffer.Find(",")-1)));

				switch (reValue)
				{
				case 0:
					return false; ///< 不能转账

				case 1:
					///< 可以转账，扣税
					TransferMoney.stTransfer.bTranTax = 1;
					break;

				case 2:
					///< 可以转账，不扣税
					TransferMoney.stTransfer.bTranTax = 2;
					break;

				case 3:
					///< 散户间转账
					TransferMoney.stTransfer.bTranTax = 3;
					break;

				default :
					break;
				}
			}

			m_SQLDataManage.PushLine(&TransferMoney.DataBaseHead,sizeof(TransferMoney),DTK_GR_TRANSFER_MONEY_INGAME,uIndex,dwHandleID);
			return true;
		}
	case ASS_GR_CHANGE_PASSWD:
		{
			if (uSize != sizeof(MSG_GR_S_ChangePasswd_t))
			{
				 return false;
			}
			MSG_GR_S_ChangePasswd_t *pChangePasswd = (MSG_GR_S_ChangePasswd_t *)pData;

			/// 处理数据，找出用户名，发送调用存储过程的指令到队列中
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL)
				return false;
			if(pChangePasswd->UserID != pUserInfo->m_UserData.dwUserID)
				return false;
			/// 
			DL_GR_I_ChangePasswd_t changePasswd;
			changePasswd.stChangePasswd.UserID = pUserInfo->m_UserData.dwUserID;
			_tcscpy(changePasswd.stChangePasswd.szMD5PassNew, pChangePasswd->szMD5PassNew);
			_tcscpy(changePasswd.stChangePasswd.szMD5PassOld, pChangePasswd->szMD5PassOld);
			m_SQLDataManage.PushLine(&changePasswd.DataBaseHead,sizeof(changePasswd),DTK_GR_CHANGE_PASSWD,uIndex,dwHandleID);

			return true;
		}
	case ASS_GR_TRANSFER_RECORD:
		{
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL)
			{
				return false;
			}
			MSG_GR_S_TransferRecord_t *pTransferRecord = (MSG_GR_S_TransferRecord_t *)pData;
			/// 如果用户名不是自己的用户名，则失败
			if (pUserInfo->m_UserData.dwUserID != pTransferRecord->dwUserID)
			{
				return false;
			}
			DL_GR_I_TransferRecord_t transferRecord;
			transferRecord.stTransfer.dwUserID = pUserInfo->m_UserData.dwUserID;
			m_SQLDataManage.PushLine(&transferRecord.DataBaseHead,sizeof(transferRecord),DTK_GR_TRANSFER_RECORD,uIndex,dwHandleID);
		
			break;
		}
	case ASS_GR_TRANSFER_RECORD_INGAME:
		{
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL)
			{
				return false;
			}
			MSG_GR_S_TransferRecord_t *pTransferRecord = (MSG_GR_S_TransferRecord_t *)pData;
			/// 如果用户名不是自己的用户名，则失败
			if (pUserInfo->m_UserData.dwUserID != pTransferRecord->dwUserID)
			{
				return false;
			}
			DL_GR_I_TransferRecord_t transferRecord;
			transferRecord.stTransfer.dwUserID = pUserInfo->m_UserData.dwUserID;
			m_SQLDataManage.PushLine(&transferRecord.DataBaseHead,sizeof(transferRecord),DTK_GR_TRANSFER_RECORD_INGAME,uIndex,dwHandleID);
			
			break;
		}
	}
	return true;
}

//玩家返利修改
bool CGameMainManage::OnUserSetRebateRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GR_UserRebateSetResult *pCmd=(MSG_GR_UserRebateSetResult*)pData;

	DL_GR_I_UserRebateSetStruct cmd;
	cmd.bTag = pCmd->bTag;
	cmd.iUserID = pCmd->iUserID;
	cmd.iMoney = pCmd->iMoney;

	m_SQLDataManage.PushLine(&cmd.DataBaseHead,sizeof(DL_GR_I_UserRebateSetStruct),
		DTK_GR_BANK_SET_USER_REBATE,uIndex,dwHandleID);

	return true;
}

/// 转账用户ID比较
/// @param UserID 玩家自己的ID，UserIDDest目标ID,散户ID NomalIDFrom开始，散户ID NomalIDEnd结束
///@return  0不能转账，1 可以转账,扣税。2 可以转账,不扣税.3 可以转账都为非指定，检查转账金额
int CGameMainManage::TransferIDCompare(long UserID,long UserIDDest,long NomalIDFrom,long NomalIDEnd)
{
	char BufID[12]     = {0};
	char BufIDDest[12] = {0};

	BOOL specificID     = TRUE;  ///< 是否指定ID，21111100 21111000。自己的ID号
	BOOL specificIDDest = TRUE;  ///< 是否指定ID，21111100 21111000。对方的ID号
	BOOL sameNumID      = TRUE;  ///< 是否连号ID，11111 444444。自己的ID号
	BOOL sameNumIDDest  = TRUE;  ///< 是否连号ID，11111 444444。对方的ID号
	BOOL sameID         = TRUE;  ///< 两个ID是否相同
	int CntZero         = 0;   ///< 0的个数
	int CntZeroDest     = 0;   ///< 0的个数
	int Len             = 0;   ///< ID长度
	int LenDest         = 0;   ///< ID长度
	int CntSame         = 0;   ///< 自己ID跟对方ID相同的个数

	if ((UserID>NomalIDFrom && UserID<NomalIDEnd) && (UserIDDest>NomalIDFrom && UserIDDest<NomalIDEnd))///< 散户
	{
		return 3;
	}

	if (UserID<NomalIDFrom && UserIDDest<NomalIDFrom)
	{
		sprintf(BufID,"%ld",UserID);
		sprintf(BufIDDest,"%ld",UserIDDest);

		for (int i=0; 0!=BufID[i] || 0!=BufIDDest[i]; i++)
		{
			if (0!=BufID[i])
			{
				if (specificID)
				{
					if ('0' == BufID[i])
					{
						CntZero++;
					}
					else if ('0' != BufID[i] && 0 != CntZero)
					{
						specificID = FALSE; ///< 不是指定ID(不是客户指定ID)
					}

					if (i > 0 && BufID[0] != BufID[i] && sameNumID) ///< 是否连号ID
					{
						sameNumID = FALSE;
					}
				}
				Len++;

				if (BufID[i] == BufIDDest[i] && sameID)
				{
					CntSame++;
				}
				else
				{
					sameID = FALSE;
				}
			}

			if (0!=BufIDDest[i])
			{
				if (specificIDDest)
				{
					if ('0' == BufIDDest[i])
					{
						CntZeroDest++;
					}
					else if ('0'!=BufIDDest[i] && 0!=CntZeroDest)
					{
						specificIDDest = FALSE; ///< 不是指定ID(不是客户指定ID)
					}

					if (i > 0 && BufIDDest[0] != BufIDDest[i] && sameNumIDDest) ///< 是否连号ID
					{
						sameNumIDDest = FALSE;
					}
				}
				LenDest++;
			}
		}

		char BufFromID[12] = {0};
		int  LenFromID     = 0;

		sprintf(BufFromID,"%ld",NomalIDFrom);
		LenFromID = strlen(BufFromID);
		if (LenFromID != LenDest) ///< 长度不够不是指定ID
		{
			specificIDDest = FALSE;
		}

		if (LenFromID != Len) ///< 长度不够不是指定ID
		{
			specificID = FALSE;
		}

		if (sameNumID && sameNumIDDest) ///< 两个ID都是连号
		{
		//	if (Len>=LenDest && LenDest>4)
			if (Len>4 && LenDest>4) ///< 任意连号ID间可以相互转账--策划 张
			{
				return 2;
			}
		}
		else if (sameNumID && Len>4 && !sameNumIDDest && specificIDDest) ///< 自己ID是连号，对方ID是指定账号
		{
			if (CntZeroDest < Len-1)
			{
				if (2 == Len-CntZeroDest)
				{
					if ('4' == BufIDDest[LenFromID-CntZeroDest-1])
					{
						return 0;
					}
				}

				if (CntZeroDest>0 || '4'==BufIDDest[LenFromID-1])
					return 2;
				else 
					return 1; ///< 与卒转账扣税
			}
			else
			{
				return 0;
			}
		}
		else if (sameNumIDDest && LenDest>4 && !sameNumID && specificID) ///< 对方是连号ID，自己是指定账号
		{
			if (CntZero < LenDest-1)
			{
				if (2 == LenDest-CntZero)
				{
					if ('4' == BufID[LenFromID-CntZero-1]) ///< 你是副职，与连号ID同等级
					{
						return 0;
					}
				}

				if (CntZero>0 || '4'==BufID[LenFromID-1])
					return 2;
				else
					return 2;
			}
			else ///< 对方级别小于您的级别，不能转账。
			{
				return 0;
			}
		}

		if (specificID && specificIDDest)
		{
			for (int i=Len-1; i>=CntSame; i--)
			{
				if ((BufID[i] != BufIDDest[i]) && CntSame == i)
				{
					if ('0' == BufID[i])///< 正职
					{
						if ('4' == BufIDDest[i]) ///< 正职向副职转账
						{
							return 2;
						}
						else ///< 上下级
						{
							if (CntZeroDest > 0) ///< 炮级以上转账
								return 2;
							else
								return 1;
						}
					}
					else if ('4' == BufID[i]) ///< 副职
					{
						if ('0' == BufIDDest[i]) ///< 副职向正职转账，不扣税
						{
							return 2;
						}
						else ///< 副职向下级
						{
							if (CntZeroDest > 0) ///< 炮级以上转账
								return 2;
							else
								return 1;
						}
					}
					else
					{
						if ('0' == BufIDDest[i] || '4' == BufIDDest[i]) ///< 向上级转账
						{
							if (CntZero > 0) ///< 炮级以上转账
								return 2;
							else
								return 2; ///< 卒给炮转账不扣税 2010 5.7 修改需求;
						}
						else ///< 平级
						{
							if (Len-1 - CntSame) ///< 炮以上平级转账
							{
								return 2;
							}
							else
							{
								return 0;
							}
						}
					}
				}
				else if (BufID[i] != BufIDDest[i] && CntSame != i)///< 不是同一上级
				{
					return 0;
				}
				else if (BufID[i]==BufIDDest[i] && '0'!=BufID[i])///< 不是同一上级
				{
					return 0;
				}
			}
		}
		else if (!specificID && !specificIDDest && specificID && specificIDDest)
		{
			return 3; ///< 此两个非特别ID可转账。
		}
		else ///< 不能转账。
		{
			return 0;
		}
	}

	///< 新需求散户跟连号之间转账
	int RetUserID;
	int RetUserIDDest;
	RetUserID = GetIDInformation(UserID,NomalIDFrom,NomalIDEnd);
	RetUserIDDest = GetIDInformation(UserIDDest,NomalIDFrom,NomalIDEnd);

	if (-1==RetUserID && RetUserIDDest<-1) ///< 散户给连号转账
	{
		return 2; ///< 不扣税
	}
	else if (RetUserID<-1 && -1==RetUserIDDest) ///< 连号给散户转账
	{
		return 1; ///< 扣税
	}

	return 0;
}

/// 获取玩家ID信息，是否散户，特别账号，指派账号
/// @param UserID 玩家ID,NomalIDFrom散户ID开始，NomalIDEnd散户ID结束
/// @return 0到10是指派账号后面的0的个数，-1为散户，-2为无效账号，-5为5连号，-6为6连号
int CGameMainManage::GetIDInformation(long UserID,long NomalIDFrom,long NomalIDEnd)
{
	char BufID[12]     = {0};
	int CntZero        = 0;   ///< 0的个数
	int len            = 0;
	BOOL sameNumID     = TRUE;  ///< 是否连号ID，11111 444444。自己的ID号

	if (UserID>NomalIDFrom && UserID<NomalIDEnd) ///< 散户
	{
		return -1;
	}

	if (UserID < NomalIDFrom)
	{
		sprintf(BufID,"%ld",UserID);

		for (int i=0; 0!=BufID[i]; i++)
		{
			if ('0'==BufID[i])
			{
				CntZero++;
			}
			else if ('0'!=BufID[i] && 0!=CntZero) ///< 无效账号
			{
				return -2;
			}
			
			if (i>0 && BufID[0]!=BufID[i] && sameNumID)
			{
				sameNumID = FALSE;
			}
			len++;
		}

		if (sameNumID) ///< 是连号ID
		{
			if (len >= 5)
				return -len;
			else
				return -2; ///< 无效账号
		}

		char BufIDFrom[12] = {0};
		sprintf(BufIDFrom,"%ld",NomalIDFrom);
		if (strlen(BufIDFrom) == len)
		{
			if ('4' == BufID[len-CntZero-1]) ///< 副职
			{
				return CntZero+1; ///< 4在倒数第CntZero+1位
			}
			return CntZero;
		}
		else
			return -2; ///< 无效账号
	}

	return -2; ///< 无效账号
}

/// 区取某张游戏桌的指针
const CGameDesk* CGameMainManage::GetGameDesk(BYTE bDeskNo)
{
	if (bDeskNo >= m_uDeskCount || m_uDeskCount < 0)
	{
		return NULL;
	}

	return *(m_pDesk + bDeskNo);
}

/// 打印房间所有玩家的消息
/// Added By JianGuankun 2011.12.5
void CGameMainManage::PrintUsersLog()
{
	CString strPath = CBcfFile::GetAppPath();
	//f.open("");

	TCHAR szPathName[MAX_PATH];
	TCHAR szPath[MAX_PATH];
	TCHAR szName[MAX_PATH];
	SYSTEMTIME t;

	memset(szPathName,NULL,MAX_PATH);
	::GetCurrentDirectory(MAX_PATH,szPath);
	::strcat(szPath,"\\wlogs");
	::CreateDirectory(szPath,NULL);

	::GetLocalTime(&t);

	//形成日志ID
	wsprintf(szName,TEXT("%03d_%02d%02d_%02d%02d%02d"),m_InitData.uRoomID,
		t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond);


	wsprintf(szPathName,TEXT("%s\\%s.log"),szPath,szName);

	CFile f;
	if (!f.Open(szPathName,CFile::modeReadWrite|CFile::modeCreate|CFile::modeNoTruncate))
	{
		return;
	}

	//输出日志头
	CString strOutput;
	strOutput.Format("日志: 房间ID = %d 时间 = %02d.%02d %02d:%02d:%02d"
		,m_InitData.uRoomID,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond);
	Writeline(&f,strOutput);

	Writeline(&f,"===============================================================================================================");

	Writeline(&f,"");

	strOutput.Format("总在线人数：%d",m_UserManage.m_uOnLineCount);
	Writeline(&f,strOutput);

	Writeline(&f,"");

	CAFCPtrArray<CGameUserInfo>* pUserList = &m_UserManage.m_OnLineUserInfo;

	strOutput.Format("--在线玩家列表：--------------------------------------------------------------------------------------------------------",
		pUserList->GetPermitCount());
	Writeline(&f,strOutput);

	Writeline(&f,"");

	int uFindCount = 0;

	for (int i = 0; i < pUserList->GetPermitCount(); ++i)
	{
		CGameUserInfo* pUserInfo = pUserList->GetArrayItem(i);
		if (pUserInfo)
		{
			strOutput.Format("  ID:%-9d Name:%-20s NickName:%-30s Access = %1d Network = (%05d,%05d) Station = (%03d,%03d,%03d)",
				pUserInfo->m_UserData.dwUserID,pUserInfo->m_UserData.szName,pUserInfo->m_UserData.nickName,pUserInfo->m_bAccess,
				pUserInfo->m_uSocketIndex,pUserInfo->m_dwHandleID,
				pUserInfo->m_UserData.bDeskNO,pUserInfo->m_UserData.bDeskStation,pUserInfo->m_UserData.bUserState);
			Writeline(&f,strOutput);
			if (pUserInfo->IsAccess())
			{
				uFindCount++;
			}
		}
	}

	Writeline(&f,"");
	strOutput.Format("--(共%d人)--",uFindCount);
	Writeline(&f,strOutput);
	Writeline(&f,"");

	pUserList = &m_UserManage.m_NetCutUserInfo;

	strOutput.Format("--离线玩家列表：--------------------------------------------------------------------------------------------------------",
		pUserList->GetPermitCount());
	Writeline(&f,strOutput);

	Writeline(&f,"");

	uFindCount = 0;

	for (int i = 0; i < pUserList->GetPermitCount(); ++i)
	{
		CGameUserInfo* pUserInfo = pUserList->GetArrayItem(i);
		if (pUserInfo)
		{
			strOutput.Format("  ID:%-9d Name:%-20s NickName:%-30s Access = %1d Network = (%05d,%05d) Station = (%03d,%03d,%03d)",
				pUserInfo->m_UserData.dwUserID,pUserInfo->m_UserData.szName,pUserInfo->m_UserData.nickName,pUserInfo->m_bAccess,
				pUserInfo->m_uSocketIndex,pUserInfo->m_dwHandleID,
				pUserInfo->m_UserData.bDeskNO,pUserInfo->m_UserData.bDeskStation,pUserInfo->m_UserData.bUserState);
			Writeline(&f,strOutput);
			if (pUserInfo->IsAccess())
			{
				uFindCount++;
			}
		}
	}

	Writeline(&f,"");
	strOutput.Format("--(共%d人)--",uFindCount);
	Writeline(&f,strOutput);

	Writeline(&f,"");
	Writeline(&f,"注释：ID,Name,NickName,Access,Network = (SocketIndex,HandleCode),Station = (DeskNO,DeskStation,UserState)");
	Writeline(&f,"");

	f.Close();

	return;
}

//写日志
bool CGameMainManage::Writeline(CFile* p,CString str)
{
	if (p == NULL)
	{
		return false;
	}

	p->Write(str.GetBuffer(),str.GetLength());
	p->Write(_TEXT("\r\n"),2);

	return true;
}


//管理消息处理
bool CGameMainManage::OnManageMessage(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	ASSERT(pNetHead->bMainID==MDM_GR_MANAGE);
	switch (pNetHead->bAssistantID)
	{
	case ASS_GR_ALONE_DESK://封桌
		{	
			if(uSize!=sizeof(MSG_GR_SR_MatchDelete)) return false;
			MSG_GR_SR_MatchDelete *aloneDesk=(MSG_GR_SR_MatchDelete*) pData;

			if (aloneDesk->bDeskIndex >= m_uDeskCount)	return false;

			CGameDesk * pDesk=*(m_pDesk+aloneDesk->bDeskIndex);
			 
			if(aloneDesk->bDeskStation==1) pDesk->VirtualLockDesk();

			//即时封桌功能
			if (pDesk)
			{
				DL_GR_I_ALone_Data aloneDeskData;
				memset(& aloneDeskData,0,sizeof(aloneDeskData));
				//桌子索引号
				aloneDeskData.bDeskIndex = pDesk->m_bDeskIndex;
				//封桌标识
				aloneDeskData.bDeskStation = aloneDesk->bDeskStation;
				//该桌的最多玩家数
				aloneDeskData.bDeskPeople = pDesk->m_bMaxPeople;
				//房间ID号
				aloneDeskData.uRoomID = pDesk->m_pDataManage->m_InitData.uRoomID;
				//写数据库记录
				m_SQLDataManage.PushLine(&aloneDeskData.DataBaseHead,sizeof(aloneDeskData),DTR_GR_ALONEDESK_INTIME,uIndex,dwHandleID);
			}
			m_TCPSocket.SendDataBatch(NULL,0,MDM_GR_DESK,aloneDesk->bDeskIndex,aloneDesk->bDeskStation);
			return true;

		}
	case ASS_GR_DEALONE_DESK://
		{
			if(uSize!=sizeof(MSG_GR_SR_MatchDelete)) return false;
			MSG_GR_SR_MatchDelete *aloneDesk=(MSG_GR_SR_MatchDelete*) pData;

			if (aloneDesk->bDeskIndex >= m_uDeskCount)	return false;
			CGameDesk * pDesk=*(m_pDesk+aloneDesk->bDeskIndex);
			
			if(aloneDesk->bDeskStation==2) pDesk->UnVirtualLockDesk();

			//即时解封桌功能
			if (pDesk)
			{
				DL_GR_I_ALone_Data aloneDeskData;
				memset(& aloneDeskData,0,sizeof(aloneDeskData));
				//桌子索引号
				aloneDeskData.bDeskIndex = pDesk->m_bDeskIndex;
				//封桌标识
				aloneDeskData.bDeskStation = aloneDesk->bDeskStation;
				//该桌的最多玩家数,解封桌时，人数为负
				aloneDeskData.bDeskPeople =  pDesk->m_bMaxPeople;

				aloneDeskData.bDeskPeople = -aloneDeskData.bDeskPeople;
				//房间ID号
				aloneDeskData.uRoomID = pDesk->m_pDataManage->m_InitData.uRoomID;
				//写数据库记录
				m_SQLDataManage.PushLine(&aloneDeskData.DataBaseHead,sizeof(aloneDeskData),DTR_GR_ALONEDESK_INTIME,uIndex,dwHandleID);
			}

			m_TCPSocket.SendDataBatch(NULL,0,MDM_GR_DESK,aloneDesk->bDeskIndex,aloneDesk->bDeskStation);
			return true;
		}
	case ASS_GR_WARN_MSG:			//警告消息
		{
			//效验数据
			MSG_GR_SR_Warning * pWarningUser=(MSG_GR_SR_Warning *)pData;
			if ((uSize<=(sizeof(MSG_GR_SR_Warning)-sizeof(pWarningUser->szWarnMsg)))||(uSize>sizeof(MSG_GR_SR_Warning))) return false;

			//处理数据
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL) return false;
			CGameUserInfo * pTargetUserInfo=m_UserManage.FindOnLineUser(pWarningUser->dwTargetID);
			if (pTargetUserInfo==NULL) return true;
			pWarningUser->szWarnMsg[pWarningUser->uLength]=0;

			//发送数据
			m_TCPSocket.SendData(pTargetUserInfo->m_uSocketIndex,pWarningUser,uSize,MDM_GR_MANAGE,ASS_GR_WARN_MSG,0,pTargetUserInfo->m_dwHandleID);

			//写入数据库
			DL_GR_I_Warn_User_Record WarningUserRecord;
			memset(&WarningUserRecord,0,sizeof(WarningUserRecord));
			WarningUserRecord.dwGMID=pUserInfo->m_UserData.dwUserID;
			WarningUserRecord.dwTargetID=pWarningUser->dwTargetID;
			lstrcpy(WarningUserRecord.szWarnMsg,pWarningUser->szWarnMsg);
			m_SQLDataManage.PushLine(&WarningUserRecord.DataBaseHead,sizeof(WarningUserRecord),DTK_GR_GM_WARNING_USER_RECORD,uIndex,dwHandleID);
			return true;
		}
	case ASS_GR_CUT_USER:			//踢用户下线//安全解除用户游戏
		{
			//效验数据
			if (uSize!=sizeof(MSG_GR_S_CutUser)) return false;
			MSG_GR_S_CutUser * pCutUser=(MSG_GR_S_CutUser *)pData;

			//处理数据
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL) return false;
			CGameUserInfo * pTargetUserInfo=m_UserManage.FindOnLineUser(pCutUser->dwTargetID);
			if (pTargetUserInfo==NULL) return true;

			//发送数据
			pCutUser->dwUserID=pUserInfo->m_UserData.dwUserID;
			if(pNetHead->bHandleCode==0)//踢用户下线
				m_TCPSocket.SendData(pTargetUserInfo->m_uSocketIndex,MDM_GR_MANAGE,ASS_GR_CUT_USER,0,pTargetUserInfo->m_dwHandleID);
			else if(pNetHead->bHandleCode==1)//安全解除用户游戏
				m_TCPSocket.SendData(pTargetUserInfo->m_uSocketIndex,MDM_GR_MANAGE,ASS_GR_CUT_USER,1,pTargetUserInfo->m_dwHandleID);

			//写入数据库
			DL_GR_I_Kick_User_Record DT_KickUserRecord;
			memset(&DT_KickUserRecord,0,sizeof(DT_KickUserRecord));
			lstrcpy(DT_KickUserRecord.szTargetUserName,pTargetUserInfo->m_UserData.szName);
			DT_KickUserRecord.GMID=pUserInfo->m_UserData.dwUserID;
			m_SQLDataManage.PushLine(&DT_KickUserRecord.DataBaseHead,sizeof(DT_KickUserRecord),DTK_GR_GM_KICK_USER_RECORD,uIndex,dwHandleID);
			return true;
		}
	case ASS_GR_CALL_GM://呼叫GM
		{
			//效验数据
			if (uSize!=sizeof(MSG_GR_Call_GM)) return false;
			MSG_GR_Call_GM * pcallGM=(MSG_GR_Call_GM *)pData;

			//处理数据
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL) return false;
			DL_GR_I_CallGM dl_callGM;
			dl_callGM.dwUserID=pUserInfo->m_UserData.dwUserID;
			lstrcpy(dl_callGM.szName,pUserInfo->m_UserData.szName);
			dl_callGM.uDeskNO=pUserInfo->m_UserData.bDeskNO;
			dl_callGM.uDeskStation=pUserInfo->m_UserData.bDeskStation;
			dl_callGM.bCallReason=pcallGM->bCallReason;
			dl_callGM.szMsg[pcallGM->uLength]=0;
			lstrcpy(dl_callGM.szMsg,pcallGM->szMsg);
			m_SQLDataManage.PushLine(&dl_callGM.DataBaseHead,sizeof(dl_callGM),DTK_GR_CALL_GM,uIndex,dwHandleID);
			return true;
		}
	case ASS_GR_GRM_SET://房间管理窗口数据修改消息
		{
			//效验数据
			if (uSize!=sizeof(MSG_GR_GRM_Set)) 
				return false;
			MSG_GR_GRM_Set * pCmd = (MSG_GR_GRM_Set *)pData;

			//房间管理窗口数据
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL) return false;

			if (!IsExistGRMUserID(pCmd->dwUserID) || !IsExistGRMRoom(m_uNameID))
			{
				//发送结果消息给客户端
				m_TCPSocket.SendData(uIndex,pCmd,sizeof(MSG_GR_GRM_Set),MDM_GR_MANAGE,ASS_GR_GRM_SET_RES,RES_GR_GRM_SET_RES_ERR3,0);
				return true;
			}

			//根据数据进行修改
			//奖池区域
			CString s = CINIFile::GetAppPath ();/////本地路径
			CString temp = _T("");
			CString key;
			key.Format("%s",GRM_SET_KEY);
			temp.Format("%s%d_s.ini",s,m_uNameID);
			CINIFile f(temp);
			CString sTemp;

			bool bResult_Success = RES_GR_GRM_SET_RES_SUC;//操作结果

			if (pCmd->bAIWinAndLostAutoCtrl)//控制奖池
			{
				//通知游戏服务端修改
				//通知本房间所有游戏服务端修改

				for (UINT i=0;i<m_uDeskCount;i++)
				{
					if (!(*(m_pDesk+i))->SetRoomPond(1)
						|| !(*(m_pDesk+i))->SetRoomPondEx(pCmd->iAIWantWinMoney, pCmd->iAIWinLuckyAt))
					{
						//发送结果消息给客户端
						m_TCPSocket.SendData(uIndex,pCmd,sizeof(MSG_GR_GRM_Set),MDM_GR_MANAGE,ASS_GR_GRM_SET_RES,RES_GR_GRM_SET_RES_ERR1,0);
						return true;
					}
				}


				//将【机器人控制输赢开关】写入配置文件当中
				sTemp.Format("%d",pCmd->bAIWinAndLostAutoCtrl);
				f.SetKeyValString(key,"AIWinAndLostAutoCtrl",sTemp);

				//将【重置机器人已经赢钱的数目】写入配置文件当中
				sTemp.Format("%I64d",pCmd->iReSetAIHaveWinMoney);
				f.SetKeyValString(key,"ReSetAIHaveWinMoney",sTemp);

				//将【机器人赢钱区域1,2,3 4所用断点】写入配置文件当中
				sTemp.Format("%I64d",pCmd->iAIWantWinMoney[0]);
				f.SetKeyValString(key,"AIWantWinMoneyA1",sTemp);
				sTemp.Format("%I64d",pCmd->iAIWantWinMoney[1]);
				f.SetKeyValString(key,"AIWantWinMoneyA2",sTemp);
				sTemp.Format("%I64d",pCmd->iAIWantWinMoney[2]);
				f.SetKeyValString(key,"AIWantWinMoneyA3",sTemp);

				//将【机器人在区域1,2,3,4赢钱的概率】写入配置文件当中
				sTemp.Format("%d",pCmd->iAIWinLuckyAt[0]);
				f.SetKeyValString(key,"AIWinLuckyAtA1",sTemp);
				sTemp.Format("%d",pCmd->iAIWinLuckyAt[1]);
				f.SetKeyValString(key,"AIWinLuckyAtA2",sTemp);
				sTemp.Format("%d",pCmd->iAIWinLuckyAt[2]);
				f.SetKeyValString(key,"AIWinLuckyAtA3",sTemp);
				sTemp.Format("%d",pCmd->iAIWinLuckyAt[3]);
				f.SetKeyValString(key,"AIWinLuckyAtA4",sTemp);
			}
			else//不控制奖池
			{
				//通知本房间所有游戏服务端修改
				for (UINT i=0;i<m_uDeskCount;i++)
				{
					if (!(*(m_pDesk+i))->SetRoomPond(0))
					{
						//发送结果消息给客户端
						m_TCPSocket.SendData(uIndex,pCmd,sizeof(MSG_GR_GRM_Set),MDM_GR_MANAGE,ASS_GR_GRM_SET_RES,RES_GR_GRM_SET_RES_ERR1,0);
						return true;
					}
				}

				//将【机器人控制输赢开关】写入配置文件当中
				sTemp.Format("%d",pCmd->bAIWinAndLostAutoCtrl);
				f.SetKeyValString(key,"AIWinAndLostAutoCtrl",sTemp);
			}

			//玩家输赢概率区域
			if (pCmd->bWinProbCtrl)
			{
				//游戏开始的时候游戏会读取配置文件所以此处无需通知游戏
				CString sTemp2;
				key.Format("%s",GRM_SET_KEY_WIN);
				sTemp.Format("%d",20);
				f.SetKeyValString(key,"count",sTemp);
				for (int i = 0; i < 20; ++i)
				{
					if (pCmd->dwUserID_win[i] <= 0)
					{
						//将【机器人控制输赢开关】写入配置文件当中
						sTemp.Format("%ld",0);
						sTemp2.Format("UserID%d",i);
						f.SetKeyValString(key,sTemp2,sTemp);
						sTemp.Format("%d",0);
						sTemp2.Format("prob%d",i);
						f.SetKeyValString(key,sTemp2,sTemp);
					}
					else
					{
						//将【机器人控制输赢开关】写入配置文件当中
						sTemp.Format("%ld",pCmd->dwUserID_win[i]);
						sTemp2.Format("UserID%d",i);
						f.SetKeyValString(key,sTemp2,sTemp);
						sTemp.Format("%d",pCmd->iProb_win[i]);
						sTemp2.Format("prob%d",i);
						f.SetKeyValString(key,sTemp2,sTemp);
					}
				}
				key.Format("%s",GRM_SET_KEY_LOS);
				sTemp.Format("%d",20);
				f.SetKeyValString(key,"count",sTemp);
				for (int i = 0; i < 20; ++i)
				{
					if (pCmd->dwUserID_los[i] <= 0)
					{
						//将【机器人控制输赢开关】写入配置文件当中
						sTemp.Format("%ld",0);
						sTemp2.Format("UserID%d",i);
						f.SetKeyValString(key,sTemp2,sTemp);
						sTemp.Format("%d",0);
						sTemp2.Format("prob%d",i);
						f.SetKeyValString(key,sTemp2,sTemp);
					}
					else
					{
						//将【机器人控制输赢开关】写入配置文件当中
						sTemp.Format("%ld",pCmd->dwUserID_los[i]);
						sTemp2.Format("UserID%d",i);
						f.SetKeyValString(key,sTemp2,sTemp);
						sTemp.Format("%d",pCmd->iProb_los[i]);
						sTemp2.Format("prob%d",i);
						f.SetKeyValString(key,sTemp2,sTemp);
					}
				}
			}
			else
			{
				key.Format("%s",GRM_SET_KEY_WIN);
				sTemp.Format("%d",0);
				f.SetKeyValString(key,"count",sTemp);
				key.Format("%s",GRM_SET_KEY_LOS);
				sTemp.Format("%d",0);
				f.SetKeyValString(key,"count",sTemp);
			}

			//发送结果消息给客户端
			m_TCPSocket.SendData(uIndex,pCmd,sizeof(MSG_GR_GRM_Set),MDM_GR_MANAGE,ASS_GR_GRM_SET_RES,bResult_Success,0);

			return true;
		}
	case ASS_GR_GRM_UPDATA:
		{
			SendRoomPondUpDataALL();
			return true;
		}
	}
	return false;
}

//通知管理员客户端奖池已经更新
void CGameMainManage::SendRoomPondUpData(__int64 iAIHaveWinMoney)
{
	MSG_GR_GRM_UpData cmd;
	cmd.iAIHaveWinMoney = iAIHaveWinMoney;

	for (int i = 0; i < m_UserManage.m_OnLineUserInfo.GetPermitCount(); ++i)
	{
		CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(i);
		if (pUserInfo == NULL) 
			continue;

		if (IsExistGRMUserID(pUserInfo->m_UserData.dwUserID))
		{
			//发送奖池更新消息给客户端
			m_TCPSocket.SendData(pUserInfo->m_uSocketIndex, &cmd,sizeof(MSG_GR_GRM_UpData),MDM_GR_MANAGE, ASS_GR_GRM_UPDATA, RES_GR_GRM_UP1, 0);
		}
	}

	return;
}

//通知管理员客户端完整更新
void CGameMainManage::SendRoomPondUpDataALL()
{
	MSG_GR_GRM_UpData cmd;
	CString s = CINIFile::GetAppPath ();/////本地路径
	CString temp;
	temp.Format("%s%d_s.ini",s,m_uNameID);
	CINIFile f( temp);
	CString key;
	key.Format("%s",GRM_SET_KEY);
	cmd.bAIWinAndLostAutoCtrl = f.GetKeyVal(key,"AIWinAndLostAutoCtrl",0);
	cmd.iAIHaveWinMoney = f.GetKeyVal(key,"AIHaveWinMoney ",__int64(0));
	cmd.iReSetAIHaveWinMoney	= f.GetKeyVal(key,"ReSetAIHaveWinMoney ",__int64(0)); 

	cmd.iAIWantWinMoney[0]	= f.GetKeyVal(key,"AIWantWinMoneyA1 ",__int64(500000));
	cmd.iAIWantWinMoney[1]	= f.GetKeyVal(key,"AIWantWinMoneyA2 ",__int64(5000000));
	cmd.iAIWantWinMoney[2]	= f.GetKeyVal(key,"AIWantWinMoneyA3 ",__int64(50000000));
	cmd.iAIWinLuckyAt[0]	= f.GetKeyVal(key,"AIWinLuckyAtA1 ",90);
	cmd.iAIWinLuckyAt[1]	= f.GetKeyVal(key,"AIWinLuckyAtA2 ",70);
	cmd.iAIWinLuckyAt[2]	= f.GetKeyVal(key,"AIWinLuckyAtA3 ",50);
	cmd.iAIWinLuckyAt[3]	= f.GetKeyVal(key,"AIWinLuckyAtA4 ",30);

	//输赢概率控制
	cmd.bWinProbCtrl = false;
	key.Format("%s",GRM_SET_KEY_WIN);//赢玩家统计
	int count = f.GetKeyVal(key,"count",0);
	if(count > 0 )
	{		
		cmd.bWinProbCtrl = true;
	}

	TCHAR szName[21];
	int probability = 0;
	long int dwUserID = 0;			///ID 号码

	for(int i = 0; i < 20; i ++)
	{
		wsprintf(szName,"UserID%d",i);
		cmd.dwUserID_win[i] = f.GetKeyVal(key,szName,0);

		wsprintf(szName,"prob%d",i);
		cmd.iProb_win[i] = f.GetKeyVal(key,szName,0);
	}

	key.Format("%s",GRM_SET_KEY_LOS);//输玩家统计
	count = f.GetKeyVal(key,"count",0);
	if(count > 0 )
	{
		cmd.bWinProbCtrl = true;
	}

	probability = 0;
	dwUserID = 0;			///ID 号码

	for(int i = 0; i < 20; i ++)
	{
		wsprintf(szName,"UserID%d",i);
		cmd.dwUserID_los[i] = f.GetKeyVal(key,szName,0);

		wsprintf(szName,"prob%d",i);
		cmd.iProb_los[i] = f.GetKeyVal(key,szName,0);
	}

	for (int i = 0; i < m_UserManage.m_OnLineUserInfo.GetPermitCount(); ++i)
	{
		CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(i);
		if (pUserInfo == NULL) 
			continue;

		if (IsExistGRMUserID(pUserInfo->m_UserData.dwUserID))
		{
			//发送奖池更新消息给客户端
			m_TCPSocket.SendData(pUserInfo->m_uSocketIndex, &cmd,sizeof(MSG_GR_GRM_UpData),MDM_GR_MANAGE, ASS_GR_GRM_UPDATA, RES_GR_GRM_UP2, 0);
		}
	}

	return;
}

//经验盒处理
bool CGameMainManage::OnPointMessage(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	switch (pNetHead->bAssistantID)
	{
	case ASS_GR_OPEN_POINT:
		{
			DL_GR_I_OpenPoint op;
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL)
				return false;			
			op.dwUserID = pUserInfo->m_UserData.dwUserID;
			m_SQLDataManage.PushLine(&op.DataBaseHead,sizeof(op),DTK_GR_OPEN_POINT,uIndex,dwHandleID);
		}
		return true;
	case ASS_GR_GET_POINT:
		{
			/*DL_GR_I_GetPoint getpoint;
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL)
				return false;	
			if(uSize != sizeof(MSG_GR_S_GetPoint))
				return false;
			MSG_GR_S_GetPoint *pGetPoint = (MSG_GR_S_GetPoint *)pData;
			getpoint.dwUserID = pUserInfo->m_UserData.dwUserID;
			getpoint.dwsrcPoint = pUserInfo->m_UserData.dwPoint;
			getpoint.dwPoint = pGetPoint->dwPoint;
			m_SQLDataManage.PushLine(&getpoint.DataBaseHead,sizeof(getpoint),DTK_GR_GET_POINT,uIndex,dwHandleID);*/
		}
		return true;
	}
	return false;
}

//好友处理
bool CGameMainManage::OnFriendManageMsg(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	CGameUserInfo * pSenderUserInfo = m_UserManage.GetOnLineUserInfo(uIndex);
	if (pSenderUserInfo == NULL) 
		return false;
	return true;
}

//排队处理
bool CGameMainManage::OnQueueMsg(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	ASSERT(pNetHead->bMainID == MDM_GR_QUEUE_MSG);
	switch(pNetHead->bAssistantID)//排队消息处理
	{
	case ASS_GR_ADD_QUEUE://加入排队
		MSG_GR_Queue stQueue;
		memset(&stQueue,0,sizeof(stQueue));
		m_TCPSocket.SendData(uIndex,&stQueue,sizeof(stQueue),MDM_GR_QUEUE_MSG,ASS_GR_ADD_QUEUE_RESULT,0,0);
		break;
	case ASS_GR_AGREE_PLAN_SIT://同意坐下(系统安排坐位)
		break;
	case ASS_GR_REFUSE_PLAN_SIT://不同意坐某位置桌
		break;
	}
	return true;
}

bool CGameMainManage::IsContestGameOver()
{
	
	if (!(m_InitData.dwRoomRule & GRR_CONTEST) && !(m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
		return false;
	//如果还没开始比赛，则不存在是否结束比赛的说法
	
	if (1 != m_InitData.iRoomState)
		return false;
	
	int iRemainPeople = 0;
	if (m_pIAutoAllotDesk)
	{
		iRemainPeople = m_pIAutoAllotDesk->GetQueueUserCount();
		
	}
	//判断比赛是否结束的条件是，当比赛队列中的人数少于指定人数时，比赛结束
	if (iRemainPeople <= m_InitData.iLeasePeople)
	{
		m_ContestTimeover = true;
	}
	if (m_ContestTimeover)
	{
		KillTimer(IDT_QUEUE_GAME);
		AutoAllotDeskDeleteAll();
		//判断是否有用户在游戏中
		int nDeskNo = 0;
		CGameDesk* pGameDesk = NULL;
		bool bResetState = true;
		while(nDeskNo < m_InitData.uDeskCount)
		{
			pGameDesk=*(m_pDesk+nDeskNo++);
			if (pGameDesk != NULL && pGameDesk->IsPlayingByGameStation())
			{
				bResetState = false;
				break;
			}
		}
		if (bResetState)
		{		
			m_InitData.iRoomState = 0;
			//向数据库发送比赛结束消息，处理最后排名
			DL_GR_I_UserContestRank _p;//此参数无意义
			m_SQLDataManage.PushLine(&_p.DataBaseHead,sizeof(DL_GR_I_UserContestRank),DTK_GR_CONTEST_GAME_OVER,0,0);

			m_ContestTimeover = false;
		}
		return true;
	}
	if (0 == m_iResetTime)
	{//在m_iQueueTime指定的时间内，如果有其他桌子结束比赛，则放到同一个时钟里配桌，配桌之后重置该判断条件，供下一次使用
		m_iResetTime = 1;
		
		SetTimer(IDT_QUEUE_GAME, m_iQueueTime*1000);
		
	}
	return false;
}

bool CGameMainManage::IsLeaseTimeOver()
{
	if (m_InitData.iLeaseID != 0)
	{
		CTime _t = CTime::GetCurrentTime();
		if (_t.GetTime() > m_InitData.i64LeaseTimeEnd) return true;
	}
	return false;
}

///排队机处理
void CGameMainManage::queuerGame()
{
    static int* pArray = NULL;
		
	if (IsLeaseTimeOver()) return;

    // PengJiLin, 2010-5-20
    if(m_pGameUserInfoList.size() < m_uQueuerMaxPeople) return;    
    if(NULL == pArray) pArray = new int [m_uQueuerMaxPeople];

	int iDeskIndex = -1;						//桌子索引
	int iDeskStation = 0;						//座位索引
	int iADeskPlayerNum = 0;					//桌子中的人数
	bool isGameing = false;						//此桌是否开始游戏了
	//bool isADeskGameBegin = false;				//是否有桌子排队成功，并开始游戏了
	BYTE bErrorCode=ERR_GR_ERROR_UNKNOW;
	
	CGameUserInfo* pGameUserInfo = NULL;
	CGameDesk* pGameDesk = NULL;
	
	list<CGameUserInfo*> tempList;				//备份原来的排队list
	list<CGameUserInfo*> sitdownList;			//保存已经坐下的玩家
	list<CGameUserInfo*>::iterator iter1;


	//{增加 rongqiufen 2010.06.29
	//是否排除成功
	bool hasSucceed = false;

	int iDeskPlayerNum = (*m_pDesk)->m_bMaxPeople;
	if(m_pGameUserInfoList.size() >= m_uQueuerPeople + iDeskPlayerNum)
	{
		srand(unsigned(time(0)));
		///每支队伍进行随机加权0-49
		for (iter1 = m_pGameUserInfoList.begin(); iter1 != m_pGameUserInfoList.end(); iter1++)
		{
			if ((*iter1)->m_UserData.bDeskNO == 255) 
			{
				if ((*iter1)->GetWeigth() < MAXLONG - 50) 
				{
					int r = rand() % 50;
					(*iter1)->m_SelectWeight += r;
				}
			}
		}
		{
			///按权值进行排序，权值高的会优先进行搓桌
			m_pGameUserInfoList.sort(compare());
        //增加 rongqiufen}

			//CString s1, s2;
			//s1 = "";
			//s1 = "";
			//for (iter1 = m_pGameUserInfoList.begin(); iter1 != m_pGameUserInfoList.end(); ++iter1)
			//{
			//	s2.Format("(%s, %d)", (*iter1)->m_UserData.szName, (*iter1)->GetWeigth());
			//	s1 += " | " + s2;
			//}
			//ouf<<s1<<endl;

			 // PengJiLin, 2010-5-20, 随机数组只使用一次，第二次或者第三次基本上是随机的
			 // 同时避免不能一起分配桌子的问题
			BOOL bUseOnce = FALSE;
			//查找没有开始游戏的桌子
			while (m_pGameUserInfoList.size() >= m_uQueuerPeople + iDeskPlayerNum && (1 + iDeskIndex) < m_uDeskCount)
			{
				iADeskPlayerNum = 0;
				isGameing = true;
				pGameDesk = NULL;

				++iDeskIndex;
				pGameDesk=*(m_pDesk+iDeskIndex);
				
				if (NULL != pGameDesk)
				{
					isGameing = pGameDesk->IsPlayingByGameStation();
					if (!isGameing)
					{
						//isADeskGameBegin = false;
						//求此桌中有多少人
						iADeskPlayerNum = pGameDesk->GetDeskPlayerNum();

						/// 此桌中人未满，包括，有人和没有人
						if (iADeskPlayerNum < pGameDesk->m_bMaxPeople)
						{
							int iQueueSitDownNum = 0;	//一个桌子中排队坐下的人数					
							tempList = m_pGameUserInfoList;

							// PengJiLin, 2010-5-20
							UINT iCount = 0;

							//一次排队一个桌子
							for (iter1 = tempList.begin(); iter1 != tempList.end(); ++iter1)
							{
								pGameUserInfo = *iter1;						
								iDeskStation = pGameDesk->GetDeskNoPlayerStation();		//获取现在的空座位索引

								if (-1 == iDeskStation)//满人了
								{
									break;
								}

								if (NULL != pGameUserInfo)
								{
									if (pGameUserInfo->m_UserData.bDeskNO == 255)//没有坐到桌子
									{
										//判断桌子中是否有ip相同的玩家
										if (pGameDesk->IsDeskIpSame(pGameUserInfo))
										{
											continue;
										}

										//// PengJiLin, 2010-5-20, 忽略剔除的位置
										//if((pArray[iCount] == -1) && (FALSE == bUseOnce))
										//{
										//	++iCount;
										//	continue;
										//}

										++iQueueSitDownNum;
										pGameUserInfo->m_UserData.bUserState = USER_ARGEE; //USER_SITTING;
										pGameUserInfo->m_UserData.bDeskNO = iDeskIndex;
										pGameUserInfo->m_UserData.bDeskStation=iDeskStation;								
										pGameDesk->SetDeskUserInfo(pGameUserInfo, iDeskStation);

										//从排队机中删除已经分配座位的玩家
										m_pGameUserInfoList.remove(pGameUserInfo);
										//保存已经坐下的玩家
										sitdownList.push_back(pGameUserInfo);
			
										if (iQueueSitDownNum == pGameDesk->m_bMaxPeople)
										{
											break;//满人了
										}
									}
								}
							}//end of for

							// PengJiLin, 2010-5-20
							bUseOnce = TRUE;
							
							//有人坐下了，包括满人、不满人的情况
							if (0 < iQueueSitDownNum)
							{
								//是否可以坐下
								if (pGameDesk->CanBeginGame())
								{
									//pGameDesk->GameBegin(0);
									//CString s;
									//s.Format("%d, %d", IsUserPlayGame(pGameDesk->m_pUserInfo[0]), pGameDesk->m_pUserInfo[0]->m_UserData.bUserState);
									//AfxMessageBox(s);
									//有一个桌子排队成功，开始游戏了，并给client发坐下、游戏开始消息，从sitdownList中取玩家信息
									//发送房间坐下消息
									for (iter1 = sitdownList.begin(); iter1 != sitdownList.end(); ++iter1)
									{
										pGameUserInfo = NULL;
										pGameUserInfo = *iter1;
										if (NULL != pGameUserInfo)
										{
											hasSucceed = true;
											pGameUserInfo->m_SelectWeight = 0;
											MSG_GR_R_UserSit UserSit;
											memset(&UserSit, 0, sizeof(UserSit));
											UserSit.dwUserID = pGameUserInfo->m_UserData.dwUserID;
											UserSit.bDeskIndex = pGameUserInfo->m_UserData.bDeskNO;
											UserSit.bDeskStation = pGameUserInfo->m_UserData.bDeskStation;
											UserSit.bUserState = USER_SITTING;
											m_TCPSocket.SendDataBatch(&UserSit,sizeof(UserSit),MDM_GR_USER_ACTION,ASS_GR_QUEUE_USER_SIT,ERR_GR_SIT_SUCCESS);
										}
									}							
									//end of 给client发消息
									//pGameDesk->GameBegin(0);
								}
								else//此桌排队后不能开始游戏，还原之前设置的一些信息
								{
									for (iter1 = sitdownList.begin(); iter1 != sitdownList.end(); ++iter1)
									{
										pGameUserInfo = NULL;
										pGameUserInfo = *iter1;
										if (NULL != pGameUserInfo)
										{
											pGameDesk->SetDeskUserInfo(NULL, pGameUserInfo->m_UserData.bDeskStation);//清空座位上的玩家信息
											pGameUserInfo->m_UserData.bDeskNO=255;
											pGameUserInfo->m_UserData.bDeskStation=255;
											pGameUserInfo->m_UserData.bUserState=USER_LOOK_STATE;
										}
									}						
									m_pGameUserInfoList.swap(tempList);//排队失败还原的list
								}

								tempList.clear();
								sitdownList.clear();
							}
						}//end of if (iADeskPlayerNum < pGameDesk->m_bMaxPeople)
					}//end of if (!isGameing)
				}//end of if (NULL != pGameDesk)			
			}// end of while (m_pGameUserInfoList.size() > 0 && (1 + iDeskIndex) < m_uDeskCount)
		}
	

		//{增加 rongqiufen 2010.06.29
		///对于排队不成功的进行加权
		if (hasSucceed)
		{
			for (iter1 = m_pGameUserInfoList.begin(); iter1 != m_pGameUserInfoList.end(); iter1++)
			{
				if ((*iter1)->m_UserData.bDeskNO == 255) 
				{
					if ((*iter1)->GetWeigth() < MAXLONG - 50) 
						(*iter1)->m_SelectWeight += 30;
				}
			}
		}
       //增加 rongqiufen}

		//s1 = "---";
		//for (iter1 = m_pGameUserInfoList.begin(); iter1 != m_pGameUserInfoList.end(); ++iter1)
		//{
		//	s2.Format("%s, %d", (*iter1)->m_UserData.szName, (*iter1)->GetWeigth());
		//	s1 += " | " + s2;
		//}
		//ouf<<s1<<endl;
	}

/*

	
    // PengJiLin, 2010-5-20, 将排队机位置随机剔除
    memset(pArray, 0, m_uQueuerMaxPeople);
    if(m_uQueuerMaxPeople > m_KernelData.uDeskPeople)
    {
        int iCount = m_uQueuerMaxPeople - m_KernelData.uDeskPeople;
        for(int i = 0; i < iCount; ++i)
        {
            UINT uIndex = (UINT)rand() % m_uQueuerMaxPeople;

            if(pArray[uIndex] == 0) pArray[uIndex] = -1;
            else
            {
                for(int i = 0; i < m_uQueuerMaxPeople; ++i)
                {
                    if(pArray[i] == 0)
                    {
                        pArray[i] = -1;
                        break;
                    }
                }
            }
        }
    }

    // PengJiLin, 2010-5-20, 随机数组只使用一次，第二次或者第三次基本上是随机的
    // 同时避免不能一起分配桌子的问题
    BOOL bUseOnce = FALSE;

	//查找没有开始游戏的桌子
	while (m_pGameUserInfoList.size() > 0 && (1 + iDeskIndex) < m_uDeskCount)
	{
		iADeskPlayerNum = 0;
		isGameing = true;
		pGameDesk = NULL;

		++iDeskIndex;
		pGameDesk=*(m_pDesk+iDeskIndex);
		
		if (NULL != pGameDesk)
		{
			isGameing = pGameDesk->IsPlayingByGameStation();
			if (!isGameing)
			{
				//isADeskGameBegin = false;
				//求此桌中有多少人
				iADeskPlayerNum = pGameDesk->GetDeskPlayerNum();

				/// 此桌中人未满，包括，有人和没有人
				if (iADeskPlayerNum < pGameDesk->m_bMaxPeople)
				{
					int iQueueSitDownNum = 0;	//一个桌子中排队坐下的人数					
					tempList = m_pGameUserInfoList;

                    // PengJiLin, 2010-5-20
                    UINT iCount = 0;

					//一次排队一个桌子
					for (iter1 = tempList.begin(); iter1 != tempList.end(); ++iter1)
					{
						pGameUserInfo = *iter1;						
						iDeskStation = pGameDesk->GetDeskNoPlayerStation();		//获取现在的空座位索引

						if (-1 == iDeskStation)//满人了
						{
							break;
						}

						if (NULL != pGameUserInfo)
						{
							if (pGameUserInfo->m_UserData.bDeskNO == 255)//没有坐到桌子
							{
								//判断桌子中是否有ip相同的玩家
								if (pGameDesk->IsDeskIpSame(pGameUserInfo))
								{
									continue;
								}

                                // PengJiLin, 2010-5-20, 忽略剔除的位置
                                if((pArray[iCount] == -1) && (FALSE == bUseOnce))
                                {
                                    ++iCount;
                                    continue;
                                }

								++iQueueSitDownNum;
								pGameUserInfo->m_UserData.bUserState = USER_ARGEE; //USER_SITTING;
								pGameUserInfo->m_UserData.bDeskNO = iDeskIndex;
								pGameUserInfo->m_UserData.bDeskStation=iDeskStation;								
								pGameDesk->SetDeskUserInfo(pGameUserInfo, iDeskStation);

								//从排队机中删除已经分配座位的玩家
								m_pGameUserInfoList.remove(pGameUserInfo);
								//保存已经坐下的玩家
								sitdownList.push_back(pGameUserInfo);
	
								if (iQueueSitDownNum == pGameDesk->m_bMaxPeople)
								{
									break;//满人了
								}
							}
						}
					}//end of for

                    // PengJiLin, 2010-5-20
                    bUseOnce = TRUE;
					
					//有人坐下了，包括满人、不满人的情况
					if (0 < iQueueSitDownNum)
					{
						//是否可以坐下
						if (pGameDesk->CanBeginGame())
						{
							//有一个桌子排队成功，开始游戏了，并给client发坐下、游戏开始消息，从sitdownList中取玩家信息
							//发送房间坐下消息
							for (iter1 = sitdownList.begin(); iter1 != sitdownList.end(); ++iter1)
							{
								pGameUserInfo = NULL;
								pGameUserInfo = *iter1;
								if (NULL != pGameUserInfo)
								{
									MSG_GR_R_UserSit UserSit;
									memset(&UserSit, 0, sizeof(UserSit));
									UserSit.dwUserID=pGameUserInfo->m_UserData.dwUserID;
									UserSit.bDeskIndex=pGameUserInfo->m_UserData.bDeskNO;
									UserSit.bDeskStation=pGameUserInfo->m_UserData.bDeskStation;
									UserSit.bUserState=USER_SITTING;
									m_TCPSocket.SendDataBatch(&UserSit,sizeof(UserSit),MDM_GR_USER_ACTION,ASS_GR_QUEUE_USER_SIT,ERR_GR_SIT_SUCCESS);
								}
							}							
							//end of 给client发消息
							//pGameDesk->GameBegin(0);
						}
						else//此桌排队后不能开始游戏，还原之前设置的一些信息
						{
							for (iter1 = sitdownList.begin(); iter1 != sitdownList.end(); ++iter1)
							{
								pGameUserInfo = NULL;
								pGameUserInfo = *iter1;
								if (NULL != pGameUserInfo)
								{
									pGameDesk->SetDeskUserInfo(NULL, pGameUserInfo->m_UserData.bDeskStation);//清空座位上的玩家信息
									pGameUserInfo->m_UserData.bDeskNO=255;
									pGameUserInfo->m_UserData.bDeskStation=255;
									pGameUserInfo->m_UserData.bUserState=USER_LOOK_STATE;
								}
							}						
							m_pGameUserInfoList.swap(tempList);//排队失败还原的list
						}

						tempList.clear();
						sitdownList.clear();
					}
				}//end of if (iADeskPlayerNum < pGameDesk->m_bMaxPeople)
			}//end of if (!isGameing)
		}//end of if (NULL != pGameDesk)			
	}// end of while (m_pGameUserInfoList.size() > 0 && (1 + iDeskIndex) < m_uDeskCount)
*/	
	return;
}

//用户房间处理
bool CGameMainManage::OnRoomMessage(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	ASSERT(pNetHead->bMainID==MDM_GR_ROOM);
	switch (pNetHead->bAssistantID)
	{
	case ASS_GR_NORMAL_TALK:	//普通聊天
		{
			MSG_GR_RS_NormalTalk * pNormalTalk=(MSG_GR_RS_NormalTalk *)pData;
			
			if(m_InitData.dwRoomRule& GRR_FORBID_ROOM_TALK)//禁止房间聊天
				return false;
			//效验数据 
			if ((uSize<=(sizeof(MSG_GR_RS_NormalTalk)-sizeof(pNormalTalk->szMessage)))||(uSize>sizeof(MSG_GR_RS_NormalTalk))) return false;
			if ((pNormalTalk->iLength+1)!=(uSize+sizeof(pNormalTalk->szMessage)-sizeof(MSG_GR_RS_NormalTalk))) return false;

			//处理数据
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL) return false;

			/// 如果禁言时间有效，则不发送
			CString strTime;
			strTime.Format("%d",CTime::GetCurrentTime());
			int curTime=atoi(strTime);
//			if (pUserInfo->m_UserData.userInfoEx2 > curTime)
			{
	//			return true;
			}

			//群发数据
			pNormalTalk->szMessage[pNormalTalk->iLength]=0;
			pNormalTalk->dwSendID=pUserInfo->m_UserData.dwUserID;
			
			string str_temp(pNormalTalk->szMessage);
			m_HNFilter.censor(str_temp,false);
			strcpy(pNormalTalk->szMessage,str_temp.c_str());
			pNormalTalk->iLength = strlen(pNormalTalk->szMessage) + 1;
			pNormalTalk->szMessage[pNormalTalk->iLength + 1] = 0;
			m_TCPSocket.SendDataBatch(pNormalTalk,sizeof(MSG_GR_RS_NormalTalk)-sizeof(pNormalTalk->szMessage)+pNormalTalk->iLength+1,MDM_GR_ROOM,ASS_GR_NORMAL_TALK,0);

			return true;
		}
	case ASS_GR_SHORT_MSG:		//短信息聊天
		{
			//效验数据
			MSG_GR_RS_NormalTalk * pNormalTalk=(MSG_GR_RS_NormalTalk *)pData;
			if ((uSize<=(sizeof(MSG_GR_RS_NormalTalk)-sizeof(pNormalTalk->szMessage)))||(uSize>sizeof(MSG_GR_RS_NormalTalk))) return false;
			if ((pNormalTalk->iLength+1)!=(uSize+sizeof(pNormalTalk->szMessage)-sizeof(MSG_GR_RS_NormalTalk))) return false;

			//处理数据
			CGameUserInfo * pSendUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			CGameUserInfo * pRecvUserInfo=m_UserManage.FindOnLineUser(pNormalTalk->dwTargetID);
			if ((pSendUserInfo==NULL)||(pRecvUserInfo==NULL)) return true;
			pNormalTalk->szMessage[pNormalTalk->iLength]=0;
			pNormalTalk->dwSendID=pSendUserInfo->m_UserData.dwUserID;

			string str_temp(pNormalTalk->szMessage);
			m_HNFilter.censor(str_temp,false);
			strcpy(pNormalTalk->szMessage,str_temp.c_str());
			pNormalTalk->iLength = strlen(pNormalTalk->szMessage) + 1;
			pNormalTalk->szMessage[pNormalTalk->iLength + 1] = 0;

			//发送数据
			m_TCPSocket.SendData(pSendUserInfo->m_uSocketIndex,pNormalTalk,sizeof(MSG_GR_RS_NormalTalk)-sizeof(pNormalTalk->szMessage)+pNormalTalk->iLength+1,MDM_GR_ROOM,ASS_GR_SHORT_MSG,0,pSendUserInfo->m_dwHandleID);
			m_TCPSocket.SendData(pRecvUserInfo->m_uSocketIndex,pNormalTalk,sizeof(MSG_GR_RS_NormalTalk)-sizeof(pNormalTalk->szMessage)+pNormalTalk->iLength+1,MDM_GR_ROOM,ASS_GR_SHORT_MSG,0,pRecvUserInfo->m_dwHandleID);

			return true;
		}
	case ASS_GR_ROOM_SET:		//房间设置
		{
			//效验数据
			if (uSize!=sizeof(MSG_GR_S_UserSet)) return false;
			MSG_GR_S_UserSet * pUserSetInfo=(MSG_GR_S_UserSet *)pData;

			//处理数据
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL) return false;
			pUserSetInfo->m_Rule.szPass[sizeof(pUserSetInfo->m_Rule.szPass)/sizeof(pUserSetInfo->m_Rule.szPass[0])-1]=0;
			pUserInfo->m_Rule=pUserSetInfo->m_Rule;

			return true;
		}
	case ASS_GR_INVITEUSER:		//邀请用户
		{
			//效验数据
			if (uSize!=sizeof(MSG_GR_SR_InviteUser)) return false;
			MSG_GR_SR_InviteUser * pInviteUser=(MSG_GR_SR_InviteUser *)pData;

			//处理数据
			CGameUserInfo * pSendUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			CGameUserInfo * pRecvUserInfo=m_UserManage.FindOnLineUser(pInviteUser->dwTargetID);
			if ((pSendUserInfo==NULL)||(pRecvUserInfo==NULL)) return true;
			if (pSendUserInfo->m_UserData.bDeskNO==255) return true;
			pNetHead->bHandleCode=pSendUserInfo->m_UserData.bDeskNO;
			pInviteUser->dwUserID=pSendUserInfo->m_UserData.dwUserID;

			//发送数据
			m_TCPSocket.SendData(pRecvUserInfo->m_uSocketIndex,pInviteUser,uSize,MDM_GR_ROOM,ASS_GR_INVITEUSER,
				pNetHead->bHandleCode,pRecvUserInfo->m_dwHandleID);

			return true;
		}
    case ASS_GR_GET_NICKNAME_ONID:  // 根据ID获取昵称
        {
            // 效验数据
            if (uSize!=sizeof(MSG_GR_S_GetNickNameOnID_t)) return false;
            MSG_GR_S_GetNickNameOnID_t *pGetNickNameOnID =(MSG_GR_S_GetNickNameOnID_t *)pData;

            // 加入数据库
            DL_GR_I_GetNickNameOnID stGetNickNameOnID;
            stGetNickNameOnID.stGetNickNameOnID.iUserID = pGetNickNameOnID->iUserID;
            m_SQLDataManage.PushLine(&stGetNickNameOnID.DataBaseHead,sizeof(stGetNickNameOnID),DTK_GR_GETNICKNAME_ONID,uIndex,dwHandleID);

            return true;
        }
    break;
    case ASS_GR_GET_NICKNAME_ONID_INGAME:  // 根据ID获取昵称
        {
            // 效验数据
            if (uSize!=sizeof(MSG_GR_S_GetNickNameOnID_t)) return false;
            MSG_GR_S_GetNickNameOnID_t *pGetNickNameOnID =(MSG_GR_S_GetNickNameOnID_t *)pData;

            // 加入数据库
            DL_GR_I_GetNickNameOnID stGetNickNameOnID;
            stGetNickNameOnID.stGetNickNameOnID.iUserID = pGetNickNameOnID->iUserID;
            m_SQLDataManage.PushLine(&stGetNickNameOnID.DataBaseHead,sizeof(stGetNickNameOnID),DTK_GR_GETNICKNAME_ONID_INGAME,uIndex,dwHandleID);

            return true;
        }
    break;
    case ASS_GR_OWNER_T_ONE_LEFT_ROOM:      // 房主踢玩家离开房间
        {
            //效验数据
            if (uSize!=sizeof(MSG_GR_S_CutUser)) return false;
            MSG_GR_S_CutUser * pCutUser=(MSG_GR_S_CutUser *)pData;

            CGameUserInfo * pTargetUserInfo=m_UserManage.FindOnLineUser(pCutUser->dwTargetID);
            if (pTargetUserInfo==NULL) return true;

            char bufID[12] = {0};
            wsprintf(bufID,"%ld",pCutUser->dwTargetID);
            m_TCPSocket.SendData(pTargetUserInfo->m_uSocketIndex, bufID, sizeof(bufID),
                                MDM_GR_ROOM,ASS_GR_OWNER_T_ONE_LEFT_ROOM,0,pTargetUserInfo->m_dwHandleID);

            return true;
        }
        break;

	case ASS_GR_AVATAR_LOGO_CHANGE: //房间的某玩家logo形象改变
		{
			//效验数据
			if (uSize!=sizeof(MSG_GR_AvatarChange)) return false;
			MSG_GR_AvatarChange* pAvatarChange = (MSG_GR_AvatarChange*)pData;

			CGameUserInfo * pTargetUserInfo = m_UserManage.FindOnLineUser(pAvatarChange->dwUserID);
			if (pTargetUserInfo == NULL) return true;

			pTargetUserInfo->GetUserData(NULL)->bLogoID = pAvatarChange->nNewLogoID;

			m_TCPSocket.SendDataBatch(pAvatarChange,sizeof(MSG_GR_AvatarChange),MDM_GR_ROOM,ASS_GR_AVATAR_LOGO_CHANGE,0);

			return true;
		}
		break;
	case ASS_GR_CONTEST_EXIT:
		{
			if (uSize!=0) return false;

			if (!(m_InitData.dwRoomRule & GRR_CONTEST) && !(m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
			{
				return true;
			}

			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if (pUserInfo==NULL) return false;
			//判断是否可以离开
			if (pUserInfo->m_UserData.bDeskNO!=255)
			{
				m_TCPSocket.SendData(uIndex,MDM_GR_ROOM,ASS_GR_CONTEST_EXIT,1,dwHandleID);
				return true;
			}

			DL_GR_I_ContestExit _in;
			_in.iUserID = pUserInfo->m_UserData.dwUserID;
			 m_SQLDataManage.PushLine(&_in.DataBaseHead,sizeof(_in),DTK_GR_CONTEST_EXIT,uIndex,dwHandleID);

			 m_TCPSocket.SendData(uIndex,MDM_GR_ROOM,ASS_GR_CONTEST_EXIT,0,dwHandleID);

			 AutoAllotDeskDeleteUser(pUserInfo);

			return true;
		}break;
	}
	return false;
}

//用户登陆处理 
bool CGameMainManage::OnUserLogonMessage(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	ASSERT(pNetHead->bMainID==MDM_GR_LOGON);
	switch (pNetHead->bAssistantID)
	{
	case ASS_GR_LOGON_BY_ID:		//通过 ID 登陆
		{
			//效验数据
			if (uSize!=sizeof(MSG_GR_S_RoomLogon)) return false;
			MSG_GR_S_RoomLogon * pLogonInfo=(MSG_GR_S_RoomLogon *)pData;
			if (pLogonInfo->uNameID!=m_KernelData.uNameID) return false;
			pLogonInfo->szMD5Pass[sizeof(pLogonInfo->szMD5Pass)/sizeof(pLogonInfo->szMD5Pass[0])-1]=0;
			m_bStopLogon=m_InitData.uStopLogon;
			//判断登陆状态
			if (m_bStopLogon)
			{
				SendRoomMessage(uIndex,dwHandleID,TEXT("游戏房间已经暂停进入！"),SHS_MESSAGE_BOX|SHS_SYSTEM_SHOW,0,TRUE);
				m_TCPSocket.SendData(uIndex,MDM_GR_LOGON,ASS_GR_LOGON_ERROR,ERR_GR_STOP_LOGON,dwHandleID);
				return true;
			}

			//查找在线用户
			CGameUserInfo * pGameUserInfo=m_UserManage.FindOnLineUser(pLogonInfo->dwUserID);
			if (pGameUserInfo!=NULL)
			{
				int SocketID = pGameUserInfo->GetSocketIndex();
				m_TCPSocket.CloseSocket(SocketID,0);
				OnSocketClose(uAccessIP,SocketID,0);
			}

			if (m_InitData.bVIPRoom &&			 ///< 在密码房间
				0!=strlen(m_szVIPPassWord) &&	 ///< 已经设置密码
				NULL==pGameUserInfo &&			 ///< 设置密码的人不在房间
				m_bIsVIPLogon &&
				m_uVIPIndex==uIndex &&			 ///< 设置密码的人的套接字索引
				m_lVIPID!=pLogonInfo->dwUserID)  ///< 此人的ID不是VIP ID
			{
				m_bIsVIPLogon = FALSE;
				m_TCPSocket.SendData(uIndex,MDM_GR_LOGON,ASS_GR_LOGON_ERROR,ERR_GR_STOP_LOGON,dwHandleID);
				return 0;
			}
			else if (m_InitData.bVIPRoom &&
				0!=strlen(m_szVIPPassWord) &&
				NULL==pGameUserInfo &&
				m_bIsVIPLogon &&
				m_uVIPIndex==uIndex &&
				m_lVIPID==pLogonInfo->dwUserID)
			{
				m_bIsVIPLogon = FALSE;
			}

			pGameUserInfo=m_UserManage.FindOnLineUser(pLogonInfo->dwUserID);
			if (pGameUserInfo!=NULL)
			{
				m_TCPSocket.CloseSocket(pGameUserInfo->m_uSocketIndex, 0);
				m_TCPSocket.SendData(uIndex,MDM_GR_LOGON,ASS_GR_LOGON_ERROR,ERR_GR_ACCOUNTS_IN_USE,dwHandleID);
				return true;
			}

			//查找断线用户
			CGameUserInfo * pLostUserInfo=m_UserManage.FindNetCutUser(pLogonInfo->dwUserID);
			if (pLostUserInfo!=NULL)
			{
				//效验用户密码
				if (lstrcmp(pLostUserInfo->m_szMD5Pass,pLogonInfo->szMD5Pass)!=0) 
				{
					m_TCPSocket.SendData(uIndex,MDM_GR_LOGON,ASS_GR_LOGON_ERROR,ERR_GR_USER_PASS_ERROR,dwHandleID);
					return true;
				}
//				if ((pLostUserInfo->m_UserData.userInfoEx2 & 0x80000000) > 0)
				{
				//	m_TCPSocket.SendData(uIndex,MDM_GR_LOGON,ASS_GR_LOGON_ERROR,ERR_GR_STOP_LOGON,dwHandleID);
				//	return true;
				}

				//加入用户资料
				pLostUserInfo->m_UserData.bUserState=pLostUserInfo->bBeforeState;
				pGameUserInfo=m_UserManage.ActiveUser(&pLostUserInfo->m_UserData,uIndex,dwHandleID,pLostUserInfo->m_dwGamePower,
					pLostUserInfo->m_dwMasterPower,pLostUserInfo->m_dwAccessLogID);

				//写入用户附加信息
				pGameUserInfo->SetMD5Pass(pLogonInfo->szMD5Pass);
				pGameUserInfo->m_ChangePoint=pLostUserInfo->m_ChangePoint;
				pGameUserInfo->SetUserScrPoint(pLostUserInfo->m_dwScrPoint);

				//发送房间信息
				if (SendRoomInfo(pGameUserInfo,uIndex,dwHandleID,NULL,!pGameUserInfo->m_UserData.bLogonbyphone)==false)
					return false;

				//断线状态处理
				//ASSERT(pGameUserInfo->m_UserData.bDeskNO!=255);
				if (pGameUserInfo->m_UserData.bDeskNO!=255 && pGameUserInfo->m_UserData.bDeskNO<m_uDeskCount)
				{
					CGameDesk * pGameDesk=*(m_pDesk+pGameUserInfo->m_UserData.bDeskNO);
                    if (NULL != pGameDesk)
                    {
                        pGameDesk->UserReCome(pGameUserInfo->m_UserData.bDeskStation,pGameUserInfo);
                    }
				}

				if (m_InitData.dwRoomRule & GRR_CONTEST || m_InitData.dwRoomRule & GRR_TIMINGCONTEST)
				{
					AutoAllotDeskAddUser(pGameUserInfo);
				}

				//清理断线资料
				m_UserManage.FreeUser(pLostUserInfo,false);
				SetTimer(IDT_UPDATE_ROOM_PEOPLE,100);//20081201
				return true;
			}

			//加入数据库
			DL_GR_I_LogonByIDStruct DL_Logon;
			DL_Logon.uAccessIP=uAccessIP;
			DL_Logon.LogonInfo=*pLogonInfo;
			m_SQLDataManage.PushLine(&DL_Logon.DataBaseHead,sizeof(DL_Logon),DTK_GR_LOGON_BY_ID,uIndex,dwHandleID);

			return true;
		}
	}
	return false;
}

//该房间是否启动房间管理窗口
bool CGameMainManage::IsExistGRMRoom(UINT iRoomID)
{
	if (m_GRMRoomID.empty())
		return false;
	std::vector<UINT>::iterator it;
	it = std::find( m_GRMRoomID.begin(), m_GRMRoomID.end(), iRoomID );
	if (it != m_GRMRoomID.end())
	{
		return true;
	}

	return false;
}

//该玩家是否可以使用房间管理窗口
bool CGameMainManage::IsExistGRMUserID(long int UserID)
{
	if (m_GRMUserID.empty())
		return false;
	std::vector<long int>::iterator it;
	it = std::find( m_GRMUserID.begin(), m_GRMUserID.end(), UserID );
	if (it != m_GRMUserID.end())
	{
		return true;
	}

	return false;
}

//离开桌子函数
bool CGameMainManage::UserLeftDesk(CGameUserInfo * pUserInfo,bool bLeft/*=true*/)
{
	if (pUserInfo->m_UserData.bDeskNO >= m_uDeskCount) return false;

	//获取桌子
	CGameDesk * pLeftDesk=*(m_pDesk+pUserInfo->m_UserData.bDeskNO);

	///如果是排队机，不在游戏中有人起来， 解散桌子， 并且让其它人进行排队准备状态           
	if (IsQueueGameRoom() && !IsUserPlayGame(pUserInfo))
	{
		pLeftDesk->m_bLeave = bLeft;
		pLeftDesk->UserLeftDesk(pUserInfo->m_UserData.bDeskStation,pUserInfo);
		/*MSG_GR_R_UserAgree UserAgree;
		for (int i = 0; i < pLeftDesk->m_bMaxPeople; i++)
		{
			if (pLeftDesk->m_pUserInfo[i] != NULL)
			{
				CGameUserInfo *p = m_UserManage.FindOnLineUser(pLeftDesk->m_pUserInfo[i]->m_UserData.dwUserID);
				
				if (p && p != pUserInfo)
				{
					 UINT uSocketIndex = p->m_uSocketIndex;
					 m_TCPSocket.SendData(uSocketIndex,&p->m_UserData.dwUserID,sizeof(int),MDM_GR_ROOM,ASS_GR_ROOM_QUEUE_READY, 0, 0);

					 pLeftDesk->m_bLeave = false;
					 pLeftDesk->UserLeftDesk(p->m_UserData.bDeskStation,p);
					 pLeftDesk->m_bLeave = true;

					 if((m_InitData.dwRoomRule & GRR_CONTEST) || (m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
					 {
						 //重置该桌子的状态
						 pLeftDesk->m_pUserInfo[i]->m_UserData.bDeskNO=255;
						 pLeftDesk->m_pUserInfo[i]->m_UserData.bDeskStation=255;
						 pLeftDesk->m_pUserInfo[i]->m_UserData.bUserState=USER_LOOK_STATE;

						 pLeftDesk->m_uCutTime[i]=0L;
						 pLeftDesk->m_bConnect[i]=false;
						 pLeftDesk->m_pUserInfo[i]=NULL;
					 }
					 else
					 {
						 m_pIAutoAllotDesk->Add(p);
					 }
				}
			}
		}*/
		return true;
	}

	if (pUserInfo->m_UserData.bUserState==USER_WATCH_GAME)
	{
		pLeftDesk->WatchUserLeftDesk(pUserInfo->m_UserData.bDeskStation,pUserInfo);
	}
	else 
	{
		pLeftDesk->m_bLeave = bLeft;
		pLeftDesk->UserLeftDesk(pUserInfo->m_UserData.bDeskStation,pUserInfo);
		pLeftDesk->m_bLeave = true;
	}

	//设置变量
	pUserInfo->m_UserData.bDeskNO=255;
	pUserInfo->m_UserData.bDeskStation=255;
	pUserInfo->m_UserData.bUserState=USER_LOOK_STATE;

	//强退罚权
	CBcfFile fConf(CBcfFile::GetAppPath()+"Function.bcf");
	int fleeSubWeight = fConf.GetKeyVal("OnlineCoin","FleeSubWeight", 0);

	pUserInfo->m_SelectWeight = fleeSubWeight * 50;

	return true;
}
//用户是否游戏
bool CGameMainManage::IsUserPlayGame(CGameUserInfo * pUserInfo)
{
	if ((pUserInfo->m_UserData.bDeskNO!=255 && pUserInfo->m_UserData.bDeskNO<m_uDeskCount)&&(pUserInfo->m_UserData.bUserState!=USER_WATCH_GAME))
	{
		CGameDesk * pGameDesk=*(m_pDesk+pUserInfo->m_UserData.bDeskNO);
		return (pGameDesk->IsPlayGame(pUserInfo->m_UserData.bDeskStation) || ((m_InitData.dwRoomRule & GRR_GAME_BUY) && pGameDesk->m_iRunGameCount>0));
	}
	return false;
}
//用户是否游戏
bool CGameMainManage::IsUserPlayGameByStation(CGameUserInfo * pUserInfo)
{
	if ((pUserInfo->m_UserData.bDeskNO!=255 && pUserInfo->m_UserData.bDeskNO<m_uDeskCount)&&(pUserInfo->m_UserData.bUserState!=USER_WATCH_GAME))
	{
		CGameDesk * pGameDesk=*(m_pDesk+pUserInfo->m_UserData.bDeskNO);
		return (pGameDesk->IsPlayingByGameStation() || ((m_InitData.dwRoomRule & GRR_GAME_BUY) && pGameDesk->m_iRunGameCount>0));
	}
	return false;
}

//是否限制聊天
bool CGameMainManage::IsLimitedTalk(CGameUserInfo * pUserInfo, bool bRoom)
{
	if (bRoom==true)
	{
		for (INT_PTR i=0;i<m_RoomLimited.GetCount();i++)
		{
			if (m_RoomLimited.GetAt(i).dwUserID==pUserInfo->m_UserData.dwUserID) return true;
		}
	}
	else
	{
		for (INT_PTR i=0;i<m_GameLimited.GetCount();i++)
		{
			if (m_GameLimited.GetAt(i).dwUserID==pUserInfo->m_UserData.dwUserID) return true;
		}
	}
	return false;
}

//SOCKET 关闭
bool CGameMainManage::OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, long int lConnectTime)
{
	CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uSocketIndex);
	if (pUserInfo!=NULL)
	{
		//是否在游戏中
		CGameUserInfo * pNetCutUserInfo=NULL;
		CGameUserInfo *pAlreadyCutUserInfo = m_UserManage.FindNetCutUser(pUserInfo->m_UserData.dwUserID);
		if (pUserInfo->m_UserData.bDeskNO!=255 && pUserInfo->m_UserData.bDeskNO<m_uDeskCount) 
		{
			//注册断线用户
			CGameDesk * pGameDesk=*(m_pDesk+pUserInfo->m_UserData.bDeskNO);
			if (pGameDesk && pGameDesk->m_bIsBuy && pGameDesk->m_bInLocation)
			{
				pGameDesk->UserNotAgreePrevent(pUserInfo->m_UserData.dwUserID);
			}
			/// 不能判断是否可以断线，应该是任何时候都可以断线，否则玩家在百人类游戏中快输时强退，会导致不结算
			if (IsUserPlayGame(pUserInfo) || (pUserInfo->m_UserData.dwUserID == pGameDesk->m_iDeskMaster))
			{
				if (pAlreadyCutUserInfo == NULL)
				{
					pNetCutUserInfo=m_UserManage.RegNetCutUser(pUserInfo);
				}
				else
				{
					pNetCutUserInfo=pAlreadyCutUserInfo;
				}
			}
			if (pNetCutUserInfo!=NULL && pUserInfo->m_UserData.bUserState != USER_WATCH_GAME)
			{
				pGameDesk->UserNetCut(pNetCutUserInfo->m_UserData.bDeskStation,pNetCutUserInfo);
			}
			else
			{
				/*if((m_InitData.dwRoomRule & GRR_CONTEST) || (m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
				{
					pGameDesk->PlayerNetCutContest(pUserInfo->m_UserData.bDeskStation);
				}
				else*/
				{
					UserLeftDesk(pUserInfo);
					//清除断线标志
					DL_GR_I_ClearOnlineFlag dtClearOnlineFlag;
					memset(&dtClearOnlineFlag, 0, sizeof(dtClearOnlineFlag));
					dtClearOnlineFlag.lUserID = pUserInfo->m_UserData.dwUserID;
					m_SQLDataManage.PushLine(&dtClearOnlineFlag.DataBaseHead, sizeof(dtClearOnlineFlag), DTK_GR_CLEAR_ONLINE_FLAG, 0, 0);

				}
			}
		}
		else if((m_InitData.dwRoomRule & GRR_CONTEST) || (m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
		{
			DL_GR_I_ClearOnlineFlag dtClearOnlineFlag;
			memset(&dtClearOnlineFlag, 0, sizeof(dtClearOnlineFlag));
			dtClearOnlineFlag.lUserID = pUserInfo->m_UserData.dwUserID;
			m_SQLDataManage.PushLine(&dtClearOnlineFlag.DataBaseHead, sizeof(dtClearOnlineFlag), DTK_GR_CLEAR_ONLINE_FLAG, 0, 0);

			if (!IsUserPlayGame(pUserInfo))
			{
				IsContestGameOver();
			}
		}
		//用户还没有坐下就离开了
		if (pNetCutUserInfo==NULL)
		{
			m_pGameUserInfoList.remove(pUserInfo);	//删除正在排队的玩家
			CleanUserInfo(pUserInfo);
		}

		if ((m_InitData.dwRoomRule & GRR_CONTEST) || (m_InitData.dwRoomRule & GRR_TIMINGCONTEST) && m_InitData.iRoomState == 1)
		{
			DL_GR_I_UpdateNetCutTime _in;
			memset(&_in, 0, sizeof(_in));
			_in.iUserID = pUserInfo->m_UserData.dwUserID;
			_in.iMatchID = m_InitData.iMatchID;
			m_SQLDataManage.PushLine(&_in.DataBaseHead, sizeof(_in), DTK_GR_UPDATE_NETCUTTIME, 0, 0);
		}

		if ((m_InitData.dwRoomRule & GRR_CONTEST) && m_InitData.iRoomState == 0)
		{
			//如果是比赛房间，且未开赛，则要为该用户退回报名费
			DL_GR_I_ContestReturnFee dtReturnFee;
			memset(&dtReturnFee, 0, sizeof(dtReturnFee));
			dtReturnFee.iUserID = pUserInfo->m_UserData.dwUserID;
			dtReturnFee.iGameID = m_InitData.iGameID;
			dtReturnFee.iRoomID = m_InitData.uRoomID;
			dtReturnFee.iContestID = m_InitData.iContestID;
			m_SQLDataManage.PushLine(&dtReturnFee.DataBaseHead, sizeof(dtReturnFee), DTK_GR_CONTEST_RETURN_FEE, 0, 0);
		}

		if (m_pIAutoAllotDesk)
		{
			AutoAllotDeskDeleteUser(pUserInfo);	//删除正在排队的玩家
		}

		//清理用户
		m_UserManage.FreeUser(pUserInfo,true);
		SetTimer(IDT_UPDATE_ROOM_PEOPLE,100);
	}
	else
	{
		
	}
	return false;
}

//数据库处理结果
bool CGameMainManage::OnDataBaseResult(DataBaseResultLine * pResultData)
{
	switch (pResultData->uHandleKind)
	{
	case DTK_GR_BANK_UPDATE_REBATE:
		{
			return OnGetBankRebateResponse(pResultData);
		}
	case DTK_GR_BANK_UPDATE_USER_REBATE:
	case DTK_GR_BANK_UPDATE_USER_REBATE_INGAME:
		{
			return OnGetBankUserRebateResponse(pResultData, pResultData->uHandleKind);
		}
	case DTK_GR_BANK_SET_USER_REBATE://玩家修改返利数据
	case DTK_GR_BANK_SET_USER_REBATE_INGAME:
		{
			return OnGetBankUserSetRebateResponse(pResultData, pResultData->uHandleKind);
		}
	case DTK_GR_DEMAND_MONEY_IN_GAME:	//从游戏界面打开钱柜
	case DTK_GR_DEMAND_MONEY:	//打开钱柜
		{
			return OnAboutMoneyResult(pResultData, pResultData->uHandleKind);
		}
	case DTK_GR_CHECKIN_MONEY:	//房间－＞钱柜
	case DTK_GR_CHECKIN_MONEY_INGAME:	//房间－＞钱柜
		{
			return OnCheckInMoney(pResultData, pResultData->uHandleKind);
		}
	case DTK_GR_CHECKOUT_MONEY_INGAME: //游戏中取钱<－钱柜
	case DTK_GR_CHECKOUT_MONEY: //房间<－钱柜
		{
			return OnCheckOutMoney(pResultData, pResultData->uHandleKind);
		}
	case DTK_GR_TRANSFER_MONEY://转帐
	case DTK_GR_TRANSFER_MONEY_INGAME://游戏中转帐
		{
			return OnTransferMoney(pResultData, pResultData->uHandleKind);
		}
	case DTK_GR_CHANGE_PASSWD:		 ///<修改密码
	case DTK_GR_CHANGE_PASSWD_INGAME:///<游戏中修改密码
		{
			return OnChangePasswd(pResultData, pResultData->uHandleKind);
		}
	case DTK_GR_TRANSFER_RECORD:			///<转帐记录
	case DTK_GR_TRANSFER_RECORD_INGAME:		///<游戏中转帐记录
		{
			return OnTransferRecord(pResultData, pResultData->uHandleKind);
		}
	case DTK_GR_LOGON_BY_ID:	//房间登陆
		{
			return OnRoomLogonResult(pResultData);
		}
	case DTK_GR_CHECK_CONNECT:	//检测连接
		{
			return OnCheckConnect(pResultData);
		}
	case DTK_GR_CALL_GM_RESULT://呼叫网管结果
		{
			return OnCallGMResult(pResultData);
		}
	case DTK_GPR_PROP_SMALL_BRD:
	case DTK_GR_PROP_USE://使用道具 
		{
			return m_PropService->OnDataBaseResult(pResultData);
		}
	case DTK_GR_SENDMONEY_TIMES:
		{
			return SendUserMoenyByTimesResult(pResultData);		
		}
		//混战房间游戏记录结果
	case DTK_GR_BATTLEROOM_RES:
		{
			return OnBattleRoomRecord(pResultData);		
		}
	//case DTK_GR_READ_USER_HOLD_ITEM://持有道俱结果
	//case DTK_GR_USER_USE_ONE_ITEM://使用道俱结果
	//case DTK_GR_GET_BUY_ITEM_DATA://打开购买列表结果
	//case DTK_GR_BUY_ITEM:			//购买道俱
	////case DTK_GR_CHECK_VIP_TIMER://vip时间检测
	//	{
	//		return m_ItemDate->OnDataBaseResult(pResultData);
	//	}

		//case DTK_GR_FRIEND_MANAGE://好友管理
		//	{
		//		return OnFriendManageDBOperateResult(pResultData);
		//	}
		//case DTK_GR_BUFFER_IM_MESSAGE://聊天消息
		//	{
		//		return OnBufferIMMessageDBOperateResult(pResultData);
		//	}
    case DTK_GR_GETNICKNAME_ONID:
	case DTK_GR_GETNICKNAME_ONID_INGAME:
    {
        return OnGetNickNameOnIDResult(pResultData);
    }
    case DTK_GR_USE_KICK_PROP:  // 踢人卡
    {
        return OnUseKickPropResult(pResultData, TRUE);
    }
    case DTK_GR_USE_ANTI_KICK_PROP: // 防踢卡
    {
        return OnUseKickPropResult(pResultData, FALSE);
    }
    case DTK_GR_USER_NEW_KICK_USER_PROP:    // 使用踢人卡踢人
    {
        return OnNewKickUserPropResult(pResultData);        
    }
	case DTK_GR_BANK_NORMAL:
	case DTK_GR_BANK_TRANSFER:
	case DTK_GR_BANK_DEPOSIT:
	case DTK_GR_BANK_WALLET:
	case DTK_GR_BANK_QUERY_DEPOSIT:
	{
		if (m_pBankService != NULL)
		{
			m_pBankService->OnDataBaseResult(pResultData);
		}
	}
    break;
	case DTK_GR_GET_CONTEST_RESULT: //获取比赛当前排名，成绩等
		{
			return OnGetContestRankResult(pResultData);
		}
		break;
	case DTK_GR_CONTEST_BEGIN_RESULT:
		{
			OnGetContestBeginResult(pResultData);
		}
		break;
	case DTK_GR_CONTEST_APPLY:
		{
			OnGetContestApplyResult(pResultData);
		}
		break;
	case DTK_GR_CONTEST_ABANDON:
		{
			OnGetContestAbandonResult(pResultData);
		}
		break;
	case DTK_GR_CONTEST_BROADCAST:
		{
			OnBroadCastConstestInfo(pResultData);
		}break;
	case DTK_GR_CONTEST_GAME_OVER_RESULT:
		{
			OnGetContestGameOverResult(pResultData);
		}
		break;
	case DTK_GR_PROP_CHANGE:
		{
			return OnPropChangeResult(pResultData);
		}
		break;
	case DTK_GR_TIMINGMATCH_GETQUEQUEUSERS:
		{
			return OnTimingMatchBeginFillQueue(pResultData);
		}break;
	case DTK_GR_FINDDESKBUYSTATION:
		{
			return OnFindBuyDeskStationResult(pResultData);
		}break;
	case DTK_GR_RETURNEDESK:
		{
			return OnReturnDeskResult(pResultData);
		}break;
	case DTK_GR_CREATE_GAMESN:
		{
			return OnCreateGameSNResult(pResultData);
		}
	case DTK_GR_GET_DESKINFO:
		{
			return OnGetDeskInfo(pResultData);
		}
	case DTK_GR_GETRELEASEINFO:
		{
			return OnSetReturnGameInfo(pResultData);
		}
	case DTK_GR_CONTEST_REGIST:
		{
			return OnContestRegistResult(pResultData);
		}
	case DTK_GR_CONTEST_ACTIVE:
		{
			return OnContestActiveResult(pResultData);
		}
	case DTK_GR_BUYROOM_COSTFEE:
		{
			return OnCostFee(pResultData);
		}
	case DTK_GR_GET_ROOMINFO:
		{
			return OnGetRoomInfoResult(pResultData);
		}
	case DTK_GR_GET_CONTESTINFO:
		{
			return OnSetContestInfoResult(pResultData);
		}
    case DTK_GR_DISSMISSDESKBYMS:
        {
            return OnDissmissDeskByMS(pResultData);
        }
	}
	return false;
}

//获取指定桌子数据
CGameDesk * CGameMainManage::GetDeskObject(BYTE bIndex)
{
	return 	*(m_pDesk+bIndex);
}

//wushuqun 2009.6.5
//混战房间记录结果处理
bool CGameMainManage::OnBattleRoomRecord(DataBaseResultLine * pResultData)
{
	if (pResultData->uHandleRusult == DTR_GR_BATTLEROOM_TIMEOUT_ERROR)
	{
		//混战房间已不在有效时间内
		DL_GR_O_BattleRecordResult * battleRoomRet = (DL_GR_O_BattleRecordResult * )pResultData;

		//通知客户端混战房间的时间已经无效
		BattleRoomRecordRes BattleRoomResult;
		memset(& BattleRoomResult,0,sizeof(BattleRoomResult));
		BattleRoomResult.uRoomID = battleRoomRet->uRoomID;
		BattleRoomResult.uRecordResult = RES_BATTLEROOM_TIMEOUT_ERROR;
		
		m_TCPSocket.SendDataBatch(&BattleRoomResult,sizeof(BattleRoomResult),MDM_GR_ROOM,ASS_GR_BATTLEROOM_RESULT,0);
	}
	return true;
}

//清理用户资料
bool CGameMainManage::CleanUserInfo(CGameUserInfo * pUserInfo)
{
	//发送消息
	if (pUserInfo == nullptr)
		return true;
	MSG_GR_R_UserLeft UserLeft;
	UserLeft.dwUserID=pUserInfo->m_UserData.dwUserID;
	/*if (pUserInfo->m_UserData.bGameMaster==0)*/
	{
		m_TCPSocket.SendDataBatch(&UserLeft,sizeof(UserLeft),MDM_GR_USER_ACTION,ASS_GR_USER_LEFT,0);
	}

	//记录用户分数
	DL_GR_I_UserLeft DT_UserLeft;
	DT_UserLeft.dwUserID=pUserInfo->m_UserData.dwUserID;
	//DT_UserLeft.uMaster=pUserInfo->m_UserData.bGameMaster;
	DT_UserLeft.dwScrPoint=pUserInfo->m_dwScrPoint;
	DT_UserLeft.dwChangePoint=pUserInfo->m_ChangePoint.dwPoint;
	DT_UserLeft.dwChangeMoney=pUserInfo->m_ChangePoint.dwMoney;
	DT_UserLeft.dwChangeTaxCom=pUserInfo->m_ChangePoint.dwTaxCom;
	//DT_UserLeft.dwResultPoint=pUserInfo->m_UserData.dwPoint;
	DT_UserLeft.uWinCount=pUserInfo->m_ChangePoint.uWinCount;
	DT_UserLeft.uLostCount=pUserInfo->m_ChangePoint.uLostCount;
	DT_UserLeft.uMidCount=pUserInfo->m_ChangePoint.uMidCount;
	DT_UserLeft.uCutCount=pUserInfo->m_ChangePoint.uCutCount;
	DT_UserLeft.dwAccessLogID=pUserInfo->m_dwAccessLogID;
	DT_UserLeft.dwPlayTimeCount=pUserInfo->m_dwPlayCount;
	DT_UserLeft.uAccessIP=pUserInfo->m_UserData.dwUserIP;
	DT_UserLeft.dwOnLineTimeCount=(long int)time(NULL)-pUserInfo->m_dwLogonTime;
	m_SQLDataManage.PushLine(&DT_UserLeft.DataBaseHead,sizeof(DT_UserLeft),DTK_GR_USER_LEFT,0,0);
	SetTimer(IDT_UPDATE_ROOM_PEOPLE,1000);

	return true;
}

//发送房间消息
bool CGameMainManage::SendRoomMessage(UINT uSocketIndex, DWORD dwHandleID, TCHAR * szMessage, BYTE bShowStation, BYTE bFontSize, BYTE bCloseFace)
{
	//定义数据 
	MSG_GA_S_Message Message;
	memset(&Message,0,sizeof(Message));

	//效验数据
	UINT uLength=lstrlen(szMessage);
	if (uLength>=sizeof(Message.szMessage)/sizeof(TCHAR)) return false;
	UINT uSendSize=sizeof(Message)-sizeof(Message.szMessage)+sizeof(TCHAR)*(uLength+1);

	//发送数据
	Message.bFontSize=bFontSize;
	Message.bCloseFace=bCloseFace;
	Message.bShowStation=bShowStation;
	lstrcpy(Message.szMessage,szMessage);
	if (uSocketIndex==m_InitData.uMaxPeople) m_TCPSocket.SendDataBatch(&Message,uSendSize,MDM_GR_MESSAGE,ASS_GR_SYSTEM_MESSAGE,0);
	else m_TCPSocket.SendData(uSocketIndex,&Message,uSendSize,MDM_GR_MESSAGE,ASS_GR_SYSTEM_MESSAGE,0,dwHandleID);

	return true;
}

//发送游戏消息
bool CGameMainManage::SendGameMessage(UINT uSocketIndex, DWORD dwHandleID, TCHAR * szMessage, BYTE bShowStation, BYTE bFontSize, BYTE bCloseFace)
{
	//定义数据 
	MSG_GA_S_Message Message;
	memset(&Message,0,sizeof(Message));

	//效验数据
	UINT uLength=lstrlen(szMessage);
	if (uLength>=sizeof(Message.szMessage)/sizeof(TCHAR)) return false;
	UINT uSendSize=sizeof(Message)-sizeof(Message.szMessage)+sizeof(TCHAR)*(uLength+1);

	//发送数据
	Message.bFontSize=bFontSize;
	Message.bCloseFace=bCloseFace;
	Message.bShowStation=bShowStation;
	lstrcpy(Message.szMessage,szMessage);
	if (uSocketIndex==m_InitData.uMaxPeople) m_TCPSocket.SendDataBatch(&Message,uSendSize,MDM_GM_MESSAGE,ASS_GM_SYSTEM_MESSAGE,0);
	else m_TCPSocket.SendData(uSocketIndex,&Message,uSendSize,MDM_GM_MESSAGE,ASS_GM_SYSTEM_MESSAGE,0,dwHandleID);

	return true;
}

//发送比赛信息
bool CGameMainManage::SendMatchInfo(UINT uSocketIndex, DWORD dwHandleID)
{
	if ((m_uMatchUserCount!=0L)&&(m_pMatchUser!=NULL))
	{
		//发送数据
		UINT uBeenSendCount=0L;
		while (uBeenSendCount<m_uMatchUserCount)
		{
			UINT uSendCount=__min(m_uMatchUserCount-uBeenSendCount,MAX_SEND_SIZE/sizeof(MSG_GR_SR_MatchUser));
			m_TCPSocket.SendData(uSocketIndex,m_pMatchUser+uBeenSendCount,sizeof(MSG_GR_SR_MatchUser)*uSendCount,
				MDM_GR_MATCH_INFO,ASS_GR_MATCH_USER,0,dwHandleID);
			uBeenSendCount+=uSendCount;
		};
	}

	return true;
}

//发送房间信息
bool CGameMainManage::SendRoomInfo(CGameUserInfo * pUserInfo, UINT uSocketIndex, DWORD dwHandleID,DL_GR_O_LogonResult * pLogonResult,bool bSend)
{
	//发送登陆成功信息
	MSG_GR_R_LogonResult MSGLogonResult;
	MSGLogonResult.pUserInfoStruct=pUserInfo->m_UserData;
	MSGLogonResult.nVirtualUser = m_InitData.uVirtualUser; ///< 虚拟玩家人数
	MSGLogonResult.iRemainPeople = m_iRemainPeople;
	
	//登陆金币不足自动赠送
	if (NULL != pLogonResult)
	{
		MSGLogonResult.strRecMoney = pLogonResult->strRecMoney;	
	}

	if ((m_InitData.dwRoomRule & GRR_CONTEST || m_InitData.dwRoomRule & GRR_TIMINGCONTEST ) && pUserInfo->m_UserData.iContestCount == 0 ) 
	{
		pUserInfo->m_UserData.i64ContestScore = m_InitData.i64Chip;
	}


	if (NULL != pLogonResult)
	{
		MSGLogonResult.nVirtualUser = pLogonResult->nVirtualUser; ///< 即时获取房间虚拟玩家人数
	}

	if (NULL != pLogonResult)
	{
		m_TCPSocket.SendData(uSocketIndex,&MSGLogonResult,sizeof(MSGLogonResult),MDM_GR_LOGON,ASS_GR_LOGON_SUCCESS,
			pLogonResult->ResultHead.uHandleRusult,dwHandleID);
	}
	else
	{
		m_TCPSocket.SendData(uSocketIndex,&MSGLogonResult,sizeof(MSGLogonResult),MDM_GR_LOGON,ASS_GR_LOGON_SUCCESS,
			ERR_GR_LOGON_SUCCESS,dwHandleID);
	}
	


	if (bSend)
	{
		//发送在线用户信息
		bool bFinish=false;
		UINT uFillCount=0,uIndex=0;
		char bBuffer[MAX_SEND_SIZE];
		do
		{
			UINT uCopyPos=m_UserManage.FillOnLineUserInfo(bBuffer,sizeof(bBuffer),uIndex,uFillCount,bFinish);
			if (bFinish) 
			{
				if (m_TCPSocket.SendData(uSocketIndex,bBuffer,(UINT)uCopyPos*sizeof(UserInfoStruct),
					MDM_GR_USER_LIST,ASS_GR_ONLINE_USER,ERR_GR_LIST_FINISH,dwHandleID)!=uCopyPos*sizeof(UserInfoStruct))
				{
					return false;
				}
			}
			else 
			{
				if (m_TCPSocket.SendData(uSocketIndex,bBuffer,(UINT)uCopyPos*sizeof(UserInfoStruct),
					MDM_GR_USER_LIST,ASS_GR_ONLINE_USER,ERR_GR_LIST_PART,dwHandleID)!=uCopyPos*sizeof(UserInfoStruct))
				{			
					return false;
				}
				::Sleep(10);
			}
		} while (bFinish==false);

		//发送断线用户信息
		bFinish=false;
		UINT uBeginPos=0;
		do
		{
			UINT uCopyPos=m_UserManage.FillNetCutUserInfo(bBuffer,sizeof(bBuffer),uBeginPos,bFinish);
			if (bFinish) 
			{
				if (m_TCPSocket.SendData(uSocketIndex,bBuffer,(UINT)uCopyPos*sizeof(UserInfoStruct),
					MDM_GR_USER_LIST,ASS_GR_NETCUT_USER,ERR_GR_LIST_FINISH,dwHandleID)!=uCopyPos*sizeof(UserInfoStruct))
					return false;
			}
			else 
			{
				if (m_TCPSocket.SendData(uSocketIndex,bBuffer,(UINT)uCopyPos*sizeof(UserInfoStruct),
					MDM_GR_USER_LIST,ASS_GR_NETCUT_USER,ERR_GR_LIST_PART,dwHandleID)!=(UINT)uCopyPos*sizeof(UserInfoStruct))
					return false;
			}
		} while (bFinish==false);
	}

	//发送桌子信息(在数据库中去处理桌子是否被购买)
	MSG_GR_DeskStation DeskStation;
	memset(&DeskStation,0,sizeof(DeskStation));
	for (UINT i=0;i<m_uDeskCount && i<100;i++)
	{
		if ((*(m_pDesk+i))->IsLock()==true) 
			DeskStation.bDeskLock[i]|=1;
		if ((*(m_pDesk+i))->IsVirtualLock()==true) 
			DeskStation.bVirtualDesk[i]|=1;
		if ((*(m_pDesk+i))->IsPlayingByGameStation())
			DeskStation.bDeskStation[i]|=1;
		DeskStation.bIsBuy[i]=(*(m_pDesk+i))->m_bIsBuy;//桌子是否被购买
		DeskStation.bDeskUser[i] = (*(m_pDesk+i))->GetDeskPlayerNum();
	}
	m_TCPSocket.SendData(uSocketIndex,&DeskStation,sizeof(DeskStation),MDM_GR_USER_LIST,ASS_GR_DESK_STATION,m_uDeskCount,dwHandleID);

	//发送用户进入消息
	MSG_GR_R_UserCome UserCome;
	UserCome.pUserInfoStruct=pUserInfo->m_UserData;
	m_TCPSocket.SendDataBatch(&UserCome,sizeof(UserCome),MDM_GR_USER_ACTION,ASS_GR_USER_COME,0);


	//发送完成登陆信息
	m_TCPSocket.SendData(uSocketIndex,MDM_GR_LOGON,ASS_GR_SEND_FINISH,0,dwHandleID);

	//发送消息
	if (m_szMsgRoomLogon[0]!=0) SendRoomMessage(uSocketIndex,dwHandleID,m_szMsgRoomLogon);

	return true;
}

//检测连接处理
bool CGameMainManage::OnCheckConnect(DataBaseResultLine * pResultData)
{
	if (m_bRun==true) SetTimer(IDT_CHECK_DATA_CONNECT,60000L);
	return true;
}

//房间登陆处理
bool CGameMainManage::OnRoomLogonResult(DataBaseResultLine * pResultData)
{
	BYTE bHandleCode=ERR_GR_ERROR_UNKNOW;
	switch (pResultData->uHandleRusult)
	{
	case DTR_GR_CONTEST_NOSIGNUP:
	case DTR_GR_CONTEST_NOSTART:
	case DTR_GR_LOGON_SUCCESS:		//登陆成功
		{
			//效验数据
			if (pResultData->LineHead.uSize!=sizeof(DL_GR_O_LogonResult)) return false;
			DL_GR_O_LogonResult * pLogonResult=(DL_GR_O_LogonResult *)pResultData;

			try
			{

				if (m_InitData.dwRoomRule & GRR_CONTEST || m_InitData.dwRoomRule & GRR_TIMINGCONTEST)
				{
					if (pResultData->uHandleRusult == DTR_GR_CONTEST_NOSIGNUP)
					{
						bHandleCode=ERR_GR_CONTEST_NOSIGNUP;
					}
					else if (pResultData->uHandleRusult == DTR_GR_CONTEST_NOSTART)
					{
						bHandleCode=ERR_GR_CONTEST_NOTSTRAT;
					}
					else
					{
					
					}
					
				}


				//判断是否还存在连接
				if (m_TCPSocket.IsConnectID(pResultData->uIndex,pResultData->dwHandleID)==false) throw DTR_GR_ERROR_UNKNOW;

				//查找在线用户
				CGameUserInfo * pGameUser=m_UserManage.FindOnLineUser(pLogonResult->pUserInfoStruct.dwUserID);
				if (pGameUser!=NULL)
				{
					m_TCPSocket.CloseSocket(pGameUser->m_uSocketIndex, 0);
					bHandleCode=ERR_GR_ACCOUNTS_IN_USE;
					break;
				}

				//效验用户数目
				if (((m_UserManage.GetGameRoomCount())>=m_UserManage.m_OnLineUserInfo.GetPermitCount()-m_UserManage.m_OnLineUserInfo.GetVipReseverCount()))
				{
					bHandleCode=ERR_GR_PEOPLE_FULL;
					break;
				}
				//恢复房间及重启服务器时更新玩家金币（可创建房间及比赛）
				if (m_InitData.uComType==TY_MONEY_GAME && (m_InitData.dwRoomRule & GRR_EXPERCISE_ROOM) && (m_InitData.dwRoomRule & GRR_GAME_BUY)) //创建房间
				{
					CGameDesk *pGameDesk=NULL;
					for (int i=0;i<m_InitData.uDeskCount;i++)
					{
						pGameDesk=(*(m_pDesk+i));
						if(!pGameDesk)
						{
							continue;
						}
						auto iter = find_if(pGameDesk->m_pReturnInfo.begin(),pGameDesk->m_pReturnInfo.end(),[=](struct MSG_GM_S_ReturnGameInfo obj){return obj.iUserID == pLogonResult->pUserInfoStruct.dwUserID;});
						if (iter != pGameDesk->m_pReturnInfo.end())
						{
							pLogonResult->pUserInfoStruct.i64Money = iter->i64Score;
							break;
						}
					}
				}
				//加入用户资料
				pGameUser=m_UserManage.ActiveUser(&pLogonResult->pUserInfoStruct,pResultData->uIndex,pResultData->dwHandleID,pLogonResult->dwGamePower,pLogonResult->dwMasterPower,0);
				if (pGameUser==NULL) throw DTR_GR_ERROR_UNKNOW;
				SetTimer(IDT_UPDATE_ROOM_PEOPLE,100);//20081201

				//设置用户信息
				pGameUser->SetMD5Pass(pLogonResult->szMD5Pass);
				
				//房间管理窗口数据
				//先查找是否存在该管理员用户名，存在则跳过，否则添加
				if (!IsExistGRMUserID(pLogonResult->pUserInfoStruct.dwUserID))
				{
					CBcfFile f(CBcfFile::GetAppPath()+"GRMInfo.bcf");
					CString _UserID;
					_UserID.Format("%ld",pLogonResult->pUserInfoStruct.dwUserID);
					bool bUserGRM = f.GetKeyVal("UserID",_UserID, 0);

					if (bUserGRM)//只有确认有效才添加
					{
						m_GRMUserID.push_back(pLogonResult->pUserInfoStruct.dwUserID);
						pLogonResult->bGRMUser = true;
					}
				}
				else
				{
					pLogonResult->bGRMUser = true;
				}

				if (!IsExistGRMRoom(pLogonResult->bGRMRoomID))
				{
					pLogonResult->bGRMRoomID = 0;
				}

				//发送房间信息

				//增加即时获取房间虚拟玩家参数
				pLogonResult->ResultHead.uHandleRusult = bHandleCode;
				if (SendRoomInfo(pGameUser,pResultData->uIndex,pResultData->dwHandleID,pLogonResult,!pGameUser->m_UserData.bLogonbyphone)==false)
				{
					throw DTR_GR_ERROR_UNKNOW;
				}

				if (1 == m_InitData.iRoomState && (m_InitData.dwRoomRule & GRR_CONTEST || m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
				{
					AutoAllotDeskAddUser(pGameUser);
				}

				//修改禁止登录问题，修改插入TWLoginRecord 记录时机
				DL_GR_I_WriteWLoginRecord DlWriteWLogin;
				DlWriteWLogin.dwUserID = pGameUser->m_UserData.dwUserID;
				DlWriteWLogin.uRoomID = m_InitData.uRoomID;
				m_SQLDataManage.PushLine(&DlWriteWLogin.DataBaseHead,sizeof(DlWriteWLogin),DTK_GR_WRITE_WLOGINRECORD,0,0);

			}
			catch (...)
			{
				TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
				///////////////////////////////
				//防止更新数据库后，用户登陆失败
				if (pLogonResult->bRegOnline==TRUE)
				{
					DL_GR_I_UnRegisterUser UnRegisterUser;
					UnRegisterUser.dwUserID=pLogonResult->pUserInfoStruct.dwUserID;
					m_SQLDataManage.PushLine(&UnRegisterUser.DataBaseHead,sizeof(UnRegisterUser),DTK_GR_UNREGISTER_ONLINE,0,0);
				}
				///////////////////////////////
				return false;
			}

			return true;
		}
	case DTR_GR_IN_OTHER_ROOM:		//在其他房间
		{
			//效验数据
			if (pResultData->LineHead.uSize!=sizeof(DL_GR_O_LogonError)) return false;
			DL_GR_O_LogonError * pLogonResult=(DL_GR_O_LogonError *)pResultData;

			//处理数据
			MSG_GR_R_OtherRoom MSGOtherRoom;
			ZeroMemory(&MSGOtherRoom, sizeof(MSG_GR_R_OtherRoom));
			if (pLogonResult != NULL)
			{
				MSGOtherRoom.iRoomID = pLogonResult->uRoomID;
				lstrcpy(MSGOtherRoom.szGameRoomName,pLogonResult->szGameRoomName);
			}
			m_TCPSocket.SendData(pResultData->uIndex,&MSGOtherRoom,sizeof(MSGOtherRoom),MDM_GR_LOGON,
				ASS_GR_LOGON_ERROR,ERR_GR_IN_OTHER_ROOM,pResultData->dwHandleID);

			return true;
		}
	case DTR_GR_USER_NO_FIND: {	bHandleCode=ERR_GR_USER_NO_FIND; break; }
	case DTR_GR_USER_PASS_ERROR: { bHandleCode=ERR_GR_USER_PASS_ERROR; break; }
	case DTR_GR_USER_VALIDATA: { bHandleCode=ERR_GR_STOP_LOGON; break; }
	case DTR_GR_USER_IP_LIMITED: { bHandleCode=ERR_GR_USER_IP_LIMITED; break; }
	case DTR_GR_IP_NO_ORDER: { bHandleCode=ERR_GR_IP_NO_ORDER; break; }
	case DTR_GR_ONLY_MEMBER: { bHandleCode=ERR_GR_ONLY_MEMBER; break; }
	case DTR_GR_MATCH_LOGON: { bHandleCode=ERR_GR_MATCH_LOGON; break; }	
	case DTR_GR_OVER_TIMER:{bHandleCode=ERR_GR_TIME_OVER;break;}//游戏时间到期
	//wushuqun 2009.6.5
	//不在混战场活动时间内
	case DTR_GR_BATTLEROOM_TIMEOUT:
		{
			bHandleCode=ERR_GR_BATTLEROOM_OUTTIME;
			break;
		}
	case DTR_GR_CONTEST_TIMEROUT:
		{
			bHandleCode = ERR_GR_CONTEST_TIMEOUT;
			break;
		}
	case DTR_GR_CONTEST_OVER:
		{
			bHandleCode = ERR_GR_CONTEST_OVER;
			break;
		}

	case DTR_GR_CONTEST_BEGUN:
		{
			bHandleCode = ERR_GR_CONTEST_BEGUN;
			break;
		}
	}

	//默认处理
	m_TCPSocket.SendData(pResultData->uIndex,MDM_GR_LOGON,ASS_GR_LOGON_ERROR,bHandleCode,pResultData->dwHandleID);

	return true;
}

//根据用户ID获取昵称
bool CGameMainManage::OnGetNickNameOnIDResult(DataBaseResultLine * pResultData)
{
    BYTE bHandleCode=ERR_GR_ERROR_UNKNOW;
	
	//游戏中获取昵称特殊处理
	int nMainID = MDM_GR_ROOM;
	int nAssistantID = ASS_GR_GET_NICKNAME_ONID;
	if(pResultData->uHandleKind == DTK_GR_GETNICKNAME_ONID_INGAME)
	{
		nMainID = MDM_GR_ROOM2GAME;
		nAssistantID = ASS_GR_GET_NICKNAME_ONID_INGAME;
	}

    switch (pResultData->uHandleRusult)
    {
    case DTR_GR_GETNICKNAME_SUCCESS:
        {
            //效验数据
            if (pResultData->LineHead.uSize!=sizeof(DL_GR_O_GetNickNameOnIDResult)) return false;
            DL_GR_O_GetNickNameOnIDResult* pGetNickNameOnID =(DL_GR_O_GetNickNameOnIDResult *)pResultData;

            //处理数据
            bHandleCode = RES_GR_GETNICKNAME_SUCCESS;
            MSG_GR_S_GetNickNameOnID_t stGetNickNameOnID;
            stGetNickNameOnID.iUserID = pGetNickNameOnID->iUserID;
            lstrcpy(stGetNickNameOnID.szNickName, pGetNickNameOnID->szNickName);
            m_TCPSocket.SendData(pResultData->uIndex,&stGetNickNameOnID,sizeof(stGetNickNameOnID),
                            nMainID, nAssistantID,bHandleCode,pResultData->dwHandleID);

            return true;
        }
        break;
    case DTR_GR_GETNICKNAME_NOTEXIST:
        {
            bHandleCode=RES_GR_GETNICKNAME_NOTEXIST;
        }
        break;
    case DTR_GR_GETNICKNAME_ISNULL:
        {
            bHandleCode=RES_GR_GETNICKNAME_ISNULL;
        }
        break;
    default:
        {
            bHandleCode=ERR_GR_ERROR_UNKNOW;
        }
        break;
    }
    m_TCPSocket.SendData(pResultData->uIndex, nMainID, nAssistantID, 
                        bHandleCode, pResultData->dwHandleID);

    return true;
}

// 踢人卡、防踢卡功能
bool CGameMainManage::OnUseKickPropResult(DataBaseResultLine * pResultData, BOOL bIsKickUser)
{
    int iHandleIDNum = ASS_GM_USE_KICK_PROP;
    if(FALSE == bIsKickUser)
    {
        iHandleIDNum = ASS_GM_USE_ANTI_KICK_PROP;
    }

    BYTE bHandleCode=ERR_GR_ERROR_UNKNOW;
    switch (pResultData->uHandleRusult)
    {
    case DTR_GR_USE_KICKPROP_NULL:      // 没有道具
        {
            bHandleCode = ERR_GR_KICK_PROP_NULL;
        }
        break;
    case DTR_GR_USE_KICKPROP_SET_ERROR: // 设置时间错误
        {
            bHandleCode = ERR_GR_KICK_SET_TIME_FAIL;
        }
        break;
    case DTR_GR_USE_KICKPROP_SET_SUCCESS:   // 设置时间成功
        {
            bHandleCode = ERR_GR_KICK_SET_TIME_SUCCESS;
        }
        break;
    }

    DL_GR_O_UseKickProp* pUseKickProp = (DL_GR_O_UseKickProp*)pResultData;
    MSG_GR_RS_KickProp KickProp;
    memset(&KickProp, 0, sizeof(KickProp));
    KickProp.dwUserID = pUseKickProp->dwUserID;
    KickProp.iPropID = pUseKickProp->iPropID;
    KickProp.iTotalTime = pUseKickProp->iTotalTime;

    m_TCPSocket.SendData(pResultData->uIndex, &KickProp, sizeof(KickProp), MDM_GM_GAME_FRAME, 
                         iHandleIDNum, bHandleCode, pResultData->dwHandleID);
    return true;
}

// 使用踢人卡踢人
bool CGameMainManage::OnNewKickUserPropResult(DataBaseResultLine * pResultData)
{
    DL_GR_O_UseNewKickUserProp* pNewKickProp = (DL_GR_O_UseNewKickUserProp*)pResultData;

    BYTE bHandleCode = ERR_GR_ERROR_UNKNOW;
    switch (pResultData->uHandleRusult)
    {
    case DTR_GR_NEW_KICKUSER_SUCCESS:
        bHandleCode = ERR_GR_NEW_KICKUSER_SUCCESS;
        break;
    case DTR_GR_NEW_KICKUSER_NOTIME:
        bHandleCode = ERR_GR_NEW_KICKUSER_NOTIME;
        break;
    case DTR_GR_NEW_KICKUSER_HAD_ANTI:
        bHandleCode = ERR_GR_NEW_KICKUSER_HAD_ANTI;
        break;
    case DTR_GR_NEW_KICKUSER_HAD_VIP:
        bHandleCode = ERR_GR_NEW_KICKUSER_HAD_VIP;
        break;
    }

    _TAG_NEW_KICKUSER stKick;
    stKick.dwUserID = pNewKickProp->dwUserID;
    stKick.dwDestID = pNewKickProp->dwDestID;

    // 操作成功
    if(ERR_GR_NEW_KICKUSER_SUCCESS == bHandleCode)
    {
        CGameUserInfo * pUserInfo = m_UserManage.GetOnLineUserInfo(pResultData->uIndex);
        if(pUserInfo == NULL) return true;
        CGameUserInfo * pDestUserInfo = m_UserManage.FindOnLineUser(pNewKickProp->dwDestID);
        if(pDestUserInfo == NULL) return true;
        if(pUserInfo->m_UserData.bDeskNO == 255)return true;

        CGameDesk *pDesk = GetDeskObject(pUserInfo->m_UserData.bDeskNO);
        if(pDesk == NULL) return true;

        if(pDesk->IsPlayingByGameStation()) //游戏中踢人
        {
            bHandleCode = ERR_GR_NEW_KICKUSER_PLAYING;
        }
        else
        {
            if (pDestUserInfo->m_UserData.bUserState==USER_WATCH_GAME) 
            { 
                pDesk->WatchUserLeftDesk(pDestUserInfo->m_UserData.bDeskStation,pDestUserInfo); 
            } 
            else  
            { 
				pDesk->m_bLeave = true;
                pDesk->UserLeftDesk(pDestUserInfo->m_UserData.bDeskStation,pDestUserInfo); 
            }
        } 
    }

    // 给自己发消息
    m_TCPSocket.SendData(pResultData->uIndex, &stKick, sizeof(stKick), MDM_GR_PROP, ASS_PROP_NEW_KICKUSER, 
                         bHandleCode, pResultData->dwHandleID);

    // 给目标发消息
    m_TCPSocket.SendData(pNewKickProp->dwDestIndex, &stKick, sizeof(stKick), MDM_GR_PROP, ASS_PROP_NEW_KICKUSER, 
                        bHandleCode, pNewKickProp->dwDestHandleID);

    return true;
}


// 踢人卡踢人功能
bool CGameMainManage::OnNewKickUserProp(NetMessageHead *pNetHead, void* pData, UINT uSize, 
                                        ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
    if(uSize != sizeof(_TAG_NEW_KICKUSER))return false;
    _TAG_NEW_KICKUSER* pNewKickUser = (_TAG_NEW_KICKUSER*)pData;

    CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
    if(pUserInfo == NULL) return false;
    CGameUserInfo* pDestUserInfo = m_UserManage.FindOnLineUser(pNewKickUser->dwDestID);
    if(NULL == pDestUserInfo) return false;

    DL_GR_I_NewKickUserProp KickProp;
    memset(&KickProp, 0, sizeof(KickProp));
    KickProp.dwUserID = pNewKickUser->dwUserID;
    KickProp.dwDestID = pNewKickUser->dwDestID;
    KickProp.dwDestIndex = pDestUserInfo->m_uSocketIndex;
    KickProp.dwDestHandleID = pDestUserInfo->m_dwHandleID;
    m_SQLDataManage.PushLine(&KickProp.DataBaseHead, sizeof(KickProp), DTK_GR_USER_NEW_KICK_USER_PROP, 
                              uIndex, dwHandleID);
    return true;
}

//钱柜划账
bool CGameMainManage::OnAboutMoneyResult(DataBaseResultLine * pResultData, UINT uCode)
{
	// 区别不同的指令
	UINT uCmd = ASS_GR_OPEN_WALLET;
	if (DTK_GR_DEMAND_MONEY_IN_GAME==uCode)
	{
		uCmd = ASS_GR_OPEN_WALLET_INGAME;
	}
	BYTE bHandleCode=ERR_GR_ERROR_UNKNOW;
	switch (pResultData->uHandleRusult)
	{
	case DTR_GR_OPEN_WALLET_ERROR://打开钱柜错误
		{
			bHandleCode=ASS_GR_OPEN_WALLET_ERROR;
			m_TCPSocket.SendData(pResultData->uIndex, MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
			break;
		}
	case DTR_GR_OPEN_WALLET_ERROR_PSW://打开钱柜二级密码错误
		{
			bHandleCode = ASS_GR_OPEN_WALLET_ERROR_PSW;
			m_TCPSocket.SendData(pResultData->uIndex, MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
			break;
		}		
	case DTR_GR_OPEN_WALLET_SUC://打开钱柜成功
		{
			bHandleCode=ASS_GR_OPEN_WALLET_SUC;
			DL_GR_O_OpenWalletResult *pOpenWalletResult=(DL_GR_O_OpenWalletResult*) pResultData;
			MSG_GR_UserBank UserBank = pOpenWalletResult->stUserBank;

			//打开银行后更新内存中的银行数值
			CGameUserInfo* pUserInfo = m_UserManage.FindOnLineUser(UserBank.dwUserID);
			if (pUserInfo)
			{
				pUserInfo->m_UserData.i64Bank = UserBank.i64Bank;
			}
			
			m_TCPSocket.SendData(pResultData->uIndex,&UserBank,sizeof(UserBank),MDM_GR_MONEY, uCmd, bHandleCode,pResultData->dwHandleID);
			break;
		}

	}

	return true;
}

//将返利数据返还给房间sdp
bool CGameMainManage::OnGetBankRebateResponse(DataBaseResultLine* pResultData)
{
	DL_GR_O_RebateUpDateStruct *pCmd=(DL_GR_O_RebateUpDateStruct*)pResultData;

	m_iVIP_Rate = pCmd->iVIP_Rate;

	return true;
}

//将玩家返利查询数据返还给房间sdp
bool CGameMainManage::OnGetBankUserRebateResponse(DataBaseResultLine* pResultData, UINT uCode)
{
	// 区别不同的指令
	UINT uCmd = ASS_GR_TRANSFER_UPDATE_USER_REBATE;
	if (DTK_GR_BANK_UPDATE_USER_REBATE_INGAME==uCode)
	{
		uCmd = ASS_GR_TRANSFER_UPDATE_USER_REBATE_INGAME;
	}
	DL_GR_O_UserRebateUpDateStruct *pCmd=(DL_GR_O_UserRebateUpDateStruct*)pResultData;

	MSG_GR_UserRebateUpDate cmd;//将更新数据发给房间
	cmd.iUserID = pCmd->iUserID;				//玩家ID
	cmd.iTx_Money = pCmd->iTx_Money;			//剩余返利数目
	cmd.iTx_Smony = pCmd->iTx_Smony;			//已经取走的数目

	m_TCPSocket.SendData(pResultData->uIndex, &cmd, sizeof(MSG_GR_UserRebateUpDate), MDM_GR_MONEY, uCmd, 0, pResultData->dwHandleID);

	return true;
}

//将玩家返利修改结果返还给房间
bool CGameMainManage::OnGetBankUserSetRebateResponse(DataBaseResultLine* pResultData, UINT uCode)
{
	// 区别不同的指令
	UINT uCmd = ASS_GR_TRANSFER_SET_REBATE_RES;
	if (DTK_GR_BANK_SET_USER_REBATE_INGAME==uCode)
	{
		uCmd = ASS_GR_TRANSFER_SET_REBATE_RES_INGAME;
	}
	DL_GR_O_UserRebateSetStruct *pCmd=(DL_GR_O_UserRebateSetStruct*)pResultData;

	MSG_GR_UserRebateSetResult cmd;//将更新数据发给房间
	cmd.iUserID = pCmd->iUserID;					//玩家ID
	cmd.iMoney = pCmd->iMoney;						//操作后数目
	cmd.bTag = pCmd->bTag;							//0 存，1 取
	cmd.bResult = pCmd->bResult;					//处理结果
	CopyMemory(cmd.Msg, pCmd->Msg, sizeof(cmd.Msg));//消息

	m_TCPSocket.SendData(pResultData->uIndex, &cmd, sizeof(MSG_GR_UserRebateSetResult), MDM_GR_MONEY, uCmd, 0, pResultData->dwHandleID);

	if (true == pCmd->bResult)//执行成功才帮大厅刷新数据
	{
		UINT uHandleKind = DTK_GR_BANK_UPDATE_USER_REBATE;
		if (DTK_GR_BANK_SET_USER_REBATE_INGAME==uCode)
		{
			uHandleKind = DTK_GR_BANK_UPDATE_USER_REBATE_INGAME;
		}

		//查询玩家返利数据并告知玩家
		DL_GR_I_UserRebateUpDateStruct UserRebate;
		UserRebate.iUserID = pCmd->iUserID;//赋值玩家id
		m_SQLDataManage.PushLine(&UserRebate.DataBaseHead, sizeof(UserRebate), uHandleKind, pResultData->uIndex, 0);
	}

	return true;
}

//房间－＞钱柜
bool CGameMainManage::OnCheckInMoney(DataBaseResultLine * pResultData, UINT uCode)
{
	// 区别不同的指令
	UINT uCmd = ASS_GR_CHECK_IN;
	if (DTK_GR_CHECKIN_MONEY_INGAME==uCode)
	{
		uCmd = ASS_GR_CHECK_IN_INGAME;
	}

	BYTE bHandleCode=ERR_GR_ERROR_UNKNOW;
	switch (pResultData->uHandleRusult)
	{
	case DTR_GR_ERROR_UNKNOW://
		{
			bHandleCode=ASS_GR_CHECKIN_ERROR;
			m_TCPSocket.SendData(pResultData->uIndex, MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
			break;
		}
	case DTR_GR_CHECKIN_MONEY_INV:
		{
			bHandleCode=ASS_GR_CHECKIN_ERROR_PLAYING;
			m_TCPSocket.SendData(pResultData->uIndex, MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
			break;
		}
	case DTR_GR_CHECKIN_MONEY_SUC://存入成功
		{
			bHandleCode=ASS_GR_CHECKIN_SUC;
			//效验数据
			if (pResultData->LineHead.uSize!=sizeof(DL_GR_O_CheckOutMoney)) 
			{
				bHandleCode=ASS_GR_CHECKIN_ERROR;
				m_TCPSocket.SendData(pResultData->uIndex, MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
				return false;
			}
			DL_GR_O_CheckOutMoney * pCheckInResult=(DL_GR_O_CheckOutMoney *)pResultData;

			//修改用户资料

			CGameUserInfo * pUserInfo=m_UserManage.FindOnLineUser(pCheckInResult->dwUserID);
			if (pUserInfo==NULL) return false;
			else
			{
				pUserInfo->m_UserData.i64Money-=pCheckInResult->i64CheckOut;
				pUserInfo->m_UserData.i64Bank+=pCheckInResult->i64CheckOut;
			}

			MSG_GR_CheckMoney CheckInMoney;
			memset(&CheckInMoney,0,sizeof(CheckInMoney));
			CheckInMoney.i64CheckOut=pCheckInResult->i64CheckOut;
			CheckInMoney.i64MoneyInBank=pCheckInResult->i64MoneyInBank;
			CheckInMoney.i64MoneyInRoom=pCheckInResult->i64MoneyInRoom;
			CheckInMoney.dwUserID=pCheckInResult->dwUserID;
			CheckInMoney.uGameID = m_KernelData.uNameID;
			m_TCPSocket.SendDataBatch(&CheckInMoney,sizeof(CheckInMoney),MDM_GR_MONEY,uCmd,bHandleCode);
			break;
		}
	}

	return true;
}

//房间＜－钱柜
bool CGameMainManage::OnCheckOutMoney(DataBaseResultLine * pResultData, UINT uCode)
{
	// 区别不同的指令
	UINT uCmd = ASS_GR_CHECK_OUT;
	if (DTK_GR_CHECKOUT_MONEY_INGAME==uCode)
	{
		uCmd = ASS_GR_CHECK_OUT_INGAME;
	}

	BYTE bHandleCode=ERR_GR_ERROR_UNKNOW;
	switch (pResultData->uHandleRusult)
	{
	case DTR_GR_ERROR_UNKNOW://
		{
			bHandleCode=ASS_GR_CHECKOUT_ERROR;
			m_TCPSocket.SendData(pResultData->uIndex, MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
			break;
		}
	case DTR_GR_CHECKOUT_MONEY_INV:
		{
			bHandleCode=ASS_GR_CHECKOUT_ERROR_PLAYING;
			m_TCPSocket.SendData(pResultData->uIndex, MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
			break;
		}
	case DTR_GR_CHECKOUT_MONEY_SUC://取出成功
		{
			bHandleCode=ASS_GR_CHECKOUT_SUC;
			//效验数据
			if (pResultData->LineHead.uSize!=sizeof(DL_GR_O_CheckOutMoney)) 
			{
				bHandleCode=ASS_GR_CHECKOUT_ERROR;
				m_TCPSocket.SendData(pResultData->uIndex, MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
				return false;
			}
			DL_GR_O_CheckOutMoney * pCheckOutResult=(DL_GR_O_CheckOutMoney *)pResultData;


			CGameUserInfo * pUserInfo=m_UserManage.FindOnLineUser(pCheckOutResult->dwUserID);
			if (pUserInfo==NULL) return false;
			else
			{
				pUserInfo->m_UserData.i64Money+=pCheckOutResult->i64CheckOut;
				pUserInfo->m_UserData.i64Bank-=pCheckOutResult->i64CheckOut;
			}

			MSG_GR_CheckMoney CheckOutMoney;
			memset(&CheckOutMoney,0,sizeof(CheckOutMoney));
			CheckOutMoney.i64CheckOut=pCheckOutResult->i64CheckOut;
			CheckOutMoney.i64MoneyInBank=pCheckOutResult->i64MoneyInBank;
			CheckOutMoney.i64MoneyInRoom=pCheckOutResult->i64MoneyInRoom;
			CheckOutMoney.dwUserID=pCheckOutResult->dwUserID;
			CheckOutMoney.uGameID = m_KernelData.uNameID;
			m_TCPSocket.SendDataBatch(&CheckOutMoney,sizeof(CheckOutMoney),MDM_GR_MONEY,uCmd,bHandleCode);
			break;
		}
	}

	return true;
}
//转帐记录结果
bool CGameMainManage::OnTransferRecord(DataBaseResultLine * pResultData, UINT uCode)
{
	UINT uCmd = ASS_GR_TRANSFER_RECORD;
	if (DTK_GR_TRANSFER_RECORD_INGAME == uCode)
	{
		uCmd = ASS_GR_TRANSFER_RECORD_INGAME;
	}
	if (pResultData->LineHead.uSize!=sizeof(DL_GR_O_TransferRecord_t)) 
	{
		return false;
	}
	DL_GR_O_TransferRecord_t *pTransferRecordResult = (DL_GR_O_TransferRecord_t *)pResultData;
	MSG_GR_R_TransferRecord_t transferRecord;
	transferRecord.nSeriNo			= pTransferRecordResult->transferRecord.nSeriNo;
	transferRecord.dwUserID			= pTransferRecordResult->transferRecord.dwUserID;
	transferRecord.destUserID		= pTransferRecordResult->transferRecord.destUserID;
	transferRecord.i64Money			= pTransferRecordResult->transferRecord.i64Money;
	transferRecord.i64ActualTransfer	= pTransferRecordResult->transferRecord.i64ActualTransfer;
	transferRecord.oleDateTime		= pTransferRecordResult->transferRecord.oleDateTime;

	BYTE bHandleCode=ASS_GR_TRANSFER_SUC;

	m_TCPSocket.SendData(pResultData->uIndex, &transferRecord, sizeof(transferRecord), MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);

	return true;
}

/// 修改密码
bool CGameMainManage::OnChangePasswd(DataBaseResultLine * pResultData, UINT uCode)
{
	// 区别不同的指令
	UINT uCmd = ASS_GR_CHANGE_PASSWD;
	if (DTK_GR_CHANGE_PASSWD_INGAME==uCode)
	{
		uCmd = ASS_GR_CHANGE_PASSWD_INGAME;
	}
	BYTE bHandleCode=ERR_GR_ERROR_UNKNOW;
	if (pResultData->LineHead.uSize!=sizeof(DL_GR_O_ChangePasswd_t)) 
	{
		bHandleCode=ASS_GR_PASSWORD_ERROR;
		m_TCPSocket.SendData(pResultData->uIndex, MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
		return true;
	}
	
	if (pResultData->uHandleRusult != DTR_GR_OPEN_WALLET_SUC)
	{
		bHandleCode=ASS_GR_PASSWORD_ERROR;
		m_TCPSocket.SendData(pResultData->uIndex, MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
	}
	else
	{
		bHandleCode=ASS_GR_OPEN_WALLET_SUC;
		m_TCPSocket.SendData(pResultData->uIndex, MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
	}
	
	return true;
}

/// 转帐
bool CGameMainManage::OnTransferMoney(DataBaseResultLine * pResultData, UINT uCode)
{
	// 区别不同的指令
	UINT uCmd = ASS_GR_TRANSFER_MONEY;
	if (DTK_GR_TRANSFER_MONEY_INGAME==uCode)
	{
		uCmd = ASS_GR_TRANSFER_MONEY_INGAME;
	}

	BYTE bHandleCode=ERR_GR_ERROR_UNKNOW;
	if (pResultData->LineHead.uSize!=sizeof(DL_GR_O_TransferMoney)) 
	{
		bHandleCode=ASS_GR_TRANSFER_ERROR;
		m_TCPSocket.SendData(pResultData->uIndex, MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
		return false;
	}
	DL_GR_O_TransferMoney * pTransferMoney=(DL_GR_O_TransferMoney *)pResultData;
	MSG_GR_S_TransferMoney TransferMoney;
	TransferMoney.destUserID = pTransferMoney->stTransfer.destUserID;
	TransferMoney.i64Money = pTransferMoney->stTransfer.i64Money;
	TransferMoney.UserID = pTransferMoney->stTransfer.UserID;
	TransferMoney.bUseDestID = pTransferMoney->stTransfer.bUseDestID;
	strcpy(TransferMoney.szNickName, pTransferMoney->stTransfer.szNickName);		//增加昵称信息
	strcpy(TransferMoney.szDestNickName, pTransferMoney->stTransfer.szDestNickName);
	TransferMoney.i64ActualTransfer = pTransferMoney->stTransfer.i64ActualTransfer;
	switch (pResultData->uHandleRusult)
	{
	case DTR_GR_OPEN_WALLET_ERROR_PSW: //密码错误
		{
			bHandleCode=ASS_GR_PASSWORD_ERROR;
			m_TCPSocket.SendData(pResultData->uIndex, &TransferMoney, sizeof(TransferMoney), MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
			break;
		}
	case DTR_GR_TRANSFER_TOTAL_NOT_ENOUGH:	///< 银行存款太少，不允许转帐
		{
			bHandleCode=ASS_GR_TRANSFER_TOTAL_LESS;
			m_TCPSocket.SendData(pResultData->uIndex, &TransferMoney, sizeof(TransferMoney), MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
			break;
		}
	case DTR_GR_TRANSFER_TOO_LESS:			///<单笔转帐额太小
		{
			bHandleCode=ASS_GR_TRANSFER_TOO_LESS;
			m_TCPSocket.SendData(pResultData->uIndex, &TransferMoney, sizeof(TransferMoney), MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
			break;
		}
	case DTR_GR_TRANSFER_MULTIPLE:			///<单笔转帐额必须是某数的整数倍
		{
			bHandleCode=ASS_GR_TRANSFER_MULTIPLE;
			m_TCPSocket.SendData(pResultData->uIndex, &TransferMoney, sizeof(TransferMoney), MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
			break;
		}
	case DTR_GR_TRANSFER_NO_ENOUGH_MONEY:	///<银行存款不足本次转帐
		{
			bHandleCode=ASS_GR_TRANSFER_NOT_ENOUGH;
			m_TCPSocket.SendData(pResultData->uIndex, &TransferMoney, sizeof(TransferMoney), MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
			break;
		}
	case DTR_GR_TRANSFER_TOO_MANY_TIME:  ///< 散户间当天转账数额达到一定限度
		{
			bHandleCode = ASS_GR_TRANSFER_TOO_MANY_TIME;
			m_TCPSocket.SendData(pResultData->uIndex, &TransferMoney, sizeof(TransferMoney), MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
			break;
		}
	case DTR_GR_TRANSFER_MONEY_NO_DEST:		///< 目标不存在
		{
			bHandleCode=ASS_GR_TRANSFER_NO_DEST;
			m_TCPSocket.SendData(pResultData->uIndex, &TransferMoney, sizeof(TransferMoney), MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
			break;
		}
	case DTR_GR_ERROR_UNKNOW://转帐失败
		{
			bHandleCode=ASS_GR_TRANSFER_ERROR;
			m_TCPSocket.SendData(pResultData->uIndex, &TransferMoney, sizeof(TransferMoney), MDM_GR_MONEY, uCmd, bHandleCode, pResultData->dwHandleID);
			break;
		}
	case DTR_GR_TRANSFER_MONEY_SUC://转帐成功
		{
			bHandleCode=ASS_GR_TRANSFER_SUC;
			//效验数据
			CGameUserInfo * pUserInfo=m_UserManage.FindOnLineUser(pTransferMoney->stTransfer.UserID);
			if (pUserInfo==NULL)
			{
				return false;
			}

			m_TCPSocket.SendData(pResultData->uIndex,&TransferMoney,sizeof(TransferMoney),MDM_GR_MONEY,uCmd,bHandleCode, pResultData->dwHandleID);
			
			//转账成功后继续判断玩家是否为vip，进行返利操作
			CString stime;
			stime.Format("%d",CTime::GetCurrentTime());
			int curtime=atoi(stime);
			if(pUserInfo->m_UserData.iVipTime>curtime)
			{
				DL_GR_I_UserRebateSetStruct cmd;
				cmd.bTag = 0;
				cmd.iUserID = pTransferMoney->stTransfer.UserID;
				double iRate = (pTransferMoney->stTransfer.i64Money - pTransferMoney->stTransfer.i64ActualTransfer) * m_iVIP_Rate / 100;
				cmd.iMoney = iRate;

				UINT uHandleKind = DTK_GR_BANK_SET_USER_REBATE;
				if (DTK_GR_TRANSFER_MONEY_INGAME==uCode)
				{
					uHandleKind = DTK_GR_BANK_SET_USER_REBATE_INGAME;
				}

				m_SQLDataManage.PushLine(&cmd.DataBaseHead,sizeof(DL_GR_I_UserRebateSetStruct),
					uHandleKind,pResultData->uIndex,pResultData->dwHandleID);
			}

			break;
		}
	}

	return true;
}
bool CGameMainManage::SendUserMoenyByTimesResult(DataBaseResultLine * pResultData)
{

	//赠送金币成功
	if (pResultData->uHandleRusult == 0)
	{
		//BYTE bHandleCode = 0;
		//
		//SendUserMoneyByTimes senResult;
		//memset(&senResult,0,sizeof(senResult));
		////memcpy(&senResult,pResultData,sizeof(senResult));
		//DL_GR_O_SendMonet_By_Times* pSendMoeny = (DL_GR_O_SendMonet_By_Times*)pResultData;
		//senResult.dwAllSendTImes = pSendMoeny->dwAllSendTImes;
		//senResult.dwSendFailedCause = pSendMoeny->dwSendFailedCause;
		//senResult.dwSendGameMoney = pSendMoeny->dwSendGameMoney;
		//senResult.dwSendTimes = pSendMoeny->dwSendTimes;
		//senResult.dwSrcGameMoney = pSendMoeny->dwSrcGameMoney;
		//senResult.dwSrcMoneyInBank = pSendMoeny->dwSrcMoneyInBank;
		//senResult.dwUserID = pSendMoeny->dwUserID;
		//CGameUserInfo* pGameUser = m_UserManage.FindOnLineUser(senResult.dwUserID);
		//if (pGameUser != NULL)
		//{
		//	pGameUser->SetUserSendedMoeny(senResult.dwSendGameMoney);
		//	m_TCPSocket.SendDataBatch(&senResult,sizeof(senResult),MDM_GR_MONEY,ASS_GR_SENDMONEY_TIMES,bHandleCode);
		//}
		
		///< 已经在void CGameDesk::PresentCoin()中发送给客户端。
	}
	return true;

}

bool CGameMainManage::OnGetContestApplyResult(DataBaseResultLine *pResultData)
{
	DL_GR_O_ContestApply *pOContestApply = (DL_GR_O_ContestApply*)pResultData;
	if (NULL == pOContestApply)
		return false;
	CGameUserInfo* pUserInfo = m_UserManage.FindOnLineUser(pOContestApply->_data.iUserID);
	if (NULL == pUserInfo)
		return true;
	m_TCPSocket.SendData(pUserInfo->m_uSocketIndex,&pOContestApply->_data, sizeof(pOContestApply->_data), MDM_GR_USER_ACTION, ASS_GR_CONTEST_APPLY, 0, 0);

	if(1 == pOContestApply->iContestBegin)
	{
		if ((m_InitData.dwRoomRule & GRR_CONTEST) && 1 != m_InitData.iRoomState)
		{	
			BeforeContestBegin();
		}
	}

	return true;
}

bool CGameMainManage::OnGetContestAbandonResult(DataBaseResultLine *pResultData)
{
	DL_GR_O_ContestAbandon *p = (DL_GR_O_ContestAbandon*)pResultData;
	if (NULL == p)
		return false;
	if (p->iUserNum>0)
	{
		for (int i=0;i<p->iUserNum;i++)
		{
			CGameUserInfo* pUserInfo = m_UserManage.FindOnLineUser(p->pData[i].iUserID);
			if (pUserInfo)
				m_TCPSocket.SendData(pUserInfo->m_uSocketIndex, &p->pData[i],sizeof(MSG_GR_S_ContestAbandon),MDM_GR_ROOM, ASS_GR_CONTEST_ABANDON, 0, 0);
		}
		SafeDeleteArray(p->pData);
	}
	
	OnContestRoomReset();

	return true;
}

bool CGameMainManage::OnBroadCastConstestInfo(DataBaseResultLine * pResultData)
{
	DL_GR_O_ContestInfo *pOContestInfo = (DL_GR_O_ContestInfo*)pResultData;
	if (NULL == pOContestInfo)
		return false;
	MSG_GR_I_ContestInfo msgContestInfo;
	msgContestInfo.iContestNum = pOContestInfo->_data.iContestNum;
	msgContestInfo.iContestBegin = pOContestInfo->_data.iContestBegin;
	m_TCPSocket.SendDataBatch(&msgContestInfo,sizeof(msgContestInfo),MDM_GR_ROOM,ASS_GR_CONTEST_APPLYINFO,0);

	return true;
}

//随机初始化排名
bool CGameMainManage::RandRank(int *pData, int iLen)
{
	if (!pData || 0 >= iLen)
		return false;
	int index;
	srand(time(NULL));
	for (int i = 0; i <iLen; i++)
	{
		index = rand() % (iLen - i) + i;//获取随机索引  
		if (index != i)
		{//将两个数值交换
			pData[i] ^= pData[index];
			pData[index] ^= pData[i];
			pData[i] ^= pData[index];
		}
	}
	return true;
}

//比赛开始前，从数据库获取比赛ID
bool CGameMainManage::OnGetContestBeginResult(DataBaseResultLine *pResultData)
{
	DL_GR_O_ContestBegin *p = (DL_GR_O_ContestBegin*)pResultData;

	if (!p || 0 >= p->iUserNum || p->iUserNum >= 255)
	{
		OnContestRoomReset();
		return true;
	}

	m_InitData.iMatchID = p->uMatchID;
	m_ContestTimeover = false;

	m_iResetTime = 0;
	CGameUserInfo * pUserInfo = NULL;
	//随机分配初始排名
	int pRankNum[255];
	for (int i=0; i<p->iUserNum; ++i)
	{
		pRankNum[i] = i+1;
	}
	RandRank(pRankNum, p->iUserNum);
	int* pTemp = pRankNum;
	for (int i=0; i<p->iUserNum; ++i)
	{
		pUserInfo = m_UserManage.FindOnLineUser(*(p->pUserIDs+i));
		if (pUserInfo==NULL)
		{
			continue;
		}
		//设置初始排名和剩余人数
		pUserInfo->m_UserData.iRankNum = *(pTemp++);
		AutoAllotDeskAddUser(pUserInfo);
		MSG_GR_ContestChange oContest;
		oContest.dwUserID = pUserInfo->m_UserData.dwUserID;
		oContest.iRankNum = pUserInfo->m_UserData.iRankNum;
		oContest.iRemainPeople = p->iUserNum;
		HNLOG_G(m_InitData.uRoomID,"OnGetContestBeginResult %d %d %d",pUserInfo->m_UserData.dwUserID,oContest.iRankNum,oContest.iRemainPeople);
		m_TCPSocket.SendData(pUserInfo->m_uSocketIndex, &oContest, sizeof(MSG_GR_ContestChange), MDM_GR_ROOM, ASS_GR_INIT_CONTEST, 0, 0);
	}

	m_iRemainPeople = p->iUserNum;

	delete []p->pUserIDs;
	p->pUserIDs = NULL;

	//比赛开始后，通知该房间未报名用户退场
	m_TCPSocket.SendDataBatch(NULL,0,MDM_GR_ROOM,ASS_GR_CAHNGE_ROOM,0);
	KillTimer(IDT_QUEUE_GAME);
	SetTimer(IDT_QUEUE_GAME, m_iQueueTime*1000);

	if (m_InitData.iContestTime>0)
	{
		SetTimer(IDT_TIMING_CONTEST_END,m_InitData.iContestTime*60*1000);
	}
	return true;
}

bool CGameMainManage::OnGetContestGameOverResult(DataBaseResultLine *pResultData)
{
	DL_GR_O_ContestGameOver *p = (DL_GR_O_ContestGameOver*)pResultData;

	if (!p)
		return false;
	if (0 == pResultData->uHandleRusult)
	{
		//更新用户的奖励信息
		CGameUserInfo* pUser = m_UserManage.FindOnLineUser(p->oAward.dwUserID);
		if (!pUser)
			return false;
		if (m_InitData.pAwards && pUser->m_UserData.iRankNum<=m_InitData.iRankNum && 0 < pUser->m_UserData.iRankNum&&p->oAward.iAward==0)
		{
			p->oAward.iAward = m_InitData.pAwards[p->oAward.iRank-1];
			p->oAward.iAwardType = m_InitData.pAwardTypes[p->oAward.iRank-1];
		}
		else if(p->oAward.iAward==1)
		{
			p->oAward.iAward = -1;
		}
		//给所有用户发送奖励信息
		m_TCPSocket.SendData(pUser->m_uSocketIndex,&p->oAward,sizeof(MSG_GR_ContestAward),MDM_GR_ROOM,ASS_GR_CONTEST_GAMEOVER,0,0);
	}
	else if (1 == pResultData->uHandleRusult)
	{
		DataBaseLineHead _out;
		memset(&_out,0, sizeof(DataBaseLineHead));
		m_SQLDataManage.PushLine(&_out, sizeof(DataBaseLineHead), DTK_GR_CONTEST_ABANDON, 0, 0);
	}
	
	return true;
}

//获取比赛当前排名，成绩等
bool CGameMainManage::OnGetContestRankResult(DataBaseResultLine* pResultData)
{
	DL_GR_O_UserContestRank* pRank = (DL_GR_O_UserContestRank*)pResultData;

	CGameUserInfo* pUser = m_UserManage.FindOnLineUser(pRank->_data.dwUserID);
	if (!pUser)
	{
		return true;
	}
	//更新在线用户的比赛信息
	pUser->m_UserData.iRankNum = pRank->_data.iRankNum;
	pUser->m_UserData.i64ContestScore = pRank->_data.i64ContestScore;
	pUser->m_UserData.iContestCount = pRank->_data.iContestCount;
	m_iRemainPeople = pRank->_data.iRemainPeople;
	
	HNLOG_G(m_InitData.uRoomID,"OnGetContestRankResult %d %d %d",pRank->_data.dwUserID,pRank->_data.iRankNum,pRank->_data.iRemainPeople);
	m_TCPSocket.SendDataBatch(&pRank->_data,sizeof(MSG_GR_ContestChange),MDM_GR_ROOM,ASS_GR_USER_CONTEST,0);

	return true;
}

bool CGameMainManage::OnPropChangeResult(DataBaseResultLine *  pResultData)
{
	if (!(m_InitData.dwRoomRule & GRR_GAME_U3D))
	{
		return true;
	}
	TpropChange_O* _p = (TpropChange_O*)pResultData;

	if (_p != NULL)
	{
		CGameUserInfo * pUserInfo = m_UserManage.FindOnLineUser(_p->info.iUserId);
		if (pUserInfo != NULL)
		{
			TPropChange _q;
			_q = _p->info;

			if (pUserInfo->m_UserData.bDeskNO != 255 && pUserInfo->m_UserData.bDeskNO<m_uDeskCount)
			{
				CGameDesk *pDesk = *(m_pDesk+pUserInfo->m_UserData.bDeskNO);
				if (pDesk != NULL)
				{
					pDesk->PropInfo(_q);
				}
			}
		}
	}

	return true;
}

//好友管理
bool CGameMainManage::OnFriendManageDBOperateResult(DataBaseResultLine *pResultData)
{
	return true;
}
//好友聊天
bool CGameMainManage::OnBufferIMMessageDBOperateResult(DataBaseResultLine *pResultData)
{
	return true;
}
//呼叫管理员结果处理
bool CGameMainManage::OnCallGMResult(DataBaseResultLine * pResultData)
{

	switch(pResultData->uHandleRusult)
	{
	case DTK_GR_CALL_GM_SUCCESS:		//呼叫成功
		{
			m_TCPSocket.SendData(pResultData->uIndex,MDM_GR_MANAGE,ASS_GR_CALL_GM_SUCCESS,0,pResultData->dwHandleID);
			break;
		}
	case DTK_GR_CALL_GM_IN_ONE_MI:		//呼叫在１分钟内超过１次
		{
			m_TCPSocket.SendData(pResultData->uIndex,MDM_GR_MANAGE,ASS_GR_CALL_GM_FAIL,1,pResultData->dwHandleID);
			break;
		}
	case DTR_GR_ERROR_UNKNOW:
	case DTK_GR_CALL_GM_FAIL:
		{

			m_TCPSocket.SendData(pResultData->uIndex,MDM_GR_MANAGE,ASS_GR_CALL_GM_FAIL,0,pResultData->dwHandleID);
			break;
		}
	}
	return true;
}
//定时器消息
bool CGameMainManage::OnTimerMessage(UINT uTimerID)
{
	//游戏定时器
	if (uTimerID>=TIME_START_ID)
	{
		BYTE bDeskIndex=(BYTE)((uTimerID-TIME_START_ID)/TIME_SPACE);
		if (bDeskIndex<m_InitData.uDeskCount) 
		{
			return (*(m_pDesk+bDeskIndex))->OnTimer((uTimerID-TIME_START_ID)%TIME_SPACE);
		}
	}

	//定时赛定时器
	if(uTimerID >= IDT_TIMING_CONTEST_INFO && uTimerID <= IDT_TIMING_CONTEST_INFO_END)
	{
		KillTimer(uTimerID);
		OnBeforeMatchBeginInfo();
		return true;
	}

	//内部定时器
	switch (uTimerID)
	{
	case IDT_WRITE_SERVER_INFO:		//更新服务器信息
		{
			HNLOG_G(m_InitData.uRoomID,"OnTimerMessage：IDT_WRITE_SERVER_INFO uNameID =%d uRoomID = %d uOnLineUserCount = %d SQLQueueCount = %d", m_KernelData.uNameID, m_InitData.uRoomID, m_UserManage.GetGameRoomCount(), m_SQLDataManage.m_DataLine.GetDataCount());
			//处理数据
			DL_GR_I_UpdateServerInfo UpdateInfo;
			UpdateInfo.uRoomID=m_InitData.uRoomID;
			UpdateInfo.uNameID=m_KernelData.uNameID;
			UpdateInfo.uOnLineUserCount=m_UserManage.GetGameRoomCount();
			m_SQLDataManage.PushLine(&UpdateInfo.DataBaseHead,sizeof(UpdateInfo),DTK_GR_UPDATE_INFO,0,0);

			return true;
		}break;
	case IDT_CHECK_DATA_CONNECT:	//检测数据库连接
		{
			//KillTimer(IDT_CHECK_DATA_CONNECT);
			DataBaseLineHead DataBaseHead;
			m_SQLDataManage.PushLine(&DataBaseHead,sizeof(DataBaseHead),DTK_GR_CHECK_CONNECT,0,0);
			return true;
		}break;
	case IDT_UPDATE_ROOM_PEOPLE:		//更新服务器信息
		{
			//处理数据
			KillTimer(IDT_UPDATE_ROOM_PEOPLE);
			DL_GR_I_UpdateServerInfo UpdateInfo;
			UpdateInfo.uRoomID=m_InitData.uRoomID;
			UpdateInfo.uNameID=m_KernelData.uNameID;
			UpdateInfo.uOnLineUserCount=m_UserManage.GetGameRoomCount();
			m_SQLDataManage.PushLine(&UpdateInfo.DataBaseHead,sizeof(UpdateInfo),DTK_GR_UPDATE_INFO,0,0);
			return true;
		}break;
	case IDT_QUEUE_GAME: /// 排队机定时器（比赛场也会走这里）
		{
			KillTimer(IDT_QUEUE_GAME);
			NewQueueGame();
			m_iResetTime = 0;

			if (IsQueueGameRoom() && !(m_InitData.dwRoomRule & GRR_CONTEST) && !(m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
			{
				SetTimer(IDT_QUEUE_GAME, m_iQueueTime * 1000);
			}
			return true;
		}break;
	case IDT_LEASE_TIMEOVER:
		{
			KillTimer(IDT_LEASE_TIMEOVER);
			if (IsLeaseTimeOver())
			{
				for (int i = 0; i < m_UserManage.m_OnLineUserInfo.GetPermitCount(); ++i)
				{
					CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(i);
					if (pUserInfo!=NULL && !IsUserPlayGameByStation(pUserInfo)) 
					{
						AutoAllotDeskDeleteUser(pUserInfo);
						m_pGameUserInfoList.remove(pUserInfo);
						m_TCPSocket.SendData(pUserInfo->m_uSocketIndex, MDM_GR_USER_ACTION, ASS_GR_LEASE_TIMEOVER, 0, 0);

						//m_TCPSocket.CloseSocket(pUserInfo->m_uSocketIndex, 0);
						CleanUserInfo(pUserInfo);
						m_UserManage.FreeUser(pUserInfo, true);
					}
				}
			}
			SetTimer(IDT_LEASE_TIMEOVER, 10000);
			return true;
		}break;
	case IDT_CONNECT_OTHERSERVER_SOCKET:
		{
			return OnTimerMessageForUpdateToOtherServer(uTimerID);
		}break;
	case IDT_TIMING_CONTEST_BEGIN:
		{
			KillTimer(uTimerID);
			BeforeContestBegin();
			return true;
		}break;
	case IDT_TIMING_CONTEST_END:
		{
			m_ContestTimeover = true;	// 结束循环定时赛
			IsContestGameOver();	    // 调用比赛结束
		}break;
    case IDT_SET_DYNAMIC:
        {
            if(m_bSetDynamiced)
            {
                KillTimer(IDT_SET_DYNAMIC);
                DL_GR_Set_DynamicInfo data;

                memcpy(data.szDynamicInfo1, m_szDynamicPlayingMethod1, sizeof(data.szDynamicInfo1));

                data.iGameID = m_InitData.iGameID;
                m_SQLDataManage.PushLine(&data.DataBaseHead,sizeof(data),DTK_GR_SETDYNAMICINFO,0,0);
            }
            return true;
        }break;
    case IDT_SET_ROOMINFO:
        {
            if(m_bSetRoomDynamiced)
            {
                KillTimer(IDT_SET_ROOMINFO);

                DL_GR_I_SetGameRoomInfo roomdata;
                roomdata.iRoomID = m_InitData.uRoomID;
                memcpy(roomdata.bRoomConfig, m_szDynaminRoomInfo, sizeof(roomdata.bRoomConfig));
                m_SQLDataManage.PushLine(&roomdata.DataBaseHead, sizeof(DL_GR_I_SetGameRoomInfo), DTK_GR_SET_SPROOMINFO, 0, 0);
            }
            return true;
        }break;
	}
	return false;
}

// 向其他服务器更新信息
bool CGameMainManage::OnTimerMessageForUpdateToOtherServer(UINT uTimerID)
{
	if (uTimerID != IDT_CONNECT_OTHERSERVER_SOCKET)
		return false;

	if (nullptr != m_pMainServerSocket)
	{
		m_pMainServerSocket->CheckReConnect();
		m_pMainServerSocket->CheckKeepLive();
		
	}
}


///判断是否是排队机房间
///@return true:是排队机房间；false：不是排队机房间
bool CGameMainManage::IsQueueGameRoom()
{
	return m_InitData.dwRoomRule & GRR_QUEUE_GAME || m_InitData.dwRoomRule & GRR_CONTEST || m_InitData.dwRoomRule & GRR_TIMINGCONTEST;
}

//初始化游戏桌
bool CGameMainManage::InitGameDesk(UINT uDeskCount, UINT uDeskType)
{
	//建立游戏桌子
	UINT uDeskClassSize=0;
	m_pDeskArray=CreateDeskObject(uDeskCount,uDeskClassSize);
	if ((m_pDeskArray==NULL)||(uDeskClassSize==0))
		throw new CAFCException(TEXT("CGameMainManage::InitGameDesk 内存申请失败"),0x418);

	//申请内存
	m_uDeskCount=uDeskCount;
	m_pDesk=new CGameDesk * [m_uDeskCount];
	if (m_pDesk==NULL)
	{
		throw new CAFCException(TEXT("CGameMainManage::InitGameDesk 内存申请失败"),0x419);
	}

    // 排队机场，配置成人数达到时才开始分配桌子
    // 小于桌子的人数则使用桌子的人数，否则使用配置的人数
    CBcfFile fConf(CBcfFile::GetAppPath()+"Function.bcf");
    m_uQueuerMaxPeople = fConf.GetKeyVal("OnlineCoin","QueuerMaxPeople",0);

	///当排队队列中人数>=m_uQueuerPeople + 每桌游戏数时开始排队
	m_uQueuerPeople = m_uQueuerMaxPeople;

    if(m_uQueuerMaxPeople < m_KernelData.uDeskPeople) m_uQueuerMaxPeople = m_KernelData.uDeskPeople;
    srand((unsigned)time(NULL));

	//设置指针
	for (UINT i=0;i<m_uDeskCount;i++)
	{
		*(m_pDesk+i)=(CGameDesk *)((char *)m_pDeskArray+i*uDeskClassSize);
		(*(m_pDesk+i))->GetGRM_SET_KEY(GRM_SET_KEY);
		(*(m_pDesk+i))->GetGRM_SET_KEY_Win(GRM_SET_KEY_WIN);
		(*(m_pDesk+i))->GetGRM_SET_KEY_Los(GRM_SET_KEY_LOS);
		(*(m_pDesk+i))->Init(i,m_KernelData.uDeskPeople,this,uDeskType);//百家乐
	}

	return true;
}


/// 创建自动撮桌对象
IAutoAllotDesk *CGameMainManage::CreateAutoAllotDesk(int iDeskPeopleCount, int iDeskCount)
{
	return new CAutoAllotDesk(iDeskPeopleCount, iDeskCount, m_KernelData.uMinDeskPeople);
}

///排队机添加用户
int CGameMainManage::AutoAllotDeskAddUser(CGameUserInfo *pInfo)
{
	if (m_pIAutoAllotDesk)
	{
		m_pIAutoAllotDesk->Add(pInfo);	///< 加入排队的玩家
	}
	return 0;
}

///排队机删除用户
int CGameMainManage::AutoAllotDeskDeleteUser(CGameUserInfo *pInfo)
{
	if (m_pIAutoAllotDesk)
	{
		m_pIAutoAllotDesk->Delete(pInfo);
	}
	return 0;
}

int CGameMainManage::AutoAllotDeskDeleteAll()
{
	if (m_pIAutoAllotDesk)
	{
		m_pIAutoAllotDesk->DeleteAll();
	}
	return 0;
}

void CGameMainManage::NewQueueGame()
{
	if (NULL == m_pIAutoAllotDesk || 1 != m_InitData.iRoomState)
	{
		return;
	}
	int nCount = 0;
	int *pArrDesk = NULL;
	m_pIAutoAllotDesk->AllotDesk(nCount, pArrDesk, (m_InitData.dwRoomRule&GRR_CONTEST || m_InitData.dwRoomRule&GRR_TIMINGCONTEST));
	if (nCount <= 0 || NULL == pArrDesk)
	{
		IsContestGameOver();
		HNLOG_G(m_InitData.uRoomID,"ycl::AllotDesk Fail %d，%d",m_pIAutoAllotDesk->GetQueueUserCount(),m_InitData.uRoomID);
		return;
	}
	/// 目标桌号
	int nDeskNo=0;
	MSG_GR_S_UserQueueInfo* _pQueueBuffer = m_pUserQueueBuffer;
	memset(_pQueueBuffer,0,sizeof(MSG_GR_S_UserQueueInfo)*m_KernelData.uDeskPeople);
	CGameUserInfo *pUserInfo = NULL;
	int nMaxQueue = nCount*m_KernelData.uDeskPeople;
	CGameDesk* pGameDesk = NULL;
	for (int i=0; i<nCount; ++i)
	{
		memset(_pQueueBuffer,0,sizeof(MSG_GR_S_UserQueueInfo)*m_KernelData.uDeskPeople);
		/// 找空桌
		while(nDeskNo < m_InitData.uDeskCount)
		{
			pGameDesk=*(m_pDesk+nDeskNo);
			if (pGameDesk != NULL && pGameDesk->GetDeskPlayerNum() ==0) break;
			nDeskNo++;
		}
			/// 没找到就退出循环
		if (((UINT)nDeskNo>=m_InitData.uDeskCount)||(pGameDesk->GetDeskPlayerNum() != 0))
		{
			int nPos = i*m_KernelData.uDeskPeople;
			while (nPos<nMaxQueue)
			{
				pUserInfo = m_UserManage.FindOnLineUser(pArrDesk[nPos++]);
				m_pIAutoAllotDesk->Add(pUserInfo);
			}
			break;
		}
		int bDeskStation=0;
		int iusercount = 0;
		vector<CGameUserInfo*> pDeskUserArr;
		pDeskUserArr.clear();
		/// 每个玩家坐桌
		for (bDeskStation=0; bDeskStation<m_KernelData.uDeskPeople; ++bDeskStation)
		{
			pUserInfo = m_UserManage.FindOnLineUser(pArrDesk[i*m_KernelData.uDeskPeople + bDeskStation]);
			if (NULL == pUserInfo)
			{				
				continue;
			}
			if ((pUserInfo->m_UserData.bDeskNO < m_InitData.uDeskCount) || (pUserInfo->m_UserData.bDeskStation < m_KernelData.uDeskPeople))
			{
				// 玩家已经坐下了，不能坐第二次
				break;
			}
			else
			{
				pDeskUserArr.push_back(pUserInfo);
			}
			iusercount++;
		}

		if (1 != m_InitData.iRoomState)
		{
			HNLOG_G(m_InitData.uRoomID,"ycl::RoomState Error %d,%d",m_InitData.iRoomState,m_InitData.uRoomID);
			return;
		}
		/// 判断坐桌过程是否失败
		if (pDeskUserArr.size() <= m_KernelData.uDeskPeople && pDeskUserArr.size() >= m_KernelData.uMinDeskPeople)
		{
			for (bDeskStation=0; bDeskStation<pDeskUserArr.size(); ++bDeskStation)
			{
				pUserInfo = pDeskUserArr[bDeskStation];
				if (NULL != pUserInfo)
				{
					MSG_GR_S_UserSit  UserSitInfo;
					UserSitInfo.bDeskIndex = nDeskNo;
					UserSitInfo.bDeskStation = bDeskStation;
					pGameDesk->UserSitDesk(&UserSitInfo, pUserInfo);

					pUserInfo->m_UserData.bDeskNO		= nDeskNo;
					pUserInfo->m_UserData.bDeskStation	= bDeskStation;
					pUserInfo->m_UserData.bUserState = USER_LOOK_STATE;

					_pQueueBuffer[bDeskStation].iUserID = pUserInfo->m_UserData.dwUserID;
					_pQueueBuffer[bDeskStation].bDeskStation = bDeskStation;					
					pGameDesk->SetDeskUserInfo(pUserInfo, bDeskStation);
					
					if (!(m_InitData.dwRoomRule & GRR_CONTEST) && !(m_InitData.dwRoomRule & GRR_TIMINGCONTEST))
					{
						m_pIAutoAllotDesk->Delete(pUserInfo);
					}
				}
			}

			for (bDeskStation=0; bDeskStation<pDeskUserArr.size(); ++bDeskStation)
			{
				if (pGameDesk->m_pUserInfo[bDeskStation]->m_UserData.bLogonbyphone)
					pGameDesk->SendDeskUserInfo(pGameDesk->m_pUserInfo[bDeskStation]->GetSocketIndex());
			}
			//广播玩家座位等信息
			m_TCPSocket.SendDataBatch(_pQueueBuffer, sizeof(MSG_GR_S_UserQueueInfo)*m_KernelData.uDeskPeople,MDM_GR_USER_ACTION,ASS_GR_QUEUE_USER_SIT,nDeskNo);

			pGameDesk->SetTimer(IDT_CONTEST_SIT_TIMEOUT,5000);

		}
		else
		{
			if (pDeskUserArr.size() == 0)
				return;
			/// 将非空玩家放回撮桌模块
			for (bDeskStation=0; bDeskStation<pDeskUserArr.size(); ++bDeskStation)
			{
				pUserInfo = pDeskUserArr[bDeskStation];
				if (NULL != pUserInfo)
				{
					pGameDesk->m_bLeave = false;
					pGameDesk->UserLeftDesk(bDeskStation, pUserInfo);
					pGameDesk->m_bLeave = true;
					
					pUserInfo->m_UserData.bDeskNO = 255;
					pUserInfo->m_UserData.bDeskStation = 255;
					m_pIAutoAllotDesk->Add(pUserInfo);
				}
			}
		}
	}
	return;
}

//启动定时赛定时器
void CGameMainManage::OnStartTimingMatchTimer(COleDateTime MatchStartTime)
{
	m_MatchbeginTime = MatchStartTime;

	COleDateTime nowTime = COleDateTime::GetCurrentTime();
	CString cs;
	if(MatchStartTime > nowTime)
	{
		COleDateTimeSpan tmDiff = (MatchStartTime - nowTime);
		long lsec = (long)tmDiff.GetTotalSeconds();

		if(lsec <=100 && m_InitData.iContestType == 1)
		{
			AfxMessageBox(TEXT("定时赛，开赛时间距当前时间 间隔不超过 100秒，请调整时间，加大间隔后，并重启游戏服务器"));
		}
			
		//设置提醒定时器
		if (lsec > 9*60)
		{
			SetTimer(IDT_TIMING_CONTEST_INFO+2,(lsec-9*60)*1000);
		}
		if (lsec > 8*60)
		{
			SetTimer(IDT_TIMING_CONTEST_INFO+3,(lsec-8*60)*1000);
		}
		if (lsec > 7*60)
		{
			SetTimer(IDT_TIMING_CONTEST_INFO+4,(lsec-7*60)*1000);
		}
		if (lsec > 6*60)
		{
			SetTimer(IDT_TIMING_CONTEST_INFO+5,(lsec-6*60)*1000);
		}
		if (lsec > 5*60)
		{
			SetTimer(IDT_TIMING_CONTEST_INFO,(lsec-5*60)*1000);
		}
		if (lsec > 4*60)
		{
			SetTimer(IDT_TIMING_CONTEST_INFO+6,(lsec-4*60)*1000);
		}
		if (lsec > 3*60)
		{
			SetTimer(IDT_TIMING_CONTEST_INFO+7,(lsec-3*60)*1000);
		}
		if (lsec > 2*60)
		{
			SetTimer(IDT_TIMING_CONTEST_INFO+8,(lsec-2*60)*1000);
		}
		if (lsec>60)
		{
			SetTimer(IDT_TIMING_CONTEST_INFO+1,(lsec-60)*1000);
		}

		//定时赛开赛定时器
		SetTimer(IDT_TIMING_CONTEST_BEGIN,lsec*1000);
	}
	
}

//定时赛比赛开始前1分钟，通知消息
void CGameMainManage::OnBeforeMatchBeginInfo()
{
	MSG_MG_S_CONTEST_NOTICE _p;
	_p.iRoomID = m_InitData.uRoomID;
	_p.iCountID = m_InitData.iContestID;

	m_pMainServerSocket->SendData(&_p,sizeof(MSG_MG_S_CONTEST_NOTICE),MDM_MG_CONTEST,ASS_MG_CONTEST_NOTICE,0);
}

bool CGameMainManage::OnFindBuyDeskStationResult(DataBaseResultLine * pResultData)
{
	DL_GR_O_FindDeskBuyStationResult *pReceiveData= (DL_GR_O_FindDeskBuyStationResult*)pResultData;
	if(!pReceiveData)
	{
		return false;
	}
	CGameDesk *pGameDesk=NULL;
	
	pGameDesk=(*(m_pDesk+pReceiveData->iDeskID));
	if(!pGameDesk)
	{
		return false;
	}
	pGameDesk->m_bIsBuy=true;
	pGameDesk->m_iVipGameCount=pReceiveData->iBuyCount;
    pGameDesk->m_iBuyMinutesDeskBeginTime=pReceiveData->BeginTime;
    pGameDesk->m_iBuyMinutes=pReceiveData->iBuyMinutes;
	pGameDesk->m_iDeskMaster=pReceiveData->iUserID;
	memcpy(pGameDesk->m_szDeskPassWord,pReceiveData->szDeskPassWord,sizeof(pGameDesk->m_szDeskPassWord));
	memcpy(pGameDesk->m_szDeskConfig,pReceiveData->szDeskConfig,sizeof(pGameDesk->m_szDeskConfig));
	pGameDesk->m_iPayType=pReceiveData->iPayType;
	pGameDesk->m_iRunGameCount = pReceiveData->iNowCount;
	pGameDesk->m_bFinishCondition = pReceiveData->bFinishCondition;
	pGameDesk->m_bPlayerNum = pReceiveData->bPlayerNum;
	pGameDesk->m_bPositionLimit = pReceiveData->bPositionLimit;
	pGameDesk->m_iAANeedJewels = pReceiveData->iAANeedJewels;
	pGameDesk->m_bMasterState = pReceiveData->bMasterState;
	pGameDesk->M_bMidEnter = pReceiveData->bMidEnter;
	pGameDesk->m_bIsPlay = pReceiveData->bIsPlay;
	pGameDesk->m_iClubID = pReceiveData->iClubID;
	pGameDesk->m_bPositionCheck = pReceiveData->bPositionLimit && m_InitData.bPositionCheck;
	pGameDesk->m_bIPCheck = pReceiveData->bPositionLimit && m_InitData.bIPCheck;

	pGameDesk->SetTimer(IDT_CHECK_DESK,5*60*1000);
    if (pGameDesk->m_bPlayerNum > 0)
    {
        pGameDesk->m_bMaxPeople = pGameDesk->m_bPlayerNum;
    }
	else
	{
		pGameDesk->m_bMaxPeople = m_KernelData.uDeskPeople;
	}

	pGameDesk->CanGetDeskInfo();

	if (DESK_TYPE_BJL!=pGameDesk->m_uDeskType)
	{
		DL_GR_I_ReturnDesk input;
		input.iDeskID = pReceiveData->iDeskID;
		input.iRoomID = m_InitData.uRoomID;
		m_SQLDataManage.PushLine(&input.DataBaseHead, sizeof(input), DTK_GR_RETURNEDESK, 0, 0);
	}
}

bool CGameMainManage::OnGetDeskInfo(DataBaseResultLine * pResultData)
{
	DL_GR_O_GET_DESKINFO *pReceiveData= (DL_GR_O_GET_DESKINFO*)pResultData;
	if(!pReceiveData)
	{
		return false;
	}
	CGameDesk *pGameDesk=NULL;

	if (pReceiveData->iDeskID >= m_uDeskCount)	return true;

	pGameDesk=(*(m_pDesk+pReceiveData->iDeskID));
	if(!pGameDesk)
	{
		return false;
	}
	if (pReceiveData->iMasterID > 0)
	{
		//更新桌子信息
		pGameDesk->m_bIsBuy=true;
		pGameDesk->m_iVipGameCount=pReceiveData->iBuyCount;
        pGameDesk->m_iBuyMinutes=pReceiveData->iBuyMinutes;
        pGameDesk->m_iBuyMinutesDeskBeginTime=pReceiveData->BeginTime;
		pGameDesk->m_iDeskMaster=pReceiveData->iMasterID;
		memcpy(pGameDesk->m_szDeskPassWord,pReceiveData->szDeskPass,sizeof(pGameDesk->m_szDeskPassWord));
		memcpy(pGameDesk->m_szDeskConfig,pReceiveData->szDeskConfig,sizeof(pGameDesk->m_szDeskConfig));
		pGameDesk->m_iPayType=pReceiveData->iPayType;
		pGameDesk->m_iRunGameCount = pReceiveData->iNowCount;
		pGameDesk->m_bFinishCondition = pReceiveData->bFinishCondition;
		pGameDesk->m_bPlayerNum = pReceiveData->bPlayerNum;
		pGameDesk->m_bPositionLimit = pReceiveData->bPositionLimit;
		pGameDesk->m_iAANeedJewels = pReceiveData->iAANeedJewels;
		pGameDesk->m_bMasterState = pReceiveData->bMasterState;
		pGameDesk->M_bMidEnter = pReceiveData->bMidEnter;
		pGameDesk->m_bIsPlay = pReceiveData->bIsPlay;
		pGameDesk->m_iClubID = pReceiveData->iClubID;
		pGameDesk->m_bPositionCheck = pReceiveData->bPositionLimit && m_InitData.bPositionCheck;
		pGameDesk->m_bIPCheck = pReceiveData->bPositionLimit && m_InitData.bIPCheck;

        if (pGameDesk->m_bPlayerNum > 0)
        {
            pGameDesk->m_bMaxPeople = pGameDesk->m_bPlayerNum;
        }
		else
		{
			pGameDesk->m_bMaxPeople = m_KernelData.uDeskPeople;
		}

        WriteLog("更新内存桌子信息，DeskPass = %s， RoomID = %d, DeskID = %d", pGameDesk->m_szDeskPassWord, m_InitData.uRoomID, pReceiveData->iDeskID);
		pGameDesk->CanGetDeskInfo();
	}
	//返回MServer信息
	if (pReceiveData->iType == TYPE_BUYROOM_NOTICE)
	{
		MSG_MG_S_BUY_DESK_NOTICE  _outdata;
		memcpy_s(&_outdata,sizeof(_outdata),&pReceiveData->_BuyData,sizeof(MSG_MG_S_BUY_DESK_NOTICE));
		_outdata.bSuccess = (pReceiveData->iMasterID > 0?true:false);
		
		m_pMainServerSocket->SendData(&_outdata,sizeof(_outdata),MDM_MG_CREATEROOM,ASS_MG_BUYROOM_NOTICE,0);
	}
	else if (pReceiveData->iType == TYPE_RETURNDESK_NOTICE)
	{
		MSG_MG_S_RETURN_DESK_NOTICE  _outdata;
		memcpy_s(&_outdata,sizeof(_outdata),&pReceiveData->_ReturnData,sizeof(MSG_MG_S_RETURN_DESK_NOTICE));
		_outdata.bSuccess = (pReceiveData->iMasterID > 0?true:false);

		m_pMainServerSocket->SendData(&_outdata,sizeof(_outdata),MDM_MG_CREATEROOM,ASS_MG_RETURNDESK_NOTICE,0);
	}
	else if (pReceiveData->iType == TYPE_CLUB_BUYDESK)
	{
		MSG_MG_S_CLUB_BUY_DESK_NOTICE  _outdata;
		memcpy_s(&_outdata,sizeof(_outdata),&pReceiveData->_ClubBuyData,sizeof(MSG_MG_S_CLUB_BUY_DESK_NOTICE));
		_outdata.bSuccess = (pReceiveData->iMasterID > 0?true:false);

		m_pMainServerSocket->SendData(&_outdata,sizeof(_outdata),MDM_MG_CLUB,ASS_MG_CLUB_BUYDESK,0);
	}
	
	return true;
}


bool CGameMainManage::OnReturnDeskResult(DataBaseResultLine * pResultData)
{
	DL_GR_O_ReturnDeskResult *pReceiveData= (DL_GR_O_ReturnDeskResult*)pResultData;
	if(!pReceiveData)
	{
		return false;
	}
	CGameDesk *pGameDesk=NULL;

	if (pReceiveData->iDeskID >= m_uDeskCount)	return true;

	pGameDesk=(*(m_pDesk+pReceiveData->iDeskID));
	if(!pGameDesk)
	{
		return false;
	}

	for (int i=0;i<pGameDesk->m_bMaxPeople;i++)
	{
		MSG_GM_S_ReturnGameInfo _Info;

		_Info.iUserSitNum = i;
		_Info.iUserID = pReceiveData->iUserID[i];
		_Info.i64Score = pReceiveData->i64Score[i];
		_Info.LogoID = pReceiveData->LogoID[i];
		memcpy(_Info.nickName,pReceiveData->nickName[i],sizeof(_Info.nickName));
		memcpy(_Info.HeadUrl,pReceiveData->HeadUrl[i],sizeof(_Info.HeadUrl));

		pGameDesk->m_pReturnInfo.push_back(_Info);
		pGameDesk->m_GameUserInfo[i] = pReceiveData->iUserID[i];
		pGameDesk->m_iRunGameCount = pReceiveData->iRunCount;
	}

	DL_GR_I_GET_GAMEINFO _p;
	_p.iDeskID = pReceiveData->iDeskID;
	_p.iRoomID = m_InitData.uRoomID;
	m_SQLDataManage.PushLine(&_p.DataBaseHead, sizeof(DL_GR_I_GET_GAMEINFO), DTK_GR_GETRELEASEINFO, 0, 0);

	return true;
}


bool CGameMainManage::OnCreateGameSNResult(DataBaseResultLine * pResultData)
{
	DL_GR_O_CREATE_GAMESN *pReceiveData= (DL_GR_O_CREATE_GAMESN*)pResultData;
	if(!pReceiveData)
	{
		return false;
	}
	CGameDesk *pGameDesk=NULL;

	if (pReceiveData->iDeskID >= m_uDeskCount)	return true;

	pGameDesk=(*(m_pDesk+pReceiveData->iDeskID));
	if(!pGameDesk)
	{
		return false;
	}

	strcpy_s(pGameDesk->m_GameSN,pReceiveData->szGameSN);

	pGameDesk->SetGameSN();

	return true;
}

bool CGameMainManage::OnSetReturnGameInfo(DataBaseResultLine * pResultData)
{
	DL_GR_O_GET_GAMEINFO *pReceiveData= (DL_GR_O_GET_GAMEINFO*)pResultData;
	if(!pReceiveData)
	{
		return false;
	}

	CGameDesk *pGameDesk=NULL;

	if (pReceiveData->iDeskID >= m_uDeskCount)	return true;

	pGameDesk=(*(m_pDesk+pReceiveData->iDeskID));
	if(!pGameDesk)
	{
		return false;
	}

	pGameDesk->SetReturnGameInfo(pReceiveData->iUserID,pReceiveData->GameInfo,sizeof(pReceiveData->GameInfo),pReceiveData->iCount);
	
	return true;
}

bool CGameMainManage::OnContestRegistResult(DataBaseResultLine * pResultData)
{
	if (pResultData->uHandleRusult == 1)
	{
		AfxMessageBox(TEXT("RoomInfo Error!!"), MB_ICONERROR);
	}
	else if (pResultData->uHandleRusult == 2)
	{
		AfxMessageBox(TEXT("ContestType Error!!"), MB_ICONERROR);
	}
	return true;
}

bool CGameMainManage::OnContestActiveResult(DataBaseResultLine * pResultData)
{
	if (pResultData->uHandleRusult != 0)
	{
		DL_GR_O_ContestActive  *p = (DL_GR_O_ContestActive*)pResultData;

		m_pMainServerSocket->SendData(&p->_data,sizeof(p->_data),MDM_MG_CONTEST,ASS_MG_CONTEST_APPLY,pResultData->uHandleRusult);
	}
	else		//激活成功
	{
		DL_GR_O_ContestActive  *p = (DL_GR_O_ContestActive*)pResultData;
		OnStartTimingMatchTimer(p->_data.BeginTime);
	}
	
	return true;
}

bool CGameMainManage::OnUserAction_FastSit(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	if (uSize!=sizeof(MSG_GR_S_UserSit)) return false;
	MSG_GR_S_UserSit * pUserSitInfo=(MSG_GR_S_UserSit *)pData;

	if (IsQueueGameRoom())
	{//排队机场不接收快速坐下消息
		return true;
	}
	CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
	if (pUserInfo==NULL)
	{
		return false;
	}

	//设置用户离开原位置
	if (pUserInfo->m_UserData.bDeskNO!=255)
	{
		if (IsUserPlayGame(pUserInfo)==true)
		{
			if(m_InitData.uComType==TY_MATCH_GAME) return true;
			m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_SIT_ERROR,ERR_GR_CAN_NOT_LEFT,dwHandleID);
			return true;
		}
		UserLeftDesk(pUserInfo);
	}

	if (255 != pUserSitInfo->bDeskIndex && pUserSitInfo->bDeskIndex<m_uDeskCount)
	{
		CGameDesk* pGameDesk = *(m_pDesk + pUserSitInfo->bDeskIndex);
		if (pGameDesk == nullptr)
		{
			return false;
		}

		if ((m_InitData.dwRoomRule&GRR_GAME_BUY) && strcmp(pGameDesk->m_szDeskPassWord,pUserSitInfo->szPassword))
		{
			HNLOG_G(m_InitData.uRoomID,"桌子密码错误 UserID:%d PSW:%s DeskID:%d DeskPsw:%s",pUserInfo->m_UserData.dwUserID,pUserSitInfo->szPassword,pUserSitInfo->bDeskIndex,pGameDesk->m_szDeskPassWord);
			SendErrorUserSit(uIndex, ERR_GR_VIP_PASS_ERROR,dwHandleID);
			return true;
		}

		if (pGameDesk->GetDeskPlayerNum() >= pGameDesk->m_bMaxPeople)
		{
			SendErrorUserSit(uIndex, ERR_GR_DESK_FULL,dwHandleID);
		}
		else
		{
			MSG_GR_S_UserSit UserSit;
			UserSit.bDeskIndex = pUserSitInfo->bDeskIndex;
			int iRes;
			for (int j=0; j!=m_KernelData.uDeskPeople; ++j)
			{
				UserSit.bDeskStation = j;
				iRes=pGameDesk->CanSitDesk(&UserSit, pUserInfo);
				if (ERR_GR_SIT_SUCCESS == iRes)
				{
					iRes = pGameDesk->UserSitDesk(&UserSit,pUserInfo);
					if (ERR_GR_SIT_SUCCESS != iRes)
					{
						SendErrorUserSit(uIndex, iRes,dwHandleID);
						return true;
					}
					else
					{
						if(pGameDesk->m_iPayType==2 && pGameDesk->m_bIsBuy && pGameDesk->M_bMidEnter && pGameDesk->IsPlayGame(0))
						{
							DL_GR_I_MidCostJewel input;
							input.iUserID   = pUserInfo->m_UserData.dwUserID;
							input.iAAJewels = pGameDesk->m_iAANeedJewels;
							m_SQLDataManage.PushLine(&input.DataBaseHead, sizeof(input), DTK_GR_MIDCOSTFEE, 0, 0);
						}
						return true;
					}
				}
			}
			SendErrorUserSit(uIndex, iRes,dwHandleID);
			return true;
		}
	}
	else
	{
		if (m_InitData.dwRoomRule&GRR_GAME_BUY)
		{
			HNLOG_G(m_InitData.uRoomID,"桌子密码错误 创建房间桌子号255 UserID:%d PSW:%s DeskID:%d",pUserInfo->m_UserData.dwUserID,pUserSitInfo->szPassword,pUserSitInfo->bDeskIndex);
			SendErrorUserSit(uIndex, ERR_GR_VIP_PASS_ERROR,dwHandleID);
			return true;
		}
		//设置用户到新位置
		BYTE bErrorCode=ERR_GR_ERROR_UNKNOW;
		CGameDesk* pGameDesk = NULL;
		MSG_GR_S_UserSit UserSit,UserFinalSit;
		ZeroMemory(&UserFinalSit,sizeof(UserFinalSit));
		UserFinalSit.bDeskIndex = 255;
		bool bDeskFlag=false;
		//找桌子
		for (int i=0; i!=m_InitData.uDeskCount; ++i)
		{	
			pGameDesk = *(m_pDesk + i);
			if (pGameDesk == NULL)
			{
				continue;
			}

			if (pGameDesk->IsLock() || pGameDesk->IsVirtualLock())
			{
				continue;
			}

			//找座位
			UserSit.bDeskIndex = i;
			for (int j=0; j!=m_KernelData.uDeskPeople; ++j)
			{
				UserSit.bDeskStation = j;
                bErrorCode = pGameDesk->CanSitDesk(&UserSit, pUserInfo);
				if (ERR_GR_SIT_SUCCESS == bErrorCode)
				{
					if (UserFinalSit.bDeskIndex == 255)
					{
						UserFinalSit.bDeskIndex = i;
						UserFinalSit.bDeskStation = j;
						bDeskFlag=true;
						break;
					}
					else
					{
						CGameDesk* pGameDeskOld = *(m_pDesk + UserFinalSit.bDeskIndex);
						if (pGameDesk->GetDeskPlayerNum() > pGameDeskOld->GetDeskPlayerNum())
						{
							UserFinalSit.bDeskIndex = i;
							UserFinalSit.bDeskStation = j;
							bDeskFlag=true;
							break;
						}
					}
				}
			}
			if(bDeskFlag)
			{
				break;
			}
		}

		if (UserFinalSit.bDeskIndex == 255)
		{
			//没有找到
			SendErrorUserSit(uIndex, bErrorCode,dwHandleID);
			return true;
		}

		pGameDesk = *(m_pDesk+UserFinalSit.bDeskIndex);

		if (nullptr == pGameDesk)
		{
			//没有找到
			SendErrorUserSit(uIndex, ERR_GR_FAST_SIT,dwHandleID);
		}
		else
		{
			int iRes = pGameDesk->UserSitDesk(&UserFinalSit,pUserInfo);
			if (ERR_GR_SIT_SUCCESS != iRes)
			{
				//没有找到
				SendErrorUserSit(uIndex, iRes,dwHandleID);
			}
		}
	}
	return true;
}

bool CGameMainManage::OnUserAction_ChangeDesk(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	if (m_InitData.dwRoomRule & GRR_TIMINGCONTEST || m_InitData.dwRoomRule & GRR_CONTEST)		// 比赛场不处理
	{
		return true ;
	}

	CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
	if (pUserInfo==NULL)
	{
		return false;
	}

	CGameDesk* pGameDesk = NULL;
	BYTE bOldDesk = 255;
	if (pUserInfo->m_UserData.bDeskNO != 255 && pUserInfo->m_UserData.bDeskNO<m_uDeskCount) 
	{
		pGameDesk = *(m_pDesk + pUserInfo->m_UserData.bDeskNO);
		bOldDesk = pUserInfo->m_UserData.bDeskNO;
	}

	if (IsUserPlayGame(pUserInfo)==true)
	{
		if(m_InitData.uComType==TY_MATCH_GAME) return true;
		m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_SIT_ERROR,ERR_GR_CAN_NOT_LEFT,dwHandleID);
		return true;
	}

	if (pGameDesk)
	{
		UserLeftDesk(pUserInfo,false);
	}

	if (IsQueueGameRoom())		// 排队机加入排队重新排队
	{
		if (m_InitData.uComType==TY_MONEY_GAME && pUserInfo->m_UserData.i64Money < m_InitData.uLessPoint)//排队机钱不足
		{
			m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_JOIN_QUEUE,1,dwHandleID);
			return true;
		}
		m_pIAutoAllotDesk->Add(pUserInfo);
	}
	else						// 普通房间换桌
	{
		//设置用户到新位置
		BYTE bErrorCode=ERR_GR_ERROR_UNKNOW;
		CGameDesk* pGameDesk = NULL;
		MSG_GR_S_UserSit UserSit,UserFinalSit;
		ZeroMemory(&UserFinalSit,sizeof(UserFinalSit));
		UserFinalSit.bDeskIndex = 255;
		bool bDeskFlag=false;
		//找桌子
		for (int i=0; i!=m_InitData.uDeskCount; ++i)
		{	
			pGameDesk = *(m_pDesk + i);
			if (pGameDesk == NULL)
			{
				continue;
			}

			if (pGameDesk->IsLock() || pGameDesk->IsVirtualLock())
			{
				continue;
			}

			if (i == bOldDesk)
			{
				continue;
			}

			//找座位
			UserSit.bDeskIndex = i;
			for (int j=0; j!=m_KernelData.uDeskPeople; ++j)
			{
				UserSit.bDeskStation = j;
				if (ERR_GR_SIT_SUCCESS == pGameDesk->CanSitDesk(&UserSit, pUserInfo))
				{
					if (UserFinalSit.bDeskIndex == 255)
					{
						UserFinalSit.bDeskIndex = i;
						UserFinalSit.bDeskStation = j;
						bDeskFlag=true;
						break;
					}
					else
					{
						CGameDesk* pGameDeskOld = *(m_pDesk + UserFinalSit.bDeskIndex);
						if (pGameDesk->GetDeskPlayerNum() > pGameDeskOld->GetDeskPlayerNum())
						{
							UserFinalSit.bDeskIndex = i;
							UserFinalSit.bDeskStation = j;
							bDeskFlag=true;
							break;
						}
					}
				}
			}
			if(bDeskFlag)
			{
				break;
			}
		}

		if (UserFinalSit.bDeskIndex == 255)
		{
			//没有找到
			SendErrorUserSit(uIndex, ERR_GR_FAST_SIT,dwHandleID);
			return true;
		}

		pGameDesk = *(m_pDesk+UserFinalSit.bDeskIndex);

		if (nullptr == pGameDesk)
		{
			//没有找到
			SendErrorUserSit(uIndex, ERR_GR_FAST_SIT,dwHandleID);
		}
		else
		{
			int iRes = pGameDesk->UserSitDesk(&UserFinalSit,pUserInfo);
			if (ERR_GR_SIT_SUCCESS != iRes)
			{
				//没有找到
				SendErrorUserSit(uIndex, iRes,dwHandleID);
			}
		}
	}

	return true;
}

bool CGameMainManage::OnGetDeskUserInfo(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	if (uSize!=sizeof(MSG_GR_I_GetDeskUserInfo)) return false;
	MSG_GR_I_GetDeskUserInfo * pDeskUserInfo=(MSG_GR_I_GetDeskUserInfo *)pData;
	MSG_GR_O_GetDeskUserInfo _out;

	CGameUserInfo * pMyUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
	if (pMyUserInfo==NULL)
	{
		return false;
	}
	//if (pMyUserInfo->m_UserData.bDeskNO == 255)
	//{
	//	m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_GET_USERINFO,ERR_GR_USER_NOT_SITTING_MY,dwHandleID);
	//	return true;
	//}

	CGameUserInfo *pUserInfo = m_UserManage.FindOnLineUser(pDeskUserInfo->iUserID);
	if (pUserInfo==NULL)
	{
		m_TCPSocket.SendData(uIndex,MDM_GR_USER_ACTION,ASS_GR_GET_USERINFO,ERR_GR_USER_NOT_ONLINE,dwHandleID);
		return true;
	}

	memcpy_s(&_out.UserInfo,sizeof(UserInfoStruct),&pUserInfo->m_UserData,sizeof(UserInfoStruct));
	if (pUserInfo->m_UserData.bDeskNO == 255 || pUserInfo->m_UserData.bDeskNO>=m_uDeskCount)
	{
		m_TCPSocket.SendData(uIndex,&_out,sizeof(MSG_GR_O_GetDeskUserInfo),MDM_GR_USER_ACTION,ASS_GR_GET_USERINFO,ERR_GR_USER_NOT_SITTING,dwHandleID);
		return true;
	}
	else if (pUserInfo->m_UserData.bDeskNO != pMyUserInfo->m_UserData.bDeskNO)
	{
		m_TCPSocket.SendData(uIndex,&_out,sizeof(MSG_GR_O_GetDeskUserInfo),MDM_GR_USER_ACTION,ASS_GR_GET_USERINFO,ERR_GR_USER_IN_DIFF_DESK,dwHandleID);
		return true;
	}

	CGameDesk* pGameDesk = *(m_pDesk + pUserInfo->m_UserData.bDeskNO);
	if (!pGameDesk)
	{
		return false;
	}

	_out.SitInfo.bLock=pGameDesk->m_bLock;
	_out.SitInfo.bDeskIndex=pGameDesk->m_bDeskIndex;
	_out.SitInfo.bDeskStation=pUserInfo->m_UserData.bDeskStation;
	_out.SitInfo.bUserState=pUserInfo->m_UserData.bUserState;
	_out.SitInfo.dwUserID=pUserInfo->m_UserData.dwUserID;
	_out.SitInfo.bIsDeskOwner=(pGameDesk->m_dwOwnerUserID==pUserInfo->m_UserData.dwUserID);//是台主
	_out.SitInfo.bDeskMaster=(pGameDesk->m_bIsBuy&&pGameDesk->m_iDeskMaster==pUserInfo->m_UserData.dwUserID);//房主

	m_TCPSocket.SendData(uIndex,&_out,sizeof(MSG_GR_O_GetDeskUserInfo),MDM_GR_USER_ACTION,ASS_GR_GET_USERINFO,ERR_GR_GET_USERINFO_SUCCESS,dwHandleID);
	return true;


}


///提示发送坐下错误信息
void CGameMainManage::SendErrorUserSit(UINT uIndex, UINT bHandleCode, DWORD dwHandleID)
{
	CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
	if (pUserInfo)
	{
		//发送坐下错误消息
		MSG_GR_R_UserSit UserSit;
		//UserSit.bLock=0;
		UserSit.bDeskIndex=pUserInfo->m_UserData.bDeskNO;
		UserSit.bDeskStation=pUserInfo->m_UserData.bDeskStation;
		UserSit.bUserState=pUserInfo->m_UserData.bUserState;
		UserSit.dwUserID=pUserInfo->m_UserData.dwUserID;
		//UserSit.bIsDeskOwner=0;//是台主
		m_TCPSocket.SendData(uIndex, &UserSit,sizeof(UserSit),MDM_GR_USER_ACTION,ASS_GR_SIT_ERROR,bHandleCode,dwHandleID);
	}
}
//传达语音成功与否的情况
bool CGameMainManage::OnVoiceInfo(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	if(pNetHead->bMainID!=MDM_GR_VOIEC || pNetHead->bAssistantID!=ASS_GR_VOIEC)
		return false;

	if(uSize!=sizeof(VoiceInfo)) return false;
	VoiceInfo* pCmd=(VoiceInfo*)pData;

	m_TCPSocket.SendDataBatch(pCmd,sizeof(VoiceInfo),MDM_GR_VOIEC,ASS_GR_VOIEC,RES_GR_VOIEC_SUC);

	return true;
}
//用户房间处理
bool CGameMainManage::OnDeskDissmiss(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	if (pNetHead->bMainID != MDM_GR_DESKRUNOUT)
	{
		return false;
	}

	switch (pNetHead->bAssistantID)
	{
	case ASS_GR_DISSMISS://申请解散桌子
		{
			//效验用户
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if(!pUserInfo)
			{
				return false;
			}
			CGameDesk * pGameDesk=GetDeskInfo(pUserInfo->m_UserData.bDeskNO);
			if(!pGameDesk)
			{
				return false;
			}

			if (pGameDesk->m_bInDissmissing)
			{
				return true;
			}

			if(pGameDesk->IsPlayGame(0)|| pGameDesk->m_iRunGameCount>0 || pGameDesk->m_bIsPlay)
			{
				if (pGameDesk->GetOnlineDeskPlayerNum()>1)
				{
					pGameDesk->m_bInDissmissing = true;
					pGameDesk->m_iDissmissUserID=pUserInfo->m_UserData.dwUserID;
					pGameDesk->m_iDissmissAgreePeople=1;
					pGameDesk->m_VecAgreeUserID.push_back(pUserInfo->m_UserData.dwUserID);
					for(int i=0;i<pGameDesk->m_bMaxPeople;i++)
					{
						if(!pGameDesk->m_pUserInfo[i])
						{
							continue;
						}
						VipDeskDismissNotify desk_notify;
						desk_notify.iUserID=pUserInfo->m_UserData.dwUserID;
						desk_notify.iTimeCount = (TIME_DISMISS_TIMEOUT/1000);
						m_TCPSocket.SendData(pGameDesk->m_pUserInfo[i]->m_uSocketIndex,&desk_notify,sizeof(desk_notify),MDM_GR_DESKRUNOUT,ASS_GR_DISSMISS,0,0);
					}

					

					if ( (pGameDesk->GetDeskPlayerNum() - pGameDesk->GetOnlineDeskPlayerNum()) >0)		//断线默认同意
					{
						for(int i=0;i<pGameDesk->m_bMaxPeople;i++)
						{
							if(!pGameDesk->m_pUserInfo[i] || !pGameDesk->m_bCutGame[i])
							{
								continue;
							}
							//pGameDesk->m_iDissmissAgreePeople++;
							pGameDesk->m_VecAgreeUserID.push_back(pGameDesk->m_pUserInfo[i]->m_UserData.dwUserID);
							VipDeskDismissAgreeRes tAgreeData;
							tAgreeData.bAgree=true;
							tAgreeData.iUserID=pGameDesk->m_pUserInfo[i]->m_UserData.dwUserID;
							for(int i=0;i<pGameDesk->m_bMaxPeople;i++)
							{
								if(!pGameDesk->m_pUserInfo[i])
								{
									continue;
								}
								m_TCPSocket.SendData(pGameDesk->m_pUserInfo[i]->m_uSocketIndex,&tAgreeData,sizeof(tAgreeData),MDM_GR_DESKRUNOUT,ASS_GR_DISSMISS_AGREE,0,0);
							}
						}
					}

					//解散定时器
					pGameDesk->SetTimer(IDT_DISMISS_TIMEOUT,TIME_DISMISS_TIMEOUT);
					pGameDesk->m_DismissTime = COleDateTime::GetCurrentTime();
				}
				else
				{
					pGameDesk->GameFinish(0,GFF_DISSMISS_FINISH);
				}
			}
			else
			{
				if(pGameDesk->m_iDeskMaster==pUserInfo->m_UserData.dwUserID)
				{
					pGameDesk->GameFinish(0,GFF_DISSMISS_FINISH);
				}
				else//解散失败
				{
					VipDeskRunOut desk_runout;
					m_TCPSocket.SendData(pUserInfo->m_uSocketIndex,&desk_runout,sizeof(desk_runout),MDM_GR_DESKRUNOUT,ASS_GR_DESKRUNOUT,0,0);
				}
			}
			return true;
		}break;
	case ASS_GR_DISSMISS_AGREE:
		{
			VipDeskDismissAgree *pReceiveData=(VipDeskDismissAgree *)pData;
			if(!pReceiveData)
			{
				return false;	
			}
			//效验用户
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if(!pUserInfo)
			{
				return false;
			}
			CGameDesk * pGameDesk=GetDeskInfo(pUserInfo->m_UserData.bDeskNO);
			if(!pGameDesk)
			{
				return false;
			}
			if(pReceiveData->bAgree)
			{
				auto ite = find(pGameDesk->m_VecAgreeUserID.begin(),pGameDesk->m_VecAgreeUserID.end(),pUserInfo->m_UserData.dwUserID);
				if (ite != pGameDesk->m_VecAgreeUserID.end())
				{
					return true;
				}
				
				pGameDesk->m_iDissmissAgreePeople++;
				pGameDesk->m_VecAgreeUserID.push_back(pUserInfo->m_UserData.dwUserID);
				VipDeskDismissAgreeRes tAgreeData;
				tAgreeData.bAgree=pReceiveData->bAgree;
				tAgreeData.iUserID=pUserInfo->m_UserData.dwUserID;
				for(int i=0;i<pGameDesk->m_bMaxPeople;i++)
				{
					if(!pGameDesk->m_pUserInfo[i])
					{
						continue;
					}
					m_TCPSocket.SendData(pGameDesk->m_pUserInfo[i]->m_uSocketIndex,&tAgreeData,sizeof(tAgreeData),MDM_GR_DESKRUNOUT,ASS_GR_DISSMISS_AGREE,0,0);
				}
				if(pGameDesk->m_iDissmissAgreePeople>=pGameDesk->GetOnlineDeskPlayerNum())
				{
					pGameDesk->GameFinish(0,GFF_DISSMISS_FINISH);
				}
			}
			else
			{
				pGameDesk->m_bInDissmissing = false;
				pGameDesk->m_iDissmissUserID=0;
				pGameDesk->m_iDissmissAgreePeople=0;
				pGameDesk->m_VecAgreeUserID.clear();

				VipDeskRunOut desk_runout;
				for(int i=0;i<pGameDesk->m_bMaxPeople;i++)
				{
					if(!pGameDesk->m_pUserInfo[i])
					{
						continue;
					}
					m_TCPSocket.SendData(pGameDesk->m_pUserInfo[i]->m_uSocketIndex,&desk_runout,sizeof(desk_runout),MDM_GR_DESKRUNOUT,ASS_GR_DESKRUNOUT,0,0);
				}
				pGameDesk->KillTimer(IDT_DISMISS_TIMEOUT);
			}
			return true;
		}break;
	case ASS_GR_GETDESKMASTER:
		{
            WriteLog("ASS_GR_GETDESKMASTER...");
			VipDeskInfo *pReceiveData=(VipDeskInfo *)pData;
			if(!pReceiveData)
			{
                WriteLog("ReceiveData IsNULL");
				return false;	
			}
			CGameDesk * pGameDesk=GetDeskInfo(pReceiveData->byDeskIndex);
			if(!pGameDesk)
			{
                WriteLog("GetDeskInfo Failed. DeskIndex = %d", pReceiveData->byDeskIndex);
				return false;
			}

			VipDeskInfoResult outdata;
			outdata.byDeskIndex = pGameDesk->m_bDeskIndex;
			outdata.iMasterID = pGameDesk->m_iDeskMaster;
			outdata.iPlayCount = pGameDesk->m_iVipGameCount; 
			outdata.iNowCount = (pGameDesk->IsPlayGame(0) || (pGameDesk->m_bIsPlay && pGameDesk->m_iRunGameCount==0))?(pGameDesk->m_iRunGameCount+1):pGameDesk->m_iRunGameCount;	
			strcpy_s(outdata.szPassWord,pGameDesk->m_szDeskPassWord);
			strcpy_s(outdata.szGameName,m_szGameName);
			outdata.iType = pGameDesk->m_bFinishCondition;

            if(pGameDesk->m_bIsPlay)
            {
                COleDateTime nowTime = COleDateTime::GetCurrentTime();
                int iDay  = pGameDesk->m_iBuyMinutes / (60*24);
                int iHour = (pGameDesk->m_iBuyMinutes - iDay * 60 * 24) / 60;
                int iMinutes = (pGameDesk->m_iBuyMinutes - iDay * 60 * 24 - iHour * 60);
                COleDateTimeSpan timespan( iDay, iHour, iMinutes, 0 );
                COleDateTimeSpan tmDiff = (pGameDesk->m_iBuyMinutesDeskBeginTime + timespan - nowTime);
                outdata.iSeconds = (long)tmDiff.GetTotalSeconds();
                if(outdata.iSeconds < 0) outdata.iSeconds = 1;
                outdata.bTimeKeeper = pGameDesk->m_iRunGameCount > 0?1:0;
            }
            else
            {
                outdata.iSeconds = pGameDesk->m_iBuyMinutes * 60;
                outdata.bTimeKeeper = 0;
            }

            if(outdata.iType == 3 && outdata.bTimeKeeper == 1)
            {// 时效房间
                if(pGameDesk->GetDeskPlayerNum() == 1)
                {// 第一个请求的人
                    SetTimer(IDT_DESKRUNOUT_TIMER, outdata.iSeconds * 1000);
                }
            }

			int nRet = m_TCPSocket.SendData(uIndex,&outdata,sizeof(outdata) ,MDM_GR_DESKRUNOUT,ASS_GR_GETDESKMASTER,0,dwHandleID);
            WriteLog("Send Res To Client, nRet = %d, DeskPass = %s, sizeof(VipDeskInfoResult) = %d", nRet, outdata.szPassWord, sizeof(VipDeskInfoResult));

			//pGameDesk->SendReturnInfo(uIndex);
			pGameDesk->SendDissmissingData(uIndex);

			return true;
		}break;
	case ASS_GR_DISTANCE_AGREE:
		{
			VipDeskDismissAgree *pReceiveData=(VipDeskDismissAgree *)pData;
			if(!pReceiveData)
			{
				return false;	
			}
			//效验用户
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if(!pUserInfo)
			{
				return false;
			}
			CGameDesk * pGameDesk=GetDeskInfo(pUserInfo->m_UserData.bDeskNO);
			if(!pGameDesk)
			{
				return false;
			}
			if(pReceiveData->bAgree)
			{
				pGameDesk->m_iLocationAgreeNum++;
				VipDeskDismissAgreeRes tAgreeData;
				tAgreeData.bAgree=pReceiveData->bAgree;
				tAgreeData.iUserID=pUserInfo->m_UserData.dwUserID;
				for(int i=0;i<pGameDesk->m_bMaxPeople;i++)
				{
					if(!pGameDesk->m_pUserInfo[i])
					{
						continue;
					}
					m_TCPSocket.SendData(pGameDesk->m_pUserInfo[i]->m_uSocketIndex,&tAgreeData,sizeof(tAgreeData),MDM_GR_DESKRUNOUT,ASS_GR_DISTANCE_AGREE,0,0);
				}
				if(pGameDesk->m_iLocationAgreeNum>=pGameDesk->GetOnlineDeskPlayerNum())
				{
					pGameDesk->m_bInLocation = false;
					pGameDesk->m_iLocationAgreeNum=0;
					pGameDesk->m_bAllAgreeLocation = true;
					pGameDesk->onCheckGameBegin();
				}
			}
			else
			{
				pGameDesk->UserNotAgreePrevent(pUserInfo->m_UserData.dwUserID);
				if (pUserInfo->m_UserData.dwUserID != pGameDesk->m_iDeskMaster)
					UserLeftDesk(pUserInfo,true);
			}
			return true;
		}break;
	case ASS_GR_GETDISMISS_TIMECOUNT:
		{
			CGameUserInfo * pUserInfo=m_UserManage.GetOnLineUserInfo(uIndex);
			if(!pUserInfo)
			{
				return false;
			}
			CGameDesk * pGameDesk=GetDeskInfo(pUserInfo->m_UserData.bDeskNO);
			if(!pGameDesk)
			{
				return false;
			}

			if (pGameDesk->m_bInDissmissing && pGameDesk->m_iDissmissUserID!=0 && DESK_TYPE_BJL!=pGameDesk->m_uDeskType)
			{
				COleDateTime nowTime = COleDateTime::GetCurrentTime();
				COleDateTimeSpan tmDiff = (nowTime - pGameDesk->m_DismissTime);
				long lsec = (long)tmDiff.GetTotalSeconds();
				VipDeskDismissTimeCount p;
				p.iTimeCount = (TIME_DISMISS_TIMEOUT/1000 - lsec);

				m_TCPSocket.SendData(uIndex,&p,sizeof(p),MDM_GR_DESKRUNOUT,ASS_GR_GETDISMISS_TIMECOUNT,0,0);
			}
			else
			{
				m_TCPSocket.SendData(uIndex,MDM_GR_DESKRUNOUT,ASS_GR_GETDISMISS_TIMECOUNT,1,0);
			}
			
			return true;
		}break;
	}
	return false;	
}

bool CGameMainManage::OnCostFee(DataBaseResultLine * pResultData)
{
	DL_GR_O_BuyRoom_CostFee  *p = (DL_GR_O_BuyRoom_CostFee*)pResultData;

	CGameUserInfo * pUserInfo=m_UserManage.FindOnLineUser(p->_Data.iUserID);

	if (pUserInfo)
	{
		m_TCPSocket.SendData(pUserInfo->m_uSocketIndex,&p->_Data,sizeof(MSG_GR_S_BuyRoom_CostFee),MDM_GR_DESKRUNOUT,ASS_GR_JEWES_CHANGE,0,0);
	}

	return true;
}

bool CGameMainManage::OnGetRoomInfoResult(DataBaseResultLine * pResultData)
{
	DL_GR_O_Get_RoomInfo  *p = (DL_GR_O_Get_RoomInfo*)pResultData;

	strcpy_s(m_szMsgRoomLogon,p->szMsgRoomLogon);
	strcpy_s(m_szMsgGameLogon,p->szMsgGameLogon);
	strcpy_s(m_szGameName,p->szGameName);

	return true;
}

bool CGameMainManage::OnSetContestInfoResult(DataBaseResultLine * pResultData)
{
	DL_GR_O_Get_ContestInfo  *p = (DL_GR_O_Get_ContestInfo*)pResultData;

	if((m_InitData.dwRoomRule & GRR_TIMINGCONTEST) && 1==m_InitData.iContestType)
	{
		OnStartTimingMatchTimer(p->MatchStartTime);
	}

	return true;
}

bool CGameMainManage::OnDissmissDeskByMS(DataBaseResultLine *pSourceData)
{
    if(pSourceData == NULL) return false;
    DL_GR_O_DissmissDeskByMS *pDiss = (DL_GR_O_DissmissDeskByMS*)pSourceData;

    MSG_MG_S_DissmissDesk *p = &pDiss->data;
    if (p->iRoomID!=m_InitData.uRoomID)
    {
        return true;
    }

    CGameDesk * pGameDesk=GetDeskInfo(p->iDeskID);
    if(!pGameDesk)
    {
        return true;
    }

    if (pGameDesk->m_iDeskMaster != p->iUserID)			//不是房主
    {
        p->iResult = ERR_GP_NOT_DESK_MASTER;
        m_pMainServerSocket->SendData(p,sizeof(MSG_MG_S_DissmissDesk),MDM_MG_CREATEROOM,ASS_MG_DISSMISSDESK,0);
        return true;
    }

    if (strcmp(pGameDesk->m_szDeskPassWord,p->szDeskPass) != 0)		//桌子密码不对
    {
        p->iResult = ERR_GP_DESKPASS_ERROR;
        m_pMainServerSocket->SendData(p,sizeof(MSG_MG_S_DissmissDesk),MDM_MG_CREATEROOM,ASS_MG_DISSMISSDESK,0);
        return true;
    }

    if(pGameDesk->IsPlayGame(0)|| pGameDesk->m_iRunGameCount>0)		//桌子正在游戏中
    {
        p->iResult = ERR_GP_DESK_INGAME;
        m_pMainServerSocket->SendData(p,sizeof(MSG_MG_S_DissmissDesk),MDM_MG_CREATEROOM,ASS_MG_DISSMISSDESK,0);
        return true;
    }

    pGameDesk->GameFinish(0,GFF_DISSMISS_FINISH);
    //解散成功
    p->iResult = ERR_GP_DISSMISSDESK_SUCCESS;
    m_pMainServerSocket->SendData(p,sizeof(MSG_MG_S_DissmissDesk),MDM_MG_CREATEROOM,ASS_MG_DISSMISSDESK,0);

    return true;
}


//用户断线解散桌子
void CGameMainManage::OnUserNetCutDissmiss(CGameUserInfo * pUserInfo)
{
	
}
//得到桌子指针
CGameDesk *CGameMainManage::GetDeskInfo(BYTE byDeskNo)
{
	CGameDesk *pDesk=NULL;
	if(byDeskNo>=0 && byDeskNo<m_uDeskCount)
	{
		pDesk=*(m_pDesk+byDeskNo);
	}
	return pDesk;
}

bool CGameMainManage::OnClearDesk(BYTE byDeskIndex)
{
	CGameDesk * pDesk=GetDeskInfo(byDeskIndex);
	if(pDesk)
	{
		if(pDesk->m_bIsBuy && !pDesk->IsBuyDeskPlaying() && pDesk->m_iPayType == 0)
		{
			DL_GR_I_BuyRoom_CostFee _indata;
			strcpy_s(_indata.szDeskPassWord,pDesk->m_szDeskPassWord);
			_indata.iType = 1;
			m_SQLDataManage.PushLine(&_indata.DataBaseHead, sizeof(_indata), DTK_GR_BUYROOM_COSTFEE, 0, 0);
		}

		if (pDesk->m_bIsBuy && pDesk->m_iClubID != 0)		// 俱乐部房间通知俱乐部玩家
		{
			MSG_GP_O_Club_RoomChange  _outdata;
			_outdata.bCreate = false;
			_outdata.iClub = pDesk->m_iClubID;
			strcpy_s(_outdata._data.szDeskPass,pDesk->m_szDeskPassWord);
            _outdata._data.bAllowEnter = false;
            _outdata._data.bIsPlay = false;
			m_pMainServerSocket->SendData(&_outdata,sizeof(_outdata),MDM_MG_CLUB,ASS_MG_CLUB_CLEARDESK,0);
		}

		DL_GR_I_BuyDeskOut input_cleardesk;
		memcpy(input_cleardesk.szDeskPassWord,pDesk->m_szDeskPassWord,sizeof(input_cleardesk.szDeskPassWord));
		m_SQLDataManage.PushLine(&input_cleardesk.DataBaseHead, sizeof(input_cleardesk), DTK_GR_CLEARBUYDESK, 0, 0);

		pDesk->m_bIsBuy=false;
		pDesk->m_iVipGameCount=0;
        pDesk->m_iBuyMinutes = 0;
		pDesk->m_iRunGameCount=0;
		pDesk->m_iDeskMaster=0;
		pDesk->m_bInDissmissing = false;
		pDesk->m_iDissmissUserID=0;
		pDesk->m_iDissmissAgreePeople=0;
		pDesk->m_bPositionCheck = false;
		pDesk->m_bIPCheck = false;
		pDesk->m_bAllAgreeLocation = false;
		pDesk->m_bInLocation = false;
		pDesk->m_bIsPlay = false;
		pDesk->m_iClubID = 0;
		pDesk->m_VecAgreeUserID.clear();
		pDesk->m_pDistance.clear();
		memset(pDesk->m_szDeskPassWord,0,sizeof(pDesk->m_szDeskPassWord));
		memset(pDesk->m_szDeskConfig,0,sizeof(pDesk->m_szDeskConfig));
		memset(pDesk->m_GameUserInfo,0,sizeof(pDesk->m_GameUserInfo));
		VipDeskRunOut desk_runout;
		desk_runout.byDeskIndex=byDeskIndex;
		desk_runout.bSuccess=true;
		for(int i=0;i<pDesk->m_bMaxPeople;i++)
		{
			if(!pDesk->m_pUserInfo[i])
			{
				continue;
			}
			m_TCPSocket.SendData(pDesk->m_pUserInfo[i]->m_uSocketIndex,&desk_runout,sizeof(desk_runout),MDM_GR_DESKRUNOUT,ASS_GR_DESKRUNOUT,0,0);

			pDesk->MakeUserOffLine(i,false);
		}

		pDesk->m_pReturnInfo.clear();

		pDesk->KillTimer(IDT_CHECK_DESK);
		pDesk->KillTimer(IDT_DISMISS_TIMEOUT);
        pDesk->KillTimer(IDT_DESKRUNOUT_TIMER);

		return true;
	}
	return false;
}

bool CGameMainManage::OnReleaseDesk(BYTE byDeskIndex)
{
	CGameDesk * pDesk=GetDeskInfo(byDeskIndex);
	if(pDesk)
	{
		DL_GR_I_ReleaseDesk input_releasedesk;		
		input_releasedesk.iRunCount = pDesk->m_iRunGameCount;
		memcpy(input_releasedesk.szDeskPassWord,pDesk->m_szDeskPassWord,sizeof(input_releasedesk.szDeskPassWord));
		m_SQLDataManage.PushLine(&input_releasedesk.DataBaseHead, sizeof(input_releasedesk), DTK_GR_RESLEASEDESK, 0, 0);

		pDesk->ReleaseRoomBefore();

		pDesk->m_bIsBuy=false;
		pDesk->m_iVipGameCount=0;
        pDesk->m_iBuyMinutes = 0;
		pDesk->m_iRunGameCount=0;
		pDesk->m_iDeskMaster=0;
		pDesk->m_bInDissmissing = false;
		pDesk->m_iDissmissUserID=0;
		pDesk->m_iDissmissAgreePeople=0;
		pDesk->m_bPositionCheck = false;
		pDesk->m_bIPCheck = false;	
		pDesk->m_bAllAgreeLocation = false;
		pDesk->m_bInLocation = false;
		pDesk->m_bIsPlay = false;
		pDesk->m_iClubID = 0;
		pDesk->m_VecAgreeUserID.clear();
		pDesk->m_pDistance.clear();
		memset(pDesk->m_szDeskPassWord,0,sizeof(pDesk->m_szDeskPassWord));
		memset(pDesk->m_szDeskConfig,0,sizeof(pDesk->m_szDeskConfig));
		memset(pDesk->m_GameUserInfo,0,sizeof(pDesk->m_GameUserInfo));
		for(int i=0;i<pDesk->m_bMaxPeople;i++)
		{
			pDesk->MakeUserOffLine(i);
		}

		pDesk->m_pReturnInfo.clear();

		pDesk->KillTimer(IDT_CHECK_DESK);
		pDesk->KillTimer(IDT_DISMISS_TIMEOUT);
        pDesk->KillTimer(IDT_DESKRUNOUT_TIMER);
		return true;
	}
	return false;
}

bool CGameMainManage::OnHandleMSMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	if (m_pHandleMSMessage)
		return m_pHandleMSMessage->OnNetMessage(pNetHead,pNetData,uDataSize,pClientSocket);

	return true;
}

bool CGameMainManage::BeforeContestBegin()
{
	if ((m_InitData.dwRoomRule & GRR_CONTEST) && 1 != m_InitData.iRoomState)
	{
		//设置比赛开始标志
		m_InitData.iRoomState = 1;
		DL_GR_I_ContestBegin DL_ContestBegin;
		DL_ContestBegin.uNameID = m_InitData.iGameID;
		DL_ContestBegin.uRoomID = m_InitData.uRoomID;
		DL_ContestBegin.iContestID = m_InitData.iContestID;
		m_SQLDataManage.PushLine(&DL_ContestBegin.DataBaseHead, sizeof(DL_ContestBegin), DTK_GR_CONTEST_BEGIN, 0, 0);
	}
	else if  ((m_InitData.dwRoomRule & GRR_TIMINGCONTEST) && 1 != m_InitData.iRoomState)
	{
		DL_GR_I_TimingMatchUsers TimingMatchInfo;
		TimingMatchInfo.iContestID = m_InitData.iContestID;
		TimingMatchInfo.iGameID = m_InitData.iGameID;
		TimingMatchInfo.iRoomID = m_InitData.uRoomID;
		m_SQLDataManage.PushLine(&TimingMatchInfo.DataBaseHead,sizeof(DL_GR_I_TimingMatchUsers),DTK_GR_TIMINGMATCH_GETQUEQUEUSERS,0,0);
	}

	return true;
}


bool CGameMainManage::OnTimingMatchBeginFillQueue(DataBaseResultLine * pResultData)
{
	DL_GR_O_TimingMatchUsers *pUserList = (DL_GR_O_TimingMatchUsers*)pResultData;
	if(!pUserList)
	{
		return false;
	}
	int iPlayerCount = pUserList->iUserCount;
	int iReserveCount = 0;

	for (int i = 0; i < iPlayerCount; i++)
	{
		CGameUserInfo *puser = m_UserManage.FindOnLineUser(pUserList->iArrayUserid[i]);
		if(puser)
		{
			iReserveCount++;
		}
		else if (puser == nullptr) //清理未进入游戏玩家报名信息
		{
			DL_GR_I_ContestApply DL_ContestApply;

			DL_ContestApply.iUserID = pUserList->iArrayUserid[i];
			DL_ContestApply.iTypeID = 1;

			m_SQLDataManage.PushLine(&DL_ContestApply.DataBaseHead, sizeof(DL_ContestApply), DTK_GR_CONTEST_APPLY, pResultData->uIndex, 0);
		}
	}

	if (iReserveCount<m_InitData.iMinPeople || iReserveCount<m_KernelData.uMinDeskPeople)			//人数不足比赛结束，退回报名费
	{
		DataBaseLineHead _out;
		memset(&_out,0, sizeof(DataBaseLineHead));
		m_SQLDataManage.PushLine(&_out, sizeof(DataBaseLineHead), DTK_GR_CONTEST_ABANDON, 0, 0);
		return true;
	}

	//从数据库获取比赛序号，并通知比赛开始
	//设置比赛开始标志
	m_InitData.iRoomState = 1;
	DL_GR_I_ContestBegin DL_ContestBegin;
	DL_ContestBegin.uNameID = m_InitData.iGameID;
	DL_ContestBegin.uRoomID = m_InitData.uRoomID;
	DL_ContestBegin.iContestID = m_InitData.iContestID;
	m_SQLDataManage.PushLine(&DL_ContestBegin.DataBaseHead, sizeof(DL_ContestBegin), DTK_GR_CONTEST_BEGIN, pResultData->uIndex, 0);

	return true;
}

void CGameMainManage::OnContestRoomReset()
{
	KillTimer(IDT_TIMING_CONTEST_BEGIN);
	KillTimer(IDT_TIMING_CONTEST_END);

	for (int i=IDT_TIMING_CONTEST_INFO;i<=IDT_TIMING_CONTEST_INFO_END;i++)
	{
		KillTimer(i);
	}

	m_InitData.iRoomState = 0;
	m_pIAutoAllotDesk->DeleteAll();
}