/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#ifndef CAFCLOADERAPP_H_2003
#define CAFCLOADERAPP_H_2003

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "Resource.h"
#include "ServiceManage.h"

//应用程序类
class CAFCLoaderApp : public CWinApp
{
	//函数定义
public:
	bool CheckBanBen();
	//构造函数
	CAFCLoaderApp();
	//初始化函数
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern	CAFCLoaderApp	theApp;				//应用程序
extern	CServiceManage	g_Service;			//游戏服务

#endif