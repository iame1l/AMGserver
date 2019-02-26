#include "StdAfx.h"
#include "DataManage.h"
#include "../../Server/ServerManage.h"
#include "Config.h"
#include "CardDefine.h"
#include "algorithm.h"
#include <string>
#include <algorithm>
#include "ExtensionLogger.h"
DataManage::DataManage()
{
	_pDesk  = nullptr;
	_userData.clear();
}
DataManage::~DataManage()
{
	_pDesk  = nullptr;
	_userData.clear();
}
bool DataManage::isNomalUser(uchar bSeatNO)
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

bool DataManage::isWatcher(uchar bSeatNO)
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


uchar DataManage::nextUser(uchar bSeatNO)
{
	if (_userData.size() <= 1)
	{
		return bSeatNO;
	}
	uchar next = (bSeatNO - 1 + _userData.size())%_userData.size();
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

uchar DataManage::prevUser(uchar bSeatNO)
{
	if (_userData.size() <= 1)
	{
		return bSeatNO;
	}
	uchar next = (bSeatNO + 1 )%_userData.size();
	do 
	{
		if (next == bSeatNO) break;
		auto iter = _userData.find(next);
		if(iter != _userData.end())
		{
			if(!iter->second.bWatcher)break;
		}
		next = (next + 1 )%_userData.size();
	} while (true);
	return next;
}

uchar DataManage::nextActiveUser(uchar bSeatNO)
{
	if (_userData.size() <= 1)
	{
		return bSeatNO;
	}
	uchar next = (bSeatNO - 1 + _userData.size())%_userData.size();
	do 
	{
		if (next == bSeatNO) break;
		auto iter = _userData.find(next);
		if(iter != _userData.end())
		{
			if(!iter->second.bWatcher && !iter->second.bCardOver)break;
		}
		next = (next - 1 + _userData.size())%_userData.size();
	} while (true);
	return next;
}

bool DataManage::addUser(uchar bSeatNO)
{
	if (nullptr == _pDesk) return false;
	if (bSeatNO >= PLAY_COUNT) return false;
	if (nullptr == _pDesk->m_pUserInfo[bSeatNO]) return false;
	auto iter = _userData.find(bSeatNO);
	sGameUserInf inf;
	inf.userID = _pDesk->m_pUserInfo[bSeatNO]->m_UserData.dwUserID;
	inf.iMoney = _pDesk->m_pUserInfo[bSeatNO]->m_UserData.i64Money;
	inf.bIsVirtual = _pDesk->m_pUserInfo[bSeatNO]->m_UserData.isVirtual;
	strcpy_s(inf.sName, _pDesk->m_pUserInfo[bSeatNO]->m_UserData.szName);
	strcpy_s(inf.sNickName, _pDesk->m_pUserInfo[bSeatNO]->m_UserData.nickName);

	if (iter == _userData.end())
	{
		_userData.insert(pair<UID, sGameUserInf>(bSeatNO, inf));
	}
	else
	{
		alterUserInfo(bSeatNO, inf);
	}
	notiDataMoney();
	return true;
}


void DataManage::updateDataMoney()
{
	for (auto iter = _userData.begin();iter != _userData.end();)
	{
		if (nullptr == _pDesk->m_pUserInfo[iter->first])
		{
			iter = _userData.erase(iter);
		}
		else
		{
			dataMoney.push(MoneyManage(iter->first,iter->second.iMoney));
			vWinCount.push_back(WinGreater(iter->first,iter->second.iRecordWinCount));
			if (dataMoney.size() > USER_MAX_MONEY_NUM)
			{
				dataMoney.pop();
			}
			iter++;
		}
	}
}
void DataManage::ProcessData(BYTE userMaxMoney[])
{
	if (vWinCount.size()>=3)
	{
		partial_sort(vWinCount.begin(), vWinCount.begin() + 3, vWinCount.end(),[](WinGreater &a,WinGreater&b){ return a.count>b.count; });
	}
	else if(vWinCount.size() ==2 )
	{
		if (vWinCount[1].count > vWinCount[0].count)
		{
			std::swap(vWinCount[0],vWinCount[1]);
		}
	}
	int j=0;
	for(int i=1;i < USER_MAX_MONEY_NUM && j < vWinCount.size();i+=2)
	{
		userMaxMoney[i]=vWinCount[j++].deskNO;
	}
	BYTE temp[USER_MAX_MONEY_NUM];
	memset(temp, 255, sizeof(temp));
	for (int i=dataMoney.size()-1;i>=0;i--)
	{
		if (!dataMoney.empty())
		{
			temp[i] = dataMoney.top().deskNO;
			dataMoney.pop();
		}
	}
	int k=0;
	for(int i=0;i < USER_MAX_MONEY_NUM ;++i)
	{
		bool flag=true;
		for (int j=0;j < vWinCount.size() && j < 3;++j)
		{
			if(/*vWinCount[j].deskNO == temp[i] ||*/ temp[i] == 255)
			{
				flag=false;
				break;
			}
		}
		if(flag)
		{
			userMaxMoney[k] = temp[i];
			k+=2;
			if (k >= USER_MAX_MONEY_NUM)
			{
				break;
			}
		}
	}
	vWinCount.clear();
}
void DataManage::notiDataMoney()
{
	if (_pDesk->getGameStation() != GS_WAIT_SETGAME)
	{
		S_C_UserMaxMoney Noti;
		memset(Noti.userMaxMoney,255,sizeof(Noti.userMaxMoney));
		updateDataMoney();
		ProcessData(Noti.userMaxMoney);
		for (auto iter = _userData.begin();iter != _userData.end();iter++)
		{
			_pDesk->send2(iter->first,(char*)&Noti,sizeof(Noti),S_C_USER_MAX_MONEY);
		}
	}
}


bool DataManage:: addRecoveryUser(uint userID)
{
// 	auto itr_bak = usrInfobak.find(userID);
// 	if(itr_bak == usrInfobak.end()) return false;
// 	sGameUserInf inf;
// 	uchar bSeatNO = itr_bak->second.bSeatNO;
// 	inf.iTotalScore = itr_bak->second.iTotalScore;
// 	inf.iRunCnt = itr_bak->second.iRunCnt;
// 	uchar bBanker = BYTE_ERR;
// 
// 	for (int i = 0;i < sizeof(itr_bak->second.vRecord)/sizeof(itr_bak->second.vRecord[0]);i++)
// 	{
// 		if(E_RESULT_NIL ==  itr_bak->second.vRecord[i])break;
// 		inf.vRecord.push_back(itr_bak->second.vRecord[i]);
// 	}
// 
// 	auto iter = _userData.find(itr_bak->second.bSeatNO);
// 
// 	if (iter == _userData.end())
// 	{
// 		_userData.insert(pair<UID, sGameUserInf>(bSeatNO, inf));
// 	}
// 	else
// 	{
// 		alterUserInfo(bSeatNO, inf);
// 	}

	return true;
}

bool DataManage::eraseUser(uchar bSeatNO)
{
	auto iter = _userData.find(bSeatNO);
	if (iter != _userData.end())
	{
		_userData.erase(iter);
		notiDataMoney();
		return true;
	}
	return false;
}


void DataManage::init(CServerGameDesk *p)
{
	_pDesk = p;
	_userData.clear();
	_curUser = BYTE_ERR;
	_lastActionUser = BYTE_ERR;
	_lastBetUser = BYTE_ERR;
	_banker = BYTE_ERR;
	memset(byRunSeq,0,sizeof(byRunSeq));
	byGameBeen = 0;
	byRunLongCount = 0; 
	byRunHuCount = 0;   
	byRunHeCount = 0;   
	_FriendShow = false;
}


void DataManage::updateUserMoney()
{
	if (nullptr == _pDesk) return ;
	for (auto iter = _userData.begin();iter != _userData.end();)
	{
		if (nullptr == _pDesk->m_pUserInfo[iter->first])
		{
			iter = _userData.erase(iter);
		}
		else
		{
			iter->second.iMoney = _pDesk->m_pUserInfo[iter->first]->m_UserData.i64Money;
			iter++;
		}
	}
}


void DataManage::initOnce()
{
	if (nullptr == _pDesk) return ;
	for (auto iter = _userData.begin();iter != _userData.end();)
	{
		if (nullptr == _pDesk->m_pUserInfo[iter->first])
		{
			iter = _userData.erase(iter);
		}
		else
		{
			iter->second.userID = _pDesk->m_pUserInfo[iter->first]->m_UserData.dwUserID;
			iter->second.iMoney = _pDesk->m_pUserInfo[iter->first]->m_UserData.i64Money;
			iter->second.bIsVirtual = _pDesk->m_pUserInfo[iter->first]->m_UserData.isVirtual;
			strcpy_s(iter->second.sName,_pDesk->m_pUserInfo[iter->first]->m_UserData.szName);
			strcpy_s(iter->second.sNickName,_pDesk->m_pUserInfo[iter->first]->m_UserData.nickName);
			iter->second.bWatcher  = false;
			iter->second.bOffline  = false;
			iter->second.ePlayCardState = E_PLAYCARD_NIL;
			iter->second.iScore    = 0;
			iter->second.cHandCards.Clear();
			iter->second.iBonusScore = 0;
			iter->second.iFineScore = 0;
			iter->second.iNomalScore = 0;
			iter->second.cScoreCards.Clear();
			iter->second.bCardOver = false;
			iter->second.isTeamA = false;
			iter->second.iXiaZhuMoney = 0;
			memset(iter->second.i64UserXiaZhuData,0,sizeof(iter->second.i64UserXiaZhuData));
			if(_pDesk->isCreatedRoomBegin())
			{
				iter->second.iTotalScore = 0;
				iter->second.vRecord.clear();	
				iter->second.iRunCnt = 0;
			}
			iter++;
		}

	}
	_curUser = BYTE_ERR;
	_lastBetUser = BYTE_ERR;
	_lastActionUser = BYTE_ERR;
	_lastOutUser = BYTE_ERR;
	iRoomFen = 1;
	_bFriendCard = BYTE_ERR;
	_FriendShow = false;
	m_isSupSet =false;
	m_haveTimeSs = 0;
	byWinQuYu = 0;
	memset(rAresData,0,sizeof(rAresData));
	memset(i64QuYuZhu,0,sizeof(i64QuYuZhu));
	_Rank.clear();
	vSendCard.clear();
}


void myErase(vector<CardBase> &src,vector<CardBase> &v)
{
	for (auto itb = v.begin();
		itb != v.end();itb++)
	{
		auto ita = find(src.begin(),src.end(),*itb);
		if (ita == src.end()) continue;
		else
		{
			src.erase(ita);
		}
	}
}

bool DataManage::shuffleCard()
{
	_cardHeap.Clear();
	_cardHeap.addOnePairCard();
	_cardHeap.randCard();
	vSendCard.assign(_cardHeap.getArray().begin(),
		_cardHeap.getArray().begin() + ONE_HAND_CARD_COUNT);

	if (vSendCard[0].getRealVal() == vSendCard[1].getRealVal())
	{
		byWinQuYu = 3;
	}
	else if(vSendCard[0].getRealVal()==14)
	{
		byWinQuYu = 2;
	}
	else if(vSendCard[1].getRealVal()==14)
	{
		byWinQuYu = 1;
	}
	else if(vSendCard[0].getRealVal() > vSendCard[1].getRealVal())
	{
		byWinQuYu = 1;
	}
	else if(vSendCard[0].getRealVal() < vSendCard[1].getRealVal())
	{
		byWinQuYu = 2;
	}
	return true;
}

vector<CardBase> SplitCString(CString strSource, CString ch)
{
	vector <CardBase> vecString;
	int iPos = 0;
	CString strTmp;
	strTmp = strSource.Tokenize(ch, iPos);
	while (strTmp.Trim() != _T(""))
	{
		CardBase card;
		card = _ttoi(strTmp);
		vecString.push_back(card);
		strTmp = strSource.Tokenize(ch, iPos);
	}
	return vecString;
}

//读取机器人输赢控制
void DataManage::GetAIContrlSetFromIni()
{
	CString s = CINIFile::GetAppPath ();/////本地路径
	CINIFile f(s +SKIN_FOLDER  + _T("_s.ini"));
	CString szSec = TEXT("game");
	///机器人输赢自动控制
	m_bAIWinAndLostAutoCtrl = f.GetKeyVal(szSec,"AIWinAndLostAutoCtrl",1);		///是否开启机器人输赢自动控制
	m_i64AIWantWinMoneyA1	= f.GetKeyVal(szSec,"AIWantWinMoneyA1 ",100000);		/**<机器人赢钱区域1  */
	m_i64AIWantWinMoneyA2	= f.GetKeyVal(szSec,"AIWantWinMoneyA2 ",1000000);		/**<机器人赢钱区域2  */
	m_i64AIWantWinMoneyA3	= f.GetKeyVal(szSec,"AIWantWinMoneyA3 ",10000000);		/**<机器人赢钱区域3  */
	m_iAIWinLuckyAtA1	= f.GetKeyVal(szSec,"AIWinLuckyAtA1 ",68);				/**<机器人在区域1赢钱的概率  */
	m_iAIWinLuckyAtA2	= f.GetKeyVal(szSec,"AIWinLuckyAtA2 ",60);				/**<机器人输赢控制：机器人在区域2赢钱的概率  */
	m_iAIWinLuckyAtA3	= f.GetKeyVal(szSec,"AIWinLuckyAtA3 ",50);				/**<机器人输赢控制：机器人在区域3赢钱的概率  */
	m_iAIWinLuckyAtA4	= f.GetKeyVal(szSec,"AIWinLuckyAtA4 ",30);				/**<机器人输赢控制：机器人在区域4赢钱的概率  */
	m_i64AIHaveWinMoney	= f.GetKeyVal(szSec,"ReSetAIHaveWinMoney ",0);			/**<机器人输赢控制：直接从配置文件中读取机器人已经赢钱的数目  */
}

//奖池记录
void DataManage::RecordAiWinMoney()
{
	//统计真人的输赢
	__int64 i64Money=0;
	DataManage::sGameUserInf userinf;
	for(int i=0; i<PLAY_COUNT; i++)
	{
		if(!getUserInfo(i,userinf))continue;
		if (userinf.bIsVirtual) continue;
		i64Money += userinf.iScore;
	}

	CString s = CINIFile::GetAppPath ();/////本地路径
	CINIFile f(s +SKIN_FOLDER  + _T("_s.ini"));
	CString szSec = TEXT("game");

	//将真人的赢钱数减掉
	m_i64AIHaveWinMoney -= i64Money;
	
	CString sTemp="";
	sTemp.Format("%I64d",m_i64AIHaveWinMoney);
	f.SetKeyValString(szSec,"ReSetAIHaveWinMoney ",sTemp);
}


void DataManage::LoadConfCardUser()
{
	CString s = CINIFile::GetAppPath();/////本地路径
	CINIFile f(s + "config.ini");

	CString key;
	key = TEXT("default");
	if(0 == f.GetKeyVal(key, "enable", 0)) return;

	map<uchar, vector<CardBase>> mUserConfCard;
	for (auto itr = _userData.begin();itr !=_userData.end();itr++)
	{
		char nameBuf[256] = "Cardlist";
		CString cardListStr = f.GetKeyVal(key, nameBuf, "");
		if (cardListStr == "")continue;
		vector<CardBase> vCards = SplitCString(cardListStr, ",");
		itr->second.cHandCards = vCards;
		//LOGGER_FILE(m_Context,""<<itr->second.userID<<" :"<<itr->second.cHandCards);
	}

	char nameBuf[256] = "Cardlist";
	CString cardListStr = f.GetKeyVal(key, nameBuf, "");
	if (cardListStr != "")
	{
		vSendCard = SplitCString(cardListStr, ",");
		if (vSendCard.size()<1)
		{
			return ;
		}
		if (vSendCard[0].getRealVal() == vSendCard[1].getRealVal())
		{
			byWinQuYu = 3;
		}
		else if(vSendCard[0].getRealVal()==14)
		{
			byWinQuYu = 2;
		}
		else if(vSendCard[1].getRealVal()==14)
		{
			byWinQuYu = 1;
		}
		else if(vSendCard[0].getRealVal() > vSendCard[1].getRealVal())
		{
			byWinQuYu = 1;
		}
		else if(vSendCard[0].getRealVal() < vSendCard[1].getRealVal())
		{
			byWinQuYu = 2;
		}
	}
}

uchar DataManage::getLastOutUser()
{
	return _lastOutUser;
}

void  DataManage::setLastOutUser(uchar bSeatNO)
{
	_lastOutUser = bSeatNO;
}

uchar  DataManage::getLastActionUser()
{
	return _lastActionUser;
}
void  DataManage::setLastActionUser(uchar bSeatNO)
{
	_lastActionUser = bSeatNO;
}

uchar  DataManage::getLastBetUser()
{
	return _lastBetUser;
}
void  DataManage::setLastBetUser(uchar bSeatNO)
{
	_lastBetUser = bSeatNO;
}

uchar DataManage::getCurUser()
{
	return _curUser;
}

void DataManage::setCurUser(uchar bSeatNO)
{
	_curUser = bSeatNO;
}

bool  DataManage::alterUserInfo(uchar bSeatNO,sGameUserInf &inf)
{
	auto iter = _userData.find(bSeatNO);
	if(_userData.end()== iter) return false;
	iter->second = inf;
	return true;
}

bool DataManage::getUserInfo(uchar bSeatNO,sGameUserInf &inf)
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


uchar DataManage::getMasterDeskNO()const
{
	uchar master = BYTE_ERR;
	auto itr = find_if(_userData.begin(),_userData.end(),[this](pair<uchar,DataManage::sGameUserInf> user)
	{
		return user.second.userID == _pDesk->m_iDeskMaster;
	});
	if(itr != _userData.end())
	{
		master = itr->first;
	}
	return master;
}

bool DataManage::setBanker(byte bSeatNO)
{
	if(!isNomalUser(bSeatNO)) 
	{
		LOGGER_ERROR_FILE(m_Context,""<<(int)bSeatNO);
		return false;
	}
	_banker = bSeatNO;
	return true;
}

byte  DataManage::getBanker()
{
	return _banker;
}


vector<uchar> DataManage::getRank()
{
	return _Rank;
}




bool DataManage::getUserHandCard(uchar bSeatNO,uchar *pCard,size_t& sLen)
{
	auto iter = _userData.find(bSeatNO);
	if(_userData.end()== iter)
	{
		LOGGER_ERROR_FILE(m_Context,"");
		return false;
	}
	sLen = iter->second.cHandCards.reAssign(pCard,ONE_HAND_CARD_COUNT);
	return sLen == ONE_HAND_CARD_COUNT;
}



E_RESULT_TYPE DataManage::getResultType()const
{
	return _resultType;
}

void DataManage::setResultType(E_RESULT_TYPE e)
{
	_resultType = e;
}



