// Auto_AIDlg.h : 头文件
//
#include "stdafx.h"
#include "resource.h"

#include "Platconfig.h"


#include <vector>
#include <functional>
#include <algorithm>
#include "afxwin.h"
#include "afxcmn.h"
using namespace std;

#pragma once

// 定时器ID
#define				 TIME_CONNECT_ASERVER		1			    //连接AServer事件
#define				 TIME_CONNECT_MSERVER		2			    //连接MServer事件
#define              TIMER_CHECK_UNCONNECT_ACCOUNT 5            //定时检测未连接的账号
#define              TIMER_CHECK_MODIFY_PARA    6               //检测是否有修改动态配置
#define              TIMER_UPDATE_USERSTATUS 7 // 更新用户状态信息
#define              TIMER_SORT_USERSTATUS 8 // 排序
#define              TIMER_PLC_CLOSE_SOCKET 10

// 自定义消息
#define				 IDM_CLOSE_GAME_ROOM		WM_USER+200		//关闭房间


/*-----------------------------------------------------------------------------------------------*/
// CAuto_AIDlg 对话框
class CAuto_AIDlg : public CBaseRoom
{
	// 构造
public:
	//构造函数
	CAuto_AIDlg(CWnd * pParent=NULL);
	~CAuto_AIDlg();

	// 对话框数据
	enum { IDD = IDD_AUTO_AI_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


	// 实现
public:
	HICON									m_hIcon;
    bool                                    m_bExit;
    MSG_GP_R_LogonResult					m_CurPlaceUserInfo;	//用户资料
    volatile int                            m_iLogonIndex; // 当前登陆索引,GLOBALDATA 的Robots索引
    std::map<CString,int>                   m_notHandleMsgs;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();

	afx_msg void OnPaint();

	afx_msg HCURSOR OnQueryDragIcon();

    DECLARE_MESSAGE_MAP()
    // 屏蔽OnOK和OnCancel
    virtual void OnOK() override;
    virtual void OnCancel() override;
	//确定函数,启动登陆
	virtual void OnStartRobot();
	//取消函数,退出机器人
	virtual void OnStopRobot();
    //保存配置
    void SaveConfig();
    //恢复修改
    void ResetConfig();
public:
    // 发起
    //连接AServer
    void OnConnectToCenterServer();
    //登陆MServer
    void OnConnectToLogonServer();
    // 登陆MServer
    void LoginMServer();
    // 进入房间
    void EnterGameRoom(int roomid);
    //获取游戏列表
    bool GetGameRoomList();
    //向服务器获取比赛房间ID	
    void GetContestRoomID(int roomid);
public:
    // 应答
	//连接消息处理
	bool OnConnectMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//登陆消息处理
	bool OnLogonMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//列表信息
	bool OnListMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//处理服务端发送的比赛房间ID
	bool EnterContestRoom(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//关闭房间请求
	LRESULT OnCloseGameRoom(WPARAM wparam, LPARAM lparam);

public:
	//计时器
	void OnTimer(UINT_PTR nIDEvent);

	//接口函数
public:
	//网络读取消息
	virtual bool OnSocketReadEvent(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//网络连接消息
	virtual bool OnSocketConnectEvent(UINT uErrorCode, CTCPClientSocket * pClientSocket);
	//网络关闭消息
	virtual bool OnSocketCloseEvent();
protected:
	//发送网络消息函数
	inline virtual int SendGameData(BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode);
	//发送网络消息函数
	inline virtual int SendGameData(void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode);

private:
    // 执行连接AServer
	void DoOnConnectToCenterServer();
    // 执行连接MServer
    void DoOnConnectToLogonServer();
    // 查找下一个登陆索引m_iCurLogonIndex
	int	FindNextLogonIndex();
    // 查找下一个任务
    bool NextTask();
    // 检测是否修改了配置
    bool ModifyPara();
    ///存取钱
    void CheckMoney(__int64 iMoney, int type);
    //机器人根据金币操作银行
    void AiControlBank();
    //界面状态设置
    void InitUI(bool bEnable);
    void UIQueueGame(bool bEnable);

    //状态信息
    static int CALLBACK CompareProc(LPARAM p1, LPARAM p2, LPARAM p3);
    int m_sort_column; // 记录点击的列
    bool m_bDescSort; // 记录比较方法
    int CompareFunc(LPARAM p1, LPARAM p2);
    CListCtrl m_ListUserStatus;
    CString m_ShowBeginNo;
    CString m_ShowEndNo;
    void ShowUserStatus();
    afx_msg void OnLvnColumnclickListStatus(NMHDR *pNMHDR, LRESULT *pResult);

public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnBnClickedButtonLogin();
    afx_msg void OnBnClickedButtonCancel();
    afx_msg void OnBnClickedButtonClose();
    afx_msg void OnBnClickedButtonMin();
    afx_msg void OnBnClickedButtonSave();
    afx_msg void OnBnClickedButtonReset();
public:
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point); // 用于窗口拖动
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnBnClickedButtonTasklist();
    afx_msg void OnBnClickedButtonHallinfo();
    afx_msg void OnBnClickedButtonRoominfo();
    afx_msg void OnBnClickedButtonAddtask();
};
/*-----------------------------------------------------------------------------------------------*/