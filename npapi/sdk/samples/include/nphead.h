// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�:
#include <windows.h>

// C ����ʱͷ�ļ�
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// C++ ����ʱͷ�ļ�
#include <sstream>
#include <iostream>
#include <string>
#include <map>

#ifndef _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��
#endif  

#include <atlbase.h>
#include <atlstr.h>
#include <atlcom.h>
#include <atlhost.h>
#include <atlwin.h>
#include <atlctl.h>
using namespace ATL;

#include "Utils.h"

// Turn off warnings about debug symbols for templates being too long
#pragma warning(disable : 4786)

#define TRACE_METHOD(fn) \
{ \
	ATLTRACE(_T("0x%04x %s()\n"), (int) GetCurrentThreadId(), _T(#fn)); \
}
#define TRACE_METHOD_ARGS(fn, pattern, args) \
{ \
	ATLTRACE(_T("0x%04x %s(") _T(pattern) _T(")\n"), (int) GetCurrentThreadId(), _T(#fn), args); \
}

#define NS_ASSERTION(x, y)

#pragma comment(lib, "version")

#define TRACE_DEBUG_MSG(bDebug)\
{\
	if( bDebug ) ::MessageBox(NULL, L"NPPlugin", NULL, MB_OK); \
}

// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�