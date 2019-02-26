/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#ifndef __HNRC4_H__
#define __HNRC4_H__

#include <string>

namespace HN
{

class HNRC4
{
public:
	// 构造
	HNRC4();
   
	// 初始化
	void init(unsigned char* key, int keyLen);

	// 析构
	virtual ~HNRC4();

	// 加密
	void encrpyt(unsigned char* data, int len);

	// 解密
	void decrypt(unsigned char* data, int len);

private:
	// 最大长度
	static const int MAX_LEN = 256;

	// 秘钥
	unsigned char _key[MAX_LEN];

	// 随机因子
	unsigned char _box[MAX_LEN];

	// 秘钥长度
	int _keyLen;

	// 执行加解密
	void rc4(unsigned char* data, int len);
};

};

#endif // !__HNRC4_H__