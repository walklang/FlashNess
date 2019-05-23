// FlashNess.cpp : CFlashNess µÄÊµÏÖ
#include "stdafx.h"
#include "FlashNess.h"
#include <iostream>
#include <memory>
#include <string>

// CFlashNess

#define WaterCard L"water_card.dll"

extern HMODULE _Instance;

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
        *device = (SHORT)9001;
        CComBSTR path;
        if (S_FALSE == GetModulePath(&path)) return S_FALSE;
        path.Append(WaterCard);
        library_.Reset(internal::LoadLibrary(std::wstring((BSTR)path), nullptr));
        if (!library_.is_valid()) return S_FALSE;
    }

    auto initcom = library_.GetFunctionPointer<void, char*>("initcom");
    //auto initcom = library_.GetFunctionPointer<void, char*>((int)13);
    if (initcom == nullptr) {
        *device = (SHORT)9002;
        return S_FALSE;
    }

    std::string multibyte = CT2CAEX<>(port);
    std::unique_ptr<char[]> p(new char[multibyte.length() + 1]);
    std::memset(p.get(), 0, multibyte.length() + 1);
    strncpy_s(p.get(), multibyte.length() + 1, multibyte.c_str(), multibyte.length());
    initcom(p.get());

    *device = (SHORT)100;
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

    ::MessageBox(NULL, L"debug", L"NPAPI", NULL);

    {
        CComBSTR path;
        if (S_FALSE == GetModulePath(&path)) {
            *pVal = CComBSTR(L"0").Detach();
            return S_FALSE;
        }
        path.Append(L"XXX.dll");
        utils::DynamicLibrary library(std::wstring((BSTR)path));
        if (!library.is_valid()) {
            *pVal = CComBSTR(L"1").Detach();
            return S_FALSE;
        }
        auto sum = library.GetFunctionPointer<int>("fnxxx");
        if (sum == nullptr) {
            *pVal = CComBSTR(L"2").Detach();
            return S_FALSE;
        }

        std::string result = std::to_string(sum());
        *pVal = CComBSTR(result.c_str()).Detach();

    }

    CComBSTR path;
    if (S_FALSE == GetModulePath(&path)) {
        *pVal = CComBSTR(L"0").Detach();
        return S_FALSE;
    }
    path.Append(L"go.dll");
    utils::DynamicLibrary library(std::wstring((BSTR)path));
    if (!library.is_valid()) {
        *pVal = CComBSTR(L"1").Detach();
        return S_FALSE;
    }
    auto sum = library.GetFunctionPointer<int, int, int>("Sum");
    if (sum == nullptr) {
        *pVal = CComBSTR(L"2").Detach();
        return S_FALSE;
    }

    std::wstring result = std::to_wstring(sum(1, 2));
    *pVal = CComBSTR(result.c_str()).Detach();
    auto memory_library = library.Release();

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

STDMETHODIMP CFlashNess::GetModulePath(BSTR* path) {
    if (!path) return S_FALSE;
    TCHAR module[MAX_PATH + 1] = { 0 };
    ::GetModuleFileName(_Instance, module, MAX_PATH);
    std::wstring directory = module;
    std::wstring::size_type npos = directory.rfind('\\');
    if (std::wstring::npos != npos) {
        directory.erase(npos);
        directory += L"\\";
        *path = CComBSTR(directory.c_str()).Detach();
        return S_OK;
    }
    *path = L"";
    return S_FALSE;
}

STDMETHODIMP CFlashNess::CreateService(BSTR path) {
    if (!path) return S_FALSE;
    CComBSTR service_path(path);
    ::ShellExecute(NULL, L"open", (BSTR)service_path, L"", L"", SW_SHOWNORMAL);
    return S_OK;
}