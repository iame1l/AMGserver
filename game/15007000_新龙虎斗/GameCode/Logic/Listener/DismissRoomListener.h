//DismissRoomListener.h
//Create by lijj 22/08/2017
//解散房间监听器

#pragma once
#include "stdafx.h"
#include "Listener.h"

class DismissRoomListener : public Listener
{
public:
	DismissRoomListener();
	~DismissRoomListener();
	virtual void OnAdd();    //添加一个监听器
	virtual void Record(uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode);
private:
	bool NotiTotalResult();
};