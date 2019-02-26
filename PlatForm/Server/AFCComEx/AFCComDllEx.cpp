/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "stdafx.h"
#include <afxdllx.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static AFX_EXTENSION_MODULE AFCComDllExDLL = { NULL, NULL };

//DLL Èë¿Úº¯Êý
extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason==DLL_PROCESS_ATTACH)
	{
		if (!AfxInitExtensionModule(AFCComDllExDLL, hInstance))	return 0;
		new CDynLinkLibrary(AFCComDllExDLL);

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		AfxTermExtensionModule(AFCComDllExDLL);
	}
	return 1; 
}
