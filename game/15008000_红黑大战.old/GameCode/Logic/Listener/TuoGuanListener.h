//TuoGaunListener.h
//ÍĞ¹Ü¼àÌıÆ÷
#pragma once
#include "Listener.h"

class TuoGuanListener : public Listener
{
public:
	TuoGuanListener();
	virtual void Listen(uchar playerPos, uint msgID, void* msgData,uint uSize);
};