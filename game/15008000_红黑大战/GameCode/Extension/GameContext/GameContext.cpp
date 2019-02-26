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

GameContext::GameContext(){}

GameContext::~GameContext(){}

void GameContext::SetGameDesk(CServerGameDesk *desk)
{
	m_pDesk = desk;
}

CServerGameDesk* GameContext::GetGameDesk()
{
	return m_pDesk;
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
