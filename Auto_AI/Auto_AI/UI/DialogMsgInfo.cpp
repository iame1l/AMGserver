// DialogMsgInfo.cpp : 实现文件
//

#include "stdafx.h"
#include "Auto_AI.h"
#include "DialogMsgInfo.h"
#include "afxdialogex.h"
#include "Platglobaldata.h"
#include "Platlock.h"


// CDialogMsgInfo 对话框

IMPLEMENT_DYNAMIC(CDialogMsgInfo, CDialogEx)

CDialogMsgInfo::CDialogMsgInfo(Msg_Type type, CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogMsgInfo::IDD, pParent), m_type(type)
{

}

CDialogMsgInfo::~CDialogMsgInfo()
{
}

void CDialogMsgInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogMsgInfo, CDialogEx)
    	ON_WM_TIMER()
        ON_BN_CLICKED(IDC_CHECK_ERRORINFO, &CDialogMsgInfo::OnBnClickedCheckErrorinfo)
END_MESSAGE_MAP()


// CDialogMsgInfo 消息处理程序

BOOL CDialogMsgInfo::OnInitDialog()
{
    __super::OnInitDialog();

    ((CButton*)GetDlgItem(IDC_CHECK_ERRORINFO))->SetCheck(1);

    if(m_type == Hall_Msg)
    {
        SetWindowText(_T("大厅消息"));
        UpdateMServerInfo();
    }
    else
    {
        SetWindowText(_T("房间消息"));
        UpdateGServerInfo();
    }

    SetTimer(TIMER_UPDATE_MSGINFO, 10000, NULL);
    return true;

}

void CDialogMsgInfo::OnTimer(UINT_PTR nIDEvent)
{
    if(nIDEvent == TIMER_UPDATE_MSGINFO)
    {
        if(m_type == Hall_Msg)
        {
            UpdateMServerInfo();
        }
        else
        {
            UpdateGServerInfo();
        }
    }
}

void CDialogMsgInfo::UpdateMServerInfo()
{
    if(((CButton*)GetDlgItem(IDC_CHECK_STOPUPDATEINFO))->GetCheck()) 
    {
        return;
    }
    std::queue<UILogInfos> tmpInfos;
    AutoPlatLock lock(&UIDATA->LogMServerSection);
    CString tip_tolinfo = "";
    int onlyError = ((CButton*)GetDlgItem(IDC_CHECK_ERRORINFO))->GetCheck();
    while(UIDATA->LogMServerInfos.size() > 0)
    {
        UILogInfos uiLog = UIDATA->LogMServerInfos.front();
        UIDATA->LogMServerInfos.pop();
        tmpInfos.push(uiLog);
        if(onlyError && uiLog.type == 0) continue;
        CString strTmp;
        strTmp.Format("%s%s\r\n\r\n", tip_tolinfo, uiLog.Msg.c_str());
        tip_tolinfo = strTmp;
    }
    while(tmpInfos.size() > 0)
    {
        UIDATA->LogMServerInfos.push(tmpInfos.front());
        tmpInfos.pop();
    }
    GetDlgItem(IDC_EDIT_MSGINFO)->SetWindowText(tip_tolinfo);
}

void CDialogMsgInfo::UpdateGServerInfo()
{

    if(((CButton*)GetDlgItem(IDC_CHECK_STOPUPDATEINFO))->GetCheck()) 
    {
        return;
    }
    std::queue<UILogInfos> tmpInfos;
    AutoPlatLock lock(&UIDATA->LogGServerSection);
    CString tip_tolinfo = "";
    int onlyError = ((CButton*)GetDlgItem(IDC_CHECK_ERRORINFO))->GetCheck();
    while(UIDATA->LogGServerInfos.size() > 0)
    {
        UILogInfos uiLog = UIDATA->LogGServerInfos.front();
        UIDATA->LogGServerInfos.pop();
        tmpInfos.push(uiLog);
        if(onlyError && uiLog.type == 0) continue;
        CString strTmp;
        strTmp.Format("%s%s\r\n\r\n", tip_tolinfo, uiLog.Msg.c_str());
        tip_tolinfo = strTmp;
    }
    while(tmpInfos.size() > 0)
    {
        UIDATA->LogGServerInfos.push(tmpInfos.front());
        tmpInfos.pop();
    }
    GetDlgItem(IDC_EDIT_MSGINFO)->SetWindowText(tip_tolinfo);
}


void CDialogMsgInfo::OnBnClickedCheckErrorinfo()
{
    // TODO: 在此添加控件通知处理程序代码
    if(m_type == Hall_Msg)
    {
        UpdateMServerInfo();
    }
    else
    {
        UpdateGServerInfo();
    }
}
