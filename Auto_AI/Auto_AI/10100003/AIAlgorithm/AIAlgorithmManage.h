/****************************************************************************
Copyright (c) 2014-2017 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2017 ShenZhen Redbird Network Polytron Technologies Inc.
 
http://www.hotniao.com

All of the content of the software, including code, pictures, 
resources, are original. For unauthorized users, the company 
reserves the right to pursue its legal liability.
****************************************************************************/
////  算法工厂类  管理着不同的 AI出牌算法
#pragma once
#include "IAIAlgorithm.h"
#include "HandsNumberAndWeight.h"
#include "..\Card\CardArrayBase.h"
#include "def.h"

namespace HN_AI
{
	class CAIAlgorithmManage
	{
		public:
			CAIAlgorithmManage(void);
			~CAIAlgorithmManage(void);
		public:
			//检测组牌，可根据需求添加不同的iType
			bool CheckType(HN::CardArrayBase& tmp, int iType);
			//设置是首出检查还是跟牌检查
			void SetFirstFlag(bool bFirst);


			/*
			*@brief 设置天癞子牌
			*@param CardBase byCard
			*@return bool
			*/
			bool SetTianLaiZiCard(HN::CardBase byCard);
			HN::CardBase GetTianLaiZiCard();
			/*
			*@brief 设置地癞子牌
			*@param CardBase byCard
			*@return bool
			*/
			bool SetDiLaiZiCard(HN::CardBase byCard);
			HN::CardBase GetDiLaiZiCard();
			/*
			*@brief 清理癞子
			*@return bool
			*/
			bool ClearLaiZi();
		public:
			//// 配置文件  /////////////////////////////
			SSysConfig GetCardRulesConfig();
			void SetCardRulesConfig(const SSysConfig & tSysConfig);
			//// 获取牌类型 ////////////////////////////
			/*
			*@brief 底牌牌型
			*@return EBackArrayType
			*/
			EBackArrayType GetBackType(const HN::CardArrayBase & tSrcArray) const;
			/*
			*@brief 检查牌型是否正确
			*@param tCard 出牌
			*@return bool
			*/
			bool bCheckCardType(const T_C2S_PLAY_CARD_REQ & tCard);
			/*
			*@brief 牌型识别 出牌 把某一张牌假设为癞子牌后 可以组合的所有类型牌
			*@return bool
			*/
			bool GetTypeListToPut(const HN::CardArrayBase & tSrcArray,std::vector<T_C2S_PLAY_CARD_REQ> & tCardTypeList)const;
			/*
			*@brief 牌型识别 出牌 把某一张牌假设为癞子牌后 可以组合的所有类型牌 每种类型只需最大最小 四癞需要变成 四个2带牌 或者 四个3带牌
			*@return bool
			*/
			bool GetTypeListToPutByMaxMin(const HN::CardArrayBase & tSrcArray,std::vector<T_C2S_PLAY_CARD_REQ> & tCardTypeList)const;

			/*
			*@brief 牌型识别 跟牌 把某一张牌假设为癞子牌后 可以组合的所有类型牌
			*@return bool
			*/
			bool GetTypeListToFollow(const HN::CardArrayBase & tSrcArray,const T_C2S_PLAY_CARD_REQ & tLastCard,std::vector<T_C2S_PLAY_CARD_REQ> & tCardTypeList)const;
			/*
			*@brief 牌型识别 跟牌 把某一张牌假设为癞子牌后 可以最大组合的类型牌
			*@return bool
			*/
			bool GetMaxTypeToFollow(const HN::CardArrayBase & tSrcArray,const T_C2S_PLAY_CARD_REQ & tLastCard,T_C2S_PLAY_CARD_REQ & tCardType)const;
			//// 出牌算法 //////////////////////////////
			/*
			*@brief  获取自动出牌组合
			*@param  tSrcArray 原数据
			*@param  tPutCardList 组合结果 
			*@return bool 可以跟则返回true
			*/
			bool GetPutArrayList(const HN::CardArrayBase & tSrcArray,std::vector<T_C2S_PLAY_CARD_REQ> & tPutCardList)const;
			/*
			*@brief  获取自动出牌组合 从小到大 第一手牌
			*@param  tSrcArray 原数据
			*@param  tPutCardList 组合结果 
			*@return bool 可以跟则返回true
			*/
			bool GetPutArrayListByInc(const HN::CardArrayBase & tSrcArray,std::vector<T_C2S_PLAY_CARD_REQ> & tPutCardList)const; 
			//// 跟牌算法  ///////////////////////////////
			/*
			*@brief 获取能跟牌的组合  组合按癞子数的大小排序 
			*@param HN::CardArrayBase & tSrcArray
			*@param T_C2S_PLAY_CARD_REQ & tLastCard  std::vector<T_C2S_PLAY_CARD_REQ> & tFollowArrayList
			*@return bool 可以跟则返回true
			*/
			bool GetFollowArrayList(const HN::CardArrayBase & tSrcArray,const T_C2S_PLAY_CARD_REQ & tLastCard,std::vector<T_C2S_PLAY_CARD_REQ> & tFollowArrayList) const;
			/*
			*@brief 获取能跟牌的组合 按打出这个组合后 手数的大小和权值 排序
			*@param HN::CardArrayBase & tSrcArray
			*@param T_C2S_PLAY_CARD_REQ & tLastCard std::vector<T_C2S_PLAY_CARD_REQ> & tFollowArrayList 
			*@return bool 可以跟则返回true
			*/
			bool GetFollowArrayListByHands(const HN::CardArrayBase & tSrcArray,T_C2S_PLAY_CARD_REQ tLastCard,std::vector<T_C2S_PLAY_CARD_REQ> & tFollowArrayList) const;        

			void CleanBomb(std::vector<T_C2S_PLAY_CARD_REQ> &tFollowArrayList) const;

			/*
			*@brief 获取能跟牌的组合  从小到大
			*@param HN::CardArrayBase & tSrcArray
			*@param T_C2S_PLAY_CARD_REQ & tLastCard std::vector<T_C2S_PLAY_CARD_REQ> & tFollowArrayList 
			*@return bool 可以跟则返回true
			*/
			bool GetFollowArrayListByInc(const HN::CardArrayBase & tSrcArray,const T_C2S_PLAY_CARD_REQ & tLastCard,std::vector<T_C2S_PLAY_CARD_REQ> & tFollowArrayList) const;
			//// 公共方法 ///////////////////
			/*
			*@brief 比较两组牌
			*@param -1无法比较 0:相等 1:大于 2:小于
			*@return int
			*/
			int CompareCard(const T_C2S_PLAY_CARD_REQ & tA,const T_C2S_PLAY_CARD_REQ & tB)const; 
			/*
			*@brief tA 在 tSrcArray 是不是用完所有相同的牌
			*@param 如: tA 3  如果在 tSrcArray 只有一个3 true 如果有两个或者两个以上 则 false
			*@return bool 
			*/
			bool IsOnlyCardByArray(const T_C2S_PLAY_CARD_REQ & tA,const HN::CardArrayBase & tSrcArray)const; 
       private:
			///// 牌型获取 ////////////////////////////////////////////////////////
			/*
			*@brief 牌型识别 主动出牌 把某一张牌假设为癞子牌后 可以组合的所有类型牌
			*@return bool
			*/
			bool GetTypeList(const HN::CardArrayBase & tSrcArray,std::vector<HN_AI::SCombinationNode> & tCardNodeList)const;
			//// 主动出牌算法 ///////////////////////////
			/*
			*@brief 获取自动出牌组合 
			*@param  tSrcArray 原数据
			*@param  SCombinationResult & sCombinationResul
			*@return bool 可以跟则返回true
			*/
			bool GetPutArrayList(const HN::CardArrayBase & tSrcArray,SCombinationResult & sCombinationResul)const; 
			/*
			*@brief 获取自动出牌组合
			*@param  tSrcArray 原数据
			*@param  SCombinationResult & sCombinationResul
			*@param  HN_AI::SSysConfig tSysConfig 配置
			*@return bool 可以跟则返回true
			*/
			bool GetPutArrayList(const HN::CardArrayBase & tSrcArray,SCombinationResult & sCombinationResul,HN_AI::SSysConfig tSysConfig)const;
			//// 跟牌算法 ///////////////////////////////
			/*
			*@brief 获取能跟牌的组合
			*@param const HN_AI::SCombinationNode & tLastNode  std::vector<SCombinationNode> & tFollowArrayList 
			*@return bool 可以跟则返回true
			*/
		    bool GetFollowArrayList(const HN::CardArrayBase & tSrcArray,const HN_AI::SCombinationNode & tLastNode,std::vector<HN_AI::SCombinationNode> & tFollowArrayList) const;
			/*
			*@brief 获取能跟牌的组合 拆组合
			*@param const HN_AI::SCombinationNode & tLastNode  std::vector<SCombinationNode> & tFollowArrayList 
			*@return bool 可以跟则返回true
			*/
			bool GetFollowListByRemove(const HN::CardArrayBase & tSrcArray,const HN_AI::SCombinationNode & tLastNode,std::vector<HN_AI::SCombinationNode> & tFollowArrayList)const;
			/*
			*@brief 获取能跟牌的组合 癞子本身
			*@param bFlag false:结果不需要包含全部手牌 true：结果必须包含全部手牌
			*@param const HN_AI::SCombinationNode & tLastNode std::vector<HN_AI::SCombinationNode> & tFollowArrayList
			*@return bool 可以跟则返回true
			*/
			bool GetFollowNodeByLaiZiValue(const HN::CardArrayBase & tSrcArray,const HN_AI::SCombinationNode & tLastNode,std::vector<HN_AI::SCombinationNode> & tFollowArrayList,bool bFlag=false)const;
			/*
			*@brief 获取能跟牌的组合 在最优组合中
			*@param const HN_AI::SCombinationNode & tLastNode  std::vector<SCombinationNode> & tFollowArrayList 
			*@return bool 可以跟则返回true
			*/
			bool GetFollowListByNodeList(const HN::CardArrayBase & tSrcArray,const HN_AI::SCombinationNode & tLastNode,std::vector<HN_AI::SCombinationNode> & tFollowArrayList)const;
			///////// 底牌类型判断算法 ///////////////////////////////////////////////////
			bool isOneKing(const HN::CardArrayBase & tSrcArray)const;
			bool isTwoKing(const HN::CardArrayBase & tSrcArray)const;
			bool isShunZi(const HN::CardArrayBase & tSrcArray)const;
			bool isTongHua(const HN::CardArrayBase & tSrcArray)const;
			bool isSanZhang(const HN::CardArrayBase & tSrcArray)const;
			//// 公共算法 ///////////////////////////////
			/*
			*@brief 去除手牌中所有的癞子 tSrcArray去除所有癞子和花色后保存在bySrcData中 iSrcLen：新数组长度 iLaiZiNumber：癞子个数
			*@param const HN::CardArrayBase & tSrcArray,unsigned char *bySrcData,int & iLen,int & iLaiZiNumber
			*@return void
			*/
			void GetSrcToNoLaiZi(const HN::CardArrayBase & tSrcArray,unsigned char *bySrcData,int & iSrcLen,int & iLaiZiNumber)const;
			/*
			*@brief 手牌去花色
			*@param const T_C2S_PLAY_CARD_REQ & tPlayCard
			*@return void
			*/
			void GetPlayCardVal(T_C2S_PLAY_CARD_REQ & tPlayCard)const;
			/*
			*@brief 手牌去花色
			*@param const HN::CardArrayBase & tSrcArray,unsigned char *bySrcData,int & iSrcLen
			*@return void
			*/
			void GetPlayCardVal(const HN::CardArrayBase & tSrcArray,unsigned char *bySrcData,int & iSrcLen)const;
			/*
			*@brief 组牌结果添加花色 
			*@param  const HN::CardArrayBase & tSrcArray,std::vector<SCombinationNode> & vecCombinationResult,
			*@param  bool bIsCycle 每一组组合是否重新获取手牌
			*@return bool
			*/
			bool AddRealCardToNode(const HN::CardArrayBase & tSrcArray,std::vector<SCombinationNode> & tNodeList,bool bIsCycle = false)const;
			/*
			*@brief 花色添加
			*@param  const unsigned char *pData,int iLen,SCombinationNode & tCombinationNode
			*@return bool
			*/
			bool ChangeResultDataToSrc(unsigned char *pData,int iLen,SCombinationNode & tCombinationNode)const;
			/*
			*@brief 牌列表转换 癞子添加到结果
			*@param  std::vector<HN_AI::SCombinationNode> tCardList,std::vector<CardArrayBase> & tArrayList
			*@param  bIsFollow 是否是跟牌
			*@return void
			*/
			void AddLaiZiCardToArrayList(const std::vector<SCombinationNode> & tCardList,const std::vector<HN::CardBase> & tLaiZiList,std::vector<T_C2S_PLAY_CARD_REQ> & tArrayList,bool bIsFollow)const;
		    /*
			*@brief 牌型转换
			*@param  SCombinationNode & sCombinationNode
			*@return EArrayType
			*/
			EArrayType AICardTypeToArrayType(const SCombinationNode & sCombinationNode) const;
			/*
			*@brief 结构体转换
			*@param const T_C2S_PLAY_CARD_REQ & tPlayCard ,HN_AI::SCombinationNode & tCombinationNode
			*@return void
			*/
			void CPlayCardToNode(const T_C2S_PLAY_CARD_REQ & tPlayCard,HN_AI::SCombinationNode & tCombinationNode)const;
			/*
			*@brief 获取手牌中所有的癞子
			*@param const HN::CardArrayBase & tSrcArray,std::vector<HN::CardBase> & tLaiZiList
			*@return void
			*/
			void GetLaiZiListBySrc(const HN::CardArrayBase & tSrcArray,std::vector<HN::CardBase> & tLaiZiList)const;
	    private:
			IAIAlgorithm *m_pCurAlgorithm;
			HN::CardBase m_tianLaiZi;
			HN::CardBase m_diLaiZi;
	};
}