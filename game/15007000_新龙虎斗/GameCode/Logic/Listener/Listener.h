//Listener.h
//Create by lijj 05/08/2017
//各个阶段监听器的父类
#pragma once

#include "stdafx.h"
#include "CardArrayBase.h"
#include "DataManage.h"
#include "Config.h"
#include "GameContext.h"
enum ListenerType
{
	LT_WaitSet = 0,
	LT_WaitAgree,		//等待同意
	LT_WaitNext,		//等待下一局
	LT_GameBegin,		//游戏开始
	LT_SendCard_Player,	//发牌
	LT_PlayCard,		//打牌
	LT_GameFinish,		//结束
	LT_DismissRoom,		//解散房间
};



//监听平台和客户端消息并作出响应
class Listener:public GameContextExtension
{
public:
	Listener();
	/*
	*@brief:监听器被添加初始化
	*@Returns:   void
	*/
	virtual void OnAdd();    
	/*
	*@brief:监听器被删除
	*@Returns:   void
	*/
	virtual void OnRemove(); 
	/*
	*@brief:监听客户端发来的消息
	*@Returns:   void
	*@Parameter: uchar playerPos
	*@Parameter: uint msgID
	*@Parameter: void * msgData
	*@Parameter: uint uSize
	*/
	virtual void Listen(uchar playerPos, uint msgID, void* msgData,uint uSize);
	/*
	*@brief:定时器消息
	*@Returns:   void
	*@Parameter: uint timerID
	*/
	virtual void OnTimer(uint timerID);
	/*
	*@brief:重连
	*@Returns:   void
	*@Parameter: uchar bDeskStation
	*@Parameter: uint uSocketID
	*@Parameter: bool bWatchUser
	*/
	virtual void OnReconnect(uchar bDeskStation, uint uSocketID, bool bWatchUser);
	/*
	*@brief:录像文件
	*@Returns:   void
	*@Parameter: uchar playerPos
	*@Parameter: void * msgData
	*@Parameter: uint dataSize
	*@Parameter: uchar mainID
	*@Parameter: uchar assID
	*@Parameter: uchar handleCode
	*/
	virtual void Record(uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode);
	/*
	*@brief:监视器标记开关
	*@Returns:   bool
	*/
	bool         Flag();
protected:
	bool AssureMsg(uchar playerPos, uint msgID, void* msgData);        //消息检查
	void sendBaseState(uchar playerPos, uint socketID, bool isWatchUser);

	int  getRunTime();
protected:	
	uchar              m_State;
	bool               m_Flag;
	int				   m_startTime;
};