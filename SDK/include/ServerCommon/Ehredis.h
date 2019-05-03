#pragma once
#include "../hiredis/hiredis.h"
#define NO_QFORKIMPL //这一行必须加才能正常使用
//#include "win32fixes.h" //?
#include <iostream>
#include <string>


using namespace std;

class Ehredis
{
public:
	Ehredis(void);
	~Ehredis(void);


	//基本链接
public:
	void Connect();
	void disConnect();
private:
	void freeReply();
	bool isError();

	//操作
public:
	void setString(const string & key, const string & value);
	void setString(const string & key, const int & value);
	void setString(const string & key, const float & value);
private:
	redisContext * _context;
	redisReply * _reply;
	//RedisConf _conf;
};


//class RedisConf
//{
//public:
//	std::string IP;
//	int PORT;
//	timeval t;
//};

