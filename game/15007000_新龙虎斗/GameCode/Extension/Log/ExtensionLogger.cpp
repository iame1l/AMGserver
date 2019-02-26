#include "ExtensionLogger.h"
#include <iostream>
#include <time.h>

void ExtensionLogger::SetEnable(bool bFlag)
{
	m_bEnable = bFlag;
}

void ExtensionLogger::Open(const std::string &filename)
{
	Close();
	m_Fstrm.open(filename, std::ios::app);
}

void ExtensionLogger::Close()
{
	if(m_Fstrm.is_open()) m_Fstrm.close();
}

void ExtensionLogger::SetTag(const std::string &tag)
{
	m_Tag = tag;
}

std::string ExtensionLogger::GetTag() const
{
	time_t nowTs = time(NULL);
	tm time_data;
	localtime_s(&time_data, &nowTs);
	char timeStrBuf[256];
	sprintf_s(timeStrBuf, " %d-%02d-%02d %02d:%02d:%02d", 1900 + time_data.tm_year,
		time_data.tm_mon + 1,
		time_data.tm_mday,
		time_data.tm_hour,
		time_data.tm_min,
		time_data.tm_sec);

	std::string ret = m_Tag;
	ret.insert(ret.end(), timeStrBuf, timeStrBuf + strlen(timeStrBuf));
	return ret;
}

void ExtensionLogger::PrintLog(const std::stringstream &strm)
{
	if(m_Fstrm.is_open())
	{
		Logger::Log(m_Fstrm, strm);
		m_Fstrm.flush();
	}
}
