/****************************************************************************
Copyright (c) 2014-2017 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2017 ShenZhen Redbird Network Polytron Technologies Inc.
 
http://www.hotniao.com

All of the content of the software, including code, pictures, 
resources, are original. For unauthorized users, the company 
reserves the right to pursue its legal liability.
****************************************************************************/
/******* 算法说明 *****************************************************
参考路径:http://blog.csdn.net/zikehaozhuang24/article/details/50072635
权重定义：
为每一种牌型定义权值的大小：
单张                              1
对子                              2
三带                              3
连牌                              4  (每多一张牌权值+1)
连对                              5（每多一对牌，权值+2）
飞机                              6（每对以飞机，权值在基础上+3）
炸弹                              7（包括对王在内） 

第一步：找牌
定义对应的牌型vector
vector<three> //3条
vector<lianzi> //连子
vector<duizi>//对子
vector<danzhang> //单张
vector<fly> //飞机
首先 找出一副牌中只能组成一种牌型的牌（3条，对子，单张为一种牌型。）
意思就是有一张牌和剩余牌中的任何一张牌没有联系。当然是否和剩余的牌有联系是需要定义一个规则的：是否和剩余的牌能组成连子，连队。
如果这2个条件都不满足那么我们称这张牌和剩余的牌没有任何联系。一般情况下通过这个步骤就能找出几张牌，这样能为以后的拆牌减轻任务量。

第二步：计算每一种牌的手数和权值问题（拆牌中的难点）
分几种情况来讨论,每次只找出剩余牌中的5张连牌，然后拿这5张连牌和剩余牌依次拼接看能否组合成更长的连牌
提出牌之后，剩余的全部牌全部看做为单牌，不管是炸弹，3条，还是对子。
1 在剩余牌中提出QQQ ，J1098766544  首先找出5张单牌，从最小还是最大由你来决定。我们这里就以最小来讨论。
第一次：45678  剩余牌为 4 6 9 10 J
第二次：45678910J  46
最后拆出来的2组牌都没有联系，拆牌结束。
方案为：45678910J       4   6   QQQ
对应的权值和手数：7+1+1+3 =12       3（因为3个可以带一张或者是1对，当有对子和单张张的时候手数要减去1）
以下的几种方案也是同样的步骤，我就不写过程了，直接写最终的方案。
2在剩余牌中提出 QQQ 66 
方案为： QQQ 66   78910j   5   44
对应的权值和手数：  3+2+4+1+2=12            4

第三步：选出最优的一组牌作为最后的拆牌方案
通过以上步骤我们确定出了几种出牌方案，我们最后找出出牌手数最小的方案。
如果手数相同的情况下，找权值最大的一组拆牌方案。
如果2者都相同，就随机选一种就是了。（这种情况很小很小） 
通过比较我们确定了下面的这一种拆牌方案。
方案为：45678910J       4   6   QQQ
对应的权值和手数：7+1+1+3 =12       3（因为3个可以带一张或者是1对，当有对子和单张张的时候手数要减去1）
最后加入我们最先找出的牌，最终的拆牌方案为：
vector<three> 222 ，QQQ
vector<duizi> AA
vector<danzhang>  小鬼 ,4,6
vector<lianzi>  45678910J
*******************************************************************************/
#pragma once
#include "IAIAlgorithm.h"
#include <string.h>

namespace HN_AI
{
	#define  LOG_ALL 0
	typedef unsigned char CARDATA[16];
	typedef std::vector<unsigned char> SVBYTE;

	class CHandsNumberAndWeight :
		public IAIAlgorithm
	{
		struct SShunZiNode
		{
			unsigned char byValue;
			int iLen;
			SShunZiNode()
			{
				byValue =0;
				iLen =0;
			}
		};
		struct SZhaDanLongNode     /// 长炸弹
		{
			unsigned char byValue;
			int iLen;
			SZhaDanLongNode()
			{
				byValue =0;
				iLen =0;
			}
		};
		enum EAddCardMainType
		{
		   eAddCardMainType_UnKnow = 0,
		   eAddCardMainType_SanZhang,
		   eAddCardMainType_FeiJi,
		   eAddCardMainType_SiZhan,
		};
	public:
		CHandsNumberAndWeight(void);
		~CHandsNumberAndWeight(void);
	public:
		///  设置配置文件
		void SetCardRulesConfig(const SSysConfig & tSysConfig);
		SSysConfig GetCardRulesConfig();
		///  出牌算法
		bool GetPutCardList(const unsigned char *pSrcCard,int iSrcCardLen,std::vector<SCombinationNode> & vecCombinationResult);
		bool GetPutCardList(const unsigned char *pSrcCard,int iSrcCardLen,SCombinationResult & sCombinationResult);
		bool GetPutCardList(const unsigned char *pSrcCard,int iSrcCardLen,int iLaiZiNumber,std::vector<SCombinationNode> & vecCombinationResult);
		bool GetPutCardList(const unsigned char *pSrcCard,int iSrcCardLen,int iLaiZiNumber,SCombinationResult & sCombinationResult);

		///  跟牌算法
		bool GetFollowCardList(const unsigned char *pSrcCard,int iSrcCardLen,const SCombinationNode & tLastCard,std::vector<SCombinationNode> & vecFollowCardList);
		bool GetFollowCardList(const unsigned char *pSrcCard,int iSrcCardLen,const unsigned char *pLastCard,int iLastCardLen,std::vector<SCombinationNode> & vecFollowCardList);
		bool GetFollowCardList(const unsigned char *pSrcCard,int iSrcCardLen,const SCombinationNode & tLastCard,int iLaiZiNumber,std::vector<SCombinationNode> & vecFollowCardList);
		
		/// 叫分算法 返回0分则不叫分
		int GetCallScore(const unsigned char *pSrcCard,int iSrcCardLen,int iMinScore,int iMaxScore);
		//////// 辅助函数 ///////////////////////////
		//// 从 tSrcList 筛选出 比tFilterNode 大的结果 保存在 tDstList 中
		bool GetNodeListByFilter(const std::vector<SCombinationNode> & tSrcList,std::vector<SCombinationNode> & tDstList,const SCombinationNode & tFilterNode);
		///  获取牌类型
		bool GetCardType(const unsigned char *pSrcCard,int iSrcCardLen,SCombinationNode & tCardType);
		bool GetCardType(const unsigned char *pSrcCard,int iSrcCardLen,int iLaiZiLen,std::vector<SCombinationNode> & tCardList);

		int GetMainByCombinationNode(unsigned char *pSrcCard,int iSrcCardLen,const SCombinationNode & tCombinationNode );
		int GetSubByCombinationNode(unsigned char *pSrcCard,int iSrcCardLen,const SCombinationNode & tCombinationNode);
		int GetMainAndSubByCombinationNode(unsigned char *pSrcCard,int iSrcCardLen,const SCombinationNode & tCombinationNode);
		/// 比较两Node的大小 -1:不能比较 0:相等 1:tNode1 > tNode2  2：tNode1 < tNode2
		int  CompareNodeFun(const SCombinationNode & tNode1,const SCombinationNode & tNode2)const;
	private:
		//// 牌型获取 ///////////////////////////////////////////////////////////
		bool GetCardTypeBySrc(const CARDATA pSrcCard,SCombinationNode & tCardType)const;
		void GetCardTypeByLaiZi(CARDATA pSrcCard,int iStartIndex,int iLaiZiLen,std::vector<SCombinationNode> & tCardList)const;
		void GetLongZhaDanTypeByLaiZi(const CARDATA pSrcCard,int iLaiZiLen,std::vector<SCombinationNode> & tCardList);
		///  癞子自动出牌算法 //////////////////////////////
		///  在最优组合基础上 添加癞子 使组合更好
		void GetCombinationByLaiZi(int iLaiziNumber,SCombinationResult & tCombinationResult) const;
		///  添加癞子后，重新组合最优结果
		void GetMaxCombinationByLaiZi(SCombinationResult & tCombinationResult)const;
		///  添加一张癞子 单张能否形成顺子
		bool IsMakeShunZi(SCombinationResult & tCombinationResult)const;
		///  添加一张癞子 让顺子更长
		bool IsMakeShunZiMore(SCombinationResult & tCombinationResult)const;
		///  添加一张癞子 让三张形成炸弹
		bool IsMakeZhaDan(SCombinationResult & tCombinationResult)const;
		///  添加一张癞子 让对子形成三张
		bool IsMakeSanZhang(SCombinationResult & tCombinationResult)const;
		///  添加一张癞子 让单张形成对子
		bool IsMakeDuiZi(SCombinationResult & tCombinationResult)const;
		///  添加一张癞子 让炸弹更长
		bool IsMakeZhaDanLonger(SCombinationResult & tCombinationResult)const;
		//// 自动出牌算法 //////////////////////////////////
		///  获取出最优组合
		void GetCombinationList();
		///  保存最大组合牌
		void GetMaxCombination();									       
		///  不带牌 不连牌组合获取
		void GetCombinationResultNoWith(const CARDATA pSrcCard,std::vector<SCombinationNode> & tVecCombinationResult,int & tCurValue,int & tCurHandsNumber)const; 
		void GetCombinationResultNoWith(const CARDATA pSrcCard,std::vector<SCombinationNode> & tVecCombinationResult)const;  
		///  带牌 连牌组合获取
		void GetCombinationResult(const CARDATA pSrcCard,std::vector<SCombinationNode> & tVecCombinationResult,int & tCurValue,int & tCurHandsNumber)const;		 
		////// 跟牌算法 /////////////////////////
		void GetDanZhangListByLastCard(const CARDATA pSrcCard,unsigned char tLastCard,std::vector<SCombinationNode> & vecFollowCardList)const;
		void GetDuiZiListByLastCard(const CARDATA pSrcCard,unsigned char tLastCard,std::vector<SCombinationNode> & vecFollowCardList)const;
		void GetSanZhangListByLastCard(const CARDATA pSrcCard,unsigned char tLastCard,EAddCardType tAddCardType,std::vector<SCombinationNode> & vecFollowCardList) const;
		void GetZhaDanWListByLastCard(const CARDATA pSrcCard,unsigned char tLastCard,EAddCardType tAddCardType,std::vector<SCombinationNode> & vecFollowCardList) const;
		void GetShunZiListByLastCard(const CARDATA pSrcCard,const unsigned char tStart,int iSunZiLen,std::vector<SCombinationNode> & vecFollowCardList) const;
		void GetLianDuiListByLastCard(const CARDATA pSrcCard,const unsigned char tStart,int iDuiZiLen,std::vector<SCombinationNode> & vecFollowCardList) const;
		void GetFeiJiListByLastCard(const CARDATA pSrcCard,const unsigned char tStart,int iFeiJiLen,EAddCardType tAddCardType,std::vector<SCombinationNode> & vecFollowCardList)const;
		void GetFollowCardListNoZhaDan(const CARDATA pSrcCard,const SCombinationNode & tLastCard,std::vector<SCombinationNode> & vecFollowCardList)const;
		/////  癞子跟牌算法 ////////////////////
		//// 获取出非炸弹 
		void GetDuiZiListByLaiZi(CARDATA pSrcCard,unsigned char tLastCard,int iLaiZiNumber,std::vector<SCombinationNode> & vecFollowCardList)const;
		void GetShunZiListByLaiZi(CARDATA pSrcCard,const unsigned char tLastStart,int iLastSunZiLen,int iStartIndex,int iLaiZiNumber,std::vector<SCombinationNode> & vecFollowCardList) const;
		void GetLianDuiListByLaiZi(CARDATA pSrcCard,const unsigned char tLastStart,int iLastDuiZiLen,int iStartIndex,int iLaiZiNumber,std::vector<SCombinationNode> & vecFollowCardList) const;
		void GetAddCardTypeListByLaiZi(CARDATA pSrcCard,const SCombinationNode & tLastCard,int iStartIndex,int iLaiZiNumber,std::vector<SCombinationNode> & vecFollowCardList)const;
		void GetFollowListNoZhaDanByLaiZi(CARDATA pSrcCard,const SCombinationNode & tLastCard,int iLaiZiNumber,std::vector<SCombinationNode> & vecFollowCardList)const;
		//// 如果tLastCard 为炸弹并且是不带牌 获取跟牌需要特殊处理( 0:12软炸 > .... > 5纯 >5软 >四癞 > 王炸 > 硬炸 > 软炸  1:王炸 > 12软炸 > .... > 5纯 >5软 > 四癞 > 硬炸 > 软炸 )
		void GetFollowListZhaDan(const CARDATA pSrcCard,const SCombinationNode & tLastCard,int iLaiZiNumber,std::vector<SCombinationNode> & vecFollowCardList)const;
		////// 公共方法 /////////////////////////
		int ChangeHandPaiData(const unsigned char *pSrcCard,int iSrcCardLen);
		void ClassificationData(const CARDATA pSrcCard,SVBYTE & tVecDanZhang,SVBYTE & tVecDuiZi,SVBYTE & tVecSanZhang,SVBYTE & tVecSiZhang,std::vector<SZhaDanLongNode> & tVecZhaDanLong)const; 
		int GetCardCount()const;

		/// 是否是顺子 iFlag:0 以byCardValue为中心向连边寻找 1：以byCardValue为最小点向上寻找 2：以byCardValue为最大点向下寻找
		bool IsShunZi(const CARDATA pSrcCard,unsigned char byCardValue,int iFlag,int iShunZiLen = 5)const;  /// iShunZiLen 顺子长度 byCardValue 在pSrcCard中能不能形成顺子 pSrcCard 必须是 1~13 对应着 下标 1-13 按着顺序存
		bool IsShunZi(const SVBYTE & tVec,unsigned char byCardValue,int iFlag,int iShunZiLen = 5)const;
		bool IsShunZi(const SVBYTE & tVec)const;													/// 是否全部相连
		
		/// 获取出最优的带牌 tVecDuiZi 对子 tVecDanZhang 单张  iSubCardNumber 需要带多少组 tVecSubCard 带牌结果
		bool GetSubCard(const EAddCardMainType & tAddCardMainType,SVBYTE & tVecDuiZi,SVBYTE & tVecDanZhang,int iSubCardNumber,SVBYTE & tVecSubCard,EAddCardType & eAddCardType)const;			
		/// 获取出最坏的带牌 iNumber:最坏的牌个数 单张:1 对子:2
		unsigned char GetSubCard(const CARDATA pSrcCard,int iNumber)const;  
		/// 连对的获取 会删除已经形成连对牌
		void GetLianDui( SVBYTE & tVecDuiZi,std::vector<SCombinationNode> & tVecCombinationResult,int & tCurValue,int & tCurHandsNumber)const;		
		/// 飞机的获取 会删除已经形成飞机牌
		void GetFeiJi( SVBYTE & tVecSanZhang,std::vector<SCombinationNode> & tVecCombinationResult,int & tCurValue,int & tCurHandsNumber)const;	
		
		/// iFlag:0 升序 1降序 2:癞子数升序
		void ResultSort(std::vector<SCombinationNode> & tVecCombinationResult,int iFlag)const;  
		
		/// A 2 大于 3~13
		bool SpecialComparisonFun(int a,int b)const;
		/// 比较 两个CombinationNode 是否相等 flag:0 完全相等 1:带牌数据不计算
		bool IsEqualByNode(const SCombinationNode & tNode1,const SCombinationNode & tNode2,int flag)const;
		//// tNode1 是否包含 tVecData里面所有数据
		bool IsNodeContainByVector(const SCombinationNode & tNode1,const SVBYTE & tVecData)const;
		//// vecA 是否包含 vecB里面所有数据
		bool IsVecContainVec(const SVBYTE & vecA,const SVBYTE & vecB )const;
		//// tNode1 是否已经存在于 vecNodeList 里面
		std::vector<SCombinationNode>::iterator IsVectorNodeContainByNode(std::vector<SCombinationNode> & vecNodeList,const SCombinationNode & tNode1)const;

		///  删除tVecDelete 中 byValue
		bool DeleteParamByValue(SVBYTE & tVecDelete,unsigned char byValue)const;
		///  tVecAdd 添加iNumber个byValue 
		bool AddParamByValue(SVBYTE & tVecAdd,unsigned char byValue,int iNumber=1)const;
		/// 基本牌型节点添加 
		bool AddShunZiToNode(unsigned char ucStart,int iShunZiLen,SCombinationNode & tNode)const;
		bool AddShunZiToNode(const SVBYTE & tVecDanZhang,SCombinationNode & tNode)const;
		bool AddDanZhangToNode(unsigned char ucValue,SCombinationNode & tNode)const;
		bool AddDuiZiToNode(unsigned char ucValue,SCombinationNode & tNode)const;
		bool AddSanZhangToNode(unsigned char ucValue,unsigned char ucDaiPai,EAddCardType tAddCardType,SCombinationNode & tNode)const;
		bool AddZhaDanToNode(unsigned char ucValue,unsigned char ucDaiPai1,unsigned char ucDaiPai2,EAddCardType tAddCardType,SCombinationNode & tNode)const;
		bool AddZhaDanLongToNode(unsigned char ucValue,int iNumber,SCombinationNode & tNode)const;
		bool AddLianDuiToNode(unsigned char ucStart,int iLianDuiLen,SCombinationNode & tNode)const;
		bool AddLianDuiToNode(const SVBYTE & tVecDuiZi,SCombinationNode & tNode)const;
		bool AddFeiJiToNode(unsigned char ucStart,int iFeiJiLen,const SVBYTE & tVecDaiPai,EAddCardType tAddCardType,SCombinationNode & tNode)const;
		bool AddFeiJiToNode(const SVBYTE & tVecSanZhang,const SVBYTE & tVecDaiPai,EAddCardType tAddCardType,SCombinationNode & tNode)const;
		bool AddAllLaiZiToNode(int iLaiZiLen,SCombinationNode & tNode)const;

		//设置检查类型
		virtual void SetCheckType(int iType);
		virtual void SetFirstFlag(bool bFirst);
		//获取检查类型，返回-1表示检查失败
		virtual int GetCheckType();
		//检查
		void DoCheck();
		//映射后重新比较类型
		bool CompareType(ECardType eCardTypeA, ECardType eCardTypeB) const;
		//是否包含某张牌
		bool IncludeTheValue(SCombinationNode &tNode, unsigned char byValue);
		//添加一个结果
		void AddAResult(SCombinationNode &tNode);
		//节点是否相同
		bool NodeEquale(const SCombinationNode &tNodeA, const SCombinationNode &tNodeB) const;
		//初始化数据
		void Init();
	private:
		/// 配置文件
		SSysConfig m_sSysConfig;
		//// 牌 1~K 1~13 小鬼和大鬼 14 15  0号下标预留
		CARDATA m_byCardData;		 ////  初始数据
		/// 牌 1~K 1~13 小鬼和大鬼 14 15 0号下标预留 拆牌后结果 单张、对子、3张、四张
		CARDATA m_byCombinationData;  ////  拆牌后数据
		std::vector<SShunZiNode> m_vetorShunZi;  /// 保存顺子(顺子只保存开始牌 个数就是顺子长度)
		/// 最优结果保存
		int m_iMaxValue;           /// 最大权值
		int m_iMinHandsNumber;     /// 最少手数
		std::vector<SCombinationNode>  m_vecCombinationResult;        ///组牌结果
		std::vector<SCombinationNode>  m_vecResultNoWith;  ///组牌结果，没有带牌
		
		int	m_iCheckType;									//检查类型
		bool m_bFirstOut;									//首出
		unsigned char m_byMinCard;							//最小手牌
		std::vector<SCombinationNode>  m_vecMinCArdResult;  ///组牌结果，包含最小牌
		SCombinationNode		m_tMaxNode;					//首出时的最优节点
		BYTE	m_byTypeCount[8];							//每个类型的数量，按照这个顺序“炸弹>飞机＞顺子＞连对＞三带一（对）＞对子＞单牌）”

	private:
		//按类型优先级比较两个节点大小
		int CompareNoByType(const SCombinationNode& tA, const SCombinationNode& tB);
		//获取首出该出的牌
		void GetFirstOutCard();
		//能否组成飞机
		void MakeFeiJi(SCombinationNode &tNode);
		//能否组成顺子
		void MakeShunZi(SCombinationNode &tNode);
		//组合连队
		void MakeLianDui(SCombinationNode &tNode);
		//组合3带1
		void Make3Dai1(SCombinationNode &tNode);
		//组合对子
		void MakeDuiZi(SCombinationNode &tNode);
		//组合单张
		void MakeDanZhang(SCombinationNode &tNode);

		//删除首出牌
		void DeleteFirstOut();
		
		//配置带牌
		bool GetSubCardEx(const EAddCardMainType & tAddCardMainType,SVBYTE &tVecSanZhang, SVBYTE & tVecDuiZi,SVBYTE & tVecDanZhang,int iSubCardNumber,SVBYTE & tVecSubCard,EAddCardType & eAddCardType, bool bSpecial = false)const;			
	};
}


