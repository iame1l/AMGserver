#pragma once

#include "../../../../sdk/include/ServerCommon/GameDesk.h"
#include "../GameMessage/UpgradeMessage.h"

//定时器 ID
#define TIME_MY_TIMER				20				//公用定时器，只有这个定时器是实际定时器，其余都是从这个定时器经过判断来的
#define MY_ELAPSE					250				// 定时器间隔
#define TIMER_COUNT					11				// 用到的定时器数目
/*---------------------------------------------------------------------*/
// 下列为游戏中使用到的定时器，由上面的TIME_MY_TIMER计数进行统一处理
#define	TIME_MIN					30				//最小计时器ID
#define	TIME_MAX					50				//最大计时器ID
/*---------------------------------------------------------------------*/

//游戏结束标志定义
#define GF_NORMAL					10				//游戏正常结束
#define GF_SALE						11				//游戏安全结束


/*---------------------------------------------------------------------*/
//奖池控制参数
//奖池等级
static	__int64		G_i64RewardLevel[3] = {100000, 1000000, 10000000};
//奖池等级对应的概率值
static	int			G_iStorLevelProbability[4]={80, 70, 60, 50};
//当前库存值
//上一次的奖池金额
static	__int64		G_i64PreStorMoney = 0;





/*---------------------------------------------------------------------*/
//游戏桌类
class CServerGameDesk : public CGameDesk
{
	CRITICAL_SECTION m_csForIPCMessage;

private:		
	bool		   m_bIsPlaying;			        //游戏是否已经开始
	

private:		/*可配置项*/

	bool			m_bClearPreWin;		            //是否自动清空上轮赢钱数

	int				m_iBeilu_20_10;
	int				m_iBeilu_30_15;
	int				m_iBeilu_40_20;

	int				m_iSmallFruits;
	int				m_iBigFruits;
	int				m_LuckEgg;
	int				m_iBar;
	
	int             m_iCellMoney;		            //单线下注数

	
	int             m_iRobotPlayMaxTime;            //机器人最大切换桌子时间
	int             m_iRobotPlayMinTime;            //机器人最小切换桌子时间

	int             m_iAreaBetLimit;                //各区域下注限制
	int             m_iChangeRate;                 //兑换比率


    static ULONG     g_Administrator;                       //超级管理员  
	static	__int64	 G_i64CurrStorMoney;                 //当前库存量
	static __int64  G_i64Pond;								//全屏库存
	static bool     g_IsLoadSetting;                        //是否读取静态配置文件
private:
	long long		m_i64UserMoney;					//玩家身上的金币数	
	INT             m_iLevel;                       //玩家当前等级
	__int64         m_iExp;                         //玩家当前经验值
	DWORD           m_NowTime;						//当前时间

	//图形排列
	int				m_iAllWinMoney;			        //总体赢钱

	bool            m_bIsVisited[24];               //当前水果是否已经被访问过

	__int64			m_iRewardLeve1;					//区域1的彩金范围
	__int64			m_iRewardLeve2;					//区域2的彩金范围
	__int64			m_iRewardLeve3;					//区域3的彩金范围

	int				m_iAIWinLuckyAtA1;				//区域1中奖概率
	int				m_iAIWinLuckyAtA2;				//区域2中奖概率
	int				m_iAIWinLuckyAtA3;				//区域3中奖概率
	int				m_iAIWinLuckyAtA4;				//超出区域3中奖概率
	int				m_bWinAndLostAutoCtrl;			//发奖控制开关
	int             m_iAreaBetNum[BET_AREA];               //各区域下注数
	int             m_iAreaWin[BET_AREA];           //每个区域赢的钱
	int             m_iLBeilv;                      //左边三个图案的倍率
	int             m_iRBeilv;                      //右边三个图案的倍率
	int             m_iBibeiType;                   //比倍类型
	int             m_iLampNo;                     //送出几个灯了                      
	vector<int>     m_lvOpenResult;                 //开奖结果保存在该容器中
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
	///判断此游戏桌是否开始游戏
	virtual BOOL	IsPlayingByGameStation();
	//游戏数据包处理函数
	virtual bool	HandleNotifyMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser);
	//框架消息处理函数
	virtual bool	HandleFrameMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser);
	//用户离开游戏桌
	virtual BYTE	UserLeftDesk(BYTE bDeskStation, CGameUserInfo * pUserInfo);
	///用户坐到游戏桌
	virtual BYTE	UserSitDesk(MSG_GR_S_UserSit * pUserSit, CGameUserInfo * pUserInfo);
	//用户断线
	virtual bool	UserNetCut(BYTE bDeskStation, CGameUserInfo * pUserInfo);
	//初始化桌子信息
	virtual bool	InitDeskGameStation();
	//获取游戏状态信息
	virtual bool	OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser);
	//重置游戏状态
	virtual bool	ReSetGameState(BYTE bLastStation);
	//定时器消息
	virtual bool	OnTimer(UINT uTimerID);

	virtual  bool SetRoomPond(bool	bAIWinAndLostAutoCtrl);
	virtual  bool SetRoomPondEx(__int64	iAIWantWinMoney[], int	iAIWinLuckyAt[]);
	virtual  bool SetRoomPondEx(__int64	iReSetAIHaveWinMoney);
	/// 判断
	virtual BOOL Judge();
	/// 胜
	virtual BOOL JudgeWiner();
	/// 输
	virtual BOOL JudgeLoser();


private:
	//玩家启动滚动
	void OnHandleStartRoll(BYTE byStation,void * pData, UINT uSize);

private:
	//开始启动游戏
	void	StartGame();
	//是否超级用户
	bool	IsSuperUser(BYTE byStation);
	//是否黑名单用户
	bool IsBlackListUser(BYTE byStation, TMSG_USER_DATA &stUserData);
	bool IsWhiteListUser(BYTE byStation, TMSG_USER_DATA &stUserData);
	//判断奖池等级
	int		GetStorLevel();
	//保存奖池记录
	void	RecordPool();
	//产生结果 @para：iranddigit作为函数里产生随机数的种子
	void  ProduceResult(int iranddigit);
	//获取一个水果
	int getALamp(int iranddigit);
	//计算得分
	void countfen();
	//每一个开奖类型得分
	int GetPoint(int iType);
	void OnHandleBibei(BYTE byStation,void * pData, UINT uSize);
	//验证超端用户
	void ExamineSuperUser(BYTE byDeskStation);
private:
	//清空该位置的数据
	void	IniUserData(BYTE byStation);
	//读取静态变量
	BOOL	LoadIni();	
	//根据房间ID加载底注和上限
	BOOL	LoadExtIni(int iRoomID);
	
	//static void LoadSetting();//加载静态配置文件

public:
	//查询是否可以开出大奖
	bool GetMainPond(void);
	void CtrlWinLose(void);

	// 获取控制结果
	bool CanControl( BYTE bDeskStation, TMSG_USER_DATA &stUserData );
	void DoControl( S_C_RollResult &stTypeScroll, TMSG_USER_DATA &stUserData );
	// 是否全屏赠送
	bool CanGiveFullScene( BYTE bDeskStation, int& iTypeID );
	// 全屏赠送数据
	void DoGiveFullScene( S_C_RollResult &TTypeScroll, int typeID, CMD_S_Pond &pondMsg );

	///////////// 管理员控制 处理 相关
private:
	void OnMsgAdminControlSendData( BYTE deskNumber, void *pData, UINT bufferLen );

	void OnMsgAdminControl_Ntf( BYTE deskNumber );
	bool OnMsgAdminiControl( BYTE deskNumber, BYTE msgID, void *pData, UINT size, bool isWatchUser );
	bool OnMsgAdminControl_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser );
	bool OnMsgAdminControlQueryStore_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser );
	bool OnMsgAdminControlClearStore_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser );
	bool OnMsgAdminControlAddStore_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser );
	bool OnMsgAdminControlSubStore_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser );
	bool OnMsgAdminControlUpdateAddList_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser );
	bool OnMsgAdminControlUpdateSubList_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser );
	bool OnMsgAdminControlUpdateFullList_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser );
	bool OnMsgAdminControlDeleteFullList_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser );

	bool GetPlayerDataByID( DWORD userID, TMSG_USER_DATA &stUserData );
	bool GetOnlinePlayer( vector<TMSG_USER_DATA> &vUserData );
	static void LoadSetting();//加载静态配置文件
	static void SaveSetting();//保存静态配置文件
	// 整理 列表数据
	void ClearUpListData( const TMSG_USER_DATA &stUserData, __int64 money );
	static vector<TMSG_USER_DATA>	      m_vWhitePlayer;	            // 超级用户列表(白名单)
	static vector<TMSG_USER_DATA>	      m_vBlackPlayer;	            // 超级用户列表(黑名单)
	static vector<TMSG_FULL_SCREEN_DATA>  m_vFullScreenPlayer;          // 全屏赠送用户列表
};

/******************************************************************************************************/
