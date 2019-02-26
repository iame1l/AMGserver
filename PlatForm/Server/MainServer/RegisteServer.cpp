/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "StdAfx.h"
#include "RegisteServer.h"
#include "GameLogonManage.h"
#include "commonuse.h"

CRegisteServer::CRegisteServer(CGameLogonManage* pLogonManage)
	:_pLogonManage(pLogonManage)
{
	
}

CRegisteServer::~CRegisteServer(void)
{

}


bool CRegisteServer::OnNetMessage(NetMessageHead* pNetHead,void* pData,UINT uSize,ULONG uAccessIP,UINT uIndex,DWORD dwHandleID)
{
	if (!_pLogonManage) 
	{
		return true;
	}

	if (MDM_GP_REGISTER != pNetHead->bMainID)
	{
		return true;
	}

	return registerNewUserRequest(pData,uSize,uIndex,dwHandleID);

	return true ;
}

bool CRegisteServer::OnDataBaseResult(DataBaseResultLine* pResultData)
{
	if (!_pLogonManage) 
	{
		return true;
	}

	switch(pResultData->uHandleKind)
	{
	case DTK_GP_REGISTER:
		{
			registerNewUserResponse(pResultData);
		}break;
	default:
		return true;
	}

	return true;
}
//收到注册请求
bool CRegisteServer::registerNewUserRequest(void* pData,UINT uSize,UINT uIndex,DWORD dwHandleID)
{
	if (uSize != sizeof(MSG_GP_S_Register))
	{
		return false;
	}

	DL_GP_I_Register DL_Data;
	ZeroMemory(&DL_Data, sizeof(DL_Data));
	memcpy(&DL_Data._data, pData, sizeof(MSG_GP_S_Register));
  /*  if(DL_Data._data.byFastRegister!=4)*/
    {
	    string str_temp1(DL_Data._data.szName);
	    bool bLimite = _pLogonManage->m_HNFilter.censor(str_temp1);
	    if (bLimite)
	    {
	    	MSG_GP_S_Register data;
		    memset(&data,0,sizeof(MSG_GP_S_Register));
		    _pLogonManage->m_TCPSocket.SendData(uIndex, &data,sizeof(MSG_GP_S_Register),MDM_GP_REGISTER,ASS_GP_REGISTER,ERR_REGISTER_NAME_LIMITE,0);
		    return true;
	    }
    }
	_pLogonManage->m_SQLDataManage.PushLine(&DL_Data.DataBaseHead,sizeof(DL_Data),DTK_GP_REGISTER,uIndex,dwHandleID);//提交数据库

	return true;
}

bool CRegisteServer::registerNewUserResponse(DataBaseResultLine* pResultData)
{
	DL_GP_O_Register* pData = (DL_GP_O_Register*)pResultData;
	_pLogonManage->m_TCPSocket.SendData(pResultData->uIndex, &pData->_data, sizeof(pData->_data), MDM_GP_REGISTER,ASS_GP_REGISTER,pResultData->uHandleRusult,pResultData->dwHandleID);

	return true;
}