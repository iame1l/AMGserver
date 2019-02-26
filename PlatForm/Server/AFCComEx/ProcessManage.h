/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#ifndef A41A948D_8E6F_405c_AB5B_04AE11E53BD
#define A41A948D_8E6F_405c_AB5B_04AE11E53BD

#include "AFCLock.h"

//进程管理类
class EXT_CLASS CProcessManage
{
	//变量定义
protected:
	

protected:
	static bool							g_bInit;					//是否初始化
	static CPtrArray					g_pManageArray;				//进程管理类数组
	static CAFCSignedLock				g_Lock;						//资源访问锁

	//函数定义
public:
	//构造函数
	CProcessManage(void);
	//析构函数
	~CProcessManage(void);

	//功能函数
public:
	//启动进程

};

#endif