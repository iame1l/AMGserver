/****************************************************************************
Copyright (c) 2014-2017 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2017 ShenZhen Redbird Network Polytron Technologies Inc.
 
http://www.hotniao.com

All of the content of the software, including code, pictures, 
resources, are original. For unauthorized users, the company 
reserves the right to pursue its legal liability.
****************************************************************************/

#ifndef ARRAY_CARD_BASE_H
#define ARRAY_CARD_BASE_H

#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
#include "CardBase.h"
#include ".\AIAlgorithm\IAIAlgorithm.h"

using namespace std;

namespace HN
{
	class CardArrayBase
	{
	public:
		CardArrayBase();
		CardArrayBase(const CardArrayBase& v);
		CardArrayBase(const vector<CardBase>& v);
		virtual ~CardArrayBase();

		virtual const CardArrayBase& operator = (const CardArrayBase& v);
	public:	
		/*
		*@brief 乱序
		*@param size_t iNumber  /// 以多少张来洗牌
		*@return void
		*/
		virtual void randCard(size_t iNumber = 1);
		/*
		*@brief 炸弹补充   就算不洗牌 炸弹数也不会太多
		*@param size_t iMaxNumber   最大补充多少个炸弹   补充炸弹数 0~iMaxNumber 之间
		*@return void
		*/
		void fillBomb(size_t iMaxNumber);

		/*
		*@brief 排序
		*@return void
		*/
		void Sort();
		/*
		*@brief 癞子排序
		*@return void
		*/
		void Sort(CardBase tianLaiZi,CardBase diLaiZi = CardBase::NIL);
		/*
		*@brief 牌大小
		*@return size_t
		*/
		virtual size_t Size()const;

		virtual bool Empty()const;
		/*
		*@brief 清理
		*@return void
		*/
		virtual void Clear();
		/*
		*@brief 获取牌组
		*@return const vector<CardBase>&
		*/
		const vector<CardBase>&getArray()const;
		/*
		*@brief 将数据copy到数组
		*@param ucard * arrCard
		*@param size_t len
		*@return size_t
		*/
		size_t reAssign(ucard *arrCard, size_t len) const;
        /*
		*@brief 将数据copy到数组 无花色
		*@param ucard * arrCard
		*@param size_t len
		*@return size_t
		*/
		size_t reAssignVal(ucard *arrCard, size_t len) const;

		/*
		*@brief 获取所有byCard 类型牌
		*@param  byte byCard std::vector<CardBase> & vecLaiZiData
		*@return int 个数
		*/
		int getCardAllType(CardBase byCard,std::vector<CardBase> & vecData)const;
		/*
		*@brief 获取所有byCard 类型牌
		*@param  byte byCard 
		*@return bool 
		*/
		bool getCardAllType(CardBase byCard)const;

		/*
		*@brief 初始化牌组
		*@param ucard * arrCard
		*@param size_t len
		*@return void
		*/
		void assign(ucard *arrCard, size_t len);		
		/*
		*@brief 是否包含V
		*@param const CardArrayBase & v
		*@return bool
		*/
		bool contain(const CardArrayBase&v)const;

		bool contain(const CardBase &c)const;
		/*
		*@brief 删除（每个元素只删一个）
		*@param const CardArrayBase & v
		*@return bool
		*/
		bool erase(const CardArrayBase&v);

		/*
		*@brief 删除所有
		*@param const CardBase & c
		*@return bool
		*/
		bool erase(const CardBase& c);
		/*
		*@brief 压栈
		*@param const CardBase & c
		*@return bool
		*/
		bool push_back(const CardBase& c);

		/*
		*@brief 添加
		*@param const CardArrayBase & v
		*@return bool
		*/
		bool add(const CardArrayBase&v);

		/*
		*@brief 添加一副手牌
		*@return void
		*/
		void addOnePairCard();
	private:
		vector<CardBase> _vCard;
	};

}
#endif//ARRAY_CARD_BASE_H
