/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#include "AfxTempl.h"
#include "MainManageForWeb.h"
#include "GamePlaceMessage.h"
#include "ZServerManager.h"

/*
 * 游戏登陆管理类
 */
class CCenterServerManage : public CBaseMainManageForWeb
{
public:
	// 构造函数
	CCenterServerManage(void);
	
	// 析构函数
	virtual ~CCenterServerManage(void);

public:
	// 数据管理模块启动
	virtual bool OnStart();

	// 数据管理模块关闭
	virtual bool OnStop();

private:
	// 数据管理模块初始化
	virtual bool OnInit(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData);

	// 数据管理模块卸载
	virtual bool OnUnInit();

	// 获取信息函数
	virtual bool PreInitParameter(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData);

	// SOCKET 数据读取
	virtual bool OnSocketRead(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);

	// SOCKET 关闭
	virtual bool OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, long int lConnectTime);

	// 数据库处理结果
	virtual bool OnDataBaseResult(DataBaseResultLine * pResultData);

	// 定时器消息
	virtual bool OnTimerMessage(UINT uTimerID);

	// A端负载均衡
	void RandAServer();

	// 读取配置文件
	void GetINIFile();

	///< 功能 从配置文件中读取URL
	///< @param strKey
	///< @Return void
	void GetURL(char* strKey);

	// 从Function.bcf中读取功能配置
	void GetFunction();
	

	class CMainserverList
	{
	private:
		// 清除服务器列表
		void clear();

		// 互斥锁
		CRITICAL_SECTION cs;
	//public:	void ReadINIFile(CString TMLkey);
	public:
		// 获取服务器配置
		void LoadServerList(DWORD cfgHandle);
		struct Node
		{
			CString IPAddr;
			long Port;
			int id;
		};

		// 服务器列表
		CPtrList m_List;

		// 加锁
		void Lock(){}//EnterCriticalSection(&cs);}

		// 解锁
		void UnLock(){}//LeaveCriticalSection(&cs);}

		// 构造函数
		CMainserverList();

		// 析构函数
		virtual ~CMainserverList();

		// 负载均衡
		bool RandAServer(char* ipaddr,long& port);
	};
	//CMainserverList m_MainserverList;
public:
	// 游戏全局参数
	CenterServerMsg m_msgSendToClient;
	CZServerManager m_ZServerManager;

};

/******************************************************************************************************/

/*
 * 游戏登陆模块
 */
class CCenterServerModule : public IModuleManageService
{
public:
	// 登陆数据管理
	CCenterServerManage						m_LogonManage;					

public:
	// 构造函数
	CCenterServerModule(void);

	// 析构函数
	virtual ~CCenterServerModule(void);

public:
	// 初始化函数
	virtual bool InitService(ManageInfoStruct * pInitData) override;

	// 卸载函数
	virtual bool UnInitService() override;

	// 开始函数
	virtual bool StartService(UINT &errCode) override;

	// 停止函数
	virtual bool StoptService() override;

	// 删除函数
	virtual bool DeleteService() override;
};

/******************************************************************************************************/
