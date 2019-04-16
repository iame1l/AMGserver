//SendCardPlayerListener.h
//Create by lijj 05/08/2017
//游戏发牌阶段监听器
#pragma once
#include "../../Server/stdafx.h"
#include "Listener.h"

class SendCardPlayerListener : public Listener
{
public:
	SendCardPlayerListener();
	~SendCardPlayerListener();
	virtual void OnAdd();    //添加一个监听器
	virtual void OnRemove(); //删除一个监听器
	virtual void OnTimer(uint timerID);
	virtual void OnReconnect(uchar bDeskStation, uint uSocketID, bool bWatchUser);
	virtual void Record(uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode);
	virtual void Listen(uchar playerPos, uint msgID, void* msgData,uint uSize);
public:
	static void SendCardPlayerListener::DelaySendCardOver(void **data, int dataCnt);
	static void SendCardPlayerListener::DelayTiming(void **data, int dataCnt);
	static void SendCardPlayerListener::DelayWait(void **data, int dataCnt);
	static void SendCardPlayerListener::DelayListen(void **data, int dataCnt);

private:
	//设置开始玩家位置
	void setFirstPlayer();
	//发牌
	bool sendCard();
	//检测罚分（摸到王没有炸弹（包括4个王）会罚分）
	bool checkKingFine(uchar playerPos);
	//算分
	bool countScore();
	//计算玩家金币
	bool CountUesrWinMoney();

	//让机器人输钱
	bool CountUesrLoseMoney();

	//根据区域类型生成牌型
	void GenerateByType(emWinAreaType emType);


	//通知罚分
	void notiFine();



	//20190415eil //todelete
	bool pingjuFunction();


};