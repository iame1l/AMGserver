#include "StdAfx.h"
#include "ServerManage.h"
#include <math.h>
#include <time.h>
#include "../GameMessage/Util.h"
/*---------------------------------------------------------------------------*/
/*
* 打印日志文件
*/
void DebugPrintf(int iRoomID, const char *p, ...)
{
	return;
	CTime time = CTime::GetCurrentTime();
	char szFilename[256];
	sprintf(szFilename, "%d_%d-%d%d%d-log.txt", NAME_ID, iRoomID, time.GetYear(), time.GetMonth(), time.GetDay());
	FILE *fp = fopen(szFilename, "a");
	if (NULL == fp)
	{
		return;
	}
	va_list arg;
	va_start(arg, p);
	vfprintf(fp, p, arg);
	fprintf(fp,"\n");

	fclose(fp);
}
//全局变量定义
//全局变量定义
ULONG CServerGameDesk::g_Administrator = 0;
__int64	CServerGameDesk::G_i64CurrStorMoney=0;                 
__int64  CServerGameDesk::G_i64Pond=0;	
bool CServerGameDesk::g_IsLoadSetting = false;


vector<TMSG_USER_DATA> CServerGameDesk::m_vWhitePlayer;
vector<TMSG_USER_DATA> CServerGameDesk::m_vBlackPlayer;
vector<TMSG_FULL_SCREEN_DATA> CServerGameDesk::m_vFullScreenPlayer;

void CServerGameDesk::SaveSetting()
{
	TCHAR skin[MAX_NAME_INFO];
	CString s = CINIFile::GetAppPath ();/////本地路径
	CString temp = _T("");
	temp.Format("%s%s_s.ini",s,GET_SKIN_FOLDER(skin));
	if (!CINIFile::IsFileExist(temp))
	{
		return;
	}

	CINIFile f(temp);
	CString key;
	CString sText;
	CString sValue;
	long lUserID = 0;

	key = TEXT("game");
	CString csPond;
	csPond.Format("%I64d",G_i64Pond);
	f.SetKeyValString(key,"PondMoney",csPond);
	//超级用户名单
	key = TEXT("SuperUser");
	for(int i = 1; i <= CONTROL_USER_MAX; i++)
	{

		if(i > m_vWhitePlayer.size())
		{
			sText.Format("SuperUserID%d", i);
			f.SetKeyValString(key,sText,"");

			sText.Format("LimitScore%d", i);
			f.SetKeyValString(key,sText,"");

			sText.Format("IsWin%d", i);
			f.SetKeyValString(key,sText,"");
			continue;
		}
		sText.Format("SuperUserID%d", i);
		sValue.Format("%d",m_vWhitePlayer[i-1].userID);
		f.SetKeyValString(key,sText,sValue);

		sText.Format("LimitScore%d",i);
		sValue.Format("%d",m_vWhitePlayer[i-1].moneyLimit);
		f.SetKeyValString(key,sText,sValue);

		sText.Format("IsWin%d",i);
		sValue.Format("%d",m_vWhitePlayer[i-1].isWin);
		f.SetKeyValString(key,sText,sValue);
	}

	//黑名单
	key = TEXT("BlackList");
	//黑名单个数
	for(int i = 1; i <= CONTROL_USER_MAX; i++)
	{
		if(i > m_vBlackPlayer.size())
		{
			sText.Format("BlackListid%d", i);
			f.SetKeyValString(key,sText,"");

			sText.Format("LimitScore%d", i);
			f.SetKeyValString(key,sText,"");

			sText.Format("IsWin%d", i);
			f.SetKeyValString(key,sText,"");
			continue;
		}
		sText.Format("BlackListid%d", i);
		sValue.Format("%d",m_vBlackPlayer[i -1].userID);
		f.SetKeyValString(key,sText,sValue);

		sText.Format("LimitScore%d",i);
		sValue.Format("%d",m_vBlackPlayer[i-1].moneyLimit);
		f.SetKeyValString(key,sText,sValue);

		sText.Format("IsWin%d",i);
		sValue.Format("%d",m_vBlackPlayer[i-1].isWin);
		f.SetKeyValString(key,sText,sValue);
	}

	key = TEXT("fullscreen");
	for(int i = 1; i <= CONTROL_USER_MAX; i++)
	{
		if(i > m_vFullScreenPlayer.size())
		{
			sText.Format("GameID%d",i);
			f.SetKeyValString(key, sText, "");

			sText.Format("ScoreLimit%d",i);
			f.SetKeyValString(key, sText, "");

			sText.Format("TypeID%d",i);
			f.SetKeyValString(key, sText, "");

			sText.Format("Times%d",i);
			f.SetKeyValString(key, sText, "");

			continue;
		}
		sText.Format("GameID%d",i);
		sValue.Format("%d",m_vFullScreenPlayer[i - 1].stUserData.userID);
		f.SetKeyValString(key, sText, sValue);

		sText.Format("ScoreLimit%d",i);
		sValue.Format("%d",m_vFullScreenPlayer[i - 1].stUserData.moneyLimit);
		f.SetKeyValString(key, sText, sValue);

		sText.Format("TypeID%d",i);
		sValue.Format("%d",m_vFullScreenPlayer[i - 1].typeID);
		f.SetKeyValString(key, sText, sValue);

		sText.Format("Times%d",i);
		sValue.Format("%d",m_vFullScreenPlayer[i - 1].count);
		f.SetKeyValString(key, sText, sValue);
	}
}

void CServerGameDesk::LoadSetting()
{

	TCHAR skin[MAX_NAME_INFO];
	CString s = CINIFile::GetAppPath ();/////本地路径
	CString temp = _T("");
	temp.Format("%s%s_s.ini",s,GET_SKIN_FOLDER(skin));
	if (!CINIFile::IsFileExist(temp))
	{
		return;
	}

	CINIFile f(temp);
	CString key;
	key=TEXT("game");
	G_i64CurrStorMoney = f.GetKeyVal(key,"StoreMoney",0);
	G_i64Pond = f.GetKeyVal(key,"PondMoney",0);

	key = TEXT("administrator");
	g_Administrator = f.GetKeyVal(key,"loginID",00000);

	CString sText;
	//超级用户名单
	key = TEXT("SuperUser");
	for(int i = 1; i <= CONTROL_USER_MAX; i++)
	{
		TMSG_USER_DATA stUserData;
		sText.Format("SuperUserID%d", i);
		stUserData.userID = f.GetKeyVal(key, sText, 0);

		sText.Format("LimitScore%d",i);
		stUserData.moneyLimit = f.GetKeyVal(key, sText, 0);

		sText.Format("IsWin%d",i);
		stUserData.isWin = f.GetKeyVal(key, sText, 0);
		if (stUserData.userID > 0)
		{
			m_vWhitePlayer.push_back( stUserData );
		}
	}

	//黑名单
	key = TEXT("BlackList");
	//黑名单个数
	for(int i = 1; i <= CONTROL_USER_MAX; i++)
	{
		TMSG_USER_DATA stUserData;
		sText.Format("BlackListID%d", i);
		stUserData.userID = f.GetKeyVal(key, sText, 0);

		sText.Format("LimitScore%d",i);
		stUserData.moneyLimit = f.GetKeyVal(key, sText, 0);

		sText.Format("IsWin%d",i);
		stUserData.isWin = f.GetKeyVal(key, sText, 0);
		if (stUserData.userID > 0)
		{
			m_vBlackPlayer.push_back( stUserData );
		}
	}

	key = TEXT("fullscreen");
	for(int i = 1; i <= CONTROL_USER_MAX; i++)
	{
		TMSG_FULL_SCREEN_DATA stFullData;
		sText.Format("GameID%d",i);
		stFullData.stUserData.userID = f.GetKeyVal(key, sText, 0);

		sText.Format("ScoreLimit%d",i);
		stFullData.stUserData.moneyLimit = f.GetKeyVal(key, sText, (__int64)0);

		sText.Format("TypeID%d",i);
		stFullData.typeID = f.GetKeyVal(key, sText, 0);

		sText.Format("Times%d",i);
		stFullData.count = f.GetKeyVal(key, sText, 0);

		if (stFullData.stUserData.userID > 0)
		{
			m_vFullScreenPlayer.push_back( stFullData );
		}
	}
	g_IsLoadSetting = true;
}
/*---------------------------------------------------------------------------*/
//构造函数
CServerGameDesk::CServerGameDesk(void):CGameDesk(0)
{
	m_bGameStation = GS_WAIT_ARGEE;

	m_bClearPreWin	= true;

	m_i64UserMoney	= 0;		//玩家身上的金币数	
	m_iAllWinMoney  = 0;        //总体赢钱
	m_bWinAndLostAutoCtrl = 1;
	m_iChangeRate = 1;
	//当前时间
	m_NowTime = GetTickCount();
	InitializeCriticalSection(&m_csForIPCMessage);
}
/*---------------------------------------------------------------------------*/
//析构函数
CServerGameDesk::~CServerGameDesk(void)
{
	DeleteCriticalSection(&m_csForIPCMessage);
}

/*---------------------------------------------------------------------------*/
bool CServerGameDesk::InitDeskGameStation()
{
	if(!g_IsLoadSetting)
		LoadSetting();

	//G_i64Pond	= 100000;	  //底注
	m_iRewardLeve1	= 100000;				
	m_iRewardLeve2	= 10000000;		     
	m_iRewardLeve3	= 1000000000;	

	m_iAIWinLuckyAtA1	  = 90;	
	m_iAIWinLuckyAtA2	  = 70;	
	m_iAIWinLuckyAtA3	  = 30;	
	m_iAIWinLuckyAtA4	  = 10;
	m_bWinAndLostAutoCtrl = 1;
	//加载配置文件;
	LoadIni();
	LoadExtIni(m_pDataManage->m_InitData.uRoomID);

	return true;
}
/*---------------------------------------------------------------------------*/
//加载ini配置文件
BOOL	CServerGameDesk::LoadIni()
{
	TCHAR skin[MAX_NAME_INFO];
	CString s = CINIFile::GetAppPath ();/////本地路径
	CString temp = _T("");
	temp.Format("%s%s_s.ini",s,GET_SKIN_FOLDER(skin));

// 	if (!CINIFile::IsFileExist(temp))		//找不到配置文件就采用默认的,不然的话直接返回会读取不到默认的参数,导致进入游戏崩溃
// 	{
// 		return true;
// 	}
	CINIFile f(temp);
	CString key = TEXT("game");
	
	//G_i64Pond	= f.GetKeyVal(key,"ServerBaseMoney", G_i64Pond);	  //底注

	G_i64Pond = f.GetKeyVal(key,"PondMoney",1000);
	m_iAreaBetLimit=f.GetKeyVal(key,"AreaBetLimit",10);

	if(m_iAreaBetLimit<10)
	{
		m_iAreaBetLimit=10;
	}
	m_iChangeRate=f.GetKeyVal(key,"ChangeRate",10);
	if(m_iChangeRate<=0)
	{
		m_iChangeRate=1;
	}

	m_iBeilu_20_10=f.GetKeyVal(key,"Beilu_20_10",50);
	m_iBeilu_30_15=f.GetKeyVal(key,"Beilu_30_15",30);
	m_iBeilu_40_20=f.GetKeyVal(key,"Beilu_40_20",20);

	m_iSmallFruits=f.GetKeyVal(key,"SmallFruits",40);
	if(m_iSmallFruits < 40)
		m_iSmallFruits = 40;

	m_iBigFruits=f.GetKeyVal(key,"BigFruits",30);
	if(m_iBigFruits < 30)
		m_iBigFruits = 30;

	m_LuckEgg=f.GetKeyVal(key,"LuckEgg",20);
	if(m_LuckEgg < 20)
		m_LuckEgg = 20;

	m_iBar=f.GetKeyVal(key,"Bar",10);
	if(m_iBar < 10)
		m_iBar = 10;

	return true;
}
/*---------------------------------------------------------------------------*/
//根据房间ID加载配置文件
BOOL CServerGameDesk::LoadExtIni(int iRoomID)
{	
	return true;
}
/*---------------------------------------------------------------------------*/
//清空该位置的数据
void	CServerGameDesk::IniUserData(BYTE byStation)
{
}

/*---------------------------------------------------------------------------*/
bool	CServerGameDesk::OnTimer(UINT uTimerID)
{
	if ((uTimerID >= TIME_MAX) || (uTimerID <= TIME_MIN))
	{
		return __super::OnTimer(uTimerID);
	}

	return true;
}
/*---------------------------------------------------------------------------*/

bool CServerGameDesk::HandleFrameMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	switch(pNetHead->bAssistantID)
	{
	case ASS_GM_FORCE_QUIT:		//强行退出//安全退出
		{
			return true;
		}
	}
	return __super::HandleFrameMessage( bDeskStation,  pNetHead,  pData,  uSize,  uSocketID,  bWatchUser);
}

//游戏数据包处理函数
bool CServerGameDesk::HandleNotifyMessage(BYTE bDeskStation, NetMessageHead * pNetHead, void * pData, UINT uSize, UINT uSocketID, bool bWatchUser)
{
	if(bWatchUser)
	{
		return true;
	}

	if(bDeskStation < 0 || bDeskStation >= PLAY_COUNT)
	{
		return true;
	}

	switch (pNetHead->bAssistantID)
	{
		// 管理员控制 消息
	case MSG_ADMIN_CONTROL_BASE:
		{
			TMSG_ADMIN_CONTROL_BASE *pContext = static_cast<TMSG_ADMIN_CONTROL_BASE*>(pData);
			if (pContext == NULL)    return false;

			BYTE msgID = pContext->msgID;
			OnMsgAdminiControl( bDeskStation, msgID, pData, uSize, bWatchUser );

			return true;   // 如果返回 false 服务器会踢掉客户端- -！
		}
	case C_S_START_ROLL:		        //启动游戏滚动
		{
			OnHandleStartRoll(bDeskStation, pData, uSize);
			return true;
		}
	case C_S_BIBEI:				//押大小消息
		{
			OnHandleBibei(bDeskStation,pData,uSize);
			return true;
		}
	case C_S_AWARD_LAMP:
		{
			if(m_iLampNo>=m_lvOpenResult.size()-1)
			{
				return true;
			}
			m_iLampNo++;
			S_C_AwardLampResult TAwardResult;
			TAwardResult.iCount=m_lvOpenResult.size()-1-m_iLampNo;
			TAwardResult.iLamp=m_lvOpenResult.at(m_iLampNo);
			if(TAwardResult.iLamp==9||TAwardResult.iLamp==21)
			{
				TAwardResult.iAreaWin=0;
			}
			else 
			{
				//TAwardResult.iAreaWin = m_iAreaWin[G_iLampToBetArea[TAwardResult.iLamp]];
				TAwardResult.iAreaWin = GetPoint(TAwardResult.iLamp);
			}
			for (int i = 0; i < PLAY_COUNT; i ++) 
			{
				SendGameData(i,&TAwardResult,sizeof(TAwardResult), MDM_GM_GAME_NOTIFY, S_C_AWARD_LAMP_REULT, 0);
			}
			return true;
		}
	default:
		{
			return true;
		}
	}
	return __super::HandleNotifyMessage(bDeskStation,pNetHead,pData,uSize,uSocketID,bWatchUser);
}
//处理比倍
void CServerGameDesk::OnHandleBibei(BYTE byStation,void * pData, UINT uSize)
{
	if(uSize!=sizeof(C_S_BiBeiStru))
	{
		return;
	}
	if(m_iAllWinMoney<=0)
	{
		return;
	}
	C_S_BiBeiStru *pBibei=(C_S_BiBeiStru *)pData;
	m_iBibeiType=pBibei->iBibeiType;
	int iBibeiMoney=pBibei->iBibeiMoney,iWinMoney=0;
	if(iBibeiMoney>m_i64UserMoney)
	{
		return;
	}
	m_i64UserMoney-=iBibeiMoney;
	G_i64Pond+=iBibeiMoney;
	
	//比大小的范围
	int iTempResult=0;
	for(int i=0;i<200;i++)
	{
		//srand(GetTickCount()+i);//i控制了种子池
		srand(GetTickCount());//正常种子池
		int irand=rand()%100;
		iTempResult=rand()%2+1;
		if(iTempResult==m_iBibeiType)
		{
			iWinMoney=iBibeiMoney*2;
		}
		else
		{
			iWinMoney=0;
		}
		//if((irand >= 0)&& (irand <= 85))//85以下的概率(因为有i),但15的能赢不变

		if((irand >= 0)&& (irand <= 49))//正常的概率
		{
			if(iWinMoney == 0)
			{
				break;
			}
		}
		else
		{
			//奖池有钱的情况下才给
			if(G_i64Pond - iWinMoney>0)//保证奖池为正
			{
				break;
			}
		}
	}
	if(iWinMoney>0)
	{
		m_i64UserMoney += (iWinMoney);
	}
	
	G_i64Pond-=iWinMoney;

	/*********************************写入配置文件************************************/
	TCHAR skin[MAX_NAME_INFO];
	CString s = CINIFile::GetAppPath ();/////本地路径
	CString temp = _T("");
	temp.Format("%s%s_s.ini",s,GET_SKIN_FOLDER(skin));
	// 	if (!CINIFile::IsFileExist(temp))
	// 	{
	// 		return ;
	// 	}
	CINIFile f(temp);
	CString key = TEXT("game");
	CString csPond;
	csPond.Format("%I64d",G_i64Pond);
	f.SetKeyValString(key,"PondMoney",csPond);
	/**********************************************************************************/


	///*-------------------------------结算-------------------------------------*/

	////计算玩家本局开始前和现在的金币差额 
	__int64 iTmpMoney = m_i64UserMoney*m_iChangeRate - (m_pUserInfo[byStation]->m_UserData.i64Money/m_iChangeRate)*m_iChangeRate;
	
	////写入数据库				
	bool temp_cut[PLAY_COUNT];
	memset(temp_cut,0,sizeof(temp_cut));

	__int64	iTurePoint[PLAY_COUNT];
	memset(iTurePoint,0,sizeof(iTurePoint));
	iTurePoint[byStation] = iTmpMoney;

	//记录玩家金币变化
	__int64		iChangeMoney[PLAY_COUNT];				//玩家金币
	memset(iChangeMoney, 0 , sizeof(iChangeMoney));
	__super::RecoderGameInfo(iChangeMoney);


	ChangeUserPointint64(iTurePoint, temp_cut);

	///*--------------------------------------------------------------------------*/
	S_C_BiBeiResultStru TBiBeiResult;
	TBiBeiResult.iType=iTempResult;
	TBiBeiResult.iwinmoney=iWinMoney;
	
	for(int i=0;i<PLAY_COUNT;i++)
	{
		SendGameData(i,&TBiBeiResult,sizeof(TBiBeiResult),MDM_GM_GAME_NOTIFY,S_C_BIBEI_RESULT,0);
	}
}

//产生结果
void  CServerGameDesk::ProduceResult(int iranddigit) //伪随机数，所以添加一个参数做随机数的种子
{
	m_lvOpenResult.swap(vector<int>());
	//m_lvOpenResult.clear();
	memset(m_bIsVisited,0,sizeof(m_bIsVisited));
	int iResult=getALamp(iranddigit);
	//test
	//iResult = 9;

	m_bIsVisited[iResult]=1;
	m_lvOpenResult.push_back(iResult);
	if(iResult==9||iResult==21)
	{
		srand(clock()+iranddigit);
		int irand=rand()%6;//随机送1-5个灯，0表示吃灯
		if(irand==0)
			return;
		for(int i=0;i<irand;i++)
		{
			int iTmpResult=getALamp(i+iranddigit);
			while(m_bIsVisited[iTmpResult]||iTmpResult==9||iTmpResult==21)
			{
				iTmpResult=getALamp(i+iranddigit+1);
			}
			m_lvOpenResult.push_back(iTmpResult);
			m_bIsVisited[iTmpResult]=1;
		}
	}

}
int CServerGameDesk::getALamp(int iranddigit)//获取灯
{
	int ifirst[7]={5,8,11,14,17,20,23};//优先考虑开小水果
	int isecond[13]={0,1,4,6,7,10,12,13,15,16,18,19,22};//大水果
	int ithird[2]={9,21};//幸运蛋
	int iFourth[2]={2,3};//bar
	srand(GetTickCount()+iranddigit);
	int irand=rand()%100;
	int iresult=0;
	
	if(irand < m_iSmallFruits)
	{
		iresult=ifirst[rand()%7];
		//iresult=isecond[rand()%13];
	}
	else if(irand < m_iSmallFruits + m_iBigFruits)
	{
		iresult=isecond[rand()%13];
		//iresult=ifirst[rand()%7];
	}
	else if(irand < m_iSmallFruits + m_iBigFruits + m_LuckEgg)
	{
		iresult=ithird[rand()%2];
	}
	else 
	{
		iresult=iFourth[rand()%2];
	}
	return iresult;
}
//计算得分
void CServerGameDesk::countfen()
{
	memset(m_iAreaWin,0,sizeof(m_iAreaWin));
	m_iAllWinMoney=0;
	if(m_bIsVisited[2])//bar*50
	{
		m_iAreaWin[0]+=m_iAreaBetNum[0]*50;
		m_iAllWinMoney+=m_iAreaBetNum[0]*50;
	}
	if(m_bIsVisited[3])//bar*100
	{
		m_iAreaWin[0]+=m_iAreaBetNum[0]*100;
		m_iAllWinMoney+=m_iAreaBetNum[0]*100;
	}
	if(m_bIsVisited[14])//小双7
	{
		m_iAreaWin[1]+=3*m_iAreaBetNum[1];
		m_iAllWinMoney+=3*m_iAreaBetNum[1];
	}
	if(m_bIsVisited[15])//大双7
	{
		m_iAreaWin[1]+=m_iLBeilv*m_iAreaBetNum[1];
		m_iAllWinMoney+=m_iLBeilv*m_iAreaBetNum[1];
	}
	if(m_bIsVisited[19])//大双星
	{
		m_iAreaWin[2]+=m_iLBeilv*m_iAreaBetNum[2];
		m_iAllWinMoney+=m_iLBeilv*m_iAreaBetNum[2];
	}
	if(m_bIsVisited[20])//小双星
	{
		m_iAreaWin[2]+=3*m_iAreaBetNum[2];
		m_iAllWinMoney+=3*m_iAreaBetNum[2];
	}
	if(m_bIsVisited[7])//大西瓜
	{
		m_iAreaWin[3]+=m_iLBeilv*m_iAreaBetNum[3];
		m_iAllWinMoney+=m_iLBeilv*m_iAreaBetNum[3];
	}
	if(m_bIsVisited[8])//小西瓜
	{
		m_iAreaWin[3]+=3*m_iAreaBetNum[3];
		m_iAllWinMoney+=3*m_iAreaBetNum[3];
	}
	if(m_bIsVisited[1])//大铃铛
	{
		m_iAreaWin[4]+=m_iRBeilv*m_iAreaBetNum[4];
		m_iAllWinMoney+=m_iRBeilv*m_iAreaBetNum[4];
	}
	if(m_bIsVisited[13])//大铃铛
	{
		m_iAreaWin[4]+=m_iRBeilv*m_iAreaBetNum[4];
		m_iAllWinMoney+=m_iRBeilv*m_iAreaBetNum[4];
	}
	if(m_bIsVisited[23])//小铃铛
	{
		m_iAreaWin[4]+=3*m_iAreaBetNum[4];
		m_iAllWinMoney+=3*m_iAreaBetNum[4];
	}
	if(m_bIsVisited[6])//大柠檬
	{
		m_iAreaWin[5]+=m_iRBeilv*m_iAreaBetNum[5];
		m_iAllWinMoney+=m_iRBeilv*m_iAreaBetNum[5];
	}
	if(m_bIsVisited[18])//大柠檬
	{
		m_iAreaWin[5]+=m_iRBeilv*m_iAreaBetNum[5];
		m_iAllWinMoney+=m_iRBeilv*m_iAreaBetNum[5];
	}
	if(m_bIsVisited[17])//小柠檬
	{
		m_iAreaWin[5]+=3*m_iAreaBetNum[5];
		m_iAllWinMoney+=3*m_iAreaBetNum[5];
	}
	if(m_bIsVisited[0])//大橘子
	{
		m_iAreaWin[6]+=m_iRBeilv*m_iAreaBetNum[6];
		m_iAllWinMoney+=m_iRBeilv*m_iAreaBetNum[6];
	}
	if(m_bIsVisited[12])//大橘子
	{
		m_iAreaWin[6]+=m_iRBeilv*m_iAreaBetNum[6];
		m_iAllWinMoney+=m_iRBeilv*m_iAreaBetNum[6];
	}
	if(m_bIsVisited[11])//小橘子
	{
		m_iAreaWin[6]+=3*m_iAreaBetNum[6];
		m_iAllWinMoney+=3*m_iAreaBetNum[6];
	}
	if(m_bIsVisited[4])//大苹果
	{
		m_iAreaWin[7]+=5*m_iAreaBetNum[7];
		m_iAllWinMoney+=5*m_iAreaBetNum[7];
	}
	if(m_bIsVisited[10])
	{
		m_iAreaWin[7]+=5*m_iAreaBetNum[7];
		m_iAllWinMoney+=5*m_iAreaBetNum[7];
	}
	if(m_bIsVisited[16])
	{
		m_iAreaWin[7]+=5*m_iAreaBetNum[7];
		m_iAllWinMoney+=5*m_iAreaBetNum[7];
	}
	if(m_bIsVisited[22])
	{
		m_iAreaWin[7]+=5*m_iAreaBetNum[7];
		m_iAllWinMoney+=5*m_iAreaBetNum[7];
	}
	if(m_bIsVisited[5])
	{
		m_iAreaWin[7]+=3*m_iAreaBetNum[7];
		m_iAllWinMoney+=3*m_iAreaBetNum[7];
	}
}

int CServerGameDesk::GetPoint(int iType)
{
	int iTemp = 0;
	switch(iType)
	{
	case 2:		//bar*50
		{
			iTemp = m_iAreaBetNum[0]*50;
		}break;
	case 3:		//bar*100
		{
			iTemp = m_iAreaBetNum[0]*100;
		}break;
	case 14:	//小双7
		{
			iTemp = 3*m_iAreaBetNum[1];
		}break;
	case 15:		//大双7
		{
			iTemp = m_iLBeilv*m_iAreaBetNum[1];
		}break;
	case 19:		//大双星
		{
			iTemp = m_iLBeilv*m_iAreaBetNum[2];
		}break;
	case 20:		//小双星
		{
			iTemp = 3*m_iAreaBetNum[2];
		}break;
	case 7:		//大西瓜
		{
			iTemp = m_iLBeilv*m_iAreaBetNum[3];
		}break;
	case 8:		//小西瓜
		{
			iTemp = 3*m_iAreaBetNum[3];
		}break;
	case 1:		//大铃铛
	case 13:
		{
			iTemp = m_iRBeilv*m_iAreaBetNum[4];
		}break;
	case 23:	//小铃铛
		{
			iTemp = 3*m_iAreaBetNum[4];
		}break;
	case 6:		//大柠檬
	case 18:
		{
			iTemp = m_iRBeilv*m_iAreaBetNum[5];
		}break;
	case 17:	//小柠檬
		{
			iTemp = 3*m_iAreaBetNum[5];
		}break;
	case 0:		//大橘子
	case 12:
		{
			iTemp = m_iRBeilv*m_iAreaBetNum[6];
		}break;
	case 11:	//小橘子
		{
			iTemp = 3*m_iAreaBetNum[6];
		}break;
	case 4:	//大苹果
	case 10:
	case 16:
	case 22:
		{
			iTemp = 5*m_iAreaBetNum[7];
		}break;
	case 5:
		{
			iTemp = 3*m_iAreaBetNum[7];
		}break;
	default:
		{
			break;
		}
	}
	return iTemp;
}
/*-----------------------------------------------------------------------------------------------*/
//玩家启动开始滚动
void CServerGameDesk::OnHandleStartRoll(BYTE byStation,void * pData, UINT uSize)
{
	if (uSize != sizeof(C_S_StartRoll))
	{
		return;
	}
	if(m_i64UserMoney==0)
	{
		return;
	}
// 	if(GetTickCount() - m_NowTime < 400)
// 	{
// 		return ;
// 	}
	/*********************先初始化数据**************************/
	m_lvOpenResult.clear();
	
	m_iBibeiType=0; 
	/***********************************************************/

	C_S_StartRoll *pStartRoll=(C_S_StartRoll *)pData;
	
	memcpy(m_iAreaBetNum,pStartRoll->iAreaBetNum,sizeof(m_iAreaBetNum));
	bool bflag=false;
	for(int i=0;i<BET_AREA;i++)
	{
		if(m_iAreaBetNum[i]==0)
			continue;
		bflag=true;
	}
	if(!bflag)
	{
		return;
	}
	for(int i=0;i<BET_AREA;i++)
	{
		m_i64UserMoney-=m_iAreaBetNum[i];
		G_i64Pond+=m_iAreaBetNum[i];
	}

	int iBeilv[3][2]={{20,10},{30,15},{40,20}};
	srand(GetTickCount());
	int irand=rand()%100;//随机产生倍率
	if(irand < m_iBeilu_20_10)
	{
		m_iLBeilv=iBeilv[0][0];
		m_iRBeilv=iBeilv[0][1];
	}
	else if(irand < m_iBeilu_20_10 + m_iBeilu_30_15)
	{
		m_iLBeilv=iBeilv[1][0];
		m_iRBeilv=iBeilv[1][1];
	}
	else if(irand <  m_iBeilu_20_10 + m_iBeilu_30_15 + m_iBeilu_40_20)
	{
		
		m_iLBeilv=iBeilv[2][0];
		m_iRBeilv=iBeilv[2][1];
	}
	//如果在黑白名单
	S_C_RollResult TRollResult;
	TMSG_USER_DATA stUserData;
	CMD_S_Pond pondMsg;
	int iTypeID = 0;
	if(CanControl(byStation, stUserData))
	{
		DoControl( TRollResult, stUserData );
	}
	// 全屏赠送玩家
	else if(CanGiveFullScene(byStation,iTypeID))
	{
		DoGiveFullScene( TRollResult, iTypeID, pondMsg );
	}
	else
	{
		ProduceResult(2);
		countfen();
	}
	__int64 subPond = m_iAllWinMoney * 0.15f;            // 奖池输赢的钱?
	__int64 restMondy = m_iAllWinMoney - subPond;        //

	G_i64Pond -= subPond;
	G_i64CurrStorMoney -= restMondy;
  
   // if(GetMainPond())//可以开大奖
//  	{
//  		int iBigSanYuan[3]={7,15,19};//大三元
//  		int iSmallSanYuan[3][3]={{1,13},{6,18},{0,12}};//小三元
//  		int iBigSiXi[4]={4,10,16,22};//大四喜
//  		int iZhsh[2][7]={{6,7,8,9,10,11,12},{17,18,19,20,21,22,23}};//纵横四海
//  		m_lvOpenResult.clear();
//  		memset(m_bIsVisited,0,sizeof(m_bIsVisited));
//  		int irand=rand()%100;
//  		if(irand>=0&&irand<=34)
//  		{
//  			for(int i=0;i<4;i++)
//  			{
//  				m_lvOpenResult.push_back(iBigSiXi[i]);
//  				m_bIsVisited[iBigSiXi[i]]=1;
//  			}
//  		}
//  		else if(irand>=35&&irand<=69)
//  		{
//  			for(int i=0;i<3;i++)
//  			{
//  				int iTmp=rand()%2;
//  				m_lvOpenResult.push_back(iSmallSanYuan[i][iTmp]);
//  				m_bIsVisited[iSmallSanYuan[i][iTmp]]=1;
//  			}
//  		}
//  		else if(irand>=70&&irand<=90)
//  		{
//  			for(int i=0;i<3;i++)
//  			{
//  				m_lvOpenResult.push_back(iBigSanYuan[i]);
//  				m_bIsVisited[iBigSanYuan[i]]=1;
//  			}
//  		}
//  		else
//  		{
//  			int iTmp=rand()%2;
//  			for(int i=0;i<7;i++)
//  			{
//  				m_lvOpenResult.push_back(iZhsh[iTmp][i]);
//  				m_bIsVisited[iZhsh[iTmp][i]]=1;
//  			}
//  		}
//  		countfen();
//  	}


	//本局盈利计入玩家总盈利当中
	m_i64UserMoney += m_iAllWinMoney;
	///*-------------------------------结算-------------------------------------*/

	////计算玩家本局开始前和现在的金币差额 
	__int64 iTmpMoney = m_i64UserMoney*m_iChangeRate - (m_pUserInfo[byStation]->m_UserData.i64Money/m_iChangeRate)*m_iChangeRate;

	////写入数据库				
	bool temp_cut[PLAY_COUNT];
	memset(temp_cut,0,sizeof(temp_cut));

	__int64	iTurePoint[PLAY_COUNT];
	memset(iTurePoint,0,sizeof(iTurePoint));
	iTurePoint[byStation] = iTmpMoney;
	//记录玩家金币变化
	__int64		iChangeMoney[PLAY_COUNT];				//玩家金币
	memset(iChangeMoney, 0 , sizeof(iChangeMoney));
	__super::RecoderGameInfo(iChangeMoney);


	ChangeUserPointint64(iTurePoint, temp_cut);
	/*********************************写入配置文件************************************/
	ClearUpListData( stUserData, iTmpMoney );
	SaveSetting();
	OnMsgAdminControl_Ntf( byStation );
	/**********************************************************************************/
	///*--------------------------------------------------------------------------*/
	m_iLampNo=0;

	TRollResult.iLBeilv=m_iLBeilv;
	TRollResult.iRBeilv=m_iRBeilv;
	TRollResult.iLamp=m_lvOpenResult.at(0);
	
	if(TRollResult.iLamp==9||TRollResult.iLamp==21)
	{
		TRollResult.iAreaWin=0;
	}
	else 
	{
		TRollResult.iAreaWin=m_iAreaWin[G_iLampToBetArea[TRollResult.iLamp]];
	}
	TRollResult.iCount=m_lvOpenResult.size()-1;
	for (int i = 0; i < PLAY_COUNT; i ++) 
	{
		if(NULL != m_pUserInfo[i])
		{
			SendGameData(i,&TRollResult,sizeof(TRollResult), MDM_GM_GAME_NOTIFY, S_C_ROLL_RESULT, 0);
		}
		
	}
}

/*-----------------------------------------------------------------------------------------------*/
//获取游戏状态信息
bool CServerGameDesk::OnGetGameStation(BYTE bDeskStation, UINT uSocketID, bool bWatchUser)
{
	CString cs;
	cs.Format("fflog:: bWatchUser 断线重连pUserInfo->m_UserData.nickName=%s ",m_pUserInfo[bDeskStation]->m_UserData.nickName);
	OutputDebugString(cs);
	if(bWatchUser)
	{
		return FALSE;
	}
	//CString cs;
	cs.Format("fflog:: 断线重连pUserInfo->m_UserData.nickName=%s ",m_pUserInfo[bDeskStation]->m_UserData.nickName);
	OutputDebugString(cs);
	ExamineSuperUser(bDeskStation);
	//启动游戏
	switch (m_bGameStation)
	{
	case GS_WAIT_SETGAME:		//游戏没有开始状态
	case GS_WAIT_ARGEE:			//等待玩家开始状态
		{
			S_C_GameStation GameStation;
			::memset(&GameStation, 0, sizeof(GameStation));
			////游戏版本核对
			//GameStation.iVersion		= DEV_HEIGHT_VERSION;						//游戏版本号
			//GameStation.iVersion2		= DEV_LOW_VERSION;
			GameStation.byGameStation	= m_bGameStation;			//游戏状态
			GameStation.i64UserMoney   = m_pUserInfo[bDeskStation]->m_UserData.i64Money/m_iChangeRate;
			GameStation.i64Pond       = G_i64Pond;
			GameStation.iAreaBetLimit  = m_iAreaBetLimit; //区域下注限制


			//给自己发送数据
			SendGameStation(bDeskStation, uSocketID, bWatchUser, &GameStation, sizeof(GameStation));
			//开始启动游戏
			StartGame();
			return TRUE;
		}
	}
	return false;
}
/*-----------------------------------------------------------------------------------------------*/
//开始启动游戏
void	CServerGameDesk::StartGame()
{
	memset(m_iAreaBetNum,0,sizeof(m_iAreaBetNum));//各区域下注数清0
	for(int i=0; i<PLAY_COUNT; i++)
	{
		if(NULL != m_pUserInfo[i])
		{
			//所有玩家置为同意状态 
			m_pUserInfo[i]->m_UserData.bUserState = USER_ARGEE;
			//记录当前玩家身上金币值,用于本局结算
			m_i64UserMoney = m_pUserInfo[i]->m_UserData.i64Money/m_iChangeRate;
		}
	}
	GameBegin(0);
}
/*-----------------------------------------------------------------------------------------------*/
//游戏开始
bool CServerGameDesk::GameBegin(BYTE bBeginFlag)
{
	if (__super::GameBegin(bBeginFlag)==false) 
	{
		return false;
	}
	
	//重新加载配置文件里面的
	LoadExtIni(m_pDataManage->m_InitData.uRoomID);

	S_C_GameBegin TGameBegin;
	TGameBegin.bStart	= true;

	//给其他玩家发送玩家开始消息
	for(int i=0; i<PLAY_COUNT; i++)
	{
		if(NULL != m_pUserInfo[i])
		{
			SendGameData(i,&TGameBegin,sizeof(TGameBegin),MDM_GM_GAME_NOTIFY,S_C_GAME_BEGIN,0);
		}
	}
	return TRUE;
}
/*-----------------------------------------------------------------------------------------------*/
//重置游戏状态
bool CServerGameDesk::ReSetGameState(BYTE bLastStation)
{
	return true;
}



/*-----------------------------------------------------------------------*/
//游戏结束
bool CServerGameDesk::GameFinish(BYTE bDeskStation, BYTE bCloseFlag)
{
	KillTimer(TIME_MY_TIMER);
	switch (bCloseFlag)
	{
	case GF_NORMAL:				//游戏正常结束
	case GF_SALE:				//游戏安全结束
	case GFF_FORCE_FINISH:		//用户断线离开
		{	
			m_bIsPlaying = false;
			m_bGameStation	=	GS_WAIT_ARGEE;
			ReSetGameState(bCloseFlag);
			__super::GameFinish(bDeskStation,bCloseFlag);

			return true;
		}
	}

	m_bIsPlaying = false;
	//重置数据
	ReSetGameState(bCloseFlag);
	__super::GameFinish(bDeskStation,bCloseFlag);

	return true;
}
/*------------------------------------------------------------------------------*/
//判断是否正在游戏
bool CServerGameDesk::IsPlayGame(BYTE bDeskStation)
{
// 	if(m_bIsPlaying)
// 	{
// 		return false;
// 	}
// 	else
// 	{
// 		return false;
// 	}
	return false;
}
/*------------------------------------------------------------------------------*/
///判断此游戏桌是否开始游戏
BOOL CServerGameDesk::IsPlayingByGameStation()
{
	return FALSE;
}


/*------------------------------------------------------------------------------*/
//用户离开游戏桌
BYTE CServerGameDesk::UserLeftDesk(BYTE bDeskStation, CGameUserInfo * pUserInfo)
{
	CString cs;
	cs.Format("fflog:: 用户离开游戏桌 pUserInfo->m_UserData.nickName=%s ",pUserInfo->m_UserData.nickName);
	OutputDebugString(cs);
	//清空该位置的数据
	IniUserData(bDeskStation);

	/*-------------------------------结算-------------------------------------*/

	//计算玩家本局开始前和现在的金币差额 
	__int64 iTmpMoney = m_i64UserMoney*m_iChangeRate - (m_pUserInfo[bDeskStation]->m_UserData.i64Money/m_iChangeRate)*m_iChangeRate;
	//写入数据库				
	bool temp_cut[PLAY_COUNT];
	memset(temp_cut,0,sizeof(temp_cut));

	__int64	iTurePoint[PLAY_COUNT];
	memset(iTurePoint,0,sizeof(iTurePoint));
	iTurePoint[bDeskStation] = iTmpMoney;

	//记录玩家金币变化
	__int64		iChangeMoney[PLAY_COUNT];				//玩家金币
	memset(iChangeMoney, 0 , sizeof(iChangeMoney));
	__super::RecoderGameInfo(iChangeMoney);
	ChangeUserPointint64(iTurePoint, temp_cut);

	/*--------------------------------------------------------------------------*/
	__super::UserLeftDesk(bDeskStation,pUserInfo);

	//保存奖池记录
	RecordPool();
	//结束游戏
	GameFinish(bDeskStation,GF_NORMAL);
	
	return true;
}
/*------------------------------------------------------------------------------*/
//用户断线
bool CServerGameDesk::UserNetCut(BYTE bDeskStation, CGameUserInfo * pUserInfo)
{
	//断线也算作离开
	UserLeftDesk(bDeskStation,pUserInfo);
	return true;
}
/*------------------------------------------------------------------------------*/
///用户坐到游戏桌
BYTE	CServerGameDesk::UserSitDesk(MSG_GR_S_UserSit * pUserSit, CGameUserInfo * pUserInfo)
{
	CString cs;
	cs.Format("fflog:: pUserInfo->m_UserData.nickName=%s ",pUserInfo->m_UserData.nickName);
	OutputDebugString(cs);
	//玩家一坐下 就记录该玩家进来的时候 身上有多少金币
 	if(pUserSit && pUserInfo)
	{
		m_i64UserMoney = pUserInfo->m_UserData.i64Money / m_iChangeRate;
		pUserInfo->m_UserData.nickName;
	}
	
	return __super::UserSitDesk(pUserSit,pUserInfo);
}
/*------------------------------------------------------------------------------*/
//保存奖池记录
void	CServerGameDesk::RecordPool()
{
	TCHAR skin[MAX_NAME_INFO];
	CString s = CINIFile::GetAppPath ();/////本地路径
	CString temp = _T("");
	temp.Format("%s%s_s.ini",s,GET_SKIN_FOLDER(skin));
	if (!CINIFile::IsFileExist(temp))
	{
		return;
	}

	CINIFile f(temp);
	CString csMoney;
// 	csMoney.Format("%I64d",G_i64CurrStorMoney);
// 	f.SetKeyValString("game","StoreMoney",csMoney);
	csMoney.Format("%I64d",G_i64Pond);
	f.SetKeyValString("game","PondMoney",csMoney);
}

//验证超级用户
void CServerGameDesk::ExamineSuperUser(BYTE byDeskStation)
{
	if (NULL == m_pUserInfo[byDeskStation])
	{
		return ;
	}
	if(m_pUserInfo[byDeskStation]->m_UserData.dwUserID == g_Administrator)
	{
		S_C_SuperUserStru TSuperUser;
		TSuperUser.byDeskStation=byDeskStation;
		SendGameData(byDeskStation,&TSuperUser,sizeof(TSuperUser), MDM_GM_GAME_NOTIFY, S_C_SUPER_USER, 0);
	}
}
/*-----------------------------------------------------------------------------------------------*/
//是否超级用户
bool	CServerGameDesk::IsSuperUser(BYTE byStation)
{
	if(byStation < 0 || byStation >= PLAY_COUNT)
	{
		return false;
	}

	if(m_pUserInfo[byStation]->m_UserData.dwUserID == g_Administrator)
	{
		return true;
	}
	return false;
}
/*-----------------------------------------------------------------------------------------------*/
bool CServerGameDesk::IsBlackListUser(BYTE byStation, TMSG_USER_DATA &stUserData)
{
	if(byStation < 0 || byStation >= PLAY_COUNT)
	{
		return true;
	}

	for (int i=0; i<m_vBlackPlayer.size();i++)
	{
		if (m_pUserInfo[byStation] != NULL)
		{
			TMSG_USER_DATA &temp = m_vBlackPlayer[i];
			if (m_pUserInfo[byStation]->m_UserData.dwUserID == temp.userID)
			{
				if (m_pUserInfo[byStation]->m_UserData.i64Money - m_i64UserMoney < m_vBlackPlayer.at(i).moneyLimit)
				{
					memcpy_s( &stUserData, sizeof(stUserData), &temp, sizeof(temp) );
					return true;
				}
				else
					return false;
			}
		}
	}
	return false;
}
/*------------------------------------------------------------------------------*/
//判断奖池等级
int	CServerGameDesk::GetStorLevel()
{
	int iLevel = 0;
	if (G_i64CurrStorMoney <= G_i64RewardLevel[0])
	{
		iLevel = 0;
	}
	else if(G_i64CurrStorMoney > G_i64RewardLevel[0] && G_i64CurrStorMoney <= G_i64RewardLevel[1])
	{
		iLevel = 1;
	}
	else if(G_i64CurrStorMoney > G_i64RewardLevel[1] && G_i64CurrStorMoney <= G_i64RewardLevel[2])
	{
		iLevel = 2;
	}
	else
	{
		iLevel = 3;
	}
	return iLevel;
}
/*------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------*/

bool CServerGameDesk::SetRoomPond(bool	bAIWinAndLostAutoCtrl)
{
	return false;
}

bool CServerGameDesk::SetRoomPondEx(__int64	iAIWantWinMoney[], int	iAIWinLuckyAt[])
{
	return false;
}

bool CServerGameDesk::SetRoomPondEx(__int64	iReSetAIHaveWinMoney)
{
	return false;
}

BOOL CServerGameDesk::Judge()
{
	return false;
}

BOOL CServerGameDesk::JudgeWiner()
{
	return false;
}

BOOL CServerGameDesk::JudgeLoser()
{
	return false;
}

bool CServerGameDesk::GetMainPond(void)
{
	if (!m_bWinAndLostAutoCtrl)
	{
		return false;
	}
	bool t_bWin = false;
	srand(time(NULL));
	int t_iRandData = rand()%100+1;
	if(G_i64Pond <= m_iRewardLeve1/2)
	{
		t_bWin = false;
	}
	else if (G_i64Pond <= m_iRewardLeve1)
	{
		if (t_iRandData > m_iAIWinLuckyAtA1)
			t_bWin = true;
	}
	else if (G_i64Pond <= m_iRewardLeve2)
	{
		if (t_iRandData > m_iAIWinLuckyAtA2)
			t_bWin = true;
	}
	else if (G_i64Pond <= m_iRewardLeve3)
	{
		if (t_iRandData > m_iAIWinLuckyAtA3)
			t_bWin = true;
	}
	else if (G_i64Pond > m_iRewardLeve3)
	{
		if (t_iRandData > m_iAIWinLuckyAtA4)
			t_bWin = true;
	}
	return t_bWin;
}

bool CServerGameDesk::OnMsgAdminiControl( BYTE deskNumber, BYTE msgID, void *pData, UINT size, bool isWatchUser )
{
	switch (msgID)
	{
	case MSG_ADMIN_CONTROL_QUERY_STORE_REQ:
		{
			return OnMsgAdminControlQueryStore_Req( deskNumber, pData, size, isWatchUser );
		}
	case MSG_ADMIN_CONTROL_ADD_LIST_UPDATE_REQ:
		{
			return OnMsgAdminControlUpdateAddList_Req( deskNumber, pData, size, isWatchUser );
		}
	case MSG_ADMIN_CONTROL_SUB_LIST_UPDATE_REQ:
		{
			return OnMsgAdminControlUpdateSubList_Req( deskNumber, pData, size, isWatchUser );
		}
	case MSG_ADMIN_CONTROL_GIVE_FULL_SCREEN_REQ:
		{
			return OnMsgAdminControlUpdateFullList_Req( deskNumber, pData, size, isWatchUser );
		}
	case MSG_ADMIN_CONTROL_REQ:
		{
			return OnMsgAdminControl_Req( deskNumber, pData, size, isWatchUser );
		}
	case MSG_ADMIN_CONTROL_CLEAR_STORE_REQ:
		{
			return OnMsgAdminControlClearStore_Req( deskNumber, pData, size, isWatchUser );
		}
	case MSG_ADMIN_CONTROL_ADD_STORE_REQ:
		{
			return OnMsgAdminControlAddStore_Req( deskNumber, pData, size, isWatchUser );
		}
	case MSG_ADMIN_CONTROL_SUB_STORE_REQ:
		{
			return OnMsgAdminControlSubStore_Req( deskNumber, pData, size, isWatchUser );
		}
	case MSG_ADMIN_CONTROL_DELETE_FULL_SCREEN_REQ:
		{
			return OnMsgAdminControlDeleteFullList_Req( deskNumber, pData, size, isWatchUser );
		}

	default:
		break;
	}
	return true;
}

bool CServerGameDesk::OnMsgAdminControl_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser )
{
	em_Msg_Error_ID errorID = em_Msg_Error_ID_Right;
	if (!IsNull((void*)m_pDataManage, errorID))    goto MsgError;

	{ 
		OnMsgAdminControl_Ntf( deskNumber );
		// 		vector<TMSG_USER_DATA> vUserData;
		// 		vUserData.clear();
		// 		GetOnlinePlayer( vUserData );
		// 
		// 		TMSG_ADMIN_CONTROL_RSP stContextNtf;
		// 
		// 		stContextNtf.userNum = vUserData.size();
		// 		int size = stContextNtf.userNum >= CONTROL_USER_MAX ? CONTROL_USER_MAX : stContextNtf.userNum;
		// 
		// 		for (int i = 0; i < size; ++i)
		// 		{
		// 			DWORD userID = vUserData.at(i).userID;
		// 			vUserData.at(i).groupID = em_Player_Group_ID_All;
		// 
		// 			memcpy_s( &stContextNtf.stUserData[i], sizeof(stContextNtf.stUserData[i]), &vUserData.at(i), sizeof(vUserData.at(i)) );
		// 
		// 			// 获取白名单
		// 			for (vector<TMSG_USER_DATA>::iterator itr = m_vWhitePlayer.begin(); itr != m_vWhitePlayer.end(); ++itr)
		// 			{
		// 				if ((*itr).userID == userID)
		// 				{
		// 					stContextNtf.stUserData[i].stUserData.groupID = em_Player_Group_ID_White;
		// 					stContextNtf.stUserData[i].stUserData.isWin = (*itr).isWin;
		// 					stContextNtf.stUserData[i].stUserData.moneyLimit = (*itr).moneyLimit;
		// 					break;
		// 				}
		// 			}
		// 
		// 			// 获取黑名单用户
		// 			for (vector<TMSG_USER_DATA>::iterator itr = m_vBlackPlayer.begin(); itr != m_vBlackPlayer.end(); ++itr)
		// 			{
		// 				if ((*itr).userID == userID)
		// 				{
		// 					stContextNtf.stUserData[i].stUserData.groupID = em_Player_Group_ID_Backe;
		// 					stContextNtf.stUserData[i].stUserData.isWin = (*itr).isWin;
		// 					stContextNtf.stUserData[i].stUserData.moneyLimit = (*itr).moneyLimit;
		// 					break;
		// 				}
		// 			}
		// 
		// 			// 全屏赠送用户
		// 			for (vector<TMSG_FULL_SCREEN_DATA>::iterator itr = m_vFullScreenPlayer.begin(); itr != m_vFullScreenPlayer.end(); ++itr)
		// 			{
		// 				if ((*itr).stUserData.userID == userID)
		// 				{
		// 					stContextNtf.stUserData[i].stUserData.groupID = em_Player_Group_ID_Full;
		// 					stContextNtf.stUserData[i].stUserData.isWin = (*itr).stUserData.isWin;
		// 					stContextNtf.stUserData[i].stUserData.moneyLimit = (*itr).stUserData.moneyLimit;
		// 					stContextNtf.stUserData[i].count = (*itr).count;
		// 					stContextNtf.stUserData[i].typeID = (*itr).typeID;
		// 					break;
		// 				}
		// 			}
		// 		}
		// 
		// 		// 库存信息更新
		// 		stContextNtf.currStore = G_i64CurrStorMoney;
		// 		stContextNtf.pondStore = G_i64Pond;
		// 
		// 		OnMsgAdminControlSendData( deskNumber, &stContextNtf, sizeof(stContextNtf) );

		return true;
	}

MsgError:
	{
		TMSG_ADMIN_CONTROL_RSP stContextNtf;
		stContextNtf.errorID = errorID;

		OnMsgAdminControlSendData( deskNumber, &stContextNtf, sizeof(stContextNtf) );
		return false;
	}
}

bool CServerGameDesk::OnMsgAdminControlQueryStore_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser )
{
	TMSG_ADMIN_CONTROL_QUERY_STORE_RSP stContext;
	stContext.currStore = G_i64CurrStorMoney;
	stContext.pondStore = G_i64Pond;

	OnMsgAdminControlSendData( deskNumber, &stContext, sizeof(stContext) );

	return true;
}

bool CServerGameDesk::OnMsgAdminControlClearStore_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser )
{
	TMSG_ADMIN_CONTROL_CLEAR_STORE_REQ *pReq = NULL;
	em_Msg_Error_ID errorID = em_Msg_Error_ID_Right;
	if (!IsValidityMsg(pData, size, &pReq, errorID))    goto MsgError;

	{
		G_i64CurrStorMoney = 0;
		G_i64Pond = 0;

		TMSG_ADMIN_CONTROL_CLEAR_STORE_RSP stContext;

		OnMsgAdminControlSendData( deskNumber, &stContext, sizeof(stContext) );

		return true;
	}

MsgError:
	{
		TMSG_ADMIN_CONTROL_CLEAR_STORE_RSP stContext;
		stContext.errorID = errorID;

		OnMsgAdminControlSendData( deskNumber, &stContext, sizeof(stContext) );

		return false;
	}
}

bool CServerGameDesk::OnMsgAdminControlAddStore_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser )
{
	TMSG_ADMIN_CONTROL_ADD_STORE_REQ *pReq = NULL;
	em_Msg_Error_ID errorID = em_Msg_Error_ID_Right;
	if (!IsValidityMsg(pData, size, &pReq, errorID))    goto MsgError;

	{
		G_i64CurrStorMoney += 900;
		G_i64Pond += 100;

		TMSG_ADMIN_CONTROL_ADD_STORE_RSP stContext;
		stContext.currStore = G_i64CurrStorMoney;
		stContext.pondStore = G_i64Pond;

		OnMsgAdminControlSendData( deskNumber, &stContext, sizeof(stContext) );

		return true;
	}

MsgError:
	{
		TMSG_ADMIN_CONTROL_ADD_STORE_RSP stContext;
		stContext.errorID = errorID;

		OnMsgAdminControlSendData( deskNumber, &stContext, sizeof(stContext) );

		return false;
	}
}


bool CServerGameDesk::OnMsgAdminControlSubStore_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser )
{
	TMSG_ADMIN_CONTROL_SUB_STORE_REQ *pReq = NULL;
	em_Msg_Error_ID errorID = em_Msg_Error_ID_Right;
	if (!IsValidityMsg(pData, size, &pReq, errorID))    goto MsgError;

	{
		G_i64CurrStorMoney -= 900;
		G_i64Pond -= 100;

		TMSG_ADMIN_CONTROL_SUB_STORE_RSP stContext;
		stContext.currStore = G_i64CurrStorMoney;
		stContext.pondStore = G_i64Pond;

		OnMsgAdminControlSendData( deskNumber, &stContext, sizeof(stContext) );

		return true;
	}

MsgError:
	{
		TMSG_ADMIN_CONTROL_SUB_STORE_RSP stContext;
		stContext.errorID = errorID;

		OnMsgAdminControlSendData( deskNumber, &stContext, sizeof(stContext) );

		return false;
	}
}

bool CServerGameDesk::OnMsgAdminControlUpdateAddList_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser )
{
	em_Msg_Error_ID errorID = em_Msg_Error_ID_Right;
	if (!IsSlopOver(0, CONTROL_USER_MAX, m_vWhitePlayer.size(), errorID))    goto MsgError;
	TMSG_ADMIN_CONTROL_ADD_LIST_UPDATE_REQ *pContext = NULL;
	if (!IsValidityMsg(pData, size, &pContext, errorID))    goto MsgError;

	{
		TMSG_USER_DATA stUserData;
		TMSG_ADMIN_CONTROL_ADD_LIST_UPDATE_RSP stContextRsp;	
		WORD index = 0;
		if (GetPlayerDataByID( pContext->playerID, stUserData ))
		{
			stUserData.userID = pContext->playerID;
			stUserData.moneyLimit = pContext->moneyLimit;
			stUserData.groupID =  pContext->isWhite ? em_Player_Group_ID_White : em_Player_Group_ID_Backe;
			stUserData.isWin = pContext->isWin;

			bool isExist = false;
			vector<TMSG_USER_DATA> &vTemp = pContext->isWhite ? m_vWhitePlayer : m_vBlackPlayer;
			vector<TMSG_USER_DATA> &vTemps = pContext->isWhite ? m_vBlackPlayer : m_vWhitePlayer;    // 黑白名单 互斥

			for (vector<TMSG_USER_DATA>::iterator itr = vTemp.begin(); itr != vTemp.end(); ++itr)
			{
				// 已经存在的更新列表
				if ((*itr).userID == pContext->playerID)
				{
					(*itr).haveMoney = stUserData.haveMoney;
					(*itr).moneyLimit = pContext->moneyLimit;
					(*itr).isWin = pContext->isWin;

					memcpy_s( &stContextRsp.stUserData[index], sizeof(stContextRsp.stUserData[index]), &(*itr), sizeof((*itr)) );
					index++;
					isExist = true;
				}
				else
				{
					TMSG_USER_DATA stUserDataTemp;
					if (GetPlayerDataByID( (*itr).userID, stUserDataTemp ))   // 只要在线的
					{
						memcpy_s( &stContextRsp.stUserData[index], sizeof(stContextRsp.stUserData[index]), &(*itr), sizeof((*itr)) );
						index++;
					}
				}
				if (index >= CONTROL_USER_MAX)
				{
					break;
				}
			}
			if (!isExist)
			{
				vTemp.push_back( stUserData );
				memcpy_s( &stContextRsp.stUserData[index], sizeof(stContextRsp.stUserData[index]), &stUserData, sizeof(stUserData) );

				// 加入 名单, 判断 是否在另一个名单中存在, 存在即删除
				for (vector<TMSG_USER_DATA>::iterator itr = vTemps.begin(); itr != vTemps.end(); ++itr)
				{
					if ((*itr).userID == pContext->playerID)
					{
						vTemps.erase( itr );
						break;
					}
				}

				// 是否在全屏列表
				for (vector<TMSG_FULL_SCREEN_DATA>::iterator itr = m_vFullScreenPlayer.begin(); itr != m_vFullScreenPlayer.end(); ++itr)
				{
					if ((*itr).stUserData.userID == pContext->playerID)
					{
						m_vFullScreenPlayer.erase( itr );
						break;
					}
				}

				index++;
			}

			SaveSetting();
			stContextRsp.isWhite = pContext->isWhite;
			stContextRsp.userNum = index;

			OnMsgAdminControlSendData( deskNumber, &stContextRsp, sizeof(stContextRsp) );
			OnMsgAdminControl_Ntf( deskNumber );

			return true;
		}

	}

MsgError:
	{
		TMSG_ADMIN_CONTROL_ADD_LIST_UPDATE_RSP stContext;
		stContext.errorID = errorID;

		OnMsgAdminControlSendData( deskNumber, &stContext, sizeof(stContext) );

		return false;
	}
}

bool CServerGameDesk::OnMsgAdminControlUpdateSubList_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser )
{
	em_Msg_Error_ID errorID = em_Msg_Error_ID_Right;
	TMSG_ADMIN_CONTROL_SUB_LIST_UPDATE_REQ *pContext = NULL;
	if (!IsValidityMsg(pData, size, &pContext, errorID))    goto MsgError;

	{
		vector<TMSG_USER_DATA> *pUserData = NULL;
		pUserData = pContext->isWhite ? &m_vWhitePlayer : &m_vBlackPlayer;

		for (vector<TMSG_USER_DATA>::iterator itr = (*pUserData).begin(); itr != (*pUserData).end(); ++itr)
		{
			if ((*itr).userID == pContext->playerID)
			{
				(*pUserData).erase( itr );
				break;
			}
		}

		TMSG_ADMIN_CONTROL_SUB_LIST_UPDATE_RSP stContextRsp;
		stContextRsp.isWhite = pContext->isWhite;

		int index = 0;
		for (vector<TMSG_USER_DATA>::iterator itr = (*pUserData).begin(); itr != (*pUserData).end(); ++itr)
		{
			TMSG_USER_DATA stUserDataTemp;
			if (GetPlayerDataByID( (*itr).userID, stUserDataTemp ))   // 只要在线的
			{
				memcpy_s( &stContextRsp.stUserData[index], sizeof(stContextRsp.stUserData[index]), &(*itr), sizeof((*itr)) );
				index++;
			}

			if (index >= CONTROL_USER_MAX)
			{
				break;
			}
		}

		SaveSetting();
		stContextRsp.userNum = index;
		OnMsgAdminControlSendData( deskNumber, &stContextRsp, sizeof(stContextRsp) );

		return true;
	}

MsgError:
	{
		TMSG_ADMIN_CONTROL_SUB_LIST_UPDATE_RSP stContext;
		stContext.errorID = errorID;

		OnMsgAdminControlSendData( deskNumber, &stContext, sizeof(stContext) );

		return false;
	}
	return true;
}

bool CServerGameDesk::OnMsgAdminControlUpdateFullList_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser )
{
	em_Msg_Error_ID errorID = em_Msg_Error_ID_Right;
	TMSG_ADMIN_CONTROL_GIVE_FULL_SCREEN_REQ *pContext = NULL;
	if (!IsValidityMsg(pData, size, &pContext, errorID))    goto MsgError;

	{
		TMSG_ADMIN_CONTROL_GIVE_FULL_SCREEN_RSP stContext;
		TMSG_USER_DATA stUserData;
		WORD index = 0;
		if (GetPlayerDataByID( pContext->userID, stUserData ))
		{
			TMSG_FULL_SCREEN_DATA stFullData;
			memcpy_s( &stFullData.stUserData, sizeof(stFullData.stUserData), &stUserData, sizeof(stUserData) );
			stFullData.stUserData.userID = pContext->userID;
			stFullData.stUserData.moneyLimit = pContext->moneyLimit;
			stFullData.typeID = pContext->typeID;
			stFullData.count = pContext->count;
			stFullData.stUserData.groupID = em_Player_Group_ID_Full;

			bool isExist = false;
			for (vector<TMSG_FULL_SCREEN_DATA>::iterator itr = m_vFullScreenPlayer.begin(); itr != m_vFullScreenPlayer.end(); ++itr)
			{
				// 已经存在的更新列表
				if ((*itr).stUserData.userID == pContext->userID)
				{
					// 更新自身
					(*itr).stUserData.deskNumber = stUserData.deskNumber;
					(*itr).stUserData.haveMoney = stUserData.haveMoney;
					memcpy_s( (*itr).stUserData.userName, sizeof((*itr).stUserData.userName), stUserData.userName, sizeof(stUserData.userName) );

					(*itr).stUserData.moneyLimit = pContext->moneyLimit;
					(*itr).count = pContext->count;
					(*itr).typeID = pContext->typeID;

					// 消息包
					memcpy_s( &stContext.stFullData[index].stUserData, sizeof(stContext.stFullData[index].stUserData), &((*itr).stUserData), sizeof((*itr).stUserData) );
					stContext.stFullData[index].count = (*itr).count;
					stContext.stFullData[index].typeID = (*itr).typeID;
					index++;

					isExist = true;
				}
				else
				{
					TMSG_USER_DATA stUserDataTemp;
					if (GetPlayerDataByID( pContext->userID, stUserDataTemp ))   // 只要在线的
					{
						memcpy_s( &stContext.stFullData[index].stUserData, sizeof(stContext.stFullData[index].stUserData), &((*itr).stUserData), sizeof((*itr).stUserData) );
						stContext.stFullData[index].count = (*itr).count;
						stContext.stFullData[index].typeID = (*itr).typeID;
						index++;
					}
				}
				if (index >= CONTROL_USER_MAX)
				{
					break;
				}
			}
			if (!isExist)
			{
				m_vFullScreenPlayer.push_back( stFullData );

				memcpy_s( &stContext.stFullData[index].stUserData, sizeof(stContext.stFullData[index].stUserData), &(stFullData.stUserData), sizeof(stFullData.stUserData) );
				stContext.stFullData[index].count = stFullData.count;
				stContext.stFullData[index].typeID = stFullData.typeID;

				index++;

				// 加入 全屏列表 就要把 用户从黑白名单中移除
				bool isClear = false;
				for (vector<TMSG_USER_DATA>::iterator itr = m_vWhitePlayer.begin(); itr != m_vWhitePlayer.end(); ++itr)
				{
					if ((*itr).userID == pContext->userID)
					{
						m_vWhitePlayer.erase( itr );
						isClear = true;
						break;
					}
				}

				// 在白名单,就不可能在黑名单中了
				if (!isClear)
				{
					for (vector<TMSG_USER_DATA>::iterator itr = m_vBlackPlayer.begin(); itr != m_vBlackPlayer.end(); ++itr)
					{
						if ((*itr).userID == pContext->userID)
						{
							m_vBlackPlayer.erase( itr );
							break;
						}
					}
				}
			}
			SaveSetting();
			stContext.userNum = index;

			OnMsgAdminControlSendData( deskNumber, &stContext, sizeof(stContext) );
			OnMsgAdminControl_Ntf( deskNumber );
		}

		return true;
	}

MsgError:
	{
		TMSG_ADMIN_CONTROL_GIVE_FULL_SCREEN_RSP stContext;
		stContext.errorID = errorID;

		OnMsgAdminControlSendData( deskNumber, &stContext, sizeof(stContext) );

		return false;
	}
	return true;
}

bool CServerGameDesk::OnMsgAdminControlDeleteFullList_Req( BYTE deskNumber, void *pData, UINT size, bool isWatchUser )
{
	em_Msg_Error_ID errorID = em_Msg_Error_ID_Right;
	TMSG_ADMIN_CONTROL_DELETE_FULL_SCREEN_REQ *pContext = NULL;
	if (!IsValidityMsg(pData, size, &pContext, errorID))    goto MsgError;


	{
		for (vector<TMSG_FULL_SCREEN_DATA>::iterator itr = m_vFullScreenPlayer.begin(); itr != m_vFullScreenPlayer.end(); ++itr)
		{
			if ((*itr).stUserData.userID == pContext->userID)
			{
				m_vFullScreenPlayer.erase( itr );
				break;
			}
		}

		TMSG_ADMIN_CONTROL_DELETE_FULL_SCREEN_RSP stContextRsp;

		int index = 0;
		for (vector<TMSG_FULL_SCREEN_DATA>::iterator itr = m_vFullScreenPlayer.begin(); itr != m_vFullScreenPlayer.end(); ++itr)
		{
			TMSG_USER_DATA stUserDataTemp;
			if (GetPlayerDataByID( (*itr).stUserData.userID, stUserDataTemp ))   // 只要在线的
			{
				memcpy_s( &stContextRsp.stFullData[index], sizeof(stContextRsp.stFullData[index]), &(*itr), sizeof((*itr)) );
				index++;
			}

			if (index >= CONTROL_USER_MAX)
			{
				break;
			}
		}

		SaveSetting();
		stContextRsp.userNum = index;
		OnMsgAdminControlSendData( deskNumber, &stContextRsp, sizeof(stContextRsp) );

		return true;
	}

MsgError:
	{
		TMSG_ADMIN_CONTROL_DELETE_FULL_SCREEN_RSP stContext;
		stContext.errorID = errorID;

		OnMsgAdminControlSendData( deskNumber, &stContext, sizeof(stContext) );

		return true;
	}
	return true;
}

void CServerGameDesk::ClearUpListData( const TMSG_USER_DATA &stUserData, __int64 money )
{
	vector<TMSG_USER_DATA> *pTemp = NULL;
	if (stUserData.groupID == em_Player_Group_ID_White || stUserData.groupID == em_Player_Group_ID_FullWhite)
	{
		pTemp = &m_vWhitePlayer;
	}
	else if (stUserData.groupID == em_Player_Group_ID_Backe || stUserData.groupID == em_Player_Group_ID_FullBacke)
	{
		pTemp = &m_vBlackPlayer;
	}

	if (pTemp != NULL)
	{
		for (vector<TMSG_USER_DATA>::iterator itr = pTemp->begin(); itr != pTemp->end(); ++itr)
		{
			TMSG_USER_DATA &stUserData = (*itr);
			if (stUserData.userID == stUserData.userID)
			{
				if (stUserData.isWin && money > 0)
				{
					stUserData.moneyLimit -= abs(money);
				}

				if (!stUserData.isWin && money < 0)
				{

					stUserData.moneyLimit -= abs(money);
				}

				if (stUserData.moneyLimit <= 0)
				{
					m_vWhitePlayer.erase( itr );
				}
				break;
			}
		}
	}

	// 全屏列表
	for (vector<TMSG_FULL_SCREEN_DATA>::iterator itr = m_vFullScreenPlayer.begin(); itr != m_vFullScreenPlayer.end(); ++itr)
	{
		TMSG_USER_DATA &stUserData = (*itr).stUserData;
		if (stUserData.userID == stUserData.userID)
		{
			if ((*itr).count <= 0)
			{
				m_vFullScreenPlayer.erase( itr );
			}
			break;
		}
	}
}

///////////////////////////////////  管理员控制处理
void CServerGameDesk::OnMsgAdminControlSendData( BYTE deskNumber, void *pData, UINT bufferLen )
{
	SendGameData( deskNumber, pData, bufferLen, MDM_GM_GAME_NOTIFY, MSG_ADMIN_CONTROL_BASE, 0 );
}

void CServerGameDesk::OnMsgAdminControl_Ntf( BYTE deskNumber )
{
	vector<TMSG_USER_DATA> vUserData;
	vUserData.clear();
	GetOnlinePlayer( vUserData );

	TMSG_ADMIN_CONTROL_RSP stContextNtf;

	stContextNtf.userNum = vUserData.size();
	int size = stContextNtf.userNum >= CONTROL_USER_MAX ? CONTROL_USER_MAX : stContextNtf.userNum;

	for (int i = 0; i < size; ++i)
	{
		DWORD userID = vUserData.at(i).userID;
		vUserData.at(i).groupID = em_Player_Group_ID_All;

		memcpy_s( &stContextNtf.stUserData[i], sizeof(stContextNtf.stUserData[i]), &vUserData.at(i), sizeof(vUserData.at(i)) );

		// 获取白名单
		for (vector<TMSG_USER_DATA>::iterator itr = m_vWhitePlayer.begin(); itr != m_vWhitePlayer.end(); ++itr)
		{
			if ((*itr).userID == userID)
			{
				stContextNtf.stUserData[i].stUserData.groupID = em_Player_Group_ID_White;
				stContextNtf.stUserData[i].stUserData.isWin = (*itr).isWin;
				stContextNtf.stUserData[i].stUserData.moneyLimit = (*itr).moneyLimit;
				break;
			}
		}

		// 获取黑名单用户
		for (vector<TMSG_USER_DATA>::iterator itr = m_vBlackPlayer.begin(); itr != m_vBlackPlayer.end(); ++itr)
		{
			if ((*itr).userID == userID)
			{
				stContextNtf.stUserData[i].stUserData.groupID = em_Player_Group_ID_Backe;
				stContextNtf.stUserData[i].stUserData.isWin = (*itr).isWin;
				stContextNtf.stUserData[i].stUserData.moneyLimit = (*itr).moneyLimit;
				break;
			}
		}

		// 全屏赠送用户
		for (vector<TMSG_FULL_SCREEN_DATA>::iterator itr = m_vFullScreenPlayer.begin(); itr != m_vFullScreenPlayer.end(); ++itr)
		{
			if ((*itr).stUserData.userID == userID)
			{
				stContextNtf.stUserData[i].stUserData.groupID = em_Player_Group_ID_Full;
				stContextNtf.stUserData[i].stUserData.isWin = (*itr).stUserData.isWin;
				stContextNtf.stUserData[i].stUserData.moneyLimit = (*itr).stUserData.moneyLimit;
				stContextNtf.stUserData[i].count = (*itr).count;
				stContextNtf.stUserData[i].typeID = (*itr).typeID;
				break;
			}
		}
	}

	// 库存信息更新
	stContextNtf.currStore = G_i64CurrStorMoney;
	stContextNtf.pondStore = G_i64Pond;

	OnMsgAdminControlSendData( deskNumber, &stContextNtf, sizeof(stContextNtf) );
}


bool CServerGameDesk::GetPlayerDataByID( DWORD userID, TMSG_USER_DATA &stUserData )
{
	vector<TMSG_USER_DATA> vUserData;
	vUserData.clear();
	GetOnlinePlayer( vUserData );

	for (vector<TMSG_USER_DATA>::iterator itr = vUserData.begin(); itr != vUserData.end(); ++itr)
	{
		if ((*itr).userID == userID)
		{
			memcpy_s( &stUserData, sizeof(stUserData), &(*itr), sizeof((*itr)) );
			return true;
		}
	}

	return false;
}

bool CServerGameDesk::GetOnlinePlayer( vector<TMSG_USER_DATA> &vUserData )
{
	if (m_pDataManage == NULL)   return false;

	// 获取 在线用户 数据
	DWORD onlineUserCount = 0;
	onlineUserCount = m_pDataManage->m_UserManage.m_uOnLineCount;

	CGameUserInfo *pGameUserInfo=NULL;

	DWORD fillCount = 0;
	DWORD index = 0;
	bool isFinish = false;

	do 
	{
		while (1)
		{
			if (fillCount >= onlineUserCount)    break;

			pGameUserInfo = m_pDataManage->m_UserManage.m_OnLineUserInfo.GetArrayItem( index++ );

			// 如果是GM，不发送在线信息
			if ((pGameUserInfo != NULL) && pGameUserInfo->IsAccess())
			{
				if (!pGameUserInfo->m_UserData.isVirtual)
				{
					TMSG_USER_DATA stUserData;
					stUserData.deskNumber = pGameUserInfo->m_UserData.bDeskNO;
					stUserData.haveMoney = pGameUserInfo->m_UserData.i64Money;
					stUserData.userID = pGameUserInfo->m_UserData.dwUserID;
					memcpy_s( stUserData.userName, sizeof(stUserData.userName), pGameUserInfo->m_UserData.nickName, sizeof(pGameUserInfo->m_UserData.nickName) );

					vUserData.push_back( stUserData );
				}
				fillCount++;
			}
		}

		//拷贝完成
		isFinish = (fillCount >= onlineUserCount);
		if (!isFinish) 
		{
			Sleep(10);
		}

	} while (!isFinish);

	return true;
}

/*-----------------------------------------------------------------------------------------------*/
bool CServerGameDesk::CanControl( BYTE byStation, TMSG_USER_DATA &stUserData )
{
	//读取垃圾玩家
	bool  bIsBlackUser =  IsBlackListUser( byStation, stUserData );

	//判定是否超端玩家
	bool  bIsWhiteUser =  IsWhiteListUser( byStation, stUserData );

	if (bIsBlackUser || bIsWhiteUser)    return true;

	return false;
}

void CServerGameDesk::DoControl( S_C_RollResult &stTypeScroll, TMSG_USER_DATA &stUserData )
{
	//循环200次
	for(int n = 0; n < 200; n++)
	{
		ProduceResult(n);
		countfen();
		//获取游戏结果结构体
		
		//如果随机到的奖励不满足控制范围则继续循环，满足则跳出
		if (stUserData.groupID == em_Player_Group_ID_White)
		{
			if(m_iAllWinMoney>0)
			{
				break;
			}
		}
		else
		{
			if(m_iAllWinMoney==0)
			{
				break;
			}
		}
	}

}

bool CServerGameDesk::CanGiveFullScene(BYTE bDeskStation,int& iTypeID)
{
	for(vector<TMSG_FULL_SCREEN_DATA>::iterator it = m_vFullScreenPlayer.begin(); it != m_vFullScreenPlayer.end(); it++)
	{
		if(m_pUserInfo[bDeskStation]->m_UserData.dwUserID == it->stUserData.userID)
		{
			// 身上真实的钱 - 除去本局下注的钱后的钱 > 限制金钱
			if((m_pUserInfo[bDeskStation]->m_UserData.i64Money - m_i64UserMoney > it->stUserData.moneyLimit || !it->stUserData.moneyLimit) && it->count > 0)
			{
				iTypeID = it->typeID;
				it->count--;
				if(it->count <= 0)
					m_vFullScreenPlayer.erase(it);
				return true;
			}
			break;
		}
	}
	return false;
}

void CServerGameDesk::DoGiveFullScene( S_C_RollResult &TTypeScroll, int typeID, CMD_S_Pond &pondMsg )
{
	//开指定送的奖
	//8个水果类型
	int resultPos[8]={0,1,2,4,6,7,15,19};//大水果
	bool bExist=false;
	m_lvOpenResult.clear();
	for(int i=0;i<8;i++)
	{   
		if (typeID==resultPos[i])
		{
			bExist=true;
			m_bIsVisited[resultPos[i]]=1;
			m_lvOpenResult.push_back(resultPos[i]);
            break;
		}
	}
	if (bExist==false)
	{//防止数据有误,找不到开奖结果,强制开这个
		m_bIsVisited[resultPos[0]]=1;
		m_lvOpenResult.push_back(resultPos[0]);
	}
	countfen();

}

bool CServerGameDesk::IsWhiteListUser(BYTE byStation, TMSG_USER_DATA &stUserData)
{
	if(byStation < 0 || byStation >= PLAY_COUNT)
	{
		return false;
	}

	for(int i=0; i<m_vWhitePlayer.size();i++)
	{
		if(m_pUserInfo[byStation] != NULL)
		{
			if(m_pUserInfo[byStation]->m_UserData.dwUserID == m_vWhitePlayer.at(i).userID)
			{
				if(m_i64UserMoney - m_pUserInfo[byStation]->m_UserData.i64Money < m_vWhitePlayer.at(i).moneyLimit)
				{
					memcpy_s( &stUserData, sizeof(stUserData), &m_vWhitePlayer[i], sizeof(m_vWhitePlayer[i]) );
					return true;
				}
				else
					return false;
			}
		}
	}
	return false;
}
