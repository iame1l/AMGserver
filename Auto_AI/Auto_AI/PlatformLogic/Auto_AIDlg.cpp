// Auto_AIDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Auto_AI.h"
#include "Auto_AIDlg.h"
//#include "afcfunction.h"
#include "iphlpapi.h"	//锁定机器 zxj 2009-11-12
#include "Platconfig.h"
#include "Platglobaldata.h"
#include "PlatLog.h"
#include "Platlock.h"
#include "DiaogTaskList.h"
#include "DialogMsgInfo.h"

#pragma comment(lib,"Iphlpapi.lib")	//锁定机器 zxj 2009-11-12
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define COL_USRNAME 0
#define COL_NICKNAME 1
#define COL_GSERVERLOGIN 2
#define COL_ROOMID 3
#define COL_DESKNO 4
#define COL_DESKSTATION 5
#define COL_USERSTATE 6
#define COL_TASKINFO 7

// CAuto_AIDlg 对话框


CAuto_AIDlg::CAuto_AIDlg( CWnd * pParent)
: CBaseRoom(IDD_AUTO_AI_DIALOG)//Dialog(CAuto_AIDlg::IDD, pParent)
{
    m_ShowBeginNo = "";
    m_ShowEndNo = "";
    m_bExit = false;

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_iLogonIndex = -2;

    PLATLOG->Init();
	//初始化 SOCKET
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return;
	}
}

CAuto_AIDlg::~CAuto_AIDlg()
{

    PLATLOG->UnInit();
	::WSACleanup();
}

void CAuto_AIDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_STATUS, m_ListUserStatus);
}

BEGIN_MESSAGE_MAP(CAuto_AIDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()

	ON_WM_CREATE()
	ON_MESSAGE(IDM_CLOSE_GAME_ROOM,OnCloseGameRoom)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()

    ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CAuto_AIDlg::OnBnClickedButtonLogin)
    ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CAuto_AIDlg::OnBnClickedButtonCancel)
    ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CAuto_AIDlg::OnBnClickedButtonClose)
    ON_BN_CLICKED(IDC_BUTTON_MIN, &CAuto_AIDlg::OnBnClickedButtonMin)
    ON_BN_CLICKED(IDC_BUTTON_SAVE, &CAuto_AIDlg::OnBnClickedButtonSave)
    ON_BN_CLICKED(IDC_BUTTON_RESET, &CAuto_AIDlg::OnBnClickedButtonReset)
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_BUTTON_TASKLIST, &CAuto_AIDlg::OnBnClickedButtonTasklist)
    ON_BN_CLICKED(IDC_BUTTON_HALLINFO, &CAuto_AIDlg::OnBnClickedButtonHallinfo)
    ON_BN_CLICKED(IDC_BUTTON_ROOMINFO, &CAuto_AIDlg::OnBnClickedButtonRoominfo)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_STATUS, &CAuto_AIDlg::OnLvnColumnclickListStatus)
    ON_BN_CLICKED(IDC_BUTTON_ADDTASK, &CAuto_AIDlg::OnBnClickedButtonAddtask)
END_MESSAGE_MAP()


// CAuto_AIDlg 消息处理程序

BOOL CAuto_AIDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	::SetWindowLong(m_hWnd,GWL_STYLE,GetWindowLong(m_hWnd,GWL_STYLE)|WS_MINIMIZEBOX);

    //建立树型例表
    GLOBALDATA->GameList.Create(TVS_HASBUTTONS | TVS_HASLINES | TVS_DISABLEDRAGDROP | TVS_TRACKSELECT | WS_VISIBLE | WS_TABSTOP,  CRect(0, 0,200,200), this, 200);
    GLOBALDATA->GameList.ShowWindow(SW_HIDE);
    GLOBALDATA->GameList.Init();

    {
        AutoPlatLock lock(&PLATCONFIG->m_DynamicSection);
        auto it = PLATCONFIG->m_DynamicConfigs.begin();
        while(it != PLATCONFIG->m_DynamicConfigs.end())
        {
            CString errorMsg;
            if(!it->second.IsValid(errorMsg))
            {
                MSERVER_LOG_ERROR("任务%s错误：%s", it->second.strTime, errorMsg);
                CString ErrorTip;
                ErrorTip.Format("任务%s错误:%s", it->second.strTime, errorMsg);
                MessageBox(ErrorTip);
                PostQuitMessage(0);
                return false;
            }
            it++;
        }
    }
    

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图

    SetWindowText(PLATCONFIG->strTitleText);

    // 展示静态参数
    GetDlgItem(IDC_STATIC_TILEHEAD)->SetWindowText(PLATCONFIG->strTitleText);
    CString strStaticTip = _T("");
    CString strTmp = _T("");
    strStaticTip.Format(_T("%s游戏类别:%d\r\n\r\n"), strTmp, PLATCONFIG->KindID);
    strTmp = strStaticTip;
    strStaticTip.Format(_T("%s游戏ID  :%d\r\n\r\n"), strTmp, PLATCONFIG->NameID);
    strTmp = strStaticTip;
    strStaticTip.Format(_T("%s房间ID  :%d\r\n\r\n"), strTmp, PLATCONFIG->RoomID);
    strTmp = strStaticTip;
    strStaticTip.Format(_T("%s中心地址:%s:%d\r\n\r\n"), strTmp, PLATCONFIG->CenterServerIPAddr, PLATCONFIG->CenterServerPort);
    strTmp = strStaticTip;
    strStaticTip.Format(_T("%s登陆密码:%s\r\n\r\n"), strTmp, PLATCONFIG->strAIPWD);
    strTmp = strStaticTip;
    if(PLATCONFIG->bFishGame)
        strStaticTip.Format(_T("%s捕鱼类游戏:是\r\n\r\n"), strTmp);
    else
        strStaticTip.Format(_T("%s捕鱼类游戏:否\r\n\r\n"), strTmp);
    strTmp = strStaticTip;
    SYSTEMTIME lTime;
    GetLocalTime(&lTime);
    strStaticTip.Format(_T("%s启动时间:%04d-%02d-%02d %02d:%02d:%02d\r\n\r\n"), strTmp, lTime.wYear, lTime.wMonth, lTime.wDay, lTime.wHour, lTime.wMinute, lTime.wSecond);
    strTmp = strStaticTip;
    GetDlgItem(IDC_EDIT_STATIC)->SetWindowText(strStaticTip);

    ((CEdit*)GetDlgItem(IDC_EDIT_STATIC))->SetSel(-1,0);

    if(NextTask())
    {
        // 展示动态参数
        ResetConfig();
    }

    MSERVER_LOG_INFO("启动机器人%s",PLATCONFIG->strTitleText);
    ((CButton*)GetDlgItem(IDC_BUTTON_SAVE))->EnableWindow(FALSE);
    ((CButton*)GetDlgItem(IDC_BUTTON_RESET))->EnableWindow(FALSE);
    ((CButton*)GetDlgItem(IDC_BUTTON_CANCEL))->EnableWindow(FALSE);

    SetTimer(TIMER_CHECK_MODIFY_PARA, 10*1000, NULL);

    DWORD dwStyle = m_ListUserStatus.GetExtendedStyle();
    dwStyle |= LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES;
    m_ListUserStatus.SetExtendedStyle(dwStyle);
    m_ListUserStatus.ModifyStyle(0, LVS_REPORT);
    m_ListUserStatus.InsertColumn(COL_USRNAME, _T("用户名"), LVCFMT_LEFT,120);
    m_ListUserStatus.InsertColumn(COL_NICKNAME, _T("昵称"), LVCFMT_LEFT,120);
    m_ListUserStatus.InsertColumn(COL_GSERVERLOGIN, _T("登陆"), LVCFMT_LEFT,60);
    m_ListUserStatus.InsertColumn(COL_ROOMID, _T("房间ID"), LVCFMT_LEFT,60);
    m_ListUserStatus.InsertColumn(COL_DESKNO, _T("桌子号"), LVCFMT_LEFT,60);
    m_ListUserStatus.InsertColumn(COL_DESKSTATION, _T("座位号"), LVCFMT_LEFT,60);
    m_ListUserStatus.InsertColumn(COL_USERSTATE, _T("游戏状态"), LVCFMT_LEFT,60);
    m_ListUserStatus.InsertColumn(COL_TASKINFO, _T("任务信息"), LVCFMT_LEFT,300);
    ShowUserStatus();
    SetTimer(TIMER_UPDATE_USERSTATUS, 5000, NULL);

	int argc = __argc;
	if(argc >= 2)
	{//添加命令行参数 1 方便脚本启动时登陆
		CString str = __argv[1];	
		if(str == "1")
		{
			OnBnClickedButtonLogin();
		}
	}
	GLOBALDATA->bLoginEnable = false;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CAuto_AIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CAuto_AIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAuto_AIDlg::OnOK()
{

}
void CAuto_AIDlg::OnCancel()
{
    if(ModifyPara())
    {
        CString strTip;
        strTip.Format(_T("是否保存修改?"));
        if(IDOK == MessageBox(strTip, _T("参数修改"), MB_OKCANCEL))
        {
            SaveConfig();
        }
    }
    CString strTip;
    strTip.Format(_T("确认退出%s?"), PLATCONFIG->strTitleText);
    if(IDOK == MessageBox(strTip, _T("退出"), MB_OKCANCEL))
    {
        MSERVER_LOG_INFO("关闭机器人%s",PLATCONFIG->strTitleText);
        // TODO: 在此添加控件通知处理程序代码\
        //add by 20090211zht修改关闭时先将正在游戏中的机器人退出
        bool bExit = true;
        for (int i=0; i< GLOBALDATA->CurDynamicConfig.nLogonCount; i++)
        {
            if (GLOBALDATA->RoomInfo[i].pGameRoomWnd != NULL)
            {
                bExit = false;
                ::PostMessage(GLOBALDATA->RoomInfo[i].pGameRoomWnd->GetSafeHwnd(),IDM_CLOSE,0,0);
            }
        }
        KillTimer(TIMER_CHECK_MODIFY_PARA);
        m_bExit = true;
        if(bExit) PostQuitMessage(0);
        //add by 20090211zht修改关闭时先将正在游戏中的机器人退出
    }
}

void	CAuto_AIDlg::OnStartRobot()
{
    InitUI(false);
    MSERVER_LOG_INFO("StartRobot");
    ((CButton*)GetDlgItem(IDC_BUTTON_LOGIN))->EnableWindow(FALSE);
    ((CButton*)GetDlgItem(IDC_BUTTON_CANCEL))->EnableWindow(TRUE);
    GLOBALDATA->bConnectToWitchServer = 0;

    m_ShowBeginNo.Format("%s%d", GLOBALDATA->CurDynamicConfig.strRobotPreName, GLOBALDATA->CurDynamicConfig.nBeginSequenceNo);
    m_ShowEndNo.Format("%s%d", GLOBALDATA->CurDynamicConfig.strRobotPreName, GLOBALDATA->CurDynamicConfig.nBeginSequenceNo + 10);

    ((CEdit*)GetDlgItem(IDC_EDIT_CHECKBEGINNO))->SetWindowText(m_ShowBeginNo);
    ((CEdit*)GetDlgItem(IDC_EDIT_CHECKENDNO))->SetWindowText(m_ShowEndNo);

    OnConnectToCenterServer();
    SetTimer(TIMER_CHECK_UNCONNECT_ACCOUNT, 5000, NULL);
}

void CAuto_AIDlg::OnStopRobot()
{
    CString strTip;
    strTip.Format(_T("确认取消登陆%s?"), PLATCONFIG->strTitleText);
    if(IDOK == MessageBox(strTip, _T("退出"), MB_OKCANCEL))
    {
        MSERVER_LOG_INFO("StopRobot");
        m_TCPSocket->CloseSocket(TRUE);
        ((CButton*)GetDlgItem(IDC_BUTTON_LOGIN))->EnableWindow(TRUE);
        ((CButton*)GetDlgItem(IDC_BUTTON_CANCEL))->EnableWindow(FALSE);
        KillTimer(TIMER_CHECK_UNCONNECT_ACCOUNT);
        m_iLogonIndex = -2;
        GLOBALDATA->ReInit();
        UIDATA->ReInit();
        // TODO: 在此添加控件通知处理程序代码\
        //add by 20090211zht修改关闭时先将正在游戏中的机器人退出
        for (int i=0; i < MAX_GAME_ROOM; i++)
        {
            if (GLOBALDATA->RoomInfo[i].pGameRoomWnd != NULL)
                ::PostMessage(GLOBALDATA->RoomInfo[i].pGameRoomWnd->GetSafeHwnd(),IDM_CLOSE,0,0);
        }

        GetDlgItem(IDC_STATIC_LOGINSTATUS)->SetWindowText("连接状态");
        //add by 20090211zht修改关闭时先将正在游戏中的机器人退出
        if(NextTask())
        {
            // 展示动态参数
            ResetConfig();
        }
        InitUI(true);
        UIQueueGame(true);
    }
}


//连接AServer
void CAuto_AIDlg::OnConnectToCenterServer()
{
    KillTimer(TIME_CONNECT_ASERVER);
    SetTimer(TIME_CONNECT_ASERVER, 1000, NULL);
}

void CAuto_AIDlg::DoOnConnectToCenterServer()
{
    if (m_TCPSocket != NULL)
    {
        delete m_TCPSocket;
    }
    m_TCPSocket = new CTCPClientSocket(this);
    CString str;
    str.Format("正在连接AServer %s:%d", PLATCONFIG->CenterServerIPAddr,PLATCONFIG->CenterServerPort);
    GetDlgItem(IDC_STATIC_LOGINSTATUS)->SetWindowText(str);
    if(m_TCPSocket->Connect(PLATCONFIG->CenterServerIPAddr,PLATCONFIG->CenterServerPort)==false)
    {// 连接失败,重新启动连接
        MSERVER_LOG_ERROR("连接AServer失败%s:%d", PLATCONFIG->CenterServerIPAddr, PLATCONFIG->CenterServerPort);
        CString str;
        str.Format("连接AServer失败 %s:%d", PLATCONFIG->CenterServerIPAddr,PLATCONFIG->CenterServerPort);
        GetDlgItem(IDC_STATIC_LOGINSTATUS)->SetWindowText(str);
        OnConnectToCenterServer();
    }
}

//连接MServer
void CAuto_AIDlg::OnConnectToLogonServer()
{

    KillTimer(TIME_CONNECT_MSERVER);
    SetTimer(TIME_CONNECT_MSERVER, 1000, NULL);
	
}

void CAuto_AIDlg::DoOnConnectToLogonServer()
{
    SetTimer(TIMER_PLC_CLOSE_SOCKET, 30*1000, NULL);
    if (m_TCPSocket != NULL)
    {
        delete m_TCPSocket;
    }
    m_TCPSocket = new CTCPClientSocket(this);
    CString str;
    str.Format("正在连接MServer %s:%d", GLOBALDATA->MainServerIPAddr,GLOBALDATA->MainServerPort);
    GetDlgItem(IDC_STATIC_LOGINSTATUS)->SetWindowText(str);
    if(m_TCPSocket->Connect(GLOBALDATA->MainServerIPAddr,GLOBALDATA->MainServerPort)==false)
    {// 连接失败,重新启动连接
        MSERVER_LOG_ERROR("连接MServer失败%s:%d", GLOBALDATA->MainServerIPAddr,GLOBALDATA->MainServerPort);
        CString str;
        str.Format("连接MServer失败 %s:%d", GLOBALDATA->MainServerIPAddr,GLOBALDATA->MainServerPort);
        GetDlgItem(IDC_STATIC_LOGINSTATUS)->SetWindowText(str);
    }
}


//接口函数 
//发送网络消息函数
int CAuto_AIDlg::SendGameData(BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode)
{
	return m_TCPSocket->SendData(bMainID,bAssistantID,bHandleCode);//处理主类型
}

//发送网络消息函数
int CAuto_AIDlg::SendGameData(void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode)
{
	return m_TCPSocket->SendData(pData,uBufLen,bMainID,bAssistantID,bHandleCode);
}

//网络读取消息
bool CAuto_AIDlg::OnSocketReadEvent(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	if(!GLOBALDATA->bLoginEnable) return true;
	switch (pNetHead->bMainID)
	{
	case MDM_CONNECT:	//连接消息 
		{
			return OnConnectMessage(pNetHead,pNetData,uDataSize,pClientSocket);
		}
	case MDM_GP_REQURE_GAME_PARA:	//中心服务全局参数//请求游戏全局参数
		{
            GLOBALDATA->bConnectToWitchServer = 1;
			CenterServerMsg * msg=(CenterServerMsg *)pNetData;
			
            MSERVER_LOG_INFO("%s:%d AServer应答全局参数信息 MServer:%s-%d", PLATCONFIG->CenterServerIPAddr, PLATCONFIG->CenterServerPort,msg->m_strMainserverIPAddr, msg->m_iMainserverPort);
            GLOBALDATA->MainServerIPAddr = msg->m_strMainserverIPAddr;
            GLOBALDATA->MainServerPort = msg->m_iMainserverPort;

			m_TCPSocket->CloseSocket (true);		

			return true;
		}
	case MDM_GP_LOGON:	//登陆信息
		{
			return OnLogonMessage(pNetHead,pNetData,uDataSize,pClientSocket);
		}
	case MDM_GP_LIST:	//游戏列表
		{
			return OnListMessage(pNetHead,pNetData,uDataSize,pClientSocket);
		}
	case MDM_GP_CONTEST:
		{
			if (!EnterContestRoom(pNetHead,pNetData,uDataSize,pClientSocket))
			{
                m_TCPSocket->CloseSocket(true);
                return false;
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
                    break;
                }
            }
            {
                MSERVER_LOG_INFO("%s 未处理消息 %d:%d", m_CurPlaceUserInfo.szName, pNetHead->bMainID, pNetHead->bAssistantID);
                m_notHandleMsgs[strKey] = sysTime.wHour;
            }
        }
        
        break;
	}
	return true;
}

//网络连接消息
bool CAuto_AIDlg::OnSocketConnectEvent(UINT uErrorCode, CTCPClientSocket * pClientSocket)
{
	if(GLOBALDATA->bConnectToWitchServer == 0)//中心服务器
	{
        if(uErrorCode == 0)
        {
            MSERVER_LOG_INFO("连接AServer事件%s:%d-%d", PLATCONFIG->CenterServerIPAddr, PLATCONFIG->CenterServerPort,uErrorCode);
            CString str;
            str.Format("连接AServer成功 %s:%d", PLATCONFIG->CenterServerIPAddr, PLATCONFIG->CenterServerPort);
            GetDlgItem(IDC_STATIC_LOGINSTATUS)->SetWindowText(str);
        }
        else
        {
            MSERVER_LOG_ERROR("连接AServer事件%s:%d-%d", PLATCONFIG->CenterServerIPAddr, PLATCONFIG->CenterServerPort,uErrorCode);
            CString str;
            str.Format("连接AServer失败 %s:%d-%d", PLATCONFIG->CenterServerIPAddr, PLATCONFIG->CenterServerPort,uErrorCode);
            GetDlgItem(IDC_STATIC_LOGINSTATUS)->SetWindowText(str);
            OnConnectToCenterServer();
        }
	}
	else if (GLOBALDATA->bConnectToWitchServer ==1)
	{
        if(uErrorCode == 0)
        {
            MSERVER_LOG_INFO("连接MServer事件%s:%d-%d", GLOBALDATA->MainServerIPAddr, GLOBALDATA->MainServerPort,uErrorCode);
            CString str;
            str.Format("连接MServer成功 %s:%d", GLOBALDATA->MainServerIPAddr, GLOBALDATA->MainServerPort);
            GetDlgItem(IDC_STATIC_LOGINSTATUS)->SetWindowText(str);
        }
        else
        {
            MSERVER_LOG_ERROR("连接MServer事件%s:%d-%d", GLOBALDATA->MainServerIPAddr, GLOBALDATA->MainServerPort,uErrorCode);
            CString str;
            str.Format("连接MServer失败 %s:%d-%d", GLOBALDATA->MainServerIPAddr, GLOBALDATA->MainServerPort,uErrorCode);
            GetDlgItem(IDC_STATIC_LOGINSTATUS)->SetWindowText(str);
            OnConnectToLogonServer();
        }
	}
    else
    {
        MSERVER_LOG_ERROR("未知连接事件");
    }
	return true;
}

//网络关闭消息
bool CAuto_AIDlg::OnSocketCloseEvent()
{    
    {
        if(GLOBALDATA->bConnectToWitchServer == 0)
        {
            OnConnectToCenterServer();
        }
        m_iLogonIndex = -1;
    }
	return true;
}

//连接消息处理
bool CAuto_AIDlg::OnConnectMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	if (pNetHead->bAssistantID == ASS_NET_TEST)	//网络测试信息
	{
		pClientSocket->SendData(MDM_CONNECT,ASS_NET_TEST,0);//连接消息类型
		return true;
	}
	else if ((pNetHead->bAssistantID==2)||(pNetHead->bAssistantID == 3))	//连接成功
	{
		MSG_S_ConnectSuccess * _p = (MSG_S_ConnectSuccess *)pNetData;
		if (_p != NULL)
		{
			pClientSocket->SetCheckCode(_p->i64CheckCode, SECRET_KEY);
		}

		if(GLOBALDATA->bConnectToWitchServer == 0)//中心服务器
		{
            MSERVER_LOG_INFO("向AServer请求全局参数信息%s:%d", PLATCONFIG->CenterServerIPAddr, PLATCONFIG->CenterServerPort);
			m_TCPSocket->SendData(NULL,0,MDM_GP_REQURE_GAME_PARA,0,0);
			return true;
		}
		LoginMServer();
		return true;
	}
    else
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
        {
            MSERVER_LOG_INFO("%s 未处理消息 %d:%d", m_CurPlaceUserInfo.szName, pNetHead->bMainID, pNetHead->bAssistantID);
            m_notHandleMsgs[strKey] = sysTime.wHour;
        }
    }
	return false;
}

//获取机器码相关函数 zxj 2009-11-12 锁定机器
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

//获取机器码相关函数 zxj 2009-11-12 锁定机器
CString coreGetCode()
{
	CString strRet="";
	CString str=innerGetMac(),s;
	if(str=="")
		return "未检测到系统中有Ethernet设备。";

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

//登陆消息处理
bool CAuto_AIDlg::OnLogonMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	if (pNetHead->bAssistantID==ASS_GP_LOGON_SUCCESS)		//登陆成功
	{
		//设置信息
		m_CurPlaceUserInfo = *((MSG_GP_R_LogonResult *)pNetData);

		wsprintf(m_CurPlaceUserInfo.szMD5Pass,"%s",PLATCONFIG->strAIPWD);

        MSERVER_LOG_INFO("%s 登陆成功", m_CurPlaceUserInfo.szName);
        {

            AutoPlatLock lock(&UIDATA->uiSection);
            auto it = UIDATA->usrInfos.find(m_CurPlaceUserInfo.szName);
            if(it != UIDATA->usrInfos.end())
            {
                it->second.strUserName = m_CurPlaceUserInfo.szName;
                it->second.strNickName = m_CurPlaceUserInfo.nickName;
            }
            else
            {
                MSERVER_LOG_ERROR("%s 没有找到UI信息", m_CurPlaceUserInfo.szName);
            }
        }

        // 先进行一次银行存取操作
        AiControlBank();

		//获取游戏列表
		return GetGameRoomList();
	}
	else if (pNetHead->bAssistantID==ASS_GP_LOGON_ERROR)	//登陆失败
	{
		//显示错误信息
		CString strMessage;
		switch (pNetHead->bHandleCode)
		{
		case ERR_GP_USER_NO_FIND:
			{
				strMessage=TEXT("很抱歉，错误帐号！");
				break;
			}
		case ERR_GP_USER_PASS_ERROR:
			{
				strMessage=TEXT("很抱歉，密码错误！");
				break;
			}
		case ERR_GP_USER_VALIDATA:
			{
				strMessage=TEXT("此帐号已被禁止登录！");
				break;
			}
		case ERR_GP_USER_IP_LIMITED:
			{
				strMessage=TEXT("你所在的 IP 地址被禁止登陆！");
				break;
			}
		case ERR_GP_USER_EXIST:
			{
				strMessage=TEXT("很抱歉，这个用户名已经有人注册了！");
				break;
			}
		case ERR_GP_PASS_LIMITED:
			{
				strMessage=TEXT("输入的密码错误！");
				break;
			}
		case ERR_GP_IP_NO_ORDER:
			{
				strMessage=TEXT("你需要在指定的 IP 地址的系统登陆！");
				break;
			}
		case ERR_GP_USER_LOGON:
			{
				strMessage=TEXT("此帐号已经登录！");
				break;
			}
		case ERR_GP_USERNICK_EXIST:
			{
				strMessage=TEXT("此昵称已经存在！");
				break;
			}		
		default:strMessage=TEXT("系统登录出现错误！请与管理员联系！");
		}

        char  szLogonName[61] = {0};
        sprintf(szLogonName, "%s", m_CurPlaceUserInfo.szName);
        MSERVER_LOG_ERROR("%s 登陆失败%d %s", szLogonName, pNetHead->bHandleCode, strMessage);
	
		m_TCPSocket->CloseSocket(true);
		return true;
	}
    else
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
        MSERVER_LOG_INFO("%s 未处理消息 %d:%d", m_CurPlaceUserInfo.szName, pNetHead->bMainID, pNetHead->bAssistantID);
        m_notHandleMsgs[strKey] = sysTime.wHour;
    }
	return true;
}

//获取游戏列表
bool CAuto_AIDlg::GetGameRoomList()
{
    MSERVER_LOG_INFO("%s 请求游戏列表", m_CurPlaceUserInfo.szName);
	m_TCPSocket->SendData(MDM_GP_LIST,ASS_GP_LIST_KIND,0);//请求游戏列表
	return true;
}

//列表信息
bool CAuto_AIDlg::OnListMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	switch (pNetHead->bAssistantID)
	{
	case ASS_GP_LIST_KIND:		//获取的是游戏类型
		{
            MSERVER_LOG_INFO("%s 成功获取游戏类型ASS_GP_LIST_KIND", m_CurPlaceUserInfo.szName);
			//处理数据
			GLOBALDATA->GameList.HandleKindData((ComKindInfo *)pNetData,uDataSize/sizeof(ComKindInfo));//在树上加入游戏类型
			return true;
		}
	case ASS_GP_LIST_NAME:		//获取的游戏名字
		{
            MSERVER_LOG_INFO("%s 成功获取游戏名字ASS_GP_LIST_NAME", m_CurPlaceUserInfo.szName);
			//处理数据
			GLOBALDATA->GameList.HandleNameData((ComNameInfo *)pNetData,uDataSize/sizeof(ComNameInfo));

			//更新提示
			if (pNetHead->bHandleCode==ERR_GP_LIST_FINISH) //发送游戏类型列表结束标志
			{
				MSG_GP_SR_GetRoomStruct GetList;
				GetList.uKindID = PLATCONFIG->KindID;
				GetList.uNameID = PLATCONFIG->NameID;
				m_TCPSocket->SendData(&GetList,sizeof(GetList),MDM_GP_LIST,ASS_GP_LIST_ROOM,0);
			}
			return true;
		}
	case ASS_GP_LIST_ROOM:		//获取的游戏房间
		{
			//效验数据 
			if (uDataSize<sizeof(MSG_GP_SR_GetRoomStruct))
            {
                MSERVER_LOG_ERROR("%s MSG_GP_SR_GetRoomStruct 结构体不一致", m_CurPlaceUserInfo.szName);
				return false;
            }
            MSERVER_LOG_INFO("%s 成功获取房间信息ASS_GP_LIST_ROOM", m_CurPlaceUserInfo.szName);
			MSG_GP_SR_GetRoomStruct * pNetRoomBuf=(MSG_GP_SR_GetRoomStruct *)pNetData;
			//处理数据
			GLOBALDATA->GameList.HandleRoomData((ComRoomInfo *)((char *)pNetData+sizeof(MSG_GP_SR_GetRoomStruct)),
				(uDataSize-sizeof(MSG_GP_SR_GetRoomStruct))/sizeof(ComRoomInfo),pNetRoomBuf->uKindID,pNetRoomBuf->uNameID);

			if (pNetHead->bHandleCode == ERR_GP_LIST_FINISH) //发送游戏列表结束标志
			{
				//增加比赛房间判断
				CAFCRoomItem *pGameRoomItem = GLOBALDATA->GameList.FindRoomItem(PLATCONFIG->RoomID);
				if (NULL == pGameRoomItem)
				{
                    MSERVER_LOG_ERROR("%s 未获取到指定房间信息", m_CurPlaceUserInfo.szName);
					return false;
				}

                {
                    AutoPlatLock lock(&UIDATA->uiSection);
                    UIDATA->roomInfo = pGameRoomItem->m_RoomInfo;
                    GetDlgItem(IDC_STATIC_ROOMNAME)->SetWindowText(pGameRoomItem->m_RoomInfo.szGameRoomName);
                }

                if (IsContestGame(pGameRoomItem->m_RoomInfo.dwRoomRule))
                {
                    UIQueueGame(false);
                    GetContestRoomID(pGameRoomItem->m_RoomInfo.uRoomID);
                }
                else
                {
                    if((pGameRoomItem->m_RoomInfo.dwRoomRule &GRR_QUEUE_GAME) != 0)
                    {
                        UIQueueGame(false);
                    }
                    else
                    {
                        UIQueueGame(true);
                    }
                    EnterGameRoom(pGameRoomItem->m_RoomInfo.uRoomID);
                }
			}
			return true;
		}
	case ASS_GP_LIST_COUNT:		//在线人数
		{
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
                    break;;
                }
            }
            MSERVER_LOG_INFO("%s 未处理消息 %d:%d", m_CurPlaceUserInfo.szName, pNetHead->bMainID, pNetHead->bAssistantID);
            m_notHandleMsgs[strKey] = sysTime.wHour;
        }
        break;
	}
	return TRUE;
}

//计时器
void CAuto_AIDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case TIME_CONNECT_ASERVER:
		{
            KillTimer(nIDEvent);
			DoOnConnectToCenterServer();
			break;
		}
    case TIME_CONNECT_MSERVER:
        {
            KillTimer(nIDEvent);
            DoOnConnectToLogonServer();
            break;
        }
    case TIMER_CHECK_UNCONNECT_ACCOUNT:
        {
            if(m_iLogonIndex != -1 || GLOBALDATA->bConnectToWitchServer == 0 || m_iLogonIndex == -2) break;
            m_iLogonIndex = FindNextLogonIndex();
            if(m_iLogonIndex != -1)
            {// 有帐号需要登陆
                OnConnectToLogonServer();
            }
        }
        break;
    case TIMER_CHECK_MODIFY_PARA:
        if(ModifyPara())
        {
            ((CButton*)GetDlgItem(IDC_BUTTON_SAVE))->EnableWindow(TRUE);
            ((CButton*)GetDlgItem(IDC_BUTTON_RESET))->EnableWindow(TRUE);
        }
        else
        {
            ((CButton*)GetDlgItem(IDC_BUTTON_SAVE))->EnableWindow(FALSE);
            ((CButton*)GetDlgItem(IDC_BUTTON_RESET))->EnableWindow(FALSE);
        }
        break;
    case TIMER_UPDATE_USERSTATUS:
        if(((CButton*)GetDlgItem(IDC_CHECK_STOPSTATUS))->GetCheck()) return;
        ShowUserStatus();
        break;
    case TIMER_SORT_USERSTATUS:
        KillTimer(nIDEvent);
        m_ListUserStatus.SortItems(CompareProc, (DWORD_PTR)this);//排序第二个参数是比较函数的第三个参数
        break;
    case TIMER_PLC_CLOSE_SOCKET:
        {
            KillTimer(nIDEvent);
            if (NULL != m_TCPSocket)
            {
                //获取大厅数据超时，断开连接
                m_TCPSocket->CloseSocket(true);
                OnConnectToLogonServer();
            }
        }
        break;
    default:
        MSERVER_LOG_ERROR("未处理定时器 %d", nIDEvent);
        break;
	}
	return __super::OnTimer(nIDEvent);
}

//关闭房间请求
LRESULT CAuto_AIDlg::OnCloseGameRoom(WPARAM wparam, LPARAM lparam)
{
	RoomInfoStruct * pRoomInfo=(RoomInfoStruct *)wparam;
	if ((pRoomInfo!=NULL)&&(pRoomInfo->bAccess)&&(pRoomInfo->pGameRoomWnd!=NULL)&&(IsWindow(pRoomInfo->pGameRoomWnd->GetSafeHwnd())))
	{
		
		for (int i = 0; i < MAX_GAME_ROOM; i++)
		{
			if (GLOBALDATA->Robots[i].iUserID == lparam)
			{
				GLOBALDATA->Robots[i].pRoomItem = NULL;
                break;
			}
		}
		delete pRoomInfo->pGameRoomWnd;
		pRoomInfo->pGameRoomWnd = NULL;
		memset(pRoomInfo,0,sizeof(RoomInfoStruct));
	}
    if(!m_bExit) return 0;
    for (int i = 0; i < MAX_GAME_ROOM; i++)
    {
        if(GLOBALDATA->Robots[i].pRoomItem != NULL)
        {
            return 0;
        }
        else
        {
            if(i + 1 ==  MAX_GAME_ROOM)
            {
                m_bExit = false;
                PostQuitMessage(0);
            }
        }
    }
	return 0;
}

bool CAuto_AIDlg::EnterContestRoom(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
    if(pNetHead->bAssistantID != ASS_GP_GET_CONTEST_ROOMID)
    {
        return true;
    }
	if (0 != pNetHead->bHandleCode)
	{
		MSERVER_LOG_INFO("%s 获取比赛房间信息错误 %d", m_CurPlaceUserInfo.szName, pNetHead->bHandleCode);
		return false;
	}

    if(uDataSize != sizeof(MSG_GP_GetContestRoomID_Result))
    {
        MSERVER_LOG_ERROR("MSG_GP_GetContestRoomID_Result 结构体不一致");
        return false;
    }

	MSG_GP_GetContestRoomID_Result *pContestRoom = (MSG_GP_GetContestRoomID_Result*)pNetData;

	EnterGameRoom(pContestRoom->tRoomInfo.uRoomID);
	return true;
}
//
BOOL CAuto_AIDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	return CBaseRoom::PreTranslateMessage(pMsg);
}

int CAuto_AIDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBaseRoom::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}

void CAuto_AIDlg::LoginMServer()
{
	CString LogonName;
	LogonName.Format("%s", m_CurPlaceUserInfo.szName);

	int rNum=0;
	CString TML_SN="";
	CString allword="0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";
	for (int i = 0;i <= 120; i ++)
	{
		rNum =(rand() * 1000)% 62;
		TML_SN+=allword[rNum];
	}
	TML_SN += "";

	CString MatchineCode= coreGetCode();	//取得本机机器码 zxj 2009-11-12 锁定机器
	

	MSG_GP_S_LogonByNameStruct LogonByName;///定义一个结构
	::memset(&LogonByName,0,sizeof(LogonByName));
	//LogonByName.uRoomVer=GAME_PLACE_MAX_VER;//结构的版本
	lstrcpy(LogonByName.TML_SN,TML_SN);
	lstrcpy(LogonByName.szName, LogonName);//结构的用户名
	wsprintf(LogonByName.szMD5Pass,"%s",PLATCONFIG->strAIPWD);
/*	lstrcpy(LogonByName.szMathineCode, MatchineCode);*/	//锁定机器，把本机机器码传递到登录数据包里 zxj 2009-11-12
	//LogonByName.gsqPs = 5471;
	m_TCPSocket->SendData(&LogonByName,sizeof(LogonByName),MDM_GP_LOGON,ASS_GP_LOGON_BY_NAME,0);
    MSERVER_LOG_INFO("%s 登陆Mserver", LogonName);

    
    {
        AutoPlatLock lock(&UIDATA->uiSection);
        auto it = UIDATA->usrInfos.find(LogonByName.szName);
        if(it != UIDATA->usrInfos.end())
        {
            it->second.cof = GLOBALDATA->CurDynamicConfig;
        }
        else
        {
            UserItemUIInfo itemInfo;
            itemInfo.cof = GLOBALDATA->CurDynamicConfig;
            UIDATA->usrInfos[LogonByName.szName] = itemInfo;
        }
    }
}

//获取合适的比赛房间ID
void CAuto_AIDlg::GetContestRoomID(int roomid)
{
	CAFCRoomItem *pGameRoomItem = GLOBALDATA->GameList.FindRoomItem(roomid);
	if (!IsContestGame(pGameRoomItem->m_RoomInfo.dwRoomRule))
	{
        MSERVER_LOG_ERROR("%s 非比赛房间信息%d", m_CurPlaceUserInfo.szName, roomid);
		return;
	}
    MSERVER_LOG_INFO("%s 获取比赛房间信息 %d", m_CurPlaceUserInfo.szName, pGameRoomItem->m_RoomInfo.iContestID);
	MSG_GP_GetContestRoomID getContestRoomID;
	getContestRoomID.iUserID = m_CurPlaceUserInfo.dwUserID;
	getContestRoomID.iContestID = pGameRoomItem->m_RoomInfo.iContestID;
	m_TCPSocket->SendData(&getContestRoomID, sizeof(MSG_GP_GetContestRoomID), MDM_GP_CONTEST, ASS_GP_GET_CONTEST_ROOMID, 0);

}

void CAuto_AIDlg::EnterGameRoom(int roomid)
{
	CAFCRoomItem *pGameRoomItem = GLOBALDATA->GameList.FindRoomItem(roomid);

	if (pGameRoomItem == NULL) 
	{
		MSERVER_LOG_ERROR("%s 获取不到房间信息%d", m_CurPlaceUserInfo.szName, roomid);
        m_TCPSocket->CloseSocket(true);
		return;
	}
    

    MSERVER_LOG_INFO("%s 建立GameRoomEx", m_CurPlaceUserInfo.szName);

	//判断是否特殊房间
	ComRoomInfo * pComRoomInfo=&pGameRoomItem->m_RoomInfo;
	if (((pComRoomInfo->uComType!=TY_NORMAL_GAME)&&(pComRoomInfo->uComType!=TY_MATCH_GAME)
		&&(pComRoomInfo->uComType!=TY_MONEY_GAME)))
	{
        MSERVER_LOG_ERROR("%s 不支持房间%d", m_CurPlaceUserInfo.szName, roomid);
		m_TCPSocket->CloseSocket(true);
		AfxGetApp( )->m_pMainWnd->DestroyWindow ();
		return;		
	}	
	//判断游戏是否安装并效验版本
	TCHAR szProessName[31],szGameName[61];
	GLOBALDATA->GameList.GetGameName(pGameRoomItem,szGameName,sizeof(szGameName));
	GLOBALDATA->GameList.GetProcessName(pGameRoomItem,szProessName,sizeof(szProessName));


	//进入游戏大厅
    int index = m_iLogonIndex;
	RoomInfoStruct *pRoomInfoItem=&GLOBALDATA->RoomInfo[index];

	try
	{
        {
            AutoPlatLock lock(&UIDATA->uiSection);
            auto it = UIDATA->usrInfos.find(m_CurPlaceUserInfo.szName);
            if(it != UIDATA->usrInfos.end())
            {
                it->second.roomID = roomid;
            }
            else
            {
                MSERVER_LOG_ERROR("%s 没有找到UI信息", m_CurPlaceUserInfo.szName);
            }
        }
		pRoomInfoItem->bAccess=true;
		pRoomInfoItem->stComRoomInfo=*pComRoomInfo;
		lstrcpy(pRoomInfoItem->szProcessName,szProessName);

		if (pRoomInfoItem->pGameRoomWnd != NULL)
		{
			delete pRoomInfoItem->pGameRoomWnd;
			pRoomInfoItem->pGameRoomWnd = NULL;
		}
		pRoomInfoItem->pGameRoomWnd=new CGameRoomEx(szGameName,pRoomInfoItem,&m_CurPlaceUserInfo,GLOBALDATA->CurDynamicConfig);

		CGameRoomEx * pGameRoom = (CGameRoomEx *)pRoomInfoItem->pGameRoomWnd;

        if(index >= 0 && index < MAX_GAME_ROOM)
        {
		    GLOBALDATA->Robots[index].pRoomItem = pRoomInfoItem;
		    GLOBALDATA->Robots[index].iUserID = m_CurPlaceUserInfo.dwUserID;
        }
        else
        {
            MSERVER_LOG_ERROR("不合法索引%d", index);
            throw TEXT("建立A新组件失败");
        }

		if (pRoomInfoItem->pGameRoomWnd->Create(IDD_GAME_ROOM,this)==FALSE) 
			throw TEXT("建立A新组件失败");
	}
	catch (...)
	{
        MSERVER_LOG_ERROR("%s 建立GameRoomEx异常", m_CurPlaceUserInfo.szName);
		//清理信息
		delete pRoomInfoItem->pGameRoomWnd;
		memset(pRoomInfoItem,0,sizeof(RoomInfoStruct));
        m_TCPSocket->CloseSocket(true);
		return; 
	}

    KillTimer(TIMER_PLC_CLOSE_SOCKET);
    m_TCPSocket->CloseSocket(true);
	return;
}

//机器人根据金币操作银行
void CAuto_AIDlg::AiControlBank()
{
    if (GLOBALDATA->CurDynamicConfig.nCheckOutMinMoney != 0 && m_CurPlaceUserInfo.i64Money < GLOBALDATA->CurDynamicConfig.nCheckOutMinMoney)
    {// 取钱
        __int64 i64RandMoney = ((GLOBALDATA->CurDynamicConfig.nCheckInMaxMoney - GLOBALDATA->CurDynamicConfig.nCheckOutMinMoney) / 2 +  GLOBALDATA->CurDynamicConfig.nCheckOutMinMoney) - m_CurPlaceUserInfo.i64Money;
        if(rand()%2 == 0)
        {
            i64RandMoney -= (rand() % ((GLOBALDATA->CurDynamicConfig.nCheckInMaxMoney - GLOBALDATA->CurDynamicConfig.nCheckOutMinMoney) / 4));
        }
        else
        {
            i64RandMoney += (rand() % ((GLOBALDATA->CurDynamicConfig.nCheckInMaxMoney - GLOBALDATA->CurDynamicConfig.nCheckOutMinMoney) / 4));
        }
        if(i64RandMoney > m_CurPlaceUserInfo.i64Bank)
        {
            i64RandMoney = m_CurPlaceUserInfo.i64Bank;
        }
        if(i64RandMoney > 0)			
            CheckMoney(i64RandMoney, 1);			
    }
    else if (GLOBALDATA->CurDynamicConfig.nCheckInMaxMoney != 0 && m_CurPlaceUserInfo.i64Money >  GLOBALDATA->CurDynamicConfig.nCheckInMaxMoney)
    {// 存钱
        __int64 i64RandMoney = m_CurPlaceUserInfo.i64Money - ((GLOBALDATA->CurDynamicConfig.nCheckInMaxMoney -GLOBALDATA->CurDynamicConfig.nCheckOutMinMoney) / 2 +  GLOBALDATA->CurDynamicConfig.nCheckOutMinMoney);
        if(rand()%2 == 0)
        {
            i64RandMoney -= (rand() % ((GLOBALDATA->CurDynamicConfig.nCheckInMaxMoney - GLOBALDATA->CurDynamicConfig.nCheckOutMinMoney) / 4));
        }
        else
        {
            i64RandMoney += (rand() % ((GLOBALDATA->CurDynamicConfig.nCheckInMaxMoney - GLOBALDATA->CurDynamicConfig.nCheckOutMinMoney) / 4));
        }
        if(i64RandMoney > 0)
            CheckMoney(i64RandMoney, 2);	
    }
}

void CAuto_AIDlg::CheckMoney(__int64 iMoney, int type)
{
    TMSG_GP_BankCheck checkMoney;
    checkMoney._game_id = 0;
    checkMoney._money = iMoney;
    checkMoney._operate_type = type;
    checkMoney._user_id = m_CurPlaceUserInfo.dwUserID;
    memcpy(checkMoney._szMD5Pass,m_CurPlaceUserInfo.szMD5Pass,sizeof(m_CurPlaceUserInfo.szMD5Pass));
    m_TCPSocket->SendData(&checkMoney, sizeof(TMSG_GP_BankCheck), MDM_GP_BANK, ASS_GP_BANK_CHECK, 0);
}

bool CAuto_AIDlg::NextTask()
{
    SYSTEMTIME lTime;
    GetLocalTime(&lTime);
    CString key;
    key.Format("%02d%02d", lTime.wHour, lTime.wMinute);
    DynamicConfig cof;
    cof.nLogonCount = -1;
    
    {
        AutoPlatLock lock(&PLATCONFIG->m_DynamicSection);
        auto it = PLATCONFIG->m_DynamicConfigs.begin();
        while(it != PLATCONFIG->m_DynamicConfigs.end())
        {
            if(it->first <= key.GetBuffer())
            {
                cof = it->second;
            }
            else
            {// 找到
                if(GLOBALDATA->CurDynamicConfig.strTime != cof.strTime)
                {
                    MSERVER_LOG_INFO("Start Task %s", cof.strTime);
                    GLOBALDATA->CurDynamicConfig = cof;
                    CString strInfo;
                    strInfo.Format("当前任务:%s", GLOBALDATA->CurDynamicConfig.strTime);
                    GetDlgItem(IDC_STATIC_TASKINFO)->SetWindowText(strInfo);
                    ResetConfig();
                    return true;
                }
                else
                {
                    break;
                }
            }
            it++;
        }
    }
    if(cof.nLogonCount != -1 && GLOBALDATA->CurDynamicConfig.strTime != cof.strTime)
    {// 找到
        MSERVER_LOG_INFO("Start Task %s", cof.strTime);
        GLOBALDATA->CurDynamicConfig = cof;
        CString strInfo;
        strInfo.Format("当前任务:%s", GLOBALDATA->CurDynamicConfig.strTime);
        GetDlgItem(IDC_STATIC_TASKINFO)->SetWindowText(strInfo);
        ResetConfig();
        return true;
    }
    
    return false;
}

int	CAuto_AIDlg::FindNextLogonIndex()
{
    static int s_index = -1;
    if(NextTask())
    {
        s_index = -1;
    }
    int i = s_index + 1;
    if(i >= GLOBALDATA->CurDynamicConfig.nLogonCount) i = 0;

	for (; i < GLOBALDATA->CurDynamicConfig.nLogonCount; i++)
	{
		if (GLOBALDATA->Robots[i].pRoomItem == NULL)
		{
            s_index = i;
            ZeroMemory(m_CurPlaceUserInfo.szName, sizeof(m_CurPlaceUserInfo.szName));
            sprintf_s(m_CurPlaceUserInfo.szName, "%s%d", GLOBALDATA->CurDynamicConfig.strRobotPreName, i + GLOBALDATA->CurDynamicConfig.nBeginSequenceNo);
			return i;
		}
	}
    if( i == GLOBALDATA->CurDynamicConfig.nLogonCount)
    {
        for (i = 0; i <= s_index; i++)
        {
            if (GLOBALDATA->Robots[i].pRoomItem == NULL)
            {
                s_index = i;
                ZeroMemory(m_CurPlaceUserInfo.szName, sizeof(m_CurPlaceUserInfo.szName));
                sprintf_s(m_CurPlaceUserInfo.szName, "%s%d", GLOBALDATA->CurDynamicConfig.strRobotPreName, i + GLOBALDATA->CurDynamicConfig.nBeginSequenceNo);
                return i;
            }
        }
    }

	return -1;
}

//保存配置
void  CAuto_AIDlg::SaveConfig()
{
    CString strTmp;
    bool bAddUser = false;
    DynamicConfig conf;
    GetDlgItem(IDC_EDIT_LOGINCOUNT)->GetWindowText(strTmp);
    if(!IsValidNumber(strTmp))
    {
        MessageBox("登陆人数设置错误");
        return;
    }
    if(conf.nLogonCount < atoi(strTmp) ) bAddUser = true;
    conf.nLogonCount = atoi(strTmp);
    GetDlgItem(IDC_EDIT_HEADNAME)->GetWindowText(strTmp);
    conf.strRobotPreName = strTmp;
    GetDlgItem(IDC_EDIT_BEGINNO)->GetWindowText(strTmp);
    if(!IsValidNumber(strTmp))
    {
        MessageBox("起始序列设置错误");
        return;
    }
    conf.nBeginSequenceNo = atoi(strTmp);
    GetDlgItem(IDC_EDIT_CHECKOUT)->GetWindowText(strTmp);
    if(!IsValidNumber(strTmp))
    {
        MessageBox("取钱阙值设置错误");
        return;
    }
    conf.nCheckOutMinMoney = _atoi64(strTmp);
    GetDlgItem(IDC_EDIT_CHECKIN)->GetWindowText(strTmp);
    if(!IsValidNumber(strTmp))
    {
        MessageBox("存钱阙值设置错误");
        return;
    }
    conf.nCheckInMaxMoney = _atoi64(strTmp);

    GetDlgItem(IDC_EDIT_ONEDESKROBOTNO)->GetWindowText(strTmp);
    if(!IsValidNumber(strTmp))
    {
        MessageBox("每桌机器人个数设置错误");
        return;
    }
    conf.bMachineDeskCount = atoi(strTmp);
    GetDlgItem(IDC_EDIT_TIMEFISHGAME)->GetWindowText(strTmp);
    if(!IsValidNumber(strTmp))
    {
        MessageBox("捕鱼游戏多久离桌设置错误");
        return;
    }
    conf.nFishGameTimeLeave = atoi(strTmp);
    GetDlgItem(IDC_EDIT_KEEPINDESK)->GetWindowText(strTmp);
    if(!IsValidNumber(strTmp))
    {
        MessageBox("等待游戏开始时长设置错误");
        return;
    }
    conf.nKeepInDeskSeconds = atoi(strTmp);
    GetDlgItem(IDC_EDIT_GAMEENDLEAVE)->GetWindowText(strTmp);
    if(!IsValidNumber(strTmp))
    {
        MessageBox("游戏结束离开概率设置错误");
        return;
    }
    conf.nGameEndLeaveDesk = atoi(strTmp);
    if(((CButton*)GetDlgItem(IDC_CHECK_ALLOWROBOTWITHPLAYER))->GetCheck() == 1)
    {
        conf.bMachineAndPlayer = true;
    }
    else
    {
        conf.bMachineAndPlayer = false;
    }
    CString errorMsg;
    if(!conf.IsValid(errorMsg))
    {
        MessageBox(errorMsg);
        return;
    }
    conf.strTime = GLOBALDATA->CurDynamicConfig.strTime;
    GLOBALDATA->CurDynamicConfig = conf;
    {
        AutoPlatLock lock(&PLATCONFIG->m_DynamicSection);
        auto it = PLATCONFIG->m_DynamicConfigs.find(GLOBALDATA->CurDynamicConfig.strTime.GetBuffer());
        if(it != PLATCONFIG->m_DynamicConfigs.end())
        {
            it->second = GLOBALDATA->CurDynamicConfig;
        }
    }

    PLATCONFIG->Save();

    if(bAddUser && GLOBALDATA->bConnectToWitchServer == 1)
    {// 增加用户，启动登陆
        OnConnectToLogonServer();
    }
}
//恢复修改
void  CAuto_AIDlg::ResetConfig()
{
    if(GLOBALDATA->CurDynamicConfig.nLogonCount == -1)
    {
        CString strTmp = "";
        GetDlgItem(IDC_STATIC_TASKINFO)->SetWindowText("当前任务");
        GetDlgItem(IDC_EDIT_LOGINCOUNT)->SetWindowText(strTmp);
        GetDlgItem(IDC_EDIT_HEADNAME)->SetWindowText(strTmp);
        GetDlgItem(IDC_EDIT_BEGINNO)->SetWindowText(strTmp);
        GetDlgItem(IDC_EDIT_CHECKOUT)->SetWindowText(strTmp);
        GetDlgItem(IDC_EDIT_CHECKIN)->SetWindowText(strTmp);
        GetDlgItem(IDC_EDIT_ONEDESKROBOTNO)->SetWindowText(strTmp);
        GetDlgItem(IDC_EDIT_TIMEFISHGAME)->SetWindowText(strTmp);
        GetDlgItem(IDC_EDIT_KEEPINDESK)->SetWindowText(strTmp);
        GetDlgItem(IDC_EDIT_GAMEENDLEAVE)->SetWindowText(strTmp);
        ((CButton*)GetDlgItem(IDC_CHECK_ALLOWROBOTWITHPLAYER))->SetCheck(0);
        return;
    }

    CString strTmp;
    strTmp.Format(_T("%d"),GLOBALDATA->CurDynamicConfig.nLogonCount);
    GetDlgItem(IDC_EDIT_LOGINCOUNT)->SetWindowText(strTmp);
    strTmp.Format(_T("%s"),GLOBALDATA->CurDynamicConfig.strRobotPreName);
    GetDlgItem(IDC_EDIT_HEADNAME)->SetWindowText(strTmp);
    strTmp.Format(_T("%d"),GLOBALDATA->CurDynamicConfig.nBeginSequenceNo);
    GetDlgItem(IDC_EDIT_BEGINNO)->SetWindowText(strTmp);
    strTmp.Format(_T("%d"),GLOBALDATA->CurDynamicConfig.nCheckOutMinMoney);
    GetDlgItem(IDC_EDIT_CHECKOUT)->SetWindowText(strTmp);
    strTmp.Format(_T("%d"),GLOBALDATA->CurDynamicConfig.nCheckInMaxMoney);
    GetDlgItem(IDC_EDIT_CHECKIN)->SetWindowText(strTmp);
    strTmp.Format(_T("%d"),GLOBALDATA->CurDynamicConfig.bMachineDeskCount);
    GetDlgItem(IDC_EDIT_ONEDESKROBOTNO)->SetWindowText(strTmp);
    strTmp.Format(_T("%d"),GLOBALDATA->CurDynamicConfig.nFishGameTimeLeave);
    GetDlgItem(IDC_EDIT_TIMEFISHGAME)->SetWindowText(strTmp);
    if(!PLATCONFIG->bFishGame)
    {
        GetDlgItem(IDC_EDIT_TIMEFISHGAME)->EnableWindow(FALSE);
    }
    strTmp.Format(_T("%d"),GLOBALDATA->CurDynamicConfig.nKeepInDeskSeconds);
    GetDlgItem(IDC_EDIT_KEEPINDESK)->SetWindowText(strTmp);
    if(PLATCONFIG->bFishGame)
    {
        GetDlgItem(IDC_EDIT_KEEPINDESK)->EnableWindow(FALSE);
    }
    strTmp.Format(_T("%d"),GLOBALDATA->CurDynamicConfig.nGameEndLeaveDesk);
    GetDlgItem(IDC_EDIT_GAMEENDLEAVE)->SetWindowText(strTmp);
    if(PLATCONFIG->bFishGame)
    {
        GetDlgItem(IDC_EDIT_GAMEENDLEAVE)->EnableWindow(FALSE);
    }
    if(GLOBALDATA->CurDynamicConfig.bMachineAndPlayer)
    {
        ((CButton*)GetDlgItem(IDC_CHECK_ALLOWROBOTWITHPLAYER))->SetCheck(1);
    }
    else
    {
        ((CButton*)GetDlgItem(IDC_CHECK_ALLOWROBOTWITHPLAYER))->SetCheck(0);
    }
}

bool CAuto_AIDlg::ModifyPara()
{
    if(GLOBALDATA->CurDynamicConfig.nLogonCount == -1) return false;
    bool bModify = false;
    /////////////
    CString strTmp;
    GetDlgItem(IDC_EDIT_LOGINCOUNT)->GetWindowText(strTmp);
    if(GLOBALDATA->CurDynamicConfig.nLogonCount != atoi(strTmp))
    {
        bModify = true;
    }
    GetDlgItem(IDC_STATIC_LOGINCOUNT)->Invalidate();

    /////////////
    GetDlgItem(IDC_EDIT_HEADNAME)->GetWindowText(strTmp);
    if(GLOBALDATA->CurDynamicConfig.strRobotPreName != strTmp)
    {
        bModify = true;
    }
    GetDlgItem(IDC_STATIC_HEADNAME)->Invalidate();

    ////////////////
    GetDlgItem(IDC_EDIT_BEGINNO)->GetWindowText(strTmp);
    if(GLOBALDATA->CurDynamicConfig.nBeginSequenceNo != atoi(strTmp))
    {
        bModify = true;
    }
    GetDlgItem(IDC_STATIC_BEGINNO)->Invalidate();

    //////////////
    GetDlgItem(IDC_EDIT_CHECKOUT)->GetWindowText(strTmp);
    if(GLOBALDATA->CurDynamicConfig.nCheckOutMinMoney != _atoi64(strTmp))
    {
        bModify = true;
    }
    GetDlgItem(IDC_STATIC_CHECKOUT)->Invalidate();

    //////////////
    GetDlgItem(IDC_EDIT_CHECKIN)->GetWindowText(strTmp);
    if(GLOBALDATA->CurDynamicConfig.nCheckInMaxMoney != _atoi64(strTmp))
    {
        bModify = true;
    }
    GetDlgItem(IDC_STATIC_CHECKIN)->Invalidate();

    //////////////
    GetDlgItem(IDC_EDIT_ONEDESKROBOTNO)->GetWindowText(strTmp);
    if(GLOBALDATA->CurDynamicConfig.bMachineDeskCount != atoi(strTmp))
    {
        bModify = true;
    }
    GetDlgItem(IDC_STATIC_ONDESKROBOTNO)->Invalidate();

    /////////////////
    GetDlgItem(IDC_EDIT_KEEPINDESK)->GetWindowText(strTmp);
    if(GLOBALDATA->CurDynamicConfig.nKeepInDeskSeconds != atoi(strTmp))
    {
        bModify = true;
    }
    GetDlgItem(IDC_STATIC_KEEPINDESK)->Invalidate();

    /////////////////
    GetDlgItem(IDC_EDIT_TIMEFISHGAME)->GetWindowText(strTmp);
    if(GLOBALDATA->CurDynamicConfig.nFishGameTimeLeave != atoi(strTmp))
    {
        bModify = true;
    }
    GetDlgItem(IDC_STATIC_TIMEFISHGAME)->Invalidate();

    /////////////////
    GetDlgItem(IDC_EDIT_GAMEENDLEAVE)->GetWindowText(strTmp);
    if(GLOBALDATA->CurDynamicConfig.nGameEndLeaveDesk != atoi(strTmp))
    {
        bModify = true;
    }
    GetDlgItem(IDC_STATIC_GAMEENDLEAVE)->Invalidate();

    //////////////////
    bool b;
    if(((CButton*)GetDlgItem(IDC_CHECK_ALLOWROBOTWITHPLAYER))->GetCheck() == 1)
    {
        b = true;
    }
    else
    {
        b = false;
    }
    if(b != GLOBALDATA->CurDynamicConfig.bMachineAndPlayer)
    {
        bModify = true;
    }
    GetDlgItem(IDC_CHECK_ALLOWROBOTWITHPLAYER)->Invalidate();

    return bModify;
}

void CAuto_AIDlg::InitUI(bool bEnable)
{
    GetDlgItem(IDC_EDIT_HEADNAME)->EnableWindow(bEnable);
    GetDlgItem(IDC_EDIT_BEGINNO)->EnableWindow(bEnable);
    if(PLATCONFIG->bFishGame) GetDlgItem(IDC_EDIT_TIMEFISHGAME)->EnableWindow(bEnable);
}

void CAuto_AIDlg::UIQueueGame(bool bEnable)
{
    GetDlgItem(IDC_EDIT_ONEDESKROBOTNO)->EnableWindow(bEnable);
    if(PLATCONFIG->bFishGame)   GetDlgItem(IDC_EDIT_TIMEFISHGAME)->EnableWindow(bEnable);
    GetDlgItem(IDC_EDIT_KEEPINDESK)->EnableWindow(bEnable);
    GetDlgItem(IDC_EDIT_GAMEENDLEAVE)->EnableWindow(bEnable);
    GetDlgItem(IDC_CHECK_ALLOWROBOTWITHPLAYER)->EnableWindow(bEnable);
}

void CAuto_AIDlg::OnBnClickedButtonLogin()
{
    // TODO: 在此添加控件通知处理程序代码
	GLOBALDATA->bLoginEnable = true;
    OnStartRobot();
}


void CAuto_AIDlg::OnBnClickedButtonCancel()
{
    // TODO: 在此添加控件通知处理程序代码
	GLOBALDATA->bLoginEnable = false;
    OnStopRobot();
}


void CAuto_AIDlg::OnBnClickedButtonClose()
{
    // TODO: 在此添加控件通知处理程序代码
    OnCancel();
}


void CAuto_AIDlg::OnBnClickedButtonMin()
{
    // TODO: 在此添加控件通知处理程序代码
    ::SendMessage(GetSafeHwnd(),WM_SYSCOMMAND,SC_MINIMIZE,0);
}


void CAuto_AIDlg::OnBnClickedButtonSave()
{
    // TODO: 在此添加控件通知处理程序代码
    SaveConfig();
}


void CAuto_AIDlg::OnBnClickedButtonReset()
{
    // TODO: 在此添加控件通知处理程序代码
    ResetConfig();
}

void CAuto_AIDlg::OnLButtonDown(UINT nFlags,CPoint point)
{
    PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x,point.y));
}


HBRUSH CAuto_AIDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CBaseRoom::OnCtlColor(pDC, pWnd, nCtlColor);
    if(GLOBALDATA->CurDynamicConfig.nLogonCount < 0) return hbr;
    CString strTmp;
    bool b;
    switch(pWnd->GetDlgCtrlID())
    {
    case IDC_STATIC_LOGINCOUNT:
        GetDlgItem(IDC_EDIT_LOGINCOUNT)->GetWindowText(strTmp);
        if(GLOBALDATA->CurDynamicConfig.nLogonCount != atoi(strTmp))
        {
            pDC->SetTextColor(RGB(255,0,0));
        }
        break;
    case IDC_STATIC_HEADNAME:
        GetDlgItem(IDC_EDIT_HEADNAME)->GetWindowText(strTmp);
        if(GLOBALDATA->CurDynamicConfig.strRobotPreName != strTmp)
        {
            pDC->SetTextColor(RGB(255,0,0));
        }
        break;
    case IDC_STATIC_BEGINNO:
        GetDlgItem(IDC_EDIT_BEGINNO)->GetWindowText(strTmp);
        if(GLOBALDATA->CurDynamicConfig.nBeginSequenceNo != atoi(strTmp))
        {
            pDC->SetTextColor(RGB(255,0,0));
        }
        break;
    case IDC_STATIC_CHECKOUT:
        GetDlgItem(IDC_EDIT_CHECKOUT)->GetWindowText(strTmp);
        if(GLOBALDATA->CurDynamicConfig.nCheckOutMinMoney != _atoi64(strTmp))
        {
            pDC->SetTextColor(RGB(255,0,0));
        }
        break;
    case IDC_STATIC_CHECKIN:
        GetDlgItem(IDC_EDIT_CHECKIN)->GetWindowText(strTmp);
        if(GLOBALDATA->CurDynamicConfig.nCheckInMaxMoney != _atoi64(strTmp))
        {
            pDC->SetTextColor(RGB(255,0,0));
        }
        break;
    case IDC_STATIC_ONDESKROBOTNO:
        GetDlgItem(IDC_EDIT_ONEDESKROBOTNO)->GetWindowText(strTmp);
        if(GLOBALDATA->CurDynamicConfig.bMachineDeskCount != atoi(strTmp))
        {
            pDC->SetTextColor(RGB(255,0,0));
        }
        break;
    case IDC_STATIC_TIMEFISHGAME:
        GetDlgItem(IDC_EDIT_TIMEFISHGAME)->GetWindowText(strTmp);
        if(GLOBALDATA->CurDynamicConfig.nFishGameTimeLeave != atoi(strTmp))
        {
            pDC->SetTextColor(RGB(255,0,0));
        }
        break;
    case IDC_STATIC_KEEPINDESK:
        GetDlgItem(IDC_EDIT_KEEPINDESK)->GetWindowText(strTmp);
        if(GLOBALDATA->CurDynamicConfig.nKeepInDeskSeconds != atoi(strTmp))
        {
            pDC->SetTextColor(RGB(255,0,0));
        }
        break;
    case IDC_STATIC_GAMEENDLEAVE:
        GetDlgItem(IDC_EDIT_GAMEENDLEAVE)->GetWindowText(strTmp);
        if(GLOBALDATA->CurDynamicConfig.nGameEndLeaveDesk != atoi(strTmp))
        {
            pDC->SetTextColor(RGB(255,0,0));
        }
        break;
    case IDC_CHECK_ALLOWROBOTWITHPLAYER:
        if(((CButton*)GetDlgItem(IDC_CHECK_ALLOWROBOTWITHPLAYER))->GetCheck() == 1)
        {
            b = true;
        }
        else
        {
            b = false;
        }
        if(b != GLOBALDATA->CurDynamicConfig.bMachineAndPlayer)
        {
            pDC->SetTextColor(RGB(255,0,0));
        }
        break;
    default:
        break;
    }

    // TODO:  在此更改 DC 的任何特性

    // TODO:  如果默认的不是所需画笔，则返回另一个画笔
    return hbr;
}



void CAuto_AIDlg::OnBnClickedButtonTasklist()
{
    // TODO: 在此添加控件通知处理程序代码
    CDiaogTaskList taskDlg;
    taskDlg.DoModal();
    if(GLOBALDATA->bConnectToWitchServer != 1)
    {
        if(GLOBALDATA->CurDynamicConfig.nLogonCount != -1)
        {
            {
                AutoPlatLock lock(&PLATCONFIG->m_DynamicSection);
                auto it = PLATCONFIG->m_DynamicConfigs.find(GLOBALDATA->CurDynamicConfig.strTime.GetBuffer());
                if(it != PLATCONFIG->m_DynamicConfigs.end())
                {
                    GLOBALDATA->CurDynamicConfig = it->second;
                }
                else
                {
                    GLOBALDATA->CurDynamicConfig.nLogonCount = -1;
                }
            }
            NextTask();
            ResetConfig();
        }
        else
        {
            if(NextTask())
            {
                ResetConfig();
            }
        }
        
    }
}


void CAuto_AIDlg::OnBnClickedButtonHallinfo()
{
    // TODO: 在此添加控件通知处理程序代码
    CDialogMsgInfo dlg(CDialogMsgInfo::Hall_Msg, this);
    dlg.DoModal();
}


void CAuto_AIDlg::OnBnClickedButtonRoominfo()
{
    // TODO: 在此添加控件通知处理程序代码
    CDialogMsgInfo dlg(CDialogMsgInfo::Room_Msg, this);
    dlg.DoModal();
}

void CAuto_AIDlg::ShowUserStatus()
{
    std::map<string, UserItemUIInfo> usrInfos;
    {
        AutoPlatLock lock(&UIDATA->uiSection);
        usrInfos = UIDATA->usrInfos;
    }
    m_ListUserStatus.SetRedraw(FALSE); // 防止闪烁 
    m_ListUserStatus.DeleteAllItems();
    auto it = usrInfos.begin();
    int nRow = 0;
    while(it != usrInfos.end())
    {
        if(m_ShowBeginNo > CString(it->second.strUserName.c_str()))
        {
            it++;
            continue;
        }
        if(m_ShowEndNo < CString(it->second.strUserName.c_str()))
        {
            it++;
            continue;
        }
        CString strTmp;
        m_ListUserStatus.InsertItem(nRow, "USERINFO");
        m_ListUserStatus.SetItemData(nRow,nRow);
        m_ListUserStatus.SetItemText(nRow, COL_USRNAME, it->first.c_str());
        m_ListUserStatus.SetItemText(nRow, COL_NICKNAME, it->second.strNickName.c_str());
        switch(it->second.GLoginStatus)
        {
        case 0:
            strTmp.Format(_T("未登陆(%d)"),it->second.GLoginStatus);
            break;
        case 1:
            strTmp.Format(_T("正在登陆(%d)"),it->second.GLoginStatus);
            break;
        case 2:
            strTmp.Format(_T("已登陆(%d)"),it->second.GLoginStatus);
            break;
        case 3:
            strTmp.Format(_T("登陆失败(%d)"),it->second.GLoginStatus);
            break;
        default:
            strTmp.Format(_T("未知(%d)"),it->second.GLoginStatus);
            break;
        }
        m_ListUserStatus.SetItemText(nRow, COL_GSERVERLOGIN, strTmp);
        strTmp.Format("%d", it->second.roomID);
        m_ListUserStatus.SetItemText(nRow, COL_ROOMID, strTmp);
        strTmp.Format("%d", it->second.bDeskNo);
        m_ListUserStatus.SetItemText(nRow, COL_DESKNO, strTmp);
        strTmp.Format("%d", it->second.bDeskStation);
        m_ListUserStatus.SetItemText(nRow, COL_DESKSTATION, strTmp);
        switch(it->second.bUserState)
        {
        case USER_NO_STATE:
            strTmp.Format(_T("未知(%d)"), it->second.bUserState);
            break;
        case USER_SITTING:
            strTmp.Format(_T("坐下(%d)"), it->second.bUserState);
            break;
        case USER_ARGEE:
            strTmp.Format(_T("准备(%d)"), it->second.bUserState);
            break;
        case USER_CUT_GAME:
            strTmp.Format(_T("断线(%d)"), it->second.bUserState);
            break;
        case USER_PLAY_GAME:
            strTmp.Format(_T("游戏中(%d)"), it->second.bUserState);
            break;
        default:
            strTmp.Format(_T("未知(%d)"), it->second.bUserState);
            break;
        }
        m_ListUserStatus.SetItemText(nRow, COL_USERSTATE, strTmp);
        strTmp.Format("任务时间:%s,存取阙值:%lld-%lld,捕鱼离桌:%d,等待开始:%d,允许真人同桌:%d,每桌机器人个数:%d,结束站起:%d",it->second.cof.strTime
            ,it->second.cof.nCheckOutMinMoney,it->second.cof.nCheckInMaxMoney
            ,it->second.cof.nFishGameTimeLeave,it->second.cof.nKeepInDeskSeconds
            ,it->second.cof.bMachineAndPlayer,it->second.cof.bMachineDeskCount
            ,it->second.cof.nGameEndLeaveDesk);
        m_ListUserStatus.SetItemText(nRow, COL_TASKINFO, strTmp);
        nRow++;
        it++;
    }
    m_ListUserStatus.SetRedraw(TRUE); 
    SetTimer(TIMER_SORT_USERSTATUS, 10, NULL);
}
int CAuto_AIDlg::CompareProc(LPARAM p1, LPARAM p2, LPARAM p3)
{
    CAuto_AIDlg *pDialog = (CAuto_AIDlg*)p3;
    if(pDialog != NULL)
        return pDialog->CompareFunc(p1, p2);
    return 0;
}

int CAuto_AIDlg::CompareFunc(LPARAM p1, LPARAM p2)
{
    if(m_sort_column == -1) return 0;
    int row1 = (int)p1;
    int row2 =(int)p2;
    CString lp1 = m_ListUserStatus.GetItemText(row1,m_sort_column);
    CString lp2 = m_ListUserStatus.GetItemText(row2,m_sort_column);

    if(lp1 == lp2) return 0;
    if(lp1 < lp2)
    {
        if(!m_bDescSort) return -1;
        return 1;
    }
    if(!m_bDescSort) return 1;
    return -1;
}

void CAuto_AIDlg::OnLvnColumnclickListStatus(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码
    *pResult = 0;

    m_bDescSort = !m_bDescSort;

    m_sort_column = pNMLV->iSubItem;  

    SetTimer(TIMER_SORT_USERSTATUS, 3000, NULL);
    *pResult =0;
}


void CAuto_AIDlg::OnBnClickedButtonAddtask()
{
    // TODO: 在此添加控件通知处理程序代码
    ((CEdit*)GetDlgItem(IDC_EDIT_CHECKBEGINNO))->GetWindowText(m_ShowBeginNo);
    ((CEdit*)GetDlgItem(IDC_EDIT_CHECKENDNO))->GetWindowText(m_ShowEndNo);
}
