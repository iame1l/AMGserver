// DialogTaskEdit.cpp : 实现文件
//

#include "stdafx.h"
#include "Auto_AI.h"
#include "DialogTaskEdit.h"
#include "afxdialogex.h"
#include "Platlock.h"

// CDialogTaskEdit 对话框

IMPLEMENT_DYNAMIC(CDialogTaskEdit, CDialogEx)

CDialogTaskEdit::CDialogTaskEdit(const DynamicConfig*pconf, TaskEditType type, CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogTaskEdit::IDD, pParent)
{
    m_type = type;
    if(pconf != NULL)
    {
        m_conf = *pconf;
    }
    else
    {
        m_conf.bMachineAndPlayer = true;
        m_conf.bMachineDeskCount = 4;
        m_conf.nBeginSequenceNo = 1101;
        m_conf.nCheckInMaxMoney = 100000;
        m_conf.nCheckOutMinMoney = 10000;
        m_conf.nFishGameTimeLeave = 10;
        m_conf.nGameEndLeaveDesk = 30;
        m_conf.nKeepInDeskSeconds = 30;
        m_conf.nLogonCount = 10;
        m_conf.strRobotPreName = "C011";
        SYSTEMTIME sysTime;
        GetLocalTime(&sysTime);
        m_conf.strTime.Format("%02d%02d", sysTime.wHour, sysTime.wMinute);
    }
}

CDialogTaskEdit::~CDialogTaskEdit()
{
}

void CDialogTaskEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogTaskEdit, CDialogEx)
    ON_BN_CLICKED(IDOK, &CDialogTaskEdit::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CDialogTaskEdit::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDialogTaskEdit 消息处理程序

BOOL CDialogTaskEdit::OnInitDialog()
{
    __super::OnInitDialog();
    CComboBox *boxHour = (CComboBox *)GetDlgItem(IDC_COMBO_HOUR);
    CString strTmp;
    for(int i = 0; i < 24; ++i)
    {
        strTmp.Format(("%d"), i);
        boxHour->AddString(strTmp);
        boxHour->SetItemData(i, i);
    }
    boxHour->SetCurSel(0);
    CComboBox *boxMinute = (CComboBox *)GetDlgItem(IDC_COMBO_MINUTE);
    for(int i = 0; i < 60; ++i)
    {
        strTmp.Format(("%d"), i);
        boxMinute->AddString(strTmp);
        boxMinute->SetItemData(i, i);
    }
    boxMinute->SetCurSel(0);
    InitUI();
    return TRUE;
}

void CDialogTaskEdit::InitUI()
{
    if(m_type == Task_Mod)
    {
        SetWindowText(_T("修改任务"));
    }
    else
    {
        SetWindowText(_T("添加任务"));
    }
    if(!PLATCONFIG->bFishGame)
    {
        GetDlgItem(IDC_EDIT_FISHGAMELEAVE)->EnableWindow(FALSE);
    }
    else
    {
        GetDlgItem(IDC_EDIT_WAITTIME)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_GAMEENDLEAVE)->EnableWindow(FALSE);
    }

    // 设置启动时间
    CComboBox *boxHour = (CComboBox *)GetDlgItem(IDC_COMBO_HOUR);
    boxHour->SetCurSel(atoi(m_conf.strTime) / 100);
    CComboBox *boxMinute = (CComboBox *)GetDlgItem(IDC_COMBO_MINUTE);
    boxMinute->SetCurSel(atoi(m_conf.strTime) % 100);

    CString strTmp;
    // 设置登陆人数
    strTmp.Format("%d", m_conf.nLogonCount);
    GetDlgItem(IDC_EDIT_LOGINCOUNT)->SetWindowText(strTmp);
    // 名称前缀
    strTmp.Format("%s", m_conf.strRobotPreName);
    GetDlgItem(IDC_EDIT_HEADNAME)->SetWindowText(strTmp);
    // 起始序号
    strTmp.Format("%d", m_conf.nBeginSequenceNo);
    GetDlgItem(IDC_EDIT_BEGINNO)->SetWindowText(strTmp);
    // 取钱阙值
    strTmp.Format("%lld", m_conf.nCheckOutMinMoney);
    GetDlgItem(IDC_EDIT_CHECKOUT2)->SetWindowText(strTmp);
    // 存钱阙值
    strTmp.Format("%lld", m_conf.nCheckInMaxMoney);
    GetDlgItem(IDC_EDIT_CHECKIN2)->SetWindowText(strTmp);
    // 捕鱼多久离桌
    strTmp.Format("%d", m_conf.nFishGameTimeLeave);
    GetDlgItem(IDC_EDIT_FISHGAMELEAVE)->SetWindowText(strTmp);
    // 等待游戏开始时长
    strTmp.Format("%d", m_conf.nKeepInDeskSeconds);
    GetDlgItem(IDC_EDIT_WAITTIME)->SetWindowText(strTmp);
    // 每桌机器人个数
    strTmp.Format("%d", m_conf.bMachineDeskCount);
    GetDlgItem(IDC_EDIT_AINUMBER)->SetWindowText(strTmp);
    // 游戏结束站起概率
    strTmp.Format("%d", m_conf.nGameEndLeaveDesk);
    GetDlgItem(IDC_EDIT_GAMEENDLEAVE)->SetWindowText(strTmp);
    // 是否与真人同桌
    if(m_conf.bMachineAndPlayer)
    {
        ((CButton*)GetDlgItem(IDC_CHECK_ALLOWWITHPLAYER))->SetCheck(1);
    }
    else
    {
        ((CButton*)GetDlgItem(IDC_CHECK_ALLOWWITHPLAYER))->SetCheck(0);
    }
}

bool CDialogTaskEdit::SavePara()
{
    DynamicConfig conf;
    // 设置启动时间
    CComboBox *boxHour = (CComboBox *)GetDlgItem(IDC_COMBO_HOUR);
    int hour = boxHour->GetCurSel();
    if(hour == -1) hour = 0;
    CComboBox *boxMinute = (CComboBox *)GetDlgItem(IDC_COMBO_MINUTE);
    int minute = boxMinute->GetCurSel();
    if(minute == -1) minute = 0;
    conf.strTime.Format("%02d%02d", hour, minute);

    CString strTmp;
    // 设置登陆人数
    GetDlgItem(IDC_EDIT_LOGINCOUNT)->GetWindowText(strTmp);
    if(!IsValidNumber(strTmp))
    {
        MessageBox("登陆人数设置错误");
        return false;
    }
    conf.nLogonCount = atoi(strTmp);
    // 名称前缀
    GetDlgItem(IDC_EDIT_HEADNAME)->GetWindowText(strTmp);
    conf.strRobotPreName = strTmp;
    // 起始序号
    GetDlgItem(IDC_EDIT_BEGINNO)->GetWindowText(strTmp);
    if(!IsValidNumber(strTmp))
    {
        MessageBox("起始序号设置错误");
        return false;
    }
    conf.nBeginSequenceNo = atoi(strTmp);
    // 取钱阙值
    GetDlgItem(IDC_EDIT_CHECKOUT2)->GetWindowText(strTmp);
    if(!IsValidNumber(strTmp))
    {
        MessageBox("取钱阙值设置错误");
        return false;
    }
    conf.nCheckOutMinMoney = _atoi64(strTmp);
    // 存钱阙值
    GetDlgItem(IDC_EDIT_CHECKIN2)->GetWindowText(strTmp);
    if(!IsValidNumber(strTmp))
    {
        MessageBox("存钱阙值设置错误");
        return false;
    }
    conf.nCheckInMaxMoney = _atoi64(strTmp);
    // 捕鱼多久离桌
    GetDlgItem(IDC_EDIT_FISHGAMELEAVE)->GetWindowText(strTmp);
    if(!IsValidNumber(strTmp))
    {
        MessageBox("捕鱼多久离桌设置错误");
        return false;
    }
    conf.nFishGameTimeLeave = atoi(strTmp);
    // 等待游戏开始时长
    GetDlgItem(IDC_EDIT_WAITTIME)->GetWindowText(strTmp);
    if(!IsValidNumber(strTmp))
    {
        MessageBox("等待游戏开始时长设置错误");
        return false;
    }
    conf.nKeepInDeskSeconds = atoi(strTmp);
    // 每桌机器人个数
    GetDlgItem(IDC_EDIT_AINUMBER)->GetWindowText(strTmp);
    if(!IsValidNumber(strTmp))
    {
        MessageBox("每桌机器人个数设置错误");
        return false;
    }
    conf.bMachineDeskCount = atoi(strTmp);
    // 游戏结束站起概率
    GetDlgItem(IDC_EDIT_GAMEENDLEAVE)->GetWindowText(strTmp);
    if(!IsValidNumber(strTmp))
    {
        MessageBox("游戏结束站起概率设置错误");
        return false;
    }
    conf.nGameEndLeaveDesk = atoi(strTmp);
    // 是否与真人同桌
    if(((CButton*)GetDlgItem(IDC_CHECK_ALLOWWITHPLAYER))->GetCheck())
    {
        conf.bMachineAndPlayer = true;
    }
    else
    {
        conf.bMachineAndPlayer = false;
    }

    m_conf = conf;
    return true;
}

void CDialogTaskEdit::OnBnClickedOk()
{
    DynamicConfig tmpConf = m_conf;
    // TODO: 在此添加控件通知处理程序代码
    if(!SavePara()) return;
    CString errorMsg;
    if(!m_conf.IsValid(errorMsg))
    {
        MessageBox(errorMsg);
        return;
    }
    if(m_type == Task_Mod)
    {// 修改
        AutoPlatLock lock(&PLATCONFIG->m_DynamicSection);
        auto it = PLATCONFIG->m_DynamicConfigs.find(m_conf.strTime.GetBuffer());
        if(it != PLATCONFIG->m_DynamicConfigs.end())
        {
            if(it->second.strTime != tmpConf.strTime)
            {
                MessageBox(_T("该时间点已存在任务"));
                return;
            }
            else if(IDOK == MessageBox(_T("保存修改?"), _T("提示"), MB_OKCANCEL))
            {
                it->second = m_conf;
                PLATCONFIG->Save();
            }
        }
        else
        {// 先删除后添加
            auto it = PLATCONFIG->m_DynamicConfigs.find(tmpConf.strTime.GetBuffer());
            if(it != PLATCONFIG->m_DynamicConfigs.end())
            {
                PLATCONFIG->m_DynamicConfigs.erase(it);
            }
            PLATCONFIG->m_DynamicConfigs[m_conf.strTime.GetBuffer()] = m_conf;
        }
    }
    else
    {// 添加
        AutoPlatLock lock(&PLATCONFIG->m_DynamicSection);
        auto it = PLATCONFIG->m_DynamicConfigs.find(m_conf.strTime.GetBuffer());
        if(it != PLATCONFIG->m_DynamicConfigs.end())
        {
            MessageBox(_T("该时间点已存在任务"));
            return;
        }
        else
        {
            if(IDOK == MessageBox(_T("确认添加?"), _T("提示"), MB_OKCANCEL))
            {
                PLATCONFIG->m_DynamicConfigs[m_conf.strTime.GetBuffer()] = m_conf;
                PLATCONFIG->Save();
            }
        }
    }

    CDialogEx::OnOK();
}


void CDialogTaskEdit::OnBnClickedCancel()
{
    // TODO: 在此添加控件通知处理程序代码
    CDialogEx::OnCancel();
}
