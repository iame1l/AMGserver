//GameBeginListener.h
//游戏开始阶段监听器
#include "stdafx.h"
#include "ServerManage.h"
#include "GameBeginListener.h"
#include "DataManage.h"
#include "Algorithm.h"
GameBeginListener::GameBeginListener()
{
}

GameBeginListener::~GameBeginListener()
{
	
}

//玩家和桌子的初始化
void GameBeginListener::OnAdd()
{
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	__super::OnAdd();

	exDataMgr->shuffleCard();
	exDataMgr->LoadConfCardUser();
	//超端换牌
	for(int i = 0; i < PLAY_COUNT; i++)
	{
		if(NULL == m_Context->GetGameDesk()->m_pUserInfo[i])
		{
			continue;
		}
		if(m_Context->GetGameDesk()->m_SuperSetData[i].bSetSuccess)
		{
			exDataMgr->m_isSupSet = true;
			if (m_Context->GetGameDesk()->m_SuperSetData[i].byWinQuYu != exDataMgr->byWinQuYu)
			{
				if(exDataMgr->byWinQuYu == 3)
				{
					if(exDataMgr->vSendCard[1].getRealVal()==14)
					{
						if(m_Context->GetGameDesk()->m_SuperSetData[i].byWinQuYu == 1)
						{
							exDataMgr->vSendCard[0].setData(CardBase(D_10));
						}
						else if(m_Context->GetGameDesk()->m_SuperSetData[i].byWinQuYu == 2)
						{
							exDataMgr->vSendCard[1].setData(CardBase(D_10));
						}
					}
					else
					{
						if(m_Context->GetGameDesk()->m_SuperSetData[i].byWinQuYu == 1)
						{
							exDataMgr->vSendCard[1].setData(CardBase(H_A));
						}
						else if(m_Context->GetGameDesk()->m_SuperSetData[i].byWinQuYu == 2)
						{
							exDataMgr->vSendCard[0].setData(CardBase(H_A));
						}
					}
				}
				else if(3 == m_Context->GetGameDesk()->m_SuperSetData[i].byWinQuYu)
				{
					uchar type=exDataMgr->vSendCard[0].getType();
					if (type < TYPE_SPADE )
					{
						type+=1;
					}
					else 
					{
						type = 0;
					}
					uchar val=exDataMgr->vSendCard[0].getVal();
					exDataMgr->vSendCard[1].setData(CardBase((type<<4)+val));
				}
				else
				{
					std::swap(exDataMgr->vSendCard[0],exDataMgr->vSendCard[1]);
				}
				exDataMgr->byWinQuYu = m_Context->GetGameDesk()->m_SuperSetData[i].byWinQuYu;
			}
			break;
		}
	}
	//只生效一把
	memset(m_Context->GetGameDesk()->m_SuperSetData, 0, sizeof(m_Context->GetGameDesk()->m_SuperSetData));

	OBJ_GET_EXT(m_Context,ExtensionListenerController,exListenerControl);
	exListenerControl->OnListenEnd(LT_GameBegin);
}

void GameBeginListener::OnTimer( uint timerID )
{
}

void GameBeginListener::Record( uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode )
{
}
