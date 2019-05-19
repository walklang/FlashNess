// dllmain.cpp : DllMain 的实现。

#include "stdafx.h"
#include "resource.h"
#include "FlashNess_i.h"
#include "dllmain.h"
#include "compreg.h"
#include "xdlldata.h"

CFlashNessModule _AtlModule;
HMODULE _Instance = nullptr;

// DLL 入口点
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) {
#ifdef _MERGE_PROXYSTUB
	if (!PrxDllMain(hInstance, dwReason, lpReserved))
		return FALSE;
#endif
    _Instance = hInstance;
	return _AtlModule.DllMain(dwReason, lpReserved); 
}
