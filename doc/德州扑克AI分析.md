## 德州扑克AI分析

```C++
bool CClientGameDlg::HandleGameMessage(NetMessageHead * pNetHead, void * pNetData, UINT uDataSize, CTCPClientSocket * pClientSocket)
{
	if (pNetHead->bMainID == MDM_GM_GAME_NOTIFY)
	{
		switch(pNetHead->bAssistantID)

		{
		case ASS_GAME_BEGIN:
			{
				//游戏开始
				SetStationParameter(GS_PLAY_GAME);
				// 初始化数据
				InitData();
				return true;
			}
		case ASS_SYS_DESK_CFG:
			{
				//收到系统配置桌子的信息包，并处理
				TDeskCfg* pDeskCfg = (TDeskCfg*)pNetData;

				//更改数据
				::memcpy(&m_tagDeskCfg,pDeskCfg,sizeof(TDeskCfg));
				return true;
			}
		case ASS_CFG_DESK: 
			{
				//修正游戏的状态
				SetStationParameter(GS_CONFIG_NOTE);
				return true;
			}
		case ASS_CFG_DESK_TIMEOUT:
			{
				return true;
			}
		case ASS_DESK_CFG: 
			{
				//修正游戏的状态
				SetStationParameter(GS_CONFIG_NOTE);

				TDeskCfg* pDeskCfg = (TDeskCfg*)pNetData;

				//更改数据
				::memcpy(&m_tagDeskCfg,pDeskCfg,sizeof(TDeskCfg));
				return true;
			}
		case ASS_SHOW_IN_MONEY: 
			{
				//收到代入金额信息包，并处理
				TSubMoney* pSubMoney = (TSubMoney*)pNetData;

				memcpy(m_iMoneys, pSubMoney->nMoney, sizeof(m_iMoneys));
				return true;
			}
		case ASS_SEND_A_CARD:
			{
				//收到服务器发一张牌包，并处理
				TCards* pCards = (TCards*)pNetData;

				if (m_byNTUser == 255)
				{
					return false;
				}

				m_nTypeCard = pCards->nTypeCard;	

				// 自己的牌数据
				::memcpy(m_iUserDataCard, pCards->byCards[GetMeUserInfo()->bDeskStation], MAX_HANDCARDS_NUM);
				m_iCardCounts = pCards->iCardsNum;

				memset(m_byRobotInAllCard,255,sizeof(m_byRobotInAllCard));
				m_nRobotInAllCardNum = 0;
				// 将自己的牌数据拷贝到机器人总牌数据
				::memcpy(m_byRobotInAllCard,pCards->byCards[GetMeUserInfo()->bDeskStation],sizeof(BYTE)*pCards->iCardsNum);
				m_nRobotInAllCardNum = pCards->iCardsNum;

				SendGameData(MDM_GM_GAME_NOTIFY, ASS_SEND_CARD_FINISH, NULL);

				return true;
			}
		case ASS_BETPOOL_UP:
			{
				//收到服务器边池更新包，并处理
				TBetPool* pBetPool = (TBetPool *)pNetData;

				memcpy(&m_tagBetPool, pBetPool, sizeof(m_tagBetPool));

				return true;
			}
		case ASS_SEND_3_CARD:
			{
				//收到服务器发一张牌包，并处理
				TCards * pCards = (TCards *)pNetData;
				
				// 拷贝三张公共牌
				memcpy(m_iCardData, pCards->byPubCards, sizeof(BYTE) * (pCards->iCardsNum));
				m_iCardDataCounts += pCards->iCardsNum;

				// 将自己的牌数据拷贝到机器人总牌数据
				::memcpy(&m_byRobotInAllCard[m_nRobotInAllCardNum],pCards->byPubCards,sizeof(BYTE)*pCards->iCardsNum);
				m_nRobotInAllCardNum += pCards->iCardsNum;

				return true;
			}			
		case ASS_SEND_4_5_CARD:
			{
				//收到服务器发一张牌包，并处理
				TCards * pCards = (TCards *)pNetData;

				// 拷贝第四与第五张公共牌
				memcpy(m_iCardData, pCards->byPubCards, sizeof(BYTE) * (pCards->iCardsNum));
				m_iCardDataCounts = pCards->iCardsNum;

				// 将自己的牌数据拷贝到机器人总牌数据
				::memcpy(&m_byRobotInAllCard[m_nRobotInAllCardNum],pCards->byPubCards,sizeof(m_byRobotInAllCard[m_nRobotInAllCardNum]));
				m_nRobotInAllCardNum += 1;

				return true;
			}
		case ASS_SEND_CARD_FINISH:
			{
				//收到服务器报告所有玩家发牌完成的消息，并处理
				
				return true;
			}
		case ASS_BET_INFO:
			{
				//收到玩家下注的消息
				TBet* pBet = (TBet*)pNetData;

				if (pBet->nType == ET_AUTO || pBet->nType == ET_BET || pBet->nType == ET_ADDNOTE || pBet->nType == ET_ALLIN || pBet->nType == ET_CALL)
				{
					m_nBetMoney[pBet->byUser] += pBet->nMoney; 
					m_iMoneys[pBet->byUser] -= pBet->nMoney;
				}

				// 记录全下与放弃玩家
				if (pBet->nType == ET_ALLIN)
				{
					m_bAllBet[pBet->byUser] = true;
				}
				else if (pBet->nType == ET_FOLD)
				{
					m_bGiveUp[pBet->byUser] = true;		
				}

				// 记录这个玩家操作类型
				m_emLastOperation[pBet->byUser] = pBet->nType;

				return true;
			}
		case ASS_TOKEN:
			{
				//收到令牌消息，激活用户
				TToken* pToken = (TToken*)pNetData;

				//更新数据
				m_byTokenUser = pToken->byUser;

				// 允许动作标志
				BYTE byVerbFlag = pToken->byVerbFlag;

				int iWeight = 0;
				int nRandByWeight = 0;

				// 模拟下注消息
				if (m_byTokenUser == m_byMeStation)
				{
					if (m_nRobotInAllCardNum >= 2)
					{
						iWeight = m_Logic.RobotHandCardAnalysis(m_byRobotInAllCard,m_nRobotInAllCardNum);

						if ((byVerbFlag & UD_VF_BET) && (byVerbFlag & UD_VF_CHECK) && (byVerbFlag & UD_VF_FOLD))
						{
							nRandByWeight = GetRandBetByWeight(iWeight);
						}
						else if ((byVerbFlag & UD_VF_ADD) && (byVerbFlag & UD_VF_CALL) && (byVerbFlag & UD_VF_FOLD))
						{
							nRandByWeight = GetRandAddByWeight(iWeight);
						}
						else if ((byVerbFlag & UD_VF_ALLIN) && (byVerbFlag & UD_VF_FOLD))
						{
							nRandByWeight = GetRandAllinByWeight(iWeight);
						}
					}

					switch (byVerbFlag & nRandByWeight)
					{
					case UD_VF_BET:		// 允许下注
						{
							srand((unsigned) time (NULL) + m_byMeStation);
							int iRandTime = rand() % 3 + 1;
							SetGameTimer(m_byMeStation,iRandTime,IDT_BET_TIMER);
							break;
						}
					case UD_VF_CALL:	// 允许跟注
						{
							m_iFollowNum++;			//机器人跟注次数
							m_nCallMoney = pToken->nCallMoney;
							srand((unsigned) time (NULL) + m_byMeStation);
							int iRandTime = rand() % 3 + 1;
							SetGameTimer(m_byMeStation,iRandTime,IDT_CALL_TIMER);
							break;
						}
					case UD_VF_ADD:		// 允许加注
						{
							m_nCallMoney = pToken->nCallMoney;
							srand((unsigned) time (NULL) + m_byMeStation);
							int iRandTime = rand() % 3 + 1;
							SetGameTimer(m_byMeStation,iRandTime,IDT_RAISE_TIMER);
							break;
						}
					case UD_VF_CHECK:  // 允许过牌
						{
							srand((unsigned) time (NULL) + m_byMeStation);
							int iRandTime = rand() % 3 + 1;
							SetGameTimer(m_byMeStation,iRandTime,IDT_CHECK_TIMER);
							break;
						}
					case UD_VF_FOLD:   // 允许弃牌
						{
							srand((unsigned) time (NULL) + m_byMeStation);
							int iRandTime = rand() % 3 + 1;
							SetGameTimer(m_byMeStation,iRandTime,IDT_FOLO_TIMER);
							break;
						}
					case UD_VF_ALLIN:  // 允许全下
						{
							__int64 nMoney = m_nBetMoney[GetLastUserStation(m_byMeStation)] - m_nBetMoney[m_byMeStation];

							if (m_iMoneys[m_byMeStation] <= nMoney)
							{
								srand((unsigned) time (NULL) + m_byMeStation);
								int iRandTime = rand() % 3 + 1;

								SetGameTimer(m_byMeStation,iRandTime,IDT_ALLIN_TIMER);
							}
							break;
						}
					default:
						{
							OutputDebugString("jan: 未知操作\n");
							SetGameTimer(m_byMeStation,1,IDT_FOLO_TIMER);
							break;
						}
					}	
				}

				return true;
			}
		case ASS_COMPARE_CARD:
			{
				//比牌消息
				TCompareCard *pCmd = (TCompareCard*)pNetData;

				return true;
			}
		case ASS_RESULT:
			{
				//修正游戏的状态
				SetStationParameter(GS_WAIT_NEXT_ROUND);
				//收到结算消息包，并处理
				TResult* pResult = (TResult*)pNetData;

				//记录游戏数据
				::memcpy(&m_tagResult,pResult,sizeof(TResult));

				if (m_iMoneys[m_byMeStation] <= m_tagDeskCfg.dz.iUntenMoney)
				{
					// 机器人当前准备带入的金币
					m_iCurrentMoney = m_tagDeskCfg.dz.iSubPlanMoney;

					// 金币数据不相等就更新
					if (m_iCurrentMoney < m_iMinMoney)
					{
						m_iCurrentMoney = m_iMinMoney;
					}
					else if (m_iCurrentMoney > m_iMaxMoney)
					{
						m_iCurrentMoney = m_iMaxMoney;
					}

					m_iSelfMoney = GetMeUserInfo()->i64Money;

					// 防止代入金币超过自身金币
					if (m_iCurrentMoney > m_iSelfMoney)
					{
						m_iCurrentMoney = m_iSelfMoney;
					}


					if (m_iSelfMoney < m_iMinMoney)
					{
						// 本身金币少于最低带入将玩家踢出
						if (m_pUserInfo[m_byMeStation] != NULL)
						{
							if (m_tagResult.nSelfMoney[m_byMeStation] < m_tagDeskCfg.dz.iSubMinMoney)
							{
								SetGameTimer(m_byMeStation, 500, IDT_MONEY_LACK);
							}
						}
						return false;
					}

					srand((unsigned) time (NULL) + m_byMeStation) ; 
					int iRandTime = rand() % 3 + 1; 

					// 机器人自动带入金额定时器
					SetGameTimer(m_byMeStation,iRandTime,IDT_IN_MONEY_TIMER);
				}
				else
				{
					srand((unsigned) time (NULL) + m_byMeStation) ; 
					int iRandTime = rand() % 3 + 1; 

					// 机器人自动带入金额定时器
					SetGameTimer(m_byMeStation,iRandTime,IDT_PODO_MONEY_TIMER);
				}

				return true;
			}
		case ASS_NEW_ROUND_BEGIN:
			{
				//收到新的一回合开始消息包，并处理
				TNextRoundBeginInfo* pNextRoundBeginInfo = (TNextRoundBeginInfo*)pNetData;

				//记录庄家的位置
				m_byNTUser = pNextRoundBeginInfo->byNTUser;

				//记录大小盲注
				m_bySmallBlind = pNextRoundBeginInfo->bSmallBlind;
				m_byBigBlind = pNextRoundBeginInfo->bBigBlind;


				//底牌清空，数量归零
				::memset(m_byBackCard,0,MAX_BACKCARDS_NUM);
				m_iBackCardNums = 0;


				//初始一些桌面配置/////////////////////////////

				TDeskCfg::TRule* pRule = &m_tagDeskCfg.Rule;
				return true;
			}
		case ASS_NO_PLAYER:
			{
				//没有玩家进行游戏，退出处理
				//OnHandleNoPlayer(buffer,nLen);
				return true;
			}
		case ASS_AGREE:
			{
				//玩家是否同意的游戏的，处理
				//return m_GameMachine.OnHandleAgree(pNetData,uDataSize);
				return true;
			}
		case ASS_NOT_ENOUGH_MONEY:
			{
				//玩家手上的钱不足够时，处理
				//OnHandleNotEnoughMoney(buffer,nLen);
				return true;
			}
		case ASS_USER_LEFT_DESK:
			{
				return true;
			}
		case ASS_SORT_OUT:
			{
				//系统自动清理片
				//OnUserSortOut();
				return true;
			}
		default:
			break;

		}	
	}
	return __super::HandleGameMessage(pNetHead,pNetData,uDataSize,pClientSocket);
}
```





行为分析

```C++
case ASS_BET_INFO:
			{
				//收到玩家下注的消息
				TBet* pBet = (TBet*)pNetData;

				if (pBet->nType == ET_AUTO || pBet->nType == ET_BET || pBet->nType == ET_ADDNOTE || pBet->nType == ET_ALLIN || pBet->nType == ET_CALL)
				{
					m_nBetMoney[pBet->byUser] += pBet->nMoney; 
					m_iMoneys[pBet->byUser] -= pBet->nMoney;
				}

				// 记录全下与放弃玩家
				if (pBet->nType == ET_ALLIN)
				{
					m_bAllBet[pBet->byUser] = true;//获取玩家位置的,后面需要绕开这些位置
				}
				else if (pBet->nType == ET_FOLD)
				{
					m_bGiveUp[pBet->byUser] = true;		
				}

				// 记录这个玩家操作类型
				m_emLastOperation[pBet->byUser] = pBet->nType;

				return true;
			}
```









```C++
case ASS_TOKEN:
			{
				//收到令牌消息，激活用户
				TToken* pToken = (TToken*)pNetData;

				//更新数据
				m_byTokenUser = pToken->byUser;

				// 允许动作标志
				BYTE byVerbFlag = pToken->byVerbFlag;

				int iWeight = 0;
				int nRandByWeight = 0;

				// 模拟下注消息
				if (m_byTokenUser == m_byMeStation)
				{
					if (m_nRobotInAllCardNum >= 2)
					{
						iWeight = m_Logic.RobotHandCardAnalysis(m_byRobotInAllCard,m_nRobotInAllCardNum);

						if ((byVerbFlag & UD_VF_BET) && (byVerbFlag & UD_VF_CHECK) && (byVerbFlag & UD_VF_FOLD))
						{
							nRandByWeight = GetRandBetByWeight(iWeight);
						}
						else if ((byVerbFlag & UD_VF_ADD) && (byVerbFlag & UD_VF_CALL) && (byVerbFlag & UD_VF_FOLD))
						{
							nRandByWeight = GetRandAddByWeight(iWeight);
						}
						else if ((byVerbFlag & UD_VF_ALLIN) && (byVerbFlag & UD_VF_FOLD))
						{
							nRandByWeight = GetRandAllinByWeight(iWeight);
						}
					}

					switch (byVerbFlag & nRandByWeight)
					{
					case UD_VF_BET:		// 允许下注
						{
							srand((unsigned) time (NULL) + m_byMeStation);
							int iRandTime = rand() % 3 + 1;
							SetGameTimer(m_byMeStation,iRandTime,IDT_BET_TIMER);
							break;
						}
					case UD_VF_CALL:	// 允许跟注
						{
							m_iFollowNum++;			//机器人跟注次数
							m_nCallMoney = pToken->nCallMoney;
							srand((unsigned) time (NULL) + m_byMeStation);
							int iRandTime = rand() % 3 + 1;
							SetGameTimer(m_byMeStation,iRandTime,IDT_CALL_TIMER);
							break;
						}
					case UD_VF_ADD:		// 允许加注
						{
							m_nCallMoney = pToken->nCallMoney;
							srand((unsigned) time (NULL) + m_byMeStation);
							int iRandTime = rand() % 3 + 1;
							SetGameTimer(m_byMeStation,iRandTime,IDT_RAISE_TIMER);
							break;
						}
					case UD_VF_CHECK:  // 允许过牌
						{
							srand((unsigned) time (NULL) + m_byMeStation);
							int iRandTime = rand() % 3 + 1;
							SetGameTimer(m_byMeStation,iRandTime,IDT_CHECK_TIMER);
							break;
						}
					case UD_VF_FOLD:   // 允许弃牌
						{
							srand((unsigned) time (NULL) + m_byMeStation);
							int iRandTime = rand() % 3 + 1;
							SetGameTimer(m_byMeStation,iRandTime,IDT_FOLO_TIMER);
							break;
						}
					case UD_VF_ALLIN:  // 允许全下
						{
							__int64 nMoney = m_nBetMoney[GetLastUserStation(m_byMeStation)] - m_nBetMoney[m_byMeStation];

							if (m_iMoneys[m_byMeStation] <= nMoney)
							{
								srand((unsigned) time (NULL) + m_byMeStation);
								int iRandTime = rand() % 3 + 1;

								SetGameTimer(m_byMeStation,iRandTime,IDT_ALLIN_TIMER);
							}
							break;
						}
					default:
						{
							OutputDebugString("jan: 未知操作\n");
							SetGameTimer(m_byMeStation,1,IDT_FOLO_TIMER);
							break;
						}
					}	
				}

				return true;
			}
```

