#ifndef PLAYER_ACTION_DELEGATE_H
#define PLAYER_ACTION_DELEGATE_H

#include "GameContext.h"
#include "ExtensionTimer.h"
#include <map>
#include "GameDefine.h"
typedef void (*PlayerActionDelegateCallStateback)(GameContext *context, int type, uchar playerPos, void *data);

struct DelegateInfo
{
	uchar playerPos;
	int actionType;
	TimerData data;
	PlayerActionDelegateCallStateback callback;
	void *otherData;
};

// 托管开启/关闭的监听器
class PlayerActionDelegateEnabledListener
{
public:
	virtual ~PlayerActionDelegateEnabledListener(){}

	virtual bool OnEnable(GameContext *context, uchar playerPos, bool b) = 0;
};


class ExtensionPlayerActionDelegate : public GameContextExtension
{
	typedef std::map<int, DelegateInfo*> ActionDelegateInfo;
	typedef std::map<uchar, ActionDelegateInfo> PlayerActionDelegateInfo;
	typedef std::map<uchar, RefCntObj<PlayerActionDelegateEnabledListener> > ListenerList;

	PlayerActionDelegateInfo m_DelegateInfo;

	void EraseDelegateInfo(DelegateInfo *info);

	ListenerList m_EnabledListeners;

public:

	// 设定玩家超时行为代理
	void AddAction(uchar playerPos, int type, float time, PlayerActionDelegateCallStateback callback, void *data = NULL, bool replace = false);

	// 通知玩家行为代理，有玩家作出相应了
	void OnPlayerResponse(uchar playerPos, int type);

	// 删除玩家超时行为代理
	void RemoveAction(uchar playerPos, int type);

	void RemoveAll();

	void AddListener(uchar playerPos, const RefCntObj<PlayerActionDelegateEnabledListener> &listener);

	void RemoveListener(uchar playerPos);

	void OnEnableActionDelegate(uchar playerPos, bool b);

	void OnGameStart();

	void OnGameFinish();

	void OnGameReset();
};

#endif