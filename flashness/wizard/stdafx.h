// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#ifndef _ATL_APARTMENT_THREADED
#define _ATL_APARTMENT_THREADED
#endif

#ifndef _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_NO_AUTOMATIC_NAMESPACE
#endif

#ifndef _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��
#endif

#include <comsvcs.h>

#ifndef ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW
#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW
#endif

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>

#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
#include <windows.h>					// Windows ͷ�ļ�

using namespace ATL;

#include <algorithm>