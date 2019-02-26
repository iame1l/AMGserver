/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/
// UpGradeDLL.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include <afxdllx.h>
#include "ServerManage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static AFX_EXTENSION_MODULE UpGradeDLLDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		if (!AfxInitExtensionModule(UpGradeDLLDLL, hInstance))
			return 0;
		new CDynLinkLibrary(UpGradeDLLDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		AfxTermExtensionModule(UpGradeDLLDLL);
	}
	return 1;   // 确定
}

//获取游戏组件信息
extern "C" __declspec(dllexport) BOOL GetServiceInfo(ServerDllInfoStruct * pServiceInfo, uint uVer)
{
	if (pServiceInfo==NULL) return FALSE;
	memset(pServiceInfo,0,sizeof(ServerDllInfoStruct));
	pServiceInfo->uServiceVer=GAME_MAX_VER;
	pServiceInfo->uNameID=NAME_ID;
	pServiceInfo->uDeskPeople=PLAY_COUNT;
	pServiceInfo->uSupportType=SUPPER_TYPE;
	lstrcpy(pServiceInfo->szGameName,GAMENAME);
	lstrcpy(pServiceInfo->szWriter,TEXT(""));
	lstrcpy(pServiceInfo->szDllNote,TEXT(""));
	return TRUE;
}

//获取游戏管理接口函数
extern "C" __declspec(dllexport) IModuleManageService * CreateServiceInterface(uint uVer)
{
	try	{ return new CGameModuleTemplate<CServerGameDesk,1,10>; }
	catch (...) {}
	return NULL;
}
