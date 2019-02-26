#include "stdafx.h"
#include "Config.h"


int Config::s_TickInterval = 100;

bool Config::s_PlayBackDebug = false;

bool Config::s_PrintLog = true;

int Config::iXiaZhuTime = 15;

int Config::iKaiPaiTime = 2;

int Config::iFreeTime = 3;

vector<long> Config::s_SuperID;

__int64 Config::chouma[CHOUMA_NUM] = {1,10,50,100,500,1000};

int Config::baozibeilv = 10;
int Config::shunjinbeilv = 5;
int Config::jinhuabeilv = 3;
int Config::shunzibeilv = 2;
int Config::duizibeilv = 2;


void Config::Init()
{
	CString path = CINIFile::GetAppPath();//本地路径
	CINIFile f(path + SKIN_FOLDER + "_s.ini");
	//游戏配置信息
	CString key = TEXT("game");

	for (int i = 0; i < CHOUMA_NUM; i++)
	{
		CString strText;
		strText.Format("chouma%d", i);
		chouma[i] = f.GetKeyVal(key, strText, 100);
	}
	baozibeilv = f.GetKeyVal(key, "baozibeilv", 10);
	shunjinbeilv = f.GetKeyVal(key, "shunjinbeilv", 5);
	jinhuabeilv = f.GetKeyVal(key, "jinhuabeilv", 3);
	shunzibeilv = f.GetKeyVal(key, "shunzibeilv", 2);
	duizibeilv = f.GetKeyVal(key, "duizibeilv", 2);


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
