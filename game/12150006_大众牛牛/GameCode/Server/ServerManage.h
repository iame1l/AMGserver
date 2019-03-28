#pragma once

#include "StdAfx.h"
#include"..\GameMessage\UpgradeLogic.h"
#include "..\GameMessage\UpgradeMessage.h"
#include<vector>
#include <time.h>
//定时器 ID
#define TIME_NOTE                   31              //下注计时器
#define TIME_NOTE_FINISH			32				//下注完成定时器
#define TIME_SEND_CARD_FINISH		33				//发牌结束定时器
#define TIME_AUTO_BULL				34				//自动摆牛计时器
#define TIME_GAME_FINISH			35				//游戏结束定时器
#define TIME_CALL_ROBNT				36				//抢庄定时器
#define TIME_ROBNT_FINISH		    37				//抢庄结束定时器
#define TIME_GAME_BEGIN				38				//游戏开始定时器
#define TIME_AUTO_NEXT				39				//自动开始下一局定时器
#define TIME_AUTO_BEGIN				40				//游戏自动开始定时器
#define TIME_OPEN_DELAY				41				//庄家牌显示音效时间

#define TIME_AUTO_OPERATOR			48				//托管自动操作
#define TIMER_ID_KICK				49				//剔除玩家定时器

//游戏结束标志定义
#define GF_NORMAL					10				//游戏正常结束


#define MODE_SET_CARD_DEBUG				TRUE//设置牌模式|配牌器开关


//mark
struct TAIControl
{
	__int64 iMoney;
	int		iLucky;
	int		iMinBull;
	int		iMaxBull;
};


static __int64					G_iAIHaveWinMoney;	//机器人赢钱数20121126	dwj 这里声明为静态全局变量，因为多个桌子需要及时修改该变量
static __int64					G_iReSetAIHaveWinMoney;	/**<机器人输赢控制：重置机器人已经赢钱的数目  该变量也要声明为静态全局变量 否则每个桌子都会去重置*/


typedef BYTE PlayerStation;

//游戏桌类
class CServerGameDesk : public CGameDesk
{
private:
	
	bool					m_bAICtrl;			//是否开启机器人控制
	vector<TAIControl>		m_vAIContrl;		//机器人控制细节

	bool					m_bAIRobAndNote;	//是否机器人智能抢庄下注
	int						m_iAIRobNTShape;	//机器人抢庄牌型
	int						m_iAINoteShape;		//机器人下注牌型

private:
	int		m_iSitTime[PLAY_COUNT];				//玩家坐下时间
	int		m_iTickTime;						//踢人时间
	bool	m_bAuto[PLAY_COUNT];				//玩家是否托管
	bool	m_bNormalFirst;						//是否第一把
	bool	m_bHaveBeginButton;					//是否有开始按钮
	int		m_iBasePoint;						//游戏底分
	int		m_RoomId;							//房间id
public:


	void PrepareCard();

	//通知Ai牌形
	void NotifyAiCardShape();

	void GameBeginInit();

	void SendGameBegin();

	void MakeGoodCard();

	//获取更真实的牌型
	int GetRealShape(int iMinBull, int iMaxBull);

	//是否是有效的牛牛类型
	bool IsValidBull(int iShape);

	//是否需要机器人控制
	bool NeedAIControl();

	//游戏返回游戏数据，并保存至数据库
	//szUserGameInfo 玩家数据
	//iSize 数据长度
	virtual void ReleaseRoomBefore() ;
	//恢复房间发送游戏数据到游戏服务器
	//iCount数据总数量
	virtual void SetReturnGameInfo(int UserID,void* szUserGameInfo,int iSize,int iCount);
	// 玩家坐下
	virtual BYTE UserSitDesk( MSG_GR_S_UserSit * pUserSit, CGameUserInfo * pUserInfo );
public:
	//自动抢庄
	void AutoRobNT(BYTE byUser);

	//自动下注
	void AutoNote(BYTE byUser);

	//自动摊牌
	bool AutoTanPai(BYTE byUser);

	//自动处理
	void AutoAction(BYTE byUser);

	//玩家托管请求
	void HandleAuto(BYTE byUser, bool bAuto);

	//读取默认玩法
	void LoadPlayMode();

	//最后初始化数据
	void InitLastData();
	//记录上一局信息
	void RecoderLastGame(__int64 iChangeMoney[], const int iSize );
	//查找牌型
	bool FindShape(BYTE bUser, BYTE byRandCards[], int iShape);
	//设置超端配牌
	void SetSuPerUserCards();
	//获取最大推注
	int GetMaxTuiZhu( );
	//设置能够下注的分数
	void SetCanNote( vector<PlayerStation> &vRandLoser = vector<PlayerStation>( ) );
	//获取底分
	int GetBasePoint( );
	//获取最大抢庄倍数
	int GetMaxRobNT( );
	//是否有某种玩法
	bool IsPlayMode( int iMode );
	//是否是有效玩家
	bool IsValidPlayer( int iNum );
	//获取极限下注值
	int GetLimitNote( BYTE byUser, bool bMin );
	//获取已经在玩游戏的玩家
	int GetPlayingPlayerCount( );
	//设置有效的特殊牛牛
	void SetValidNN( );
	//设置牛的倍数
	void SetNNPoint( );
	///获得胜利玩家
	BYTE GetWinPeo( );
	///获得最大的牛牛玩家
	BYTE MaxNNUser( );
	//玩家提示翻牌
	void HandleTiShi(BYTE bDeskStation, TMSG_TI_SHI_REQ* tTiShiReq);
	//处理庄家开始游戏
	void HandleMasterStart(BYTE bDeskStation, TMSG_MASTER_START_GAME_REQ* pMasterStart);


	bool SendGameDataEx(BYTE bDeskStation, BYTE bMainID, BYTE bAssID, BYTE bHandleCode);
	bool SendWatchDataEx(BYTE bDeskStation, BYTE bMainID, BYTE bAssID, BYTE bHandleCode);

	bool SendGameDataEx(BYTE bDeskStation, void * pData, UINT uSize, BYTE bMainID, BYTE bAssID, BYTE bHandleCode);
	bool SendWatchDataEx(BYTE bDeskStation, void * pData, UINT uSize, BYTE bMainID, BYTE bAssID, BYTE bHandleCode);
public:
	unsigned int					m_iPlayMode;							//玩法
	vector<PlayerStation>			m_vPlayingPlayer;						//正在玩游戏的玩家
	int								m_iCanNote[PLAY_COUNT][3];				//能够下注的分数
	int								m_iLastNote[PLAY_COUNT];				//上一把玩家下注分数
	int								m_iLastWinPoint[PLAY_COUNT];			//上一把玩家得分分数
	bool							m_bNNValid[6];							//有效的特殊牛牛
	BYTE							m_byLastGradePeople;					//上一把庄家位置	
	BYTE							m_bySonStation[PLAY_COUNT];				//摊牌期间的子状态
	bool							m_bMasterStartGame;						//房主点击开始游戏
	bool							m_bFirstLoad;							//第一次加载
	bool							m_bSystemOperate;						//是否系统操作
	TMSG_LAST_GAME_RSP				m_tLastGameInfo;						//上一局信息
protected:
	/*---------------------------游戏基础数据----------------------------------*/
	CUpGradeGameLogic		m_Logic;						//游戏逻辑
	
	BYTE					m_iAllCardCount;							//游戏所用的牌数
	BYTE					m_iTotalCard[NUM_CARD_ARRAY_WITH_FACE];		//总的牌

	/*---------------------------游戏基础数据每局需要重置的数据----------------*/
	SuperUserSetData		m_SuperSetData[PLAY_COUNT];					//超端设置的数据
	bool                    m_bUserReady[PLAY_COUNT] ;			///玩家准备状态
	BYTE					m_byUpGradePeople;					//庄家位置	
	BYTE                    m_iFirstJiao;						//第一个抢庄玩家位置	

	BYTE					m_byUserStation[PLAY_COUNT];			//记录用户状态	DWJ
	BYTE					m_iUserCardCount[PLAY_COUNT];			//用户手上扑克数目
	BYTE					m_iUserCard[PLAY_COUNT][SH_USER_CARD];	//用户手上的扑克
	int						m_iRobNTBase[PLAY_COUNT];				//玩家抢庄状态 -1表示还未操作，0表示不抢庄 1表示抢庄，开房间模式下1-4表示抢庄倍数	DWJ
	int						m_iNTBase;								//抢庄倍数/庄家倍数
	int						m_iUserBase[PLAY_COUNT];				//闲家倍数

	int						m_iOpenShape[PLAY_COUNT];
	BYTE					m_byOpenUnderCount[PLAY_COUNT];		//底牌张数
	BYTE					m_byOpenUnderCard[PLAY_COUNT][3];	//底牌的三张牌
	BYTE					m_byOpenUpCard[PLAY_COUNT][2];		//升起来的2张牌
	BYTE                    m_byBullCard[PLAY_COUNT][3];          //玩家摆的牛牌 


	/*---------------------------游戏可配置参数----------------------------------*/
	BYTE				m_iBeginTime;			//游戏开始时间
	BYTE				m_iThinkTime;			//游戏摆牛思考时间
	BYTE				m_iXiaZhuTime;			//下注时间
	BYTE				m_iRobNTTime;		    //抢庄时间( 开房间模式 )
	BYTE				m_bCardShapeBase[MAX_SHAPE_COUNT];//0-10代表无牛到牛牛间的倍率, 11-15为花样玩法牌型的倍率
	//超端参数
	vector<long>		m_vlSuperID;				//保存超端玩家的容器

	//以下参数都是可配置的 但是没有写在配置文件中 就是防止客户乱配出现问题
	BYTE				m_iSendCardTime;		//发牌时间-控制客户端发一张牌的速度 
	BYTE				m_iRandomNTTime;		//随机庄家动画时间间隔( ms )
	BYTE				m_iRandomNTRound;		//随机庄家动画播放圈数
	BOOL				m_bTurnRule;			//游戏顺序 0-顺时针 1-逆时针
	int					m_iGoodCard;			//发大牌机率牛七以上
	bool				m_bHaveKing;			//是否有王

	time_t				tRobNTTime;
	time_t				tShowCardTime;
	time_t				tNoteTime;
	
	CalculateBoard			m_TCalculateBoard;
public:
	//构造函数
	CServerGameDesk( ); 
	//析构函数
	virtual ~CServerGameDesk( );

	//重载函数
public:
	//初始化游戏逻辑
	virtual bool InitDeskGameStation( );
	//游戏开始
	virtual bool	GameBegin( BYTE bBeginFlag );

	//游戏结束
	virtual bool	GameFinish( BYTE bDeskStation, BYTE bCloseFlag );
	//判断是否正在游戏
	virtual bool IsPlayGame( BYTE bDeskStation );

	//判断是否正在游戏
	virtual bool IsPlayGameEx( BYTE bDeskStation );

	//框架消息处理函数
	virtual bool HandleFrameMessage( BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser );
	//游戏数据包处理函数
	virtual bool HandleNotifyMessage( BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser );
	//用户离开游戏桌
	virtual BYTE UserLeftDesk( BYTE bDeskStation, CGameUserInfo * pUserInfo );
	//
	virtual bool UserNetCut( BYTE bDeskStation, CGameUserInfo *pLostUserInfo );
	//用户断线重来
	virtual bool UserReCome( BYTE bDeskStation, CGameUserInfo * pNewUserInfo );
	//中途大结算
	virtual void DissMissRoomBefore( );

	///是否可以开始游戏
	virtual bool CanBeginGame();

	//重载纯虚函数
public:
	//获取游戏状态信息
	virtual bool OnGetGameStation( BYTE bDeskStation, UINT uSocketID, bool bWatchUser );
	//重置游戏状态
	virtual bool ReSetGameState( BYTE bLastStation );
	//记录机器人输赢值
	void RecordAiHaveWinMoney( GameEndStruct *GameEnd );
	//定时器消息
	virtual bool OnTimer( UINT uTimerID );
	//处理函数
private:
	//获得游戏基础信息
	void GetGameBaseStat( GameStation_Base& TGameStation );
public:
	//读取静态变量
	BOOL	LoadIni( );			
	//超级用户验证
	void	SuperUserExamine( BYTE bDeskStation );
	//是否超端用户 超端用户要发送结果给他看到
	bool	IsSuperUser( BYTE byDeskStation );
	//统计当前还未下注的玩家个数
	BYTE	CountNoNotePeople( );
	//房间倍数
	int		GetRoomBasePoint( );
	//计算各家分数
	int		ComputePoint( BYTE DeskStation );

	//换牌
	BOOL	ChangeCard( BYTE bDestStation, BYTE bWinStation );
	//交换指定两个玩家的手牌
	void	ChangeTwoUserCard( BYTE byFirstDesk, BYTE bySecondDesk );
	
	//获取下一个玩家位置
	BYTE	GetNextDeskStation( BYTE bDeskStation );
	//通知抢庄
	BOOL	NotifyRobNT( );
	//抢庄
	BOOL	HandleUserRobNT( BYTE bDeskStation, int iVal );
	//是否抢庄结束
	BOOL IsRobNTFinish( );
	//叫分结束, 
	BOOL	RobNTFinish( bool bShowDing, int iPersonCount );
	//通知用户下注
	BOOL	NoticeUserNote( );
	//处理用户下注结果
	bool	HandleUserNoteResult( BYTE bDeskStation, int iNoteType = 0 );
	//发送扑克给用户
	BOOL	NotifySendCard( );
	//发牌结束
	BOOL	NotifySendCardFinish( );
	//开牌
	BOOL	BeginOpenCard( );
	//bClient: 客户端true( 检测客户端数据合法性及牛的正确性 ), 服务端false
	BOOL	UserOpenCard( BYTE byDeskStation, void * pData );
private:
	void GetBullCard( BYTE byDeskStation );
public:
	//检测是否结束
	BOOL	CheckFinish( );

	int GetCurPlayerCount( );

	//自动开始游戏
	void AutoBegin( );
	
	///清理所有计时器
	void	KillAllTimer( );

	///获取当前牌中最大的牌型，
	int		GetMaxCardShape( BYTE iCard[], int iCardCount );
private:
	//修改奖池数据1( 平台->游戏 )
	/// @param bAIWinAndLostAutoCtrl 机器人控制输赢开关
	/// @return 是否成功
	virtual bool SetRoomPond( bool	bAIWinAndLostAutoCtrl );

	//修改奖池数据2( 平台->游戏 )
	/// @param m_iAIWantWinMoney 机器人输赢控制：机器人赢钱区域1, 2, 3 4所用断点
	/// @param m_iAIWinLuckyAt 机器人输赢控制：机器人在区域1, 2, 3, 4赢钱的概率
	/// @return 是否成功
	virtual bool SetRoomPondEx( __int64	iAIWantWinMoney[], int	iAIWinLuckyAt[] );

	//修改奖池数据3( 平台->游戏 )
	/// @param iReSetAIHaveWinMoney 机器人输赢控制：重置机器人已经赢钱的数目
	/// @return 是否成功
	virtual bool SetRoomPondEx( __int64	iReSetAIHaveWinMoney );

	//控制胜负
	/// 判断
	virtual BOOL Judge( );
	/// 胜
	virtual BOOL JudgeWiner( );
	/// 输
	virtual BOOL JudgeLoser( );
	//GRM函数
	//更新奖池数据( 游戏->平台 )
	/// @return 机器人赢钱数
	virtual void UpDataRoomPond( __int64 iAIHaveWinMoney );

	void UpdateCalculateBoard( );

#if MODE_SET_CARD_DEBUG
private:
	//读取配牌配置
	void LoadSetCard( );	
	//改牌函数
	void ChangeCard2Test( );
private:
	bool m_bTestSwitch;
	BYTE m_byCardArr[PLAY_COUNT][SH_USER_CARD];
#endif // MODE_SET_CARD_DEBUG

};

/******************************************************************************************************/
