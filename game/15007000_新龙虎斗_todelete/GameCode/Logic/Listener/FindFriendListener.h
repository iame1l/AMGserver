//组牌阶段监听器

#pragma once
#include "Listener.h"

class FindFriendListener : public Listener
{
public:
	FindFriendListener();
	~FindFriendListener();
	virtual void OnAdd();    //添加一个监听器
	virtual void OnTimer(uint timerID);
	virtual void OnReconnect(uchar bDeskStation, uint uSocketID, bool bWatchUser);
	virtual void Record(uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode); //记录到复盘文件
	virtual void Listen(uchar playerPos, uint msgID, void* msgData,uint uSize);
private:
	/*
	*@brief:处理玩家设置朋友牌
	*@Returns:   void
	*@Parameter: uchar playerPos
	*@Parameter: C_S_Button * req
	*@Parameter: uint uSize
	*/
	//bool recvSetFindFriendCard(uchar playerPos,C_S_Button *req,uint uSize);

	void notiSetFindFriend();
public:
	static void DelayFindFriendOver(void **data, int dataCnt);

	static void AutoFindFriend(GameContext *context, int type,uchar playerPos,void *data);
};