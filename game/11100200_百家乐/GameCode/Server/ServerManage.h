/// ServerManage.h: Headers and defines for CServerGameDesk
///


#if !defined(_SERVERMANAGE_H__INCLUDED_)
#define _SERVERMANAGE_H__INCLUDED_

#include "StdAfx.h"
#include "../GameMessage/UpGradeLogic.h"
#include <deque>
#include "Banker.h"
///游戏结束标志定义
#define GF_NORMAL					10			   /**<  游戏正常结束   */
#define GF_SAFE						11			   /**<  游戏安全结束   */


/*-------------------------------------------------------------------------*/
#define ID_TIMER_SEND_CARD			30				// 开始发牌
#define ID_TIMER_NO_XIA_ZHU			31				// 禁止下注
#define ID_TIMER_SEND_CARD_FINISH   32				// 发牌结束
#define ID_TIMER_SHOW_WIN_FINISH	33				// 显示结果结束
#define ID_TIMER_GAME_NEW_GAME		34				// 新一局
#define ID_TIMER_REMAINTIME_GAME	35				// 倒计时计时器
#define TIMER_UPDATE_AI_NT			37		//更新机器人上下庄信息
/*-------------------------------------------------------------------------*/





///百家乐下注区域定义定义
#define GF_XIAN_AREA			0			   /**<  闲   */
#define GF_X_TIANWANG_AREA		1			   /**<  闲天王   */
#define GF_X_DUIZI_AREA			2			   /**<  闲对子   */
#define GF_ZHUANG_AREA			3			   /**<  庄   */
#define GF_Z_TIANWANG_AREA		4			   /**<  庄天王   */
#define GF_Z_DUIZI_AREA			5			   /**<  庄对子   */
#define GF_HE_AREA				6			   /**<  和   */
#define GF_T_HE_AREA			7			   /**<  同点和   */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
///游戏服务器处理类
class CServerGameDesk :public CGameDesk
{
public:
	__int64 m_i64ChouMaValue[CHOUMANUM];
public:
	CServerGameDesk(void);
public:
	~CServerGameDesk(void);	
public:
	///游戏开始
	virtual bool	GameBegin(BYTE bBeginFlag);
	///游戏结束
	virtual bool	GameFinish(BYTE bDeskStation, BYTE bCloseFlag);
	///判断是否正在游戏
	virtual bool	IsPlayGame(BYTE bDeskStation);
	///游戏数据包处理函数
	virtual bool	HandleNotifyMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser);
	/// 框架消息处理函数
	virtual bool	HandleFrameMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser);
	///用户离开游戏桌
	virtual BYTE	UserLeftDesk(BYTE bDeskStation, CGameUserInfo * pUserInfo);
	///用户坐到游戏桌
	virtual BYTE	UserSitDesk(MSG_GR_S_UserSit * pUserSit, CGameUserInfo * pUserInfo);
	///玩家掉线
	virtual	bool	UserNetCut(BYTE bDeskStation, CGameUserInfo * pLostUserInfo);

	/// 定时器消息
	virtual bool	OnTimer(UINT uTimerID);
	//用来改变用户断线条件的函数
	//virtual bool	CanNetCut(BYTE bDeskStation);
	///用户断线离开
	//virtual bool	UserNetCut(BYTE bDeskStation, CGameUserInfo * pLostUserInfo);
public:
	/// 获取游戏状态信息
	virtual bool OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser);
	///重置游戏状态
	virtual bool ReSetGameState(BYTE bLastStation);
	///初始化游戏逻辑
	virtual bool InitDeskGameStation();

	//初始化所有数据
	void	InitAllData();
	bool	isRobot(BYTE bDeakNO);
private:
	//收到申请上下庄的消息
	void	OnHandleShangZhuang(BYTE bDeskStation,C_S_ApplyShangZhuang *pdata);
	//是否在上庄列表中
	bool	CheckInZhuangList(BYTE bDeskStation);
	///统计机器人上庄数量
	int		CountRobortNum();

	//收到玩家下注消息
	void	OnHandleXiaZhu(BYTE bDeskStation,C_S_UserNote *pUserNoTe);
	//计算各区域还可以下注多少
	bool	CountAreasMaxNote(int	iNoteArea,int iChouMaType);

	//超端设置消息
	void	OnHandleSuperSet(BYTE bDeskStation,SuperUserSetData *pSuperUserSet);
private:
	/// 加载配置文件
	BOOL	LoadIni();
	BOOL	LoadExtIni(int iRoomID);
	/// 清除所有定时器
	void	KillAllTimer(void);

private:
	///游戏开始
	bool	StartGame();
	///初始化大部分数据，有些数据不能初始化的：当前庄家进行了几局，前30局的数据，
	void	ServerInit();
	///庄家下庄后获取下一个庄家列表中的庄家
	bool	ChangeNT();	
	///五庄等待
	void	NoNtWait();
	/**从配置文件中读取机器人控制设置的参数值*/
	void	GetAIContrlSetFromIni(int iRoomID);			
	///描述：获取每个区域最大下注
	void	GetQuYuCanNote();

	//下注结束 开始发牌
	void	OnSendAllCard();
	///发牌
	void	SendCard(int iIndex=1);
	//获取一张牌
	void	GetACardValue(BYTE &byCard);
	///计算牌值
	void	CountPaiZhi(int count);
	///获取第三张牌
	bool	GetTheThird();
	///pai:闲家的第三张牌值	///闲家拿了第三张牌情况下，庄家是要拿牌
	bool	IsGetTheThird();
	///获取赢钱的区域
	void	GetWinAres();
	///@param num,检测前几张牌 同点检测
	bool	CheckTongDian(int num);
	//超端设置去换牌
	void	OnSuperSetChange();
	///描述：机器人输赢自动控制
	void	AiWinAutoCtrl();
	///描述：计算当前牌机器人的输赢钱
	int		CountAIWinMoney();

	//显示中奖区域
	void	OnSendWinAreas();

	///本局算分
	void	CountFen();
	///记录本局的成绩数据
	void	RecordLuziData();
	//清理上庄列表
	void	CleanZhuangList();
	//奖池记录
	void	RecordAiWinMoney();
	
	
private:
	// 验证超级用户权限
	void	AuthPermissions(BYTE bDeskStation); 
	//判断是否超端玩家
	bool	IsSuperUser(BYTE byDeskStation);
	
	

private:
	CUpGradeGameLogic		m_Logic;     						//游戏逻辑	
private:
	bool		m_bAIWinAndLostAutoCtrl;///机器人输赢控制：是否开启机器人输赢自动控制
	__int64		m_i64AIWantWinMoneyA1;		/**<机器人输赢控制：机器人赢钱区域1  */
	__int64		m_i64AIWantWinMoneyA2;		/**<机器人输赢控制：机器人赢钱区域2  */
	__int64		m_i64AIWantWinMoneyA3;		/**<机器人输赢控制：机器人赢钱区域3  */
	int			m_iAIWinLuckyAtA1;			/**<机器人输赢控制：机器人在区域1赢钱的概率  */
	int			m_iAIWinLuckyAtA2;			/**<机器人输赢控制：机器人在区域2赢钱的概率  */
	int			m_iAIWinLuckyAtA3;			/**<机器人输赢控制：机器人在区域3赢钱的概率  */
	int			m_iAIWinLuckyAtA4;			/**<机器人输赢控制：机器人在区域4赢钱的概率  */
	__int64		m_i64AIHaveWinMoney;		///机器人输赢控制：机器人已经赢了多少钱



private:	
	//需要配置的变量
	int				m_iXiaZhuTime;				/**< 下注时间	*/		
	int				m_iKaiPaiTime;     			/**< 开牌时间*/
	int				m_iShowResult;				//显示结算框时间
	int				m_iFreeTime;				/**< 空闲时间*/
	int				m_iRobotSZCount;            /**< 允许机器人上庄数量 */
	int				m_iMaxZhuang;				/**< 庄家一次最多做多少局，默认30局 */
	int				m_iNtTax;					///庄家抽水控制.0-只抽庄家的税 1-抽所有赢家的税收/收取所有玩家的台费
	__int64			m_i64ShangZhuangLimit; 		/**< 上庄需要的最少金币*/
	__int64			m_i64MaxNote;				///最大能下的总注
	__int64			m_i64UserMaxNote;			///每局玩家最大下注数
	__int64			m_i64Max[BET_ARES];				/**< 本把每个区域控制的最大下注额*/

	int				m_haveTimeSs;				///定时器运行几秒了
private:
	int				m_iNowNtStation;						/**< 当前庄家位置*/
	int				m_iZhuangBaShu;							/**< 庄家进行了几把*/
	__int64			m_i64NtMoney;							//庄家的金币
	__int64			m_i64NtWin;            					/**< 当前庄家赢的金币*/
	bool			m_bXiaZhuang;							/**< 当前庄家申请下庄*/	
	deque<BYTE>		m_qZhuangList;							///抢庄列表
	deque<LuziData>	m_DqLuziData;							//路子数据只保存50局

	deque<UserPlayResult> m_dqUserPlayResult;				//玩家玩游戏的详细记录  用于统计命中率	


	
private:
	__int64			m_i64PerMaxNotes;			/// 本局最大下注总注


private:	//下注信息
	__int64			m_i64ZhongZhu;   					/**< 本把当前总注额*/
	__int64			m_i64QuYuZhu[BET_ARES]; 				/**< 本把每个区域下的注额*/
	__int64			m_i64QuYuZhuTrue[BET_ARES];					///真实玩家的下注值
	__int64			m_i64UserXiaZhuData[PLAY_COUNT][BET_ARES];	
	
private:	//结算信息
	__int64			m_i64UserAreasFen[PLAY_COUNT][BET_ARES];/// 各个玩家本局的得分
	__int64			m_i64UserFen[PLAY_COUNT];				/// 各个玩家本局的得分
	__int64			m_i64UserMoney[PLAY_COUNT];				/// 各个玩家本局应的金币
	__int64			m_i64UserWin[PLAY_COUNT];				//玩家的输赢情况

private:	//一局结束后需要重置的变量
	bool			m_bCanXiaZhu;						//是否能下注
	BYTE			m_iTotalCard[52];					//总的牌
	
	bool			m_bThreeCard;						/**< 庄或闲拿了3张牌*/
	BYTE			m_byUserCard[2][3];					//庄闲的牌数据0-闲家 1-庄家
	int				m_iZPaiXing[5];    					/**< 庄家牌型,元素0前两张牌的值，元素1总牌值，元素2天王，元素3对子，元素4和*/
	int				m_iXPaiXing[5];    					/**< 闲家牌型,元素0前两张牌的值，元素1总牌值，元素2天王，元素3对子，元素4和*/
	int				m_iWinQuYu[BET_ARES];				/**< 游戏的赢钱区域 0闲，1闲天王，2闲对子，3庄，4庄天王，5庄对子，6和，7同点和*/
	__int64			m_i64AreasMaxZhu[BET_ARES];  		/**< 本把每个区域最大能下的注额*/
	
	
	
	vector	<long int>	m_vtSuperUserID;				//超端帐号
	SuperUserSetData	m_SuperSetData;					//超端设置的数据

	Banker				banker;
	//AI上下庄策略
	void NTDecision();

	__int64						m_iRobotRectNoteMax[BET_ARES];//机器人区域下注限制(0-不限制)
public:		
	//修改奖池数据1(平台->游戏)
	/// @param bAIWinAndLostAutoCtrl 机器人控制输赢开关
	/// @return 是否成功
	virtual bool SetRoomPond(bool	bAIWinAndLostAutoCtrl){return false;};
	//修改奖池数据2(平台->游戏)
	/// @param m_iAIWantWinMoney 机器人输赢控制：机器人赢钱区域1,2,3 4所用断点
	/// @param m_iAIWinLuckyAt 机器人输赢控制：机器人在区域1,2,3,4赢钱的概率
	/// @return 是否成功
	virtual bool SetRoomPondEx(__int64	iAIWantWinMoney[], int	iAIWinLuckyAt[]){return false;};
	//修改奖池数据3(平台->游戏)
	/// @param iReSetAIHaveWinMoney 机器人输赢控制：重置机器人已经赢钱的数目
	/// @return 是否成功
	virtual bool SetRoomPondEx(__int64	iReSetAIHaveWinMoney){return false;};
	//控制胜负
	virtual BOOL Judge(){return FALSE;};
	/// 胜
	virtual BOOL JudgeWiner(){return FALSE;};
	/// 输
	virtual BOOL JudgeLoser(){return FALSE;};
};
#endif 