#pragma once

#ifndef tstring
#include <string>
using std::string;
using std::wstring;
#if defined(UNICODE) || defined(_UNICODE)
#define tstring wstring
#else
#define tstring string
#endif // UNICODE
#endif // tstring

#ifndef vstring
#include <vector>
#define vstring	std::vector<tstring>
#endif // vtstring