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
	void LogCharToChar(BYTE src[],string & dst,int len,int type = 0);
	void DebugPrintf( const char *p, ...);

	string  GetCHNameByAssID(IN UINT bAssistantID);
};

#define LOGMODULEINSTANCE CLogModule::GetInstance()
