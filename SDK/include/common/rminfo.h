#include "HNBaseType.h"
#include "basemessage.h"
#include "comstruct.h"
#include "gameplacemessage.h"
#include "gameroommessage.h"

#ifndef _ROOMINFO_BASE_
#define _ROOMINFO_BASE_

struct MSG_MG_S_BUY_DESK_NOTICE	//购买桌子结果
{
	bool bSuccess;
	MSG_GP_S_BUY_DESK_RES  _data;
	MSG_MG_S_BUY_DESK_NOTICE()
	{
		memset(this,0,sizeof(MSG_MG_S_BUY_DESK_NOTICE));
	}
};

struct MSG_MG_S_RETURN_DESK_NOTICE	//恢复桌子结果
{
	bool bSuccess;
	int	 iType;			//1进入方间 2断线重连
	MSG_GP_S_GET_CUTNETROOMINFO_RES _data;
	MSG_MG_S_RETURN_DESK_NOTICE()
	{
		memset(this,0,sizeof(MSG_MG_S_RETURN_DESK_NOTICE));
	}
};

struct MSG_MG_S_NETCUT_PLACE	//玩家大厅掉线
{
	int iUserID;
};

struct MSG_MG_S_CREATEDESK_ERROR	//
{
	int iUserID;//用户ID
	int iType;
};

struct MSG_MG_S_CONTEST_ACTIVE	//激活比赛房间
{
	int									iRoonID;
	int									iMatchID;
	bool								bNotify;
    int                                 iApplyNum;
	COleDateTime						BeginTime;
};

struct MSG_MG_S_CONTEST_NOTICE	//赛前通知
{
	int			iRoomID;
	int			iCountID;
};

struct MSG_MG_S_DissmissDesk	//解散房间
{
	int		iUserID;
	int		iRoomID;
	int		iDeskID;
	int		iResult;
	char    szDeskPass[20];
};

struct MSG_MG_R_ClubDissmissDesk	//俱乐部解散同时解散桌子
{
	int		iRoomID;
	int		iDeskID;
	char    szDeskPass[20];
};

struct MSG_MG_S_CLUB_BUY_DESK_NOTICE	// 俱乐部购买桌子结果
{
	bool								bSuccess;
	int									iRoomID;
	int									iDeskID;
	int									iJewels;
	int									iGameID;
	MSG_GP_O_Club_BuyDesk				_data;
	MSG_MG_S_CLUB_BUY_DESK_NOTICE()
	{
		memset(this,0,sizeof(MSG_MG_S_CLUB_BUY_DESK_NOTICE));
	}
};



#define MDM_ZG_MESSAGE					140		//MServer与GServer 通信消息主ID起始
#define ASS_ZG_MESSAGE					1

#define MDM_MG_SENDROMMID				141		//G端发送房间号
#define ASS_MG_SENDROMMID				1

#define MDM_MG_CREATEROOM				142		//创建房间相关
#define ASS_MG_BUYROOM_NOTICE			1
#define ASS_MG_RETURNDESK_NOTICE		2
#define ASS_MG_CREATENDESK_ERROR		3
#define ASS_MG_DISSMISSDESK				4		//大厅解散房间

#define MDM_MG_NETCUT_PLACE				143		//玩家大厅掉线
#define ASS_MG_NETCUT_PLACE				1

#define MDM_MG_CONTEST					144		//比赛消息
#define ASS_MG_CONTEST_APPLY			1
#define ASS_MG_CONTEST_NOTICE			2		//赛前通知

#define MDM_MG_CLUB						145		//俱乐部消息
#define ASS_MG_CLUB_DISSMISSDESK		1		//俱乐部解散同时解散桌子
#define ASS_MG_CLUB_BUYDESK				2		//俱乐部购买桌子
#define ASS_MG_CLUB_CLEARDESK			3		//俱乐部房间解散通知俱乐部玩家

#define TYPE_BUYROOM_NOTICE			1
#define TYPE_RETURNDESK_NOTICE		2
#define TYPE_CLUB_BUYDESK			3

#endif