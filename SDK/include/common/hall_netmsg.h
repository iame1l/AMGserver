#ifndef _HALL_NETMSG_H
#define _HALL_NETMSG_H

#include "HNBaseType.h"

enum E_NET_CONNECT_MAINID
{
	mainid_net_connect=1,//连接服务器
};

enum E_NET_CONNECT_ASSID
{
	assid_net_connect=1,//连接服务器
	assid_net_test,		//网络测试
};

enum E_HALL_MAINID
{
	mainid_hall_contest=51,		//比赛消息
	MDM_GP_PLAYER_TO_GM=107,	/// 普通玩家与GM之间的通信协议
	mainid_hall_bank=121,	//银行消息
	mainid_hall_sign=133,	//签到
	mainid_hall_im=136,		//好友相关
	mainid_hall_prop=140,//道具消息

	mainid_hall_register,	//注册
	mainid_hall_lucky_content,//获取抽奖奖品信息
};

enum E_HALL_ASSID_CONTENT
{
	assid_hall_get_contest_roomid=1,//比赛房间，用户双击列表时向服务器索要房间号
};
enum E_HALL_ASSID_PROP
{
	assid_hall_prop_get=1, //获得道具
	assid_hall_prop_buy=2, //购买道具   
};

enum E_HALL_ASSID_BANK
{
	assid_hall_bank_open=1,	//打开银行
	assid_hall_bank_store,	//存款
	assid_hall_bank_get,	//取款
	assid_hall_bank_trans,	//转账
	assid_hall_bank_translog,//转账记录
	assid_hall_bank_modpswd,//修改银行密码
};

enum E_HALL_ASSID_SIGN
{
	assid_hall_sign_record=1,	//获取本月签到记录
	assid_hall_sign_do,	//执行签到
};

enum E_HALL_ASSID_IM
{
	assid_hall_im_getfriend=1,//好友列表
	assid_hall_im_addfriend_search,//通过昵称查找好友
	assid_hall_im_addfriend,//添加好友	
	assid_hall_im_addfriend_trans,//转发好友请求，发送好友请求结果
	assid_hall_im_delfriend,//删除好友
	assid_hall_im_online,	//好友上线
	assid_hall_im_offline,	//好友下线
	assid_hall_im_enterroom,//进入房间
	assid_hall_im_leaveroom,//离开房间
	assid_hall_im_friendtalk,//好友聊天
	assid_hall_im_getsociallist,//商会列表
	assid_hall_im_getsocial,//商会信息
	assid_hall_im_getsocialmem,//商会成员列表	
	assid_hall_im_createsocial,//创建商会
	assid_hall_im_joinsocial,//加入商会
	assid_hall_im_joinsocial_trans,//转发给会长审批，或会长发送审批结果
	assid_hall_im_quitsocial,//退出商会
	assid_hall_im_kicksocial,//请出商会
	assid_hall_im_disband,	//解散商会
	assid_hall_im_socialtalk,//商会聊天
	assid_hall_im_socialnotice,//商会公告
};

enum E_HALL_ASSID_REGISTER
{
	assid_hall_register_normal=1,	//普通注册
	assid_hall_register_fast,	//快速注册
	assid_hall_register,		//服务端返回
};

enum E_HALL_ASSID_LUCKY_CONTENT
{
	assid_hall_lucky_content=1,	//获取抽奖列表
};

#pragma pack(1)
/**************************************************assid_hall_get_contest_roomid***********************************/
struct MSG_GP_GetContestRoomID
{
	DWORD		dwUserID;			//用户ID
	UINT		iGameID;			//游戏ID
	int			iContestID;			//比赛ID
	UINT		iUpPeople;			//比赛达到此人数后才开赛
};

struct DL_GetContestRoomID
{
	UINT				iGameID;		//游戏ID
	int					iRoomID;		//比赛房间ID
	int					iContestID;		//比赛ID
};
/**************************************************assid_hall_get_contest_roomid***********************************/

/**************************************************assid_hall_bank_open***********************************/
enum E_HALL_BANK_OPEN_RESULT
{
	rs_hall_bank_open_error,//未知错误
	rs_hall_bank_open_ok,	//操作成功	
};
struct NET_HALL_BANK_OPEN_RESULT
{
	CHAR	szPswd[PASSWORD_MD5_LENGTH];	//银行密码
	UINT	uTransMoneyLimit;	//转账最低限额
	UINT	uTransTax;	//转账税率
	UINT	uTransMaxTax;//转账最高税额
	LLONG	llWallet;	//钱包
	LLONG	llBank;		//银行
	LLONG	llDragon;	//龙币
};
/**************************************************assid_hall_bank_open***********************************/

/**************************************************assid_hall_bank_store***********************************/
enum E_HALL_BANK_STORE_RESULT
{
	rs_hall_bank_store_error,			//未知错误
	rs_hall_bank_store_ok,				//操作成功
	rs_hall_bank_store_money_not_enough,//存款时钱包金币不足
	rs_hall_bank_store_maxlimit,		//存款额达到上限
};

struct NET_HALL_BANK_STORE
{
	LLONG	llMoney;					//存入额
};

//刷新机器人钱包信息
struct NET_ROOM_ROBOT_WALLET_REFRESH
{
	DWORD dwUserID;						//用户ID
	LLONG llWallet;						//钱包的钱
};

struct NET_HALL_BANK_STORE_RESULT
{
	LLONG	llWallet;					//存入后钱包
	LLONG	llBank;						//存入后银行
};

//存款超过上限，需要提升军衔等级
struct NET_HALL_BANK_STORE_LIMIT_RESULT
{
	LLONG	llBank;						//现在银行余额
	LLONG	llMaxStore;					//当前级别的银行存款上限
};

/**************************************************assid_hall_bank_store***********************************/
/**************************************************assid_hall_bank_get***********************************/
enum E_HALL_BANK_GET_RESULT
{
	rs_hall_bank_get_error,				//未知错误
	rs_hall_bank_get_ok,				//操作成功	
	rs_hall_bank_get_pswd_error,		//银行密码错误
	rs_hall_bank_get_money_not_enough,	//银行存款不足
};

struct NET_HALL_BANK_GET
{
	CHAR	szPswd[PASSWORD_MD5_LENGTH];//银行密码
	LLONG	llMoney;					//取出额
};

struct NET_HALL_BANK_GET_RESULT
{
	LLONG	llWallet;					//取出后钱包
	LLONG	llBank;						//取出后银行
};
/**************************************************assid_hall_bank_get***********************************/

/**************************************************assid_hall_bank_trans***********************************/
enum E_HALL_BANK_TRANS_RESULT
{
	rs_hall_bank_trans_error,			//未知错误
	rs_hall_bank_trans_ok,				//操作成功	
	rs_hall_bank_trans_pswd_error,		//银行密码错误
	rs_hall_bank_trans_money_not_enough,//金额不足
	rs_hall_bank_trans_dest_notexist,	//转账时收款人不存在
	rs_hall_bank_trans_money_limit,		//转账金额太低
};

struct NET_HALL_BANK_TRANS
{
	union 
	{
		CHAR	szDestName[NAME_LENGTH];	//收款方用户名
		DWORD	dwDestUserID;					//收款方ID
	}targetUser;	
	CHAR	szPswd[PASSWORD_MD5_LENGTH];//银行密码
	LLONG	llMoney;					//转账金额
};

//转账成功给发起者用户和目标用户发送的消息
struct NET_HALL_BANK_TRANS_RESULT
{
	union 
	{
		CHAR	szSrcName[NAME_LENGTH];		//发起者用户名
		DWORD	dwSrcUserID;				//发起者ID
	}srcUser;

	union 
	{
		CHAR	szDestName[NAME_LENGTH];	//目标用户名
		DWORD	dwDestUserID;				//收款方ID
	}targetUser;
	UINT	uTaxMoney;					//转账收税
	LLONG	llMoney;					//转账金额
	LLONG	llSrcBank;					//转账后发起者银行金额
	LLONG	llDestBank;					//转账后接收者银行金额
	LLONG	llActualMoney;				//实际到账金额
};

//由于转账金额太低，导致转账失败
struct NET_HALL_BANK_TRANS_MONEYLIMIT_RESULT
{
	union 
	{
		CHAR	szDestName[NAME_LENGTH];	//收款方用户名
		DWORD	dwDestUserID;					//收款方ID
	}targetUser;
	UINT	uTransMoneyLimit;			//转账最低限额
	LLONG	llMoney;					//转账金额
};

//由于银行存款不足导致转账失败
struct NET_HALL_BANK_TRANS_MONEY_NOTENOUGH_RESULT
{
	union 
	{
		CHAR	szDestName[NAME_LENGTH];	//收款方用户名
		DWORD	dwDestUserID;					//收款方ID
	}targetUser;
	LLONG	llBank;						//银行中金额
	LLONG	llMoney;					//转账金额
};

//由于接收方不存在导致转账失败
struct NET_HALL_BANK_TRANS_DEST_NOTEXIST_RESULT
{
	union 
	{
		CHAR	szDestName[NAME_LENGTH];	//收款方用户名
		DWORD	dwDestUserID;					//收款方ID
	}targetUser;	
	LLONG	llMoney;					//转账金额
};

/**************************************************assid_hall_bank_trans***********************************/

/**************************************************assid_hall_bank_modpswd***********************************/
enum E_HALL_BANK_MODPSWD_RESULT
{
	rs_hall_bank_modpswd_error,
	rs_hall_bank_modpswd_ok,
	rs_hall_bank_modpswd_pswderror,		//旧密码错误
};

struct NET_HALL_BANK_MODPSWD
{
	CHAR	szOldPswd[PASSWORD_MD5_LENGTH];
	CHAR	szNewPswd[PASSWORD_MD5_LENGTH];
};

struct NET_HALL_BANK_MODPSWD_RESULT
{
	CHAR	szNewPswd[PASSWORD_MD5_LENGTH];
};
/**************************************************assid_hall_bank_modpswd***********************************/

/**************************************************assid_hall_bank_translog***********************************/
enum E_HALL_BANK_TRANSLOG_RESULT
{
	rs_hall_bank_translog_error,
	rs_hall_bank_translog_ok
};

struct NET_HALL_BANK_TRANSLOG_RESULT
{
	union 
	{
		CHAR	szSrcName[NAME_LENGTH];	//转账发起者用户名
		DWORD   dwSrcUserID;			//ID
	}srcUser;
	union 
	{
		CHAR	szDestName[NAME_LENGTH];//接收者用户名
		DWORD   dwDestUserID;			//ID
	}targetUser;
	UINT	uTaxMoney;				//手续费
	LLONG	llTransMoney;			//转账发生金额
	LLONG	llActualMoney;			//实际到账金额
	LLONG	t_time;					//转账日期
};
/**************************************************assid_hall_bank_translog***********************************/

/******************************************assid_hall_sign_record************************************************/
//签到记录
struct TSignInLog
{
	DWORD dwUserID;	
	int	  iGetPoint;//下次签到可获得的经验值
	int   iGetMoney;//下次签到可获得的金币
	char  szProp[20];//下次签到可获得的道具
	BYTE  byDay[32];//本月已签到的天数
	char  szTime[22];//本月签到
};
/******************************************assid_hall_sign_record************************************************/

/******************************************assid_hall_sign_do************************************************/
//签到
struct TSign_In
{
	DWORD dwUserID;
	int	  iRs;	//签到结果:1-签到成功，2-已经签到
	int	  iGetPoint;
	int   iGetMoney;
	char  szProp[20];	//道具
	BYTE  byCountDay;	//连续签到天数
	char  szTime[22];
};
/******************************************assid_hall_sign_do************************************************/

/******************************************assid_hall_prop_get************************************************/
enum E_HALL_PROP_RESULT
{
	rs_hall_prop_error,	//操作失败，未知错误
	rs_hall_prop_ok,	//操作成功
};

struct NET_HALL_PROP_GET
{
	BYTE byPropID;		//查询的道具ID，0-查询所有，大于0时表示查询对应的道具ID
};

/*
struct _TAG_PROP_BUY
{
	long	dwUserID;					//购买者ID
	int		nPropID;					//道具ID
	int		nPropBuyCount;				//道具数量
};*/

struct _TAG_PROP_BUY_RESULT
{
	long	dwUserID;					//购买者ID
	int		nPropID;					//道具ID
	int		nPropBuyCount;				//道具数量
	int     iResult;                    //0=失败,1=成功
	char    szMessage[50];              //结果信息
};


struct NET_HALL_PROP_GET_RESULT
{
	CHAR	szPropName[20];	//道具名称
	BYTE	byPropID;		//道具ID号
	INT		iHoldCount;		//拥有道具的数量
	INT		iValue;			//防踢卡时效或对应魅力值
};
/*******************************************assid_hall_prop_get***********************************************/

/*******************************************assid_hall_im_getfriend***********************************************/
struct NET_HALL_IM_GETFRIEND_RESULT
{
	CHAR	szNick[NAME_LENGTH];	//昵称
	CHAR	szSocialName[NAME_LENGTH];//商会名称
	BYTE	byOnline;	//在线标志 0-不在线，1-在线
	UINT	uGroupID;	//分组ID
	UINT	uSocialID;	//商会ID
	UINT	uLogoID;	//头像ID
	UINT	uFriendID;	//玩家ID
};
/*******************************************assid_hall_im_getfriend***********************************************/

/*******************************************assid_hall_im_addfriend_search***********************************************/
struct NET_HALL_IM_ADDFRIEND_SEARCH
{
	CHAR	szNick[NAME_LENGTH];	//查询的昵称
};
struct NET_HALL_IM_ADDFRIEND_SEARCH_RESULT
{
	CHAR	szNick[NAME_LENGTH];
	UINT	dwUserID;	//玩家ID
	UINT	uLogoID;	//头像ID
};
/*******************************************assid_hall_im_addfriend_search***********************************************/

/*******************************************assid_hall_im_addfriend***********************************************/
enum E_HALL_IM_ADDFRIEND_RESULT
{
	rs_hall_im_addfriend_error,
	rs_hall_im_addfriend_ok,
	rs_hall_im_addfriend_refuse,	//被拒绝
	rs_hall_im_addfriend_count_limit,//好友数量达到上限
};

//添加成功后返回好友信息
struct NET_HALL_IM_ADDFRIEND
{
	CHAR	szSrcNick[NAME_LENGTH];	//请求者昵称
	CHAR	szContent[ADDFRIEND_CONTENT_LEN];//请求内容
	UINT	uDestUserID;
};

/*******************************************assid_hall_im_addfriend***********************************************/

/*******************************************assid_hall_im_addfriend_trans***********************************************/
struct NET_HALL_IM_ADDFRIEND_TRANS_RESULT
{
	CHAR	szSrcNick[NAME_LENGTH];	//请求者昵称
	CHAR	szContent[ADDFRIEND_CONTENT_LEN];//请求内容
	UINT	uSrcUserID;
};

struct NET_HALL_IM_ADDFRIEND_TRANS
{
	CHAR	szDestNick[NAME_LENGTH];//被请求者昵称
	UINT	uSrcUserID;	//请求者ID
};
/*******************************************assid_hall_im_addfriend_trans***********************************************/

/*******************************************assid_hall_im_delfriend***********************************************/
enum E_HALL_IM_DELFRIEND_RESULT
{
	rs_hall_im_delfriend_error,
	rs_hall_im_delfriend_ok,
};
struct NET_HALL_IM_DELFRIEND
{
	UINT	uDestUserID;//对方ID
};
struct NET_HALL_IM_DELFRIEND_RESULT
{
	UINT	uSrcUserID;
	UINT	uDestUserID;
};
/*******************************************assid_hall_im_delfriend***********************************************/

/*******************************************assid_hall_im_online***********************************************/
//服务端主动发送
struct NET_HALL_IM_ONLINE_RESULT
{
	UINT	uFriendID;	//上线者玩家ID
};
/*******************************************assid_hall_im_online***********************************************/

/*******************************************assid_hall_im_offline***********************************************/
//服务端主动发送
struct NET_HALL_IM_OFFLINE_RESULT
{
	UINT	uFriendID;	//下线者玩家ID
};
/*******************************************assid_hall_im_offline***********************************************/

/*******************************************assid_hall_im_enterroom***********************************************/
struct NET_HALL_IM_ENTERROOM
{
	CHAR	szGameName[GAMENAME_LEN];
	CHAR	szRoomName[GAMEROOM_LEN];
	UINT	uKindID;
	UINT	uNameID;
	UINT	uRoomID;
};
struct NET_HALL_IM_ENTERROOM_RESULT
{
	CHAR	szGameName[GAMENAME_LEN];
	CHAR	szRoomName[GAMEROOM_LEN];
	UINT	uFriendID;
	UINT	uKindID;
	UINT	uNameID;
	UINT	uRoomID;
};
/*******************************************assid_hall_im_enterroom***********************************************/

/*******************************************assid_hall_im_leaveroom***********************************************/
struct NET_HALL_IM_LEAVEROOM_RESULT
{
	UINT	uFriendID;
};
/*******************************************assid_hall_im_leaveroom***********************************************/

/*******************************************assid_hall_im_friendtalk***********************************************/
struct S_FontInfo				
{
public:
	CHAR	szName[NAME_LENGTH];	// 字体名称
	INT		iSize;	// 字体大小
	INT		iBold;	//是否粗体
	INT		iItalic;//是否斜体
	INT		iUnderLine;//是否下划线
	ULONG	clrText;// 字体颜色
	S_FontInfo()
	{
		iSize = 9;
		clrText = RGB(0,0,0);
		sprintf_s(szName, NAME_LENGTH, "宋体");
		iBold = 0;
		iItalic = 0;
		iUnderLine = 0;
	}
};
struct NET_HALL_IM_FRIENDTALK
{
	CHAR	szMsg[TALK_CONTENT_LEN];//聊天内容
	UINT	uSrcUserID;	//发送者
	UINT	uDestUserID;//接收者ID
	LLONG	t_time;		//聊天时间
	S_FontInfo font;	//字体信息
};
/*******************************************assid_hall_im_friendtalk***********************************************/

/*******************************************assid_hall_im_getsocial***********************************************/
//已经加入了商会才发送此消息包
struct NET_HALL_IM_GETSOCIAL
{
	UINT	uSocialID;	//请求的商会ID
};

struct NET_HALL_IM_GETSOCIAL_RESULT
{
	CHAR	szCreatorNick[NAME_LENGTH];//创建者昵称
	CHAR	szSocialName[NAME_LENGTH];//商会名称
	CHAR	szNotice[SOCIAL_NOTICE_LEN];//商会公告
	UINT	uSocialID;	//商会ID
	UINT	uCreatorID;	//创建者ID
	UINT	uMemCount;	//成员数量
	UINT	uMaxCount;	//成员上限
	LLONG	llCreateTime;//创建时间
};
/*******************************************assid_hall_im_getsocial***********************************************/

/*******************************************assid_hall_im_getsocialmem***********************************************/
enum E_HALL_IM_GETSOCIALMEM_RESULT
{
	rs_hall_im_getsocialmem_error,
	rs_hall_im_getsocialmem_part,	//发送部分成员
	rs_hall_im_getsocialmem_finish,	//发送成员完成 
};
//商会中玩家成员角色
enum E_SOCIAL_USERTYPE
{
	SOCIAL_MEM=1,	//商会成员
	SOCIAL_CREATOR,	//商会创建者
	SOCIAL_MANAGER,	//商会管理员
};

struct NET_HALL_IM_GETSOCIALMEM_RESULT
{
	CHAR	szNick[NAME_LENGTH];//会员昵称
	UINT	dwUserID;	//会员ID
	UINT	uSocialID;	//商会ID
	UINT	uLogoID;	//会员头像
	UINT	uUserType;	//商会角色
	LLONG	llJoinTime;	//入会时间
};
/*******************************************assid_hall_im_getsocialmem***********************************************/

/*******************************************assid_hall_im_createsocial***********************************************/
enum E_HALL_IM_CREATESOCIAL_RESULT
{
	rs_hall_im_createsocial_error,
	rs_hall_im_createsocial_ok,
	rs_hall_im_createsocial_limit,	//等级限制
	rs_hall_im_createsocial_othersocial,//已经加入了其他商会
};

struct NET_HALL_IM_CREATESOCIAL
{
	CHAR	szSocialName[NAME_LENGTH];	//商会名称
};

struct NET_HALL_IM_CREATESOCIAL_RESULT
{
	UINT	uSocialID;	//商会ID
};
//创建失败时返回
struct NET_HALL_IM_CREATESOCIAL_LIMIT_RESULT
{
	CHAR	szSocialName[NAME_LENGTH];	//商会名称
};
/*******************************************assid_hall_im_createsocial***********************************************/

/*******************************************assid_hall_im_joinsocial***********************************************/
enum E_HALL_IM_JOINSOCIAL_RESULT
{
	rs_hall_im_joinsocial_error,
	rs_hall_im_joinsocial_ok,
	rs_hall_im_joinsocial_refuse,	//被拒绝
	rs_hall_im_joinsocial_rightlimit,//被请求者权限不足
	rs_hall_im_joinsocial_othersocial,//已经加入了其他商会
	rs_hall_im_joinsocial_full,		//商会已满员
};

struct NET_HALL_IM_JOINSOCIAL
{
	CHAR	szNick[NAME_LENGTH];//请求者昵称
	UINT	uSocialID;	//请求加入的商会ID
};

struct NET_HALL_IM_JOINSOCAL_RESULT
{
	UINT	uSocialID;	//成功加入的商会ID
};

/*******************************************assid_hall_im_joinsocial***********************************************/

/*******************************************assid_hall_im_joinsocial_trans***********************************************/
struct NET_HALL_IM_JOINSOCIAL_TRANS_RESULT
{
	CHAR	szNick[NAME_LENGTH];//请求者昵称
	UINT	uSocialID;	//请求加入的商会ID
	UINT	uSrcUserID;	//请求者玩家ID
};

struct NET_HALL_IM_JOINSOCIAL_TRANS
{
	BYTE	byResult;
	UINT	uSocialID;
	UINT	uSrcUserID;
};
/*******************************************assid_hall_im_joinsocial_trans***********************************************/

/*******************************************assid_hall_im_quitsocial***********************************************/
enum E_HALL_IM_QUITSOCIAL_RESULT
{
	rs_hall_im_quitsocial_error,
	rs_hall_im_quitsocial_ok,
};
struct NET_HALL_IM_QUITSOCIAL
{
	CHAR	szNick[NAME_LENGTH];//请求者昵称
	UINT	uSocialID;	//请求退出的商会ID
};

struct NET_HALL_IM_QUITSOCIAL_RESULT
{
	CHAR	szNick[NAME_LENGTH];//请求者昵称
	UINT	dwUserID;	//请求者ID
	UINT	uSocialID;	//请求退出的商会ID
};
/*******************************************assid_hall_im_quitsocial***********************************************/

/*******************************************assid_hall_im_kicksocial***********************************************/
enum E_HALL_IM_KICKSOCIAL_RESULT
{
	rs_hall_im_kicksocial_error,
	rs_hall_im_kicksocial_ok,
	rs_hall_im_kicksocial_rightlimit,	//权限不够
};
struct NET_HALL_IM_KICKSOCIAL
{
	CHAR	szNick[NAME_LENGTH];//被请出者昵称
	UINT	dwUserID;//被请出者ID
	UINT	uSocialID;//被请出的商会ID
};

struct NET_HALL_IM_KICKSOCIAL_RESULT
{
	CHAR	szNick[NAME_LENGTH];//被请出者昵称
	UINT	dwUserID;//被请出者ID
	UINT	uSocialID;//被请出的商会ID
};
/*******************************************assid_hall_im_kicksocial***********************************************/

/*******************************************assid_hall_im_disband**************************************************/
enum E_HALL_IM_DISBANDSOCIAL_RESULT
{
	rs_hall_im_disbandsocial_error,
	rs_hall_im_disbandsocial_ok,
	rs_hall_im_disbandsocial_rightlimit,	//权限不够
};

struct NET_HALL_IM_DISBANDSOCIAL
{
	UINT	uSocialID;	//商会ID
};

struct NET_HALL_IM_DISBANDSOCIAL_RESULT
{
	CHAR	szSocialName[NAME_LENGTH];//被解散的商会名
	UINT	uSocialID;	//商会ID
};
/*******************************************assid_hall_im_disband**************************************************/

/*******************************************assid_hall_im_disband***********************************************/
struct NET_HALL_IM_DISBAND
{
	UINT	uSocialID;	//被解散的商会ID
};

struct NET_HALL_IM_DISBAND_RESULT
{
	CHAR	szSocialName[NAME_LENGTH];
	UINT	uSocialID;	//被解散的商会ID
};
/*******************************************assid_hall_im_disband***********************************************/

/*******************************************assid_hall_im_socialtalk***********************************************/
struct NET_HALL_IM_SOCIALTALK
{
	CHAR	szMsg[TALK_CONTENT_LEN];//聊天内容
	UINT	uSrcUserID;	//发送者
	UINT	uSocialID;	//商会ID
	S_FontInfo font;	//字体信息
};
/*******************************************assid_hall_im_socialtalk***********************************************/

/*******************************************assid_hall_im_socialnotice***********************************************/
enum E_HALL_IM_SOCIALNOTICE_RESULT
{
	rs_hall_im_socialnotice_error,
	rs_hall_im_socialnotice_ok,
	rs_hall_im_socialnotice_rightlimit,	//权限不够
};
struct NET_HALL_IM_SOCIALNOTICE
{
	CHAR	szNotice[SOCIAL_NOTICE_LEN];//公告内容
	UINT	uSocialID;	//商会ID
};

struct NET_HALL_IM_SOCIALNOTICE_RESULT
{
	CHAR	szNotice[SOCIAL_NOTICE_LEN];//公告内容
	UINT	uSocialID;	//商会ID
};
/*******************************************assid_hall_im_socialnotice***********************************************/

/*******************************************assid_hall_register_normal/assid_hall_register_fast***********************************************/
enum E_HALL_REGISTER_RESULT
{
	rs_hall_register_error,
	rs_hall_register_ok,
	rs_hall_register_name_exist,//用户名已存在
};
struct NET_HALL_REGISTER_NORMAL
{
	CHAR	szUserName[NAME_LENGTH];//用户名
	CHAR	szPass[PASSWORD_MD5_LENGTH];//密码
};

struct NET_HALL_REGISTER_FAST
{
	CHAR	szMacID[MAC_LEN];	//设备ID
};

struct NET_HALL_REGISTER_RESULT
{
	CHAR	szUserName[NAME_LENGTH];//用户名
	CHAR	szPass[PASSWORD_MD5_LENGTH];//密码
};
/*******************************************assid_hall_register_normal/assid_hall_register_fast***********************************************/

/*******************************************assid_hall_lucky_content***********************************************/
enum E_HALL_LUCKY_CONTENT_TYPE
{
	LUCKY_PROP=1,	//道具
	LUCKY_EXP,		//经验值
	LUCKY_MONEY,	//龙豆
	LUCKY_DRAGION,	//龙币	
};
struct NET_HALL_LUCKY_CONTENT_RESULT
{
	CHAR	szContentName[NAME_LENGTH];//奖品名称
	BYTE	byID;	//奖品ID
	BYTE	byContentType;//类型，见E_HALL_LUCKY_CONTENT_TYPE
	BYTE	byPropID;//如果奖品为道具，则此值为道具ID，否则为0
	UINT	uNum;	//数量
};
/*******************************************assid_hall_lucky_content***********************************************/

#pragma pack()

#endif