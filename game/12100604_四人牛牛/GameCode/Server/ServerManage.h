#pragma once

#include "StdAfx.h"
#include"..\GameMessage\UpgradeLogic.h"
#include<vector>
//定时器 ID
#define TIME_CALL_NT				30				//叫庄
#define TIME_NOTE                   31              //下注计时器
#define TIME_NOTE_FINISH			32				//下注完成定时器
#define TIME_SEND_CARD_FINISH		33				//发牌定时器
#define TIME_AUTO_BULL				34				//自动摆牛计时器
#define TIME_GAME_FINISH			35				//游戏结束定时器
#define TIME_CALL_ROBNT				36				//抢庄
#define TIME_DOUBLE                 37              //加倍计时器
#define TIME_DOUBLE_FINISH			38				//下注完成定时器
#define TIME_GAME_BEGIN				39				//游戏开始
#define TIME_CHECK_READY			40				//检测是否准备


//游戏结束标志定义
#define GF_NORMAL					10				//游戏正常结束
#define GF_SALE						11				//游戏安全结束

#define NUM_ONE_SECOND_MS			1000//1s=1000ms



//游戏桌类
class CServerGameDesk : public CGameDesk
{
protected:
	/*---------------------------游戏基础数据----------------------------------*/
	CUpGradeGameLogic		m_Logic;						//游戏逻辑
	
	BYTE					m_iAllCardCount;		//游戏所用的牌数
	BYTE					m_iTotalCard[52];		//总的牌

	/*---------------------------游戏基础数据每局需要重置的数据----------------*/
	SuperUserSetData		m_SuperSetData;					//超端设置的数据
	bool                    m_bUserReady[PLAY_COUNT] ;			///玩家准备状态
	BYTE					m_byUpGradePeople;					//庄家位置	
	BYTE                    m_iFirstJiao;						//第一个抢庄玩家位置	
	BYTE					m_byCurrOperateUser;				//当前操作的玩家	

	__int64					m_iPerJuTotalNote[PLAY_COUNT];		//用户每局压总注 dwj
	__int64					m_iLimitNote[PLAY_COUNT];			//上限
	__int64					m_i64UserNoteLimite[PLAY_COUNT][4];	//玩家四个下注数字
	__int64					m_iLimit;

	BYTE					m_byUserStation[PLAY_COUNT];			//记录用户状态	DWJ
	BYTE					m_iUserCardCount[PLAY_COUNT];			//用户手上扑克数目
	BYTE					m_iUserCard[PLAY_COUNT][SH_USER_CARD];	//用户手上的扑克
	int						m_iCallScore[PLAY_COUNT];				//玩家抢庄状态 -1表示还未操作，0表示不抢庄 1表示抢庄，开房间模式下1-4表示抢庄倍数	DWJ
	int						m_iNTBase;								//抢庄倍数
	int						m_iUserBase[PLAY_COUNT];				//闲家倍数, 0表示未加倍，1-3表示加倍数

	int						m_iOpenShape[PLAY_COUNT];
	BYTE					m_byOpenUnderCount[PLAY_COUNT];		//底牌张数
	BYTE					m_byOpenUnderCard[PLAY_COUNT][3];	//底牌的三张牌
	BYTE					m_byOpenUpCard[PLAY_COUNT][2];		//升起来的2张牌
	BYTE                    m_bBullCard[PLAY_COUNT][3];          //玩家摆的牛牌 

	//Added by QiWang 20171106, 中途进入或断线重连需要知道剩余时间
	time_t				tRobNTTime;
	time_t				tDoubleTime;
	time_t				tShowCardTime;

	/*---------------------------游戏可配置参数----------------------------------*/
	BYTE				m_iBeginTime;			//游戏开始时间
	BYTE				m_iThinkTime;			//游戏摆牛思考时间
	BYTE				m_iCallScoreTime;		//叫分时间
	BYTE				m_iXiaZhuTime;			//下注时间
	BYTE				m_iRobNTTime;		    //抢庄时间(开房间模式)
	BYTE				m_iDoubleTime;			//加倍时间
	BYTE				m_bCardShapeBase[MAX_SHAPE_COUNT];//0-10代表无牛到牛牛间的倍率
	int					m_iBasePoint;			//游戏基础分, Added by QiWang 20180226
	//超端参数
	vector<long>		m_vlSuperID;				//保存超端玩家的容器
	//奖池控制参数
	bool				m_bAIWinAndLostAutoCtrl;	//机器人控制输赢
	__int64				m_iAIWantWinMoneyA1;		/**<机器人输赢控制：机器人赢钱区域1  */
	__int64				m_iAIWantWinMoneyA2;		/**<机器人输赢控制：机器人赢钱区域2  */
	__int64				m_iAIWantWinMoneyA3;		/**<机器人输赢控制：机器人赢钱区域3  */
	int					m_iAIWinLuckyAtA1;			/**<机器人输赢控制：机器人在区域1赢钱的概率  */
	int					m_iAIWinLuckyAtA2;			/**<机器人输赢控制：机器人在区域2赢钱的概率  */
	int					m_iAIWinLuckyAtA3;			/**<机器人输赢控制：机器人在区域3赢钱的概率  */
	int					m_iAIWinLuckyAtA4;			/**<机器人输赢控制：机器人在区域4赢钱的概率  */

	//一下参数都是可配置的 但是没有写在配置文件中 就是防止客户乱配出现问题
	int					m_iSendCardTime;		//发牌时间-控制客户端发一张牌的速度
	BOOL				m_bTurnRule;			//游戏顺序 0-顺时针 1-逆时针
	int					m_iGoodCard;			//发大牌机率牛七以上
	bool				m_bHaveKing;			//是否有王
	DWORD				m_iCardShape;			//牌型
	
	CalculateBoard			m_TCalculateBoard[PLAY_COUNT];
	//vector<bool>			m_vecWinInfo;
	CArray<BYTE ,bool >		m_vecWinInfo[PLAY_COUNT];

	//Added by QiWang 20171106, 普通场准备时间到未准备T人相关
	bool m_bKickPerson;//可以T人与否标记
	bool m_bTimerCheckSet; //Timer Check Ready is Set or not
	BYTE m_byNoReadyTimeArr[PLAY_COUNT];//客户端未准备时间数组

	//函数定义
public:
	//构造函数
	CServerGameDesk(); 
	//析构函数
	virtual ~CServerGameDesk();

	//重载函数
public:
	//初始化游戏逻辑
	virtual bool InitDeskGameStation();
	//游戏开始
	virtual bool	GameBegin(BYTE bBeginFlag);
	//游戏结束
	virtual bool	GameFinish(BYTE bDeskStation, BYTE bCloseFlag);
	//判断是否正在游戏
	virtual bool IsPlayGame(BYTE bDeskStation);
	//判断需要发送大结算与否
	virtual bool IsNeedSendBoard();
	//框架消息处理函数
	virtual bool HandleFrameMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser);
	//游戏数据包处理函数
	virtual bool HandleNotifyMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser);
	//用户离开游戏桌
	virtual BYTE UserLeftDesk(BYTE bDeskStation, CGameUserInfo * pUserInfo);
	//
	virtual bool UserNetCut(BYTE bDeskStation, CGameUserInfo *pLostUserInfo);
	//用户断线重来
	virtual bool UserReCome(BYTE bDeskStation, CGameUserInfo * pNewUserInfo);
	//中途大结算
	virtual void DissMissRoomBefore();

	//释放房间资源时绩录玩家信息
	//通知游戏保存玩家游戏数据
	virtual void ReleaseRoomBefore();
	//恢复房间发送游戏数据到游戏服务器
	//iCount数据总数量
	virtual void SetReturnGameInfo(int UserID,void* szUserGameInfo,int iSize,int iCount);

	//重载纯虚函数
public:
	//获取游戏状态信息
	virtual bool OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser);
	//重置游戏状态
	virtual bool ReSetGameState(BYTE bLastStation);
	//记录机器人输赢值
	void RecordAiHaveWinMoney(GameEndStruct *GameEnd);
	//定时器消息
	virtual bool OnTimer(UINT uTimerID);
	//处理函数
public:
	//读取静态变量
	BOOL	LoadIni();	
	//根据房间ID加载底注和上限
	BOOL	LoadExtIni(int iRoomID);
	/**从配置文件中读取机器人控制设置的参数值*/
	void	GetAIContrlSetFromIni(int iRoomID);			
	//超级用户验证
	void	SuperUserExamine(BYTE bDeskStation);
	//是否超端用户 超端用户要发送结果给他看到
	bool	IsSuperUser(BYTE byDeskStation);
	//统计游戏人数
	BYTE	CountPlayer();
	//统计当前还未下注的玩家个数
	BYTE	CountNoNotePeople();
	//房间倍数
	int		GetRoomBasePoint();
	//计算各家分数
	int		ComputePoint(BYTE DeskStation);
	//得到最大牌和最小牌,win值为1时得到最大,其它为最小
	BYTE	GetIndex(int win);


	//换牌
	BOOL	ChangeCard(BYTE bDestStation,BYTE bWinStation);
	//交换指定两个玩家的手牌
	void	ChangeTwoUserCard(BYTE byFirstDesk,BYTE bySecondDesk);
	
	//获取下一个玩家位置
	BYTE	GetNextDeskStation(BYTE bDeskStation);
	//发送给第一个叫庄
	BOOL	SendCallScore(BYTE bDeskStation);
	//发送给第一个抢庄
	BOOL	SendRobNT(BYTE bDeskStation);
	//叫庄
	BOOL	UserCallScore(BYTE bDeskStation, int iVal);
	//抢庄
	BOOL	UserRobNT(BYTE bDeskStation, int iVal);
	//是否抢庄结束
	BOOL IsRobNTFinish();
	//叫分结束
	BOOL	CallScoreFinish();
	//叫分结束
	BOOL	RobNTFinish();
	//通知用户下注
	BOOL	NoticeUserNote();
	//通知用户加倍
	BOOL	NoticeUserDouble();
	//处理用户下注结果
	BOOL	UserNoteResult(BYTE bDeskStation, BYTE iVerbType,int iNoteType = 0);
	//处理用户加倍结果
	BOOL	UserDoubleResult(BYTE bDeskStation, BYTE iVerbType,int iNoteType = 0);
	//发送扑克给用户
	BOOL	SendCard();
	//发牌结束
	BOOL	SendCardFinish();
	//开牌
	BOOL	BeginOpenCard();
	//玩家摆牛
	//bClient: 客户端true(检测客户端数据合法性及牛的正确性),服务端false
	BOOL	UserOpenCard(BYTE byDeskStation,void * pData, bool bClient);
	//检测是否结束
	BOOL	CheckFinish();
	//算出玩家投注最大上限
	__int64 GetLimit(int bDeskStation);
	
	///清理所有计时器
	void	KillAllTimer();;

	///获取当前牌中最大的牌型，
	int		GetMaxCardShape(BYTE iCard[], int iCardCount);

	///20121122dwj机器人输赢自动控制,
	void	IAWinAutoCtrl();
	///20121122dwj计算当前牌机器人的输赢钱
	int		CountAIWinMoney();

	//Added by QiWang 20171106, 普通场T人相关
	void SetTimerCheckReadyInFinish();

private:
	//Added by QiWang 20180302, 获得牛底牌
	void GetBullCard(BYTE byDeskStation);


private:
	//修改奖池数据1(平台->游戏)
	/// @param bAIWinAndLostAutoCtrl 机器人控制输赢开关
	/// @return 是否成功
	virtual bool SetRoomPond(bool	bAIWinAndLostAutoCtrl);

	//修改奖池数据2(平台->游戏)
	/// @param m_iAIWantWinMoney 机器人输赢控制：机器人赢钱区域1,2,3 4所用断点
	/// @param m_iAIWinLuckyAt 机器人输赢控制：机器人在区域1,2,3,4赢钱的概率
	/// @return 是否成功
	virtual bool SetRoomPondEx(__int64	iAIWantWinMoney[], int	iAIWinLuckyAt[]);

	//修改奖池数据3(平台->游戏)
	/// @param iReSetAIHaveWinMoney 机器人输赢控制：重置机器人已经赢钱的数目
	/// @return 是否成功
	virtual bool SetRoomPondEx(__int64	iReSetAIHaveWinMoney);

	//控制胜负
	/// 判断
	virtual BOOL Judge();
	/// 胜
	virtual BOOL JudgeWiner();
	/// 输
	virtual BOOL JudgeLoser();
	//GRM函数
	//更新奖池数据(游戏->平台)
	/// @return 机器人赢钱数
	virtual void UpDataRoomPond(__int64 iAIHaveWinMoney);

	void UpdateCalculateBoard();

};

/******************************************************************************************************/
