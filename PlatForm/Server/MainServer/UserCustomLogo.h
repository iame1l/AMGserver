/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once
#ifndef _USER_CUSTOM_LOGO_CLASS_HEADER_
#define _USER_CUSTOM_LOGO_CLASS_HEADER_

#include "UserlogoMessage.h"
#include "UserlogoDatabaseMessage.h"

class CUserCustomLogo
{
	CWnd    *pParentWnd;
public:
	explicit CUserCustomLogo(CWnd *pWnd){pParentWnd=pWnd;};
	void OnNetMessage(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	void OnUploadFace(void *pData,UINT uSize,UINT uIndex,DWORD dwHandleID);
public:
	~CUserCustomLogo(void);
public:
	void OnRequestLogoInformation(void * pData, UINT uSize, UINT uIndex, DWORD dwHandleID);
public:
	void OnDownloadRequest(void* pData, UINT uSize, UINT uIndex, DWORD dwHandleID);
public:
	void downloadCustomFace(long dwUserID, long nBlockIndex, UINT uIndex, DWORD dwHandleID);
public:
	void OnDownloadResult(void * pData, UINT uSize, UINT uIndex, DWORD dwHandleID);
};

#endif