#pragma once

//定庄状态基类
class CBaseFixBankerLogic
{
	//成员函数
public:
	//构造函数		
	CBaseFixBankerLogic(void);
	//析构函数
	virtual ~CBaseFixBankerLogic();

	//该模块主要动作的执行都在次函数中(扔色子，画牌墙，定庄，连局，连庄)
	virtual bool Run();

	//扔色子
	virtual bool InitShaiZiResult();
	//不记录游戏规则的临时一次扔单个色子动作结果
	virtual BYTE RandShaiZiResult();
	//获得每一次扔色子的总点数
	virtual BYTE GetShaiZiResult(BYTE uTime);
	//获得每一次扔色子某一个色子的点数
	//BYTE uShaiZiID 每一次扔的其中第几个色子的点数
	//BYTE uTime 第几次扔的色子
	virtual BYTE GetOneShaiZiResult(BYTE uShaiZiID,BYTE uTime);

	//画牌墙
	virtual bool PaintWall();
	//定庄家(连局，连庄)
	virtual bool InitBanker();

	//混乱所以的麻将子
	virtual bool RandCard();

	//成员变量

public:

};