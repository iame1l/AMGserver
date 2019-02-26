#include "stdafx.h"
#include "MyTickCount.h"

static LARGE_INTEGER m_liPerfStart;
static LARGE_INTEGER m_liPerfFreq;
static bool m_IsInit = false;

void CMyGetTickCount::Init()
{
	if (m_IsInit)
		return;

	m_IsInit = true;
	memset(&m_liPerfFreq,0,sizeof(m_liPerfFreq));	
	memset(&m_liPerfStart,0,sizeof(m_liPerfStart));

	QueryPerformanceFrequency(&m_liPerfFreq);	//获取每秒多少CPU Performance Tick 
	QueryPerformanceCounter(&m_liPerfStart); 	//CPU运行到现在的频数
}

 long long CMyGetTickCount::GetTime()
{
	Init();
	LARGE_INTEGER liPerfNow={0};
	QueryPerformanceCounter(&liPerfNow);				//计算CPU运行到现在的频数

	long long freqMs = m_liPerfFreq.QuadPart / 1000;	//获取每毫秒多少CPU Performance Tick 

	long long  ulInterval = ( ((liPerfNow.QuadPart - m_liPerfStart.QuadPart)) / freqMs);
	return ulInterval;
}