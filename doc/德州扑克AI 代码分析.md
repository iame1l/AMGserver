## 德州扑克AI 代码分析

```C++
//令牌包，用来激活用户进行活动，如出牌等等
struct TToken
{
    BYTE	  byUser;									// 所属玩家
    BYTE	  byVerbFlag;								// 允许动作标志
    int		  iSelPoints[5];							// 加注值选择表
	emToken	  nemToken;									// 托管状态类型
    bool	  bNewTurn;                                 // 是否为新的一轮开始
	bool	  bNewTurns;                                // 是否为新的一轮开始
	int		  nTurnNums;								// 当前游戏活动的圈数
	__int64   nCallMoney;                               // 当前可以跟注的金币

    TAG_INIT(TToken)
    {
		nemToken = TO_UNKNOWN;						// 未知类型
        byUser = 255;								// 所属玩家
        byVerbFlag = 0;								// 允许动作标志
        bNewTurn = false;
		bNewTurns = false;
		nCallMoney = 0;
		nTurnNums = 0;
        memset(iSelPoints,0,sizeof(iSelPoints));	// 加注值选择表
    }
};
```





##### 机器人的行为和动作,一般会用BTYPE(一个字节去完成)  

```C++

#define UD_VF_CALL					0x01  //可以跟注
#define UD_VF_ADD					0x02  //可以加注
#define UD_VF_BET					0x04  //可以下注
#define UD_VF_FOLD					0x08  //可以弃牌
#define UD_VF_CHECK					0x10  //可以过牌
#define UD_VF_ALLIN					0x20  //可以全下
#define UD_VF_CHECKMANDATE			0x40  //可以过牌托管
#define UD_VF_CALLMANDATE			0x80  //可以跟注托管
#define UD_VF_CALLANYMANDATE		0x100 //可以跟任何注托管
#define UD_VF_PASSABANDONMANDATE	0x200 //可以过牌/弃牌托管

#define UD_VF_BEGIN_GAME			0x11  //开始游戏
#define UD_VF_ALTER_STAKE			0x12  //修改带入额
```



##### 机器人获得玩家手牌的函数?

```C++
///名称：GetCards
///描述：获得某个玩家手上的所有牌
///@param pCards取得的牌值放于此
///@return 
void CUserDataCliObject::GetCards(BYTE* pCards)
{
	::memcpy(pCards,m_byMyCards,m_iCardsNum);
	return;
}
```



20190306,AI的行为概率更新

```C++
bool CClientGameDlg::UserNoteAccordingToActionProb(const BYTE byVerbFlag)
{
	//未选择行为概率策略，返回
	if (!m_bSelectActionProb)
	{
		return false;
	}

	//选择错误的行为概率策略，返回
	if (m_nEnumActionStatus < ENUM_SET_ACTION_STATUS_Big
		|| m_nEnumActionStatus >= ENUM_SET_ACTION_STATUS_Max)
	{
		return false;
	}

	//不是机器人说话，返回
	if (m_byTokenUser != m_byMeStation)
	{
		return false;
	}

	//随机确定策略
	//srand(GetTickCount()+GetMeUserInfo()->bDeskStation);
	struct timeb timeSeed;
	ftime(&timeSeed);
	srand((UINT)(timeSeed.time * NUM_ONE_SECOND_MS + timeSeed.millitm));  // milli time
	BYTE iRandNumTemp = 0;
	iRandNumTemp = rand() % 100;
	BYTE byProbFold = m_tSetActionProb.byProbFold[m_nEnumActionStatus];
	BYTE byProbFollow = byProbFold + m_tSetActionProb.byProbFollow[m_nEnumActionStatus];
	BYTE byProbRaise = byProbFollow + m_tSetActionProb.byProbRaise[m_nEnumActionStatus];
	BYTE byProbAllIn = byProbRaise + m_tSetActionProb.byProbAllIn[m_nEnumActionStatus];
	BYTE bySelectActionTemp = 0;
	if (iRandNumTemp < byProbFold)
	{
		bySelectActionTemp = UD_VF_FOLD;
	} 
	else if (iRandNumTemp < byProbFollow)
	{
		bySelectActionTemp = UD_VF_CALL;
	} 
	else if (iRandNumTemp < byProbRaise)
	{
		bySelectActionTemp = UD_VF_ADD;
	} 
	else if (iRandNumTemp < byProbAllIn)
	{
		bySelectActionTemp = UD_VF_ALLIN;
	}

	const static BYTE bVerbFlagCheckArr[] =
	{
		UD_VF_ALLIN,
		UD_VF_ADD,
		UD_VF_BET,
		UD_VF_CALL,
		UD_VF_CHECK,
	};

	//特殊情况修改
	if (0 == (byVerbFlag & bySelectActionTemp))
	{
		//如果没有可选行为，即寻找同级或下级行为
		bool bSelectGotReplace = false;
		switch(bySelectActionTemp)
		{
		case UD_VF_ALLIN:
			for (int i=0; i < NUM_GET_ARR(bVerbFlagCheckArr); i++)
			{
				if (byVerbFlag & bVerbFlagCheckArr[i])
				{
					bySelectActionTemp = bVerbFlagCheckArr[i];
					bSelectGotReplace = true;
					break;
				}
			}
			break;
		case UD_VF_ADD:
			for (int i=2; i >= 0; i--)
			{
				if (byVerbFlag & bVerbFlagCheckArr[i])
				{
					bySelectActionTemp = bVerbFlagCheckArr[i];
					bSelectGotReplace = true;
					break;
				}
			}

			if (!bSelectGotReplace)
			{
				for (int i=3; i < NUM_GET_ARR(bVerbFlagCheckArr); i++)
				{
					if (byVerbFlag & bVerbFlagCheckArr[i])
					{
						bySelectActionTemp = bVerbFlagCheckArr[i];
						bSelectGotReplace = true;
						break;
					}
				}
			}
			break;
		case UD_VF_CALL:
			for (int i=4; i < NUM_GET_ARR(bVerbFlagCheckArr); i++)
			{
				if (byVerbFlag & bVerbFlagCheckArr[i])
				{
					bySelectActionTemp = bVerbFlagCheckArr[i];
					bSelectGotReplace = true;
					break;
				}
			}

			if (!bSelectGotReplace)
			{
				for (int i=0; i >= 0; i--)
				{
					if (byVerbFlag & bVerbFlagCheckArr[i])
					{
						bySelectActionTemp = bVerbFlagCheckArr[i];
						bSelectGotReplace = true;
						break;
					}
				}
			}

			break;
		default:
			break;
		}

		if (!bSelectGotReplace)
		{
			bySelectActionTemp = UD_VF_FOLD;
		}
	}

	bool bGotAction = false;
	int iRandTime = rand() % 3 + 1;
	switch(byVerbFlag & bySelectActionTemp)
	{
	case UD_VF_BET:		// 允许下注
		{
			SetGameTimer(m_byMeStation,iRandTime,IDT_BET_TIMER);
			bGotAction = true;
			break;
		}
	case UD_VF_CALL:	// 允许跟注
		{
			m_iFollowNum++;			//机器人跟注次数
			SetGameTimer(m_byMeStation,iRandTime,IDT_CALL_TIMER);
			bGotAction = true;
			break;
		}
	case UD_VF_ADD:		// 允许加注
		{
			SetGameTimer(m_byMeStation,iRandTime,IDT_RAISE_TIMER);
			bGotAction = true;
			break;
		}
	case UD_VF_CHECK:  // 允许过牌
		{
			SetGameTimer(m_byMeStation,iRandTime,IDT_CHECK_TIMER);
			bGotAction = true;
			break;
		}
	case UD_VF_FOLD:   // 允许弃牌
		{
			SetGameTimer(m_byMeStation,iRandTime,IDT_FOLO_TIMER);
			bGotAction = true;
			break;
		}
	case UD_VF_ALLIN:  // 允许全下
		{
			SetGameTimer(m_byMeStation,iRandTime,IDT_ALLIN_TIMER);
			bGotAction = true;
			break;
		}
	default:
		{
			break;
		}
	}

	//没有成功执行返回错误
	if (!bGotAction)
	{
		return false;
	}

	return true;
}

```

