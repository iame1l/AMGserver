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
#include <vector>
#include "..\util\log\LogModule.h"
#include <afx.h>
#include <string>
#include <algorithm>
#include "AIAlgorithmCommom.h"

extern bool g_bFirstTimeNtOut;

namespace HN_AI
{
	CHandsNumberAndWeight::CHandsNumberAndWeight(void)
	{
		m_sSysConfig.bIsWBomb_Separate = true;   /// 王炸是否可以拆开
		m_sSysConfig.bIs3W1_One = true;			 /// 是否三带 1单张
		m_sSysConfig.bIs3W1_Double  = true;      /// 是否三带 1对子
		m_sSysConfig.bIs4W2_One  = true;	     /// 是否四带 2单张
		m_sSysConfig.bIs4W2_Double  = true;		 /// 是否四带 2对子
		m_sSysConfig.bIsLongBomb = false;		 /// 是否支持长炸弹 大于5的炸弹 属于长炸弹
		m_sSysConfig.iBombCompareType = 0;       /// 炸弹比较类型

		m_iCheckType = InvalidCheck;
	}

	CHandsNumberAndWeight::~CHandsNumberAndWeight(void)
	{
	
	}
	/// 设置配制文件
	void CHandsNumberAndWeight::SetCardRulesConfig(const SSysConfig & tSysConfig)
	{
		m_sSysConfig = tSysConfig;
	}
	HN_AI::SSysConfig CHandsNumberAndWeight::GetCardRulesConfig()
	{
		return m_sSysConfig;
	}

	void CHandsNumberAndWeight::Init()
	{
		/// 初始化数据
		m_iMaxValue = 0;           /// 最大权值
		m_iMinHandsNumber = 255;   /// 最少手数
		m_vecCombinationResult.clear();  ///组牌结果
		m_vecResultNoWith.clear(); /// 不带牌 组牌结果
		memset(m_byCombinationData,0,sizeof(m_byCombinationData));

		m_vecMinCArdResult.clear();
		m_tMaxNode.clear();
		m_byMinCard = 255;
		memset(m_byTypeCount, 0, sizeof(m_byTypeCount));
	}

	bool CHandsNumberAndWeight::GetSubCardEx(const EAddCardMainType & tAddCardMainType,SVBYTE &tVecSanZhang, SVBYTE & tVecDuiZi,SVBYTE & tVecDanZhang,int iSubCardNumber,SVBYTE & tVecSubCard,EAddCardType & eAddCardType, bool bSpeicial)const
	{
		if(iSubCardNumber<= 0)
		{
			return false;
		}
		/// 判断是否能带牌
		auto IsAddDanZhang  = [&]() ->bool
		{
			switch(tAddCardMainType)
			{
			case eAddCardMainType_FeiJi:
			case eAddCardMainType_SanZhang:
				{
					if( m_sSysConfig.bIs3W1_One  )
					{
						return true;
					}
				}
				break;
			case eAddCardMainType_SiZhan:
				{
					if( m_sSysConfig.bIs4W2_One )
					{
						return true;
					}
				} 
				break;
			default:
				break;
			}
			return false;
		};
		auto IsAddDuiZi= [&]() ->bool
		{
			switch(tAddCardMainType)
			{
			case eAddCardMainType_FeiJi:
			case eAddCardMainType_SanZhang:
				{
					if( m_sSysConfig.bIs3W1_Double  )
					{
						return true;
					}
				}
				break;
			case eAddCardMainType_SiZhan:
				{
					if( m_sSysConfig.bIs4W2_Double )
					{
						return true;
					}
				} 
				break;
			default:
				break;
			}
			return false;
		};

		if(bSpeicial)
		{
			//3带1（对）的话根据最小牌数量来，飞机每次带1张最小牌
			if(tAddCardMainType == eAddCardMainType_SanZhang)
			{
				if(m_byCardData[m_byMinCard] == 1)
				{
					eAddCardType = eAddCardType_DanZhang;
					AddParamByValue(tVecSubCard,m_byMinCard);
				}
				else
				{
					eAddCardType = eAddCardType_DuiZi;
					AddParamByValue(tVecSubCard,m_byMinCard, 2);
				}
				return true;
			}
			eAddCardType = eAddCardType_DanZhang;
			int iMinNumber = m_byCardData[m_byMinCard];		//最小牌数量
			int iMinCardUse = 0;	//已经用调的最小牌
			//优先带掉所有的最小牌,然后补单排
			for(int i = 0; i < iSubCardNumber; ++i)
			{
				if(iMinCardUse < iMinNumber)
				{
					AddParamByValue(tVecSubCard,m_byMinCard);
					iMinCardUse++;
					if(iMinCardUse >= iMinNumber)
					{
						switch(iMinNumber)
						{
						case 1:
							tVecDanZhang.erase(find(tVecDanZhang.begin(), tVecDanZhang.end(), m_byMinCard));
							break;
						case 2:
							tVecDuiZi.erase(find(tVecDuiZi.begin(), tVecDuiZi.end(), m_byMinCard));
							break;
						case 3:
							tVecSanZhang.erase(find(tVecSanZhang.begin(), tVecSanZhang.end(), m_byMinCard));
							break;
						}
					}
				}
				else
				{
					bool bFind = false;
					do
					{
						//单排中查找
						int i=0;
						for(auto iter = tVecDanZhang.begin();i < iSubCardNumber && iter != tVecDanZhang.end();)
						{
							AddParamByValue(tVecSubCard,*iter);
							iter = tVecDanZhang.erase(iter);
							i++;
							bFind = true;
							break;
						}
						if(!bFind)
						{
							//对子中查找
							if(!tVecDuiZi.empty())
							{
								BYTE byCard = tVecDuiZi[0];
								AddParamByValue(tVecDanZhang, byCard, 2);
								tVecDuiZi.erase(tVecDuiZi.begin());
							}
							else
							{
								if(!tVecSanZhang.empty())
								{
									//三张中查找
									BYTE byCard = tVecSanZhang[0];
									AddParamByValue(tVecDanZhang, byCard,3);
									tVecDuiZi.erase(tVecDuiZi.begin());
								}
								else
								{
									eAddCardType = eAddCardType_No;
									//不带牌
									break;
								}
							}
							
						}
					}
					while(!bFind);
				}
			}
			return true;
		}

		/// 先匹配单张 后匹配对子
		if( tVecDanZhang.size() >= iSubCardNumber && IsAddDanZhang() )
		{
			int i=0;
			for(auto iter = tVecDanZhang.begin();i < iSubCardNumber && iter != tVecDanZhang.end();)
			{
				AddParamByValue(tVecSubCard,*iter);
				iter = tVecDanZhang.erase(iter);
				i++;
			}
		}
		else if( tVecDuiZi.size() >= iSubCardNumber && IsAddDuiZi() )
		{
			int i=0;
			for(auto iter = tVecDuiZi.begin();i < iSubCardNumber && iter != tVecDuiZi.end();)
			{
				AddParamByValue(tVecSubCard,*iter,2);
				iter = tVecDuiZi.erase(iter);
				i++;
			}
		}
		else if( iSubCardNumber >=3 && IsAddDanZhang() )  //// 拆对子去补 对子只会整除拆
		{
			if( (tVecDuiZi.size()*2+tVecDanZhang.size()) >= iSubCardNumber )  /// 先使用对子 在使用单张去补
			{
				if( iSubCardNumber%2 == 0 || tVecDanZhang.size() > 0 )		  /// 整除就全部使用对子 如果不能整除就必须要有单张
				{
					int i=0;
					for(auto iter = tVecDuiZi.begin();i < iSubCardNumber && iter != tVecDuiZi.end();)    // 使用对子填补
					{
						AddParamByValue(tVecSubCard,*iter,2);
						iter = tVecDuiZi.erase(iter);
						i+=2;
						if( i+1 == iSubCardNumber ) /// 如果只剩一张 由单张补全
						{
							break;
						}
					}
					if( i < iSubCardNumber)
					{
						for(auto iter = tVecDanZhang.begin();i < iSubCardNumber && iter != tVecDanZhang.end();) // 使用单张填补
						{
							AddParamByValue(tVecSubCard,*iter);
							iter = tVecDanZhang.erase(iter);
							i++;
						}
					}
				}
			}
		}
		else	// 不带
		{

		}
		//// 统计带牌类型
		if(tVecSubCard.size() == 0)
		{
			eAddCardType = eAddCardType_No;
		}
		else if( tVecSubCard.size() ==  iSubCardNumber  )
		{
			eAddCardType = eAddCardType_DanZhang;
		}
		else if( tVecSubCard.size() ==  iSubCardNumber*2  )
		{
			eAddCardType = eAddCardType_DuiZi;
		}
		else
		{
			eAddCardType = eAddCardType_Unknown;
		}
		return true;
	}

	//能否组成顺子
	void CHandsNumberAndWeight::MakeShunZi(SCombinationNode &tNode)
	{
		int iMaxLen = 0;

		//顺子5张起
		for(int iSunZiLen = 5;iSunZiLen <= 12;iSunZiLen++)
		{
			bool ret =  IsShunZi(m_byCardData, m_byMinCard, 1, iSunZiLen);
			if(ret) 
			{
				iMaxLen = iSunZiLen;
			}
			else
			{
				break;
			}
		}
		if(iMaxLen != 0)
		{
			tNode.eCardType = eCardType_ShunZi;
			for(int i = 0; i < iMaxLen; ++i)
			{
				tNode.vecMainCard.push_back(m_byMinCard + i);
			}
		}
	}

	//能否组成飞机
	void CHandsNumberAndWeight::MakeFeiJi(SCombinationNode &tNode)
	{
		SVBYTE tVecDanZhang;							 /// 单张  必须是有序的 下面算法均是在有序基础上实现
		SVBYTE tVecDuiZi;								 /// 对子  必须是有序的 下面算法均是在有序基础上实现
		SVBYTE tVecSanZhang;							 /// 三张  必须是有序的 下面算法均是在有序基础上实现
		SVBYTE tVecSiZhang;								 /// 四张  必须是有序的 下面算法均是在有序基础上实现
		std::vector<SZhaDanLongNode> tVecZhaDanLong;     /// 四张以上  必须是有序的 下面算法均是在有序基础上实现
		ClassificationData(m_byCardData,tVecDanZhang,tVecDuiZi,tVecSanZhang,tVecSiZhang,tVecZhaDanLong);


		//飞机可以带两张散牌
		int iSanZhangSize = tVecSanZhang.size();
		int iFeiJiLen = 0;
		for(int i=0;i < iSanZhangSize;)
		{
			iFeiJiLen = 2;
			bool ret = false;
			do 
			{
				//需要的单张是否能组成顺子?
				ret = IsShunZi(tVecSanZhang,tVecSanZhang[i],1,iFeiJiLen);
				if(ret)
				{
					iFeiJiLen++;
				}
			} while (ret);

			if( iFeiJiLen > 2 )
			{
				bool bSpecial = true;
				AddFeiJiToNode(tVecSanZhang[i],iFeiJiLen-1,tVecSanZhang,eAddCardType_No,tNode);
				if(find(tNode.vecMainCard.begin(), tNode.vecMainCard.end(), m_byMinCard) != tNode.vecMainCard.end())
				{
					bSpecial = false;
				}
				//清理三张数据
				for(auto subIter = tNode.vecMainCard.begin();subIter != tNode.vecMainCard.end();subIter++ )
				{
					DeleteParamByValue(tVecSanZhang,*subIter);
				}

				//任何情况，首出不带的牌：A、2、王、炸
				for(auto iter = tVecDanZhang.begin();iter != tVecDanZhang.end();)
				{
					if( (*iter) == 1 || (*iter) == 2 || (*iter) == 14 || (*iter) == 15  )
					{
						iter = tVecDanZhang.erase(iter);
					}
					else
					{
						iter++;
					}
				}
				///   对张 处理不带 A 2
				for(auto iter = tVecDuiZi.begin();iter != tVecDuiZi.end();)
				{
					if( (*iter) == 1 || (*iter) == 2  )
					{
						iter = tVecDuiZi.erase(iter);
					}
					else
					{
						iter++;
					}
				}
				///  飞机带牌
				int iFeiJiLen = tNode.vecMainCard.size() / 3;
				GetSubCardEx(eAddCardMainType_FeiJi,tVecSanZhang, tVecDuiZi,tVecDanZhang,iFeiJiLen,tNode.vecSubCard,tNode.eAddCardType,bSpecial);

				//找到了飞机
				return ;
			}
			else
			{
				i++;
			}
		}
		//没找到飞机
		return ;
	}
	//组合连队
	void CHandsNumberAndWeight::MakeLianDui(SCombinationNode &tNode)
	{
		SVBYTE tVecDuiZi;
		tVecDuiZi.clear();
		for(int i = 1;i < 16; i++)
		{
			if( m_byCardData[i] >= 2 )
			{
				tVecDuiZi.push_back(i);
			}
		}

		int iLianDuiLen = 0;
		{
			iLianDuiLen = 3;
			bool ret = false;
			do 
			{
				ret = IsShunZi(tVecDuiZi, m_byMinCard, 1, iLianDuiLen);
				if(ret)
				{
					iLianDuiLen++;
				}
			} while (ret);

			if( iLianDuiLen > 3 )
			{
				tNode.eCardType = eCardType_LianDui;
				for(int i = 0; i < iLianDuiLen -1; ++i)
				{
					AddParamByValue(tNode.vecMainCard, m_byMinCard + i, 2);
				}
				//找到连队了
			}
			else
			{
				//没找到
			}
		}
	}

	//组合对子
	void CHandsNumberAndWeight::MakeDuiZi(SCombinationNode &tNode)
	{
		if(m_byCardData[m_byMinCard] == 2)
		{
			tNode.eCardType = eCardType_DuiZi;
			AddParamByValue(tNode.vecMainCard, m_byMinCard, 2);
		}
	}

	//组合单张
	void CHandsNumberAndWeight::MakeDanZhang(SCombinationNode &tNode)
	{
		if(m_byCardData[m_byMinCard] == 1)
		{
			tNode.eCardType = eCardType_DanZhang;
			AddParamByValue(tNode.vecMainCard, m_byMinCard);
		}
	}

	//组合3带1
	void CHandsNumberAndWeight::Make3Dai1(SCombinationNode &tNode)
	{
		SVBYTE tVecDanZhang;							 /// 单张  必须是有序的 下面算法均是在有序基础上实现
		SVBYTE tVecDuiZi;								 /// 对子  必须是有序的 下面算法均是在有序基础上实现
		SVBYTE tVecSanZhang;							 /// 三张  必须是有序的 下面算法均是在有序基础上实现
		SVBYTE tVecSiZhang;								 /// 四张  必须是有序的 下面算法均是在有序基础上实现
		std::vector<SZhaDanLongNode> tVecZhaDanLong;     /// 四张以上  必须是有序的 下面算法均是在有序基础上实现
		ClassificationData(m_byCardData,tVecDanZhang,tVecDuiZi,tVecSanZhang,tVecSiZhang,tVecZhaDanLong);

		if(!tVecSanZhang.empty())
		{
			BYTE byCard = tVecSanZhang[0];
			for(int i = 1; i < (int)tVecSanZhang.size(); ++i)
			{
				if(SpecialComparisonFun(byCard, tVecSanZhang[i]))
				{
					byCard = tVecSanZhang[i];
				}
			}

			bool bSpecial = true;
			if( byCard == m_byMinCard)
			{
				bSpecial = false;
			}
			AddSanZhangToNode(byCard, 0, eAddCardType_No, tNode);

			//清理三张数据
			for(auto subIter = tNode.vecMainCard.begin();subIter != tNode.vecMainCard.end();subIter++ )
			{
				DeleteParamByValue(tVecSanZhang,*subIter);
			}

			//log
			//FILE *fp = fopen("123.txt", "a");
			//fprintf(fp, "HandsNumberAndWeight.cpp,sandaiyi,497");
			//fclose(fp);

			//任何情况，首出不带的牌：A、2、王、炸
			for(auto iter = tVecDanZhang.begin();iter != tVecDanZhang.end();)
			{
				if( (*iter) == 1 || (*iter) == 2 || (*iter) == 14 || (*iter) == 15  )
				{
					iter = tVecDanZhang.erase(iter);
				}
				else
				{
					iter++;
				}
			}
			///   对张 处理不带 A 2
			for(auto iter = tVecDuiZi.begin();iter != tVecDuiZi.end();)
			{
				if( (*iter) == 1 || (*iter) == 2  )
				{
					iter = tVecDuiZi.erase(iter);
				}
				else
				{
					iter++;
				}
			}

			GetSubCardEx(eAddCardMainType_SanZhang,tVecSanZhang, tVecDuiZi,tVecDanZhang,1,tNode.vecSubCard,tNode.eAddCardType, bSpecial);
		}
	}
	//删除首出牌
	void CHandsNumberAndWeight::DeleteFirstOut()
	{
		for(std::vector<unsigned char>::iterator it = m_tMaxNode.vecMainCard.begin(); it != m_tMaxNode.vecMainCard.end(); it++)
		{
			m_byCardData[*it]--;
		}

		for(std::vector<unsigned char>::iterator it = m_tMaxNode.vecSubCard.begin(); it != m_tMaxNode.vecSubCard.end(); it++)
		{
			m_byCardData[*it]--;
		}
	}

	//mark
	//获取首出该出的牌
	//首出&主动出牌，不能包含A、2、王，必须包含最小牌。（牌型组成优先级：飞机＞顺子＞连对＞三带一（对）＞对子＞单牌）
	//非首出&主动出牌，必须包含最小牌。（牌型组成优先级：飞机＞顺子＞连对＞三带一（对）＞对子＞单牌）
	//炸弹不能拆
	void CHandsNumberAndWeight::GetFirstOutCard()
	{
		m_vecCombinationResult.clear();

		CARDATA byCardData;
		if(g_bFirstTimeNtOut)
		{
			//先备份数据			
			memcpy(byCardData, m_byCardData, sizeof(m_byCardData));
			//任何情况，首出不带的牌：A、2、王、炸
			for(int i = 0; i < 16; ++i)
			{
				if(i == 1 || i == 2 || i == 14 || i == 15)
				{
					m_byCardData[i] = 0;
				}
			}
		}

		//计算最小牌，不包含炸弹(为下面的组牌服务)
		const int orderCard[] = {3,4,5,6,7,8,9,10,11,12,13,1,2,14,15};
		for(int card = 0; card < sizeof(orderCard) / sizeof(orderCard[0]); card++)
		{
			if(m_byCardData[ orderCard[card] ] != 4 && m_byCardData[orderCard[card]] != 0)
			{
				//最小牌是小王又有王炸时不算最小牌
				if( orderCard[card] == 14 && (m_byCardData[14] >= 1 && m_byCardData[15] >= 1) ) break;;
				m_byMinCard = orderCard[card];
				break;
			}
		}

		SCombinationNode tNode;
		//只剩小王和大王或大王
		if(m_byMinCard == 255)
		{
			tNode.eCardType = eCardType_ZhaDan;
			//只剩炸弹出炸弹
			for(int i = 0; i < 16; ++i)
			{
				if(m_byCardData[i] >= 4)
				{
					AddParamByValue(tNode.vecMainCard, i, 4);
					break;
				}
			}
			//没有普通炸弹出王炸
			if(tNode.vecMainCard.empty())
			{
				AddParamByValue(tNode.vecMainCard, 14);
				AddParamByValue(tNode.vecMainCard, 15);
			}
		}
		else
		{
			//炸弹不可拆，先备份手牌后还原
			CARDATA byCardData;			
			memcpy(byCardData, m_byCardData, sizeof(m_byCardData));
			//去除炸弹
			for(int i = 0; i < 16; ++i)
			{
				if(m_byCardData[i] >= 4)
				{
					m_byCardData[i] = 0;
				}
			}
			//20190424 修改连对大于顺子
			//出牌顺序:飞机->连对->顺子->三带一->对子->单张
			if(tNode.eCardType == eCardType_Unknown)
			{
				MakeFeiJi(tNode);
			}
			if (tNode.eCardType == eCardType_Unknown)
			{
				MakeLianDui(tNode);
			}
			if(tNode.eCardType == eCardType_Unknown)
			{
				MakeShunZi(tNode);
			}
			if(tNode.eCardType == eCardType_Unknown)
			{
				Make3Dai1(tNode);
			}
			if(tNode.eCardType == eCardType_Unknown)
			{
				MakeDuiZi(tNode);
			}
			if(tNode.eCardType == eCardType_Unknown)
			{
				MakeDanZhang(tNode);
			}
			//还原手牌
			memcpy(m_byCardData, byCardData, sizeof(m_byCardData));
		}
		m_tMaxNode = tNode;
		//组牌结果推进
		m_vecCombinationResult.push_back(tNode);
		if(g_bFirstTimeNtOut)
		{
			//处理完后还原数据
			memcpy(m_byCardData, byCardData, sizeof(m_byCardData));
		}
		return;
	}

	/// 出牌算法
	bool CHandsNumberAndWeight::GetPutCardList(const unsigned char *pSrcCard,int iSrcCardLen,SCombinationResult & sCombinationResult)
	{
		if( iSrcCardLen == 0 || pSrcCard==nullptr)
		{
			return false;
		}
#if LOG_ALL
		CTime STime = CTime::GetCurrentTime();
		std::string strBuf;
		LOGMODULEINSTANCE.LogCharToChar((unsigned char *)pSrcCard,strBuf,iSrcCardLen);;
		LOGMODULEINSTANCE.WriteLog("%02d,%02d,%02d,PutCardList_Start,ShouPai:%s\n",STime.GetHour(),STime.GetMinute(),STime.GetSecond(),strBuf.c_str());
#endif
		
		Init();
		sCombinationResult.clear();

		//适配
		ChangeHandPaiData(pSrcCard,iSrcCardLen);  ///// 初始化手牌数据 

		//如果首出，先获得出牌结果，然后从手牌中去掉后，剩下的按原有算法组合。
		if(m_bFirstOut)
		{
			//mark 算出可出牌入口
			GetFirstOutCard();

			sCombinationResult.vecCombinationResult.insert(sCombinationResult.vecCombinationResult.end(),m_vecCombinationResult.begin(),m_vecCombinationResult.end());

			//???
			DeleteFirstOut();
		}

		//// 首先 获取出 小鬼和大鬼 和 2 //////////////////
		m_byCombinationData[14] =  m_byCardData[14];
		m_byCombinationData[15] =  m_byCardData[15];
		m_byCombinationData[2] =   m_byCardData[2];
		m_byCardData[14] = 0;
		m_byCardData[15] = 0;
		m_byCardData[2] = 0;

		///// 拆牌 /////////////////////////
		GetCombinationList();

		/// 结果排序
		ResultSort(m_vecCombinationResult,4);
		ResultSort(m_vecResultNoWith,4);

		/// 保存结果
		//sCombinationResult.clear();
		sCombinationResult.iMaxValue = m_iMaxValue;
		sCombinationResult.iMinHandsNumber = m_iMinHandsNumber;
		memcpy(sCombinationResult.byTypeCount, m_byTypeCount, sizeof(m_byTypeCount));
		sCombinationResult.vecCombinationResult.insert(sCombinationResult.vecCombinationResult.end(),m_vecCombinationResult.begin(),m_vecCombinationResult.end());
		sCombinationResult.vecResultNoWith.insert(sCombinationResult.vecResultNoWith.end(),m_vecResultNoWith.begin(),m_vecResultNoWith.end());
#if LOG_ALL
		CTime ETime = CTime::GetCurrentTime();
		int iTimeInterval = (ETime.GetHour() - STime.GetHour())*3600+(ETime.GetMinute()-STime.GetMinute())*60+ETime.GetSecond()-STime.GetSecond();
		for(auto iter = sCombinationResult.vecCombinationResult.begin();iter!=sCombinationResult.vecCombinationResult.end();iter++)
		{
			std::string strBuf;
			unsigned char strData[21]={0};
			int iLen = GetMainAndSubByCombinationNode(strData,21,*iter);
			LOGMODULEINSTANCE.LogCharToChar(strData,strBuf,iLen);
			LOGMODULEINSTANCE.WriteLog("%02d,%02d,%02d,PutCardList_List,Result:%s\n",ETime.GetHour(),ETime.GetMinute(),ETime.GetSecond(),strBuf.c_str());
		}
		LOGMODULEINSTANCE.WriteLog("%02d,%02d,%02d,PutCard_End,Time:%d\n",ETime.GetHour(),ETime.GetMinute(),ETime.GetSecond(),iTimeInterval);
#endif
		return true;
	}
	bool CHandsNumberAndWeight::GetPutCardList(const unsigned char *pSrcCard,int iSrcCardLen,std::vector<SCombinationNode> & vecCombinationResult)
	{
		if( iSrcCardLen == 0 || pSrcCard==nullptr )
		{
			return false;
		}
		SCombinationResult sCombinationResult;
		GetPutCardList(pSrcCard,iSrcCardLen,sCombinationResult);
		vecCombinationResult.clear();
		vecCombinationResult.insert(vecCombinationResult.end(),sCombinationResult.vecCombinationResult.begin(),sCombinationResult.vecCombinationResult.end());
		return true;
	}
	/// 癞子出牌
	bool CHandsNumberAndWeight::GetPutCardList(const unsigned char *pSrcCard,int iSrcCardLen,int iLaiZiNumber,std::vector<SCombinationNode> & vecCombinationResult)
	{
		if( pSrcCard==nullptr || (iSrcCardLen == 0 && iLaiZiNumber == 0 ) )
		{
			return false;
		}
		SCombinationResult  sCombinationResult;
		GetPutCardList(pSrcCard,iSrcCardLen,iLaiZiNumber,sCombinationResult);
		vecCombinationResult.clear();
		vecCombinationResult.insert(vecCombinationResult.end(),sCombinationResult.vecCombinationResult.begin(),sCombinationResult.vecCombinationResult.end());
		return true;
	}
	bool CHandsNumberAndWeight::GetPutCardList(const unsigned char *pSrcCard,int iSrcCardLen,int iLaiZiNumber,SCombinationResult & sCombinationResult)
	{
		if( pSrcCard==nullptr || (iSrcCardLen == 0 && iLaiZiNumber == 0 ) )
		{
			return false;
		}
#if LOG_ALL
		CTime STime = CTime::GetCurrentTime();
		std::string strBuf;
		LOGMODULEINSTANCE.LogCharToChar((unsigned char *)pSrcCard,strBuf,iSrcCardLen);;
		LOGMODULEINSTANCE.WriteLog("%02d,%02d,%02d,PutCardList_L_Start,ShouPai:%s,iLaiZi:%d\n",STime.GetHour(),STime.GetMinute(),STime.GetSecond(),strBuf.c_str(),iLaiZiNumber);
#endif
		/// 获取出没有癞子牌的 最优结果
		GetPutCardList(pSrcCard,iSrcCardLen,sCombinationResult);
		/// 在最优组合上 添加癞子牌 形成新的最优组合
		//sCombinationResult.iLaiZiNumber = iLaiZiNumber;
		//GetCombinationByLaiZi(iLaiZiNumber,sCombinationResult);
		sCombinationResult.iLaiZiNumber = iLaiZiNumber;           /// 还原回癞子数
		if(m_bFirstOut)
		{
			//sCombinationResult.vecCombinationResult.clear();
			//sCombinationResult.vecCombinationResult.insert(sCombinationResult.vecCombinationResult.end(), m_vecMinCArdResult.begin(), m_vecMinCArdResult.end());
			ResultSort(sCombinationResult.vecCombinationResult, 5);
		}
		else
		{
			/// 结果排序
			ResultSort(sCombinationResult.vecResultNoWith,4);
			ResultSort(sCombinationResult.vecCombinationResult,4);
		}
		
#if LOG_ALL
		CTime ETime = CTime::GetCurrentTime();
		int iTimeInterval = (ETime.GetHour() - STime.GetHour())*3600+(ETime.GetMinute()-STime.GetMinute())*60+ETime.GetSecond()-STime.GetSecond();
		for(auto iter = sCombinationResult.vecCombinationResult.begin();iter!=sCombinationResult.vecCombinationResult.end();iter++)
		{
			std::string strBuf,laiZiBuf;
			unsigned char strData[21]={0};
			int iLen = GetMainAndSubByCombinationNode(strData,21,*iter);
			LOGMODULEINSTANCE.LogCharToChar(strData,strBuf,iLen);
			for(auto iLaizi = (*iter).vecLaiZiCard.begin(); iLaizi !=(*iter).vecLaiZiCard.end();iLaizi++ )
			{
				char tem[10];
				sprintf(tem,"%d ",(*iLaizi)%16);
				laiZiBuf += std::string(tem);
			}
			LOGMODULEINSTANCE.WriteLog("%02d,%02d,%02d,PutCardList_L_List,Result:%s_%s\n",ETime.GetHour(),ETime.GetMinute(),ETime.GetSecond(),strBuf.c_str(),laiZiBuf.c_str());
		}
		LOGMODULEINSTANCE.WriteLog("%02d,%02d,%02d,PutCard_L_End,Time:%d\n",ETime.GetHour(),ETime.GetMinute(),ETime.GetSecond(),iTimeInterval);
#endif
		return true;
	}
	/// 跟牌算法
	bool CHandsNumberAndWeight::GetFollowCardList(const unsigned char *pSrcCard,int iSrcCardLen,const unsigned char *pLastCard,int iLastCardLen,std::vector<SCombinationNode> & vecFollowCardList)
	{
		if( iSrcCardLen == 0 || pSrcCard==nullptr || pLastCard == nullptr || iLastCardLen == 0)
		{
			return false;
		}
		SCombinationNode tLastCard;
		GetCardType(pLastCard,iLastCardLen,tLastCard);
		return GetFollowCardList(pSrcCard,iSrcCardLen,tLastCard,vecFollowCardList);
	}
	bool CHandsNumberAndWeight::GetFollowCardList(const unsigned char *pSrcCard,int iSrcCardLen,const SCombinationNode & tLastCard,std::vector<SCombinationNode> & vecFollowCardList)
	{
#if 0
		CTime STime = CTime::GetCurrentTime();
		string strBuf,lastBuf;
		unsigned char lastData[21]={0};
		LOGMODULEINSTANCE.LogCharToChar((unsigned char *)pSrcCard,strBuf,iSrcCardLen);
		int iLen = GetMainAndSubByCombinationNode(lastData,21,tLastCard);
		LOGMODULEINSTANCE.LogCharToChar(lastData,lastBuf,iLen);
		LOGMODULEINSTANCE.WriteLog("%02d,%02d,%02d,FollowList_Start,ShouPai:%s,LastCart:%s\n",STime.GetHour(),STime.GetMinute(),STime.GetSecond(),strBuf.c_str(),lastBuf.c_str());
#endif
		ChangeHandPaiData(pSrcCard,iSrcCardLen);  ///// 初始化手牌数据
		if( tLastCard.eCardType == eCardType_ZhaDan && tLastCard.eAddCardType == eAddCardType_No )
		{
			/// 获取出炸弹
			GetFollowListZhaDan(m_byCardData,tLastCard,0,vecFollowCardList);
		}
		else
		{
		    /// 获取 跟牌数组
		    GetFollowCardListNoZhaDan(m_byCardData,tLastCard,vecFollowCardList);
		    /// 获取出炸弹
		    GetFollowListZhaDan(m_byCardData,tLastCard,0,vecFollowCardList);
		}
		/// 结果排序
		ResultSort(vecFollowCardList,0);
#if 0
		CTime ETime = CTime::GetCurrentTime();
		int iTimeInterval = (ETime.GetHour() - STime.GetHour())*3600+(ETime.GetMinute()-STime.GetMinute())*60+ETime.GetSecond()-STime.GetSecond();
	    for(auto iter = vecFollowCardList.begin();iter!=vecFollowCardList.end();iter++)
		{
			string lastBuf;
			unsigned char lastData[21]={0};
			int iLen = GetMainAndSubByCombinationNode(lastData,21,*iter);
			LOGMODULEINSTANCE.LogCharToChar(lastData,lastBuf,iLen);
		   LOGMODULEINSTANCE.WriteLog("%02d,%02d,%02d,FollowList_List,Result:%s\n",ETime.GetHour(),ETime.GetMinute(),ETime.GetSecond(),lastBuf.c_str());
		}
		LOGMODULEINSTANCE.WriteLog("%02d,%02d,%02d,FollowList_End,Time:%d\n",ETime.GetHour(),ETime.GetMinute(),ETime.GetSecond(),iTimeInterval);
#endif
		return true;
	}
	/// 癞子跟牌算法
	bool CHandsNumberAndWeight::GetFollowCardList(const unsigned char *pSrcCard,int iSrcCardLen,const SCombinationNode & tLastCard,int iLaiZiNumber,std::vector<SCombinationNode> & vecFollowCardList)
	{
		if( (iSrcCardLen == 0 && iLaiZiNumber == 0 ) || pSrcCard==nullptr || tLastCard.vecMainCard.size() == 0 )
		{
			return false;
		}
#if LOG_ALL
		CTime STime = CTime::GetCurrentTime();
		std::string strBuf,lastBuf;
		unsigned char lastData[21]={0};
		LOGMODULEINSTANCE.LogCharToChar((unsigned char *)pSrcCard,strBuf,iSrcCardLen);
		int iLen = GetMainAndSubByCombinationNode(lastData,21,tLastCard);
		LOGMODULEINSTANCE.LogCharToChar(lastData,lastBuf,iLen);
		LOGMODULEINSTANCE.WriteLog("%02d,%02d,%02d,FollowList_L_Start,ShouPai:%s,LastCart:%s,LaiZi:%d\n",STime.GetHour(),STime.GetMinute(),STime.GetSecond(),strBuf.c_str(),lastBuf.c_str(),iLaiZiNumber);
#endif
		if( tLastCard.eCardType == eCardType_ZhaDan && tLastCard.eAddCardType == eAddCardType_No )
		{
			CARDATA tSrcData = {0};
			ChangeHandPaiData(pSrcCard,iSrcCardLen);
			memcpy(tSrcData,m_byCardData,sizeof(tSrcData));
			GetFollowListZhaDan(tSrcData,tLastCard,iLaiZiNumber,vecFollowCardList);
		}
		else if( tLastCard.eCardType != eCardType_Unknown )
		{
			ChangeHandPaiData(pSrcCard,iSrcCardLen);
			/// 获取出没有癞子能跟的牌
			GetFollowCardListNoZhaDan(m_byCardData,tLastCard,vecFollowCardList);
			/// 获取出所有能跟的炸弹
			GetFollowListZhaDan(m_byCardData,tLastCard,iLaiZiNumber,vecFollowCardList);
			/// 获取出非炸弹能跟的牌
			CARDATA tSrcData = {0};
			memcpy(tSrcData,m_byCardData,sizeof(tSrcData));
			GetFollowListNoZhaDanByLaiZi(tSrcData,tLastCard,iLaiZiNumber,vecFollowCardList);
		}
		//// 结果排序
		ResultSort(vecFollowCardList,2);
#if LOG_ALL
		CTime ETime = CTime::GetCurrentTime();
		int iTimeInterval = (ETime.GetHour() - STime.GetHour())*3600+(ETime.GetMinute()-STime.GetMinute())*60+ETime.GetSecond()-STime.GetSecond();
		for(auto iter = vecFollowCardList.begin();iter!=vecFollowCardList.end();iter++)
		{
			std::string lastBuf,laiZiBuf;
			unsigned char lastData[21]={0};
			int iLen = GetMainAndSubByCombinationNode(lastData,21,*iter);
			for(int i=0;i<iLen;i++)
			{
				char tem[10];
				sprintf(tem,"%d ",(lastData[i])%16);
				lastBuf += std::string(tem);
			}
			for(auto iLaizi = (*iter).vecLaiZiCard.begin(); iLaizi !=(*iter).vecLaiZiCard.end();iLaizi++ )
			{
				char tem[10];
				sprintf(tem,"%d ",(*iLaizi)%16);
				laiZiBuf += std::string(tem);
			}
			LOGMODULEINSTANCE.WriteLog("%02d,%02d,%02d,FollowList_L_List,Result:%s_%s\n",ETime.GetHour(),ETime.GetMinute(),ETime.GetSecond(),lastBuf.c_str(),laiZiBuf.c_str());
		}
		LOGMODULEINSTANCE.WriteLog("%02d,%02d,%02d,FollowList_L_End,Time:%d\n",ETime.GetHour(),ETime.GetMinute(),ETime.GetSecond(),iTimeInterval);
#endif
		return true;
	}
    /// 叫分算法 
	int CHandsNumberAndWeight::GetCallScore(const unsigned char *pSrcCard,int iSrcCardLen,int iMinScore,int iMaxScore)
	{
		/*
		1、手上有1个王+1个2以上时必抢地主
		2、当手上只有1个2时或没有2时，不抢地主
		3、当手上炸弹数量>2个时，必抢地主
		4、当手上癞子数量>2张时，必抢地主
		*/
		if( iSrcCardLen == 0 || pSrcCard==nullptr)
		{
			return false;
		}
		ChangeHandPaiData(pSrcCard,iSrcCardLen);  ///// 初始化手牌数据
		if( ( m_byCardData[14] > 0 || m_byCardData[15] > 0 ) &&  m_byCardData[2] > 0  )
		{
		    return iMaxScore;
		}
		else if(m_byCardData[2] <= 1)
		{
		    return 0;
		}
		////确定炸弹个数
		int iZhaDanSize = 0;
		for(int i=1;i<14;i++)
		{
		   if( m_byCardData[i] == 4 )
		   {
		       iZhaDanSize++;
		   }
		}
		if( m_byCardData[14] ==1 && m_byCardData[15] ==1 )
		{
		   iZhaDanSize++;
		}
		if( iZhaDanSize > 2 )
		{
		   return iMaxScore;
		}
		return false;
	}
	///// 辅助函数 //////////////////////////////////////////////
	bool CHandsNumberAndWeight::GetNodeListByFilter(const std::vector<SCombinationNode> & tSrcList,std::vector<SCombinationNode> & tDstList,const SCombinationNode & tFilterNode)
	{
		tDstList.clear();
		for(auto iter = tSrcList.begin();iter != tSrcList.end();iter++)
		{
			int ret = CompareNodeFun(*iter,tFilterNode);
			if( ret == 1 )
			{
				tDstList.push_back(*iter);
			}
		}
		if( tDstList.size() > 0  )
		{
			return true;
		}
		return false;
	}
	/// 获取牌类型
	bool CHandsNumberAndWeight::GetCardType(const unsigned char *pSrcCard,int iSrcCardLen,SCombinationNode & tCardType)
	{
		if( iSrcCardLen == 0 || pSrcCard==nullptr)
		{
			return false;
		}
		ChangeHandPaiData(pSrcCard,iSrcCardLen);  ///// 初始化手牌数据
		return GetCardTypeBySrc(m_byCardData,tCardType);
	}
	bool CHandsNumberAndWeight::GetCardType(const unsigned char *pSrcCard,int iSrcCardLen,int iLaiZiLen,std::vector<SCombinationNode> & tCardList)
	{
		if( pSrcCard==nullptr || ( iSrcCardLen == 0 && iLaiZiLen == 0 ) )
		{
			return false;
		}
		if(iSrcCardLen == 0 && iLaiZiLen > 0)   /// 纯癞子特殊处理
		{
			SCombinationNode tCombinationNode;
			AddAllLaiZiToNode(iLaiZiLen,tCombinationNode);
			tCardList.push_back(tCombinationNode);
		}
		else										  ////  不是纯癞子 处理
		{
			ChangeHandPaiData(pSrcCard,iSrcCardLen);  ///// 初始化手牌数据
			CARDATA tSrcData = {0};
			memcpy(tSrcData,m_byCardData,sizeof(tSrcData));
			GetCardTypeByLaiZi(tSrcData,1,iLaiZiLen,tCardList);
			//// 构造长炸弹
			GetLongZhaDanTypeByLaiZi(m_byCardData,iLaiZiLen,tCardList);
		}
		if( tCardList.size() > 0 )
		{
			return true;
		}
		return false;
	}

	int CHandsNumberAndWeight::GetMainByCombinationNode(unsigned char *pSrcCard,int iSrcCardLen,const SCombinationNode & tCombinationNode)
	{
		if( pSrcCard == nullptr)
		{
			return 0;
		}
		int i = 0;
		int iMainSize = tCombinationNode.vecMainCard.size();
		for(i=0;i < iSrcCardLen && i< iMainSize;i++)
		{
			pSrcCard[i] = tCombinationNode.vecMainCard[i];
		}
		return i;
	}
	int CHandsNumberAndWeight::GetSubByCombinationNode(unsigned char *pSrcCard,int iSrcCardLen,const SCombinationNode & tCombinationNode)
	{
		if( pSrcCard == nullptr)
		{
			return 0;
		}
		int i = 0;
		int iSubSize = tCombinationNode.vecSubCard.size();
		for(i=0;i < iSrcCardLen && i< iSubSize;i++)
		{
			pSrcCard[i] = tCombinationNode.vecSubCard[i];
		}
		return i;
	}
	int CHandsNumberAndWeight::GetMainAndSubByCombinationNode(unsigned char *pSrcCard,int iSrcCardLen,const SCombinationNode & tCombinationNode)
	{
		if( pSrcCard == nullptr)
		{
			return 0;
		}
		int i = 0;
		for(auto iter = tCombinationNode.vecMainCard.begin();i< iSrcCardLen && iter != tCombinationNode.vecMainCard.end();iter++)
		{
			pSrcCard[i++] = *iter;
		}
		for(auto iter = tCombinationNode.vecSubCard.begin();i< iSrcCardLen && iter != tCombinationNode.vecSubCard.end();iter++)
		{
			pSrcCard[i++] = *iter;
		}
		return i;
	}
	////////////////////  私有方法 ///////////////////////////////////////////////////
	//// 主动出牌 癞子算法///////////////////////////////////////////////////////////
	void CHandsNumberAndWeight::GetCombinationByLaiZi(int iLaiziNumber,SCombinationResult & tCombinationResult) const
	{
		if( iLaiziNumber <= 0 )
		{
			GetMaxCombinationByLaiZi(tCombinationResult);
		    return;
		}
		///// 癞子优先级 癞子优先形成牌可视情况而定 ////
		do 
		{
			bool ret = false;
			///1、先判断 加癞子 单张牌中是否形成顺子
            ret = IsMakeShunZi(tCombinationResult);
			if( ret == true)
			{
				tCombinationResult.iLaiZiNumber--;
			    break;
			}
			///2、3张形成炸弹
			ret = IsMakeZhaDan(tCombinationResult);
			if( ret == true)
			{
				tCombinationResult.iLaiZiNumber--;
				break;
			}
			///3、形成3张
			ret = IsMakeSanZhang(tCombinationResult);
			if( ret == true)
			{
				tCombinationResult.iLaiZiNumber--;
				break;
			}
			///4、让炸弹更长 如果支持长炸弹
			if( m_sSysConfig.bIsLongBomb == true )
			{
				ret = IsMakeZhaDanLonger(tCombinationResult);
				if( ret == true)
				{
					tCombinationResult.iLaiZiNumber--;
					break;
				}
			}
			///5、形成顺子 让顺子更长
		    ret = IsMakeShunZiMore(tCombinationResult);
			if( ret == true)
			{
				tCombinationResult.iLaiZiNumber--;
				break;
			}
			///6、形成对子
		    ret = IsMakeDuiZi(tCombinationResult);
			if( ret == true)
			{
				tCombinationResult.iLaiZiNumber--;
				break;
			}
			///7、单张  0 此刻不需要癞子
		} while (false);
		//// 继续寻找癞子
		GetCombinationByLaiZi(iLaiziNumber-1,tCombinationResult);
	}
	void CHandsNumberAndWeight::GetMaxCombinationByLaiZi(SCombinationResult & tCombinationResult) const
	{
		//// 重新组合带牌组合 
		tCombinationResult.iMaxValue = 0;
		tCombinationResult.iMinHandsNumber = 0;
		tCombinationResult.vecCombinationResult.clear();
		CARDATA tCombinationData;  //// 牌 1~K 1~13 小鬼和大鬼 14 15  0号下标预留
		memset(tCombinationData,0,sizeof(tCombinationData));
		for(auto iter =tCombinationResult.vecResultNoWith.begin();iter != tCombinationResult.vecResultNoWith.end();iter++ )
		{
			if( (*iter).eCardType == eCardType_ShunZi )
			{
				tCombinationResult.iMaxValue += (*iter).vecMainCard.size() -1;
				tCombinationResult.iMinHandsNumber++;
				tCombinationResult.vecCombinationResult.push_back(*iter);
			}
			else if( (*iter).eCardType == eCardType_ZhaDan && (*iter).vecMainCard.size() == 2 )  ///王炸
			{
				tCombinationResult.iMaxValue += 7;
				tCombinationResult.iMinHandsNumber++;
				tCombinationResult.vecCombinationResult.push_back(*iter);
			}
			else
			{
				if( (*iter).vecMainCard.size()!=0 && (*iter).vecMainCard.front() >=1 && (*iter).vecMainCard.front() < 16 )
				{
					unsigned char ucTemp = (*iter).vecMainCard.front();
					switch( (*iter).eCardType )
					{
					case eCardType_DanZhang:
						tCombinationData[ucTemp] = 1;
						break;
					case eCardType_DuiZi:
						tCombinationData[ucTemp] = 2;
						break;
					case eCardType_SanZhang:
						tCombinationData[ucTemp] = 3;
						break;
					case eCardType_ZhaDan:
						tCombinationData[ucTemp] = (*iter).vecMainCard.size();
						break;
					default:
						break;
					}
				}
				else
				{
					//// 非法数据
				}
			}
		}
		/// 获取带牌组合
		GetCombinationResult(tCombinationData,tCombinationResult.vecCombinationResult,tCombinationResult.iMaxValue,tCombinationResult.iMinHandsNumber);
		/// 癞子牌赋值
		int iResultSize = tCombinationResult.vecCombinationResult.size();
		for(int i=0;i<iResultSize;i++)
		{
			for(auto iter = tCombinationResult.vecResultNoWith.begin();iter != tCombinationResult.vecResultNoWith.end();iter++)
			{
				if( iter->eCardType == eCardType_ShunZi )  //// 顺子在第一步已经处理
				{
				    continue;
				}
				bool ret = IsEqualByNode((*iter),tCombinationResult.vecCombinationResult[i],1);
				if( ret )
				{
					tCombinationResult.vecCombinationResult[i].vecLaiZiCard.insert(tCombinationResult.vecCombinationResult[i].vecLaiZiCard.end(),
						(*iter).vecLaiZiCard.begin(),(*iter).vecLaiZiCard.end()
						);
				}
			}
		}
		/// 添加剩余癞子 组合
		if( tCombinationResult.iLaiZiNumber > 0 )
		{
			SCombinationNode tCombinationNode;
			AddAllLaiZiToNode(tCombinationResult.iLaiZiNumber,tCombinationNode);
			tCombinationResult.vecResultNoWith.push_back(tCombinationNode);
			tCombinationResult.vecCombinationResult.push_back(tCombinationNode);
		}	
	}
	bool CHandsNumberAndWeight::IsMakeShunZi(SCombinationResult & tCombinationResult) const
	{
		//// 获取出所有单张牌 ////////////////////////////
		//// 牌 1~K 1~13 小鬼和大鬼 14 15  0号下标预留
		CARDATA tbyCardData;		 ////  初始数据
		int iResultNoWithSize = tCombinationResult.vecResultNoWith.size();
		memset(tbyCardData,0,sizeof(tbyCardData));
		for(int i=0;i<iResultNoWithSize;i++)
		{
			if(  tCombinationResult.vecResultNoWith[i].eCardType == eCardType_DanZhang )
			{
				if( tCombinationResult.vecResultNoWith[i].vecMainCard.size() ==0  )
				{
					continue;
				}
				unsigned char ucTemp = tCombinationResult.vecResultNoWith[i].vecMainCard.front();
				if( ucTemp >= 1 && ucTemp <= 13 && ucTemp != 2)
				{
				    tbyCardData[ucTemp]++;
				}
			}
		}
		/// 寻找顺子 //////////////////////////////////
		for(int i=1;i<=13;i++)
		{
		    if(i==2)
			{
			    continue;;
			}
			bool ret = IsShunZi(tbyCardData,i,0);
			/// 可以形成顺子
			if(ret == true)
			{
				/// 寻找最长顺子
				int iShunZiLen = 5;
			    while( IsShunZi(tbyCardData,i,0,iShunZiLen+1) )   
				{
				   iShunZiLen++;
				}
				/// 获取顺子的起点
				int iShunZiStart = 3;
				if( i == 1 )
				{
				    iShunZiStart = 13 - iShunZiLen +2;
				}
				else
				{
				   iShunZiStart = i;
				   while( tbyCardData[iShunZiStart-1] > 0 && iShunZiStart >= 4 )
				   {
				       iShunZiStart--;
				   }
				}
				/// 添加新组合
				SCombinationNode tCombinationNode;
			    AddShunZiToNode(iShunZiStart,iShunZiLen,tCombinationNode);
				AddParamByValue(tCombinationNode.vecLaiZiCard,i);
				tCombinationResult.vecResultNoWith.push_back(tCombinationNode);
				/// 删除形成顺子的单张
				for(auto tIter = tCombinationNode.vecMainCard.begin();tIter != tCombinationNode.vecMainCard.end();tIter++)
				{
				    if( (*tIter) == i )
					{
					    continue;
					}
					for(auto iter = tCombinationResult.vecResultNoWith.begin();iter != tCombinationResult.vecResultNoWith.end();iter++)
					{
						if( (*iter).eCardType == eCardType_DanZhang )
						{
							if( (*iter).vecMainCard.size()!=0 && (*iter).vecMainCard.front() == (*tIter) )
							{
								tCombinationResult.vecResultNoWith.erase(iter);
								break;
							}
						}
					}
				}
			    return true;
			}
		}
		return false;
	}
	bool CHandsNumberAndWeight::IsMakeShunZiMore(SCombinationResult & tCombinationResult) const
	{
		SCombinationNode tTempNode;
		bool bRet = false; 
		for( auto iter =  tCombinationResult.vecResultNoWith.begin();iter!= tCombinationResult.vecResultNoWith.end();iter++ )
		{
			if( (*iter).eCardType == eCardType_ShunZi )
			{
				if( (*iter).vecMainCard.size() ==0 || (*iter).vecMainCard.size() == 12 )
				{
					continue;
				}
				unsigned char ucTmp = 0;
				if( (*iter).vecMainCard.back() == 13  )
				{
					ucTmp = 1;
				}
				else if( (*iter).vecMainCard.back() != 1 )
				{
					ucTmp = (*iter).vecMainCard.back()+1;
				}
				else if( (*iter).vecMainCard.front() > 3 )  /// 往前插入
				{
					ucTmp = (*iter).vecMainCard.front() -1;
				}
				if( ucTmp != 0)
				{
					//// 保存顺子 再次查询单张 是否能形成最大顺子 在结果中删除现有的顺子
					tTempNode = (*iter);
					tCombinationResult.vecResultNoWith.erase(iter);
					
					if( ucTmp == tTempNode.vecMainCard.front() -1  ) /// 前插
					{
						tTempNode.vecMainCard.insert(tTempNode.vecMainCard.begin(),ucTmp);
					}
					else
					{
						tTempNode.vecMainCard.push_back( ucTmp );
					}
					tTempNode.vecLaiZiCard.push_back(ucTmp);
					bRet = true;
					break;
				}
			}
		}

		if( bRet == false )
		{
		    return false;
		}
		//// 再次查询单张 是否能形成最大的顺子///////
		//// 获取出顺子的头和尾
		unsigned char ucStart = tTempNode.vecMainCard.front();
		unsigned char ucEnd =   tTempNode.vecMainCard.back();
		for(auto iter = tCombinationResult.vecResultNoWith.begin();iter!=tCombinationResult.vecResultNoWith.end();)
		{
			if( (*iter).eCardType == eCardType_DanZhang )
			{
				if( (*iter).vecMainCard.size() ==0  )
				{
					continue;
				}
				unsigned char ucCurValue = (*iter).vecMainCard.front();
				if( ucStart > 3 && ucStart == ucCurValue+1 ) /// 是否能加入顺子头
				{
					tCombinationResult.vecResultNoWith.erase(iter);
					iter = tCombinationResult.vecResultNoWith.begin();   /// 需要重头开始查询
					ucStart--;
					tTempNode.vecMainCard.insert(tTempNode.vecMainCard.begin(),ucStart);
					continue;
				}
				else if( ucEnd >= 7 && ucEnd < 13 && ucEnd == ucCurValue-1 ) /// 是否能加入顺子尾
				{
					iter = tCombinationResult.vecResultNoWith.erase(iter);
					ucEnd++;
					tTempNode.vecMainCard.push_back(ucEnd);
					continue;
				}
				else if( ucEnd == 13 && ucCurValue == 1 )
				{
					iter = tCombinationResult.vecResultNoWith.erase(iter);
					ucEnd = 1;
					tTempNode.vecMainCard.push_back(ucEnd);
					continue;
				}
			}
            iter++;
		}
		//// 添加回更长的顺子
		tCombinationResult.vecResultNoWith.push_back(tTempNode);
		return true;
	}
	bool CHandsNumberAndWeight::IsMakeZhaDan(SCombinationResult & tCombinationResult) const
	{
		int iResultNoWithSize = tCombinationResult.vecResultNoWith.size();
		for(int i=0;i<iResultNoWithSize;i++)
		{
			if(  tCombinationResult.vecResultNoWith[i].eCardType == eCardType_SanZhang )
			{
				if( tCombinationResult.vecResultNoWith[i].vecMainCard.size() ==0 )
				{
					continue;
				}
				//// 修改结果
				unsigned char ucTmp = tCombinationResult.vecResultNoWith[i].vecMainCard.front();
				tCombinationResult.vecResultNoWith[i].eCardType = eCardType_ZhaDan;
				AddParamByValue(tCombinationResult.vecResultNoWith[i].vecMainCard,ucTmp);
				AddParamByValue(tCombinationResult.vecResultNoWith[i].vecLaiZiCard,ucTmp);
				return true;
			}
		}
		return false;
	}
	bool CHandsNumberAndWeight::IsMakeSanZhang(SCombinationResult & tCombinationResult) const
	{
		int iResultNoWithSize = tCombinationResult.vecResultNoWith.size();
		for(int i=0;i<iResultNoWithSize;i++)
		{
			if(  tCombinationResult.vecResultNoWith[i].eCardType == eCardType_DuiZi )
			{
				if( tCombinationResult.vecResultNoWith[i].vecMainCard.size() ==0 )
				{
					continue;
				}
				//// 修改结果
				unsigned char ucTmp = tCombinationResult.vecResultNoWith[i].vecMainCard.front();
				tCombinationResult.vecResultNoWith[i].eCardType = eCardType_SanZhang;
				AddParamByValue(tCombinationResult.vecResultNoWith[i].vecMainCard,ucTmp);
				AddParamByValue(tCombinationResult.vecResultNoWith[i].vecLaiZiCard,ucTmp);
				return true;
			}
		}
		return false;
	}
	bool CHandsNumberAndWeight::IsMakeDuiZi(SCombinationResult & tCombinationResult) const
	{
		int iResultNoWithSize = tCombinationResult.vecResultNoWith.size();
		for(int i=iResultNoWithSize-1;i>=0;i--)				   //// 从最大的单张开始查询
		{
			if(  tCombinationResult.vecResultNoWith[i].eCardType == eCardType_DanZhang )
			{
				if( tCombinationResult.vecResultNoWith[i].vecMainCard.size() ==0 )
				{
					continue;
				}
				//// 癞子不能为王
				if( tCombinationResult.vecResultNoWith[i].vecMainCard.front() == 14 || tCombinationResult.vecResultNoWith[i].vecMainCard.front() == 15 )
				{
					continue;
				}
				//// 修改结果
				unsigned char ucTmp = tCombinationResult.vecResultNoWith[i].vecMainCard.front();
				tCombinationResult.vecResultNoWith[i].eCardType = eCardType_DuiZi;
				AddParamByValue(tCombinationResult.vecResultNoWith[i].vecMainCard,ucTmp);
				AddParamByValue(tCombinationResult.vecResultNoWith[i].vecLaiZiCard,ucTmp);
				return true;
			}
		}
		return false;
	}

	bool CHandsNumberAndWeight::IsMakeZhaDanLonger(SCombinationResult & tCombinationResult) const
	{
		int iResultNoWithSize = tCombinationResult.vecResultNoWith.size();
		for(int i=0;i<iResultNoWithSize;i++)
		{
			if(  tCombinationResult.vecResultNoWith[i].eCardType == eCardType_ZhaDan )
			{
				if( tCombinationResult.vecResultNoWith[i].vecMainCard.size() < 4 )
				{
					continue;
				}
				//// 修改结果
				unsigned char ucTmp = tCombinationResult.vecResultNoWith[i].vecMainCard.front();
				tCombinationResult.vecResultNoWith[i].eCardType = eCardType_ZhaDan;
				AddParamByValue(tCombinationResult.vecResultNoWith[i].vecMainCard,ucTmp);
				AddParamByValue(tCombinationResult.vecResultNoWith[i].vecLaiZiCard,ucTmp);
				return true;
			}
		}
		return false;
	}
	//// 主动出牌算法 ///////////////////////////////////////////////////////////////
	void CHandsNumberAndWeight::GetCombinationList()
	{
		//如果是检查，并且已经检查通过，不再检查
		if(m_iCheckType == SuccessCheck)	return;

		for(int i=1;i<16;i++)
		{
			//炸弹不拆开
			if(4 == m_byCardData[i])
			{
				m_byCombinationData[i] =  m_byCardData[i];
				m_byCardData[i] = 0;
			}
		}

		/// 第一步 找牌 获取出和剩下牌不能组成关联牌 ////////////////////////////////////////////
		for(int i=1;i<16;i++)
		{
			if(m_byCardData[i] == 0)
			{
				 continue;
			}
			///   斗地主: 单张 对子 三张 四张  单张连就叫顺子 对子连就叫双顺 三张连就叫飞机
			///   2张 需要判断 是否组成顺子
			///   3张 需要判断 是否组成连对 顺子        如果拆了能组成连对和顺子 这种情况已经处理 如果拆了只能组成连对，就会剩下一张所以感觉也不是最好
			///   4张 需要判断 是否组成飞机 连对 顺子   把四张拆了组成飞机 感觉没必要
			////  是否组成顺子
			bool ret = IsShunZi(m_byCardData,i,0);	 
			if(ret == false)			 ///   如果不能组成顺子 则添加进无关联牌
			{
				m_byCombinationData[i] =  m_byCardData[i];
				m_byCardData[i] = 0;
			}
		}
		//// 如果剩下的牌都不能组成顺子 则拆牌结束   ////////////////////////////////////////////
		if( GetCardCount() == 0 )
		{
			if( InvalidCheck == m_iCheckType)
			{
				/// 第三步 选最优结果
				GetMaxCombination();
			}
			else
			{
				//检查
				DoCheck();
			}
			return;
		}
		/// 第二步 计算每一种牌的手数和权值问题（拆牌中的难点） ////////////////////////////////////////////////////////////
		/// 对子 三张 四张 组成顺子 可拆 可不拆 
		int iMinChaiPai = 16;
		int iMaxChaiPai = 0;
		bool bIsChaiPai = false;

		for(int i=1;i<14;i++)		//// 组成顺子 只会是 3~KA
		{
			if(m_byCardData[i] == 0 || i == 2)
			{//2跳过
				continue;
			}
			if( m_byCardData[i] > 1 )
			{
				///// 不拆牌  /////////////////////////////////////////////////
				///// 保存数据 
				unsigned char tCardData[16];						////  初始数据
				unsigned char tCombinationData[16];				////  拆牌后数据
				std::vector<SShunZiNode> tVetorShunZi;  ////  保存顺子(顺子只保存开始牌 个数就是顺子长度)
				memcpy(tCardData,m_byCardData,sizeof(m_byCardData));
				memcpy(tCombinationData,m_byCombinationData,sizeof(m_byCombinationData));
				tVetorShunZi.insert(tVetorShunZi.end(),m_vetorShunZi.begin(),m_vetorShunZi.end());

				//// 添加结果 移除手牌
				m_byCombinationData[i] =  m_byCardData[i];
				m_byCardData[i] = 0;

				/// 剩下的牌 继续组合 计算不拆手牌后的组合
				GetCombinationList();

				//// 还原数据 计算拆手牌后组合 /////////////////////////////////////////////////
				memcpy(m_byCardData,tCardData,sizeof(m_byCardData));
				memcpy(m_byCombinationData,tCombinationData,sizeof(m_byCombinationData));

				m_vetorShunZi.clear();
				m_vetorShunZi.insert(m_vetorShunZi.end(),tVetorShunZi.begin(),tVetorShunZi.end());
				///// 拆牌 就必须要组成顺子 /////////////////////////////////////////////////
				if( i ==1 )           /// 在这里A 就是14 特殊处理
				{
					iMaxChaiPai = 14;
				}
				if( i < iMinChaiPai)
				{
					iMinChaiPai = i;
				}
				if(i > iMaxChaiPai)
				{
					iMaxChaiPai = i;
				}
				bIsChaiPai = true;
			}
		}
		/// 顺子最长12 最短为5; 3~KA 3456789 10 JQKA 
		/// 获取出顺子 而且必须用完 所有能拆的牌 (可以拆牌而不拆 上面已经处理了)
		for(int iSunZiLen = 5;iSunZiLen <= 12;iSunZiLen++)
		{
			for(int i=3;i<=10;i++)  /// 顺子开始最小是3 最大是10
			{
				if( (i+iSunZiLen > 15))    /// 如果是10了 不可能还会有长度 6 的顺子
				{
					break;
				}
				if( bIsChaiPai && ( i > iMinChaiPai || i+iSunZiLen < iMaxChaiPai) )  //  获取出来的顺子必须包含 最小值~最大值 才合理
				{
					continue;
				}
				if( m_byCardData[i] > 0 )
				{
					bool ret =  IsShunZi(m_byCardData,i,1,iSunZiLen);
					if(ret == true )
					{
						/// 保存数据 //////////////////////////////////
						unsigned char tCardData[16];						////  初始数据
						unsigned char tCombinationData[16];				////  拆牌后数据
						std::vector<SShunZiNode> tVetorShunZi;  ////  保存顺子(顺子只保存开始牌 个数就是顺子长度)
						memcpy(tCardData,m_byCardData,sizeof(m_byCardData));
						memcpy(tCombinationData,m_byCombinationData,sizeof(m_byCombinationData));
						tVetorShunZi.insert(tVetorShunZi.end(),m_vetorShunZi.begin(),m_vetorShunZi.end());
						/// 移除手牌  ///////////////////////////////
						for(int j=0;j<iSunZiLen;j++)
						{
							if( j+i > 14 )
							{
								break;
							}
							if( j+i == 14 )
							{
								m_byCardData[1] -=1;
							}
							else
							{
								m_byCardData[j+i] -=1;
							}
						}
						/// 添加结果 //////////////////////////////
						SShunZiNode tShunZiNode;
						tShunZiNode.byValue = i;
						tShunZiNode.iLen = iSunZiLen;
						m_vetorShunZi.push_back(tShunZiNode);
						/// 剩下的牌 继续组合
						GetCombinationList();
						//// 还原数据 ///////////////////////////////////////////////////////////////////
						memcpy(m_byCardData,tCardData,sizeof(m_byCardData));
						memcpy(m_byCombinationData,tCombinationData,sizeof(m_byCombinationData));
						m_vetorShunZi.clear();
						m_vetorShunZi.insert(m_vetorShunZi.end(),tVetorShunZi.begin(),tVetorShunZi.end());
					}
				}
			}
		}
		return;
	}
	void CHandsNumberAndWeight::GetMaxCombination()
	{
		 //// 权值定义
		 /* 单张							  小于等于10为1 大于等于10为2 
			对子                              小于等于10为1 大于等于10为2 
			三带                              3
			连牌                              4 (每多一张牌权值+1)
			连对                              5（每多一对牌，权值+2）
			飞机                              6（每对以飞机，权值在基础上+3）
			炸弹                              7（包括对王在内） 
		 */
		int tCurValue = 0;           /// 当前权值
		int tCurHandsNumber =0;      /// 当前手数
		std::vector<SCombinationNode>  tVecCombinationResult;  ///当前组牌结果
		std::vector<SCombinationNode>  tVecResultNoWith; ///当前组牌 不带牌结果
		tVecCombinationResult.clear();
		tVecResultNoWith.clear();
		////  计算王炸 手数和权值 //////////////////////////////////////////////
		if( m_byCombinationData[14] > 0 && m_byCombinationData[15] > 0 )
		{
			tCurHandsNumber++;
			tCurValue += 7;
			/// 清空已经计算的结果
			m_byCombinationData[14] = 0;
			m_byCombinationData[15] = 0;
			/// 保存结果
			SCombinationNode tCombinationNode;
			AddZhaDanToNode(14,0,0,eAddCardType_No,tCombinationNode);
			tVecCombinationResult.push_back(tCombinationNode);
			tVecResultNoWith.push_back(tCombinationNode);
		}
		///// 计算顺子的 手数和权值 ////////////////////////////////////////////
		for(auto iter = m_vetorShunZi.begin();iter!= m_vetorShunZi.end();iter++)
		{
			tCurHandsNumber++;
			tCurValue += (*iter).iLen - 1;
			/// 保存结果
			SCombinationNode tCombinationNode;
			AddShunZiToNode((*iter).byValue,(*iter).iLen,tCombinationNode);
			tVecCombinationResult.push_back(tCombinationNode);
			tVecResultNoWith.push_back(tCombinationNode);
		}
		/// 计算单独的牌型 权值 //////////////////////////////////////////////////////
		/// 计算单独的牌型 不可以带牌和连牌 手数和权数  
		GetCombinationResultNoWith(m_byCombinationData,tVecResultNoWith);
		/// 计算单独的牌型 可以带牌和连牌 手数和权数
		GetCombinationResult(m_byCombinationData,tVecCombinationResult,tCurValue,tCurHandsNumber);
		/// 如果有最优结果 则保存最优组合

		BYTE byTemp[8] = {0};
		for(int i = 0; i < (int)tVecCombinationResult.size(); ++i)
		{
			byTemp[tVecCombinationResult[i].eCardType]++;
		}
		int bOK = 0;
		const int iSwitchMap[8] = {7, 6, 2, 4, 5, 3, 1, 0};
		for(int i = 0; i < 8; ++i)
		{
			if(byTemp[iSwitchMap[i]] > m_byTypeCount[iSwitchMap[i]])
			{
				bOK = 1;
				break;
			}

			if(byTemp[iSwitchMap[i]] < m_byTypeCount[iSwitchMap[i]])
			{
				bOK = -1;
				break;
			}
		}
		//大于直接ok或者等于但是权值可以也ok
		if( bOK == 1 || ( bOK == 0 && ( tCurHandsNumber < m_iMinHandsNumber || (tCurHandsNumber == m_iMinHandsNumber && tCurValue > m_iMaxValue) ) ) )
		{
			memcpy(m_byTypeCount, byTemp, sizeof(m_byTypeCount));
			m_iMaxValue = tCurValue;
			m_iMinHandsNumber = tCurHandsNumber;

			m_vecResultNoWith.clear();
			m_vecResultNoWith.insert(m_vecResultNoWith.end(),tVecResultNoWith.begin(),tVecResultNoWith.end());
			m_vecCombinationResult.clear();
			m_vecCombinationResult.insert(m_vecCombinationResult.end(),tVecCombinationResult.begin(),tVecCombinationResult.end());
		}
		/*
		if(m_bFirstOut)
		{
			//1.首出只出包含最小牌的组合；2.根据类型优先级出，类型优先级相同则按照权值和手数
			for(int j = 0; j < (int)tVecCombinationResult.size();j++)
			{
				if(IncludeTheValue(tVecCombinationResult[j], m_byMinCard) )
				{
					bool bFindMax = false;
					if(1 == CompareNoByType(tVecCombinationResult[j], m_tMaxNode))
					{
						bFindMax = true;
					}
					else if ( 0 == CompareNoByType(tVecCombinationResult[j], m_tMaxNode) )
					{
						if( tCurHandsNumber < m_iMinHandsNumber || (tCurHandsNumber == m_iMinHandsNumber && tCurValue > m_iMaxValue) )
						{
							bFindMax = true;
						}
					}
					if( bFindMax )
					{
						m_iMaxValue = tCurValue;
						m_iMinHandsNumber = tCurHandsNumber;
						m_tMaxNode = tVecCombinationResult[j];
						m_vecMinCArdResult.clear();
						m_vecMinCArdResult.insert(m_vecMinCArdResult.end(), tVecCombinationResult.begin(), tVecCombinationResult.end());
						break;
					}
				}
			}
			return;
		}
		*/
	}

#define CMP(a,b) ( (a) > (b) ? 1 : ( (a) == (b) ? 0 : -1 ) )

	//按类型优先级比较两个节点大小
	int CHandsNumberAndWeight::CompareNoByType(const SCombinationNode& tA, const SCombinationNode& tB)
	{
		//（牌型组成优先级：飞机＞顺子＞连对＞三带一（对）＞对子＞单牌）
		if(tA.eCardType != tB.eCardType)
		{
			const int arrCmpMap[8] = {0, 2, 6, 3, 5, 4, 7, 1};
			return CMP(arrCmpMap[tA.eCardType], arrCmpMap[tB.eCardType]);
		}
		else
		{
			return CMP((int)tA.vecMainCard.size(), (int)tB.vecMainCard.size());
		}
	}

	//是否包含某张牌
	bool CHandsNumberAndWeight::IncludeTheValue(SCombinationNode &tNode, unsigned char byValue)
	{
		return find(tNode.vecMainCard.begin(), tNode.vecMainCard.end(), byValue) != tNode.vecMainCard.end()
			|| find(tNode.vecSubCard.begin(), tNode.vecSubCard.end(), byValue) != tNode.vecSubCard.end();
	}
	//添加一个结果
	void CHandsNumberAndWeight::AddAResult(SCombinationNode &tNode)
	{
		bool bFind = false;
		for(int i = 0; i < (int)m_vecMinCArdResult.size();++i)
		{
			if(NodeEquale(m_vecMinCArdResult[i], tNode))
			{
				bFind = true;
				break;
			}
		}
		if(!bFind)
		{
			m_vecMinCArdResult.push_back(tNode);
		}
	}
	//节点是否相同
	bool CHandsNumberAndWeight::NodeEquale(const SCombinationNode &tNodeA, const SCombinationNode &tNodeB) const
	{
		bool bRet = true;
		if(tNodeA.eAddCardType != tNodeB.eAddCardType) bRet = false;
		if(tNodeA.eCardType != tNodeB.eCardType) bRet = false;
		if((int)tNodeA.vecMainCard.size() != (int)tNodeB.vecMainCard.size()) bRet = false;
		if((int)tNodeA.vecSubCard.size() != (int)tNodeB.vecSubCard.size()) bRet = false;
		if((int)tNodeA.vecLaiZiCard.size() != (int)tNodeB.vecLaiZiCard.size()) bRet = false;

		if(!bRet)
			return bRet;
		for(int i = 0; i < (int)tNodeA.vecMainCard.size(); i++)
		{
			if(tNodeA.vecMainCard[i] != tNodeB.vecMainCard[i])
			{
				bRet = false;
				break;
			}
		}
		for(int i = 0; i < (int)tNodeA.vecSubCard.size(); i++)
		{
			if(tNodeA.vecSubCard[i] != tNodeB.vecSubCard[i])
			{
				bRet = false;
				break;
			}
		}
		for(int i = 0; i < (int)tNodeA.vecLaiZiCard.size(); i++)
		{
			if(tNodeA.vecLaiZiCard[i] != tNodeB.vecLaiZiCard[i])
			{
				bRet = false;
				break;
			}
		}
		return bRet;
	}


	void CHandsNumberAndWeight::GetCombinationResultNoWith(const CARDATA pSrcCard,std::vector<SCombinationNode> & tVecCombinationResult,int & tCurValue,int & tCurHandsNumber)const
	{
		 /* 单张							  小于等于10为1 大于等于10为2 
			对子                              小于等于10为1 大于等于10为2 
			三带                              3
			连牌                              4 (每多一张牌权值+1)
			连对                              5（每多一对牌，权值+2）
			飞机                              6（每对以飞机，权值在基础上+3）
			炸弹                              7（包括对王在内） 
		 */
		SVBYTE tVecDanZhang;  /// 单张  必须是有序的 下面算法均是在有序基础上实现
		SVBYTE tVecDuiZi;	 /// 对子  必须是有序的 下面算法均是在有序基础上实现
		SVBYTE tVecSanZhang;  /// 三张  必须是有序的 下面算法均是在有序基础上实现
		SVBYTE tVecSiZhang;   /// 四张  必须是有序的 下面算法均是在有序基础上实现
		std::vector<SZhaDanLongNode> tVecZhaDanLong;     /// 四张以上  必须是有序的 下面算法均是在有序基础上实现
		ClassificationData(pSrcCard,tVecDanZhang,tVecDuiZi,tVecSanZhang,tVecSiZhang,tVecZhaDanLong);
		for(auto iter = tVecDanZhang.begin();iter!= tVecDanZhang.end();iter++ )
		{
			tCurHandsNumber++;
			if( *iter > 10)
			{
				tCurValue+=2;
			}
			else
			{
				tCurValue+=1;
			}
			/// 赋值组牌 不可以带牌和不可连牌 的结果
			SCombinationNode tCombinationNode;
			AddDanZhangToNode(*iter,tCombinationNode);
			tVecCombinationResult.push_back(tCombinationNode);
		}
		for(auto iter = tVecDuiZi.begin();iter!= tVecDuiZi.end();iter++ )
		{
			tCurHandsNumber++;
			if( *iter > 10)
			{
				tCurValue+=2;
			}
			else
			{
				tCurValue+=1;
			}
			/// 赋值组牌 不可以带牌和不可连牌 的结果
			SCombinationNode tCombinationNode;
			AddDuiZiToNode(*iter,tCombinationNode);
			tVecCombinationResult.push_back(tCombinationNode);
		}
		for(auto iter = tVecSanZhang.begin();iter!= tVecSanZhang.end();iter++ )
		{
			tCurHandsNumber++;
			tCurValue+=3;
			/// 赋值组牌 不可以带牌和不可连牌 的结果
			SCombinationNode tCombinationNode;
			AddSanZhangToNode(*iter,0,eAddCardType_No,tCombinationNode);
			tVecCombinationResult.push_back(tCombinationNode);
		}
		for(auto iter = tVecSiZhang.begin();iter!= tVecSiZhang.end();iter++ )
		{
			tCurHandsNumber++;
			tCurValue+=7;
			/// 赋值组牌 不可以带牌和不可连牌 的结果
			SCombinationNode tCombinationNode;
			AddZhaDanToNode(*iter,0,0,eAddCardType_No,tCombinationNode);
			tVecCombinationResult.push_back(tCombinationNode);
		}
		for(auto iter = tVecZhaDanLong.begin();iter!= tVecZhaDanLong.end();iter++ )
		{
			tCurHandsNumber++;
			tCurValue+=7;
			/// 赋值组牌 不可以带牌和不可连牌 的结果
			SCombinationNode tCombinationNode;
			AddZhaDanLongToNode(iter->byValue,iter->iLen,tCombinationNode);
			tVecCombinationResult.push_back(tCombinationNode);
		}
	}
	void CHandsNumberAndWeight::GetCombinationResultNoWith(const CARDATA pSrcCard,std::vector<SCombinationNode> & tVecCombinationResult)const
	{
		int  tCurValue = 0;
		int  tCurHandsNumber=0;
		GetCombinationResultNoWith(pSrcCard,tVecCombinationResult,tCurValue,tCurHandsNumber);
	}
	void CHandsNumberAndWeight::GetCombinationResult(const CARDATA pSrcCard,std::vector<SCombinationNode> & tVecCombinationResult,int & tCurValue,int & tCurHandsNumber)const
	{
		 /* 单张							  小于等于10为1 大于等于10为2 
			对子                              小于等于10为1 大于等于10为2 
			三带                              3
			连牌                              4 (每多一张牌权值+1)
			连对                              5（每多一对牌，权值+2）
			飞机                              6（每对以飞机，权值在基础上+3）
			炸弹                              7（包括对王在内） 
		 */
		SVBYTE tVecDanZhang;		/// 单张  必须是有序的 下面算法均是在有序基础上实现
		SVBYTE tVecDuiZi;			/// 对子  必须是有序的 下面算法均是在有序基础上实现
		SVBYTE tVecSanZhang;		/// 三张  必须是有序的 下面算法均是在有序基础上实现
		SVBYTE tVecSiZhang;			/// 四张  必须是有序的 下面算法均是在有序基础上实现
		std::vector<SZhaDanLongNode> tVecZhaDanLong;     /// 四张以上  必须是有序的 下面算法均是在有序基础上实现
		ClassificationData(pSrcCard,tVecDanZhang,tVecDuiZi,tVecSanZhang,tVecSiZhang,tVecZhaDanLong);
		/// 连队获取
		if( (tVecSiZhang.size()*2 + tVecSanZhang.size()) < (tVecDuiZi.size() + tVecDanZhang.size()) )  /// 如果可以带的牌数 小于 需要带的牌数  看看是否有连队
		{
			GetLianDui(tVecDuiZi,tVecCombinationResult,tCurValue,tCurHandsNumber);
		}
		/// 飞机获取 （这种情况不予考虑了：3条涉及到带牌 有时候组成了飞机确没有符合的牌带 反而增加了手数）
		std::vector<SCombinationNode>  tVecCombinationFeiJi;					/// 保存能组成飞机的结果 飞机带牌后才存入最终结果
		GetFeiJi(tVecSanZhang,tVecCombinationFeiJi,tCurValue,tCurHandsNumber);	
		//// 带牌 处理 
		///  处理不能带的牌 A 2 鬼 
		///  单张 处理不带 A 2 鬼
		for(auto iter = tVecDanZhang.begin();iter != tVecDanZhang.end();)
		{
			if( (*iter) == 1 || (*iter) == 2 || (*iter) == 14 || (*iter) == 15  )
			{
				SCombinationNode tCombinationNode;
				AddDanZhangToNode(*iter,tCombinationNode);
				tVecCombinationResult.push_back(tCombinationNode);
				tCurValue += 2;
				tCurHandsNumber++;
				iter = tVecDanZhang.erase(iter);
			}
			else
			{
				iter++;
			}
		}
		///   对张 处理不带 A 2
		for(auto iter = tVecDuiZi.begin();iter != tVecDuiZi.end();)
		{
			if( (*iter) == 1 || (*iter) == 2  )
			{
				SCombinationNode tCombinationNode;
				AddDuiZiToNode(*iter,tCombinationNode);
				tVecCombinationResult.push_back(tCombinationNode);
				tCurValue += 2;
				tCurHandsNumber++;
				iter = tVecDuiZi.erase(iter);
			}
			else
			{
				iter++;
			}
		}
		///  飞机带牌
		for(int i = 0; i < tVecCombinationFeiJi.size();i++)
		{
			int iFeiJiLen = tVecCombinationFeiJi[i].vecMainCard.size()/3;
			GetSubCard(eAddCardMainType_FeiJi,tVecDuiZi,tVecDanZhang,iFeiJiLen,tVecCombinationFeiJi[i].vecSubCard,tVecCombinationFeiJi[i].eAddCardType);
		}
		///  保存飞机带牌后结果
		tVecCombinationResult.insert(tVecCombinationResult.end(),tVecCombinationFeiJi.begin(),tVecCombinationFeiJi.end());
		///  三张 带牌
		for(auto iter = tVecSanZhang.begin();iter != tVecSanZhang.end();iter++)
		{
			SCombinationNode tCombinationNode;
			AddSanZhangToNode(*iter,0,eAddCardType_No,tCombinationNode);
			GetSubCard(eAddCardMainType_SanZhang,tVecDuiZi,tVecDanZhang,1,tCombinationNode.vecSubCard,tCombinationNode.eAddCardType);
			/// 保存结果
			tVecCombinationResult.push_back(tCombinationNode);

			tCurValue += 3;
			tCurHandsNumber++;
		}
		/// 炸弹 带牌
		for(auto iter = tVecSiZhang.begin();iter != tVecSiZhang.end();iter++)
		{
			SCombinationNode tCombinationNode;
			AddZhaDanToNode(*iter,0,0,eAddCardType_No,tCombinationNode);
			GetSubCard(eAddCardMainType_SiZhan,tVecDuiZi,tVecDanZhang,2,tCombinationNode.vecSubCard,tCombinationNode.eAddCardType);
			/// 保存结果
			tVecCombinationResult.push_back(tCombinationNode);

			tCurValue += 7;
			tCurHandsNumber++;
		}
		/// 长炸弹 
		for(auto iter = tVecZhaDanLong.begin();iter != tVecZhaDanLong.end();iter++)
		{
			SCombinationNode tCombinationNode;
			AddZhaDanLongToNode(iter->byValue,iter->iLen,tCombinationNode);
			/// 保存结果
			tVecCombinationResult.push_back(tCombinationNode);

			tCurValue += 7;
			tCurHandsNumber++;
		}
		/// 两张 剩余
		for(auto iter = tVecDuiZi.begin();iter != tVecDuiZi.end();iter++)
		{
			SCombinationNode tCombinationNode;
			AddDuiZiToNode(*iter,tCombinationNode);
			if( (*iter) > 10 || (*iter) == 1 || (*iter) == 2 )
			{
				tCurValue += 2;
			}
			else
			{
				tCurValue += 1;
			}
			tVecCombinationResult.push_back(tCombinationNode);
			tCurHandsNumber++;
		}
		/// 单张 剩余
		for(auto iter = tVecDanZhang.begin();iter != tVecDanZhang.end();iter++)
		{
			SCombinationNode tCombinationNode;
			AddDanZhangToNode(*iter,tCombinationNode);
			if( (*iter) > 10 || (*iter) == 1 || (*iter) == 2 )
			{
				tCurValue += 2;
			}
			else
			{
				tCurValue += 1;
			}
			tVecCombinationResult.push_back(tCombinationNode);
			tCurHandsNumber++;
		}
	}
	/// 跟牌私有方法 ////////////////////////////////////////////////////////////////////
	void CHandsNumberAndWeight::GetDanZhangListByLastCard(const CARDATA pSrcCard,unsigned char tLastCard,std::vector<SCombinationNode> & vecFollowCardList)const
	{
		if( nullptr == pSrcCard )
		{
		    return;
		}
		for(int i=1;i<16;i++)
		{
		   /// 大小王特殊处理
		   if(pSrcCard[i] >= 1 && ( pSrcCard[i] == 14 || pSrcCard[i] == 15 ) )
		   {
				if( pSrcCard[14] > 0 && pSrcCard[15] > 0 && m_sSysConfig.bIsWBomb_Separate == false )  //// 大小王不能拆
				{
				   continue;
				}
		   }
		   bool ret = SpecialComparisonFun(i,tLastCard);
		   if( pSrcCard[i] > 0 && ret )
		   {
			  SCombinationNode tCombinationNode;
			  AddDanZhangToNode(i,tCombinationNode);
		      vecFollowCardList.push_back(tCombinationNode);
		   }
		}
	}
	void CHandsNumberAndWeight::GetDuiZiListByLastCard(const CARDATA pSrcCard,unsigned char tLastCard,std::vector<SCombinationNode> & vecFollowCardList) const
	{
		if( nullptr == pSrcCard  )
		{
			return;
		}
		//// 获取出所有能跟的牌
		for(int i=1;i<16;i++)
		{
			bool ret = SpecialComparisonFun(i,tLastCard);
			if( pSrcCard[i] >= 2 && ret )
			{
				SCombinationNode tCombinationNode;
				AddDuiZiToNode(i,tCombinationNode);
				vecFollowCardList.push_back(tCombinationNode);
			}
		}
	}
	void CHandsNumberAndWeight::GetSanZhangListByLastCard(const CARDATA pSrcCard,unsigned char tLastCard,EAddCardType tAddCardType,std::vector<SCombinationNode> & vecFollowCardList) const
	{
		if( nullptr == pSrcCard || tAddCardType == eAddCardType_Unknown )
		{
			return;
		}
		for(int i=0;i<16;i++)
		{
			bool ret = SpecialComparisonFun(i,tLastCard);
		    if( pSrcCard[i] >= 3 && ret)
			{
				unsigned char uSubCard=0;
				/// 获取出最坏牌
				CARDATA pSubCard;
				memcpy(pSubCard,pSrcCard,sizeof(pSubCard));
				pSubCard[i]=0;   /// 去除本身
			    if( tAddCardType == eAddCardType_No )
				{
					
				}
				else if(tAddCardType == eAddCardType_DanZhang)
				{  
					uSubCard = GetSubCard(pSubCard,1);
					if( uSubCard == 0 )
					{
						return;
					}
				}
				else if(tAddCardType == eAddCardType_DuiZi)
				{
					uSubCard = GetSubCard(pSubCard,2);
					if( uSubCard == 0 )
					{
						return;
					}
				}
				SCombinationNode tCombinationNode;
				AddSanZhangToNode(i,uSubCard,tAddCardType,tCombinationNode);
				vecFollowCardList.push_back(tCombinationNode);
			}
		}
	}
	void CHandsNumberAndWeight::GetZhaDanWListByLastCard(const CARDATA pSrcCard,unsigned char tLastCard,EAddCardType tAddCardType,std::vector<SCombinationNode> & vecFollowCardList) const
	{
		if( nullptr == pSrcCard || tAddCardType == eAddCardType_Unknown )
		{
			return;
		}
		for(int i=0;i<16;i++)
		{
			bool ret = SpecialComparisonFun(i,tLastCard);
			if( pSrcCard[i] >= 4 && ret)
			{
				unsigned char uSubCard1 =0;
				unsigned char uSubCard2 =0;
				CARDATA pSubCard;
				memcpy(pSubCard,pSrcCard,sizeof(pSubCard));
				pSubCard[i]=0;  /// 去除本身
				if( tAddCardType == eAddCardType_No )
				{
					  return;
				}
				else if(tAddCardType == eAddCardType_DanZhang)
				{  
					uSubCard1 = GetSubCard(pSubCard,1);
					if( uSubCard1 == 0)
					{
					   return;
					}
					pSubCard[uSubCard1]=0;
					uSubCard2 = GetSubCard(pSubCard,1);
					if( uSubCard2 == 0 )
					{
						return;
					}
				}
				else if(tAddCardType == eAddCardType_DuiZi)
				{
					/// 获取出最坏牌
					uSubCard1 = GetSubCard(pSubCard,2);
					if( uSubCard1 == 0 )
					{
						return;
					}
					pSubCard[uSubCard1]=0;
					uSubCard2 = GetSubCard(pSubCard,2);
					if( uSubCard2 == 0 )
					{
						return;
					}
				}
				SCombinationNode tCombinationNode;
				AddZhaDanToNode(i,uSubCard1,uSubCard2,tAddCardType,tCombinationNode);
				vecFollowCardList.push_back(tCombinationNode);
			}
		}
	}
	void CHandsNumberAndWeight::GetShunZiListByLastCard(const CARDATA pSrcCard,const unsigned char tStart,int iSunZiLen,std::vector<SCombinationNode> & vecFollowCardList) const
	{
		if( iSunZiLen < 5 || nullptr == pSrcCard )
		{
			return;
		}
		for(int i=3;i <= 15-iSunZiLen;i++)
		{
			if( pSrcCard[i] >= 1 && i > tStart )
			{
				bool ret =  IsShunZi(pSrcCard,i,1,iSunZiLen);
				if(ret == true )
				{
					SCombinationNode tCombinationNode;
					AddShunZiToNode(i,iSunZiLen,tCombinationNode);
					vecFollowCardList.push_back(tCombinationNode);
				}
			}
		}
	}
	void CHandsNumberAndWeight::GetLianDuiListByLastCard(const CARDATA pSrcCard,const unsigned char tStart,int iDuiZiLen,std::vector<SCombinationNode> & vecFollowCardList)const
	{
		if( nullptr == pSrcCard)
		{
		   return;
		}
		SVBYTE tTmpVecDuiZi;
		std::vector<SCombinationNode> tTmpVecCombinationResult;
		int  tCurValue = 0;
		int  tCurHandsNumber =0;
		for(int i=1;i<16;i++)
		{
		    if( pSrcCard[i] >= 2 )
			{
			    tTmpVecDuiZi.push_back(i);
			}
		}
		GetLianDui(tTmpVecDuiZi,tTmpVecCombinationResult,tCurValue,tCurHandsNumber);
		int iLastLianDuiLen = iDuiZiLen;
		for(auto iter = tTmpVecCombinationResult.begin();iter != tTmpVecCombinationResult.end(); iter++ )
		{
			int iLianDuiLen = (*iter).vecMainCard.size();
			for(int i=0; iLianDuiLen >= iLastLianDuiLen && i < (*iter).vecMainCard.size(); )
			{
				 bool ret = SpecialComparisonFun((*iter).vecMainCard[i],tStart);
			     if( ret )
				 {
					 SCombinationNode tCombinationNode;
					 AddLianDuiToNode((*iter).vecMainCard[i],iLastLianDuiLen/2,tCombinationNode);
					 vecFollowCardList.push_back(tCombinationNode);
				 }
				 i+=2;
				 iLianDuiLen -= 2;
			}
		}
	}
	void CHandsNumberAndWeight::GetFeiJiListByLastCard( const CARDATA pSrcCard,const unsigned char tStart,int iFeiJiLen,EAddCardType tAddCardType,std::vector<SCombinationNode> & vecFollowCardList) const
	{
		if( nullptr == pSrcCard || tAddCardType == eAddCardType_Unknown )
		{
			return;
		}
		SVBYTE tTmpVecSanZhang;
		std::vector<SCombinationNode> tTmpVecCombinationResult;
		int  tCurValue = 0;
		int  tCurHandsNumber =0;
		for(int i=1;i<16;i++)
		{
			if( pSrcCard[i] >= 3 )
			{
				tTmpVecSanZhang.push_back(i);
			}
		}
		GetFeiJi(tTmpVecSanZhang,tTmpVecCombinationResult,tCurValue,tCurHandsNumber);
		int iLastFeiJiLen = iFeiJiLen;
		for(auto iter = tTmpVecCombinationResult.begin();iter != tTmpVecCombinationResult.end(); iter++ )
		{
			int iFeiJiLen = (*iter).vecMainCard.size();
			for(auto subIter = (*iter).vecMainCard.begin(); iFeiJiLen >= iLastFeiJiLen && subIter != (*iter).vecMainCard.end();subIter++ )
			{
				bool ret = SpecialComparisonFun(*subIter,tStart);
				if( ret )
				{
					/// 获取出最坏牌
					SVBYTE tVecDaiPai;
					CARDATA pSubCard;
					memcpy(pSubCard,pSrcCard,sizeof(pSubCard));
					for(int i = 0;i< iLastFeiJiLen/3;i++)
					{
						int tIndex = (*subIter)+i;
						if( tIndex >= 14 )
						{
							tIndex = 1;
						}
					    pSubCard[tIndex] = 0;
					}
					/// 保存结果
					if( tAddCardType == eAddCardType_No )
					{
						
					}
					else if(tAddCardType == eAddCardType_DanZhang)
					{
						for(int i = 0;i< iLastFeiJiLen/3;i++)
						{
							unsigned char uSubCard = GetSubCard(pSubCard,1);
							if( uSubCard == 0 )
							{
								return;
							}
							tVecDaiPai.push_back(uSubCard);
							pSubCard[uSubCard]=0;
						}
					}
					else if( tAddCardType == eAddCardType_DuiZi )
					{
						for(int i = 0;i< iLastFeiJiLen/3;i++)
						{
							unsigned char uSubCard = GetSubCard(pSubCard,2);
							if( uSubCard == 0 )
							{
								return;
							}
							tVecDaiPai.push_back(uSubCard);
							pSubCard[uSubCard]=0;
						}
					}
					SCombinationNode tCombinationNode;
					AddFeiJiToNode(*subIter,iLastFeiJiLen/3,tVecDaiPai,tAddCardType,tCombinationNode);
					vecFollowCardList.push_back(tCombinationNode);
				}
				else
				{
					subIter++;
					subIter++;
					iFeiJiLen -=3;
				}
			}
		}
	}
	void CHandsNumberAndWeight::GetFollowCardListNoZhaDan(const CARDATA pSrcCard,const SCombinationNode & tLastCard,std::vector<SCombinationNode> & vecFollowCardList)const
	{
		if( nullptr == pSrcCard || tLastCard.vecMainCard.size() == 0)
		{
			return;
		}
		//// 跟牌结果获取
		switch(tLastCard.eCardType)
		{
		case eCardType_DanZhang:
			{
				GetDanZhangListByLastCard(pSrcCard,tLastCard.vecMainCard.front(),vecFollowCardList);
			}
			break;
		case eCardType_DuiZi:
			{
				GetDuiZiListByLastCard(pSrcCard,tLastCard.vecMainCard.front(),vecFollowCardList);
			}
			break;
		case eCardType_ShunZi:
			{
				GetShunZiListByLastCard(pSrcCard,tLastCard.vecMainCard.front(),tLastCard.vecMainCard.size(),vecFollowCardList);
			}
			break;
		case eCardType_LianDui:
			{
				GetLianDuiListByLastCard(pSrcCard,tLastCard.vecMainCard.front(),tLastCard.vecMainCard.size(),vecFollowCardList);
			}
			break;
		case eCardType_SanZhang:
			{
				GetSanZhangListByLastCard(pSrcCard,tLastCard.vecMainCard.front(),tLastCard.eAddCardType,vecFollowCardList);
			}
			break;
		case eCardType_ZhaDan:
			{
				GetZhaDanWListByLastCard(pSrcCard,tLastCard.vecMainCard.front(),tLastCard.eAddCardType,vecFollowCardList);
			}
			break;
		case eCardType_FeiJi:
			{
				GetFeiJiListByLastCard(pSrcCard,tLastCard.vecMainCard.front(),tLastCard.vecMainCard.size(),tLastCard.eAddCardType,vecFollowCardList);
			}
			break;
		default:
			break;
		}
	}
	/// 癞子跟牌算法 ////////////////////////////
	void CHandsNumberAndWeight::GetDuiZiListByLaiZi(CARDATA pSrcCard,unsigned char tLastCard,int iLaiZiNumber,std::vector<SCombinationNode> & vecFollowCardList) const
	{
		if( iLaiZiNumber <= 0 || nullptr == pSrcCard )
		{
			return;
		}
		std::vector<unsigned char> vecLaiZi;
		/// 癞子 替换
		for(int i=1;i <= 13 && iLaiZiNumber >=1;i++)
		{
			if( pSrcCard[i] == 1 )
			{
				pSrcCard[i]++;
				vecLaiZi.push_back(i);
			}
			else
			{
			    pSrcCard[i] = 0;
			}
		}
		//// 获取出结果
		GetDuiZiListByLastCard(pSrcCard,tLastCard,vecFollowCardList);
		//// 结果添加癞子
		for(auto iter = vecFollowCardList.begin();iter != vecFollowCardList.end();iter++)
		{
			if(iter->vecMainCard.empty() )
			{ 
				continue;
			}
			auto iLaiZiTer =  find(vecLaiZi.begin(),vecLaiZi.end(),iter->vecMainCard.front());
			if(iLaiZiTer!=vecLaiZi.end())
			{
				iter->vecLaiZiCard.push_back(*iLaiZiTer);
			}
		}
	}
	void CHandsNumberAndWeight::GetShunZiListByLaiZi(CARDATA pSrcCard,const unsigned char tLastStart,int iLastSunZiLen,int iStartIndex,int iLaiZiNumber,std::vector<SCombinationNode> & vecFollowCardList) const
	{
		if( iLaiZiNumber <= 0 || nullptr == pSrcCard || iStartIndex < 1 || iStartIndex > 13)
		{
			return;
		}
		static std::vector<unsigned char> vecLaiZi;
		for(int i = iStartIndex;i<= 13;i++)
		{
			if( pSrcCard[i] != 0 || i == 2)
			{
				continue;
			}
			/// 算法优化 前后都没有相连牌 则可以过滤这种情况
			if( i!= 1 && i!=13 &&
				pSrcCard[i] == 0 && pSrcCard[i-1] == 0 && pSrcCard[i+1] == 0)
			{
				continue;
			}
			pSrcCard[i]++;
			vecLaiZi.push_back(i);
			/// 添加一个癞子后 获取能跟的牌
			std::vector<SCombinationNode>  tVecFollowCardList;
			GetShunZiListByLastCard(pSrcCard,tLastStart,iLastSunZiLen,tVecFollowCardList);
			/// 保存含有这个癞子的跟牌结果
			for(auto iter = tVecFollowCardList.begin();iter!=tVecFollowCardList.end();iter++ )
			{
				bool ret = IsNodeContainByVector(*iter,vecLaiZi);
				if(ret)					//// 该组合包含当前所有的癞子
				{
					/// 判断该组合是否已经在 vecFollowCardList中
					auto tIter = IsVectorNodeContainByNode(vecFollowCardList,*iter);
					if( tIter == vecFollowCardList.end() )     
					{
						//// 添加新的组合
						iter->vecLaiZiCard.insert((*iter).vecLaiZiCard.end(),vecLaiZi.begin(),vecLaiZi.end());
						vecFollowCardList.push_back(*iter);
					}
					else   /// 选择癞子最少的组合
					{
						if( tIter->vecLaiZiCard.size() > vecLaiZi.size() )
						{
							vecFollowCardList.erase(tIter);
							//// 添加新的组合
							iter->vecLaiZiCard.insert((*iter).vecLaiZiCard.end(),vecLaiZi.begin(),vecLaiZi.end());
							vecFollowCardList.push_back(*iter);
						}
					}
				}
			}
			//// 获取再添加iLaiZiNumber-1 个癞子的结果
			if( pSrcCard[i] == 1 && i > 1) 
			{
				GetShunZiListByLaiZi(pSrcCard,tLastStart,iLastSunZiLen,i-1,iLaiZiNumber-1,vecFollowCardList);
			}
			else
			{
				GetShunZiListByLaiZi(pSrcCard,tLastStart,iLastSunZiLen,i,iLaiZiNumber-1,vecFollowCardList);
			}
			////还原数据
			pSrcCard[i]--;
			vecLaiZi.pop_back();
		}
	}
	void CHandsNumberAndWeight::GetLianDuiListByLaiZi(CARDATA pSrcCard,const unsigned char tLastStart,int iLastDuiZiLen,int iStartIndex,int iLaiZiNumber,std::vector<SCombinationNode> & vecFollowCardList) const
	{
		if( iLaiZiNumber <= 0 || nullptr == pSrcCard || iStartIndex < 1 || iStartIndex > 13)
		{
			return;
		}
		static std::vector<unsigned char> vecLaiZi;
		for(int i = iStartIndex;i<= 13;i++)
		{
			if( pSrcCard[i] >= 2 || i == 2)
			{
				continue;
			}
			/// 算法优化 前后都没有相连牌 则可以过滤这种情况
			if( i!= 1 && i!=13 &&
				pSrcCard[i] == 0 && pSrcCard[i-1] == 0 && pSrcCard[i+1] == 0)
			{
				continue;
			}
			pSrcCard[i]++;
			vecLaiZi.push_back(i);
			/// 添加一个癞子后 获取能跟的牌
			std::vector<SCombinationNode>  tVecFollowCardList;
			GetLianDuiListByLastCard(pSrcCard,tLastStart,iLastDuiZiLen,tVecFollowCardList);
			/// 保存含有这个癞子的跟牌结果
			for(auto iter = tVecFollowCardList.begin();iter!=tVecFollowCardList.end();iter++ )
			{
				bool ret = IsNodeContainByVector(*iter,vecLaiZi);
				if(ret)					//// 该组合包含当前所有的癞子
				{
					/// 判断该组合是否已经在 vecFollowCardList中
					auto tIter = IsVectorNodeContainByNode(vecFollowCardList,*iter);
					if( tIter == vecFollowCardList.end() )     
					{
						//// 添加新的组合
						iter->vecLaiZiCard.insert((*iter).vecLaiZiCard.end(),vecLaiZi.begin(),vecLaiZi.end());
						vecFollowCardList.push_back(*iter);
					}
					else   /// 选择癞子最少的组合
					{
						if( tIter->vecLaiZiCard.size() > vecLaiZi.size() )
						{
							vecFollowCardList.erase(tIter);
							//// 添加新的组合
							iter->vecLaiZiCard.insert((*iter).vecLaiZiCard.end(),vecLaiZi.begin(),vecLaiZi.end());
							vecFollowCardList.push_back(*iter);
						}
					}
				}
			}
			//// 获取再添加iLaiZiNumber-1 个癞子的结果
			if( pSrcCard[i] == 1 && i > 1 )
			{
			   GetLianDuiListByLaiZi(pSrcCard,tLastStart,iLastDuiZiLen,i-1,iLaiZiNumber-1,vecFollowCardList);
			}
			else
			{
			   GetLianDuiListByLaiZi(pSrcCard,tLastStart,iLastDuiZiLen,i,iLaiZiNumber-1,vecFollowCardList);
			}
			////还原数据
			pSrcCard[i]--;
			vecLaiZi.pop_back();
		}
	}
	void CHandsNumberAndWeight::GetAddCardTypeListByLaiZi(CARDATA pSrcCard,const SCombinationNode & tLastCard,int iStartIndex,int iLaiZiNumber,std::vector<SCombinationNode> & vecFollowCardList) const
	{
		if( iLaiZiNumber <= 0 || nullptr == pSrcCard || iStartIndex < 1 || iStartIndex > 13)
		{
			return;
		}
		static std::vector<unsigned char> vecLaiZi;
		for(int i = iStartIndex;i<= 13;i++)
		{
			if(pSrcCard[i] >= 4 )
			{
				continue;
			}
			/// 算法优化 前后都没有相连牌 则可以过滤这种情况
			if( i!= 1 && i!=13 &&
				pSrcCard[i] == 0 && pSrcCard[i-1] == 0 && pSrcCard[i+1] == 0)
			{
				continue;
			}
			pSrcCard[i]++;
			vecLaiZi.push_back(i);
			/// 添加一个癞子后 获取能跟的牌
			std::vector<SCombinationNode>  tVecFollowCardList;
			GetFollowCardListNoZhaDan(pSrcCard,tLastCard,tVecFollowCardList);
			/// 保存含有这个癞子的跟牌结果
			for(auto iter = tVecFollowCardList.begin();iter!=tVecFollowCardList.end();iter++ )
			{
				std::vector<unsigned char> tLaiZiList;
				tLaiZiList.insert(tLaiZiList.end(),vecLaiZi.begin(),vecLaiZi.end());
				
				bool ret = IsNodeContainByVector(*iter,tLaiZiList);
				if(ret)					//// 该组合包含当前所有的癞子
				{
					/// 过滤 主牌全部都是癞子牌
					bool bIsMain =  IsVecContainVec(tLaiZiList,iter->vecMainCard);
					if( bIsMain == true )
					{
						continue;
					}
					///过滤 全部是副牌结果
					bool bIsSub = IsVecContainVec(tLaiZiList,iter->vecSubCard);
					if(bIsSub == true && iter->vecSubCard.size() >= 4 )  /// 过滤带牌四癞结果
					{
						continue;
					}
					if( iter->vecSubCard.size() == 1 && bIsSub == true )
					{
						for( int i=0;i < tLaiZiList.size();i++ )
						{
							if( tLaiZiList[i] == iter->vecSubCard[0] ) 
							{
								tLaiZiList[i] = 0;
								iter->vecSubCard[0] = 0;
								break;
							}
						}
					}
					else if( iter->vecSubCard.size() == 4 && bIsSub == false ) /// 带牌
					{
						SVBYTE tSubList;
						AddParamByValue(tSubList,iter->vecSubCard[0],2);
						bool ret =  IsVecContainVec(tLaiZiList,tSubList);
						if( ret == true)
						{
						   for( int i=0;i < tLaiZiList.size();i++ )
						   {
						       if( tLaiZiList[i] == iter->vecSubCard[0] ) 
							   {
								   tLaiZiList[i] = 0;
								   iter->vecSubCard[0] = 0;
							   }
							   else if( tLaiZiList[i] == iter->vecSubCard[1])
							   {
								   tLaiZiList[i] = 0;
								   iter->vecSubCard[1] = 0;
							   }
						   }
						}
						tSubList.clear();
						AddParamByValue(tSubList,iter->vecSubCard[2],2);
						ret =  IsVecContainVec(tLaiZiList,tSubList);
						if( ret == true)
						{
							for( int i=0;i < tLaiZiList.size();i++ )
							{
								if( tLaiZiList[i] == iter->vecSubCard[2] ) 
								{
									tLaiZiList[i] = 0;
									iter->vecSubCard[2] = 0;
								}
								else if(tLaiZiList[i] == iter->vecSubCard[3])
								{
									tLaiZiList[i] = 0;
									iter->vecSubCard[3] = 0;
								}
							}
						}
					}
					else if( iter->vecSubCard.size() == 2 )
					{
						if( bIsSub == true )
						{
							for( int i=0;i < tLaiZiList.size();i++ )
							{
								if( tLaiZiList[i] == iter->vecSubCard[0] ) 
								{
									tLaiZiList[i] = 0;
									iter->vecSubCard[0] = 0;
								}
								else if(tLaiZiList[i] == iter->vecSubCard[1])
								{
									tLaiZiList[i] = 0;
									iter->vecSubCard[1] = 0;
								}
							}
						}
						else if( iter->vecSubCard[0] != iter->vecSubCard[1] )
						{
							SVBYTE tSubList;
							AddParamByValue(tSubList,iter->vecSubCard[0],1);
							bool ret =  IsVecContainVec(tLaiZiList,tSubList);
							if( ret == true)
							{
								for( int i=0;i < tLaiZiList.size();i++ )
								{
									if( tLaiZiList[i] == iter->vecSubCard[0] ) 
									{
										tLaiZiList[i] = iter->vecSubCard[1];
										break;
									}
								}
								iter->vecSubCard[0] = iter->vecSubCard[1];
							}
							tSubList.clear();
							AddParamByValue(tSubList,iter->vecSubCard[1],1);
							ret =  IsVecContainVec(tLaiZiList,tSubList);
							if( ret == true)
							{
								for( int i=0;i < tLaiZiList.size();i++ )
								{
									if( tLaiZiList[i] == iter->vecSubCard[1] ) 
									{
										tLaiZiList[i] = iter->vecSubCard[0];
										break;
									}
								}
								iter->vecSubCard[1] = iter->vecSubCard[0];
							}    
						}
					}
					/// 判断该组合是否已经在 vecFollowCardList中
					auto tIter = IsVectorNodeContainByNode(vecFollowCardList,*iter);
					if( tIter == vecFollowCardList.end() )     
					{
						//// 添加新的组合
						iter->vecLaiZiCard.insert((*iter).vecLaiZiCard.end(),tLaiZiList.begin(),tLaiZiList.end());
						vecFollowCardList.push_back(*iter);
					}
					else   /// 选择癞子最少的组合
					{
						if( tIter->vecLaiZiCard.size() > tLaiZiList.size() )
						{
							vecFollowCardList.erase(tIter);
							//// 添加新的组合
							iter->vecLaiZiCard.insert((*iter).vecLaiZiCard.end(),tLaiZiList.begin(),tLaiZiList.end());
							vecFollowCardList.push_back(*iter);
						}
					}
				}
			}
			//// 获取再添加iLaiZiNumber-1 个癞子的结果
			if(  pSrcCard[i] == 1 && i > 1  )
			{
			   GetAddCardTypeListByLaiZi(pSrcCard,tLastCard,i-1,iLaiZiNumber-1,vecFollowCardList);
			}
			else
			{
			   GetAddCardTypeListByLaiZi(pSrcCard,tLastCard,i,iLaiZiNumber-1,vecFollowCardList);
			}
			////还原数据
			pSrcCard[i]--;
			vecLaiZi.pop_back();
		}
	}
	void CHandsNumberAndWeight::GetFollowListNoZhaDanByLaiZi(CARDATA pSrcCard,const SCombinationNode & tLastCard,int iLaiZiNumber,std::vector<SCombinationNode> & vecFollowCardList)const
	{
		if( iLaiZiNumber <= 0 || nullptr == pSrcCard )
		{
		    return;
		}
		std::vector<unsigned char> vecLaiZi;
		//// 跟牌结果获取
		switch(tLastCard.eCardType)
		{
		case eCardType_DanZhang:
			{
				//GetDanZhangListByLastCard(pSrcCard,tLastCard.vecMainCard.front(),vecFollowCardList);
			}
			break;
		case eCardType_DuiZi:
			{
				GetDuiZiListByLaiZi(pSrcCard,tLastCard.vecMainCard.front(),iLaiZiNumber,vecFollowCardList);
			}
			break;
		case eCardType_ShunZi:
			{
				GetShunZiListByLaiZi(pSrcCard,tLastCard.vecMainCard.front(),tLastCard.vecMainCard.size(),1,iLaiZiNumber,vecFollowCardList);
			}
			break;
		case eCardType_LianDui:
			{
				GetLianDuiListByLaiZi(pSrcCard,tLastCard.vecMainCard.front(),tLastCard.vecMainCard.size(),1,iLaiZiNumber,vecFollowCardList);
			}
			break;
		case eCardType_SanZhang:
		case eCardType_ZhaDan:
		case eCardType_FeiJi:
			{
				GetAddCardTypeListByLaiZi(pSrcCard,tLastCard,1,iLaiZiNumber,vecFollowCardList);
			}
			break;
		default:
			break;
		}		
	}
	//// 如果tLastCard 为炸弹并且是不带牌 获取跟牌需要特殊处理(  0:四癞 > 王炸 > 硬炸 > 软炸 1:王炸 > 四癞 > 硬炸 > 软炸 )
	void CHandsNumberAndWeight::GetFollowListZhaDan(const CARDATA pSrcCard,const SCombinationNode & tLastCard,int iLaiZiNumber,std::vector<SCombinationNode> & vecFollowCardList)const
	{
		if( iLaiZiNumber < 0 || pSrcCard == nullptr )
		{
			return;
		}
		//// 添加王炸
		if( pSrcCard[14] == 1 && pSrcCard[15] == 1 )
		{
			SCombinationNode tCombinationNode;
			AddZhaDanToNode(14,0,0,eAddCardType_No,tCombinationNode);
			int ret = CompareNodeFun(tCombinationNode,tLastCard);
			if( ret == 1  )
			{
			   vecFollowCardList.push_back(tCombinationNode);
			}
		}
		///  添加纯癞子炸弹
		for(int i=4;i<=iLaiZiNumber;i++)
		{
			SCombinationNode tCombinationNode;
			if( m_sSysConfig.bIsLongBomb == true )
			{
				AddAllLaiZiToNode(i,tCombinationNode);
			}
			else if( i == 4 )
			{
				AddAllLaiZiToNode(4,tCombinationNode);
			}
			int ret = CompareNodeFun(tCombinationNode,tLastCard);
			if( ret == 1  )
			{
				vecFollowCardList.push_back(tCombinationNode);
			}
		}
		///  添加普通炸弹
		for(int i=1;i<14;i++)
		{
			if( pSrcCard[i] == 0 )
			{
				continue;
			}
		    if( pSrcCard[i] == 4 )						//// 硬炸
			{
				if( m_sSysConfig.bIsLongBomb == true )
				{
					for(int j=0; j<= iLaiZiNumber;j++)
					{
						SCombinationNode tCombinationNode;
						int ret = 0;
						if( pSrcCard[i] + j == 4 )     
						{
							AddZhaDanToNode(i,0,0,eAddCardType_No,tCombinationNode);
							AddParamByValue(tCombinationNode.vecLaiZiCard,i,j);
							ret = CompareNodeFun(tCombinationNode,tLastCard);
						}
						else if(pSrcCard[i] + j > 4)     //// 软炸
						{
							AddZhaDanLongToNode(i,pSrcCard[i] + j,tCombinationNode);
							AddParamByValue(tCombinationNode.vecLaiZiCard,i,j);
							ret = CompareNodeFun(tCombinationNode,tLastCard);
						}
						if( ret == 1  )
						{
							vecFollowCardList.push_back(tCombinationNode);
						}
					}
				}
				else
				{
					SCombinationNode tCombinationNode;
					AddZhaDanToNode(i,0,0,eAddCardType_No,tCombinationNode);
					int ret = CompareNodeFun(tCombinationNode,tLastCard);
					if( ret == 1  )
					{
						vecFollowCardList.push_back(tCombinationNode);
					}
				}
			}
			else if( pSrcCard[i] + iLaiZiNumber >= 4 )   //// 软炸
			{
				if( m_sSysConfig.bIsLongBomb == true )
				{
					for(int j= 4-pSrcCard[i];j<=iLaiZiNumber;j++)
					{
						 SCombinationNode tCombinationNode;
						 int ret = 0;
					     if( pSrcCard[i] + j == 4 )     //// 软炸
						 {
							 AddZhaDanToNode(i,0,0,eAddCardType_No,tCombinationNode);
							 AddParamByValue(tCombinationNode.vecLaiZiCard,i,j);
							 ret = CompareNodeFun(tCombinationNode,tLastCard);
						 }
						 else if(pSrcCard[i] + j > 4)
						 {
							 AddZhaDanLongToNode(i,pSrcCard[i] + j,tCombinationNode);
							 AddParamByValue(tCombinationNode.vecLaiZiCard,i,j);
							 ret = CompareNodeFun(tCombinationNode,tLastCard);
						 }
						 if( ret == 1  )
						 {
							 vecFollowCardList.push_back(tCombinationNode);
						 }
					}
				}
				else
				{
					SCombinationNode tCombinationNode;
					AddZhaDanToNode(i,0,0,eAddCardType_No,tCombinationNode);
					AddParamByValue(tCombinationNode.vecLaiZiCard,i,4-pSrcCard[i]);
					int ret = CompareNodeFun(tCombinationNode,tLastCard);
					if( ret == 1  )
					{
						vecFollowCardList.push_back(tCombinationNode);
					}
				}
			 }
		 }
	}
	////// 公共方法 ///////////////////////////////////////////////
	/// 获取出 当前最优的带牌 带牌算法
	bool CHandsNumberAndWeight::GetSubCard(const EAddCardMainType & tAddCardMainType,SVBYTE & tVecDuiZi,SVBYTE & tVecDanZhang,int iSubCardNumber,SVBYTE & tVecSubCard,EAddCardType & eAddCardType)const
	{
		if(iSubCardNumber<= 0)
		{
			return false;
		}
		/// 判断是否能带牌
		auto IsAddDanZhang  = [&]() ->bool
		{
			switch(tAddCardMainType)
			{
			case eAddCardMainType_FeiJi:
			case eAddCardMainType_SanZhang:
				{
					if( m_sSysConfig.bIs3W1_One  )
					{
					   return true;
					}
				}
				break;
			case eAddCardMainType_SiZhan:
				{
					if( m_sSysConfig.bIs4W2_One )
					{
					   return true;
					}
				} 
			    break;
			default:
				break;
			}
		    return false;
		};
		auto IsAddDuiZi= [&]() ->bool
		{
			switch(tAddCardMainType)
			{
			case eAddCardMainType_FeiJi:
			case eAddCardMainType_SanZhang:
				{
					if( m_sSysConfig.bIs3W1_Double  )
					{
						return true;
					}
				}
				break;
			case eAddCardMainType_SiZhan:
				{
					if( m_sSysConfig.bIs4W2_Double )
					{
						return true;
					}
				} 
				break;
			default:
				break;
			}
			return false;
		};
		/// 先匹配单张 后匹配对子
		if( tVecDanZhang.size() >= iSubCardNumber && IsAddDanZhang() )
		{
			int i=0;
			for(auto iter = tVecDanZhang.begin();i < iSubCardNumber && iter != tVecDanZhang.end();)
			{
				AddParamByValue(tVecSubCard,*iter);
				iter = tVecDanZhang.erase(iter);
				i++;
			}
		}
		else if( tVecDuiZi.size() >= iSubCardNumber && IsAddDuiZi() )
		{
			int i=0;
			for(auto iter = tVecDuiZi.begin();i < iSubCardNumber && iter != tVecDuiZi.end();)
			{
				AddParamByValue(tVecSubCard,*iter,2);
				iter = tVecDuiZi.erase(iter);
				i++;
			}
		}
		else if( iSubCardNumber >=3 && IsAddDanZhang() )  //// 拆对子去补 对子只会整除拆
		{
			if( (tVecDuiZi.size()*2+tVecDanZhang.size()) >= iSubCardNumber )  /// 先使用对子 在使用单张去补
			{
				if( iSubCardNumber%2 == 0 || tVecDanZhang.size() > 0 )		  /// 整除就全部使用对子 如果不能整除就必须要有单张
				{
					int i=0;
					for(auto iter = tVecDuiZi.begin();i < iSubCardNumber && iter != tVecDuiZi.end();)    // 使用对子填补
					{
						AddParamByValue(tVecSubCard,*iter,2);
						iter = tVecDuiZi.erase(iter);
						i+=2;
						if( i+1 == iSubCardNumber ) /// 如果只剩一张 由单张补全
						{
							 break;
						}
					}
					if( i < iSubCardNumber)
					{
						for(auto iter = tVecDanZhang.begin();i < iSubCardNumber && iter != tVecDanZhang.end();) // 使用单张填补
						{
							AddParamByValue(tVecSubCard,*iter);
							iter = tVecDanZhang.erase(iter);
							i++;
						}
					}
				}
			}
		}
		else	// 不带
		{
	    
		}
		//// 统计带牌类型
		if(tVecSubCard.size() == 0)
		{
			 eAddCardType = eAddCardType_No;
		}
		else if( tVecSubCard.size() ==  iSubCardNumber  )
		{
			 eAddCardType = eAddCardType_DanZhang;
		}
		else if( tVecSubCard.size() ==  iSubCardNumber*2  )
		{
			eAddCardType = eAddCardType_DuiZi;
		}
		else
		{
			eAddCardType = eAddCardType_Unknown;
		}
		return true;
	}
	/// 获取出最坏的牌 iNumber:最坏的牌个数 单张:1 对子:2
	unsigned char CHandsNumberAndWeight::GetSubCard(const CARDATA pSrcCard,int iNumber)const
	{
		if( iNumber <= 0 )
		{
		   return 0;
		}
		unsigned char uMinValue = 255;
		unsigned char uBadCard = 0;
		for(int i=0;i<16;i++)
		{
			/// 大小王特殊处理
			if(pSrcCard[i]>0 && ( pSrcCard[i] == 14 || pSrcCard[i] == 15 ) )
			{
				if( pSrcCard[14] > 0 && pSrcCard[15] > 0 && m_sSysConfig.bIsWBomb_Separate == false )  //// 大小王不能拆
				{
					continue;
				}
			}
		    if( pSrcCard[i] >= iNumber )
			{
			    unsigned char tValue = i;
				if( i == 1 || i == 2 || i == 14 || i == 15 )
				{
				    tValue+=13;
				}
				tValue = tValue + (pSrcCard[i]-1)*13;
				if( tValue < uMinValue )
				{
				     uMinValue = tValue;
					 uBadCard = i;
				}
			}
		}
		return uBadCard;
	}
	void CHandsNumberAndWeight::GetLianDui(SVBYTE & tVecDuiZi,std::vector<SCombinationNode> & tVecCombinationResult,int & tCurValue,int & tCurHandsNumber)const
	{
		int iDuiZiSize = tVecDuiZi.size();
		unsigned char tStartLianDui = 0;
		int iLianDuiLen = 0;
		for(int i=0;i < iDuiZiSize;)
		{
			iLianDuiLen = 3;
			bool ret = false;
			do 
			{
			   ret = IsShunZi(tVecDuiZi,tVecDuiZi[i],1,iLianDuiLen);
			   if(ret)
			   {
			      iLianDuiLen++;
			   }
			} while (ret);

			if( iLianDuiLen > 3 )
			{
				SCombinationNode tCombinationNode;
			    AddLianDuiToNode(tVecDuiZi[i],iLianDuiLen-1,tCombinationNode);
				tVecCombinationResult.push_back(tCombinationNode);
				
				tCurValue += (5+(iLianDuiLen-4)*2);
				tCurHandsNumber++;

				i+= (iLianDuiLen-1);
			}
			else
			{
			    i++;
			}
		}
		//// 清理已经形成的连对
		for(auto iter = tVecCombinationResult.begin();iter!= tVecCombinationResult.end();iter++)
		{
			if( iter->eCardType != eCardType_LianDui )
			{
				continue;
			}
		    for(auto subIter = iter->vecMainCard.begin();subIter != iter->vecMainCard.end();subIter++ )
			{
			    DeleteParamByValue(tVecDuiZi,*subIter);
			}
		}
	}
	void CHandsNumberAndWeight::GetFeiJi(SVBYTE & tVecSanZhang,std::vector<SCombinationNode> & tVecCombinationResult,int & tCurValue,int & tCurHandsNumber)const
	{
		int iSanZhangSize = tVecSanZhang.size();
		unsigned char tStartFeiJi = 0;
		int iFeiJiLen = 0;
		for(int i=0;i < iSanZhangSize;)
		{
			iFeiJiLen = 2;
			bool ret = false;
			do 
			{
				ret = IsShunZi(tVecSanZhang,tVecSanZhang[i],1,iFeiJiLen);
				if(ret)
				{
					iFeiJiLen++;
				}
			} while (ret);

			if( iFeiJiLen > 2 )
			{
				SCombinationNode tCombinationNode;
				AddFeiJiToNode(tVecSanZhang[i],iFeiJiLen-1,tVecSanZhang,eAddCardType_No,tCombinationNode);
				tVecCombinationResult.push_back(tCombinationNode);

				tCurValue += (6+(iFeiJiLen-3)*3);
				tCurHandsNumber++;
				i+= (iFeiJiLen-1);
			}
			else
			{
			    i++;
			}
		}
		//// 清理已经形成的飞机
		for(auto iter = tVecCombinationResult.begin();iter!= tVecCombinationResult.end();iter++)
		{
			if( iter->eCardType != eCardType_FeiJi )
			{
				continue;
			}
			for(auto subIter = iter->vecMainCard.begin();subIter != iter->vecMainCard.end();subIter++ )
			{
				DeleteParamByValue(tVecSanZhang,*subIter);
			}
		}
	}
	bool CHandsNumberAndWeight::IsShunZi(const CARDATA pSrcCard,BYTE byCardValue,int iFlag,int iShunZiLen)const
	{
		if( pSrcCard == nullptr)
		{
		   return false;
		}
		//2和大.小王不参与顺子
		if( byCardValue <= 0 || byCardValue >= 14 || byCardValue == 2 )
		{
			return false;
		}
		int iLeftNumber = 0;
		int iRightNumber = 0;
		/// 检查左边
		int iLeftIndex = byCardValue-1;    //// 不包含本身
		if( iLeftIndex == 0 ) /// 顺子最大是A时候，还需特殊处理
		{
		    iLeftIndex = 13;
		}
		while( iLeftIndex >= 3 &&  iLeftIndex <= 13 )    ///最小是3, 最大是K  3~7 10~A   
		{
			if( pSrcCard[iLeftIndex] > 0  )
			{
				iLeftNumber++;
				iLeftIndex--;
			}
			else
			{
				break;
			}
		}
		///  检查右边
		int iRightIndex = byCardValue+1; //// 不包含本身
		while( iRightIndex >= 3 && iRightIndex <= 13 )     ///最小是3, 最大是K  3~7 10~A   
		{
			if( pSrcCard[iRightIndex] > 0 )
			{
				iRightNumber++;
				iRightIndex++;
			}
			else
			{
				break;
			}
		}
	    if( iRightIndex == 14 && pSrcCard[1] > 0 )        /// 顺子最大是A，如果是A时候，还需特殊处理
		{
		    iRightNumber++;
		}
		//// 判断是否 大于 iShunZiLen
		if( iFlag == 0 )  /// iFlag:0 以byCardValue为中心向连边寻找  
		{
			return (iLeftNumber+iRightNumber+1)>=iShunZiLen?true:false;
		}
		else if(iFlag == 1) /// 1：以byCardValue为最小点向上寻找
		{
			return (iRightNumber+1)>=iShunZiLen?true:false;
		}
		else if( iFlag == 2 ) // 2：以byCardValue为最大点向下寻找
		{
			return (iLeftNumber+1)>=iShunZiLen?true:false;
		}
		return false;
	}

	bool CHandsNumberAndWeight::IsShunZi(const SVBYTE & tVec,BYTE byCardValue,int iFlag,int iShunZiLen /*= 5*/)const
	{
		CARDATA tTemp;
		memset(tTemp,0,sizeof(tTemp));
		for(auto iter = tVec.begin();iter!= tVec.end();iter++)
		{
		    if( (*iter) > 0 && (*iter) < 16)
			{
			    tTemp[*iter]++;
			}
			else
			{
			    return false;
			}
		}
		return IsShunZi(tTemp,byCardValue,iFlag,iShunZiLen);
	}

	bool CHandsNumberAndWeight::IsShunZi(const SVBYTE & tVec)const
	{
		if( tVec.size() == 0 )
		{
		    return true;
		}
		return IsShunZi(tVec,tVec.front(),0,tVec.size());
	}
	/// A 2 大于 3~13
	bool CHandsNumberAndWeight::SpecialComparisonFun(int a,int b) const
	{
		if( (a == 1 || a== 2) && (b >= 3 && b <= 13) )
		{
			return true;
		}
		else if( (b == 1 || b == 2) && (a >=3 && a <= 13))
		{
			return false;
		}
		else
		{
			return a > b;
		}
	}
	/// 比较两Node的大小 -1:不能比较 0:tNode1 == tNode2 1：tNode1 > tNode2  2：tNode1 < tNode2
	int CHandsNumberAndWeight::CompareNodeFun(const SCombinationNode & tNode1,const SCombinationNode & tNode2)const
	{
		if( tNode1.vecMainCard.size() == 0 || tNode2.vecMainCard.size() == 0 )
		{
		    return -1;
		}
		///// 先比炸弹 ////////////////////////////////////////////////
		 if( tNode1.eCardType == eCardType_ZhaDan && tNode1.eAddCardType == eAddCardType_No
			 && ( tNode2.eCardType != eCardType_ZhaDan || (tNode2.eCardType == eCardType_ZhaDan && tNode2.eAddCardType != eAddCardType_No) )
			 ) /// 炸弹能打非炸弹牌
		{
			return 1;
		}
		else if( tNode2.eCardType == eCardType_ZhaDan && tNode2.eAddCardType == eAddCardType_No 
			&& ( tNode1.eCardType != eCardType_ZhaDan || (tNode1.eCardType == eCardType_ZhaDan && tNode1.eAddCardType != eAddCardType_No) )
			)
		{
			return 2;
		}
		///// 两个炸弹比较 /////////////////////////////////////////////
		else if( tNode2.eCardType == eCardType_ZhaDan && tNode2.eAddCardType == eAddCardType_No 
			 &&  tNode1.eCardType == eCardType_ZhaDan && tNode1.eAddCardType == eAddCardType_No
			)
		{
			if( tNode1.vecMainCard.size() == 2 )			 //// 王炸
			{
				if( m_sSysConfig.iBombCompareType == 0 )
				{
					//// 王炸 纯癞子和长炸弹可以跟
					if( tNode2.vecMainCard.size() >= 4)
					{
						return 2;
					}
				}
			}
			else if( tNode1.vecLaiZiCard.size() == 0 )    //// 硬炸
			{
				if( tNode1.vecMainCard.size() < tNode2.vecMainCard.size() )  /// 长炸弹比短炸弹大
				{
					return 2;
				}
				if( tNode2.vecMainCard.size() == 2 || tNode2.vecLaiZiCard.size() == 4                             /// 王炸和4癞可以管
					|| ( tNode2.vecLaiZiCard.size() == 0 && SpecialComparisonFun(tNode2.vecMainCard.front(),tNode1.vecMainCard.front()))
					)
				{
					return 2;
				}
				if( tNode2.vecLaiZiCard.size() == 0 && tNode2.vecMainCard.front() == tNode1.vecMainCard.front() )
				{
				    return 0;
				}
			}
			else if( tNode1.vecLaiZiCard.size() > 0 && tNode1.vecLaiZiCard.size() != tNode1.vecMainCard.size() ) /// 软炸
			{ 
				if( tNode1.vecMainCard.size() < tNode2.vecMainCard.size() )  /// 长炸弹比短炸弹大
			    {
						 return 2;
				}
				if( m_sSysConfig.iBombCompareType == 1 || tNode1.vecMainCard.size() == 4 )  /// 王炸最大
				{
					if( tNode2.vecMainCard.size() == 2 )    
					{
						return 2;
					}
				}
				if(tNode1.vecMainCard.size() == tNode2.vecMainCard.size() )  /// 相等
				{
				    if( tNode2.vecMainCard.size() == tNode2.vecLaiZiCard.size() )	/// 纯炸弹比非纯炸弹大					 {
					{
						 return 2;
					}
					else if( tNode2.vecLaiZiCard.size() == 0 )					    /// 硬炸弹比软炸弹大					 {
					{
						return 2;
					}
					else if( SpecialComparisonFun(tNode2.vecMainCard.front(),tNode1.vecMainCard.front()) )
					{
						 return 2;
					}
					else if( tNode2.vecMainCard.front() == tNode1.vecMainCard.front() )
					{
					     return 0;
					}
				}
			}
			else if( tNode1.vecLaiZiCard.size() == tNode1.vecMainCard.size() )   /// 纯癞子 炸弹
			{
				if( tNode1.vecMainCard.size() < tNode2.vecMainCard.size() )  /// 长炸弹比短炸弹大
				{
					return 2;
				}
				if( m_sSysConfig.iBombCompareType == 1 )  /// 王炸最大
				{
					if( tNode2.vecMainCard.size() == 2 )    
					{
						return 2;
					}
				}
				if(tNode1.vecMainCard.size() == tNode2.vecMainCard.size() )  /// 相等
				{
					if( tNode2.vecMainCard.size() == tNode2.vecLaiZiCard.size()
						&& SpecialComparisonFun(tNode2.vecMainCard.front(),tNode1.vecMainCard.front()) )
					{
						return 2;
					}
					else if( tNode2.vecMainCard.size() == tNode2.vecLaiZiCard.size()
						&& tNode2.vecMainCard.front() == tNode1.vecMainCard.front() )
					{
						return 0;
					}
				}
			 }
			return 1;
		}
		//// 非炸弹比较 //////////////////////////
		else if( tNode1.eCardType == tNode2.eCardType && tNode1.eAddCardType == tNode2.eAddCardType && tNode1.vecMainCard.size() == tNode2.vecMainCard.size() )
		{
			bool ret = SpecialComparisonFun(tNode1.vecMainCard.front(),tNode2.vecMainCard.front());
			if( ret )
			{
			   return 1;
			}
			else if( tNode1.vecMainCard.front() == tNode2.vecMainCard.front() )
			{
			   return 0;
			}
			else
			{
			   return 2;
			}
		}
		return -1;
	}
	///  数据分类
	void CHandsNumberAndWeight::ClassificationData(const CARDATA pSrcCard,SVBYTE & tVecDanZhang,SVBYTE & tVecDuiZi,SVBYTE & tVecSanZhang,SVBYTE & tVecSiZhang,std::vector<SZhaDanLongNode> & tVecZhaDanLong) const
	{
		for(int i=1;i<16;i++)
		{

			if( pSrcCard[i] == 1 )
			{
				tVecDanZhang.push_back(i);
			}
			else if( pSrcCard[i] == 2 )
			{
				tVecDuiZi.push_back(i);
			}
			else if( pSrcCard[i] == 3 )
			{
				tVecSanZhang.push_back(i);
			}
			else if( pSrcCard[i] == 4 )
			{
				tVecSiZhang.push_back(i);
			}
			else if(pSrcCard[i] > 4)
			{
				SZhaDanLongNode tNode;
				tNode.byValue = i;
				tNode.iLen = pSrcCard[i];
			    tVecZhaDanLong.push_back(tNode);
			}
		}
	}

	int CHandsNumberAndWeight::ChangeHandPaiData(const unsigned char *pSrcCard,int iSrcCardLen)
	{
		if(pSrcCard == nullptr || iSrcCardLen < 0)
		{
			return 0;
		}
		int iCount = 0;
		memset(m_byCardData,0,sizeof(m_byCardData));
		for (int i=0;i<iSrcCardLen;i++)
		{
			if( pSrcCard[i] >0 && pSrcCard[i] < 16 )
			{
				m_byCardData[ pSrcCard[i] ]++;
				iCount++;
			}
		}
		return iCount;
	}
	int CHandsNumberAndWeight::GetCardCount()const
	{
		int iCount = 0;
		for(int i=0;i<16;i++)
		{
			iCount += m_byCardData[i];
		}
		return iCount;
	}
	bool CHandsNumberAndWeight::IsEqualByNode(const SCombinationNode & tNode1,const SCombinationNode & tNode2,int flag)const
	{
		do 
		{
			if( tNode1.vecMainCard.size() ==0 || tNode2.vecMainCard.size() ==0)
			{
			   return false;
			}
			if( tNode1.eCardType != tNode2.eCardType)
			{
				return false;
			} 
			if( tNode1.vecMainCard.front() != tNode2.vecMainCard.front() )
			{
				return false;
			}
			if( flag == 1)
			{
				return true;
			}
			if( tNode1.eAddCardType != tNode2.eAddCardType )
			{
				return false;
			}
			if( flag == 0)
			{
			   return true;
			}

		} while (false);
		return false;
	}
	bool CHandsNumberAndWeight::IsNodeContainByVector(const SCombinationNode & tNode1,const SVBYTE & tVecData) const
	{
		SCombinationNode tTmpNode = tNode1;
		for(auto iter = tVecData.begin();iter!=tVecData.end();iter++)
		{
			/// 在主牌中查询
			auto mainIter = find(tTmpNode.vecMainCard.begin(),tTmpNode.vecMainCard.end(), *iter);
			if(mainIter != tTmpNode.vecMainCard.end())
			{
				tTmpNode.vecMainCard.erase(mainIter);
				continue;
			}
			/// 在副牌中查询
			auto subIter = find(tTmpNode.vecSubCard.begin(),tTmpNode.vecSubCard.end(), *iter);
			if(subIter != tTmpNode.vecSubCard.end())
			{
				tTmpNode.vecSubCard.erase(subIter);
			}
			else
			{
			    return false;
			}
		}
		return true;
	}
	std::vector<SCombinationNode>::iterator CHandsNumberAndWeight::IsVectorNodeContainByNode(std::vector<SCombinationNode> & vecNodeList,const SCombinationNode & tNode1) const
	{
		for(auto iter = vecNodeList.begin();iter!=vecNodeList.end();iter++)
		{
		   bool ret = IsEqualByNode(*iter,tNode1,0);
		   if( ret == true)
		   {
		       return iter;
		   }
		}
		return vecNodeList.end();
	}
	bool CHandsNumberAndWeight::IsVecContainVec(const SVBYTE & vecA,const SVBYTE & vecB)const
	{
		SVBYTE tTmpVec;
		tTmpVec.insert(tTmpVec.end(),vecA.begin(),vecA.end());
		for(auto iter = vecB.begin();iter!= vecB.end();iter++)
		{
			auto tmpIter = find(tTmpVec.begin(),tTmpVec.end(),*iter);
			if( tmpIter != tTmpVec.end())
			{
				tTmpVec.erase(tmpIter);
				continue;
			}
			else
			{
				return false;
			}	
		}
		return true;
	}

	/// 冒泡排序
	void CHandsNumberAndWeight::ResultSort(std::vector<SCombinationNode> & tVecCombinationResult,int iFlag)const
	{
		auto ComparisonFun = [&](SCombinationNode & a, SCombinationNode & b) -> bool 
		{
			if( a.vecMainCard.empty() || b.vecMainCard.empty() )
			{
				return false;
			}
			bool bIsACR = ( (a.eCardType > b.eCardType) || ( a.eCardType == b.eCardType && SpecialComparisonFun(a.vecMainCard.front(),b.vecMainCard.front()) ) );
			switch(iFlag)
			{
			case 0:
				return bIsACR;
			case 1:
				return !bIsACR;
			case 2:
				{
				    if( a.vecLaiZiCard.size() > b.vecLaiZiCard.size() )
					{
					    bIsACR = true;
					}
					else if( a.vecLaiZiCard.size() < b.vecLaiZiCard.size() )
					{
					   bIsACR = false;
					}
					else
					{
					    /// bIsACR 升序排序
					}
				}
			case 4:
				return ( !CompareType(a.eCardType, b.eCardType) && a.eCardType != b.eCardType) || (a.eCardType == b.eCardType && SpecialComparisonFun(a.vecMainCard.front(),b.vecMainCard.front()));
			case 5:
				{
					//包含最小牌的牌型最先出，其他的按照牌型优先级
					if(NodeEquale(a, m_tMaxNode) && !NodeEquale(b, m_tMaxNode)) return false;
					if(!NodeEquale(a, m_tMaxNode) && NodeEquale(b, m_tMaxNode)) return true;

					return ( !CompareType(a.eCardType, b.eCardType) && a.eCardType != b.eCardType) || (a.eCardType == b.eCardType && SpecialComparisonFun(a.vecMainCard.front(),b.vecMainCard.front()));
				}
			default: 
				break;
			}
			return a.eCardType < b.eCardType; 
		};
		int iResultLen = tVecCombinationResult.size();
		for(int i=0;i< iResultLen-1;i++)
		{
			 for(int j=i+1;j<iResultLen;j++)
			 {
				 if( ComparisonFun(tVecCombinationResult[i],tVecCombinationResult[j]) == true )
				 {
					  SCombinationNode tCombinationNode;
					  tCombinationNode = tVecCombinationResult[i];
					  tVecCombinationResult[i] = tVecCombinationResult[j];
					  tVecCombinationResult[j] = tCombinationNode;
				 }
			 }
		}
	}
	/// 删除某一个元素
	bool CHandsNumberAndWeight::DeleteParamByValue(SVBYTE & tVecDelete,BYTE byValue) const
	{
		auto iter = find(tVecDelete.begin(),tVecDelete.end(),byValue);
		if( iter!= tVecDelete.end())
		{
			tVecDelete.erase(iter);
			return true;
		}
		return false;
	}
	///  tVecAdd 添加iNumber个byValue 
	bool CHandsNumberAndWeight::AddParamByValue(SVBYTE & tVecAdd,BYTE byValue,int iNumber)const
	{
		for(int i=0;i<iNumber;i++)
		{
			tVecAdd.push_back(byValue);
		}
		return true;
	}
	///// 顺子添加
	bool CHandsNumberAndWeight::AddShunZiToNode(unsigned char ucStart,int iShunZiLen,SCombinationNode & tNode) const
	{
		if( iShunZiLen < 5 || ucStart < 3 || ucStart > 10 )
		{
		    return false;
		}
		tNode.clear();
		tNode.eCardType = eCardType_ShunZi;
		tNode.eAddCardType = eAddCardType_No;
		for(int i=0;i< iShunZiLen;i++)
		{
			if( ucStart + i == 14)						//// A特殊处理
			{
				AddParamByValue(tNode.vecMainCard,1);
			}
			else
			{
				AddParamByValue(tNode.vecMainCard,ucStart + i);
			}
		}
		return true;
	}

	bool CHandsNumberAndWeight::AddShunZiToNode(const SVBYTE & tVecDanZhang,SCombinationNode & tNode) const
	{
		if( tVecDanZhang.size() < 0 || false == IsShunZi(tVecDanZhang) )
		{
			return false;
		}
		unsigned char ucStart = tVecDanZhang.front();
		if( ucStart == 1)
		{
			ucStart = tVecDanZhang[1];
		}
		return AddShunZiToNode(ucStart,tVecDanZhang.size(),tNode);
	}

	bool CHandsNumberAndWeight::AddDanZhangToNode(unsigned char ucValue,SCombinationNode & tNode) const
	{
		if(ucValue < 1 || ucValue > 15)
		{
		    return false;
		}
		tNode.clear();
		tNode.eCardType = eCardType_DanZhang;
		tNode.eAddCardType = eAddCardType_No;
		AddParamByValue(tNode.vecMainCard,ucValue,1);
		return true;
	}
	bool CHandsNumberAndWeight::AddDuiZiToNode(unsigned char ucValue,SCombinationNode & tNode) const
	{
		if(ucValue < 1 || ucValue > 15)
		{
			return false;
		}
		tNode.clear();
		tNode.eCardType = eCardType_DuiZi;
		tNode.eAddCardType = eAddCardType_No;
		AddParamByValue(tNode.vecMainCard,ucValue,2);
		return true;
	}
	bool CHandsNumberAndWeight::AddSanZhangToNode(unsigned char ucValue,unsigned char ucDaiPai,EAddCardType tAddCardType,SCombinationNode & tNode) const
	{
		if(ucValue < 1 || ucValue > 15)
		{
			return false;
		}
		tNode.clear();
		tNode.eCardType = eCardType_SanZhang;
		tNode.eAddCardType = tAddCardType;
		AddParamByValue(tNode.vecMainCard,ucValue,3);
		switch(tAddCardType)
		{
		case  eAddCardType_DuiZi:
			{
				AddParamByValue(tNode.vecSubCard,ucDaiPai,2);
			}
			break;
		case  eAddCardType_DanZhang:
			{
				AddParamByValue(tNode.vecSubCard,ucDaiPai,1);
			}
			break;
		case eAddCardType_No:
			{
			
			}
			break;
		default:
			return false;
		}
		return true;
	}
	bool CHandsNumberAndWeight::AddZhaDanToNode(unsigned char ucValue,unsigned char ucDaiPai1,unsigned char ucDaiPai2,EAddCardType tAddCardType,SCombinationNode & tNode) const
	{
		if(ucValue < 1 || ucValue > 15)
		{
			return false;
		}
		tNode.clear();
		tNode.eCardType = eCardType_ZhaDan;
		if( ucValue == 14 )
		{
		   tAddCardType = eAddCardType_No;
		   AddParamByValue(tNode.vecMainCard,14);
		   AddParamByValue(tNode.vecMainCard,15);
		}
		else
		{
		   AddParamByValue(tNode.vecMainCard,ucValue,4);
		}
        /// 配牌添值
		tNode.eAddCardType = tAddCardType;
		switch(tAddCardType)
		{
		case  eAddCardType_DuiZi:
			{
				AddParamByValue(tNode.vecSubCard,ucDaiPai1,2);
				AddParamByValue(tNode.vecSubCard,ucDaiPai2,2);
			}
			break;
		case  eAddCardType_DanZhang:
			{
				AddParamByValue(tNode.vecSubCard,ucDaiPai1,1);
				AddParamByValue(tNode.vecSubCard,ucDaiPai2,1);
			}
			break;
		case eAddCardType_No:
			{

			}
			break;
		default:
			return false;
		}
		return true;
	}

	bool CHandsNumberAndWeight::AddZhaDanLongToNode(unsigned char ucValue,int iNumber,SCombinationNode & tNode) const
	{
		if(ucValue < 1 || ucValue > 15 || iNumber <= 4)
		{
			return false;
		}
		tNode.clear();
		tNode.eCardType = eCardType_ZhaDan;
		AddParamByValue(tNode.vecMainCard,ucValue,iNumber);
		/// 配牌添值
		tNode.eAddCardType = eAddCardType_No;
		return true;
	}

	bool CHandsNumberAndWeight::AddLianDuiToNode(unsigned char ucStart,int iLianDuiLen,SCombinationNode & tNode) const
	{
		if( iLianDuiLen < 3 || ucStart < 3 || ucStart > 12 )
		{
			return false;
		}
		tNode.clear();
		tNode.eCardType = eCardType_LianDui;
		tNode.eAddCardType = eAddCardType_No;
		for(int i=0;i< iLianDuiLen;i++)
		{
			if( ucStart + i == 14)
			{
				AddParamByValue(tNode.vecMainCard,1,2);
			}
			else
			{
				AddParamByValue(tNode.vecMainCard,ucStart + i,2);
			}
		}
		return true;
	}
	bool CHandsNumberAndWeight::AddLianDuiToNode(const SVBYTE & tVecDuiZi,SCombinationNode & tNode) const
	{
		if( tVecDuiZi.size() < 3 || false == IsShunZi(tVecDuiZi) )
		{
		   return false;
		}
		unsigned char ucStart = tVecDuiZi.front();
		if( ucStart == 1)
		{
			ucStart = tVecDuiZi[1];
		}
		return AddLianDuiToNode(ucStart,tVecDuiZi.size(),tNode);
	}

	bool CHandsNumberAndWeight::AddFeiJiToNode(unsigned char ucStart,int iFeiJiLen,const SVBYTE & tVecDaiPai,EAddCardType tAddCardType,SCombinationNode & tNode) const
	{
		if( iFeiJiLen < 2 || ucStart < 3 || ucStart > 13  )
		{
			return false;
		}
		tNode.clear();
		tNode.eCardType = eCardType_FeiJi;
		for(int i=0;i< iFeiJiLen;i++)
		{
			if( ucStart + i == 14)
			{
				AddParamByValue(tNode.vecMainCard,1,3);
			}
			else
			{
				AddParamByValue(tNode.vecMainCard,ucStart + i,3);
			}
		}
		/// 配牌添值
		tNode.eAddCardType = tAddCardType;
		switch(tAddCardType)
		{
		case  eAddCardType_DuiZi:
			{
				for(int i = 0;i<iFeiJiLen && i<tVecDaiPai.size();i++)
				{
				   AddParamByValue(tNode.vecSubCard,tVecDaiPai[i],2);
				}
			}
			break;
		case  eAddCardType_DanZhang:
			{
				for(int i = 0;i<iFeiJiLen && i<tVecDaiPai.size();i++)
				{
					AddParamByValue(tNode.vecSubCard,tVecDaiPai[i],1);
				}
			}
			break;
		case eAddCardType_No:
			{

			}
			break;
		default:
			return false;
		}
		return true;
	}

	bool CHandsNumberAndWeight::AddFeiJiToNode(const SVBYTE & tVecSanZhang,const SVBYTE & tVecDaiPai,EAddCardType tAddCardType,SCombinationNode & tNode) const
	{
		if( tVecSanZhang.size() < 2 || false == IsShunZi(tVecSanZhang) )
		{
			return false;
		}
		unsigned char ucStart = tVecSanZhang.front();
		if( ucStart == 1)
		{
			ucStart = tVecSanZhang[1];
		}
		return AddFeiJiToNode(ucStart,tVecSanZhang.size(),tVecDaiPai,tAddCardType,tNode);
	}

	//设置检查类型
	void CHandsNumberAndWeight::SetCheckType(int iType)
	{
		m_iCheckType = iType;
	}

	void CHandsNumberAndWeight::SetFirstFlag(bool bFirst)
	{
		m_bFirstOut = bFirst;
	}

	//获取检查类型，返回-1表示检查失败
	int CHandsNumberAndWeight::GetCheckType()
	{
		return m_iCheckType;
	}

	//映射后重新比较类型
	bool CHandsNumberAndWeight::CompareType(ECardType eCardTypeA, ECardType eCardTypeB)const
	{
		//初始比较
		/*enum ECardType
		{
			eCardType_Unknown=0,	/// 未知类型
			eCardType_DanZhang=1,	/// 单张
			eCardType_ShunZi,		/// 顺子
			eCardType_DuiZi,		/// 对子
			eCardType_LianDui,		/// 连对
			eCardType_SanZhang,		/// 3张
			eCardType_FeiJi,		/// 飞机
			eCardType_ZhaDan,	    /// 炸弹
		};*/
		//新需求：飞机＞顺子＞连对＞三带一（对）＞对子＞单牌）,不包括炸弹，炸弹最后出

		const int iBase[8] = {0,1,5,2,4,3,6,0};
		return iBase[eCardTypeA] >= iBase[eCardTypeB];
	}

	//检查
	void CHandsNumberAndWeight::DoCheck()
	{
		//已经检查通过，不再检查
		if(m_iCheckType == SuccessCheck)	return;

		 //// 权值定义
		 /* 单张							  小于等于10为1 大于等于10为2 
			对子                              小于等于10为1 大于等于10为2 
			三带                              3
			连牌                              4 (每多一张牌权值+1)
			连对                              5（每多一对牌，权值+2）
			飞机                              6（每对以飞机，权值在基础上+3）
			炸弹                              7（包括对王在内） 
		 */
		int tCurValue = 0;           /// 当前权值
		int tCurHandsNumber =0;      /// 当前手数
		std::vector<SCombinationNode>  tVecCombinationResult;  ///当前组牌结果
		std::vector<SCombinationNode>  tVecResultNoWith; ///当前组牌 不带牌结果
		tVecCombinationResult.clear();
		tVecResultNoWith.clear();
		////  计算王炸 手数和权值 //////////////////////////////////////////////
		if( m_byCombinationData[14] > 0 && m_byCombinationData[15] > 0 )
		{
			tCurHandsNumber++;
			tCurValue += 7;
			/// 清空已经计算的结果
			m_byCombinationData[14] = 0;
			m_byCombinationData[15] = 0;
			/// 保存结果
			SCombinationNode tCombinationNode;
			AddZhaDanToNode(14,0,0,eAddCardType_No,tCombinationNode);
			tVecCombinationResult.push_back(tCombinationNode);
			tVecResultNoWith.push_back(tCombinationNode);
		}
		///// 计算顺子的 手数和权值 ////////////////////////////////////////////
		for(auto iter = m_vetorShunZi.begin();iter!= m_vetorShunZi.end();iter++)
		{
			tCurHandsNumber++;
			tCurValue += (*iter).iLen - 1;
			/// 保存结果
			SCombinationNode tCombinationNode;
			AddShunZiToNode((*iter).byValue,(*iter).iLen,tCombinationNode);
			tVecCombinationResult.push_back(tCombinationNode);
			tVecResultNoWith.push_back(tCombinationNode);
		}
		/// 计算单独的牌型 权值 //////////////////////////////////////////////////////
		/// 计算单独的牌型 不可以带牌和连牌 手数和权数  
		GetCombinationResultNoWith(m_byCombinationData,tVecResultNoWith);
		/// 计算单独的牌型 可以带牌和连牌 手数和权数
		GetCombinationResult(m_byCombinationData,tVecCombinationResult,tCurValue,tCurHandsNumber);
		/// 如果有最优结果 则保存最优组合
		if( tCurHandsNumber < m_iMinHandsNumber || (tCurHandsNumber == m_iMinHandsNumber && tCurValue > m_iMaxValue) )
		{
			m_iMaxValue = tCurValue;
			m_iMinHandsNumber = tCurHandsNumber;
			m_vecResultNoWith.clear();
			m_vecResultNoWith.insert(m_vecResultNoWith.end(),tVecResultNoWith.begin(),tVecResultNoWith.end());
			m_vecCombinationResult.clear();
			m_vecCombinationResult.insert(m_vecCombinationResult.end(),tVecCombinationResult.begin(),tVecCombinationResult.end());
		}

		int iSanZhangCount	 = 0;									//单张
		int iDanZhangCount	 = 0;									//三张
		int iDuiZiCount		 = 0;									//对子
		int iBombCount		 = 0;									//炸弹
		int iLianDuiCount	 = 0;									//连队
		int iShunZiCount	 = 0;									//顺子
		bool bHaveA(false), bHave2(false), bHaveKing(false);		//有A、有2、有王
		int iDanZhangLess2	 = 0;									//小于2的非王单张
		int iDuiZiLess10	 = 0;									//小于10的对子数
		for(int i = 0; i < (int)tVecCombinationResult.size(); ++i)
		{
			if(tVecCombinationResult[i].eCardType == eCardType_SanZhang)
			{
				iSanZhangCount++;
			}
			if(tVecCombinationResult[i].eCardType == eCardType_FeiJi)
			{
				iSanZhangCount += (tVecCombinationResult[i].vecMainCard.size() / 3);
			}
			if(tVecCombinationResult[i].eCardType == eCardType_DanZhang)
			{
				iDanZhangCount++;
			}
			if(tVecCombinationResult[i].eCardType == eCardType_DuiZi)
			{
				iDuiZiCount++;
			}
			if(tVecCombinationResult[i].eCardType == eCardType_LianDui)
			{
				iLianDuiCount++;
			}
			if(tVecCombinationResult[i].eCardType == eCardType_ShunZi)
			{
				iShunZiCount++;
			}
			if(tVecCombinationResult[i].eCardType == eCardType_ZhaDan)
			{
				iBombCount++;
			}
			if(tVecCombinationResult[i].eCardType == eCardType_DanZhang && tVecCombinationResult[i].vecMainCard[0] == 1)
			{
				bHaveA = true;
			}
			if(tVecCombinationResult[i].eCardType == eCardType_DanZhang && tVecCombinationResult[i].vecMainCard[0] == 2)
			{
				bHave2 = true;
			}
			if(tVecCombinationResult[i].eCardType == eCardType_DanZhang && tVecCombinationResult[i].vecMainCard[0] == 0x4E)
			{
				bHaveKing = true;
			}
			if(tVecCombinationResult[i].eCardType == eCardType_DanZhang && tVecCombinationResult[i].vecMainCard[0] == 0x4F)
			{
				bHaveKing = true;
			}
			if(tVecCombinationResult[i].eCardType == eCardType_DanZhang && SpecialComparisonFun(2, tVecCombinationResult[i].vecMainCard[0]))
			{
				iDanZhangLess2++;
			}
			if(tVecCombinationResult[i].eCardType == eCardType_DuiZi && SpecialComparisonFun(10, tVecCombinationResult[i].vecMainCard[0]))
			{
				iDuiZiLess10++;
			}
		}

		switch(m_iCheckType)
		{
		case SanZhangCheck:
			if(iSanZhangCount >= 3)
			{
				m_iCheckType = SuccessCheck;
			}
			break;
		case NoSanPaiCheck:
			if(iDanZhangCount + iDuiZiCount < 1)
			{
				m_iCheckType = SuccessCheck;
			}
			break;
		case SanZhang_Double:
			{
				if(iSanZhangCount >= 4 ) m_iCheckType = SuccessCheck;
				if(iSanZhangCount == 3 && (iShunZiCount >=1 || iLianDuiCount >= 1 || iBombCount >= 1)) m_iCheckType = SuccessCheck;
				break;
			}
		case A2King_Double:
			{
				if(iShunZiCount == 3 && bHaveA && bHave2 && bHaveKing) m_iCheckType = SuccessCheck;
				break;
			}
		case DanZhang_Double:
			{
				if(iDanZhangCount + iDuiZiCount < 1 && iDanZhangLess2 <= 1) m_iCheckType = SuccessCheck;
				break;
			}
		case SanPai_Not:
			{
				if(iDuiZiLess10 + iDanZhangLess2 < 3)	m_iCheckType = SuccessCheck;
				break;
			}
		}
	}

	bool CHandsNumberAndWeight::AddAllLaiZiToNode(int iLaiZiLen,SCombinationNode & tNode)const
	{
		if( iLaiZiLen <= 0 )
		{
		   return false;
		}
		tNode.clear();
		switch(iLaiZiLen)
		{
		case 1:
			tNode.eCardType = eCardType_DanZhang;
			break;
		case 2:
			tNode.eCardType = eCardType_DuiZi;
			break;
		case 3:
			tNode.eCardType = eCardType_SanZhang;
			break;
		default:
			tNode.eCardType = eCardType_ZhaDan;
			break;
		}
		tNode.eAddCardType = eAddCardType_No;
		for(int i=0;i<iLaiZiLen;i++)
		{
		   tNode.vecMainCard.push_back(0);      /// 0 代表癞子本身值
		   tNode.vecLaiZiCard.push_back(0);
		}
	    return true;
	}
	/////// 牌型获取 ////////////////////////////////////////
	bool CHandsNumberAndWeight::GetCardTypeBySrc(const CARDATA pSrcCard,SCombinationNode & tCardType)const
	{
		//// 对牌型分类
		SVBYTE tVecDanZhang;  /// 单张  必须是有序的 下面算法均是在有序基础上实现
		SVBYTE tVecDuiZi;	  /// 对子  必须是有序的 下面算法均是在有序基础上实现
		SVBYTE tVecSanZhang;  /// 三张  必须是有序的 下面算法均是在有序基础上实现
		SVBYTE tVecSiZhang;   /// 四张  必须是有序的 下面算法均是在有序基础上实现
		std::vector<SZhaDanLongNode> tVecZhaDanLong;     /// 四张以上  必须是有序的 下面算法均是在有序基础上实现
		ClassificationData(pSrcCard,tVecDanZhang,tVecDuiZi,tVecSanZhang,tVecSiZhang,tVecZhaDanLong);
		if( tVecDanZhang.size() == 1 && tVecDuiZi.empty() && tVecSanZhang.empty() &&  tVecSiZhang.empty() && tVecZhaDanLong.empty() )  /// 单张
		{
			return AddDanZhangToNode(tVecDanZhang[0],tCardType);
		}
		if( tVecDanZhang.size() == 2 && tVecDuiZi.empty() && tVecSanZhang.empty() &&  tVecSiZhang.empty() && tVecZhaDanLong.empty() ) /// 王炸
		{
			if( tVecDanZhang[0]==14 && tVecDanZhang[1] == 15 )
			{
				return AddZhaDanToNode(14,0,0,eAddCardType_No,tCardType);
			}
		}
		if( tVecDuiZi.size() == 1 && tVecDanZhang.empty() &&  tVecSanZhang.empty() &&  tVecSiZhang.empty() && tVecZhaDanLong.empty() ) /// 对子
		{
			return AddDuiZiToNode(tVecDuiZi[0],tCardType);
		}
		if(  tVecSanZhang.size() == 1 &&  tVecSiZhang.empty() && tVecZhaDanLong.empty() )											   /// 三张
		{
			if( tVecDuiZi.size() == 1 && tVecDanZhang.empty() && m_sSysConfig.bIs3W1_Double )
			{
				return AddSanZhangToNode(tVecSanZhang[0],tVecDuiZi[0],eAddCardType_DuiZi,tCardType);
			}
			else if(  tVecDuiZi.empty()  && tVecDanZhang.size()== 1 && m_sSysConfig.bIs3W1_One )
			{
				return AddSanZhangToNode(tVecSanZhang[0],tVecDanZhang[0],eAddCardType_DanZhang,tCardType);
			}
			else if(  tVecDuiZi.empty() && tVecDanZhang.empty() )
			{
				return AddSanZhangToNode(tVecSanZhang[0],0,eAddCardType_No,tCardType);
			}
		}
		if(  tVecSiZhang.size() == 1 &&  tVecSanZhang.empty() && tVecZhaDanLong.empty())											   //// 四张
		{
			if( tVecDuiZi.size() == 2 && tVecDanZhang.empty() && m_sSysConfig.bIs4W2_Double )
			{
				return AddZhaDanToNode(tVecSiZhang[0],tVecDuiZi[0],tVecDuiZi[1],eAddCardType_DuiZi,tCardType);
			}
			else if(tVecDuiZi.size() == 1 && tVecDanZhang.empty() && m_sSysConfig.bIs4W2_One )
			{
				return AddZhaDanToNode(tVecSiZhang[0],tVecDuiZi[0],tVecDuiZi[0],eAddCardType_DanZhang,tCardType);
			}
			else if(  tVecDuiZi.empty()  && tVecDanZhang.size()== 2 && m_sSysConfig.bIs4W2_One )
			{
				return AddZhaDanToNode(tVecSiZhang[0],tVecDanZhang[0],tVecDanZhang[1],eAddCardType_DanZhang,tCardType);
			}
			else if(  tVecDuiZi.empty() && tVecDanZhang.empty() )
			{
				return AddZhaDanToNode(tVecSiZhang[0],0,0,eAddCardType_No,tCardType);
			}
		}

		if(  tVecSiZhang.size() == 2 &&  tVecSanZhang.empty() && tVecZhaDanLong.empty() && tVecDanZhang.empty() && tVecDuiZi.empty() )					//// 特殊 牌型
		{
			if( m_sSysConfig.bIs4W2_Double )
			{
				return AddZhaDanToNode(tVecSiZhang[1],tVecSiZhang[0],tVecSiZhang[0],eAddCardType_DuiZi,tCardType);
			}
		}

		if(  tVecSiZhang.size() > 2 &&  tVecSanZhang.empty() && tVecZhaDanLong.empty() && tVecDanZhang.empty() && tVecDuiZi.empty() )					//// 特殊 牌型
		{
			if( m_sSysConfig.bIs3W1_One )
			{
				return AddFeiJiToNode(tVecSiZhang,tVecSiZhang,eAddCardType_DanZhang,tCardType);
			}
		}

		if( tVecZhaDanLong.size() == 1 &&  tVecSanZhang.empty() && tVecSiZhang.empty() && tVecDuiZi.empty() && tVecDanZhang.empty() ) //// 长炸弹
		{
			return AddZhaDanLongToNode(tVecZhaDanLong[0].byValue,tVecZhaDanLong[0].iLen,tCardType);
		}
		if( tVecDanZhang.size() >= 5 && tVecDuiZi.empty() && tVecSanZhang.empty() &&  tVecSiZhang.empty() && tVecZhaDanLong.empty() ) /// 顺子
		{
			return AddShunZiToNode(tVecDanZhang,tCardType);
		}
		if( tVecDuiZi.size() >= 3 && tVecDanZhang.empty() && tVecSanZhang.empty() &&  tVecSiZhang.empty() && tVecZhaDanLong.empty() ) //连对
		{
			return AddLianDuiToNode(tVecDuiZi,tCardType);
		}
		if( tVecSanZhang.size() >= 2  &&  tVecSiZhang.empty() && tVecZhaDanLong.empty() )											  //飞机
		{
			if( tVecDuiZi.size() == tVecSanZhang.size() && tVecDanZhang.empty() && m_sSysConfig.bIs3W1_Double )
			{
				return AddFeiJiToNode(tVecSanZhang,tVecDuiZi,eAddCardType_DuiZi,tCardType);
			}
			else if( tVecDanZhang.size() ==  tVecSanZhang.size() && tVecDuiZi.empty() && m_sSysConfig.bIs3W1_One )
			{
				return AddFeiJiToNode(tVecSanZhang,tVecDanZhang,eAddCardType_DanZhang,tCardType);
			}
			else if( (tVecDanZhang.size()+tVecDuiZi.size()*2) == tVecSanZhang.size() && m_sSysConfig.bIs3W1_One )
			{
				int iDuiZiSize = tVecDuiZi.size();
				for( int i=0;i<iDuiZiSize;i++ )
				{
					AddParamByValue(tVecDanZhang,tVecDuiZi[i],2);
				}
				return AddFeiJiToNode(tVecSanZhang,tVecDanZhang,eAddCardType_DanZhang,tCardType);
			}
			else if(  tVecDuiZi.empty() && tVecDanZhang.empty() )
			{
				return AddFeiJiToNode(tVecSanZhang,tVecDanZhang,eAddCardType_No,tCardType);
			}
		}
		if( tVecSanZhang.size() == 2  &&  tVecSiZhang.size() == 1 && tVecZhaDanLong.empty() && tVecDanZhang.empty() && tVecDuiZi.empty() )	//飞机 3 3 3 4 4 4 带 5 5 5 5
		{
			if( m_sSysConfig.bIs3W1_Double )
			{
				tVecDuiZi.push_back(tVecSiZhang.front());
				tVecDuiZi.push_back(tVecSiZhang.front());
				return AddFeiJiToNode(tVecSanZhang,tVecDuiZi,eAddCardType_DuiZi,tCardType);
			}
		}
		return false;
	}
	void CHandsNumberAndWeight::GetCardTypeByLaiZi(CARDATA pSrcCard,int iStartIndex,int iLaiZiLen,std::vector<SCombinationNode> & tCardList)const
	{
		if( iStartIndex < 1 || iStartIndex > 13 )
		{
		    return;
		}
		static std::vector<unsigned char> tCurLaiZiList;
		if( iLaiZiLen == 0 )
		{
			SCombinationNode tCombinationNode;
		    bool ret = GetCardTypeBySrc(pSrcCard,tCombinationNode);
			if( ret == true )
			{
				/// 癞子保存
				tCombinationNode.vecLaiZiCard.insert( tCombinationNode.vecLaiZiCard.end(),tCurLaiZiList.begin(),tCurLaiZiList.end());
				/// 获取主牌全部都是癞子牌
				bool bIsMain =  IsVecContainVec(tCombinationNode.vecLaiZiCard,tCombinationNode.vecMainCard);
				if( bIsMain == true )
				{
					if( tCombinationNode.vecMainCard.size() > 4 )
					{ 
						return;
					}
				    for(int i=0;i<tCombinationNode.vecMainCard.size();i++)
					{
					    for(int j = 0;j<tCombinationNode.vecLaiZiCard.size();j++)
						{
						    if( tCombinationNode.vecMainCard[i] == tCombinationNode.vecLaiZiCard[j] )
							{
							     tCombinationNode.vecMainCard[i] =0;
								 tCombinationNode.vecLaiZiCard[j] =0;
							}
						}
					}
				}
				/// 获取带牌全部都是癞子牌
				bool bIsSub =   IsVecContainVec(tCombinationNode.vecLaiZiCard,tCombinationNode.vecSubCard);
				 if(bIsSub == true && tCombinationNode.vecSubCard.size() >= 4 )
				{
					return;
				}

				if( bIsSub == true && tCombinationNode.vecSubCard.size() == 1 )
				{
					for( int i=0;i < tCombinationNode.vecLaiZiCard.size();i++ )
					{
						if( tCombinationNode.vecLaiZiCard[i] == tCombinationNode.vecSubCard[0] ) 
						{
							tCombinationNode.vecLaiZiCard[i] = 0;
							tCombinationNode.vecSubCard[0] = 0;
							break;
						}
					}
				}
				else if( tCombinationNode.vecSubCard.size() == 4 && bIsSub == false ) /// 带牌
				{
					SVBYTE tSubList;
					AddParamByValue(tSubList,tCombinationNode.vecSubCard[0],2);
					bool ret =  IsVecContainVec(tCombinationNode.vecLaiZiCard,tSubList);
					if( ret == true)
					{
						for( int i=0;i < tCombinationNode.vecLaiZiCard.size();i++ )
						{
							if( tCombinationNode.vecLaiZiCard[i] == tCombinationNode.vecSubCard[0] ) 
							{
								tCombinationNode.vecLaiZiCard[i] = 0;
								tCombinationNode.vecSubCard[0] = 0;
							}
							else if( tCombinationNode.vecLaiZiCard[i] == tCombinationNode.vecSubCard[1])
							{
								tCombinationNode.vecLaiZiCard[i] = 0;
								tCombinationNode.vecSubCard[1] = 0;
							}
						}
					}
					tSubList.clear();
					AddParamByValue(tSubList,tCombinationNode.vecSubCard[2],2);
					ret =  IsVecContainVec(tCurLaiZiList,tSubList);
					if( ret == true)
					{
						for( int i=0;i < tCombinationNode.vecLaiZiCard.size();i++ )
						{
							if( tCombinationNode.vecLaiZiCard[i] == tCombinationNode.vecSubCard[2] ) 
							{
								tCombinationNode.vecLaiZiCard[i] = 0;
								tCombinationNode.vecSubCard[2] = 0;
							}
							else if(tCombinationNode.vecLaiZiCard[i] == tCombinationNode.vecSubCard[3])
							{
								tCombinationNode.vecLaiZiCard[i] = 0;
								tCombinationNode.vecSubCard[3] = 0;
							}
						}
					}
				}
				else if( tCombinationNode.vecSubCard.size() == 2 )
				{
					if( bIsSub == true )
					{
						for( int i=0;i < tCombinationNode.vecLaiZiCard.size();i++ )
						{
							if( tCombinationNode.vecLaiZiCard[i] == tCombinationNode.vecSubCard[0] ) 
							{
								tCombinationNode.vecLaiZiCard[i] = 0;
								tCombinationNode.vecSubCard[0] = 0;
							}
							else if(tCombinationNode.vecLaiZiCard[i] == tCombinationNode.vecSubCard[1])
							{
								tCombinationNode.vecLaiZiCard[i] = 0;
								tCombinationNode.vecSubCard[1] = 0;
							}
						}
					}
					else if( tCombinationNode.vecSubCard[0] != tCombinationNode.vecSubCard[1] )
					{
						SVBYTE tSubList;
						AddParamByValue(tSubList,tCombinationNode.vecSubCard[0],1);
						bool ret =  IsVecContainVec(tCombinationNode.vecLaiZiCard,tSubList);
						if( ret == true)
						{
							for( int i=0;i < tCombinationNode.vecLaiZiCard.size();i++ )
							{
								if( tCombinationNode.vecLaiZiCard[i] == tCombinationNode.vecSubCard[0] ) 
								{
									tCombinationNode.vecLaiZiCard[i] = tCombinationNode.vecSubCard[1];
									break;
								}
							}
							tCombinationNode.vecSubCard[0] = tCombinationNode.vecSubCard[1];
						}
						tSubList.clear();
						AddParamByValue(tSubList,tCombinationNode.vecSubCard[1],1);
						ret =  IsVecContainVec(tCombinationNode.vecLaiZiCard,tSubList);
						if( ret == true)
						{
							for( int i=0;i < tCombinationNode.vecLaiZiCard.size();i++ )
							{
								if( tCombinationNode.vecLaiZiCard[i] == tCombinationNode.vecSubCard[1] ) 
								{
									tCombinationNode.vecLaiZiCard[i] = tCombinationNode.vecSubCard[0];
									break;
								}
							}
							tCombinationNode.vecSubCard[1] = tCombinationNode.vecSubCard[0];
						}    
					}
				}
				/// 过滤相同结果
				auto tIter = IsVectorNodeContainByNode(tCardList,tCombinationNode);
				/// 保存结果
				if( tIter == tCardList.end() )
				{  
					tCardList.push_back(tCombinationNode);
				}
			}
			return;
		}
		//// 癞子装换为具体值
		for(int i = iStartIndex;i <= 13;i++)
		{
			if( pSrcCard[i] >= 4 )
			{
				continue;
			}
			/// 算法优化 前后都没有相连牌 则可以过滤这种情况
			if( i!= 1 && i!=13 &&
				pSrcCard[i] == 0 && pSrcCard[i-1] == 0 && pSrcCard[i+1] == 0)
			{
				continue;
			}
			pSrcCard[i]++;
			tCurLaiZiList.push_back(i);
			if( pSrcCard[i] == 1 && i >1 ) ///顺子需要从前面一个开始
			{
				GetCardTypeByLaiZi(pSrcCard,i-1,iLaiZiLen-1,tCardList);
			}
			else
			{
			    GetCardTypeByLaiZi(pSrcCard,i,iLaiZiLen-1,tCardList);
			}
			tCurLaiZiList.pop_back();
			pSrcCard[i]--;
		}
	}

	void CHandsNumberAndWeight::GetLongZhaDanTypeByLaiZi(const CARDATA pSrcCard,int iLaiZiLen,std::vector<SCombinationNode> & tCardList)
	{
		//// 由于长炸弹不能带牌 所以含有 大小王 都不能组成炸弹
		if(pSrcCard[14] > 0 || pSrcCard[15] > 0)
		{
			return;
		}
		int iType = 0;
		unsigned char tCardValue = 0;
		for(int i=1;i <=13;i++)
		{
		    if(pSrcCard[i] > 0)
			{
			   iType++;
			   tCardValue = i;
			}
		}
		if( iType == 1 && ( pSrcCard[tCardValue] + iLaiZiLen) > 4  )    ////  填充长炸弹
		{
			SCombinationNode tCombinationNode;
			tCombinationNode.eCardType = eCardType_ZhaDan;
			tCombinationNode.eAddCardType = eAddCardType_No;
			AddParamByValue(tCombinationNode.vecMainCard,tCardValue,pSrcCard[tCardValue]);
			for(int i=0;i<iLaiZiLen;i++)
			{
				tCombinationNode.vecMainCard.push_back(tCombinationNode.vecMainCard.front());
				tCombinationNode.vecLaiZiCard.push_back(tCombinationNode.vecMainCard.front());
			}
			tCardList.push_back(tCombinationNode);
		}
	}
}







