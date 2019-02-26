/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#include "PropDatabaseMessage.h"
class CPropService
{
	CWnd *pParentWnd;
public:
	 explicit CPropService(CWnd *pWnd);
public:
	~CPropService(void);
public:
	bool OnNetMessage(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	bool OnUseProp(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	bool OnUserSmallBoardcase(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);

	bool OnDataBaseResult(DataBaseResultLine * pResultData);
	bool OnUsePropResult(DataBaseResultLine *pResultData);
	bool OnUserSmallBoardcaseResult(DataBaseResultLine *pResultData);
};
