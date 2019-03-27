## 德州扑克配置和AI胜率控制

```C++
void CServerGameDesk::AiWinAutoCtrl()
{
	bool bAIWin = false;
	//srand((unsigned)GetTickCount());
	struct timeb timeSeed;
	ftime(&timeSeed);
	srand((UINT)(timeSeed.time * NUM_ONE_SECOND_MS + timeSeed.millitm));  // milli time
	int iResult = rand()%100;

	if (0 >=G_i64AIHaveWinMoney)
	{
		//机器人赢的钱少于0 机器人必赢
		bAIWin = true;		
	}
	else if((0 < G_i64AIHaveWinMoney) && (G_i64AIHaveWinMoney < m_iAIWantWinMoneyA1))
	{
		// 机器人的赢钱在0-A1区域 并且概率符合m_iAIWinLuckyAtA1 机器人要赢
		if (iResult <=m_iAIWinLuckyAtA1)
		{
			bAIWin = true;		
		}
	}
	else if ((m_iAIWantWinMoneyA1 <= G_i64AIHaveWinMoney)  && (G_i64AIHaveWinMoney <m_iAIWantWinMoneyA2))
	{
		// 机器人的赢钱在A1-A2区域 并且概率符合m_iAIWinLuckyAtA2 机器人要赢
		if (iResult <=m_iAIWinLuckyAtA2)
		{
			bAIWin = true;	
		}
	}
	else if ((m_iAIWantWinMoneyA2 <= G_i64AIHaveWinMoney)  && (G_i64AIHaveWinMoney <m_iAIWantWinMoneyA3))
	{
		// 机器人的赢钱在A2-A3区域 并且概率符合m_iAIWinLuckyAtA3 机器人要赢
		if (iResult <=m_iAIWinLuckyAtA3)
		{
			bAIWin = true;
		}
	}
	else
	{
		// 机器人的赢钱超过A3区域 并且概率符合m_iAIWinLuckyAtA4 机器人要赢
		if (iResult <=G_i64AIHaveWinMoney)
		{
			bAIWin = true;
		}
	}
	CString llog;
	llog.Format("lbdzpk :bAIWin = %d",bAIWin);
	OutputDebugString(llog);

	BYTE byResultCard[8][7];
	memset(byResultCard, 0, sizeof(byResultCard));
	BYTE byDeskCard[5] = {0};	//桌面牌

	::CopyMemory(byDeskCard,&m_GameData.m_byCards[m_GameData.m_iGetCardPos],sizeof(BYTE)*5);	//获取低牌
	// 分析牌型
	for (int i = 0;i < PLAY_COUNT;i++)
	{
		m_Logic.AnalysisCard(m_GameData.m_UserData[i].m_byMyCards, 2, byDeskCard, 5, byResultCard[i]);
		memcpy(m_GameData.m_bResultCards[i],byResultCard[i], sizeof(BYTE) * 5);  // 五张最大的牌
	}
	CByteArray arrayResult;
	CalculateWin(0,arrayResult,true);
	bool bTraveled[PLAY_COUNT] = {0};

	if (bAIWin)
	{
		for (int j = 0;j < arrayResult.GetSize();j++)
		{								
			CString llog;
			llog.Format("lbdzpk::::::::00000000000赢家 = %d",arrayResult.GetAt(j));
			OutputDebugString(llog);
			for (int i = 0;i < PLAY_COUNT;i++)
			{
				if (!m_pUserInfo[i]) continue;
				if (!m_pUserInfo[i]->m_UserData.isVirtual) continue; //不是机器人
				if (!m_GameData.m_UserData[i].m_bIn) continue; //没带入金币
				if (bTraveled[i]) continue;

				// 比牌(-1:输, 0:和, 1:赢)
				//if (m_Logic.CompareCard(m_GameData.m_bResultCards[i], 5, m_GameData.m_bResultCards[arrayResult.GetAt(j)], 5) > 0)
				{
					llog.Format("lbdzpk::::::::00000000000赢家 = %s ,最大牌者  = %s",m_pUserInfo[arrayResult.GetAt(j)]->m_UserData.nickName,m_pUserInfo[i]->m_UserData.nickName);
					OutputDebugString(llog);
					Change2UserCard(i,arrayResult.GetAt(j));
					bTraveled[i] = true;
					break;
				}
			}
		}
	}
	else
	{
		for (int j = 0;j < arrayResult.GetSize();j++)
		{
			for (int i = 0;i < PLAY_COUNT;i++)
			{
				if (!m_pUserInfo[i]) continue;
				if (m_pUserInfo[i]->m_UserData.isVirtual) continue; //是机器人
				if (!m_GameData.m_UserData[i].m_bIn) continue; //没带入金币
				if (bTraveled[i]) continue;
				llog.Format("lbdzpk::::::::111111赢家 = %s ,最大牌者  = %s",m_pUserInfo[arrayResult.GetAt(j)]->m_UserData.nickName,m_pUserInfo[i]->m_UserData.nickName);
				OutputDebugString(llog);
				Change2UserCard(i,arrayResult.GetAt(j));
				bTraveled[i] = true;
				break;
			}
		}
	}
}

```







```C++
void	CServerGameDesk::Change2UserCard(BYTE byFirstDesk,BYTE bySecondDesk)
{
	BYTE byTmpCard[MAX_BACKCARDS_NUM];
	memcpy(byTmpCard,	m_GameData.m_UserData[byFirstDesk].m_byMyCards,	sizeof(byTmpCard));
	memcpy(m_GameData.m_UserData[byFirstDesk].m_byMyCards,	m_GameData.m_UserData[bySecondDesk].m_byMyCards,	sizeof(m_GameData.m_UserData[byFirstDesk].m_byMyCards));
	memcpy(m_GameData.m_UserData[bySecondDesk].m_byMyCards,byTmpCard,sizeof(m_GameData.m_UserData[bySecondDesk].m_byMyCards));
}
```

