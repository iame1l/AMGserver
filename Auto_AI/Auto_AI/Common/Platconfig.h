#ifndef __PLATCONFIG_H__
#define __PLATCONFIG_H__


class DynamicConfig
{
public:
    CString strTime; // 任务时间
    //  动态配置
    int nLogonCount; // 登陆账号个数
    CString strRobotPreName; // 机器人名称前缀
    int nBeginSequenceNo; // 起始序列号,与机器人名称前缀组合
    __int64 nCheckOutMinMoney; // 取钱下限
    __int64 nCheckInMaxMoney; // 存钱上限
    BYTE bMachineDeskCount;//每桌机器人个数
    bool bMachineAndPlayer; //机器人与玩家是否同桌
    int  nFishGameTimeLeave; // 捕鱼游戏占桌时长
    int  nKeepInDeskSeconds; // 占座位时长,单位秒
    int  nGameEndLeaveDesk; // 游戏结束站起概率

    bool operator == (const DynamicConfig & other)
    {
        return strTime == other.strTime && nLogonCount == other.nLogonCount && strRobotPreName == other.strRobotPreName
            && nBeginSequenceNo == other.nBeginSequenceNo && nCheckInMaxMoney == other.nCheckInMaxMoney && nCheckOutMinMoney == other.nCheckOutMinMoney
            && bMachineAndPlayer == other.bMachineAndPlayer && bMachineDeskCount == other.bMachineDeskCount && nFishGameTimeLeave == other.nFishGameTimeLeave
            && nKeepInDeskSeconds == other.nKeepInDeskSeconds && nGameEndLeaveDesk == other.nGameEndLeaveDesk;
    }
    bool operator != (const DynamicConfig & other)
    {
        return strTime != other.strTime || nLogonCount != other.nLogonCount || strRobotPreName != other.strRobotPreName
            || nBeginSequenceNo != other.nBeginSequenceNo || nCheckInMaxMoney != other.nCheckInMaxMoney || nCheckOutMinMoney != other.nCheckOutMinMoney
            || bMachineAndPlayer != other.bMachineAndPlayer || bMachineDeskCount != other.bMachineDeskCount || nFishGameTimeLeave != other.nFishGameTimeLeave
            || nKeepInDeskSeconds != other.nKeepInDeskSeconds || nGameEndLeaveDesk != other.nGameEndLeaveDesk;
    }
    bool IsValid(CString &errorMsg);
};

// 机器人通用配置文件读写类
class CPlatConfig 
{
public:
    static CPlatConfig*getInstance();
    virtual ~CPlatConfig();

    //  静态配置
    CString strLogFile; // 日志文件名
    CString strTitleText;// 标题名
    int KindID; // 游戏类型
    int NameID; // 游戏ID
    int RoomID; // 房间ID
    CString	CenterServerIPAddr; // AServer IP地址
    int		CenterServerPort; // AServer 端口
    CString strAIPWD; // AI登陆密码
    bool bFishGame; // 是否为捕鱼类游戏

    CRITICAL_SECTION m_DynamicSection;
    std::map<string, DynamicConfig> m_DynamicConfigs; // 动态配置

    void Save();
private:
    CPlatConfig();
    CPlatConfig(const CPlatConfig&);
    CPlatConfig &operator=(const CPlatConfig&);
};

#define PLATCONFIG CPlatConfig::getInstance()

#endif