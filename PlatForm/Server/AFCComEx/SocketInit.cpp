/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "StdAfx.h"
#include <winsock.h>
#include "socketinit.h"

CSocketInit::CSocketInit(void)
{
	WSAData wsaData;
	int nReturnCode = ::WSAStartup(MAKEWORD(2,0), &wsaData);
}

CSocketInit::~CSocketInit(void)
{
	::WSACleanup();
}

static CSocketInit   g_static_sktinit;
