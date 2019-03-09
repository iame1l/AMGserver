#ifndef EXTENSION_LOGGER_H
#define EXTENSION_LOGGER_H
#include "GameContext.h"
#include "Logger.h"

class ExtensionLogger: public GameContextExtension
{
	std::fstream m_Fstrm;
	std::string m_Tag;
	bool m_bEnable;
public:
	void Open(const std::string &filename);

	void Close();

	void SetTag(const std::string &tag);

	std::string GetTag() const;

	void PrintLog(const std::stringstream &strm);

	void SetEnable(bool bFlag);
	bool IsEnable(){return m_bEnable;}
};

#define LOGGER_FILE(ctx, str){\
	OBJ_GET_EXT(ctx, ExtensionLogger, extLogger);\
	if(extLogger->IsEnable()){\
		std::stringstream strm;\
		strm<<extLogger->GetTag()<<" "<<__FUNCTION__<<" "<<__LINE__<<": "<<str;\
		extLogger->PrintLog(strm);\
	}\
}\

#define LOGGER_ERROR_FILE(ctx, str){\
	OBJ_GET_EXT(ctx, ExtensionLogger, extLogger);\
	if(extLogger->IsEnable()){\
		std::stringstream strm;\
		strm<<extLogger->GetTag()<<" _ERROR_ "<<__FUNCTION__<<" "<<__LINE__<<": "<<str;\
		extLogger->PrintLog(strm);\
	}\
}

#endif