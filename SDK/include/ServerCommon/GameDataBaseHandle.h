/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#include "AFCDataBase.h"
#include "GameRoomLogonDT.h"


///游戏数据库处理
class AFX_EXT_CLASS CGameDataBaseHandle : public CDataBaseHandle
{
public:
	CGameDataBaseHandle();
	///辅助函数
private:
	///数据库处理接口
	virtual UINT HandleDataBase(DataBaseLineHead * pSourceData);
	///处理函数
private:
	///大厅登陆处理
	UINT OnRoomLogon(DataBaseLineHead * pSourceData);
	///用户离开处理
	UINT OnUserLeft(DataBaseLineHead * pSourceData);
	///更新服务器信息
	UINT OnUpdateServerInfo(DataBaseLineHead * pSourceData);
	///记录游戏信息
	UINT OnRecordGameInfo(DataBaseLineHead * pSourceData);
	///检测数据库连接
	UINT OnCheckDataConnect(DataBaseLineHead * pSourceData);
	///检测是否服务到期
	UINT OnTimeServiceControl(DataBaseLineHead * pSourceData);
	///呼叫GM处理
	UINT OnCallGM(DataBaseLineHead * pSourceData);
	///管理权限修改
	UINT OnGamePower(DataBaseLineHead * pSourceData);
	///管理员踢人动作纪录
	UINT OnGMKickUserRecord(DataBaseLineHead * pSourceData);
	///管理员法警告消息纪录
	UINT OnGMWarningUserRecord(DataBaseLineHead * pSourceData);

	//更新转账返利数据
	UINT OnUpdateRebate(DataBaseLineHead * pSourceData);
	//更新玩家返利数据
	UINT OnUpdateUserRebate(DataBaseLineHead * pSourceData, UINT uCode = DTK_GR_BANK_UPDATE_USER_REBATE);
	//提取或者存储返利
	UINT OnBankTransferRebate(DataBaseLineHead* pSourceData, UINT uCode = DTK_GR_BANK_SET_USER_REBATE);

	///打开钱柜
	UINT OnOpenWallet(DataBaseLineHead *pSourceData, UINT uCode = DTK_GR_DEMAND_MONEY);
	///取出金币
	UINT OnCheckOutMoney(DataBaseLineHead *pSourceData, UINT uCode= DTK_GR_CHECKOUT_MONEY);
	///存入金币
	UINT OnCheckInMoney(DataBaseLineHead *pSourceData, UINT uCode= DTK_GR_CHECKIN_MONEY);
	///转帐
	UINT OnTransferMoney(DataBaseLineHead *pSourceData, UINT uCode= DTK_GR_TRANSFER_MONEY);
	///修改密码
	UINT OnChangePasswd(DataBaseLineHead *pSourceData, UINT uCode= DTK_GR_CHANGE_PASSWD);
	///转帐记录
	UINT OnTransferRecord(DataBaseLineHead *pSourceData, UINT uCode= DTK_GR_TRANSFER_RECORD);
	///更新用户信息
	UINT OnUpdateUserResult(DataBaseLineHead * pSourceData);
	///一次性更新同桌所有用户信息
	UINT OnUpdateAllUserResult(DataBaseLineHead * pSourceData);
	//比赛开始，获取比赛ID，并修改报名表中用户状态
	UINT OnContestBegin(DataBaseLineHead * pSourceData);
	//比赛结束，获取奖励
	UINT OnContestGameOver(DataBaseLineHead * pSourceData);
	///更新用户比赛信息
	UINT OnUpdateContestInfo(DataBaseLineHead * pSourceData);
	//获取赠送的金币值 //2013-01-28 wangzhitu
	void GetWalletMoney(RECEIVEMONEY &strRecMoney,DWORD dwUserID);

	///得到用户比赛信息
	UINT OnGetContestInfo(DataBaseLineHead * pSourceData);
	//玩家报名比赛
	UINT OnContestApply(DataBaseLineHead *pSourceData);
	///即时更新分数金币
	UINT OnModifyUserMoney(DataBaseLineHead * pSourceData);
	///即时修改玩家魅力值
	UINT OnUpdateUserCharm(DataBaseLineHead * pSourceData);
	///使用道俱
	UINT OnUserUseOneProp(DataBaseLineHead *pSourceData);
	UINT OnUseSmallBoardcast(DataBaseLineHead * pSourceData);
	///通过按照次数和金额给用户送金币
	UINT OnSendUserMoenyByTimes(DataBaseLineHead *pSourceData);
	///删除登陆信息
	bool UnRegisterUserLogon(long int dwUserID, bool bAllLogonInfo);
	///清理用户断线
	UINT OnUnRegisterOnLine(DataBaseLineHead * pSourceData);
	///

	///wushuqun 2009.5.22
	///修改禁止登录问题，修改插入TWLoginRecord 记录时机
	///执行时机：登录房间后发送完游戏房间列表执行
	UINT OnWriteTWLoginRecord(DataBaseLineHead * pSourceData);
	///wushuqun 2009.6.6
	///即时封桌功能
	UINT OnAloneDeskInTime(DataBaseLineHead * pSourceData);

	///辅助函数

	UINT OnClearNetCutUserOnlineFlag(DataBaseLineHead * pSourceData);	///2009-4-17添加 zxj

	UINT OnReWriteWLoginRecord(DataBaseLineHead * pSourceData);		///2009-4-28添加 zxj

	UINT OnContestUserLeft(DataBaseLineHead * pSourceData);		///2009-4-28添加 zxj

	UINT OnContestReturnFee(DataBaseLineHead * pSourceData);//退回报名费

	/// 记录修改房间密码
	UINT OnRoomPWChangeRecord(DataBaseLineHead * pSourceData);

	// PengJiLin, 2010-8-23, 根据用户ID获取昵称
	UINT OnGetNickNameOnID(DataBaseLineHead * pSourceData);

	// PengJiLin, 2010-9-10, 使用踢人卡、防踢卡道具
	UINT OnUseKickProp(DataBaseLineHead * pSourceData, BOOL bIsKickUser);

	// PengJiLin, 2010-9-13, 使用踢人卡踢人功能
	UINT OnNewKickUserProp(DataBaseLineHead * pSourceData);


	UINT OnBankOperateNormal(DataBaseLineHead * pSourceData);

	UINT OnBankOperateTransfer(DataBaseLineHead * pSourceData);

	UINT OnBankOperateDeposit(DataBaseLineHead * pSourceData);

	UINT OnBankGetWallet(DataBaseLineHead * pSourceData);

	UINT OnBankQueryDeposits(DataBaseLineHead * pSourceData);

	UINT OnPropChange(DataBaseLineHead * pSourceData);
	UINT FillTimingMatchQueueUser(DataBaseLineHead *pSourceData);
	int OnFindBuyDeskStation(DataBaseLineHead *pSourceData);
	int OnClearBuyDesk(DataBaseLineHead *pSourceData);
	int OnReleaseBuyDesk(DataBaseLineHead *pSourceData);
	int OnReturnBuyDesk(DataBaseLineHead *pSourceData);
	int OnGetDeskInfo(DataBaseLineHead *pSourceData);

	int OnRecordGame(DataBaseLineHead *pSourceData);
	int OnCreateGameSN(DataBaseLineHead *pSourceData);

	int OnReleaseRecord(DataBaseLineHead *pSourceData);
	int OnGetReleaseRecord(DataBaseLineHead *pSourceData);

	int OnCutNet(DataBaseLineHead *pSourceData);
	int OnCutNetRecordStart(DataBaseLineHead *pSourceData);

	int OnContestRegist(DataBaseLineHead *pSourceData);
	int OnContestActive(DataBaseLineHead *pSourceData);

	//比赛取消
	UINT OnContestAbandon(DataBaseLineHead *pSourceData);

	int OnCostJewels(DataBaseLineHead *pSourceData);
	int OnClearData(DataBaseLineHead *pSourceData);
	int	OnMiddleCostFee(DataBaseLineHead *pSourceData);

	int OnGetRoomInfo(DataBaseLineHead *pSourceData);
	int OnSetContestInfo(DataBaseLineHead *pSourceData);

	int OnUpdateNetCutTime(DataBaseLineHead *pSourceData);

	int OnMasterLeave(DataBaseLineHead *pSourceData);

	int OnContestExit(DataBaseLineHead *pSourceData);

	int OnRobotRandIPList(DataBaseLineHead *pSourceData);

	int OnChangeUserStation(DataBaseLineHead *pSourceData);
    int OnSetGameDynamicInfo(DataBaseLineHead *pSourceData);
    int OnUpdateBeginTime(DataBaseLineHead *pSourceData);
    int OnDissmissDeskByMS(DataBaseLineHead *pSourceData);
	int                 m_ErrorSQLCount;
    int     OnSetSPRoomInfo(DataBaseLineHead *pSourceData);

	static std::vector<ULONG> m_vecRandIPList;
	

};