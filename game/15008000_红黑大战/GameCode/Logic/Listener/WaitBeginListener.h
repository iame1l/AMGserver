//WaitBeginListener.h
//Create by lijj 05/08/2017
//准备阶段监听器
#pragma once
#include "../../Server/stdafx.h"
#include "Listener.h"

class WaitSetGameListener : public Listener
{
public:
	WaitSetGameListener();
	~WaitSetGameListener();
	virtual void OnAdd();    //添加一个监听器
	virtual void OnReconnect(uchar bDeskStation, uint uSocketID, bool bWatchUser);
	
protected:
	void Record(uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode);
};

class WaitAgreeListener : public Listener
{
public:
	WaitAgreeListener();
	~WaitAgreeListener();
	virtual void OnAdd();    //添加一个监听器
	virtual void Listen(uchar playerPos, uint msgID, void* msgData,uint uSize);
	virtual void OnReconnect(uchar bDeskStation, uint uSocketID, bool bWatchUser);

protected:
	void Record(uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode);
	bool NotiReady();
	bool NotiReadyInfo(uchar playerPos, uint socketID, bool isWatchUser);
};

class WaitNextListener : public Listener
{
public:
	WaitNextListener();
	virtual void OnAdd();    //添加一个监听器
	virtual void OnReconnect(uchar bDeskStation, uint uSocketID, bool bWatchUser);

protected:
	void Record(uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode);
	bool NotiReady();
	bool NotiReadyInfo(uchar playerPos);
};