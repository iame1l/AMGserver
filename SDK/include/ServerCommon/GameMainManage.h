/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#ifndef GAMEMAINMANAGE_H_FILE
#define GAMEMAINMANAGE_H_FILE

#include "Afxtempl.h"
#include "MainManageForweb.h"
#include "GameDesk.h"
#include "UserInfo.h"
#include "AFCSocket.h"
#include "PropService.h"

#include "GM_MessageDefine.h"
#include "InterfaceForGM.h"

#include "BankService.h"
#include "CmdGameLogic.h"

#include "IAutoAllotDesk.h"
#include "GameDataBaseHandle.h"
#include "mainserversocket.h"

#include "HNHandleMSMessage.h"

#include "HNFilter.h"
#include "rminfo.h"
#include <map>
#include <list>

using namespace std;

///类说明
class CGameDesk;
class CBaseMainManage;
class CGameUserInfoManage;

///模式数值定义
#define ACCEPT_THREAD_COUNT				1									///应答线程数目
#define SOCKET_THREAD_COUNT				4									///网络线程数目

///结构说明
struct MSG_GR_SR_MatchUser;
struct DL_GR_I_RegisterLogon;
struct DL_GR_O_RegisterLogon;

///宏定义
#define TIME_SPACE						60L									///游戏 ID 间隔
#define TIME_START_ID					100L								///定时器开始 ID
                                 
//房间管理窗口定义宏
#define GRM_SET_KEY							TEXT("GRM_SET")					//读取配置文件时所用的关键字
#define GRM_SET_KEY_WIN						TEXT("GRM_win")					//读取配置文件时所用的关键字(胜率)
#define GRM_SET_KEY_LOS						TEXT("GRM_los")					//读取配置文件时所用的关键字(输率)

/*******************************************************************************************************/



/*******************************************************************************************************/

///比赛信息结构
struct MatchInfoStruct
{
	long int									dwUserID;							///用户 ID
	TCHAR										szUserNname[61];					///用户名字
	UINT										uPlayCount;							///限制局数
	long int									dwPlayPoint;						///限制分数
};

///限制聊天信息
struct LimitTalkStruct
{
	long int									dwUserID;							///用户 ID
	long int									dwLimitedTime;						///限制时间
};

/*******************************************************************************************************/

///基础数据管理类
class AFX_EXT_CLASS CGameMainManage : public CBaseMainManageForWeb
{
	friend class CGameDesk;

	///变量定义
protected:
	UINT										m_uDeskCount;						///游戏桌数目
	CGameDesk									* * m_pDesk;						///游戏桌指针
	CGameDesk									* m_pDeskArray;						///游戏桌指针

	UINT										m_uNameID;							///游戏名字 ID 号码
public:
	char										m_szGameName[64];					///游戏名字
	char										m_szPlayBackURL[256];				///回放文件保存地址

    bool                                        m_bSetDynamiced;
    BYTE                                        m_szDynamicPlayingMethod1[128];
    bool                                        m_bSetRoomDynamiced;
    BYTE                                        m_szDynaminRoomInfo[128];

	///状态变量
protected:
	bool										m_bStopLogon;						///停止登陆

	///比赛信息
private:
	UINT										m_uMatchUserCount;					///比赛数量
	MatchInfoStruct								* m_pMatchInfo;						///比赛信息
	MSG_GR_SR_MatchUser							* m_pMatchUser;						///比赛用户
	bool										m_ContestTimeover;					///比赛结束
	///限制聊天信息
private:
	CArray<LimitTalkStruct,LimitTalkStruct>		m_RoomLimited;						///房间限制
	CArray<LimitTalkStruct,LimitTalkStruct>		m_GameLimited;						///游戏限制

	///信息变量
protected:
	TCHAR										m_szMsgRoomLogon[500];				///房间消息
	TCHAR										m_szMsgGameLogon[500];				///游戏消息
	TCHAR                                       m_szVIPPassWord[50];                ///< VIP房间密码
	long                                        m_lVIPID;                           ///< 本房间第一个设置密码的VIP ID
	UINT										m_uVIPIndex;                        ///< 本房间第一个设置密码的VIP的套接字索引
	BOOL										m_bIsVIPLogon;                      ///< 是否设置密码的VIP重返密码房间

public:
	CGameUserInfoManage							m_UserManage;						///用户管理
	BYTE                                        m_GameFinishCount;                  ///游戏结束计数
	list<CGameUserInfo*>						m_pGameUserInfoList;				///<排队机list
	int											m_nRate;							///< 税率的分母.100则表示百分几，1000则千分几。分子是 m_pDataManage-m_InitData.uTax
	bool                                        m_bPresentCoin;						///< 是否赠送金币。20把，每天玩家低于若干值时赠送若干金币。
	COleDateTime								m_MatchbeginTime;					//定时赛开赛时间

	CMainServerSocket*							m_pMainServerSocket;				//游戏服务器与主服务器通信
	IAutoAllotDesk*								m_pIAutoAllotDesk;					///< 自动撮桌处理对象接口指针，视情况由OnInit创建，如为空，表示不需要撮桌机制

private:	
	///	CAFCItemData								*m_ItemDate;
	CBankService								*m_pBankService;
	CPropService								*m_PropService;
	CHandleMSMessage							*m_pHandleMSMessage;				//处理MServer消息
	
	double				m_iVIP_Rate;	//vip返利率

    UINT                m_uQueuerMaxPeople;      // 排队机配置项
	int					m_uQueuerPeople;
	int					m_iQueueTime;

	int					m_iRemainPeople;		//比赛场剩余人数
	
	MSG_GR_S_UserQueueInfo* m_pUserQueueBuffer;

private:
	//房间管理窗口数据
	std::vector<long int> m_GRMUserID;		//管理员用户id
	std::vector<UINT> m_GRMRoomID;			//被管理房间id

	///函数定义
public:
	///构造函数
	CGameMainManage(void);
	///析构函数
	virtual ~CGameMainManage(void);
	CGameMainManage(CGameMainManage&);
	CGameMainManage& operator=(CGameMainManage&); 
	//比赛场初始排名随机
	bool RandRank(int *pData, int iLen);
	///判断是否是排队机房间
	///@return true:是排队机房间；false：不是排队机房间
	bool IsQueueGameRoom();

	//该房间是否启动房间管理窗口
	bool IsExistGRMRoom(UINT iRoomID);
	//该玩家是否可以使用房间管理窗口
	bool IsExistGRMUserID(long int UserID);
	//通知管理员客户端奖池已经更新
	void SendRoomPondUpData(__int64 iAIHaveWinMoney);
	//通知管理员客户端完整更新
	void SendRoomPondUpDataALL();

		/// 创建自动撮桌对象
	virtual IAutoAllotDesk *CreateAutoAllotDesk(int iDeskPeopleCount, int iDeskCount);
		   ///服务接口函数 （主线程调用）
	///排队机添加用户
	virtual int AutoAllotDeskAddUser(CGameUserInfo *pInfo);
	///排队机删除用户
	virtual int AutoAllotDeskDeleteUser(CGameUserInfo *pInfo);
	//删除所有排队机用户
	virtual int AutoAllotDeskDeleteAll();
	void NewQueueGame();

	//启动定时赛定时器
	void OnStartTimingMatchTimer(COleDateTime MatchStartTime);
	bool OnTimingMatchBeginFillQueue(DataBaseResultLine * pResultData);

	void UpdateDeskInfo();

	bool OnFindBuyDeskStationResult(DataBaseResultLine * pResultData);
	bool OnReturnDeskResult(DataBaseResultLine * pResultData);
	bool OnCreateGameSNResult(DataBaseResultLine * pResultData);
	bool OnGetDeskInfo(DataBaseResultLine * pResultData);
	bool OnSetReturnGameInfo(DataBaseResultLine * pResultData);

	bool OnContestRegistResult(DataBaseResultLine * pResultData);
	bool OnContestActiveResult(DataBaseResultLine * pResultData);

	bool OnCostFee(DataBaseResultLine * pResultData);
	bool OnGetRoomInfoResult(DataBaseResultLine * pResultData);
	bool OnSetContestInfoResult(DataBaseResultLine * pResultData);
    bool OnDissmissDeskByMS(DataBaseResultLine *pSourceData);
private:
	///数据管理模块初始化
	virtual bool OnInit(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData);
	///数据管理模块卸载
	virtual bool OnUnInit();
	///数据管理模块启动
	virtual bool OnStart();
	///数据管理模块关闭
	virtual bool OnStop();
public:
	///关闭定时器
	virtual bool StopTimer();
	
	///服务扩展接口函数 （本处理线程调用）
private:
	///SOCKET 数据读取
	virtual bool OnSocketRead(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	///SOCKET 关闭
	virtual bool OnSocketClose(ULONG uAccessIP, UINT uSocketIndex, long int lConnectTime);
	///数据库处理结果
	virtual bool OnDataBaseResult(DataBaseResultLine * pResultData);
	///定时器消息
	virtual bool OnTimerMessage(UINT uTimerID);
	///网络处理扩展
public:
	bool OnHandleMSMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
public:
	/// 转账用户ID比较
	/// @param UserID 玩家自己的ID，UserIDDest目标ID,散户ID NomalIDFrom开始，散户ID NomalIDEnd结束
	/// @return  0不能转账，1 可以转账,扣税。2 可以转账,不扣税.3 可以转账都为非特别ID，检查转账金额
	static int TransferIDCompare(long UserID,long UserIDDest,long NomalIDFrom,long NomalIDEnd);
	/// 获取玩家ID信息，是否散户，特别账号，指派账号
	/// @param UserID 玩家ID,NomalIDFrom散户ID开始，NomalIDEnd散户ID结束
	/// @return 0到8是指派账号后面的0的个数，-1为散户，-2为无效账号，-5为5连号，-6为6连号
	static int GetIDInformation(long UserID,long NomalIDFrom,long NomalIDEnd);
	/// 区取某张游戏桌的指针
	const CGameDesk* GetGameDesk(BYTE bDeskNo);

private:
	//{{ Added by zxd 20100806
	/// GM消息处理模块
	bool OnGMSocketMessage(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	// Added by zxd 20100806}}

	///用户登陆处理 
	bool OnUserLogonMessage(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	///用户动作处理
	bool OnUserActionMessage(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	///用户房间处理
	bool OnRoomMessage(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	///管理消息处理
	bool OnManageMessage(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	///钱柜划账处理
	bool OnAboutMoney(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);

	bool OnUserAction_FastSit(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);

	bool OnUserAction_ChangeDesk(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);

	bool OnGetDeskUserInfo(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	/// 打印房间所有玩家的消息
	/// Added By JianGuankun 2011.12.5
	void PrintUsersLog();
	static bool Writeline(CFile* p,CString str);

	///经验盒处理
	bool OnPointMessage(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	///新增加功能2008.3.5
	///小喇叭
	///bool OnBroadcastMessage(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	///好友
	bool OnFriendManageMsg(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	///排队处理
	bool OnQueueMsg(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	///桌子解散处理
	bool OnDeskDissmiss(NetMessageHead * pNetHead, void * pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	///排队机处理
	void queuerGame();

	bool IsContestGameOver();


	///好友管理结果
	bool OnFriendManageDBOperateResult(DataBaseResultLine *pResultData);
	///好友聊天结果
	bool OnBufferIMMessageDBOperateResult(DataBaseResultLine *pResultData);
	///数据库处理扩展

public:
	//租赁时间到期
	bool IsLeaseTimeOver();

	void OnBeforeMatchBeginInfo();

	///定时器消息用来更新数据给其他服务器
	bool OnTimerMessageForUpdateToOtherServer(UINT uTimerID);
private:
	//玩家返利修改
	bool OnUserSetRebateRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID);

	//将返利数据返还给房间sdp
	bool OnGetBankRebateResponse(DataBaseResultLine* pResultData);
	//将玩家返利查询数据返还给房间sdp
	bool OnGetBankUserRebateResponse(DataBaseResultLine* pResultData, UINT uCode);
	//将玩家返利修改结果返还给房间sdp
	bool OnGetBankUserSetRebateResponse(DataBaseResultLine* pResultData, UINT uCode);
	///房间登陆处理
	bool OnRoomLogonResult(DataBaseResultLine * pResultData);
	///检测连接处理
	bool OnCheckConnect(DataBaseResultLine * pResultData);
	///呼叫网管结果处理
	bool OnCallGMResult(DataBaseResultLine * pResultData);
	///房间权限设置结果处理
	bool OnGamePowerSetResult(DataBaseResultLine * pResultData);
	///钱柜
	bool OnAboutMoneyResult(DataBaseResultLine * pResultData, UINT uCode=ASS_GR_OPEN_WALLET);
	///房间－＞钱柜
	bool OnCheckInMoney(DataBaseResultLine * pResultData, UINT uCode=DTK_GR_CHECKIN_MONEY);
	///房间＜－钱柜
	bool OnCheckOutMoney(DataBaseResultLine * pResultData, UINT uCode=DTK_GR_CHECKOUT_MONEY);
	///转帐
	bool OnTransferMoney(DataBaseResultLine * pResultData, UINT uCode=DTK_GR_TRANSFER_MONEY);
	///修改密码结果
	bool OnChangePasswd(DataBaseResultLine * pResultData, UINT uCode=DTK_GR_CHANGE_PASSWD);
	///转帐记录结果
	bool OnTransferRecord(DataBaseResultLine * pResultData, UINT uCode=DTK_GR_TRANSFER_RECORD);
	///赠送金币结果，add by yjj 080703
	bool SendUserMoenyByTimesResult(DataBaseResultLine * pResultData);
	
	///wushuqun 2009.6.5
	///混战房间记录结果处理
	bool OnBattleRoomRecord(DataBaseResultLine * pResultData);
	///

    //根据用户ID获取昵称
    bool OnGetNickNameOnIDResult(DataBaseResultLine * pResultData);

    //踢人卡、防踢卡功能
    bool OnUseKickPropResult(DataBaseResultLine * pResultData, BOOL bIsKickUser);

    //踢人卡踢人功能
    bool OnNewKickUserProp(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);

    //使用踢人卡踢人
    bool OnNewKickUserPropResult(DataBaseResultLine * pResultData);

	//获取比赛当前排名，成绩等
	bool OnGetContestRankResult(DataBaseResultLine * pResultData);

	//获取比赛当前报名人数
	bool OnBroadCastConstestInfo(DataBaseResultLine * pResultData);

	//比赛开始
	bool OnGetContestBeginResult(DataBaseResultLine *pResultData);

	//用户报名/退赛 比赛房间
	bool OnGetContestApplyResult(DataBaseResultLine *pResultData);

	//比赛取消
	bool OnGetContestAbandonResult(DataBaseResultLine *pResultData);

	//获取比赛结束的信息
	bool OnGetContestGameOverResult(DataBaseResultLine *pResultData);

	bool OnPropChangeResult(DataBaseResultLine *  pResultData);

	//传达语音成功与否的情况
	bool OnVoiceInfo(NetMessageHead *pNetHead, void* pData, UINT uSize, ULONG uAccessIP, UINT uIndex, DWORD dwHandleID);
	///内部函数
private:
	///离开桌子函数
	bool UserLeftDesk(CGameUserInfo * pUserInfo,bool bLeft=true);
	///用户是否游戏
	bool IsUserPlayGame(CGameUserInfo * pUserInfo);
	///用户是否游戏
	bool IsUserPlayGameByStation(CGameUserInfo * pUserInfo);
	///清理用户资料
	bool CleanUserInfo(CGameUserInfo * pUserInfo);
	///是否限制聊天
	bool IsLimitedTalk(CGameUserInfo * pUserInfo, bool bRoom);
	///发送房间信息

	/// 发送用户信息
	/// @param pUserInfo用户信息,uSocketIndex套接字索引,pLogonResult登陆信息
	/// @return 成功为true
	bool SendRoomInfo(CGameUserInfo * pUserInfo, UINT uSocketIndex, DWORD dwHandleID,DL_GR_O_LogonResult * pLogonResult,bool bSend);

	///发送比赛信息
	bool SendMatchInfo(UINT uSocketIndex, DWORD dwHandleID);
	///发送房间消息
	bool SendRoomMessage(UINT uSocketIndex, DWORD dwHandleID, TCHAR * szMessage, BYTE bShowStation=SHS_TALK_MESSAGE, BYTE bFontSize=0, BYTE bCloseFace=FALSE);
	///发送游戏消息
	bool SendGameMessage(UINT uSocketIndex, DWORD dwHandleID, TCHAR * szMessage, BYTE bShowStation=SHS_TALK_MESSAGE, BYTE bFontSize=0, BYTE bCloseFace=FALSE);

	///内部函数
private:
	///初始化游戏桌
	bool InitGameDesk(UINT uDeskCount, UINT	uDeskType);///百家乐
	///获取游戏桌子信息
	virtual CGameDesk * CreateDeskObject(UINT uInitDeskCount, UINT & uDeskClassSize)=0;
public:
	virtual CGameDesk * GetDeskObject(BYTE bIndex);
	///提示发送坐下错误信息
	void SendErrorUserSit(UINT uIndex, UINT bHandleCode, DWORD dwHandleID);
	//用户断线解散桌子
	void OnUserNetCutDissmiss(CGameUserInfo * pUserInfo);
	//得到桌子指针
	CGameDesk *GetDeskInfo(BYTE byDeskNo);
	bool OnClearDesk(BYTE byDeskIndex);
	bool OnReleaseDesk(BYTE byDeskIndex);

	bool BeforeContestBegin();
	void OnContestRoomReset();
};

typedef CGameUserInfo *PGameUserInfo; 

class compare
{       
public:       
    bool operator ()(PGameUserInfo& t1,  PGameUserInfo& t2) const       
    {       
		return t1->GetWeigth() > t2->GetWeigth();  
    }       
};  


/*******************************************************************************************************/

///游戏数据管理类模板
template <class GameDeskClass, UINT uBasePoint, UINT uLessPointTimes> class CGameMainManageTemplate : public CGameMainManage
{
	///函数定义
public:
	///构造函数
	CGameMainManageTemplate(void) {};
	///析构函数
	virtual ~CGameMainManageTemplate(void) {};

	///重载函数
private:
	///获取信息函数 （必须重载）
	virtual bool PreInitParameter(ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData)
	{
		///设置版本信息
		pKernelData->bMaxVer=GAME_MAX_VER;
		pKernelData->bLessVer=GAME_LESS_VER;

		///设置使用网络
		pKernelData->bStartTCPSocket=TRUE;

		///设置数据库信息
		pKernelData->bLogonDataBase=TRUE;
		pKernelData->bNativeDataBase=TRUE;
		pKernelData->bStartSQLDataBase=TRUE;

		///设置线程数目
		pKernelData->uAcceptThreadCount=ACCEPT_THREAD_COUNT;
		pKernelData->uSocketThreadCount=SOCKET_THREAD_COUNT;

		///设置游戏信息
		pKernelData->uNameID=NAME_ID;
		pKernelData->uDeskPeople=PLAY_COUNT;

        if(pKernelData->uMinDeskPeople <= 0)
		    pKernelData->uMinDeskPeople = pKernelData->uDeskPeople;

		///调节人数
		if ((pInitData->uDeskCount*PLAY_COUNT+50)<pInitData->uMaxPeople)
		{
			pInitData->uMaxPeople=pInitData->uDeskCount*PLAY_COUNT+50;
		}

		///调节表名字
		if (lstrcmp(pInitData->szLockTable,TEXT("N/A"))==0) pInitData->szLockTable[0]=0;
		if (lstrcmp(pInitData->szIPRuleTable,TEXT("N/A"))==0) pInitData->szIPRuleTable[0]=0;
		if (lstrcmp(pInitData->szNameRuleTable,TEXT("N/A"))==0) pInitData->szNameRuleTable[0]=0;
		CString msg;
		msg.Format("%d.dll",NAME_ID);

		if (lstrcmp(pInitData->szGameTable,TEXT("N/A"))==0) lstrcpy(pInitData->szGameTable,msg);

		if ((pInitData->uComType==TY_MATCH_GAME)&&(pInitData->szLockTable[0]==0)) 
		{
			AfxMessageBox("比赛房间，但是没有锁定一个表ComLogonMoney");
			return false;
		}

		///金币场必须记录日志
		if (pInitData->uComType==TY_MONEY_GAME)
		{
			pInitData->dwRoomRule|=GRR_RECORD_GAME;
		}

		///调节基础分
		if (pInitData->uBasePoint==0) pInitData->uBasePoint=uBasePoint;///统一为1
		if ((pInitData->uComType==TY_MATCH_GAME)||(pInitData->uComType==TY_MONEY_GAME))
		{
			if (pInitData->uLessPoint<pInitData->uBasePoint)///*uLessPointTimes)///最小入场分为基础分
				pInitData->uLessPoint=pInitData->uBasePoint;///*uLessPointTimes;
			
            // 因为服务费的设置可以很大，不做上限。
            if(pInitData->uTax<0)
				pInitData->uTax=0;

		}

		return true;
	};

	///获取游戏桌子信息
	virtual CGameDesk * CreateDeskObject(UINT uInitDeskCount, UINT & uDeskClassSize)
	{
		uDeskClassSize=sizeof(GameDeskClass);
		return new GameDeskClass [uInitDeskCount];
	};
};

/*******************************************************************************************************/

///游戏模块类模板
template <class GameDeskClass, UINT uBasePoint, UINT uLessPointTimes> class CGameModuleTemplate : public IModuleManageService
{
	///变量定义
public:
	CGameDataBaseHandle														m_DataBaseHandle;	///数据库处理模块
	CGameMainManageTemplate<GameDeskClass,uBasePoint,uLessPointTimes>		m_GameMainManage;	///游戏数据管理

	///函数定义
public:
	///构造函数
	CGameModuleTemplate(void) {};
	///析构函数
	virtual ~CGameModuleTemplate(void) {};

	///接口函数
public:
	///初始化函数 
	virtual bool InitService(ManageInfoStruct * pInitData)
	{
		KernelInfoStruct KernelData;
		memset(&KernelData,0,sizeof(KernelData));
		return m_GameMainManage.Init(pInitData,&m_DataBaseHandle);
	}
	///卸载函数 
	virtual bool UnInitService() { return m_GameMainManage.UnInit(); }
	///开始函数 
	virtual bool StartService(UINT &errCode) { return m_GameMainManage.Start(); }
	///停止函数 
	virtual bool StoptService() { return m_GameMainManage.Stop(); }
	///删除函数
	virtual bool DeleteService() { delete this; return true; }
	///停止计时器
	virtual bool StopAllTimer() {return m_GameMainManage.StopTimer();}
};

/*******************************************************************************************************/

#endif