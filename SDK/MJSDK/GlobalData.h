#pragma once
#include "stdafx.h"
#include <map>
using namespace std;
typedef map<BYTE, BYTE> UDT_MAP_MJ_DATA;

//////////////////////////////////////////////////////////////////////////
//  游戏中逻辑位置对应方位的分布 (4人)	    2人	
// 				 0(东)					     0	
// 	             |						     |
// 				 |						     |
//  （北）3 —————— 1（南）             |
// 				 |                           |
// 				 |                           |      
// 				 2（西）                     1
// 都是顺时针
//////////////////////////////////////////////////////////////////////////

//麻将框架基本状态定义区==========================================================
//使用的状态ID
enum _E_MJ_BASE_STATION
{
	_E_MJ_GS_WAIT_FORGAME=0,//游戏等待状态
	_E_MJ_GS_FIX_BANKER,	//定庄状态
	_E_MJ_GS_DEAL,			//发牌状态(一般用不着)
	_E_MJ_GS_BUHUA,			//补花状态
	_E_MJ_GS_SHOWCARD,		//出牌状态
	_E_MJ_GS_HU,			//胡牌结算状态
	_E_MJ_GS_NUM //数目
};

//服务端麻将计时器定义区==========================================================
//基础计时器ID
enum _E_MJ_BASE_TIME_ID
{
	_E_BASE_TIME_ID_WAIT_DEAL_AND_FIX_BANKER=30,//定庄和发牌后延时
	_E_BASE_TIME_ID_WAIT_BUHUA,//补花后延时
	_E_BASE_TIME_ID_OUT_CARD,//出牌等待计时器
	_E_BASE_TIME_ID_BREAK_CARD,//拦牌等待计时器
	_E_BASE_TIME_ID_TAKE_CARD_NORMAL,//正常摸牌等待计时器
	_E_BASE_TIME_ID_TAKE_CARD_GANG,//杠牌摸牌等待计时器
	_E_BASE_TIME_ID_TAKE_CARD_BUHUA,//补花摸牌等待计时器
	_E_BASE_TIME_ID_TAKE_OUT_CARD,//摸牌后缓冲通知出牌计时器
	_E_BASE_TIME_ID_NUM//数目
};

//配置项数据定义区==========================================================
#define SET_KIND					_T("MJ")			//主键名

#define SET_NAME_ALLNUM				_T("AllNum")		//麻将张数
#define SET_NAME_EXIST_WAN			_T("ExistWan")		//是否存在万牌
#define SET_NAME_EXIST_TONG			_T("ExistTong")		//是否存在筒牌
#define SET_NAME_EXIST_TIAO			_T("ExistTiao")		//是否存在条牌
#define SET_NAME_EXIST_FENG			_T("ExistFeng")		//是否存在风牌
#define SET_NAME_EXIST_JIAN			_T("ExistJian")		//是否存在箭牌
#define SET_NAME_EXIST_HUA			_T("ExistHua")		//是否存在花牌
#define SET_NAME_WILDCARD			_T("WildCard")		//是否存在万能牌

#define SET_NAME_SHAIZI_NUM			_T("ShaiZiNum")		//色子数
#define SET_NAME_SHAIZI_TIME		_T("ShaiZiTime")	//扔的次数
#define SET_NAME_DRAWORDER			_T("DrawOrder")		//抓牌墙牌顺序
#define SET_NAME_OUTORDER			_T("OutOrder")		//打牌顺序

#define SET_NAME_FIRSTHANDCARD		_T("FirstHandCard")	//起手手牌张数

#define SET_NAME_NUM_FOR_TUN		_T("NumForTun")		//牌墙码牌时是几张为一屯

#define SET_NAME_OUT_CARD_TIME		_T("OutCardTime")		//出牌思考时间
#define SET_NAME_BREAK_CARD_TIME	_T("BreakCardTime")		//拦牌思考时间
#define SET_NAME_WAIT_BEGIN_TIME	_T("WaitBeginTime")		//等待开始时间

#define SET_NAME_DE_FEN0			_T("DeFen0")		//庄家底分
#define SET_NAME_DE_FEN1			_T("DeFen1")		//闲家底分

//麻将子数据定义区==========================================================
#define MJ_KIND_INTERVAL 10 //麻将类型之间的间隔长度(这个长度一定不能小于某一类型的子项数目)
//麻将类型ID
enum _E_MJ_KIND_ID
{
	Eie_BASEKIND_WAN=1,		//万牌
	Eie_BASEKIND_TIAO,		//条牌
	Eie_BASEKIND_TONG,		//筒牌
	Eie_BASEKIND_FENG,		//风牌
	Eie_BASEKIND_JIAN,		//箭牌
	Eie_BASEKIND_HUA,		//花牌
	Eie_BASEKIND_NUM = Eie_BASEKIND_HUA - Eie_BASEKIND_WAN + 1 //类型数目
};

//万牌
enum _E_MJ_WAN
{
	Eie_BASE_WAN1 = 1,		//1万
	Eie_BASE_WAN2,		//2万
	Eie_BASE_WAN3,		//3万
	Eie_BASE_WAN4,		//4万
	Eie_BASE_WAN5,		//5万
	Eie_BASE_WAN6,		//6万
	Eie_BASE_WAN7,		//7万
	Eie_BASE_WAN8,		//8万
	Eie_BASE_WAN9,		//9万
	Eie_BASE_WAN_NUM = Eie_BASE_WAN9 - Eie_BASE_WAN1 + 1 //数目
};
//条牌
enum _E_MJ_TIAO
{
	Eie_BASE_TIAO1 = 11,		//1条
	Eie_BASE_TIAO2,		//2条
	Eie_BASE_TIAO3,		//3条
	Eie_BASE_TIAO4,		//4条
	Eie_BASE_TIAO5,		//5条
	Eie_BASE_TIAO6,		//6条
	Eie_BASE_TIAO7,		//7条
	Eie_BASE_TIAO8,		//8条
	Eie_BASE_TIAO9,		//9条
	Eie_BASE_TIAO_NUM = Eie_BASE_TIAO9 - Eie_BASE_TIAO1 + 1 //数目
};
//筒牌
enum _E_MJ_TONG
{
	Eie_BASE_TONG1 = 21,		//1筒
	Eie_BASE_TONG2,		//2筒
	Eie_BASE_TONG3,		//3筒
	Eie_BASE_TONG4,		//4筒
	Eie_BASE_TONG5,		//5筒
	Eie_BASE_TONG6,		//6筒
	Eie_BASE_TONG7,		//7筒
	Eie_BASE_TONG8,		//8筒
	Eie_BASE_TONG9,		//9筒
	Eie_BASE_TONG_NUM = Eie_BASE_TONG9 - Eie_BASE_TONG1 + 1 //数目
};
//风牌
enum _E_MJ_FENG
{
	Eie_BASE_FENG_DONG = 31,		//东风
	Eie_BASE_FENG_NAN,		//南风
	Eie_BASE_FENG_XI,		//西风
	Eie_BASE_FENG_BEI,		//北风
	Eie_BASE_FENG_NUM = Eie_BASE_FENG_BEI - Eie_BASE_FENG_DONG + 1 //数目
};
//箭牌
enum _E_MJ_JIAN
{
	Eie_BASE_JIAN_ZHONG = 35,		//中
	Eie_BASE_JIAN_FA,		//发
	Eie_BASE_JIAN_BAI,		//白
	Eie_BASE_JIAN_NUM = Eie_BASE_JIAN_BAI - Eie_BASE_JIAN_ZHONG + 1 //数目
};
//花牌
enum _E_MJ_HUA
{
	Eie_BASE_HUA_CHUN = 41,		//春
	Eie_BASE_HUA_XIA,		//夏
	Eie_BASE_HUA_QIU,		//秋
	Eie_BASE_HUA_DONG,		//冬
	Eie_BASE_HUA_MEI,		//梅
	Eie_BASE_HUA_LAN,		//兰
	Eie_BASE_HUA_ZU,		//竹
	Eie_BASE_HUA_JU,		//菊
	Eie_BASE_HUA_NUM = Eie_BASE_HUA_JU - Eie_BASE_HUA_CHUN + 1 //数目
};

//桌面上所有麻将的宏观信息
struct MJ_DESK_DATABASE
{
	BYTE uAllNum;//麻将张数
	bool bExistKind[Eie_BASEKIND_NUM];

	MJ_DESK_DATABASE()
	{
		ZeroMemory(this, sizeof(MJ_DESK_DATABASE));
	}
};

struct MJ_EAT//吃组成的牌数据
{
	BYTE bDeskStation;//被吃玩家
	BYTE udata[3];
	BYTE udata2;//被吃的牌

	void init()
	{
		ZeroMemory(this, sizeof(MJ_EAT));
		bDeskStation=255;
	}

	MJ_EAT()
	{
		init();
	}
};

struct MJ_PENG//碰组成的牌数据
{
	BYTE bDeskStation;//被碰玩家
	BYTE udata;//牌

	void init()
	{
		ZeroMemory(this, sizeof(MJ_PENG));
		bDeskStation=255;
	}

	MJ_PENG()
	{
		init();
	}
};

struct MJ_GANG//杠组成的牌数据
{
	BYTE bDeskStation;//被杠玩家
	BYTE udata;//牌
	bool bkind;//杠牌类型 默认明杠暗杠false , true为补杠
	void init()
	{
		ZeroMemory(this, sizeof(MJ_GANG));
		bDeskStation=255;
	}

	MJ_GANG()
	{
		init();
	}
};

//玩家牌数据 包括：手牌、打出的牌、自己面前牌墙的牌、吃的牌、碰的牌、杠的牌
struct MJ_USER_CARD
{
	BYTE bDeskStation;//用户逻辑id
	UDT_MAP_MJ_DATA hand;//手牌(包含暗杠)
	UDT_MAP_MJ_DATA out;//打出的牌
	UDT_MAP_MJ_DATA wall;//自己面前的牌墙
	UDT_MAP_MJ_DATA hua;//花牌(补花后被去除的花牌)
	map<BYTE, MJ_EAT> eat;//吃的牌
	map<BYTE, MJ_PENG> peng;//碰的牌
	map<BYTE, MJ_GANG> gang;//杠的牌
	BYTE bTakeCard;//现在摸到的牌
	void init()
	{
		hand.clear();
		out.clear();
		wall.clear();
		hua.clear();
		eat.clear();
		peng.clear();
		gang.clear();
		bDeskStation=255;
		bTakeCard = 0;
	}

	MJ_USER_CARD()
	{
		init();
	}
};

//万能牌信息
struct MJ_WILD_CARDINFO
{
	bool bWildCard;//是否存在万能牌
	BYTE uCard;//什么牌是万能牌

	void init()
	{
		ZeroMemory(this, sizeof(MJ_WILD_CARDINFO));
	}

	MJ_WILD_CARDINFO()
	{
		init();
	}
};

//色子数据定义区==========================================================
struct MJ_SHAIZI_DATABASE
{
	BYTE uAllNum;//色子数
	BYTE utime;//扔的次数

	//按顺序存储每次扔的每一个色子的结果，注意如果要获取第几次的第几个色子，记得考虑色子数
	map<BYTE, BYTE> ShaiZiMap;

	void init()
	{
		uAllNum=0;
		utime=0;
		ShaiZiMap.clear();
	}

	MJ_SHAIZI_DATABASE()
	{
		init();
	}
};

//每局一开始摸牌的位置（方位和第几张）
struct MJ_FIRST_DEAL_POSTINFO
{
	BYTE uPosition;//方位
	BYTE uFirstPort;//第几张
	//初始摸牌结束位置
	BYTE uEndPosition;//方位
	BYTE uEndPort;//第几张

	void init()
	{
		ZeroMemory(this, sizeof(MJ_FIRST_DEAL_POSTINFO));
	}

	MJ_FIRST_DEAL_POSTINFO()
	{
		init();
	}
};

//出牌阶段各种数据定义区==========================================================
//拦牌类型
enum _E_MJ_BREAK_KIND//（动作级别：胡>碰、杠>吃）
{
	Eie_BREAK_GUO = 0,	//过
	Eie_BREAK_CHI,		//吃
	Eie_BREAK_PENG,		//碰
	Eie_BREAK_GANG,		//杠
	Eie_BREAK_TING,		//听
	Eie_BREAK_HU,		//胡
	Eie_BREAK_ERROR,	//无动作
	Eie_BREAK_NUM = Eie_BREAK_ERROR - Eie_BREAK_GUO + 1//数目
};

enum _E_MJ_EAT_KIND
{
	Eie_EAT_FRONT=0,//前
	Eie_EAT_MIDD,//中
	Eie_EAT_LATER//后
};

//摸牌类型
enum _E_MJ_TAKE_KIND
{
	Eie_TAKE_NORMAL = 0,	//正常摸牌
	Eie_TAKE_BUHUA,			//补花摸牌
	Eie_TAKE_GANG,			//杠牌摸牌
	Eie_TAKE_WILD			//万能牌摸牌
};
//拦牌时判断针对别人打出的牌自己手牌满足哪种拦牌类型
struct MJ_BREAK_ALLOW
{
	bool data[Eie_BREAK_NUM];

	void init()
	{
		memset(data,false,sizeof(data));
	}

	MJ_BREAK_ALLOW()
	{
		init();
	}
};

//当前摸牌所在牌墙位置信息结构体
struct MJ_TAKE_CARD_DATABASE
{
	BYTE uDesk;//当前抓牌的牌墙方位
	BYTE uPort;//当前抓的是牌墙上第几张牌

	void init()
	{
		ZeroMemory(this, sizeof(MJ_TAKE_CARD_DATABASE));
		uDesk = 255;
	}

	MJ_TAKE_CARD_DATABASE()
	{
		init();
	}
};

//摸万能牌的基础数据
struct MJ_TakeWildCardBase
{
	BYTE ShaiZi[2];//一次扔两个色子
	BYTE uResPosition;//当时抓牌的方位
	BYTE uPort;//当时抓牌的哪一方位的哪张牌
	BYTE uCard;//摸到的牌
	BYTE bDesk;//翻金的人

	void init()
	{
		ZeroMemory(this, sizeof(MJ_TakeWildCardBase));
		bDesk = 255;
	}

	MJ_TakeWildCardBase()
	{
		init();
	}
};

//在摸万能牌是所留下的记忆
struct MJ_TakeWildCardInfo
{
	map<BYTE, MJ_TakeWildCardBase> data;

	void init()
	{
		data.clear();
	}

	MJ_TakeWildCardInfo()
	{
		init();
	}
};



//胡牌阶段各种数据定义区==========================================================
//胡牌类型
enum _E_MJ_HU_KIND
{
	Eie_HU_ZIMO = 1,	//自摸(赋值为1，否则无法计算)
	Eie_HU_FANGPAO,		//放炮
	Eie_HU_KIND_NUM = Eie_HU_FANGPAO - Eie_HU_ZIMO + 1 //数目
};
//番的基本数据
struct MJ_HU_FAN
{
	BYTE uHuAllowKind;//如果即允许自摸又允许放炮 则为 Eie_HU_ZIMO+Eie_HU_FANGPAO
	TCHAR Name[26];//名字
	BYTE uFanValue[Eie_HU_KIND_NUM];//自摸或者放炮的番值
	BYTE uKindID;//对应的番型ID

	void init()
	{
		ZeroMemory(this, sizeof(MJ_HU_FAN));
		uKindID = 255;
	}

	MJ_HU_FAN()
	{
		init();
	}
};
//糊牌检测临时数据
struct CheckHuStruct
{
	BYTE  data[20][2];   //牌数据，0牌值，1张数
	BYTE  conut;//牌的种类个数
	void Add(BYTE pai)
	{
		if(GetPaiCount(pai)>0)
		{
			for(int i=0;i<20;i++)
			{
				if(data[i][0] == pai && pai != 255)
					data[i][1]++;
			}
		}
		else
		{
			data[conut][0] = pai;
			data[conut][1] = 1;
			conut++;
		}
	};
	BYTE GetPaiCount(BYTE pai)
	{
		for(int i=0;i<20;i++)
		{
			if(data[i][0] == pai && pai != 255 && data[i][1] != 255)
				return data[i][1];
		}
		return 0;
	};
	BYTE GetDataCount()
	{
		conut = 0;
		for(int i=0;i<20;i++)
		{
			if(data[i][0] != 255)
				conut++;
		}
		return conut;
	};
	BYTE GetAllPaiCount()
	{
		BYTE num = 0;
		for(int i=0;i<20;i++)
		{
			if(data[i][0] != 255)
				num += data[i][1];
		}
		return num;
	};
	void SetPaiCount(BYTE pai,BYTE num)
	{
		for(int i=0;i<20;i++)
		{
			if(data[i][0] == pai && pai != 255)
			{
				data[i][1] = num;
			}
		}
	}
	void Init()
	{
		memset(data,255,sizeof(data));
		for(int i=0;i<20;i++)
		{
			data[i][1] = 0;
		}
		conut=0;
	};
};
//平糊组牌数据结构
struct PingHuStruct
{
	BYTE  byType[6];    //组牌类型
	BYTE  data[6][4];   //数据
	bool  isjing[6][4];//是否财神替代
	BYTE  count;        //节点数
	PingHuStruct()
	{
	};
	void AddData(BYTE type,BYTE pai[],bool jing[])
	{
		for(int i=0;i<6;i++)
		{
			if(byType[i] == 255)
			{
				byType[i] = type;
				data[i][0] = pai[0];data[i][1] = pai[1];data[i][2] = pai[2];data[i][3] = pai[3];
				isjing[i][0] = jing[0];isjing[i][1] = jing[1];isjing[i][2] = jing[2];isjing[i][3] = jing[3];
				count++;
				break;
			}
		}
	};
	void DeleteData(BYTE type,BYTE pai[],bool jing[])//删除元素
	{
		for(int i=0;i<6;i++)
		{
			if(byType[i] == type && data[i][0] == pai[0 ]&& data[i][1] == pai[1] && isjing[i][0] == jing[0] && isjing[i][1] == jing[1])
			{
				byType[i] = 255;
				memset(data[i],255,sizeof(data[i]));
				memset(isjing[i],0,sizeof(isjing[i]));
				count--;
				break;
			}
		}
	};
	void SortData()
	{
		for(int i=0;i<5;i++)
		{
			if(byType[i] != 255)
				continue;
			for(int j=i;j<5;j++)
			{
				byType[j] = byType[j+1];
				memcpy(data[j],data[j+1],sizeof(data[j]));//数据
				memcpy(isjing[j],isjing[j+1],sizeof(isjing[j]));//是否财神替代
			}
		}
		count = 0;
		for(int i=0;i<5;i++)
		{
			if(byType[i] != 255)
				count++;
		}
	};
	void Init()
	{
		memset(this,255,sizeof(PingHuStruct));
		memset(isjing,0,sizeof(isjing));
		count = 0;
	};

};
#define TYPE_JIANG_PAI			130				//将牌类型
#define TYPE_JINGDIAO_JIANG		131				//精钓将牌类型
#define TYPE_SHUN_ZI			132				//顺子类型
#define TYPE_AN_KE				133				//暗刻类型
#define TYPE_JING_KE			134				//财神刻子类型
//日志打印函数
void MyDebugString(LPCTSTR lpPre, LPCTSTR lpFormat, ...);