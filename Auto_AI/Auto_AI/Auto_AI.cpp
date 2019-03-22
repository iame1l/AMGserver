// Auto_AI.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "Auto_AI.h"
#include "Auto_AIDlg.h"
#include <DbgHelp.h>  
#pragma comment(lib, "dbghelp.lib")  
#pragma comment(lib, "dbghelp.lib")
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static inline BOOL IsDataSectionNeeded(const WCHAR* pModuleName)  
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

static inline BOOL CALLBACK MiniDumpCallback(PVOID                            pParam,  
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

static inline void CreateMiniDump(PEXCEPTION_POINTERS pep, LPCTSTR strFileName)  
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


static LONG __stdcall MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)  
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
	strFile.Format(_T("%sError-game-%d-%d-%d&%d-%d-%d.dmp"), strPath,tm.GetYear(),tm.GetMonth(),tm.GetDay(),tm.GetHour(),tm.GetMinute(),tm.GetSecond());
	CreateMiniDump(pExceptionInfo, strFile);  
	AfxMessageBox(strPath);
	return EXCEPTION_EXECUTE_HANDLER;
}  

// 此函数一旦成功调用，之后对 SetUnhandledExceptionFilter 的调用将无效  
static void DisableSetUnhandledExceptionFilter()  
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

static void InitMinDump()  
{  
	//注册异常处理函数  
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);  

	//使SetUnhandledExceptionFilter  
	DisableSetUnhandledExceptionFilter();  
} 

// CAuto_AIApp

BEGIN_MESSAGE_MAP(CAuto_AIApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CAuto_AIApp 构造

CAuto_AIApp::CAuto_AIApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中

}


// 唯一的一个 CAuto_AIApp 对象

CAuto_AIApp theApp;


// CAuto_AIApp 初始化

BOOL CAuto_AIApp::InitInstance()
{
	//win10mark
	InitMinDump();

	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	CAuto_AIDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此处放置处理何时用“确定”来关闭
		//  对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用“取消”来关闭
		//  对话框的代码
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}
