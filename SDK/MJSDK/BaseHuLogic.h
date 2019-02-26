#pragma once
#include "GlobalData.h"

//胡牌状态基类
class CBaseHuLogic
{
	//成员函数
public:
	//构造函数		
	CBaseHuLogic(void);
	//析构函数
	virtual ~CBaseHuLogic();

	// 规则使用的函数指针
	typedef bool (*CHECKFUNC)(BYTE bDesk, UDT_MAP_MJ_DATA card);
	//胡牌配置文件设置
	virtual bool InitData(CString file, CString key) = 0;
	//判断番是否成立
	virtual bool CheckFanNode(BYTE bDesk, UDT_MAP_MJ_DATA card)=0;
	//为番型结构体填充数据
	//BYTE uKindID 对应的番型ID
	//TCHAR text[] 名字
	//BYTE uFanValue[] 自摸或者放炮的番值
	//BYTE uHuAllowKind; 如果即允许自摸又允许放炮 则为 Eie_HU_ZIMO+Eie_HU_FANGPAO
	bool AddFanInfo(BYTE uKindID, TCHAR text[], BYTE uFanValue[], BYTE uHuKind);

	//判断该手牌是否胡了
	virtual bool CheckHu(BYTE bDesk, UDT_MAP_MJ_DATA card);

	//判断该手牌是否胡了//无万能牌
	bool CheckHuEx(BYTE bDesk, UDT_MAP_MJ_DATA card);

	//判断该手牌是否胡普通牌型(一对将和几组刻子的牌)
	virtual bool CheckNormalHu(BYTE bDesk, UDT_MAP_MJ_DATA card);

	//会改变牌数据，删除所有刻子(三个相同的 或者 连续的三张 或者 暗杠)
	//map<BYTE, BYTE> &SameCard;//前一个是什么牌，后一个是记录牌张数
	bool DeleteKeZi(UDT_MAP_MJ_DATA &card, map<BYTE, BYTE> &SameCard);
	//会改变牌数据，删除所有刻子(三个相同的 或者 连续的三张 或者 暗杠)
	//map<BYTE, BYTE> &SameCard;//前一个是什么牌，后一个是记录牌张数
	bool DeleteKeZiEx(UDT_MAP_MJ_DATA &card, map<BYTE, BYTE> &SameCard);

	//提取连续的三张牌(必须是同类型的牌,会改变牌数据)
	bool GetShunZi(UDT_MAP_MJ_DATA &card, UDT_MAP_MJ_DATA &ResultCard);

	//去除掉重复的牌
	bool SetAloneCard(UDT_MAP_MJ_DATA &card);
public:
	//成员变量
	map<BYTE, MJ_HU_FAN> m_FanBaseInfo;//番的各种基础数据

};