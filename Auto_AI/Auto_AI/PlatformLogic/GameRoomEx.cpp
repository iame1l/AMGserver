#include "StdAfx.h"
#include "GameRoomEx.h"
#include "auto_aidlg.h"
#include "gameroommessage.h"
#include "Platglobaldata.h"
#include "Platconfig.h"
#include "PlatLog.h"
#include "Platlock.h"

BEGIN_MESSAGE_MAP(CGameRoomEx, CBaseRoom)
	ON_WM_TIMER()
	ON_MESSAGE(IDM_SET_GAME_TIME,SetGameTime)
	ON_MESSAGE(IDM_KILL_GAME_TIME,KillGameTime)
	ON_MESSAGE(IDM_CLOSE,OnCloseGame)//20090211
    ON_MESSAGE(IDM_STAND_UP,OnStandingDesk)
END_MESSAGE_MAP()

//构造函数
CGameRoomEx::CGameRoomEx(TCHAR * szGameName, RoomInfoStruct * pRoomInfo, MSG_GP_R_LogonResult * pLogonInfo, DynamicConfig cof) 
: CBaseRoom(IDD_GAME_ROOM)
{
    m_CurDynamicConfig = cof;
	m_IGameFrame = NULL; 	//游戏接口
    m_bInitContest = false;
	memset(&m_MServerLogonInfo,0,sizeof(m_MServerLogonInfo));

	m_pRoomInfo = pRoomInfo;
	////初始化共享数据
	memset(&m_GameInfo,0,sizeof(m_GameInfo));
	lstrcpy(m_GameInfo.szGameName,szGameName);
	m_GameInfo.pISocketSend=this;
	m_GameInfo.pIFindUser=&m_UserManage;
	m_GameInfo.bShowUserInfo=true;
	m_GameInfo.bEnableSound=true;
	m_GameInfo.bEnableWatch=false;
	m_GameInfo.uNameID=pRoomInfo->stComRoomInfo.uNameID;
	m_GameInfo.uRoomID=pRoomInfo->stComRoomInfo.uRoomID;
	//m_GameInfo.dwGameLSVer=pRoomInfo->dwGameLSVer;
	m_GameInfo.uComType=pRoomInfo->stComRoomInfo.uComType;
	m_GameInfo.uDeskPeople=pRoomInfo->stComRoomInfo.uDeskPeople;
	m_GameInfo.dwRoomRule = pRoomInfo->stComRoomInfo.dwRoomRule ; 

	CopyMemory(&m_MServerLogonInfo,(MSG_GP_R_LogonResult*)pLogonInfo,sizeof(m_MServerLogonInfo));

	

	return;
}

//析构函数
CGameRoomEx::~CGameRoomEx()
{
	if (m_IGameFrame != NULL)
	{
		delete m_IGameFrame;
		m_IGameFrame = NULL;
	}
}

//数据绑定函数
void CGameRoomEx::DoDataExchange(CDataExchange * pDX)
{
	__super::DoDataExchange(pDX);

}

//初始化函数
BOOL CGameRoomEx::OnInitDialog()
{
	__super::OnInitDialog();
	ConnectToGServer();
	return TRUE;
}
//登陆游戏房间
void  CGameRoomEx::ConnectToGServer()
{
    KillTimer(TIMER_CONNECT_TO_GSERVER);
    SetTimer(TIMER_CONNECT_TO_GSERVER, 1000, NULL);
	return ;
}
//网络连接消息
bool CGameRoomEx::OnSocketConnectEvent(UINT uErrorCode, CTCPClientSocket * pClientSocket)
{
	if (uErrorCode!=0)	//连接出现错误
	{
        GSERVER_LOG_ERROR("连接GServer失败 %s:%d - %d", m_pRoomInfo->stComRoomInfo.szServiceIP, m_pRoomInfo->stComRoomInfo.uServicePort, uErrorCode);
		::PostMessage(GetParent()->m_hWnd,IDM_CLOSE_GAME_ROOM,(WPARAM)m_pRoomInfo, m_MServerLogonInfo.dwUserID);
		return true;
	}
	return true;
}

//网络读取消息
bool CGameRoomEx::OnSocketReadEvent(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	switch (pNetHead->bMainID)
	{
	case MDM_CONNECT:			//连接消息,准备进入房间 
		{
			return OnConnectMessage(pNetHead,pNetData,uDataSize,pClientSocket);
		}
	case MDM_GR_LOGON:			//房间登陆
		{
			return OnLogonMessage(pNetHead,pNetData,uDataSize,pClientSocket);
		}
	case MDM_GR_USER_LIST:		//用户列表
		{
			return OnUserListMessage(pNetHead,pNetData,uDataSize,pClientSocket);
		}
	case MDM_GR_USER_ACTION:	//用户动作
		{
			return OnUserActionMessage(pNetHead,pNetData,uDataSize,pClientSocket);
		}
	case MDM_GR_ROOM:			//房间信息
		{
			return OnRoomMessage(pNetHead,pNetData,uDataSize,pClientSocket);
		}
    case MDM_GM_GAME_NOTIFY:
        break;
    case MDM_GM_GAME_FRAME:
        break;
    default:
        {
            CString strKey;
            strKey.Format("%d_%d", pNetHead->bMainID, pNetHead->bAssistantID);
            SYSTEMTIME sysTime;
            GetLocalTime(&sysTime);
            if(m_notHandleMsgs.find(strKey) != m_notHandleMsgs.end())
            {
                if(m_notHandleMsgs[strKey] == sysTime.wHour)
                {
                    break;;
                }
            }
            GSERVER_LOG_INFO("%s 未处理消息 %d:%d", m_MServerLogonInfo.szName, pNetHead->bMainID, pNetHead->bAssistantID);
            m_notHandleMsgs[strKey] = sysTime.wHour;
        }
        break;
	}

	////游戏信息
	if (m_IGameFrame!=NULL)
		if(!m_IGameFrame->OnSocketReadEvent(pNetHead,pNetData,uDataSize,pClientSocket))
        {
            {

                CString strKey;
                strKey.Format("%d_%d", pNetHead->bMainID, pNetHead->bAssistantID);
                SYSTEMTIME sysTime;
                GetLocalTime(&sysTime);
                if(m_notHandleMsgs.find(strKey) != m_notHandleMsgs.end())
                {
                    if(m_notHandleMsgs[strKey] == sysTime.wHour)
                    {
                        return false;
                    }
                }
                GSERVER_LOG_INFO("%s 未处理消息 %d:%d", m_MServerLogonInfo.szName, pNetHead->bMainID, pNetHead->bAssistantID);
                m_notHandleMsgs[strKey] = sysTime.wHour;
            }
            return false;
        }
	
	return true;
}

//网络关闭消息
bool CGameRoomEx::OnSocketCloseEvent()
{
	try
	{
		if(m_IGameFrame)
		{
			m_IGameFrame->AFCCloseFrame();
			//起身
		}
	}catch(...)
	{
		if (m_IGameFrame != NULL)
		{
			delete m_IGameFrame;
		}
		m_IGameFrame = NULL;
	}
    AutoPlatLock lock(&UIDATA->uiSection);
    auto it = UIDATA->usrInfos.find(m_MServerLogonInfo.szName);
    if(it != UIDATA->usrInfos.end())
    {
        if(it->second.bUserState == USER_PLAY_GAME && !PLATCONFIG->bFishGame)
        {
            it->second.bUserState = USER_CUT_GAME;
        }
        else
        {
            if(it->second.bUserState != USER_CUT_GAME)
            {
                it->second.GLoginStatus = 0;
                it->second.bUserState = USER_NO_STATE;
                it->second.bDeskNo = 255;
                it->second.bDeskStation = 255;
            }
        }
    }
    else
    {
        MSERVER_LOG_ERROR("%s 没有找到UI信息", m_MServerLogonInfo.szName);
    }
	::PostMessage(GetParent()->m_hWnd,IDM_CLOSE_GAME_ROOM,(WPARAM)m_pRoomInfo, m_MServerLogonInfo.dwUserID);
	return true;
}
//关闭房间
void CGameRoomEx:: CloseGameRoom()
{
	m_TCPSocket->CloseSocket(false);
	try
	{
		if(m_IGameFrame)
		{
			m_IGameFrame->AFCCloseFrame();
			//起身
		}
	}catch(...)
	{
		if (m_IGameFrame != NULL)
		{
			delete m_IGameFrame;
		}
		m_IGameFrame = NULL;
	}
    AutoPlatLock lock(&UIDATA->uiSection);
    auto it = UIDATA->usrInfos.find(m_MServerLogonInfo.szName);
    if(it != UIDATA->usrInfos.end())
    {
        if(it->second.bUserState == USER_PLAY_GAME && !PLATCONFIG->bFishGame)
        {
            it->second.bUserState = USER_CUT_GAME;
        }
        else
        {
            if(it->second.bUserState != USER_CUT_GAME)
            {
                it->second.GLoginStatus = 0;
                it->second.bUserState = USER_NO_STATE;
                it->second.bDeskNo = 255;
                it->second.bDeskStation = 255;
            }
        }
    }
    else
    {
        MSERVER_LOG_ERROR("%s 没有找到UI信息", m_MServerLogonInfo.szName);
    }
	::PostMessage(GetParent()->m_hWnd,IDM_CLOSE_GAME_ROOM,(WPARAM)m_pRoomInfo, m_MServerLogonInfo.dwUserID);
	return ;
}
//连接消息处理
bool CGameRoomEx::OnConnectMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	if (pNetHead->bAssistantID==ASS_NET_TEST)	//网络测试信息
	{
		pClientSocket->SendData(MDM_CONNECT,ASS_NET_TEST,0);
		return true;
	}
	else if ((pNetHead->bAssistantID==ASS_CONNECT_SUCCESS)||(pNetHead->bAssistantID==3))	//连接成功
	{
		MSG_S_ConnectSuccess * _p = (MSG_S_ConnectSuccess *)pNetData;
		if (_p != NULL)
		{
			pClientSocket->SetCheckCode(_p->i64CheckCode, SECRET_KEY);
		}

		//检测成功
		LoginGServer();
		return true;
	}
	return true;
}

//001,房间登陆处理,一进入房间  
bool CGameRoomEx::OnLogonMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	switch (pNetHead->bAssistantID)
	{
	case ASS_GR_LOGON_SUCCESS:		//登陆成功
		{
			//效验数据
			if (uDataSize!=sizeof(MSG_GR_R_LogonResult)) 
            {
                GSERVER_LOG_ERROR("MSG_GR_R_LogonResult 结构体不一致");
                return false;
            }

			MSG_GR_R_LogonResult * pLogonInfo=(MSG_GR_R_LogonResult *)pNetData;

			/*m_GameInfo.uLessPoint=pLogonInfo->uLessPoint;*/
			m_GameInfo.pMeUserInfo = m_UserManage.AddGameUser(&pLogonInfo->pUserInfoStruct,1,0);

            GSERVER_LOG_INFO("%s 登陆GServer成功", m_MServerLogonInfo.szName);
						
			if(m_GameInfo.pMeUserInfo == NULL)
			{
				GSERVER_LOG_ERROR("AddGameUser 失败 %s", m_MServerLogonInfo.szName);
				
				CloseGameRoom();
				return false;
			}

            AutoPlatLock lock(&UIDATA->uiSection);
            auto it = UIDATA->usrInfos.find(m_MServerLogonInfo.szName);
            if(it != UIDATA->usrInfos.end())
            {
                it->second.GLoginStatus = 2;
                it->second.bUserState = pLogonInfo->pUserInfoStruct.bUserState;
            }
            else
            {
                MSERVER_LOG_ERROR("%s 没有找到UI信息", m_MServerLogonInfo.szName);
            }
			return true;
		}
	case ASS_GR_LOGON_ERROR:		//登陆失败
		{
            GSERVER_LOG_ERROR("%s 登陆失败", m_MServerLogonInfo.szName);
            // 登陆失败关闭房间
            CloseGameRoom();

            AutoPlatLock lock(&UIDATA->uiSection);
            auto it = UIDATA->usrInfos.find(m_MServerLogonInfo.szName);
            if(it != UIDATA->usrInfos.end())
            {
                it->second.GLoginStatus = 3;
            }
            else
            {
                MSERVER_LOG_ERROR("%s 没有找到UI信息", m_MServerLogonInfo.szName);
            }
			break;
		}
	case ASS_GR_SEND_FINISH:		//登陆完成
		{	
            GSERVER_LOG_INFO("%s 登陆GServer完成", m_MServerLogonInfo.szName);
            if(m_GameInfo.pMeUserInfo == NULL)
            {
                GSERVER_LOG_ERROR("%s 用户信息为空", m_MServerLogonInfo.szName);
            }
            else if(m_GameInfo.pMeUserInfo->GameUserInfo.bDeskNO == 255)
            {// 非断线重连
                // 坐桌
                SitDesk();
            }
			break;
		}
    default:
        {

            CString strKey;
            strKey.Format("%d_%d", pNetHead->bMainID, pNetHead->bAssistantID);
            SYSTEMTIME sysTime;
            GetLocalTime(&sysTime);
            if(m_notHandleMsgs.find(strKey) != m_notHandleMsgs.end())
            {
                if(m_notHandleMsgs[strKey] == sysTime.wHour)
                {
                    return true;
                }
            }
            GSERVER_LOG_INFO("%s 未处理消息 %d-%d", m_MServerLogonInfo.szName, pNetHead->bMainID, pNetHead->bAssistantID);
            m_notHandleMsgs[strKey] = sysTime.wHour;
        }
        break;
	}
	return true;
}

//从桌子上站起来
LRESULT CGameRoomEx::OnStandingDesk(WPARAM,LPARAM)
{
    StandingDesk();
    return TRUE;
}

//起立
bool CGameRoomEx::StandingDesk()
{
    //下面发送站起的信息
    if (m_IGameFrame!=NULL)
    {
        CClientGameDlg *dlg = NULL;
        dlg = (CClientGameDlg*)m_IGameFrame;
        dlg->KillAllTimer();
    }
	m_TCPSocket->SendData(MDM_GR_USER_ACTION,ASS_GR_USER_UP,0);
	return true;
}

//坐下
bool CGameRoomEx::SitDesk()
{
	if((m_GameInfo.dwRoomRule &GRR_QUEUE_GAME) != 0)
	{// 排队机
		m_TCPSocket->SendData(MDM_GR_USER_ACTION, ASS_GR_JOIN_QUEUE,0);
		return  true ; 
	}
    if(IsContestGame(m_GameInfo.dwRoomRule))
    {// 比赛报名
        m_TCPSocket->SendData(MDM_GR_USER_ACTION, ASS_GR_CONTEST_APPLY, 0);
        return true;
    }

	BYTE bDeskIndex = 255;
	BYTE bDeskStation = 255;

	OnFindBestDesk(bDeskIndex,bDeskStation);

	if(bDeskIndex == 255||bDeskStation == 255)
	{
        GSERVER_LOG_ERROR("没有找到合适的桌子%d-%d", bDeskIndex, bDeskStation);
        CloseGameRoom();
		return true;
	}

    if(PLATCONFIG->bFishGame)
    {
        KillTimer(TIMER_FISHGAME_AIBANK);
        SetTimer(TIMER_FISHGAME_AIBANK, 5000, NULL);
        KillTimer(TIMER_FISHGAME_LEAVE);
        int iRand = (m_CurDynamicConfig.nFishGameTimeLeave / 2) - m_CurDynamicConfig.nFishGameTimeLeave / 4;
        if(iRand == 0 || iRand == 1) iRand = 2;
        SetTimer(TIMER_FISHGAME_LEAVE, 
            (m_CurDynamicConfig.nFishGameTimeLeave + (rand() % iRand))* 60 * 1000, NULL);
    }
    else
    {
        SetTimer(TIMER_CHECK_USERSTATE, (m_CurDynamicConfig.nKeepInDeskSeconds + (rand() % (m_CurDynamicConfig.nKeepInDeskSeconds / 2) - m_CurDynamicConfig.nKeepInDeskSeconds / 4)) * 1000, NULL);
    }
	MSG_GR_S_UserSit UserSit;
	UserSit.bDeskIndex = bDeskIndex;
	UserSit.bDeskStation = bDeskStation;
	//下面发送坐下的信息
	m_TCPSocket->SendData(&UserSit,sizeof(UserSit),MDM_GR_USER_ACTION,ASS_GR_USER_SIT,0);

    GSERVER_LOG_INFO("%s 请求坐下 %d-%d", m_MServerLogonInfo.szName, bDeskIndex, bDeskStation);
	return true;
}

// 机智查找
bool CGameRoomEx::OnFindBestDesk(BYTE &bDeskIndex, BYTE &bDeskStation)
{
    int nTruePlayerNumber = -1;
    int nAIPlayerNumber = -1;
    //从每一个桌子开始查找
    for(UINT desk = 0; desk < m_pRoomInfo->stComRoomInfo.uDeskCount; desk ++)
    {
        if(CountMachine(desk) >= m_CurDynamicConfig.bMachineDeskCount)
        {// 桌子上机器人已达限制
            continue;
        }
        int nRealNumber = CountTruePlayer(desk);
        int nAINumber = CountMachine(desk);
        if(!m_CurDynamicConfig.bMachineAndPlayer && nRealNumber > 0)
        {// 机器人不允许和真人同桌
            continue;
        }
        //坐位
        for(UINT station = 0; station < m_GameInfo.uDeskPeople; station++)
        {// 判断座位上是否有人,并且是否可以入座
            int i = 0;
            int nUsrNumber = m_UserManage.GetOnLineUserCount();
            for(; i < nUsrNumber; i++)
            {
                UserItemStruct * pFindUserItem =  m_UserManage.FindOnLineUserByIndex(i);
                if(pFindUserItem == NULL)
                    continue;
                if(pFindUserItem->GameUserInfo.bDeskNO == desk &&pFindUserItem->GameUserInfo.dwUserID < 0)
                {// 桌子被锁
                    break;
                }
                if(pFindUserItem->GameUserInfo.bDeskNO == desk &&pFindUserItem->GameUserInfo.bDeskStation == station)
                {
                    break;
                }
            }
            if(i < nUsrNumber)
            {// 座位不合适
                continue;
            }
            else
            {
                if(m_CurDynamicConfig.bMachineAndPlayer)
                {// 允许与真人同桌
                    if(nRealNumber > nTruePlayerNumber)
                    {// 优先真人最多的桌子
                        nTruePlayerNumber = nRealNumber;
                        nAIPlayerNumber = nAINumber;
                        bDeskIndex = desk;
                        bDeskStation = station;
                    }
                    else if (nRealNumber == nTruePlayerNumber)
                    {// 真人数相同
                        if(nAINumber > nAIPlayerNumber)
                        {
                            nTruePlayerNumber = nRealNumber;
                            nAIPlayerNumber = nAINumber;
                            bDeskIndex = desk;
                            bDeskStation = station;
                        }
                    }
                }
                else
                {
                    if(nAINumber > nAIPlayerNumber)
                    {// 选择机器人最多的桌子
                        nTruePlayerNumber = nRealNumber;
                        nAIPlayerNumber = nAINumber;
                        bDeskIndex = desk;
                        bDeskStation = station;
                    }
                }
            }
        }
    }
    return true;
}

//统计某桌机器人数量
BYTE CGameRoomEx::CountMachine(BYTE bDeskIndex)
{
	int iCount = 0;
	for(int i = 0 ; i < m_UserManage.GetOnLineUserCount(); i ++)
	{
		UserItemStruct * pFindUserItem =  m_UserManage.FindOnLineUserByIndex(i);
		if(pFindUserItem == NULL)
			continue;
		if(pFindUserItem->GameUserInfo.bDeskNO == bDeskIndex && pFindUserItem->GameUserInfo.isVirtual)
		{
			iCount ++ ;
		}
	}
	return iCount;
}

//统计某桌真人数量
BYTE CGameRoomEx::CountTruePlayer(BYTE bDeskIndex)
{
	int iCount = 0;
	for(int i = 0 ; i < m_UserManage.GetOnLineUserCount(); i ++)
	{
		UserItemStruct * pFindUserItem =  m_UserManage.FindOnLineUserByIndex(i);
		if(pFindUserItem == NULL)
			continue;
		if(pFindUserItem->GameUserInfo.bDeskNO == bDeskIndex && !pFindUserItem->GameUserInfo.isVirtual)
		{
			iCount++ ;
		}
	}
	return iCount;
}

//发送登陆信息
bool CGameRoomEx::LoginGServer()
{
    GSERVER_LOG_INFO("%s 登陆GServer", m_MServerLogonInfo.szName);
	//发送用户登录信息
	MSG_GR_S_RoomLogon RoomLogon;
	//RoomLogon.uGameVer = 0;//m_GameInfo.dwGameLSVer;
	//RoomLogon.uRoomVer = GAME_PLACE_MAX_VER;
	RoomLogon.uNameID = m_pRoomInfo->stComRoomInfo.uNameID;
	RoomLogon.dwUserID = m_MServerLogonInfo.dwUserID;
	lstrcpy(RoomLogon.szMD5Pass,m_MServerLogonInfo.szMD5Pass);
    RoomLogon.bLogonbyphone = false;
	//发送登陆
	m_TCPSocket->SendData(&RoomLogon,sizeof(RoomLogon),MDM_GR_LOGON,ASS_GR_LOGON_BY_ID,0);
    AutoPlatLock lock(&UIDATA->uiSection);
    auto it = UIDATA->usrInfos.find(m_MServerLogonInfo.szName);
    if(it != UIDATA->usrInfos.end())
    {
        it->second.GLoginStatus = 1;
    }
    else
    {
        MSERVER_LOG_ERROR("%s 没有找到UI信息", m_MServerLogonInfo.szName);
    }


	return true;
}

//用户列表处理
bool CGameRoomEx::OnUserListMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	switch (pNetHead->bAssistantID)
	{
	case ASS_GR_ONLINE_USER:
	case ASS_GR_NETCUT_USER:	//用户列表
		{
			UINT uUserCount = uDataSize/sizeof(UserInfoStruct);

            if(pNetHead->bAssistantID == ASS_GR_ONLINE_USER)
            {
                GSERVER_LOG_INFO("%s 收到在线用户列表%d个", m_MServerLogonInfo.szName, uUserCount);
            }
            else
            {
                GSERVER_LOG_INFO("%s 收到断线用户列表%d个", m_MServerLogonInfo.szName, uUserCount);
            }
            
			if (uUserCount>0L)
			{
				//定义变量
				UserItemStruct * pUserItem = NULL;
				UserInfoStruct * pUserInfo = (UserInfoStruct *)pNetData;

				for (UINT i = 0; i < uUserCount; i ++)
				{
					pUserItem = m_UserManage.AddGameUser(pUserInfo+i,1,0);
				}
			}

			return true;
		}
	case ASS_GR_DESK_STATION:	//桌子状态 
		{
			if (uDataSize!=sizeof(MSG_GR_DeskStation)) 
            {
                GSERVER_LOG_ERROR("%s MSG_GR_DeskStation结构体不一致", m_MServerLogonInfo.szName);
                return false;
            }
			MSG_GR_DeskStation * pDeskStation=(MSG_GR_DeskStation *)pNetData;
			for (UINT i=0;i<pNetHead->bHandleCode;i++)
			{
				if ((pDeskStation->bVirtualDesk[i/8]&(1<<(i%8)))!=0)    //change by yjj 090325
				{
					UserInfoStruct pUserInfo;      //玩家信息
					int c=0;

					for (int k=0; k<m_GameInfo.uDeskPeople;k++)
					{
						
						pUserInfo.bUserState=USER_PLAY_GAME;
						pUserInfo.bDeskNO=i;
						pUserInfo.bDeskStation=k;
						pUserInfo.dwUserID = -1;//虚拟用户

						m_UserManage.AddGameUser(&pUserInfo,1,0);
					}
				}
			}
			return true;
		}
    default:
        {
            CString strKey;
            strKey.Format("%d_%d", pNetHead->bMainID, pNetHead->bAssistantID);
            SYSTEMTIME sysTime;
            GetLocalTime(&sysTime);
            if(m_notHandleMsgs.find(strKey) != m_notHandleMsgs.end())
            {
                if(m_notHandleMsgs[strKey] == sysTime.wHour)
                {
                    return true;
                }
            }
            GSERVER_LOG_INFO("%s 未处理消息 %d:%d", m_MServerLogonInfo.szName, pNetHead->bMainID, pNetHead->bAssistantID);
            m_notHandleMsgs[strKey] = sysTime.wHour;
        }
        break;
	}
	return true;
}

//房间信息处理
bool CGameRoomEx::OnRoomMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	ASSERT(pNetHead->bMainID==MDM_GR_ROOM);

	switch (pNetHead->bAssistantID)
	{
	case ASS_GR_INIT_CONTEST:	
        {
            m_bInitContest = true;
            return true;
        }
	case ASS_GR_CAHNGE_ROOM:
		{// 服务器推送的退出房间请求
            if(!m_bInitContest)
			CloseGameRoom();//关闭房间				
			return true;
		}
	case ASS_GR_CONTEST_GAMEOVER:
	case ASS_GR_CONTEST_KICK:
		{// 被淘汰
            m_bInitContest = false;
			CloseGameRoom();//关闭房间
			return true;
		}
    case ASS_GR_USER_AGREE:
        {   
            if(sizeof(MSG_GR_R_UserAgree) != uDataSize)
            {
                GSERVER_LOG_ERROR("MSG_GR_R_UserAgree 结构体不一致");
                return false;
            }
            MSG_GR_R_UserAgree *pUsrAgree = (MSG_GR_R_UserAgree*)pNetData;
            AutoPlatLock lock(&UIDATA->uiSection);
            auto it = UIDATA->usrInfos.find(m_MServerLogonInfo.szName);
            if(it != UIDATA->usrInfos.end())
            {
                if(pUsrAgree->bDeskNO == it->second.bDeskNo && pUsrAgree->bDeskStation == it->second.bDeskStation) it->second.bUserState = USER_ARGEE;
            }
            else
            {
                MSERVER_LOG_ERROR("%s 没有找到UI信息", m_MServerLogonInfo.szName);
            }
        }
        return true;
	case ASS_GR_USER_CONTEST:
		{
			//处理数据
			if (uDataSize != sizeof(MSG_GR_ContestChange)) 
            {
                GSERVER_LOG_ERROR("MSG_GR_ContestChange 结构体不一致");
                return false;
            }
			MSG_GR_ContestChange* _p = (MSG_GR_ContestChange*)pNetData;

			UserItemStruct * pUserItem=m_UserManage.FindOnLineUser(_p->dwUserID);
			if (pUserItem != NULL)
			{
				//被淘汰玩家此时的排名比在线人数的值要大，比如用户被淘汰此时排名为7，则在线人数为6，此处修改一下显示
				_p->iRemainPeople = _p->iRemainPeople>_p->iRankNum ? _p->iRemainPeople : _p->iRankNum;
				pUserItem->GameUserInfo.iContestCount = _p->iContestCount;
				pUserItem->GameUserInfo.i64ContestScore = _p->i64ContestScore;
				pUserItem->GameUserInfo.iRankNum = _p->iRankNum;
			}
			return true;
		}
	case ASS_GR_GAME_BEGIN:		//游戏开始
		{
			KillTimer(TIMER_STAND_UP);
			//效验数据
			if (uDataSize!=0) 
            {
                GSERVER_LOG_ERROR("ASS_GR_GAME_BEGIN 结构体不一致");
                return false;
            }

			//处理数据
			BYTE bDeskIndex = pNetHead->bHandleCode;
			for(UINT i = 0 ; i < m_UserManage.GetOnLineUserCount(); i ++)
			{
				UserItemStruct * pFindUserItem =  m_UserManage.FindOnLineUserByIndex(i);
				if(pFindUserItem == NULL)
					continue;
				if(pFindUserItem->GameUserInfo.bDeskNO == bDeskIndex)
				{
					pFindUserItem->GameUserInfo.bUserState = USER_PLAY_GAME;					
				}
			}

            {
                AutoPlatLock lock(&UIDATA->uiSection);
                auto it = UIDATA->usrInfos.find(m_MServerLogonInfo.szName);
                if(it != UIDATA->usrInfos.end())
                {
                    if(pNetHead->bHandleCode == it->second.bDeskNo) it->second.bUserState = USER_PLAY_GAME;
                }
                else
                {
                    MSERVER_LOG_ERROR("%s 没有找到UI信息", m_MServerLogonInfo.szName);
                }
            }
			return true;
		}
	case ASS_GR_GAME_END:		//游戏结束
		{
            if (m_GameInfo.pMeUserInfo == NULL) return true;
			//效验数据
			if (uDataSize!=0) 
			{
                GSERVER_LOG_ERROR("ASS_GR_GAME_END 结构体不一致");
				return false;
			}
			if (IsContestGame(m_GameInfo.dwRoomRule))
			{
				//比赛场时，结束一盘后房间不需采取任何措施，直接等待服务端配桌
                {
                    AutoPlatLock lock(&UIDATA->uiSection);
                    auto it = UIDATA->usrInfos.find(m_MServerLogonInfo.szName);
                    if(it != UIDATA->usrInfos.end())
                    {
                        if(pNetHead->bHandleCode == it->second.bDeskNo)
                        {
                            it->second.bDeskNo = 255;
                            it->second.bDeskStation = 255;
                            it->second.bUserState = USER_NO_STATE;
                        }
                    }
                    else
                    {
                        MSERVER_LOG_ERROR("%s 没有找到UI信息", m_MServerLogonInfo.szName);
                    }
                }
				return true;
			}
            {
                AutoPlatLock lock(&UIDATA->uiSection);
                auto it = UIDATA->usrInfos.find(m_MServerLogonInfo.szName);
                if(it != UIDATA->usrInfos.end())
                {
                    if(pNetHead->bHandleCode == it->second.bDeskNo) it->second.bUserState = USER_SITTING;
                }
                else
                {
                    MSERVER_LOG_ERROR("%s 没有找到UI信息", m_MServerLogonInfo.szName);
                }
            }

            if(CheckNeedMoneyOp() || m_CurDynamicConfig != GLOBALDATA->CurDynamicConfig)
            {// 金币数达到存取要求,参数配置变化
                CloseGameRoom();
                return true;
            }

		    bool bHaveplayer=false; //是否有真实玩家
			//处理数据
			BYTE bDeskIndex = pNetHead->bHandleCode;
			for(int i = 0 ; i < m_UserManage.GetOnLineUserCount(); i ++)
			{
				UserItemStruct * pFindUserItem =  m_UserManage.FindOnLineUserByIndex(i);
				if(pFindUserItem == NULL)
				{
					continue;
				}
				if(pFindUserItem->GameUserInfo.bDeskNO == bDeskIndex)
				{
					pFindUserItem->GameUserInfo.bUserState = USER_DESK_AGREE;
					if (pFindUserItem->GameUserInfo.isVirtual == false)
					{
						bHaveplayer = true;
					}
				}
			}
            if (bHaveplayer && m_CurDynamicConfig.bMachineAndPlayer == false)//不与真人同桌
            {
                // 站起
              //  StandingDesk();
				SetTimer(TIMER_STAND_UP,(rand()%5 + 3)*1000,NULL);
            }	
            else
            {
			    //本桌游戏结束有机率离开
			    if(m_GameInfo.pMeUserInfo!=NULL&&bDeskIndex == m_GameInfo.pMeUserInfo->GameUserInfo.bDeskNO)
			    {
			        if(rand() % 100 < m_CurDynamicConfig.nGameEndLeaveDesk)
                    {
                        // 站起
                    //    StandingDesk();
						SetTimer(TIMER_STAND_UP,(rand()%5 + 3)*1000,NULL);
						
                    }
                    else
                    {
                        SetTimer(TIMER_CHECK_USERSTATE, (m_CurDynamicConfig.nKeepInDeskSeconds + (rand() % (m_CurDynamicConfig.nKeepInDeskSeconds / 2) - m_CurDynamicConfig.nKeepInDeskSeconds / 4)) * 1000, NULL);
                    }
			    }
            }			
			return true;
		}
	case ASS_GR_USER_POINT:		//用户金币积分
		{
			if(pNetHead->bHandleCode==10)//同步金币
			{
				//效验数据
				if (uDataSize!=sizeof(MSG_GR_S_RefalshMoney)) 
				{
                    GSERVER_LOG_ERROR("MSG_GR_S_RefalshMoney 结构体不一致");
					return false;
				}
				MSG_GR_S_RefalshMoney * pReflashMoney=(MSG_GR_S_RefalshMoney*)pNetData;

				//处理数据
				UserItemStruct * pUserItem=m_UserManage.FindOnLineUser(pReflashMoney->dwUserID);
				if (pUserItem==NULL) 
				{
                    GSERVER_LOG_ERROR("%s 未找到在线用户 %d", m_MServerLogonInfo.szName, pReflashMoney->dwUserID);
					return false;
				}

				pUserItem->GameUserInfo.i64Money=pReflashMoney->i64Money;

				if(m_GameInfo.pMeUserInfo!=NULL && pReflashMoney->dwUserID == m_GameInfo.pMeUserInfo->GameUserInfo.dwUserID)
				{
					__int64 Temp = (m_GameInfo.pMeUserInfo->GameUserInfo.i64Money+m_GameInfo.pMeUserInfo->GameUserInfo.i64Bank);

				}

				return true;
			}else if(pNetHead->bHandleCode==11)//同步经验值
			{
				//效验数据
				if (uDataSize!=sizeof(MSG_GR_S_RefalshMoney)) 
				{
                    GSERVER_LOG_ERROR("MSG_GR_S_RefalshMoney 结构体不一致");
					return false;
				}
				MSG_GR_S_RefalshMoney * pReflashMoney=(MSG_GR_S_RefalshMoney*)pNetData;

				//处理数据
				UserItemStruct * pUserItem=m_UserManage.FindOnLineUser(pReflashMoney->dwUserID);
				if (pUserItem==NULL) 
				{
                    GSERVER_LOG_ERROR("%s 未找到在线用户 %d", m_MServerLogonInfo.szName, pReflashMoney->dwUserID);
					return false;
				}
				//pUserItem->GameUserInfo.dwPoint=pReflashMoney->i64Money;

				if(m_GameInfo.pMeUserInfo!=NULL && pReflashMoney->dwUserID == m_GameInfo.pMeUserInfo->GameUserInfo.dwUserID)
				{
					__int64 Temp = (m_GameInfo.pMeUserInfo->GameUserInfo.i64Money+m_GameInfo.pMeUserInfo->GameUserInfo.i64Bank);

				}
				return true;
			}

			//效验数据
			if (uDataSize!=sizeof(MSG_GR_R_UserPoint)) return false;
			MSG_GR_R_UserPoint * pUserPoint=(MSG_GR_R_UserPoint *)pNetData;

			//处理数据
			UserItemStruct * pUserItem=m_UserManage.FindOnLineUser(pUserPoint->dwUserID);
			if (pUserItem!=NULL)
			{
				//设置数据

				//pUserItem->GameUserInfo.dwPoint+=pUserPoint->dwPoint;
				pUserItem->GameUserInfo.i64Money+=pUserPoint->dwMoney;
				pUserItem->GameUserInfo.uWinCount+=pUserPoint->bWinCount;
				pUserItem->GameUserInfo.uLostCount+=pUserPoint->bLostCount;
				pUserItem->GameUserInfo.uMidCount+=pUserPoint->bMidCount;
				pUserItem->GameUserInfo.uCutCount+=pUserPoint->bCutCount;

				if(m_GameInfo.pMeUserInfo!=NULL && pUserPoint->dwUserID == m_GameInfo.pMeUserInfo->GameUserInfo.dwUserID)
				{
					__int64 Temp = (m_GameInfo.pMeUserInfo->GameUserInfo.i64Money+m_GameInfo.pMeUserInfo->GameUserInfo.i64Bank);
				}
			}
            else
            {
                GSERVER_LOG_ERROR("%s 未找到在线用户 %d", m_MServerLogonInfo.szName, pUserPoint->dwUserID);
            }
			return true;
		}
	default:
        {
            CString strKey;
            strKey.Format("%d_%d", pNetHead->bMainID, pNetHead->bAssistantID);
            SYSTEMTIME sysTime;
            GetLocalTime(&sysTime);
            if(m_notHandleMsgs.find(strKey) != m_notHandleMsgs.end())
            {
                if(m_notHandleMsgs[strKey] == sysTime.wHour)
                {
                    return true;
                }
            }
            GSERVER_LOG_INFO("%s 未处理消息%d-%d", m_MServerLogonInfo.szName, pNetHead->bMainID, pNetHead->bAssistantID);
            m_notHandleMsgs[strKey] = sysTime.wHour;
        }
		return true;
	}
	return true;
}

//用户动作处理
bool CGameRoomEx::OnUserActionMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	if(NULL == m_GameInfo.pMeUserInfo )
	{
	    return true;
	}
	switch (pNetHead->bAssistantID)
	{
	case ASS_GR_CONTEST_APPLY:
		{
			if (uDataSize != sizeof(MSG_GR_O_ContestApply))	
            {
                GSERVER_LOG_ERROR("%s MSG_GR_ContestApply 结构体不一致", m_MServerLogonInfo.szName);
                return false;
            }

			MSG_GR_O_ContestApply *pMsgContestApply = (MSG_GR_O_ContestApply*)pNetData;


			//根据服务端反馈的报名状况做处理
			switch (pMsgContestApply->bResult)
			{
			case 1://报名成功
				{
                    GSERVER_LOG_INFO("%s 比赛报名成功 %d", m_MServerLogonInfo.szName, pMsgContestApply->bResult);
					break;
				}
			default:
				{
                    GSERVER_LOG_ERROR("%s 比赛报名错误 %d", m_MServerLogonInfo.szName, pMsgContestApply->bResult);
					CloseGameRoom();
					break;
				}
			}
			return true;
		}	
	case ASS_GR_USER_HIT_BEGIN:
		{
			//效验数据
			if(uDataSize!=sizeof(MSG_GM_WatchSet))
            {
                GSERVER_LOG_ERROR("%s MSG_GM_WatchSet 结构体不一致", m_MServerLogonInfo.szName);
                return false;
            }
			MSG_GM_WatchSet* pUserID = (MSG_GM_WatchSet*) pNetData;

			//处理数据
			UserItemStruct * pUserItem=m_UserManage.FindOnLineUser(pUserID->dwUserID);
            if(pUserItem != NULL)
			    pUserItem->GameUserInfo.bUserState = USER_DESK_AGREE;
            else
                GSERVER_LOG_ERROR("%s 未找到在线用户 %d", m_MServerLogonInfo.szName, pUserID->dwUserID);
			return true;
		}
	case ASS_GR_USER_UP:		//用户起来
		{
			//效验数据
			if (uDataSize!=sizeof(MSG_GR_R_UserSit))
            {
                GSERVER_LOG_ERROR("%s MSG_GR_R_UserSit 结构体不一致", m_MServerLogonInfo.szName);
                return false;
            }
			MSG_GR_R_UserSit * pUserUp=(MSG_GR_R_UserSit *)pNetData;

			//处理数据
			UserItemStruct * pUserItem=m_UserManage.FindOnLineUser(pUserUp->dwUserID);

			if (pUserItem!=NULL)
			{
				//桌号
				BYTE bTemp = pUserItem->GameUserInfo.bDeskNO;
				//设置数据
				pUserItem->GameUserInfo.bDeskNO = 255;
				pUserItem->GameUserInfo.bDeskStation = 255;
				pUserItem->GameUserInfo.bUserState = pUserUp->bUserState;

				if(pUserUp->dwUserID == m_MServerLogonInfo.dwUserID)//自己起来
				{
                    {
                        AutoPlatLock lock(&UIDATA->uiSection);
                        auto it = UIDATA->usrInfos.find(m_MServerLogonInfo.szName);
                        if(it != UIDATA->usrInfos.end())
                        {
                            it->second.bDeskNo = 255;
                            it->second.bDeskStation = 255;
                            it->second.bUserState = pUserUp->bUserState;
                        }
                        else
                        {
                            MSERVER_LOG_ERROR("%s 没有找到UI信息", m_MServerLogonInfo.szName);
                        }
                    }
                    GSERVER_LOG_INFO("%s 站起", m_MServerLogonInfo.szName);
                    if(!IsContestGame(m_GameInfo.dwRoomRule)) CloseGameRoom();
					return true;
				}
			}
			return true;
		}
	case ASS_GR_USER_SIT:		//用户坐下
		{
	//效验数据

			if (uDataSize!=sizeof(MSG_GR_R_UserSit)) 
			{
                GSERVER_LOG_ERROR("%s MSG_GR_R_UserSit 结构体不一致", m_MServerLogonInfo.szName);
				return false;
			}
			MSG_GR_R_UserSit * pUserSit=(MSG_GR_R_UserSit *)pNetData;

			//处理数据
			UserItemStruct * pUserItem=m_UserManage.FindOnLineUser(pUserSit->dwUserID);
			if (pUserItem!=NULL)
			{
				/////////////////////////////////////////
				//设置数据
				pUserItem->GameUserInfo.bDeskNO=pUserSit->bDeskIndex;

				pUserItem->GameUserInfo.bDeskStation=pUserSit->bDeskStation;

				pUserItem->GameUserInfo.bUserState=pUserSit->bUserState;
				if(pUserItem->GameUserInfo.dwUserID == m_MServerLogonInfo.dwUserID )//本人坐下
				{
                    {
                        AutoPlatLock lock(&UIDATA->uiSection);
                        auto it = UIDATA->usrInfos.find(m_MServerLogonInfo.szName);
                        if(it != UIDATA->usrInfos.end())
                        {
                            it->second.bDeskNo = pUserSit->bDeskIndex;
                            it->second.bDeskStation = pUserSit->bDeskStation;
                            it->second.bUserState = pUserSit->bUserState;
                            if(PLATCONFIG->bFishGame)
                            {// 捕鱼类游戏坐下即认为是在游戏中
                                it->second.bUserState = USER_PLAY_GAME;
                            }
                        }
                        else
                        {
                            MSERVER_LOG_ERROR("%s 没有找到UI信息", m_MServerLogonInfo.szName);
                        }
                    }
                    GSERVER_LOG_INFO("%s 坐下成功 %d:%d", m_MServerLogonInfo.szName, pUserSit->bDeskIndex, pUserSit->bDeskStation);
					StartGameClient();
					//控制客户端
					CM_UserState_Change ControlMessage;
					ControlMessage.bActionCode=ACT_USER_SIT;
					ControlMessage.bDeskStation = pUserItem->GameUserInfo.bDeskStation;
					ControlMessage.pUserItem = m_GameInfo.pMeUserInfo;
					ControlGameClient(CM_USER_STATE,&ControlMessage,sizeof(ControlMessage));

				}
				else
				{
					//控制客户端
					CM_UserState_Change ControlMessage;
					ControlMessage.bActionCode=ACT_USER_SIT;
					ControlMessage.bDeskStation = pUserItem->GameUserInfo.bDeskStation;
					ControlMessage.pUserItem = pUserItem;
					ControlGameClient(CM_USER_STATE,&ControlMessage,sizeof(ControlMessage));

					if (m_CurDynamicConfig.bMachineAndPlayer == false)//不与机真人同桌
					{
						if(pUserItem->GameUserInfo.bDeskNO == m_GameInfo.pMeUserInfo->GameUserInfo.bDeskNO)
						{//如果是真人,机器人起身
							if(!pUserItem->GameUserInfo.isVirtual)
                            {
                                // 站起
								StandingDesk();
                            }
						}
					}
                    else
                    {// 与真人同桌
                        if(PLATCONFIG->bFishGame && !pUserItem->GameUserInfo.isVirtual)
                        {// 捕鱼类游戏,真人坐下
                            if(pUserItem->GameUserInfo.bDeskNO != m_GameInfo.pMeUserInfo->GameUserInfo.bDeskNO)
                            {// 机器人与真人不在同一桌
                                if(CountTruePlayer(m_GameInfo.pMeUserInfo->GameUserInfo.bDeskNO) == 0)
                                {// 桌子上没有真人
                                    // 站起
                                    //SetTimer(TIMER_FISHGAME_WITHTRUEPAYER, (m_CurDynamicConfig.nKeepInDeskSeconds + \
									//	(rand() % (m_CurDynamicConfig.nKeepInDeskSeconds / 2) - m_CurDynamicConfig.nKeepInDeskSeconds / 4)) * 1000, NULL);
                                }
                            }
                        }
                    }
			
				}
			}
			return true;
		}
	case ASS_GR_QUEUE_USER_SIT:
		{
			/*if(pNetHead->uMessageSize - sizeof(NetMessageHead) != sizeof(MSG_GR_S_UserQueueInfo)*m_GameInfo.uDeskPeople)
            {
                GSERVER_LOG_ERROR("ASS_GR_QUEUE_USER_SIT结构体不一致");
                return false;
            }*/
			MSG_GR_S_UserQueueInfo *pQueueInfo = (MSG_GR_S_UserQueueInfo*)pNetData;

			bool bIsSelf = false;
			for (int i = 0; i < m_GameInfo.uDeskPeople; i++)
			{
				UserItemStruct * pUserItem = m_UserManage.FindOnLineUser((long)pQueueInfo->iUserID);
				if (pUserItem != NULL)
				{
					// PengJiLin, 2010-5-19，排队机不在桌子上显示玩家信息
					if(pUserItem->GameUserInfo.dwUserID !=m_GameInfo.pMeUserInfo->GameUserInfo.dwUserID && m_GameInfo.dwRoomRule & GRR_NOTCHEAT)
						strcpy(pUserItem->GameUserInfo.nickName, "玩家");

					pUserItem->GameUserInfo.bDeskNO= pNetHead->bHandleCode;
					pUserItem->GameUserInfo.bDeskStation = pQueueInfo->bDeskStation;
					pUserItem->GameUserInfo.bUserState = USER_SITTING;
					if ((pUserItem->GameUserInfo.dwUserID == m_GameInfo.pMeUserInfo->GameUserInfo.dwUserID))
					{
                        {
                            AutoPlatLock lock(&UIDATA->uiSection);
                            auto it = UIDATA->usrInfos.find(m_MServerLogonInfo.szName);
                            if(it != UIDATA->usrInfos.end())
                            {
                                it->second.bDeskNo = pNetHead->bHandleCode;
                                it->second.bDeskStation = pQueueInfo->bDeskStation;
                                it->second.bUserState = USER_SITTING;
                                if(PLATCONFIG->bFishGame)
                                {// 捕鱼类游戏坐下即认为是在游戏中
                                    it->second.bUserState = USER_PLAY_GAME;
                                }
                            }
                            else
                            {
                                MSERVER_LOG_ERROR("%s 没有找到UI信息", m_MServerLogonInfo.szName);
                            }
                        }
						bIsSelf = true;
						m_GameInfo.pMeUserInfo->GameUserInfo.bDeskNO = pNetHead->bHandleCode;
						m_GameInfo.pMeUserInfo->GameUserInfo.bDeskStation = pQueueInfo->bDeskStation;
					}
				}
                else
                {
                    GSERVER_LOG_ERROR("未找到在线用户%d", pQueueInfo->iUserID);
                }
                pQueueInfo++;

			}

			if (bIsSelf)
			{
                GSERVER_LOG_INFO("%s 收到排队机坐下", m_MServerLogonInfo.szName);

				StartGameClient();

				CM_UserState_Change ControlMessage;
				ControlMessage.bActionCode= ACT_USER_SIT;
				ControlMessage.bDeskStation= m_GameInfo.pMeUserInfo->GameUserInfo.bDeskStation;
				ControlMessage.pUserItem=m_GameInfo.pMeUserInfo;
				ControlGameClient(CM_USER_STATE,&ControlMessage,sizeof(ControlMessage));
			}
			return true ;
		}
	case ASS_GR_USER_COME:		//用户进入
		{
			//效验数据
			if (uDataSize!=sizeof(MSG_GR_R_UserCome)) return false;
			MSG_GR_R_UserCome * pUserCome=(MSG_GR_R_UserCome *)pNetData;

			//处理数据
			UserItemStruct * pUserItem=m_UserManage.AddGameUser(&pUserCome->pUserInfoStruct,1,0);

            if(pUserItem == NULL)
            {
                GSERVER_LOG_ERROR("%s AddGameUser失败 %d", m_MServerLogonInfo.szName, pUserCome->pUserInfoStruct.dwUserID);
            }
            else
            {
                if(pUserCome->pUserInfoStruct.dwUserID == m_MServerLogonInfo.dwUserID)
                {
                    AutoPlatLock lock(&UIDATA->uiSection);
                    auto it = UIDATA->usrInfos.find(m_MServerLogonInfo.szName);
                    if(it != UIDATA->usrInfos.end())
                    {
                        it->second.bDeskNo = pUserItem->GameUserInfo.bDeskNO;
                        it->second.bDeskStation = pUserItem->GameUserInfo.bDeskStation;
                        it->second.bUserState = pUserItem->GameUserInfo.bUserState;
                    }
                    else
                    {
                        MSERVER_LOG_ERROR("%s 没有找到UI信息", m_MServerLogonInfo.szName);
                    }
                }
            }

			return true;
		}
	case ASS_GR_USER_LEFT:		//用户离开
		{
			//效验数据
			if (uDataSize!=sizeof(MSG_GR_R_UserLeft)) return false;
			MSG_GR_R_UserLeft * pUserLeft=(MSG_GR_R_UserLeft *)pNetData;


			UserItemStruct * pUserItem=m_UserManage.FindOnLineUser(pUserLeft->dwUserID);
			if (pUserItem!=NULL)
			{
                if(pUserLeft->dwUserID == m_MServerLogonInfo.dwUserID)
                {
                    CloseGameRoom();
                }
				m_UserManage.DeleteGameUser(pUserItem);
			}
            else
            {
                GSERVER_LOG_ERROR("%s 未找到在线用户 %d", m_MServerLogonInfo.szName, pUserLeft->dwUserID);
            }

			return true;
		}
	case ASS_GR_USER_CUT:		//用户断线
		{
			//效验数据
			if (uDataSize!=sizeof(MSG_GR_R_UserCut)) return false;
			MSG_GR_R_UserCut * pUserCut=(MSG_GR_R_UserCut *)pNetData;
			//处理数据
			UserItemStruct * pUserItem=m_UserManage.FindOnLineUser(pUserCut->dwUserID);
			if (pUserItem!=NULL)
			{
				//设置数据
				pUserItem->GameUserInfo.bUserState=USER_CUT_GAME;
			}
            else
            {
                GSERVER_LOG_ERROR("%s 未找到在线用户 %d", m_MServerLogonInfo.szName, pUserCut->dwUserID);
            }

			return true;
		}
	case ASS_GR_SET_TABLE_BASEPOINT:	//设置用户桌子倍数
		{	
            GSERVER_LOG_INFO("ASS_GR_SET_TABLE_BASEPOINT");
			return true;
		}
	case ASS_GR_SIT_ERROR:		//坐下错误
		{
            GSERVER_LOG_ERROR("%s 坐下错误 %d", m_MServerLogonInfo.szName, pNetHead->bHandleCode);
			CloseGameRoom();
			return true;
		}
    default:
        {
            CString strKey;
            strKey.Format("%d_%d", pNetHead->bMainID, pNetHead->bAssistantID);
            SYSTEMTIME sysTime;
            GetLocalTime(&sysTime);
            if(m_notHandleMsgs.find(strKey) != m_notHandleMsgs.end())
            {
                if(m_notHandleMsgs[strKey] == sysTime.wHour)
                {
                    return true;
                }
            }
            GSERVER_LOG_INFO("%s 未处理消息 %d-%d", m_MServerLogonInfo.szName, pNetHead->bMainID, pNetHead->bAssistantID);
            m_notHandleMsgs[strKey] = sysTime.wHour;
        }
        break;
	}
	return true;
}

//启动游戏客户端
bool CGameRoomEx::StartGameClient()
{
	try
	{
		//获取对象 
		if (m_IGameFrame != NULL)
		{
			delete m_IGameFrame;
			m_IGameFrame = NULL;
		}
		//if (m_IGameFrame==NULL)
		{
			m_IGameFrame = new CClientGameDlg();

			m_IGameFrame->InitFrame(&m_GameInfo,this);
		}
		return true;
	}
	catch (...) {}

	m_IGameFrame->AFCCreateFrame();

	return false;
}

//发送控制消息 
UINT CGameRoomEx::ControlGameClient(UINT uControlCode, void * pControlData, UINT uDataSize)
{
	try
	{
		if (m_IGameFrame!=NULL)
			m_IGameFrame->OnControlMessage(uControlCode,pControlData,uDataSize);
	}
	catch (...) {}
	return 0;
}

//计时器消息
LRESULT CGameRoomEx::SetGameTime(WPARAM wparam,LPARAM lparam)
{
	SetTimer(wparam,lparam,NULL);
	return TRUE;
}

//关闭游戏,在游戏中的机器人发强退消息，
//add 20090211
//zht
LRESULT CGameRoomEx::OnCloseGame(WPARAM wparam,LPARAM lparam)
{
	//if (NULL != m_IGameFrame)
	//{
	//	CClientGameDlg* FrameGame = (CClientGameDlg*)m_IGameFrame;
	//	//sdp20140624 写死判断条件，不用宏，防止每次编译可能不通过
	//	if (FrameGame->GetStationParameter() >= 20 &&FrameGame->GetStationParameter() <28)
	//	{
	//		SendGameData(MDM_GM_GAME_FRAME,ASS_GM_FORCE_QUIT,0);
	//	}

	//	m_IGameFrame->AFCCloseFrame();
	//}
    CloseGameRoom();
	return TRUE;
}

//计时器消息
LRESULT CGameRoomEx::KillGameTime(WPARAM wparam,LPARAM lparam)
{
	if((UINT)wparam == 0)
	{
		for(int i = 100;i < 110;i++)
		{
			KillTimer(i);
		}
		return 0;

	}
	KillTimer((UINT)wparam);
	return TRUE;
}

//计时器
void CGameRoomEx::OnTimer(UINT_PTR uIDEvent)
{
	if(uIDEvent >= 100 && uIDEvent <= 200 )//100-110计时器为游戏逻辑使用
	{
		KillTimer(uIDEvent);
		if (m_IGameFrame!=NULL)
		{
			m_IGameFrame->OnGameTimer(0,uIDEvent,0);
		}
		return;
	}
    switch(uIDEvent)
    {
    case TIMER_CONNECT_TO_GSERVER:
        KillTimer(uIDEvent);
		//链接Gserver
        DoConnectToGServer();
        break;
    case TIMER_CHECK_USERSTATE:
        KillTimer(uIDEvent);
        if(m_GameInfo.pMeUserInfo == NULL) break;
        if(m_GameInfo.pMeUserInfo->GameUserInfo.bUserState != USER_PLAY_GAME && !PLATCONFIG->bFishGame)
        {
            CloseGameRoom();
        }
        break;
    case TIMER_FISHGAME_AIBANK:
        if(m_GameInfo.pMeUserInfo == NULL) break;
        if(CheckNeedMoneyOp() || m_CurDynamicConfig != GLOBALDATA->CurDynamicConfig)
        {
            CloseGameRoom();
        }
        break;
    case TIMER_FISHGAME_LEAVE:
        StandingDesk();
		 KillTimer(uIDEvent);
        break;
    case TIMER_FISHGAME_WITHTRUEPAYER:
        StandingDesk();
		 KillTimer(uIDEvent);
        break;
	case TIMER_STAND_UP:
		KillTimer(uIDEvent);
		StandingDesk();
        break;
    default:
        GSERVER_LOG_ERROR("未处理定时器 %d", uIDEvent);
        break;
    }
	return __super::OnTimer(uIDEvent);
}

bool CGameRoomEx::CheckNeedMoneyOp()
{
    if (m_GameInfo.pMeUserInfo == NULL) return false;
    if(IsContestGame(m_GameInfo.dwRoomRule))
    {// 定时塞和比赛场不操作银行
        return false;
    }
    if (m_CurDynamicConfig.nCheckOutMinMoney != 0 && m_GameInfo.pMeUserInfo->GameUserInfo.i64Money < m_CurDynamicConfig.nCheckOutMinMoney)
    {// 取钱
        __int64 i64RandMoney = ((m_CurDynamicConfig.nCheckInMaxMoney - m_CurDynamicConfig.nCheckOutMinMoney) / 2 +  m_CurDynamicConfig.nCheckOutMinMoney) - m_GameInfo.pMeUserInfo->GameUserInfo.i64Money;
        if(i64RandMoney > m_GameInfo.pMeUserInfo->GameUserInfo.i64Bank)
        {
            i64RandMoney = m_GameInfo.pMeUserInfo->GameUserInfo.i64Bank;
        }
        if(i64RandMoney > 0)			
            return true;			
    }
    else if (m_CurDynamicConfig.nCheckInMaxMoney != 0 && m_GameInfo.pMeUserInfo->GameUserInfo.i64Money >  m_CurDynamicConfig.nCheckInMaxMoney)
    {// 存钱
        __int64 i64RandMoney = m_GameInfo.pMeUserInfo->GameUserInfo.i64Money - ((m_CurDynamicConfig.nCheckInMaxMoney -m_CurDynamicConfig.nCheckOutMinMoney) / 2 +  m_CurDynamicConfig.nCheckOutMinMoney);
        if(i64RandMoney > m_GameInfo.pMeUserInfo->GameUserInfo.i64Bank)
        {
            i64RandMoney = m_GameInfo.pMeUserInfo->GameUserInfo.i64Bank;
        }
        if(i64RandMoney > 0)
            return true;	
    }
    return false;
}

void CGameRoomEx::DoConnectToGServer()
{
    if(m_TCPSocket != NULL)
    {
        delete m_TCPSocket;
    }
    m_TCPSocket = new CTCPClientSocket(this);
    if(m_TCPSocket->Connect(m_pRoomInfo->stComRoomInfo.szServiceIP,m_pRoomInfo->stComRoomInfo.uServicePort)==false)
    {
        GSERVER_LOG_ERROR("连接GServer失败%s:%d", m_pRoomInfo->stComRoomInfo.szServiceIP,m_pRoomInfo->stComRoomInfo.uServicePort);
        ConnectToGServer();
    }
}