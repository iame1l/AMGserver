/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

#include "StdAfx.h"
#include "Math.h"
#include "AFCDataBase.h"
#include "AFCFunction.h"
#include "AFCSocket.h"
#include "OleDBErr.h"

// 处理线程结构定义
struct ThreadStartStruct
{
	HANDLE						hEvent;									// 退出事件
	HANDLE						hCompletionPort;						// 完成端口
	CAFCDataBaseManage			* pDataManage;							// 数据库管理类指针
};

// 构造函数
CAFCDataBaseManage::CAFCDataBaseManage()
{
	m_bInit         = false;
	m_bRun          = false;
	m_hThread       = NULL;
	m_hCompletePort = NULL;
	m_pInitInfo     = NULL;
	m_pKernelInfo   = NULL;
	m_pHandleService= NULL;

	hDBLogon  = NULL;
	hDBNative = NULL;

	m_sqlClass = 0;
	m_nPort    = 0;
	m_bsqlInit = FALSE;

	m_pConnection.CreateInstance(__uuidof(Connection));

	// 初始化Recordset指针
	m_pRecordset.CreateInstance(_uuidof(Recordset));

	m_pEngineSink = nullptr;	
	
}

// 析构函数
CAFCDataBaseManage::~CAFCDataBaseManage(void)
{
	if (nullptr != m_pEngineSink)
	{
		dklSQLEngineDelete(m_pEngineSink);
		m_pEngineSink = nullptr;
	}
}

// 开始服务
bool CAFCDataBaseManage::Start()
{
	if ((this == NULL) || m_bRun || !m_bInit)
	{
		throw new CAFCException(TEXT("CAFCDataBaseManage::Start 没有初始化参数"), 0x401);
	}
	
	// 建立事件
	CEvent StartEvent(FALSE, TRUE, NULL, NULL);
	if (StartEvent == NULL)
	{
		throw new CAFCException(TEXT("CAFCDataBaseManage::Start 事件建立失败"),0x402);
	}

	// 建立完成端口
	m_hCompletePort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if(m_hCompletePort == NULL) 
	{
		throw new CAFCException(TEXT("CAFCDataBaseManage::Start m_hCompletePort 建立失败"), 0x403);
	}
	m_DataLine.SetCompletionHandle(m_hCompletePort);
		
	// 通过 sqlengine 连接数据库	
	//SQLConnectReset();
	SQLConnect();

	int x = 0;
	
	// 建立数据处理线程
	unsigned dwThreadID = 0;
	ThreadStartStruct ThreadStartData;
	ThreadStartData.pDataManage = this;
	ThreadStartData.hEvent=StartEvent;
	ThreadStartData.hCompletionPort = m_hCompletePort;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, DataServiceThread, &ThreadStartData, 0, &dwThreadID);
	if (m_hThread == NULL)
	{
		throw new CAFCException(TEXT("CAFCDataBaseManage::Start DataServerThread 线程建立失败"), 0x407);
	}

	WaitForSingleObject(StartEvent, INFINITE);
	ResetEvent(StartEvent);

	// 启动成功
	m_bRun = true;
	return true;
}

// 停止服务
bool CAFCDataBaseManage::Stop()
{
	// 设置数据
	bool bFlush = m_bRun;
	m_bRun = false;
	m_DataLine.SetCompletionHandle(NULL);

	// 关闭完成端口
	if (m_hCompletePort != NULL)
	{
		// 退出处理线程
		if (m_hThread != NULL) 
		{
			PostQueuedCompletionStatus(m_hCompletePort, 0, NULL, NULL);
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
		CloseHandle(m_hCompletePort);
		m_hCompletePort = NULL;
	}

	// 关闭数据库连接
	if (m_pKernelInfo != NULL)
	{
		if (nullptr != m_pEngineSink)
		{
			dklSQLEngineDelete(m_pEngineSink);
			m_pEngineSink = nullptr;
		}
	}

	return true;
}

// 取消初始化
bool CAFCDataBaseManage::UnInit()
{
	// 停止服务
	if (m_bRun) Stop();
	m_DataLine.CleanLineData();

	// 恢复数据
	m_bInit          = false;
	m_pInitInfo      = NULL;
	m_hThread        = NULL;
	m_hCompletePort  = NULL;
	m_pKernelInfo    = NULL;
	m_pHandleService = NULL;

	//if (m_pConnection->State)
	//	m_pConnection->Close();
	return true;
}

// 加入处理队列
bool CAFCDataBaseManage::PushLine(DataBaseLineHead * pData, UINT uSize, UINT uHandleKind, UINT uIndex,DWORD dwHandleID)
{
	//处理数据
	pData->dwHandleID=dwHandleID;
	pData->uIndex=uIndex;
	pData->uHandleKind=uHandleKind;
	return (m_DataLine.AddData(&pData->DataLineHead,uSize,0)!=0);
}

// 数据库处理线程
unsigned __stdcall CAFCDataBaseManage::DataServiceThread(LPVOID pThreadData)
{
	// 数据定义
	ThreadStartStruct		* pData          = (ThreadStartStruct *)pThreadData;	// 线程启动数据指针
	CAFCDataBaseManage		* pDataManage    = pData->pDataManage;				    // 数据库管理指针
	CDataLine				* pDataLine      = &pDataManage->m_DataLine;			// 数据队列指针
	IDataBaseHandleService	* pHandleService = pDataManage->m_pHandleService;	    // 数据处理接口
	HANDLE					hCompletionPort  = pData->hCompletionPort;			    // 完成端口

	// 线程数据读取完成
	::SetEvent(pData->hEvent);

	// 重叠数据
	void					* pIOData     = NULL;							// 数据
	DWORD					dwThancferred = 0;								// 接收数量
	ULONG					dwCompleteKey = 0L;								// 重叠 IO 临时数据
	LPOVERLAPPED			OverData;										// 重叠 IO 临时数据

	// 数据缓存
	BOOL					bSuccess = FALSE;
	BYTE					szBuffer[LD_MAX_PART];

	while (TRUE)
	{
		// 等待完成端口
		bSuccess = GetQueuedCompletionStatus(hCompletionPort, &dwThancferred, &dwCompleteKey, (LPOVERLAPPED *)&OverData, INFINITE);
		if (!bSuccess || dwThancferred == 0)
		{
			_endthreadex(0);
			return 0;
		}

		
		while(pDataLine->GetDataCount())
		{
			//DebugPrintf("pDataLine->GetDataCount()=%d",pDataLine->GetDataCount());
			try
			{
				// 处理完成端口数据
				if (pDataLine->GetData((DataLineHead *)szBuffer, sizeof(szBuffer)) < sizeof(DataBaseLineHead))
				{
					continue;
				}
				//DebugPrintf("GetData %d",((DataBaseLineHead *)szBuffer)->uHandleKind);
				pHandleService->HandleDataBase((DataBaseLineHead *)szBuffer);
				//DebugPrintf("HandleDataBase %d",((DataBaseLineHead *)szBuffer)->uHandleKind);
			}
			catch (...) 
			{ 
				TRACE("CATCH:%s with %s\n",__FILE__,__FUNCTION__);
				continue; 
			}
		}
	}

	_endthreadex(0);
	return 0;
}


// 重联数据库
bool CAFCDataBaseManage::SQLConnectReset()
{
	if (nullptr != m_pEngineSink)
	{
		dklSQLEngineDelete(m_pEngineSink);
		m_pEngineSink = nullptr;
	}

	if (!m_bsqlInit)
	{
		tinyxml2::XMLDocument doc;
		if (tinyxml2::XMLError::XML_SUCCESS != doc.LoadFile("HNGameLocal.xml"))
		{
			AfxMessageBox("Load HNGameLocal.xml Error-0",0);
			return false;
		}
		tinyxml2::XMLElement* element1 = doc.FirstChildElement("SQLSERVER");
		if (NULL == element1)
		{
			AfxMessageBox("Load HNGameLocal.xml Error-0-1",0);
			return false;
		}
		for (const tinyxml2::XMLElement* xml_SqlServer = element1->FirstChildElement(); xml_SqlServer; xml_SqlServer = xml_SqlServer->NextSiblingElement()) 
		{
			// 数据库IP地址
			if (!strcmp(xml_SqlServer->Value(), "DBSERVER")) 
			{
				m_szServer = xml_SqlServer->Attribute("key");
			}
			else if (!strcmp(xml_SqlServer->Value(), "DBPORT")) 
			{
				// 端口
				CString sPort = xml_SqlServer->Attribute("key");
				m_nPort = _ttoi(sPort);
			}
			else if (!strcmp(xml_SqlServer->Value(), "DATABASE")) 
			{
				// 数据库名
				m_szDatabase = xml_SqlServer->Attribute("key");
			}
			
			else if (!strcmp(xml_SqlServer->Value(), "DBACCOUNT")) 
			{
				// 数据库登录帐号
				m_szAccount = xml_SqlServer->Attribute("key");
			}
			else if (!strcmp(xml_SqlServer->Value(), "DBPASSWORD")) 
			{
				// 数据库登录密码
				m_szPassword = xml_SqlServer->Attribute("key");
			}
		}
		m_szDetectTable = TEXT("");
		m_bsqlInit      = TRUE;
	}
	CString szPort;                                                                                                                                                                              
	szPort.Format(TEXT(",%d"), m_nPort);
	CString szServer;
	szServer = m_szServer + szPort;
	DBCreateParam createParam;
	_tcscpy_s(createParam.server, szServer);
	_tcscpy_s(createParam.db, m_szDatabase);
	_tcscpy_s(createParam.uid, m_szAccount);
	_tcscpy_s(createParam.pwd, m_szPassword);

	int code = dklSQLEngineCreate(createParam, &m_pEngineSink);
	if(code!=0)
	{
		m_pEngineSink=nullptr;
		throw new CAFCException(TEXT("CAFCDataBaseManage::Start 连接本地数据库失败a"),code);
	}

	return true;
}

// 检测数据连接
bool CAFCDataBaseManage::CheckSQLConnect()
{
	return true;
}

void CAFCDataBaseManage::SQLConnect()
{
	if (!m_bsqlInit)
	{
		tinyxml2::XMLDocument doc;
		if (tinyxml2::XMLError::XML_SUCCESS != doc.LoadFile("HNGameLocal.xml"))
		{
			AfxMessageBox("Load HNGameLocal.xml Error-1",0);
			return ;
		}
		tinyxml2::XMLElement* element1 = doc.FirstChildElement("SQLSERVER");
		if (NULL == element1)
		{
			AfxMessageBox("Load HNGameLocal.xml Error-1-1",0);
			return ;
		}
		for (const tinyxml2::XMLElement* xml_SqlServer = element1->FirstChildElement(); xml_SqlServer; xml_SqlServer = xml_SqlServer->NextSiblingElement()) 
		{
			// 数据库IP地址
			if (!strcmp(xml_SqlServer->Value(), "DBSERVER")) 
			{
				m_szServer = xml_SqlServer->Attribute("key");
			}
			else if (!strcmp(xml_SqlServer->Value(), "DBPORT")) 
			{
				// 端口
				CString sPort = xml_SqlServer->Attribute("key");
				m_nPort = _ttoi(sPort);
			}
			else if (!strcmp(xml_SqlServer->Value(), "DATABASE")) 
			{
				// 数据库名
				m_szDatabase = xml_SqlServer->Attribute("key");
			}

			else if (!strcmp(xml_SqlServer->Value(), "DBACCOUNT")) 
			{
				// 数据库登录帐号
				m_szAccount = xml_SqlServer->Attribute("key");
			}
			else if (!strcmp(xml_SqlServer->Value(), "DBPASSWORD")) 
			{
				// 数据库登录密码
				m_szPassword = xml_SqlServer->Attribute("key");
			}
		}
		m_szDetectTable = TEXT("");
		m_bsqlInit      = TRUE;
	}


	if (nullptr != m_pEngineSink)
	{
		dklSQLEngineDelete(m_pEngineSink);
		m_pEngineSink = nullptr;
	}

	CString szPort;
	szPort.Format(TEXT(",%d"), m_nPort);
	CString szServer;
	szServer = m_szServer + szPort;
	DBCreateParam createParam;
	_tcscpy_s(createParam.server, szServer);
	_tcscpy_s(createParam.db, m_szDatabase);
	_tcscpy_s(createParam.uid, m_szAccount);
	_tcscpy_s(createParam.pwd, m_szPassword);

	int code = dklSQLEngineCreate(createParam, &m_pEngineSink);
	CString str;
	str.Format("ycl::dklSQLEngineCreate:%s %s %s %s",szServer,m_szDatabase,m_szAccount,m_szPassword);
	OutputDebugString(str);
	if(code!=0)
	{
		m_pEngineSink=nullptr;
		throw new CAFCException(TEXT("CAFCDataBaseManage::Start 连接本地数据库失败b"),code);
	}	
	
}

//***********************************************************************************************//

// 构造函数
CDataBaseHandle::CDataBaseHandle(void)
{
	m_pInitInfo       = NULL;
	m_pKernelInfo     = NULL;
	m_pRusultService  = NULL;
	m_pDataBaseManage = NULL;
}

// 析构函数 
CDataBaseHandle::~CDataBaseHandle(void)
{
}

// 设置参数
bool CDataBaseHandle::SetParameter(IDataBaseResultService * pRusultService, CAFCDataBaseManage * pDataBaseManage, ManageInfoStruct * pInitData, KernelInfoStruct * pKernelData)
{
	m_pInitInfo       = pInitData;
	m_pKernelInfo     = pKernelData;
	m_pRusultService  = pRusultService;
	m_pDataBaseManage = pDataBaseManage;

	return true;
}



// 初始化函数
bool CAFCDataBaseManage::Init(ManageInfoStruct * pInitInfo, KernelInfoStruct * pKernelInfo, 
							  IDataBaseHandleService * pHandleService,IDataBaseResultService * pResultService)
{
	// 效验参数
	if ((this == NULL) || m_bInit || m_bRun)
	{
		throw new CAFCException(TEXT("CAFCDataBaseManage::Init 状态效验失败"), 0x408);
	}

	// 设置数据
	m_pInitInfo      = pInitInfo;
	m_pKernelInfo    = pKernelInfo;
	m_pHandleService = pHandleService;
	m_DataLine.CleanLineData();

	// 设置数据
	m_bInit = true;
	return true;
}

bool CDataBaseHandle::sqlSPSetNameEx(LPCTSTR szSPName,bool bReturnValue)
{
	if (nullptr == m_pDataBaseManage)
	{
		return false;
	}

	if (nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		m_pDataBaseManage->SQLConnectReset();
	}

	if (nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return false;
	}

	int iRes = m_pDataBaseManage->m_pEngineSink->setStoredProc(szSPName, bReturnValue);
	if (0 != iRes)
	{
		return false;
	}

	return true;
}

void CDataBaseHandle::closeRecord()
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return;
	}

	m_pDataBaseManage->m_pEngineSink->closeRecord();
}
bool CDataBaseHandle::adoBOF()
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return false;
	}

	return m_pDataBaseManage->m_pEngineSink->adoBOF();
}
bool CDataBaseHandle::adoEndOfFile()
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return false;
	}

	return m_pDataBaseManage->m_pEngineSink->adoEndOfFile();
}
bool CDataBaseHandle::moveFirst()
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return false;
	}

	return m_pDataBaseManage->m_pEngineSink->moveFirst();
}
bool CDataBaseHandle::moveNext()
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return false;
	}

	return m_pDataBaseManage->m_pEngineSink->moveNext();
}
bool CDataBaseHandle::moveLast()
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return false;
	}

	return m_pDataBaseManage->m_pEngineSink->moveLast();
}
// 获取记录数
long CDataBaseHandle::getRecordCount()
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return 0;
	}

	return m_pDataBaseManage->m_pEngineSink->getRecordCount();
}
// 获取字段数
long CDataBaseHandle::getFieldCount()
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return 0;
	}

	return m_pDataBaseManage->m_pEngineSink->getFieldCount();
}

int CDataBaseHandle::execSQL(LPCTSTR sql, bool returnValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -3;
	}
	//mark 指针错误点
	try
	{
		return m_pDataBaseManage->m_pEngineSink->execSQL(sql, returnValue);
	}
	catch(...)
	{
		return -3;
	}
}
int CDataBaseHandle::execStoredProc()
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -3;
	}
	int iResult = m_pDataBaseManage->m_pEngineSink->execStoredProc();
	if (0 != iResult)
	{
		static COleDateTime LastTime = COleDateTime::GetCurrentTime();
		COleDateTime nowTime = COleDateTime::GetCurrentTime();
		COleDateTimeSpan tmDiff = (nowTime - LastTime);
		long lsec = (long)tmDiff.GetTotalSeconds();
		if (lsec > 60)
		{
			m_pDataBaseManage->SQLConnectReset();
			LastTime = nowTime;
		}
		
	}
	return iResult;
}

long CDataBaseHandle::getReturnValue()
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -1;
	}

	return m_pDataBaseManage->m_pEngineSink->getReturnValue();
}

int CDataBaseHandle::addInputParameter(LPCTSTR fieldName, bool fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->addInputParameter(fieldName, fieldValue);
}
int CDataBaseHandle::addInputParameter(LPCTSTR fieldName, char fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->addInputParameter(fieldName, fieldValue);
}
int CDataBaseHandle::addInputParameter(LPCTSTR fieldName, unsigned char fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->addInputParameter(fieldName, fieldValue);
}
int CDataBaseHandle::addInputParameter(LPCTSTR fieldName, short fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->addInputParameter(fieldName, fieldValue);
}
int CDataBaseHandle::addInputParameter(LPCTSTR fieldName, unsigned short fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->addInputParameter(fieldName, fieldValue);
}
int CDataBaseHandle::addInputParameter(LPCTSTR fieldName, int fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->addInputParameter(fieldName, fieldValue);
}
int CDataBaseHandle::addInputParameter(LPCTSTR fieldName, unsigned int fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->addInputParameter(fieldName, fieldValue);
}
int CDataBaseHandle::addInputParameter(LPCTSTR fieldName, long long fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->addInputParameter(fieldName, fieldValue);
}
int CDataBaseHandle::addInputParameter(LPCTSTR fieldName, unsigned long long fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->addInputParameter(fieldName, fieldValue);
}
int CDataBaseHandle::addInputParameter(LPCTSTR fieldName, float fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->addInputParameter(fieldName, fieldValue);
}
int CDataBaseHandle::addInputParameter(LPCTSTR fieldName, double fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->addInputParameter(fieldName, fieldValue);
}
int CDataBaseHandle::addInputParameter(LPCTSTR fieldName, LPCTSTR fieldValue, unsigned int fieldSize)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->addInputParameter(fieldName, fieldSize, fieldValue);
}

int CDataBaseHandle::addInputParameter(LPCTSTR fieldName, BYTE* fieldValue, unsigned int fieldSize)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->addInputParameter(fieldName, fieldSize, fieldValue);
}

int CDataBaseHandle::getValue(LPCTSTR fieldName, bool* fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->getValue(fieldName, fieldValue);
}
int CDataBaseHandle::getValue(LPCTSTR fieldName, char* fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->getValue(fieldName, fieldValue);
}
int CDataBaseHandle::getValue(LPCTSTR fieldName, unsigned char* fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->getValue(fieldName, fieldValue);
}
int CDataBaseHandle::getValue(LPCTSTR fieldName, short* fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->getValue(fieldName, fieldValue);
}
int CDataBaseHandle::getValue(LPCTSTR fieldName, unsigned short* fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->getValue(fieldName, fieldValue);
}
int CDataBaseHandle::getValue(LPCTSTR fieldName, int* fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->getValue(fieldName, fieldValue);
}
int CDataBaseHandle::getValue(LPCTSTR fieldName, unsigned int* fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->getValue(fieldName, fieldValue);
}
int CDataBaseHandle::getValue(LPCTSTR fieldName, long* fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->getValue(fieldName, fieldValue);
}
int CDataBaseHandle::getValue(LPCTSTR fieldName, unsigned long* fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->getValue(fieldName, fieldValue);
}
int CDataBaseHandle::getValue(LPCTSTR fieldName, long long* fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->getValue(fieldName, fieldValue);
}
int CDataBaseHandle::getValue(LPCTSTR fieldName, unsigned long long* fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->getValue(fieldName, fieldValue);
}
int CDataBaseHandle::getValue(LPCTSTR fieldName, float* fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->getValue(fieldName, fieldValue);
}
int CDataBaseHandle::getValue(LPCTSTR fieldName, double* fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->getValue(fieldName, fieldValue);
}
int CDataBaseHandle::getValue(LPCTSTR fieldName, COleDateTime* fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->getValue(fieldName, fieldValue);
}
int CDataBaseHandle::getValue(LPCTSTR fieldName, GUID* fieldValue)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->getValue(fieldName, fieldValue);
}
int CDataBaseHandle::getValue(LPCTSTR fieldName, LPTSTR dstField, unsigned int dstFieldLength)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->getValue(fieldName, dstField, dstFieldLength);
}

int CDataBaseHandle::getValue(LPCTSTR fieldName, BYTE* dstField, unsigned int dstFieldLength)
{
	if (nullptr == m_pDataBaseManage || nullptr == m_pDataBaseManage->m_pEngineSink)
	{
		return -2;
	}

	return m_pDataBaseManage->m_pEngineSink->getValue(fieldName, dstField, dstFieldLength);
}

//***********************************************************************************************//
