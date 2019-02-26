#pragma once
#include "../GameMessage/GameData.h"

#pragma pack(1)
typedef struct tagZhongNiao
{
	BYTE byMenPai[PLAY_COUNT][MEN_CARD_NUM];//门牌数据
	int byCard[ZAMA_COUNT];//中鸟数据
	tagZhongNiao()
	{
		clear();
	}
	void clear()
	{
		memset(this, 255 ,sizeof(tagZhongNiao));
		memset(byCard, 255, sizeof(byCard));	//需要补花玩家位置
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
#pragma pack()
//全局对象
extern GameDataEx g_GameData;