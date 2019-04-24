/****************************************************************************
Copyright (c) 2014-2017 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2017 ShenZhen Redbird Network Polytron Technologies Inc.
 
http://www.hotniao.com

All of the content of the software, including code, pictures, 
resources, are original. For unauthorized users, the company 
reserves the right to pursue its legal liability.
****************************************************************************/
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             #include "StdAfx.h"
#include "DataManage.h"
#include "./util/log/log.h"
#include <string>
#include <algorithm>
#include "./util/log/LogModule.h"
#include "./AIAlgorithm/AIAlgorithmManage.h"

namespace HN
{
	#define  DM_RETURN_FALSE false
//#define  DM_RETURN_FALSE \
//{\
//	if( nullptr != pDesk && nullptr != pDesk->m_pDataManage)\
//	{\
//		errorlog(pDesk->m_pDataManage->m_InitData.uRoomID,pDesk->getRoomDeskFlag(),\
//			"%s-%s:%d",__FILE__,__FUNCTION__,__LINE__);\
//	}\
//	return false;\
//}
	DataManage::DataManage()
	{
		pDesk  = nullptr;
		_userData.clear();
	}
	DataManage::~DataManage()
	{
		pDesk  = nullptr;
		_userData.clear();
	}
	bool DataManage::isNomalUser(byte bSeatNO)const
	{
		auto iter = _userData.find(bSeatNO);
		if (iter != _userData.end())
		{
			if (!iter->second.bWatcher)
			{
				return true;
			}
		}
		return false;
	}

	bool DataManage::isWatcher(byte bSeatNO)
	{
		auto iter = _userData.find(bSeatNO);
		if (iter != _userData.end())
		{
			if (iter->second.bWatcher)
			{
				return true;
			}
		}
		return false;
	}

	bool DataManage::isTrustee(byte bSeatNO)
	{
		if(!isNomalUser(bSeatNO)) return false;
		return _userData[bSeatNO].bTrustee;
	}

	bool DataManage::isRobot(byte bSeatNO)
	{
		if(!isNomalUser(bSeatNO)) return false;
		return _userData[bSeatNO].bRobot;
	}

	byte DataManage::nextUser(byte bSeatNO)
	{
		if (_userData.size() <= 1)
		{
			return bSeatNO;
		}
		byte next = (bSeatNO - 1 + _userData.size())%_userData.size();
		do 
		{
			if (next == bSeatNO) break;
			auto iter = _userData.find(next);
			if(iter != _userData.end())
			{
				if(!iter->second.bWatcher)break;
			}
			next = (next - 1 + _userData.size())%_userData.size();
		} while (true);
		return next;
	}

	byte DataManage::nextActiveUser(byte bSeatNO)
	{
		if (_userData.size() <= 1)
		{
			return bSeatNO;
		}
		byte next = (bSeatNO - 1 + _userData.size())%_userData.size();
		do 
		{
			if (next == bSeatNO) break;
			auto iter = _userData.find(next);
			if(iter != _userData.end())
			{
				if(!iter->second.bCardOver && !iter->second.bWatcher)break;
			}
			next = (next - 1 + _userData.size())%_userData.size();
		} while (true);
		return next;
	}

	bool DataManage::addUser(byte bSeatNO)
	{
		/*if (nullptr == pDesk) DM_RETURN_FALSE;
		if (bSeatNO >= PLAY_COUNT) DM_RETURN_FALSE;
		if (nullptr == pDesk->m_pUserInfo[bSeatNO]) DM_RETURN_FALSE;
		auto iter = _userData.find(bSeatNO);
		if (iter == _userData.end())
		{
			sSJUserInf inf;
			inf.userID = pDesk->m_pUserInfo[bSeatNO]->m_UserData.dwUserID;
			inf.iMoney = pDesk->m_pUserInfo[bSeatNO]->m_UserData.i64Money;
			strcpy_s(inf.sName,	pDesk->m_pUserInfo[bSeatNO]->m_UserData.szName);
			inf.bRobot = pDesk->m_pUserInfo[bSeatNO]->m_UserData.isVirtual>0;
			_userData.insert(pair<UID,sSJUserInf>(bSeatNO,inf));
			
			return true;
		}*/
		return false;
	}

	bool DataManage::eraseUser(byte bSeatNO)
	{
		auto iter = _userData.find(bSeatNO);
		if (iter != _userData.end())
		{
			_userData.erase(iter);
			return true;
		}
		return false;
	}


	void DataManage::init(CServerGameDesk *p)
	{
		pDesk = p;
		_userData.clear();
		_curUser = BYTE_ERR;
		_lastOutUser = BYTE_ERR;
		_lastActionUser = BYTE_ERR;
		_banker = BYTE_ERR;

		_iBackScore = 0;
		_sGameMul.clear();
		_bFirstCaller = BYTE_ERR;

		_backCardArray.Clear();			     //底牌数据
		_vAllHasOut.Clear();				 //所有打出的牌
		_tianLaiZiCard= CardBase::NIL;	     //天癞子牌
		_diLaiZiCard= CardBase::NIL;         //地癞子牌
		AIMANAGEINSTANCE.ClearLaiZi();       //清理癞子
	}

	void DataManage::initOnce()
	{
		if (nullptr == pDesk) return ;
		for (auto iter = _userData.begin();iter != _userData.end();)
		{
			/*if (nullptr == pDesk->m_pUserInfo[iter->first])
			{
			iter = _userData.erase(iter);
			}
			else*/
			{
				/*iter->second.userID = pDesk->m_pUserInfo[iter->first]->m_UserData.dwUserID;
				iter->second.iMoney = pDesk->m_pUserInfo[iter->first]->m_UserData.i64Money;
				iter->second.bRobot = pDesk->m_pUserInfo[iter->first]->m_UserData.isVirtual>0;
				strcpy_s(iter->second.sName,pDesk->m_pUserInfo[iter->first]->m_UserData.szName);*/
				iter->second.cHand.Clear();
				iter->second.vHasOut.clear();

				iter->second.bWatcher  = false;
				iter->second.bOffline  = false;
				iter->second.bTrustee  = false;
				iter->second.bCardOver = false;
			
				iter->second.iRate     = 0;
				iter->second.iScore    = 0;
				iter->second.iBombCount =0;
				iter->second.bRobNTState = BYTE_ERR;            //抢地主情况(255:未操作 0：不叫，1：叫地主，2：不抢，3：抢地主)
				iter->second.bCallScoreState = BYTE_ERR;	       //玩家叫分状态 (255:未操作 0:不叫 1~3：叫分 )
				iter->second.bAddRateState = BYTE_ERR;		   //加倍状态	  (255:未操作 0:不加 1：加倍 )
				iter->second.bIsPassState = false;         //是否不出
				iter->second.bIsMingState = false;		   //是否明牌

				/*if(pDesk->isCreatedRoomBegin())
				{
					iter->second.iTotalScore = 0;
					iter->second.vRecord.clear();
					iter->second.iTotalBombCount = 0;
					iter->second.iMaxWinMoney = 0;
				}*/
				iter++;
			}

		}
		_curUser = BYTE_ERR;
		_lastOutUser = BYTE_ERR;
		_lastActionUser = BYTE_ERR;
		_banker = BYTE_ERR;
		_bFirstCaller = BYTE_ERR;
		_iBackScore = 0;
		_sGameMul.clear();
		_backCardArray.Clear();				 //底牌数据
		_vAllHasOut.Clear();				 //所有打出的牌
		_tianLaiZiCard= CardBase::NIL;	     //天癞子牌
		_diLaiZiCard= CardBase::NIL;         //地癞子牌
		AIMANAGEINSTANCE.ClearLaiZi();       //清理癞子
	}

	bool DataManage::shuffleCard()
	{
		_cardHeap.Clear();
		for (int i = 0;i < CARD_PAIRS;i++)
		{
			_cardHeap.addOnePairCard();
		}
		bool ret = shuffleCard(_cardHeap,1,0);
		return ret;
	}

	bool DataManage::shuffleCard(const CardArrayBase & tCardArrayBase,uint iNumber,uint iFillMaxBombNumber)
	{
		if( tCardArrayBase.Size() != MAX_CARD_COUNT )
		{
		    _cardHeap.Clear();
		    _cardHeap.addOnePairCard();
		}
		else
		{
 		    _cardHeap = tCardArrayBase;
		}
		_cardHeap.randCard(iNumber);
		_cardHeap.fillBomb(iFillMaxBombNumber);
		for (auto iter = _userData.begin();iter != _userData.end();iter++)
		{
			if(!isNomalUser(iter->first))continue;
			vector<CardBase> tmp ;
			tmp.assign(_cardHeap.getArray().begin(),
				_cardHeap.getArray().begin() + ONE_HAND_CARD_COUNT-BACK_CARD_COUNT);
			iter->second.cHand = tmp;
			iter->second.cHand.Sort(_tianLaiZiCard,_diLaiZiCard);
			_cardHeap.erase(tmp);
		}
		_backCardArray = _cardHeap;
		//// 配牌器
		testSetUserCard();
		return true;
	}

	byte DataManage::getLastOutUser()
	{
		return _lastOutUser;
	}

	void  DataManage::setLastOutUser(byte bSeatNO)
	{
		_lastOutUser = bSeatNO;
	}

	byte  DataManage::getLastActionUser()
	{
		return _lastActionUser;
	}
	void  DataManage::setLastActionUser(byte bSeatNO)
	{
		_lastActionUser = bSeatNO;
	}

	byte DataManage::getCurUser()
	{
		return _curUser;
	}

	void DataManage::setCurUser(byte bSeatNO)
	{
		_curUser = bSeatNO;
	}

	//byte  DataManage::getLastTop()
	//{
	//	return _lLastRank.empty()?BYTE_ERR:_lLastRank.front();
	//}

	bool DataManage::setBanker(byte bSeatNO)
	{
		if(!isNomalUser(bSeatNO)) DM_RETURN_FALSE;
		_banker = bSeatNO;
		return true;
	}

	byte  DataManage::getBanker()
	{
		return _banker;
	}

	bool  DataManage::alterUserInfo(byte bSeatNO,sSJUserInf &inf)
	{
		auto iter = _userData.find(bSeatNO);
		if(_userData.end()== iter) return false;
		iter->second = inf;
		return true;
	}

	bool DataManage::getUserInfo(byte bSeatNO,sSJUserInf &inf)
	{
		auto iter = _userData.find(bSeatNO);
		if(_userData.end()== iter) return false;
		inf = iter->second ;
		return true;
	}

	size_t DataManage::UserCount()
	{
		return _userData.size();
	}

	bool DataManage::getUserHandCard(byte bSeatNO,byte *pCard,size_t& sLen)
	{
		auto iter = _userData.find(bSeatNO);
		if(_userData.end()== iter) DM_RETURN_FALSE;
		sLen = iter->second.cHand.reAssign(pCard,ONE_HAND_CARD_COUNT);
		return sLen == ONE_HAND_CARD_COUNT;
	}

	bool DataManage::getAllUserHandCard(byte bSeatNO,byte pCard[][ONE_HAND_CARD_COUNT],uint *pLen)
	{
		if( nullptr == pLen )
		{
		    return false;
		}
		for(int i=0;i<PLAY_COUNT;i++)
		{
			if( isNomalUser(i) )
			{
				size_t sLen = 0;
				getUserHandCard(i,pCard[i],sLen);
			    pLen[i] = sLen;
			}
			auto iter = _userData.find(i);
			if(_userData.end()== iter) DM_RETURN_FALSE;
			if( i != bSeatNO && iter->second.bIsMingState == false)
			{
				memset(pCard[i],0,pLen[i]);
			}
		}
		return true;
	}

	bool DataManage::getAllUserHandCard(byte pCard[][ONE_HAND_CARD_COUNT],uint *pLen)
	{
		if( nullptr == pLen )
		{
			return false;
		}
		for(int i=0;i<PLAY_COUNT;i++)
		{
			if( isNomalUser(i) )
			{
				size_t sLen = 0;
				getUserHandCard(i,pCard[i],sLen);
				pLen[i] = sLen;
			}
		}
		return true;
	}

	bool DataManage::getBackCard(byte *pCard,size_t& sLen)
	{
		sLen = _backCardArray.reAssign(pCard,BACK_CARD_COUNT);
		return sLen == BACK_CARD_COUNT;
	}

	EBackArrayType DataManage::getBackCardType()
	{
		return AIMANAGEINSTANCE.GetBackType(_backCardArray);
	}

	size_t DataManage::getUserHandCardNum(byte bSeatNO)
	{
		auto iter = _userData.find(bSeatNO);
		if(_userData.end()== iter) return 0;
		return iter->second.cHand.Size();
	}

	bool DataManage::playCard(byte bSeatNO,T_C2S_PLAY_CARD_REQ & sPlayCardREQ)
	{
		auto iter = _userData.find(bSeatNO);
		if (iter == _userData.end()) 
		{
			return false;
		}
		/// 删除手牌
		CardArrayBase tCard,tCardDst;
		tCard.assign(sPlayCardREQ.uActualCards,sPlayCardREQ.iCardCount);
		tCardDst.assign(sPlayCardREQ.uCards,sPlayCardREQ.iCardCount);
		if (!iter->second.cHand.erase(tCard)) 
		{
		   return false;
		}
		/// 出牌保存
		iter->second.vHasOut.push_back(sPlayCardREQ);
		_vAllHasOut.add(tCard);
		if (0 == iter->second.cHand.Size())  /// 出牌结束后 保存所有牌 
		{
			iter->second.bCardOver = true;
			for(int j=0;j<PLAY_COUNT;j++)
			{
			    auto tIter = _userData.find(j);
				if ( tIter == _userData.end()) 
				{
					continue;
				}
				_vAllHasOut.add( tIter->second.cHand);
			}
		}
		return true;
	}

	bool DataManage::isHaveWBomb(byte bSeatNO)
	{
		if (!isNomalUser(bSeatNO)) DM_RETURN_FALSE;	
		uval tData[2] = {CardBase::SJ,CardBase::BJ};
		CardArrayBase tCard;
		tCard.assign(tData,2);
		return _userData[bSeatNO].cHand.contain(tCard);
	}

	bool DataManage::checkPlayCard(byte bSeatNO,T_C2S_PLAY_CARD_REQ & sPlayCardREQ)
	{
		if (!isNomalUser(bSeatNO)) DM_RETURN_FALSE;	
		/// 牌类型判断
		bool ret = AIMANAGEINSTANCE.bCheckCardType(sPlayCardREQ);
		if( ret == false || sPlayCardREQ.eArrayType == ARRAY_TYPE_ERROR ) 
		{
			return false;
		}
		/// 跟牌 需要和上次的牌型相对比
		if ( BYTE_ERR != _lastOutUser)
		{
			int ret = AIMANAGEINSTANCE.CompareCard(sPlayCardREQ,_userData[_lastOutUser].vHasOut.back());
			if( ret != 1 )
			{
#if Log_PlayCard
				CTime time = CTime::GetCurrentTime();
				EArrayType tLastType = _userData[_lastOutUser].vHasOut.back().eArrayType;
				LOGMODULEINSTANCE.WriteLog("%02d,%02d,%02d,PlayCard_Error,UserId:%d,Type:%d_%d\n",time.GetHour(),time.GetMinute(),time.GetSecond(),bSeatNO,sPlayCardREQ.eArrayType,tLastType);
#endif
			    return false;
			}
		}
		/// 手牌判断
		CardArrayBase tCard;
		tCard.assign(sPlayCardREQ.uActualCards,sPlayCardREQ.iCardCount);
		if(!_userData[bSeatNO].cHand.contain(tCard))
		{
#if Log_PlayCard
			CTime time = CTime::GetCurrentTime();
			string str,str1;
			unsigned char data[21];
			int iLen = _userData[bSeatNO].cHand.reAssign(data,21);
			LOGMODULEINSTANCE.LogCharToChar(sPlayCardREQ.uActualCards,str,sPlayCardREQ.iCardCount);
			LOGMODULEINSTANCE.LogCharToChar(data,str1,iLen);
			LOGMODULEINSTANCE.WriteLog("%02d,%02d,%02d,PlayCard_Error,UserId:%d,OutSrc:%s\n",time.GetHour(),time.GetMinute(),time.GetSecond(),bSeatNO,str.c_str());
			LOGMODULEINSTANCE.WriteLog("%02d,%02d,%02d,PlayCard_Error,UserId:%d,HandSrc:%s\n",time.GetHour(),time.GetMinute(),time.GetSecond(),bSeatNO,str1.c_str());
#endif
			return false;
		}
		return true;
	}

	bool DataManage::setBackScore(int iBackScore)
	{
		if(iBackScore < 0 )
		{
		    return false;
		}
		_iBackScore = iBackScore;
		return true;
	}

	int DataManage::getBackScore()
	{
		return _iBackScore;
	}

	bool DataManage::setCallScore(byte bSeatNO,int iCallScore)
	{
		auto iter = _userData.find(bSeatNO);
		if (iter == _userData.end()) DM_RETURN_FALSE;
		iter->second.bCallScoreState = iCallScore;
		return true;
	}

	int DataManage::getCallScore(byte bSeatNO)
	{
		auto iter = _userData.find(bSeatNO);
		if (iter == _userData.end()) DM_RETURN_FALSE;
		if( iter->second.bCallScoreState == 255 )
		{
		    return 0;
		}
		return iter->second.bCallScoreState;
	}

	bool DataManage::getAllCallScoreState(byte *pCallScoreState)
	{
		if( nullptr == pCallScoreState )
		{
		    return false;
		}
		for (int i=0;i<PLAY_COUNT;i++)
		{
			auto iter = _userData.find(i);
			if (iter == _userData.end()) DM_RETURN_FALSE;
			pCallScoreState[i]  = iter->second.bCallScoreState;
		}
		return true;
	}

	bool DataManage::setRobNT(byte bSeatNO,uint iRobNTValue)
	{
		auto iter = _userData.find(bSeatNO);
		if (iter == _userData.end()) DM_RETURN_FALSE;
		iter->second.bRobNTState = iRobNTValue;
		return true;
	}

	bool DataManage::getAllRobNTState(byte *pRobNTState)
	{
		if( nullptr == pRobNTState )
		{
			return false;
		}
		for (int i=0;i<PLAY_COUNT;i++)
		{
			auto iter = _userData.find(i);
			if (iter == _userData.end()) DM_RETURN_FALSE;
			pRobNTState[i]  = iter->second.bRobNTState;
		}
		return true;
	}

	bool DataManage::setGameRate(const SGameMultiple & tGameMul)
	{
		_sGameMul = tGameMul;
		return true;
	}

	bool DataManage::addGameRate(int iGameBaseMul,EAddGameMulType eAddGameMulType)
	{
		if( iGameBaseMul < 0 )
		{
		    return false;
		}
		
		switch(eAddGameMulType)
		{
		case eAddGameMulType_BaseMul:
			{
				_sGameMul.iBaseMul *= iGameBaseMul;
			}
			break;
		case eAddGameMulType_MingPaiBegine:
			{
				_sGameMul.iMingPaiBegine *= iGameBaseMul;
			}
			break;
		case eAddGameMulType_MingPaiAfter:
			{
			    _sGameMul.iMingPaiAfter *= iGameBaseMul;
			}
			break;
		case eAddGameMulType_CallNT:
			{
				_sGameMul.iCallNT *= iGameBaseMul;
			}
			break;
		case eAddGameMulType_RobNT:
			{
				_sGameMul.iRobNT *= iGameBaseMul;
			}
			break;
		case eAddGameMulType_Spring:
			{
			    _sGameMul.iSpring *= iGameBaseMul;
			}
			break;
		case eAddGameMulType_HardBomb:
			{
				_sGameMul.iHardBomb *= iGameBaseMul;
			}
			break;
		case eAddGameMulType_SoftBomb:
			{
				_sGameMul.iSoftBomb *= iGameBaseMul;
			}
			break;
		case eAddGameMulType_KingBomb:
			{
			   _sGameMul.iKingBomb *= iGameBaseMul;
			}
			break;
		case eAddGameMulType_FLaiZiBomb:
			{
			   _sGameMul.iFLaiZiBomb *= iGameBaseMul;
			}
			break;
		case eAddGameMulType_LongBomb:
			{
			   _sGameMul.iLongBomb *= iGameBaseMul;
			}
			break;
		case eAddGameMulType_Back_OneKing:
			{
			   _sGameMul.iBack_OneKing *= iGameBaseMul;
			}
			break;
		case eAddGameMulType_Back_ShunZi:
			{
			   _sGameMul.iBack_ShunZi *= iGameBaseMul;
			}
			break;
		case eAddGameMulType_Back_SameColor:
			{
			   _sGameMul.iBack_SameColor *= iGameBaseMul;
			}
			break;
		case eAddGameMulType_Back_TwoKing:
			{
				_sGameMul.iBack_TwoKing *= iGameBaseMul;
			}
			break;
		case eAddGameMulType_Back_SanZhang:
			{
				_sGameMul.iBack_SanZhang *= iGameBaseMul;
			}
			break;
		default:
			break;
		}
		return true;
	}

	int DataManage::getGameRate()
	{
		SGameMultiple  tGameMul;
		return getGameRate(tGameMul);
	}

	int DataManage::getGameRate(SGameMultiple & tGameMul)
	{
		tGameMul = _sGameMul;
		int iAllMul = 1;
		if( _sGameMul.iBaseMul >0 )
		{
		    iAllMul *= _sGameMul.iBaseMul;	
		}
		if( _sGameMul.iMingPaiBegine >0 )
		{
			iAllMul *= _sGameMul.iMingPaiBegine;	
		}
		if( _sGameMul.iMingPaiAfter >0 )
		{
			iAllMul *= _sGameMul.iMingPaiAfter;	
		}
		if( _sGameMul.iCallNT >0 )
		{
			iAllMul *= _sGameMul.iCallNT;	
		}
		if( _sGameMul.iRobNT >0 )
		{
			iAllMul *= _sGameMul.iRobNT;	
		}
		if( _sGameMul.iSpring >0 )
		{
			iAllMul *= _sGameMul.iSpring;	
		}
		if( _sGameMul.iHardBomb >0 )
		{
			iAllMul *= _sGameMul.iHardBomb;	
		}
		if( _sGameMul.iSoftBomb >0 )
		{
			iAllMul *= _sGameMul.iSoftBomb;	
		}
		if( _sGameMul.iKingBomb >0 )
		{
			iAllMul *= _sGameMul.iKingBomb;	
		}
		if( _sGameMul.iFLaiZiBomb >0 )
		{
			iAllMul *= _sGameMul.iFLaiZiBomb;	
		}
		if( _sGameMul.iLongBomb >0 )
		{
			iAllMul *= _sGameMul.iLongBomb;	
		}
		if( _sGameMul.iBack_OneKing >0 )
		{
			iAllMul *= _sGameMul.iBack_OneKing;	
		}
		if( _sGameMul.iBack_ShunZi >0 )
		{
			iAllMul *= _sGameMul.iBack_ShunZi;	
		}
		if( _sGameMul.iBack_SameColor >0 )
		{
			iAllMul *= _sGameMul.iBack_SameColor;	
		}
		if( _sGameMul.iBack_TwoKing >0 )
		{
			iAllMul *= _sGameMul.iBack_TwoKing;	
		}
		if( _sGameMul.iBack_SanZhang >0 )
		{
			iAllMul *= _sGameMul.iBack_SanZhang;	
		}
		return iAllMul;
	}

	bool DataManage::setFirstCaller(byte bSeatNO)
	{
		if(!isNomalUser(bSeatNO)) DM_RETURN_FALSE;
		_bFirstCaller = bSeatNO;
		return true;
	}

	byte DataManage::getFirstCaller()
	{
		return _bFirstCaller;
	}

	bool DataManage::AddBackCard(byte bSeatNO)
	{
		auto iter = _userData.find(bSeatNO);
		if (iter == _userData.end()) DM_RETURN_FALSE;
		iter->second.cHand.add(_backCardArray);
		iter->second.cHand.Sort(_tianLaiZiCard,_diLaiZiCard);
		return true;
	}

	bool DataManage::SortCardByLaiZi()
	{
		for(int i =0;i< PLAY_COUNT;i++)
		{
			auto iter = _userData.find(i);
			if (iter == _userData.end()) DM_RETURN_FALSE;
			iter->second.cHand.Sort(_tianLaiZiCard,_diLaiZiCard);
		}
		return true;
	}

	bool DataManage::AddBombCount(byte bSeatNO,int iBombCount)
	{
		if( iBombCount < 0 )
		{
		    return false;
		}
		auto iter = _userData.find(bSeatNO);
		if (iter == _userData.end()) DM_RETURN_FALSE;
		iter->second.iBombCount += iBombCount;
		return true;
	}

	bool DataManage::setUserRate(byte bSeatNO,int iUserRate)
	{
		if(iUserRate < 0 )
		{
		    return false;
		}
		auto iter = _userData.find(bSeatNO);
		if (iter == _userData.end()) DM_RETURN_FALSE;
		iter->second.iRate = iUserRate;
		iter->second.bAddRateState = iUserRate;
		return true;
	}

	int DataManage::getUserRate(byte bSeatNO)
	{
		auto iter = _userData.find(bSeatNO);
		if (iter == _userData.end()) DM_RETURN_FALSE;
		return iter->second.iRate;
	}

	bool DataManage::getUserAllRate(uint *bUserRate)
	{
		if( nullptr == bUserRate )
		{
		    return false;
		}
		auto iterBanker = _userData.find(_banker);
		if (iterBanker == _userData.end())
		{
			uint iRate = 1;
			for(int i=0;i<PLAY_COUNT;i++)
			{
				auto iter = _userData.find(i);
				if (iter == _userData.end()) DM_RETURN_FALSE;
				bUserRate[i] = (iter->second.iRate+1) * getGameRate();
			}
		}
		else
		{
			for(int i=0;i<PLAY_COUNT;i++)
			{
				auto iter = _userData.find(i);
				if (iter == _userData.end()) DM_RETURN_FALSE;
				if( i == _banker )
				{
					continue;
				}
				else
				{
					bUserRate[i] = (iter->second.iRate+1) * getGameRate() * (iterBanker->second.iRate+1);
					bUserRate[_banker] += bUserRate[i];
				}
			}
		}
		return true;
	}

	bool DataManage::getAllUserRateState(byte *bAddRateState)
	{
		if( nullptr == bAddRateState )
		{
			return false;
		}
		for (int i=0;i<PLAY_COUNT;i++)
		{
			auto iter = _userData.find(i);
			if (iter == _userData.end()) DM_RETURN_FALSE;
			bAddRateState[i]  = iter->second.bAddRateState;
		}
		return true;
	}

	bool DataManage::setPassState(byte bSeatNO)
	{
		auto iter = _userData.find(bSeatNO);
		if (iter == _userData.end()) DM_RETURN_FALSE;
		iter->second.bIsPassState = true;
		return true;
	}
	bool DataManage::clearAllPassState()
	{
		for (int i=0;i<PLAY_COUNT;i++)
		{
			auto iter = _userData.find(i);
			if (iter == _userData.end()) DM_RETURN_FALSE;
			iter->second.bIsPassState = false;
		}
		return true;
	}
	bool DataManage::getAllPassState(bool *pPass)
	{
	    if( nullptr == pPass )
		{
		    return false;
		}
		for (int i=0;i<PLAY_COUNT;i++)
		{
			auto iter = _userData.find(i);
			if (iter == _userData.end()) DM_RETURN_FALSE;
			pPass[i] = iter->second.bIsPassState;
		}
		return true;
	}

	bool DataManage::setMingCardState(byte bSeatNO,bool bIsMing)
	{
		auto iter = _userData.find(bSeatNO);
		if (iter == _userData.end()) DM_RETURN_FALSE;
		iter->second.bIsMingState = bIsMing;
		return true;
	}

	bool DataManage::getAllMingCardState(bool *pMingCard)
	{
		if( nullptr == pMingCard )
		{
			return false;
		}
		for (int i=0;i<PLAY_COUNT;i++)
		{
			auto iter = _userData.find(i);
			if (iter == _userData.end()) DM_RETURN_FALSE;
			pMingCard[i] = iter->second.bIsMingState;
		}
		return true;
	}

	bool DataManage::clearAllMingCardState()
	{
		for (int i=0;i<PLAY_COUNT;i++)
		{
			setMingCardState(i,false);
		}
		return true;
	}

	bool DataManage::SetTianLaiZiCard(byte byCard)
	{
		CardBase tCard(byCard);
		bool ret = tCard.isValid();
		if(ret)
		{
			_tianLaiZiCard = tCard;
			AIMANAGEINSTANCE.SetTianLaiZiCard(_tianLaiZiCard);
			return true;
		}
		return false;
	}

	byte DataManage::GetTianLaiZiCard()
	{
		return _tianLaiZiCard.getData();
	}

	bool DataManage::SetDiLaiZiCard(byte byCard)
	{
		CardBase tCard(byCard);
		bool ret = tCard.isValid();
		if(ret)
		{
			_diLaiZiCard = tCard;
			AIMANAGEINSTANCE.SetDiLaiZiCard(_diLaiZiCard);
			return true;
		}
		return false;
	}

	byte DataManage::GetDiLaiZiCard()
	{
		return _diLaiZiCard.getData();
	}

	bool DataManage::CheckType(CardArrayBase& HandCard, int iType)
	{
		AIMANAGEINSTANCE.SetFirstFlag(false);
		return AIMANAGEINSTANCE.CheckType(HandCard, iType);
	}

	bool DataManage::getPickUpFollowAll(HN::CardArrayBase &HandCard, T_C2S_PLAY_CARD_REQ &tLastCard, std::vector<T_C2S_PLAY_CARD_REQ> &tPlayCardList)
	{
		AIMANAGEINSTANCE.SetFirstFlag(false);

		tPlayCardList.clear();
		AIMANAGEINSTANCE.GetFollowArrayListByHands(HandCard,tLastCard,tPlayCardList);
		/// 结果返回
		if( tPlayCardList.empty() == false )
		{
			return true;
		}
		return false;
	}

	void DataManage::LogCharToChar(const vector<CardBase> & src,string & dst)
	{
		for(auto iter = src.begin();iter != src.end();iter++)
		{
			char tem[10];
			sprintf(tem,"%d ",iter->getVal());
			dst += string(tem);  
		}
	}
	bool DataManage::autoPickUpFirst(byte bSeatNO,T_C2S_PLAY_CARD_REQ & tPlayCardREQ)
	{
		/// 获取出牌组合
        std::vector<T_C2S_PLAY_CARD_REQ> tPlayCardAllList;
		tPlayCardAllList.clear();
		getPickUpOutAll(bSeatNO,tPlayCardAllList,2);
	    //// 出牌组合选择
		if( tPlayCardAllList.empty() == false )
		{
			/// 出牌组合选择
			tPlayCardREQ = tPlayCardAllList.front();
		    return true;
		}
		else
		{
		    return false;
		}
	}
	bool DataManage::autoPickUpFollow(byte bSeatNO,T_C2S_PLAY_CARD_REQ & tFollowCardREQ)
	{
		/// 获取出牌组合
		std::vector<T_C2S_PLAY_CARD_REQ> tFollowCardAllList;
		tFollowCardAllList.clear();
		getPickUpFollowAll(bSeatNO,tFollowCardAllList,2);
		//// 选择出牌组合
		if( tFollowCardAllList.empty() == false )
		{
			/// 出牌组合选择
			tFollowCardREQ = tFollowCardAllList.front();
			return true;
		}
		else
		{
			return false;
		}
	}
	//第一个出牌的时候
	bool DataManage::getPickUpOutAll(HN::CardArrayBase &HandCard, std::vector<T_C2S_PLAY_CARD_REQ> & tPlayCardList)
	{
		AIMANAGEINSTANCE.SetFirstFlag(true);
		//mark//算出手牌可出的入口
		AIMANAGEINSTANCE.GetPutArrayList(HandCard,tPlayCardList);
		/// 结果返回
		if( tPlayCardList.empty() ==  false )
		{
			return true;
		}
		return false;
	}

	bool DataManage::getPickUpOutAll(byte bSeatNO,std::vector<T_C2S_PLAY_CARD_REQ> & tPlayCardList,int iFlag)
	{
		if(!isNomalUser(bSeatNO)) DM_RETURN_FALSE;
		/// 获取手牌
 		CardArrayBase tmp = _userData[bSeatNO].cHand;
		/// 测试数据 ////
		/*unsigned char tData[13] = {6,2,13,13,12,9,8,8,3};
		tmp.assign(tData,9);
		AIMANAGEINSTANCE.SetTianLaiZiCard( CardBase(6) );
		AIMANAGEINSTANCE.SetDiLaiZiCard( CardBase(7) );*/
		/// 获取出手牌最佳组合
		tPlayCardList.clear();
		if(iFlag == 0 || iFlag == 1)
		{
		    AIMANAGEINSTANCE.GetPutArrayList(tmp,tPlayCardList);
		}
		else if(iFlag == 2)
		{
		    AIMANAGEINSTANCE.GetPutArrayListByInc(tmp,tPlayCardList);
		}
		else
		{
		    AIMANAGEINSTANCE.GetPutArrayListByInc(tmp,tPlayCardList);
		}
		/// 结果返回
		if( tPlayCardList.empty() ==  false )
		{
			return true;
		}
		return false;
	}

	bool DataManage::bCheckCanOut(byte bSeatNO)
	{
		if( bSeatNO == _lastOutUser || _lastOutUser == BYTE_ERR )
		{
			return true;
		}
		else
		{
			std::vector<T_C2S_PLAY_CARD_REQ>  tFollowCardList;
			return getPickUpFollowAll(bSeatNO,tFollowCardList);
		}
		return true;
	}

	bool DataManage::getPickUpFollowAll(byte bSeatNO,std::vector<T_C2S_PLAY_CARD_REQ> & tFollowCardList,int iFlag)
	{
		if(!isNomalUser(bSeatNO)) DM_RETURN_FALSE;
		/// 获取手牌
		CardArrayBase tmp = _userData[bSeatNO].cHand;
		T_C2S_PLAY_CARD_REQ tLastCard = _userData[_lastOutUser].vHasOut.back();
		///// 测试数据 //// 
		/*unsigned char tData[21] = {2,2,2,2,4,4,4,4};
		unsigned char tData1[21] = {12,12,12,12,5,5,4,4};
		unsigned char tData2[21] = {12,12,12,12,5,5,4,4};
		tLastCard.iCardCount = 8;
		memcpy(tLastCard.uCards,tData1,tLastCard.iCardCount); 
		memcpy(tLastCard.uActualCards,tData2,tLastCard.iCardCount); 
		tmp.assign(tData,8);
		AIMANAGEINSTANCE.SetTianLaiZiCard(CardBase(3));
		AIMANAGEINSTANCE.SetDiLaiZiCard(CardBase(8));*/
		/// 获取所有能跟牌的组合
		tFollowCardList.clear();
		if( iFlag == 0)
		{
			AIMANAGEINSTANCE.GetFollowArrayList(tmp,tLastCard,tFollowCardList);
			//AIMANAGEINSTANCE.GetFollowArrayListByInc(tmp,tLastCard,tFollowCardList);
		}
		else if(iFlag == 1)
		{
			AIMANAGEINSTANCE.GetFollowArrayListByHands(tmp,tLastCard,tFollowCardList);
			// AIMANAGEINSTANCE.GetFollowArrayList(tmp,tLastCard,tFollowCardList);
		}
		else if(iFlag == 2)
		{
		    AIMANAGEINSTANCE.GetFollowArrayListByInc(tmp,tLastCard,tFollowCardList);
		}
		else
		{
		    AIMANAGEINSTANCE.GetFollowArrayListByInc(tmp,tLastCard,tFollowCardList);
		}
		/// 结果返回
		if( tFollowCardList.empty() == false )
		{
			return true;
		}
		return false;
	}
	//// 验证出牌牌型 是否可以组成多种牌型
	bool DataManage::getCardTypeByPlayCard(byte bSeatNO,T_C2S_PLAY_CARD_REQ & tPlayCard,std::vector<T_C2S_PLAY_CARD_REQ> & tCardTypeList)
	{
		CardArrayBase tCardType;
		tCardType.assign(tPlayCard.uCards,tPlayCard.iCardCount);
		/// 获取出多有组合
		if( _lastOutUser == BYTE_ERR || _lastOutUser == _curUser )  //// 自动出牌
		{
			// AIMANAGEINSTANCE.GetTypeListToPut(tCardType,tCardTypeList);
			AIMANAGEINSTANCE.GetTypeListToPutByMaxMin(tCardType,tCardTypeList);
		}
		else							///  跟牌出牌
		{
			T_C2S_PLAY_CARD_REQ tLastPlay = _userData[_lastOutUser].vHasOut.back();
			/*tPlayCard.uCards[0] = 1;
			tPlayCard.iCardCount = 1;
			_LaiZiCard = 1;
			tCardType.assign(tPlayCard.uCards,tPlayCard.iCardCount);
			tLastPlay.uCards[0] = 13;
			tLastPlay.uActualCards[0] = 13;
			tLastPlay.iCardCount = 1;*/
			T_C2S_PLAY_CARD_REQ tReq;
			bool ret = AIMANAGEINSTANCE.GetMaxTypeToFollow(tCardType,tLastPlay,tReq);
			if( ret == true)
			{
			    tCardTypeList.push_back(tReq);
			}
#if Log_PlayCard
			if( tCardTypeList.size() == 0)
			{
				CTime time = CTime::GetCurrentTime();
				string str;
				string str1;
				LOGMODULEINSTANCE.LogCharToChar(tLastPlay.uCards,str,tLastPlay.iCardCount,2);
				LOGMODULEINSTANCE.LogCharToChar(tLastPlay.uActualCards,str1,tLastPlay.iCardCount,2);
				LOGMODULEINSTANCE.WriteLog("%02d,%02d,%02d,Check_Out,LaiZi=%d,Number:%d,Out:%s_%s\n",time.GetHour(),time.GetMinute(),time.GetSecond(),_tianLaiZiCard.getVal(),tLastPlay.iCardCount,str.c_str(),str1.c_str());
			}
#endif
		}
		if(tCardTypeList.empty() == false )
		{
		    return true;
		}
		return false;
	}
	void DataManage::testSetUserCard()
	{
		CString s = CINIFile::GetAppPath ();/////本地路径    
		CINIFile f(s +_T("\\")+SKIN_FOLDER  + _T("_s.ini"));

		int isOpen = f.GetKeyVal("peipai","OPenPeiPai",0);
		if(isOpen != 1)
		{
			return;
		}
		CardArrayBase CardData0;
		CardArrayBase CardData1;
		CardArrayBase CardData2;
		CardArrayBase CardData3;
		//// 0号玩家
		for(int i=1;i<=17;i++)
		{
			CString str;
			unsigned char ucTemp;
			str.Format("OneHand%02d",i);
			ucTemp = f.GetKeyVal("peipai",str,rand()%16+1);
			CardData0.push_back(CardBase(ucTemp));
		}
		//// 1号玩家
		for(int i=1;i<=17;i++)
		{
			CString str;
			unsigned char ucTemp;
			str.Format("TwoHand%02d",i);
			ucTemp = f.GetKeyVal("peipai",str,rand()%16+1);
			CardData1.push_back(CardBase(ucTemp));
		}
		//// 2号玩家
		for(int i=1;i<=17;i++)
		{
			CString str;
			unsigned char ucTemp;
			str.Format("ThreeHand%02d",i);
			ucTemp = f.GetKeyVal("peipai",str,rand()%16+1);
			CardData2.push_back(CardBase(ucTemp));
		}
		/// 门牌  
		for(int i=1;i <= BACK_CARD_COUNT;i++)
		{
			CString str;
			unsigned char ucTemp;
			str.Format("Backpai%02d",i);
			ucTemp = f.GetKeyVal("peipai",str,rand()%16+1);
			CardData3.push_back(CardBase(ucTemp));
		}
		//// 修改数据
 		for (auto iter = _userData.begin();iter != _userData.end();iter++)
		{
			if(!isNomalUser(iter->first))continue;
			if( iter->first == 0)
			{
				iter->second.cHand = CardData0;
			}
			else if(iter->first == 1)
			{
			   iter->second.cHand = CardData1;
			}
			else
			{
			   iter->second.cHand = CardData2;
			}
			iter->second.cHand.Sort(_tianLaiZiCard,_diLaiZiCard);
		}
		_backCardArray = CardData3;
	}

	CardArrayBase & DataManage::getAllHasOutCard()
	{
		return _vAllHasOut;
	}

	HN_AI::CAIAlgorithmManage & DataManage::AIAlgorithmManageInstance()
	{
		return _AIAlgorithmManage;
	}

	void DataManage::SetCardRulesConfig(const HN_AI::SSysConfig & tSysConfig)
	{
		AIMANAGEINSTANCE.SetCardRulesConfig(tSysConfig);
	}
}

