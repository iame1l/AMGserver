#include "stdafx.h"
#include "Platconfig.h"

#define CONFIG_FILENAME ".\\RobotSet.ini"
#define STATIC_SECTION "Static"
#define DYNAMIC_SECTION "Dynamic"

bool DynamicConfig::IsValid(CString &errorMsg)
{
    if((nCheckInMaxMoney != 0 && nCheckOutMinMoney != 0 && nCheckInMaxMoney < nCheckOutMinMoney + 1000)
    || nCheckInMaxMoney < 0 || nCheckOutMinMoney < 0)
    {
        errorMsg.Format("存取钱%lld-%lld设置错误(至少相隔1000金币)",nCheckOutMinMoney,nCheckInMaxMoney);
        return false;
    }
    if(strRobotPreName.GetLength() > 30)
    {
        errorMsg.Format("机器人名称前缀过长%s",strRobotPreName);
        return false;
    }
    if(nLogonCount < 0)
    {
        errorMsg.Format("登陆人数设置错误%d",nLogonCount);
        return false;
    }
    if(nBeginSequenceNo < 0)
    {
        errorMsg.Format("起始序列设置错误%d",nBeginSequenceNo);
        return false;
    }
    if(bMachineDeskCount <= 0)
    {
        errorMsg.Format("每桌机器人个数设置错误%d",bMachineDeskCount);
        return false;
    }
    if(nFishGameTimeLeave <= 0)
    {
        errorMsg.Format("捕鱼游戏占桌时长这是错误%d",nFishGameTimeLeave);
        return false;
    }
    if(nKeepInDeskSeconds <= 15)
    {
        errorMsg.Format("等待游戏开始时长设置错误(最少15秒)%d",nKeepInDeskSeconds);
        return false;
    }
    if(nGameEndLeaveDesk < 0 || nGameEndLeaveDesk > 100)
    {
        errorMsg.Format("游戏结束站起概率设置错误(0-100)%d",nGameEndLeaveDesk);
        return false;
    }
    return true;
}

CPlatConfig*CPlatConfig::getInstance()
{
    static CPlatConfig s_config;
    return &s_config;
}
CPlatConfig::~CPlatConfig()
{
    Save();
    DeleteCriticalSection(&m_DynamicSection);
}

void CPlatConfig::Save()
{
    TCHAR szTmp[256]= {0};

    auto it = m_DynamicConfigs.begin();
    int i = 0;
    while(it != m_DynamicConfigs.end())
    {
        CString key;
        key.Format("Time_%d",i);
        sprintf_s(szTmp, sizeof(szTmp), _T("%s"), it->second.strTime);
        WritePrivateProfileString(DYNAMIC_SECTION, key, szTmp, CONFIG_FILENAME);
        ZeroMemory(szTmp, sizeof(szTmp)); // 任务启动时间
        key.Format("LogonCount_%d",i);
        sprintf_s(szTmp, sizeof(szTmp), _T("%d"), it->second.nLogonCount);
        WritePrivateProfileString(DYNAMIC_SECTION, key, szTmp, CONFIG_FILENAME);
        ZeroMemory(szTmp, sizeof(szTmp));// 登陆人数
        key.Format("RobotPreName_%d",i);
        sprintf_s(szTmp, sizeof(szTmp), _T("%s"), it->second.strRobotPreName);
        WritePrivateProfileString(DYNAMIC_SECTION, key, szTmp, CONFIG_FILENAME);
        ZeroMemory(szTmp, sizeof(szTmp)); // 机器人名称前缀
        key.Format("BeginSequenceNo_%d",i);
        sprintf_s(szTmp, sizeof(szTmp), _T("%d"), it->second.nBeginSequenceNo);
        WritePrivateProfileString(DYNAMIC_SECTION, key, szTmp, CONFIG_FILENAME);
        ZeroMemory(szTmp, sizeof(szTmp));// 登陆起始序列号
        key.Format("CheckOutMinMoney_%d",i);
        sprintf_s(szTmp, sizeof(szTmp), _T("%d"), it->second.nCheckOutMinMoney);
        WritePrivateProfileString(DYNAMIC_SECTION, key, szTmp, CONFIG_FILENAME);
        ZeroMemory(szTmp, sizeof(szTmp));// 取钱下限
        key.Format("CheckInMaxMoney_%d",i);
        sprintf_s(szTmp, sizeof(szTmp), _T("%d"), it->second.nCheckInMaxMoney);
        WritePrivateProfileString(DYNAMIC_SECTION, key, szTmp, CONFIG_FILENAME);
        ZeroMemory(szTmp, sizeof(szTmp));// 存钱上限
        key.Format("MachineDeskCount_%d",i);
        sprintf_s(szTmp, sizeof(szTmp), _T("%d"), it->second.bMachineDeskCount);
        WritePrivateProfileString(DYNAMIC_SECTION, key, szTmp, CONFIG_FILENAME);
        ZeroMemory(szTmp, sizeof(szTmp));// 每桌机器人个数
        key.Format("MachineAndPlayer_%d",i);
        sprintf_s(szTmp, sizeof(szTmp), _T("%d"), it->second.bMachineAndPlayer);
        WritePrivateProfileString(DYNAMIC_SECTION, key, szTmp, CONFIG_FILENAME);
        ZeroMemory(szTmp, sizeof(szTmp));// 机器人与玩家是否同桌
        key.Format("FishGameTimeLeave_%d",i);
        sprintf_s(szTmp, sizeof(szTmp), _T("%d"), it->second.nFishGameTimeLeave);
        WritePrivateProfileString(DYNAMIC_SECTION, key, szTmp, CONFIG_FILENAME);
        ZeroMemory(szTmp, sizeof(szTmp));// 捕鱼游戏占座时长,单位分钟
        key.Format("KeepInDeskSeconds_%d",i);
        sprintf_s(szTmp, sizeof(szTmp), _T("%d"), it->second.nKeepInDeskSeconds);
        WritePrivateProfileString(DYNAMIC_SECTION, key, szTmp, CONFIG_FILENAME);
        ZeroMemory(szTmp, sizeof(szTmp));// 占座位时长,单位秒
        key.Format("GameEndLeaveDesk_%d",i);
        sprintf_s(szTmp, sizeof(szTmp), _T("%d"), it->second.nGameEndLeaveDesk);
        WritePrivateProfileString(DYNAMIC_SECTION, key, szTmp, CONFIG_FILENAME);
        ZeroMemory(szTmp, sizeof(szTmp));// 游戏结束站起概率
        i++;
        it++;
    }
    sprintf_s(szTmp, sizeof(szTmp), _T("%d"), i);
    WritePrivateProfileString(DYNAMIC_SECTION, "TaskNumber", szTmp, CONFIG_FILENAME);
}

CPlatConfig::CPlatConfig()
{
    InitializeCriticalSection(&m_DynamicSection);
    TCHAR szTmp[256]= {0};
    // 日志文件名
    GetPrivateProfileString(STATIC_SECTION, "LogFile", _T("RobotLog"), szTmp, sizeof(szTmp), CONFIG_FILENAME);
    strLogFile.Format(_T("%s"), szTmp);
    ZeroMemory(szTmp, sizeof(szTmp));
    // 标题名
    GetPrivateProfileString(STATIC_SECTION, "TitleText", _T("机器人"), szTmp, sizeof(szTmp), CONFIG_FILENAME);
    strTitleText.Format(_T("%s"), szTmp);
    ZeroMemory(szTmp, sizeof(szTmp));
    // 游戏类型
    KindID = GetPrivateProfileInt(STATIC_SECTION, "KindID", 1, CONFIG_FILENAME);
    // 游戏ID
    NameID = GetPrivateProfileInt(STATIC_SECTION, "NameID", 10100103, CONFIG_FILENAME);
    // 游戏ID
    RoomID = GetPrivateProfileInt(STATIC_SECTION, "RoomID", 1, CONFIG_FILENAME);
    // AServerIP地址
    GetPrivateProfileString(STATIC_SECTION, "CenterServerIPAddr", _T("127.0.0.1"), szTmp, sizeof(szTmp), CONFIG_FILENAME);
    CenterServerIPAddr.Format(_T("%s"), szTmp);
    ZeroMemory(szTmp, sizeof(szTmp));
    // AServer端口
    CenterServerPort = GetPrivateProfileInt(STATIC_SECTION, "CenterServerPort", 13025, CONFIG_FILENAME);
    // AI密码
    GetPrivateProfileString(STATIC_SECTION, "AIPWD", _T("6fc175a72fe0af5093a6e1dfb2c8d1d5"), szTmp, sizeof(szTmp), CONFIG_FILENAME);
    strAIPWD.Format(_T("%s"), szTmp);
    ZeroMemory(szTmp, sizeof(szTmp));
    // 是否为捕鱼类游戏
    bFishGame = GetPrivateProfileInt(STATIC_SECTION, "FishGame", 1, CONFIG_FILENAME) != 0?true:false;

    // 任务个数
    int nNumber = GetPrivateProfileInt(DYNAMIC_SECTION, "TaskNumber", 0, CONFIG_FILENAME);

    for(int i = 0; i < nNumber; ++i)
    {
        DynamicConfig cof;

        // 任务启动时间
        CString key;
        CString strMapKey;
        key.Format("Time_%d", i);
        GetPrivateProfileString(DYNAMIC_SECTION, key, _T("0000"), szTmp, sizeof(szTmp), CONFIG_FILENAME);
        strMapKey.Format(_T("%s"), szTmp);
        cof.strTime = strMapKey;
        ZeroMemory(szTmp, sizeof(szTmp));
        // 登陆游戏人数
        key.Format("LogonCount_%d", i);
        cof.nLogonCount = GetPrivateProfileInt(DYNAMIC_SECTION, key, 10, CONFIG_FILENAME);
        // 机器人名称前缀
        key.Format("RobotPreName_%d", i);
        GetPrivateProfileString(DYNAMIC_SECTION, key, _T("C011"), szTmp, sizeof(szTmp), CONFIG_FILENAME);
        cof.strRobotPreName.Format(_T("%s"), szTmp);
        ZeroMemory(szTmp, sizeof(szTmp));
        // 登陆起始序列号
        key.Format("BeginSequenceNo_%d", i);
        cof.nBeginSequenceNo = GetPrivateProfileInt(DYNAMIC_SECTION, key, 1001, CONFIG_FILENAME);
        // 取钱下限
        key.Format("CheckOutMinMoney_%d", i);
        cof.nCheckOutMinMoney = GetPrivateProfileInt(DYNAMIC_SECTION, key, 10000, CONFIG_FILENAME);
        // 存钱上限
        key.Format("CheckInMaxMoney_%d", i);
        cof.nCheckInMaxMoney = GetPrivateProfileInt(DYNAMIC_SECTION, key, 100000, CONFIG_FILENAME);
        // 每桌机器人个数
        key.Format("MachineDeskCount_%d", i);
        cof.bMachineDeskCount = GetPrivateProfileInt(DYNAMIC_SECTION, key, PLAY_COUNT, CONFIG_FILENAME);
        // 机器人与玩家是否同桌
        key.Format("MachineAndPlayer_%d", i);
        cof.bMachineAndPlayer = GetPrivateProfileInt(DYNAMIC_SECTION, key, 1, CONFIG_FILENAME) != 0?true:false;
        // 捕鱼游戏占桌时长,单位分钟
        key.Format("FishGameTimeLeave_%d", i);
        cof.nFishGameTimeLeave = GetPrivateProfileInt(DYNAMIC_SECTION, key, 30, CONFIG_FILENAME);
        // 占座位时长,单位秒
        key.Format("KeepInDeskSeconds_%d", i);
        cof.nKeepInDeskSeconds = GetPrivateProfileInt(DYNAMIC_SECTION, key, 30, CONFIG_FILENAME);
        // 游戏结束站起概率
        key.Format("GameEndLeaveDes_%d", i);
        cof.nGameEndLeaveDesk = GetPrivateProfileInt(DYNAMIC_SECTION, key, 30, CONFIG_FILENAME);
        m_DynamicConfigs[strMapKey.GetBuffer()] = cof;
    }    
}
