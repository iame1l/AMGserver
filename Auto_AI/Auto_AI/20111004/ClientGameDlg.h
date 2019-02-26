#pragma once

#include "StdAfx.h"
#include "Resource.h"
#include "UpgradeMessage.h"
#include "UpgradeMessageEx.h"
//#include "UserDataEx.h"
//#include "WzCmdGameLogic.h"
//#include "GameDataEx.h"


//控制消息定义
#define CM_USER_STATE					1						//	状态改变
#define IDM_GETPOINT						WM_USER + 430		//	积分
#define IDM_GET_ROOMNAME					WM_USER+435			//	得到房间名称

////动作代码
//#define ACT_USER_UP						1						//用户起来
//#define ACT_WATCH_UP					2						//旁观起来
//#define ACT_USER_SIT					3						//用户坐下
//#define ACT_WATCH_SIT					4						//旁观坐下
//#define ACT_USER_CUT					5						//用户断线
//#define ACT_USER_AGREE					6						//用户同意
//#define ACT_GAME_BEGIN					7						//游戏开始
//#define ACT_GAME_END					8						//游戏结束
//#define ACT_USER_POINT					9						//用户经验值

//////////////自定义宏///////////////////////////////////////////////////////////////////////////////

#define ACTION_TRIPLET_TING				0x08	// 碰听
#define ACTION_COLLECT_TING				0x09	// 吃听牌
#define MJ_STATE_SELECT_COLLECT_TING	    5	// 吃听牌选择
#define MJ_STATE_SELECT_DAO_PAI				6	// 倒牌选择


#define TIME_BEGIN				101      // 自动开始
#define TIME_OUT_CARD			102
#define TIME_BLOCK_ME			103
#define TIME_BLOCK_CARD			104

#define TIME_WAIT_BLOCK_ME			105 //等待拦牌
#define TIME_WAIT_BLOCK_CARD		106 //等待拦别人的牌

#define TIME_OPERATE_HU				107
#define TIME_OPERATE_GANG			108
#define TIME_OPERATE_PENG			109
#define TIME_OPERATE_CHI			110

#define FLAG_Collect		0x01	 //吃标志位
#define FLAG_Triplet		0x02	 //碰标志位
#define FLAG_Quadruplet		0x04	 //杠标志位
#define FLAG_Ting			0x08	 //听标志位
#define FLAG_Win			0x10	 //胡标志位

class UserData
{
public:
	UserData(void);
	~UserData(void);

public:	
	/// 门牌数据
	MjMenPaiData    m_MenPai;      
	//	手牌不超过HAND_CARD_NUM张，0为结束标志	DWJ
	BYTE	m_byArHandPai[PLAY_COUNT][HAND_CARD_NUM];
	//	手牌张数	DWJ
	BYTE	m_byArHandPaiCount[PLAY_COUNT];
	///	门牌不超过40张	DWJ
	BYTE	m_iArMenPai[PLAY_COUNT][MEN_CARD_NUM];
	///	花牌不超过20张，0为结束标志
	BYTE	m_byArHuaPai[PLAY_COUNT][HUA_CARD_NUM];
	///	出牌不超过160张，0为结束标志	
	BYTE	m_byArOutPai[PLAY_COUNT][OUT_CARD_NUM];
	//	出牌的张数
	BYTE	m_byArOutPaiCount[PLAY_COUNT];	
	///	当前出牌位置	DWJ
	BYTE	m_byNowOutStation;
	//	记录玩家是否出过牌了(用于检测天胡地胡)
	bool	m_bHaveOut[PLAY_COUNT];
	//	吃碰杠
	GCPStructEx	m_UserGCPData[PLAY_COUNT][5];
	//	玩家是否听牌
	bool	m_bTing[PLAY_COUNT];
	//	听牌类型(明楼 还是暗楼--山东麻将特有)
	BYTE	m_byTingType[PLAY_COUNT];
	//玩家是否托管
	bool	m_bTuoGuan[PLAY_COUNT];










	///转换后的门牌数据
	BYTE m_byMenPaiData[4][MEN_CARD_NUM/2][2];

	///关系数据
	BYTE		m_byGuanXi[4][4];

	///是否可用的牌
	bool		m_bEnableCard[PLAY_COUNT][HAND_CARD_NUM]; 


	///当前玩家的逻辑位置（客户端使用）
	BYTE        m_byStation;
	///最后打出的牌
	BYTE        m_byLastOutPai;
	///最后抓到的牌
	BYTE		m_byLastZhuaPai[4];
	///当前玩家最后抓拍
	BYTE        m_byMeZhuaPai;

	///当前抓牌方向，true 顺时针，false 逆时针	DWJ
	bool		m_bZhuaPaiDir;
	///上次出牌位置
	BYTE		m_byLastOutStation;
	///包牌玩家
	BYTE        m_byBaoPaiUser;
	///当前其他玩家打出的牌
	BYTE		m_byOtherOutPai;
	///停牌或其他操作后能打出的牌
	BYTE       m_byCanOutCard[4][HAND_CARD_NUM];
	///游戏已经进行的局数（强退，安全退出，解散等重新开始计算）
	BYTE		byPlayCount;
	///圈风
	BYTE		m_byQuanFeng;
	///门风
	BYTE		m_byMenFeng[4];

	//玩家是否返回了色子结束消息
	bool		m_bSiziFinish[4];
	//是否处理等待色子动画结束状态
	bool		m_bWaitSezi;
	///玩家是否已经断线
	bool        m_bFoceLeave[4];
	///玩家离开的数量
	BYTE		m_byFoceLeavCount;

	///当前玩家是否听牌
	bool		m_bisTing;

	///玩家是否赢家
	bool		m_bWinner[4];
	///是否一炮多响
	bool		m_bIsDuoXiang;
	//是否我出牌
	bool		m_bMeOutPai;


	///当前选中排索引
	BYTE        m_bySelectIndex;
	///精牌数据
	JingStruct  m_StrJing;
	//同意玩家离开的人数
	BYTE		m_byAgreeLeaveNum;


	///控件变量
	bool                m_bIsBoy[4];

	///发牌数据
	SuperStruct			m_SetData[4];

	///设置下一张摸牌（测试使用）
	BYTE				m_bySetMoPai[4];

	BYTE				m_byApplyMaxAction;//玩家当前申请的最大事件
	BYTE				m_byCanDoMaxAction;//本次检查玩家能做的最大动作
public:

	//////////////////手牌操作/////////////////////////////////////

	///检查是否存在指定的手牌
	virtual bool IsHaveAHandPai(BYTE station,BYTE pai);
	///检查手牌中某张牌的个数
	virtual int GetAHandPaiCount(BYTE station,BYTE pai);
	///检查手牌的张数
	virtual int GetHandPaiCount(BYTE station);
	///拷贝玩家的手牌
	virtual int CopyHandPai(BYTE pai[][HAND_CARD_NUM],BYTE station,bool show);
	///描述：拷贝某个玩家的手牌
	virtual int CopyOneUserHandPai(BYTE pai[HAND_CARD_NUM] ,BYTE station);
	///手牌排序，small是否从小到大排
	virtual void SortHandPai(BYTE station,bool big);
	///添加一张牌到手牌中
	virtual void AddToHandPai(BYTE station,BYTE pai);
	///设置手牌数据
	virtual void SetHandPaiData(BYTE station,BYTE pai[],BYTE byCount);
	///删除一张指定的手牌
	virtual void DelleteAHandPai(BYTE station,BYTE pai);
	///设置手牌牌背
	virtual void SetHandPaiBack(BYTE station,BYTE num);
	///获得最后一张手牌
	virtual BYTE GetLastHandPai(BYTE station);
	///获得选中的一张手牌
	virtual BYTE GetSelectHandPai(BYTE station);
	///是否出牌玩家
	virtual bool IsOutPaiPeople(BYTE station);
	///检测手牌中是否存在花牌
	virtual bool IsHuaPaiInHand(BYTE station);
	///将手牌中的花牌移到花牌数组中
	virtual int MoveHuaPaiFormHand(BYTE station);
	///通过索引获取一张手牌
	virtual BYTE GetHandPaiFromIndex(BYTE station,int index);
	///通过索引设置一张手牌
	virtual bool SetHandPaiFromIndex(BYTE station,int index,BYTE pai);
	///索引是否有牌
	virtual bool IsHaveHandPaiFromIndex(BYTE station,int index);
	//////////////////吃碰杠牌操作/////////////////////////////////////

	///获取玩家杠牌的个数(暗杠，明杠，补杠)
	virtual int GetUserGangNum(BYTE station,BYTE &an,BYTE &ming , BYTE &bu);
	///玩家吃牌次数
	virtual int GetUserChiNum(BYTE station);
	///玩家杠牌次数
	virtual int GetUserPengNum(BYTE station);
	///玩家杠了某个牌
	virtual bool IsUserHaveGangPai(BYTE station,BYTE pai,BYTE &type);
	///玩家是否碰了某张牌
	virtual bool IsUserHavePengPai(BYTE station,BYTE pai);
	///玩家是否吃过某张牌
	virtual bool IsUserHaveChiPai(BYTE station,BYTE pai);
	///添加一组数据到杠吃碰数组中
	virtual void AddToGCP(BYTE station,GCPStructEx *gcpData);
	///删除刚吃碰数组中指定的数据组
	virtual void DelleteAGCPData(BYTE station,BYTE type ,BYTE pai);
	///拷贝杠吃碰数组中的数据组
	virtual int CopyGCPData(BYTE station,GCPStructEx gcpData[]);
	///设置吃碰数组中的数据组
	virtual void SetGCPData(BYTE station,GCPStructEx gcpData[]);
	///获取杠吃碰的节点个数
	virtual int GetGCPCount(BYTE station);
	///描述：获取杠吃碰的中某种牌的数量
	virtual int GetOnePaiGCPCount(BYTE station,BYTE pai);
	///排序杠吃碰数组
	virtual void ShortGCP(BYTE station);
	///描述：初始化杠吃碰数组
	virtual void InitGCP(BYTE station);

	///////////////////出牌操作///////////////////////////////////////////////////////

	///添加一张牌到出牌数组中
	virtual void AddToOutPai(BYTE station,BYTE pai);
	///删除最后一张出牌
	virtual void DelleteLastOutPai(BYTE station);
	///设置出牌数据
	virtual void SetOutPaiData(BYTE station,BYTE pai[],BYTE byCount);
	///拷贝玩家的出牌
	virtual int CopyOutPai(BYTE station,BYTE pai[]);
	///获取玩家的出牌数量
	virtual int GetOutPaiCount(BYTE station);
	///描述：获取玩家某张牌的出牌数量
	virtual int GetOneOutPaiCount(BYTE station,BYTE pai);

	//////////////////花牌操作///////////////////////////////////////////////////////

	///添加一张花牌到花牌数组中
	virtual void AddToHuaPai(BYTE station,BYTE pai);
	///设置花牌数据
	virtual void SetHuaPaiData(BYTE station,BYTE pai[]);
	///拷贝玩家的花牌
	virtual int CopyHuaPai(BYTE station,BYTE pai[]);

	///检测是否花牌
	virtual bool CheckIsHuaPai(BYTE pai);

	///////////////////门牌牌操作///////////////////////////////////////////////////////

	///设置门牌数据
	virtual void SetMenPaiData(BYTE station,BYTE pai[]);
	///拷贝玩家的门牌
	virtual int CopyMenPai(BYTE station,BYTE pai[]);
	///描述：门牌转换
	virtual void ChangeMenPai(BYTE station);
	///描述：获取某玩家的门牌数量
	virtual int GetMenPaiCount(BYTE station);

	///////////////////财神操作//////////////////////////////////////////////
	/////检测pai是否财神
	//virtual bool CheckIsCaiShen(BYTE pai);
	/////清空财神结构
	//virtual void InitCaiShen();

	/////////////////超级客户端设置牌操作///////////////////////
	///换所有手牌
	virtual void ChangeAllHandPai(BYTE station,BYTE pai[],BYTE count);
	///换指定手牌
	virtual void ChangeSomeHandPai(BYTE station,BYTE pai[],BYTE handpai[],BYTE count);
	///设置自己的下一张牌
	virtual void SetMeNextPai(BYTE station,BYTE pai);
	///设置牌墙的下一张牌
	virtual void SetGameNextPai(BYTE pai);

	/////////////////////////////////////////////////////

	///获取牌数组的有效牌张数
	virtual int GetPaiNum(BYTE station,BYTE pai[],BYTE count);
	///初始化数据
	virtual void InitData();

};

class UserDataEx: public UserData
{
public:
	UserDataEx(void);
	~UserDataEx(void);

public:	

	bool    m_bIsHu[PLAY_COUNT];//是否胡牌


	BYTE	m_byCaiPiaoStation;//财飘位置，该值不为255是不允许吃碰杠和放炮（杭州麻将专用）
	BYTE    m_byGuanXi[PLAY_COUNT][PLAY_COUNT]; //吃碰杠关系
	bool	m_bGangKai[PLAY_COUNT];//杠开状态玩家
	BYTE    m_byLianZhuang;//连庄数
	bool    m_bTianHu;//庄家天糊状态
	bool	m_bOutJing[PLAY_COUNT];//玩家是否打出了财神牌
	bool	m_bChanPai;		//玩家是否铲牌

	bool   m_bLookBao[PLAY_COUNT];//看宝状态

	BYTE   m_byFirstHandPai[PLAY_COUNT][HAND_CARD_NUM];//第一手牌，出牌，吃碰杠后抓到的不算

	BYTE    m_byDingQue[PLAY_COUNT];//定的缺，0表万  1表条 2表筒

	int    m_byGFXY[PLAY_COUNT][PLAY_COUNT+1];   //刮风下雨所得
	//刮风（补杠） 向未胡者收1
	//下雨（暗杠） 向未胡者收2
	//下雨（直杠） 向引杠都收2

	int    m_byGengCount[PLAY_COUNT];//糊牌时根数



	bool	m_bIsGangPao[PLAY_COUNT];//正在杠后炮的状态
	bool    m_bQiHu[PLAY_COUNT];//玩家是否弃糊状态

	BYTE    m_NoFenGang[PLAY_COUNT][4];//记录无分杠（补杠，手中有4张牌先碰后杠无分，抓牌后能补杠但是不在当前圈杠无分）

	//补杠得分的牌
	BYTE    m_byBuGangPai;

	//记录每个杠牌
	GangFenRecordEx m_stGangFen[PLAY_COUNT];

public:

	//////////////////手牌操作/////////////////////////////////////

	///检查是否存在指定的手牌
	virtual bool IsHaveAHandPai(BYTE station,BYTE pai);
	///检查手牌中某张牌的个数
	virtual int GetAHandPaiCount(BYTE station,BYTE pai);
	///检查某种花色牌的个数（0万 1筒 2条)
	int GetAKindPaiCount(BYTE station,BYTE kind);
	///得到某种花色牌（0万 1筒 2条)
	int GetAKindPai(BYTE station,BYTE kind);
	///检查手牌的张数
	virtual int GetHandPaiCount(BYTE station);
	///拷贝玩家的手牌
	virtual int CopyHandPai(BYTE pai[][HAND_CARD_NUM],BYTE station,bool show);
	///描述：拷贝某个玩家的手牌
	virtual int CopyOneUserHandPai(BYTE pai[HAND_CARD_NUM] ,BYTE station);
	///手牌排序，small是否从小到大排
	virtual void SortHandPai(BYTE station,bool big);
	///添加一张牌到手牌中
	virtual void AddToHandPai(BYTE station,BYTE pai);
	///设置手牌数据
	virtual void SetHandPaiData(BYTE station,BYTE pai[], BYTE byCount);
	///删除一张指定的手牌
	virtual void DelleteAHandPai(BYTE station,BYTE pai);
	///设置手牌牌背
	virtual void SetHandPaiBack(BYTE station,BYTE num);
	///获得最后一张手牌
	virtual BYTE GetLastHandPai(BYTE station);
	///获得选中的一张手牌
	virtual BYTE GetSelectHandPai(BYTE station);
	///是否出牌玩家
	virtual bool IsOutPaiPeople(BYTE station);
	///检测手牌中是否存在花牌
	virtual bool IsHuaPaiInHand(BYTE station);
	///将手牌中的花牌移到花牌数组中
	virtual int MoveHuaPaiFormHand(BYTE station);

	///获取手牌的牌色数
	virtual int GetPaiSeCount(BYTE station);
	///是否还有缺门的牌
	bool IsHaveQueMen(BYTE station,BYTE type);
	//获取一张data中没有的牌，用来换牌
	BYTE GetChangePai(BYTE station,BYTE data[]);

	//////////////////吃碰杠牌操作/////////////////////////////////////

	///获取玩家杠牌的个数(暗杠，明杠，补杠)
	virtual int GetUserGangNum(BYTE station,BYTE &an,BYTE &ming , BYTE &bu);
	///玩家吃牌次数
	virtual int GetUserChiNum(BYTE station);
	///玩家杠牌次数
	virtual int GetUserPengNum(BYTE station);
	///玩家杠了某个牌
	virtual bool IsUserHaveGangPai(BYTE station,BYTE pai,BYTE &type);
	///玩家是否碰了某张牌
	virtual bool IsUserHavePengPai(BYTE station,BYTE pai);
	///玩家是否吃过某张牌
	virtual bool IsUserHaveChiPai(BYTE station,BYTE pai);
	///添加一组数据到杠吃碰数组中
	virtual void AddToGCP(BYTE station,GCPStructEx *gcpData);
	///删除刚吃碰数组中指定的数据组
	virtual void DelleteAGCPData(BYTE station,BYTE type ,BYTE pai);
	///拷贝杠吃碰数组中的数据组
	virtual int CopyGCPData(BYTE station,GCPStructEx gcpData[]);
	///设置吃碰数组中的数据组
	virtual void SetGCPData(BYTE station,GCPStructEx gcpData[]);
	///获取杠吃碰的节点个数
	virtual int GetGCPCount(BYTE station);
	///描述：获取杠吃碰的中某种牌的数量
	virtual int GetOnePaiGCPCount(BYTE station,BYTE pai);
	///排序杠吃碰数组
	virtual void ShortGCP(BYTE station);
	///描述：初始化杠吃碰数组
	virtual void InitGCP(BYTE station);


	///////////////////出牌操作///////////////////////////////////////////////////////

	///添加一张牌到出牌数组中
	virtual void AddToOutPai(BYTE station,BYTE pai);
	///删除最后一张出牌
	virtual void DelleteLastOutPai(BYTE station);
	///设置出牌数据
	virtual void SetOutPaiData(BYTE station,BYTE pai[],BYTE byCount);
	///拷贝玩家的出牌
	virtual int CopyOutPai(BYTE station,BYTE pai[]);
	///获取玩家的出牌数量
	virtual int GetOutPaiCount(BYTE station);
	///描述：获取玩家某张牌的出牌数量
	virtual int GetOneOutPaiCount(BYTE station,BYTE pai);

	//////////////////花牌操作///////////////////////////////////////////////////////

	///添加一张花牌到花牌数组中
	virtual void AddToHuaPai(BYTE station,BYTE pai);
	///设置花牌数据
	virtual void SetHuaPaiData(BYTE station,BYTE pai[]);
	///拷贝玩家的花牌
	virtual int CopyHuaPai(BYTE station,BYTE pai[]);

	///检测是否花牌
	virtual bool CheckIsHuaPai(BYTE pai);

	///////////////////门牌牌操作///////////////////////////////////////////////////////

	///设置门牌数据
	virtual void SetMenPaiData(BYTE station,BYTE pai[]);
	///拷贝玩家的门牌
	virtual int CopyMenPai(BYTE station,BYTE pai[]);

	/////////////////////////////////////////////////////

	///获取牌数组的有效牌张数
	virtual int GetPaiNum(BYTE station,BYTE pai[],BYTE count);

	///初始化数据
	virtual void InitData();

	///描述：玩家是否存在某种牌的大杠
	bool IsUserHaveBigGangPai(BYTE station,BYTE pai);

	///描述：将抓牌索引转换成抓牌玩家的门牌索引
	bool ZhuaPaiIndex(BYTE &index,BYTE &dir);		

};

class GameData
{
public:
	GameData(void);
	~GameData(void);

public:	   

	BYTE					m_byNtStation;	//	庄家位置	DWJ

	int						m_iHuangZhuangCount;	//荒庄次数	DWJ

	BYTE					ApplyThingID;	///要申请的事件	DWJ

	BYTE					m_byThingDoing;	///正在发生的事件号  

	BYTE					m_byWatingThing;//当前等待的事件ID	DWJ

	MjRuleSet				m_mjRule;		/// 麻将常规属性	DWJ

	bool					m_bGangState[PLAY_COUNT];//记录杠牌状态，拥于杠开和杠后炮

	bool					m_bShowTax;//是否显示台费

	int						m_iRemaindTime;	//倒计时剩余时间(客户端用)




	/// 麻将动作属性
	ActionRuleSet			m_mjAction;               
	/// 麻将算分属性
	CountFenRuleSet			m_mjFenRule;				
	/// 麻将糊牌属性
	HuPaiRuleSet			m_mjHuRule;				
	/// 麻将糊牌类型
	HuPaiTypeSet			m_mjHuType;				
	///记录游戏所有的流程事件（抓牌，出牌，吃碰杠糊，补花等，最后一个元素299记录事件总数）
	BYTE					m_byThingRecord[300];

	/// 每个事件的后接事件（最大100个事件）
	BYTE					m_byThingNext[100];



	////////////基本数据定义/////////////////////////////////////////////////////////



	///待执行事件
	BYTE					m_byNextAction;

	///房间的倍率
	int						basepoint;	



	///吃牌牌事件,临时数据
	tagChiPaiEx				temp_ChiPai;
	///碰牌牌事件,临时数据
	tagPengPaiEx			temp_PengPai;
	///杠牌牌事件,临时数据
	tagGangPaiEx			temp_GangPai;
	////糊牌牌事件,临时数据


public:////////////事件数据对象定义//////////////////////
	///游戏开始事件	DWJ
	tagBeginEx				T_Begin;
	///以东为庄事件	DWJ
	tagDongNtEx				T_DongNt;
	///掷2颗色子确定起牌位置事件	DWJ
	tagTwoSeziDirEx			T_TwoSeziDir;
	///掷2颗色子确定起牌位置（点数和）和起牌点（最小点）事件	DWJ
	tagTwoSeziDirAndGetPaiEx	T_TwoSeziDirAndGetPai;
	///发牌事件	DWJ
	tagSendPaiEx			T_SendPai;
	///所有玩家补花事件	DWJ
	tagAllBuHuaEx			T_AllBuHua;
	///开始出牌通知事件	DWJ
	tagBeginOutPaiEx		T_BeginOutPai;
	///出牌事件	DWJ
	tagOutPaiEx				T_OutPai;
	///吃碰杠糊牌通知事件牌事件	DWJ
	tagCPGNotifyEx			T_CPGNotify[PLAY_COUNT];
	///抓牌牌事件	DWJ	
	tagZhuaPaiEx			T_ZhuaPai;
	///单个玩家补花事件	DWJ
	tagOneBuHuaEx			T_OneBuHua;



	///吃牌牌事件	DWJ
	tagChiPaiEx				T_ChiPai;
	///碰牌牌事件	DWJ
	tagPengPaiEx			T_PengPai;
	///杠牌牌事件	DWJ
	tagGangPaiEx			T_GangPai;
	///听牌牌事件	DWJ
	tagTingPaiEx			T_TingPai;
	///糊牌牌事件	DWJ
	tagHuPaiEx				T_HuPai;
	///算分事件		DWJ
	tagCountFenEx			T_CountFen;













	///掷2颗色子的点数和为庄事件
	tagTwoSeziNtEx			T_TwoSeziNt;
	///掷2颗色子确定庄家和起牌点位置事件
	tagTwoSeziNtAndGetPaiEx	T_TwoSeziNtAndGetPai;
	///轮流掷2颗色子确定庄家
	tagTurnSeziNtEx			T_TurnSeziNt;


	///掷2颗色子确定起牌敦数事件
	tagTwoSeziGetPaiEx		T_TwoSeziGetPai;

	///跳牌事件
	tagTiaoPaiEx			T_TiaoPai;
	///掷色子2颗色子定精牌事件
	tagTwoSeziJingEx		T_TwoSeziJing;
	///掷色子1颗色子定金牌事件
	tagOneSeziJinEx			T_OneSeziJin;




	///游戏结束处理事件
	tagEndHandleEx			T_EndHandle;
	///非正常结束处理事件
	tagUnnormalEndHandleEx   T_UnnormalEndHandle;




public:

	///设置所有事件的后接事件
	virtual void SetThingNext();
	///设置某个事件的后接事件
	virtual void SetThingNext(BYTE id);
	//virtual 初始化数据
	virtual void InitData();
	///初始化数据
	virtual void LoadIni();

};

typedef struct tagZhongNiao
{
	BYTE byMenPai[PLAY_COUNT][MEN_CARD_NUM];//门牌数据
	int byCard[4];//中鸟数据
	tagZhongNiao()
	{
		clear();
	}
	void clear()
	{
		memset(this, 255 ,sizeof(tagZhongNiao));
	}
}ZhongNiao;

///游戏数据管理类
class GameDataEx: public GameData
{

public:
	GameDataEx(void);
	~GameDataEx(void);
public:
	///麻将时间等待：时间到了服务器代为处理
	MJ_WaitTimeEx   m_MjWait;
	ZhongNiao m_ZhongNiao;
public:
	///设置所有事件的后接事件
	virtual void SetThingNext();
	///设置某个事件的后接事件
	virtual void SetThingNext(BYTE id);
	//virtual 初始化数据
	virtual void InitData();
	///初始化数据
	virtual void LoadIni();

};

//游戏框架类 
class CClientGameDlg : public CLoveSendClass//CGameFrameDlg
{

	//函数定义
public:
	//构造函数
	CClientGameDlg();
	//析构函数
	virtual ~CClientGameDlg();
	int GetGameStatus();

protected:
	//初始化函数
	virtual BOOL OnInitDialog();


private:	///< 以下为私有成员，不允许改为公有成员
    //CBaseRoom                  *m_pAIRoom;			//
	GameInfoStruct				* m_pGameInfo;					//游戏信息指针
	UserItemStruct				* m_pUserInfo[MAX_PEOPLE];		//游戏玩家信息

public:
	bool	m_bWatchMode;
	BYTE    m_byMyStation;                    /**< 自己的位置 */
	BYTE    m_byGameState;                    /**< 游戏状态 */
	BYTE	m_byOperationStation;				/**< 当前操作人 */

	//游戏数据
	GameDataEx m_GameData;	
	UserDataEx m_UserData;//玩家数据

	bool m_bUserAgree ;//玩家是否举手了

	// 配置文件
	int                     m_iBeginTime;               //准备时间
	int                     m_iOutCardTime;             //出牌时间
	int                     m_iBlockTime;               //拦牌时间

	
public:

	//获取自己信息
	//UserInfoStruct * GetMeUserInfo() { return &m_pGameInfo->pMeUserInfo->GameUserInfo; };






	//重载函数
public:
	//设置游戏状态
	virtual bool SetGameStation(void * pStationData, UINT uDataSize);
	//游戏消息处理函数
	virtual bool HandleGameMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket);
	//重新设置界面
	virtual void ResetGameFrame();
	//定时器消息
	virtual bool OnGameTimer(BYTE bDeskStation, UINT uTimeID, UINT uTimeCount);
	//清除所有定时器
	void KillAllTimer();
	//同意开始游戏 
	virtual bool OnControlHitBegin();
	//安全结束游戏
	virtual bool OnControlSafeFinish(){return false;};
	//能否离开桌子
	virtual BOOL CanLeftDesk();
	//桌号换算到视图位置
	virtual BYTE ViewStation(BYTE bDeskStation);
	//消息处理函数
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//退出函数
	virtual void OnCancel();


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	/**
	 *  @brief 收到通知拦牌玩家出牌消息
	 */	
	void OnReciveBlockOut(void * buffer, int nLen);	

//////////////////////////////////////////////////////////////////////////////////////

/// 负责处理接收的消息函数

	 //  @brief 房间基础信息
	void  OnNotifyBaseInfo(void * buffer, int nLen);

	//  @brief 接受到进入、重回、数据恢复消息
	void  OnNotifyGameInfo(void * buffer, int nLen);

	//  @brief 处理设置连局信息(游戏开始发送的的第一条游戏消息)
	void  OnNotifyRunNbr(void * buffer, int nLen);

	//  @brief 接受到通知庄家位置消息
	void  OnNotifyDealer(void * buffer, int nLen);

	//  @brief 接受到开局发牌消息后
	void  OnNotifyDeal(void * buffer, int nLen);

	// @brief 处理补花结束消息
	void OnNotifyFlowerOver(void * buffer, int nLen);

	//  @brief 接受到摸牌消息
	void  OnNotifyDraw(void * buffer, int nLen);

	//  @brief 接受到请求出牌结果消息
	void  OnNotifyGive(void * buffer, int nLen);

	//  @brief 接受到请求拦牌结果消息
	void  OnNotifyBlockResult(void * buffer, int nLen);

	//  @brief 接受到结束消息
	void  OnNotifyFinish(void * buffer, int nLen);

	//  @brief 接受到修改旁观状态消息
	void  OnNotifyChangeLookOn(void * buffer, int nLen);

	//  @brief 接受到刷新手牌消息
	void OnNotifyUpdateHandTiles(void * buffer, int nLen);


	// 接收到GM命令执行结果消息---
	void OnNotifyGMCmd(void * buffer, int nLen);
	// 接收提示信息---
	void OnNotifyShowDlg(void * buffer, int nLen);
	//  接收到在线玩家的信息---
	void serverOnlineUsersInfoNotify(void *buffer, int nLen);
	// 接收到精牌的信息---
	void serverJpInfoNotify(void *buffer, int nLen);
	// 接收到服务端买顶底的信息---
	void serverMddInfoNofity(void *buffer, int nLen);
	// 接收到服务端买顶底的信息----
	void serverMddTimeoutNotify();
	// 庄家买底的通知---
	void serverZmInfoNotify(void *buffer, int nLen);
	//获取连庄的消息---
	void serverLianInfoNotify(void *buffer, int nLen);



	//@brief 自定义初使化数据
	void InitData();


///////////////////////////////////////////////////////////////////////////////////////

	//机器人自动出牌
	void AutoOutCard();
	//获取最无用的牌
	BYTE FindUselessPai();
	//获取牌值
	int GetPaiValue(BYTE pai);
	//是非常重要的牌，或者不可或缺的，比如幺九牌。增加牌值，减少被打出的机会
	bool IsImport(BYTE pai);
	//机器人自动拦牌
	//void AutoBlockCard();
	//机器人自动拦牌（自摸胡牌）
	//void AutoBlockMe();
	//	游戏开始打牌了
	void	OnBeginOutCard(void *buffer, int nLen);
	//	抓牌	DWJ
	void	OnNotifyZhuaPai(void *buffer, int nLen);
	//	拦牌
	void	OnNotifyBlock(void *buffer, int nLen);
	void ReSetGameData();
	DECLARE_MESSAGE_MAP()

	afx_msg void OnTimer(UINT nIDEvent);
};
