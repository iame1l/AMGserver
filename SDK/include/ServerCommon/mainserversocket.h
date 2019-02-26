/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once
#include "HNBaseType.h"
#include "AFCInterface.h"
#include "rminfo.h"

//动态创建房间
#define MDM_GP_CREATE_ROOM				136
#define ASS_GP_CREATE_ROOM              			2
#define ASS_GP_MODIFY_ROOM              			3

class CGameMainManage;
class CMainServerSocket : public IClientSocketService
{
public:
	CMainServerSocket(CGameMainManage* pManager);
	~CMainServerSocket();

	virtual bool OnSocketReadEvent(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	virtual bool OnSocketConnectEvent(UINT uErrorCode, CTCPClientSocket * pClientSocket);
	virtual bool OnSocketCloseEvent();

	CTCPClientSocket *GetTCPClientSocket() { return m_pSocket; }

	INT32 Initial();
	INT32 Connect(const char* szServerIP,UINT32 uPort);

	int SendData(void *pData,UINT uSize,UINT bMainID,UINT bAssistID,UINT uHandleCode);
	int SendData(UINT uMainID, UINT uAssistantID,UINT uHandleCode);
	void CloseSocket(bool BNotify);

	bool CheckReConnect();
	bool CheckKeepLive();

private:
	CGameMainManage			*m_pManager;
	CTCPClientSocket		*m_pSocket;
	int						m_bConnect;			//0表示末开始连接，1表示已经发起连接但没返回，2表示连接成功
	int						m_nKeepAliveCount;
	CString					m_strHostIP;		//网关ip地址
};