#pragma once

#include "../../../../sdk/include/ServerCommon/GameDesk.h"
#include "../GameMessage/UpGradeLogic.h"
#include <vector>

/*---------------------------------------------------------------------*/
#define TIME_SEND_CARD				30				//发牌定时器
#define TIME_BEGIN_PLAY				31				//发牌完成 开始下注
#define TIME_WAIT_ACTION			32				//等待操作
#define TIME_NOTICE_ACTION			33				//通知操作
#define TIME_GAME_FINISH			34				//游戏结束定时器
#define TIME_READY_                 35              //玩家准备计时器 35，36，37，38，39，40
/*---------------------------------------------------------------------*/

//游戏结束标志定义
#define GF_NORMAL					10				//游戏正常结束
#define GF_SALE						11				//游戏安全结束


static __int64					G_i64AIHaveWinMoney;	//机器人赢钱数	 这里声明为静态全局变量，因为多个桌子需要及时修改该变量
static __int64					G_i64ReSetAIHaveWinMoney;	/**<机器人输赢控制：重置机器人已经赢钱的数目  该变量也要声明为静态全局变量 否则每个桌子都会去重置*/


//游戏桌类
class CServerGameDesk : public CGameDesk
{
private:
	CUpGradeGameLogic		m_Logic;     						//游戏逻辑

	TGameBaseData			m_TGameData;				//游戏数据

private:
	//奖池控制参数
	bool					m_bAIWinAndLostAutoCtrl;	//机器人控制输赢
	__int64					m_iAIWantWinMoneyA1;		/**<机器人输赢控制：机器人赢钱区域1  */
	__int64					m_iAIWantWinMoneyA2;		/**<机器人输赢控制：机器人赢钱区域2  */
	__int64					m_iAIWantWinMoneyA3;		/**<机器人输赢控制：机器人赢钱区域3  */
	int						m_iAIWinLuckyAtA1;			/**<机器人输赢控制：机器人在区域1赢钱的概率  */
	int						m_iAIWinLuckyAtA2;			/**<机器人输赢控制：机器人在区域2赢钱的概率  */
	int						m_iAIWinLuckyAtA3;			/**<机器人输赢控制：机器人在区域3赢钱的概率  */
	int						m_iAIWinLuckyAtA4;			/**<机器人输赢控制：机器人在区域4赢钱的概率  */


private:	//每局游戏结束/开始需要重置的数据
	BYTE			m_iTotalCard[52];					//总的牌
	int				m_iThisGuoDi;				//本局锅底值	
	int				m_iThisDiZhu;				//本局底注 
	__int64			m_i64ThisDingZhu;			//本局个人顶注 -总下注上限	达到上限 就要开牌	
	//__int64			m_i64ThisShangXian;			//本局个人下注上限 (即 最大每轮最大下注数不能超过这个值) 暗注的，但显示为明注的

	CArray<BYTE ,bool >		m_vecWinInfo[PLAY_COUNT];
	CalculateBoard			m_TCalculateBoard[PLAY_COUNT];

public:
	int				m_OperateCount[PLAY_COUNT];			//用户本局操作的次数
	bool			m_bOpenRecord[PLAY_COUNT][PLAY_COUNT];


	//断线重连需要用到的时间数据
	int     m_iLastTimerTime ;
	int		m_dwLastGameStartTime ;
  
private:	//配置文件配置参数
	vector	<long>		m_vclSuperUserID;	 		//超端玩家容器

public:
	//构造函数
	CServerGameDesk(); 
	//析构函数
	virtual ~CServerGameDesk();

	//重载函数
public:
	//游戏开始
	virtual bool	GameBegin(BYTE bBeginFlag);
	//游戏结束
	virtual bool	GameFinish(BYTE bDeskStation, BYTE bCloseFlag);
	//判断是否正在游戏
	virtual bool	IsPlayGame(BYTE bDeskStation);
	//游戏数据包处理函数
	virtual bool	HandleNotifyMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser);
	//框架消息处理函数
	virtual bool HandleFrameMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser);
	//用户离开游戏桌
	virtual BYTE	UserLeftDesk(BYTE bDeskStation, CGameUserInfo * pUserInfo);
	//用户断线
	virtual bool	UserNetCut(BYTE bDeskStation, CGameUserInfo * pUserInfo);
	virtual bool	InitDeskGameStation();
	//重载纯虚函数
public:
	//获取游戏状态信息
	virtual bool	OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser);
	//重置游戏状态
	virtual bool	ReSetGameState(BYTE bLastStation);
	//定时器消息
	virtual bool	OnTimer(UINT uTimerID);

private:
	//响应超端设定
	void	OnHandleSuperSet(BYTE bDeskStation,C_S_SuperUserSet *pSuperUserSet);
	//收到客户端发来的玩家看牌消息
	void	OnHandleUserLookCard(BYTE byDeskStation,void * pData);
	//收到客户端发来的玩家跟注消息
	void	OnHandleUserFollow(BYTE byDeskStation,void * pData);
	//收到客户端发来的玩家加注消息
	void	OnHandleUserAddNote(BYTE byDeskStation,void * pData);
	//收到客户端发来的玩家弃牌消息
	void	OnHandleUserGiveUp(BYTE byDeskStation,void * pData);
	//收到客户端发来的玩家比牌消息
	void	OnHandleUserOpenCard(BYTE byDeskStation,void * pData);
public:
	void	ReSetGameData();
	//读取静态变量
	void	LoadIni();	
	//根据房间ID加载底注和上限
	BOOL	LoadExtIni(int iRoomID);
	//发送扑克给用户
	BOOL	SendCard();
	//游戏开始
	BOOL	BeginPlayUpGrade();
	//通知玩家下注
	void	NoticeUserAction();

	///机器人输赢自动控制,
	void	AiWinAutoCtrl();
	//记录机器人输赢值
	void	RecordAiHaveWinMoney(__int64 i64UserScore[]);

public:
	//自动弃牌
	void	AutoGiveUp();
	//检测是否结束
	bool	CheckFinish();  
	//获取最终胜利的玩家
	BYTE	GetFinalWiner();
	//统计游戏人数
	BYTE	CountPlayer();
	//逃跑扣分
	int		GetRunPublish();
	//得到下一个出牌玩家的位置
	BYTE	GetNextNoteStation(BYTE bCurDeskStation);
	//返回最小金币玩家的金币
	__int64		GetMinMoney();
	//是否需要封顶
	bool	bNeedFengDing();
	//获取剩余时间
	int GetGameRemainTime();
	//能否下为主
	bool	CanXiZhu();
	//获取玩家当前的金币
	__int64	GetUserMoney();

private:

	//验证是否超端
	void	SpuerProof(BYTE byDeskStation);
	//检查是否超端玩家
	bool	IsSuperUser(BYTE byDeskStation);
	//交换两个人的牌
	void	Change2UserCard(BYTE byFirstDesk,BYTE bySecondDesk);


public:
	//修改奖池数据1(平台->游戏)
	virtual bool SetRoomPond(bool	bAIWinAndLostAutoCtrl){return false;};

	//修改奖池数据2(平台->游戏)
	virtual bool SetRoomPondEx(__int64	iAIWantWinMoney[], int	iAIWinLuckyAt[]){return false;};

	//修改奖池数据3(平台->游戏)
	virtual bool SetRoomPondEx(__int64	iReSetAIHaveWinMoney){return false;};
	//控制胜负
	/// 判断
	virtual BOOL Judge(){return false;};
	/// 胜
	virtual BOOL JudgeWiner(){return FALSE;};
	/// 输
	virtual BOOL JudgeLoser(){return FALSE;};
	//GRM函数
	//更新奖池数据(游戏->平台)
	/// @return 机器人赢钱数
	virtual void UpDataRoomPond(__int64 iAIHaveWinMoney){};

	void	UpdateCalculateBoard(bool dismiss);

	int PlayerCount() const;

	int DismissRoom();
};

/******************************************************************************************************/
