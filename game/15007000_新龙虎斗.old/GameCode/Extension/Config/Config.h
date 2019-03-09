//Config.h
//配置文件和游戏初始的配置
#pragma once
#include "UpgradeMessage.h"
#include "CardBase.h"

class Config
{
public:
    static void Init();
	static void Init(uint roomID);
public:
	static int					iXiaZhuTime;		//下注时间
	static int					iKaiPaiTime;		//开牌时间
	static int					iFreeTime;		    //空闲时间
	static int					s_TickInterval;     //定时器精度间隔
	static bool                 s_PlayBackDebug;
	static bool					s_PrintLog;	
	static vector<long>         s_SuperID;
};