#ifndef DATAMANAGE_H
#define DATAMANAGE_H
#include "CardArrayBase.h"
#include "UpgradeMessage.h"
#include "MoneyManage.h"
#include <queue>
#include <deque>
#include <list>
#include <set>
#include "GameDefine.h"
#include "GameContext.h"
class CServerGameDesk;

class DataManage:public GameContextExtension
{
public:
	DataManage();
	~DataManage();

	struct sBaseUserInf
	{
		//ID 号码
		UID			  userID;		
		//金币
		__int64		  iMoney;		
		//登录名
		char		  sName[61];
		//昵称
		char		  sNickName[61];
		//是否是旁观
		bool		  bWatcher;
		//是否离线
		bool		  bOffline;
		//是否是机器人
		bool          bIsVirtual;
		//是否是A队(庄家一起的)
		bool		  isTeamA;
		sBaseUserInf()
		{	
			userID			= 0;
			iMoney			= 0;
			memset(sName,0,sizeof(sName));
			memset(sNickName,0,sizeof(sNickName));
			bWatcher		= false;
			bOffline		= false;
			isTeamA			= false;
			bIsVirtual      = false;
		}
	};

	struct sGameUserInf:sBaseUserInf
	{	
		int					  iNomalScore;								//输赢分							
		int					  iTotalScore;								//累计结束得分	
		int                   iWinCount;								//累计押中次数
		int                   iRecordWinCount;                          //记录最高连胜次数
		__int64               iXiaZhuMoney;                             //已下注的钱
		__int64				  i64UserXiaZhuData[BET_ARES];			    //玩家区域下注信息
		__int64               iScore;	                                //小局分累计输赢
		vector<E_RECORD>	  vRecord;									//战绩
		uchar				  iRunCnt;									//总运行局数
		CardArrayBase		  cHandCards;								//玩家牌	
		E_PLAYCARD_STATE	  ePlayCardState;							//出牌状态		
		bool				  bCardOver;								//是否出完牌
		CardArrayBase		  cScoreCards;								//赢得分数牌
		int					  iFineScore;								//罚分
		int					  iBonusScore;								//奖分				  
		sGameUserInf()
		{
			iScore			= 0;
			iTotalScore		= 0;
			iWinCount       = 0;
			iRecordWinCount = 0;
			vRecord.clear();
			iRunCnt = 0;
			cHandCards.Clear();
			bCardOver = false;
			cScoreCards.Clear();
			iFineScore = 0;
			iBonusScore = 0;
			iNomalScore = 0;
			iXiaZhuMoney = 0;
			memset(i64UserXiaZhuData,0,sizeof(i64UserXiaZhuData));
		}
	};

	struct UserInfoBakeUp
	{
		//玩家座位号
		uchar		  bSeatNO;
		//大结算累计
		int			iTotalScore;
		//战绩
		E_RECORD	  vRecord[64];
		//连庄局数
		uchar		  bLnkBankerCnt;
		//总运行局数
		uchar		  iRunCnt;
		UserInfoBakeUp()
		{
			iTotalScore = 0;
			for (int i = 0;i < 64;i++)
			{
				vRecord[i] = E_RESULT_NIL;
			}
			bLnkBankerCnt = 0;
		}
	};

	typedef map<uchar,sGameUserInf> MUSERS;

	/*
	*@brief 判断是否是正常游戏玩家
	*@param uchar bSeatNO
	*@return bool
	*/
	bool isNomalUser(uchar bSeatNO);
	/*
	*@brief 判断是否是旁观玩家
	*@param uchar bSeatNO
	*@return bool
	*/
	bool isWatcher(uchar bSeatNO);
	/*
	*@brief 下一个玩家
	*@param uchar bSeatNO
	*@return uchar
	*/
	uchar nextUser(uchar bSeatNO);
	/*
	*@brief:与next反方向
	*@Returns:   uchar
	*@Parameter: uchar bSeatNO
	*/
	uchar prevUser(uchar bSeatNO);
	/*
	*@brief 下一个可以操作的玩家
	*@param uchar bSeatNO
	*@return uchar
	*/
	uchar nextActiveUser(uchar bSeatNO);
	/*
	*@brief 添加玩家
	*@param uchar bSeatNO
	*@return bool
	*/
	bool addUser(uchar bSeatNO);

	/*
	*@brief:恢复数据添加玩家
	*@Returns:   bool
	*@Parameter: uint UserID
	*/
	bool addRecoveryUser(uint userID);
	/*
	*@brief 更新玩家信息
	*@return bool
	*/
	bool updateUser();
	/*
	*@brief 删除玩家
	*@param uchar bSeatNO
	*@return bool
	*/

	bool eraseUser(uchar bSeatNO);

	/*
	*@brief 更新玩家金币
	*@return bool
	*/
	void updateUserMoney();

	/*
	*@brief 初始化
	*@return void
	*/
	void init(CServerGameDesk *p);

	/*
	*@brief 每局初始化调用
	*@return void
	*/
	void initOnce();
	/*
	*@brief 上一个出牌玩家
	*@return byte
	*/
	uchar getLastOutUser();
	void setLastOutUser(uchar bUser);
	/*
	*@brief 上一个操作玩家
	*@return uchar
	*/
	uchar getLastActionUser();
	void setLastActionUser(uchar bSeatNO);

	/*
	*@brief 上一个下注玩家
	*@return uchar
	*/
	uchar getLastBetUser();
	void setLastBetUser(uchar bSeatNO);


	/*
	*@brief 当前应该操作的玩家
	*@return uchar
	*/
	uchar  getCurUser();
	void  setCurUser(uchar bSeatNO);

	/*
	*@brief 修改玩家信息
	*@param uchar bSeatNO
	*@param sGameUserInf & inf
	*@return bool
	*/
	bool alterUserInfo(uchar bSeatNO,sGameUserInf &inf);
	bool getUserInfo(uchar bSeatNO,sGameUserInf &inf);

	size_t UserCount();

	/*
	*@brief 洗牌分配牌
	*@return bool
	*/
	bool shuffleCard();
	
	//排序手牌
	void sortusercard(uchar card[]);
	//获取牌型
	int  GetCardShape(uchar card[]);
	//是否豹子
	bool baozi(uchar card[]);
	//是否顺金
	bool shunjin(uchar card[]);
	//是否金花
	bool jinhua(uchar card[]);
	//是否顺子
	bool shunzi(uchar card[]);
	//是否对子
	bool duizi(uchar card[]);
	//是否单张
	bool danzhang(uchar card[]);
	//获取牌值
	int getvalue(uchar card);
	//获取花色
	int gethuase(uchar card);
	//判断手牌花色是否都相同
	bool allhuase(uchar card[]);
	//是否顺子
	bool isshunzi(uchar card[]);
	//双方牌型相同再继续对比大小花色返回获胜方，1为红
	int getwinuser(uchar hongcard[], uchar heicard[], BYTE shape);
	//豹子选最大的
	int getbaoziuser(uchar hongcard[], uchar heicard[]);
	//顺金选最大的
	int getshunjinuser(uchar hongcard[], uchar heicard[]);
	//金花选最大的
	int getjinhuauser(uchar hongcard[], uchar heicard[]);
	//顺子选最大的
	int getshunziuser(uchar hongcard[], uchar heicard[]);
	//对子选最大的
	int getduiziuser(uchar hongcard[], uchar heicard[]);
	//单张选最大的
	int getdanzhanguser(uchar hongcard[], uchar heicard[]);
	//最大的顺子玩家
	int maxshunziwinuser(uchar hongcard[], uchar heicard[]);
	//通过概率强行安排一波,当为true时用来控制某一方的输赢
	void gailvcontrol(bool flag=false, int user=0);
	//推送特殊牌型倍率
	int getbeilv(int shape);
	/*
	*@brief 配牌
	*@param map<uchar
	*@param vector<CardBase>> & cards
	*@return bool
	*/
	void LoadConfCardUser();

	/*
	*@brief 玩家撤销数据处理
	*@param byte bSeatNO
	*@param const CardArrayBase &Card
	*@return bool
	*/

	void opCancelCard(uchar bSeatNO);

	//获取桌主信息
	uchar getMasterDeskNO()const;
	//通知更新
	void notiDataMoney();

	//处理通知数据更新
	void ProcessData(BYTE userMaxMoney[]);
	//更新金币堆
	void updateDataMoney();

	/**从配置文件中读取机器人控制设置的参数值*/
	void GetAIContrlSetFromIni();	
	//奖池记录
	void	RecordAiWinMoney();

	bool checkPlayCard(uchar bSeatNO,uchar *bCards,size_t sLen);
	/*
	*@brief 玩家出牌数据处理
	*@param byte bSeatNO
	*@param byte bCards
	*@param size_t sLen
	*@return bool
	*/
	bool playCard(uchar bSeatNO,uchar *bCards,size_t sLen);
	/*
	*@brief 获取玩家手牌
	*@param byte bSeatNO
	*@param byte * pCard
	*@param size_t & sLen
	*@return bool
	*/
	bool getUserHandCard(uchar bSeatNO,uchar *pCard,size_t& sLen);

	/*
	*@brief 结果类型
	*@return E_RESULT_TYPE
	*/
	E_RESULT_TYPE DataManage::getResultType()const;
	void DataManage::setResultType(E_RESULT_TYPE e);


	/*@brief 庄家
	*@param byte bSeatNO
	*@return bool
	*/
	bool  setBanker(uchar bSeatNO);
	uchar getBanker();


	/*
	*@brief:获取名次
	*@Returns:   vector<uchar>
	*/
	vector<uchar> getRank();
private:


public:
	struct WinGreater
	{
		WinGreater(unsigned int id,int _count):deskNO(id),count(_count){};
		unsigned int deskNO;
		int count;
	};
	map<uint,UserInfoBakeUp>		usrInfobak;				//游戏中断备份玩家信息
	priority_queue<MoneyManage,vector<MoneyManage>,stuless>   dataMoney;  //存储游戏中金币数最多的玩家
	vector<CardBase>                vSendCard;              //存储发的牌
	BYTE										UserCard[2][3];			//6张牌，一维0代表红,另外代表黑
	BYTE										UserShape[2];				//两方牌型，0代表红
	BYTE										WinUserShape;			//赢家的牌型
	vector<WinGreater>              vWinCount;              //存储游戏中连续押中最多的玩家
	bool                            m_isSupSet;             //超端是否设置
	bool							m_bAIWinAndLostAutoCtrl; ///机器人输赢控制：是否开启机器人输赢自动控制
	BYTE					        byRunSeq[MAXCOUNT];     //游戏每局运行结果序列
	BYTE							byRunCardShape[MAXCOUNT];	//游戏每局运行结果序列牌型
	BYTE                            byGameBeen;             //游戏当前运行局数
	BYTE							byRunHongCount;         //中红的局数个数
	BYTE							byRunHeiCount;           //中黑的局数个数
	//BYTE							byRunHeCount;           //中和的局数个数
	BYTE							byWinQuYu;			  //中奖区域   1：红    2：黑  
	bool							m_bSpecialCardWin;	//是否中了特殊牌型区域
	int                             iRoomFen;                 //房间分数
	int								m_haveTimeSs;           //当前剩余时间
	int								rAresData[BET_ARES][CHOUMA_NUM];    //每个区域里的各种筹码的个数
	int								m_iAIWinLuckyAtA1;			/**<机器人输赢控制：机器人在区域1赢钱的概率  */
	int								m_iAIWinLuckyAtA2;			/**<机器人输赢控制：机器人在区域2赢钱的概率  */
	int								m_iAIWinLuckyAtA3;			/**<机器人输赢控制：机器人在区域3赢钱的概率  */
	int								m_iAIWinLuckyAtA4;			/**<机器人输赢控制：机器人在区域4赢钱的概率  */  
	__int64							i64QuYuZhu[BET_ARES]; 	 /**< 本把每个区域已下的总注额*/
	__int64							m_i64AIWantWinMoneyA1;		/**<机器人输赢控制：机器人赢钱区域1  */
	__int64							m_i64AIWantWinMoneyA2;		/**<机器人输赢控制：机器人赢钱区域2  */
	__int64							m_i64AIWantWinMoneyA3;		/**<机器人输赢控制：机器人赢钱区域3  */
	__int64							m_i64AIHaveWinMoney;		///机器人输赢控制：机器人已经赢了多少钱
	                        
private:
	//玩家表
	map<uchar,sGameUserInf> _userData;
	//庄家
	uchar _banker;
	//上一个操作玩家
	uchar _lastActionUser;
	//上一个出牌玩家
	byte _lastOutUser;
	//当前可以操作的玩家
	uchar _curUser;
	//上一个下注（跟注）的玩家
	uchar _lastBetUser;
	//牌堆
	CardArrayBase _cardHeap;
	//桌子指针
	CServerGameDesk *_pDesk;
	//判定朋友的牌
	uchar		  _bFriendCard;
	//朋友关系是否明确
	bool _FriendShow;

	//名次
	vector<uchar> _Rank;
	//结局类型
	E_RESULT_TYPE _resultType;
};
#endif // DATAMANAGE_H