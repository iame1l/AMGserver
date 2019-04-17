#pragma once

class CLogModule
{
public:
	CLogModule(void);
	~CLogModule(void);
public:
	static  CLogModule & GetInstance()
	{
		return m_pLogModule;
	};
	static CLogModule m_pLogModule;
	/**
	* ¥Ú”°»’÷æ
	*
	*/
	void WriteLog(const char *pPutFromat, ...);
	void LogCharToChar(BYTE src[],string & dst,int len);
	void DebugPrintf( const char *p, ...);
};

//#define LOGMODULEINSTANCE CLogModule::GetInstance()
#define LOGMODULEINSTANCE (CLogModule::m_pLogModule)
