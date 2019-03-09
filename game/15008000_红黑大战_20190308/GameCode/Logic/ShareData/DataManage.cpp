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
			if(/*vWinCount[j].deskNO == temp[i] || */temp[i] == 255)
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
	memset(byRunCardShape, 0, sizeof(byRunCardShape));
	byGameBeen = 0;
	byRunHongCount = 0; 
	byRunHeiCount = 0;   
	//byRunHeCount = 0;   
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
	m_bSpecialCardWin = false;
	memset(rAresData,0,sizeof(rAresData));
	memset(i64QuYuZhu,0,sizeof(i64QuYuZhu));
	memset(UserCard, 0, sizeof(UserCard));
	memset(UserShape, 0, sizeof(UserShape));
	WinUserShape = 0;
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
	int temp = 0;
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			UserCard[i][j] = vSendCard[temp].getData();
			temp++;
		}
		sortusercard(UserCard[i]);
		UserShape[i] = GetCardShape(UserCard[i]);
	}

	//牌型大小判断
	//豹子＞顺金＞金花＞顺子＞对子＞单张
	if (UserShape[0] > UserShape[1])
	{
		byWinQuYu = 1;
		WinUserShape = UserShape[0];
	}
	if(UserShape[0] < UserShape[1])
	{
		byWinQuYu = 2;
		WinUserShape = UserShape[1];
	}
	if (UserShape[0] == UserShape[1])     //如果牌型相同，再对比大小花色来决定获胜区域
	{
		WinUserShape = UserShape[0];
		byWinQuYu = getwinuser(UserCard[0], UserCard[1], WinUserShape);
	}
	if (WinUserShape > UG_DAN_ZHANG)
	{
		m_bSpecialCardWin = true;
	}

	return true;
}

//排序
void DataManage::sortusercard(uchar card[])
{
	//一方有三张牌
	//先排大小，如果大小相同再排花色
	uchar temp;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 2; j > i; j--)
		{
			if (getvalue(card[i]) > getvalue(card[j]))
			{
				temp = card[i];
				card[i] = card[j];
				card[j] = temp;
			}
		}
	}

	//最后一张是最大的，所以判断相同与否
	if (getvalue(card[2]) == getvalue(card[1]))
	{
		if (getvalue(card[2]) == getvalue(card[0]))
		{
			//三张相同
			for (int i = 0; i < 3; i++)
			{
				for (int j = 2; j > i; j--)
				{
					if (gethuase(card[i]) > gethuase(card[j]))
					{
						temp = card[i];
						card[i] = card[j];
						card[j] = temp;
					}
				}
			}
		}
		else
		{
			//两张相同
			if (gethuase(card[1]) > gethuase(card[2]))
			{
				temp = card[1];
				card[1] = card[2];
				card[2] = temp;
			}
		}
	}

}

//获取牌型
int DataManage::GetCardShape(uchar card[])
{
	int shape = 0;
	if (baozi(card))	return UG_BAO_ZI;
	if (shunjin(card))return UG_SHUN_JIN;
	if (jinhua(card))return UG_JIN_HUA;
	if (shunzi(card))return UG_SHUN_ZI;
	if (duizi(card))return UG_DUI_ZI;
	if (danzhang(card))return UG_DAN_ZHANG;
	
	return shape;
}

//豹子：三张点数相同的牌
bool DataManage::baozi(uchar card[])
{
	bool flag = false;
	if (getvalue(card[0]) == getvalue(card[1]) && getvalue(card[1]) == getvalue(card[2]))
		return true;
	return flag;
}

//顺金：花色相同的顺子
bool DataManage::shunjin(uchar card[])
{
	bool flag = false;
	if (allhuase(card) && isshunzi(card))
		return true;

	return flag;
}

//金花：花色相同，非顺子
bool DataManage::jinhua(uchar card[])
{
	bool flag = false;
	if (allhuase(card) && !isshunzi(card))
		return true;
	return flag;
}

//顺子：花色不同的顺子
bool DataManage::shunzi(uchar card[])
{
	bool flag = false;
	if (!allhuase(card) && isshunzi(card))
		return true;
	return flag;
}

//对子：两张点数相同的牌
bool DataManage::duizi(uchar card[])
{
	bool flag = false;
	int a = getvalue(card[0]);
	int b = getvalue(card[1]);
	int c = getvalue(card[2]);
	if (a == b && a != c)
		return true;
	if (a == c && a != b)
		return true;
	if (b == c && b != a)
		return true;

	return flag;
}

//单张：以上类型都不是则默认返回单张
bool DataManage::danzhang(uchar card[])
{
	return true;
}

//获取牌值
int DataManage::getvalue(uchar card)
{
	int val = (card & 0x0f) + 1;
	return val;
}

//获取花色
int DataManage::gethuase(uchar card)
{
	//0方块，1梅花，2红桃，3黑桃
	int flag = card>>4;
	return flag;
}

//所有花色是否相同
bool DataManage::allhuase(uchar card[])
{
	bool flag = false;
	if (gethuase(card[0]) == gethuase(card[1]) && gethuase(card[1]) == gethuase(card[2]))
		return true;
	return flag;
}

//是否顺子
bool DataManage::isshunzi(uchar card[])
{
	//除了A是14,其它一一对应牌值，例如2就是2，J是11
	bool flag = false;
	int a = getvalue(card[0]);
	int b = getvalue(card[1]);
	int c = getvalue(card[2]);

	if ((a + 1) == b && (a + 2) == c)
		return true;
	if (a == 2 && b == 3 && c == 14)//A23,因为手牌已提前排好序，所以23A顺序判断
		return true;

	return flag;
}

//获取赢家1为红2为黑
int DataManage::getwinuser(uchar hongcard[], uchar heicard[], BYTE shape)
{
	int temp = 1;
	switch (shape)
	{
	case UG_BAO_ZI:
	{
		return getbaoziuser(hongcard, heicard);
	}
	case UG_SHUN_JIN:
	{
		return getshunjinuser(hongcard, heicard);
	}
	case UG_JIN_HUA:
	{
		return getjinhuauser(hongcard, heicard);
	}
	case UG_SHUN_ZI:
	{
		return getshunziuser(hongcard, heicard);
	}
	case UG_DUI_ZI:
	{
		return getduiziuser(hongcard, heicard);
	}
	case UG_DAN_ZHANG:
	{
		return getdanzhanguser(hongcard, heicard);
	}
	default:
		break;
	}
	return temp;
}

//获取豹子中最大的
int DataManage::getbaoziuser(uchar hongcard[], uchar heicard[])
{
	int user = 1;
	//因为牌是已经排好序的，所以判断最大张时取最后一张就行
	if (getvalue(hongcard[2]) > getvalue(heicard[2]))
	{
		return 1;
	}
	else
	{
		return 2;
	}

	return user;
}

//顺金最大者
int DataManage::getshunjinuser(uchar hongcard[], uchar heicard[])
{
	return maxshunziwinuser(hongcard, heicard);
}

//金花最大者
int DataManage::getjinhuauser(uchar hongcard[], uchar heicard[])
{
	int user = 1;
	int a = getvalue(hongcard[2]);
	int b = getvalue(heicard[2]);
	if (a > b)
	{
		return 1;
	}
	if (a < b)
	{
		return 2;
	}
	//a==b
	if (getvalue(hongcard[1]) > getvalue(heicard[1]))
	{
		return 1;
	}
	if (getvalue(hongcard[1]) < getvalue(heicard[1]))
	{
		return 2;
	}
	//对比第三张 
	if (getvalue(hongcard[0]) > getvalue(heicard[0]))
	{
		return 1;
	}
	if (getvalue(hongcard[0]) < getvalue(heicard[0]))
	{
		return 2;
	}
	//都一样点数则比花色
	if (gethuase(hongcard[2]) > gethuase(heicard[2]))
	{
		return 1;
	}
	else
	{
		return 2;
	}

	return user;
}

//顺子最大者
int DataManage::getshunziuser(uchar hongcard[], uchar heicard[])
{
	return maxshunziwinuser(hongcard, heicard);
}

//对子最大者
int DataManage::getduiziuser(uchar hongcard[], uchar heicard[])
{
	int user = 1;
	//先判断最大的对子，相同则比另外一张
	//因为已经排序了，所以如果是对子的话，则取中间进行判断就行
	int a = getvalue(hongcard[1]);
	int b = getvalue(heicard[1]);
	if (a > b)
	{
		return 1;
	}
	if (a < b)
	{
		return 2;
	}
	//a==b,比单张
	int c = 0;
	int d = 0;
	if (getvalue(hongcard[0]) != a)
	{
		c = getvalue(hongcard[0]);
	}
	else
	{
		c = getvalue(hongcard[3]);
	}

	if (getvalue(heicard[0]) != a)
	{
		d = getvalue(heicard[0]);
	}
	else
	{
		d = getvalue(heicard[3]);
	}

	if (c > d)
	{
		return 1;
	}
	if (c < d)
	{
		return 2;
	}

	//单张还是相同的话，比对子花色
	if (gethuase(hongcard[1]) > gethuase(heicard[1]))
	{
		return 1;
	}
	else
	{
		return 2;
	}

	return user;
}

//单张最大者
int DataManage::getdanzhanguser(uchar hongcard[], uchar heicard[])
{
	int user = 1;
	int a = getvalue(hongcard[2]);
	int b = getvalue(heicard[2]);
	if (a > b)
	{
		return 1;
	}
	if (a < b)
	{
		return 2;
	}
	//a==b比第二张第三张
	if (getvalue(hongcard[1]) > getvalue(heicard[1]))
	{
		return 1;
	}
	if (getvalue(hongcard[1]) < getvalue(heicard[1]))
	{
		return 2;
	}
	//比第三张
	if (getvalue(hongcard[0]) > getvalue(heicard[0]))
	{
		return 1;
	}
	if (getvalue(hongcard[0]) < getvalue(heicard[0]))
	{
		return 2;
	}
	//比花色
	if (gethuase(hongcard[2]) > gethuase(heicard[2]))
	{
		return 1;
	}
	else
	{
		return 2;
	}

	return user;
}

//获取最大的顺子玩家
int DataManage::maxshunziwinuser(uchar hongcard[], uchar heicard[])
{
	int user = 1;
	int a = getvalue(hongcard[2]);//1
	int b = getvalue(heicard[2]);//2
	//先判断大小后花色
	//特殊牌型23A
	//14代表A
	if (a > b)
	{
		if (a == 14 && getvalue(hongcard[1]) == 3)//23A
		{
			return 2;
		}
		return 1;
	}

	if (a < b)
	{
		if (b == 14 && getvalue(heicard[1]) == 3)//23A
		{
			return 1;
		}
		return 2;
	}

	if (a == b)
	{
		if (getvalue(hongcard[1]) == getvalue(heicard[1]))
		{
			//点数相同开始判断花色大小
			if (gethuase(hongcard[2]) > gethuase(heicard[2]))
			{
				return 1;
			}
			else
			{
				return 2;
			}
		}
		if (getvalue(hongcard[1]) > getvalue(heicard[1]))
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}

	return user;
}

//概率控制牌型赢
void DataManage::gailvcontrol(bool flag, int user)
{
	for (int i = 0; i < 50; i++)			//暂定只循环五十次获取所需的牌型就可以，如果50次都不符合，无话可说，归结于人品问题
	{
		shuffleCard();
		if (flag)
		{
			if (WinUserShape == UG_DAN_ZHANG && user == byWinQuYu)
				break;
		}
		else
		{
			if (WinUserShape > UG_DAN_ZHANG)
				break;
		}
	}

}

//推送牌型倍率
int DataManage::getbeilv(int shape)
{
	int flag = 1;
	switch (shape)
	{
	case UG_BAO_ZI:
		return Config::baozibeilv;
	case UG_SHUN_JIN:
		return Config::shunjinbeilv;
	case UG_JIN_HUA:
		return Config::jinhuabeilv;
	case UG_SHUN_ZI:
		return Config::shunzibeilv;
	case UG_DUI_ZI:
		return Config::duizibeilv;
	default:
		return 1;
	}
	return flag;
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



