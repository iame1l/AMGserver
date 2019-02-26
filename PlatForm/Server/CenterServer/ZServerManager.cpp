/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "StdAfx.h"
#include "ZServerManager.h"
#include "gameplacemessage.h"


static char *G_GetAppPath()
{
	static char s_Path[MAX_PATH];
	bool s_bIsReady = false;
	if (!s_bIsReady)
	{
		ZeroMemory(s_Path,sizeof(s_Path));
		DWORD dwLength=GetModuleFileNameA(GetModuleHandle(NULL), s_Path, sizeof(s_Path));
		char *p = strrchr(s_Path, '\\');
		if(p != NULL)	*p = '\0';
		s_bIsReady = true;
	}
	return s_Path;
}

static void ProgramPrintf(int nLevel, char *pTag, const char *p, ...)
{
	/*nLevel = 
	0 == Print All
	1 == Debug
	2 == Warning
	3 == Info */
	//if(nLevel < PRN_LEVEL)
	//{
	//	return;
	//}
	char szTimeStr[32];
	char szDateStr[32];
	char szFilename[256];
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	GetTimeFormatA( LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &sysTime, "HH:mm:ss", szTimeStr, 32);
	GetDateFormatA( LOCALE_USER_DEFAULT, LOCALE_USE_CP_ACP, &sysTime, "yyyy-MM-dd", szDateStr, 32);
	wsprintf( szFilename, TEXT("%s\\Log\\%s_HN%s.txt"), G_GetAppPath(), szDateStr, "ZServerManager");
	va_list arg;
	va_start( arg, p );
	FILE *fp = NULL;
	fopen_s(&fp, szFilename, "a" );
	if (NULL == fp)
	{
		return;
	}
	fprintf(fp,"[%s %s]--", szDateStr, szTimeStr);
	vfprintf(fp, p, arg );
	fprintf(fp,"\n");
	fclose(fp);
}


CMainserverList::CMainserverList()
{
	m_strHostIP = "";
	PHOSTENT hostinfo;
	char hostname[MAX_PATH];
	if (gethostname(hostname, sizeof(hostname)) == 0)	{//获取主机名
		if ((hostinfo = gethostbyname(hostname)) != NULL)//确认
		{
			m_strHostIP = inet_ntoa(*(struct in_addr *)*hostinfo->h_addr_list);//转大端
		}
	}
}

CMainserverList::~CMainserverList()
{
	clear();
}

void CMainserverList::clear()
{
	Lock();
	m_List.clear();//服务器列表清除
	UnLock();
}

//负载均衡(需要ip+端口)
bool CMainserverList::RandAServer(char* ipaddr,UINT& port)
{
	bool bRet = false;
	Lock();
	if(!m_List.empty())
	{		
		__int64 _minCnt = MAXDWORD;
		vector<ZSvrNode_st>::iterator _minit = m_List.begin();

		for(vector<ZSvrNode_st>::iterator it = m_List.begin(); it != m_List.end(); it++)
		{//遍历
			if (it->onlineUserSocket.size() < _minCnt)
			{//找人数最小的
				_minCnt = (int)it->onlineUserSocket.size();
				_minit = it;
			}
		}

		CString ip = _minit->szIPAddr ;
		port = _minit->lPort;
		memcpy(ipaddr, ip.GetBuffer(ip.GetLength ()), 20);
		bRet = true;
	}
	UnLock();
	return bRet;
}

// 根据SocketID找相应服务器
ZSvrNode_st* CMainserverList::GetServerBySocket(UINT uSocket)
{
	vector<ZSvrNode_st>::iterator it = m_List.begin();
	for(;it != m_List.end(); it++)
	{
		if(it->uSocket == uSocket)
		{
			return &(*it);
		}
	}
	return NULL;
}

// 根据UserId找相应服务器
ZSvrNode_st* CMainserverList::find(DWORD dUserId)
{
	vector<ZSvrNode_st>::iterator it = m_List.begin();
	for(;it != m_List.end(); it++)
	{
		if(it->onlineUserSocket.find(dUserId) != it->onlineUserSocket.end())
		{
			return &(*it);
		}
	}
	return NULL;
}

// 添加一个Z服务器
bool CMainserverList::InsertServer(CString szIP, long lPort, int nZid, UINT uSocket)
{
	// 检测服务器IP是否为空
	if (m_strHostIP.IsEmpty() || szIP.IsEmpty())
	{
		return false;
	}

	// 比较下IP地址是否为本机地址
	if (strcmp(m_strHostIP, szIP))
	{
		return false;
	}

	ZSvrNode_st node;
	node.szIPAddr = szIP;
	node.lPort    = lPort;
	node.nZid     = nZid;
	node.uSocket  = uSocket;
	m_List.push_back(node);
	//写日志用的
	ProgramPrintf(1, "Distri", "ZSvrList, InsertServer(szIP=%s, lPort=%d, nZid=%d, uSocket=%d)", szIP, lPort, nZid, uSocket);

	return true;
}

// 删除一个Z服务器
bool CMainserverList::RemoveServer(UINT uSocket)
{
	bool bSucc = false;
	vector<ZSvrNode_st>::iterator it = m_List.begin();
	for(;it != m_List.end(); it++)
	{
		if(it->uSocket == uSocket)
		{
			bSucc = true;
			m_List.erase(it);
			ProgramPrintf(1, "Distri", "ZSvrList, RemoveServer(uSocket=%d)", uSocket);
			break;
		}
	}
	return bSucc;
}

// 调试函数
void CMainserverList::PrintDebug()
{
	ProgramPrintf(1, "Distri", "begin 用户列表");
	vector<ZSvrNode_st>::iterator it = m_List.begin();
	for(;it != m_List.end(); it++)
	{
		ProgramPrintf(1, "Distri", "服务器socket = %d, zid = %d", it->uSocket, it->nZid);
		for(set<UINT>::iterator it_1 = it->onlineUserSocket.begin(); it_1 != it->onlineUserSocket.end(); it_1++)
		{
			ProgramPrintf(1, "Distri", "    用户id=%d", *it_1);
		}
	}
	ProgramPrintf(1, "Distri", "end 用户列表");
}
/******************************************************************************************************/


CZServerManager::CZServerManager(void)
{
}

CZServerManager::~CZServerManager(void)
{
}

// 获取信息函数
bool CZServerManager::PreInitParameter(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData)
{
	// 设置使用数据库
	pKernelData->bLogonDataBase    = FALSE; //TRUE;
	pKernelData->bNativeDataBase   = FALSE; //TRUE;
	pKernelData->bStartSQLDataBase = FALSE; //TRUE;

	// 设置使用网络
	pKernelData->bStartTCPSocket = TRUE;
	pInitData->uListenPort       = CENTER_DISTRI_PORT;
	pInitData->uMaxPeople        = 300;

	return true;
}

// SOCKET 数据读取，请求游戏全局参数
bool CZServerManager::OnSocketRead(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	if (pNetHead->bMainID == MDM_ZD_PACKAGE)
	{
		//Z服务器信息
		if(pNetHead->bAssistantID == ASS_ZD_CONNECT)
		{
			sockaddr_in socket;
			CString szIP = m_TCPSocket.GetServerInfo(uIndex).c_str();
			TZServerInfo* _P = (TZServerInfo*)pData;
			if (_P != NULL)
			{
				m_MainserverList.InsertServer(szIP, _P->iServerPort, _P->iZid, uIndex);
			}
		}
		else if(pNetHead->bAssistantID == ASS_ZD_LOGON)	//用户登陆
		{
			MSG_ZDistriInfo	*pInfo = (MSG_ZDistriInfo*)pData;
			if(pNetHead->bHandleCode == HDC_ZD_CONNECT)
			{
				ProgramPrintf(1, "Distri", "logon user=%d, socket=%d at zid=%d", pInfo->dwUserID, uIndex, pInfo->nZID);

				ZSvrNode_st* _tmp = m_MainserverList.find(pInfo->dwUserID);
				ZSvrNode_st* _p   = m_MainserverList.GetServerBySocket(uIndex);
				if (_tmp != NULL)
				{
					if (_tmp != _p)
					{
						m_TCPSocket.SendData(_tmp->uSocket, &pInfo->dwUserID, sizeof(pInfo->dwUserID), MDM_ZD_PACKAGE, ASS_ZD_LOGON, HDC_ZD_KICK, 0);
						_tmp->onlineUserSocket.erase(pInfo->dwUserID);
					}
				}
				if (_p != NULL && _p->onlineUserSocket.find(pInfo->dwUserID) == _p->onlineUserSocket.end())
				{
					_p->onlineUserSocket.insert(pInfo->dwUserID);
				}
				m_MainserverList.PrintDebug();	
			}
			else if(pNetHead->bHandleCode == HDC_ZD_DISCONNECT)
			{
				ZSvrNode_st* _p = m_MainserverList.GetServerBySocket(uIndex);
				if (_p != NULL)
				{
					if (_p->onlineUserSocket.find(pInfo->dwUserID) != _p->onlineUserSocket.end())
					{
						_p->onlineUserSocket.erase(pInfo->dwUserID);
					}
				}
				m_MainserverList.PrintDebug();
			}
		}
		else if(pNetHead->bAssistantID == ASS_ZD_BATCH)
		{
			m_TCPSocket.SendDataBatch(pData, uSize, pNetHead->bMainID, pNetHead->bAssistantID, pNetHead->bHandleCode);
		}
		else if(pNetHead->bAssistantID == ASS_ZD_SINGLE)
		{
			UINT uUserID;
			memcpy(&uUserID, &((char*)pData)[0], sizeof(uUserID));

			ZSvrNode_st* _p = m_MainserverList.find(uUserID);
			if (_p != NULL)
			{
				NetMessageHead *ph = (NetMessageHead *)pData;

				ProgramPrintf(1, "Distri", "SOCKET pair begin");
				int re = m_TCPSocket.SendData(_p->uSocket, pData, uSize, pNetHead->bMainID, pNetHead->bAssistantID, pNetHead->bHandleCode, 0);
				ProgramPrintf(1, "Distri", "SOCKET pair : User(%d) vs Socket(%d), %d, %d, %d, %d, result = %d", uUserID, _p->uSocket, ph->bMainID, ph->bAssistantID, pNetHead->bHandleCode, pNetHead->bReserve, re);
			}
			else
			{
				ProgramPrintf(1, "Distri", "SINGLE data user(id=%d) NOT found.", uUserID);
				m_TCPSocket.SendData(uIndex, pData, uSize, MDM_ZD_PACKAGE, ASS_ZD_SINGLE, HDC_ZD_FAIL, 0);
			}

		}
		return true;
	}
	return false;
}

// SOCKET 关闭
bool CZServerManager::OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, long int lConnectTime)
{
	m_MainserverList.RemoveServer(uSocketIndex);
	return true;
}
