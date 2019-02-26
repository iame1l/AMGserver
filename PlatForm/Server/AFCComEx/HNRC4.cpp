/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/
#include "Stdafx.h"
#include "HNRC4.h"

namespace HN
{

// 构造
HNRC4::HNRC4()
{
	memset(_box, 0, MAX_LEN);
	memset(_key, 0, MAX_LEN);
}

// 析构
HNRC4::~HNRC4()
{

}

// 加密
void HNRC4::encrpyt(unsigned char* data, int len)
{
	rc4(data, len);
}

// 解密
void HNRC4::decrypt(unsigned char* data, int len)
{
	rc4(data, len);
}

// 执行加解密
void HNRC4::rc4(unsigned char* data, int len)
{
	unsigned char tBox[MAX_LEN] = { 0 };
	memcpy(tBox, _box, MAX_LEN);

	int i = 0, j = 0, t = 0;
	unsigned long k = 0;
	unsigned char tmp;
	for (k = 0; k < len; k++)
	{
		i = (i + 1) % MAX_LEN;
		j = (j + tBox[i]) % MAX_LEN;
		tmp     = tBox[i];
		tBox[i] = tBox[j];
		tBox[j] = tmp;
		t = (tBox[i] + tBox[j]) % MAX_LEN;
		data[k] ^= tBox[t];
	}
}

// 初始化
void HNRC4::init(unsigned char* key, int keyLen)
{
	_keyLen = keyLen;
	memcpy(_key, key, keyLen);

	unsigned char k[MAX_LEN] = { 0 };	
	for (int i = 0; i < MAX_LEN; i++) 
	{
		_box[i] = i;
		k[i]    = _key[i % _keyLen];
	}

	unsigned char tmp = 0;
	for (int i = 0, j = 0; i < MAX_LEN; i++) 
	{
		j = (j + _box[i] + k[i]) % MAX_LEN;
		tmp     = _box[i];
		_box[i] = _box[j];
		_box[j] = tmp;
	}
}

}