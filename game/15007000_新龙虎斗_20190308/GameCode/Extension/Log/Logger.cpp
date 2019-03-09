#include "Logger.h"
#include <cstdarg>
#include <cstdio>
#include <iostream>

void Logger::Log(const char *format, ...)
{
    va_list args;
    va_start(args,format);
    vprintf(format,args);
    va_end(args);
    printf("\n");
}

void Logger::Log(int i)
{
    printf("%d\n", i);
}

void Logger::Console(const std::stringstream &strm)
{
    std::cout<<strm.str()<<std::endl;
}

void Logger::Log(std::fstream &fstrm, const std::stringstream &strm)
{
	fstrm << strm.str() <<std::endl;
}
