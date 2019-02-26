#include "stdafx.h"
#include "PlatLog.h"
#include "Platconfig.h"
#include "Platlock.h"
#include "Platglobaldata.h"
#include "shlobj.h "
#define MAX_MSG_NUMBER 200

CPlatLog *CPlatLog::getInstacne()
{
    static CPlatLog s_log;
    return &s_log;
}

CPlatLog::CPlatLog()
{
    m_bInit = false;
    m_bEnd = false;
    m_logEvent = INVALID_HANDLE_VALUE;
    m_logExitEvent = INVALID_HANDLE_VALUE;
}

CPlatLog::~CPlatLog()
{
    
}

bool CPlatLog::Init()
{
    if (m_bInit) return true;
    InitializeCriticalSection(&m_logSection);
    m_bEnd = false;
    std::queue<std::string> empty;
    std::swap(m_strLogs, empty);
    m_logEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_logExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    _beginthreadex(NULL, 0, WriteFile, this, 0, NULL);
    MSERVER_LOG_INFO("log thread start");
    m_bInit = true;
    return true;
}
void CPlatLog::UnInit()
{
    if (!m_bInit) return;
    MSERVER_LOG_INFO("log thread exit");
    m_bEnd = true;
    SetEvent(m_logEvent);
    WaitForSingleObject(m_logExitEvent, -1);
    CloseHandle(m_logEvent);
    m_logEvent = INVALID_HANDLE_VALUE;
    CloseHandle(m_logExitEvent);
    m_logExitEvent = INVALID_HANDLE_VALUE;
    DeleteCriticalSection(&m_logSection);
    std::queue<std::string> empty;
    std::swap(m_strLogs, empty);
    m_bInit = false;
}

void CPlatLog::WriteLog( const char*szFormat, ...)
{
    if (m_bEnd || !m_bInit) return;
    char szContext[1024] = { 0 };
    va_list arg;
    va_start(arg, szFormat);
    vsnprintf_s(szContext, sizeof(szContext), szFormat, arg);
    va_end(arg);
    SYSTEMTIME sysTime;
    GetLocalTime(&sysTime);
    char szLog[2048] = { 0 };
    sprintf_s(szLog, sizeof(szLog), "%04d-%02d-%02d %02d:%02d:%02d-%s", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, szContext);

    {
        AutoPlatLock lockLog(&m_logSection);
        m_strLogs.push(szLog);
    }
    SetEvent(m_logEvent);
}

unsigned int CPlatLog::WriteFile(void*p)
{
    CPlatLog *pLog = (CPlatLog*)p;
    while (!pLog->m_bEnd)
    {
        WaitForSingleObject(pLog->m_logEvent, 1000);
        while (pLog->m_strLogs.size() > 0)
        {
            std::string strLog = "";
            {
                AutoPlatLock lockLog(&pLog->m_logSection);
                strLog = pLog->m_strLogs.front();
                pLog->m_strLogs.pop();
            }
            SYSTEMTIME sysTime;
            GetLocalTime(&sysTime);
            char szTimeInfo[32] = {0};
            sprintf_s(szTimeInfo, "%04d-%02d-%02d %02d:%02d:%02d\r\n", sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
            int pos = strLog.find("[IM]");
            if( pos != -1)
            {
                AutoPlatLock lock(&UIDATA->LogMServerSection);
                UILogInfos loginfo;
                loginfo.type = 0;
                loginfo.Msg = string(szTimeInfo) + strLog.substr(pos,strLog.length());
                UIDATA->LogMServerInfos.push(loginfo);
                if(UIDATA->LogMServerInfos.size() > MAX_MSG_NUMBER) UIDATA->LogMServerInfos.pop();
            }
            else
            {
                pos = strLog.find("[EM]");
                if(pos != -1)
                {
                    AutoPlatLock lock(&UIDATA->LogMServerSection);
                    UILogInfos loginfo;
                    loginfo.type = 1;
                    loginfo.Msg = string(szTimeInfo) + strLog.substr(pos,strLog.length());
                    UIDATA->LogMServerInfos.push(loginfo);
                    if(UIDATA->LogMServerInfos.size() > MAX_MSG_NUMBER) UIDATA->LogMServerInfos.pop();
                }
                else
                {
                    pos = strLog.find("[IG]");
                    if(pos != -1)
                    {
                        AutoPlatLock lock(&UIDATA->LogGServerSection);
                        UILogInfos loginfo;
                        loginfo.type = 0;
                        loginfo.Msg = string(szTimeInfo) + strLog.substr(pos,strLog.length());
                        UIDATA->LogGServerInfos.push(loginfo);
                        if(UIDATA->LogGServerInfos.size() > MAX_MSG_NUMBER) UIDATA->LogGServerInfos.pop();
                    }
                    else
                    {
                        pos = strLog.find("[EG]");
                        if(pos != -1)
                        {
                            AutoPlatLock lock(&UIDATA->LogGServerSection);
                            UILogInfos loginfo;
                            loginfo.type = 1;
                            loginfo.Msg = string(szTimeInfo) + strLog.substr(pos,strLog.length());
                            UIDATA->LogGServerInfos.push(loginfo);
                            if(UIDATA->LogGServerInfos.size() > MAX_MSG_NUMBER) UIDATA->LogGServerInfos.pop();
                        }
                    }
                }
            }

			CString strPath = CINIFile::GetAppPath() +"Log";
			SHCreateDirectoryEx(NULL, strPath, NULL);

            if(PLATCONFIG->strLogFile == "NULL") continue;
            char szFileName[MAX_PATH] = { 0 };
            sprintf_s(szFileName, sizeof(szFileName), "%s/%s_%04d%02d%02d.txt",strPath.GetBuffer(strPath.GetLength()), PLATCONFIG->strLogFile, sysTime.wYear, sysTime.wMonth, sysTime.wDay);
            FILE *file = NULL; 
            int err = fopen_s(&file, szFileName, "a");
            if (file == NULL) break;
            fwrite(strLog.c_str(), strLog.length(), 1, file);
            fwrite("\r\n", 1, 2, file);
            fclose(file);
        } 
    }
    SetEvent(pLog->m_logExitEvent);
    return 0;
}