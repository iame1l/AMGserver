#ifndef GAME_CONTEXT_CREATOR_H
#define GAME_CONTEXT_CREATOR_H

#include "GameContextCreatorImpl.h"

class GameContextCreator
{
public:
    static GameContextCreatorImpl *CreatorImpl;
    static GameContext *Create();
    static void Destroy(GameContext *context);
};

#endif
