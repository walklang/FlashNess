#include "stdafx.h"
#include "regs/PerUserRegistration.h"
#include <windows.h>
#include <atlbase.h>
#include <Winreg.h>
#pragma comment(lib, "Kernel32")
#pragma comment(lib, "Advapi32")


CPerUserRegistration::CPerUserRegistration( bool bEnable ) 
	: _mapping(false)
{
	ATL::AtlSetPerUserRegistration(bEnable);
	SetPerUserRegister(bEnable);	
}

CPerUserRegistration::~CPerUserRegistration()
{
	if( _mapping )
	{
		::RegOverridePredefKey(HKEY_CLASSES_ROOT, NULL);
	}
}

void CPerUserRegistration::SetPerUserRegister(bool bEnable)
{
	if( bEnable )
		return;

	HKEY hKey;
	LONG errorCode = ::RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Classes", 0, MAXIMUM_ALLOWED, &hKey);
	if( ERROR_SUCCESS == errorCode )
	{
		errorCode = ::RegOverridePredefKey(HKEY_CLASSES_ROOT, hKey);
		::RegCloseKey(hKey);
	}
	if( ERROR_SUCCESS == errorCode )
	{
		EnablePerUserTLibRegistration();
		_mapping = true;
	}
}

void CPerUserRegistration::EnablePerUserTLibRegistration()
{
	HMODULE hOleAut32 = ::GetModuleHandle(L"Oleaut32.dll");
	if( NULL == hOleAut32 )
	{
		return;
	}
	typedef void(WINAPI* EnablePerUserTLibRegistrationA)(void);
	EnablePerUserTLibRegistrationA enablePerUserTLibRegistration =
		reinterpret_cast<EnablePerUserTLibRegistrationA>(GetProcAddress(hOleAut32, "OaEnablePerUserTLibRegistration"));
	if( enablePerUserTLibRegistration )
	{
		enablePerUserTLibRegistration();
	}
}
