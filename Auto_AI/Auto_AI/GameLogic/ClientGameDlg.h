#pragma once

#include "StdAfx.h"
#include "Resource.h"
#include "UpGradeLogic.h"
#include "UpGradeMessage.h"


//定时器 ID
#define ID_BEGIN_TIME				101					//开始定时器
#define TIME_LOOK_CARD				102					//看牌定时器
#define TIME_ADD_NOTE               103	                //加注计时器
#define TIME_FOLLOW_NOTE            104	                //跟注计时器
#define TIME_OPEN_CARD	            105	                //比牌计时器
#define TIME_GIVE_UP	            106	                //弃牌计时器



//游戏框架类 
class CClientGameDlg : public CLoveSendClass
{
public:
	CUpGradeGameLogic		m_Logic;					//升级逻辑

	//变量定义
public:
	TGameBaseData				m_TCGameData;
	int							m_iNoteCount;			//根据自己的牌型 跟注次数
	int							m_iMyCardShape;			//自己的牌型
	//函数定义
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

private:
	//收到游戏开始消息
	void	OnHandleGameBegin(void * buffer,int nLen);
	//收到开始发牌消息
	void	OnHandleSendCard(void * buffer,int nLen);
	//开始进行下注
	void	OnHandleBeginPlay(void * buffer,int nLen);
	//接收到通知操作的消息
	void	OnHandleNoticeAction(void * buffer,int nLen);
	//收到玩家看牌消息
	void	OnHandleLookCardResult(void * buffer,int nLen);
	//收到跟注消息
	void	OnHandleFollowResult(void * buffer,int nLen);
	//收到玩家加注消息
	void	OnHandleAddResult(void * buffer,int nLen);
	//收到比牌结果消息
	void	OnHandleBiPaiResult(void * buffer,int nLen);
	//收到弃牌消息
	void	OnHandleGiveUpResult(void * buffer,int nLen);
	//收到游戏正常结束消息
	void	OnHandleGameFinish(void * buffer,int nLen);

	//获取当前最大牌玩家
	BYTE	GetCurMaxUser();	

	//消息函数
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
