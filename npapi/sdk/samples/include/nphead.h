// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件:
#include <windows.h>

// C 运行时头文件
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// C++ 运行时头文件
#include <sstream>
#include <iostream>
#include <string>
#include <map>

#ifndef _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的
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

// TODO: 在此处引用程序需要的其他头文件