#include "StdAfx.h"
#include "LogModule.h"

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
	char szFilename[256];
	CString strPath = CINIFile::GetAppPath() +"\\GameLog\\";
	SHCreateDirectoryEx(NULL, strPath, NULL);
	CTime time = CTime::GetCurrentTime();
	sprintf(szFilename, "%s%d_%d%02d%02d_UserLog.csv",strPath ,NAME_ID,time.GetYear(),time.GetMonth(),time.GetDay());
	FILE *fp = fopen( szFilename, "a" );
	if (NULL == fp)
	{
		return;
	}
	char s[1024];
	va_list arg_ptr;
	va_start(arg_ptr, pPutFromat);
	_vsnprintf(s, sizeof(s)-1,pPutFromat,arg_ptr);
	fprintf(fp,"%s",s);
	fclose(fp);
}

void CLogModule::LogCharToChar(BYTE src[],string & dst,int len,int type)
{
	switch(type)
	{
	case 0:
		{
			for(int j=0;j<len;j++)
			{
				if(src[j]!=255 && src[j]!=0 )
				{
					char tem[10];
					sprintf(tem,"%d ",src[j]);
					dst += string(tem);
				}   
			}
			break;
		}
	case 1:
		{
			for(int j=0;j<len;j++)
			{
				if(src[j]!=255 && src[j]!=0)
				{
					char tem[10];
					sprintf(tem,"%dt_%d ",src[j]>>4,(src[j]&0x0F));
					dst += string(tem);
				}   
			}
			break;
		}
	case 2:
		{
			for(int j=0;j<len;j++)
			{
				if(src[j]!=255 && src[j]!=0)
				{
					char tem[10];
					sprintf(tem,"%d ",(src[j]&0x0F) );
					dst += string(tem);
				}   
			}
			break;
		}
	default:
		break;
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

string  CLogModule::GetCHNameByAssID(IN UINT bAssistantID)
{
/////////////// 初始化 ///////////////////////////////
	////  alt + R 可以局部选中 
	struct SNode
	{
	    UINT uId;
		string sName;
	};    
	SNode NameList[] = {
		{51	,		"//游戏开始	 "	},
		{52	,		"//发牌消息 "	},
		{53,		"//通知叫分		 "	},
		{54,		"//叫分结果/请求叫分  "	},
		{55,		"//通知抢地主  "	},
		{56	,		"//抢地主结果/请求抢地主	 "	},
		{57  ,      "//定庄消息 "	},
		{58,		"//通知加倍  "	},
		{59,		"//加倍结果/请求加倍		 "	},
		{60,		"//通知明牌			 "	},
		{61,		"//明牌请求		 "	},
		{62,		"//明牌结果			 "	},
		{63	,		"//癞子牌通知		 "	},
		{64	,		"//开始出牌		 "	},
		{65	,		"//出牌请求	 "	},
		{66  ,      "//出牌结果	 "	},
		{67,		"//癞子组合通知   "		},
		{68,		"//过牌请求	   "		},
		{69,		"//过牌结果		 "	},
		{70,		"//提示请求			 "	},
		{71,		"//提示结果		 "	},	
		{72,		"//错误的牌型通知		 "	},
		{73,		"//小结算"	},
		{74,		"//大结算	 "	},
		{75,		"//托管"	},	
		{76,		"//更新倍数		 "	}
	};				 
///////////// 获取 ////////////////////////////////////////
	string  cName = "";
	int len = sizeof(NameList) / sizeof(SNode);
	for(int i=0;i<len;i++)
	{
	    if( NameList[i].uId == bAssistantID)
		{
		    cName = NameList[i].sName;
			break;
		}
	}
	return cName;
}