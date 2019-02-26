#pragma once
#ifndef AFC_GAMEROOMEX_HEAD_FILE
#define AFC_GAMEROOMEX_HEAD_FILE
#include "resource.h"
#include "Stdafx.h"

#include "BaseRoom.h"
#include "clientcomstruct.h"
#include "gameusermanage.h"
#include "gameroommessage.h"
#include "gameframedlg.h"
#include "map"
#include "string"
#include "Platconfig.h"

//结构说明
struct RoomInfoStruct;

// 消息事件
#define IDM_SET_GAME_TIME				WM_USER+100
#define IDM_KILL_GAME_TIME				WM_USER+101
#define IDM_CLOSE                       WM_USER+102  //20090211
#define IDM_STAND_UP					WM_USER+103 // 提供给游戏代码支持站起

// 定时器消息
#define TIMER_CONNECT_TO_GSERVER        1 // 连接GServer定时器
#define TIMER_FISHGAME_AIBANK 2         // 捕鱼类游戏金币变化检测
#define TIMER_FISHGAME_LEAVE 3          // 捕鱼离桌定时器，用来防止一直占有桌子
#define TIMER_CHECK_USERSTATE 4         // 用户过状态检测,防止长时间占桌
#define TIMER_FISHGAME_WITHTRUEPAYER  5 // 真人坐下后,机器人起桌优先与真人配桌
#define TIMER_STAND_UP 6

//游戏房间类
class CGameRoomEx : public CBaseRoom
{
	//信息变量
protected:
	HINSTANCE						m_hGameInstance;					//游戏进程
protected:
	
    bool                            m_bInitContest;                     //比赛是否初始化
	CGameUserManage					m_UserManage;						//用户管理
	
	GameInfoStruct					m_GameInfo;							//游戏信息
	IFrameInterface					* m_IGameFrame;						//游戏框架
	RoomInfoStruct                  * m_pRoomInfo;                      //大厅对应的房间信息

	MSG_GP_R_LogonResult			m_MServerLogonInfo;				    //大厅登陆信息

    DynamicConfig                   m_CurDynamicConfig;

    std::map<CString,int>           m_notHandleMsgs;

//登陆属性
	//函数定义
public:
	//构造函数
	CGameRoomEx(TCHAR * szGameName, RoomInfoStruct * pRoomInfo, MSG_GP_R_LogonResult * pLogonInfo, DynamicConfig cof); 
	//析构函数
	virtual ~CGameRoomEx();

	DECLARE_MESSAGE_MAP()

	//重载函数
public:
	//数据绑定函数
	virtual void DoDataExchange(CDataExchange * pDX);
	//初始化函数
	virtual BOOL OnInitDialog();
	//接口函数 
public:
	//网络连接消息
	virtual bool OnSocketConnectEvent(UINT uErrorCode, CTCPClientSocket * pClientSocket);
	//网络读取消息
	virtual bool OnSocketReadEvent(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//网络关闭消息
	virtual bool OnSocketCloseEvent();

    // 房间请求消息
protected:
    // 连接房间
    void ConnectToGServer();
    // 登陆房间
    bool LoginGServer();
    //坐下
    bool SitDesk();
    //起立
    bool StandingDesk();
    //发送控制消息 
    inline UINT ControlGameClient(UINT uControlCode, void * pControlData, UINT uDataSize);

	//房间应答消息
protected:
	//连接消息处理
	bool OnConnectMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//房间登陆处理
	bool OnLogonMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//用户列表处理
	bool OnUserListMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//用户动作处理
	bool OnUserActionMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//房间信息处理
	bool OnRoomMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);

public:
	//关闭房间
	void CloseGameRoom();
    //最佳查找座位
    bool OnFindBestDesk(BYTE &bDeskIndex, BYTE &bDeskStation);
	//启动游戏逻辑
	bool StartGameClient();

    //消息事件处理
public:
    LRESULT OnStandingDesk(WPARAM,LPARAM);
	//计时器消息
	LRESULT SetGameTime(WPARAM,LPARAM);
	//计时器消息
	LRESULT KillGameTime(WPARAM,LPARAM);
    //关闭游戏
    LRESULT OnCloseGame(WPARAM wparam,LPARAM lparam);//20090211
	//计时器
	void OnTimer(UINT_PTR uIDEvent);

private:

    bool CheckNeedMoneyOp();

    // 连接GServer
    void DoConnectToGServer();

	//统计某桌机器人数量
	BYTE CountMachine(BYTE bDeskIndex);
	//统计某桌真人数量
	BYTE CountTruePlayer(BYTE bDeskIndex);
};
#endif