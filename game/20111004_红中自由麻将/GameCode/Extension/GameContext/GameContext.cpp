#include "GameContext.h"

#define IterateExtensionsToDo(func) {\
    for(ExtensionList::iterator itr = m_Extensions.begin();\
        itr != m_Extensions.end();\
        ++itr)\
    {\
        func(((GameContextExtension *)itr->second));\
    }\
}

#define IterateExtensionsToCall(func) {\
    for(ExtensionList::iterator itr = m_Extensions.begin();\
        itr != m_Extensions.end();\
        ++itr)\
    {\
        ((GameContextExtension *)itr->second)->func;\
    }\
}

GameContext::GameContext() :m_PlayerCount(4){}

GameContext::~GameContext(){}

int GameContext::GetPlayerCount() const
{
    return m_PlayerCount;
}

void GameContext::SetPlayerCount(int cnt)
{
	m_PlayerCount = cnt;
}

void GameContext::OnAddExtension(Extension *ext)
{
    GameContextExtension *gameCtxExt = (GameContextExtension *)ext;
    gameCtxExt->SetContext(this);
    gameCtxExt->OnCreate();
}

void GameContext::OnRemoveExtension(Extension *ext)
{
    GameContextExtension *gameCtxExt = (GameContextExtension *)ext;
    gameCtxExt->OnDestroy();
}


void GameContext::GameStart()
{
    IterateExtensionsToCall(OnGameStart());
}

void GameContext::GameFinish()
{
    IterateExtensionsToCall(OnGameFinish());
}

void GameContext::Reset()
{
    IterateExtensionsToCall(OnGameReset());
}
