/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/


#pragma once

#include "SysInfoList.h"
#include "afxcmn.h"

#include "afxwin.h"

#include "CenterServerManage.h"
#include "TrayIcon.h"


// CCenterServerDlg 对话框
class CCenterServerDlg : public CDialog
{
private:
	// 状态栏提示
	void setStatusbar();

	// 隐藏窗体
	void hideWindow();

	// 检测版本
	void checkVersion();

	// 定时器检测证书
	void checkLienceValid();

	// 任务栏图标
	CTrayIcon							m_TrayIcon;

	// 游戏登陆模块
	CCenterServerModule					m_CenterServerModule;

public:
	// 显示系统运行消息
    void OutputFun(CString message);

	// 标准构造函数
	CCenterServerDlg(CWnd* pParent = NULL);

	// 托盘图标
	LRESULT OnCreateTray(WPARAM wp, LPARAM lp);

	// 对话框数据
	enum { IDD = IDD_MAIN_DIALOG};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	CStatic m_strStatusBar;
	CSysInfoList m_listSysRunInfo;
	afx_msg void OnDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);	
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedOk();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	LRESULT OnTrayIconNotify(WPARAM wID, LPARAM lEvent);
};
