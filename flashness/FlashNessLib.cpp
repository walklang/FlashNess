// FlashNess.cpp : DLL ������ʵ�֡�

//
// ע��: COM+ 1.0 ��Ϣ:
//      ���ס���� Microsoft Transaction Explorer �԰�װ�����
//      Ĭ������²�����ע�ᡣ

#include "stdafx.h"
#include "resource.h"
#include "FlashNess_i.h"
#include "dllmain.h"
#include "compreg.h"
#include "xdlldata.h"
#include "regs/AxRegistration.h"


// ����ȷ�� DLL �Ƿ���� OLE ж�ء�
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

// ����һ���๤���Դ������������͵Ķ���
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	SetPerUserRegistration(true);

	#ifdef _MERGE_PROXYSTUB
	if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
		return S_OK;
#endif
		return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

// DllRegisterServer - ��ϵͳע���������
STDAPI DllRegisterServer(void)
{
	AxRegisterServer();

	{
		SetPerUserRegistration(false);
		// ע��������Ϳ�����Ϳ��е����нӿ�
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
		// ע��������Ϳ�����Ϳ��е����нӿ�
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

// DllUnregisterServer - ��ϵͳע������Ƴ��
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

// DllInstall - ���û��ͼ������ϵͳע�������һ���/�Ƴ��
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


