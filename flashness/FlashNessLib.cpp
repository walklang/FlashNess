// FlashNess.cpp : DLL 导出的实现。

//
// 注意: COM+ 1.0 信息:
//      请记住运行 Microsoft Transaction Explorer 以安装组件。
//      默认情况下不进行注册。

#include "stdafx.h"
#include "resource.h"
#include "FlashNess_i.h"
#include "dllmain.h"
#include "compreg.h"
#include "xdlldata.h"
#include "regs/AxRegistration.h"


// 用于确定 DLL 是否可由 OLE 卸载。
STDAPI DllCanUnloadNow(void)
{
	SetPerUserRegistration(true);

	#ifdef _MERGE_PROXYSTUB
	HRESULT hr = PrxDllCanUnloadNow();
	if (hr != S_OK)
		return hr;
#endif
	return _AtlModule.DllCanUnloadNow();
}

// 返回一个类工厂以创建所请求类型的对象。
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	SetPerUserRegistration(true);

	#ifdef _MERGE_PROXYSTUB
	if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
		return S_OK;
#endif
		return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

// DllRegisterServer - 在系统注册表中添加项。
STDAPI DllRegisterServer(void)
{
	AxRegisterServer();

	{
		SetPerUserRegistration(false);
		// 注册对象、类型库和类型库中的所有接口
		HRESULT hr = _AtlModule.DllRegisterServer();
#ifdef _MERGE_PROXYSTUB
		if (SUCCEEDED(hr))
		{
			hr = PrxDllRegisterServer();
		}
#endif
	}
	
	{
		SetPerUserRegistration(true);
		// 注册对象、类型库和类型库中的所有接口
		HRESULT hr = _AtlModule.DllRegisterServer();
#ifdef _MERGE_PROXYSTUB
		if (SUCCEEDED(hr))
		{
			hr = PrxDllRegisterServer();
		}
#endif
	}

	return S_OK;
}

// DllUnregisterServer - 在系统注册表中移除项。
STDAPI DllUnregisterServer(void)
{
	AxUnregisterServer();	

	{
		SetPerUserRegistration(true);
		HRESULT hr = _AtlModule.DllUnregisterServer();
#ifdef _MERGE_PROXYSTUB
		if (SUCCEEDED(hr)) {
			hr = PrxDllRegisterServer();
			if (SUCCEEDED(hr)) {
				hr = PrxDllUnregisterServer();
			}
		}
#endif		
	}

	{
		SetPerUserRegistration(false);
		HRESULT hr = _AtlModule.DllUnregisterServer();
#ifdef _MERGE_PROXYSTUB
		if (SUCCEEDED(hr)) {
			hr = PrxDllRegisterServer();
			if (SUCCEEDED(hr)) {
				hr = PrxDllUnregisterServer();
			}
		}
#endif		
	}
	
	return S_OK;
}

// DllInstall - 按用户和计算机在系统注册表中逐一添加/移除项。
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
	HRESULT hr = E_FAIL;
	static const wchar_t szUserSwitch[] = L"user";

	if (pszCmdLine != NULL)
	{
		if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0)
		{
			ATL::AtlSetPerUserRegistration(true);
		}
	}

	if (bInstall)
	{	
		hr = DllRegisterServer();
		if (FAILED(hr))
		{
			DllUnregisterServer();
		}
	}
	else
	{
		hr = DllUnregisterServer();
	}

	return hr;
}


