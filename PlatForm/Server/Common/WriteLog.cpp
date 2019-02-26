/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "stdafx.h"
#include "WriteLog.h"
#include "afx.h"
#include "Shlwapi.h"
#include "ATLComTime.h"
#include "commonuse.h"
  
static CString GetAppPath(bool bFource=true)
{
	bFource = true;
	if(!bFource)
	{
		char lpBuffer[MAX_STRING_SIZE];
		ZeroMemory(lpBuffer,sizeof(lpBuffer));
		GetCurrentDirectory(MAX_STRING_SIZE,lpBuffer);
		CString ss=lpBuffer;
		ss += "\\";
		return ss;
	}
	else
	{
		TCHAR szModuleName[MAX_PATH];
		ZeroMemory(szModuleName,sizeof(szModuleName));
		DWORD dwLength=GetModuleFileName(AfxGetInstanceHandle(),szModuleName,sizeof(szModuleName));
		CString filepath=szModuleName;
		CString path;
		int nSlash = filepath.ReverseFind(_T('/'));
		if (nSlash == -1)
			nSlash = filepath.ReverseFind(_T('\\'));
		if (nSlash != -1 && filepath.GetLength() > 1)
		{
			path = filepath.Left(nSlash+1);
			SetCurrentDirectory(path);
			return path;
		}
		else
		{
			char lpBuffer[MAX_STRING_SIZE];
			ZeroMemory(lpBuffer,sizeof(lpBuffer));
			GetCurrentDirectory(MAX_STRING_SIZE,lpBuffer);
			CString ss=lpBuffer;
			ss += "\\";
			return ss;
		}

	}
}


//删除
void Dellog(int iRoomID,int iSmallRoomID)
{
	TCHAR szFileName	[50];
	wsprintf(szFileName,"HNLog\\%d-%d.txt",iRoomID,iSmallRoomID);
	try
	{
		::DeleteFile(szFileName);
	}
	catch(...)
	{TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		return ;
	}
	return ;
}
//写正常日志
void WriteStr(TCHAR *str,int iRoomID,int iSmallRoomID)
{
	try
	{	
		CFile file;
		TCHAR szFileName	[50];
		TCHAR szNewLine[]="\r\n";
		wsprintf(szFileName,"HNLog\\%d-%d.txt",iRoomID,iSmallRoomID);
		file.Open(szFileName,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite);//建立新文件
		file.SeekToEnd();
		file.Write(szNewLine,strlen(szNewLine));			//换行	
		file.Write(str,strlen(str));						//写入游戏信息
		file.Close();
	}
	catch(CException *e)
	{		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		//AfxMessageBox("some exception!");
		e->Delete();
		return ; 
	}
}

void WriteStr(int iValue)
{
	TCHAR sz[200];
	wsprintf(sz,"iValue = %d",iValue);
	WriteStr(sz);
}

void WriteStr(long iValue)
{
	TCHAR sz[200];
	wsprintf(sz,"iValue = %ld",iValue);
	WriteStr(sz);
}
void  WriteError(TCHAR *pMessage ,int iRoomID,int iSmallRoomID )
{
	try
	{
		char FileName[50];
		TCHAR path[MAX_PATH];
	
        lstrcpy(path, GetAppPath());

		wsprintf(FileName,"%s\\HNLog\\Error%d-%d.txt\0",path,iRoomID,iSmallRoomID);
		CFile syslog;
		syslog.Open(FileName,CFile::modeNoTruncate| CFile::modeCreate |CFile::modeReadWrite);
		syslog.SeekToEnd();
		syslog.Write(pMessage,strlen(pMessage));

		syslog.SeekToEnd();
		TCHAR nextLine[] = "\r\n";
		syslog.Write(nextLine,strlen(nextLine));

		syslog.Close();
	}
	catch(CException *e)
	{		TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
		//AfxMessageBox("some exception!");
		e->Delete();
		return ; 
	}
}

void WriteLog(const char* pFormat, ...)
{
    FILE*  fp = NULL;
    char chFileName[64] = {0};

    CString strPath;
    char szPath[MAX_PATH];
    memset(szPath, 0, MAX_PATH);
    GetCurrentDirectory(MAX_PATH, szPath);
    strPath.ReleaseBuffer();
    strPath.Format("%s\\HNLog\\", szPath);
    CreateDirectory(strPath, NULL);


    SYSTEMTIME tm;
    GetLocalTime(&tm);
    sprintf(chFileName, "HNLog\\%d-%d-%d_%02d.txt", tm.wYear, tm.wMonth, tm.wDay, tm.wHour);

    fp = fopen(chFileName, "a+");
    if (NULL == fp)
    {
        return ;
    }
    fprintf(fp, "err = %d, ", GetLastError());

    va_list va;
    va_start(va, pFormat);
    vfprintf(fp, pFormat, va);
    vprintf(pFormat, va);
    va_end(va);

    fprintf(fp, " %u:%u:%u:%u %u年%u月%u日\n", tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds, tm.wYear, tm.wMonth, tm.wDay);
    printf(" %u:%u:%u:%u %u年%u月%u日\n", tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds, tm.wYear, tm.wMonth, tm.wDay);
    if (NULL != fp)
    {
        fclose(fp);
    }
}