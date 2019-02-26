#pragma once

#include "GlobalData.h"

//出牌状态基类
class CBaseShowCardLogic
{
	//成员函数
public:
	//构造函数		
	CBaseShowCardLogic(void);
	//析构函数
	virtual ~CBaseShowCardLogic();

	//此牌先前碰过
	bool BePeng(BYTE bDesk, BYTE uCard);
	//能否吃
	//BYTE bDesk 谁准备拦牌
	//BYTE uOutDesk 上家是谁
	//BYTE uCard 上家打出的牌
	//MJ_EAT eat[] 能够吃的情况
	bool CanBreakForChi(BYTE bDesk, BYTE uOutDesk, BYTE uCard, MJ_EAT eat[],bool bBaiBanFlag);
	//能否碰
	bool CanBreakForPeng(BYTE bDesk, BYTE uCard);
	//能否杠
	//BYTE bDesk 谁准备杠
	//BYTE uCard 上家打出的牌
	//bool &kindGang 杠的类型 （初始化false 明杠false 补杠true）
	bool CanBreakForGang(BYTE bDesk, BYTE uOutDesk, BYTE uCard, bool &bkindGang,bool bFetchFlag=false);
	//能否听
	//BYTE &bResCard 听会确定目标牌(听找出的是去除掉就可以听的牌)
	bool CanBreakForTing(BYTE bDesk, BYTE uCard, BYTE &bResCard);
	//能否胡
	bool CanBreakForHu(BYTE bDesk, BYTE uCard);
	//能否拦牌
	//BYTE bDesk 谁准备拦牌
	//BYTE uOutDesk 上家是谁
	//BYTE uCard 上家打出的牌
	//MJ_BREAK_ALLOW &kind 成立的拦牌类型
	//MJ_EAT eat[] 能够吃的情况
	//bool &kindGang 杠的类型 （初始化false 明杠false 补杠true）
	//BYTE &bResCard 听会确定目标牌(听找出的是去除掉就可以听的牌)
	virtual bool CanBreak(BYTE bDesk, BYTE uOutDesk, BYTE uCard, MJ_BREAK_ALLOW &kind, MJ_EAT eat[], bool &bkindGang, BYTE &bResCard,bool ZiMo=false);
	//吃
	bool BreakForChi(BYTE bDesk, BYTE uOutDesk, BYTE uCard, MJ_EAT eat);
	//碰
	bool BreakForPeng(BYTE bDesk, BYTE uOutDesk, BYTE uCard);
	//杠
	bool BreakForGang(BYTE bDesk, BYTE uOutDesk, BYTE uCard, bool bkindGang);
	//听
	bool BreakForTing(BYTE bDesk, BYTE uOutDesk, BYTE uCard);
	//胡
	virtual bool BreakForHu(BYTE bDesk, BYTE uOutDesk, BYTE uCard,bool bZiMo=false);	
	//拦牌
	//BYTE bDesk 拦牌玩家
	//BYTE uOutDesk 被拦牌玩家
	//BYTE uCard 要拦的牌
	//BYTE kind 成立的拦牌类型ID
	//MJ_EAT ea 吃牌需要用到具体怎么吃
	//BYTE &bResCard 听会确定目标牌(听找出的是去除掉就可以听的牌)
	//bool &kindGang 杠的类型 （初始化false 明杠false 补杠true）
	virtual bool Break(BYTE bDesk, BYTE uOutDesk, BYTE uCard, BYTE kind, MJ_EAT ea, BYTE &bResCard, bool bkindGang);
	//多人拦牌时判断谁拦牌成功
	//BYTE uOutDesk 被拦牌的玩家
	//BaseMess_BreakCard &BreakCard 成功拦牌的数据
	//bool clockwise 优先级是否按顺时针判断
	virtual bool GetBreakResult(BYTE uOutDesk, BaseMess_BreakCard &BreakCard, bool clockwise); 
	//比较两个拦牌操作的优先级
	//BYTE kindA 类型A
	//BYTE kindB 类型B
	//返回值 0一样大 1类型A大 2类型B大 255错误
	BYTE GetPriorityBreak(BYTE kindA, BYTE kindB);
	//能否出牌
	virtual bool CanOut(BYTE bDesk, BYTE uCard);
	//出牌
	virtual bool Out(BYTE bDesk, BYTE uCard);
	//能否摸牌
	//BYTE ukind 摸牌类型
	virtual bool CanTake(BYTE bDesk, BYTE ukind);
	//摸牌
	//BYTE bDesk 摸牌玩家
	//BYTE ukind 摸牌类型
	//BYTE &uCard 摸到的牌
	virtual bool Take(BYTE bDesk, BYTE ukind, BYTE &uCard);
	//设置下一个摸牌牌墙位置(摸一张牌)
	//BYTE ukind 摸牌类型
	virtual bool SetNextTakePortInfo(BYTE ukind);
	//设置当前摸牌牌墙位置
	//BYTE uDesk;//当前抓牌的牌墙方位
	//BYTE uPort;//当前抓的是牌墙上第几张牌
	virtual bool SetNowTakePortInfo(BYTE uDesk, BYTE uPort);
	//摸取万能牌
	virtual bool TakeWildCard(BYTE uDesk);
	//删除一张在一组碰牌中的指定的牌
	bool DeletePeng(map<BYTE, MJ_PENG> &peng, BYTE uCard);
	//能否三金倒胡牌
	bool CanSanJinHuPai(BYTE byDeskStation,BYTE byFetchCard);
	//能否胡牌(lw添加)
	bool CanHu(BYTE byDeskStation,BYTE byOutCard,bool bZiMo);
	//七对
	bool CheckQiDui();
	//平糊检测
	bool CheckPingHu();
	//平糊组牌
	bool MakePingHu(CheckHuStruct &PaiData,PingHuStruct &hupaistruct,int csnum=0);
	//冒泡排序
	void MaoPaoSort(BYTE a[], int count,bool big);
	//转换手牌数据的存储方式，为了方便糊牌检测
	void ChangeHandPaiData(BYTE byArHandPai[],int iCount,BYTE byNoCaiShenPai[],int NoCaiShenCount,int iBaiBanNums);
	//成员变量
public:
	int	m_iMaxFan;//当前最大番数值
	BYTE m_byArHandPai[20];//手牌不超过20张
	BYTE m_byJiangCard[2];//做将的牌
	BYTE m_byJingNum;//财神牌数量
	CheckHuStruct	m_HuTempData;//糊牌检测临时数据(带财神)
	CheckHuStruct	m_NoJingHuTempData;//糊牌检测临时数据(无财神)
	PingHuStruct	m_hupaistruct;///糊牌数结构数组
	PingHuStruct	m_TempHupaistruct;//糊牌数结构数组
};