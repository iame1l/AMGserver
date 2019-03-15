//GameBeginListener.h
//Create by lijj 08/08/2017
//游戏结束阶段监听器
#pragma once
#include "stdafx.h"
#include "Listener.h"
#include "UpgradeMessage.h"
#include <map>

using namespace std;
//结算->通知平台游戏结束
class GameFinishListener : public Listener
{
public:
	GameFinishListener();
	virtual void OnAdd();    //添加一个监听器
	virtual void Record(uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode);
	virtual void OnReconnect(uchar bDeskStation, uint uSocketID, bool bWatchUser);
private:

	//通知结算
	//void NotiResult();

	//算分
	bool countScore();

	bool isCreateRoomEnd();

	static void DelayResultOver(void **data, int dataCnt);
	
};