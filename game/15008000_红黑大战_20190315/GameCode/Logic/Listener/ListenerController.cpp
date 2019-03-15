#include "stdafx.h"
#include "ServerManage.h"
#include "ListenerController.h"
#include "ExtensionLogger.h"

ExtensionListenerController::ExtensionListenerController( )
{
}


ExtensionListenerController::~ExtensionListenerController()
{
}

void ExtensionListenerController::Add( ListenerType listenerType )
{
	LOGGER_FILE(m_Context,"Listen Add "<<(int)listenerType);
	auto itr = m_CurrListeners.find(listenerType);
	if(itr != m_CurrListeners.end())
	{
		_Add(itr->second);
	}
}

void ExtensionListenerController::Remove( ListenerType listenerType )
{
	LOGGER_FILE(m_Context,"Listen Remove "<<(int)listenerType);
	auto itr = m_CurrListeners.find(listenerType);
	if(itr != m_CurrListeners.end())
	{
		_Remove(itr->second);
	}
}

void ExtensionListenerController::Listen( uchar playerPos, uint msgID, void* msgData ,uint uSize)
{
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	DataManage::sGameUserInf User;
	if(!exDataMgr->getUserInfo(playerPos,User))return;

	LOGGER_FILE(m_Context,"recv<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"<<" User("<<User.userID<<")  MsgID("<<(int)msgID<<")  size("<<uSize<<")");
	for (auto iter = m_CurrListeners.begin();iter != m_CurrListeners.end(); ++iter)
	{
		if ((*iter).second->Flag())
			(*iter).second->Listen(playerPos, msgID, msgData,uSize);
	}
}

void ExtensionListenerController::_Remove( Listener *listener )
{
	if (listener->Flag())
		listener->OnRemove();
}

void ExtensionListenerController::_Add( Listener *listener )
{
	if (!listener->Flag())
		listener->OnAdd();
}

void ExtensionListenerController::Reconnect(uchar playerPos, uint socketID, bool isWatchUser)
{	
	for (auto iter = m_CurrListeners.begin();iter != m_CurrListeners.end(); ++iter)
	{
		if ((*iter).second->Flag())
			(*iter).second->OnReconnect(playerPos, socketID, isWatchUser);
	}
}

void ExtensionListenerController::Clear()
{
	for (auto iter = m_CurrListeners.begin();iter != m_CurrListeners.end(); ++iter)
	{
		if ((*iter).second->Flag())
			(*iter).second->OnRemove();
	}
}

void ExtensionListenerController::OnTimer( uint timerID )
{	
	for (auto iter = m_CurrListeners.begin(); iter != m_CurrListeners.end(); ++iter)
	{
		if ((*iter).second->Flag())
			(*iter).second->OnTimer(timerID);
	}
}

void ExtensionListenerController::Recode( uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode )
{
	for (auto iter = m_CurrListeners.begin(); iter != m_CurrListeners.end(); ++iter)
	{
		if ((*iter).second->Flag())
			(*iter).second->Record(playerPos, msgData, dataSize, mainID, assID, handleCode);
	}
}

void ExtensionListenerController::OnListenEnd( ListenerType listenerType )
{
	LOGGER_FILE(m_Context,"Listen End "<<(int)listenerType);

	switch(listenerType)
	{
	case LT_GameBegin: 
		{ 
			Remove(LT_GameBegin); 
			Add(LT_PlayCard); 
		} 
		break;
	case LT_SendCard_Player:  
		{ 
			Remove(LT_SendCard_Player); 
			Add(LT_GameFinish);
		} 
		break;
	case LT_GameFinish:  
		{ 
			Remove(LT_GameFinish);
			m_Context->GetGameDesk()->GameFinish(0, GF_NORMAL);
			Add(LT_WaitNext);
		} 
		break;
	case LT_PlayCard:
		{
			Remove(LT_PlayCard); 
			Add(LT_SendCard_Player); 
		}
	}
}

void ExtensionListenerController::InitListener()
{
	{
		OBJ_GET_EXT(m_Context,WaitSetGameListener	,exWaitSet		 );
		OBJ_GET_EXT(m_Context,WaitNextListener		,exWaitNext		 );
		OBJ_GET_EXT(m_Context,GameBeginListener		,exGameBegin	 );
		OBJ_GET_EXT(m_Context,SendCardPlayerListener,exSendCardPlayer);
		OBJ_GET_EXT(m_Context,GameFinishListener	,exGameFinish	 );
		OBJ_GET_EXT(m_Context,PlayCardListener		,exPlayCard		 );

		m_CurrListeners.clear();
		m_CurrListeners[LT_WaitSet]			= exWaitSet;
		m_CurrListeners[LT_WaitNext]		= exWaitNext;
		m_CurrListeners[LT_GameBegin]		= exGameBegin;
		m_CurrListeners[LT_SendCard_Player]	= exSendCardPlayer;
		m_CurrListeners[LT_GameFinish]		= exGameFinish;
		m_CurrListeners[LT_PlayCard]		= exPlayCard;
	
		for (auto itr = m_CurrListeners.begin();itr != m_CurrListeners.end();itr++)
		{
			itr->second->SetContext(m_Context);
		}
	}
}