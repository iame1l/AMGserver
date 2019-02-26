// DiaogTaskList.cpp : 实现文件
//

#include "stdafx.h"
#include "Auto_AI.h"
#include "DiaogTaskList.h"
#include "afxdialogex.h"
#include "Platconfig.h"
#include "DialogTaskEdit.h"
#include "Platglobaldata.h"
#include "DialogTaskEdit.h"
#include "Platlock.h"

#define COL_TASKTIME 0 // 任务时间
#define COL_LOGONCOUNT 1 // 登陆人数
#define COL_HEADNAME 2 // 名称前缀
#define COL_BEGINNO 3 // 起始序列
#define COL_CHECKOUT 4 // 取钱阙值
#define COL_CHECKIN 5 // 存钱阙值
#define COL_FISHGAMELEAVE 6 // 捕鱼多久离桌
#define COL_WAITGAMEBEGIN 7 // 等待开始时长
#define COL_AINUMBER 8 // 每桌机器人个数
#define COL_ALLOWAIWITHPLAER 9 // 是否允许真人同桌
#define COL_GAMEENDLEAVE 10 // 游戏结束站起概率


// CDiaogTaskList 对话框

IMPLEMENT_DYNAMIC(CDiaogTaskList, CDialogEx)

CDiaogTaskList::CDiaogTaskList(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDiaogTaskList::IDD, pParent)
{

}

CDiaogTaskList::~CDiaogTaskList()
{
}

void CDiaogTaskList::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_TASKLIST, m_TaskList);
}


BEGIN_MESSAGE_MAP(CDiaogTaskList, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_ADDTASK, &CDiaogTaskList::OnBnClickedButtonAddtask)
    ON_BN_CLICKED(IDC_BUTTON_DELTASK, &CDiaogTaskList::OnBnClickedButtonDeltask)
    ON_BN_CLICKED(IDC_BUTTON_MODTASK, &CDiaogTaskList::OnBnClickedButtonModtask)
END_MESSAGE_MAP()


// CDiaogTaskList 消息处理程序


BOOL CDiaogTaskList::OnInitDialog()
{
    __super::OnInitDialog();
    DWORD dwStyle = m_TaskList.GetExtendedStyle();
    dwStyle |= LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES;
    m_TaskList.SetExtendedStyle(dwStyle);
    m_TaskList.ModifyStyle(0, LVS_REPORT);
    m_TaskList.InsertColumn(COL_TASKTIME, _T("任务时间"), LVCFMT_LEFT,60);
    m_TaskList.InsertColumn(COL_LOGONCOUNT, _T("登陆人数"), LVCFMT_LEFT,60);
    m_TaskList.InsertColumn(COL_HEADNAME, _T("名称前缀"), LVCFMT_LEFT,60);
    m_TaskList.InsertColumn(COL_BEGINNO, _T("起始序列"), LVCFMT_LEFT,60);
    m_TaskList.InsertColumn(COL_CHECKOUT, _T("取钱阙值"), LVCFMT_LEFT,60);
    m_TaskList.InsertColumn(COL_CHECKIN, _T("存钱阙值"), LVCFMT_LEFT,60);
    m_TaskList.InsertColumn(COL_FISHGAMELEAVE, _T("捕鱼多久离桌"), LVCFMT_LEFT,60);
    m_TaskList.InsertColumn(COL_WAITGAMEBEGIN, _T("等待开始时长"), LVCFMT_LEFT,60);
    m_TaskList.InsertColumn(COL_AINUMBER, _T("每桌机器人个数"), LVCFMT_LEFT,60);
    m_TaskList.InsertColumn(COL_ALLOWAIWITHPLAER, _T("是否允许真人同桌"), LVCFMT_LEFT,60);
    m_TaskList.InsertColumn(COL_GAMEENDLEAVE, _T("游戏结束站起概率"), LVCFMT_LEFT,60);
    ShowTaskInfo();
    return TRUE;
}

void CDiaogTaskList::ShowTaskInfo()
{
    m_TaskList.SetRedraw(FALSE);
    m_TaskList.DeleteAllItems();
    AutoPlatLock lock(&PLATCONFIG->m_DynamicSection);
    auto it = PLATCONFIG->m_DynamicConfigs.begin();
    int nRow = 0;
    while(it != PLATCONFIG->m_DynamicConfigs.end())
    {
        CString strTmp;
        m_TaskList.InsertItem(nRow, "TASKINFO");
        m_TaskList.SetItemData(nRow,nRow);
        m_TaskList.SetItemText(nRow, COL_TASKTIME, it->second.strTime);
        strTmp.Format("%d", it->second.nLogonCount);
        m_TaskList.SetItemText(nRow, COL_LOGONCOUNT, strTmp);
        m_TaskList.SetItemText(nRow, COL_HEADNAME, it->second.strRobotPreName);
        strTmp.Format("%d", it->second.nBeginSequenceNo);
        m_TaskList.SetItemText(nRow, COL_BEGINNO, strTmp);
        strTmp.Format("%lld", it->second.nCheckOutMinMoney);
        m_TaskList.SetItemText(nRow, COL_CHECKOUT, strTmp);
        strTmp.Format("%lld", it->second.nCheckInMaxMoney);
        m_TaskList.SetItemText(nRow, COL_CHECKIN, strTmp);
        strTmp.Format("%d", it->second.nFishGameTimeLeave);
        m_TaskList.SetItemText(nRow, COL_FISHGAMELEAVE, strTmp);
        strTmp.Format("%d", it->second.nKeepInDeskSeconds);
        m_TaskList.SetItemText(nRow, COL_WAITGAMEBEGIN, strTmp);
        strTmp.Format("%d", it->second.bMachineDeskCount);
        m_TaskList.SetItemText(nRow, COL_AINUMBER, strTmp);
        strTmp.Format("%d", it->second.bMachineAndPlayer);
        m_TaskList.SetItemText(nRow, COL_ALLOWAIWITHPLAER, strTmp);
        strTmp.Format("%d", it->second.nGameEndLeaveDesk);
        m_TaskList.SetItemText(nRow, COL_GAMEENDLEAVE, strTmp);
        nRow++;
        it++;
    }
    m_TaskList.SetRedraw(TRUE);
}



void CDiaogTaskList::OnBnClickedButtonAddtask()
{
    // TODO: 在此添加控件通知处理程序代码
    POSITION pos = m_TaskList.GetFirstSelectedItemPosition();
    if(pos == NULL)
    {
        CDialogTaskEdit dlgEdit(NULL, CDialogTaskEdit::Task_Add, this);
        dlgEdit.DoModal();
        ShowTaskInfo();
    }
    else
    {
        int index = m_TaskList.GetNextSelectedItem(pos);
        CString strTime = m_TaskList.GetItemText(index, COL_TASKTIME);
        DynamicConfig conf;
        {
            AutoPlatLock lock(&PLATCONFIG->m_DynamicSection);
            auto it = PLATCONFIG->m_DynamicConfigs.find(strTime.GetBuffer());
            if(it != PLATCONFIG->m_DynamicConfigs.end())
            {
                conf = it->second;
            }
            else
            {
                MessageBox(_T("未知异常"));
                return;
            }
        }
        CDialogTaskEdit dlgEdit(&conf, CDialogTaskEdit::Task_Add, this);
        dlgEdit.DoModal();
        ShowTaskInfo();
    }
}


void CDiaogTaskList::OnBnClickedButtonDeltask()
{
    // TODO: 在此添加控件通知处理程序代码
    POSITION pos = m_TaskList.GetFirstSelectedItemPosition();
    if(pos == NULL)
    {
        MessageBox(_T("请选择要删除的任务?"));
        return;
    }

    int index = m_TaskList.GetNextSelectedItem(pos);
    CString strTime = m_TaskList.GetItemText(index, COL_TASKTIME);
    if(strTime == GLOBALDATA->CurDynamicConfig.strTime && GLOBALDATA->bConnectToWitchServer != 255)
    {
        // 正在执行的任务不允许删除
        MessageBox(_T("正在执行的任务不允许删除?"));
        return;
    }
    {

        AutoPlatLock lock(&PLATCONFIG->m_DynamicSection);
        auto it = PLATCONFIG->m_DynamicConfigs.find(strTime.GetBuffer());
        if(it != PLATCONFIG->m_DynamicConfigs.end())
        {
            CString tip;
            tip.Format("确认删除任务%s?",strTime);
            if(IDOK == MessageBox(tip, "警告", MB_OKCANCEL))
            {
                PLATCONFIG->m_DynamicConfigs.erase(it);
            }
        }
    }
    ShowTaskInfo();
}


void CDiaogTaskList::OnBnClickedButtonModtask()
{

    // TODO: 在此添加控件通知处理程序代码
    POSITION pos = m_TaskList.GetFirstSelectedItemPosition();
    if(pos == NULL)
    {
        MessageBox(_T("请选则要修改的任务?"));
        return;
    }

    int index = m_TaskList.GetNextSelectedItem(pos);
    CString strTime = m_TaskList.GetItemText(index, COL_TASKTIME);
    if(strTime == GLOBALDATA->CurDynamicConfig.strTime && GLOBALDATA->bConnectToWitchServer != 255)
    {
        // 正在执行的任务不允许修改
        MessageBox(_T("正在执行的任务不允许修改?"));
        return;
    }
    DynamicConfig conf;
    {
        auto it = PLATCONFIG->m_DynamicConfigs.find(strTime.GetBuffer());
        if(it != PLATCONFIG->m_DynamicConfigs.end())
        {
            conf = it->second;
        }
        else
        {
            MessageBox(_T("未知异常"));
            return;
        }
    }
    CDialogTaskEdit dlgEdit(&conf, CDialogTaskEdit::Task_Mod, this);
    dlgEdit.DoModal();
    ShowTaskInfo();
}
