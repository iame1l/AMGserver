#pragma once

#include "StdAfx.h"
#include "Resource.h"
#include "UpGradeLogic.h"
#include <list>
#define MAST_PEOPLE			-1							//是否为管理员

//定时器 ID100以上
const int TIME_BEGIN =	103	;			//开始
const int TIME_NOTIC_OUTPAI = 110;		//出牌

//游戏框架类 
class CClientGameDlg : public CLoveSendClass//CGameFrameDlg
{
private:
	//控件变量
	int m_iPrepareTime;				//准备时间
	BYTE m_byGameStation;			//当前游戏状态
	int  m_iMyDeskStation;			//座子号

	///精牌数据
	CUpGradeGameLogic m_cLogic;

	//函数定义
public:
	//构造函数
	CClientGameDlg();
	//析构函数
	virtual ~CClientGameDlg();


protected:
	//初始化函数
	virtual BOOL OnInitDialog();

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
	virtual bool SetGameTimer(BYTE bDeskStation,UINT uTimeCount,UINT uTimeID,bool bUseMillSecond = false) ; 
	//清除所有定时器
	void KillAllTimer();
	//同意开始游戏 
	virtual bool OnControlHitBegin();
	//安全结束游戏
	virtual bool OnControlSafeFinish(){return false;};
	//能否离开桌子
	virtual BOOL CanLeftDesk();
	//桌号换算到视图位置
	virtual BYTE ViewStation(BYTE bDeskStation);
	//消息处理函数
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//退出函数
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnTimer(UINT nIDEvent);

public:
	//初始化游戏数据
	void InitGameData();

	//加载配置文件
	void LoadGameIni();

public:
	///初始化部分数据
	void	ResetGameData();
	//=====================================================================================
	//处理网络消息
};
