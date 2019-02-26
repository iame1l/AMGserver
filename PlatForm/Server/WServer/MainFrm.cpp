/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "Stdafx.h"
#include "AFCLoader.h"
#include "MainFrm.h"
#include "BuildTime.h"
#include "RegKeyDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define VK_B								66						 //'B'key
#define VK_Z								90						 //'Z'key
#define VK_W								87						 //'W'key


IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_MESSAGE(WM_MOUSEHOVER,OnMouseHover)
	ON_MESSAGE(WM_MOUSELEAVE,OnMouseLeave)
	ON_MESSAGE(IDM_TRAY_NOTIFY_MSG,OnTrayIconNotify)
	ON_COMMAND(IDM_LOGON_SYSTEM, OnLogonSystem)
	ON_COMMAND(IDM_GET_ROOM_LIST, OnGetRoomList)
	ON_REGISTERED_MESSAGE(WM_CREATETRAYBAR, OnCreateTray)
END_MESSAGE_MAP()

CServiceManage					g_Service;					//游戏服务

//获取任务栏图标
CTrayIcon & AFCGetTrayIcon() { return ((CMainFrame *)AfxGetMainWnd())->m_TrayIcon; };

//构造函数
CMainFrame::CMainFrame() : m_TrayIcon(10)
{
	m_bMouseTrack = true;
}

//析构函数
CMainFrame::~CMainFrame()
{
}

//建立函数
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	//系统内部建立
	if (CFrameWnd::OnCreate(lpCreateStruct)==-1) return -1;
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,CRect(0,0,0,0),this,AFX_IDW_PANE_FIRST,NULL))	return -1;

	//建立任务栏图标
	HICON hIcon=AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_TrayIcon.SetNotifyWnd(this,IDM_TRAY_NOTIFY_MSG);
	m_TrayIcon.SetIcon(hIcon,APP_TITLE);
	SetIcon(hIcon,FALSE);
	DestroyIcon(hIcon);
	CenterWindow();

	//登陆系统
	g_Service.m_Info.LoadData();
	g_Service.SetMessageInterface(this);
	g_Service.Init();
	PostMessage(WM_COMMAND,IDM_LOGON_SYSTEM,0);

	CString s,scaption;
	GetWindowText(s);
	scaption.Format("%s v%d.%d.%d %s",s,VER_MAIN,VER_MIDDLE,VER_RESVERSE,VER_BUILDTIME);
	SetWindowText(scaption);	

	//SetTimer(1,6000,NULL);

	return 0;
}

//消息解释
BOOL CMainFrame::PreTranslateMessage(MSG * pMsg)
{
	if (this->GetSafeHwnd()==NULL)
	{
		return TRUE;
	}
	
	
	if (pMsg->message==WM_KEYDOWN)		
	{
		if((GetAsyncKeyState(VK_CONTROL) < 0)
		&& (GetAsyncKeyState(VK_B) < 0) 
		&& (GetAsyncKeyState(VK_Z) < 0) 
		&& (GetAsyncKeyState(VK_W) < 0))
		{
			CString CheckVersionHotKey = "test7";
			AfxMessageBox(CheckVersionHotKey);
		
			return TRUE;
		}
	}

	return __super::PreTranslateMessage(pMsg);
}

//初始化建立参数
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs)) return FALSE;
	cs.style=WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX;
	cs.dwExStyle&=~WS_EX_CLIENTEDGE;
	cs.lpszClass=AfxRegisterWndClass(0);

	return TRUE;
}

//焦点函数
void CMainFrame::OnSetFocus(CWnd * pOldWnd)
{
	m_wndView.SetFocus();
}

//命令函数
BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo)) return TRUE;

	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

//任务栏图标消息
LRESULT CMainFrame::OnMouseHover(WPARAM wID, LPARAM lEvent)
{
	return 0;
}

//任务栏图标消息
LRESULT CMainFrame::OnMouseLeave(WPARAM wID, LPARAM lEvent)
{
	m_TrayIcon.SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), g_Service.GetRoomName());
	m_bMouseTrack=true;
	return 0;
}

//任务栏图标消息
LRESULT CMainFrame::OnTrayIconNotify(WPARAM wID, LPARAM lEvent)
{
	switch (lEvent)
	{
	case WM_MOUSEMOVE:
		{
			if (m_bMouseTrack)
			{
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(tme); //结构体缓冲区大小
				tme.dwFlags = TME_LEAVE|TME_HOVER; //注册WM_MOUSEHOVER消息
				tme.dwHoverTime = 10; //WM_MOUSEHOVER消息触发间隔时间
				tme.hwndTrack = GetSafeHwnd(); //当前窗口句柄

				::TrackMouseEvent(&tme); //注册发送消息

				m_bMouseTrack = false;
			}
			return 1;
		}
	case WM_LBUTTONDBLCLK:	//双击事件
		{
			////if (g_Service.m_Info.IsLogonSystem())
			{
				ShowWindow(SW_SHOWMAXIMIZED);
				SetForegroundWindow();
			}
			return 1;
		}
	case WM_RBUTTONUP:		//右键信息
		{
			CMenu Menu;
			Menu.LoadMenu(IDR_MAINFRAME);
			CMenu * pSubMenu=Menu.GetSubMenu(0);
			CPoint MousePoint;
			::GetCursorPos(&MousePoint);	
			SetForegroundWindow();

			//控制菜单
			if (g_Service.m_Info.IsLockSystem()==false)	pSubMenu->EnableMenuItem(IDM_LOCK_SYSTEM,MF_BYCOMMAND|MF_GRAYED);
			if (g_Service.m_Info.m_uSystemState!=STS_NO_LOGON) pSubMenu->EnableMenuItem(IDM_LOGON_SYSTEM,MF_BYCOMMAND|MF_GRAYED);
			::TrackPopupMenu(pSubMenu->GetSafeHmenu(),0,MousePoint.x,MousePoint.y,0,GetSafeHwnd(),NULL);

			return 1;
		}
	}
	return 0;
}

//位置消息
void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);
	if (nType==SIZE_MINIMIZED)	ShowWindow(SW_HIDE);
}

//登陆系统
void CMainFrame::OnLogonSystem()
{
	g_Service.LogonSystem("pmb","123");
}

//显示信息
bool CMainFrame::ShowMessage(const TCHAR * szMessage)
{
	m_TrayIcon.ShowBalloonTip(szMessage,APP_TITLE);
	return true;
}

//获取大厅列表
void CMainFrame::OnGetRoomList()
{
	//更新信息
	g_Service.UpdateComDLLList(&m_wndView.m_ServiceControl.m_InstallList);
	g_Service.UpdateServiceInfo(&m_wndView.m_ServiceControl.m_StartList);

	return;
}

LRESULT CMainFrame::OnCreateTray(WPARAM wp, LPARAM lp)
{
#define APP_TITLEe "GServer"
	m_TrayIcon.SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME),APP_TITLE,TRUE);
	return 0;
}
