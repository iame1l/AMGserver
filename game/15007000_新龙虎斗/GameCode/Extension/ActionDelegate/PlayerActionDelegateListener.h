#ifndef PLAYER_ACTION_DELEGATE_LISTENER_H
#define PLAYER_ACTION_DELEGATE_LISTENER_H

#include "PlayerActionDelegate.h"

// 玩家托管行为监听器, 处理玩家打开/关闭托管的情况

template<typename T>
class PlayerActionDelegateListener : public PlayerActionDelegateEnabledListener
{

	T *m_ActionHandler;
	PlayerActionDelegateCallStateback m_Callback;
	int m_Type;
	float m_Delay;

public:
	PlayerActionDelegateListener(T *handler, PlayerActionDelegateCallStateback callback, float delay, int type):
	  m_ActionHandler(handler), m_Callback(callback), m_Delay(delay), m_Type(type)
	{}

	bool OnEnable(GameContext *context, uchar playerPos, bool b)
	{
		if(b)
		{
			// 打开托管了，设置一个马上执行的行为代理
			AddActionDelegate(context, playerPos, m_Callback, 1, m_Type, m_ActionHandler,true);
		}
		else
		{
			// 关闭托管了，设置一个延迟执行的行为代理
			AddActionDelegate(context, playerPos, m_Callback, m_Delay, m_Type, m_ActionHandler,true);
		}

		// 返回false, 自动删除
		return false;
	}

	static void AddActionDelegate(GameContext *context, uchar playerPos, PlayerActionDelegateCallStateback callback, 
		float delay, int type, T *handler, bool replace = false)
	{
		// 设置超时自动决定拦牌选项
		OBJ_GET_EXT(context, ExtensionPlayerActionDelegate, extPlayerActDelegate);
		extPlayerActDelegate->AddAction(playerPos, type, 
			delay,
			callback, handler, replace);
	}

	static RefCntObj<PlayerActionDelegateEnabledListener> Create(T *handler, PlayerActionDelegateCallStateback callback, float delay, int type)
	{
		return RefCntObj<PlayerActionDelegateEnabledListener>(new PlayerActionDelegateListener(handler, callback, delay, type));
	}
};

#endif