#pragma once

class CLogModule
{
private:
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
	void LogCharToChar(unsigned char src[],std::string & dst,int len,int type = 0);
	void DebugPrintf( const char *p, ...);

	std::string  GetCHNameByAssID(unsigned int bAssistantID);
};

#define LOGMODULEINSTANCE CLogModule::GetInstance()
