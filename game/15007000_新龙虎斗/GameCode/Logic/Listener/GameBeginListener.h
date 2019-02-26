//GameBeginListener.h
//游戏开始阶段监听器
#pragma once
#include "../../Server/stdafx.h"
#include "Listener.h"

//游戏开始->发牌
//阶段结束通知控制者，调用回调
class GameBeginListener : public Listener
{
public:
	GameBeginListener();
	~GameBeginListener();
	virtual void OnAdd();    //添加一个监听器
	virtual void OnTimer(uint timerID);
	virtual void Record(uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode);

private:
	void ReadPlayMode();
};