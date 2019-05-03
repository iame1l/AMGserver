#include "stdafx.h"

#include "Ehredis.h"


Ehredis::Ehredis(void)
{
	_context=NULL;
	_reply=NULL;

	//memset(_conf,0,sizeof(_conf));
}


Ehredis::~Ehredis(void)
{
	if(_context!=NULL)
	{
		redisFree(_context);
		_context=NULL;
	}
	if(_reply!=NULL)
	{
		freeReplyObject(_reply);
		_reply=NULL;
	}
}



void Ehredis::Connect()
{
	//Ì××Ö½Ú³õÊ¼»¯
	WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 1), &wsaData);


	//this->_conf.IP="127.0.0.1";
	//this->_conf.PORT=6379;

	string IP="127.0.0.1";
	int PORT=6379;


	_context = redisConnect(IP.c_str(),PORT);
	if(_context->err)
	{	
		redisFree(_context);
		return ;	
	}
	printf("Connect to redisServer Success\n");
	//system("pause");
	return ;

}
void Ehredis::disConnect()
{
	if(_context)
	{
		redisFree(_context);
		_context=NULL;
	}
}

void Ehredis::freeReply()
{
	 if(_reply)
     {
         freeReplyObject(_reply);
         _reply = NULL;
     }
}
bool Ehredis::isError()
{
	if(NULL == _reply)
	{
		freeReply();
		disConnect();
		Connect();
		return true;
	}
	return false;
}

void Ehredis::setString(const string & key, const string & value)
{
	if(key.empty() || value.empty()) return;
	
	this->_reply=(redisReply *)redisCommand(_context, "SET %s %s", key.c_str(), value.c_str());
	
}
void Ehredis::setString(const string & key, const int & value)
{
	if(key.empty()) return;

	this->_reply=(redisReply *)redisCommand(_context, "SET %s %d", key.c_str(), value);
}
void Ehredis::setString(const string & key, const float & value)
{
	if(key.empty()) return;
	this->_reply=(redisReply *)redisCommand(_context, "SET %s %f", key.c_str(), value);
}
