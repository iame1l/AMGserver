#pragma once

#include "Platconfig.h"

// CDialogTaskEdit 对话框

class CDialogTaskEdit : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogTaskEdit)

public:
    enum TaskEditType
    {
        Task_Add, // 添加
        Task_Mod // 修改
    };

public:
	CDialogTaskEdit(const DynamicConfig*pconf, CDialogTaskEdit::TaskEditType type, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogTaskEdit();

// 对话框数据
	enum { IDD = IDD_DIALOG_TASK_EDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

    // 生成的消息映射函数
    virtual BOOL OnInitDialog() override;

    void InitUI();

    bool SavePara();

private:
    DynamicConfig m_conf;
    TaskEditType m_type;

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
};
