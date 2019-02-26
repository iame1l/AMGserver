/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

// CenterServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MainServer.h"
#include "MainServerDlg.h"
#include "BuildTime.h"

#include "MD5.h"
#include "yxyDES.h"

#define IDM_TRAY_NOTIFY_MSG (WM_USER +17)

// 定时器id定义
enum M_TIMER_ID
{
	// 隐藏窗体
	TIMER_HIDE_WINDOW   = 1,

	// 检测推送消息
	TIMER_CHECK_MESSAGE = 2,

	// 证书检测
	TIMER_CHECK_LICENCE = 3,
};


CString GetCPUID()

{

	CString CPUID;

	unsigned long s1,s2;

	unsigned char vendor_id[]="------------";

	char sel;

	sel='1';

	CString VernderID;

	CString MyCpuID,CPUID1,CPUID2;

	switch(sel)

	{

	case '1':

		__asm{

			xor eax,eax      //eax=0:取Vendor信息

				cpuid    //取cpu id指令，可在Ring3级使用

				mov dword ptr vendor_id,ebx

				mov dword ptr vendor_id[+4],edx

				mov dword ptr vendor_id[+8],ecx

		}

		VernderID.Format("%s-",vendor_id);

		__asm{

			mov eax,01h   //eax=1:取CPU序列号

				xor edx,edx

				cpuid

				mov s1,edx

				mov s2,eax

		}

		CPUID1.Format("%08X%08X",s1,s2);

		__asm{

			mov eax,03h

				xor ecx,ecx

				xor edx,edx

				cpuid

				mov s1,edx

				mov s2,ecx

		}

		CPUID2.Format("%08X%08X",s1,s2);

		break;

	case '2':

		{

			__asm{

				mov ecx,119h

					rdmsr

					or eax,00200000h

					wrmsr

			}

		}

		AfxMessageBox("CPU id is disabled.");

		break;

	}

	MyCpuID = CPUID1+CPUID2;

	CPUID = MyCpuID;

	return CPUID;

}

DWORD GetHarddiskNum()  
{  
	char cVolume[256];                                         //   
	char cFileSysName[256];   
	DWORD dwSerialNum;                                          //硬盘序列号   
	DWORD dwFileNameLength;   
	DWORD dwFileSysFlag;   

	::GetVolumeInformation("C://", cVolume, 256, &dwSerialNum, &dwFileNameLength,   
		&dwFileSysFlag, cFileSysName, 256);  

	return dwSerialNum;  
} 

static unsigned __stdcall LicenseMessage(LPVOID pThreadData)
{
	CString strcpuid;
	strcpuid.Format(_T("%u"),GetHarddiskNum());

	CString s;
	s.Format("你好！您的服务器证书即将过期，请与服务商联系。\n\n请将以下机器码发送给服务商，获取注册码文件：\n\n%s\n\n",strcpuid.GetBuffer());

	DWORD cfgHandle=cfgOpenFile("HNGameGate.BCF");
	if(cfgHandle<0x10)
	{
		_endthreadex(0);
		return 0;
	}
	string srvDomain = cfgGetValue(cfgHandle,"MailSet","ServerDomain","");
	string userName = cfgGetValue(cfgHandle,"MailSet","SenderUserName","");
	string password = cfgGetValue(cfgHandle,"MailSet","SenderPassword","");
	string targetEmail = cfgGetValue(cfgHandle,"MailSet","TargetEmail","");
	cfgClose(cfgHandle);

	CSmtp smtp(25,srvDomain,userName,password,targetEmail,"服务器证书过期提醒!",s.GetBuffer());
	smtp.SendEmail_Ex();

	_endthreadex(0);
	return 0;
}

bool checkLicense()
{
		return true;
	CString strcpuid;
	strcpuid.Format(_T("%u"),GetHarddiskNum());
	
	unsigned char szMDTemp[16];
	MD5_CTX Md5;
	Md5.MD5Update((unsigned char *)strcpuid.GetBuffer(),strcpuid.GetLength());
	Md5.MD5Final(szMDTemp);

	char m_szMD5Pass[50];
	for (int i = 0; i < 16; i ++) 
		wsprintf(&m_szMD5Pass[i * 2], "%02x", szMDTemp[i] );
	CString StrMd5CpuID = m_szMD5Pass;

	DWORD cfgHandle=cfgOpenFile("HNGameGate.BCF");
	if(cfgHandle<0x10)
		return false;
	CString license = cfgGetValue(cfgHandle,"GateServer","License","");
	cfgClose(cfgHandle);

	yxyDES des;
	string key = strcpuid.GetBuffer();
	des.InitializeKey(key);


	des.DecryptAnyLength(license.GetBuffer());
	string strtmp = des.GetPlaintextAnyLength();
	string lsecpuid;
	string date;
	if (strtmp.length() == 40)
	{
		lsecpuid = strtmp.substr(0, 32);
		date = strtmp.substr(32, 40);
	}



	string lcs = m_szMD5Pass;

	CTime tmnow = CTime::GetCurrentTime();
	string strNow = tmnow.Format("%Y%m%d").GetBuffer();


	if (lcs == lsecpuid &&  strNow <= date)
	{
		if(atoi(date.c_str()) - atoi(strNow.c_str()) <= 3)
		{
			::_beginthreadex(NULL,0,LicenseMessage,NULL,0,NULL);
		}
		return true;
	}
	else
	{
		CString s;
		s.Format("您的服务器未注册或已过期，请与服务商联系。\n\n请将以下机器码发送给服务商，获取注册码文件：\n\n%s\n\n",strcpuid.GetBuffer());
		MessageBox(NULL, s,"提示",MB_ICONERROR);

		s="机器码已复制到您的剪贴板中，直接Ctrl+V粘贴机器码！";
		MessageBox(NULL, s,"提示",MB_ICONINFORMATION);

		OpenClipboard(NULL);
		EmptyClipboard();
		HANDLE hData=GlobalAlloc(GMEM_MOVEABLE, strcpuid.GetLength()+5); 
		if (hData==NULL)  
		{ 
			CloseClipboard(); 
			return TRUE; 
		}
		LPTSTR szMemName=(LPTSTR)GlobalLock(hData); 
		lstrcpy(szMemName,strcpuid); 
		SetClipboardData(CF_TEXT,hData); 
		GlobalUnlock(hData);  
		GlobalFree(hData); 
		CloseClipboard(); 
		return false;
	}

}

void OutputFun(char* message)
{
	CString msg = message;
	CMainServerDlg* p = (CMainServerDlg*)AfxGetApp()->m_pMainWnd ;
	p->OutputFun(msg);
}

void CMainServerDlg::OutputFun(const CString & message)
{
	m_listSysRunInfo.DisplayMsg(COutputDisplay::Message, message);
}

CMainServerDlg::CMainServerDlg(CWnd* pParent /*=NULL*/)
	:CDialog(IDD_MAINSERVER_DIALOG, pParent),m_TrayIcon(10)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_ListShortNotice.clear();
}

void CMainServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SYS_RUN_INFO, m_listSysRunInfo);
}

BEGIN_MESSAGE_MAP(CMainServerDlg, CDialog)
	ON_WM_CREATE()////////////
	ON_WM_SIZE()//////////
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_WM_SYSCOMMAND()
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_MESSAGE(IDM_TRAY_NOTIFY_MSG,OnTrayIconNotify)///////////
	ON_BN_CLICKED(IDC_BTNNEWS, &CMainServerDlg::OnBnClickedBtnnews)
	ON_REGISTERED_MESSAGE(WM_CREATETRAYBAR, OnCreateTray)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// 建立函数
int CMainServerDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
#define APP_TITLEe "MServer"
	
	// 系统内部建立
	if (CDialog::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}
	//if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,CRect(0,0,0,0),this,AFX_IDW_PANE_FIRST,NULL))	return -1;

	if(!checkLicense())
	{
		PostQuitMessage(0);
		ExitProcess(0);
		return 0;
	}

	SetTimer(M_TIMER_ID::TIMER_CHECK_LICENCE, 2*60*60*1000, NULL);

	// 建立任务栏图标
	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_TrayIcon.SetNotifyWnd(this, IDM_TRAY_NOTIFY_MSG);
	m_TrayIcon.SetIcon(hIcon, APP_TITLEe);
	
	//m_TrayIcon.ShowBalloonTip("正在启动MainServer主服务系统，请稍后...   ",APP_TITLEe);
	SetIcon(hIcon, FALSE);
	DestroyIcon(hIcon);

	m_TrayIcon.ShowBalloonTip("启动主服务系统成功", APP_TITLEe);
	ShowWindow(SW_SHOW);
	CenterWindow();
	return 0;
}
//任务栏图标消息
LRESULT CMainServerDlg::OnTrayIconNotify(WPARAM wID, LPARAM lEvent)
{
	switch (lEvent)
	{
	// 双击事件
	case WM_LBUTTONDBLCLK:
		{
			ShowWindow(SW_SHOW);
			//PostQuitMessage(0);
			return 1;
		}
	}
	return 0;
}
// 位置消息
void CMainServerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	if (nType == SIZE_MINIMIZED)
	{
		ShowWindow(SW_HIDE);
	}
}

// CCenterServerDlg 消息处理程序

BOOL CMainServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	// 执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	
	SetTimer(M_TIMER_ID::TIMER_HIDE_WINDOW, 2000, NULL);

	ManageInfoStruct Init;
	memset(&Init, 0, sizeof(Init));
	
	// 初始化服务
	if (!m_GameLogon.InitService(&Init))
	{
		OutputFun("初始化服务器失败！");
		AfxMessageBox("InitService Error !");
		return false;
	}
	
	// 启动服务
	UINT	errCode;
	if (!m_GameLogon.StartService(errCode))
	{
		CString stip;
		stip.Format("Start Service Failed ,Error Code:%X",errCode);
		OutputFun(stip);
		AfxMessageBox(stip);
		return false;
	}

	// 添加防止账户同时登陆
	::Sleep(2000);
	bool bres=m_GameLogon.m_LogonManage.m_SQLDataManage.m_pEngineSink->setStoredProc("SP_ClearLoginRecord",true);
	if(0!=bres)
	{
		return 0;
	}

	m_GameLogon.m_LogonManage.m_SQLDataManage.m_pEngineSink->addInputParameter("@ZID",m_GameLogon.m_LogonManage.m_ID);
	if(0 != m_GameLogon.m_LogonManage.m_SQLDataManage.m_pEngineSink->execStoredProc())
	{
		m_GameLogon.m_LogonManage.m_SQLDataManage.m_pEngineSink->closeRecord();
		return 0;
	}
	m_GameLogon.m_LogonManage.m_SQLDataManage.m_pEngineSink->closeRecord();

	CString s;
	s.Format("MServer启动成功 支持最大人数:%d Port:%d   Spe:110 Tec:29001  普通规则  允许进入 ",
		m_GameLogon.m_LogonManage.m_uMaxPeople,m_GameLogon.m_LogonManage.m_nPort );

	OutputFun(s);
	
	CString scaption;
	GetWindowText(s);
	scaption.Format("%s v%d.%d.%d %s",s,VER_MAIN,VER_MIDDLE,VER_RESVERSE,VER_BUILDTIME);
	SetWindowText(scaption);

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
// 来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
// 这将由框架自动完成。
void CMainServerDlg::OnPaint() 
{
	if (IsIconic())
	{
		// 用于绘制的设备上下文
		CPaintDC dc(this);

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
		// 用于绘制的设备上下文
		CPaintDC dc(this); 

		// 透明的, 显然的, 明晰的背景颜色
		dc.SetBkMode(TRANSPARENT);

		// 黄色
		dc.SetTextColor(RGB(255, 255, 255));
		dc.DrawText(TMLcopyright,lstrlen(TMLcopyright),CRect(2,457,720,836),DT_TOP|DT_RIGHT|DT_NOCLIP|DT_SINGLELINE);
		
		// 黄色
		dc.SetTextColor(RGB(200, 200, 200));
		dc.DrawText(TMLcopyright,lstrlen(TMLcopyright),CRect(1,456,720,836),DT_TOP|DT_RIGHT|DT_NOCLIP|DT_SINGLELINE);
		
		// 黄色
		dc.SetTextColor(RGB(0, 0, 0));
		dc.DrawText(TMLcopyright,lstrlen(TMLcopyright),CRect(0,455,720,836),DT_TOP|DT_RIGHT|DT_NOCLIP|DT_SINGLELINE);
		CDialog::OnPaint();
	}
}

// 当用户拖动最小化窗口时系统调用此函数取得光标显示
HCURSOR CMainServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMainServerDlg::OnDestroy()
{
	m_GameLogon.UnInitService();
	CDialog::OnDestroy();

	// TODO: 在此添加消息处理程序代码
}

void CMainServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	switch(nID)
	{
	case SC_CLOSE:
		//		server.Stop();
		KillTimer(M_TIMER_ID::TIMER_HIDE_WINDOW);
		CDialog::OnSysCommand(nID, lParam);
		return;
	}

	CDialog::OnSysCommand(nID, lParam);
}

void CMainServerDlg::OnBnClickedOk()
{
	//	server.Stop();
	KillTimer(M_TIMER_ID::TIMER_HIDE_WINDOW);
	OnOK();
}


void CMainServerDlg::OnBnClickedBtnnews()
{
	DWORD cfgHandle=cfgOpenFile("ShortNotice.bcf");

	int iOnOff = cfgGetValue(cfgHandle,"ShortNotice","OnOff", 0);
	int inum   = cfgGetValue(cfgHandle,"ShortNotice","num", 0);
	if (iOnOff == 1 && inum > 0)
	{	
		TCHAR str[MAX_PATH];
		m_ListShortNotice.empty();
		for (int i = 0; i < inum; ++i)
		{
			wsprintf(str, "News%d", i+1);
			CString msg = cfgGetValue(cfgHandle,"ShortNotice",str,"");

			if (msg.IsEmpty())
			{
				continue;
			}
			m_ListShortNotice.push_back(msg);
		}

		KillTimer(M_TIMER_ID::TIMER_CHECK_MESSAGE);
		SetTimer(M_TIMER_ID::TIMER_CHECK_MESSAGE, 2000, NULL);

		cfgClose(cfgHandle);
	}
}

LRESULT CMainServerDlg::OnCreateTray(WPARAM wp, LPARAM lp)
{
#define APP_TITLE "主服务MainServer"
	m_TrayIcon.SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME),APP_TITLE,TRUE);
	return 0;
}

void CMainServerDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case M_TIMER_ID::TIMER_HIDE_WINDOW:
		{
			KillTimer(M_TIMER_ID::TIMER_HIDE_WINDOW);
			this->ShowWindow(SW_HIDE);
		}
		break;
	case M_TIMER_ID::TIMER_CHECK_MESSAGE:
		{
			if(!m_ListShortNotice.empty())
			{
				MSG_GR_RS_NormalTalk news;
				ZeroMemory(&news,sizeof(MSG_GR_RS_NormalTalk));
				wsprintf(news.szMessage, "%s", m_ListShortNotice.front());
				news.iLength = m_ListShortNotice.front().GetLength() + 1;
				
				// 清除掉显示过的
				m_ListShortNotice.pop_front();
				m_GameLogon.m_LogonManage.m_TCPSocket.SendDataBatch(&news, sizeof(MSG_GR_RS_NormalTalk) - sizeof(news.szMessage) + news.iLength + 1, MDM_GP_MESSAGE, ASS_GP_NEWS_SYSMSG, 0);
			}
			else
			{
				KillTimer(M_TIMER_ID::TIMER_CHECK_MESSAGE);
			}
		}
		break;
	case M_TIMER_ID::TIMER_CHECK_LICENCE:
		if(!checkLicense())
		{
			KillTimer(3);
			PostQuitMessage(0);
			ExitProcess(0);
			break;
		}
	default:
		;
	}

	CDialog::OnTimer(nIDEvent);
}
