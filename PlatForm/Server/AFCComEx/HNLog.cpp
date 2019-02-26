
#include "Stdafx.h"
#include "HNLog.h"


char *GetAppPathEx()
{
	static char s_Path[MAX_PATH];
	static bool s_bIsReady = false;
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

EXT_CLASS void DebugPrintf(const char *name,int iRoomID,const char *p, ...)
{

	char szTimeStr[32];
	char szDateStr[32];
	char szFilename[256];
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	GetTimeFormatA( LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &sysTime, "HH:mm:ss", szTimeStr, 32);
	GetDateFormatA( LOCALE_USER_DEFAULT, LOCALE_USE_CP_ACP, &sysTime, "yyyy-MM-dd", szDateStr, 32);
	if (0 == iRoomID)
	{
		wsprintf( szFilename, TEXT("%s\\HNLog\\%s_%s.txt"), GetAppPathEx(), name,szDateStr);
	}
	else
	{
		wsprintf( szFilename, TEXT("%s\\HNLog\\%s_%d_%s.txt"), GetAppPathEx(),name,iRoomID,szDateStr);
	}
	
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
