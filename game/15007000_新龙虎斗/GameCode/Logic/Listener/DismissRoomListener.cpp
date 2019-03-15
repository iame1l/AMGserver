//DismissRoomListener.h
//解散房间监听器
#include "stdafx.h"
#include "ServerManage.h"
#include "DismissRoomListener.h"
#include "Algorithm.h"
#include "ExtensionLogger.h"
#include "ExtensionTimer.h"
#include "DataManage.h"
DismissRoomListener::DismissRoomListener()
{
}

DismissRoomListener::~DismissRoomListener()
{
}

void DismissRoomListener::OnAdd()
{
	__super::OnAdd();
	NotiTotalResult();
	OBJ_GET_EXT(m_Context,ExtensionListenerController,exListenerControl);
	exListenerControl->OnListenEnd(LT_DismissRoom);
}

void DismissRoomListener::Record( uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode )
{
}


bool DismissRoomListener::NotiTotalResult()
{
// 	if(nullptr == m_Context->GetGameDesk()) return false;
// 
// 	if(!m_Context->GetGameDesk()->isCreatedRoom()) return false;
// 	if(!m_Context->GetGameDesk()->m_GameStarted) return false;
// 
// 	TMSG_BIG_SETTLEMENT_NTF Noti;
// 	DataManage::sGameUserInf user;		
// 
// 	int maxScore  = 0;
// 	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
// 	for (size_t i = 0;i < exDataMgr->UserCount();i++)
// 	{
// 		if(!exDataMgr->getUserInfo(i,user)) continue;
// 		if(user.iTotalScore > maxScore) maxScore = user.iTotalScore;
// 	}
// 
// 	for (size_t i = 0;i < exDataMgr->UserCount();i++)
// 	{
// 		if(!exDataMgr->getUserInfo(i,user)) continue;	
// 		Noti.i64TotalGrade[i] = user.iTotalScore;
// 		if(user.iTotalScore == maxScore && maxScore > 0)Noti.bBigWinner[i] = true;	
// 		Noti.byWinnerCount[i]=user.iWinnerCount;
// 		Noti.iFailureCount[i]=user.iFailureCount;
// 	}
// 
// 	LOGGER_FILE(m_Context,"*************************大结算***********************");
// 	for (size_t i = 0;i <  exDataMgr->UserCount();i++)
// 	{
// 		if(!exDataMgr->getUserInfo(i,user)) continue;
// 		LOGGER_FILE(m_Context,"["<<user.sName<<","<<user.userID<<"]  输赢金币 "\
// 			<< Noti.i64TotalGrade[i]\
// 			<<",大赢家 "<<Noti.bBigWinner[i]?"Y":"N");
// 	}
// 	return m_Context->GetGameDesk()->send2all((char*)&Noti,sizeof(Noti),MSG_BIG_SETTLEMENT_NTF);
	return true;
}

