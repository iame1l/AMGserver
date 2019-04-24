/****************************************************************************
Copyright (c) 2014-2017 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2017 ShenZhen Redbird Network Polytron Technologies Inc.
 
http://www.hotniao.com

All of the content of the software, including code, pictures, 
resources, are original. For unauthorized users, the company 
reserves the right to pursue its legal liability.
****************************************************************************/
#ifndef DATAMANAGE_H
#define DATAMANAGE_H

#include "./Card/CardArrayBase.h"
#include <deque>
#include <list>
#include <set>
#include "./AIAlgorithm/AIAlgorithmManage.h"
#include "./AIAlgorithm/AIAlgorithmCommom.h"

#define DEBUGLOG(format,...)\
{\
	if( nullptr != pDesk && nullptr != pDesk->m_pDataManage)\
	{\
		debuglog(pDesk->m_pDataManage->m_InitData.uRoomID,pDesk->getRoomDeskFlag(),\
	format,##__VA_ARGS__);\
	}\
}

#define ERRORLOG(format,...)\
{\
	if( nullptr != pDesk && nullptr != pDesk->m_pDataManage)\
	{\
		errorlog(pDesk->m_pDataManage->m_InitData.uRoomID,pDesk->getRoomDeskFlag(),\
	format,##__VA_ARGS__);\
	}\
}

#define  RETURN_FALSE \
{\
	ERRORLOG("%s-%s:%d",__FILE__,__FUNCTION__,__LINE__);\
	return false;\
}

class CServerGameDesk;
class CAIAlgorithmManage;
namespace HN
{
	#define AIMANAGEINSTANCE _AIAlgorithmManage
	enum EAddGameMulType
	{
		eAddGameMulType_BaseMul = 1,		  ///基础倍率
		eAddGameMulType_MingPaiBegine,        ///明牌开始
		eAddGameMulType_MingPaiAfter,         ///明牌之后
		eAddGameMulType_CallNT,			 	  ///叫地主
		eAddGameMulType_RobNT,				  ///抢地主
		eAddGameMulType_Spring,				  ///春天
		eAddGameMulType_HardBomb,			  ///硬炸
		eAddGameMulType_SoftBomb,			  ///软炸
		eAddGameMulType_KingBomb,			  ///王炸
		eAddGameMulType_FLaiZiBomb,			  ///4张纯癞子炸弹
		eAddGameMulType_LongBomb,			  ///长炸弹 5张以上的炸弹
		eAddGameMulType_Back_OneKing,         ///底牌 单王
		eAddGameMulType_Back_ShunZi,		  ///底牌 顺子
		eAddGameMulType_Back_SameColor,		  ///底牌 同花
		eAddGameMulType_Back_TwoKing,		  ///底牌 双王
		eAddGameMulType_Back_SanZhang,		  ///底牌 三张
	};
	struct SGameMultiple
	{
		uint iBaseMul;				///基础倍率
		uint iMingPaiBegine;        ///明牌开始
		uint iMingPaiAfter;         ///明牌之后
		uint iCallNT;				///叫地主
		uint iRobNT;				///抢地主
		uint iSpring;				///春天
		uint iHardBomb;				///硬炸
		uint iSoftBomb;				///软炸
		uint iKingBomb;				///王炸
		uint iFLaiZiBomb;			///4张纯癞子炸弹
		uint iLongBomb;				///长炸弹 5张以上的炸弹

		uint iBack_OneKing;         ///底牌 单王
		uint iBack_ShunZi;			///底牌 顺子
		uint iBack_SameColor;		///底牌 同花
		uint iBack_TwoKing;			///底牌 双王
		uint iBack_SanZhang;		///底牌 三张
		SGameMultiple()
		{
			clear();
		}
		void clear()
		{
			iBaseMul =1;			 ///基础倍率
			iMingPaiBegine =1;       ///明牌开始
			iMingPaiAfter =1;        ///明牌之后
			iCallNT =1;				 ///叫地主
			iRobNT =1;				 ///抢地主
			iSpring =1;				 ///春天
			iHardBomb =1;		     ///硬炸
			iSoftBomb =1;			 ///软炸
			iKingBomb =1;			 ///王炸
			iFLaiZiBomb =1;			 ///4张纯癞子炸弹
			iLongBomb =1;			 ///长炸弹 5张以上的炸弹

			iBack_OneKing =1;        ///底牌 单王
			iBack_ShunZi =1;		 ///底牌 顺子
			iBack_SameColor =1;		 ///底牌 同花
			iBack_TwoKing =1;		 ///底牌 双王
			iBack_SanZhang =1;		 ///底牌 三张
		}
	};

	class DataManage
	{
	public:
		DataManage();
		~DataManage();
		typedef unsigned int UID;
		typedef unsigned char byte;

		enum E_RECORD
		{
			E_RESULT_DRAW = 0,
			E_RESULT_WIN  = 1,
			E_RESULT_LOST = 2,
		};
		struct sBaseUserInf
		{
			//ID 号码
			UID			  userID;		
			//金币
			__int64		  iMoney;		
			//登录名
			char		  sName[61];										
			//是否是旁观
			bool		  bWatcher;
			//是否离线
			bool		  bOffline;
			//是否托管
			bool		  bTrustee;
			//是否是机器人
			bool		  bRobot;
			//是否出完牌
			bool		  bCardOver;

			sBaseUserInf()
			{	
				userID			= 0;
				iMoney			= 0;
				memset(sName,0,sizeof(sName));
				bWatcher		= false;
				bOffline		= false;
				bTrustee		= false;
				bCardOver		= false;
				bRobot			= false;
			}
		};

		struct sSJUserInf:sBaseUserInf
		{
			//玩家倍率 0 不加倍 1加1倍
			int			  iRate;	
			//结束得分
			__int64		  iScore; 
			//炸弹数
			int iBombCount;			   /// 炸弹数
			//手牌
			CardArrayBase cHand;
			//已经出的牌 
			vector<T_C2S_PLAY_CARD_REQ> vHasOut;
			//状态
			byte bRobNTState;		   //玩家抢地主状态 (255:未操作 0：不叫，1：叫地主，2：不抢，3：抢地主)
			byte bCallScoreState;	   //玩家叫分状态 (255:未操作 0:不叫 1~3：叫分 )
			byte bAddRateState;		   //加倍状态	  (255:未操作 0:不加 1：加倍 )
			bool bIsPassState;         //是否不出
			bool bIsMingState;		   //是否明牌
			//////////////////////////////////
			//累计结束得分
			__int64		  iTotalScore;
			//战绩
			vector<E_RECORD>	 vRecord;
			//炸弹数
			int iTotalBombCount;			   /// 炸弹数
			//单局最高赢钱
			__int64		  iMaxWinMoney;        /// 单局最大赢钱数
			sSJUserInf()
			{
				iRate           = 0;
				iScore			= 0;
				iBombCount      = 0;
				iTotalScore		= 0;
				iTotalBombCount      = 0;
				iMaxWinMoney    = 0;
				cHand.Clear();
				vHasOut.clear();
				
				bRobNTState  = BYTE_ERR;	   //玩家抢地主状态 (255:未操作 0：不叫，1：叫地主，2：不抢，3：抢地主)
				bCallScoreState = BYTE_ERR;	   //玩家叫分状态 (255:未操作 0:不叫 1~3：叫分 )
				bAddRateState = BYTE_ERR;	   //加倍状态	  (255:未操作 0:不加 1：加倍 )
				bIsPassState = false;          //是否不出
				bIsMingState = false;	       //是否明牌
			}
		};

		/*
		*@brief 判断是否是正常游戏玩家
		*@param byte bSeatNO
		*@return bool
		*/
		bool isNomalUser(byte bSeatNO)const;

		/*
		*@brief 判断是否是旁观玩家
		*@param byte bSeatNO
		*@return bool
		*/
		bool isWatcher(byte bSeatNO);

		/*
		*@brief 判断是否托管
		*@param byte bSeatNO
		*@return bool
		*/
		bool isTrustee(byte bSeatNO);
		/*
		*@brief 判断是否是机器人
		*@param byte bSeatNO
		*@return bool
		*/
		bool isRobot(byte bSeatNO);
		/*
		*@brief 下一个玩家
		*@param byte bSeatNO
		*@return byte
		*/
		byte nextUser(byte bSeatNO);

		/*
		*@brief 下一个可以操作的玩家
		*@param byte bSeatNO
		*@return byte
		*/
		byte nextActiveUser(byte bSeatNO);
		/*
		*@brief 添加玩家
		*@param byte bSeatNO
		*@return bool
		*/
		bool addUser(byte bSeatNO);

		/*
		*@brief 更新玩家信息
		*@return bool
		*/
		bool updateUser();
		/*
		*@brief 删除玩家
		*@param byte bSeatNO
		*@return bool
		*/
		bool eraseUser(byte bSeatNO);

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
		*@brief 检测玩家出牌
		*@param byte bSeatNO
		*@param T_C2S_PLAY_CARD_REQ & sPlayCardREQ
		*@return bool
		*/
		bool checkPlayCard(byte bSeatNO,T_C2S_PLAY_CARD_REQ & sPlayCardREQ);

		/*
		*@brief 是否有王炸
		*@param byte bSeatNO
		*@return bool
		*/
		bool isHaveWBomb(byte bSeatNO);

		/*
		*@brief 玩家出牌数据处理
		*@param byte bSeatNO
		*@param  T_C2S_PLAY_CARD_REQ & sPlayCardREQ
		*@return bool
		*/
		bool playCard(byte bSeatNO,T_C2S_PLAY_CARD_REQ & sPlayCardREQ);

		/*
		*@brief 上一个出牌玩家
		*@return byte
		*/
		byte getLastOutUser();
		void setLastOutUser(byte bUser);

		/*
		*@brief 上一个操作玩家
		*@return byte
		*/
		byte getLastActionUser();
		void setLastActionUser(byte bSeatNO);

		/*
		*@brief 当前应该操作的玩家
		*@return byte
		*/
		byte  getCurUser();
		void  setCurUser(byte bSeatNO);
		/*
		*@brief 地主
		*@param byte bSeatNO
		*@return bool
		*/
		bool  setBanker(byte bSeatNO);
		byte  getBanker();

		/*
		*@brief 获取底牌
		*@param byte * pCard
		*@param size_t & sLen
		*@return bool
		*/
		bool getBackCard(byte *pCard,size_t& sLen);
		EBackArrayType getBackCardType();

		/*
		*@brief 添加底牌进手牌中
		*@param byte bSeatNO
		*@return bool
		*/
		bool AddBackCard(byte bSeatNO);
		/*
		*@brief 确定癞子后 重新排序手牌
		*@param 
		*@return bool
		*/
		bool SortCardByLaiZi();
		/*
		*@brief 低分
		*@param int iBackScore
		*@return bool
		*/
		bool setBackScore(int iBackScore);
		int  getBackScore();
		
		/*
		*@brief 玩家叫分状态
		*@param byte bSeatNo int iCallScore
		*@return bool
		*/
		bool  setCallScore(byte bSeatNO,int iCallScore);
		int   getCallScore(byte bSeatNO);
		bool  getAllCallScoreState(byte *pCallScoreState);

		/*
		*@brief 玩家抢地主状态
		*@param byte bSeatNo int iCallScore
		*@return bool
		*/
		bool  setRobNT(byte bSeatNO,uint iRobNTValue);
		bool  getAllRobNTState(byte *pRobNTState);

		/*
		*@brief 倍率
		*@param SGameMultiple & tGameMul
		*@return bool
		*/
		bool  setGameRate(const SGameMultiple & tGameMul);
		bool  addGameRate(int iGameBaseMul,EAddGameMulType eAddGameMulType = eAddGameMulType_BaseMul);
		/*
		*@brief 获取倍率
		*@param SGameMultiple & tGameMul 具体倍率 
		*@return int  总倍率
		*/
		int  getGameRate(SGameMultiple & tGameMul);
		int  getGameRate();
		
		/*
		*@brief 增加炸弹数
		*@param int iGameRate
		*@return bool
		*/
		bool  AddBombCount(byte bSeatNO,int iBombCount);

		/*
		*@brief 用户倍数
		*@param byte bSeatNO
		*@return bool
		*/
		bool  setUserRate(byte bSeatNO,int iUserRate);
		int   getUserRate(byte bSeatNO);
		bool  getAllUserRateState(byte *bAddRateState);
		bool  getUserAllRate(uint *bUserRate);
		/*
		*@brief 开始叫分位置
		*@param byte bSeatNO
		*@return bool
		*/
		bool  setFirstCaller(byte bSeatNO);
		byte  getFirstCaller();

		/*
		*@brief 设置玩家过牌状态
		*@param byte bSeatNO
		*@return bool
		*/
		bool setPassState(byte bSeatNO);
		bool clearAllPassState();
		bool getAllPassState(bool *pPass);

		/*
		*@brief 设置玩家是否明牌
		*@param byte bSeatNO
		*@return bool
		*/
		bool setMingCardState(byte bSeatNO,bool bIsMing);
		bool getAllMingCardState(bool *pMingCard);
		bool clearAllMingCardState();

		/*
		*@brief 设置天癞子牌
		*@param byte byCard
		*@return bool
		*/
		bool SetTianLaiZiCard(byte byCard);
		byte GetTianLaiZiCard();
		/*
		*@brief 设置地癞子牌
		*@param byte byCard
		*@return bool
		*/
		bool SetDiLaiZiCard(byte byCard);
		byte GetDiLaiZiCard();

		/*
		*@brief 修改玩家信息
		*@param byte bSeatNO
		*@param sSJUserInf & inf
		*@return bool
		*/
		bool alterUserInfo(byte bSeatNO,sSJUserInf &inf);
		bool getUserInfo(byte bSeatNO,sSJUserInf &inf);

		size_t UserCount();
		/*
		*@brief 获取玩家手牌
		*@param byte bSeatNO
		*@param byte * pCard
		*@param size_t & sLen
		*@return bool
		*/
		bool getUserHandCard(byte bSeatNO,byte *pCard,size_t& sLen);
		/*
		*@brief 获取所有玩家手牌
		*@param byte bSeatNO
		*@param byte *pCard[ONE_HAND_CARD_COUNT]
		*@param uint *pLen
		*@return bool
		*/
		bool getAllUserHandCard(byte bSeatNO,byte pCard[][ONE_HAND_CARD_COUNT],uint *pLen);
		bool getAllUserHandCard(byte pCard[][ONE_HAND_CARD_COUNT],uint *pLen);

		size_t getUserHandCardNum(byte bSeatNO);

		/*
		*@brief 洗牌分配牌
		*@return bool
		*/
		bool shuffleCard();
		/*
		*@brief 按固定的牌分配牌
		*@param const CardArrayBase & tCardArrayBase 手牌
		*@param uint iNumber = 1                      每次洗牌以所少张为一组 
		*@param uint iFillMaxBombNumber = 0           炸弹填补最大个数
		*@return bool
		*/
		bool shuffleCard(const CardArrayBase & tCardArrayBase,uint iNumber =1,uint iFillMaxBombNumber=0);

		/*
		*@brief 第一个出牌
		*@param byte bSeatNO
		*@param T_C2S_PLAY_CARD_REQ & tPlayCardREQ
		*@return bool
		*/
		bool autoPickUpFirst(byte bSeatNO,T_C2S_PLAY_CARD_REQ & tPlayCardREQ);

		/*
		*@brief 跟牌
		*@param byte bSeatNO
		*@param T_C2S_PLAY_CARD_REQ & tFollowCardREQ 跟牌结果
		*@return bool
		*/
		bool autoPickUpFollow(byte bSeatNO,T_C2S_PLAY_CARD_REQ & tFollowCardREQ);

		/*
		*@brief 获取自动出牌最优组合 iFlag 0 1:手数的大小和权值 排序 2:从右到左一手牌
		*@param byte bSeatNO
		*@return bool
		*/
		bool getPickUpOutAll(byte bSeatNO,std::vector<T_C2S_PLAY_CARD_REQ> & tPlayCardList,int iFlag=0);

		/*
		*@brief 获取跟牌所有组合 iFlag 0:组合按癞子数的大小排序  1:按打出这个组合后 手数的大小和权值 排序 2:从小到大
		*@param byte bSeatNO
		*@return bool
		*/
		bool getPickUpFollowAll(byte bSeatNO,std::vector<T_C2S_PLAY_CARD_REQ> & tFolllowCardList,int iFlag=0);

		/*
		*@brief 获取玩家是否能跟
		*@param byte bSeatNO
		*@return bool
		*/
		bool bCheckCanOut(byte bSeatNO);

		/*
		*@brief 识别牌类型 获取出 含有癞子出牌 可以组成多种牌型的列表 如: 癞子3  33334  可以组成 45678 34567 44433等等牌型
		*@param byte bSeatNO T_C2S_PLAY_CARD_REQ & tPlayCard std::vector<T_C2S_PLAY_CARD_REQ> & tCardTypeList
		*@return bool
		*/
		bool getCardTypeByPlayCard(byte bSeatNO,T_C2S_PLAY_CARD_REQ & tPlayCard,std::vector<T_C2S_PLAY_CARD_REQ> & tCardTypeList);
		/*
		*@brief 配牌器
		*@return void
		*/
		void testSetUserCard();

		/*
		*@brief 获取所有打出去的牌
		*@return CardArrayBase
		*/
		CardArrayBase & getAllHasOutCard();

		/*
		*@brief 获取所有打出去的牌
		*@return CardArrayBase
		*/
		HN_AI::CAIAlgorithmManage & AIAlgorithmManageInstance();
	private:
		//玩家表
		map<byte,sSJUserInf> _userData;
		//上一个出牌玩家
		byte _lastOutUser;
		//上一个操作玩家
		byte _lastActionUser;
		//当前可以操作的玩家
		byte _curUser;
		//牌堆
		CardArrayBase _cardHeap;
		CServerGameDesk *pDesk;
		CardArrayBase _backCardArray;			  //底牌数据
		CardBase _tianLaiZiCard;			      //天癞子牌
		CardBase _diLaiZiCard;                    //地癞子牌
		int _iBackScore;						  //底分
		SGameMultiple _sGameMul;                  //当前公共游戏倍率
		byte _banker;							  //地主
		byte _bFirstCaller;						  //开始操作位置
		//已经打出的牌 
		CardArrayBase _vAllHasOut;				  //所有打出去的牌	
		//算法
		HN_AI::CAIAlgorithmManage _AIAlgorithmManage;	  //算法句柄
	private:
		void LogCharToChar(const vector<CardBase> & src,string & dst);

	public:
		//检测组牌，可根据需求添加不同的iType
		bool CheckType(CardArrayBase& HandCard, int iType);
		//跟牌算法
		bool getPickUpFollowAll(HN::CardArrayBase &HandCard, T_C2S_PLAY_CARD_REQ &tLastCard, std::vector<T_C2S_PLAY_CARD_REQ> &tPlayCardList);
		bool getPickUpOutAll(HN::CardArrayBase &HandCard, std::vector<T_C2S_PLAY_CARD_REQ> & tPlayCardList);
		//设置规则
		void SetCardRulesConfig(const HN_AI::SSysConfig & tSysConfig);

	};
}
#endif // DATAMANAGE_H