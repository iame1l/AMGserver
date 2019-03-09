//组牌阶段监听器

#pragma once
#include "Listener.h"

class CallListener : public Listener
{
public:
	CallListener();
	~CallListener();
	virtual void OnAdd();    //添加一个监听器
	virtual void OnTimer(uint timerID);
	virtual void OnReconnect(uchar bDeskStation, uint uSocketID, bool bWatchUser);
	virtual void Record(uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode); //记录到复盘文件
	virtual void Listen(uchar playerPos, uint msgID, void* msgData,uint uSize);
private:
	/*
	*@brief:处理玩家叫牌请求
	*@Returns:   bool
	*@Parameter: uchar playerPos
	*@Parameter: C_S_Button * req
	*@Parameter: uint uSize
	*/
	//bool recvCall(uchar playerPos,C_S_Button *req,uint uSize);
	/*
	*@brief:通知玩家叫牌
	*@Returns:   void
	*/
	void notiCall();
	/*
	*@brief:监测叫牌流程是否结束
	*@Returns:   bool
	*/
	bool checkCallFinish();

public:
	static void DelayCallOver(void **data, int dataCnt);

	static void AutoCall(GameContext *context, int type,uchar playerPos,void *data);
};