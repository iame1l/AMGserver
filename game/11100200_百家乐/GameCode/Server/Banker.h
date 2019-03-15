#ifndef  HN_BANKER
#define  HN_BANKER

#include <list>
#include <set>
using namespace std;

class Banker
{
	typedef unsigned char uchar;
	typedef unsigned int  uint;
public:
	//上庄
	void onUserApplyBanker(uchar PlayerPos,bool bRobot);
	//下庄
	void onUserLeaveBanker(uchar PlayerPos,bool bRobot);
	//坐下
	void onUserSit(uchar PlayerPos,bool bRobot);
	//玩家离开的时候先调用下庄，再掉用离开
	void onUserLeave(uchar PlayerPos,bool bRobot);

public:
	set<uchar> m_NomalPlayerRobots;			//非上庄列表Robot
	set<uchar> m_PreBankerRobots;			//上庄列表 Robot
	set<uchar> m_PreBankerReal;				//上庄列表 Real
public:
	static bool set_rand(set<uchar>& s_set,uchar& one);
};

#endif


