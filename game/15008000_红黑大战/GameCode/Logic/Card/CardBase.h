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
#include <vector>
#include <assert.h>
#include "CardDefine.h"
using namespace std;

class CardBase
{
public:
	CardBase();
	CardBase(const CardBase& c);
	CardBase(uchar c);
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
	*@brief 获取牌数值
	*@return HN::CardArrayBase::emCardVal
	*/
	virtual uval getVal()const;
	virtual uval getRealVal()const;

	virtual uval getSeqRealVal() const;
	/*
	*@brief 获取牌类型
	*@return HN::CardArrayBase::emCardColor
	*/
	virtual emCardColor getType()const; 
	/*
	*@brief 获取牌
	*@return HN::ucard
	*/
	virtual uchar getData()const;

	/*
	*@brief 设置牌
	*@return HN::ucard
	*/
	virtual void setData(const CardBase c);

	/*获取牌分
	*@brief: const
	*@Returns:   uint
	*/
	virtual uint  score()const;

	/*
	*@brief: 是否是分牌
	*@Returns:   bool
	*/
	virtual bool  isScoreCard()const; 

	/*
	*@brief: 能否组顺子牌
	*@Returns:   bool
	*/

	bool canMakeSeq(uchar bySeq)const;

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
	uchar _card;
};

#endif//BASE_CARD_H
