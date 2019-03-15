#include "PlayerActionDelegate.h"

void PlayerActionDelegateToDo(void **data, int dataCnt)
{
	GameContext *context = (GameContext *)data[0];
	ExtensionPlayerActionDelegate *actionDelegate = (ExtensionPlayerActionDelegate *)data[1];
	const DelegateInfo *delegateInfo = (DelegateInfo *)data[2];

	delegateInfo->callback(context, delegateInfo->actionType, delegateInfo->playerPos, delegateInfo->otherData);

	actionDelegate->OnPlayerResponse(delegateInfo->playerPos, delegateInfo->actionType);
}


void ExtensionPlayerActionDelegate::EraseDelegateInfo(DelegateInfo *info)
{
	info->data.ShutDown();
	delete info;
}


void ExtensionPlayerActionDelegate::AddAction(uchar playerPos, int actionType, float time, PlayerActionDelegateCallStateback callback, void *data, bool replace)
{
	if(replace)
		RemoveAction(playerPos, actionType);
	else			
	{
		PlayerActionDelegateInfo::iterator itr = m_DelegateInfo.find(playerPos);
		if(itr != m_DelegateInfo.end())
		{
			ActionDelegateInfo::iterator itr2 = itr->second.find(actionType);
			if(itr2 != itr->second.end())
			{
				return;
			}
		}	
	}

	if(callback)
	{
		PlayerActionDelegateInfo::iterator itr = m_DelegateInfo.find(playerPos);
		if(itr == m_DelegateInfo.end())
		{
			std::pair<
				PlayerActionDelegateInfo::iterator,
				bool
			> insertItr = m_DelegateInfo.insert(std::make_pair(playerPos, ActionDelegateInfo()));
			itr = insertItr.first;
		}

		DelegateInfo *info = new DelegateInfo();
		itr->second[actionType] = info;

		OBJ_GET_EXT(m_Context, ExtensionTimer, extTimer);
		void *pointers[] = {m_Context, this, info};
		int pointerCnt = sizeof(pointers)/sizeof(pointers[0]);
		TimerData timerData = TimerData(pointers, pointerCnt, PlayerActionDelegateToDo);
		extTimer->Add(time, timerData);
		
		info->playerPos = playerPos;
		info->actionType = actionType;
		info->data = timerData;
		info->callback = callback;
		info->otherData = data;
	}
}

void ExtensionPlayerActionDelegate::OnPlayerResponse(uchar playerPos, int type)
{
	RemoveAction(playerPos, type);
}

void ExtensionPlayerActionDelegate::RemoveAction(uchar playerPos, int type)
{
	PlayerActionDelegateInfo::iterator itr = m_DelegateInfo.find(playerPos);
	if(itr != m_DelegateInfo.end())
	{
		ActionDelegateInfo::iterator itr2 = itr->second.find(type);
		if(itr2 != itr->second.end())
		{
			EraseDelegateInfo(itr2->second);
			itr->second.erase(itr2);
		}
	}
}

void ExtensionPlayerActionDelegate::RemoveAll()
{
	for(PlayerActionDelegateInfo::iterator itr = m_DelegateInfo.begin();
		itr != m_DelegateInfo.end();
		++itr)
	{
		for(ActionDelegateInfo::iterator itr2 = itr->second.begin();
			itr2 != itr->second.end();
			++itr2)
		{
			EraseDelegateInfo(itr2->second);
			itr->second.erase(itr2);
		}
	}

	m_DelegateInfo.clear();
}

void ExtensionPlayerActionDelegate::AddListener(uchar playerPos, const RefCntObj<PlayerActionDelegateEnabledListener> &listener)
{
	RemoveListener(playerPos);
	m_EnabledListeners.insert(std::make_pair(playerPos,listener));
}

void ExtensionPlayerActionDelegate::RemoveListener(uchar playerPos)
{
	m_EnabledListeners.erase(playerPos);
}

void ExtensionPlayerActionDelegate::OnEnableActionDelegate(uchar playerPos, bool b)
{
	ListenerList::iterator itr = m_EnabledListeners.find(playerPos);
	if(itr != m_EnabledListeners.end())
	{
		bool erase = !itr->second->OnEnable(m_Context, playerPos, b);
		if(erase)
		{
			m_EnabledListeners.erase(itr);
		}
	}
}

void ExtensionPlayerActionDelegate::OnGameStart()
{
	RemoveAll();
}

void ExtensionPlayerActionDelegate::OnGameFinish()
{
	RemoveAll();
}

void ExtensionPlayerActionDelegate::OnGameReset()
{
	RemoveAll();
}
