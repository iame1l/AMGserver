/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#ifndef SERVERDATABASEHANDLE_H_FILE
#define SERVERDATABASEHANDLE_H_FILE

#include "AFCDataBase.h"
#include "GameLogonDT.h"
#include "GM_MessageDefine.h"
#include "PropDatabaseMessage.h"
#include "IMService.h"
#include "GameListCtrl.h"
#include "UserCustomLogo.h"
#include "BankService4Z.h"

// 数据库处理服务
class CServiceDataBaseHandle : public CDataBaseHandle
{
public:
	// 构造函数
	CServiceDataBaseHandle(void);

	// 析构函数
	virtual ~CServiceDataBaseHandle();

private:
	// 数据库处理接口
	virtual UINT HandleDataBase(DataBaseLineHead * pSourceData);

private:
	// 效验字符
	bool CheckChar(TCHAR * szChar, bool bStrCheck);

private:
	// GM按玩家信息获取已经操作过的情况，玩家ID和昵称对应关系
	UINT OnGMGetOperatedUser(DataBaseLineHead * pSourceData);

	// GM获取奖励方案
	UINT OnGMGetAwardType(DataBaseLineHead * pSourceData);

	// GM获取道具ID与名称对应关系
	UINT OnGMGetPropInfo(DataBaseLineHead * pSourceData);

	// GM对玩家进行奖惩操作
	UINT OnGMOperateToUser(DataBaseLineHead * pSourceData);

	// GM登陆处理
	UINT OnGMLogon(DataBaseLineHead * pSourceData, UINT uLogonType);

	//从移动端快速注册
	UINT OnOneKeyRegister(DataBaseLineHead * pSourceData, UINT uLogonType);
	//大厅登陆处理
	UINT OnPlaceLogon(DataBaseLineHead * pSourceData, UINT uLogonType);
	//更新游戏列表
	UINT OnUpdateGameList(DataBaseLineHead * pSourceData);
	//检测数据库连接
	UINT OnCheckDataConnect(DataBaseLineHead * pSourceData);
	//
	UINT OnGetRoomListPeoCount(DataBaseLineHead * pSourceData);

	UINT OnGetLogonPeopCount(DataBaseLineHead * pSourceData);
	//玩家头像ID
	void OnUpdateUserLogoID(DataBaseLineHead *pSourceData);
	//玩家头像ID md5值
	void OnRequestUserLogoMD5(DataBaseLineHead *pSourceData);
	//取好友数量
	void OnDBIMGetUserCount(DataBaseLineHead * pSourceData);
	//服务器启动用户在线标志清0
	void OnClearOnlineFlag(DataBaseLineHead * pSourceData);
	//用户下线操作
	void OnUserOffline(DataBaseLineHead * pSourceData);
	//设置好友组
	void OnIMSetUser(DataBaseLineHead * pSourceData);
	//获取用户比赛房间ID
	void OnGetContestRoomID(DataBaseLineHead * pSourceData);
	void OnGetApplyNum(DataBaseLineHead * pSourceData);
	void OnContestApply(DataBaseLineHead * pSourceData);
	void OnContestNoticeUser(DataBaseLineHead * pSourceData);
	void OnGetApplyAward(DataBaseLineHead * pSourceData);
	//取得道具信息
	void OnPropGetList(DataBaseLineHead * pSourceDat);
	void OnPropBuy(DataBaseLineHead * pSourceData);

	// 商店道具的即时购买功能
	void OnPropBuyNew(DataBaseLineHead * pSourceData);  
	void OnPropGive(DataBaseLineHead *pSourceData);
	void OnPropBigBrd(DataBaseLineHead *pSourceData);
	void OnPropBigBrd_BuyAndUse(DataBaseLineHead *pSourceData);

	// 锁定机器
	void OnUserLockAccount(DataBaseLineHead * pSourceData);

	// 绑定手机
	void OnUserBindMobile(DataBaseLineHead * pSourceData);

	// 道具界面购买VIP
	void OnPropBuyVIP(DataBaseLineHead * pSourceData);

	// 设置用户头像
	void OnSetUserLogo(DataBaseLineHead * pSourceData);

	// 试图进入密码房间
	void OnCheckRoomPasswd(DataBaseLineHead *pSourceData);

	// 获取报名信息
	void OnGetContestInfo(DataBaseLineHead *pSourceData);

	// 修改写Z登录记录的时机，确保网络卡的时候，不至于导致用户登录不上
	UINT OnWriteUserLogonRecord(DataBaseLineHead * pSourceData);

	// 修改用户基本资料
	UINT OnUpdateUserInfo(DataBaseLineHead* pSourceData,UINT nType);

    //用户账号关联手机号
   // UINT OnAddUserPhoneNum(DataBaseLineHead* pSourceData);

	// 修改密码
	UINT OnUpdateUserPwd(DataBaseLineHead* pSourceData);

	UINT OnCharmExchangeList(DataBaseLineHead * pSourceData);

	UINT OnCharmExchange(DataBaseLineHead * pSourceData);

	UINT OnSetFriendType(DataBaseLineHead * pSourceData);

	// 查询玩家的ID或昵称
	UINT OnQueryUserNickNameID(DataBaseLineHead* pSourceData);

	// 打开银行
	UINT OnBankOpen(DataBaseLineHead* pSourceData);

	// 查询钱包
	UINT OnBankGetGameWallet(DataBaseLineHead* pSourceData);

	// 银行存取
	UINT OnBankCheck(DataBaseLineHead* pSourceData);

	// 提取或者存储返利
	UINT OnBankTransferRebate(DataBaseLineHead* pSourceData);

	// 更新转账返利数据
	UINT OnUpdateRebate(DataBaseLineHead * pSourceData);

	// 更新玩家返利数据
	UINT OnUpdateUserRebate(DataBaseLineHead * pSourceData);

	// 银行转账（增加返利功能）
	UINT OnBankTransfer(DataBaseLineHead* pSourceData);

	// 查询银行转账记录
	UINT OnBankTransRecord(DataBaseLineHead* pSourceData);

	// 存单业务
	UINT OnBankDeposit(DataBaseLineHead* pSourceData);

	// 查询存单
	UINT OnBankQueryDeposits(DataBaseLineHead* pSourceData);

	// 修改银行密码
	UINT OnBankChangePassword(DataBaseLineHead* pSourceData);

	// 获取财务状况
	UINT OnBankGetFinanceInfo(DataBaseLineHead* pSourceData);

	// 获取最近登录的游戏
	UINT OnLastLoginGame(DataBaseLineHead* pSourceData);

	// 查询排行榜
	UINT OnPaiHangBang(DataBaseLineHead* pSourceData);

	// 签到
	UINT OnSignInDo(DataBaseLineHead* pSourceData);
	UINT OnSignInCheck(DataBaseLineHead* pSourceData);

	// 在线奖励
	UINT OnLineAwardCheck(DataBaseLineHead* pSourceData); 
	UINT OnLineAwardDo(DataBaseLineHead* pSourceData); 
	UINT OnBuyDesk(DataBaseLineHead* pSourceData); 
	UINT OnEnterVIPDesk(DataBaseLineHead* pSourceData); 
	UINT OnVIPDeskConfig(DataBaseLineHead* pSourceData); 
	UINT OnTotalRecord(DataBaseLineHead* pSourceData);
	UINT OnSingleRecord(DataBaseLineHead* pSourceData);
	UINT OnGetCutRoom(DataBaseLineHead* pSourceData);

	UINT OnNetCutInfo(DataBaseLineHead* pSourceData); 

	UINT OnGetConfig(DataBaseLineHead* pSourceData);

	UINT OnContestNoticeLogon(DataBaseLineHead* pSourceData);

	UINT OnGetDeskConfig(DataBaseLineHead* pSourceData);

	UINT OnGetRecordURL(DataBaseLineHead* pSourceData);
	UINT OnGetDeskList(DataBaseLineHead* pSourceData);
	UINT OnGetDeskUser(DataBaseLineHead* pSourceData);

	UINT OnDissmissDesk(DataBaseLineHead* pSourceData);
	UINT OnDeleteRecord(DataBaseLineHead* pSourceData);

	UINT OnGetChangeConfig(DataBaseLineHead* pSourceData);
	UINT OnChangeMoney(DataBaseLineHead* pSourceData);

	UINT OnUpdateRoomConnect(DataBaseLineHead* pSourceData);
	UINT OnUpdateRoomConnectAll(DataBaseLineHead* pSourceData);

	UINT OnForgetPWD(DataBaseLineHead* pSourceData);

	//俱乐部
	UINT OnCreateClub(DataBaseLineHead* pSourceData);
	UINT OnDissmissClub(DataBaseLineHead* pSourceData);
	UINT OnJoinClub(DataBaseLineHead* pSourceData);
	UINT OnClubUserList(DataBaseLineHead* pSourceData);
	UINT OnClubRoomList(DataBaseLineHead* pSourceData);
	UINT OnClubBuyDesk(DataBaseLineHead* pSourceData);
	UINT OnClubChangename(DataBaseLineHead* pSourceData);
	UINT OnClubKickUser(DataBaseLineHead* pSourceData);
	UINT OnClubList(DataBaseLineHead* pSourceData);
	UINT OnClubReviewList(DataBaseLineHead* pSourceData);
	UINT OnClubMasterOPT(DataBaseLineHead* pSourceData);
	UINT OnClubChangeNotice(DataBaseLineHead* pSourceData);
	UINT OnEnterClub(DataBaseLineHead* pSourceData);
	UINT OnLeaveClub(DataBaseLineHead* pSourceData);
	UINT OnGetBuyDeskRecord(DataBaseLineHead* pSourceData);

	//邮件
	UINT OnMailList(DataBaseLineHead* pSourceData);
	UINT OnOpenMail(DataBaseLineHead* pSourceData);
	UINT OnGetAttachment(DataBaseLineHead* pSourceData);
	UINT OnMailDelete(DataBaseLineHead* pSourceData);
	UINT OnMailUpdate(DataBaseLineHead* pSourceData);
	UINT OnGetSysMsg(DataBaseLineHead* pSourceData);

    UINT OnLuckDrawConfig(DataBaseLineHead* pSourceData);
    UINT OnLuckDrawDo(DataBaseLineHead* pSourceData);
    UINT OnDissmissTimeOutDesk(DataBaseLineHead *pSourceData);
    UINT OnMobileIsMatch(DataBaseLineHead *pSourceData);

    UINT OnGetSpRoomInfo(DataBaseLineHead* pSourceData);
};


#endif