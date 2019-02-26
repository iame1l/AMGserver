/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "StdAfx.h"
#include "MainServer.h"
#include "GameLogonManage.h"
#include "CommonUse.h"
#include "PropDataBaseMessage.h"
#include "GM_MessageDefine.h"

#include"WriteLog.h"

// 用来保护代码的宏定义
#define CHECK_ERROR_BEGIN \
	int iErrorCode = 0;  \
	do{


// 宏定义
#define IDT_CHECK_DATA_CONNECT						2					// 检测数据连接定时器
#define IDT_GET_ROOM_LIST_PEOCOUNT					3
#define IDT_GET_MAIL_LIST							4			

#define IDT_CHECKRECONNECT						    5
#define TIMEOUT_CHECKRECONNECT						1
#define IDT_CHECKKEEPLIVE						    6
#define TIMEOUT_CHECKKEEPLIVE						15

#define TIMEOUT_GET_ROOM_LIST_PEOCOUNT				6000L
#define TIMEOUT_GET_MAIL_LIST						1

#define IDT_GET_LOGON_PEOPLE_COUNT					7
#define TIMEOUT_GET_LOGON_PEOPLE					5

#define IDT_UPDATE_ROOMCONNECT						8
#define IDT_DISSMISS_TIMEOUTDESK                    9
#define TIMEOUT_UPDATE_ROOMCONNECT					60
/***************************************************************################################***********************************/

// DLL 文件模块、函数指针
static HINSTANCE g_hOpetvDll = NULL;
static SET_LICENCE_FUNC_PTR g_lpfn_SetLicence = NULL;
static INIT_FUNC_PTR   g_lpfn_Init = NULL;
static RELEASE_FUNC_PTR g_lpfn_Reslease = NULL;
static LOGIN_FUNC_PTR  g_lpfn_Login = NULL;

// 暂存用户名和密码
static char g_chUserName[65] = {0};
static char g_chMD5Pass[65] = {0};

CGameLogonManage* g_pGameLogonManage = NULL;

// 消息处理函数保护
CRITICAL_SECTION g_CriticalSection;


// SOCKET 数据读取
bool CGameLogonManage::OnSocketRead(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	/// GM消息处理过程
	if (MDM_GAMEMASTER_MESSAGE == pNetHead->bMainID)
	{
		/// 如果不是GM，则直接返回错误
		if ((!IsGameMaster(uIndex)) && (pNetHead->bAssistantID != ASS_GM_MESSAGE_GM_LOGON))
		{
			return false;
		}
		return m_pGmMessageBase->OnSocketMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
	}
	/// 获取在线GM列表
	else if (MDM_GP_PLAYER_2_GM == pNetHead->bMainID)
	{
		OnPlayer2Gm(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		return true;
	}
	//比赛
	else if (MDM_GP_CONTEST == pNetHead->bMainID)
	{
		if (m_pContestServer)
		{
			return m_pContestServer->OnNetMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}

		return true;
	}
	else if (MDM_GP_MESSAGE == pNetHead->bMainID)
	{
		m_TCPSocket.SendDataBatch(pData, uSize, pNetHead->bMainID, pNetHead->bAssistantID, 0);		
		return true;
	}
	else if (MDM_GP_REGISTER == pNetHead->bMainID)
	{
		if (m_pRegisterServer)
		{
			return m_pRegisterServer->OnNetMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}

		return true;
	}
	
	else if (pNetHead->bMainID==MDM_GP_LOGON)		//用户登陆，，网络头的主标识为MDM_GP_LOGON
	{
		if (m_pLogonServer)
		{
			return m_pLogonServer->OnNetMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}
	}
	else if(pNetHead->bMainID == MDM_GP_LOCK_ACCOUNT )	//锁定机器
	{
		if (m_pUpdateUserInfo)
		{
			return m_pUpdateUserInfo->OnNetMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}
		return true;
	}
	else if(pNetHead->bMainID == MDM_GP_BIND_MOBILE )	//用户绑定/取决绑定手机
	{
		if (m_pUpdateUserInfo)
		{
			return m_pUpdateUserInfo->OnNetMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}
		return true;
	}
	else if (pNetHead->bMainID == MDM_GP_SET_LOGO)
	{
		if (m_pUpdateUserInfo)
		{
			return m_pUpdateUserInfo->OnNetMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}
		return true;
	}
	else if (pNetHead->bMainID==MDM_GP_LIST)	//构件列表
	{
        if(pNetHead->bAssistantID == ASS_GP_GET_SPROOMINFO)
        {
            if(uSize != sizeof(MSG_GP_I_GetSPRoomInfo)) return false;
            MSG_GP_I_GetSPRoomInfo *pIndata = (MSG_GP_I_GetSPRoomInfo*)pData;
            DL_GP_I_GetSPRoomInfo data;
            data._data = *pIndata;
            m_SQLDataManage.PushLine(&data.DataBaseHead, sizeof(DL_GP_I_GetSPRoomInfo), DTK_GP_GET_SPROOMINFO, uIndex, dwHandleID);
            return true;
        }
		return onSendGameList(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
	}
	else  if(pNetHead->bMainID == MDM_GP_LASTLOINGGAME)
	{
		LPUSER pUser = NULL;
		for(int i=0; i<m_UserList.GetCount(); i++)
		{
			pUser = (LPUSER)m_UserList.GetAt(i);
			if(pUser == NULL)
				continue;
			if(pUser->iSocketIndex == uIndex )
			{
				DL_GP_I_LastLoginGame _p;
				_p.iUserID = pUser->UserID;
				m_SQLDataManage.PushLine(&_p.DataBaseHead,sizeof(DL_GP_I_LastLoginGame), DTK_GP_LASTLOGINGAME, uIndex, dwHandleID);//提交数据库
				return true;
			}
		}
		return true;
	}	
	else  if(pNetHead->bMainID==MDM_GP_NETSIGNAL)		//测试信号
    {
        m_TCPSocket.SendData(uIndex,pData,uSize,MDM_GP_NETSIGNAL,0,0,dwHandleID);
        return true;
    }	
	else if (pNetHead->bMainID == MDM_GP_USERINFO)		//修改用户信息
	{
		if (m_pUpdateUserInfo)
		{
			return m_pUpdateUserInfo->OnNetMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}
		return true;
	}	
	else if (pNetHead->bMainID == MDM_GP_BANK || pNetHead->bMainID == MDM_GP_MONEY)		//银行服务
	{
		if (m_pBankService)
		{
			m_pBankService->OnNetMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}
		
		return true;
	}	
	else if (pNetHead->bMainID == MDM_GP_SMS)		// 用户短信服务
	{
		if (m_pSmsServer)
		{
            DL_GP_I_MobileIsMatch DL_MobileIsMatch;
            ZeroMemory(&DL_MobileIsMatch, sizeof(DL_MobileIsMatch));
            memcpy(&DL_MobileIsMatch._data, pData, sizeof(DL_GP_I_MobileIsMatch));
            m_SQLDataManage.PushLine(&DL_MobileIsMatch.DataBaseHead, sizeof(DL_GP_I_MobileIsMatch), DTK_GP_MOBILE_IS_MATCH, uIndex, dwHandleID);
			//return m_pSmsServer->OnNetMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}
		return true;
	}
	else if (MDM_GP_PAIHANGBANG == pNetHead->bMainID) //查询排行榜
	{
		if (uSize != sizeof(MSG_GP_PaiHangBang_In)) return false;
		DL_GP_I_PaiHangBang DL_PaiHangBangIn;
		ZeroMemory(&DL_PaiHangBangIn, sizeof(DL_PaiHangBangIn));
		memcpy(&DL_PaiHangBangIn._param, pData, sizeof(MSG_GP_PaiHangBang_In));
		m_SQLDataManage.PushLine(&DL_PaiHangBangIn.DataBaseHead, sizeof(DL_PaiHangBangIn), DTK_GP_PAIHANGBANG, uIndex, dwHandleID);
		return true;
	}
	else if(pNetHead->bMainID==MDM_GR_USER_LOGO)
	{
		if (m_userCustomLogo)
		{
			m_userCustomLogo->OnNetMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		}		
		return true;
	}
	else if(pNetHead->bMainID==MDM_GP_IM)
	{
		if (pNetHead->bAssistantID == ASS_SET_FRIENDTYPE)
		{
			DL_GP_I_SetFriendType _p;
			LPUSER pUser = NULL;
			for(int i=0; i<m_UserList.GetCount(); i++)
			{
				pUser = (LPUSER)m_UserList.GetAt(i);
				if(pUser->iSocketIndex == uIndex )
				{
					_p.iUserID =pUser->UserID;
					_p.iType = *((int*)pData);
					pUser->iAddFriendType = _p.iType;

					SendData(uIndex, pData, uSize, MDM_GP_IM, ASS_SET_FRIENDTYPE, 0, 0);

					m_SQLDataManage.PushLine(&_p.DataBaseHead,sizeof(DL_GP_I_SetFriendType), DTK_GP_SETFRIENDTYPE, uIndex, dwHandleID);
				}
			}
		}
		else
		{
			if (m_imService)
			{
				m_imService->OnNetMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
			}
		}
		return true;
	}
	else if(pNetHead->bMainID==MDM_GP_PROP)
	{
		OnPropMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		return true;
	}
	else if (pNetHead->bMainID == MDM_GP_CHARMEXCHANGE)
	{
		if (pNetHead->bAssistantID == ASS_GETLIST)
		{
			DataBaseLineHead DataBaseHead;
			m_SQLDataManage.PushLine(&DataBaseHead,sizeof(DataBaseHead), DTK_GP_CHARMEXCHANGE_LIST, uIndex, dwHandleID);
		}
		else if (pNetHead->bAssistantID == ASS_EXCHANGE)
		{
			int _id = *((int*)pData);
			DL_GP_I_CharmExchange _p;
			LPUSER pUser = NULL;
			for(int i=0; i<m_UserList.GetCount(); i++)
			{
				pUser = (LPUSER)m_UserList.GetAt(i);
				if(pUser == NULL) return true;
				if(pUser->iSocketIndex == uIndex )
				{
					_p.date.iUserID = pUser->UserID;
					_p.date.iID = _id;
					m_SQLDataManage.PushLine(&_p.DataBaseHead,sizeof(DL_GP_I_CharmExchange), DTK_GP_CHARMEXCHANGE, uIndex, dwHandleID);
				}
			}
		}
		return true;
	}
	else if (pNetHead->bMainID == MDM_GP_SIGN || pNetHead->bMainID == MDM_GP_ONLINE_AWARD)
	{
		//处理数据
		LPUSER pUser = NULL;
		for(int i=0; i<m_UserList.GetCount(); i++)
		{
			pUser = (LPUSER)m_UserList.GetAt(i);
			if(pUser == NULL)
				continue;
			if(pUser->iSocketIndex == uIndex )
			{
				if (pNetHead->bMainID == MDM_GP_SIGN) //签到
				{
					DL_I_HALL_SIGN _in;
					_in.dwUserID = pUser->UserID;

					if (pNetHead->bAssistantID == ASS_GP_SIGN_CHECK) //查询签到
					{		
						m_SQLDataManage.PushLine(&_in.DataBaseHead,sizeof(_in), DTK_GP_SIGNIN_CHECK, uIndex, dwHandleID);
					}
					else if (pNetHead->bAssistantID == ASS_GP_SIGN_DO) //执行签到
					{
						m_SQLDataManage.PushLine(&_in.DataBaseHead,sizeof(_in), DTK_GP_SIGNIN_DO, uIndex, dwHandleID);
					}
				}
				else if (pNetHead->bMainID == MDM_GP_ONLINE_AWARD) //手游在线奖励
				{
					DL_I_HALL_ONLINE_AWARD inData;
					inData.dwUserID = pUser->UserID;

					if (pNetHead->bAssistantID == ASS_GP_ONLINE_AWARD_CHECK)
					{
						m_SQLDataManage.PushLine(&inData.DataBaseLineHead, sizeof(inData), DTK_GP_ONLINE_AWARD_CHECK, uIndex, dwHandleID);
					}
					else if (pNetHead->bAssistantID == ASS_GP_ONLINE_AWARD_DO)
					{
						m_SQLDataManage.PushLine(&inData.DataBaseLineHead, sizeof(inData), DTK_GP_ONLINE_AWARD_DO, uIndex, dwHandleID);
					}
				}				
			}
		}		
		return true;
	}
	else if(MDM_GP_DESK_VIP==pNetHead->bMainID)//VIP桌子
	{
		m_pCreateRoom->OnNetMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		return true;
	}
	else if(MDM_GP_CLUB==pNetHead->bMainID)//俱乐部
	{
		m_pHNClub->OnNetMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		return true;
	}
	else if (MDM_GP_MAIL==pNetHead->bMainID)
	{
		m_pHNMail->OnNetMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);
		return true;
	}
	else if(MDM_GP_GET_CONFIG==pNetHead->bMainID)//手游获取大厅功能配置
	{
		if (pNetHead->bAssistantID == ASS_GP_GET_CONFIG)
		{
			DataBaseLineHead _data;
			memset(&_data,0,sizeof(_data));
			m_SQLDataManage.PushLine(&_data, sizeof(_data), DTK_GP_GET_CONFIG, uIndex, dwHandleID);
		}
		return true;
	}
	else if(MDM_GP_MONEY_CHANGE==pNetHead->bMainID)//货币兑换
	{
		if (pNetHead->bAssistantID == ASS_GP_CHANGE_CONFIG)
		{
			DataBaseLineHead _data;
			memset(&_data,0,sizeof(_data));
			m_SQLDataManage.PushLine(&_data, sizeof(_data), DTK_GP_CHANGE_CONFIG, uIndex, dwHandleID);
		}
		else if (pNetHead->bAssistantID == ASS_GP_COIN2JEWEL)
		{
			MSG_GP_I_ChangeRequest *_in = (MSG_GP_I_ChangeRequest*)pData;

			DL_I_HALL_ChangeRequest _data;
			memcpy_s(&_data._data,sizeof(MSG_GP_I_ChangeRequest),_in,sizeof(MSG_GP_I_ChangeRequest));
			_data.itype = 0;

			m_SQLDataManage.PushLine(&_data.DataBaseLineHead, sizeof(DL_I_HALL_ChangeRequest), DTK_GP_MONEY_CHANGE, uIndex, dwHandleID);
		}
		else if (pNetHead->bAssistantID == ASS_GP_JEWEL2COIN)
		{
			MSG_GP_I_ChangeRequest *_in = (MSG_GP_I_ChangeRequest*)pData;

			DL_I_HALL_ChangeRequest _data;
			memcpy_s(&_data._data,sizeof(MSG_GP_I_ChangeRequest),_in,sizeof(MSG_GP_I_ChangeRequest));
			_data.itype = 1;

			m_SQLDataManage.PushLine(&_data.DataBaseLineHead, sizeof(DL_I_HALL_ChangeRequest), DTK_GP_MONEY_CHANGE, uIndex, dwHandleID);
		}
		return true;
	}
    else if(MDM_GP_LUCKDRAW == pNetHead->bMainID)
    {
        if(pNetHead->bAssistantID == ASS_GP_LUCK_DRAW_CONFIG)
        {
            if (uSize != sizeof(MSG_GP_I_LuckDraw_Config) || pData == NULL) return false;
            MSG_GP_I_LuckDraw_Config *pRecvData = (MSG_GP_I_LuckDraw_Config*)pData;
            DL_I_HALL_LUCK_DRAW_CONFIG data;
            memset(&data,0,sizeof(data));
            memcpy(&data._data, pRecvData, sizeof(MSG_GP_I_LuckDraw_Config));
            m_SQLDataManage.PushLine(&data.DataBaseLineHead, sizeof(data), DTK_GP_LUCK_DRAW_CONFIG, uIndex, dwHandleID);
        }
        else if(pNetHead->bAssistantID == ASS_GP_LUCK_DRAW_DO)
        {
            if (uSize != sizeof(MSG_GP_I_LuckDraw_DO) || pData == NULL) return false;
            MSG_GP_I_LuckDraw_DO *pRecvData = (MSG_GP_I_LuckDraw_DO*)pData;
            DL_I_HALL_LUCK_DRAW_DO data;
            memset(&data,0,sizeof(data));
            memcpy(&data._data, pRecvData, sizeof(MSG_GP_I_LuckDraw_DO));
            m_SQLDataManage.PushLine(&data.DataBaseLineHead, sizeof(data), DTK_GP_LUCK_DRAW_DO, uIndex, dwHandleID);
        }
        return true;
    }
	
	return false;
}

bool CGameLogonManage::OnHandleGMessage(NetMessageHead *pNetHead, void *pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	if (m_pHandleGMessage)
		return m_pHandleGMessage->OnNetMessage(pNetHead,pData,uSize,uAccessIP,uIndex,dwHandleID);

	return true;
}


/// 获取在线GM列表
void CGameLogonManage::OnPlayer2Gm(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	LPUSER pUser = NULL;
	switch(pNetHead->bAssistantID)
	{
	case ASS_GP_GET_GM_LIST:		///< 获取在线GM列表并下发
		{
			MSG_GP_R_GmList_t gmList;
			for(int i=0; i<m_UserList.GetCount(); i++)
			{
				pUser = (LPUSER)m_UserList.GetAt(i);
				if(pUser == NULL)
					continue;
				if(pUser->bIsGameManager)
				{
					gmList._arGmID[gmList._nCount++] = pUser->UserID;
				}
			}
			m_TCPSocket.SendData(uIndex, &gmList, sizeof(gmList), MDM_GP_PLAYER_2_GM, ASS_GP_GET_GM_LIST, 0, dwHandleID);
			break;
		}
	case ASS_GP_TALK_2_GM:		///< 普通玩家与指定的GM聊天
		{
			if (uSize != sizeof(MSG_GP_S_Talk2GM_t))
			{
				return;
			}
			MSG_GP_S_Talk2GM_t *pMessage = (MSG_GP_S_Talk2GM_t *)pData;
			GM_Broadcast_t msg;
			msg.iGMID = pMessage->iGMID;
			msg.iUserID = pMessage->iUserID;
			CopyMemory(msg.szMsg, pMessage->szMsg, sizeof(msg.szMsg));
			msg.szMsg[sizeof(msg.szMsg)/sizeof(TCHAR)-1] = TEXT('\0');
			
			SendDataByUser(pMessage->iGMID, &msg, sizeof(GM_Broadcast_t), MDM_GP_PLAYER_2_GM, ASS_GP_TALK_2_GM, 0, 0);
			m_TCPSocket.SendData(uIndex, &msg, sizeof(GM_Broadcast_t), MDM_GP_PLAYER_2_GM, ASS_GP_TALK_2_GM, 0, 0);
		}
		break;
	case ASS_GP_MSG_FROM_GM:
		{
			GM_Broadcast_t *pMessage = (GM_Broadcast_t *)pData;
			int iDstIndex = GetIndexByID(pMessage->iUserID);
			if (iDstIndex >= 0)
			{
				m_TCPSocket.SendData(uIndex, pMessage, sizeof(GM_Broadcast_t), MDM_GP_PLAYER_2_GM, ASS_GP_MSG_FROM_GM, 0, 0);
			}
		}
	default:
		break;
	}
}
// 获取在线GM列表}}

void CGameLogonManage::OnPropMessage(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	switch(pNetHead->bAssistantID)
	{
	case ASS_PROP_BUY_VIP:                                      //道具面板传回的购买VIP消息
		{
	
			_TAG_PROP_BUY_VIP *propGetSaved=(_TAG_PROP_BUY_VIP*)pData;
			PROP_GP_I_BUY_VIP propGpIGetPropVIP;

			propGpIGetPropVIP.propBuyVIP.dwUserID = propGetSaved->dwUserID;
			propGpIGetPropVIP.propBuyVIP.nPropType = propGetSaved->nPropType;
			memcpy(propGpIGetPropVIP.propBuyVIP.szTargetUser,propGetSaved->szTargetUser,32);

 			m_SQLDataManage.PushLine(&propGpIGetPropVIP.DataBaseHead,sizeof(PROP_GP_I_BUY_VIP),DTK_GP_PROP_BUY_VIP,uIndex,dwHandleID);
		}
	case ASS_PROP_GETUSERPROP:
		{
			MSG_PROP_C_GETSAVED *propGetSaved=(MSG_PROP_C_GETSAVED*)pData;
			PROP_GP_I_GETPROP propGpIGetProp;
			propGpIGetProp.dwUserID=propGetSaved->dwUserID;
			m_SQLDataManage.PushLine(&propGpIGetProp.DataBaseHead,sizeof(PROP_GP_I_GETPROP),DTK_GP_PROP_GETPROP,uIndex,dwHandleID);
		}
		break;
	case ASS_PROP_BUY:
		{
			if(uSize!=sizeof(_TAG_PROP_BUY))
				return;
			_TAG_PROP_BUY *propBuy=(_TAG_PROP_BUY*)pData;
			PROP_GP_I_BUY propIBuy;
			ZeroMemory(&propIBuy,sizeof(PROP_GP_I_BUY));
			propIBuy.propBuy.dwUserID=propBuy->dwUserID;
			propIBuy.propBuy.nPropID=propBuy->nPropID;
			propIBuy.propBuy.iPropPayMoney=propBuy->iPropPayMoney;
			propIBuy.propBuy.nPropBuyCount=propBuy->nPropBuyCount;
			propIBuy.propBuy.bUse = propBuy->bUse;
			/// 如果道具数量是负数，则什么也不做
			if (propIBuy.propBuy.nPropBuyCount <=0)
			{
				break;
			}
			m_SQLDataManage.PushLine(&propIBuy.DataBaseHead,sizeof(PROP_GP_I_BUY),DTK_GP_PROP_BUY,uIndex,dwHandleID);
		}
		break;
    case ASS_PROP_BUY_NEW:      // 商店的即时购买功能
        {
            if(uSize!=sizeof(_TAG_PROP_BUY)) return;

            _TAG_PROP_BUY *propBuy=(_TAG_PROP_BUY*)pData;
            PROP_GP_I_BUY propIBuy;
            ZeroMemory(&propIBuy,sizeof(PROP_GP_I_BUY));
            propIBuy.propBuy.dwUserID=propBuy->dwUserID;
            propIBuy.propBuy.nPropID=propBuy->nPropID;
            propIBuy.propBuy.iPropPayMoney=propBuy->iPropPayMoney;
            propIBuy.propBuy.nPropBuyCount=propBuy->nPropBuyCount;
			propIBuy.propBuy.bUse = propBuy->bUse;
            
            // 如果道具数量是负数，则什么也不做
            if (propIBuy.propBuy.nPropBuyCount <=0)
            {
                break;
            }
            
            // 购买道具的花费服务器自取，传过来的值没有意义。
            // 此处使用这个值，作为区别是在房间还是游戏端购买
            // 0表示在房间购买，1表示在游戏端购买
            propIBuy.propBuy.iPropPayMoney = pNetHead->bHandleCode;
            m_SQLDataManage.PushLine(&propIBuy.DataBaseHead,sizeof(PROP_GP_I_BUY),
                                     DTK_GP_PROP_BUY_NEW,uIndex,dwHandleID);
        }
        break;
	case ASS_PROP_GIVE:
		{
			if(uSize!=sizeof(_TAG_PROP_GIVE))
				return;
			PROP_GP_I_GIVE propIGive;
			ZeroMemory(&propIGive,sizeof(PROP_GP_I_GIVE));
			memcpy(&propIGive.propGive,pData,uSize);
			if (propIGive.propGive.nGiveCount<=0)
			{
				break;
			}
			
			m_SQLDataManage.PushLine(&propIGive.DataBaseHead,sizeof(PROP_GP_I_GIVE),DTK_GP_PROP_GIVE,uIndex,dwHandleID);
			break;
		}
	case ASS_PROP_BIG_BOARDCASE:
		{//大喇叭
			if(uSize!=sizeof(_TAG_BOARDCAST))
				return;
			PROP_GPR_I_BOARDCAST iBoardcast;
			ZeroMemory(&iBoardcast,sizeof(PROP_GPR_I_BOARDCAST));
			memcpy(&iBoardcast.boardCast,pData,uSize);

			string str_temp(iBoardcast.boardCast.szMessage);
			m_HNFilter.censor(str_temp,false);
			strcpy(iBoardcast.boardCast.szMessage,str_temp.c_str());
			iBoardcast.boardCast.iLength = strlen(iBoardcast.boardCast.szMessage) + 1;
			iBoardcast.boardCast.szMessage[iBoardcast.boardCast.iLength+1]=0;

			m_SQLDataManage.PushLine(&iBoardcast.DataBaseHead,sizeof(PROP_GPR_I_BOARDCAST),DTK_GPR_PROP_BIG_BRD,uIndex,dwHandleID);
			break;
		}
	case ASS_PROP_BIG_BOARDCASE_BUYANDUSE:
		{
			if(uSize!=sizeof(_TAG_BOARDCAST))
				return;
			PROP_GPR_I_BOARDCAST iBoardcast;
			ZeroMemory(&iBoardcast,sizeof(PROP_GPR_I_BOARDCAST));
			memcpy(&iBoardcast.boardCast,pData,uSize);

			//大喇叭消息
			/*if (!m_TalkFilter.IsAllowableTalk(iBoardcast.boardCast.szMessage,strlen(iBoardcast.boardCast.szMessage)))
			{
				const CHAR* pTemTalk = m_TalkFilter.GetReplaceContent();
				strcpy(iBoardcast.boardCast.szMessage,pTemTalk);
			}*/
			string str_temp(iBoardcast.boardCast.szMessage);
			m_HNFilter.censor(str_temp,false);
			strcpy(iBoardcast.boardCast.szMessage,str_temp.c_str());
			iBoardcast.boardCast.iLength = strlen(iBoardcast.boardCast.szMessage) + 1;
			iBoardcast.boardCast.szMessage[iBoardcast.boardCast.iLength+1]=0;

			m_SQLDataManage.PushLine(&iBoardcast.DataBaseHead,sizeof(PROP_GPR_I_BOARDCAST),DTK_GPR_PROP_BIG_BRD_BUYADNUSE,uIndex,dwHandleID);
		}
		break;
	}

}

bool CGameLogonManage::onSendGameList(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	switch (pNetHead->bAssistantID)
	{
	case ASS_GP_LIST_KIND:	//发送游戏类型列表
		{
			//获取构件类型
			bool bFinish=false;
			INT_PTR uBeginPos=0;
			char bBuffer[MAX_SEND_SIZE];
			do
			{
				memset(bBuffer,0,sizeof(bBuffer));
				INT_PTR uCopyPos=m_GameList.FillGameKind(bBuffer,sizeof(bBuffer),uBeginPos,bFinish);
				uBeginPos+=uCopyPos;
				if (bFinish)
					m_TCPSocket.SendData(uIndex,bBuffer,(UINT)uCopyPos*sizeof(ComKindInfo),MDM_GP_LIST,ASS_GP_LIST_KIND,ERR_GP_LIST_FINISH,dwHandleID);//发送游戏类型列表结束标志
				else	
					m_TCPSocket.SendData(uIndex,bBuffer,(UINT)uCopyPos*sizeof(ComKindInfo),MDM_GP_LIST,ASS_GP_LIST_KIND,ERR_GP_LIST_PART,dwHandleID);
			} while (bFinish==false);

			//获取构件名字
			uBeginPos=0;
			bFinish=false;

			do
			{
				memset(bBuffer,0,sizeof(bBuffer));
				INT_PTR uCopyPos=m_GameList.FillGameName(bBuffer,sizeof(bBuffer),uBeginPos,bFinish);
				uBeginPos+=uCopyPos;
				if (bFinish) 
					m_TCPSocket.SendData(uIndex,bBuffer,(UINT)uCopyPos*sizeof(ComNameInfo),MDM_GP_LIST,ASS_GP_LIST_NAME,ERR_GP_LIST_FINISH,dwHandleID);
				else 
					m_TCPSocket.SendData(uIndex,bBuffer,(UINT)uCopyPos*sizeof(ComNameInfo),MDM_GP_LIST,ASS_GP_LIST_NAME,ERR_GP_LIST_PART,dwHandleID);
			} while (bFinish==false);

			return true;
		}
	case ASS_GP_LIST_ROOM:	//获取房间列表
		{
			//效验数据
			if (uSize!=sizeof(MSG_GP_SR_GetRoomStruct)) return false;
			MSG_GP_SR_GetRoomStruct * pGetRoomList=(MSG_GP_SR_GetRoomStruct *)pData;

			//获取构件房间
			bool bFinish=false;
			INT_PTR uBeginPos=0;
			char bBuffer[MAX_SEND_SIZE];
			memset(bBuffer,0,sizeof(bBuffer));
			((MSG_GP_SR_GetRoomStruct *)bBuffer)->uKindID=pGetRoomList->uKindID;
			((MSG_GP_SR_GetRoomStruct *)bBuffer)->uNameID=pGetRoomList->uNameID;
			do
			{
				INT_PTR uCopyPos=m_GameList.FillGameRoom(bBuffer+sizeof(MSG_GP_SR_GetRoomStruct),sizeof(bBuffer)-sizeof(MSG_GP_SR_GetRoomStruct),pGetRoomList->uKindID,
					pGetRoomList->uNameID,uBeginPos,bFinish);
				uBeginPos+=uCopyPos;
				if (bFinish) m_TCPSocket.SendData(uIndex,bBuffer,sizeof(MSG_GP_SR_GetRoomStruct)+(UINT)uCopyPos*sizeof(ComRoomInfo),MDM_GP_LIST,ASS_GP_LIST_ROOM,ERR_GP_LIST_FINISH,dwHandleID);
				else m_TCPSocket.SendData(uIndex,bBuffer,sizeof(MSG_GP_SR_GetRoomStruct)+(UINT)uCopyPos*sizeof(ComRoomInfo),MDM_GP_LIST,ASS_GP_LIST_ROOM,ERR_GP_LIST_PART,dwHandleID);
			} while (bFinish==false);

			return true;
		}
	case ASS_GP_ROOM_PASSWORD:	// 验证某房间的密码
		{
			//效验数据
			if (uSize!=sizeof(MSG_GP_CheckRoomPasswd)) return false;
			MSG_GP_CheckRoomPasswd * pCheckRoomPasswd=(MSG_GP_CheckRoomPasswd *)pData;

			DL_GP_I_EnterCrypticRoom enterRoom;
			enterRoom.uRoomID = pCheckRoomPasswd->uRoomID;
			strcpy(enterRoom.szMD5PassWord, pCheckRoomPasswd->szMD5PassWord);

			m_SQLDataManage.PushLine(&enterRoom.DataBaseHead,sizeof(DL_GP_I_EnterCrypticRoom), DTK_GP_CHECK_ROOM_PASSWD, uIndex, dwHandleID);//提交数据库

			return true;
		}
	case ASS_GP_LIST_CONTEST:	// 获取比赛列表
		{
			LPUSER pUser = NULL;
			for(int i=0; i<m_UserList.GetCount(); i++)
			{
				pUser = (LPUSER)m_UserList.GetAt(i);
				if(pUser == NULL)
					continue;
				if(pUser->iSocketIndex == uIndex )
				{
					DL_GP_I_ContestListStruct in;
					ZeroMemory(&in, sizeof(in));
					in.iUserID = pUser->UserID;

					m_SQLDataManage.PushLine(&in.DataBaseHead, sizeof(in), DTK_GP_CONTEST_APPLY_LIST, uIndex, dwHandleID);

				}
			}

			return true;
		}
	}
}


void CGameLogonManage::SendRoomListPeoCount()
{
	//获取构件房间
	bool bFinish=false;
	INT_PTR uBeginPos=0;
	char bBuffer[MAX_SEND_SIZE];
	memset(bBuffer,0,sizeof(bBuffer));
	//发送在线人数
	uBeginPos=0;
	bFinish=false;
	UINT dwHandleID=0;
	do
	{
		INT_PTR uCopyPos=m_GameList.FillRoomOnLineCount(bBuffer,sizeof(bBuffer),uBeginPos,bFinish);
		if(uCopyPos <= 0)break;
		uBeginPos+=uCopyPos;
		if(bFinish)dwHandleID=1;
		m_TCPSocket.SendDataBatch(
			bBuffer,
			(UINT)uCopyPos*sizeof(DL_GP_RoomListPeoCountStruct),
			MDM_GP_LIST,
			ASS_GP_LIST_COUNT,
			0    /////
			);//dwHandleID);
	} while (bFinish==false);

	uBeginPos=0;
	bFinish=false;
	dwHandleID=0;
	do
	{
		INT_PTR uCopyPos=m_GameList.FillNameOnLineCount(bBuffer,sizeof(bBuffer),uBeginPos,bFinish);
		if(uCopyPos <= 0)break;
		uBeginPos+=uCopyPos;
		if(bFinish)dwHandleID=1;
		m_TCPSocket.SendDataBatch(
			bBuffer,
			(UINT)uCopyPos*sizeof(DL_GP_RoomListPeoCountStruct),
			MDM_GP_LIST,
			ASS_GP_LIST_COUNT,
			1    /////
			);//dwHandleID);
	} while (bFinish==false);
}
//启动函数
bool CGameLogonManage::OnStart()
{
	//更新构件列表操作
	DL_GP_I_UpdateDataListStruct UpdateList;
	UpdateList.pGameList=&m_GameList;
	//更新构件列表
	m_SQLDataManage.PushLine(&UpdateList.DataBaseHead,sizeof(UpdateList),DTK_GP_UPDATE_LIST,0,0);

	//更新转账返利数据sdp
	DL_GP_I_RebateUpDateStruct Rebate;
	m_SQLDataManage.PushLine(&Rebate.DataBaseHead,sizeof(Rebate),DTK_GP_BANK_UPDATE_REBATE,0,0);
	//服务器启动，在线用户标志清0
	DL_I_HALL_ClearOnlineInfo UserOnlineClear;
	UserOnlineClear.ZID = m_ID;
	m_SQLDataManage.PushLine(&UserOnlineClear.DataBaseLineHead,sizeof(UserOnlineClear),DTK_GP_IM_CLEAR,0,0);
	
	//检测数据库连接
	SetTimer(IDT_CHECK_DATA_CONNECT,15000L);

	SetTimer(IDT_GET_ROOM_LIST_PEOCOUNT,TIMEOUT_GET_ROOM_LIST_PEOCOUNT); //直接去掉定时刷新的人数的功能，直接删除所有数据，再从服务器获取

	SetTimer(IDT_GET_MAIL_LIST, TIMEOUT_GET_MAIL_LIST * 100 * 60);

	SetTimer(IDT_CHECKRECONNECT, TIMEOUT_CHECKRECONNECT * 1000);
	SetTimer(IDT_CHECKKEEPLIVE, TIMEOUT_CHECKKEEPLIVE * 1000);

	SetTimer(IDT_GET_LOGON_PEOPLE_COUNT,TIMEOUT_GET_LOGON_PEOPLE * 1000);

    // 时效房间超时解散
    SetTimer(IDT_DISSMISS_TIMEOUTDESK, 60 * 1000);

	m_pGmMessageBase = CreateGMModule(this);
	m_pDistriManager = CreateDistriManager(this);
    
	UINT errCode;
	m_pGameManageModule->StartService(errCode);

	//重置GServer连接信息
	DL_I_HALL_UpdateRoomConnect_ALL RoomConnect;
	RoomConnect.RoomID = m_pGameManageModule->m_LogonManage.GetRoomList(&RoomConnect.iCount);
	m_SQLDataManage.PushLine(&RoomConnect.DataBaseLineHead,sizeof(RoomConnect),DTK_GP_UPDATE_CONNECT_ALL,0,0);
	SetTimer(IDT_UPDATE_ROOMCONNECT,TIMEOUT_UPDATE_ROOMCONNECT * 1000);

	return true;
}

// 处理登录消息
void HandleLogonMsg(LPMSG_CONTEXT_HEAD msg_context_ptr)
{
    int iResult = ASS_GP_LOGON_ERROR;
    int iHandleCode = ERR_GP_USER_PASS_ERROR;

    char chTemp[5] = "00";

    int iRet = 0;
	bool bres = 0;
    switch(msg_context_ptr->opr_ret)
    {
	// 登录成功，处理用户数据写入数据库等事情
    case MSG_OP_LOGON_SUCCESS:
        iResult = ASS_GP_LOGON_SUCCESS;

        // 写入数据库 SP_AddOneUse
		bres=g_pGameLogonManage->m_SQLDataManage.m_pEngineSink->setStoredProc("Web_pUsersAdd1",true);
		if(0!=bres)
		{
			iResult = ASS_GP_LOGON_ERROR;

			// 数据库操作失败
			iHandleCode = ERR_GP_DATABASE_FUNC_ERROR;  
			return;
		}

        // 检测用户名和密码是否正确
        if((strlen(g_chUserName) < 1) || 
           (strlen(g_chMD5Pass) < 20))
        {
            iResult = ASS_GP_LOGON_ERROR;
            break;
        }
        
        g_pGameLogonManage->m_SQLDataManage.m_pEngineSink->addInputParameter("@UserName", sizeof(g_chUserName), g_chUserName);
        g_pGameLogonManage->m_SQLDataManage.m_pEngineSink->addInputParameter("@NickName", sizeof(g_chUserName), g_chUserName);
        g_pGameLogonManage->m_SQLDataManage.m_pEngineSink->addInputParameter("@TGName", sizeof(chTemp), chTemp);
        g_pGameLogonManage->m_SQLDataManage.m_pEngineSink->addInputParameter("@Pass", sizeof(g_chMD5Pass), g_chMD5Pass);
        g_pGameLogonManage->m_SQLDataManage.m_pEngineSink->addInputParameter("@TwoPassword", sizeof(g_chMD5Pass), g_chMD5Pass);
        g_pGameLogonManage->m_SQLDataManage.m_pEngineSink->addInputParameter("@Sex", 0);
        g_pGameLogonManage->m_SQLDataManage.m_pEngineSink->addInputParameter("@LogoId", 1);
        g_pGameLogonManage->m_SQLDataManage.m_pEngineSink->addInputParameter("@ZJ_Number", sizeof(chTemp), chTemp);
        g_pGameLogonManage->m_SQLDataManage.m_pEngineSink->addInputParameter("@RegisterIP", sizeof(chTemp), chTemp);
        g_pGameLogonManage->m_SQLDataManage.m_pEngineSink->addInputParameter("@PhoneNum", sizeof(chTemp), chTemp);
        g_pGameLogonManage->m_SQLDataManage.m_pEngineSink->addInputParameter("@Email", sizeof(chTemp), chTemp);
        g_pGameLogonManage->m_SQLDataManage.m_pEngineSink->addInputParameter("@QQNum", sizeof(chTemp), chTemp);

		if(0 != g_pGameLogonManage->m_SQLDataManage.m_pEngineSink->execStoredProc())
		{
			g_pGameLogonManage->m_SQLDataManage.m_pEngineSink->closeRecord();
			iResult = ASS_GP_LOGON_ERROR;
			iHandleCode = ERR_GP_DATABASE_FUNC_ERROR; 
			break;
		}

        iRet = g_pGameLogonManage->m_SQLDataManage.m_pEngineSink->getReturnValue();
		g_pGameLogonManage->m_SQLDataManage.m_pEngineSink->closeRecord();

		// 存储过程执行成功, 重新去认证登录
        if(0 == iRet)   
        {
            g_pGameLogonManage->m_TCPSocket.SendData(msg_context_ptr->serial_no/*pUserInfo->iUserIndex*/, MDM_GP_LOGON, ASS_GP_LOGON_BY_SOFTWARE, iHandleCode, 0);
            return;
        }
        else            
        {
			// 存储过程执行失败
            iResult = ASS_GP_LOGON_ERROR;
			
			// 数据库返回执行错误结果
            iHandleCode = ERR_GP_DATABASE_RETURN_ERROR;   
        }

        break;

	// 用户名或密码错误
    case MSG_OP_LOGON_INFO_ERROR:           
        iHandleCode = ERR_GP_USER_PASS_ERROR;
        break;

	// 用户已登录
    case MSG_OP_LOGON_USING:                
        iHandleCode = ERR_GP_USER_LOGON;
        break;

	// 用户不存在 
    case MSG_OP_LOGON_NO_USER:
        iHandleCode = ERR_GP_USER_NOT_EXIST;
        break;

	// 用户处于禁用状态
    case MSG_OP_LOGON_FORBID:               
        iHandleCode = ERR_GP_USER_VALIDATA;
        break;

	// 用户已过期
    case MSG_OP_LOGON_OVERDATE:             
        iHandleCode = ERR_GP_USER_OVERDATE;
        break;
    default:
        break;
    }

    g_pGameLogonManage->m_TCPSocket.SendData(msg_context_ptr->serial_no, MDM_GP_LOGON, iResult, iHandleCode, 0);
}

// 消息回调函数, 用于第三方 DLL 调用
void __stdcall MsgHandleCallback(int msg_type, LPMSG_CONTEXT_HEAD msg_context_ptr)
{
    switch(msg_type)
    {
    case MSG_TYPE_LOGON:
        //::EnterCriticalSection(&g_CriticalSection); // 此处移动到他处，屏蔽
        HandleLogonMsg(msg_context_ptr);
        ::LeaveCriticalSection(&g_CriticalSection);
        break;
    default:        // 暂时直接返回登录错误
        g_pGameLogonManage->m_TCPSocket.SendData(msg_context_ptr->serial_no, MDM_GP_LOGON, ASS_GP_LOGON_ERROR, ERR_GP_USER_PASS_ERROR, 0);
        break;
    }
}

bool CGameLogonManage::SetRoomConnect(int iRoomID,bool bConnect)
{
	DL_I_HALL_UpdateRoomConnect UpdateRoom;
	UpdateRoom.bConnect = bConnect;
	UpdateRoom.iRoomID = iRoomID;
	m_SQLDataManage.PushLine(&UpdateRoom.DataBaseLineHead,sizeof(UpdateRoom),DTK_GP_UPDATE_CONNECT,0,0);
	return true;
}

// 定时器消息
bool CGameLogonManage::OnTimerMessage(UINT uTimerID)
{
	switch (uTimerID)
	{
	// 更新构件列表定时器
	case IDT_GET_ROOM_LIST_PEOCOUNT:			
		{
			DL_GP_I_UpdateDataListStruct UpdateList;
			UpdateList.pGameList=&m_GameList;
			m_SQLDataManage.PushLine(&UpdateList.DataBaseHead,sizeof(UpdateList),DTK_GP_GET_ROOM_LIST_PEOCOUNT,0,0);
			return true;
		}
	case TID_UPDATE_LIST:
		{
			DL_GP_I_UpdateDataListStruct UpdateList;
			UpdateList.pGameList=&m_GameList;
			m_SQLDataManage.PushLine(&UpdateList.DataBaseHead,sizeof(UpdateList),DTK_GP_UPDATE_LIST,0,0);
			return true;
		}
	case IDT_GET_MAIL_LIST:
		{
			HNLOG_M("IDT_GET_MAIL_LIST");
			DataBaseLineHead _p;
			m_SQLDataManage.PushLine(&_p, sizeof(DataBaseLineHead),DTK_GP_UPDATE_MAIL, 0, 0);
			return true;
		}
	case IDT_GET_LOGON_PEOPLE_COUNT:
		{
			//KillTimer(IDT_GET_LOGON_PEOPLE_COUNT);
			DataBaseLineHead _p;
			m_SQLDataManage.PushLine(&_p, sizeof(DataBaseLineHead),DTK_GP_LOGONPEOPLE_GET, 0, 0);
			//SetTimer(IDT_GET_LOGON_PEOPLE_COUNT,TIMEOUT_GET_LOGON_PEOPLE * 1000);
			return true;
		}
	case IDT_CHECK_DATA_CONNECT:	//检测数据库连接
		{
			//KillTimer(IDT_CHECK_DATA_CONNECT);
			DataBaseLineHead DataBaseHead;
			m_SQLDataManage.PushLine(&DataBaseHead,sizeof(DataBaseHead),DTK_GP_CHECK_CONNECT,0,0);
			
			return true;
		}
	case IDT_CHECKRECONNECT:
		{
			KillTimer(IDT_CHECKRECONNECT);
			if (m_pDistriManager) 
				m_pDistriManager->CheckReConnect();
			SetTimer(IDT_CHECKRECONNECT, TIMEOUT_CHECKRECONNECT * 1000);

			return true;
		}
	case IDT_CHECKKEEPLIVE:
		{
			KillTimer(IDT_CHECKKEEPLIVE);
			if (m_pDistriManager) 
				m_pDistriManager->CheckKeepLive();
			SetTimer(IDT_CHECKKEEPLIVE, TIMEOUT_CHECKKEEPLIVE * 1000);

			return true;
		}
	case IDT_UPDATE_ROOMCONNECT:
		{
			DL_I_HALL_UpdateRoomConnect_ALL RoomConnect;
			RoomConnect.RoomID = m_pGameManageModule->m_LogonManage.GetRoomList(&RoomConnect.iCount);
			m_SQLDataManage.PushLine(&RoomConnect.DataBaseLineHead,sizeof(RoomConnect),DTK_GP_UPDATE_CONNECT_ALL,0,0);

			return true;
		}
    case IDT_DISSMISS_TIMEOUTDESK:
        {
            DataBaseLineHead head;
            m_SQLDataManage.PushLine(&head,sizeof(head),DTK_GP_DISSMISS_TIMEOUTDESK,0,0);
            return true;
        }
	}
	return false;
}




// 数据库处理结果
// 以下是数据操作返回的处理函数
bool CGameLogonManage::OnDataBaseResult(DataBaseResultLine * pResultData)
{
	if ((pResultData->uHandleKind>=GM_DT_MESSAGE_START) && (pResultData->uHandleKind<=GM_DT_MESSAGE_END))
	{
		return m_pGmMessageBase->OnDataBaseResultEvent(pResultData);
	}
	switch (pResultData->uHandleKind)
	{		
	case DTK_GP_GET_ROOM_LIST_PEOCOUNT:	// 更新构件列表
		{
			// 效验数据
			if (pResultData->LineHead.uSize!=sizeof(DataBaseResultLine)) return false;
			// 处理数据
			if (pResultData->uHandleRusult==DTR_GP_GET_ROOM_LIST_PEOCOUNT_SCUESS)	
				SendRoomListPeoCount();

			return true;
		}
	case DTK_GP_REGISTER:
		{
			if (m_pRegisterServer)
			{
				return m_pRegisterServer->OnDataBaseResult(pResultData);
			}
			return true;
		}
	case DTK_GP_LOGON_BY_NAME:		//通过名字登陆
		{
			if (m_pLogonServer)
			{
				return m_pLogonServer->OnDataBaseResult(pResultData);
			}
			return true;
		}
	case DTK_GP_UPDATE_LIST:	//更新构件列表
		{
			// 效验数据
			if (pResultData->LineHead.uSize!=sizeof(DataBaseResultLine)) 
				return false;
			SetTimer(TID_UPDATE_LIST,LS_TIME_OUT);
			if (pResultData->uHandleRusult==DTR_GP_LIST_SCUESS)	
				m_GameList.UpdateGameList();
			return true;
		}
	case DTK_GP_CHECK_CONNECT:	// 检测数据库连接
		{
			//效验数据
			if (pResultData->LineHead.uSize!=sizeof(DataBaseResultLine)) return false;
			//设置定时器
			//if (m_bRun==true) SetTimer(IDT_CHECK_DATA_CONNECT,15000L);
			return true;
		}
		//返回用户头像数据
	case DTK_GP_USERLOGO_GETMD5:
		{
			UL_GP_I_UserLogoInformation *UL_Info=(UL_GP_I_UserLogoInformation*)pResultData;
			MSG_UL_S_INFORMATION *info=(MSG_UL_S_INFORMATION*)&UL_Info->UserLogoInformation;
			m_TCPSocket.SendData(pResultData->uIndex,info,sizeof(MSG_UL_S_INFORMATION),MDM_GR_USER_LOGO,
				ASS_ULS_LOGOINFORMATION,0,pResultData->dwHandleID);
		}
		break;
		//返回用户好友资料
	case DTK_GP_IM_USERLIST:
		{
			IM_GP_O_USERLIST *imOUserList=(IM_GP_O_USERLIST*)pResultData;
			MSG_IM_S_USERLIST msg;
			ZeroMemory(&msg,sizeof(MSG_IM_S_USERLIST));
			msg.nBodyLength=imOUserList->dwLength;
			msg.nListCount=imOUserList->dwListCount;
			memcpy(msg.sBody,imOUserList->szData,imOUserList->dwLength);
			m_TCPSocket.SendData(pResultData->uIndex,&msg,sizeof(MSG_IM_S_USERLIST)-sizeof(msg.sBody)+msg.nBodyLength,MDM_GP_IM,ASS_IMS_GETLIST,0,pResultData->dwHandleID);
		}
		break;
	case DTK_GP_IM_ONLINE:		//好友上线或下线都用这个函数体操作
	case DTK_GP_IM_OFFLINE:
		{
			IM_GP_O_USERINFO * imOUserInfo=(IM_GP_O_USERINFO*)pResultData;
			MSG_IM_S_GETUSERINFO msg;
			memcpy(&msg,&imOUserInfo->userInfo,sizeof(MSG_IM_S_GETUSERINFO));
			SendDataByUser(imOUserInfo->userInfo.dwUserID, &msg, sizeof(MSG_IM_S_GETUSERINFO),MDM_GP_IM,ASS_IMS_GETUSERINFO,0,pResultData->dwHandleID);
		}
		break;
	case DTK_GP_PROP_GETPROP:
		{
			PROP_GP_O_GETPROP *propOGet=(PROP_GP_O_GETPROP*)pResultData;
			MSG_PROP_S_GETPROP msg;
			memcpy(&msg,&propOGet->msgPropGet,sizeof(MSG_PROP_S_GETPROP));
			m_TCPSocket.SendData(pResultData->uIndex,&msg,sizeof(MSG_PROP_S_GETPROP),MDM_GP_PROP,ASS_PROP_GETUSERPROP,0,pResultData->dwHandleID);
		}
		break;
	case DTK_GP_PROP_BUY:
		{
			PROP_GP_O_BUY *propOBuy=(PROP_GP_O_BUY*)pResultData;
			MSG_PROP_S_GETPROP msg;
			memcpy(&msg,&propOBuy->msgPropGet,sizeof(MSG_PROP_S_GETPROP));
			m_TCPSocket.SendData(pResultData->uIndex,&msg,sizeof(MSG_PROP_S_GETPROP),MDM_GP_PROP,ASS_PROP_BUY,pResultData->uHandleRusult,pResultData->dwHandleID);
		}
		break;
	case DTK_GP_PROP_BUY_VIP:
		{
			PROP_GP_O_BUY_VIP *propOBuy=(PROP_GP_O_BUY_VIP*)pResultData;
			MSG_PROP_S_BUYVIP msg;
			memcpy(&msg,&propOBuy->msgPropGet,sizeof(MSG_PROP_S_BUYVIP));
			m_TCPSocket.SendData(pResultData->uIndex,&msg,sizeof(MSG_PROP_S_BUYVIP),MDM_GP_PROP,ASS_PROP_BUY_VIP,pResultData->uHandleRusult,pResultData->dwHandleID);
		}
		break;
    case DTK_GP_PROP_BUY_NEW:   // 商店道具的即时购买功能
        {
            PROP_GP_O_BUY *propOBuy=(PROP_GP_O_BUY*)pResultData;
            MSG_PROP_S_GETPROP msg;
            memcpy(&msg,&propOBuy->msgPropGet,sizeof(MSG_PROP_S_GETPROP));
            m_TCPSocket.SendData(pResultData->uIndex,&msg,sizeof(MSG_PROP_S_GETPROP),MDM_GP_PROP,
                                 ASS_PROP_BUY_NEW,pResultData->uHandleRusult,pResultData->dwHandleID);
        }
        break;
	case DTK_GP_PROP_GIVE:
		{
			if(pResultData->uHandleRusult==DTK_GP_PROP_GIVE_SUCCEED_BEGIVER)
			{
				PROP_GP_O_GIVE_FOR * propGiveFor=(PROP_GP_O_GIVE_FOR*)pResultData;
				SendDataByUser(propGiveFor->propBeGive.propInfo.dwUserID,&propGiveFor->propBeGive,sizeof(_TAG_PROP_GIVE_FOR),MDM_GP_PROP,ASS_PROP_GIVE,DTK_GP_PROP_GIVE_SUCCEED_BEGIVER,0);
			}
			else
			{
				PROP_GP_O_GIVE * propOGive=(PROP_GP_O_GIVE*)pResultData;
				m_TCPSocket.SendData(pResultData->uIndex,&propOGive->propGive,sizeof(_TAG_PROP_GIVE),MDM_GP_PROP,ASS_PROP_GIVE,pResultData->uHandleRusult,pResultData->dwHandleID);
			}
		}
		break;
	case DTK_GPR_PROP_BIG_BRD:
		{
			PROP_GPR_O_BOARDCAST *oBoard=(PROP_GPR_O_BOARDCAST*)pResultData;
			_TAG_BOARDCAST boardCast;
			memcpy(&boardCast,&oBoard->boardCast,sizeof(_TAG_BOARDCAST));
			//m_TCPSocket.SendDataBatch(&boardCast,sizeof(_TAG_BOARDCAST),MDM_GP_PROP,ASS_PROP_BIG_BOARDCASE,pResultData->uHandleRusult);
			SendBatchData(&boardCast,sizeof(_TAG_BOARDCAST),MDM_GP_PROP,ASS_PROP_BIG_BOARDCASE,pResultData->uHandleRusult);
		}
		break;
	case DTK_GP_CHECK_ROOM_PASSWD:  //从数据库存储过程中返回进入密码房间结果
		{
			DL_GP_O_EnterCrypticRoom* pRet = (DL_GP_O_EnterCrypticRoom*)pResultData;
			DL_CheckRoomPasswd crp;
			memset(&crp, 0, sizeof(DL_CheckRoomPasswd));
			if (pResultData->uHandleRusult==DTK_GP_PASSWD_SUCC)
			{
				crp.bRet = true;
			}
			else
			{
				crp.bRet = false;
			}
			crp.uRoomID = pRet->uRoomID;
			m_TCPSocket.SendData(pResultData->uIndex, &crp, sizeof(DL_CheckRoomPasswd), MDM_GP_LIST, ASS_GP_ROOM_PASSWORD, pResultData->uHandleRusult,pResultData->dwHandleID);
			
			break;
		}
	case DTK_GP_LOCK_UNLOCK_ACCOUNT:
	case DTK_GP_BIND_MOBILE:
	case DTK_GP_UPDATE_USERINFO_BASE:
	case DTK_GP_UPDATE_USERINFO_DETAIL:
	case DTR_GP_UPDATE_USERINFO_PWD:
   // case DTK_GP_USER_ADDPHONENUM:
		{
			if (m_pUpdateUserInfo)
			{
				return m_pUpdateUserInfo->OnDataBaseResult(pResultData);
			}
			return true;
		}break;
	case DTK_GP_UPDATE_NICKNAMEID:
	{
		if (pResultData->LineHead.uSize != sizeof(DL_GP_O_GetNickNameID))
		{
			return false;
		}

		DL_GP_O_GetNickNameID* _p = (DL_GP_O_GetNickNameID*)pResultData;
		m_TCPSocket.SendData(pResultData->uIndex,&_p->_data,sizeof(MSG_GP_NickNameID_t),
			MDM_GP_USERINFO,ASS_GP_USERINFO_NICKNAMEID,pResultData->uHandleRusult,pResultData->dwHandleID);

		return true;
	}
	case DTK_GP_CHARMEXCHANGE_LIST:
		{
			if (pResultData->LineHead.uSize != sizeof(DL_GP_O_CharmExchangeList)) return false;
			//if (pResultData->uHandleRusult == 0)
			{
				DL_GP_O_CharmExchangeList* _p = (DL_GP_O_CharmExchangeList*)pResultData;
				TCharmExchange_Item _out = _p->date;
				m_TCPSocket.SendData(pResultData->uIndex, &_out, sizeof(TCharmExchange_Item), 
					MDM_GP_CHARMEXCHANGE, ASS_GETLIST, pResultData->uHandleRusult, 0);
			}
			return true;
		}
	case DTK_GP_CHARMEXCHANGE:
		{
			if (pResultData->LineHead.uSize != sizeof(DL_GP_O_CharmExchangeList)) return false;
			//if (pResultData->uHandleRusult == 5)
			{
				DL_GP_O_CharmExchangeList* _p = (DL_GP_O_CharmExchangeList*)pResultData;
				TCharmExchange_Item _out = _p->date;
				m_TCPSocket.SendData(pResultData->uIndex, &_out, sizeof(TCharmExchange_Item), 
					MDM_GP_CHARMEXCHANGE, ASS_EXCHANGE, pResultData->uHandleRusult, 0);
			}
			return true;
		}
	case DTK_GP_LOGONPEOPLE_GET:
		{
			if (pResultData->uHandleRusult == 1)
			{
				DL_GP_O_GetLogonPeopleCount* _p = (DL_GP_O_GetLogonPeopleCount*)pResultData;
				if (_p != NULL)
				{
					m_TCPSocket.SendDataBatch(&_p->logonUsers, sizeof(ONLINEUSERSCOUNT), MDM_GP_LOGONUSERS, ASS_GP_LOGONUSERS_COUNT, 0);
				}
			}
		}
		
		break;
	
	//银行处理模块
	case DTK_GP_BANK_OPEN:
	case DTK_GP_BANK_GETGM_WALLET:
	case DTK_GP_BANK_CHECK:
	case DTK_GP_BANK_TRANSFER:
	case DTK_GP_BANK_TRANS_RECORD:
	case DTK_GP_BANK_DEPOSIT:
	case DTK_GP_BANK_QUERY_DEPOSIT:
	case DTK_GP_BANK_CHPWD:
	case DTK_GP_BANK_FINANCE_INFO:
	case DTK_GP_BANK_UPDATE_REBATE://转账返利
	case DTK_GP_BANK_UPDATE_USER_REBATE://玩家返利查询
	case DTK_GP_BANK_SET_USER_REBATE: //玩家修改返利数据
		{
			m_pBankService->OnDataBaseResult(pResultData);
		}
		break;
	case DTK_GP_LASTLOGINGAME:
		{
			if (pResultData->uHandleRusult == 0)
			{
				DL_GP_O_LastLoginGame* _p = (DL_GP_O_LastLoginGame*)pResultData;
				if (_p != NULL)
				{
					CAFCKindItem* _pKind = m_GameList.FindKindItem(_p->_data.iKindID);
					if (_pKind != NULL)
					{
						CAFCNameItem* _pName = m_GameList.FindNameItem(_pKind, _p->_data.iGameID);

						if (_pName != NULL)
						{
							m_TCPSocket.SendData(pResultData->uIndex, &_pName->m_NameInfo, sizeof(ComNameInfo), MDM_GP_LASTLOINGGAME, ASS_GP_LASTLOINGGAME, 0, 0);
						}						
					}
				} 
			}
			else
			{
				m_TCPSocket.SendData(pResultData->uIndex, NULL, 0, MDM_GP_LASTLOINGGAME, ASS_GP_LASTLOINGGAME, 1, 0);
			}
		}break;
	case DTK_GP_PAIHANGBANG:
		{
            DL_GP_O_PaiHangBang* pHandleResult= (DL_GP_O_PaiHangBang*)pResultData;
            if(!pHandleResult)
            {
                return false;
            }

            char bBuffer[MAX_SEND_SIZE];
            memset(bBuffer,0,sizeof(bBuffer));
            int iMax = (MAX_SEND_SIZE-sizeof(NetMessageHead))/sizeof(MSG_GP_MoneyPaiHangBang_Item);
            int SendCount = 0;
            int SendTotalcount = 0;

            while(SendCount*iMax < pHandleResult->iCount)
            {
                int PreSend = (pHandleResult->iCount-SendCount*iMax)>iMax?iMax:(pHandleResult->iCount-SendCount*iMax);
                
                memcpy(bBuffer,(void*)&pHandleResult->_data[SendCount*iMax],sizeof(MSG_GP_MoneyPaiHangBang_Item) * PreSend);

                m_TCPSocket.SendData(pHandleResult->ResultHead.uIndex, bBuffer, sizeof(MSG_GP_MoneyPaiHangBang_Item)*PreSend,MDM_GP_PAIHANGBANG,0, pResultData->uHandleRusult, 0);
               
                SendCount++;

                memset(bBuffer,0,sizeof(bBuffer));
            }

            m_TCPSocket.SendData(pHandleResult->ResultHead.uIndex,MDM_GP_PAIHANGBANG,0, pResultData->uHandleRusult, 0);

            SafeDeleteArray(pHandleResult->_data);
            return true;
		}break;
	case DTK_GP_SIGNIN_CHECK:
		{
			DL_O_HALL_SIGN_CHECK *pSignin = (DL_O_HALL_SIGN_CHECK*)pResultData;
			m_TCPSocket.SendData(pResultData->uIndex, &pSignin->_data, sizeof(pSignin->_data), MDM_GP_SIGN, ASS_GP_SIGN_CHECK, pResultData->uHandleRusult, pResultData->dwHandleID);
		}break;
	case DTK_GP_SIGNIN_DO:
		{
			DL_O_HALL_SIGN_DO *pSignin = (DL_O_HALL_SIGN_DO*)pResultData;
			m_TCPSocket.SendData(pResultData->uIndex, &pSignin->_data, sizeof(pSignin->_data), MDM_GP_SIGN, ASS_GP_SIGN_DO, pResultData->uHandleRusult, pResultData->dwHandleID);
		}break;
	case DTK_GP_ONLINE_AWARD_CHECK:
		{
			DL_O_HALL_ONLINE_AWARD_CHECK_RESULT* pOnLineAward = (DL_O_HALL_ONLINE_AWARD_CHECK_RESULT*)pResultData;
			m_TCPSocket.SendData(pResultData->uIndex, &pOnLineAward->_result, sizeof(pOnLineAward->_result), MDM_GP_ONLINE_AWARD, ASS_GP_ONLINE_AWARD_CHECK, pResultData->uHandleRusult, pResultData->dwHandleID);
		}break;
	case DTK_GP_ONLINE_AWARD_DO:
		{
			DL_O_HALL_ONLINE_AWARD_DO_RESULT* pOnLineAward = (DL_O_HALL_ONLINE_AWARD_DO_RESULT*)pResultData;
			m_TCPSocket.SendData(pResultData->uIndex, &pOnLineAward->_result, sizeof(pOnLineAward->_result), MDM_GP_ONLINE_AWARD, ASS_GP_ONLINE_AWARD_DO, pResultData->uHandleRusult, pResultData->dwHandleID);
		}break;
	case DTK_GP_GET_CONFIG:
		{
			DL_O_HALL_GETCONFIG_RES* pConfig = (DL_O_HALL_GETCONFIG_RES*)pResultData;
			m_TCPSocket.SendData(pResultData->uIndex, &pConfig->_data, sizeof(pConfig->_data), MDM_GP_GET_CONFIG, ASS_GP_GET_CONFIG, pResultData->uHandleRusult, pResultData->dwHandleID);
		}break;
	case DTK_GP_BUY_DESK:
	case DTK_GP_ENTER_VIPDESK:
	case DTK_GP_TOTALRECORD:
	case DTK_GP_SINGLERECORD:
	case DTK_GP_GET_CUTROOM:
	case DTK_GP_GET_BUYDESKCONFIG:
	case DTK_GP_GET_RECORDURL:
	case DTK_GP_GET_DESKLIST:
	case DTK_GP_GET_DESKUSER:
	case DTK_GP_DISSMISSDESK:
	case DTK_GP_DELETERECORD:
    case DTK_GP_DISSMISSDESKBYMS:
		{
			m_pCreateRoom->OnDataBaseResult(pResultData);
		}break;
	case DTK_GP_CREATE_CLUB:
	case DTK_GP_DISSMISS_CLUB:
	case DTK_GP_JOIN_CLUB:
	case DTK_GP_CLUB_USERLIST:
	case DTK_GP_CLUB_ROOMLIST:
	case DTK_GP_CLUB_CREATEROOM:
	case DTK_GP_CLUB_CHANGENAME:
	case DTK_GP_CLUB_KICKUSER:
	case DTK_GP_CLUB_STATISTICS:
	case DTK_GP_CLUB_LIST:
	case DTK_GP_REVIEW_LIST:
	case DTK_GP_MASTER_OPTION:
	case DTK_GP_CLUB_NOTICE:	
	case DTK_GP_ENTER_CLUB:
	case DTK_GP_LEAVE_CLUB:
	case DTK_GP_GET_DESKRECORD:
		{
			m_pHNClub->OnDataBaseResult(pResultData);
		}break;	
	case DTK_GP_MAIL_LIST:
	case DTK_GP_OPEN_MAIL:	
	case DTK_GP_GET_ATTACHMENT:
	case DTK_GP_DEL_MAIL:
	case DTK_GP_UPDATE_MAIL:
	case DTK_GP_UPDATE_SYSTEM:
		{
			m_pHNMail->OnDataBaseResult(pResultData);
		}break;

	case DTK_GP_CONTEST_APPLY_LIST:
		{
			DL_GP_O_ContestListStruct* _p = (DL_GP_O_ContestListStruct*)pResultData;

			int totalcount = m_GameList.GetContestCount();
			if (0 == totalcount)
			{
				m_TCPSocket.SendData(pResultData->uIndex,MDM_GP_LIST, ASS_GP_LIST_CONTEST, 1, pResultData->dwHandleID);
				return true;
			}

			MSG_GP_ContestApplyInfo *_out = new MSG_GP_ContestApplyInfo[totalcount];
			if (!_out)
			{
				return false;
			}

			m_GameList.GetContestList(_out,&_p->_data);

			char bBuffer[MAX_SEND_SIZE];
			int  iMaxNum = sizeof(bBuffer)/sizeof(MSG_GP_ContestApplyInfo);
			int  iCount = 0;

			do
			{
				int iSendNum = (totalcount-iCount*iMaxNum)>iMaxNum?iMaxNum:(totalcount-iCount*iMaxNum);
				for (int i=0;i<iSendNum;i++)
				{
					CopyMemory(bBuffer+i*sizeof(MSG_GP_ContestApplyInfo),&_out[i+iCount*iMaxNum],sizeof(MSG_GP_ContestApplyInfo));
				}
				m_TCPSocket.SendData(pResultData->uIndex, bBuffer, iSendNum*sizeof(MSG_GP_ContestApplyInfo),MDM_GP_LIST,ASS_GP_LIST_CONTEST, 0, pResultData->dwHandleID);
				iCount++;

			}while ((iCount*iMaxNum) < totalcount);

			delete [] _out;
			
		}break;
	case DTK_GP_GET_APPLY_NUM:
	case DTK_GP_CONTEST_APPLY:
	case DTK_GP_CONTEST_NOTICE:
	case DTK_GP_CONTEST_NOTICE_LOGON:
	case DTK_GP_GET_CONTEST_ROOMID:
	case DTK_GP_CONTEST_GETAWARD:
		{
			if (m_pContestServer)
			{
				m_pContestServer->OnDataBaseResult(pResultData); 
			}

		}break;
	case DTK_GP_CHANGE_CONFIG:
		{
			DL_O_HALL_ChangeConfig_RES* pOut = (DL_O_HALL_ChangeConfig_RES*)pResultData;
			m_TCPSocket.SendData(pResultData->uIndex, &pOut->_result, sizeof(pOut->_result), MDM_GP_MONEY_CHANGE, ASS_GP_CHANGE_CONFIG, pResultData->uHandleRusult, pResultData->dwHandleID);
		}break;
	case DTK_GP_MONEY_CHANGE:
		{
			DL_O_HALL_ChangeResponse* pOut = (DL_O_HALL_ChangeResponse*)pResultData;
			if (0 == pOut->itype)
			{
				m_TCPSocket.SendData(pResultData->uIndex, &pOut->_result, sizeof(pOut->_result), MDM_GP_MONEY_CHANGE, ASS_GP_COIN2JEWEL, pResultData->uHandleRusult, pResultData->dwHandleID);
			}
			else
			{
				m_TCPSocket.SendData(pResultData->uIndex, &pOut->_result, sizeof(pOut->_result), MDM_GP_MONEY_CHANGE, ASS_GP_JEWEL2COIN, pResultData->uHandleRusult, pResultData->dwHandleID);
			}
			
		}break;
	case DTK_GP_USERINFO_FORGET_PWD:
		{
			m_TCPSocket.SendData(pResultData->uIndex, MDM_GP_USERINFO, ASS_GP_USERINFO_FORGET_PWD, pResultData->uHandleRusult, pResultData->dwHandleID);
		}break;
    case DTK_GP_LUCK_DRAW_CONFIG:
        {
            DL_O_HALL_LUCK_DRAW_CONFIG *pLuckConfig = (DL_O_HALL_LUCK_DRAW_CONFIG*)pResultData;
            m_TCPSocket.SendData(pResultData->uIndex, &pLuckConfig->_data, sizeof(pLuckConfig->_data), MDM_GP_LUCKDRAW, ASS_GP_LUCK_DRAW_CONFIG, pResultData->uHandleRusult, pResultData->dwHandleID);
        }break;
    case DTK_GP_LUCK_DRAW_DO:
        {
            DL_O_HALL_LUCK_DRAW_DO *pLuckDo = (DL_O_HALL_LUCK_DRAW_DO*)pResultData;
            m_TCPSocket.SendData(pResultData->uIndex, &pLuckDo->_data, sizeof(pLuckDo->_data), MDM_GP_LUCKDRAW, ASS_GP_LUCK_DRAW_DO, pResultData->uHandleRusult, pResultData->dwHandleID);
        }break;
    case DTK_GP_MOBILE_IS_MATCH:
        {
            DL_GP_O_MobileIsMatch* pMobileIsMatch = (DL_GP_O_MobileIsMatch *)pResultData;
            if (pMobileIsMatch->IsMatch.bIsMatch)
            {
                return m_pSmsServer->sendMessageRequest((void *)&pMobileIsMatch->SmsVCodeReq,sizeof(pMobileIsMatch->SmsVCodeReq),pResultData->uIndex,pResultData->dwHandleID);
            }
            else
            {
                //HandleResult = 2, 手机号不是绑定的手机号
                strcpy(pMobileIsMatch->SmsVCodeReq.szMobileNo, pMobileIsMatch->IsMatch.szMobileNum);
                m_TCPSocket.SendData(pResultData->uIndex, &pMobileIsMatch->SmsVCodeReq, sizeof(pMobileIsMatch->SmsVCodeReq), MDM_GP_SMS, ASS_GP_SMS_VCODE, 2, pResultData->dwHandleID);
            }

        }break;
	}
	return true;
}

/******************************************************************************************************/

extern CString GetAppPath(bool bFource=true);
//构造函数
CGameLogonManage::CGameLogonManage(void) : CBaseMainManageForWeb()
{
	m_nPort = GAME_LOGON_PORT;
	m_uMaxPeople = 300;
	m_userCustomLogo = NULL;
	m_bCustomFaceNeedCheck = false;
	m_CustomFacePath = "CustomFace\\";
	m_CustomFacePathUpload = "CustomFace\\";

	// 转账返利
	memset(&m_Rebate, 0, sizeof(m_Rebate));

	// IM服务模块
	m_imService = nullptr;
	
	
	// 银行服务模块
	m_pBankService = nullptr;
	m_pCreateRoom = nullptr;
	m_pHNClub = nullptr;
	m_pHNMail = nullptr;
	m_pHandleGMessage = nullptr;
	m_pContestServer = nullptr;
	m_pRegisterServer = nullptr;
	m_pLogonServer = nullptr;
	m_pUpdateUserInfo = nullptr;
	m_pSmsServer = nullptr;

    ::InitializeCriticalSection(&g_CriticalSection);

	m_pGameManageModule = new CGameManageModule();
}

//析构函数 
CGameLogonManage::~CGameLogonManage(void)
{
	//清理登陆
	for(int i=0; i<m_UserList.GetCount(); i++)
	{
		//原来是GetAt(i),现改为GetAt(0)，因为是要删除所有人的，所以一个个从头删除即可
		delete (LPUSER)m_UserList.GetAt(0);
		//m_UserList. .SetAt(i, NULL);
	}
	//不要原来的SetAt(i,NULL)，直接删除所有的项，否则仍会造成内存泄露
	m_UserList.RemoveAll();

	if (m_userCustomLogo)
	{
		delete m_userCustomLogo;
	}

    if(NULL != g_lpfn_Reslease)
    {
        g_lpfn_Reslease();
    }

	SafeDelete(m_pGameManageModule);

    ::DeleteCriticalSection(&g_CriticalSection);
}

// 停止服务
bool CGameLogonManage::OnStop()
{
	KillTimer(TID_UPDATE_LIST);
	KillTimer(IDT_CHECK_DATA_CONNECT);

	m_pGameManageModule->StoptService();

	return true;
}

bool CGameLogonManage::Init(ManageInfoStruct * pInitData, IDataBaseHandleService * pDataHandleService)
{
	__super::Init(pInitData, pDataHandleService);

	ManageInfoStruct Init;
	memset(&Init, 0, sizeof(ManageInfoStruct));
	m_pGameManageModule->InitService(pInitData);
	m_pGameManageModule->SetCGameLogonManage(this);

	return true;
}

bool CGameLogonManage::UnInit()
{
	__super::UnInit();
	m_pGameManageModule->UnInitService();

	return true;
}

// 获取信息函数
bool CGameLogonManage::PreInitParameter(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData)
{
	// 设置版本信息
	pKernelData->bMaxVer  = GAME_PLACE_MAX_VER;
	pKernelData->bLessVer = GAME_PLACE_LESS_VER;

	// 设置使用数据库
	pKernelData->bLogonDataBase    = TRUE;
	pKernelData->bNativeDataBase   = TRUE;
	pKernelData->bStartSQLDataBase = TRUE;

	// 设置使用网络
	pKernelData->bStartTCPSocket = TRUE;
	GetINIFile();
	pInitData->uListenPort = m_nPort;
	pInitData->uMaxPeople = m_uMaxPeople;
	pInitData->iSocketSecretKey = SECRET_KEY;
	
	// 上传头像初始化
	CreateDirectory(m_CustomFacePath,0);
	CreateDirectory(m_CustomFacePathUpload,0);
	m_userCustomLogo = new CUserCustomLogo((CWnd*)this);

	// 创建IM处理类
	m_imService = new CIMService((CWnd*)this);

	// 创建银行服务模块类
	m_pBankService = new CBankService4Z((CWnd*)this);
	// 创建比赛处理模块
	m_pContestServer = new CContestServer4Z(this);
	m_pRegisterServer = new CRegisteServer(this);
	m_pLogonServer = new CLogonServer(this);
	m_pUpdateUserInfo = new CUpdateUserInfo(this);
	m_pSmsServer = new CSmsServer(this);
	//创建开房服务模块类
	m_pCreateRoom = new CCreateRoom((CWnd*)this);
	m_pHandleGMessage = new CHandleGMessage(this);
	m_pHNClub = new CHNClub(this);
	m_pHNMail = new CHNMail(this);
	return true;
}

void CGameLogonManage::GetINIFile()
{
	CBcfFile f(CBcfFile::GetAppPath() + "HNGameLogon.BCF");
	CString strValue;

	if(!f.IsFileExist())
	{
		return;
	}

	m_uMaxPeople = f.GetKeyVal("SERVERSET", "MaxPeople", 300);
	m_nPort = f.GetKeyVal("SERVERSET", "MainServerPort", 3015);

	m_CustomFacePath = f.GetKeyVal("SERVERSET", "CustomFaceSave", "CustomFace");
	if(m_CustomFacePath.Right(1)!="\\")
	{
		m_CustomFacePath+"\\";
	}

	m_CustomFacePathUpload = f.GetKeyVal("SERVERSET", "CustomFaceUpload", "CustomFace");
	if(m_CustomFacePathUpload.Right(1)!="\\")
	{
		m_CustomFacePathUpload+"\\";
	}

	m_bCustomFaceNeedCheck = (bool)f.GetKeyVal("SERVERSET", "CustomFaceNeedCheck", 0);
	
	// 添加防止账户同时登陆，ZID
	m_ID = f.GetKeyVal("SERVERSET", "ID", 0);

	DWORD cfgHandle=cfgOpenFile("HNGameGate.BCF");
	m_szDownLoadURL = cfgGetValue(cfgHandle,"GateServer","PlayBackDownload","");
	cfgClose(cfgHandle);

}

// SOCKET 关闭
bool CGameLogonManage::OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, long int lConnectTime)
{
	HNLOG_M("OnSocketClose begin uSocketIndex:%d",uSocketIndex);
	m_TCPSocket.UnionRemovUserAll(uSocketIndex);
	int UserID = -1;
	for(int i = 0; i < m_UserList.GetCount(); i++)
	{
		LPUSER p = (LPUSER)m_UserList.GetAt(i);
		if(p == NULL)
		{
			continue;
		}

		if(p->iSocketIndex == uSocketIndex)
		{
			HNLOG_M("OnSocketClose uSocketIndex:%d UserID:%d",uSocketIndex,p->UserID);
			{
				IM_GP_I_USEROFFLINE imIUseroffline;
				imIUseroffline.dwUserID = p->UserID;
				imIUseroffline.iSocketID = -1;
				m_SQLDataManage.PushLine(&imIUseroffline.DataBaseHead, sizeof(imIUseroffline), DTK_GP_IM_OFFLINE, 0, 0);
				MSG_ZDistriInfo info;
				info.dwUserID = imIUseroffline.dwUserID;
				info.nZID = m_ID;
				m_pDistriManager->SendNormalData(&info, sizeof(info), MDM_ZD_PACKAGE, ASS_ZD_LOGON, HDC_ZD_DISCONNECT);

				MSG_MG_S_NETCUT_PLACE RoomMsg;
				RoomMsg.iUserID = p->UserID;
				m_pGameManageModule->m_LogonManage.m_TCPSocket.SendDataBatch(&RoomMsg,sizeof(MSG_MG_S_NETCUT_PLACE),MDM_MG_NETCUT_PLACE,ASS_MG_NETCUT_PLACE,0);
			}

			// 原来是只使用了SetAt(i,NULL)，但内存中还是会存在数据，造成内存泄露
			// 现在可以直接从队列中删除，因为已经取了其中的内容，所以可以先从队列中删除
			m_UserList.RemoveAt(i);
			UserID = p->UserID;
			IM_GP_I_USEROFFLINE imIUseroffline;
			imIUseroffline.dwUserID = UserID;
			imIUseroffline.iSocketID = -1;
			m_SQLDataManage.PushLine(&imIUseroffline.DataBaseHead,sizeof(imIUseroffline),DTK_GP_IM_OFFLINE,0,0);

			
			delete p;
			//m_UserList.SetAt(i, NULL);
			p = NULL;
			break;
		}
	}
	if (-1 == UserID)
	{
		IM_GP_I_USEROFFLINE imIUseroffline;
		imIUseroffline.dwUserID = 0;
		imIUseroffline.iSocketID = uSocketIndex;
		m_SQLDataManage.PushLine(&imIUseroffline.DataBaseHead,sizeof(imIUseroffline),DTK_GP_IM_OFFLINE,0,0);
	}

	HNLOG_M("OnSocketClose end uSocketIndex:%d",uSocketIndex);
	return true;
}


int CGameLogonManage::SendDataFail(UINT uUserID, void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, UINT bHandleCode, DWORD dwHandleID)
{
	if(bMainID == MDM_GP_PLAYER_2_GM && pData != NULL)
	{
		if(bAssistantID == ASS_GP_TALK_2_GM)
		{
			TCHAR szTmp[32]=TEXT("");
			GM_Broadcast_t msg;
			MSG_GP_S_Talk2GM_t *pMessage = (MSG_GP_S_Talk2GM_t *)pData;
			msg.iUserID = pMessage->iUserID;
			msg.iGMID = pMessage->iGMID;
			memcpy(szTmp, pMessage->szMsg, sizeof(TCHAR)*31);
			szTmp[31] = TEXT('\0');
			if (_tcslen(szTmp)>=31)
			{
				szTmp[28] = TEXT('.');
				szTmp[29] = TEXT('.');
				szTmp[30] = TEXT('.');
			}
			wsprintf(msg.szMsg, TEXT("管理员[%d]已经离线，消息[%s]发送不成功！"), pMessage->iGMID, szTmp);
			int iDstIndex = GetIndexByID(pMessage->iUserID);
			return m_TCPSocket.SendData(iDstIndex, &msg, sizeof(GM_Broadcast_t), MDM_GP_PLAYER_2_GM, ASS_GP_MSG_FROM_GM, 1, dwHandleID);
		}
		else if(bAssistantID == ASS_GP_MSG_FROM_GM)
		{
			TCHAR szTmp[32]=TEXT("");
			GM_Broadcast_t msg;
			GM_Broadcast_t *pMessage = (GM_Broadcast_t *)pData;
			msg.iUserID = pMessage->iUserID;
			msg.iGMID = pMessage->iGMID;
			memcpy(szTmp, pMessage->szMsg, sizeof(TCHAR)*31);
			szTmp[31] = TEXT('\0');
			if (_tcslen(szTmp)>=31)
			{
				szTmp[28] = TEXT('.');
				szTmp[29] = TEXT('.');
				szTmp[30] = TEXT('.');
			}
			wsprintf(msg.szMsg, TEXT("玩家[%d]已经离线，消息[%s]发送不成功！"), pMessage->iUserID, szTmp);
			int iDstIndex = GetIndexByID(pMessage->iGMID);
			return m_TCPSocket.SendData(iDstIndex, &msg, sizeof(GM_Broadcast_t), MDM_GP_PLAYER_2_GM, ASS_GP_TALK_2_GM, 1, dwHandleID);
		}
	}
	if (bMainID == MDM_GP_IM)
	{
		if (bAssistantID == ASS_IMC_ADDREQUEST)
		{
			MSG_IM_C_ADDREQUEST *msg=(MSG_IM_C_ADDREQUEST*)pData;
			if (msg != NULL) 
			{
				int iDstIndex = GetIndexByID(msg->dwUserID);
				m_TCPSocket.SendData(iDstIndex, pData, uBufLen,MDM_GP_IM,ASS_IMC_ADDREQUEST,2,0);
			}
		}
		if (bAssistantID == ASS_IMS_MSG)
		{
			MSG_IM_C_MSG *msg=(MSG_IM_C_MSG*)pData;
			if (msg != NULL) 
			{
				int iDstIndex = GetIndexByID(msg->dwUserID);
				m_TCPSocket.SendData(iDstIndex, pData, uBufLen,MDM_GP_IM,ASS_IMS_USERNOTONLINE,0,dwHandleID);
			}
		}
	}
	return 0;
}


/******************************************************************************************************/

//构造函数
CGameLogonModule::CGameLogonModule(void) 
{
	return;
}

//析构函数
CGameLogonModule::~CGameLogonModule(void)
{
	return;
}

//初始化函数 
bool CGameLogonModule::InitService(ManageInfoStruct * pInitData)
{
	try
	{
		return m_LogonManage.Init(pInitData,&m_DataBaseHandle);
	}
	catch (CAFCException * pException)	
	{ 
		pException->Delete();	
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}
	catch (...)	
	{ 
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}
	return false;
}

//卸载函数
bool CGameLogonModule::UnInitService()
{
	try
	{
		return m_LogonManage.UnInit();
	}
	catch (CAFCException * pException)	{ pException->Delete();TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);	}
	catch (...) {TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);}
	return false;
}

//开始函数 
bool CGameLogonModule::StartService(UINT &errCode)
{
	errCode=0;
	try
	{
		return  m_LogonManage.Start();
	}
	catch (CAFCException * pException)	
	{
		errCode = pException->GetErrorCode();
		pException->Delete();
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);	
	}
	catch (...) 
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}
	return false;
}

// 停止函数 
bool CGameLogonModule::StoptService()
{
	try
	{
		return m_LogonManage.Stop();
	}
	catch (CAFCException * pException)
	{
		pException->Delete();	TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}
	catch (...) 
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}
	return false;
}

// 删除函数
bool CGameLogonModule::DeleteService()
{
	try	
	{ 
		delete this; 
	}
	catch (...) 
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}
	return true;
}

/******************************************************************************************************/

