/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once
#include "ServiceControl.h"

//视图窗口类
class CChildView : public CWnd
{
	//控件变量
public:
	CServiceControl							m_ServiceControl;						//服务控制

	//函数定义
public:
	//构造函数
	CChildView();
	//析构函数
	virtual ~CChildView();

	DECLARE_MESSAGE_MAP()

	//重载函数
protected:
	//初始化建立参数
	virtual BOOL PreCreateWindow(CREATESTRUCT & cs);

	//消息函数
protected:
	//重画函数
	afx_msg void OnPaint();
	//建立消息
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//位置消息
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

