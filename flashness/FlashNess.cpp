// FlashNess.cpp : CFlashNess µÄÊµÏÖ
#include "stdafx.h"
#include "FlashNess.h"
#include <iostream>
#include <memory>
#include <string>

// CFlashNess

#define WaterCard L"water_card.dll"

extern HMODULE _Instance;

std::wstring GetPath(const std::wstring& filename) {
    TCHAR tszModule[MAX_PATH + 1] = { 0 };
    ::GetModuleFileName(_Instance, tszModule, MAX_PATH);
    std::wstring strPath = tszModule;
    std::wstring::size_type npos = strPath.rfind('\\');
    if (std::wstring::npos == npos) return filename;
    strPath.erase(npos);
    strPath += L"\\";
    strPath += filename;
    return strPath;
}

inline CFlashNess::CFlashNess()
    : m_ctlStatic(_T("Static"), this, 1)
    , library_(WaterCard) {
    m_bWindowOnly = TRUE;
    m_bRequiresSave = FALSE;
}

LRESULT CFlashNess::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
    RECT rc;
    GetWindowRect(&rc);
    rc.right -= rc.left;
    rc.bottom -= rc.top;
    rc.top = rc.left = 0;
    m_ctlStatic.Create(m_hWnd, rc);
    m_ctlStatic.ShowWindow(SW_HIDE);
    ::ShowWindow(m_hWnd, SW_HIDE);
    return 0;
}

STDMETHODIMP CFlashNess::InitDevice(BSTR port, SHORT* device) {
    if (device == nullptr || port == nullptr) return S_FALSE;

    ::MessageBox(NULL, L"debug", L"NPAPI", NULL);

    if (!library_.is_valid()) {
        *device = 9001;
        library_.Reset(internal::LoadLibrary(GetPath(WaterCard), nullptr));
        if (!library_.is_valid()) return S_FALSE;
    }

    //auto initcom = library_.GetFunctionPointer<void, char*>("initcom");
    //auto initcom = library_.GetFunctionPointer<void, char*>(17);
    auto initcom = library_.GetFunctionPointer<void, char*>(13);
    if (initcom == nullptr) {
        *device = 9002;
        return S_FALSE;
    }

    std::string multibyte = CT2CAEX<>(port);
    std::unique_ptr<char[]> p(new char[multibyte.length() + 1]);
    std::memset(p.get(), 0, multibyte.length() + 1);
    strncpy_s(p.get(), multibyte.length() + 1, multibyte.c_str(), multibyte.length());

    initcom(p.get());

    auto get_time = library_.GetFunctionPointer<char*>("GetTime");
    if (get_time == nullptr) {
        *device = 9003;
        return S_FALSE;
    }

    //get_time();

    
    *device = 100;
    return S_OK;
}

STDMETHODIMP CFlashNess::CloseDevice(SHORT device) {
    return S_OK;
}

STDMETHODIMP CFlashNess::Beep(SHORT times) {
    if (!library_.is_valid()) return S_FALSE;

    auto beep = library_.GetFunctionPointer<void>("beep");
    if (beep == nullptr) return S_FALSE;

    for (; times > 0; --times) {
        beep();
    }
    return S_OK;
}

STDMETHODIMP CFlashNess::ReadCard(BSTR* pVal) {
    if (!pVal) return S_FALSE;

    utils::DynamicLibrary library(GetPath(L"XXX.dll"));
    if (!library.is_valid()) {
        CComBSTR value(L"0");
        *pVal = value.Detach();
        return S_FALSE;
    }

    auto sum = library.GetFunctionPointer<int>("fnxxx");    if (sum == nullptr) {        CComBSTR value(L"1");
        *pVal = value.Detach();        return S_FALSE;    }

    std::string temp = std::to_string(sum());
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