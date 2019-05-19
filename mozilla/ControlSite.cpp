/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Adam Lock <adamlock@netscape.com>
 *	 Brent Booker
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "StdAfx.h"

#include <Objsafe.h>

#include "ControlSite.h"
#include "PropertyBag.h"
#include "ControlSiteIPFrame.h"
#include <string>
#include <sstream>
#pragma comment(lib, "version.lib")


#define TRACE_METHOD(fn) \
{ \
	ATLTRACE(_T("0x%04x %s()\n"), (int) GetCurrentThreadId(), _T(#fn)); \
}
#define TRACE_METHOD_ARGS(fn, pattern, args) \
{ \
	ATLTRACE(_T("0x%04x %s(") _T(pattern) _T(")\n"), (int) GetCurrentThreadId(), _T(#fn), args); \
}

#define NS_ASSERTION(x, y)

class CDefaultControlSiteSecurityPolicy : public CControlSiteSecurityPolicy
{
    // Test if the specified class id implements the specified category
    BOOL ClassImplementsCategory(const CLSID & clsid, const CATID &catid, BOOL &bClassExists);
public:
    // Test if the class is safe to host
    virtual BOOL IsClassSafeToHost(const CLSID & clsid);
    // Test if the specified class is marked safe for scripting
    virtual BOOL IsClassMarkedSafeForScripting(const CLSID & clsid, BOOL &bClassExists);
    // Test if the instantiated object is safe for scripting on the specified interface
    virtual BOOL IsObjectSafeForScripting(IUnknown *pObject, const IID &iid);
};

BOOL
CDefaultControlSiteSecurityPolicy::ClassImplementsCategory(const CLSID &clsid, const CATID &catid, BOOL &bClassExists)
{
    bClassExists = FALSE;

    // Test if there is a CLSID entry. If there isn't then obviously
    // the object doesn't exist and therefore doesn't implement any category.
    // In this situation, the function returns REGDB_E_CLASSNOTREG.

    CRegKey key;
    if (key.Open(HKEY_CLASSES_ROOT, _T("CLSID"), KEY_READ) != ERROR_SUCCESS)
    {
        // Must fail if we can't even open this!
        return FALSE;
    }
    LPOLESTR szCLSID = NULL;
    if (FAILED(StringFromCLSID(clsid, &szCLSID)))
    {
        return FALSE;
    }
    USES_CONVERSION;
    CRegKey keyCLSID;
    LONG lResult = keyCLSID.Open(key, W2CT(szCLSID), KEY_READ);
    CoTaskMemFree(szCLSID);
    if (lResult != ERROR_SUCCESS)
    {
        // Class doesn't exist
        return FALSE;
    }
    keyCLSID.Close();

    // CLSID exists, so try checking what categories it implements
    bClassExists = TRUE;
    CComQIPtr<ICatInformation> spCatInfo;
    HRESULT hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC_SERVER, IID_ICatInformation, (LPVOID*) &spCatInfo);
    if (spCatInfo == NULL)
    {
        // Must fail if we can't open the category manager
        return FALSE;
    }
    
    // See what categories the class implements
    CComQIPtr<IEnumCATID> spEnumCATID;
    if (FAILED(spCatInfo->EnumImplCategoriesOfClass(clsid, &spEnumCATID)))
    {
        // Can't enumerate classes in category so fail
        return FALSE;
    }

    // Search for matching categories
    BOOL bFound = FALSE;
    CATID catidNext = GUID_NULL;
    while (spEnumCATID->Next(1, &catidNext, NULL) == S_OK)
    {
        if (::IsEqualCATID(catid, catidNext))
        {
            return TRUE;
        }
    }
    return FALSE;
}

// Test if the class is safe to host
BOOL CDefaultControlSiteSecurityPolicy::IsClassSafeToHost(const CLSID & clsid)
{
    return TRUE;
}

// Test if the specified class is marked safe for scripting
BOOL CDefaultControlSiteSecurityPolicy::IsClassMarkedSafeForScripting(const CLSID & clsid, BOOL &bClassExists)
{
    // Test the category the object belongs to
    return ClassImplementsCategory(clsid, CATID_SafeForScripting, bClassExists);
}

// Test if the instantiated object is safe for scripting on the specified interface
BOOL CDefaultControlSiteSecurityPolicy::IsObjectSafeForScripting(IUnknown *pObject, const IID &iid)
{
    if (!pObject) {
        return FALSE;
    }
    // Ask the control if its safe for scripting
    CComQIPtr<IObjectSafety> spObjectSafety = pObject;
    if (!spObjectSafety)
    {
        return FALSE;
    }

    DWORD dwSupported = 0; // Supported options (mask)
    DWORD dwEnabled = 0; // Enabled options

    // Assume scripting via IDispatch, now we doesn't support IDispatchEx
    if (SUCCEEDED(spObjectSafety->GetInterfaceSafetyOptions(
            iid, &dwSupported, &dwEnabled)))
    {
		if (dwEnabled & dwSupported & INTERFACESAFE_FOR_UNTRUSTED_CALLER)
		{
			// Object is safe
			return TRUE;
		}
    }

    // Set it to support untrusted caller.
    if(FAILED(spObjectSafety->SetInterfaceSafetyOptions(
            iid, INTERFACESAFE_FOR_UNTRUSTED_CALLER, INTERFACESAFE_FOR_UNTRUSTED_CALLER)))
    {
        return FALSE;
    }

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////

// Constructor
CControlSite::CControlSite()
{
    TRACE_METHOD(CControlSite::CControlSite);

    m_hWndParent = NULL;
    m_CLSID = CLSID_NULL;
    m_bSetClientSiteFirst = FALSE;
    m_bVisibleAtRuntime = TRUE;
    memset(&m_rcObjectPos, 0, sizeof(m_rcObjectPos));

    m_bInPlaceActive = FALSE;
    m_bUIActive = FALSE;
    m_bInPlaceLocked = FALSE;
    m_bWindowless = FALSE;
    m_bSupportWindowlessActivation = TRUE;
    m_bSafeForScriptingObjectsOnly = FALSE;
    m_pSecurityPolicy = GetDefaultControlSecurityPolicy();

    // Initialise ambient properties
    m_nAmbientLocale = 0;
    m_clrAmbientForeColor = ::GetSysColor(COLOR_WINDOWTEXT);
    m_clrAmbientBackColor = ::GetSysColor(COLOR_WINDOW);
    m_bAmbientUserMode = true;
    m_bAmbientShowHatching = true;
    m_bAmbientShowGrabHandles = true;
    m_bAmbientAppearance = true; // 3d

    // Windowless variables
    m_hDCBuffer = NULL;
    m_hRgnBuffer = NULL;
    m_hBMBufferOld = NULL;
    m_hBMBuffer = NULL;

	m_spInner = NULL;

	m_needUpdateContainerSize = false;
	m_currentSize.cx = m_currentSize.cy = -1;
}


// Destructor
CControlSite::~CControlSite()
{
    TRACE_METHOD(CControlSite::~CControlSite);
    Detach();
	if (m_spInner && m_spInnerDeallocater) {
		m_spInnerDeallocater(m_spInner);
	}
}

// Create the specified control, optionally providing properties to initialise
// it with and a name.
HRESULT CControlSite::Create(REFCLSID clsid, PropertyList &pl,
                             LPCWSTR szCodebase, IBindCtx *pBindContext)
{
    TRACE_METHOD(CControlSite::Create);

    m_CLSID = clsid;
    m_ParameterList = pl;

    // See if security policy will allow the control to be hosted
    if (m_pSecurityPolicy && !m_pSecurityPolicy->IsClassSafeToHost(clsid))
    {
        return E_FAIL;
    }

    // See if object is script safe
    BOOL checkForObjectSafety = FALSE;
    if (m_pSecurityPolicy && m_bSafeForScriptingObjectsOnly)
    {
        BOOL bClassExists = FALSE;
        BOOL bIsSafe = m_pSecurityPolicy->IsClassMarkedSafeForScripting(clsid, bClassExists);
        if (!bClassExists && szCodebase)
        {
            // Class doesn't exist, so allow code below to fetch it
        }
        else if (!bIsSafe)
        {
            // The class is not flagged as safe for scripting, so
            // we'll have to create it to ask it if its safe.
            checkForObjectSafety = TRUE;
        }
    }

    //Now Check if the control version needs to be updated.
    BOOL bUpdateControlVersion = FALSE;
    wchar_t *szURL = NULL;
    DWORD dwFileVersionMS = 0xffffffff;
    DWORD dwFileVersionLS = 0xffffffff;

    if(szCodebase)
    {
        HKEY hk = NULL;   
        wchar_t wszData[MAX_PATH];
        LPWSTR pwszClsid = NULL;
        DWORD dwSize = 255;
        DWORD dwHandle, dwLength, dwRegReturn;
        DWORD dwExistingFileVerMS = 0xffffffff;
        DWORD dwExistingFileVerLS = 0xffffffff;
        BOOL bFoundLocalVerInfo = FALSE;

        StringFromCLSID(clsid, (LPOLESTR*)&pwszClsid);

		std::wostringstream wss(L"");
		wss << L"CLSID\\";
		wss << pwszClsid;
		wss << L"\\InprocServer32";
		std::wstring strKey = wss.str();

        if ( RegOpenKeyExW( HKEY_CLASSES_ROOT, strKey.c_str(), 0, KEY_READ, &hk ) == ERROR_SUCCESS )
        {
            dwRegReturn = RegQueryValueExW( hk, L"", NULL, NULL, (LPBYTE)wszData, &dwSize );
            RegCloseKey( hk );
        }

        if(dwRegReturn == ERROR_SUCCESS)
        {
            VS_FIXEDFILEINFO *pFileInfo;
            UINT uLen; 
            dwLength = GetFileVersionInfoSizeW( wszData , &dwHandle ); 
            LPBYTE lpData = new BYTE[dwLength]; 
            GetFileVersionInfoW(wszData, 0, dwLength, lpData ); 
            bFoundLocalVerInfo = VerQueryValue( lpData, L"\\", (LPVOID*)&pFileInfo, &uLen ); 

            if(bFoundLocalVerInfo)
            {
                dwExistingFileVerMS = pFileInfo->dwFileVersionMS;
                dwExistingFileVerLS = pFileInfo->dwFileVersionLS;
            }

            delete [] lpData;
        }

        // Test if the code base ends in #version=a,b,c,d
        const wchar_t *szHash = wcsrchr(szCodebase, wchar_t('#'));
        if (szHash)
        {
            if (_wcsnicmp(szHash, L"#version=", 9) == 0)
            {
                int a, b, c, d;
                if (swscanf_s(szHash + 9, L"%d,%d,%d,%d", &a, &b, &c, &d) == 4)
                {
                    dwFileVersionMS = MAKELONG(b,a);
                    dwFileVersionLS = MAKELONG(d,c);

                    //If local version info was found compare it
                    if(bFoundLocalVerInfo)
                    {
                        if(dwFileVersionMS > dwExistingFileVerMS)
                            bUpdateControlVersion = TRUE;

                        if((dwFileVersionMS == dwExistingFileVerMS) && (dwFileVersionLS > dwExistingFileVerLS))
                            bUpdateControlVersion = TRUE;
                    }

                }
            }
            szURL = _wcsdup(szCodebase);
            // Terminate at the hash mark
            if (szURL)
                szURL[szHash - szCodebase] = wchar_t('\0');
        }
        else
        {
            szURL = _wcsdup(szCodebase);
        }
    }

    CComPtr<IUnknown> spObject;
    HRESULT hr;
    //If the control needs to be updated do not call CoCreateInstance otherwise you will lock files
    //and force a reboot on CoGetClassObjectFromURL
    if(!bUpdateControlVersion)
    {
        // Create the object
        hr = CoCreateInstance(clsid, NULL, CLSCTX_ALL, IID_IUnknown, (void **) &spObject);
        if (SUCCEEDED(hr) && checkForObjectSafety)
        {
			m_bSafeForScriptingObjectsOnly = m_pSecurityPolicy->IsObjectSafeForScripting(spObject,  __uuidof(IDispatch));
            // Drop through, success!
        }
    }

    // Do we need to download the control?
    if ((FAILED(hr) && szCodebase) || (bUpdateControlVersion))
    {

        if (!szURL)
            return E_OUTOFMEMORY;

        CComPtr<IBindCtx> spBindContext; 
        CComPtr<IBindStatusCallback> spBindStatusCallback;
        CComPtr<IBindStatusCallback> spOldBSC;

        // Create our own bind context or use the one provided?
        BOOL useInternalBSC = FALSE;
        if (!pBindContext)
        {
            useInternalBSC = TRUE;
            hr = CreateBindCtx(0, &spBindContext);
            if (FAILED(hr))
            {
                free(szURL);
                return hr;
            }

            spBindStatusCallback = dynamic_cast<IBindStatusCallback *>(this);
            hr = RegisterBindStatusCallback(spBindContext, spBindStatusCallback, &spOldBSC, 0);
            if (FAILED(hr))
            {
                free(szURL);
                return hr;
            }
        }
        else
        {
            spBindContext = pBindContext;
        }

        //If the version from the CODEBASE value is greater than the installed control
        //Call CoGetClassObjectFromURL with CLSID_NULL to prevent system change reboot prompt 
        if(bUpdateControlVersion)
        {
            hr = CoGetClassObjectFromURL(clsid, szURL, dwFileVersionMS, dwFileVersionLS, 
                                         NULL, spBindContext, 
                                         CLSCTX_INPROC_HANDLER | CLSCTX_INPROC_SERVER, 
                                         0, IID_IClassFactory, NULL);
        }
        else
        {
            hr = CoGetClassObjectFromURL(CLSID_NULL, szURL, dwFileVersionMS, dwFileVersionLS,
                                         NULL, spBindContext, CLSCTX_ALL, NULL, IID_IUnknown, 
                                         NULL);
        }


        free(szURL);

        // Handle the internal binding synchronously so the object exists
        // or an error code is available when the method returns.
        if (useInternalBSC)
        {
            if (MK_S_ASYNCHRONOUS == hr)
            {
                m_bBindingInProgress = TRUE;
                m_hrBindResult = E_FAIL;

                // Spin around waiting for binding to complete
                HANDLE hFakeEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
                while (m_bBindingInProgress)
                {
                    MSG msg;
                    // Process pending messages
                    while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
                    {
                        if (!::GetMessage(&msg, NULL, 0, 0))
                        {
                            m_bBindingInProgress = FALSE;
                            break;
                        }
                        ::TranslateMessage(&msg);
                        ::DispatchMessage(&msg);
                    }
                    if (!m_bBindingInProgress)
                        break;
                    // Sleep for a bit or the next msg to appear
                    ::MsgWaitForMultipleObjects(1, &hFakeEvent, FALSE, 500, QS_ALLEVENTS);
                }
                ::CloseHandle(hFakeEvent);

                // Set the result
                hr = m_hrBindResult;
            }

            // Destroy the bind status callback & context
            if (spBindStatusCallback)
            {
                RevokeBindStatusCallback(spBindContext, spBindStatusCallback);
                spBindStatusCallback.Release();
                spBindContext.Release();
            }
        }
        //added to create control
        if (SUCCEEDED(hr)) 
        { 
            // Create the object 
            hr = CoCreateInstance(clsid, NULL, CLSCTX_ALL, IID_IUnknown, (void **)  &spObject); 

            if (SUCCEEDED(hr) && checkForObjectSafety) 
            { 
                // Assume scripting via IDispatch 
                m_bSafeForScriptingObjectsOnly = m_pSecurityPolicy->IsObjectSafeForScripting(spObject,  __uuidof(IDispatch));
            } 
        }
        //EOF test code
    }

    if (spObject)
    {        
        m_spObject = spObject;

		CComQIPtr<IObjectWithSite> site = m_spObject;
		if (site) {
			site->SetSite(GetUnknown());
		}
    }    
    return hr;
}

// Attach the created control to a window and activate it
HRESULT CControlSite::Attach(HWND hwndParent, const RECT &rcPos, IUnknown *pInitStream)
{
    TRACE_METHOD(CControlSite::Attach);

    if (hwndParent == NULL)
    {
        NS_ASSERTION(0, "No parent hwnd");
        return E_INVALIDARG;
    }

    m_hWndParent = hwndParent;
    m_rcObjectPos = rcPos;

    // Object must have been created
    if (m_spObject == NULL)
    {
        return E_UNEXPECTED;
    }

	m_spIOleControl = m_spObject;
    m_spIViewObject = m_spObject;
    m_spIOleObject = m_spObject;
    
    if (m_spIOleObject == NULL)
    {
        return E_FAIL;
    }
	if (m_spIOleControl) {
		m_spIOleControl->FreezeEvents(true);
	}
    
    DWORD dwMiscStatus;
    m_spIOleObject->GetMiscStatus(DVASPECT_CONTENT, &dwMiscStatus);

    if (dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST)
    {
        m_bSetClientSiteFirst = TRUE;
    }
    if (dwMiscStatus & OLEMISC_INVISIBLEATRUNTIME)
    {
        m_bVisibleAtRuntime = FALSE;
    }

    // Some objects like to have the client site as the first thing
    // to be initialised (for ambient properties and so forth)
    if (m_bSetClientSiteFirst)
    {
        m_spIOleObject->SetClientSite(this);
    }

    // If there is a parameter list for the object and no init stream then
    // create one here.
    CPropertyBagInstance *pPropertyBag = NULL;
    if (pInitStream == NULL && m_ParameterList.GetSize() > 0)
    {
        CPropertyBagInstance::CreateInstance(&pPropertyBag);
        pPropertyBag->AddRef();
        for (unsigned long i = 0; i < m_ParameterList.GetSize(); i++)
        {
            pPropertyBag->Write(m_ParameterList.GetNameOf(i),
                const_cast<VARIANT *>(m_ParameterList.GetValueOf(i)));
        }
        pInitStream = (IPersistPropertyBag *) pPropertyBag;
    }

    // Initialise the control from store if one is provided
    if (pInitStream)
    {
        CComQIPtr<IPropertyBag, &IID_IPropertyBag> spPropertyBag = pInitStream;
        CComQIPtr<IStream, &IID_IStream> spStream = pInitStream;
        CComQIPtr<IPersistStream, &IID_IPersistStream> spIPersistStream = m_spIOleObject;
        CComQIPtr<IPersistPropertyBag, &IID_IPersistPropertyBag> spIPersistPropertyBag = m_spIOleObject;

        if (spIPersistPropertyBag && spPropertyBag)
        {
            spIPersistPropertyBag->Load(spPropertyBag, NULL);
        }
        else if (spIPersistStream && spStream)
        {
            spIPersistStream->Load(spStream);
        }
    }
    else
    {
        // Initialise the object if possible
        CComQIPtr<IPersistStreamInit, &IID_IPersistStreamInit> spIPersistStreamInit = m_spIOleObject;
        if (spIPersistStreamInit)
        {
            spIPersistStreamInit->InitNew();
        }
    }

	SIZEL size, sizein;
	sizein.cx = m_rcObjectPos.right - m_rcObjectPos.left;
	sizein.cy = m_rcObjectPos.bottom - m_rcObjectPos.top;

	SetControlSize(&sizein, &size);
	m_rcObjectPos.right = m_rcObjectPos.left + size.cx;
	m_rcObjectPos.bottom = m_rcObjectPos.top + size.cy;

    m_spIOleInPlaceObject = m_spObject;
    
	if (m_spIOleControl) {
		m_spIOleControl->FreezeEvents(false);
	}

    // In-place activate the object
    if (m_bVisibleAtRuntime)
    {
        DoVerb(OLEIVERB_INPLACEACTIVATE);
    }

	m_spIOleInPlaceObjectWindowless = m_spObject;

    // For those objects which haven't had their client site set yet,
    // it's done here.
    if (!m_bSetClientSiteFirst)
    {
        m_spIOleObject->SetClientSite(this);
    }

    return S_OK;
}


// Set the control size, in pixels.
HRESULT CControlSite::SetControlSize(const LPSIZEL size, LPSIZEL out)
{
	if (!size || !out) {
		return E_POINTER;
	}
	if (!m_spIOleObject) {
		return E_FAIL;
	}

	SIZEL szInitialHiMetric, szCustomHiMetric, szFinalHiMetric;
	SIZEL szCustomPixel, szFinalPixel;
	szFinalPixel = *size;
	szCustomPixel = *size;
	
	if (m_currentSize.cx == size->cx && m_currentSize.cy == size->cy) {
		// Don't need to change.
		return S_OK;
	}
	if (SUCCEEDED(m_spIOleObject->GetExtent(DVASPECT_CONTENT, &szInitialHiMetric))) {
		if (m_currentSize.cx == -1 && szCustomPixel.cx == 300 && szCustomPixel.cy == 150) {
			szFinalHiMetric = szInitialHiMetric;
		} else {
			AtlPixelToHiMetric(&szCustomPixel, &szCustomHiMetric);
			szFinalHiMetric = szCustomHiMetric;
		}
	}
	if (SUCCEEDED(m_spIOleObject->SetExtent(DVASPECT_CONTENT, &szFinalHiMetric))) {
		if (SUCCEEDED(m_spIOleObject->GetExtent(DVASPECT_CONTENT, &szFinalHiMetric))) {
			AtlHiMetricToPixel(&szFinalHiMetric, &szFinalPixel);
		}
	}
	m_currentSize = szFinalPixel;
	if (szCustomPixel.cx != szFinalPixel.cx && szCustomPixel.cy != szFinalPixel.cy) {
		m_needUpdateContainerSize = true;
	}
	*out = szFinalPixel;
	return S_OK;
}

// Unhook the control from the window and throw it all away
HRESULT CControlSite::Detach()
{
    TRACE_METHOD(CControlSite::Detach);

    if (m_spIOleInPlaceObjectWindowless)
    {
        m_spIOleInPlaceObjectWindowless.Release();
    }
	
	CComQIPtr<IObjectWithSite> site = m_spObject;
	if (site) {
		site->SetSite(NULL);
	}

    if (m_spIOleInPlaceObject)
    {
        m_spIOleInPlaceObject->InPlaceDeactivate();
        m_spIOleInPlaceObject.Release();
    }

    if (m_spIOleObject)
    {
        m_spIOleObject->Close(OLECLOSE_NOSAVE);
        m_spIOleObject->SetClientSite(NULL);
        m_spIOleObject.Release();
    }

    m_spIViewObject.Release();
    m_spObject.Release();
    
    return S_OK;
}


// Return the IUnknown of the contained control
HRESULT CControlSite::GetControlUnknown(IUnknown **ppObject)
{
    *ppObject = NULL;
    if (m_spObject)
    {
        m_spObject->QueryInterface(IID_IUnknown, (void **) ppObject);
		return S_OK;
    } else {
		return E_FAIL;
	}
}

// Subscribe to an event sink on the control
HRESULT CControlSite::Advise(IUnknown *pIUnkSink, const IID &iid, DWORD *pdwCookie)
{
    if (m_spObject == NULL)
    {
        return E_UNEXPECTED;
    }

    if (pIUnkSink == NULL || pdwCookie == NULL)
    {
        return E_INVALIDARG;
    }

    return AtlAdvise(m_spObject, pIUnkSink, iid, pdwCookie);
}


// Unsubscribe event sink from the control
HRESULT CControlSite::Unadvise(const IID &iid, DWORD dwCookie)
{
    if (m_spObject == NULL)
    {
        return E_UNEXPECTED;
    }

    return AtlUnadvise(m_spObject, iid, dwCookie);
}


// Draw the control
HRESULT CControlSite::Draw(HDC hdc)
{
    TRACE_METHOD(CControlSite::Draw);

    // Draw only when control is windowless or deactivated
    if (m_spIViewObject)
    {
        if (m_bWindowless || !m_bInPlaceActive)
        {
            RECTL *prcBounds = (m_bWindowless) ? NULL : (RECTL *) &m_rcObjectPos;
            m_spIViewObject->Draw(DVASPECT_CONTENT, -1, NULL, NULL, NULL, hdc, prcBounds, NULL, NULL, 0);
        }
    }
    else
    {
        // Draw something to indicate no control is there
        HBRUSH hbr = CreateSolidBrush(RGB(200,200,200));
        FillRect(hdc, &m_rcObjectPos, hbr);
        DeleteObject(hbr);
    }

    return S_OK;
}


// Execute the specified verb
HRESULT CControlSite::DoVerb(LONG nVerb, LPMSG lpMsg)
{
    TRACE_METHOD(CControlSite::DoVerb);

    if (m_spIOleObject == NULL)
    {
        return E_FAIL;
    }
	
//	InstallAtlThunk();
    return m_spIOleObject->DoVerb(nVerb, lpMsg, this, 0, m_hWndParent, &m_rcObjectPos);
}


// Set the position on the control
HRESULT CControlSite::SetPosition(const RECT &rcPos)
{
	HWND hwnd;
    TRACE_METHOD(CControlSite::SetPosition);
    m_rcObjectPos = rcPos;

    if (m_spIOleInPlaceObject && SUCCEEDED(m_spIOleInPlaceObject->GetWindow(&hwnd)))
    {
		m_spIOleInPlaceObject->SetObjectRects(&m_rcObjectPos, &m_rcObjectPos);
    }
    return S_OK;
}


HRESULT CControlSite::GetControlSize(LPSIZEL size){
	if (m_spIOleObject) {
		SIZEL szHiMetric;
		HRESULT hr = m_spIOleObject->GetExtent(DVASPECT_CONTENT, &szHiMetric);
		AtlHiMetricToPixel(&szHiMetric, size);
		return hr;
	}
	return E_FAIL;
}

void CControlSite::FireAmbientPropertyChange(DISPID id)
{
    if (m_spObject)
    {
        CComQIPtr<IOleControl> spControl = m_spObject;
        if (spControl)
        {
            spControl->OnAmbientPropertyChange(id);
        }
    }
}


void CControlSite::SetAmbientUserMode(BOOL bUserMode)
{
    bool bNewMode = bUserMode ? true : false;
    if (m_bAmbientUserMode != bNewMode)
    {
        m_bAmbientUserMode = bNewMode;
        FireAmbientPropertyChange(DISPID_AMBIENT_USERMODE);
    }
}

///////////////////////////////////////////////////////////////////////////////
// CControlSiteSecurityPolicy implementation

CControlSiteSecurityPolicy *CControlSite::GetDefaultControlSecurityPolicy()
{
    static CDefaultControlSiteSecurityPolicy defaultControlSecurityPolicy;
    return &defaultControlSecurityPolicy;
}

///////////////////////////////////////////////////////////////////////////////
// IDispatch implementation


HRESULT STDMETHODCALLTYPE CControlSite::GetTypeInfoCount(/* [out] */ UINT __RPC_FAR *pctinfo)
{
	ATLTRACE("Not implemnted");
    return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CControlSite::GetTypeInfo(/* [in] */ UINT iTInfo, /* [in] */ LCID lcid, /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo)
{
	ATLTRACE("Not implemnted");
    return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CControlSite::GetIDsOfNames(/* [in] */ REFIID riid, /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames, /* [in] */ UINT cNames, /* [in] */ LCID lcid, /* [size_is][out] */ DISPID __RPC_FAR *rgDispId)
{
	ATLTRACE("Not implemnted");
    return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CControlSite::Invoke(/* [in] */ DISPID dispIdMember, /* [in] */ REFIID riid, /* [in] */ LCID lcid, /* [in] */ WORD wFlags, /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams, /* [out] */ VARIANT __RPC_FAR *pVarResult, /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo, /* [out] */ UINT __RPC_FAR *puArgErr)
{
    if (wFlags & DISPATCH_PROPERTYGET)
    {
        CComVariant vResult;

        switch (dispIdMember)
        {
        case DISPID_AMBIENT_APPEARANCE:
            vResult = CComVariant(m_bAmbientAppearance);
            break;

        case DISPID_AMBIENT_FORECOLOR:
            vResult = CComVariant((long) m_clrAmbientForeColor);
            break;

        case DISPID_AMBIENT_BACKCOLOR:
            vResult = CComVariant((long) m_clrAmbientBackColor);
            break;

        case DISPID_AMBIENT_LOCALEID:
            vResult = CComVariant((long) m_nAmbientLocale);
            break;

        case DISPID_AMBIENT_USERMODE:
            vResult = CComVariant(m_bAmbientUserMode);
            break;
        
        case DISPID_AMBIENT_SHOWGRABHANDLES:
            vResult = CComVariant(m_bAmbientShowGrabHandles);
            break;
        
        case DISPID_AMBIENT_SHOWHATCHING:
            vResult = CComVariant(m_bAmbientShowHatching);
            break;

        default:
            return DISP_E_MEMBERNOTFOUND;
        }

        VariantCopy(pVarResult, &vResult);
        return S_OK;
    }

    return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
// IAdviseSink implementation


void STDMETHODCALLTYPE CControlSite::OnDataChange(/* [unique][in] */ FORMATETC __RPC_FAR *pFormatetc, /* [unique][in] */ STGMEDIUM __RPC_FAR *pStgmed)
{
}


void STDMETHODCALLTYPE CControlSite::OnViewChange(/* [in] */ DWORD dwAspect, /* [in] */ LONG lindex)
{
    // Redraw the control
    InvalidateRect(NULL, FALSE);
}


void STDMETHODCALLTYPE CControlSite::OnRename(/* [in] */ IMoniker __RPC_FAR *pmk)
{
}


void STDMETHODCALLTYPE CControlSite::OnSave(void)
{
}


void STDMETHODCALLTYPE CControlSite::OnClose(void)
{
}


///////////////////////////////////////////////////////////////////////////////
// IAdviseSink2 implementation


void STDMETHODCALLTYPE CControlSite::OnLinkSrcChange(/* [unique][in] */ IMoniker __RPC_FAR *pmk)
{
}


///////////////////////////////////////////////////////////////////////////////
// IAdviseSinkEx implementation


void STDMETHODCALLTYPE CControlSite::OnViewStatusChange(/* [in] */ DWORD dwViewStatus)
{
}


///////////////////////////////////////////////////////////////////////////////
// IOleWindow implementation

HRESULT STDMETHODCALLTYPE CControlSite::GetWindow(/* [out] */ HWND __RPC_FAR *phwnd)
{
    *phwnd = m_hWndParent;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::ContextSensitiveHelp(/* [in] */ BOOL fEnterMode)
{
    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
// IOleClientSite implementation


HRESULT STDMETHODCALLTYPE CControlSite::SaveObject(void)
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::GetMoniker(/* [in] */ DWORD dwAssign, /* [in] */ DWORD dwWhichMoniker, /* [out] */ IMoniker __RPC_FAR *__RPC_FAR *ppmk)
{
	if (!ppmk) {
		return E_INVALIDARG;
	}

	if (dwWhichMoniker != OLEWHICHMK_CONTAINER) {
		return E_FAIL;
	}

	return CreateURLMoniker(NULL, url, ppmk);
}

void CControlSite::SetUrl(BSTR url) {
	this->url = url;
}

HRESULT STDMETHODCALLTYPE CControlSite::GetContainer(/* [out] */ IOleContainer __RPC_FAR *__RPC_FAR *ppContainer)
{
    if (!ppContainer) return E_INVALIDARG;
	CComQIPtr<IOleContainer> container(this->GetUnknown());
    *ppContainer = container;
    if (*ppContainer)
    {
        (*ppContainer)->AddRef();
    }
    return (*ppContainer) ? S_OK : E_NOINTERFACE;
}


HRESULT STDMETHODCALLTYPE CControlSite::ShowObject(void)
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::OnShowWindow(/* [in] */ BOOL fShow)
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::RequestNewObjectLayout(void)
{
	ATLTRACE("Not implemnted");
    return E_NOTIMPL;
}


///////////////////////////////////////////////////////////////////////////////
// IOleInPlaceSite implementation


HRESULT STDMETHODCALLTYPE CControlSite::CanInPlaceActivate(void)
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::OnInPlaceActivate(void)
{
    m_bInPlaceActive = TRUE;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::OnUIActivate(void)
{
    m_bUIActive = TRUE;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::GetWindowContext(/* [out] */ IOleInPlaceFrame __RPC_FAR *__RPC_FAR *ppFrame, /* [out] */ IOleInPlaceUIWindow __RPC_FAR *__RPC_FAR *ppDoc, /* [out] */ LPRECT lprcPosRect, /* [out] */ LPRECT lprcClipRect, /* [out][in] */ LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    *lprcPosRect = m_rcObjectPos;
    *lprcClipRect = m_rcObjectPos;

    CControlSiteIPFrameInstance *pIPFrame = NULL;
    CControlSiteIPFrameInstance::CreateInstance(&pIPFrame);
    pIPFrame->AddRef();

    *ppFrame = (IOleInPlaceFrame *) pIPFrame;
    *ppDoc = NULL;

    lpFrameInfo->fMDIApp = FALSE;
    lpFrameInfo->hwndFrame = NULL;
    lpFrameInfo->haccel = NULL;
    lpFrameInfo->cAccelEntries = 0;

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::Scroll(/* [in] */ SIZE scrollExtant)
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::OnUIDeactivate(/* [in] */ BOOL fUndoable)
{
    m_bUIActive = FALSE;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::OnInPlaceDeactivate(void)
{
    m_bInPlaceActive = FALSE;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::DiscardUndoState(void)
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::DeactivateAndUndo(void)
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::OnPosRectChange(/* [in] */ LPCRECT lprcPosRect)
{
    if (lprcPosRect == NULL)
    {
        return E_INVALIDARG;
    }
    SetPosition(m_rcObjectPos);
    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
// IOleInPlaceSiteEx implementation


HRESULT STDMETHODCALLTYPE CControlSite::OnInPlaceActivateEx(/* [out] */ BOOL __RPC_FAR *pfNoRedraw, /* [in] */ DWORD dwFlags)
{
    m_bInPlaceActive = TRUE;

    if (pfNoRedraw)
    {
        *pfNoRedraw = FALSE;
    }
    if (dwFlags & ACTIVATE_WINDOWLESS)
    {
        if (!m_bSupportWindowlessActivation)
        {
            return E_INVALIDARG;
        }
        m_bWindowless = TRUE;
    }
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::OnInPlaceDeactivateEx(/* [in] */ BOOL fNoRedraw)
{
    m_bInPlaceActive = FALSE;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::RequestUIActivate(void)
{
    return S_FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// IOleInPlaceSiteWindowless implementation


HRESULT STDMETHODCALLTYPE CControlSite::CanWindowlessActivate(void)
{
    // Allow windowless activation?
    return (m_bSupportWindowlessActivation) ? S_OK : S_FALSE;
}


HRESULT STDMETHODCALLTYPE CControlSite::GetCapture(void)
{
    // TODO capture the mouse for the object
    return S_FALSE;
}


HRESULT STDMETHODCALLTYPE CControlSite::SetCapture(/* [in] */ BOOL fCapture)
{
    // TODO capture the mouse for the object
    return S_FALSE;
}


HRESULT STDMETHODCALLTYPE CControlSite::GetFocus(void)
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::SetFocus(/* [in] */ BOOL fFocus)
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::GetDC(/* [in] */ LPCRECT pRect, /* [in] */ DWORD grfFlags, /* [out] */ HDC __RPC_FAR *phDC)
{
    if (phDC == NULL)
    {
        return E_INVALIDARG;
    }

    // Can't do nested painting
    if (m_hDCBuffer != NULL)
    {
        return E_UNEXPECTED;
    }

    m_rcBuffer = m_rcObjectPos;
    if (pRect != NULL)
    {
        m_rcBuffer = *pRect;
    }

    m_hBMBuffer = NULL;
    m_dwBufferFlags = grfFlags;

    // See if the control wants a DC that is onscreen or offscreen
    if (m_dwBufferFlags & OLEDC_OFFSCREEN)
    {
        m_hDCBuffer = CreateCompatibleDC(NULL);
        if (m_hDCBuffer == NULL)
        {
            // Error
            return E_OUTOFMEMORY;
        }

        long cx = m_rcBuffer.right - m_rcBuffer.left;
        long cy = m_rcBuffer.bottom - m_rcBuffer.top;

        m_hBMBuffer = CreateCompatibleBitmap(m_hDCBuffer, cx, cy);
        m_hBMBufferOld = (HBITMAP) SelectObject(m_hDCBuffer, m_hBMBuffer);
        SetViewportOrgEx(m_hDCBuffer, m_rcBuffer.left, m_rcBuffer.top, NULL);

        // TODO When OLEDC_PAINTBKGND we must draw every site behind this one
    }
    else
    {
        // TODO When OLEDC_PAINTBKGND we must draw every site behind this one

        // Get the window DC
        m_hDCBuffer = GetWindowDC(m_hWndParent);
        if (m_hDCBuffer == NULL)
        {
            // Error
            return E_OUTOFMEMORY;
        }

        // Clip the control so it can't trash anywhere it isn't allowed to draw
        if (!(m_dwBufferFlags & OLEDC_NODRAW))
        {
            m_hRgnBuffer = CreateRectRgnIndirect(&m_rcBuffer);

            // TODO Clip out opaque areas of sites behind this one

            SelectClipRgn(m_hDCBuffer, m_hRgnBuffer);
        }
    }

    *phDC = m_hDCBuffer;

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::ReleaseDC(/* [in] */ HDC hDC)
{
    // Release the DC
    if (hDC == NULL || hDC != m_hDCBuffer)
    {
        return E_INVALIDARG;
    }

    // Test if the DC was offscreen or onscreen
    if ((m_dwBufferFlags & OLEDC_OFFSCREEN) &&
        !(m_dwBufferFlags & OLEDC_NODRAW))
    {
        // BitBlt the buffer into the control's object
        SetViewportOrgEx(m_hDCBuffer, 0, 0, NULL);
        HDC hdc = GetWindowDC(m_hWndParent);

        long cx = m_rcBuffer.right - m_rcBuffer.left;
        long cy = m_rcBuffer.bottom - m_rcBuffer.top;

        BitBlt(hdc, m_rcBuffer.left, m_rcBuffer.top, cx, cy, m_hDCBuffer, 0, 0, SRCCOPY);
        
        ::ReleaseDC(m_hWndParent, hdc);
    }
    else
    {
        // TODO If OLEDC_PAINTBKGND is set draw the DVASPECT_CONTENT of every object above this one
    }

    // Clean up settings ready for next drawing
    if (m_hRgnBuffer)
    {
        SelectClipRgn(m_hDCBuffer, NULL);
        DeleteObject(m_hRgnBuffer);
        m_hRgnBuffer = NULL;
    }
    
    SelectObject(m_hDCBuffer, m_hBMBufferOld);
    if (m_hBMBuffer)
    {
        DeleteObject(m_hBMBuffer);
        m_hBMBuffer = NULL;
    }

    // Delete the DC
    if (m_dwBufferFlags & OLEDC_OFFSCREEN)
    {
        ::DeleteDC(m_hDCBuffer);
    }
    else
    {
        ::ReleaseDC(m_hWndParent, m_hDCBuffer);
    }
    m_hDCBuffer = NULL;

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::InvalidateRect(/* [in] */ LPCRECT pRect, /* [in] */ BOOL fErase)
{
    // Clip the rectangle against the object's size and invalidate it
    RECT rcI = { 0, 0, 0, 0 };
    if (pRect == NULL)
    {
        rcI = m_rcObjectPos;
    }
    else
    {
        IntersectRect(&rcI, &m_rcObjectPos, pRect);
    }
    ::InvalidateRect(m_hWndParent, &rcI, fErase);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CControlSite::InvalidateRgn(/* [in] */ HRGN hRGN, /* [in] */ BOOL fErase)
{
    if (hRGN == NULL)
    {
        ::InvalidateRect(m_hWndParent, &m_rcObjectPos, fErase);
    }
    else
    {
        // Clip the region with the object's bounding area
        HRGN hrgnClip = CreateRectRgnIndirect(&m_rcObjectPos);
        if (CombineRgn(hrgnClip, hrgnClip, hRGN, RGN_AND) != 0)
        {
            ::InvalidateRgn(m_hWndParent, hrgnClip, fErase);
        }
        DeleteObject(hrgnClip);
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CControlSite::ScrollRect(/* [in] */ INT dx, /* [in] */ INT dy, /* [in] */ LPCRECT pRectScroll, /* [in] */ LPCRECT pRectClip)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CControlSite::AdjustRect(/* [out][in] */ LPRECT prc)
{
    if (prc == NULL)
    {
        return E_INVALIDARG;
    }

    // Clip the rectangle against the object position
    RECT rcI = { 0, 0, 0, 0 };
    IntersectRect(&rcI, &m_rcObjectPos, prc);
    *prc = rcI;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CControlSite::OnDefWindowMessage(/* [in] */ UINT msg, /* [in] */ WPARAM wParam, /* [in] */ LPARAM lParam, /* [out] */ LRESULT __RPC_FAR *plResult)
{
    if (plResult == NULL)
    {
        return E_INVALIDARG;
    }
    
    // Pass the message to the windowless control
    if (m_bWindowless && m_spIOleInPlaceObjectWindowless)
    {
        return m_spIOleInPlaceObjectWindowless->OnWindowMessage(msg, wParam, lParam, plResult);
    }
	else if (m_spIOleInPlaceObject) {

		HWND wnd;
		if (SUCCEEDED(m_spIOleInPlaceObject->GetWindow(&wnd)))
			SendMessage(wnd, msg, wParam, lParam);
	}

    return S_FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// IOleControlSite implementation


HRESULT STDMETHODCALLTYPE CControlSite::OnControlInfoChanged(void)
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::LockInPlaceActive(/* [in] */ BOOL fLock)
{
    m_bInPlaceLocked = fLock;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::GetExtendedControl(/* [out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDisp)
{
	ATLTRACE("Not implemnted");
    return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CControlSite::TransformCoords(/* [out][in] */ POINTL __RPC_FAR *pPtlHimetric, /* [out][in] */ POINTF __RPC_FAR *pPtfContainer, /* [in] */ DWORD dwFlags)
{
    HRESULT hr = S_OK;

    if (pPtlHimetric == NULL)
    {
        return E_INVALIDARG;
    }
    if (pPtfContainer == NULL)
    {
        return E_INVALIDARG;
    }

    HDC hdc = ::GetDC(m_hWndParent);
    ::SetMapMode(hdc, MM_HIMETRIC);
    POINT rgptConvert[2];
    rgptConvert[0].x = 0;
    rgptConvert[0].y = 0;

    if (dwFlags & XFORMCOORDS_HIMETRICTOCONTAINER)
    {
        rgptConvert[1].x = pPtlHimetric->x;
        rgptConvert[1].y = pPtlHimetric->y;
        ::LPtoDP(hdc, rgptConvert, 2);
        if (dwFlags & XFORMCOORDS_SIZE)
        {
            pPtfContainer->x = (float)(rgptConvert[1].x - rgptConvert[0].x);
            pPtfContainer->y = (float)(rgptConvert[0].y - rgptConvert[1].y);
        }
        else if (dwFlags & XFORMCOORDS_POSITION)
        {
            pPtfContainer->x = (float)rgptConvert[1].x;
            pPtfContainer->y = (float)rgptConvert[1].y;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    else if (dwFlags & XFORMCOORDS_CONTAINERTOHIMETRIC)
    {
        rgptConvert[1].x = (int)(pPtfContainer->x);
        rgptConvert[1].y = (int)(pPtfContainer->y);
        ::DPtoLP(hdc, rgptConvert, 2);
        if (dwFlags & XFORMCOORDS_SIZE)
        {
            pPtlHimetric->x = rgptConvert[1].x - rgptConvert[0].x;
            pPtlHimetric->y = rgptConvert[0].y - rgptConvert[1].y;
        }
        else if (dwFlags & XFORMCOORDS_POSITION)
        {
            pPtlHimetric->x = rgptConvert[1].x;
            pPtlHimetric->y = rgptConvert[1].y;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    ::ReleaseDC(m_hWndParent, hdc);

    return hr;
}


HRESULT STDMETHODCALLTYPE CControlSite::TranslateAccelerator(/* [in] */ MSG __RPC_FAR *pMsg, /* [in] */ DWORD grfModifiers)
{
	ATLTRACE("Not implemnted");
    return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CControlSite::OnFocus(/* [in] */ BOOL fGotFocus)
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CControlSite::ShowPropertyFrame(void)
{
	ATLTRACE("Not implemnted");
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
// IBindStatusCallback implementation

HRESULT STDMETHODCALLTYPE CControlSite::OnStartBinding(DWORD dwReserved, 
                                                       IBinding __RPC_FAR *pib)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CControlSite::GetPriority(LONG __RPC_FAR *pnPriority)
{
    return S_OK;
}
    
HRESULT STDMETHODCALLTYPE CControlSite::OnLowResource(DWORD reserved)
{
    return S_OK;
}
    
HRESULT STDMETHODCALLTYPE CControlSite::OnProgress(ULONG ulProgress, 
                                        ULONG ulProgressMax, 
                                        ULONG ulStatusCode, 
                                        LPCWSTR szStatusText)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CControlSite::OnStopBinding(HRESULT hresult, LPCWSTR szError)
{
    m_bBindingInProgress = FALSE;
    m_hrBindResult = hresult;
    return S_OK;
}
    
HRESULT STDMETHODCALLTYPE CControlSite::GetBindInfo(DWORD __RPC_FAR *pgrfBINDF, 
                                                    BINDINFO __RPC_FAR *pbindInfo)
{
    *pgrfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE |
                 BINDF_GETNEWESTVERSION | BINDF_NOWRITECACHE;
    pbindInfo->cbSize = sizeof(BINDINFO);
    pbindInfo->szExtraInfo = NULL;
    memset(&pbindInfo->stgmedData, 0, sizeof(STGMEDIUM));
    pbindInfo->grfBindInfoF = 0;
    pbindInfo->dwBindVerb = 0;
    pbindInfo->szCustomVerb = NULL;
    return S_OK;
}
    
HRESULT STDMETHODCALLTYPE CControlSite::OnDataAvailable(DWORD grfBSCF, 
                                                        DWORD dwSize, 
                                                        FORMATETC __RPC_FAR *pformatetc, 
                                                        STGMEDIUM __RPC_FAR *pstgmed)
{
	ATLTRACE("Not implemnted");
    return E_NOTIMPL;
}
  
HRESULT STDMETHODCALLTYPE CControlSite::OnObjectAvailable(REFIID riid, 
                                                          IUnknown __RPC_FAR *punk)
{
    return S_OK;
}

// IWindowForBindingUI
HRESULT STDMETHODCALLTYPE CControlSite::GetWindow(
    /* [in] */ REFGUID rguidReason,
    /* [out] */ HWND *phwnd)
{
    *phwnd = NULL;
    return S_OK;
}


