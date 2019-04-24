/****************************************************************************
Copyright (c) 2014-2017 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2017 ShenZhen Redbird Network Polytron Technologies Inc.
 
http://www.hotniao.com

All of the content of the software, including code, pictures, 
resources, are original. For unauthorized users, the company 
reserves the right to pursue its legal liability.
****************************************************************************/
#include "StdAfx.h"
#include "HandsNumberAndWeight.h"
#include "AIAlgorithmManage.h"
#include "AIAlgorithmCommom.h"

namespace HN_AI
{
	CAIAlgorithmManage::CAIAlgorithmManage(void)
	{
		m_pCurAlgorithm = new CHandsNumberAndWeight;
		m_tianLaiZi = HN::CardBase::NIL;
		m_diLaiZi = HN::CardBase::NIL;
	}

	CAIAlgorithmManage::~CAIAlgorithmManage(void)
	{
		delete m_pCurAlgorithm;
	}

	bool CAIAlgorithmManage::CheckType(HN::CardArrayBase& tmp, int iType)
	{
		std::vector<T_C2S_PLAY_CARD_REQ>  tPlayCardList; 

		m_pCurAlgorithm->SetCheckType(iType);
		GetPutArrayList(tmp,tPlayCardList);
		if(m_pCurAlgorithm->GetCheckType() == SuccessCheck)		
		{
			m_pCurAlgorithm->SetCheckType(InvalidCheck);
			return true;
		}
		else
		{
			m_pCurAlgorithm->SetCheckType(InvalidCheck);
			return false;
		}
	}

	void CAIAlgorithmManage::SetFirstFlag(bool bFirst)
	{
		m_pCurAlgorithm->SetFirstFlag(bFirst);
	}

	bool CAIAlgorithmManage::SetTianLaiZiCard(HN::CardBase byCard)
	{
		bool ret = byCard.isValid();
		if(ret)
		{
			m_tianLaiZi = byCard;
			return true;
		}
		return false;
	}

	HN::CardBase CAIAlgorithmManage::GetTianLaiZiCard()
	{
		return m_tianLaiZi;
	}

	bool CAIAlgorithmManage::SetDiLaiZiCard(HN::CardBase byCard)
	{
		bool ret = byCard.isValid();
		if(ret)
		{
			m_diLaiZi = byCard;
			return true;
		}
		return false;
	}

	HN::CardBase CAIAlgorithmManage::GetDiLaiZiCard()
	{
		return m_diLaiZi;
	}

	bool CAIAlgorithmManage::ClearLaiZi()
	{
		m_tianLaiZi = HN::CardBase::NIL;
		m_diLaiZi = HN::CardBase::NIL;
		return true;
	}
	//// 配置文件 ///////////////////////////////////////
	HN_AI::SSysConfig CAIAlgorithmManage::GetCardRulesConfig()
	{
		return  m_pCurAlgorithm->GetCardRulesConfig();
	}
	void CAIAlgorithmManage::SetCardRulesConfig(const SSysConfig & tSysConfig)
	{
		m_pCurAlgorithm->SetCardRulesConfig(tSysConfig);
	}
	//// 牌型获取 //////////////////////////////
	bool CAIAlgorithmManage::bCheckCardType(const T_C2S_PLAY_CARD_REQ & tCard)
	{
		SCombinationNode  tNode;
		CPlayCardToNode(tCard,tNode);
		EArrayType tType = AICardTypeToArrayType(tNode);
		bool ret = ( tType == tCard.eArrayType);
		//// 特殊牌型处理
		if( tCard.eArrayType == ARRAY_TYPE_PLANE_ONE && tType == ARRAY_TYPE_4w2_DOUBLE 
			&& HN::CardBase(tNode.vecSubCard[0]).getVal() ==  HN::CardBase(tNode.vecSubCard[2]).getVal() )
		{
		    ret = true;
		}
		return ret;
	}
	EBackArrayType CAIAlgorithmManage::GetBackType(const HN::CardArrayBase & tSrcArray) const
	{
		if ( tSrcArray.Size() != BACK_CARD_COUNT )
		{
			return BACK_ARRAY_ERROR;
		}
		if( isOneKing(tSrcArray) == true )
		{
			return BACK_ARRAY_ONEKING;
		}
		if( isTwoKing(tSrcArray) == true )
		{
			return BACK_ARRAY_TWOKING;
		}
		if( isShunZi(tSrcArray) == true )
		{
			return BACK_ARRAY_SHUNZI;
		}
		if( isTongHua(tSrcArray) == true)
		{
			return BACK_ARRAY_TONGHUA;
		}
		if( isSanZhang(tSrcArray) == true )
		{
			return BACK_ARRAY_SANZHANG;
		}
		return BACK_ARRAY_ERROR;
	}
	bool CAIAlgorithmManage::GetTypeListToPut(const HN::CardArrayBase & tSrcArray,std::vector<T_C2S_PLAY_CARD_REQ> & tCardTypeList) const
	{
		std::vector<SCombinationNode>  tCardNodeList;
		GetTypeList(tSrcArray,tCardNodeList);
		/// 结果添加花色
		AddRealCardToNode(tSrcArray,tCardNodeList,true);
		/// 获取癞子牌
		std::vector<HN::CardBase> tLaiZiList;
		GetLaiZiListBySrc(tSrcArray,tLaiZiList);
		/// 结果转换 添加癞子牌
		AddLaiZiCardToArrayList(tCardNodeList,tLaiZiList,tCardTypeList,true);
		/// 结果返回
		if( tCardTypeList.size() > 0)
		{
			return true;
		}
		return false;
	}

	bool CAIAlgorithmManage::GetTypeListToPutByMaxMin(const HN::CardArrayBase & tSrcArray,std::vector<T_C2S_PLAY_CARD_REQ> & tCardTypeList) const
	{
		auto cmp = [&](const T_C2S_PLAY_CARD_REQ & tA,const T_C2S_PLAY_CARD_REQ & tB)->bool
		{
			if( tA.eArrayType != tB.eArrayType  )
			{
			    return tA.eArrayType < tB.eArrayType;
			}
			//// 比较大小
			int ret = CompareCard(tA,tB);
			if( ret == 1)
			{
				return false;
			}
			else
			{
				return true;
			}
		};
		/// 获取所有能组成的出牌
		GetTypeListToPut(tSrcArray,tCardTypeList);
		/// 把四癞子带牌变成 4个2 或者 4个3
		auto iSiLaiIter = tCardTypeList.end();
		bool bIsMin = false;
		bool bIsMax = false;
		for(auto iter = tCardTypeList.begin();iter!= tCardTypeList.end();iter++)
		{
			if(iter->eArrayType == ARRAY_TYPE_4w2_DOUBLE || iter->eArrayType == ARRAY_TYPE_4w2_ONE)
			{
				if( HN::CardBase(iter->uCards[0]).getVal() == 3 )
				{
					bIsMin = true;
				}
				if( HN::CardBase(iter->uCards[0]).getVal() == 2 )
				{
					bIsMax = true;
				}
				if( HN::CardBase(iter->uCards[0]).getVal() == m_tianLaiZi.getVal() || HN::CardBase(iter->uCards[0]).getVal() == m_diLaiZi.getVal() )
				{
					iSiLaiIter = iter;
				}
			}
		}
		if(iSiLaiIter != tCardTypeList.end() )
		{
			T_C2S_PLAY_CARD_REQ tTem = *iSiLaiIter;
			tCardTypeList.erase(iSiLaiIter);
			if( (bIsMax == true && HN::CardBase(tTem.uCards[0]).getVal() == 2)
				||  (bIsMin == true && HN::CardBase(tTem.uCards[0]).getVal() == 3)
				)
			{
				tCardTypeList.push_back(tTem);
			}
			if( bIsMax == false )
			{
				T_C2S_PLAY_CARD_REQ tMax = tTem;
				tMax.uCards[0] = 2;
				tMax.uCards[1] = 2;
				tMax.uCards[2] = 2;
				tMax.uCards[3] = 2;
				tCardTypeList.push_back(tMax);
			}
			if( bIsMin == false )
			{
				T_C2S_PLAY_CARD_REQ tMin = tTem;
				tMin.uCards[0] = 3;
				tMin.uCards[1] = 3;
				tMin.uCards[2] = 3;
				tMin.uCards[3] = 3;
				tCardTypeList.push_back(tMin);
			}
		}
		/// 从小打到大排序
		std::sort(tCardTypeList.begin(),tCardTypeList.end(),cmp);
		/// 每种类型只保留最大或者最小
		std::vector<T_C2S_PLAY_CARD_REQ>  tTypeList;
		tTypeList.clear();
		EArrayType eCurArrayType =  ARRAY_TYPE_ERROR;
		for(int i = 0;i < tCardTypeList.size();i++)
		{
			if( eCurArrayType != tCardTypeList[i].eArrayType )   /// 保存最小
			{
			   tTypeList.push_back( tCardTypeList[i] );
			   eCurArrayType = tCardTypeList[i].eArrayType;
			}
			else												 /// 保存最大
			{
			    if( i+1 < tCardTypeList.size() && tCardTypeList[i].eArrayType != tCardTypeList[i+1].eArrayType  )
				{
				    tTypeList.push_back( tCardTypeList[i] );
				}
				else if( i+1 == tCardTypeList.size() )		    ///  添加最后一个
				{
				    tTypeList.push_back( tCardTypeList[i] );
				}
			}
		}
		tCardTypeList.clear();
		tCardTypeList.insert(tCardTypeList.end(),tTypeList.begin(),tTypeList.end());
		/// 特殊处理 3 3 3 3 4 4 4 4
		if( tCardTypeList.size() == 1 && tCardTypeList[0].eArrayType == ARRAY_TYPE_4w2_DOUBLE )
		{
		    if( HN::CardBase(tCardTypeList[0].uCards[4]).getVal()  == HN::CardBase(tCardTypeList[0].uCards[6]).getVal() )
			{
				if( HN::CardBase(tCardTypeList[0].uCards[0]).getVal()  == HN::CardBase(tCardTypeList[0].uCards[4]).getVal()+1 )
				{
					T_C2S_PLAY_CARD_REQ tTmp = tCardTypeList[0];
					tTmp.eArrayType = ARRAY_TYPE_PLANE_ONE;
					tTmp.uCards[0] = tCardTypeList[0].uCards[4];
					tTmp.uCards[1] = tCardTypeList[0].uCards[5];
					tTmp.uCards[2] = tCardTypeList[0].uCards[6];
					tTmp.uActualCards[0] = tCardTypeList[0].uActualCards[4];
					tTmp.uActualCards[1] = tCardTypeList[0].uActualCards[5];
					tTmp.uActualCards[2] = tCardTypeList[0].uActualCards[6];

					tTmp.uCards[3] = tCardTypeList[0].uCards[0];
					tTmp.uCards[4] = tCardTypeList[0].uCards[1];
					tTmp.uCards[5] = tCardTypeList[0].uCards[2];
					tTmp.uActualCards[3] = tCardTypeList[0].uActualCards[0];
					tTmp.uActualCards[4] = tCardTypeList[0].uActualCards[1];
					tTmp.uActualCards[5] = tCardTypeList[0].uActualCards[2];

					tTmp.uCards[6] = tCardTypeList[0].uCards[7];
					tTmp.uCards[7] = tCardTypeList[0].uCards[3];
					tTmp.uActualCards[6] = tCardTypeList[0].uActualCards[7];
					tTmp.uActualCards[7] = tCardTypeList[0].uActualCards[3];

					tCardTypeList.push_back(tTmp);
				}
			}
		}
		/// 结果返回
		if( tCardTypeList.size() > 0)
		{
			return true;
		}
		return false;
	}

	bool CAIAlgorithmManage::GetTypeListToFollow(const HN::CardArrayBase & tSrcArray,const T_C2S_PLAY_CARD_REQ & tLastCard,std::vector<T_C2S_PLAY_CARD_REQ> & tCardTypeList) const
	{
		std::vector<SCombinationNode>  tCardNodeList;
		std::vector<SCombinationNode>  tAllCardNodeList;
		GetTypeList(tSrcArray,tAllCardNodeList);
		/// 结果选择
		SCombinationNode tLastNode;
		CPlayCardToNode(tLastCard,tLastNode);
		m_pCurAlgorithm->GetNodeListByFilter(tAllCardNodeList,tCardNodeList,tLastNode);
		/// 获取出癞子本身 能跟的牌
	    GetFollowNodeByLaiZiValue(tSrcArray,tLastNode,tCardNodeList,true);
		/// 结果添加花色
		AddRealCardToNode(tSrcArray,tCardNodeList,true);
		/// 获取癞子牌
		std::vector<HN::CardBase> tLaiZiList;
		GetLaiZiListBySrc(tSrcArray,tLaiZiList);
		/// 结果转换 添加癞子牌
		AddLaiZiCardToArrayList(tCardNodeList,tLaiZiList,tCardTypeList,true);
		/// 结果返回
		if( tCardTypeList.size() > 0)
		{
			return true;
		}
		return false;
	}
	bool CAIAlgorithmManage::GetMaxTypeToFollow(const HN::CardArrayBase & tSrcArray,const T_C2S_PLAY_CARD_REQ & tLastCard,T_C2S_PLAY_CARD_REQ & tCardType) const
	{
		std::vector<T_C2S_PLAY_CARD_REQ>  tCardTypeList;
		GetTypeListToFollow(tSrcArray,tLastCard,tCardTypeList);
		/// 四癞需要转化为四个2
		if( tLastCard.eArrayType == ARRAY_TYPE_4w2_ONE || tLastCard.eArrayType == ARRAY_TYPE_4w2_DOUBLE )
		{
			if( HN::CardBase(tLastCard.uCards[0]).getVal() != 2 )
			{
				std::vector<T_C2S_PLAY_CARD_REQ> tPutTypeList;
				GetTypeListToPutByMaxMin(tSrcArray,tPutTypeList);
				for( int i=0;i < tPutTypeList.size();i++)
				{
					if( tPutTypeList[i].eArrayType == ARRAY_TYPE_4w2_ONE || tPutTypeList[i].eArrayType == ARRAY_TYPE_4w2_DOUBLE  )
					{
					    if( HN::CardBase(tPutTypeList[i].uCards[0]).getVal() == 2 )
						{
						    tCardTypeList.push_back(tPutTypeList[i]);
						}
					}
				}
			}
		}
		if( tCardTypeList.size() == 0 )
		{
			return false;
		}
		else                                      //// 获取最大的
		{
			tCardType = tCardTypeList.front();
			for(int i=1;i<tCardTypeList.size();i++)
			{
				int ret = CompareCard(tCardType,tCardTypeList[i]);
				if( ret == 2 )
				{
					tCardType = tCardTypeList[i];
				}
			}
		}
		return true;
	}
	//// 出牌算法 //////////////////////////////
	bool CAIAlgorithmManage::GetPutArrayList(const HN::CardArrayBase & tSrcArray,std::vector<T_C2S_PLAY_CARD_REQ> & tPutCardList) const
	{
		tPutCardList.clear();
		/// 获取出牌组合
		SCombinationResult sCombinationResul;
		HN_AI::SSysConfig tPutConfig = m_pCurAlgorithm->GetCardRulesConfig();;
		tPutConfig.bIs4W2_One = false;
		tPutConfig.bIs4W2_Double = false;
		GetPutArrayList(tSrcArray,sCombinationResul,tPutConfig);
		/// 结果添加花色
		AddRealCardToNode(tSrcArray,sCombinationResul.vecCombinationResult,false);
		/// 获取癞子牌
		std::vector<HN::CardBase> tLaiZiList;
		GetLaiZiListBySrc(tSrcArray,tLaiZiList);
		/// 结果转换 添加癞子牌
		AddLaiZiCardToArrayList(sCombinationResul.vecCombinationResult,tLaiZiList,tPutCardList,false);
		/// 结果返回
		if( tPutCardList.size() > 0)
		{
			return true;
		}
		return false;
	}
	bool CAIAlgorithmManage::GetPutArrayListByInc(const HN::CardArrayBase & tSrcArray,std::vector<T_C2S_PLAY_CARD_REQ> & tPutCardList) const
	{
		std::vector<HN::CardBase> tVecCard = tSrcArray.getArray();
		for(int i= tVecCard.size()-1;i>=0;i--)
		{
		    T_C2S_PLAY_CARD_REQ tRes;
			HN::CardBase tCurCard = tVecCard[i];
			tRes.iCardCount = 0;
			do 
			{
			   i -= (tRes.iCardCount == 0?0:1);
			   tRes.uCards[tRes.iCardCount] = tVecCard[i].getData();
			   tRes.uActualCards[tRes.iCardCount] = tVecCard[i].getData();
			   tRes.iCardCount++;
			} while ( (i-1)>=0 && tCurCard.getVal() == tVecCard[i-1].getVal() );
			if(tRes.iCardCount == 1)
			{
				tRes.eArrayType = ARRAY_TYPE_SINGLE;
			}
			if(tRes.iCardCount == 2)
			{
				tRes.eArrayType = ARRAY_TYPE_DOUBLE;
			}
			if(tRes.iCardCount == 3)
			{
				tRes.eArrayType = ARRAY_TYPE_3W_;
			}
			if(tRes.iCardCount == 4)
			{
				tRes.eArrayType = ARRAY_TYPE_HBOMB;
				if( tCurCard.getVal() == m_tianLaiZi.getVal() || tCurCard.getVal() == m_diLaiZi.getVal() )
				{
				   tRes.eArrayType = ARRAY_TYPE_4L;
				}
			}
			tPutCardList.push_back(tRes);
		}
		/// 结果返回
		if( tPutCardList.size() > 0)
		{
			return true;
		}
		return false;
	}
	//// 跟牌算法 /////////////////////////////
	bool CAIAlgorithmManage::GetFollowArrayList(const HN::CardArrayBase & tSrcArray,const T_C2S_PLAY_CARD_REQ & tLastCard,std::vector<T_C2S_PLAY_CARD_REQ> & tFollowArrayList) const
	{
		//// 过滤非法数据
		if( tLastCard.eArrayType == ARRAY_TYPE_ERROR )
		{
			return false;
		}
		tFollowArrayList.clear();
		std::vector<SCombinationNode>  vecFollowCardList;
		SCombinationNode tLastNode;
		CPlayCardToNode(tLastCard,tLastNode);
		/// 获取所有的跟牌结果
		GetFollowArrayList(tSrcArray,tLastNode,vecFollowCardList);
		/// 结果添加花色
		AddRealCardToNode(tSrcArray,vecFollowCardList,true);
		/// 癞子获取
		std::vector<HN::CardBase> tLaiZiList;
		GetLaiZiListBySrc(tSrcArray,tLaiZiList);
		/// 结果转换 添加癞子牌
		AddLaiZiCardToArrayList(vecFollowCardList,tLaiZiList,tFollowArrayList,true);
		/// 结果返回
		if( tFollowArrayList.size() > 0)
		{
			return true;
		}
		return false;
	}
	//mark
	bool CAIAlgorithmManage::GetFollowArrayListByHands(const HN::CardArrayBase & tSrcArray,T_C2S_PLAY_CARD_REQ tLastCard,std::vector<T_C2S_PLAY_CARD_REQ> & tFollowArrayList) const
	{
		//// 过滤非法数据
		if( tLastCard.eArrayType == ARRAY_TYPE_ERROR )
		{
			return false;
		}
		struct SHandsNode
		{
			int iHandNumber;
			int iValue;
			BYTE	byTypeCount[8];				/// 每个类型的数量
			T_C2S_PLAY_CARD_REQ tPlayREQ;
			SHandsNode()
			{
				iHandNumber= 0;
				iValue= 0;
				memset(byTypeCount, 0, sizeof(byTypeCount));
			};
		};
		/// 获取出牌最优组合
		//SCombinationResult sNowResul;
		//GetPutArrayList(tSrcArray,sNowResul);
		/// 获取所有的跟牌结果
		GetFollowArrayList(tSrcArray,tLastCard,tFollowArrayList);
		//清楚炸弹相关的跟牌结果，防止炸弹拆开打。
		CleanBomb(tFollowArrayList);

		//// 结果挑选重新排序  按打出这个组合后 手数的大小和权值 排序
		std::vector<SHandsNode> tHandsNodeList;
		for(auto iter = tFollowArrayList.begin();iter != tFollowArrayList.end();)
		{   
			SHandsNode tHandsNode;
			tHandsNode.tPlayREQ = (*iter);
			/// 删除打出去的牌
			HN::CardArrayBase vTmp = tSrcArray;
			HN::CardArrayBase tOutCard;
			tOutCard.assign(iter->uActualCards,iter->iCardCount);
			vTmp.erase(tOutCard);
			int iBaseHandsNumber = 0;
			if(iter->eArrayType == ARRAY_TYPE_4L || iter->eArrayType == ARRAY_TYPE_LBOMB_L)
			{
			    iBaseHandsNumber += iter->iCardCount/2;
			}
			else if(iter->eArrayType == ARRAY_TYPE_WBOMB )
			{
				iBaseHandsNumber += 3;
			}
			/// 打出去牌后获取最好组合 权值和手数
			SCombinationResult tOutResul;
			HN_AI::SSysConfig tConfig = m_pCurAlgorithm->GetCardRulesConfig();
			HN_AI::SSysConfig tPutConfig = tConfig;    /// 获取最优结果是 都带牌 要不然炸弹和单张没区别
			tPutConfig.bIs4W2_One = false;
			tPutConfig.bIs4W2_Double = false;
			tPutConfig.bIs3W1_One = true;
			tPutConfig.bIs3W1_Double = true;
			GetPutArrayList(vTmp,tOutResul,tPutConfig);
			/// 保存手数和权值
			tHandsNode.iHandNumber = tOutResul.iMinHandsNumber + iBaseHandsNumber;
			tHandsNode.iValue =  tOutResul.iMaxValue;
			memcpy(tHandsNode.byTypeCount, tOutResul.byTypeCount, sizeof(tHandsNode.byTypeCount));
			//if( tOutResul.iMinHandsNumber < sNowResul.iMinHandsNumber+2 ) // 打出去的牌 形成新的组合手数要比没拆的组合手数 要在一个合理的范围 不能拆之后牌很烂
			//{
			//	tHandsNodeList.push_back(tHandsNode);
			//}
			tHandsNodeList.push_back(tHandsNode);
			iter = tFollowArrayList.erase(iter);
		}
		//// 排序 最优组合在最前面
		int iLen = tHandsNodeList.size();
		for(int i=0;i < iLen;i++)
		{
			for(int j=i+1;j < iLen;j++)
			{
				int iResult = 0;
				const int iSwitchMap[8] = {7, 6, 2, 4, 5, 3, 1, 0};
				for(int k = 0; k < 8; k++)
				{
					if(tHandsNodeList[i].byTypeCount[iSwitchMap[k]] < tHandsNodeList[j].byTypeCount[iSwitchMap[k]])
					{
						iResult = 1;
						break;
					}
					if(tHandsNodeList[i].byTypeCount[iSwitchMap[k]] > tHandsNodeList[j].byTypeCount[iSwitchMap[k]])
					{
						iResult = -1;
						break;
					}
				}
				if(iResult == 1)
				{
					SHandsNode tHandsNode;
					tHandsNode = tHandsNodeList[i];
					tHandsNodeList[i] = tHandsNodeList[j];
					tHandsNodeList[j] = tHandsNode;
				}
				else if(0 == iResult)
				{
					int iValue1 = tHandsNodeList[i].iValue + (21 - tHandsNodeList[i].iHandNumber)*7;  
					int iValue2 = tHandsNodeList[j].iValue + (21 - tHandsNodeList[j].iHandNumber)*7;  
					/*if(  > tHandsNodeList[j].iHandNumber || \
					(tHandsNodeList[i].iHandNumber == tHandsNodeList[j].iHandNumber && tHandsNodeList[i].iValue < tHandsNodeList[j].iValue) \
					)*/
					if( iValue1  < iValue2 )
					{
						SHandsNode tHandsNode;
						tHandsNode = tHandsNodeList[i];
						tHandsNodeList[i] = tHandsNodeList[j];
						tHandsNodeList[j] = tHandsNode;
					}
				}
				
			}
			tFollowArrayList.push_back(tHandsNodeList[i].tPlayREQ);
		}
		/// 结果返回
		if( tFollowArrayList.size() > 0)
		{
			return true;
		}
		return false;
	}

	bool CAIAlgorithmManage::IsOnlyCardByArray(const T_C2S_PLAY_CARD_REQ & tA,const HN::CardArrayBase & tSrcArray) const
	{
		int iCount = 0;
		HN::CardBase tCard = 0;
		unsigned char tActualCards[ONE_HAND_CARD_COUNT] = {0};	  //实际手牌 有癞子牌
		memcpy(tActualCards,tA.uActualCards,ONE_HAND_CARD_COUNT);
		for(int i=0;i < tA.iCardCount;i++)
		{
		   if( tActualCards[i] != 0 )
		   {
		      tCard = tActualCards[i];
			  iCount = 1;
			  for(int j=i+1;j< tA.iCardCount;j++)
			  {
				  if( tCard.getVal() == HN::CardBase(tActualCards[j]).getVal() )
				  {
					  tActualCards[j] = 0;
					  iCount++;
				  }
			  }
			  std::vector<HN::CardBase>  vecData;
			  int iNumber = tSrcArray.getCardAllType(tCard,vecData);
			  if( iNumber != iCount)
			  {
				  return false;
			  }
		   }
		}
		return true;
	}

	bool CAIAlgorithmManage::GetFollowArrayListByInc(const HN::CardArrayBase & tSrcArray,const T_C2S_PLAY_CARD_REQ & tLastCard,std::vector<T_C2S_PLAY_CARD_REQ> & tFollowArrayList) const
	{
		auto cmp = [&](const T_C2S_PLAY_CARD_REQ & tA,const T_C2S_PLAY_CARD_REQ & tB)->bool
		{
			//// 癞子数
			if( tA.eArrayType != ARRAY_TYPE_WBOMB && tB.eArrayType != ARRAY_TYPE_WBOMB  )
			{
				std::vector<HN::CardBase>  vecData;
				HN::CardArrayBase  tArrayA;
				HN::CardArrayBase  tArrayB;
				T_C2S_PLAY_CARD_REQ  tmpA = tA;
				T_C2S_PLAY_CARD_REQ  tmpB = tB;
				tArrayA.assign(tmpA.uActualCards,(size_t)tA.iCardCount);
				tArrayB.assign(tmpB.uActualCards,(size_t)tB.iCardCount);
			    int iLaiZiNumberA = tArrayA.getCardAllType(m_tianLaiZi,vecData) + tArrayA.getCardAllType(m_diLaiZi,vecData);
				int iLaiZiNumberB = tArrayB.getCardAllType(m_tianLaiZi,vecData) + tArrayB.getCardAllType(m_diLaiZi,vecData);
				if( iLaiZiNumberA > iLaiZiNumberB )
				{
				    return false;
				}
				else if( iLaiZiNumberA < iLaiZiNumberB )
				{
				    return true;
				}
			}
			//// 是否原来组合
			bool bIsOnlyA = IsOnlyCardByArray(tA,tSrcArray);
			bool bIsOnlyB = IsOnlyCardByArray(tB,tSrcArray);
			if( bIsOnlyA == true && bIsOnlyB == false )
			{
			    return true;
			}
			if( bIsOnlyA == false && bIsOnlyB == true )
			{
				return false;
			}
			//// 比较大小
			int ret = CompareCard(tA,tB);
			if( ret == 1)
			{
				return false;
			}
			else
			{
			    return true;
			}
		};
		//// 过滤非法数据
		if( tLastCard.eArrayType == ARRAY_TYPE_ERROR )
		{
			return false;
		}
		tFollowArrayList.clear();
		std::vector<SCombinationNode>  vecFollowCardList;
		SCombinationNode tLastNode;
		CPlayCardToNode(tLastCard,tLastNode);
		/// 获取所有的跟牌结果
		GetFollowListByRemove(tSrcArray,tLastNode,vecFollowCardList);
		/// 结果添加花色
		AddRealCardToNode(tSrcArray,vecFollowCardList,true);
		/// 癞子获取
		std::vector<HN::CardBase> tLaiZiList;
		GetLaiZiListBySrc(tSrcArray,tLaiZiList);
		/// 结果转换 添加癞子牌
		AddLaiZiCardToArrayList(vecFollowCardList,tLaiZiList,tFollowArrayList,true);
		/// 从小打到 排序
		std::sort(tFollowArrayList.begin(),tFollowArrayList.end(),cmp);
		/// 结果返回
		if( tFollowArrayList.size() > 0)
		{
			return true;
		}
		return false;
	}

	//// 私有方法  ///////////////////////////////
	//// 牌型  ///////////////////////////////////
	bool CAIAlgorithmManage::GetTypeList(const HN::CardArrayBase & tSrcArray,std::vector<HN_AI::SCombinationNode> & tCardNodeList) const
	{
		byte bySrcData[ONE_HAND_CARD_COUNT];
		/// 数据初始化
		memset(bySrcData,0,sizeof(bySrcData));
		int iLaiZiNum = 0;
		int iSrcLen = ONE_HAND_CARD_COUNT;
		tCardNodeList.clear();
		/// 转换数据 获取没有花色 无赖子的牌值
		GetSrcToNoLaiZi(tSrcArray,bySrcData,iSrcLen,iLaiZiNum);
		/// 获取出 所有能组合的类型
		m_pCurAlgorithm->GetCardType(bySrcData,iSrcLen,iLaiZiNum,tCardNodeList);
		return true;
	}
	//// 主动出牌算法 ////////////////////////////
	bool CAIAlgorithmManage::GetPutArrayList(const HN::CardArrayBase & tSrcArray,SCombinationResult & sCombinationResul,HN_AI::SSysConfig tSysConfig) const
	{
		SSysConfig tConfig = m_pCurAlgorithm->GetCardRulesConfig();
		m_pCurAlgorithm->SetCardRulesConfig(tSysConfig);
		bool ret = GetPutArrayList(tSrcArray,sCombinationResul);
		m_pCurAlgorithm->SetCardRulesConfig(tConfig);
		return ret;
	}

	bool CAIAlgorithmManage::GetPutArrayList(const HN::CardArrayBase & tSrcArray,SCombinationResult & sCombinationResul) const
	{
		byte bySrcData[ONE_HAND_CARD_COUNT];
		/// 数据初始化
		memset(bySrcData,0,sizeof(bySrcData));
		int iLaiZiNum = 0;
		int iSrcLen = ONE_HAND_CARD_COUNT;
		sCombinationResul.clear();
		/// 转换数据 获取没有花色 无赖子的牌值
		GetSrcToNoLaiZi(tSrcArray,bySrcData,iSrcLen,iLaiZiNum);
		/// 获取跟牌结果
		//int iLaiZi = Algor::rand_Mersenne(1,4);
		/*unsigned char tData[21] = {3,3,4,4,4,5,5,6,6,7,9,9,10,13,1,14};
		LOGICINSTANCE_HANDSANDWEIGHT->GetPutCardList(tData,17,1,sCombinationResul);*/
		m_pCurAlgorithm->GetPutCardList(bySrcData,iSrcLen,iLaiZiNum,sCombinationResul);
		return true;
	}
	//// 跟牌算法 ///////////////////////////////
	bool CAIAlgorithmManage::GetFollowArrayList(const HN::CardArrayBase & tSrcArray,const HN_AI::SCombinationNode & tLastNode,std::vector<HN_AI::SCombinationNode> & tFollowArrayList) const
	{
		std::vector<HN_AI::SCombinationNode> tNodeList;
		//// 1、先从组合牌中获取
		bool ret = GetFollowListByNodeList(tSrcArray,tLastNode,tNodeList);
		tFollowArrayList.insert(tFollowArrayList.end(),tNodeList.begin(),tNodeList.end());
		//// 2、拆组合 获取组合牌
		tNodeList.clear();
		ret = GetFollowListByRemove(tSrcArray,tLastNode,tNodeList);
		//// 3、过滤相同的结果
		for(auto iter = tNodeList.begin();iter!=tNodeList.end();)
		{
			bool bHave = false;
			for(int i=0;i<tFollowArrayList.size();i++)
			{
				if( tFollowArrayList[i].eCardType == iter->eCardType 
					&& tFollowArrayList[i].eAddCardType == iter->eAddCardType
					&& tFollowArrayList[i].vecMainCard.size() == iter->vecMainCard.size() 
					&& tFollowArrayList[i].vecMainCard.size() != 0
					&& tFollowArrayList[i].vecMainCard.front() == iter->vecMainCard.front() 
					)
				{
					bHave = true;
					break;
				}
			}
			if( bHave )
			{
				iter = tNodeList.erase(iter);
			}
			else
			{
				iter++;
			}
		}
		tFollowArrayList.insert(tFollowArrayList.end(),tNodeList.begin(),tNodeList.end());
		//// 结果返回
		if( tFollowArrayList.size() > 0 )
		{
			return true;
		}
		return false;
	}
	bool CAIAlgorithmManage::GetFollowListByRemove(const HN::CardArrayBase & tSrcArray,const HN_AI::SCombinationNode & tLastNode,std::vector<HN_AI::SCombinationNode> & tFollowArrayList) const
	{
		//// 过滤非法数据
		if( tLastNode.eCardType == eCardType_Unknown )
		{
		    return false;
		}
		/// 数据声明
		byte bySrcData[ONE_HAND_CARD_COUNT];
		/// 数据初始化
		memset(bySrcData,0,sizeof(bySrcData));
		int iLaiZiNum = 0;
		int iSrcLen = ONE_HAND_CARD_COUNT;
		tFollowArrayList.clear();
		/// 转换数据 获取没有花色 无赖子的牌值
		GetSrcToNoLaiZi(tSrcArray,bySrcData,iSrcLen,iLaiZiNum);
		/// 获取跟牌结果
		/*unsigned char tData[21] = {4,4,5,5,6,6,7,7,8,8,9,9,11,11,12,12,13,13,1,1};
		unsigned char tData1[21] = {3,3,4,4,5,5};
		LOGICINSTANCE_HANDSANDWEIGHT->GetFollowCardList(tData,20,tData1,6,iLaiZi,vecFollowCardList);*/
		m_pCurAlgorithm->GetFollowCardList(bySrcData,iSrcLen,tLastNode,iLaiZiNum,tFollowArrayList);
		/// 获取出癞子本身 能跟的牌
		GetFollowNodeByLaiZiValue(tSrcArray,tLastNode,tFollowArrayList,false);
		return true;
	}
	bool CAIAlgorithmManage::GetFollowListByNodeList(const HN::CardArrayBase & tSrcArray,const HN_AI::SCombinationNode & tLastNode,std::vector<HN_AI::SCombinationNode> & tFollowArrayList) const
	{
		//// 过滤非法数据
		if( tLastNode.eCardType == eCardType_Unknown )
		{
			return false;
		}
		tFollowArrayList.clear();
		/// 去除癞子牌
		HN::CardArrayBase tSrcNoLaiZi;
		for(int i=0; i < tSrcArray.getArray().size();i++)
		{
		   if( tSrcArray.getArray()[i].getVal() == m_tianLaiZi.getVal() || tSrcArray.getArray()[i].getVal() == m_diLaiZi.getVal() )
		   {
		      continue;
		   }
		   tSrcNoLaiZi.push_back(tSrcArray.getArray()[i]);
		}
		/// 设置规则
		HN_AI::SSysConfig tPutConfig = m_pCurAlgorithm->GetCardRulesConfig();;
		tPutConfig.bIs4W2_One = false;
		tPutConfig.bIs4W2_Double = false;
		tPutConfig.bIs3W1_One = false;
		tPutConfig.bIs3W1_Double = false;
		if( tLastNode.eAddCardType == eAddCardType_DanZhang )
		{
		    if(tLastNode.eCardType == eCardType_SanZhang || tLastNode.eCardType == eCardType_FeiJi )
			{
			    tPutConfig.bIs3W1_One = true;
			}
		}
		if( tLastNode.eAddCardType == eAddCardType_DuiZi )
		{
			if(tLastNode.eCardType == eCardType_SanZhang || tLastNode.eCardType == eCardType_FeiJi )
			{
				tPutConfig.bIs3W1_Double = true;
			}
		}
		/// 获取出牌最优组合
		SCombinationResult sCombinationResul;
		GetPutArrayList(tSrcNoLaiZi,sCombinationResul,tPutConfig);
		m_pCurAlgorithm->GetNodeListByFilter(sCombinationResul.vecCombinationResult,tFollowArrayList,tLastNode);
		if( tFollowArrayList.size() > 0  )
		{
			return true;
		}
		return false;
	}
	bool CAIAlgorithmManage::GetFollowNodeByLaiZiValue(const HN::CardArrayBase & tSrcArray,const HN_AI::SCombinationNode & tLastNode,std::vector<HN_AI::SCombinationNode> & tFollowArrayList,bool bFlag) const
	{
		/// 数据声明
		byte bySrcData[ONE_HAND_CARD_COUNT];
		/// 数据初始化
		memset(bySrcData,0,sizeof(bySrcData));
		int iLaiZiNum = 0; 
		int iSrcLen = ONE_HAND_CARD_COUNT;
		GetSrcToNoLaiZi(tSrcArray,bySrcData,iSrcLen,iLaiZiNum);
		if( tLastNode.vecMainCard.size() > iLaiZiNum || tLastNode.vecMainCard.size() > 4 
			|| (tLastNode.eCardType == eCardType_ZhaDan && tLastNode.eAddCardType == eAddCardType_No ) 
			)
		{
			return false;
		}
		//// 获取癞子本身值是否能跟牌
		std::vector<HN_AI::SCombinationNode> tNodeList;
		///  天癞本身能跟的牌
		bool ret = tSrcArray.getCardAllType(m_tianLaiZi);
		if( m_tianLaiZi != HN::CardBase::NIL && ret == true )
		{
			iSrcLen = ONE_HAND_CARD_COUNT;
			GetPlayCardVal(tSrcArray,bySrcData,iSrcLen);
			for(int i=0;i<iSrcLen;i++)
			{
			    if( bySrcData[i] == m_diLaiZi.getVal() )
				{
				    bySrcData[i] = m_tianLaiZi.getVal();
				}
			}
			m_pCurAlgorithm->GetFollowCardList(bySrcData,iSrcLen,tLastNode,0,tNodeList);
			/// 癞子本身还能跟牌
			for(auto iter = tNodeList.begin();iter!= tNodeList.end();iter++)
			{
				if( tLastNode.eCardType == iter->eCardType && tLastNode.eAddCardType == iter->eAddCardType 
					&& tLastNode.vecMainCard.size() == iter->vecMainCard.size()
					&& iter->vecMainCard.front() == m_tianLaiZi.getVal()
				  )
				{
					if(bFlag == true && iSrcLen != (iter->vecMainCard.size()+iter->vecSubCard.size()))
					{
					   continue;
					}
					iter->vecLaiZiCard.insert(iter->vecLaiZiCard.end(),iter->vecMainCard.begin(),iter->vecMainCard.end());
					tFollowArrayList.push_back(*iter);
				}
			}
		}
		/// 地癞能跟的牌
		ret = tSrcArray.getCardAllType(m_diLaiZi);
		if( m_diLaiZi != HN::CardBase::NIL && ret == true)
		{
			iSrcLen = ONE_HAND_CARD_COUNT;
			GetPlayCardVal(tSrcArray,bySrcData,iSrcLen);
			for(int i=0;i<iSrcLen;i++)
			{
				if( bySrcData[i] == m_tianLaiZi.getVal() )
				{
					bySrcData[i] = m_diLaiZi.getVal();
				}
			}
			m_pCurAlgorithm->GetFollowCardList(bySrcData,iSrcLen,tLastNode,0,tNodeList);
			/// 癞子本身还能跟牌
			for(auto iter = tNodeList.begin();iter!= tNodeList.end();iter++)
			{
				if( tLastNode.eCardType == iter->eCardType && tLastNode.eAddCardType == iter->eAddCardType 
					&& tLastNode.vecMainCard.size() == iter->vecMainCard.size()
					&& iter->vecMainCard.front() == m_diLaiZi.getVal()
				  )
				{
					if(bFlag == true && iSrcLen != (iter->vecMainCard.size()+iter->vecSubCard.size()))
					{
						continue;
					}
					iter->vecLaiZiCard.insert(iter->vecLaiZiCard.end(),iter->vecMainCard.begin(),iter->vecMainCard.end());
					tFollowArrayList.push_back(*iter);
				}
			}
		}
		return true;
	}
	///////// 底牌类型判断算法 ///////////////////////////////////////////////////
	bool CAIAlgorithmManage::isOneKing(const HN::CardArrayBase & tSrcArray)const
	{
		int iNumber = 0;
		for(int i=0;i< tSrcArray.Size();i++)
		{
			if( tSrcArray.getArray()[i].getData() == HN::CardBase::SJ || tSrcArray.getArray()[i].getData() == HN::CardBase::BJ )
			{
				iNumber++;
			}
		}
		if( iNumber == 1 )
		{
			return true;
		}
		return false;
	}

	bool CAIAlgorithmManage::isTwoKing(const HN::CardArrayBase & tSrcArray)const
	{
		int iNumber = 0;
		for(int i=0;i<tSrcArray.Size();i++)
		{
			if( tSrcArray.getArray()[i].getData() == HN::CardBase::SJ || tSrcArray.getArray()[i].getData() == HN::CardBase::BJ )
			{
				iNumber++;
			}
		}
		if( iNumber == 2 )
		{
			return true;
		}
		return false;
	}

	bool CAIAlgorithmManage::isShunZi(const HN::CardArrayBase & tSrcArray)const
	{
		vector<HN::CardBase> vCard;
		vCard = tSrcArray.getArray();
		std::sort(vCard.begin(),vCard.end());     /// 从小打大
		for(int i=1 ;i< vCard.size();i++)
		{
			if( vCard[i].getVal() == (vCard[0].getVal()+i) )
			{
				continue;
			}
			if( vCard[i].getVal() == HN::CardBase::VAL_A && vCard[i-1].getVal() == HN::CardBase::VAL_K )
			{
				continue;
			}
			return false;
		}
		return true;
	}

	bool CAIAlgorithmManage::isTongHua(const HN::CardArrayBase & tSrcArray)const
	{
		for(int i=1 ;i<tSrcArray.Size();i++)
		{
			if( tSrcArray.getArray()[i].getType() !=  tSrcArray.getArray()[0].getType() || tSrcArray.getArray()[i].getType() == HN::CardBase::TYPE_JOKER )
			{
				return false;
			}
		}
		return true;
	}

	bool CAIAlgorithmManage::isSanZhang(const HN::CardArrayBase & tSrcArray)const
	{
		for(int i=1 ;i< tSrcArray.Size();i++)
		{
			if( tSrcArray.getArray()[i].getVal() !=  tSrcArray.getArray()[0].getVal() )
			{
				return false;
			}
		}
		return true;
	}
	//// 公共算法 ///////////////////////////////
	bool CAIAlgorithmManage::AddRealCardToNode(const HN::CardArrayBase & tSrcArray,std::vector<SCombinationNode> & tNodeList,bool bIsCycle) const
	{
		byte bySrcData[ONE_HAND_CARD_COUNT];
		memset(bySrcData,0,sizeof(bySrcData));
		if( bIsCycle == true )
		{
		    for(auto iter = tNodeList.begin();iter!= tNodeList.end();iter++)
			{
				int iSrcLen = tSrcArray.reAssign(bySrcData,ONE_HAND_CARD_COUNT);
				ChangeResultDataToSrc(bySrcData,iSrcLen,*iter);
			}
		}
		else
		{
			int iSrcLen = tSrcArray.reAssign(bySrcData,ONE_HAND_CARD_COUNT);
			for(auto iter = tNodeList.begin();iter!= tNodeList.end();iter++)
			{	
				ChangeResultDataToSrc(bySrcData,iSrcLen,*iter);
			}
		}
		return true;
	}
	bool CAIAlgorithmManage::ChangeResultDataToSrc(unsigned char *pData,int iLen,SCombinationNode & tCombinationNode) const
	{
		//// 非法条件过滤
		if( pData == nullptr )
		{
			return false;
		}
		auto ToSrc = [&](byte KeyValue,byte & iSrcValue) -> int
		{
			for(int i=0;i<iLen;i++)
			{
				if( KeyValue == (pData[i]&CARD_MASK_VALUE) )
				{
					iSrcValue = pData[i];
					return i;
				}
			}
			return -1;
		};
	    //// 主牌
		for(int i=0;i<tCombinationNode.vecMainCard.size();i++)
		{
			int iIndex = -1;
			iIndex = ToSrc(tCombinationNode.vecMainCard[i],tCombinationNode.vecMainCard[i]);
			if( iIndex != -1)
			{
				pData[iIndex] = 0;
			}
		}
		//// 副牌
		for(int i=0;i<tCombinationNode.vecSubCard.size();i++)
		{
			int iIndex = -1;
			iIndex = ToSrc(tCombinationNode.vecSubCard[i],tCombinationNode.vecSubCard[i]);
			if( iIndex != -1)
			{
				pData[iIndex] = 0;
			}
		}
		return true;
	}
	void CAIAlgorithmManage::AddLaiZiCardToArrayList(const std::vector<SCombinationNode> & tCardList,const std::vector<HN::CardBase> & tLaiZiList,std::vector<T_C2S_PLAY_CARD_REQ> & tArrayList,bool bIsFollow) const
	{
		//// 获取手牌中所有癞子
		std::vector<HN::CardBase> tmpLaiZiList;
		tmpLaiZiList = tLaiZiList;
		int iLaiZiCount = tmpLaiZiList.size();
		/// 给含有癞子牌的组合添加癞子牌值
		for(auto iter = tCardList.begin();iter!= tCardList.end();iter++) 
		{	
			/// 如果是跟牌 那么每一组结果 都是以所有癞子来计算
			if(bIsFollow == true)   
			{
				tmpLaiZiList.clear();
				tmpLaiZiList = tLaiZiList;
				iLaiZiCount = tmpLaiZiList.size();
			}
			SCombinationNode tCombinationNode = *iter;
			byte tResultData[ONE_HAND_CARD_COUNT];
			int iResultLen = 0;
			memset(tResultData,0,sizeof(tResultData));
			iResultLen = m_pCurAlgorithm->GetMainAndSubByCombinationNode(tResultData,ONE_HAND_CARD_COUNT,tCombinationNode);
			/// 结果保存
			T_C2S_PLAY_CARD_REQ vResultData;
			vResultData.iCardCount = iResultLen;
			memcpy(vResultData.uCards,tResultData,iResultLen);
			memcpy(vResultData.uActualCards,tResultData,iResultLen);
			/// 获取最大的癞子和最小癞子
			HN::CardBase tMaxLaiZi = HN::CardBase::NIL;
			HN::CardBase tMinLaiZi = HN::CardBase::NIL;
			if( iLaiZiCount > 0 )
			{
				tMaxLaiZi = tmpLaiZiList.front().getVal();
				tMinLaiZi = tmpLaiZiList.back().getVal();
			}
			/// 去除癞子本身牌 癞子牌当本身值 已经转换为有花色牌
			for(int i=0;i < iResultLen;i++)
			{
			    auto tIter = find(tmpLaiZiList.begin(),tmpLaiZiList.end(),vResultData.uActualCards[i]);
				if( tIter != tmpLaiZiList.end() )
				{
				    tmpLaiZiList.erase(tIter);
					HN::CardBase tCard = vResultData.uActualCards[i];
					auto tLaiZiIter = find(tCombinationNode.vecLaiZiCard.begin(),tCombinationNode.vecLaiZiCard.end(),tCard.getVal());
		            if( tLaiZiIter != tCombinationNode.vecLaiZiCard.end() )
					{
					    tCombinationNode.vecLaiZiCard.erase(tLaiZiIter);
					}
					iLaiZiCount--;
				}
			}
			/// 癞子牌替换 
			for(int i = 0;i < tCombinationNode.vecLaiZiCard.size() && iLaiZiCount > 0; i++)
			{
				unsigned char tLaiZiCard = tCombinationNode.vecLaiZiCard[i];
				for(int j=0;j < iResultLen && iLaiZiCount > 0;j++)
				{
					if( vResultData.uActualCards[j] == tLaiZiCard )
					{
						/// 添加癞子本身值
						if( tLaiZiCard == 0 || tLaiZiCard == 14 || tLaiZiCard == 15 )
						{
							if(j >= tCombinationNode.vecMainCard.size() )		//// 带牌选最小
							{
								vResultData.uCards[j] = tMinLaiZi.getData();
							}
							else												//// 主牌选最大
							{
								vResultData.uCards[j] = tMaxLaiZi.getData();
							}
						}
						if(tLaiZiCard == 0 && i ==0)
						{
							vResultData.uActualCards[j] = tmpLaiZiList.front().getData();
							tmpLaiZiList.erase(tmpLaiZiList.begin());
						}
						else
						{
							vResultData.uActualCards[j] = tmpLaiZiList.back().getData();
							tmpLaiZiList.pop_back();
						}
						iLaiZiCount--;
						break;
					}
				}
			}
			/// 牌型获取
			vResultData.eArrayType = AICardTypeToArrayType(tCombinationNode);
			tArrayList.push_back(vResultData);
		}
	}
	EArrayType CAIAlgorithmManage::AICardTypeToArrayType(const SCombinationNode & sCombinationNode) const
	{
		switch(sCombinationNode.eCardType)
		{
		case eCardType_DanZhang:
			{
				return ARRAY_TYPE_SINGLE;
			}
			break;
		case eCardType_ShunZi:
			{
				return ARRAY_TYPE_STRAIGHT_ONE;
			}
			break;
		case eCardType_DuiZi:
			{
				return ARRAY_TYPE_DOUBLE;
			}
			break;
		case eCardType_LianDui:
			{
				return ARRAY_TYPE_STRAIGHT_DOUBLE;
			}
			break;
		case eCardType_SanZhang:
			{
				if(sCombinationNode.eAddCardType == eAddCardType_No)
				{
					return ARRAY_TYPE_3W_;
				}
				else if(sCombinationNode.eAddCardType == eAddCardType_DanZhang)
				{
					return ARRAY_TYPE_3W1_ONE;
				}
				else if(sCombinationNode.eAddCardType == eAddCardType_DuiZi)
				{
					return ARRAY_TYPE_3W1_DOUBLE;
				}
				else
				{
					return ARRAY_TYPE_ERROR;
				}
			}
			break;
		case eCardType_FeiJi:
			{
				if(sCombinationNode.eAddCardType == eAddCardType_No)
				{
					return ARRAY_TYPE_PLANE_;
				}
				else if(sCombinationNode.eAddCardType == eAddCardType_DanZhang)
				{
					return ARRAY_TYPE_PLANE_ONE;
				}
				else if(sCombinationNode.eAddCardType == eAddCardType_DuiZi)
				{
					return ARRAY_TYPE_PLANE_DOUBLE;
				}
				else
				{
					return ARRAY_TYPE_ERROR;
				}
			}
			break;
		case eCardType_ZhaDan:
			{
				if(sCombinationNode.eAddCardType == eAddCardType_No)   //// 炸弹类型还很多
				{
					if(sCombinationNode.vecMainCard.size() == 2)       ///  王炸
					{
						return ARRAY_TYPE_WBOMB;
					}
					if( sCombinationNode.vecLaiZiCard.size() == 4 && sCombinationNode.vecMainCard.size() == 4)
					{
						return ARRAY_TYPE_4L;
					}
					else if( sCombinationNode.vecLaiZiCard.size() > 0 && sCombinationNode.vecMainCard.size() == 4 )
					{
						return ARRAY_TYPE_SBOMB;
					}
					else if( sCombinationNode.vecLaiZiCard.size() > 0 && sCombinationNode.vecMainCard.size() > 4 
						&& sCombinationNode.vecMainCard.size() != sCombinationNode.vecLaiZiCard.size() )
					{
					    return ARRAY_TYPE_LBOMB_S;    /// 5张以上炸弹 软炸
					}
					else if(sCombinationNode.vecLaiZiCard.size() > 0 && sCombinationNode.vecMainCard.size() > 4 
						&& sCombinationNode.vecMainCard.size() == sCombinationNode.vecLaiZiCard.size() )
					{
						return ARRAY_TYPE_LBOMB_L;    /// 5张以上炸弹 纯癞子炸
					}
					return ARRAY_TYPE_HBOMB;
				}
				else if(sCombinationNode.eAddCardType == eAddCardType_DanZhang)
				{
					return ARRAY_TYPE_4w2_ONE;
				}
				else if(sCombinationNode.eAddCardType == eAddCardType_DuiZi)
				{
					return ARRAY_TYPE_4w2_DOUBLE;
				}
				else
				{
					return ARRAY_TYPE_ERROR;
				}
			}
			break;
		default:
			return ARRAY_TYPE_ERROR;
		}
	}
	void CAIAlgorithmManage::GetPlayCardVal(T_C2S_PLAY_CARD_REQ & tPlayCard) const
	{
		HN::CardArrayBase tDstData;
		HN::CardArrayBase tActData;
		tDstData.assign(tPlayCard.uCards,tPlayCard.iCardCount);
		tActData.assign(tPlayCard.uActualCards,tPlayCard.iCardCount);
		tPlayCard.iCardCount = tDstData.reAssignVal(tPlayCard.uCards,ONE_HAND_CARD_COUNT);
		tActData.reAssignVal(tPlayCard.uActualCards,ONE_HAND_CARD_COUNT);
	}

	void CAIAlgorithmManage::GetPlayCardVal(const HN::CardArrayBase & tSrcArray,unsigned char *bySrcData,int & iSrcLen) const
	{
		if( nullptr ==  bySrcData || iSrcLen != ONE_HAND_CARD_COUNT  )
		{
			return;
		}
		/// 获取无花色牌值
		memset(bySrcData,0,ONE_HAND_CARD_COUNT);
		iSrcLen = tSrcArray.reAssignVal(bySrcData,ONE_HAND_CARD_COUNT);
	}

	void CAIAlgorithmManage::CPlayCardToNode(const T_C2S_PLAY_CARD_REQ & tPlayCard,HN_AI::SCombinationNode & tCombinationNode) const
	{
		T_C2S_PLAY_CARD_REQ tCardVal = tPlayCard;
		/// 去花色
		GetPlayCardVal(tCardVal);
		/// 获取牌型
		m_pCurAlgorithm->GetCardType(tCardVal.uCards,tCardVal.iCardCount,tCombinationNode);
		/// 特殊 处理 3 3 3 4 4 4 3 4  牌型识别出来只能是四带2
		if( tPlayCard.eArrayType == ARRAY_TYPE_PLANE_ONE && tCombinationNode.eCardType == eCardType_ZhaDan && tCombinationNode.eAddCardType == eAddCardType_DuiZi
			&& tCombinationNode.vecSubCard[0] == tCombinationNode.vecSubCard[2] 
		  )
		{	  tCombinationNode.clear();
			  tCombinationNode.eCardType = eCardType_FeiJi;
			  tCombinationNode.eAddCardType = eAddCardType_DanZhang;
			  tCombinationNode.vecMainCard.push_back(tCardVal.uCards[0]);
			  tCombinationNode.vecMainCard.push_back(tCardVal.uCards[1]);
			  tCombinationNode.vecMainCard.push_back(tCardVal.uCards[2]);
			  tCombinationNode.vecMainCard.push_back(tCardVal.uCards[3]);
			  tCombinationNode.vecMainCard.push_back(tCardVal.uCards[4]);
			  tCombinationNode.vecMainCard.push_back(tCardVal.uCards[5]);

			  tCombinationNode.vecSubCard.push_back(tCardVal.uCards[6]);
			  tCombinationNode.vecSubCard.push_back(tCardVal.uCards[7]);
		}
		/// 纯癞子 需要特别处理
		if(tPlayCard.eArrayType == ARRAY_TYPE_4L || tPlayCard.eArrayType == ARRAY_TYPE_LBOMB_L )
		{
		    memset(tCardVal.uActualCards,0,sizeof(tCardVal.uActualCards));
		}
		/// 癞子牌添加
		for(int i = 0;i< tCardVal.iCardCount;i++)
		{
			if( tCardVal.uCards[i] != tCardVal.uActualCards[i] )
			{
				tCombinationNode.vecLaiZiCard.push_back( tCardVal.uCards[i] );	   
			}
		}
	}

	int CAIAlgorithmManage::CompareCard(const T_C2S_PLAY_CARD_REQ & tA,const T_C2S_PLAY_CARD_REQ & tB)const
	{
		SCombinationNode tNode1;
		SCombinationNode tNode2;
		CPlayCardToNode(tA,tNode1);
		CPlayCardToNode(tB,tNode2);
		/// 获取牌型
		return m_pCurAlgorithm->CompareNodeFun(tNode1,tNode2);
	}

	void CAIAlgorithmManage::GetLaiZiListBySrc(const HN::CardArrayBase & tSrcArray,std::vector<HN::CardBase> & tLaiZiList) const
	{
		if( m_tianLaiZi.getRealVal() > m_diLaiZi.getRealVal() )
		{
			tSrcArray.getCardAllType(m_tianLaiZi,tLaiZiList);
			tSrcArray.getCardAllType(m_diLaiZi,tLaiZiList);
		}
		else
		{
			tSrcArray.getCardAllType(m_diLaiZi,tLaiZiList);
			tSrcArray.getCardAllType(m_tianLaiZi,tLaiZiList);
		}
	}

	void CAIAlgorithmManage::GetSrcToNoLaiZi(const HN::CardArrayBase & tSrcArray,unsigned char *bySrcData,int & iSrcLen,int & iLaiZiNumber) const
	{
		if( nullptr ==  bySrcData || iSrcLen != ONE_HAND_CARD_COUNT  )
		{
			return;
		}
		HN::CardArrayBase tSrc = tSrcArray;
		/// 去除天癞
		std::vector<HN::CardBase> tTianLaiList;
		tSrc.getCardAllType(m_tianLaiZi,tTianLaiList);
		HN::CardArrayBase tTianLaiArray = tTianLaiList;
		tSrc.erase(tTianLaiArray);
		/// 去除地癞
		std::vector<HN::CardBase> tDiLaiList;
		tSrc.getCardAllType(m_diLaiZi,tDiLaiList);
		HN::CardArrayBase tDiLaiArray = tDiLaiList;
		tSrc.erase(tDiLaiArray);
		/// 获取无花色牌值
		memset(bySrcData,0,ONE_HAND_CARD_COUNT);
		iSrcLen = tSrc.reAssignVal(bySrcData,ONE_HAND_CARD_COUNT);
		iLaiZiNumber = tTianLaiList.size() + tDiLaiList.size();
	}

	void CAIAlgorithmManage::CleanBomb(std::vector<T_C2S_PLAY_CARD_REQ> &tFollowArrayList) const
	{
		//王炸不能拆开出。（防止王炸被拆开）
		bool bHaveKingBomb = false;
		for(std::vector<T_C2S_PLAY_CARD_REQ>::iterator it = tFollowArrayList.begin(); it != tFollowArrayList.end();it++)
		{
			if(it->eArrayType == ARRAY_TYPE_WBOMB) 
			{
				for(std::vector<T_C2S_PLAY_CARD_REQ>::iterator it2 = tFollowArrayList.begin(); it2 != tFollowArrayList.end();)
				{
					if( it2->eArrayType == ARRAY_TYPE_SINGLE && (it2->uCards[0] == 78 || it2->uCards[0] == 79) )
					{
						it2 = tFollowArrayList.erase(it2);
					}
					else
					{
						it2++;
					}
				}
				break;
			}
		}


		//炸弹不能拆开出。（防止炸弹被拆开）
		while(true)
		{
			bool bFindFirst = false;
			for(std::vector<T_C2S_PLAY_CARD_REQ>::iterator it = tFollowArrayList.begin(); it != tFollowArrayList.end();)
			{
				if(it->eArrayType != ARRAY_TYPE_HBOMB) 
				{
					it++;
					continue;
				}
				for(std::vector<T_C2S_PLAY_CARD_REQ>::iterator it2 = tFollowArrayList.begin(); it2 != tFollowArrayList.end(); )
				{
					if( it2 == it)
					{
						//不和自己比较
						it2++;
						continue;
					}
					//删除所有包含炸弹牌的节点
					for(int i = 0; i < it2->iCardCount; ++i)
					{
						if( (it2->uCards[i] % 16) == (it->uCards[0] % 16) )
						{
							//删除后重新开始查找
							it2 = tFollowArrayList.erase(it2);
							bFindFirst = true;
							break;
						}
					}

					if(!bFindFirst)
					{
						it2++;
					}
					else
					{
						break;;
					}
				}
				if(!bFindFirst)
				{
					//查找下一个炸弹
					it++;
				}
				else
				{
					break;;
				}
			}

			if(!bFindFirst) break;
		}
	}

}

