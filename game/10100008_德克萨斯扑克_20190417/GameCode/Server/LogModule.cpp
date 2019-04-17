#include "StdAfx.h"
#include "LogModule.h"

//Added by QiWang 20171031,添加两个变量不至于一直检测文件存不存在
static bool gbDayInit = false;
static int giDay = 0;
#define NUM_MONTH_PER_YEAR	12//一年12个月

CLogModule CLogModule::m_pLogModule; 
CLogModule::CLogModule(void)
{

}

CLogModule::~CLogModule(void)
{
   
}

/// 日志
void CLogModule::WriteLog(const char *pPutFromat, ...)
{
	if(pPutFromat == NULL)
	{
		return;
	}
	char szFilename[256]={0};//确保是字符串
	CString strPath = CINIFile::GetAppPath() +"\\GameLog\\";
	SHCreateDirectoryEx(NULL, strPath, NULL);
	CTime time = CTime::GetCurrentTime();
	sprintf(szFilename, "%s%d_%d%02d%02d_UserLog.csv",strPath ,NAME_ID,time.GetYear(),time.GetMonth(),time.GetDay());
	FILE *fp = fopen( szFilename, "a" );
	if (NULL == fp)
	{
		return;
	}
	char s[1024]={0};//确保是字符串
	va_list arg_ptr;
	va_start(arg_ptr, pPutFromat);
	_vsnprintf(s, sizeof(s)-1,pPutFromat,arg_ptr);
	fprintf(fp,"%s",s);
	va_end(arg_ptr);
	fclose(fp);


	//Added by QiWang 20171031,删除一个月之前的日志
	static int iMonthLastDay[NUM_MONTH_PER_YEAR] = 
	{
		31,
		28,//闰年2月为29日
		31,
		30,
		31,
		30,
		31,
		31,
		30,
		31,
		30,
		31,
	};
	const int iDay = time.GetDay();
	if (!gbDayInit)
	{
		gbDayInit = true;
		giDay = iDay;
	} else 
	{
		if (giDay != iDay)
		{
			giDay = iDay;
		} else 
		{
			return; // 不需要检测，即返回
		}
	}
	int iYear = time.GetYear();
	const int iMonth = time.GetMonth();
	int iPreMonth = iMonth;
	
	if (iMonth > 1)
	{
		iPreMonth -= 1;
	} else 
	{
		iPreMonth = 12;
		iYear -= 1;
	}

	if (iPreMonth < 1 || iPreMonth > 12 || iMonth < 1 || iMonth > 12 || iDay < 1 || iDay > 31)
	{
		return;//预防错误
	}

	const int iIndexMonth = iMonth - 1;
	const int iIndexPreMonth = iPreMonth - 1;

	if (iIndexMonth < 0 || iIndexMonth >= NUM_MONTH_PER_YEAR)
	{
		return;//预防错误
	}
	if (iIndexPreMonth < 0 || iIndexPreMonth >= NUM_MONTH_PER_YEAR)
	{
		return;//预防错误
	}

	WIN32_FILE_ATTRIBUTE_DATA attrs = {0}; 
	if (iDay >= iMonthLastDay[iIndexMonth]) // 本月最后一天删除上个月遗留的所有日志
	{
		for (int iDayTemp = iDay; iDayTemp <= iMonthLastDay[iIndexPreMonth]; iDayTemp++)
		{
			sprintf(szFilename, "%s%d_%d%02d%02d_UserLog.csv",strPath ,NAME_ID,iYear,iPreMonth,iDayTemp);
			if (0 != GetFileAttributesEx(szFilename,GetFileExInfoStandard,&attrs))
			{
				DeleteFile(szFilename);
			}
		}
	} else //清除上个月同一日的日志
	{
		sprintf(szFilename, "%s%d_%d%02d%02d_UserLog.csv",strPath ,NAME_ID,iYear,iPreMonth,iDay);
		if (0 != GetFileAttributesEx(szFilename,GetFileExInfoStandard,&attrs))
		{
			DeleteFile(szFilename);
		}
	}
}

void CLogModule::LogCharToChar(BYTE src[],string & dst,int len)
{
	for(int j=0;j<len;j++)
	{
		if(src[j]!=255)
		{
			char tem[10];
			sprintf(tem,"%d ",src[j]);
			dst += string(tem);
		}   
	}
}

void CLogModule::DebugPrintf( const char *p, ...)
{
	CTime time = CTime::GetCurrentTime();
	char szFilename[256];
	sprintf(szFilename, "%d-%d%d%d-log.txt", NAME_ID,time.GetYear(), time.GetMonth(), time.GetDay());
	FILE *fp = fopen(szFilename, "a");
	if (NULL == fp)
	{
		return;
	}
	va_list arg;
	va_start(arg, p);
	vfprintf(fp, p, arg);
	fprintf(fp,"\n");

	fclose(fp);
}