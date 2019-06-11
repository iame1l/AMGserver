// ClientGameDlg.h: Headers and defines for CClientGameDlg
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CLIENTGAMEDLG_H__INCLUDED_)
#define _CLIENTGAMEDLG_H__INCLUDED_

#include "StdAfx.h"
#include "upgrademessage.h"
#include "UpGradeLogic.h"
//重置函数参数
#define RS_ALL_VIEW					1					//重置所有参数
#define RS_GAME_CUT					2					//用户离开重置
#define RS_GAME_END					3					//游戏结束重置
#define RS_GAME_BEGIN				4					//游戏开始重置

// 定时器 ID
#define TIME_SHANG_ZHUANG			100			// 上庄计时器
#define TIME_XIA_ZHU				101			// 下注计时器




class CClientGameDlg :public CLoveSendClass
{
public:
	CClientGameDlg(void);
public:
	virtual ~CClientGameDlg(void);	
protected:
	DECLARE_MESSAGE_MAP()
public:
	int					m_iXiaZhuTime;			///下注时间	
	int					m_iKaiPaiTime;			///开牌时间  
	int					m_iShowResult;			///显示结算框时间
	int					m_iFreeTime;			///空闲时间	
	int                 m_iRemainTimeXZ;        //剩余下注时间
	__int64		        m_i64UserMoney;	        //玩家自身的金币数量

	__int64				m_i64ZhongZhu;				/// 本把当前总注额
	__int64				m_i64AreasMaxZhu[BET_ARES]; //每个区域能下的最大注	
	__int64				m_i64MyZhu[BET_ARES];		///本把玩家自己在每个区域下的注额
	__int64				m_i64QuYuZhu[BET_ARES];		///本把每个区域下的注额：

private:
	int					m_iNoteTimeMin;		////最小下注频率 单位毫秒
	int					m_iNoteTimeMax;		////最大下注频率 单位毫秒
	int					m_iChouMaPercent[7];//下注筹码的概率
	int					m_iAreasPercent[BET_ARES];	//3个下注区域的概率
	float               m_Clock;          //
public:
	// 清除所有定时器
	void KillAllTimer(void);
	// 上下庄申请按钮函数
	void	OnShangZhuang();
	//检测自已是否在庄上
	bool	CheckMeInZhuang(void);	
	//机器人下注
	void	OnXiaZhu();
	//获取下注的区域
	int		GetXiaZhuAreas(int iAreasRandNum);
	//获取下注筹码类型
	int		GetXiaZhuTpye(int iRandNum);
	//根据区域概率随机区域位置
	int     rand_pos(int *perent,int size);
protected:
	// 通知更新游戏界面
	virtual bool OnNotifyUpdateClientView(void * pNetData, UINT uDataSize);
	// 玩家强退结束游戏
	virtual bool OnNotifyCutEnd(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize);

	// 托管消息响应函数
	virtual void OnChangeTrustee(BYTE byDeskStation, bool bIsTrustee);
// 重载
public:
	//初始化函数
	virtual BOOL OnInitDialog();
	//设置游戏状态
	virtual bool SetGameStation(void * pStationData, UINT uDataSize);
	//游戏消息处理函数
	virtual bool HandleGameMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//重新设置游戏数据
	virtual void ResetGameStation(int iGameStation){};
	//定时器消息
	virtual bool OnGameTimer(BYTE byDeskStation, UINT uTimeID, UINT uTimeCount);
	///重载下注定时器，使改定时器可以用毫秒计时
	virtual bool SetGameTimer(BYTE bDeskStation,UINT uTimeCount,UINT uTimeID,bool bUseMillSecond = false) ;
	// 同意开始游戏 
	virtual bool OnControlHitBegin(){return true;};
	// 安全结束游戏
	virtual bool OnControlSafeFinish(){return false;};

	


public:
	void OnTimer(UINT_PTR nIDEvent);
	//加载配置文件
	void LoadIniConfig();
};
#endif // !defined(_CLIENTGAMEDLG_H__INCLUDED_)