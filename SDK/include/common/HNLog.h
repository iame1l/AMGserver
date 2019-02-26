/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#ifndef HN_LOG_HEAD_FILE
#define HN_LOG_HEAD_FILE

_declspec(dllexport) void DebugPrintf(const char *name,int iRoomID,const char *p, ...);


#define HNLOG_M( fmt, ...) DebugPrintf("Mserver", 0, fmt,  ##__VA_ARGS__)
#define HNLOG_AFC( fmt, ...) DebugPrintf("AFCComEx", 0, fmt,  ##__VA_ARGS__)
#define HNLOG_G(id, fmt, ...) DebugPrintf("Gserver", id, fmt,  ##__VA_ARGS__)

#endif