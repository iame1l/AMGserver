/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

// CenterServerDlg.h : 头文件
//

#pragma once

#include "SysInfoList.h"
#include "GameLogonManage.h"

#include "TrayIcon.h"
#include <list>

// CCenterServerDlg 对话框
class CMainServerDlg : public CDialog
{
private:
	// 任务栏图标
	CTrayIcon			m_TrayIcon;				
	CGameLogonModule	m_GameLogon;
	
	// 存储短讯数据
	list<CString>		m_ListShortNotice;

public:
    void CMainServerDlg::OutputFun(const CString & message);
	
	// 标准构造函数
	CMainServerDlg(CWnd* pParent = NULL);
	LRESULT OnCreateTray(WPARAM wp, LPARAM lp);

	// 对话框数据
	//enum { IDD = IDD_MAINSERVER_DIALOG};

protected:
	// DDX/DDV 支持
	virtual void DoDataExchange(CDataExchange* pDX);

protected:	
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	CSysInfoList m_listSysRunInfo;
	afx_msg void OnDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnBnClickedOk();

	afx_msg void OnSize(UINT nType, int cx, int cy);
	LRESULT OnTrayIconNotify(WPARAM wID, LPARAM lEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnBnClickedBtnnews();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
