
#ifndef __PLATGLOBAL_DATA_H__
#define __PLATGLOBAL_DATA_H__

#include <queue>
#include "Platconfig.h"

#define				 MAX_GAME_ROOM				500			//最大房间数
//游戏房间信息
struct RoomInfoStruct
{
    //结构变量
    bool							bAccess;							//访问标志
    UINT							uTabIndex;							//房间索引
    DWORD							dwGameMSVer;						//主版本号
    DWORD							dwGameLSVer;						//副版本号
    CBaseRoom						* pGameRoomWnd;						//房间指针
    TCHAR							szProcessName[31];					//进程名字
    ComRoomInfo						stComRoomInfo;						//设置数据
    void operator=(RoomInfoStruct & paiData)
    {
        bAccess = paiData.bAccess;
        uTabIndex = paiData.uTabIndex;
        dwGameMSVer = paiData.dwGameMSVer;
        dwGameLSVer = paiData.dwGameLSVer;
        pGameRoomWnd = paiData.pGameRoomWnd;
        memcpy(&stComRoomInfo,&paiData.stComRoomInfo,sizeof(stComRoomInfo));
        memcpy(&szProcessName,&paiData.szProcessName,sizeof(szProcessName));
    }
};

struct TRobotItem
{
    int iUserID;
    RoomInfoStruct* pRoomItem;
public:
    TRobotItem()
    {
        ZeroMemory(this, sizeof(TRobotItem));
    }
};




// 全局数据信息
class CGlobalData
{
public:
    static CGlobalData*getInstance();
    virtual ~CGlobalData();

    BYTE									bConnectToWitchServer; // 0-未请求全参数配置,1-已请求全局参数配置
    CString									MainServerIPAddr; // MServer IP地址
    int										MainServerPort; // MServer 端口
    CGameListCtrl							GameList; //游戏列表
    RoomInfoStruct							RoomInfo[MAX_GAME_ROOM];    //房间信息,每个机器人对应一个房间信息
    TRobotItem								Robots[MAX_GAME_ROOM];
    DynamicConfig                           CurDynamicConfig; // 正在执行的任务配置

	bool									bLoginEnable;

    void ReInit();

private:
    CGlobalData();
    CGlobalData(const CGlobalData&);
    CGlobalData &operator=(const CGlobalData&);
};

#define GLOBALDATA CGlobalData::getInstance()

// UI展示信息
struct UserItemUIInfo
{
    DynamicConfig cof; // 任务信息
    string strUserName;
    string strNickName;
    int GLoginStatus; // GServer登陆状态 0-未登陆,1-正在登陆,2-成功登陆,3-登陆失败
    bool bRobot; // 是否为机器人
    int roomID;
    BYTE bDeskNo; // 桌子号
    BYTE bDeskStation; // 座位号
    BYTE bUserState; // 游戏状态
    UserItemUIInfo()
    {
        roomID = 0;
        strNickName = "";
        GLoginStatus = 0;
        bRobot = true;
        bDeskNo = 255;
        bDeskStation = 255;
        bUserState = USER_NO_STATE;
    }
};

struct UILogInfos
{
    int type;
    string Msg;
};
class UIInfo
{
public:
    ~UIInfo();
    static UIInfo*getInstance();
    CRITICAL_SECTION uiSection;
    ComRoomInfo roomInfo;
    std::map<string, UserItemUIInfo> usrInfos; // 用户名为key

    CRITICAL_SECTION LogMServerSection;
    std::queue<UILogInfos> LogMServerInfos;

    CRITICAL_SECTION LogGServerSection;
    std::queue<UILogInfos> LogGServerInfos;

    void ReInit();
private:
    UIInfo();
    UIInfo(const UIInfo&);
    UIInfo &operator=(const UIInfo&);
};

#define UIDATA UIInfo::getInstance()

#endif // __PLATGLOBAL_DATA_H__