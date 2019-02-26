/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "StdAfx.h"
#include "SmsServer.h"
#include "GameLogonManage.h"
#include "commonuse.h"

CSmsServer::CSmsServer(CGameLogonManage* pLogonManage)
	:_pLogonManage(pLogonManage)
{
	m_hModual = NULL;
	m_pISms = NULL;

	//加载SMS通讯模块 -Sms.dll
	m_hModual = ::LoadLibrary(_TEXT("plugin\\Sms.dll"));
	if(m_hModual)
	{
		typedef HRESULT (*func_ptr)(REFCLSID,LPVOID FAR**);
		func_ptr lpFunc = (func_ptr)::GetProcAddress(m_hModual,_T("CreateSmsObject"));
		if (lpFunc)
		{
			lpFunc(CLSID_ZServ_Sms,(LPVOID FAR**)&m_pISms);
		}
	}
	CBcfFile f(CBcfFile::GetAppPath() + "HNGameLogon.BCF");
	CString strValue;

	if(!f.IsFileExist())
	{
		return;
	}

	char szKey[32];
	_stprintf(szKey, "SMS_Mobile_VCode_Sign");
	CString strSign = f.GetKeyVal("Plugin", szKey, "【红鸟游戏】");
	m_strSmsContent = "您好%s，短信验证码：%s";
	m_strSmsContent += strSign;

}

CSmsServer::~CSmsServer(void)
{
	::FreeLibrary(m_hModual);
}


bool CSmsServer::OnNetMessage(NetMessageHead* pNetHead,void* pData,UINT uSize,ULONG uAccessIP,UINT uIndex,DWORD dwHandleID)
{
	if (!_pLogonManage) 
	{
		return true;
	}

	if (pNetHead->bAssistantID == ASS_GP_SMS_VCODE)
	{
		return sendMessageRequest(pData,uSize,uIndex,dwHandleID);
	}

	return true ;
}

bool CSmsServer::sendMessageRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	MSG_GP_SmsVCode* pSmsVCodeReq = (MSG_GP_SmsVCode*)pData;

	//生成验证码(6位)
	char szVCode[16] = {0};
	::srand(::GetTickCount());
	for (int i = 0; i < 6; ++i)
	{
		szVCode[i] = 0x30 + ::rand() % 10;
	}

	MD5_CTX Md5;
	Md5.MD5Update((unsigned char*)szVCode,6);
	unsigned char szMDTemp[16];
	Md5.MD5Final(szMDTemp);
	for (int i=0;i<16;i++) 
	{
		wsprintf(&pSmsVCodeReq->szVCode[i*2],"%02x",szMDTemp[i]);
	}

	//在此添加发短信获取验证码的功能
	char szMessageBuf[256] = {0};
	_stprintf(szMessageBuf,m_strSmsContent,pSmsVCodeReq->szName,szVCode);

	if (m_pISms)
	{
		HRESULT hr = m_pISms->SendMessage(pSmsVCodeReq->szMobileNo,szMessageBuf);

		int nSendTimes = 3;
		while(hr != S_OK && nSendTimes != 0)
		{// 初次发送有可能失败，类似数据库链接失效，因此需要再次尝试发送
			nSendTimes--;
			hr = m_pISms->SendMessage(pSmsVCodeReq->szMobileNo,szMessageBuf);
		}

		if (hr != S_OK)
		{
			pSmsVCodeReq->szVCode[0] = 0;
		}
	}
    
	_pLogonManage->m_TCPSocket.SendData(uIndex,pData,uSize,MDM_GP_SMS,ASS_GP_SMS_VCODE,0,dwHandleID);

	return true;
	
}
