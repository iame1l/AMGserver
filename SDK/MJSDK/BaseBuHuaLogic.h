#pragma once

//补花状态基类
class CBaseBuHuaLogic
{
	//成员函数
public:
	//构造函数		
	CBaseBuHuaLogic(void);
	//析构函数
	virtual ~CBaseBuHuaLogic();
	//处理手中现有的花牌(只补一张)
	virtual bool BuHua(BYTE uDesk);
	//处理手中所有的花牌
	virtual bool BuHuaAll(BYTE uDesk);

	//成员变量
public:

};