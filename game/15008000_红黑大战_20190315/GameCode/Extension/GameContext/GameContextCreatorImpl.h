#ifndef GAME_CONTEXT_CREATOR_IMPL_H
#define GAME_CONTEXT_CREATOR_IMPL_H

#include "GameContext.h"

class GameContextCreatorImpl
{
public:
    virtual ~GameContextCreatorImpl(){}
    virtual GameContext *Create() = 0;
    virtual void Destroy(GameContext *context) = 0;
};

#endif
