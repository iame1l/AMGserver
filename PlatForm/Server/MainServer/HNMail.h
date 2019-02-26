/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#ifndef HNMAIL_SERVICE_HEAD_FILE
#define HNMAIL_SERVICE_HEAD_FILE

#include "Stdafx.h"

class CGameLogonManage;

class CHNMail
{
public:
	CHNMail(CGameLogonManage* pLogonManage);
	~CHNMail(void);

public:
	//收到网络请求
	bool OnNetMessage(NetMessageHead* pNetHead,void* pData,UINT uSize,ULONG uAccessIP,UINT uIndex,DWORD dwHandleID);

	//处理网络请求结果处理
	bool OnDataBaseResult(DataBaseResultLine* pResultData); 

public:
	bool getMailListRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		// 获取邮件列表
	bool openMailRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);			// 打开邮件
	bool getAttachmentRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);		// 获取附件
	bool delMailRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);			// 删除邮件
	bool getSysMsgRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);			//

public:
	bool getMailListResponse(DataBaseResultLine* pResultData);							//
	bool openMailResponse(DataBaseResultLine* pResultData);								//
	bool getAttachmentResponse(DataBaseResultLine* pResultData);						//
	bool delMailResponse(DataBaseResultLine* pResultData);								//
	bool mailUpdateResponse(DataBaseResultLine* pResultData);							//
	bool sysmsgUpdateResponse(DataBaseResultLine* pResultData);							//

private:
	CGameLogonManage* _pLogonManage;
};

#endif