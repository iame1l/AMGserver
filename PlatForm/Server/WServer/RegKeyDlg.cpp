/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

// RegKeyDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "resource.h"
#include "RegKeyDlg.h"


// RegKeyDlg 对话框

IMPLEMENT_DYNAMIC(RegKeyDlg, CDialog)

RegKeyDlg::RegKeyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(RegKeyDlg::IDD, pParent)
{
	m_szRegCode[0] = 0;

}

RegKeyDlg::~RegKeyDlg()
{
}

void RegKeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REGCODE, m_RegCode);
}


BEGIN_MESSAGE_MAP(RegKeyDlg, CDialog)
	ON_BN_CLICKED(IDOK, &RegKeyDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &RegKeyDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// RegKeyDlg 消息处理程序

void RegKeyDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_RegCode.GetWindowText(m_szRegCode,sizeof(m_szRegCode))==0)
	{
		MessageBox("请输入注册码！");
		GetDlgItem(IDC_REGCODE)->SetFocus();
		return;
	}

	if (strlen(m_szRegCode) != 39)
	{
		MessageBox("注册码格式不正确，请重新输入！");
		GetDlgItem(IDC_REGCODE)->SetFocus();
		return;
	}

	OnOK();
}

void RegKeyDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	OnCancel();
}
