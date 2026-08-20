#include "stdafx.h"


#ifdef _DEBUG
void DebugTrace(LPCTSTR szFormat, ...)
{
	TCHAR szTempStr[1024];

	SYSTEMTIME st;
	::GetLocalTime(&st);
	::wsprintf(szTempStr, TEXT("%02d/%02d %02d:%02d:%02d > "),
			   st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	::OutputDebugString(szTempStr);

	va_list Args;
	va_start(Args, szFormat);
	::wvsprintf(szTempStr, szFormat, Args);
	va_end(Args);

	::OutputDebugString(szTempStr);
}
#endif
