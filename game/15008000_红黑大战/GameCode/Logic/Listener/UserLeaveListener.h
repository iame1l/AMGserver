//UserLeaveListener.h
//Create by lijj 10/08/2017
//用户托管
#pragma once
#include "../../Server/stdafx.h"
#include "Listener.h"

//监听用户离开请求
//用户请求离开->等待其余玩家同意->都同意则结束游戏监听结束
class UserLeaveListener : public Listener
{
public:
	UserLeaveListener();
	virtual void OnAdd();    //添加一个监听器
	virtual void Listen(uchar playerPos, uint msgID, void* msgData,uint uSize);
	virtual void Record(uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode);

protected:
	bool AssureMsg(uchar playerPos, uint msgID, void* msgData);

private:
	void RequestLeave(uchar playerPos, uint msgID, void* msgData);
	void AgreeLeave(uchar playerPos, uint msgID, void* msgData);

private:
	uint m_RequestID;
	char m_Message[61];
};