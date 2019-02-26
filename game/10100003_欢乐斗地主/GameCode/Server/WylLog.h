#ifndef _WYLLOG_H
#define _WYLLOG_H

//#include "commonuse.h"
#include <windows.h>

static int g_iWylDebug = 1;
static CString g_strOutput;

//外部申明
void WylOutputArray(BYTE array[], const int size);
void WylOutputCstring(CString strIn);
void WylDebugPrintf(int roomid, int deskid, const char *p, ...);
void WylDebugPrintfArray(int roomid, int deskid, BYTE byArray[], const int iSize);

//外部申明结束

static void WylOutputCstring(CString strIn)
{
	if (g_iWylDebug != 1)
	{
		return;
	}
	CString strOut = "traceWYL		";
	strOut += strIn;
	OutputDebugString(strOut);
}

static void WylOutputArray(BYTE array[], const int size)
{
	if (g_iWylDebug != 1)
	{
		return;
	}
	g_strOutput = "traceWYL		";
	for (int i = 0; i < size; i++)
	{
		CString strTemp = "";
		strTemp.Format(",%d", array[i]);
		g_strOutput += strTemp;
	}
	OutputDebugString(g_strOutput);
}

static char *G_GetAppPath()
{
	static char s_Path[MAX_PATH];
	bool s_bIsReady = false;
	if (!s_bIsReady)
	{
		ZeroMemory(s_Path,sizeof(s_Path));
		DWORD dwLength=GetModuleFileNameA(GetModuleHandle(NULL), s_Path, sizeof(s_Path));
		char *p = strrchr(s_Path, '\\');
		if(p != NULL)	*p = '\0';
		s_bIsReady = true;
	}
	return s_Path;
}
/// 打印调试信息带日期
static void WylDebugPrintf(int roomid, int deskid, const char *p, ...)
{
	if (g_iWylDebug != 1)
	{
		return;
	}
	CString strAppPath = G_GetAppPath();
	CString strPath = strAppPath +"\\WylLog\\";
	SHCreateDirectoryEx(NULL, strPath, NULL);
	char szTimeStr[32];
	char szDateStr[32];
	char szFilename[256];
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	GetTimeFormatA( LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &sysTime, "HH:mm:ss", szTimeStr, 32);
	GetDateFormatA( LOCALE_USER_DEFAULT, LOCALE_USE_CP_ACP, &sysTime, "yyyy-MM-dd", szDateStr, 32);
	wsprintf( szFilename, TEXT("%s\\WylLog\\%s_%d(%d)_ServerManage.txt"), G_GetAppPath(), szDateStr, roomid, deskid);

	va_list arg;
	va_start( arg, p );
	FILE *fp = NULL;
	fp = fopen(szFilename, "a" );
	if (NULL == fp)
	{
		return;
	}
	fprintf(fp,"[%s %s]--", szDateStr, szTimeStr);
	vfprintf(fp, p, arg );
	fprintf(fp,"\n");
	fclose(fp);
}

/// 打印调试信息带日期
static void WylDebugPrintfArray(int roomid, int deskid, BYTE byArray[], const int iSize)
{
	if (g_iWylDebug != 1)
	{
		return;
	}
	g_strOutput = "";
	for (int i = 0; i < iSize; i++)
	{
		CString strTemp = "";
		strTemp.Format(",%d", byArray[i]);
		g_strOutput += strTemp;
	}
	char p[1024] = {0};
	strcpy_s(p, 1024, g_strOutput);

	CString strAppPath = G_GetAppPath();
	CString strPath = strAppPath +"\\WylLog\\";
	SHCreateDirectoryEx(NULL, strPath, NULL);
	char szTimeStr[32];
	char szDateStr[32];
	char szFilename[256];
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	GetTimeFormatA( LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &sysTime, "HH:mm:ss", szTimeStr, 32);
	GetDateFormatA( LOCALE_USER_DEFAULT, LOCALE_USE_CP_ACP, &sysTime, "yyyy-MM-dd", szDateStr, 32);
	wsprintf( szFilename, TEXT("%s\\WylLog\\%s_%d(%d)_ServerManage.txt"), G_GetAppPath(), szDateStr, roomid, deskid);

	va_list arg;
	va_start( arg, p );
	FILE *fp = NULL;
	fp = fopen(szFilename, "a" );
	if (NULL == fp)
	{
		return;
	}
	fprintf(fp,"[%s %s]--", szDateStr, szTimeStr);
	vfprintf(fp, p, arg );
	fprintf(fp,"\n");
	fclose(fp);
}

#endif
