/****************************************************************************
Copyright (c) 2014-2017 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2017 ShenZhen Redbird Network Polytron Technologies Inc.
 
http://www.hotniao.com

All of the content of the software, including code, pictures, 
resources, are original. For unauthorized users, the company 
reserves the right to pursue its legal liability.
****************************************************************************/
#ifndef HNLOG_H
#define HNLOG_H

class HNlog
{
public:
	//打印调试信息
	static void DebugPrintf_debug(int iRoomid,int iDeskIndex,const char *p, ...)
	{
		char szFilename[256];
		CString strGame;
		strGame.Format("%d\\",NAME_ID);
		CString strPath = CINIFile::GetAppPath() +"GameLog\\"+strGame+"Debug\\";
		SHCreateDirectoryEx(NULL, strPath, NULL);
		CTime time = CTime::GetCurrentTime();
		sprintf_s(szFilename,sizeof(szFilename)/sizeof(szFilename[0]), 
			"%s[%d-%d]_%d-%d-%d-%d.log",strPath ,iRoomid,iDeskIndex,time.GetYear(), 
			time.GetMonth(), time.GetDay(),time.GetHour());
		FILE *fp;
		errno_t err = fopen_s(&fp,szFilename, "a+" );
		if (NULL != err)
		{
			return;
		}
		//插入时间
		char cTime[30];
		::memset(cTime,0,sizeof(cTime));
		sprintf_s(cTime,sizeof(cTime)/sizeof(cTime[0]),"[%d:%d:%d] ",
			time.GetHour(),time.GetMinute(),time.GetSecond());
		fprintf(fp,cTime);
		va_list arg;
		va_start( arg, p );
		vfprintf(fp, p, arg );
		fprintf(fp,"\n");
		fclose(fp);
	}

	// 打印错误信息
	static void ErrorPrintf_debug(int iRoomid,int iDeskIndex,const char *p, ...)
	{
		char szFilename[256];
		CString strGame;
		strGame.Format("%d\\",NAME_ID);
		CString strPath = CINIFile::GetAppPath() +"GameLog\\"+strGame+"Error\\";
		SHCreateDirectoryEx(NULL, strPath, NULL);
		CTime time = CTime::GetCurrentTime();
		sprintf_s(szFilename,sizeof(szFilename)/sizeof(szFilename[0]), 
			"%s[%d-%d]_%d-%d-%d.log",strPath,iRoomid,iDeskIndex,time.GetYear(), 
			time.GetMonth(), time.GetDay());
		FILE *fp;
		errno_t err = fopen_s(&fp,szFilename, "a+" );
		if (NULL != err)
		{
			return;
		}
		//插入时间
		char cTime[30];
		::memset(cTime,0,sizeof(cTime));
		sprintf_s(cTime,sizeof(cTime)/sizeof(cTime[0]),"[%d:%d:%d] ",
			time.GetHour(),time.GetMinute(),time.GetSecond());
		fprintf(fp,cTime);
		va_list arg;
		va_start( arg, p );
		vfprintf(fp, p, arg );
		fprintf(fp,"\n");
		fclose(fp);
	}

	/// 打印录像信息
	static void StatePrintf_debug(int iRoomid,int iDeskIndex,const char *p, ...)
	{
		char szFilename[256];
		CString strGame;
		strGame.Format("%d\\",NAME_ID);
		CString strPath = CINIFile::GetAppPath() +"GameLog\\"+strGame+"State\\";
		SHCreateDirectoryEx(NULL, strPath, NULL);
		CTime time = CTime::GetCurrentTime();
		sprintf_s(szFilename,sizeof(szFilename)/sizeof(szFilename[0]), 
			"%s[%d-%d]_%d-%d-%d-%d.log",strPath,iRoomid,iDeskIndex,time.GetYear(), 
			time.GetMonth(), time.GetDay(),time.GetHour());
		FILE *fp;
		errno_t err = fopen_s(&fp,szFilename, "a+" );
		if (NULL != err)
		{
			return;
		}
		//插入时间
		char cTime[30];
		::memset(cTime,0,sizeof(cTime));
		sprintf_s(cTime,sizeof(cTime)/sizeof(cTime[0]),"[%d:%d:%d] ",
			time.GetHour(),time.GetMinute(),time.GetSecond());
		fprintf(fp,cTime);
		va_list arg;
		va_start( arg, p );
		vfprintf(fp, p, arg );
		fprintf(fp,"\n");
		fclose(fp);
	}

	/// 打印调试信息
	static void LBWOutputDebugString_debug(const char *lpszFormat, ...)
	{
		va_list pArg;
		va_start(pArg, lpszFormat);
		char szMessage[1024] = { 0 };
		_vsnprintf_s(szMessage, 1023, lpszFormat, pArg);
		va_end(pArg);
		OutputDebugStringA(szMessage);
	}
};

#define debuglog	HNlog::DebugPrintf_debug
#define errorlog	HNlog::ErrorPrintf_debug 
#define recordlog	HNlog::StatePrintf_debug
#define dbgview		HNlog::LBWOutputDebugString_debug

#endif 