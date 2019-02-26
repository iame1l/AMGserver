/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

// MainServer.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

// 主符号
#include "resource.h"


// 从dll模块表查询模块句柄
HMODULE ModuleGetFromMap(UINT);

// 从模块中取出组件接口
HRESULT ModuleCreateInstance(UINT,char*,LPVOID FAR**);
HRESULT ModuleCreateInstance(UINT,REFCLSID,char*,LPVOID FAR**);


// CCenterServerApp:
// 有关此类的实现，请参阅 MainServer.cpp
//

#include <map>
using namespace std;

class CMainServerApp : public CWinApp
{
public:
	// 构造函数
	CMainServerApp();

public:
	// 初始化
	virtual BOOL InitInstance();

	// 实现
	DECLARE_MESSAGE_MAP()
};

extern CMainServerApp theApp;
