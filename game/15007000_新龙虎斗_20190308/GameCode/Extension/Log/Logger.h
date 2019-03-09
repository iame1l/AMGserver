#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <sstream>
#include <fstream>

class Logger
{
public:
    static void Log(const char *format, ...);
    static void Log(int i);
    static void Console(const std::stringstream &strm);
	static void Log(std::fstream &ftrm, const std::stringstream &strm);
};

#define LOGGER_CONSOLE(str){\
    std::stringstream strm;\
    strm<<__FILE__<<" "<<__LINE__<<" "<<str;\
    Logger::Console(strm);\
}

#endif
