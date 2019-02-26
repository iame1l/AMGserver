#ifndef CFG_CARD_H
#define CFG_CARD_H
#include <vector>

namespace HN
{
	class CfgCard;
	
	typedef  unsigned char ucard;
	typedef  std::vector<ucard> vCard;
	typedef  bool (CfgCard::*pMakeCard)(vCard &card);

	const unsigned int  MAX_CARD_TYPE_NUM  = 1;
	const unsigned int  PAIR_CARD_NUM	   = 48;
	const unsigned int  HAND_CARD_NUM	   = 16 ;

	class CfgCard
	{

	public:
		CfgCard(void);
		~CfgCard(void);
		bool makeCard(vCard & card,int typeNum);
	private:
			/*
			一共 个牌型
			*/
		bool makeCard0(vCard &card);

	public:
		/*
		*@brief 生成[_Min,_Max]随机数
		*@param const int _Min
		*@param const int _Max
		*@return int
		*/
		int rand_Mersenne(const int _Min,const int _Max);

		/*
		*@brief 打乱牌
		*@param vCard & card
		*@return void
		*/
		void randCard(vCard& card);

		/*
		*@brief src删除v
		*@param vCard & src
		*@param const vCard & v
		*@return void
		*/
		void Remove(vCard& src,const vCard& v);

		/*
		*@brief src添加v
		*@param vCard & src
		*@param const vCard & v
		*@return void
		*/
		void Add(vCard& src,const vCard& v);

		/*
		*@brief 随机取N个值
		*@param const vCard & srcCard
		*@param vCard & dst
		*@param size_t len
		*@return void
		*/
		void randSel(const vCard&srcCard,vCard& dst,size_t len);

		/*
		*@brief 随机取N个炸弹
		*@param vCard & card
		*@param size_t len
		*@return void
		*/
		void randSelBomb(vCard& card,size_t len);

		/*
		*@brief 随机取N个固定牌值
		*@param vCard & card
		*@param ucard uVal
		*@param size_t len
		*@return void
		*/
		void CfgCard::randSelVal(vCard& card,ucard uVal,size_t len);

		//组排函数数组
		static  pMakeCard _makeCard[MAX_CARD_TYPE_NUM];


		/*
		*@brief 组牌之前成员数据初始化
		*@return void
		*/
		void preMakeCard();

		/*
		*@brief 组剩余随机牌
		*@param vCard & card
		*@return void
		*/
		void nxtMakeCard(vCard & card);

		/*
		*@brief vector 转为数组
		*@param const vCard & v
		*@param ucard * uCard
		*@param size_t len
		*@return void
		*/
		void reassign(const vCard& v,ucard *uCard,size_t len);

		/*
		*@brief 从牌源剩余牌中取N个牌
		*@param vCard & nxtCard
		*@param size_t len
		*@return bool
		*/
		bool nxtSel(vCard & nxtCard,size_t len);
private:
		//一副牌数据
		static  ucard _baseCard[PAIR_CARD_NUM];

		//牌源临时变量
		vCard  _tCard;

		//固定选择的牌值
		vCard  _SelCard;
	};

}
// 修改指定玩家的牌
bool GetSpecifiedCard(long int userID, unsigned char *Card,int jCardNum);

#endif//CFG_CARD_H
