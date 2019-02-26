#pragma once
#include "../../../../sdk/include/ServerCommon/GameDesk.h"
#include "../GameMessage/UpGradeLogic.h"
#include "GameDataObject.h"


//定时器 ID
#define IDT_USER_CUT				1L				//断线定时器 ID

//*****************************************************************************************
//CServerGameDesk为提高性能采用特殊的计时方式MyTimer，相关的ID和配置如下：
#define TIME_CHECK_READY			40				//检测是否准备
#define IDT_MYTIMER					0x21			//公用定时器，只有这个定时器是实际定时器，其余都是从这个定时器经过判断来的

#define MY_ELAPSE					500				//定时器间隔
#define TIMER_COUNT					25				//用到的定时器数目
#define TIMER_ALLTIMERFINSH         0x24            //总的定时器

#define IDMYT_BASE					30				//MyTimer基ID，不用作计时
#define IDMYT_EVENT_PROTECT			31				//事件流保护定时器

#define IDEV_SYS_CFG_DESK			35				//系统设定桌面配置定时器/事件
#define IDEV_USER_CFG_DESK			36				//玩家设定桌面配置定时器/事件
#define IDEV_RAND_CARD				37				//洗牌定时器/事件

#define IDEV_SENDACARD				38				//下发牌定时器/事件
#define IDEV_SEND_3_CARD			39				//发   3 张牌定时器/事件
#define IDEV_SEND_4_5_CARD			40				//发第 4, 5 张牌定时器/事件
#define IDEV_BURN_CARD				42				//烧牌定时器/事件

#define IDEV_BET_START				43				//首次下注定时器/事件
#define IDEV_BET_BLINDS_AUTO		44				//大小盲注同时下注定时器/事件(系统强制)
#define IDEV_BET_NEXT				45				//下位玩家下注定时器/事件

#define IDEV_BIGBLIND               46              //下大肓注定时器
#define IDEV_SMALL_BLIND	        47              //下小肓注定时器

#define IDEV_ROUND_FINISH			48				//一回合完成/事件
#define IDEV_RESULT					49				//游戏结算定时器/事件
#define IDEV_NEXT_ROUND_BEGIN		50				//下一回合开始的定时器/事件		
#define IDEV_COMPARE_CARD           51              //比牌
#define IDEV_RANDOM					52				//随机一个庄与大小/事件

#define IDEV_AUTO_OPERATE           53              //自动操作定时器, 如果客户端玩家未操作，则服务器代理操作

#define ONE_SECOND                  1000            // 1 秒时间

#define NUM_GET_ARR(arr)			(sizeof(arr)/sizeof(arr[0]))	//数组大小
//#define NUM_ONE_SECOND_MS			1000//1s=1000ms

//游戏结束标志定义
#define GF_NORMAL					10				//游戏正常结束
#define GF_SALE						11				//游戏安全结束

// Mode_Defs, Added by QiWang 20180312
#define MODE_DESK_DEBUG					FALSE//Debug Mode Set
#define MODE_LOG_DEBUG					TRUE//LOG Debug Mode Set
#define MODE_SET_CARD_DEBUG				FALSE//设置牌模式|配牌器开关
#define MODE_SUPER						TRUE//Mode Super(不要关闭)
#define MODE_SUPER_DEBUG				FALSE//Mode Super Debug
#define MODE_ALLIN_LIMIT				TRUE//Mode All In Limit
#define MODE_ALLIN_LIMIT_DEBUG			FALSE//Mode All In Limit Debug


// Mode_LOG_Defs, Added by QiWang 20180312
#if MODE_LOG_DEBUG
#define MODE_LOG_SEND					TRUE//发送
#define MODE_LOG_RECV					TRUE//接收
#define MODE_LOG_USER_IN				TRUE//用户进入本桌
#define MODE_LOG_USER_OUT				TRUE//用户离开本桌
//Timer(Log)
#define MODE_LOG_TIMER					TRUE//Timer
#define MODE_LOG_SET_CARD				MODE_SET_CARD_DEBUG//Set Card
#define MODE_LOG_REMIDN					TRUE//REMIND
#else
#define MODE_LOG_SEND					FALSE//发送
#define MODE_LOG_RECV					FALSE//接收
#define MODE_LOG_USER_IN				FALSE//用户进入本桌
#define MODE_LOG_USER_OUT				FALSE//用户离开本桌
//Timer(Log)
#define MODE_LOG_TIMER					FALSE//Timer
#define MODE_LOG_SET_CARD				FALSE//Set Card
#define MODE_LOG_REMIDN					FALSE//REMIND
#endif // MODE_LOG_DEBUG

//游戏桌类
class CServerGameDesk : public CGameDesk
{

public:
	//构造函数
	CServerGameDesk(); 

	//析构函数
	virtual ~CServerGameDesk();

#if MODE_LOG_DEBUG
protected:
	bool m_bLogOutput; // 输出日志与否标记,配置文件可配
#endif // MODE_LOG_DEBUG
#if MODE_SET_CARD_DEBUG
protected:
	bool m_bSetCard; //配牌器开关,配置文件可配
#endif // MODE_SET_CARD_DEBUG

//最新超端需求 20180411
protected:
	bool m_bSuperSetCardEffectNext; //超端设置牌下一局生效与否
	bool m_bSuperSetCardEffectCur; //超端设置牌当前局生效与否
	TSuperSetCardReq m_tSuperSetCardReqNext; //下一局超端设置牌数据
	TSuperSetCardReq m_tSuperSetCardReqCur; //当前局超端设置牌数据

	//机器人行为概率相关
private:
	SetActionProb	m_tSetActionProb;	//设定行为概率

private:
	GameBeginInfo	m_tGameBeginInfo;	//游戏开始信息

/*-----------------------------------------------------------------------------------------------------------
    一系列平台提供的需要重载来驱动游戏服务的函数，已适用大多数游戏的开发，但每个函数的实现因游戏而异。
  ----------------------------------------------------------------------------------------------------------*/
public:

	//初始游戏
	virtual bool InitDeskGameStation();

	//游戏开始
	virtual bool GameBegin(BYTE bBeginFlag);

	//游戏结束
	virtual bool GameFinish(BYTE bDeskStation,BYTE bCloseFlag);

	//判断是否正在游戏
	virtual bool IsPlayGame(BYTE bDeskStation);

	//根据游戏判断是否在游戏中
	virtual BOOL IsPlayingByGameStation();

	//框架消息处理函数
	virtual bool HandleFrameMessage(BYTE bDeskStation,NetMessageHead* pNetHead,void* pData,UINT uSize,UINT uSocketID,bool bWatchUser);

	//游戏数据包处理函数
	virtual bool HandleNotifyMessage(BYTE bDeskStation,NetMessageHead* pNetHead,void* pData,UINT uSize,UINT uSocketID,bool bWatchUser);

	//玩家坐下
	virtual BYTE UserSitDesk(MSG_GR_S_UserSit * pUserSit, CGameUserInfo * pUserInfo);

	//用户离开游戏桌
	virtual BYTE UserLeftDesk(BYTE bDeskStation,CGameUserInfo* pUserInfo);

	//用来截取玩家断线回来信息
	virtual bool UserReCome(BYTE bDeskStation,CGameUserInfo* pNewUserInfo);

	//用来截取玩家断线信息
	virtual bool UserNetCut(BYTE bDeskStation,CGameUserInfo* pLostUserInfo);

	//用来改变用户断线条件的函数
	virtual bool CanNetCut(BYTE bDeskStation);

	//释放房间资源时绩录玩家信息
	//通知游戏保存玩家游戏数据
	virtual void ReleaseRoomBefore();
	//恢复房间发送游戏数据到游戏服务器
	//iCount数据总数量
	virtual void SetReturnGameInfo(int UserID,void* szUserGameInfo,int iSize,int iCount);

  /*-重载纯虚函数-*/

	//获取游戏状态信息
	virtual bool OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser);

	//重置游戏状态
	virtual bool ReSetGameState(BYTE bLastStation);

	//定时器消息
	virtual bool OnTimer(UINT uTimerID);

	// 中途申请房间大结算
	virtual void DissMissRoomBefore();


/*-----------------------------------------------------------------------------------------------------------
        声明一系列桌子特有机制函数，已适用大多数游戏的开发，但每个函数的实现因游戏而异。
    函括配置机制、计时机制、动作流机制、游戏状态的管理
  ----------------------------------------------------------------------------------------------------------*/
public:

	//加载公共桌子配置
	BOOL LoadDeskCfg();

	//根据房间ID加载桌子配置
	BOOL LoadDeskCfg(int iRoomID, bool bConfigRC = true);

	/// 读取机器人行为概率
	bool LoadRobotActionProb();

	/// 发送机器人行为概率
	bool SendRobotActionProb(BYTE bDeskStation = PLAY_COUNT);

	//配制牌型
	BOOL LoadChangCard(void);

	//初始分组表
	void InitGroupTable(void);

	//添加定时器标志
	int SetMyTimer(UINT nTimeID,int uElapse);

	//清除定时器标志
	void KillMyTimer(UINT nTimeID);

	//定时器某标志的响应函数
	bool OnMyTimer(UINT uTimerID);

	//动作流控制
	bool ActionRedirect(UINT uiActID,int iDelay);

	//开打动作流保护程序
	bool OpenEventProtection(UINT uiActID,int iActLife);

	//事件进行时处理
	bool EventGoing();

	//释放事件资源
	bool CloseEventProtection();

	//获得游戏的基本的状态
	void GetGSBase(TGSBase* pGSBase,BYTE byDeskstation);

	//获得游戏的基本的状态
	void CopyGSBase(TGSBase* pTarget,const TGSBase* pSource);

	//检查是否可用牌
	bool CheckIsAvailableCard(BYTE byCard);

	//检查牌位于数组哪那个位置
	bool CheckCardPosInCardArr(BYTE byCard, int& iCardPos);

	//检查是否可以开始游戏
	bool CheckCanGameBegin();

	//检查是否可以过牌
	bool CheckCanSelectCheck(BYTE byDeskstation = PLAY_COUNT);

	//检查是否可以加注
	bool CheckCanSelectAdd(BYTE byDeskstation = PLAY_COUNT);

	//检查是否可以跟注
	bool CheckCanSelectCall(BYTE byDeskstation = PLAY_COUNT);

	//检查是否可以初始化房卡选项
	bool CheckCanInitRCOption();

	//初始化房卡选项
	bool InitRCOption();

	//初始化玩家金币
	bool InitPlayerMoney(BYTE byDeskstation);

	//同步金币值存储
	bool SyncMoney2Store(BYTE byDeskstation = PLAY_COUNT);

	//自动买入
	bool AutoBuyIn(BYTE byDeskstation, bool bIsDefault = true);

	//自动买入
	bool AutoBuyInAskMoney();

	//自动买入（玩家金币小于AllIn限制）
	bool AutoBuyInIfLTAllInLimit();

	//自动踢人
	bool AutoMakeUserOffLine();

	//自动踢人
	bool AutoMakeUserOffLine(BYTE byDeskstation);

	//自动设置下一个位置动作标记
	bool AutoMakeNextPosVerbs(BYTE bNextStation = PLAY_COUNT);
/*-----------------------------------------------------------------------------------------------------------
    声明游戏事件响应函数，功能函数，不同游戏差异较大
  ----------------------------------------------------------------------------------------------------------*/
public:

  /*-主动事件响应，桌子在游戏中自发驱动---*/

	//系统设定桌面的事件处理（主要设定时间、游戏规则等）
	bool OnEventSysCfgDesk();

	//玩家设定桌面的事件处理（主要设定底注）
	bool OnEventUserCfgDesk();

	//洗牌事件
	bool OnEventRandCard();

	//随机一个庄
	bool OnEventRandom();

	//为每个玩家发牌的事件
	bool OnEventSendACard();

	//为发3张公牌的事件
	bool OnEventSend3Card();

	//为发1张公共底牌的事件
	bool OnEventSend1Card();

	//首次下注事件
	bool OnEventBetStart();

	//大小盲注强制同时下注事件
	bool OnEventBetBlindsAuto();

	//下一位玩家下注事件
	bool OnEventBetNext();

	//开始下一回合游戏事件处理
	bool OnEventNextRoundBegin();

    //自动操作游戏事件处理
    bool OnEventAutoOperate();

    // 比牌
    bool OnCompareCard();
  /*-被动事件响应，通过客户端驱动---*/

	//处理玩家同意游戏
	bool OnUserAgreeGame(BYTE bDeskStation,
		NetMessageHead* pNetHead,void* pData,UINT uSize,UINT uSocketID,bool bWatchUser);

	//处理玩家设置底注
	bool OnUserCfgDesk(BYTE bDeskStation,
		NetMessageHead* pNetHead,void* pData,UINT uSize,UINT uSocketID,bool bWatchUser);

	//处理玩家设置底注超时
	bool OnUserCfgDeskTimeout(BYTE bDeskStation,
		NetMessageHead* pNetHead,void* pData,UINT uSize,UINT uSocketID,bool bWatchUser);

	//处理玩家同意底注
	bool OnUserAgreeDeskCfg(BYTE bDeskStation,
		NetMessageHead* pNetHead,void* pData,UINT uSize,UINT uSocketID,bool bWatchUser);

    //玩家代入金币设置
    bool OnUserSubMoney(BYTE bDeskStation, void* pData);

	//处理超端设置牌请求
	bool OnSuperSetCard(BYTE bDeskStation, const TSuperSetCardReq & tReqAlias);

	//玩家下注选择正确与否检测
	bool CheckOptionIsRightOnUserBet(emType nType);

	//更新有效加注信息
	bool UpdateValidRaiseOnUserBet(const BYTE& bDeskStation, const __int64& nAllBetMoney, const __int64& nRaiseMoneyTemp, const __int64& nAllBetMoneyValidRaisePos, bool bForceUpdate = false);

	//玩家下注信息
	bool OnUserBet(const BYTE bDeskStation, void* pData, bool bUserforceExit = false, const bool bCheckOption = false);

	//玩家下注信息
	bool AllInLimitRCEffectOnUserBet(const BYTE bDeskStation, bool &bFlagRCAllInTemp);

	//处理玩家发牌完成
	bool OnUserSendCardFinish(BYTE bDeskStation,
		NetMessageHead* pNetHead,void* pData,UINT uSize,UINT uSocketID,bool bWatchUser);

	//处理玩家对下一回合准备的信号
	bool OnUserNextRoundReq(BYTE bDeskStation,
		NetMessageHead* pNetHead,void* pData,UINT uSize,UINT uSocketID,bool bWatchUser);

	//处理玩家本身金币不足于最少带入金额
	bool OnUserMoneyLack(BYTE bDeskStation);

	//处理玩家当前买入请求
	bool OnUserCurBuyInReq(BYTE bDeskStation, void* pData, bool bForceEffect = false);

	////处理玩家线束本轮游戏的请求
	bool OnTestFinishRoundRequest(BYTE bDeskStation,
		NetMessageHead* pNetHead,void* pData,UINT uSize,UINT uSocketID,bool bWatchUser);


  /*-游戏事件保护，当某事件在生命期内没有收到任何手动销毁的消息，会自然毁灭，在毁灭前应该进行代管处理，以确保游戏流程不受影响--------*/

	//代管玩家发牌完成
	bool OnAgentSendCardsFinish();

	//出牌事件流代理
	bool OnAgentOutCard();

	//下一局开始事件流代理
	bool OnAgentNextRoundBegin();


  /*-游戏状态恢复响应，用于客户端在任何情况下启动游戏，得到最新最正确的数据-----*/
	//处理要得到游戏等待开始的请求
	bool OnGetGSWaitAgree(BYTE bDeskStation,
		UINT uSocketID,bool bWatchUser,TGSBase* pGSBase);

	//处理要得到游戏中状态的请求
	bool OnGetGSPlaying(BYTE bDeskStation,
		UINT uSocketID,bool bWatchUser,TGSBase* pGSBase);

	//处理要得到游戏回合结束的请求
	bool OnGetGSWaitNextRound(BYTE bDeskStation,
		UINT uSocketID,bool bWatchUser,TGSBase* pGSBase);

	//房卡场新追加信息
	bool OnGetRCInfoAppend(BYTE bDeskStation);


  /*-功能函数--*/

	//逆时针顺序得到下家位置
	BYTE GetNextUserStation(BYTE byStation, BOOL bIsAll = TRUE);

	//逆时针顺序得到上家位置
	BYTE GetLastUserStation(BYTE byStation, BOOL bIsAll = TRUE);

	//获取游戏玩家数量
	int GetPlayerCount(bool bFold = false);

    // 计算每轮胜利玩家
	void CalculateWin(BYTE nSectionCount, CByteArray &arrayResult,bool bForce = false);

	// 判断玩家是否属于配好牌玩家
	bool ExamSuperUser(BYTE byStation);


	int	 ExistRealUser();

	void AiWinAutoCtrl();

	//获得底牌
	void GotHandCards();

	//超端用户设定牌
	bool SuperUserSetCard();

	//设定数组对应位置的牌
	bool SetCardInArr(BYTE byCard, BYTE* byCardArr, int iIndex, bool* bSetCardArr);

	//随机设定数组对应位置的牌
	bool RandomSetCardInArr(BYTE* byCardArr, int iIndex, bool* bSetCardArr);

	bool FindUserID(const int iUserID, const TSuperSetCardReq& tSetCard, int& iFindIndex);

	void Change2UserCard(BYTE byFirstDesk,BYTE bySecondDesk);

	void RecordAiHaveWinMoney(__int64 i64UserScore[]);

	// 发送大结算消息
	void SendScoreBoard(bool dismiss);

	bool m_bLatestWinner[PLAY_COUNT]; //上一局玩家输赢标记数组

	// 玩家累计输赢
	__int64 m_TotalMoney[PLAY_COUNT];
	int m_ContinuouslyWin[PLAY_COUNT];
	int m_MaxContinuouslyWin[PLAY_COUNT];
	__int64 m_MaxWinningMoney[PLAY_COUNT];
	int m_WinCount[PLAY_COUNT];

	CalculateBoardData Statistic(int playerPos);

	void ClearStatData();

	void SendRemainingGameCount();

	//发送房卡配置
	void SendRCOption(bool bAll = true, BYTE byDeskStation = PLAY_COUNT);

	//发送当前货币
	void SendRCCurMoney(bool bAll = true, BYTE byDeskStation = PLAY_COUNT);

	//发送输赢统计信息
	void SendWinTotalInfo(bool bAll = true, BYTE byDeskStation = PLAY_COUNT);

	//发送当前可买入应答
	void SendRCCurCanBuyInRsp(bool bAll = true, BYTE byDeskStation = PLAY_COUNT);

	//发送当前可买入应答
	void SendRCCurBuyInRsp(TRCCurBuyInRsp& tRCCurBuyInRsp, bool bAll = true, BYTE byDeskStation = PLAY_COUNT);

	//发送大结算
	void SendRCScoreBoard(bool bAll = true, BYTE byDeskStation = PLAY_COUNT);

	//发送大结算
	void SendRCScoreBoard(BYTE byDeskStation);

	//Added by QiWang 20180323, 普通场T人相关
	void SetTimerCheckReadyInFinish();

	//测试新超端
#if MODE_SUPER_DEBUG
	void TestSuperSetCard();
#endif //MODE_SUPER_DEBUG

/*-----------------------------------------------------------------------------------------------------------
    定义桌面的数据，以下这些数据适用于大多数据游戏开发时使用
  ----------------------------------------------------------------------------------------------------------*/

public:

	// 每个玩家可用下注金币
	__int64			m_i64UserMoney[PLAY_COUNT];
public:
	bool			m_bAIWinAndLostAutoCtrl;	//是否开启自动控制

	__int64			m_iAIWantWinMoneyA1;				/**<机器人输赢控制：机器人赢钱区域1  */
	__int64			m_iAIWantWinMoneyA2;				/**<机器人输赢控制：机器人赢钱区域2  */
	__int64			m_iAIWantWinMoneyA3;				/**<机器人输赢控制：机器人赢钱区域3  */
	int				m_iAIWinLuckyAtA1;					/**<机器人输赢控制：机器人在区域1赢钱的概率  */
	int				m_iAIWinLuckyAtA2;					/**<机器人输赢控制：机器人在区域2赢钱的概率  */
	int				m_iAIWinLuckyAtA3;					/**<机器人输赢控制：机器人在区域3赢钱的概率  */
	int				m_iAIWinLuckyAtA4;					/**<机器人输赢控制：机器人在区域4赢钱的概率  */
protected:

	//游戏数据
	CGameDataObject m_GameData;

	//游戏逻辑
	CUpGradeGameLogic m_Logic;

	//服务器开启太多的定时器，会影响性能，故此作出优化，只开一个定时器，\
	  所有使用到的定时器都由这个定时器来触发，在触发中加以判断

	struct TMyTimerNode
	{
		//定时器标志号
		UINT uiID;

		//已经经过了次
		int  nTickCount; 

		//经过多少次后触发事件
		int  nTriggerCount; 

		TMyTimerNode(UINT nTimerID, int uElapse):\
			uiID(nTimerID),nTickCount(0),nTriggerCount(uElapse){}

		TMyTimerNode():\
			uiID(0),nTickCount(0), nTriggerCount(0){}
	};

	TMyTimerNode m_arTimer[TIMER_COUNT];

	//当前事件流保护状态的上下文
	struct TEvProtectDescr
	{
		//当前被保护的事件
		UINT uiEvID;

		//事件生命期
		int  iLiftTime;

		//已逝去的时间
		int  iPassTime;

		TEvProtectDescr():\
			uiEvID(0),iLiftTime(0),iPassTime(0){}

	};

	TEvProtectDescr m_EventProtecter;

	//Added by QiWang 20171106, 普通场准备时间到未准备T人相关
	bool m_bKickPerson;//可以T人与否标记
	bool m_bTimerCheckSet; //Timer Check Ready is Set or not
	BYTE m_byNoReadyTimeArr[PLAY_COUNT];//客户端未准备时间数组
	bool m_bInitKickPersonFlag;

	//释放房间标志,需要重新加载房间选项
	bool m_bReleaseRoom;
/*-----------------------------------------------------------------------------------------------------------
    定义桌面的数据，以下这些数据适用于大多数据游戏开发时使用
  ----------------------------------------------------------------------------------------------------------*/
private:
	bool m_bGameFinished;
	bool m_bGameStartAtleastOnce; //游戏开始至少一次标志，使用用房卡模式
protected:
	int						m_iUserStation[PLAY_COUNT];				//记录用户状态
	
	bool					m_bFaiPaiFinish;						// 发牌完成
	public:
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

#if MODE_LOG_DEBUG
private:
	//以下Log相关函数
	//Log Timer时使用
	string  GetTimerNameByID(UINT uTimerID);
	//Log Assistant Message时使用
	string  GetCHNameByAssID(UINT bAssistantID);
	//Log Message Send
	void LogMsgSend(UINT AssistID);
	void LogMsgSend(UINT AssistID, BYTE bDeskStation);
	void LogMsgSend(UINT AssistID, void *data, int dataSize);
	void LogMsgSend(UINT AssistID, BYTE bDeskStation, void *data, int dataSize);

	void LogMsgSend(UINT AssistID, const TToken& msg);
	void LogMsgSend(UINT AssistID, const TBet& msg);
	void LogMsgSend(UINT AssistID, const TRCCurMoney& msg);
	void LogMsgSend(UINT AssistID, BYTE bDeskStation, const TRCCurMoney& msg);
	void LogMsgSend(UINT AssistID, const TResult& msg);

	//Log Message Recv
	void LogMsgRecv(BYTE bDeskStation, UINT AssistID);
	void LogMsgRecv(BYTE bDeskStation, UINT AssistID, void *data, int dataSize);

	void LogMsgRecv(BYTE bDeskStation, UINT AssistID, const TBet& msg);

	//Log On Timer
	void LogOnTimer(UINT uTimerID);

	//Log On Remind
	void LogMsgRemind(const string& str);
	void LogMsgRemind(const string& str, BYTE bDeskStation);
#endif // MODE_LOG_DEBUG
};
