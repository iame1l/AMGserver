/****************************************************************************
Copyright (c) 2014-2017 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2017 ShenZhen Redbird Network Polytron Technologies Inc.
 
http://www.hotniao.com

All of the content of the software, including code, pictures, 
resources, are original. For unauthorized users, the company 
reserves the right to pursue its legal liability.
****************************************************************************/
#ifndef SERVERMANGE_H
#define SERVERMANGE_H

#include "GameDesk.h"

#include "ListenerController.h"
#include "Timer.h"
#include "DataManage.h"
#include "GameContext.h"
#define GF_NORMAL					10
#define GF_SAFE						11	
#define GF_AHEAD_END                12        

#define TIME_UPDATE					10				// 游戏更新循环定时器ID
#define ID_TIMER_GAMEBEGIN          20
		
#define ID_TIMER_GAME_NEW_GAME      21              //下一局

//游戏桌类
class CServerGameDesk : public CGameDesk
{
public:
	CServerGameDesk(); 
	virtual ~CServerGameDesk();

	//重载函数
public:
	//游戏开始
	virtual bool GameBegin(uchar bBeginFlag);
	//游戏结束
	virtual bool GameFinish(uchar bDeskStation, uchar bCloseFlag);
	//判断是否正在游戏
	virtual bool IsPlayGame(uchar bDeskStation);
	//游戏数据包处理函数
	virtual bool HandleNotifyMessage(uchar bDeskStation, 
									 NetMessageHead * pNetHead, 
									 void * pData, 
									 uint uSize, 
									 uint uSocketID, 
									 bool bWatchUser);
	virtual uchar UserSitDesk(MSG_GR_S_UserSit * pUserSit, CGameUserInfo * pUserInfo);
	//用户离开游戏桌
	virtual uchar UserLeftDesk(uchar bDeskStation, CGameUserInfo * pUserInfo);
	///用户断线离开
	virtual bool UserNetCut(uchar bDeskStation, CGameUserInfo * pLostUserInfo);
	///用户断线重来
	virtual bool UserReCome(uchar bDeskStation, CGameUserInfo * pNewUserInfo);
    ///框架消息
	virtual bool HandleFrameMessage(uchar bDeskStation, 
									NetMessageHead * pNetHead, 
									void * pData, 
									uint uSize, 
									uint uSocketID, 
									bool bWatchUser);
	//用来改变用户断线条件的函数
	virtual bool CanNetCut(uchar bDeskStation);

public:
	void gameinfo();
	//获取游戏状态信息
	virtual bool OnGetGameStation(uchar bDeskStation, uint uSocketID, bool bWatchUser);

	void SetGameStation(BYTE station);

	//检查是否超端玩家
	bool	IsSuperUser(BYTE byDeskStation);

	//验证是否超端
	void	SpuerExamine(BYTE byDeskStation);


	//重置游戏状态
	virtual bool ReSetGameState(uchar bLastStation);
	//定时器消息
	virtual bool OnTimer(uint uTimerID);
	//配置INI文件
	virtual bool InitDeskGameStation();

	virtual bool SetRoomPond(bool	bAIWinAndLostAutoCtrl){return false;}
	virtual bool SetRoomPondEx(__int64	iAIWantWinMoney[], int	iAIWinLuckyAt[]){return false;}
	virtual bool SetRoomPondEx(__int64	iReSetAIHaveWinMoney){return false;}
	virtual BOOL Judge(){return FALSE;}
	virtual BOOL JudgeWiner(){return FALSE;}
	virtual BOOL JudgeLoser(){return FALSE;}
	virtual void UpDataRoomPond(__int64 iAIHaveWinMoney){}

	uchar  getGameStation();
	void  setGameStation(uchar bGameState);
	bool isCreatedRoom();
	bool isCreatedRoomBegin();
	bool isCreatedRoomEnd();
	//释放房间资源时绩录玩家信息
	void ReleaseRoomBefore();
	//恢复房间数据
	void SetReturnGameInfo(int UserID,void* szUserGameInfo,int iSize,int iCount);
	int  getRoomDeskFlag();

	bool send2all(char *pData,size_t size,uint assID);
	bool send2(byte bSeatNO,char *pData,size_t size,uint assID);
	bool sendState(byte bSeatNO,char *pData,size_t size,uint uSocketID, bool bWatcher);	

	void StartUpdateLoop(bool b);
	void GameInit();
	void GameClear();
	void GameReset();

	void InitLogFile();

	int GetRoomID() const
	{
		return m_pDataManage->m_InitData.uRoomID;
	}

	int GetDeskID() const
	{
		return m_bDeskIndex;
	}

	bool GetDisMissed()const
	{
		return m_Dismissed;
	}

	void SetDisMissed(bool flag)
	{
		m_Dismissed = flag;
	}

public:
	GameContext		   *m_GameContext;
	SuperUserSetData	m_SuperSetData[PLAY_COUNT];	
	bool				m_GameStarted;
	
private:

	bool m_TimerStarted;
	bool m_Dismissed;
	int m_LatestUpdateTime;

};

#endif //SERVERMANGE_H
