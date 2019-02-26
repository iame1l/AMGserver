#ifndef GAME_CONTEXT_H
#define GAME_CONTEXT_H

#include "ExtensionObject.h"

class CServerGameDesk;

class GameContext : public ExtensionObject
{
protected:
	CServerGameDesk   *m_pDesk; //桌子类
public:

    GameContext();

    virtual ~GameContext();

	/*
	*@brief:桌子指针
	*@Returns:   void
	*@Parameter: CServerGameDesk * desk
	*/
	void SetGameDesk(CServerGameDesk *desk);
	CServerGameDesk* GetGameDesk();

    virtual void OnAddExtension(Extension *ext);

    virtual void OnRemoveExtension(Extension *ext);

    virtual void GameStart();

    virtual void GameFinish();

    virtual void Reset();
};

class GameContextExtension : public Extension
{
protected:
    GameContext *m_Context;
public:
    void SetContext(GameContext *context){m_Context = context;}
	GameContext * GetContext(){ return m_Context;}
    virtual void OnCreate(){}
    virtual void OnGameStart(){}
    virtual void OnGameFinish(){}
    virtual void OnGameReset(){}
    virtual void OnDestroy(){}
};

#endif
