## 德州扑克AI分析(服务端)

尝试看服务端发送给机器人的行为概率是否有效

```C++
bool CServerGameDesk::LoadRobotActionProb()
{
	//TCHAR skin[MAX_NAME_INFO];
	//CString s = CINIFile::GetAppPath ();/////本地路径
	//CString temp = _T("");
	//temp.Format("%s%s_s.ini",s,GET_SKIN_FOLDER(skin));
	//CINIFile f( temp);
	//CString key = TEXT("game");

	TCHAR szIniName[MAX_PATH];
	GET_SERVER_INI_NAME(szIniName);
	CINIFile f(szIniName);//正则
	CString key = "game";

	SetActionProb tSetActionProbTemp;//行为概率
	CString StrTemp = "";
	int iSumProb = 0;
	for(int i = 0; i < ENUM_SET_ACTION_STATUS_Max; ++i)//检测配置是否到达100
	{
		iSumProb = 0;
		StrTemp.Format("ProbFold[%d]",i);
		iSumProb += tSetActionProbTemp.byProbFold[i] = (BYTE)f.GetKeyVal(key,StrTemp,0);
		StrTemp.Format("ProbFollow[%d]",i);
		iSumProb += tSetActionProbTemp.byProbFollow[i] = (BYTE)f.GetKeyVal(key,StrTemp,0);
		StrTemp.Format("ProbRaise[%d]",i);
		iSumProb += tSetActionProbTemp.byProbRaise[i] = (BYTE)f.GetKeyVal(key,StrTemp,0);
		StrTemp.Format("ProbAllIn[%d]",i);
		iSumProb += tSetActionProbTemp.byProbAllIn[i] = (BYTE)f.GetKeyVal(key,StrTemp,0);

		if (iSumProb != 100)
		{
			tSetActionProbTemp.ReSet((ENUM_SET_ACTION_STATUS)i);
		}
	}

	m_tSetActionProb = tSetActionProbTemp;

	return true;
}
```



//发送概率给机器人

```C++
	//如果不使用就不发送
	if (!G_bUseSetActionProb)
	{
		return true;
	}

	//房卡场不发送
	if (m_bIsBuy)
	{
		return true;
	}

	//不是游戏状态不发送
	if (!IsPlayingByGameStation())
	{
		return true;
	}

	if (bDeskStation >= PLAY_COUNT)
	{
		for (int i = 0; i < PLAY_COUNT; i ++) 
		{
			//只给Robot发送
			if(NULL == m_pUserInfo[i] || !m_pUserInfo[i]->m_UserData.isVirtual)
				continue;
			SendGameData(i,&m_tSetActionProb,sizeof(m_tSetActionProb),MDM_GM_GAME_NOTIFY,ASS_SET_ACTION_PROB,0);
		}
		LOG_MSG_SEND(ASS_SET_ACTION_PROB);
	} else 
	{
		if(NULL == m_pUserInfo[bDeskStation] || !m_pUserInfo[bDeskStation]->m_UserData.isVirtual)
		{
			return true;
		}
		SendGameData(bDeskStation,&m_tSetActionProb,sizeof(m_tSetActionProb),MDM_GM_GAME_NOTIFY,ASS_SET_ACTION_PROB,0);
		LOG_MSG_SEND(ASS_SET_ACTION_PROB, bDeskStation);
	}

	return true;
}
```

