#pragma once

#include "StdAfx.h"
#include "Resource.h"
#include "UpGradeLogic.h"
#include "UpGradeMessage.h"


#define MAST_PEOPLE			-1							//是否为管理员

//定时器 ID
#define ID_BEGIN_TIME				101					//开始定时器
#define ID_ROB_NT_TIMER				102					//抢庄定时器
#define ID_NOTE_TIMER               103	                //下注计时器
#define ID_OPEN_CARD_TIMER			104					//摆牛计时器


//重置函数参数
#define RS_ALL_VIEW					1					//重置所有参数
#define RS_GAME_CUT					2					//用户离开重置
#define RS_GAME_END					3					//游戏结束重置
#define RS_GAME_BEGIN				4					//游戏开始重置

//游戏框架类 
class CClientGameDlg : public CLoveSendClass//CGameFrameDlg
{
public:
	//是否有某种玩法
	bool IsPlayMode( int iMode );

	//获取最大抢庄倍数
	int GetMaxRobNT( );

public:
	TMSG_AI_CARD_SHAPE		m_tCardCfg;
	GameStation_Base		m_tBaseInfo;
	int						m_iCanNote[3];		//能下注的分数，0为默认值不用显示
public:
	//构造函数
	CClientGameDlg();
	//析构函数
	virtual ~CClientGameDlg();

protected:
	//初始化函数
	virtual BOOL OnInitDialog();
	//数据绑定函数
	virtual void DoDataExchange(CDataExchange * pDX);
	virtual void OnWatchSetChange(void);
	virtual void OnGameSetting();
	//重载函数
public:
	//设置游戏状态
	virtual bool SetGameStation(void * pStationData, UINT uDataSize);
	//游戏消息处理函数
	virtual bool HandleGameMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//重新设置界面
	virtual void ResetGameFrame();
	//定时器消息
	virtual bool OnGameTimer(BYTE bDeskStation, UINT uTimeID, UINT uTimeCount);
	//清除所有定时器
	void KillAllTimer();
	//同意开始游戏 
	virtual bool OnControlHitBegin();
	//安全结束游戏
	virtual bool OnControlSafeFinish(){return false;};
	//用户离开
	virtual bool GameUserLeft(BYTE bDeskStation, UserItemStruct * pUserItem, bool bWatchUser);


public:
	//开始按钮函数
	LRESULT	OnHitBegin(WPARAM wparam, LPARAM lparam);


	void ResetGameStation(int iGameStation);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnTimer(UINT nIDEvent);
	//	afx_msg void OnClose();
protected:
	virtual void OnCancel();
};
