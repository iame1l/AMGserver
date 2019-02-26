#include "stdafx.h"
#include "../../Server/ServerManage.h"
#include "UserLeaveListener.h"

UserLeaveListener::UserLeaveListener()
{

}

void UserLeaveListener::Listen( uchar playerPos, uint msgID, void* msgData ,uint uSize)
{
}

void UserLeaveListener::OnAdd()
{
	__super::OnAdd();
}

bool UserLeaveListener::AssureMsg( uchar playerPos, uint msgID, void* msgData )
{
	return true;
}

void UserLeaveListener::RequestLeave( uchar playerPos, uint msgID, void* msgData )
{
}

void UserLeaveListener::AgreeLeave( uchar playerPos, uint msgID, void* msgData )
{
}

void UserLeaveListener::Record( uchar playerPos, void * msgData, uint dataSize, uchar mainID, uchar assID, uchar handleCode )
{
}

