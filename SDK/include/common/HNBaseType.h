#ifndef __HN_BaseType_H__
#define __HN_BaseType_H__

typedef		char				CHAR;
typedef		wchar_t				WCHAR;
typedef		unsigned char       BYTE;

typedef		unsigned short      WORD;

typedef		int					INT;
typedef		unsigned int		UINT;

typedef		long long			LLONG;
typedef		unsigned long		ULONG;
typedef		unsigned long		DWORD;

typedef		double              DOUBLE;
typedef		float               FLOAT;

#define	INVALID_VALUE			UINT(-1)
#define	INVALID_LLONG_VALUE		LLONG(-1)

#define	INVALID_DESKNO			BYTE(-1)
#define	INVALID_DESKSTATION		BYTE(-1)
#define INVALID_USER_ID         INT(-1)

#define MAX_BLOCK_PROP_SIZE		512	//每个消息的最大数量

#define MAX_SEND_SIZE			2044//每个数据包发送的最大大小

// 常量
#define NAME_LENGTH				22	// 呢称、用户名、真实姓名 实际保存的数组长度
#define NAME_LENGTH_MAX         20  // 限制名称的长度为 [6-20]
#define NAME_LENGTH_MIN			6		

#define PASSWORD_MD5_LENGTH		64	//MD5 加密过后的用户密码长度
#define PASSWORD_RAW_LENGTH		22	//原始密码数组保存长度
#define PASSWORD_RAW_LENGTH_MAX	20	//原始密码长度[6-20]
#define PASSWORD_RAW_LENGTH_MIN	6

#define PHONE_NUMBER_LENGTH				15		// 手机号码长度
//#define PHONE_HOME_NUMBER_LENGTH		20		// 座机号码长度
#define PHONE_VCODE_LENGTH				36		// 手机验证码长度

#define ID_CARD_NUMBER_LENGTH			19		// 身份证号码长度
#define ID_CARD_NUMBER_MD5_LENGTH		64		// MD5后的身份证号码长度

#define RANKNAME_LENGHT					22		//军衔名称

#define ADDFRIEND_CONTENT_LEN			100		//添加好友时请求内容的最大长度

#define TALK_CONTENT_LEN				200		//聊天内容大小

#define SOCIAL_NOTICE_LEN				120		//商会公告内容最大长度

#define GAMENAME_LEN					20		//游戏名称长度

#define GAMEROOM_LEN					30		//游戏房间长度

#define MAC_LEN							64		//机器设备ID字符长度

#include <cstring>

#include <cassert>

#endif	//__HN_BaseType_H__
