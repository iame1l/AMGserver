//Copyright (c) 2012—2015 Beijing TianRuiDiAn Network Technology Co,Ltd. All rights reserved
#ifndef _HALL_DATABASE_IO_H_
#define _HALL_DATABASE_IO_H_


//数据库操作类型
//数据库操作类型
enum E_HALL_DATABASE_HANDLE_TYPE
{
	// 大厅登陆注册
	DTK_GP_LOGON_BEGIN = 1000,
		DTK_GP_LOGON_BY_NAME,							// 通过名字登陆
		DTK_GP_LOGON_BY_ACC,							// 通过 ACC登陆
		DTK_GP_LOGON_BY_MOBILE,							// 通过手机登陆
		DTK_GP_RIGSTER_USER,							// 用户注册处理
		DTK_GP_REGISTER,								// 用户注册
		DTK_GP_RESET_SQL,								// 通知GServer重启数据库
		DTK_GR_CHECK_VERSION,							// 检查版本
	DTK_GP_LOGON_END,

	// 房间列表更新
	DTK_GP_LIST_BEGIN,
		DTK_GP_UPDATE_LIST,								// 更新游戏列表
		DTK_GP_UPDATE_STRING,							// 更新限制字符
		DTK_GP_CHECK_CONNECT,							// 检测断线连接
		DTK_GP_CHECK_ROOM_PASSWD,						// 进入密码房间
		DTK_GP_ZRECORD,									// 修改写Z登录记录的时机，确保网络卡的时候，不至于导致用户登录不上
		DTK_GP_LASTLOGINGAME,							// 最近登录游戏
		DTK_GP_GET_ROOM_LIST_PEOCOUNT,					// 更新房间人数
		DTK_GP_LOGONPEOPLE_GET,							// 获取登录人数
		DTK_GP_CONTEST_APPLY_LIST,						// 获取比赛报名信息
		DTK_GP_GET_CONFIG,
		DTK_GP_UPDATE_CONNECT,							// 更新单个房间连接状态
		DTK_GP_UPDATE_CONNECT_ALL,						// 更新多个房间连接状态
        DTK_GP_DISSMISS_TIMEOUTDESK,				    // 时效房间超时解散
	DTK_GP_LIST_END,

	// 用户个人信息设置
	DTK_GP_UPDATE_USERINFO_BEGIN,
		DTK_GP_LOCK_UNLOCK_ACCOUNT,						// 锁定机器或解除锁定
		DTK_GP_BIND_MOBILE,								// 手机绑定
		DTK_GP_UPDATE_USERINFO_BASE,					// 用户基本资料修改
		DTK_GP_UPDATE_USERINFO_DETAIL,					// 用户详细资料修改
		DTK_GP_UPDATE_USERINFO_PWD,						// 用户密码修改
		DTK_GP_USERINFO_FORGET_PWD,						// 忘记密码
		DTK_GP_UPDATE_NICKNAMEID,						// 查询某用户的昵称或ID
		DTK_GP_SET_LOGO,								// 设置用户头像
		DTK_GP_USERLOGO_UPDATE,							// 更新头像ID
		DTK_GP_USERLOGO_GETMD5,							// 取头像的MD5值
        DTK_GP_USER_ADDPHONENUM,                        // 用户账号关联手机号
	DTK_GP_UPDATE_USERINFO_END,

	// 活动及福利领取
	DTK_GP_CAMPAIGN_BEGIN,
		DTK_GP_RECEIVEMONEY,							// 领取金币
	DTK_GP_CAMPAIGN_END,

	// 魅力值
	DTK_GP_CHARM_BEGIN,
		DTK_GP_CHARMEXCHANGE_LIST,						// 魅力值兑换
		DTK_GP_CHARMEXCHANGE,							// 魅力值兑换
	DTK_GP_CHARM_END,

	// 好友相关
	DTK_GP_IM_BEGIN,
		DTK_GP_SETFRIENDTYPE,							// 修改用户好友和聊天的授受类型
		DTK_GP_IM_CLEAR,								// 服务器启动时，用户清0，单服务器适用
		DTK_GP_IM_GETCOUNT,								// 取得用户好友数
		DTK_GP_IM_USERLIST,								// 返回用户的好友信息(列表)
		DTK_GP_IM_USERONLINE,							// 上、下线返回用户资料
		DTK_GP_IM_ONLINE,								// 上线向用户发资料
		DTK_GP_IM_OFFLINE,								// 下线向数据库写记录
		DTK_GP_IM_SETUSER,								// 设置好友分组
	DTK_GP_IM_END,

	// 比赛房间
	DTK_GP_CONTEST_BEGIN,
		DTK_GP_GET_CONTEST_ROOMID,						// 获取比赛房间ID
		DTK_GP_UPDATE_CONTESTPEOPLE,					// 刷新比赛列表人数
		DTK_GP_GET_APPLY_NUM,							// 获取已报名人数
		DTK_GP_CONTEST_APPLY,							// 比赛报名
		DTK_GP_CONTEST_NOTICE,							// 比赛前通知
		DTK_GP_CONTEST_NOTICE_LOGON,					// 玩家登陆通知比赛
		DTK_GP_CONTEST_GETAWARD,						// 获取比赛奖励
	DTK_GP_CONTEST_END,

	// 排行榜
	DTK_GP_PAIHANGBANG_BEGIN,
		DTK_GP_PAIHANGBANG,								// 排行榜
	DTK_GP_PAIHANGBANG_END,

	// 签到
	DTK_GP_SIGNIN_BEGIN,
		DTK_GP_SIGNIN_CHECK,							// 查询签到
		DTK_GP_SIGNIN_DO,								// 执行签到
	DTK_GP_SIGNIN_END,

	// 在线奖励
	DTK_GP_ONLINE_AWARD_BEGIN,
		DTK_GP_ONLINE_AWARD_CHECK,						// 查询在线奖励
		DTK_GP_ONLINE_AWARD_DO,							// 领取在线奖励
	DTK_GP_ONLINE_AWARD_END,

	// 银行相关
	DTK_GP_BANK_BEGIN,
		DTK_GP_BANK_OPEN,								// 查询余额
		DTR_GP_BANK_OPEN_SUC,							// 打开钱柜成功
		DTR_GP_BANK_OPEN_ERROR,							// 打开钱柜错误
		DTK_GP_BANK_GETGM_WALLET,						// 打开钱包
		DTK_GP_BANK_CHECK,								// 存取
		DTK_GP_BANK_TRANSFER,							// 转账
		DTK_GP_BANK_TRANS_RECORD,						// 转账记录
		DTK_GP_BANK_DEPOSIT,							// 存单业务
		DTK_GP_BANK_QUERY_DEPOSIT,						// 查询存单
		DTK_GP_BANK_CHPWD,								// 修改银行密码
		DTK_GP_BANK_FINANCE_INFO,						// 获取个人财务情况
		DTK_GP_BANK_UPDATE_REBATE,						// 更新VIP转账限制数据
		DTK_GP_BANK_UPDATE_USER_REBATE,					// 更新玩家返利数据
		DTK_GP_BANK_SET_USER_REBATE,					// 玩家修改返利数据
	DTK_GP_BANK_END,

	// GM相关
	GM_DT_MESSAGE_START,								
		DTK_GET_AWARD_TYPE_GM,							// 获取奖励方案
		DTK_GET_OPEATED_USER_GM,						// 获取指定的玩家的被操作过的信息及当前状态
		DTK_PUT_OPEATION_USER_GM,						// 写入对玩家的操作
		DTK_GET_PROP_INFO_GM,							// 获取道具ID与名称对应关系列表
		DTK_LOGON_BY_NAME_GM,							// GM采用GM工具登录
	GM_DT_MESSAGE_END,									

	// 道具相关
	DTK_GP_PROP_BEGIN,
		DTK_GP_PROP_GETPROP,							// 更新头像ID
		DTK_GR_PROP_USE,								// 使用道具
		DTK_GP_PROP_BUY,								// 购买道具
		DTK_GP_PROP_GIVE,								// 赠送道具
		DTK_GPR_PROP_BIG_BRD,							// 大喇叭
		DTK_GPR_PROP_BIG_BRD_BUYADNUSE,					// 大喇叭
		DTK_GPR_PROP_SMALL_BRD,							// 小喇叭
		DTK_GP_PROP_BUY_NEW,							// 商店道具的即时购买功能
		DTK_GP_PROP_BUY_VIP,							// 道具面板VIP购买
	DTK_GP_PROP_END,

	//创建房间相关设置
	DTK_GP_VIP_DESK_BEGIN,
		DTK_GP_BUY_DESK,								// 购买VIP桌子
		DTK_GP_ENTER_VIPDESK,							// 进入VIP桌子
		DTK_GP_TOTALRECORD,								// 获取房间总战绩
		DTK_GP_SINGLERECORD,							// 获取房间单局战绩
		DTK_GP_GET_CUTROOM,								// 获取断线房间信息
		DTK_GP_GET_BUYDESKCONFIG,						// 获取创建房间规则
		DTK_GP_GET_RECORDURL,							// 获取回放地址
		DTK_GP_GET_DESKLIST,							// 获取桌子列表
		DTK_GP_GET_DESKUSER,							// 获取桌子玩家信息
		DTK_GP_DISSMISSDESK,							// 解散房间
		DTK_GP_DELETERECORD,							// 删除桌子信息
        DTK_GP_DISSMISSDESKBYMS,                        // 删除时效超时的房间         
	DTK_GP_VIP_DESK_END,

	//兑换金币相关
	DTK_GP_MONEY_CHANGE_BEGIN,
		DTK_GP_CHANGE_CONFIG,							// 获取兑换比例
		DTK_GP_MONEY_CHANGE,							// 金币兑换请求
	DTK_GP_MONEY_CHANGE_END,

	//俱乐部相关
	DTK_GP_CLUB_BEGIN,
		DTK_GP_CREATE_CLUB,						// 创建俱乐部
		DTK_GP_DISSMISS_CLUB,					// 解散俱乐部
		DTK_GP_JOIN_CLUB,						// 申请加入俱乐部
		DTK_GP_CLUB_USERLIST,					// 俱乐部玩家列表
		DTK_GP_CLUB_ROOMLIST,					// 俱乐部房间列表
		DTK_GP_CLUB_CREATEROOM,					// 俱乐部创建房间
		DTK_GP_CLUB_CHANGENAME,					// 俱乐部更名
		DTK_GP_CLUB_KICKUSER,					// 会长踢人
		DTK_GP_CLUB_STATISTICS,					// 俱乐部统计信息
		DTK_GP_CLUB_LIST,						// 俱乐部列表
		DTK_GP_REVIEW_LIST,						// 会长审核列表
		DTK_GP_MASTER_OPTION,					// 会长审核操作
		DTK_GP_CLUB_NOTICE,						// 俱乐部公告
		DTK_GP_ENTER_CLUB,						// 进入俱乐部（开始接受消息）
		DTK_GP_LEAVE_CLUB,						// 退出俱乐部
		DTK_GP_GET_DESKRECORD,					// 获取开房记录
	DTK_GP_CLUB_END,

	//邮件相关
	DTK_GP_MAIL_BEGIN,
		DTK_GP_MAIL_LIST,						// 获取邮件列表
		DTK_GP_OPEN_MAIL,						// 打开邮件
		DTK_GP_GET_ATTACHMENT,					// 获取附件
		DTK_GP_DEL_MAIL,						// 删除邮件
		DTK_GP_UPDATE_MAIL,						// 
		DTK_GP_UPDATE_SYSTEM,					// 
	DTK_GP_MAIL_END,

    //转盘抽奖
    DTK_GP_LUCK_DRAW_CONFIG,                            //获取奖励配置
    DTK_GP_LUCK_DRAW_DO,                                 //执行抽奖

    //验证手机号是否匹配
    DTK_GP_MOBILE_IS_MATCH,

    // 获取房间指定设置信息
    DTK_GP_GET_SPROOMINFO
};

enum E_HALL_HANDLE_RESULT_PSWROOM
{
	DTK_GP_PASSWD_SUCC = 0,								// 进入密码成功
	DTK_GP_PASSWD_ERROR = 1,							// 进入密码错误
};

enum E_HALL_HANDLE_RESULT_LOGON
{														//大厅登陆错误代码
	DTR_GP_ERROR_UNKNOW = 0,							//未知错误
	DTR_GP_DATABASE_CUT = 1,							//数据库断开
	DTR_GP_LIST_SCUESS = 18,							//更新列表成功
	DTR_GP_LIST_ERROR = 19,								//更新列表失败
	DTR_GP_STRING_SCUESS = 20,							//更新名字成功
	DTR_GP_STRING_ERROR = 21,							//更新名字失败

	DTR_GP_GET_ROOM_LIST_PEOCOUNT_SCUESS = 23,			
	DTR_GP_GET_ROOM_LIST_PEOCOUNT_ERROR = 24,			

	DTR_GP_LOCK_SUCCESS = 30,							//锁定成功	
	DTR_GP_MATHINE_UNLOCK = 31,							//帐号根本就没锁定某台机器
	DTR_GP_UNLOCK_SUCCESS = 32,							//解锁成功
	DTR_GP_CODE_ERROR = 33,								//机器码不匹配

	DTR_GP_LOCK_VALID = 38,								//锁机验证

	DTR_GP_UPDATE_USERINFO_DETAIL = 43,					//用户详细资料修改
	DTR_GP_UPDATE_USERINFO_BASE = 41,					//用户基本资料修改
	DTR_GP_UPDATE_USERINFO_PWD = 45,					//用户密码修改

	DTR_GP_UPDATE_USERINFO_ACCEPT = 47,					//用户详细资料已这被服务器收受
	DTR_GP_UPDATE_USERINFO_NOTACCEPT = 48,				//用户详细资料未这被服务器收受
};

#endif