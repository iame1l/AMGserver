#ifndef __PLATLOG_H__
#define __PLATLOG_H__

#pragma warning( disable: 4251 )

#include <string>
#include <queue>

// 写日志接口
class CPlatLog
{
public:
    static CPlatLog *getInstacne();
    void WriteLog(const char*szFormat, ...); // 写日志
    
    bool Init();
    void UnInit();
private:
    bool m_bInit;
    HANDLE m_logEvent;
    HANDLE m_logExitEvent;
    CRITICAL_SECTION m_logSection;
    std::queue<std::string> m_strLogs; // 日志信息
    volatile bool m_bEnd; // 结束标志
    CPlatLog();
    CPlatLog(const CPlatLog&);
    CPlatLog& operator = (const CPlatLog&);
    ~CPlatLog();
    static unsigned int __stdcall WriteFile(void*); // 实际写日志的函数
};
#define filename(x) strrchr(x,'\\')?strrchr(x,'\\')+1:x

#define PLATLOG CPlatLog::getInstacne()

#define PLATROBOT_LOG(p, x, ...) \
{\
    char szFormat[1024] = { 0 }; \
    sprintf_s(szFormat, sizeof(szFormat), "[%d]%s:%s:%d:[%s] %s", GetCurrentThreadId(), filename(__FILE__), __FUNCTION__, __LINE__, p, x); \
    PLATLOG->WriteLog(szFormat, ##__VA_ARGS__); \
}

#define MSERVER_LOG_INFO(x, ...) PLATROBOT_LOG("IM", x, ##__VA_ARGS__)

#define MSERVER_LOG_ERROR(x, ...) PLATROBOT_LOG("EM", x, ##__VA_ARGS__)

#define MSERVER_LOG_TEST(x,...) PLATROBOT_LOG("TM", x, ##__VA_ARGS__)

#define GSERVER_LOG_INFO(x, ...) PLATROBOT_LOG("IG", x, ##__VA_ARGS__)

#define GSERVER_LOG_ERROR(x, ...) PLATROBOT_LOG("EG", x, ##__VA_ARGS__)

#define GSERVER_LOG_TEST(x,...) PLATROBOT_LOG("TG", x, ##__VA_ARGS__)

#endif // __PLATLOG_H__