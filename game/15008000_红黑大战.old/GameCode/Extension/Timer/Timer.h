#ifndef TIMER_H
#define TIMER_H

#include "RefCntObj.h"
#include <time.h>
#include <list>
#include <windows.h>

typedef void (*TimerCallback)(void **data, int dataCnt);

struct TimerDataImpl
{
	TimerDataImpl(void **p, int cnt, TimerCallback cb);
	~TimerDataImpl();

	bool isValid;
	void **data;
	int dataCnt;
	TimerCallback callback;

	bool IsValid() const;

	void ShutDown();

	void Call();
};

class TimerData
{
	RefCntObj<TimerDataImpl> m_Impl;
public:
	TimerData();
	TimerData(void **data, int dataCnt, TimerCallback callback);
	TimerData(const TimerData &other);

	void ShutDown();
	bool IsValid() const;
	void Run();
};

class Timer
{
	struct TimerDataShell
	{
		TimerDataShell(const TimerData &data, SYSTEMTIME startTime, float length);
		TimerData data;
		SYSTEMTIME startTime;
		float length;
	};

	bool m_Enabled;

	float m_TimePassed;
	float m_TimePassedSinceLastJob;
	float m_SequenceJobInterval;

	std::list<TimerDataShell> m_TimerDataList;
	std::list<TimerDataShell> m_TimerDataQueue;
public:
	Timer();
	void Add(float length, const TimerData &data);
	void AddToQueue(float intervalToNext, const TimerData &data);
	void Start();
	void Stop();
	void Update(float delta);
	void Clear();
};

#endif