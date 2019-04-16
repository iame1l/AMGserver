//组牌阶段监听器

#pragma once
#include "Listener.h"

class PlayCardListener : public Listener
{
public:
	PlayCardListener();
	~PlayCardListener();
	virtual void OnAdd();    //添加一个监听器
	virtual void OnTimer(uint timerID);
	virtual void OnReconnect(uchar bDeskStation, uint uSocketID, bool bWatchUser);
	virtual void Record(uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode); //记录到复盘文件
	virtual void Listen(uchar playerPos, uint msgID, void* msgData,uint uSize);
private:

	/*
	*@brief:通知下注
	*@Returns:   void
	*/
	void notiBeginPlay();

	/*
	*@brief:监测打牌流程是否结束
	*@Returns:   bool
	*/
	bool checkPlayCardFinish(uchar playPos);

	//读取筹码
	void notiXiaZhu(uchar playerPos,C_S_UserNote *message);
	/*
	*@brief:通知牌分
	*@Returns:   void
	*/
	void notiCardScore();

	/*
	*@brief 第一个出牌(新增需求-不拆组合牌，先出没组合的牌，再出组合的牌)
	*/
	bool autoPickUpFirst(uchar bSeatNO,uchar *bCards,size_t& sLen);
	/*
	*@brief 跟牌
	*/
	bool autoPickUpFollow(uchar bSeatNO,uchar *bCards,size_t& sLen);
public:
	static void DelayPlayCardOver(void **data, int dataCnt);
	static void DelayTiming(void **data, int dataCnt);

	static void AutoPlayCard(GameContext *context, int type,uchar playerPos,void *data);

};