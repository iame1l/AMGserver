#ifndef EXTENSION_TIMER_H
#define EXTENSION_TIMER_H

#include "GameContext.h"
#include "Timer.h"

class ExtensionTimer : public GameContextExtension
{
	Timer m_Timer;

public:

	virtual void Add(float length, const TimerData &timerData);

	virtual void AddToQueue(float intervalFromNext, const TimerData &timerData);

	virtual void Update(float deltaTime);

	virtual void Clear();

	void OnGameStart();

	void OnGameFinish();

	void OnGameReset();
};

#endif
