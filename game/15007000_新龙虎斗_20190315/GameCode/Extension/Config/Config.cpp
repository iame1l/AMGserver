#include "stdafx.h"
#include "Config.h"


int Config::s_TickInterval = 100;

bool Config::s_PlayBackDebug = false;

bool Config::s_PrintLog = true;

int Config::iXiaZhuTime = 15;

int Config::iKaiPaiTime = 2;

int Config::iFreeTime = 3;

vector<long> Config::s_SuperID;

void Config::Init()
{
	CString path = CINIFile::GetAppPath();//本地路径
	CINIFile f(path + SKIN_FOLDER + "_s.ini");
	//游戏配置信息
	CString key = TEXT("game");

	iXiaZhuTime = f.GetKeyVal(key, "XiaZhuTime", 15);
	if(iXiaZhuTime <= 0)
		iXiaZhuTime = 15;

	iKaiPaiTime = f.GetKeyVal(key, "KaiPaiTime", 2);
	if(iKaiPaiTime <= 0)
		iKaiPaiTime = 2;

	iFreeTime = f.GetKeyVal(key, "FreeTime", 3);
	if(iFreeTime <= 0)
		iFreeTime = 2;

	s_PlayBackDebug = f.GetKeyVal(key, "PlayBackDebug", 0) > 0;

	s_PrintLog = f.GetKeyVal(key, "PrintLog", 1) > 0;

	s_TickInterval = f.GetKeyVal(key, "TickInterval", 10);

	//超端
	s_SuperID.clear();
	key = "SuperUser";
	int iCount = f.GetKeyVal(key, "SuperUserCount", 0);
	CString strText;
	long lUserID = 0;
	for (int j = 0; j < iCount; j++)
	{
		strText.Format("SuperUserID%d", j);
		lUserID = f.GetKeyVal(key, strText, 0);
		s_SuperID.push_back(lUserID);
	}

}

void Config::Init( uint roomID )
{

}
