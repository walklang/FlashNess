// FlashNess.cpp : CFlashNess µÄÊµÏÖ
#include "stdafx.h"
#include "FlashNess.h"


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
        *device = 9001;

        TCHAR tszModule[MAX_PATH + 1] = { 0 };
        ::GetModuleFileName(_Instance, tszModule, MAX_PATH);
        std::wstring strPath = tszModule;
        std::wstring::size_type npos = strPath.rfind('\\');
        if( std::wstring::npos == npos ) return S_FALSE;
        strPath.erase(npos);
        strPath += L"\\";
        strPath += WaterCard;
        library_.Reset(internal::LoadLibrary(strPath, nullptr));
        if (!library_.is_valid()) return S_FALSE;
    }

    std::wstring ports = CComBSTR(port).Detach();

    auto initcom = library_.GetFunctionPointer<void, wchar_t*>("initcom");
    if (initcom == nullptr) {
        *device = 9002;
        return S_FALSE;
    }

    initcom(L"1");
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