#pragma once

// 包括 SDKDDKVer.h 将定义可用的最高版本的 Windows 平台。

// 如果要为以前的 Windows 平台生成应用程序，请包括 WinSDKVer.h，并将
// WIN32_WINNT 宏设置为要支持的平台，然后再包括 SDKDDKVer.h。

#include <WinSDKVer.h>

#ifndef WINVER                         
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT            
#define _WIN32_WINNT 0x0501     
#endif

#ifndef _WIN32_WINDOWS          
#define _WIN32_WINDOWS 0x0501 
#endif

#ifndef _WIN32_IE			
#define _WIN32_IE 0x0600
#endif

#include <SDKDDKVer.h>
