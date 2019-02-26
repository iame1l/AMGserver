/****************************************************************************
Copyright (c) 2014-2016 Beijing TianRuiDiAn Network Technology Co.,Ltd.
Copyright (c) 2014-2016 ShenZhen Redbird Network Polytron Technologies Inc.

http://www.hotniao.com

All of the content of the software, including code, pictures,
resources, are original. For unauthorized users, the company
reserves the right to pursue its legal liability.
****************************************************************************/

// MainServer.cpp : 定义应用程序的类行为。
#include "stdafx.h"
#include "MainServer.h"
#include "MainServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// CCenterServerApp

BEGIN_MESSAGE_MAP(CMainServerApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

//dll模块表
std::map<UINT,HMODULE> g_MuduleMap;

CMainServerApp::CMainServerApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}
#include <windows.h>  
#include <DbgHelp.h>  
#include <stdlib.h>  
#pragma comment(lib, "dbghelp.lib")  

#ifndef _M_IX86  
#error "The following code only works for x86!"  
#endif  

inline BOOL IsDataSectionNeeded(const WCHAR* pModuleName)  
{  
	if(pModuleName == 0)  
	{  
		return FALSE;  
	}  

	WCHAR szFileName[_MAX_FNAME] = L"";  
	_wsplitpath(pModuleName, NULL, NULL, szFileName, NULL);  

	if(wcsicmp(szFileName, L"ntdll") == 0)  
		return TRUE;  

	return FALSE;  
}  

inline BOOL CALLBACK MiniDumpCallback(PVOID                            pParam,  
	const PMINIDUMP_CALLBACK_INPUT   pInput,  
	PMINIDUMP_CALLBACK_OUTPUT        pOutput)  
{  
	if(pInput == 0 || pOutput == 0)  
		return FALSE;  

	switch(pInput->CallbackType)  
	{  
	case ModuleCallback:  
		if(pOutput->ModuleWriteFlags & ModuleWriteDataSeg)  
			if(!IsDataSectionNeeded(pInput->Module.FullPath))  
				pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);  
	case IncludeModuleCallback:  
	case IncludeThreadCallback:  
	case ThreadCallback:  
	case ThreadExCallback:  
		return TRUE;  
	default:;  
	}  

	return FALSE;  
}  

inline void CreateMiniDump(PEXCEPTION_POINTERS pep, LPCTSTR strFileName)  
{  
	HANDLE hFile = CreateFile(strFileName, GENERIC_READ | GENERIC_WRITE,  
		FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);  

	if((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))  
	{  
		MINIDUMP_EXCEPTION_INFORMATION mdei;  
		mdei.ThreadId           = GetCurrentThreadId();  
		mdei.ExceptionPointers  = pep;  
		mdei.ClientPointers     = NULL;  

		MINIDUMP_CALLBACK_INFORMATION mci;  
		mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;  
		mci.CallbackParam       = 0;  

		::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, (pep != 0) ? &mdei : 0, NULL, &mci);  

		CloseHandle(hFile);  
	}  
}  

LONG __stdcall MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)  
{  
	CString strPath;
	char szPath[MAX_PATH];
	memset(szPath, 0, MAX_PATH);
	GetCurrentDirectory(MAX_PATH, szPath);
	strPath.ReleaseBuffer();
	strPath.Format("%s\\D-Error\\", szPath);
	CreateDirectory(strPath, NULL);
	CString strFile;
	CTime tm = CTime::GetCurrentTime();
	strFile.Format(_T("%sError%d-%d-%d&%d-%d-%d.dmp"), strPath,tm.GetYear(),tm.GetMonth(),tm.GetDay(),tm.GetHour(),tm.GetMinute(),tm.GetSecond());
	CreateMiniDump(pExceptionInfo, strFile);  
	AfxMessageBox(strPath);
	return EXCEPTION_CONTINUE_SEARCH;  
}  

// 此函数一旦成功调用，之后对 SetUnhandledExceptionFilter 的调用将无效  
void DisableSetUnhandledExceptionFilter()  
{  
	void* addr = (void*)GetProcAddress(LoadLibrary("kernel32.dll"),  
		"SetUnhandledExceptionFilter");  

	if (addr)  
	{  
		unsigned char code[16];  
		int size = 0;  

		code[size++] = 0x33;  
		code[size++] = 0xC0;  
		code[size++] = 0xC2;  
		code[size++] = 0x04;  
		code[size++] = 0x00;  

		DWORD dwOldFlag, dwTempFlag;  
		VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);  
		WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);  
		VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);  
	}  
}  

void InitMinDump()  
{  
	//注册异常处理函数  
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);  

	//使SetUnhandledExceptionFilter  
	DisableSetUnhandledExceptionFilter();  
} 
// 唯一的一个 CCenterServerApp 对象
CMainServerApp theApp;

BOOL CMainServerApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControls()。否则，将无法创建窗口。
	InitCommonControls();
	CWinApp::InitInstance();
	AfxOleInit();

	AfxEnableControlContainer();
	
	//加载组件模块------------------------------------------------------------------------

	HMODULE hModual = NULL;
	g_MuduleMap.clear();

	//end--------------------------------------------------------------------------------
	InitMinDump();
	CMainServerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO：在此放置处理何时用“确定”来关闭
		//对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO：在此放置处理何时用“取消”来关闭
		//对话框的代码
	}

	//释放组件模块------------------------------------------------------------------------

	std::map<UINT,HMODULE>::iterator itMMap = g_MuduleMap.begin();
	for (;itMMap != g_MuduleMap.end(); itMMap++)
	{
		if (itMMap->second)
		{
			::FreeLibrary(itMMap->second);
		}
	}

	//end--------------------------------------------------------------------------------

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	// 而不是启动应用程序的消息泵。
	return FALSE;
}

//从dll模块表查询模块句柄
HMODULE ModuleGetFromMap(UINT uModualID)
{
	std::map<UINT,HMODULE>::iterator itMMap = g_MuduleMap.find(uModualID);
	return (HMODULE)itMMap->second;
}


//从模块中取出组件接口

HRESULT ModuleCreateInstance(UINT uModualID,char* szCreateFunName,LPVOID FAR** ppv)
{
	typedef HRESULT (*func_ptr)(LPVOID FAR**);
	HMODULE hModule = ModuleGetFromMap(uModualID);
	if (hModule)
	{
		func_ptr lpFunc = (func_ptr)::GetProcAddress(hModule,szCreateFunName);
		if (lpFunc)
		{
			return lpFunc(ppv);
		}
	}

	return S_FALSE;
}

HRESULT ModuleCreateInstance(UINT uModualID,REFCLSID rclsid,char* szCreateFunName,LPVOID FAR** ppv)
{
	typedef HRESULT (*func_ptr)(REFCLSID,LPVOID FAR**);
	HMODULE hModule = ModuleGetFromMap(uModualID);
	if (hModule)
	{
		func_ptr lpFunc = (func_ptr)::GetProcAddress(hModule,szCreateFunName);
		if (lpFunc)
		{
			return lpFunc(rclsid,ppv);
		}
	}

	return S_FALSE;
}
