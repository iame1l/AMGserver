#include "GameContextCreator.h"
#include <cstddef>

GameContextCreatorImpl *GameContextCreator::CreatorImpl = NULL;

GameContext *GameContextCreator::Create()
{
    if(CreatorImpl) return CreatorImpl->Create();
    return CreateExtObj(GameContext);
}

void GameContextCreator::Destroy(GameContext *context)
{
    if(CreatorImpl) CreatorImpl->Destroy(context);
    DestroyExtObj(context);
}
