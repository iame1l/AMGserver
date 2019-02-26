/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "StdAfx.h"
#include "CenterServerManage.h"
#include "CommonUse.h"

//宏定义
#define IDT_UPDATE_INIFILE						20					//检测数据连接定时器
#define IDT_CHECK_DATA_CONNECT					2					//检测数据连接定时器
extern void OutputFun(char* message);
/******************************************************************************************************/

// 启动函数
bool CCenterServerManage::OnStart()
{
	// 检测数据库连接
	SetTimer(IDT_UPDATE_INIFILE,15000L);

	// m_ZServerManager对象在B服务器中无用，B中包含的Z的IP和端口都是存储在GAMEGATE.bcf文件中
	m_ZServerManager.Start();

	return true;
}

// 定时器消息
bool CCenterServerManage::OnTimerMessage(UINT uTimerID)
{
	switch (uTimerID)
	{
	case IDT_UPDATE_INIFILE:	// 更新游戏列表定时器
		{
			KillTimer(IDT_UPDATE_INIFILE);
			GetINIFile();
			SetTimer(IDT_UPDATE_INIFILE,30000L);
			break;
		}
	default:
		break;
	}

	return true;
}
void CCenterServerManage::GetINIFile()
{	
	CBcfFile f(CBcfFile::GetAppPath() + "HNGameGate.bcf");
	CString strValue;

	if(!f.IsFileExist())
	{
		return;
	}
	
	// 客户端当前版本系列号，和用户端比较不同则要用户去升级
	strValue = f.GetKeyVal("GateServer", "SerialNo", "");
	strncpy(m_msgSendToClient.m_strGameSerialNO, strValue, sizeof(m_msgSendToClient.m_strGameSerialNO)-1);
	m_msgSendToClient.m_strGameSerialNO[sizeof(m_msgSendToClient.m_strGameSerialNO)-1] = '\0';

	// 主服务器IP地址
	strValue = f.GetKeyVal("GateServer", "MainServerAddress", "");
	strncpy(m_msgSendToClient.m_strMainserverIPAddr, strValue, sizeof(m_msgSendToClient.m_strMainserverIPAddr)-1);
	m_msgSendToClient.m_strMainserverIPAddr[sizeof(m_msgSendToClient.m_strMainserverIPAddr)-1] = '\0';

	// 主服务器端口号
	m_msgSendToClient.m_iMainserverPort = f.GetKeyVal("GateServer", "MainServerPort", 6800);

	// 平台所采用的加密方式，1位MD5，2位SHA1，默认为2
	m_msgSendToClient.m_nEncryptType = f.GetKeyVal("GateServer","EncryType", 2);

	// 主页WEB地址
	strValue = f.GetKeyVal("GateServer", "WebHomeURL", "");
	strncpy(m_msgSendToClient.m_strHomeADDR, strValue, sizeof(m_msgSendToClient.m_strHomeADDR)-1);
	m_msgSendToClient.m_strHomeADDR[sizeof(m_msgSendToClient.m_strHomeADDR)-1] = '\0';

	// 网站根路径，在程序中涉及的文件子目录根据这个地址来扩展
	strValue = f.GetKeyVal("GateServer", "WebRootURL", "");
	strncpy(m_msgSendToClient.m_strWebRootADDR, strValue, sizeof(m_msgSendToClient.m_strWebRootADDR)-1);
	m_msgSendToClient.m_strWebRootADDR[sizeof(m_msgSendToClient.m_strWebRootADDR)-1] = '\0';

	// 帮助页WEB地址
	strValue = f.GetKeyVal("GateServer", "WebHelpURL", "");
	strncpy(m_msgSendToClient.m_strHelpADDR, strValue, sizeof(m_msgSendToClient.m_strHelpADDR)-1);
	m_msgSendToClient.m_strHelpADDR[sizeof(m_msgSendToClient.m_strHelpADDR)-1] = '\0';

	// 客户端安装程序下载页WEB地址
	strValue = f.GetKeyVal("GateServer", "DownLoadSetupURL", "");
	strncpy(m_msgSendToClient.m_strDownLoadSetupADDR, strValue, sizeof(m_msgSendToClient.m_strDownLoadSetupADDR)-1);
	m_msgSendToClient.m_strDownLoadSetupADDR[sizeof(m_msgSendToClient.m_strDownLoadSetupADDR)-1] = '\0';

	// 客户端更新程序下载页WEB地址
	strValue = f.GetKeyVal("GateServer", "DownLoadUpdatepURL", "");
	strncpy(m_msgSendToClient.m_strDownLoadUpdatepADDR, strValue, sizeof(m_msgSendToClient.m_strDownLoadUpdatepADDR)-1);
	m_msgSendToClient.m_strDownLoadUpdatepADDR[sizeof(m_msgSendToClient.m_strDownLoadUpdatepADDR)-1] = '\0';

	// 客户端大厅FLASH广告下载页WEB地址
	strValue = f.GetKeyVal("GateServer","RallAddvtisFlashURL","");
	strncpy(m_msgSendToClient.m_strRallAddvtisFlashADDR, strValue, sizeof(m_msgSendToClient.m_strRallAddvtisFlashADDR)-1);
	m_msgSendToClient.m_strRallAddvtisFlashADDR[sizeof(m_msgSendToClient.m_strRallAddvtisFlashADDR)-1] = '\0';

	// 客户端滚动条广告地址
	strValue = f.GetKeyVal("GateServer", "RoomRollWords", "欢迎来到红鸟网络游戏世界！");
	strncpy(m_msgSendToClient.m_strRoomRollADDR, strValue, sizeof(m_msgSendToClient.m_strRoomRollADDR)-1);
	m_msgSendToClient.m_strRoomRollADDR[sizeof(m_msgSendToClient.m_strRoomRollADDR)-1] = '\0';

	// -
	m_msgSendToClient.m_nIsUsingIMList = f.GetKeyVal("GateServer","UsingIMList",1);

	// 大厅左上角是显示ID号还是魅力值
	m_msgSendToClient.m_nHallInfoShowClass = f.GetKeyVal("GateServer","HallInforShowClass",0);

	// -
    m_msgSendToClient.m_is_haveZhuanZhang = f.GetKeyVal("GateServer","IsHaveZhuanZhang",0);
	
	// 加载服务器列表
	//m_MainserverList.LoadServerList(cfgHandle);

	// 获取服务器使用的功能
	GetFunction(); 
}

void CCenterServerManage::GetURL(char *strKey)
{
	CBcfFile f(CBcfFile::GetAppPath() + "HNGameGate.bcf");
	CString strValue;

	if (NULL == strKey)
	{
		// 客户端滚动条广告地址
		strValue = f.GetKeyVal(_T("GateServer"), _T("RoomRollWords"), _T("欢迎来到红鸟棋牌游戏中心！"));
		strncpy(m_msgSendToClient.m_strRoomRollADDR, strValue, sizeof(m_msgSendToClient.m_strRoomRollADDR)-1);
		m_msgSendToClient.m_strRoomRollADDR[sizeof(m_msgSendToClient.m_strRoomRollADDR)-1] = '\0';

		// 帮助页WEB地址
		strValue = f.GetKeyVal(_T("GateServer"), _T("WebHelpURL"), _T(""));
		strncpy(m_msgSendToClient.m_strHelpADDR, strValue, sizeof(m_msgSendToClient.m_strHelpADDR)-1);
		m_msgSendToClient.m_strHelpADDR[sizeof(m_msgSendToClient.m_strHelpADDR)-1] = '\0';

		// 主页WEB地址
		strValue = f.GetKeyVal(_T("GateServer"), _T("WebHomeURL"), _T(""));
		strncpy(m_msgSendToClient.m_strHomeADDR, strValue, sizeof(m_msgSendToClient.m_strHomeADDR)-1);
		m_msgSendToClient.m_strHomeADDR[sizeof(m_msgSendToClient.m_strHomeADDR)-1] = '\0';

		// 网站根路径
		strValue = f.GetKeyVal(_T("GateServer"), _T("WebRootURL"), _T(""));
		strncpy(m_msgSendToClient.m_strWebRootADDR, strValue, sizeof(m_msgSendToClient.m_strWebRootADDR)-1);
		m_msgSendToClient.m_strWebRootADDR[sizeof(m_msgSendToClient.m_strWebRootADDR)-1] = '\0';
	}
	else
	{
		// 主页WEB地址
		strValue = f.GetKeyVal(_T("WebHomeURL"),strKey,_T(""));
		if (strValue.IsEmpty())
		{
			GetURL(NULL);
			return;
		}
		strncpy(m_msgSendToClient.m_strHomeADDR, strValue, sizeof(m_msgSendToClient.m_strHomeADDR)-1);
		m_msgSendToClient.m_strHomeADDR[sizeof(m_msgSendToClient.m_strHomeADDR)-1] = '\0';

		// 客户端滚动条广告地址
		strValue = f.GetKeyVal(_T("RoomRollWords"), strKey,_T(""));
		strncpy(m_msgSendToClient.m_strRoomRollADDR, strValue, sizeof(m_msgSendToClient.m_strRoomRollADDR)-1);
		m_msgSendToClient.m_strRoomRollADDR[sizeof(m_msgSendToClient.m_strRoomRollADDR)-1] = '\0';

		// 帮助页WEB地址
		strValue = f.GetKeyVal(_T("WebHelpURL"), strKey, _T(""));
		strncpy(m_msgSendToClient.m_strHelpADDR, strValue, sizeof(m_msgSendToClient.m_strHelpADDR)-1);
		m_msgSendToClient.m_strHelpADDR[sizeof(m_msgSendToClient.m_strHelpADDR)-1] = '\0';

		// 网站根路径
		strValue = f.GetKeyVal(_T("WebRootURL"), strKey, _T(""));
		strncpy(m_msgSendToClient.m_strWebRootADDR, strValue, sizeof(m_msgSendToClient.m_strWebRootADDR)-1);
		m_msgSendToClient.m_strWebRootADDR[sizeof(m_msgSendToClient.m_strWebRootADDR)-1] = '\0';
	}	
}

// 从Function.bcf中读取功能配置
void CCenterServerManage::GetFunction()
{
	CBcfFile f(CBcfFile::GetAppPath() + "Function.bcf");
	CString strValue;

	// 金葫芦2代指定ID。
	strValue = f.GetKeyVal("SpecificID", "Available", "0");
	if (0 != _ttoi(strValue))
	{
		m_msgSendToClient.m_nFunction = 1;
		strValue = f.GetKeyVal("SpecificID", "NormalID", "60000000, 69999999");
		m_msgSendToClient.m_lNomalIDFrom = atol(strValue.Left(strValue.Find(",")+1));
		m_msgSendToClient.m_lNomalIDEnd  = atol(strValue.Right(strValue.GetLength()-strValue.Find(",")-1));
	}

	 // 百乐门在线送金币
	strValue = f.GetKeyVal("OnlineCoin", "Available", "0");
	if (0 != _ttoi(strValue))
	{
		m_msgSendToClient.m_nFunction |= 1<<1;
	}

	// 响当当添加虚拟玩家
	strValue = f.GetKeyVal("RobotExtend","Available","0");
	if (0 != _ttoi(strValue))
	{
		m_msgSendToClient.m_nFunction |= 2<<1;
	}

    // 配置是否禁止私聊，0 = 没有禁止
    strValue = f.GetKeyVal("CommFunc","ForbidSay","0");
    if( 0 != _ttoi(strValue))
    {
        m_msgSendToClient.m_nFunction |= 1<<3;
    }
}

bool CCenterServerManage::OnSocketRead(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID)
{
	// 请求游戏全局参数
	if (pNetHead->bMainID == MDM_GP_REQURE_GAME_PARA)	
	{
		if (0 == uSize)
		{
			GetURL(NULL);
		}
		else
		{
			char * pBuf = (char*)pData;
			GetURL(pBuf);
		}

		// 负载均衡
		//RandAServer();
		
		m_TCPSocket.SendData(uIndex, &m_msgSendToClient, sizeof(CenterServerMsg), MDM_GP_REQURE_GAME_PARA, 0, 0, 0);
		return true;
	}
	return false;
}
/******************************************************************************************************/

// 构造函数
CCenterServerManage::CCenterServerManage(void) 
	:CBaseMainManageForWeb()
{
	GetINIFile();
}

// 析构函数 
CCenterServerManage::~CCenterServerManage(void)
{
}
// 停止服务
bool CCenterServerManage::OnStop()
{
	KillTimer(IDT_CHECK_DATA_CONNECT);
	m_ZServerManager.Stop();
	return true;
}

// 数据管理模块初始化
bool CCenterServerManage::OnInit(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData)
{
	return m_ZServerManager.Init(pInitData,NULL);
}
// 数据管理模块卸载
bool CCenterServerManage::OnUnInit()
{
	return m_ZServerManager.UnInit();
}

// 获取信息函数
bool CCenterServerManage::PreInitParameter(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData)
{
	//设置版本信息
	//pKernelData->bMaxVer=GAME_PLACE_MAX_VER;
	//pKernelData->bLessVer=GAME_PLACE_LESS_VER;

	// 设置使用数据库
	pKernelData->bLogonDataBase    = FALSE; //TRUE;
	pKernelData->bNativeDataBase   = FALSE; //TRUE;
	pKernelData->bStartSQLDataBase = FALSE; //TRUE;

	// 设置使用网络
	pKernelData->bStartTCPSocket = TRUE;
	pInitData->uListenPort       = CENTER_SERVER_PORT;
	pInitData->uMaxPeople        = 300;
	pInitData->iSocketSecretKey  = SECRET_KEY;

	CString info;
	info.Format("中心服务启动成功  Port:%d", CENTER_SERVER_PORT);

	OutputFun(info.GetBuffer ());

	return true;
}
// SOCKET 关闭
bool CCenterServerManage::OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, long int lConnectTime)
{
	return true;
}

bool CCenterServerManage::OnDataBaseResult(DataBaseResultLine * pResultData)
{
	return true;
}

void CCenterServerManage::RandAServer()
{
	m_ZServerManager.m_MainserverList.RandAServer (m_msgSendToClient.m_strMainserverIPAddr, m_msgSendToClient.m_iMainserverPort);
}

bool CCenterServerManage::CMainserverList::RandAServer(char* ipaddr,long& port)
{
	bool result = false;
	Lock();
	if(!m_List.IsEmpty())
	{
		// Z服务负载均衡，平均分配法
		static int curi = 0;

		// 随机选择一个数
		long t = GetCurrentTime() + rand();

		// 余数，发现取余数比乘百分百，然后取整数简单
		int i = 0; 

		POSITION pos=m_List.GetHeadPosition ();
		
		for(int j = 0; j < m_List.GetCount(); j++)
		{
			Node* nd = (Node*)m_List.GetNext(pos);
			if(nd)
			{				
				// 得到该随机数的IP发送到客户端
				if(i == curi)
				{
					CString s = nd->IPAddr ;
					port = nd->Port ;
					memcpy(ipaddr,s.GetBuffer(s.GetLength ()),20);
					result = true;
					break;
				}
			}
			i++;
		}

		curi++;
		if(curi >= m_List.GetCount()) 
			curi = 0;
	}
	UnLock();
	return result;
}


void CCenterServerManage::CMainserverList::clear()
{
	Lock();
	while(!m_List.IsEmpty())
	{
		Node* pdesk = (Node*)m_List.RemoveHead();
		delete pdesk;
	}
	m_List.RemoveAll();
	UnLock();
}

CCenterServerManage::CMainserverList::~CMainserverList()
{
	clear();
}

void CCenterServerManage::CMainserverList::LoadServerList(DWORD cfgHandle)
{
	clear();

	CString ss;
	for(int i = 0; i < 10; i++)
	{
		ss.Format("M_IPAddr%d",i+1);
		CString ip = cfgGetValue(cfgHandle,"GateServer",ss,"");
		if(ip == "")
		{
			break;
		}

		Node* nd = new Node();
		nd->IPAddr =ip;
		ss.Format("M_Port%d",i+1);
		nd->Port = cfgGetValue(cfgHandle,"GateServer",ss,0);
		ss.Format("M_id%d",i+1);
		nd->id = cfgGetValue(cfgHandle,"GateServer",ss,0);
		Lock();
		m_List.AddTail(nd);
		UnLock();
	}
}


//void CCenterServerManage::CMainserverList::ReadINIFile(CString TMLkey)
//{
//	clear();
//	CString s=CINIFile::GetAppPath ();
//	CINIFile f( s + "CenterServer.ini");
//	//int count=f.GetKeyVal ("www.TML.cn","MainserverListCount",0);//////主服务器个数
//	for(int i=0;i<10;i++)
//	{
//		CString s;
//		s.Format ("%d",i+1);
//		CString ip=f.GetKeyVal (TMLkey,"M_IPAddr"+s,"");
//		if (ip=="")break;
//		Node* nd=new Node();
//		nd->IPAddr =ip;
//		long port=f.GetKeyVal (TMLkey,"M_Port"+s,0);
//		nd->Port =port;
//		port=f.GetKeyVal (TMLkey,"M_id"+s,0);
//		nd->id  =port;
//		Lock();
//		  m_List.AddTail (nd);
//		UnLock();
//
////原来算法
//		//CString s;
//		//s.Format ("Mainserver%d",i+1);
//		//CString ip=f.GetKeyVal (s,"IPAddr","");
//		//Node* nd=new Node();
//		//nd->IPAddr =ip;
//		//long port=f.GetKeyVal (s,"Port",0);
//		//nd->Port =port;
//		//port=f.GetKeyVal (s,"id",0);
//		//nd->id  =port;
//		//Lock();
//		//  m_List.AddTail (nd);
//		//UnLock();
//
//	}
//
//}

CCenterServerManage::CMainserverList::CMainserverList()
{
	///srand( (unsigned)time( NULL ) );
	///InitializeCriticalSection(&cs);
   /// ReadINIFile();
}
/******************************************************************************************************/

CCenterServerModule::CCenterServerModule(void) 
{

}

CCenterServerModule::~CCenterServerModule(void)
{

}

bool CCenterServerModule::InitService(ManageInfoStruct * pInitData)
{
	try
	{
		return m_LogonManage.Init(pInitData,NULL);
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

// 卸载函数
bool CCenterServerModule::UnInitService()
{
	try
	{
		return m_LogonManage.UnInit();
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

// 开始函数 
bool CCenterServerModule::StartService(UINT &errCode)
{
	try
	{
		return m_LogonManage.Start();
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

// 停止函数 
bool CCenterServerModule::StoptService()
{
	try
	{
		return m_LogonManage.Stop();
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

// 删除函数
bool CCenterServerModule::DeleteService()
{
	try	
	{ 
		delete this;
	}
	catch (...) 
	{

	}
	return true;
}

/******************************************************************************************************/
