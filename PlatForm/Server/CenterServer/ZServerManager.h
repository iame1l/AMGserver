/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.


All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once
//#include "e:\work\d_platform(2.2)分离\sdk\include\servercommon\mainmanage.h"

#include <vector>
#include <set>
using namespace std;

#include "basemessage.h"
#include "MainManage.h"



struct ZSvrNode_st
{
	// 服务器Ip地址
	CString szIPAddr;

	// 服务器端口
	long	lPort;

	// 服务器ID
	int		nZid;

	// Z服务器与B服务器的Socket号
	UINT	uSocket;

	// 服务器在线用户
	set<UINT>	onlineUserSocket;

	ZSvrNode_st()
		:szIPAddr(_T(""))
		,lPort(0)
		,nZid(0)
		,uSocket(0)
	{
	}
};


class CMainserverList
{
private:
	// 本机ip
	CString m_strHostIP;

	// 清除服务器
	void clear();

	// 临界区
	CRITICAL_SECTION cs;

public:
	// 服务器列表
	vector<ZSvrNode_st> m_List;

	// 加锁
	void Lock(){}

	// 解锁
	void UnLock(){}

	// 根据SocketID找相应服务器
	ZSvrNode_st* GetServerBySocket(UINT uSocket);

	// 根据UserId找相应服务器
	ZSvrNode_st* find(DWORD dUserId);

	// 添加一个Z服务器
	bool InsertServer(CString szIP, long lPort, int nZid, UINT uSocket);

	// 删除一个Z服务器
	bool RemoveServer(UINT uSocket);

	// 调试函数
	void PrintDebug();

	// 构造函数
	CMainserverList();

	// 析构函数
	virtual ~CMainserverList();

	// Z服务负载均衡，最优分配法，查找在线人数最少的服务器
	bool RandAServer(char* ipaddr,UINT& port);
};


class CZServerManager: public CBaseMainManage
{
public:
	// 构造函数
	CZServerManager(void);

	// 析构函数
	virtual ~CZServerManager(void);

private:
	// 获取信息函数
	virtual bool PreInitParameter(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData);

	// SOCKET 数据读取
	virtual bool OnSocketRead(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);

	// SOCKET 关闭
	virtual bool OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, long int lConnectTime);

	// 数据库处理结果
	virtual bool OnDataBaseResult(DataBaseResultLine * pResultData){return true;}

	// 定时器消息
	virtual bool OnTimerMessage(UINT uTimerID){return true;}

public:
	CMainserverList							m_MainserverList;
};