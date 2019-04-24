/****************************************************************************
Copyright (c) 2014-2017 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2017 ShenZhen Redbird Network Polytron Technologies Inc.
 
http://www.hotniao.com

All of the content of the software, including code, pictures, 
resources, are original. For unauthorized users, the company 
reserves the right to pursue its legal liability.
****************************************************************************/
//////// 算法接口类 ///////////////////////////////////
#pragma once
#include <vector>
namespace HN_AI
{
	enum ECardType
	{
		eCardType_Unknown=0,	/// 未知类型
		eCardType_DanZhang=1,	/// 单张
		eCardType_ShunZi,		/// 顺子
		eCardType_DuiZi,		/// 对子
		eCardType_LianDui,		/// 连对
		eCardType_SanZhang,		/// 3张
		eCardType_FeiJi,		/// 飞机
		eCardType_ZhaDan,	    /// 炸弹
	};
	enum EAddCardType	///  附加牌类型
	{
		eAddCardType_Unknown=0,	///  未知类型
		eAddCardType_No=1,		///  不带附加牌
		eAddCardType_DanZhang,	///  单张
		eAddCardType_DuiZi,	    ///  对子
	};
	struct SSysConfig
	{
		bool bIsWBomb_Separate;		/// 王炸是否可以拆开
	    bool bIs3W1_One;			/// 是否三带 1单张
		bool bIs3W1_Double;			/// 是否三带 1对子
		bool bIs4W2_One;			/// 是否四带 2单张
		bool bIs4W2_Double;			/// 是否四带 2对子
		bool bIsLongBomb;			/// 是否支持长炸弹 大于5的炸弹 属于长炸弹
        int  iBombCompareType;      /// 炸弹比较类型
		SSysConfig()
		{
			bIsWBomb_Separate = true;   /// 王炸是否可以拆开
			bIs3W1_One = true;			/// 是否三带 1单张
			bIs3W1_Double  = true;      /// 是否三带 1对子
			bIs4W2_One  = true;			/// 是否四带 2单张
			bIs4W2_Double  = true;      /// 是否四带 2对子
			bIsLongBomb = false;		/// 是否支持长炸弹 大于5的炸弹 属于长炸弹
			iBombCompareType = 0;		/// 炸弹比较类型 0:12软炸 > .... > 5纯 >5软 >四癞 > 王炸 > 硬炸 > 软炸  1:王炸 > 12软炸 > .... > 5纯 >5软 > 四癞 > 硬炸 > 软炸
		}
	};
	struct SCombinationNode    /// 组牌结果节点
	{
		ECardType eCardType;
		EAddCardType eAddCardType;	//// 副牌类型
		std::vector<unsigned char> vecMainCard;  /// 主牌 如:333
		std::vector<unsigned char> vecSubCard;   /// 副牌 如:4         333 + 4 
		std::vector<unsigned char> vecLaiZiCard; /// 癞子牌
        SCombinationNode()
		{
			clear();
		}
        void clear()
		{
			eCardType = eCardType_Unknown;
			eAddCardType = eAddCardType_No;
			vecMainCard.clear();
			vecSubCard.clear();
			vecLaiZiCard.clear();
		}
	};
	struct SCombinationResult	/// 组牌结果
	{
		int iMaxValue;           /// 最大权值
		int iMinHandsNumber;     /// 最少手数
		BYTE byTypeCount[8];	 /// 每个类型的数量
		int iLaiZiNumber;		 /// 癞子数
		std::vector<SCombinationNode>  vecCombinationResult;        ///组牌结果
		std::vector<SCombinationNode>  vecResultNoWith;				///组牌结果，没有带牌
		SCombinationResult()
		{
			clear();
		}
		void clear()
		{
			iMaxValue =0;						 /// 最大权值
			iMinHandsNumber =0;					 /// 最少手数
			memset(byTypeCount, 0, sizeof(byTypeCount));
			iLaiZiNumber =0;					 /// 癞子数
			vecCombinationResult.clear();        /// 组牌结果
			vecResultNoWith.clear();			 /// 组牌结果，没有带牌
		}
	};
	class IAIAlgorithm
	{
	public:
		IAIAlgorithm(){ }  
		virtual ~IAIAlgorithm(){ }  
	public:
		virtual void SetFirstFlag(bool bFirst) = 0;
		virtual void SetCheckType(int iType) = 0;
		virtual int GetCheckType() = 0;
		/// 主动出牌算法
		virtual bool GetPutCardList(const unsigned char *pSrcCard,int iSrcCardLen,std::vector<SCombinationNode> & vecCombinationResult) = 0;
		virtual bool GetPutCardList(const unsigned char *pSrcCard,int iSrcCardLen,SCombinationResult & sCombinationResult) = 0;
		/*
		* pSrcCard 没有癞子的手牌
		* int iLaiZiNumber 只需癞子个数 不需癞子牌
		*/
		virtual bool GetPutCardList(const unsigned char *pSrcCard,int iSrcCardLen,int iLaiZiNumber,std::vector<SCombinationNode> & vecCombinationResult) = 0;
		virtual bool GetPutCardList(const unsigned char *pSrcCard,int iSrcCardLen,int iLaiZiNumber,SCombinationResult & sCombinationResult) = 0;

		/// 跟牌算法
		virtual bool GetFollowCardList(const unsigned char *pSrcCard,int iSrcCardLen,const SCombinationNode & tLastCard,std::vector<SCombinationNode> & vecFollowCardList) = 0;
		virtual bool GetFollowCardList(const unsigned char *pSrcCard,int iSrcCardLen,const unsigned char *pLastCard,int iLastCardLen,std::vector<SCombinationNode> & vecFollowCardList) = 0;
		/*
		* pSrcCard 没有癞子的手牌
		* int iLaiZiNumber 只需癞子个数 不需癞子牌
		*/
		virtual bool GetFollowCardList(const unsigned char *pSrcCard,int iSrcCardLen,const SCombinationNode & tLastCard,int iLaiZiNumber,std::vector<SCombinationNode> & vecFollowCardList) = 0;
		
		/// 叫分算法
		virtual int GetCallScore(const unsigned char *pSrcCard,int iSrcCardLen,int iMinScore,int iMaxScore)=0;
		/// 配置
		virtual void SetCardRulesConfig(const SSysConfig & tSysConfig) = 0;
		virtual SSysConfig GetCardRulesConfig() = 0;
//////// 辅助函数 ///////////////////////////
		//// 从 tSrcList 筛选出 比tFilterNode 大的结果 保存在 tDstList 中
		virtual bool GetNodeListByFilter(const std::vector<SCombinationNode> & tSrcList,std::vector<SCombinationNode> & tDstList,const SCombinationNode & tFilterNode) = 0;
		/// 获取牌类型
		virtual bool GetCardType(const unsigned char *pSrcCard,int iSrcCardLen,SCombinationNode & tCardType)=0;
		virtual bool GetCardType(const unsigned char *pSrcCard,int iSrcCardLen,int iLaiZiLen,std::vector<SCombinationNode> & tCardList) = 0;
		virtual int GetMainByCombinationNode(unsigned char *pSrcCard,int iSrcCardLen,const SCombinationNode & tCombinationNode ) = 0;
		virtual int GetSubByCombinationNode(unsigned char *pSrcCard,int iSrcCardLen,const SCombinationNode & tCombinationNode) = 0;
		virtual int GetMainAndSubByCombinationNode(unsigned char *pSrcCard,int iSrcCardLen,const SCombinationNode & tCombinationNode) = 0;
		/// 比较两Node的大小 -1:不能比较 0:相等 1:tNode1 > tNode2  2：tNode1 < tNode2
		virtual int CompareNodeFun(const SCombinationNode & tNode1,const SCombinationNode & tNode2)const =0;
	};
}


