/****************************************************************************
Copyright (c) 2014-2017 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2017 ShenZhen Redbird Network Polytron Technologies Inc.
 
http://www.hotniao.com

All of the content of the software, including code, pictures, 
resources, are original. For unauthorized users, the company 
reserves the right to pursue its legal liability.
****************************************************************************/                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
#ifndef BASE_CARD_H
#define BASE_CARD_H

#include <iostream>
#include <algorithm>
#include <vector>
#include <assert.h>
using namespace std;

namespace HN
{
#define CARD_MASK_VALUE 0x0F	//扑克牌数值 4位 0~15
#define CARD_MASK_TYPE 4		//扑克牌类型 4位 0~15

#define MAKE_CARD(TYPE,VALUE)	(((TYPE)<<4) + (VALUE))
#define CARD_NULL	0
#define CARD_ERROR	255

	class CardBase;
	typedef unsigned char ucard;
	typedef unsigned char uval;
	 

	class CardBase
	{
	public:
		typedef bool (CardBase::*Filter)(); 
	public:
		//扑克牌类型 4位 0~15
		enum emCardType
		{
			TYPE_DIAMOND  = 0,//方块
			TYPE_CLUB	  = 1,//梅花
			TYPE_HEART	  = 2,//红桃
			TYPE_SPADE	  = 3,//黑桃
			TYPE_JOKER    = 4,//鬼
		};

		//扑克牌数值 4位 0~15
		enum emCardVal
		{
			VAL_E  = 0,
			VAL_MIN =1,
            VAL_A =1,
			VAL_2,
			VAL_3,
			VAL_4,
			VAL_5,
			VAL_6,
			VAL_7,
			VAL_8,
			VAL_9,
			VAL_10,
			VAL_J,
			VAL_Q,
			VAL_K,
			
			VAL_SJ = 14,
			VAL_BJ = 15,
			VAL_MAX = 15,
		};

		//扑克定义
		enum emCard
		{
			D_3  = MAKE_CARD(TYPE_DIAMOND,VAL_3),
			D_4  = MAKE_CARD(TYPE_DIAMOND,VAL_4),
			D_5  = MAKE_CARD(TYPE_DIAMOND,VAL_5),
			D_6  = MAKE_CARD(TYPE_DIAMOND,VAL_6),
			D_7  = MAKE_CARD(TYPE_DIAMOND,VAL_7),
			D_8  = MAKE_CARD(TYPE_DIAMOND,VAL_8),
			D_9  = MAKE_CARD(TYPE_DIAMOND,VAL_9),
			D_10 = MAKE_CARD(TYPE_DIAMOND,VAL_10),
			D_J  = MAKE_CARD(TYPE_DIAMOND,VAL_J),
			D_Q  = MAKE_CARD(TYPE_DIAMOND,VAL_Q),
			D_K  = MAKE_CARD(TYPE_DIAMOND,VAL_K),
			D_A  = MAKE_CARD(TYPE_DIAMOND,VAL_A),
			D_2  = MAKE_CARD(TYPE_DIAMOND,VAL_2),

			C_3  = MAKE_CARD(TYPE_CLUB,VAL_3),
			C_4  = MAKE_CARD(TYPE_CLUB,VAL_4),
			C_5  = MAKE_CARD(TYPE_CLUB,VAL_5),
			C_6  = MAKE_CARD(TYPE_CLUB,VAL_6),
			C_7  = MAKE_CARD(TYPE_CLUB,VAL_7),
			C_8  = MAKE_CARD(TYPE_CLUB,VAL_8),
			C_9  = MAKE_CARD(TYPE_CLUB,VAL_9),
			C_10 = MAKE_CARD(TYPE_CLUB,VAL_10),
			C_J  = MAKE_CARD(TYPE_CLUB,VAL_J),
			C_Q  = MAKE_CARD(TYPE_CLUB,VAL_Q),
			C_K  = MAKE_CARD(TYPE_CLUB,VAL_K),
			C_A  = MAKE_CARD(TYPE_CLUB,VAL_A),
			C_2  = MAKE_CARD(TYPE_CLUB,VAL_2),

			H_3  = MAKE_CARD(TYPE_HEART,VAL_3),
			H_4  = MAKE_CARD(TYPE_HEART,VAL_4),
			H_5  = MAKE_CARD(TYPE_HEART,VAL_5),
			H_6  = MAKE_CARD(TYPE_HEART,VAL_6),
			H_7  = MAKE_CARD(TYPE_HEART,VAL_7),
			H_8  = MAKE_CARD(TYPE_HEART,VAL_8),
			H_9  = MAKE_CARD(TYPE_HEART,VAL_9),
			H_10 = MAKE_CARD(TYPE_HEART,VAL_10),
			H_J  = MAKE_CARD(TYPE_HEART,VAL_J),
			H_Q  = MAKE_CARD(TYPE_HEART,VAL_Q),
			H_K  = MAKE_CARD(TYPE_HEART,VAL_K),
			H_A  = MAKE_CARD(TYPE_HEART,VAL_A),
			H_2  = MAKE_CARD(TYPE_HEART,VAL_2),

			S_3  = MAKE_CARD(TYPE_SPADE,VAL_3),
			S_4  = MAKE_CARD(TYPE_SPADE,VAL_4),
			S_5  = MAKE_CARD(TYPE_SPADE,VAL_5),
			S_6  = MAKE_CARD(TYPE_SPADE,VAL_6),
			S_7  = MAKE_CARD(TYPE_SPADE,VAL_7),
			S_8  = MAKE_CARD(TYPE_SPADE,VAL_8),
			S_9  = MAKE_CARD(TYPE_SPADE,VAL_9),
			S_10 = MAKE_CARD(TYPE_SPADE,VAL_10),
			S_J  = MAKE_CARD(TYPE_SPADE,VAL_J),
			S_Q  = MAKE_CARD(TYPE_SPADE,VAL_Q),
			S_K  = MAKE_CARD(TYPE_SPADE,VAL_K),
			S_A  = MAKE_CARD(TYPE_SPADE,VAL_A),
			S_2  = MAKE_CARD(TYPE_SPADE,VAL_2),

			SJ	 = MAKE_CARD(TYPE_JOKER,VAL_SJ),
			BJ   = MAKE_CARD(TYPE_JOKER,VAL_BJ),

			NIL  = CARD_NULL
		};
	public:
		CardBase();
		CardBase(const CardBase& c);
		CardBase(ucard c);
		virtual ~CardBase();

		virtual const CardBase& operator = (const CardBase& c);
		virtual bool operator ==(const CardBase& c)const;
		virtual bool operator <(const CardBase& c)const;
		virtual bool operator !=(const CardBase& c)const;

	public:	
		/*
		*@brief 牌值是否合法
		*@return bool
		*/
		virtual bool isValid()const;
		
		/*
		*@brief 获取牌类型
		*@return HN::CardArrayBase::emCardType
		*/
		virtual emCardType getType()const; 

		/*
		*@brief 获取牌数值
		*@return HN::CardArrayBase::emCardVal
		*/
		virtual uval getVal()const;

		virtual uval getRealVal()const;

		/*
		*@brief 获取牌
		*@return HN::ucard
		*/
		virtual ucard getData()const;
	private:
		/*
		*@brief 
		*@param const CardBase & c
		*@return bool
		*/
		virtual bool _equal(const CardBase & c)const;
		/*
		*@brief 
		*@param const CardBase & c
		*@return bool
		*/
		virtual bool _lessThan(const CardBase &c)const;
	private:
		ucard _card;
	};
}
#endif//BASE_CARD_H
