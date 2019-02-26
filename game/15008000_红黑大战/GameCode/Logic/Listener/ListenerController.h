//ListenerController.h
//监听器控制器
#pragma once
#include "stdafx.h"
#include "Listener.h"
#include "TuoGuanListener.h"
#include "WaitBeginListener.h"
#include "GameBeginListener.h"
#include "SendCardPlayerListener.h"
#include "GameFinishListener.h"
#include "DismissRoomListener.h"
#include "CallListener.h"
#include "PlayCardListener.h"
#include "FindFriendListener.h"
#include "GameContext.h"
class CServerGameDesk;

class ExtensionListenerController:public GameContextExtension
{
public:
	ExtensionListenerController();
	~ExtensionListenerController();
	/*
	*@brief:添加一个监听器
	*@Returns:   void
	*@Parameter: ListenerType listenerType
	*/
	void Add(ListenerType listenerType);                    
	/*
	*@brief:删除一个监听器
	*@Returns:   void
	*@Parameter: ListenerType listenerType
	*/
	void Remove(ListenerType listenerType);                 
	/*
	*@brief:消息入口
	*@Returns:   void
	*@Parameter: uchar playerPos
	*@Parameter: uint msgID
	*@Parameter: void * msgData
	*@Parameter: uint uSize
	*/
	void Listen(uchar playerPos, uint msgID, void* msgData,uint uSize);
	/*
	*@brief:重连入口
	*@Returns:   void
	*@Parameter: uchar playerPos
	*@Parameter: uint socketID
	*@Parameter: bool isWatchUser
	*/
	void Reconnect(uchar playerPos, uint socketID, bool isWatchUser);
	/*
	*@brief:删除所有的监听器
	*@Returns:   void
	*/
	void Clear();
	void OnTimer(uint timerID);
	/*
	*@brief:录像
	*@Returns:   void
	*@Parameter: uchar playerPos
	*@Parameter: void * msgData
	*@Parameter: uint dataSize
	*@Parameter: uchar mainID
	*@Parameter: uchar assID
	*@Parameter: uchar handleCode
	*/
	void Recode(uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode);

	/*
	*@brief:监听器切换管理
	*@Returns:   void
	*@Parameter: ListenerType listenerType
	*/
	void OnListenEnd( ListenerType listenerType );
	/*
	*@brief:初始化监听器
	*@Returns:   void
	*/
	void InitListener();

private:
	void _Add(Listener *listener);
	void _Remove(Listener *listener);
private:
	map<ListenerType,Listener*> m_CurrListeners; //当前的监听队列
};