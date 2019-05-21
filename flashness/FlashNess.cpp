// FlashNess.cpp : CFlashNess µÄÊµÏÖ
#include "stdafx.h"
#include "FlashNess.h"


// CFlashNess

STDMETHODIMP CFlashNess::InitDevice(BSTR port, SHORT* device) {
    if (device == nullptr) return S_FALSE;
    *device = 100;
    return S_OK;
}

STDMETHODIMP CFlashNess::CloseDevice(SHORT device) {
	return S_OK;
}

STDMETHODIMP CFlashNess::Beep(SHORT times) {
    return S_OK;
}

STDMETHODIMP CFlashNess::ReadCard(BSTR* pVal) {
    if (!pVal) return S_FALSE;
    std::string temp = "01020200002222222200330445560";
    CComBSTR value(temp.c_str());
    *pVal = value.Detach();
    return S_OK;
}

STDMETHODIMP CFlashNess::WriteData(BSTR data) {
    if (data == nullptr) return S_FALSE;
    data_ = CComBSTR(data).Detach();
    return S_OK;
}

STDMETHODIMP CFlashNess::ReadData(BSTR* data) {
    if (!data) return S_FALSE;
    *data = CComBSTR(data_.c_str()).Detach();
    return S_OK; 
}

STDMETHODIMP CFlashNess::put_name(BSTR data) {
    if (data == nullptr) return S_FALSE;
    name_ = data;
    return S_OK;
}

STDMETHODIMP CFlashNess::get_name(BSTR* pVal) {
    if (!pVal) return S_FALSE;
    *pVal = name_;
    return S_OK;
}