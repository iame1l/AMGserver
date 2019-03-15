#include "stdafx.h"
#include "Banker.h"
#include <algorithm>
#include <random>

static int rand_Mersenne(const int _Min, const int _Max)
{
	if (_Min > _Max){ return 0; }
	if (_MSC_VER < 1500){ return 0; }
	random_device rd;
	mt19937 mt(rd());
	uniform_int_distribution<> dis(_Min, _Max);
	return dis(mt);
}

void Banker::onUserApplyBanker(uchar PlayerPos,bool bRobot)
{
	CString log;
	if(bRobot)
	{
		auto itr = m_NomalPlayerRobots.find(PlayerPos);
		if(itr != m_NomalPlayerRobots.end())
		{
			m_NomalPlayerRobots.erase(itr);	
		}
		log.Format("hnlog:  banker list robot add %d,size %d <",PlayerPos,m_PreBankerRobots.size());
		OutputDebugString(log);
		m_PreBankerRobots.insert(PlayerPos);
		log.Format("hnlog:  banker list robot add %d,size %d >",PlayerPos,m_PreBankerRobots.size());
		OutputDebugString(log);
	}

	else
	{
		log.Format("hnlog:  banker list real add %d,size %d <",PlayerPos,m_PreBankerReal.size());
		OutputDebugString(log);
		m_PreBankerReal.insert(PlayerPos);
		log.Format("hnlog:  banker list  real add %d,size %d <",PlayerPos,m_PreBankerReal.size());
		OutputDebugString(log);
	}
}

void Banker::onUserLeaveBanker(uchar PlayerPos,bool bRobot)
{
	CString log;
	{
		auto itr =m_PreBankerRobots.find(PlayerPos);
		if(itr != m_PreBankerRobots.end())
		{
			m_PreBankerRobots.erase(itr);
			log.Format("hnlog:  banker list robot del %d,size %d ",PlayerPos,m_PreBankerRobots.size());
			OutputDebugString(log);
		}
	}

	{
		auto itr = m_PreBankerReal.find(PlayerPos);
		if(itr != m_PreBankerReal.end())
		{
			m_PreBankerReal.erase(itr);
			log.Format("hnlog:  banker list real del %d,size %d ",PlayerPos,m_PreBankerRobots.size());
			OutputDebugString(log);
		}
	}

	if(bRobot)
	{
		m_NomalPlayerRobots.insert(PlayerPos);
	}
}

void Banker::onUserSit(uchar PlayerPos,bool bRobot)
{
	if(bRobot)
	{
		m_NomalPlayerRobots.insert(PlayerPos);
	}
}

void Banker::onUserLeave(uchar PlayerPos,bool bRobot)
{
	auto itr = m_NomalPlayerRobots.find(PlayerPos);
	if(itr != m_NomalPlayerRobots.end())
	{
		m_NomalPlayerRobots.erase(itr);
	}
}

bool  Banker::set_rand(set<uchar>& s_set,uchar& one)
{
	uint randint = rand_Mersenne(0,s_set.size() - 1);
	uint index = 0;
	for (auto itr = s_set.begin();itr != s_set.end();itr++)
	{
		if(index >= randint)
		{
			one = *itr;
			return true;
		}
		index++;
	}
	return false;
}