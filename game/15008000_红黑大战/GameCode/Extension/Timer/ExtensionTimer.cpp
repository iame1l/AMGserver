#include "ExtensionTimer.h"

void ExtensionTimer::Add(float length, const TimerData &timerData)
{
	m_Timer.Add(length, timerData);
}

void ExtensionTimer::AddToQueue(float intervalFromNext, const TimerData &timerData)
{
	m_Timer.AddToQueue(intervalFromNext, timerData);
}

void ExtensionTimer::Update(float deltaTime)
{
	m_Timer.Update(deltaTime);
}

void ExtensionTimer::Clear()
{
	m_Timer.Clear();
}

void ExtensionTimer::OnGameStart()
{
	m_Timer.Start();
}

void ExtensionTimer::OnGameFinish()
{
	m_Timer.Clear();
}

void ExtensionTimer::OnGameReset()
{
	m_Timer.Clear();
}