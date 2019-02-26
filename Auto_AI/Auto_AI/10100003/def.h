#pragma once
#include "Card.h"
#include <iostream>

#ifndef OUT
#define OUT
#endif // OUT

#define CFG_BEGIN_TIME			"begintime"
#define CFG_THINK_TIME			"thinktime"
#define CFG_ROB_NT_TIME			"RobNtTime"
#define CFG_CALL_SCORE_TIME		"callscoretime"
#define CFG_ADD_DOUBLE_TIME		"adddoubletime"
#define CFG_ROOM_BASE_POINT		"roombasepoint"
#define CFG_RUN_PUBLISH			"runpublish"
#define CFG_CARD_SHAPE			"cardshape"
#define CFG_ROOM_RULE			"roomrule"
#define CFG_PLAYER_COUNT		"playercount"

typedef unsigned char					ucard;
typedef unsigned char					byte;
typedef unsigned int					uint;


#define INVALID_INDEX			(-1)
#define MAX_CARD_BUFFER			45

#define BYTE_ERR						255				//无符号字符类型初始化
#define ONE_HAND_CARD_COUNT				20				//手牌最大张数
#define BACK_CARD_COUNT					3				//底牌个数
#define CARD_PAIRS						1				//几副牌
#define MAX_CARD_COUNT					54*CARD_PAIRS	//牌的个数


#define GET_STRING(NUM)                 #NUM
#define INT_TO_STR(NUM)                 GET_STRING(NUM)
#define SKIN_FOLDER						TEXT(INT_TO_STR(NAME_ID))		// 客户端目录


enum ComboType
{
	Invalid							= 0,							// 无效
	Single							= 0x1,							// 单张
	Pair							= 0x2,							// 一对
	Triplet							= 0x4,							// 三张
	TripletAttachSingle				= (Triplet | Single),			// 三带一
	TripletAttachPair				= (Triplet | Pair),				// 三带一对
	Sequence						= 0x8,							// 顺子
	SequencePair					= (Sequence | Pair),			// 连对
	SequenceTriplet					= (Sequence | Triplet),			// 三张的顺子
	SequenceTripletAttachSingle		= (SequenceTriplet | Single),	// 飞机(带单张)
	SequenceTripletAttachPair		= (SequenceTriplet | Pair),		// 飞机(带对子)
	Bomb							= 0x10,							// 炸弹
	BombAttachSingle				= (Bomb | Single),				// 四带二(单张)
	BombAttachPair					= (Bomb | Pair),				// 四带二(对子)
	Rocket							= 0x20,							// 火箭
};

//#define LOG( fmt, ... ) {				\
//	CString str;						\
//	str.Format( (fmt), ##__VA_ARGS__);	\
//	OutputDebugString( str );			\
//	OutputDebugString( _T("\n") );		\
//	}

enum Card_Value
{
	FK_2 = 0x01, FK_3 = 0x02 ,FK_4 = 0x03, FK_5 = 0x04, FK_6 = 0x05, FK_7 = 0x06, FK_8 = 0x07, FK_9 = 0x08, FK_10 = 0x09, FK_J = 0x0A, FK_Q = 0x0B, FK_K = 0x0C, FK_A = 0x0D, //方块 2 - A
	MH_2 = 0x11, MH_3 = 0x12 ,MH_4 = 0x13, MH_5 = 0x14, MH_6 = 0x15, MH_7 = 0x16, MH_8 = 0x17, MH_9 = 0x18, MH_10 = 0x19, MH_J = 0x1A, MH_Q = 0x1B, MH_K = 0x1C, MH_A = 0x1D, //梅花 2 - A
	HX_2 = 0x21, HX_3 = 0x22 ,HX_4 = 0x23, HX_5 = 0x24, HX_6 = 0x25, HX_7 = 0x26, HX_8 = 0x27, HX_9 = 0x28, HX_10 = 0x29, HX_J = 0x2A, HX_Q = 0x2B, HX_K = 0x2C, HX_A = 0x2D, //红心 2 - A
	HT_2 = 0x31, HT_3 = 0x32 ,HT_4 = 0x33, HT_5 = 0x34, HT_6 = 0x35, HT_7 = 0x36, HT_8 = 0x37, HT_9 = 0x38, HT_10 = 0x39, HT_J = 0x3A, HT_Q = 0x3B, HT_K = 0x3C, HT_A = 0x3D, //黑桃 2 - A
	King_Small = 0x4E, King_big = 0x4F //小鬼，大鬼
};

enum Card_Bulk_Value
{
	Value_2 = 0x0F,
	Value_3 = 0x03,
	Value_4 = 0x04,
	Value_5 = 0x05,
	Value_6 = 0x06,

	Value_7 = 0x07,
	Value_8 = 0x08,
	Value_9 = 0x09,
	Value_10 = 0x0A,
	Value_J = 0x0B,

	Value_Q = 0x0C,
	Value_K = 0x0D,
	Value_A = 0x0E,
	Value_King_Small = 0x4E,
	Value_King_Big = 0x4F,
};

enum Choice
{
	Choice_Not,			//不允许
	Choice_OK,			//允许
	Choice_Rand,		//随机
};

/// 牌类型
enum EArrayType
{
	ARRAY_TYPE_ERROR			= 0,
	ARRAY_TYPE_SINGLE			= 1,	/// 单张
	ARRAY_TYPE_DOUBLE			   ,	/// 对子
	ARRAY_TYPE_3W_			       ,    /// 3张不带
	ARRAY_TYPE_3W1_ONE			   ,	/// 3带单张
	ARRAY_TYPE_3W1_DOUBLE		   ,	/// 3带对子
	ARRAY_TYPE_PLANE_			   ,    /// 飞机不带翅膀
	ARRAY_TYPE_PLANE_ONE		   ,    /// 飞机带单张
	ARRAY_TYPE_PLANE_DOUBLE		   ,    /// 飞机带对子
	ARRAY_TYPE_STRAIGHT_ONE        ,    /// 单顺子
	ARRAY_TYPE_STRAIGHT_DOUBLE  =10,    /// 双顺子 连对

	ARRAY_TYPE_4w2_ONE          =11,    /// 四带2张单张
	ARRAY_TYPE_4w2_DOUBLE          ,    /// 四带2张对子

	ARRAY_TYPE_SBOMB               ,    /// 软炸
	ARRAY_TYPE_HBOMB               ,    /// 硬炸
	ARRAY_TYPE_WBOMB               ,    /// 王炸
	ARRAY_TYPE_4L                  ,    /// 四癞子
	ARRAY_TYPE_LBOMB_S             ,    /// 4张以上炸弹 软炸
	ARRAY_TYPE_LBOMB_L          =18,    /// 4张以上炸弹 纯癞子炸
};
/// 底牌类型
enum EBackArrayType
{
	BACK_ARRAY_ERROR			= 0,
	BACK_ARRAY_ONEKING			= 1,	/// 单王
	BACK_ARRAY_SHUNZI			   ,	/// 顺子
	BACK_ARRAY_TONGHUA			   ,    /// 同花
	BACK_ARRAY_TWOKING			   ,	/// 双王
	BACK_ARRAY_SANZHANG			   ,	/// 三张
};
/// 玩法定义 洗牌类型永为单数  不洗牌类型永为双数
enum EGAME_RULE
{
	JINGDIAN_2D = 1,				     //2D 经典斗地主
	JINGDIAN_2D_NoShuffle = 2,           //2D 经典斗地主 不洗牌
	JINGDIAN_LAIZI_2D = 3,				 //2D 经典癞子斗地主
	JINGDIAN_LAIZI_2D_NoShuffle = 4,     //2D 经典癞子斗地主 不洗牌
	TIANDI_LAIZI_2D = 5,				 //2D 天地癞子斗地主
	TIANDI_LAIZI_2D_NoShuffle = 6,       //2D 天地癞子斗地主 不洗牌
};

//出牌请求
struct T_C2S_PLAY_CARD_REQ
{
	EArrayType eArrayType;                      //牌类型
	ucard uCards[ONE_HAND_CARD_COUNT];			//目标牌型数据 癞子牌转换成的牌
	ucard uActualCards[ONE_HAND_CARD_COUNT];	//实际手牌 有癞子牌
	uint  iCardCount;							//牌大小
	T_C2S_PLAY_CARD_REQ()
	{
		iCardCount = 0;
		memset(uCards,0,sizeof(uCards));
		memset(uActualCards,0,sizeof(uCards));
	}

	bool IsBomb(EArrayType eArrayType)
	{
		return
		(eArrayType == ARRAY_TYPE_SBOMB    ||
		 eArrayType == ARRAY_TYPE_HBOMB    ||
		 eArrayType == ARRAY_TYPE_WBOMB    ||
		 eArrayType == ARRAY_TYPE_4L       ||
		 eArrayType == ARRAY_TYPE_LBOMB_S  ||
		 eArrayType == ARRAY_TYPE_LBOMB_L);
	}
};

//出牌组合提示结果
struct T_S2C_PROMPT_CARD_RES
{
	T_C2S_PLAY_CARD_REQ sCards[ONE_HAND_CARD_COUNT];   // 牌数据
	uint  iCardCount;								   // 组合牌个数
	T_S2C_PROMPT_CARD_RES()
	{
		memset(this,0,sizeof(T_S2C_PROMPT_CARD_RES));
	}
};

struct SGetPlayCardparam
{
	int iMybSeatNO; 
	int iLastOutCardUser;   //// 上次出牌玩家
	int iBanker;
	int iUserCardCounts[PLAY_COUNT];
	bool bOutCard;		   ///  是否是新一轮
	EArrayType	iDeskShape;
	int iHandShape;
	const Card * pHandCard[PLAY_COUNT];				//玩家手牌（未经转换的手牌）
};

#define LOG( fmt, ... )