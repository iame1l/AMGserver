#include "stdafx.h"
#include "Platglobaldata.h"
#include "Platlock.h"

CGlobalData*CGlobalData::getInstance()
{
    static CGlobalData s_config;
    return &s_config;
}

CGlobalData::CGlobalData()
{
    ReInit();
}

CGlobalData::~CGlobalData()
{
    for (int i=0; i<MAX_GAME_ROOM; i++)
    {
        if (RoomInfo[i].pGameRoomWnd != NULL)
        {
            delete RoomInfo[i].pGameRoomWnd;
            RoomInfo[i].pGameRoomWnd = NULL;
        }
    }
}

UIInfo*UIInfo::getInstance()
{
    static UIInfo s_data;
    return &s_data;
}

UIInfo::UIInfo()
{
    roomInfo.uRoomID = 0;
    InitializeCriticalSection((&uiSection));
    InitializeCriticalSection(&LogMServerSection);
    InitializeCriticalSection(&LogGServerSection);
}
UIInfo::~UIInfo()
{
    DeleteCriticalSection((&uiSection));
    DeleteCriticalSection(&LogMServerSection);
    DeleteCriticalSection(&LogGServerSection);
}

void CGlobalData::ReInit()
{
    CurDynamicConfig.nLogonCount = -1;
    CurDynamicConfig.strTime = "";
    bConnectToWitchServer = 255;
    MainServerIPAddr = "127.0.0.1";
    MainServerPort = 3015;
    /*for (int i=0; i<MAX_GAME_ROOM; i++)
    {
        if (RoomInfo[i].pGameRoomWnd != NULL)
        {
            delete RoomInfo[i].pGameRoomWnd;
            RoomInfo[i].pGameRoomWnd = NULL;
        }
    }
    for (int i=0; i<MAX_GAME_ROOM; i++)
    {
        if (Robots[i].pRoomItem != NULL)
        {
            Robots[i].pRoomItem = NULL;
        }
    }*/
}

void UIInfo::ReInit()
{
    AutoPlatLock lock(&uiSection);
    usrInfos.clear();
    roomInfo.uRoomID = 0;
}