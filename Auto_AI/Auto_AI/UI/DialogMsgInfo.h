#pragma once


// CDialogMsgInfo 对话框

#define TIMER_UPDATE_MSGINFO 1

class CDialogMsgInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogMsgInfo)

    enum Msg_Type
    {
        Hall_Msg,
        Room_Msg
    };

public:
	CDialogMsgInfo(Msg_Type, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogMsgInfo();

// 对话框数据
	enum { IDD = IDD_DIALOG_MSGINFO };

    virtual BOOL OnInitDialog() override;

    Msg_Type m_type;

public:
    //计时器
    void OnTimer(UINT_PTR nIDEvent);

    // 更新大厅日志
    void UpdateMServerInfo();
    // 更新房间日志
    void UpdateGServerInfo();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedCheckErrorinfo();
};
