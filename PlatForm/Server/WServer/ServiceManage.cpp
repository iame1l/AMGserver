/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "StdAfx.h"
#include <SYS/Stat.h>
#include "ServiceManage.h"
#include "MainFrm.h"
#include "MD5.h"
#include "resource.h"

#include"../Common/CommonUse.h"



//构造函数
CServiceManage::CServiceManage(void)
{
	m_szError[0]=0;
	m_pIMessage=NULL;
	m_pEngineSink = NULL;
	tinyxml2::XMLDocument doc;
	if (tinyxml2::XMLError::XML_SUCCESS != doc.LoadFile("HNGameLocal.xml"))
	{
		AfxMessageBox("Load HNGameLocal.xml Error-2",0);
		m_ServiceName	= "HNGame";
		m_sServerIP		= "127.0.0.1";	//服务器IP
		m_sAccount		= "sa";			//账户
		m_sPassword		= "sa";			//密码
		m_szDatabase	= "hn2.0";		//数据库名
		m_nPort			= 1433;			//数据库端口
		m_sDetectTable	= TEXT("");		//记录的表名-(基本没用)
		return;
	}

	tinyxml2::XMLElement* element1 = doc.FirstChildElement("SQLSERVER");
	if (NULL == element1)
	{
		AfxMessageBox("Load HNGameLocal.xml Error-2-1",0);
		return;
	}
	for (const tinyxml2::XMLElement* xml_SqlServer = element1->FirstChildElement(); xml_SqlServer; xml_SqlServer = xml_SqlServer->NextSiblingElement()) 
	{
		//数据库IP地址
		if (!strcmp(xml_SqlServer->Value(), "DBSERVER")) 
		{
			m_sServerIP = xml_SqlServer->Attribute("key");
		}
		else if (!strcmp(xml_SqlServer->Value(), "DBPORT")) 
		{
			//端口
			CString sPort = xml_SqlServer->Attribute("key");
			m_nPort = _ttoi(sPort);
		}
		else if (!strcmp(xml_SqlServer->Value(), "DATABASE")) 
		{
			//数据库名
			m_szDatabase = xml_SqlServer->Attribute("key");
		}

		else if (!strcmp(xml_SqlServer->Value(), "DBACCOUNT")) 
		{
			//数据库登录帐号
			m_sAccount = xml_SqlServer->Attribute("key");
		}
		else if (!strcmp(xml_SqlServer->Value(), "DBPASSWORD")) 
		{
			//数据库登录密码
			m_sPassword = xml_SqlServer->Attribute("key");
		}
	}
	m_sDetectTable=TEXT("");


	element1 = doc.FirstChildElement("SERVERSET");
	if (NULL == element1)
	{
		AfxMessageBox("Load HNGameLocal.xml Error-2-2",0);
		return;
	}
	for (const tinyxml2::XMLElement* xml_SqlServer = element1->FirstChildElement(); xml_SqlServer; xml_SqlServer = xml_SqlServer->NextSiblingElement()) 
	{
		//数据库IP地址
		if (!strcmp(xml_SqlServer->Value(), "ServiceName")) 
		{
			m_ServiceName = xml_SqlServer->Attribute("key");
		}
	}
}

/********************************************************************************************************/

//DLL 接口函数定义
typedef IModuleManageService * (CreateServiceInterface)(UINT uVer);
typedef BOOL (GetServiceInfo)(ServerDllInfoStruct * pServiceInfo, UINT uVer);




//登陆系统
bool CServiceManage::LogonSystem(const TCHAR * szUserName, const TCHAR * szScrPass)
{
	return AfxGetMainWnd()->PostMessage(WM_COMMAND,IDM_GET_ROOM_LIST,0);
}

/********************************************************************************************************/

void CServiceManage::StartAllGames(CListCtrl * pListCtrl)
{
	for(int i=0;i<m_ComRoomInfo.GetSize();i++)
	{
	   SC_GameRoomInitStruct * p=(SC_GameRoomInitStruct *)m_ComRoomInfo.GetAt(i);
	   if(!p)continue;

       // 指定哪些游戏是可用的
       if(FALSE == m_GameSelecter.GameIsUsable(p->ServiceInfo.uNameID))
       {
           p->hDllInstance = NULL;
           p->hProcessHandle = NULL;
           memset(&(p->InitInfo), 0, sizeof(p->InitInfo));
           continue;
       }

		if (true)
	   {
	   //加载各组件并得到两个接口函数地址
	   if(!bGetServiceInfo(p))
	   {
		   CString strMessage;
		   strMessage.Format(TEXT("房间对应的库%s不存在：%s"),p->ServiceInfo.szDLLFileName,g_Service.GetLastErrorMsg());
		   if (m_pIMessage!=NULL) m_pIMessage->ShowMessage(strMessage);
		   continue;
	   }
	   if(!StartGameRoom(p))continue;
	   }
	   else
	   {
		   CString s=p->ServiceInfo.szGameName;
		   if(s.Left(7)!="未认证_")
		   {
			   s="未认证_"+s;
			   if(s.GetLength()>60)
				   s=s.Left(60);
		   }	
		   strcpy(p->ServiceInfo.szGameName,s);
	   }
	}
	if (m_pIMessage!=NULL) m_pIMessage->ShowMessage(TEXT("启动所有组件的子实例服务完成!"));
}


//启动游戏房间
bool CServiceManage::bGetServiceInfo(SC_GameRoomInitStruct * pComRoomInfo)
{
	try
	{
		if ((pComRoomInfo->hDllInstance!=NULL)||(pComRoomInfo->pIManageService!=NULL)) return true;

		//判断文件
		struct stat FileStat;
		if (stat(pComRoomInfo->ServiceInfo.szDLLFileName,&FileStat)!=0) 
			return false;

		//加载组件
		pComRoomInfo->hDllInstance=AfxLoadLibrary(pComRoomInfo->ServiceInfo.szDLLFileName);
		int _err = GetLastError();
		if (pComRoomInfo->hDllInstance==NULL) 
		{
			return false;
		}

		//获取管理接口
		CreateServiceInterface * pCreateServiceInterface=(CreateServiceInterface *)GetProcAddress(pComRoomInfo->hDllInstance,TEXT("CreateServiceInterface"));
		if (pCreateServiceInterface==NULL) 
			return false;//throw TEXT("非法组件");
		pComRoomInfo->pIManageService=pCreateServiceInterface(DEV_LIB_VER);
		if (pComRoomInfo->pIManageService==NULL)
			return false;//throw TEXT("组件管理接口获取失败，可能是资源不足");


		GetServiceInfo * pGetServiceInfo=(GetServiceInfo *)GetProcAddress(pComRoomInfo->hDllInstance,TEXT("GetServiceInfo"));
		if (pGetServiceInfo==NULL) 
			return false;//throw TEXT("非法组件");

        // 指定哪些游戏是可用的
        ServerDllInfoStruct ServiceDllInfo;
        if(FALSE == pGetServiceInfo(&ServiceDllInfo,DEV_LIB_VER)) return false;
        if(FALSE == m_GameSelecter.GameIsUsable(ServiceDllInfo.uNameID)) return false;

	}
	catch (...)
	{
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		return false;
	}

	return true;
}

//启动游戏房间
bool CServiceManage::StartGameRoom(SC_GameRoomInitStruct * pComRoomInfo)
{
	try
	{
	
		UINT errCode=0;
		//启动组件
		if (pComRoomInfo->pIManageService->InitService(&pComRoomInfo->InitInfo)==false) throw TEXT("组件初始化错误");
		if (pComRoomInfo->pIManageService->StartService(errCode)==false) throw TEXT("组件启动错误");
		
		return true;
	}
	catch (TCHAR * szError) { lstrcpy(m_szError,szError); TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);}
	catch (CAFCException * pException)
	{ TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		pException->GetErrorMessage(m_szError,sizeof(m_szError));
		pException->Delete();	
	}
	catch (...)	{ lstrcpy(m_szError,TEXT("发生未知异常错误")); TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);}

	//清理资源
	if (pComRoomInfo->pIManageService!=NULL)
	{
		pComRoomInfo->pIManageService->StoptService();
		pComRoomInfo->pIManageService->UnInitService();
		pComRoomInfo->pIManageService->DeleteService();
		pComRoomInfo->pIManageService=NULL;
	}
	if (pComRoomInfo->hDllInstance!=NULL)
	{
		AfxFreeLibrary(pComRoomInfo->hDllInstance);
		pComRoomInfo->hDllInstance=NULL;
	}

	return false;
}

//更新服务列表,启动时调用,将重起所有游戏
bool CServiceManage::UpdateServiceInfo(CListCtrl * pListCtrl)
{
	//读数据库得到房间信息
	GetRoomListInfo();
	
	//利用接口函数启动各游戏
    StartAllGames(pListCtrl);
	//更新所有房间列表
	UpdateRoomListInfo(pListCtrl);
	return false;
}

//构造函数
CServiceInfo::CServiceInfo(void)
{
	m_uLogonTimes=0;
	m_uModelDlgCount=0;
	m_uSystemState=STS_NO_LOGON;
	memset(m_szName,0,sizeof(m_szName));
	memset(m_szMD5Pass,0,sizeof(m_szMD5Pass));
	memset(m_szServiceIP,0,sizeof(m_szServiceIP));
	memset(m_szServerGUID,0,sizeof(m_szServerGUID));
	memset(m_szSQLName,0,sizeof(m_szSQLName));
	memset(m_szSQLPass,0,sizeof(m_szSQLPass));
	memset(m_szSQLAddr,0,sizeof(m_szSQLAddr));
	memset(m_szLogonAddr,0,sizeof(m_szLogonAddr));

}

//析构函数
CServiceInfo::~CServiceInfo(void)
{
	SaveData();
}

//初始化函数
bool CServiceInfo::LoadData()
{
	CString strBuffer;

	//读取登陆名字
	strBuffer=AfxGetApp()->GetProfileString(TEXT("ServerInfo"),TEXT("Name"),TEXT("Administrator"));
	if (strBuffer.GetLength()>20) strBuffer.GetBufferSetLength(20);
	lstrcpy(m_szName,strBuffer);

	//获取登陆地址
	strBuffer=AfxGetApp()->GetProfileString(TEXT("ServerInfo"),TEXT("LogonAddr"),TEXT("127.0.0.1"));
	if (strBuffer.GetLength()>(sizeof(m_szLogonAddr)-1)) strBuffer.GetBufferSetLength((sizeof(m_szLogonAddr)-1));
	lstrcpy(m_szLogonAddr,strBuffer);

	//读取 GUID
	strBuffer=AfxGetApp()->GetProfileString(TEXT("ServerInfo"),TEXT("GUID"));
	if (strBuffer.GetLength()>36) strBuffer.GetBufferSetLength(36);
	lstrcpy(m_szServerGUID,strBuffer);

	return true;
}

//保存数据
bool CServiceInfo::SaveData()
{
	if (m_szName[0]!=0) AfxGetApp()->WriteProfileString(TEXT("ServerInfo"),TEXT("Name"),m_szName);
	if (m_szServerGUID[0]!=0) AfxGetApp()->WriteProfileString(TEXT("ServerInfo"),TEXT("GUID"),m_szServerGUID);
	if (m_szLogonAddr[0]!=0) AfxGetApp()->WriteProfileString(TEXT("ServerInfo"),TEXT("LogonAddr"),m_szLogonAddr);
	return true;
}

//是否登陆系统
bool CServiceInfo::IsLogonSystem()
{
	bool bLogon=((m_uSystemState==STS_SYSTEM_LOCK)||(m_uSystemState==STS_LOGON_FINISH));
	return bLogon;
}

//是否锁定系统
bool CServiceInfo::IsLockSystem()
{
	bool bLock=(m_uSystemState==STS_SYSTEM_LOCK);
	return bLock;
}

//获取登陆次数
UINT CServiceInfo::GetLogonTimes() 
{ 
	return m_uLogonTimes; 
}

/********************************************************************************************************/


//析构函数
CServiceManage::~CServiceManage(void)
{
	//清理资源
	for (INT_PTR i=0;i<m_ServiceInfo.GetCount();i++)
	{
		SC_ServiceInfoStruct * pServiceInfo=(SC_ServiceInfoStruct *)(SC_ServiceInfoStruct *)m_ServiceInfo.GetAt(i);
		if (pServiceInfo!=NULL)
		{
			delete pServiceInfo;
			m_ServiceInfo.SetAt(i,NULL);
		}
	}

	for (INT_PTR i=0;i<m_ComRoomInfo.GetCount();i++)
	{
		SC_GameRoomInitStruct * pComRoomInfo=(SC_GameRoomInitStruct *)m_ComRoomInfo.GetAt(i);
		if (pComRoomInfo!=NULL)
		{
			if (pComRoomInfo->pIManageService!=NULL)
			{
				pComRoomInfo->pIManageService->StopAllTimer();
			}
		}
	}

	for (INT_PTR i=0;i<m_ComRoomInfo.GetCount();i++)
	{
		SC_GameRoomInitStruct * pComRoomInfo=(SC_GameRoomInitStruct *)m_ComRoomInfo.GetAt(i);
		if (pComRoomInfo!=NULL)
		{
			if (pComRoomInfo->pIManageService!=NULL)
			{
				pComRoomInfo->pIManageService->StoptService();
				pComRoomInfo->pIManageService->UnInitService();
				pComRoomInfo->pIManageService->DeleteService();
				pComRoomInfo->pIManageService=NULL;

				ClearRoomTWRecode(pComRoomInfo->InitInfo.uRoomID);
			}
			if (pComRoomInfo->hDllInstance!=NULL) 
			{
				AfxFreeLibrary(pComRoomInfo->hDllInstance);
				pComRoomInfo->hDllInstance=NULL;
			}
			delete pComRoomInfo;
			m_ComRoomInfo.SetAt(i,NULL);
		}
	}

	return;
}

//连接数据库是否成功
bool CServiceManage::sqlAddConnection()
{
	if (nullptr != m_pEngineSink)
	{
		dklSQLEngineDelete(m_pEngineSink);
		m_pEngineSink = nullptr;
	}
	CString szPort;
	szPort.Format(TEXT(",%d"), m_nPort);
	CString szServer;
	szServer = m_sServerIP + szPort;
	DBCreateParam createParam;
	_tcscpy_s(createParam.server, szServer);
	_tcscpy_s(createParam.db, m_szDatabase);
	_tcscpy_s(createParam.uid, m_sAccount);
	_tcscpy_s(createParam.pwd, m_sPassword);

	int code = dklSQLEngineCreate(createParam, &m_pEngineSink);
	if(code!=0)
	{
		m_pEngineSink=nullptr;
		return false;
	}

	if (nullptr == m_pEngineSink)
	{
		return false;
	}

	return true;
}


//设置信息接口
bool CServiceManage::SetMessageInterface(IMessageShow * pIMessage)
{
	m_pIMessage=pIMessage; 
	return true;
}
//更新组件列表
bool CServiceManage::UpdateComDLLList(CListCtrl * pListCtrl)
{
	if (false == sqlAddConnection())
	{
		return false;
	}

	bool bres=m_pEngineSink->setStoredProc("SP_GetGameServerList",true);
	if(0!=bres)
	{
		return 0;
	}

	if(0 != m_pEngineSink->execStoredProc())
	{
		m_pEngineSink->closeRecord();
		return 0;
	}
	//写入数据前应清空记录
	pListCtrl->DeleteAllItems();
	m_ServiceInfo.RemoveAll();

	//读取资料
	TCHAR szBuffer[50];

	while(!m_pEngineSink->adoEndOfFile())
	{
		//定义变量
		SC_ServiceInfoStruct * pServiceInfo=new SC_ServiceInfoStruct;
		memset(pServiceInfo,0,sizeof(SC_ServiceInfoStruct));
		m_ServiceInfo.Add(pServiceInfo);

		//读取数据
		m_pEngineSink->getValue(TEXT("ServerInfoID"),&pServiceInfo->uServiceID);
		m_pEngineSink->getValue(TEXT("NameID"),&pServiceInfo->ServiceInfo.uNameID);
		m_pEngineSink->getValue(TEXT("DeskPeople"),&pServiceInfo->ServiceInfo.uDeskPeople);
		m_pEngineSink->getValue(TEXT("SupportType"),&pServiceInfo->ServiceInfo.uSupportType);
		m_pEngineSink->getValue(TEXT("ServiceVer"),&pServiceInfo->ServiceInfo.uServiceVer);
		m_pEngineSink->getValue(TEXT("GameName"),pServiceInfo->ServiceInfo.szGameName,sizeof(pServiceInfo->ServiceInfo.szGameName));
		m_pEngineSink->getValue(TEXT("GameTable"),pServiceInfo->ServiceInfo.szGameTable,sizeof(pServiceInfo->ServiceInfo.szGameTable));
		m_pEngineSink->getValue(TEXT("DLLFileName"),pServiceInfo->ServiceInfo.szDLLFileName,sizeof(pServiceInfo->ServiceInfo.szDLLFileName));

        // PengJiLin, 2011-4-2, 指定哪些游戏是可用的
        if(FALSE == m_GameSelecter.GameIsUsable(pServiceInfo->ServiceInfo.uNameID))
        {
            memset(pServiceInfo,0,sizeof(SC_ServiceInfoStruct));
            m_pEngineSink->moveNext();
            continue;
        }

		//写入数据
		struct stat FileStat;
		int iItem=pListCtrl->InsertItem(pListCtrl->GetItemCount(),ltoa(pServiceInfo->uServiceID,szBuffer,10));
		pListCtrl->SetItem(iItem,1,1,pServiceInfo->ServiceInfo.szGameName,0,0,0,0);
		if (stat(pServiceInfo->ServiceInfo.szDLLFileName,&FileStat)==0) 
		{
			pListCtrl->SetItem(iItem,2,1,TEXT("已安装"),0,0,0,0);
			pServiceInfo->uServiceStation = 1;
		}
		else 
		{
			pListCtrl->SetItem(iItem,2,1,TEXT("未安装"),0,0,0,0);
			pServiceInfo->uServiceStation = 0;
		}
		pListCtrl->SetItem(iItem,3,1,ltoa(pServiceInfo->ServiceInfo.uServiceVer,szBuffer,10),0,0,0,0);
		pListCtrl->SetItem(iItem,4,1,ltoa(pServiceInfo->ServiceInfo.uDeskPeople,szBuffer,10),0,0,0,0);
		pListCtrl->SetItem(iItem,5,1,pServiceInfo->ServiceInfo.szGameTable,0,0,0,0);
		pListCtrl->SetItem(iItem,6,1,pServiceInfo->ServiceInfo.szDLLFileName,0,0,0,0);
		pListCtrl->SetItemData(iItem,(DWORD_PTR)pServiceInfo);
		m_pEngineSink->moveNext();
	}

    // 清 TWLoginRecord 相关内容，避免崩溃后无法登陆
    m_pEngineSink->closeRecord();

	bres=m_pEngineSink->setStoredProc("SP_ClearTWLoginRecord",true);
	if(0!=bres)
	{
		return 0;
	}

    
	m_pEngineSink->addInputParameter("@ServiceName",m_ServiceName.GetLength(),m_ServiceName);
	if(0 != m_pEngineSink->execStoredProc())
	{
		m_pEngineSink->closeRecord();
		return 0;
	}
	m_pEngineSink->closeRecord();
    

	dklSQLEngineDelete(m_pEngineSink);
	m_pEngineSink = nullptr;

	return true;

}


//初始化函数
bool CServiceManage::Init()
{
	return true;
}



//停止游戏房间
bool CServiceManage::StopGameRoom(SC_GameRoomInitStruct * pComRoomInfo)
{
	try
	{
		//判断状态
		if ((pComRoomInfo->hDllInstance==NULL)||(pComRoomInfo->pIManageService==NULL)) return true;

		//停止组件
		if (pComRoomInfo->pIManageService->StoptService()==false) throw TEXT("组件停止错误");
		if (pComRoomInfo->pIManageService->UnInitService()==false) throw TEXT("组件卸载错误");
		if (pComRoomInfo->pIManageService->DeleteService()==false) throw TEXT("组件清理错误");
		pComRoomInfo->pIManageService=NULL;

		//卸载组件
		AfxFreeLibrary(pComRoomInfo->hDllInstance);
		pComRoomInfo->hDllInstance=NULL;

		ClearRoomTWRecode(pComRoomInfo->InitInfo.uRoomID);

		return true;
	}
	catch (TCHAR * szError) { lstrcpy(m_szError,szError);TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__); }
	catch (...)	{ lstrcpy(m_szError,TEXT("发生未知异常错误")); TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);}

	//清理资源
	if (pComRoomInfo->pIManageService!=NULL)
	{
		pComRoomInfo->pIManageService->StoptService();
		pComRoomInfo->pIManageService->UnInitService();
		pComRoomInfo->pIManageService->DeleteService();
		pComRoomInfo->pIManageService=NULL;

		ClearRoomTWRecode(pComRoomInfo->InitInfo.uRoomID);
	}
	if (pComRoomInfo->hDllInstance!=NULL)
	{
		AfxFreeLibrary(pComRoomInfo->hDllInstance);
		pComRoomInfo->hDllInstance=NULL;
	}

	return false;
}

/********************************************************************************************************/

//网络读取消息
bool CServiceManage::OnSocketReadEvent(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	switch (pNetHead->bMainID)
	{
	case MDM_CONNECT:		//连接信息
		{
			switch (pNetHead->bAssistantID)
			{
			case ASS_NET_TEST:			//网络检测
				{
					if (uDataSize!=0) return false;
					pClientSocket->SendData(MDM_CONNECT,ASS_NET_TEST,0);
					return true;
				}
			case ASS_CONNECT_SUCCESS:	//连接成功
				{
					MSG_S_ConnectSuccess * pVerInfo=(MSG_S_ConnectSuccess *)pNetData;

					//版本效验
					if (GAME_MAX_VER<pVerInfo->bLessVer) 
					{
						if (m_pIMessage!=NULL) m_pIMessage->ShowMessage(TEXT("游戏控制器版本太低了，无法继续使用..."));
						pClientSocket->CloseSocket(false);
						return false;
					}

					//发送登陆信息
					MSG_SL_S_LogonByNameInfo MSGLogonInfo;
					lstrcpy(MSGLogonInfo.szName,m_Info.m_szName);
					lstrcpy(MSGLogonInfo.szMD5Pass,m_Info.m_szMD5Pass);
					lstrcpy(MSGLogonInfo.szServerGUID,m_Info.m_szServerGUID);
					pClientSocket->SendData(&MSGLogonInfo,sizeof(MSGLogonInfo),MDM_SL_SERVICE,ASS_SL_LOGON_BY_NAME,0);

					return true;
				}
			}
			return false;
		}
	case MDM_SL_SERVICE:	//服务登陆部分
		{
			switch (pNetHead->bAssistantID)
			{
			case ASS_SL_LOGON_SUCCESS:	//登陆成功
				{
					//效验数据
					if (uDataSize!=sizeof(MSG_SL_R_ConnectInfo)) return false;
					MSG_SL_R_ConnectInfo * pConnectInfo=(MSG_SL_R_ConnectInfo *)pNetData;

					//处理数据
					pClientSocket->CloseSocket(false);
					m_Info.m_uLogonTimes++;
					m_Info.m_uSystemState=STS_LOGON_FINISH;
					lstrcpy(m_Info.m_szSQLName,pConnectInfo->szSQLName);
					lstrcpy(m_Info.m_szSQLPass,pConnectInfo->szSQLPass);
					lstrcpy(m_Info.m_szSQLAddr,pConnectInfo->szSQLAddr);
					lstrcpy(m_Info.m_szServiceIP,pConnectInfo->szServiceIP);
					if (m_pIMessage!=NULL) m_pIMessage->ShowMessage(TEXT("用户信息效验成功，正在读取大厅信息..."));
					AfxGetMainWnd()->PostMessage(WM_COMMAND,IDM_GET_ROOM_LIST,0);

					return true;
				}
			case ASS_SL_LOGON_ERROR:	//登陆失败
				{
					//效验数据
					if (uDataSize!=0) return false;

					//处理数据
					CString strMessage=TEXT("登陆发生未知错误，请检查系统配置...");
					switch (pNetHead->bHandleCode)
					{
					case ERR_SL_GUID_ERROR: 
						{  
							strMessage=TEXT("系统授权号错误，登陆失败...");
							break;	
						}
					case ERR_SL_LOGON_IP_ERROR: 
						{ 
							strMessage=TEXT("此授权号不能在此服务器上登陆，登陆失败...");
							break;	
						}
					case ERR_SL_LOGON_NAME_ERROR: 
						{	
							strMessage=TEXT("登陆名字错误，登陆失败...");
							break;	
						}
					case ERR_SL_LOGON_NAME_LIMIT: 
						{	
							strMessage=TEXT("登陆名已经被禁用，登陆失败...");
							break;	
						}
					case ERR_SL_LOGON_PASS_ERROR: 
						{ 
							strMessage=TEXT("登陆密码错误，登陆失败...");
							break; 
						}
					}
					
					//显示错误信息
					pClientSocket->CloseSocket(false);
					m_Info.m_uSystemState=STS_NO_LOGON;
					if (m_pIMessage!=NULL) m_pIMessage->ShowMessage(strMessage);
					AfxGetMainWnd()->PostMessage(WM_COMMAND,IDM_LOGON_SYSTEM,0);

					return true;
				}
			}
			return false;
		}
	}
	return false;
}

//网络连接消息
bool CServiceManage::OnSocketConnectEvent(UINT uErrorCode, CTCPClientSocket * pClientSocket)
{
	if (uErrorCode!=0)
	{
		if (m_Info.m_uSystemState==STS_LOGON_ING) m_Info.m_uSystemState=STS_NO_LOGON;
		if (m_pIMessage!=NULL) m_pIMessage->ShowMessage(TEXT("登陆服务器没有启动，请尝试其他登陆服务器..."));
		AfxGetMainWnd()->PostMessage(WM_COMMAND,IDM_LOGON_SYSTEM,0);
	}
	return true;
}

//网络关闭消息
bool CServiceManage::OnSocketCloseEvent()
{
	if (m_Info.IsLogonSystem()==false)
	{
		m_Info.m_uSystemState=STS_NO_LOGON;
		if (m_pIMessage!=NULL) m_pIMessage->ShowMessage(TEXT("由于登陆服务器繁忙，请稍后再次登陆..."));
		AfxGetMainWnd()->PostMessage(WM_COMMAND,IDM_LOGON_SYSTEM,0);
	}
	return true;
}

//增加一条组件记录在ServerDllInfo表中，同时把新的组件加到m_ServiceInfo中
bool CServiceManage::SetupCom(CListCtrl * pListCtrl,SC_ServiceInfoStruct pServiceInfo)
{
	if (false == sqlAddConnection())
	{
		return false;
	}

	bool bres=m_pEngineSink->setStoredProc("SP_AddGameServer",true);
	if(0!=bres)
	{
		return 0;
	}

	m_pEngineSink->addInputParameter("@ServerInfoID",pServiceInfo.uServiceID);
	m_pEngineSink->addInputParameter("@NameID",pServiceInfo.ServiceInfo.uNameID);
	m_pEngineSink->addInputParameter("@DeskPeople",pServiceInfo.ServiceInfo.uDeskPeople);
	m_pEngineSink->addInputParameter("@SuppoerType",pServiceInfo.ServiceInfo.uSupportType);
	m_pEngineSink->addInputParameter("@ServerVer",pServiceInfo.ServiceInfo.uServiceVer);
	m_pEngineSink->addInputParameter("@GameName",sizeof(pServiceInfo.ServiceInfo.szGameName),pServiceInfo.ServiceInfo.szGameName);
	m_pEngineSink->addInputParameter("@GameTable",sizeof(pServiceInfo.ServiceInfo.szGameTable),pServiceInfo.ServiceInfo.szGameTable);
	m_pEngineSink->addInputParameter("@DllFileName",sizeof(pServiceInfo.ServiceInfo.szDLLFileName),pServiceInfo.ServiceInfo.szDLLFileName);

	if(0 != m_pEngineSink->execStoredProc())
	{
		m_pEngineSink->closeRecord();
		return 0;
	}

	int ret=m_pEngineSink->getReturnValue();

	m_pEngineSink->closeRecord();
	dklSQLEngineDelete(m_pEngineSink);
	m_pEngineSink = nullptr;

	if(ret==1)
	{
		if (m_pIMessage!=NULL) m_pIMessage->ShowMessage("不能新建和现有组件ServerInfoID相同的组件,请重新填写! :-) ");//AfxMessageBox("不能新建和现有组件ServerInfoID相同的组件,请重新填写! :-) ");
		return false;
	}

	m_ServiceInfo.Add(&pServiceInfo);//同时把新的组件加到m_ServiceInfo中
	UpdateComDLLList(pListCtrl);
	return true;

}

////修改一条组件记录在ServerDllInfo表中
bool CServiceManage::ModifCom(CListCtrl * pListCtrl,SC_ServiceInfoStruct * avantp,SC_ServiceInfoStruct pServiceInfo)
{
	if (false == sqlAddConnection())
	{
		return false;
	}

	bool bres=m_pEngineSink->setStoredProc("SP_EditGameServer",true);
	if(0!=bres)
	{
		return 0;
	}

	m_pEngineSink->addInputParameter("@ServerInfoID",pServiceInfo.uServiceID);
	m_pEngineSink->addInputParameter("@NameID",pServiceInfo.ServiceInfo.uNameID);
	m_pEngineSink->addInputParameter("@DeskPeople",pServiceInfo.ServiceInfo.uDeskPeople);
	m_pEngineSink->addInputParameter("@SuppoerType",pServiceInfo.ServiceInfo.uSupportType);
	m_pEngineSink->addInputParameter("@ServerVer",pServiceInfo.ServiceInfo.uServiceVer);
	m_pEngineSink->addInputParameter("@GameName",sizeof(pServiceInfo.ServiceInfo.szGameName),pServiceInfo.ServiceInfo.szGameName);
	m_pEngineSink->addInputParameter("@GameTable",sizeof(pServiceInfo.ServiceInfo.szGameTable),pServiceInfo.ServiceInfo.szGameTable);
	m_pEngineSink->addInputParameter("@DllFileName",sizeof(pServiceInfo.ServiceInfo.szDLLFileName),pServiceInfo.ServiceInfo.szDLLFileName);

	if(0 != m_pEngineSink->execStoredProc())
	{
		m_pEngineSink->closeRecord();
		return 0;
	}

	int ret=m_pEngineSink->getReturnValue();
	
	m_pEngineSink->closeRecord();
	dklSQLEngineDelete(m_pEngineSink);
	m_pEngineSink = nullptr;

	//使无重复ID
	if (ret==1)
	{
		if (m_pIMessage!=NULL) m_pIMessage->ShowMessage("ServerDllInfo 表中无此ServerInfoID相同的组件,请刷新组件! ");
		//AfxMessageBox("ServerDllInfo 表中无此ServerInfoID相同的组件,请刷新组件! ");
		return false;
	}

	for(int i=0;i<= m_ServiceInfo.GetUpperBound();i++)
	{
		SC_ServiceInfoStruct *p;
		p = (SC_ServiceInfoStruct *) m_ServiceInfo.GetAt(i);
		if(p->uServiceID==pServiceInfo.uServiceID)
		{
			m_ServiceInfo.RemoveAt(i);//删除旧的记录(要么?)
			break;
		}
	}
	m_ServiceInfo.Add(&pServiceInfo);//同时把新的组件加到m_ServiceInfo中
	UpdateComDLLList(pListCtrl);
	return true;

}

// 拷贝新的DLL库到安装目录，用于更新和安装新的组件
bool CServiceManage::CopyNewCom(SC_ServiceInfoStruct* pServiceInfo)
{
	if(AfxMessageBox("拷贝新的动态库文件到WLocalService.exe所在的目录下然后点ok将自动启动对应的房间。",IDOK)==IDOK)
		return true;
	return false;
}

//移出全部组件列表从m_ServiceInfo,删除数据库ServerDLLInfo里的记录
bool CServiceManage::RemoveDLLList(CListCtrl * pListCtrl)
{
	if(!m_ServiceInfo.IsEmpty())//是否还有记录
		return true;
	while(!m_ServiceInfo.IsEmpty())
	{
		RemoveDLLCom(pListCtrl,(SC_ServiceInfoStruct *)m_ServiceInfo.GetAt(m_ServiceInfo.GetUpperBound()));
	}
	//UpdateComDLLList(pListCtrl);
	return true;
}

//移出指定组件列表从m_ServiceInfo并删除其数据库ServerDLLInfo里的记录
bool CServiceManage::RemoveDLLCom(CListCtrl *pListCtrl,SC_ServiceInfoStruct *pServiceInfo)
{
	
	if(m_ServiceInfo.IsEmpty())//是否还有记录
	{
		lstrcpy(m_szError,TEXT("组件列表记录里已没有任何组件"));
		return false;
	}
	//搜索要删除的记录
	for(int i=0;i<= m_ServiceInfo.GetUpperBound();i++)
	{
		SC_ServiceInfoStruct* ptem;
		ptem = (SC_ServiceInfoStruct *)(m_ServiceInfo.GetAt(i)); 
		
		//找到记录
		if(ptem->uServiceID == pServiceInfo->uServiceID)
		{
			if (false == sqlAddConnection())
			{
				return false;
			}

			TCHAR szSQL[200];
			wsprintf(szSQL,TEXT("DELETE FROM TGameServerInfo WHERE ServerInfoID = %d"), pServiceInfo->uServiceID);
			m_pEngineSink->execSQL(szSQL,false);

			m_pEngineSink->closeRecord();
			dklSQLEngineDelete(m_pEngineSink);
			m_pEngineSink = nullptr;

			m_ServiceInfo.RemoveAt(i);//同时移出队列中的记录
				pListCtrl->DeleteItem(i);
			return true;
		}
	}
	//没有记录
	lstrcpy(m_szError,TEXT("列表中已无此组件记录"));
	return false;
}


// 删除数据库房间信息 
bool CServiceManage::DeleteGameRoomInfo(SC_GameRoomInitStruct * pComRoomInfo)
{
	try
	{
		if (false == sqlAddConnection())
		{
			return false;
		}

		bool bres=m_pEngineSink->setStoredProc("SP_EraseRoomInfo",true);
		if(0!=bres)
		{
			return 0;
		}

		m_pEngineSink->addInputParameter("@RoomID",pComRoomInfo->InitInfo.uRoomID);

		if(0 != m_pEngineSink->execStoredProc())
		{
			m_pEngineSink->closeRecord();
			return 0;
		}
		m_pEngineSink->closeRecord();
		dklSQLEngineDelete(m_pEngineSink);
		m_pEngineSink = nullptr;

		SC_GameRoomInitStruct *p;
		for(int i = 0;i<=m_ComRoomInfo.GetUpperBound();i++)
		{
			p=(SC_GameRoomInitStruct *)m_ComRoomInfo.GetAt(i);
			if(p->uServiceID==pComRoomInfo->uServiceID && p->InitInfo.szGameRoomName==pComRoomInfo->InitInfo.szGameRoomName)
			{
				m_ComRoomInfo.RemoveAt(i);
				break;
			}
		}

		if (m_pIMessage!=NULL) m_pIMessage->ShowMessage(TEXT("删除组件的子实例信息完成！"));
		return true;
	}
	catch (...) 
	{ TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		if (m_pIMessage!=NULL) m_pIMessage->ShowMessage(TEXT("删除组件的子实例信息发生错误，稍后再次尝试..."));
	}
	return false;
}

///desc：获得所有游戏房间信息//W_0002.2
bool CServiceManage::GetRoomListInfo()
{
	if (false == sqlAddConnection())
	{
		return false;
	}

	bool bres=m_pEngineSink->setStoredProc("SP_GetRoomList",true);
	if(0!=bres)
	{
		return 0;
	}

	try
	{

		m_pEngineSink->addInputParameter("@ServiceName",m_ServiceName.GetLength(),m_ServiceName);
		if(0 != m_pEngineSink->execStoredProc())
		{
			m_pEngineSink->closeRecord();
			return 0;
		}
		while(!m_pEngineSink->adoEndOfFile())
		{
			bool goOn=false,update=false;
			UINT ui=0,uID=0,uPort=0;//无符号
			__int64 i64 = 0;
			//INT ui=0;//符号
			//定义变量
			SC_GameRoomInitStruct * pGameRoomInitInfo=new SC_GameRoomInitStruct;
			memset(pGameRoomInitInfo,0,sizeof(SC_GameRoomInitStruct));

            // 指定哪些游戏是可用的
            m_pEngineSink->getValue(TEXT("NameID"),&ui);
			pGameRoomInitInfo->InitInfo.iGameID = ui;
            if(FALSE == m_GameSelecter.GameIsUsable(ui))
            {
                m_pEngineSink->moveNext();
                delete pGameRoomInitInfo;
                continue;
            }

			//已经读取过的不加
			m_pEngineSink->getValue(TEXT("ServerInfoID"),&uID);
			m_pEngineSink->getValue(TEXT("SocketPort"),&uPort);
			for(int i=0;i<m_ComRoomInfo.GetCount();i++)
			{
				SC_GameRoomInitStruct * p;
				p = (SC_GameRoomInitStruct *)m_ComRoomInfo.GetAt(i);
				//启动的房间
				if(p->uServiceID==uID&&p->InitInfo.uListenPort==uPort&&p->hDllInstance!=NULL&&p->pIManageService!=NULL)
				{

					goOn=true;
					break;
				}
				//停止的房间
				if(p->uServiceID==uID&&p->InitInfo.uListenPort==uPort&&p->hDllInstance==NULL&&p->pIManageService==NULL)
				{
					update=true;
					delete pGameRoomInitInfo;
					pGameRoomInitInfo=p;
					break;
				}
			}
			if(goOn)
			{
				//移动下一条
				m_pEngineSink->moveNext();
				continue;
			}
			if(!update)
				m_ComRoomInfo.Add(pGameRoomInitInfo);
			//读取附加信息
			m_pEngineSink->getValue(TEXT("IDSort"),&ui);
			pGameRoomInitInfo->uSortID=ui;
			m_pEngineSink->getValue(TEXT("KindID"),&ui);
			pGameRoomInitInfo->uKindID=ui;
			m_pEngineSink->getValue(TEXT("EnableRoom"),&pGameRoomInitInfo->bEnable);

			m_pEngineSink->getValue(TEXT("ServerInfoID"),&ui);

			pGameRoomInitInfo->uServiceID=ui;////////////////
			//读取组件信息
			m_pEngineSink->getValue(TEXT("NameID"),&ui);
			pGameRoomInitInfo->ServiceInfo.uNameID=ui;////////////
			m_pEngineSink->getValue(TEXT("DeskPeople"),&ui);
			pGameRoomInitInfo->ServiceInfo.uDeskPeople=ui;
			m_pEngineSink->getValue(TEXT("SupportType"),&ui);
			pGameRoomInitInfo->ServiceInfo.uSupportType=ui;
			m_pEngineSink->getValue(TEXT("ServiceVer"),&ui);
			pGameRoomInitInfo->ServiceInfo.uServiceVer=ui;
			m_pEngineSink->getValue(TEXT("GameName"),pGameRoomInitInfo->ServiceInfo.szGameName,sizeof(pGameRoomInitInfo->ServiceInfo.szGameName));
			m_pEngineSink->getValue(TEXT("GameTable"),pGameRoomInitInfo->ServiceInfo.szGameTable,sizeof(pGameRoomInitInfo->ServiceInfo.szGameTable));
			m_pEngineSink->getValue(TEXT("DLLFileName"),pGameRoomInitInfo->ServiceInfo.szDLLFileName,sizeof(pGameRoomInitInfo->ServiceInfo.szDLLFileName));

			//读取房间信息
			pGameRoomInitInfo->InitInfo.uNameID=pGameRoomInitInfo->ServiceInfo.uNameID;
			m_pEngineSink->getValue(TEXT("RoomRule"),&ui);
			pGameRoomInitInfo->InitInfo.dwRoomRule=ui;
			m_pEngineSink->getValue(TEXT("UserPower"),&ui);
			pGameRoomInitInfo->InitInfo.dwUserPower=ui;
			m_pEngineSink->getValue(TEXT("GameTypeID"),&ui);
			pGameRoomInitInfo->InitInfo.uComType=ui;
			m_pEngineSink->getValue(TEXT("BasePoint"),&ui);
			pGameRoomInitInfo->InitInfo.uBasePoint=ui;
			m_pEngineSink->getValue(TEXT("LessPoint"),&ui);
			pGameRoomInitInfo->InitInfo.uLessPoint=ui;
			m_pEngineSink->getValue(TEXT("MaxPoint"),&ui);
			pGameRoomInitInfo->InitInfo.uMaxPoint=ui;
			m_pEngineSink->getValue(TEXT("MaxLimite"),&ui);
			pGameRoomInitInfo->InitInfo.uMaxLimite=ui;
			m_pEngineSink->getValue(TEXT("RunPublish"),&ui);
			pGameRoomInitInfo->InitInfo.uRunPublish=ui;				
			m_pEngineSink->getValue(TEXT("Tax"),&ui);
			pGameRoomInitInfo->InitInfo.uTax=ui;
			m_pEngineSink->getValue(TEXT("SocketPort"),&ui);
			pGameRoomInitInfo->InitInfo.uListenPort=ui;
			m_pEngineSink->getValue(TEXT("MaxPeople"),&ui);
			pGameRoomInitInfo->InitInfo.uMaxPeople=ui;
			pGameRoomInitInfo->InitInfo.iSocketSecretKey = SECRET_KEY;
			m_pEngineSink->getValue(TEXT("VipResever"),&ui);
			pGameRoomInitInfo->InitInfo.uVipReseverCount=ui;
			m_pEngineSink->getValue(TEXT("VirtualUser"),&ui);
			pGameRoomInitInfo->InitInfo.uVirtualUser=ui;
			m_pEngineSink->getValue(TEXT("VirtualGameTime"),&ui);
			pGameRoomInitInfo->InitInfo.uVirtualGameTime=ui;

			m_pEngineSink->getValue(TEXT("LockUserType"),&ui);
			pGameRoomInitInfo->InitInfo.iLockType=ui;
			m_pEngineSink->getValue(TEXT("LocationLimit"),&ui);
			pGameRoomInitInfo->InitInfo.iPositionLimit=ui;
			m_pEngineSink->getValue(TEXT("SameIPNotice"),&pGameRoomInitInfo->InitInfo.bIPCheck);
			m_pEngineSink->getValue(TEXT("LocationNotice"),&pGameRoomInitInfo->InitInfo.bPositionCheck);
			
			m_pEngineSink->getValue(TEXT("CanPalyback"),&ui);
			pGameRoomInitInfo->InitInfo.bCanPlayBack=ui;

			//添加游戏混战场--begin
			m_pEngineSink->getValue(TEXT("BattleRoomID"),&ui);
			pGameRoomInitInfo->InitInfo.uBattleRoomID = ui;
			m_pEngineSink->getValue(TEXT("BattleMatchTable"),pGameRoomInitInfo->InitInfo.szBattleGameTable,
				sizeof(pGameRoomInitInfo->InitInfo.szBattleGameTable));
			////////////////--end

			///< 金葫芦二代，是否VIP房间（VIP房间设置密码）
			m_pEngineSink->getValue(TEXT("VIPRoom"),&ui);
			pGameRoomInitInfo->InitInfo.bVIPRoom = (ui!=0);

			m_pEngineSink->getValue(TEXT("DeskCount"),&ui);
			pGameRoomInitInfo->InitInfo.uDeskCount=ui;
			m_pEngineSink->getValue(TEXT("RoomID"),&ui);
			pGameRoomInitInfo->InitInfo.uRoomID=ui;
			m_pEngineSink->getValue(TEXT("StopLogon"),&ui);
			pGameRoomInitInfo->InitInfo.uStopLogon=0;
			m_pEngineSink->getValue(TEXT("RoomName"),pGameRoomInitInfo->InitInfo.szGameRoomName,sizeof(pGameRoomInitInfo->InitInfo.szGameRoomName));
			m_pEngineSink->getValue(TEXT("ServerIP"),m_Info.m_szServiceIP,sizeof(m_Info.m_szServiceIP));
			m_pEngineSink->getValue(TEXT("TableLock"),pGameRoomInitInfo->InitInfo.szLockTable,sizeof(pGameRoomInitInfo->InitInfo.szLockTable));
			
			m_pEngineSink->getValue(TEXT("DeskMinPeople"),&pGameRoomInitInfo->InitInfo.uMinDeskPeople);
			//m_pEngineSink->getValue(TEXT("RoomRule"),&pGameRoomInitInfo->InitInfo.uBattleRoomID);

			if (pGameRoomInitInfo->InitInfo.uBattleRoomID > 0)
			{
				memcpy(pGameRoomInitInfo->InitInfo.szGameTable,
					pGameRoomInitInfo->InitInfo.szBattleGameTable,
					sizeof(pGameRoomInitInfo->InitInfo.szBattleGameTable));
			}
			else
			{
				m_pEngineSink->getValue(TEXT("TableMatch"),pGameRoomInitInfo->InitInfo.szGameTable,sizeof(pGameRoomInitInfo->InitInfo.szGameTable));
			}
			////////////////////
			m_pEngineSink->getValue(TEXT("TableIPRule"),pGameRoomInitInfo->InitInfo.szIPRuleTable,sizeof(pGameRoomInitInfo->InitInfo.szIPRuleTable));
			m_pEngineSink->getValue(TEXT("TableUserNameRule"),pGameRoomInitInfo->InitInfo.szNameRuleTable,sizeof(pGameRoomInitInfo->InitInfo.szNameRuleTable));
			// 增加百家乐桌型

			//房间密码
			m_pEngineSink->getValue(TEXT("RoomPassword"),pGameRoomInitInfo->InitInfo.szRoomPassWord,sizeof(pGameRoomInitInfo->InitInfo.szRoomPassWord));
			/// 看是不是百家乐类型的游戏
			char cKey[10];
			CString sPath=CBcfFile::GetAppPath();
			int iResult = 0;
			if (CBcfFile::IsFileExist(sPath + "SpecialRule.bcf"))
			{
				CBcfFile fsr( sPath + "SpecialRule.bcf");
				sprintf(cKey, "%d", pGameRoomInitInfo->ServiceInfo.uNameID);
				iResult = fsr.GetKeyVal (_T("BJLType"), cKey, 0);
			}
			
			if (iResult!=0)
			{
				pGameRoomInitInfo->InitInfo.uDeskType = DESK_TYPE_BJL;
			}
			else
			{
				pGameRoomInitInfo->InitInfo.uDeskType = DESK_TYPE_NORMAL;
			}
			
			//比赛
			if (pGameRoomInitInfo->InitInfo.dwRoomRule & GRR_CONTEST || pGameRoomInitInfo->InitInfo.dwRoomRule & GRR_TIMINGCONTEST)
			{
				CTime _Now = CTime::GetCurrentTime();
				int nYear = 0, nMonth = 0, nDate = 0, nHour = 0, nMin = 0, nSec = 0, nss = 0;  
				CString temp;
				if (temp != "")
				{
					sscanf(temp.GetBuffer(), TEXT("%d-%d-%d %d:%d:%d.%d"), &nYear, &nMonth, &nDate, &nHour, &nMin, &nSec, &nss);

					CTime _t(nYear, nMonth, nDate, nHour, nMin, nSec); 
					pGameRoomInitInfo->InitInfo.i64TimeStart = _t.GetTime();
				}
				else
				{
					pGameRoomInitInfo->InitInfo.i64TimeStart = 0;
				}
				
				m_pEngineSink->getValue(TEXT("Chip"), &pGameRoomInitInfo->InitInfo.i64Chip);
				m_pEngineSink->getValue(TEXT("LowChip"), &pGameRoomInitInfo->InitInfo.i64LowChip);
				m_pEngineSink->getValue(TEXT("LowCount"), &ui);
				pGameRoomInitInfo->InitInfo.iLowCount = ui; 
				m_pEngineSink->getValue(TEXT("ContestID"), &ui);
				pGameRoomInitInfo->InitInfo.iContestID = ui; 
				m_pEngineSink->getValue(TEXT("Timeout"), &ui);
				pGameRoomInitInfo->InitInfo.iTimeout = ui;
				m_pEngineSink->getValue(TEXT("ContestTime"), &ui);
				pGameRoomInitInfo->InitInfo.iContestTime = ui;
				m_pEngineSink->getValue(TEXT("LeastPeople"), &ui);
				pGameRoomInitInfo->InitInfo.iLeasePeople = ui;
				m_pEngineSink->getValue(TEXT("MinPeople"), &ui);
				pGameRoomInitInfo->InitInfo.iMinPeople = ui;
				m_pEngineSink->getValue(TEXT("UpPeople"), &ui);
				pGameRoomInitInfo->InitInfo.iUpPeople = ui;
				pGameRoomInitInfo->InitInfo.uLessPoint = 0;
			}
			//租赁房间ID与时间
			m_pEngineSink->getValue(TEXT("tenancyID"), &pGameRoomInitInfo->InitInfo.iLeaseID);
			if (pGameRoomInitInfo->InitInfo.iLeaseID != 0)
			{
				int nYear = 0, nMonth = 0, nDate = 0, nHour = 0, nMin = 0, nSec = 0, nss = 0;  
				char temp[64];
				m_pEngineSink->getValue(TEXT("OpenRoomDate"),temp, sizeof(temp));
				if (temp != '\0')
				{
					sscanf(temp, TEXT("%d-%d-%d %d:%d:%d.%d"), &nYear, &nMonth, &nDate, &nHour, &nMin, &nSec, &nss);

					CTime _t(nYear, nMonth, nDate, nHour, nMin, nSec); 
					pGameRoomInitInfo->InitInfo.i64LeaseTimeStart = _t.GetTime();
				}
				else
				{
					pGameRoomInitInfo->InitInfo.i64LeaseTimeStart = 0;
				}
				int idays = 0;
				m_pEngineSink->getValue(TEXT("AppleyDays"),&idays);
				pGameRoomInitInfo->InitInfo.i64LeaseTimeEnd = pGameRoomInitInfo->InitInfo.i64LeaseTimeStart + idays * 24 * 60 * 60;
			}

			m_pEngineSink->moveNext();
		}
		m_pEngineSink->closeRecord();

		dklSQLEngineDelete(m_pEngineSink);
		m_pEngineSink = nullptr;

		return true;
	}
	catch (...) 
	{ 
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		if (m_pIMessage!=NULL) 
			m_pIMessage->ShowMessage(TEXT("组件的子实例信息读取发生错误，稍后再次尝试..."));
	}

}

bool CServiceManage::UpdateRoomListInfo(CListCtrl * pListCtrl)
{
	////注册码检测////
	//bool bCheck = false;
	//bCheck = CheckLicense();
	//if(!bCheck)
	//{
	//	return false;
	//}

	SC_GameRoomInitStruct * pGameRoomInitInfo;
	//去除所有记录
	pListCtrl->DeleteAllItems();
	//读取资料
	TCHAR szBuffer[50];
	for(int i=0;i<=m_ComRoomInfo.GetUpperBound();i++)
	{
		pGameRoomInitInfo = (SC_GameRoomInitStruct *) m_ComRoomInfo.GetAt(i);
		int iItem=pListCtrl->InsertItem(pListCtrl->GetItemCount(),ltoa(pGameRoomInitInfo->InitInfo.uRoomID,szBuffer,10));
		pListCtrl->SetItem(iItem,1,1,pGameRoomInitInfo->InitInfo.szGameRoomName,0,0,0,0);
		pListCtrl->SetItem(iItem,2,1,pGameRoomInitInfo->ServiceInfo.szGameName,0,0,0,0);
		if(pGameRoomInitInfo->hDllInstance==NULL||pGameRoomInitInfo->pIManageService==NULL)
			pListCtrl->SetItem(iItem,3,1,TEXT("停止"),0,0,0,0);
		else 
			pListCtrl->SetItem(iItem,3,1,TEXT("启动"),0,0,0,0);
		pListCtrl->SetItem(iItem,4,1,_ltoa(pGameRoomInitInfo->InitInfo.uComType,szBuffer,10),0,0,0,0);
		pListCtrl->SetItem(iItem,5,1,_ltoa(pGameRoomInitInfo->bEnable,szBuffer,10),0,0,0,0);
		pListCtrl->SetItem(iItem,6,1,m_Info.m_szServiceIP,0,0,0,0);
		pListCtrl->SetItem(iItem,7,1,ltoa(pGameRoomInitInfo->InitInfo.uListenPort,szBuffer,10),0,0,0,0);
		pListCtrl->SetItem(iItem,8,1,ltoa(pGameRoomInitInfo->InitInfo.uDeskCount,szBuffer,10),0,0,0,0);
		pListCtrl->SetItem(iItem,9,1,ltoa(0,szBuffer,10),0,0,0,0);
		pListCtrl->SetItem(iItem,10,1,ltoa(pGameRoomInitInfo->InitInfo.uMaxPeople,szBuffer,10),0,0,0,0);
		pListCtrl->SetItem(iItem,11,1,ltoa(pGameRoomInitInfo->InitInfo.uLessPoint,szBuffer,10),0,0,0,0);
		pListCtrl->SetItem(iItem,12,1,ltoa(pGameRoomInitInfo->InitInfo.uBasePoint,szBuffer,10),0,0,0,0);
		pListCtrl->SetItem(iItem,13,1,ltoa(pGameRoomInitInfo->InitInfo.uMaxLimite,szBuffer,10),0,0,0,0);
		pListCtrl->SetItem(iItem,14,1,ltoa(pGameRoomInitInfo->InitInfo.dwRoomRule,szBuffer,10),0,0,0,0);
		pListCtrl->SetItem(iItem,15,1,ltoa(pGameRoomInitInfo->InitInfo.dwUserPower,szBuffer,10),0,0,0,0);
		pListCtrl->SetItem(iItem,19,1,ltoa(pGameRoomInitInfo->uKindID,szBuffer,10),0,0,0,0);
		pListCtrl->SetItem(iItem,20,1,ltoa(pGameRoomInitInfo->ServiceInfo.uNameID,szBuffer,10),0,0,0,0);
		pListCtrl->SetItem(iItem,22,1,pGameRoomInitInfo->InitInfo.szGameTable,0,0,0,0);
		pListCtrl->SetItem(iItem,23,1,pGameRoomInitInfo->InitInfo.szLockTable,0,0,0,0);
		pListCtrl->SetItem(iItem,24,1,pGameRoomInitInfo->InitInfo.szIPRuleTable,0,0,0,0);
		pListCtrl->SetItem(iItem,25,1,pGameRoomInitInfo->InitInfo.szNameRuleTable,0,0,0,0);
		pListCtrl->SetItem(iItem,26,1,ltoa(pGameRoomInitInfo->uSortID,szBuffer,10),0,0,0,0);
		pListCtrl->SetItem(iItem,27,1,ltoa(pGameRoomInitInfo->uServiceID,szBuffer,10),0,0,0,0);
		pListCtrl->SetItem(iItem,28,1,pGameRoomInitInfo->ServiceInfo.szDLLFileName,0,0,0,0);
		pListCtrl->SetItem(iItem,29,1,pGameRoomInitInfo->szRoomNote,0,0,0,0);
		pListCtrl->SetItem(iItem,30,1,ltoa(pGameRoomInitInfo->ServiceInfo.uDeskPeople,szBuffer,10),0,0,0,0);
		if(pGameRoomInitInfo->InitInfo.uStopLogon)
			pListCtrl->SetItem(iItem,31,1,"Yes",0,0,0,0);
		else pListCtrl->SetItem(iItem,31,1,"No",0,0,0,0);
		if(pGameRoomInitInfo->InitInfo.uComType==TY_MONEY_GAME)
			pListCtrl->SetItem(iItem,33,1,ltoa(pGameRoomInitInfo->InitInfo.uTax,szBuffer,10),0,0,0,0);
		else pListCtrl->SetItem(iItem,33,1,"无",0,0,0,0);
		pListCtrl->SetItem(iItem,34,1,ltoa(pGameRoomInitInfo->InitInfo.uRunPublish,szBuffer,10),0,0,0,0);
		pListCtrl->SetItemData(iItem,(DWORD_PTR)pGameRoomInitInfo);		
	}
	return true;
}

void CServiceManage::CountOnLinePeople(CListCtrl * everyDLL, CListCtrl * everyRoom)
{
	static int licCount=0;
	if(++licCount>=10)
		licCount=0;
	try
	{
		if (false == sqlAddConnection())
		{
			return ;
		}

		bool bres=m_pEngineSink->setStoredProc("SP_GetRoomList",true);
		if(0!=bres)
		{
			return;
		}

		bres=m_pEngineSink->setStoredProc("SP_GetOnlinePeopleCount",true);
		if(!bres)
		{
			return;
		}
		m_pEngineSink->addInputParameter("@ServiceName",sizeof(m_ServiceName),m_ServiceName);
		if(0 != m_pEngineSink->execStoredProc())
		{
			m_pEngineSink->closeRecord();
			return ;
		}

		TCHAR szBuffer[50];
		while(!m_pEngineSink->adoEndOfFile())
		{
			UINT uServerInfoID=0,uSocketPort=0,uOnLineCount=0;//无符号

			//读取ServerInfoID信息
			m_pEngineSink->getValue(TEXT("ServerInfoID"),&uServerInfoID);
			//读取socketPort
			m_pEngineSink->getValue(TEXT("SocketPort"),&uSocketPort);
			for(int i=0;i<m_ComRoomInfo.GetCount();i++)
			{
				SC_GameRoomInitStruct * p;
				p = (SC_GameRoomInitStruct *)m_ComRoomInfo.GetAt(i);
				if(uServerInfoID==p->uServiceID&&uSocketPort==p->InitInfo.uListenPort)
				{
					m_pEngineSink->getValue(TEXT("OnLineCount"),&uOnLineCount);
					everyRoom->SetItem(i,9,1,ltoa(uOnLineCount,szBuffer,10),0,0,0,0);
				}
			}
			m_pEngineSink->moveNext();
		}
		m_pEngineSink->closeRecord();
		dklSQLEngineDelete(m_pEngineSink);
		m_pEngineSink = nullptr;
		return;
	}
	catch (...) 
	{ 
		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
	}
	return ;
}


CString CServiceManage::GetRoomName()
{
	CString _r = "";
	for(int i=0;i<m_ComRoomInfo.GetCount();i++)
	{
		SC_GameRoomInitStruct * p;
		p = (SC_GameRoomInitStruct *)m_ComRoomInfo.GetAt(i);
		_r += p->InitInfo.szGameRoomName;
		if (i != m_ComRoomInfo.GetCount() - 1)
			_r += "\n\r";
	}
	return _r;
}

void CServiceManage::ClearRoomTWRecode(int iRoomID)
{
	if (false == sqlAddConnection())
	{
		return ;
	}

	bool bres=m_pEngineSink->setStoredProc("SP_GetGameServerList",true);
	if(0!=bres)
	{
		return;
	}

	m_pEngineSink->closeRecord();

	bres=m_pEngineSink->setStoredProc("SP_ClearRoomTWLoginRecord",true);
	if(0!=bres)
	{
		return;
	}

	m_pEngineSink->addInputParameter("@RoomID", iRoomID);
	if(0 != m_pEngineSink->execStoredProc())
	{
		m_pEngineSink->closeRecord();
		return ;
	}
    m_pEngineSink->closeRecord();
	dklSQLEngineDelete(m_pEngineSink);
	m_pEngineSink = nullptr;
}