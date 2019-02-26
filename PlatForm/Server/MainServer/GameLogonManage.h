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
#include "GameLogonDT.h"
#include "GameListCtrl.h"

//上传头像
#include "UserCustomLogo.h"

//IM类
#include "IMService.h"

//银行类
#include "BankService4Z.h"

#include "ContestServer4Z.h"
#include "RegisteServer.h"
#include "LogonServer.h"
#include "UpdateUserInfoServer.h"
#include "SmsServer.h"
#include "HNCreateRoom.h"
#include "HNClub.h"
#include "HNMail.h"
#include "HNHandleGMessage.h"
/// GM管理工具所用头文件
#include "InterfaceForGM.h"

// 分布式处理
#include "DistriManager.h"
#include "UserManager.h"

#include "ServiceDataBaseHandle.h"
#include "gamemanagemodule.h"
#include "HNFilter.h"
#include "rminfo.h"
#include <map>
using namespace std;


//宏定义 
#define TID_UPDATE_LIST							50					//更新列表 ID
#define LS_TIME_OUT								6000L				//列表超时数



// ----------------------------------------------------------------------------
// 增加使用第三方DLL文件的声明
// ----------------------------------------------------------------------------

// 回调消息结构定义如下
typedef struct msg_head
{
    unsigned long serial_no;        // 消息流水号
    int opr_ret;                    // 操作结果
    unsigned char* data;            // 消息体
} MSG_CONTEXT_HEAD, *LPMSG_CONTEXT_HEAD;


typedef struct user_info
{
    char chUserName[50];
    char chMD5PSW[50];

    int iUserIndex;
}USER_INFO, *LPUSER_INFO;


// 消息类型
#define MSG_TYPE_LOGON              1       // 登录消息
#define     MSG_OP_LOGON_SUCCESS    0       // 用户登录成功
#define     MSG_OP_LOGON_INFO_ERROR 66537   // 用户名或密码错误
#define     MSG_OP_LOGON_USING      66539   // 用户已登录
#define     MSG_OP_LOGON_NO_USER    66542   // 用户不存在
#define     MSG_OP_LOGON_FORBID     66547   // 用户处于禁用状态
#define     MSG_OP_LOGON_OVERDATE   66551   // 用户已过期

// 消息回调函数接口定义：
typedef void (__stdcall *MSG_CALL)(int msg_type, LPMSG_CONTEXT_HEAD msg_context_ptr);

// 函数指针说明

// 验证接口动态库的license
// bCallDesc：调用方标识，定义如下：
#define KFGAME 66
typedef void (__stdcall *SET_LICENCE_FUNC_PTR)(const char* licence, unsigned char bCallDesc);

// 初始化连接接口
typedef int (__stdcall *INIT_FUNC_PTR)(MSG_CALL msg_func, int thread_num, int conn_num);

// 释放动态库接口
typedef bool (__stdcall *RELEASE_FUNC_PTR)();

// 用户合法性验证接口
typedef void (__stdcall *LOGIN_FUNC_PTR)(const char* szUser, const char* szPwd, int iSerialNo);

// 消息回调函数
void __stdcall MsgHandleCallback(int msg_type, LPMSG_CONTEXT_HEAD msg_context_ptr);


// DLL 文件名称
#define DLL_FILE_NAME_OPETV     "OpetvInterfaceDll.dll"


/******************************************************************************************************/

/******************************************************************************************************/

/*
 * 游戏登陆管理类 
 */
class CGameLogonManage : public CBaseMainManageForWeb, public IGMFunction, public IDistriManagerNetBase
{
private:
	// 用于GM消息处理过程
	CGMMessageBase					*m_pGmMessageBase;
public:
	IDistriManagerBase				*m_pDistriManager;
public:
	/// 实现从IGMFunction继承过来的接口
	/// 向客户端发送数据
	virtual int SendDataByUser(UINT uUserID, BYTE bMainID, BYTE bAssistantID, UINT bHandleCode, DWORD dwHandleID)
	{
		int nSockIndex = GetIndexByID(uUserID);
		if(nSockIndex >= 0)
		{
			return SendData(nSockIndex, bMainID, bAssistantID, bHandleCode, dwHandleID);
		}
		else
		{
			return m_pDistriManager->SendData(uUserID, bMainID, bAssistantID, bHandleCode, dwHandleID);
		}
	}
	virtual int SendDataByUser(UINT uUserID, void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, UINT bHandleCode, DWORD dwHandleID)
	{
		int nSockIndex = GetIndexByID(uUserID);
		if(nSockIndex >= 0)
		{
			return SendData(nSockIndex, pData, uBufLen, bMainID, bAssistantID, bHandleCode, dwHandleID);
		}
		else
		{
			return m_pDistriManager->SendData(uUserID, pData, uBufLen, bMainID, bAssistantID, bHandleCode, dwHandleID);
		}
	}
	virtual int SendData(UINT uIndex, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID)
	{
		return m_TCPSocket.SendData(uIndex, bMainID, bAssistantID, bHandleCode, dwHandleID);
	}
	virtual int SendData(UINT uIndex, void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode, DWORD dwHandleID)
	{
		return m_TCPSocket.SendData(uIndex, pData, uBufLen, bMainID, bAssistantID, bHandleCode, dwHandleID);
	}
	/// 向数据库模块发送访问请求
	virtual bool PushDataLine(DataBaseLineHead * pData, UINT uSize, UINT uHandleKind, UINT uIndex,DWORD dwHandleID)
	{
		return m_SQLDataManage.PushLine(pData, uSize, uHandleKind, uIndex, dwHandleID);
	}
	/// 判断某连接是否还存在
	virtual bool IsConnectID(UINT uIndex, DWORD dwHandleID)
	{
		return m_TCPSocket.IsConnectID(uIndex, dwHandleID);
	};
	/// 向客户端发送广播数据
	virtual int SendBatchData(void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, BYTE bHandleCode)
	{
		return m_TCPSocket.SendDataBatch(pData, uBufLen, bMainID, bAssistantID, bHandleCode);
	}
	/// 根据用户ID获取Socket序号
	virtual int GetIndexByID(int iUserID)
	{
		for(int i=0; i<m_UserList.GetCount(); i++)
		{
			LPUSER p = (LPUSER)m_UserList.GetAt(i);
			if(p == NULL)
				continue;
			if(p->UserID==iUserID)
			{
				return p->iSocketIndex;
			}
		}
		return -1;
	}

	virtual LPUSER GetUserByID(int iUserID)
	{
		for(int i=0; i<m_UserList.GetCount(); i++)
		{
			LPUSER p = (LPUSER)m_UserList.GetAt(i);
			if(p == NULL)
				continue;
			if(p->UserID==iUserID)
			{
				return p;
			}
		}
		return NULL;
	}


	/// 根据用户Socket序号获取用户信息
	virtual LPUSER GetUserBySockIdx(UINT uSockIdx)
	{
		if (m_UserList.GetCount() == 0)
		{
			return NULL;
		}

		for(int i = 0; i < m_UserList.GetCount(); ++i)
		{
			LPUSER p = (LPUSER)m_UserList.GetAt(i);
			if(!p)
			{
				return NULL;
			}
			if(uSockIdx == p->iSocketIndex)
			{
				return p;
			}
		}

		return NULL;
	}

	///根据
	/// 判断某玩家是否已经登录
	virtual bool IsUserAlreadyLogon(DWORD dwUserID)
	{
		for(int i=0; i<m_UserList.GetCount(); i++)
		{
			LPUSER p = (LPUSER)m_UserList.GetAt(i);
			if(p == NULL)
				continue;
			if(p->UserID==dwUserID)
			{
				return true;
			}
		}
		return false;
	}

	///根据
	///	判断某玩家是否已经登录（通过用户名）
	virtual bool IsUserAlreadyLogonByName(char * const szUserName)
	{
		if (szUserName == NULL)
		{
			return false;
		}
		for (int i=0; i<m_UserList.GetCount(); i++)
		{
			LPUSER p = (LPUSER)m_UserList.GetAt(i);
			if(p == NULL)
				continue;
			if(stricmp(p->UserName, szUserName) == 0)
			{
				return true;
			}
		}
		return false;
	}
	/// 判断某玩家是否是GM
	virtual bool IsGameMaster(UINT uIndex)
	{
		for(int i=0; i<m_UserList.GetCount(); i++)
		{
			LPUSER p = (LPUSER)m_UserList.GetAt(i);
			if(p == NULL)
				continue;
			if(p->iSocketIndex == (int)uIndex)
			{
				return p->bIsGameManager;
			}
		}
		return false;
	}
	/// 添加玩家登录记录
	virtual bool AddLogonUser(DWORD dwUserID, UINT uIndex, bool bIsGM = false)
	{
		LPUSER p = (LPUSER)new USERS();
		p->iSocketIndex = uIndex;
		p->UserID = dwUserID;
		p->bIsGameManager = bIsGM;
		m_UserList.Add(p);

		MSG_ZDistriInfo info;
		info.dwUserID = dwUserID;
		info.nZID = m_ID;
		m_pDistriManager->SendNormalData(&info, sizeof(info), MDM_ZD_PACKAGE, ASS_ZD_LOGON, HDC_ZD_CONNECT);
		
		return true;
	}


private:
	void SendRoomListPeoCount();

public:
	// 登陆服务器端口
	UINT m_nPort;															

	// 支持最大人数
	UINT m_uMaxPeople;

    
	
	//游戏列表
	CServerGameListManage						m_GameList;
	
	//用户列表
	CPtrArray									m_UserList;
	
	// 添加防止账户同时登陆,ZID
	int m_ID;

	CString		m_szDownLoadURL;

private:
	// 头像上传处理类
	CUserCustomLogo			*m_userCustomLogo;
	
	// IM处理类
	CIMService				*m_imService;	

	//银行服务处理业类
	CBankService4Z          *m_pBankService;

	// 比赛处理接口
	CContestServer4Z        *m_pContestServer;
	CRegisteServer			*m_pRegisterServer;
	CLogonServer			*m_pLogonServer;
	CUpdateUserInfo			*m_pUpdateUserInfo;
	CSmsServer				*m_pSmsServer;

public:
	// MServer 与 GServer通信
	CGameManageModule		*m_pGameManageModule;	
	CCreateRoom             *m_pCreateRoom;         //开房相关类
	CHandleGMessage			*m_pHandleGMessage;		//处理GServer消息
	CHNClub					*m_pHNClub;
	CHNMail					*m_pHNMail;

public:
	// 用户目录
	CString				m_CustomFacePath;

	// 用户传目录
	CString				m_CustomFacePathUpload;

	// 头像是否需要审核
	bool				m_bCustomFaceNeedCheck;
	
	HNFilter            m_HNFilter;

	map<int, TMailItem> m_Mails;

	TMSG_GP_RebateUpDate m_Rebate;//转账返利
	//函数定义
public:
	//构造函数
	CGameLogonManage(void);
	CGameLogonManage(CGameLogonManage&);
	CGameLogonManage & operator = (CGameLogonManage &);
	//析构函数
	virtual ~CGameLogonManage(void);
	//获取游戏列表
	const CServerGameListManage * GetGameList() { return &m_GameList; };

	//功能函数
public:
	//数据管理模块启动
	virtual bool OnStart();
	//数据管理模块关闭
	virtual bool OnStop();

	virtual bool Init(ManageInfoStruct * pInitData, IDataBaseHandleService * pDataHandleService);

	virtual bool UnInit();

	//SOCKET 关闭
	virtual bool OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, long int lConnectTime);
	//服务扩展接口函数
private:
	//获取信息函数
	virtual bool PreInitParameter(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData);
	//SOCKET 数据读取
	virtual bool OnSocketRead(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	//数据库处理结果
	virtual bool OnDataBaseResult(DataBaseResultLine * pResultData);
	//定时器消息
	virtual bool OnTimerMessage(UINT uTimerID);
	//道具消息
	void OnPropMessage(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);

	///获取在线GM列表
	void OnPlayer2Gm(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);

	//下发游戏列表
	bool onSendGameList(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);

	CGameManageModule* GetpGameManageModule() { return m_pGameManageModule; }

	void GetINIFile();

///////////////////////////////
	/// 根据用户ID获取Socket序号
	/// 向客户端发送广播数据

	/// 向客户端发送广播数据
	virtual int SendBatchDataSelf(void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, UINT bHandleCode)
	{
		return m_TCPSocket.SendDataBatch(pData, uBufLen, bMainID, bAssistantID, bHandleCode);
	}

	virtual int KickUserSocket(DWORD dwUserID)
	{
		return 0;
	}

	virtual int GetZid()
	{
		return m_ID;
	}

	virtual int GetServerPort()
	{
		return m_nPort;
	}
	// B服务器无法在Z服务器列表中找到玩家时，会反馈消息包给发起的Z服务器。这里进行容错处理。
	virtual int SendDataFail(UINT uUserID, void * pData, UINT uBufLen, BYTE bMainID, BYTE bAssistantID, UINT bHandleCode, DWORD dwHandleID);

public:
	bool OnHandleGMessage(NetMessageHead *pNetHead, void *pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);

	bool SetRoomConnect(int iRoomID,bool bConnect);
///////////////////////////////
};

/******************************************************************************************************/

//游戏登陆模块
class CGameLogonModule : public IModuleManageService
{
public:
	// 登陆数据管理
	CGameLogonManage						m_LogonManage;

	// 数据库数据处理模块
	CServiceDataBaseHandle					m_DataBaseHandle;
	
public:
	// 构造函数
	CGameLogonModule(void);

	// 析构函数
	virtual ~CGameLogonModule(void);

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
