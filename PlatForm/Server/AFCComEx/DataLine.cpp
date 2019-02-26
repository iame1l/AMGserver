/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "StdAfx.h"
#include "DataLine.h"
#include "AFCFunction.h"

struct DataBaseLineHead
{
	DataLineHead					DataLineHead;							// 队列头
	UINT							uHandleKind;							// 处理类型
	UINT							uIndex;									// 对象索引
	DWORD							dwHandleID;								// 对象标识
};

// 构造函数
CDataLine::CDataLine()
{

}

// 析构函数，如果队列中还有数据，要从内容存中清除
CDataLine::~CDataLine(void)
{
	ListItemData *pListItem = NULL;
	while(m_DataList.GetCount() > 0)
	{
		pListItem = m_DataList.GetHead();
		m_DataList.RemoveHead();
		delete pListItem->pData;
		delete pListItem;
	}
//	SafeDeleteArray(m_pDataBuffer);
}

/*
 * 功能：将数据压入到队列当中
 * 参数：
 *      pDataInfo	    :要压入队列的数据指针
 *      uAddSize	    :数据大小
 *      uDataKind	    :数据类型
 *      pAppendData	    :附加数据，可能是空的
 *      pAppendAddSize	:附加数据大小，可以为0，此时实体数据为空
 * 返回：压入队列的大小
 */
UINT CDataLine::AddData(DataLineHead * pDataInfo, UINT uAddSize, UINT uDataKind, void * pAppendData, UINT uAppendAddSize)
{
	CSignedLockObject LockObject(&m_csLock,true);

	if(!m_hCompletionPort)
	{
		return 0;
	}

	// 创建一个队列项
	ListItemData *pListItem = new ListItemData;

	// 先设为0，以保证后续不出错
	pListItem->pData = NULL;

	// 数据大小
	pListItem->stDataHead.uSize = uAddSize;

	// 数据类型
	pListItem->stDataHead.uDataKind = uDataKind;
	
	// 如果有附加数据
	if(pAppendData)
	{
		pListItem->stDataHead.uSize += uAppendAddSize;
	}

	// 申请数据项内存
	pListItem->pData = new BYTE[pListItem->stDataHead.uSize + 1];

	// 清空内存
	memset(pListItem->pData, 0, pListItem->stDataHead.uSize + 1);
	
	pDataInfo->uDataKind = uDataKind;
	pDataInfo->uSize     = pListItem->stDataHead.uSize;

	// 复制实体数据
	memcpy(pListItem->pData, pDataInfo, uAddSize);
	
	// 如果有附加数据，复制在实体数据后面
	if(pAppendData != NULL)
	{
		memcpy(pListItem->pData + uAddSize, pAppendData, uAppendAddSize);
	}

	// 加到队列尾部
	m_DataList.AddTail(pListItem);

	// 通知完成端口
	::PostQueuedCompletionStatus(m_hCompletionPort, pListItem->stDataHead.uSize, NULL, NULL);
	//DebugPrintf("PostQueuedCompletionStatus date to Completion =%d",((DataBaseLineHead *)pListItem->pData)->dwHandleID);
	// 返回大小
	return pListItem->stDataHead.uSize;
}


/*
 * 功能：从队列中取出数据
 * 参数：
 *      pDataBuffer	:取出数据的缓存
 *      uBufferSize	:缓存大小，缺省为 LD_MAX_PART = 3096
 * 返回：取出数据的实际大小
 */
UINT CDataLine::GetData(DataLineHead * pDataBuffer, UINT uBufferSize)
{
	CSignedLockObject LockObject(&m_csLock, true);
	
	memset(pDataBuffer, 0, uBufferSize);
	
	// 如果队列是空的，直接返回
	if(m_DataList.GetCount() <= 0)
	{
		return 0;
	}

	// 取数据
	ListItemData *pListItem = m_DataList.GetHead();
	m_DataList.RemoveHead();

	UINT uDataSize = pListItem->stDataHead.uSize;
	
	// 拷贝数据
	memcpy((void*)pDataBuffer, pListItem->pData, uDataSize);
	
	// 删除队列中的数据
	delete []pListItem->pData;
	delete pListItem;

	return uDataSize;
}

// 清楚所有数据
bool CDataLine::CleanLineData()
{
	CSignedLockObject LockObject(&m_csLock, true);

	// 设置数据 
	ListItemData *pListItem = NULL;
	while(m_DataList.GetCount() > 0)
	{
		pListItem = m_DataList.GetHead();
		m_DataList.RemoveHead();
		delete pListItem->pData;
		delete pListItem;
	}

	return true;
}

// 获取数据记录数量
INT_PTR CDataLine::GetDataCount(void)
{
	CSignedLockObject LockObject(&m_csLock, true);
	return m_DataList.GetCount();
}
