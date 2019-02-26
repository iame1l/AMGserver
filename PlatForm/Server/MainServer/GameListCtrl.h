/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#pragma once

#include "GameListCtrl0.h"

// 游戏列表控制类
class CServerGameListManage:public CServerGameListManage0
{
	void UpDateFromRoomPeoList();
	

	DL_GP_RoomListPeoCountStruct* FindRoom(UINT uRoomId)
	{
		for(POSITION pos=mRoomPeoChangeList.GetHeadPosition ();pos !=NULL;)
		{
			DL_GP_RoomListPeoCountStruct* p=(DL_GP_RoomListPeoCountStruct *)mRoomPeoChangeList.GetNext (pos);
			if(p->uID == uRoomId)
				return p;
		}
		return NULL;
	}
private:

	CPtrList    mRoomPeoChangeList;
	CPtrList    mNamePeoChangeList;
	//CAFCNameItem * FindNameItemFromRoomID(UINT uRoomID);
	//void RoomPeoHavChanged(int peochanged,UINT uRoomID);
public:
	INT_PTR FillAllNameOnLineCount(char * pOnLineBuffer, INT_PTR uBufferSize, INT_PTR uBeginPos, bool & bFinish);

	// 获取在线人数 
	INT_PTR FillRoomOnLineCount(char * pOnLineBuffer, INT_PTR uBufferSize, INT_PTR uBeginPos, bool & bFinish);
	INT_PTR FillNameOnLineCount(char * pOnLineBuffer, INT_PTR uBufferSize, INT_PTR uBeginPos, bool & bFinish);

	// 更新游戏列表
	bool UpdateRoomListPeoCount(IDBEngineSink* pEngineSink);
	CServerGameListManage()
	{
	}
	// 析构函数
	virtual ~CServerGameListManage()
	{
		ClearList();
	}
	void ClearList()
	{
		while(!mRoomPeoChangeList.IsEmpty ())
		{
			DL_GP_RoomListPeoCountStruct* p=(DL_GP_RoomListPeoCountStruct *)mRoomPeoChangeList.RemoveHead ();
			delete p;
		}

		while(!mNamePeoChangeList.IsEmpty ())
		{
			DL_GP_RoomListPeoCountStruct* p=(DL_GP_RoomListPeoCountStruct *)mNamePeoChangeList.RemoveHead ();
			delete p;
		}
	}
	void AddRoomList(DL_GP_RoomListPeoCountStruct& RoomRead)
	{
		bool bb=false;
		for(POSITION pos=mRoomPeoChangeList.GetHeadPosition(); NULL !=pos;)
		{
			DL_GP_RoomListPeoCountStruct* p = (DL_GP_RoomListPeoCountStruct *)mRoomPeoChangeList.GetAt(pos);
			if(p->uID == RoomRead.uID)
			{
				bb= true;
				p->uOnLineCount = RoomRead.uOnLineCount;
				break;
			}
			mRoomPeoChangeList.GetNext(pos);
		}

		if(!bb)
		{
			DL_GP_RoomListPeoCountStruct* p = new DL_GP_RoomListPeoCountStruct;
			p->uOnLineCount = RoomRead.uOnLineCount ;
			p->uID = RoomRead.uID ;
			mRoomPeoChangeList.AddTail (p);
		}
	}


};


