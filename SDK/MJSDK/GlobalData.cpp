#include "stdafx.h"
#include "GlobalData.h"

void MyDebugString(LPCTSTR lpPre, LPCTSTR lpFormat, ...)
{
	CString strMsg;
	va_list pArg;
	va_start(pArg,lpFormat);
	strMsg.FormatV(lpFormat, pArg);
	va_end(pArg);

	CString str;
	str.Format(_T("%s------%s"), lpPre, strMsg);
	OutputDebugString(str);
}