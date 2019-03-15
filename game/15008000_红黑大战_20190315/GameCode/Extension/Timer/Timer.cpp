#include "stdafx.h"
#include "Timer.h"

TimerDataImpl::TimerDataImpl(void **p, int cnt, TimerCallback cb):data(NULL), dataCnt(0), isValid(true), callback(cb)
{
	if(p && cnt > 0)
	{
		dataCnt = cnt;
		data = new void*[dataCnt];
		memcpy(data, p, sizeof(void *) * dataCnt);
	}
}

TimerDataImpl::~TimerDataImpl()
{
	if(data)
	{
		delete data;
		data = NULL;
	}
}

bool TimerDataImpl::IsValid() const
{
	return isValid;
}

void TimerDataImpl::ShutDown()
{
	isValid = false;
}

void TimerDataImpl::Call()
{
	if(callback) callback(data, dataCnt);
}

TimerData::TimerData():m_Impl(NULL){}

TimerData::TimerData(void **data, int dataCnt, TimerCallback callback):
	m_Impl(new TimerDataImpl(data, dataCnt, callback))
{
}

TimerData::TimerData(const TimerData &timerData):m_Impl(timerData.m_Impl)
{}

void TimerData::ShutDown()
{
	if(m_Impl.GetObj()) m_Impl->ShutDown();
}

bool TimerData::IsValid() const
{
	return m_Impl.GetObj() != NULL && m_Impl->IsValid();
}

void TimerData::Run()
{
	if(m_Impl->IsValid())
	{
		m_Impl->Call();
	}
}


Timer::TimerDataShell::TimerDataShell(const TimerData &timerData, SYSTEMTIME s, float l):
data(timerData),startTime(s), length(l)
{

}


Timer::Timer():m_Enabled(false), m_TimePassed(0), m_TimePassedSinceLastJob(0),
m_SequenceJobInterval(0){}

void Timer::Add(float length, const TimerData &data)
{
	SYSTEMTIME curTime;
	GetLocalTime(&curTime);
	m_TimerDataList.push_back(TimerDataShell(data, curTime, length));
}

void Timer::AddToQueue(float intervalToNext, const TimerData &data)
{
	//m_TimerDataQueue.push_back(TimerDataShell(data, m_TimePassed, intervalToNext));
}

void Timer::Start()
{
	m_Enabled = true;
}

void Timer::Stop()
{
	m_Enabled = false;
}

__int64 TimeDiff(SYSTEMTIME left, SYSTEMTIME right)  
{  
	CTime tmLeft(left.wYear, left.wMonth, left.wDay, 0, 0, 0);  
	CTime tmRight(right.wYear, right.wMonth, right.wDay, 0, 0, 0);  
	CTimeSpan sp = tmLeft - tmRight;  
	long MillisecondsL = (left.wHour*3600 + left.wMinute*60 + left.wSecond)*1000 + left.wMilliseconds;    
	long MillisecondsR = (right.wHour*3600 + right.wMinute*60 + right.wSecond)*1000 + right.wMilliseconds;    
	return  (__int64)sp.GetDays()*86400000 + (MillisecondsL - MillisecondsR);//此处返回毫秒  
}

void Timer::Update(float delta)
{
	m_TimePassed += delta;
	bool tryAgain = true;

	int tried = 0;
	int maxTry = 30; // 每次Update最多触发一定数量的定时任务

	// 处理一般的定时任务
	while(tryAgain && tried < maxTry)
	{
		tryAgain = false;
		for(std::list<TimerDataShell>::iterator itr = m_TimerDataList.begin();
			itr != m_TimerDataList.end();
			++itr)
		{
			SYSTEMTIME curTime;
			GetLocalTime(&curTime);
			__int64 timedif = TimeDiff(curTime, itr->startTime);

			if(timedif >= itr->length*1000)
			{
				tryAgain = true;

				TimerDataShell data = *itr;
				m_TimerDataList.erase(itr);

				data.data.Run();

				break;
			}
		}
		++tried;
	}

	// 处理队列里面的任务
	if(m_TimerDataQueue.size() > 0)
	{
		m_TimePassedSinceLastJob += delta;
		if(m_TimePassedSinceLastJob >= m_SequenceJobInterval)
		{
			TimerDataShell data = *m_TimerDataQueue.begin();
			m_TimePassedSinceLastJob = 0;
			m_SequenceJobInterval = data.length;
			m_TimerDataQueue.pop_front();
			data.data.Run();
		}
	}
	else
	{
		m_TimePassedSinceLastJob = 0;
		m_SequenceJobInterval = 0;
	}
}

void Timer::Clear()
{
	m_TimePassed = 0;
	m_TimePassedSinceLastJob = 0;
	m_SequenceJobInterval = 0;
	m_TimerDataList.clear();
	m_TimerDataQueue.clear();
}