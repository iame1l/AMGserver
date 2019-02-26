#pragma once

//发牌状态基类
class CBaseDealLogic
{
	//成员函数
public:
	//构造函数		
	CBaseDealLogic(void);
	//析构函数
	virtual ~CBaseDealLogic();
	//发牌，设置玩家手牌(同时是会影响到牌墙的)//先扔色子再摸牌
	virtual bool SetUserHandCard();
	//发牌，设置玩家手牌(同时是会影响到牌墙的)//先摸牌再扔色子
	virtual bool SetUserHandCardEx();
	//根据色子得出的开始抓牌方位(从扔色子的位置开始顺时针计算)
	virtual BYTE GetFirstPosition(BYTE bDesk, BYTE uTime, bool clockwise = true);
	//根据色子得出的开始抓牌方位(属于系统扔色子)
	virtual BYTE GetFirstPosition(BYTE uTime, bool clockwise = true);
	//根据色子得出抓某一方位抓第几屯
	//BYTE uStarPosition 计算得到的抓牌方位
	//BYTE &uResPosition 由于该玩家面前的牌墙不够抓得跑到下家去找牌墙
	//BYTE uTime 定屯数的色子信息
	//bool clockwise 顺时针还是逆时针
	virtual BYTE GetFirstPort(BYTE uStarPosition, BYTE &uResPosition,BYTE uTime, bool clockwise = true);
	//获取下一个牌墙节点(摸一张牌)
	//BYTE uStarPosition 计算得到的抓牌方位
	//BYTE &uResPosition 由于该玩家面前的牌墙不够抓得跑到下家去找牌墙
	//bool clockwise 顺时针还是逆时针
	BYTE GetNextPort(BYTE uStarPosition, BYTE &uResPosition, BYTE uPort, bool clockwise);
	//成员变量
public:

};