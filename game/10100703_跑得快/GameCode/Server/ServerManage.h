#pragma once

#include "GameDesk.h"
//#include "../GameMessage/UpGradeLogic.h"
#include "GameTaskLogic.h"
#include <afxmt.h>
#include "../CfgCard/CfgCard.h"
//定时器 ID
#define IDT_USER_CUT				1L										//断线定时器 ID

#define TIME_SEND_ALL_CARD			31				//发所有牌
#define TIME_SEND_CARD_ANI      	32				//等待反牌
#define TIME_SEND_CARD_FINISH       33              //发牌结束
#define TIME_AUTO_OUT_CARD			34				//自动出牌
#define TIME_OUT_CARD				36				//出牌
#define TIME_WAIT_NEWTURN			37				//新一轮时间设置
#define TIME_GAME_FINISH			38				//游戏结束定时器
#define TIME_GAME_PASS				39				//要不起要过

#define TIMER_WAIT_READY            40				//玩家准备时间				
#define	TIMER_WAIT_READY_MAX        TIMER_WAIT_READY + PLAY_COUNT-1  //玩家准备时间	

//游戏结束标志定义
#define GF_NORMAL					10				//游戏正常结束
#define GF_SAFE						11				//游戏安全结束
#define GF_TERMINATE				14				//意外停止游戏
#define GF_AHEAD_END				15				//提前结束

//*****************************************************************************************
//游戏桌类

class CServerGameDesk : public CGameDesk
{
public:
	static int		m_iBasePoint;						//游戏底分
	//静态变量
private:
	static 	bool  m_bHaveKing;      			    //是否有王(DEF=1,0)
	static 	BOOL  m_bKingCanReplace;				//王可否代替牌(DEF=1,0)
	static 	bool  m_bRobnt;         				//是否可以抢地主
	static 	bool  m_bAdddouble;    				//是否可以加棒
	static 	bool  m_bShowcard;     				//是否可以加倍
	static 	UINT  m_iPlayCard;     				//所有扑克副数(1,DEF=2,3
	static 	UINT  m_iBackCount;    				//底牌数(3,6,DEF=8,12)
	static 	UINT  m_iUserCount;    				//玩家手中牌数(12,13,DEF=25,27)

	static 	DWORD m_iCardShape;	  				//牌型
	//=============扩展
	static 	BYTE m_iThinkTime;               	//游戏思考时间
	static 	BYTE m_iBeginTime;               	//游戏开始时间
	static  BYTE m_iSendCardTime ;           	//发牌时间
	static  BOOL m_bTurnRule;					//游戏顺序
	static  BYTE m_FirstNecessaryCard;	        // 首出玩家所需的卡(例如红桃3)
	static	int  m_AutoOutCardTime;				// 自动出卡倒数
	static  BYTE m_MonkeyCard;					// 猴子

    static  BYTE m_bHaveMonkey;


	static  BOOL m_bIsAutoOutCard;				//是否自动出牌
	static  int  m_iOutTimeAutoCount;			//超时托管次数
	static  int m_ZhaDanDiFen;				    // 炸弹底分（还会剩以倍率）

	CMutex  m_Mutex;
	
protected:

	CGameTaskLogic          m_Logic ;                             ///游戏中的逻辑（包含任务逻辑）

	int  	                m_iLimitPlayGame;					   //至少打完多少局
	BYTE					m_iLeaveArgee;							//离开同意标志
	int						m_iHandCardNumber;						//游戏手牌张数(15张玩法就是15,16张玩法就是16)
	//状态信息
	BYTE					m_iUserCardCount[PLAY_COUNT];			//用户手上扑克数目
	BYTE					m_iUserCard[PLAY_COUNT][CARD_COUNT];	//用户手上的扑克
	BYTE					m_iBackCard[12];						//底牌列表
	BYTE					m_iBaseOutCount;						//出牌的数目
	BYTE					m_iDeskCardCount[PLAY_COUNT];			//桌面扑克的数目
	BYTE					m_iDeskCard[PLAY_COUNT][CARD_COUNT];	//桌面的扑克
	//发牌数据
	BYTE					m_iSendCardPos;							//发牌位置
	
	BYTE					m_iAIStation[PLAY_COUNT];				//四家机器托管状态
	BYTE					m_iHaveThingPeople;						//有事要走玩家
	BYTE					m_iGameFlag;							//游戏状态小分解
	
	bool					m_bIsLastCard;						//是否有上轮数据
	BYTE					m_iLastCardCount[PLAY_COUNT];		//上轮扑克的数目
	BYTE					m_iLastOutCard[PLAY_COUNT][CARD_COUNT];		//上轮的扑克
	BYTE					m_byteHitPass;						//记录不出
	bool                    m_byPass[PLAY_COUNT] ;             //本轮不出
	bool                    m_byLastTurnPass[PLAY_COUNT]  ;      //上轮不出

	BYTE                    m_bySendFinishCount ;               //发完牌的人

	int                     m_gameCountLimit; //进入房间局数限制
	int                     m_gameMoney;//进入房间金币数限制

	int						m_iNtFirstCount;						//地主出的第一手牌数
	int						m_iWinPeople;							//第一个出完牌玩家
	//运行信息
	int						m_iOutCardPeople;						//现在出牌用户
	int						m_iFirstOutPeople;						//先出牌的用户
	int						m_iNowBigPeople;						//当前桌面上最大出牌者
	int						m_iRecvMsg;								//收到消息计数
	int						m_iOutBombUser	;						//炸弹玩家

	int						m_iCallScore[PLAY_COUNT];
	int						m_iAwardPoint[PLAY_COUNT];				//牌形加分
	bool					m_bAuto[PLAY_COUNT];					//托管设置
	int                     m_iUserOutTime[PLAY_COUNT];             //用户超时次数，用于托管
	//比赛场使用变量
	int						m_iWinPoint[PLAY_COUNT];				//胜者
	int						m_iDealPeople;							//反牌玩家
	bool					m_bCanleave[PLAY_COUNT];				//能否离开
                    
	//斷線玩家
	BYTE					m_iFirstCutPeople;						//第一個掉線玩家
	int                     m_icountleave;                           //点离开人数统计
	DWORD					m_dThinkLeftTime;               		//剩余时间
	///玩家准备状态
	bool                    m_bUserReady[PLAY_COUNT] ; 

	bool					m_bLastHandFlag;						//最后一手牌,不处理客户端过来的牌

	vector <long>			m_vlSuperID;	//保存超端玩家的容器

	bool					m_bUserNetCut[PLAY_COUNT];	//标识玩家是否断线

	int						m_iDeskID;	//桌子ID(用于写日志)

	bool					m_bIsBombWin[PLAY_COUNT]; //是否炸弹赢取当局

	int						m_iBomMultiple[PLAY_COUNT];//炸弹倍数

	int						m_iMonkeyPlayMultiple[PLAY_COUNT];	//猴子打法的倍数

	BYTE					m_byBomOutNumber[PLAY_COUNT];		//本局出炸弹的个数

	S_C_SettlementList		m_tSettlementList;					//游戏总结算记录	

	//第一个出牌玩家
	bool					m_bFirstUserOut;

	bool					m_bAlreadSendSettlementList;		//是否已经发送了结算榜

	bool					m_bCoseSpring[PLAY_COUNT];			//检测当前是否关了春天了 当前是否被拦牌了
	//函数定义
public:
	//构造函数
	CServerGameDesk(); 
	//析构函数
	virtual ~CServerGameDesk();

	//重载函数
public:

	//游戏开始
	virtual bool GameBegin(BYTE bBeginFlag);
	//游戏结束
	virtual bool GameFinish(BYTE bDeskStation, BYTE bCloseFlag);
	//判断是否正在游戏
	virtual bool IsPlayGame(BYTE bDeskStation);
	//判断是否正在游戏
	virtual bool IsPlayGameEx(BYTE bDeskStation);
	//游戏数据包处理函数
	virtual bool HandleNotifyMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser);
	//用户离开游戏桌
	virtual BYTE UserLeftDesk(BYTE bDeskStation, CGameUserInfo * pUserInfo);
	///用户坐到游戏桌
	virtual BYTE UserSitDesk(MSG_GR_S_UserSit * pUserSit, CGameUserInfo * pUserInfo);
    ///拦截窗口消息
	virtual bool HandleFrameMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser);
	//用来改变用户断线条件的函数
	virtual bool CanNetCut(BYTE bDeskStation);//JAdd-20090116
	//重载纯虚函数
public:
	//获取游戏状态信息
	virtual bool OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser);
	//重置游戏状态
	virtual bool ReSetGameState(BYTE bLastStation);
	//定时器消息
	virtual bool OnTimer(UINT uTimerID);
	//配置INI文件
	virtual bool InitDeskGameStation();
	//处理函数
public:
	//用户设置游戏
	BOOL UserSetGame(UserArgeeGame * pGameSet);
	//初始化一系列工作(重新开始游戏才进行此种初始化)
	BOOL InitThisGame();
	//读取静态变量
	static BOOL LoadIni();
	//加载限注
	BOOL LoadExtIni();
	//根据房间ID加载底注和上限
	BOOL LoadExtIni(int iRoomID);
	//一次将所的牌全部发送
	BOOL	SendAllCard();
	//發送結束
	BOOL	SendCardFinish();
	//游戏开始
	BOOL	BeginPlay();
	//用户出牌
	BOOL	UserOutCard(BYTE bDeskStation, BYTE iOutCard[],  int iCardCount);
	//新一轮开始
	BOOL NewPlayTurn(BYTE bDeskStation);
	//用户请求离开
	BOOL UserHaveThing(BYTE bDeskStation, char * szMessage);
	//同意用户离开
	BOOL ArgeeUserLeft(BYTE bDeskStation, BOOL bArgee);
	//用户托管
	bool UserSetAuto(BYTE bDeskStation,bool bAutoCard);
	//填充日志表
	//	bool FindAllInfo(BYTE iCardList[],int iCardCount,GamePlayInfo  &iWriteInfo);
	//是否為新一輪
	BOOL IsNewTurn();

	//增加炸弹分
	void AddBomScroe();

	//殺所有計時器
	void KillAllTimer();
	//加分判断
	BOOL IsAwardPoin(BYTE iOutCard[],int iCardCount,BYTE bDeskStation);
	//桌面倍数
	int GetDeskBasePoint();
	//房间倍数
	int GetRoomMul();
	//逃跑扣分
	int GetRunPublish();
	//逃跑扣分
	int GetRunPublish(BYTE bDeskStation);
	//非强退玩家得分
	int GetRunAwayOtherGetPoint(BYTE bDeskStation,BYTE bRunDeskStation);
	//计算各家得分
	int GetTruePoint(BYTE bDeskStation);
	//机器人托管
	BOOL UseAI(BYTE bDeskStation );
	//玩家超时出牌(全部由服务端接管)
	BOOL UserAutoOutCard(BYTE bDeskStation);
	//发送取消机器人托管
	virtual BOOL SetAIMachine(BYTE bDeskStation,BOOL bState = FALSE);
	//玩家断线处理
	virtual bool UserNetCut(BYTE bDeskStation, CGameUserInfo * pLostUserInfo);
	//获取下一个玩家位置
	BYTE GetNextDeskStation(BYTE bDeskStation);
	//检测当是否是会关春天
	bool CheckCloseSpring(BYTE bDeskStation,BYTE byOutCard[],int iOutCardCount);
	//检查是否超端玩家
	bool	IsSuperUser(BYTE byDeskStation);
	//验证是否超端
	void	SpuerExamine(BYTE byDeskStation);
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

	bool IsBuyDesk()
	{
		return m_bIsBuy;
	}

	int GetSurplusCount()
	{
		return m_iVipGameCount - GetRunGameCount() + 1;
	}

	int GetRunGameCount()
	{
		return m_iRunGameCount + 1;
	}

	void ResetFirstSendRule(); // 重置出牌规则

////道俱
//bool UserProp(BYTE bsendDeskStation,BYTE brecvDeskStation,int iValue);

	//解散房间弹出结算框
	virtual void DissMissRoomBefore( void );

	//测试使用
	void IniTestSetHandPai();

	///SUPER SET CARD
	bool setCard();

	bool _isSuperMake;	//是否设置牌

	BYTE _superUserDesk;	//配牌玩家

	HN::CfgCard				m_Cfg;								 //设置牌

public:
	void  WriteLog(CString csMessage) ;//log

	size_t GetDeskId();

//20190511 读取套牌
private:
	bool dealerSendCardToAI();

	int SplitString(LPCTSTR lpszStr, LPCTSTR lpszSplit, CStringArray& rArrString, BOOL bAllowNullString);

	vector<BYTE> exchangeCardValue(const map<CString, int> &KV);

	// 转换成对应的字节
	BYTE getKeyValue(BYTE src);

	//卡牌查重
	bool isnormalCardList();
	//重新发牌,除了输入玩家位置
	void restartSendCard(int userSatation);
	//移除牌选用//从UpGradeLogic里提取的
	int removeCard(BYTE iRemoveCard[], int iRemoveCount, BYTE iCardList[], int iCardCount);
};

/******************************************************************************************************/
