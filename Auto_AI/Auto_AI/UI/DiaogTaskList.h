#pragma once
#include "afxcmn.h"


// CDiaogTaskList 对话框

class CDiaogTaskList : public CDialogEx
{
	DECLARE_DYNAMIC(CDiaogTaskList)

public:
	CDiaogTaskList(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDiaogTaskList();

// 对话框数据
	enum { IDD = IDD_DIALOG_TASKLIST };

    // 生成的消息映射函数
    virtual BOOL OnInitDialog() override;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    CListCtrl m_TaskList;

    void ShowTaskInfo();
    afx_msg void OnBnClickedButtonAddtask();
    afx_msg void OnBnClickedButtonDeltask();
    afx_msg void OnBnClickedButtonModtask();
};
