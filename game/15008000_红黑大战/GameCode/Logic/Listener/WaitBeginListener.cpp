//WaitBeginListener.h
//×¼±¸½×¶Î¼àÌýÆ÷
#include "stdafx.h"
#include "../../Server/ServerManage.h"
#include "WaitBeginListener.h"
#include "PlayerActionDelegate.h"

void overtimeAgree(GameContext *context, int type,uchar playerPos,void *data)
{
	if (context->GetGameDesk()->m_bIsBuy)
		return;
	Listener *p = (Listener*)data;
	p->GetContext()->GetGameDesk()->MakeUserOffLine(playerPos);
	OBJ_GET_EXT(context,DataManage,exDataMgr);
	exDataMgr->eraseUser(playerPos);
}

WaitSetGameListener::WaitSetGameListener()
{
	m_State = GS_WAIT_SETGAME;
}

void WaitSetGameListener::OnReconnect( uchar playerPos, uint socketID, bool isWatchUser )
{
	if(m_Context->GetGameDesk()->getGameStation() != GS_WAIT_SETGAME) return;
	sendBaseState(playerPos,socketID,isWatchUser);
}

void WaitSetGameListener::OnAdd()
{
	__super::OnAdd();
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	exDataMgr->initOnce();
	m_Context->GetGameDesk()->SetGameStation(m_State);
}

void WaitSetGameListener::Record( uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode )
{
}

WaitSetGameListener::~WaitSetGameListener()
{
}

WaitAgreeListener::WaitAgreeListener()
{
	
}

void WaitAgreeListener::OnReconnect( uchar playerPos, uint socketID, bool isWatchUser )
{
	
}

void WaitAgreeListener::OnAdd()
{

}


void WaitAgreeListener::Listen( uchar playerPos, uint msgID, void* msgData ,uint uSize)
{
}

void WaitAgreeListener::Record( uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode )
{
}

WaitAgreeListener::~WaitAgreeListener()
{
}

WaitNextListener::WaitNextListener()
{
	m_State = GS_WAIT_NEXT;
}

void WaitNextListener::OnReconnect( uchar playerPos, uint socketID, bool isWatchUser )
{
	if(m_Context->GetGameDesk()->getGameStation() != GS_WAIT_NEXT) return;
	sendBaseState(playerPos,socketID,isWatchUser);
}

void WaitNextListener::OnAdd()
{
	__super::OnAdd();
	OBJ_GET_EXT(m_Context,DataManage,exDataMgr);
	exDataMgr->initOnce();
	m_Context->GetGameDesk()->SetGameStation(m_State);
}

void WaitNextListener::Record(uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode)
{

}
