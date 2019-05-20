// FlashNess.cpp : CFlashNess 的实现
#include "stdafx.h"
#include "FlashNess.h"


// CFlashNess


STDMETHODIMP CFlashNess::CloseDevice(SHORT device)
{
	// TODO: 在此添加实现代码
	::MessageBox(NULL, L"CloseDevice", L"NPAPI", NULL);
	return S_OK;
}
