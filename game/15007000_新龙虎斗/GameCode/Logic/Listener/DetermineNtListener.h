//DetermineNtListener.h
//定庄监听器
#pragma once
#include "../../Server/stdafx.h"
#include "Listener.h"

enum NtMode
{
	NM_Fixed,  //定庄
	NM_Rotary, //轮流坐庄
	NM_Rob,    //抢庄
};

class DetermineNtListener : public Listener
{
public:
	DetermineNtListener(CServerGameDesk *desk, DataManage *dataMgr);
	~DetermineNtListener();
	virtual void OnAdd();    //添加一个监听器
	virtual void OnRemove(); //删除一个监听器
	virtual void Listen(uchar playerPos, uint msgID, void* msgData,uint uSize);
	virtual void OnTimer(uint timerID);
	virtual void OnReconnect(uchar bDeskStation, uint uSocketID, bool bWatchUser);
	virtual void Record(uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode); //记录到复盘文件

protected:
	bool AssureMsg(uchar playerPos, uint msgID, void* msgData);                                            //消息检查

private:
	void NoticeBanker();
	void NoticeDice();
	void NoticeRob();
	void NotiRobInfo();

	static void DelayNotiBanker(void **data, int dataCnt);
	static void DelayDeterminBankerOver(void **data, int dataCnt);
};