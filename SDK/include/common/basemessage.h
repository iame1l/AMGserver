/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#ifndef AFC_MESSAGE_HEAD_FILE
#define AFC_MESSAGE_HEAD_FILE

/********************************************************************************************/

///端口定义 
#define FLASH_SERVER_PORT		843								///页游认证服务器端口//国际标准端口号

#define SERVER_LOGON_PORT		3010							///服务登陆端口
#define GAME_LOGON_PORT			3015							///游戏登陆端口

#define CENTER_SERVER_PORT		13025							///中心服务端口
#define CENTER_DISTRI_PORT		13026							///中心服务中转端口

#define PORT_GAME_CONNECT_MAIN	13033						//供GServer连接 MServer 的端口
/********************************************************************************************/



///开发库版本
#define DEV_LIB_VER				1									///开发库版本
///全局版权，固定在程序里，无法修改
#define TMLcopyright			TEXT("广告")				///版权

///常量定义
#define MAX_PEOPLE				180									///最大游戏人数 百家乐 modify 8 -> 180 by wlr 20090714
#define MAX_SEND_SIZE			2044								///最大消息包
#define MAX_SEND_SIZE_OUT		2155								///实际传输最大消息包
#define MAX_TALK_LEN			500									///最大聊天数据长度
#define NORMAL_TALK_LEN			200									///普通聊天数据长度
#define US_PLAY_GAME			20									///用户游戏状态
#define NET_HEAD_SIZE			sizeof(NetMessageHead)				///数据包头

///游戏类型定义
#define TY_NORMAL_GAME			1									///普通游戏
#define TY_MATCH_GAME			2									///比赛游戏
#define TY_MONEY_GAME			3									///金币游戏
#define TY_DRILLING_GAME		4									///演练场游戏(开发中)
#define TY_FASCINATION_GAME		5									///魅力场游戏(开发中)
///
///Kylin 20090105 添加赛币场
#define TY_SHABI_GAME		6									///赛币场游戏
///
///支持类型定义
#define SUP_NORMAL_GAME			0x01								///普通游戏
#define SUP_MATCH_GAME			0x02								///比赛游戏
#define SUP_MONEY_GAME			0x04								///金币游戏

///显示位置
#define SHS_TALK_MESSAGE		0x01								///聊天信息
#define SHS_MESSAGE_BOX			0x02								///弹出信息
#define SHS_SYSTEM_SHOW			0x04								///系统消息

#define MIN_ZIP_SIZE			200									///最小压缩数据长度
#define ZIP_FLAG				0x01								///数据是否已压缩

#pragma pack(1)
///网络数据包结构头
struct NetMessageHead
{
	UINT						uMessageSize;						///数据包大小
	UINT						bMainID;							///处理主类型
	UINT						bAssistantID;						///辅助处理类型 ID
	UINT						bHandleCode;						///数据包处理代码
	UINT						bReserve;							///校验数据
	UINT						bDateRule;							///数据信息
};

//此处为密钥由客户自己管理，设置在整数范围
#define SECRET_KEY  145843514			//9位随机数字
///连接成功消息 
struct MSG_S_ConnectSuccess
{
	BYTE						bMaxVer;							///最新版本号码
	BYTE						bLessVer;							///最低版本号码
	BYTE						bReserve[2];						///保留字段
	UINT						i64CheckCode;						///加密后的校验码，由客户端解密在包头中返回
};
#pragma pack()

/********************************************************************************************/
///	系统内核使用 （MAINID 使用 0 - 49 ）
///	系统服务通信 （MAINID 使用 50 - 99 ）
///	游戏大厅服务 （MAINID 使用 100 - 149 ）
///	附加游戏通信 （MAINID 使用 151 - 199 ）
///	保留通信标志 （MAINID 使用 200 - 255 ）

/********************************************************************************************/

///特殊消息
	#define MDM_NO_FAILE					0			///不失败消息

///连接消息
	#define MDM_CONNECT						1			///连接消息类型
	///辅助处理消息标志
	#define ASS_NET_TEST					1			///网络测试
	#define ASS_CONNECT_SUCCESS 			3			///连接成功

/********************************************************************************************/

#endif