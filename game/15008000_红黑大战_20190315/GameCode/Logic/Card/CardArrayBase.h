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
#include "GameDefine.h"

using namespace std;

struct Param
{
	uval val;		   //用于单张比较的数值
	uval size;		   //张数
	bool bBomb;		   //是否是4条
	bool bJokers;	   //是否是王炸
	bool b510K;		   //是否是510K
	char bType;		   //牌型
	char bCardColor;    //花色
	bool operator<(const Param & v)
	{
		if(bType == ARRAY_TYPE_SINGLE && v.bType==ARRAY_TYPE_SINGLE)
		{
			return isJudge(v);
		}

		if (bType == ARRAY_TYPE_DOUBLE && v.bType==ARRAY_TYPE_DOUBLE)
		{
			return isJudge(v);
		}

		if (bType == ARRAY_TYPE_3 && v.bType==ARRAY_TYPE_3)
		{
			return (val<v.val)?true:false;
		}

		if (bType == ARRAY_TYPE_BOMB && v.bType==ARRAY_TYPE_BOMB)
		{
			return (val<v.val)?true:false;
		}
		
		if (bType == ARRAY_TYPE_SEQ && v.bType==ARRAY_TYPE_SEQ)
		{
			return isJudge(v);
		}
		else if(bType == ARRAY_TYPE_SEQ && v.bType> ARRAY_TYPE_SEQ)
		{
			return true;
		}

		if (bType == ARRAY_TYPE_SAMET && v.bType == ARRAY_TYPE_SAMET)
		{
			return isJudgeColor(v);			
		}
		else if(bType == ARRAY_TYPE_SAMET && v.bType> ARRAY_TYPE_SAMET)
		{
			return true;
		}

		if (bType == ARRAY_TYPE_3W2 && v.bType==ARRAY_TYPE_3W2)
		{
			return (val<v.val)?true:false;
		}
		else if(bType == ARRAY_TYPE_3W2 && v.bType> ARRAY_TYPE_3W2)
		{
			return true;
		}

		if (bType == ARRAY_TYPE_4W1 && v.bType==ARRAY_TYPE_4W1)
		{
			return (val<v.val)?true:false;
		}
		else if(bType == ARRAY_TYPE_4W1 && v.bType> ARRAY_TYPE_4W1)
		{
			return true;
		}

		if (bType == ARRAY_TYPE_SAMESEQ && v.bType == ARRAY_TYPE_SAMESEQ)
		{
			return isJudgeColor(v);			
		}
		return false;
	}
	inline bool isJudge(const Param & v)
	{
		if (val<v.val)
		{
			return true;
		}
		else if (val==v.val)
		{
			return (bCardColor<v.bCardColor)?true:false;
		}
		else
		{
			return false;
		}
	}
	inline bool isJudgeColor(const Param & v)
	{
		if (bCardColor<v.bCardColor)
		{
			return true;
		}
		else if (bCardColor==v.bCardColor)
		{
			return (val<v.val)?true:false;
		}
		else
		{
			return false;
		}
	}
	void clear()
	{
		val = 0;
		bBomb = false;
		size = 0;
		bJokers = false;
		bType = ARRAY_TYPE_ERROR;
		bCardColor = 0;
		b510K = false;
	}
	Param():val(0),bBomb(false),size(0),bJokers(false),bType(0),b510K(false),bCardColor(0){}
};

class CardArrayBase
{
public:
	friend class CTestCase;
public:
	CardArrayBase();
	CardArrayBase(const CardArrayBase& v);
	CardArrayBase(const vector<CardBase>& v);
	virtual ~CardArrayBase();

	virtual const CardArrayBase& operator = (const CardArrayBase& v);
	virtual bool operator ==(const CardArrayBase& v)const;
	virtual bool operator <(const CardArrayBase& v)const;
	friend std::ostream &operator << (std::ostream &ostrm, const CardArrayBase &cardList);
public:
	class compare
	{
	public:
		enum Enumcomp
		{ASC,DESC,NASC,NDESC};
	private:
		Enumcomp comp;
	public:
		compare(Enumcomp c):comp(c) {};
		bool operator () (const CardArrayBase& v1,const CardArrayBase& v2) 
		{
			switch(comp)
			{
			case ASC:
				return v1._lessThan(v2);
			case DESC:
				return v2._lessThan(v1);
			case NASC:
				return v1._lessThanN(v2);
			case NDESC:
				return v2._lessThanN(v1);
			}
			return false;
		}
	};



public:	

	uchar bySeq;    //順子模式
	/*
	*@brief 乱序
	*@return void
	*/
	virtual void randCard();
	/*
	*@brief 牌型
	*@return eArrayType
	*/
	virtual  eArrayType type(Param &v)const ;

	/*
	*@brief: 牌分
	*@Returns:   uint
	*/
	virtual  uint  score()const;

	/*
	*@brief: 获取分牌
	*@Returns:   CardArrayBase 
	*/
	CardArrayBase getScoreCards()const;
	/*
	*@brief 排序
	*@param _Pr _Pred
	*@return void
	*/
	template<class _Pr>
	void Sort(_Pr _Pred)
	{
		std::sort(_vCard.begin(),_vCard.end(),_Pred);
	}
	/*
	*@brief 排序
	*@return void
	*/
	virtual void Sort()
	{
		std::sort(_vCard.begin(),_vCard.end());
	}
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
	*@return bool
	*/
	size_t reAssign(uchar *arrCard, size_t len);
	/*
	*@brief 初始化牌组
	*@param ucard * arrCard
	*@param size_t len
	*@return void
	*/
	void assign(uchar *arrCard, size_t len);		
	/*
	*@brief 是否完全包含V
	*@param const CardArrayBase & v
	*@return bool
	*/
	bool contain(const CardArrayBase&v)const;

	/*
	*@brief 是否含有值c
	*@param const CardBase & c
	*@return bool
	*/
	bool contain(const CardBase &c)const;

	/*
	*@brief 是否含有真实值val
	*@param uval v
	*@return bool
	*/
	bool contain(uval v)const;
	/*
	*@brief 含有
	*@param const CardArrayBase & v
	*@return bool
	*/
	size_t has(const CardArrayBase&v)const;
	/*
	*@brief 删除（每个元素只删一个）
	*@param const CardArrayBase & v
	*@return bool
	*/

	/*
	*@brief: 统计牌值数量
	*@Returns:   size_t
	*@Parameter: const CardBase & c
	*/
	size_t count_of(const CardBase &c)const;

	bool erase(const CardArrayBase&v);

	bool eraseAll(const CardArrayBase&v);
	/*
	*@brief 删除
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

	bool pop_back();
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

	/*
	*@brief 随机选取个数为t的一张牌
	*@param size_t t
	*@return HN::CardBase
	*/
	CardBase randGetCard(size_t t);

	/*
	*@brief:	比较大小
	*@Returns:   em_CmpResult
	*@Parameter: const CardArrayBase & other
	*/
	em_CmpResult compareTo(const CardArrayBase&other)const;

	/*
	*@brief: 牌数是否一样
	*@Returns:   bool
	*@Parameter: const CardArrayBase & v
	*/
	bool equal(const CardArrayBase& v)const;

	/*
	*@brief 牌型判断-单张
	*@return bool
	*/
	bool isSingle(Param &p)const;
	/*
	*@brief 牌型判断-对子
	*@return bool
	*/
	bool isDouble(Param &p)const;

	/*
	*@brief: 牌型判断-三张
	*@Returns:   bool
	*@Parameter: Param & p
	*/
	bool isThree(Param &p)const;
	/*
	*@brief 牌型判断-三带二
	*@return bool
	*/
	bool is3W2(Param &p)const;

	/*
	*@brief 牌型判断-4带1
	*@return bool
	*/
	bool is4W1(Param &p)const;

	/*
	*@brief 牌型判断-4带
	*@return bool
	*/
	bool is4W_(uval &p)const;

	/*
	*@brief 牌型判断-三带
	*@return bool
	*/
	bool is3W_(uval &v)const;
	/*
	*@brief 牌型判断-飞机
	*@return bool
	*/
	bool isPlane(Param &v)const;

	bool isPlane_(uval &v)const;
	/*
	*@brief 牌型判断-顺子
	*@return bool
	*/
	bool isSequence(Param &v)const;

	/*
	*@brief 牌型判断-同花顺
	*@return bool
	*/
	bool isSameSeq(Param &v)const;

	/*
	*@brief 牌型判断-同花
	*@return bool
	*/
	bool isSameColor(Param &v)const;

	/*
	*@brief 牌型判断-双顺
	*@param Param &p
	*@return bool
	*/
	bool isSeq_2(Param &p)const;
	/*
	*@brief 牌型判断-3顺
	*@param Param &p
	*@return bool
	*/
	bool isSeq_3(Param &p)const;
	/*
	*@brief 牌型判断-连对
	*@return bool
	*/
	bool isSeq_N(uval &v,size_t n)const;

	/*
	*@brief 牌型判断-510K
	*@param int n
	*@return bool
	*/
	bool is510K_N(Param & p,size_t n)const;

	/*
	*@brief 牌型判断-炸弹
	*@param int n
	*@return bool
	*/
	bool isBomb_N(Param &p)const;
	/*
	*@brief 牌型判断-炸弹
	*@param int n
	*@return bool
	*/
	bool isBombWithJoker_N(Param &p)const;
	/*
	*@brief 牌型判断-Jokers
	*@param int n
	*@return bool
	*/
	bool isJokers(Param &p,size_t n)const;
	/*
	*@brief 提取牌
	*@param CardArrayBase & v	 提取的牌
	*@param uval vBegin			 最小值
	*@param size_t w			 单张的宽度（单顺、双顺、三顺...）
	*@param size_t l			 连续的长度
	*@return bool				 找到一个符合的就返回true,or return false
	*/
	bool canPickUp_W_L(CardArrayBase&v,uval vBegin,size_t w, size_t l);
	/*
	*@brief 提取顺子
	*@param CardArrayBase & v
	*@param uval vBegin
	*@param size_t w
	*@param size_t l
	*@return bool
	*/
	bool canPickUp_Seq_N(CardArrayBase&v,uval vBegin,size_t w, size_t l);

	/*
	*@brief:提取普通炸弹
	*@Returns:   bool
	*@Parameter: CardArrayBase & v
	*@Parameter: uval vBegin
	*@Parameter: size_t w
	*/
	bool canPickUp_Bomb_N(CardArrayBase&v,uval vBegin,size_t w);

	/*
	*@brief:提取王炸
	*@Returns:   bool
	*@Parameter: CardArrayBase & V
	*@Parameter: size_t w
	*/
	bool canPickUp_Jokers_N(CardArrayBase&V,size_t w);

	/*
	*@brief:	提取指定牌值的牌
	*@Returns:   CardArrayBase
	*@Parameter: vector<uval> vals
	*/
	bool pickUpByCardVal(CardArrayBase&V,vector<uval> vals);

	/*
	*@brief:牌型分类
	*@Returns:   bool
	*@Parameter: vector<CardArrayBase> & v
	*/
	bool classifyByType(vector<CardArrayBase>&v);

	/*
	*@brief:提取包含方片4所有分类组合
	*@Returns:   void
	*@Parameter: uchar (*pCard)[ONE_HAND_CARD_COUNT],uchar& CardCount
	*/
	void promptD_4Type(uchar (*pCard)[ONE_HAND_CARD_COUNT],uchar& CardCount);

	/*
	*@brief:提取所有分类组合
	*@Returns:   void
	*@Parameter: vector<CardArrayBase> & v
	*/
	void promptType(uchar (*pCard)[ONE_HAND_CARD_COUNT],uchar& CardCount);

	/*
	*@brief:提取符合的所有分类组合
	*@Returns:   void
	*@Parameter: CardArrayBase &lastCard
	*/
	void promptType(CardArrayBase &lastCard,uchar (*pCard)[ONE_HAND_CARD_COUNT],uchar& CardCount);

	/*
	*@brief:提取方片4对子、3条、4条组合系列
	*@Returns:   bool
	*@Parameter: vector<CardArrayBase> & v
	*/

	void canPickD_4Compose(vector<CardArrayBase> &v,uval vSrc,size_t w);

	/*
	*@brief:提取 单张、对子、3条、4条组合系列
	*@Returns:   bool
	*@Parameter: vector<CardArrayBase> & v
	*/

	void canPickComposes(vector<CardArrayBase> &v,uval vSrc,size_t w);

	/*
	*@brief:提取包含方片4为主牌 3带2、4带1组合系列
	*@Returns:   bool
	*@Parameter: vector<CardArrayBase> & v
	*/
	void _3w2Or4w1PickD_4Compose(vector<CardArrayBase> &v,uval vSrc,size_t w);

	/*
	*@brief:提取3带2、4带1包含方片4次牌组合系列
	*@Returns:   bool
	*@Parameter: vector<CardArrayBase> & v
	*/

	void _3w2Or4w1D_4Compose(vector<CardArrayBase> &v,size_t w);

	/*
	*@brief:提取包含3带2、4带1组合系列
	*@Returns:   bool
	*@Parameter: vector<CardArrayBase> & v
	*/

	void _3w2Or4w1Compose(vector<CardArrayBase> &v,size_t w);

	/*
	*@brief:提取方片4子顺子组合系列
	*@Returns:   bool
	*@Parameter: vector<CardArrayBase> & v
	*/

	void seqPickD_4Compose(vector<CardArrayBase> &v,uval vSrc,size_t w);

	
	/*
	*@brief:提取顺子组合系列
	*@Returns:   void
	*@Parameter: vector<CardArrayBase> & v
	*/

	void seqPickComposes(vector<CardArrayBase> &v,size_t w);

	/*
	*@brief:提取指定牌的全部个数;
	*@Returns:   void
	*@Parameter: CardBase & v
	*/
	void getMultiCard(vector<CardBase> &vCard,CardBase &v);

	/*
	*@brief:提取同花包含方片4组合系列
	*@Returns:   bool
	*@Parameter: vector<CardArrayBase> & v
	*/

	void samePickD_4Compose(vector<CardArrayBase> &v,uval vSrc,size_t w);

	/*
	*@brief:提取同花包含组合系列
	*@Returns:   void
	*@Parameter: vector<CardArrayBase> & v
	*/

	void samePickCompose(vector<CardArrayBase> &v,uval vSrc,size_t w);

	/*
	*@brief:提取同花顺组合系列
	*@Returns:   void
	*@Parameter: vector<CardArrayBase> & v
	*/

	void sameSeqPickD_4Compose(vector<CardArrayBase> &v,uval vSrc,size_t w);

	/*
	*@brief:提取同花顺组合系列
	*@Returns:   void
	*@Parameter: vector<CardArrayBase> & v
	*/

	void sameSeqPickCompose(vector<CardArrayBase> &v,uval vSrc,size_t w);

	/*
	*@brief:提取炸弹
	*@Returns:   bool
	*@Parameter: vector<CardArrayBase> & v
	*/
	bool canPickUpBombs(vector<CardArrayBase>&v);


	/*
	*@brief:提取三代二
	*@Returns:   bool
	*@Parameter: CardArrayBase & v
	*@Parameter: uval vBegin
	*@Parameter: bool full
	*/
	bool canPickUp_3W2(CardArrayBase&v,uval vBegin,bool full);
	/*
	*@brief:提取飞机
	*@Returns:   bool
	*@Parameter: CardArrayBase & v
	*@Parameter: uval vBegin
	*@Parameter: bool full
	*/
	bool canPickUp_Plane(CardArrayBase&v,uval vBegin,bool full);

	/*
	*@brief 提取510K
	*@param CardArrayBase & V
	*@param size_t w
	*@return bool
	*/
	bool canPickUp_WSQ_N(CardArrayBase&V,size_t w);
	/*
	*@brief 按花色提取510K
	*@param CardArrayBase & V
	*@param emCardColor type
	*@return bool
	*/
	bool canPickUp_WSQ_T(CardArrayBase&V,emCardColor type);
	/*
	*@brief 提取对应类型的牌
	*@param CardArrayBase::eArrayType type
	*@param vector<CardArrayBase> & v
	*@return bool
	*/
	bool canPickUp(eArrayType type,vector<CardArrayBase>&v);
private:

	/*
	*@brief 牌组是否相等
	*@param const CardBase & c
	*@return bool
	*/
	virtual bool _equal(const CardArrayBase& v)const;
	/*
	*@brief 牌组牌型比较【用于牌型排序】
	*@param const CardArrayBase & v
	*@return HN::CardArrayBase::uType
	*/
	virtual  bool _lessThan(const CardArrayBase& v)const;
	/*
	*@brief 牌组牌型比较【用于出牌判断】
	*@param const CardArray & v
	*@return bool
	*/
	virtual bool _lessThanR(const CardArrayBase& v)const;
	/*
	*@brief 牌组牌型比较【用于比较提牌的权值】
	*@param const CardArray & v
	*@return bool
	*/
	virtual bool _lessThanN(const CardArrayBase& v)const;
	/*
	*@brief 是否同牌值（数值）
	*@return bool
	*/
	virtual bool _isSameVal()const;

	/*
	*@brief 是否同类型（花色）
	*@return bool
	*/
	virtual bool _isSameType()const;

	/*
	*@brief 是否完全相同的牌
	*@return bool
	*/
	virtual bool _isSame()const;

	/*
	*@brief 值是否是顺序的
	*@return bool
	*/
	virtual bool _isSequence(uval &v,char&t)const;

	/*
	*@brief 是否是同色顺序
	*@return bool
	*/
	virtual bool _isSameSeq(uval &v,char &t)const;

	/*
	*@brief 单张
	*@return bool
	*/
	virtual bool _isSingle()const;

private:
	vector<CardBase> _vCard;
};

#endif//ARRAY_CARD_BASE_H
