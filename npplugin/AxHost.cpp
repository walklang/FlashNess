#include "stdafx.h"
#include "AxHost.h"
#include "ControlSite.h"
#include <atlcom.h>
#include <atlutil.h>
#include <comutil.h>
#include <iostream>
#include "IPersistProperty.h"
#include "npobjproxy.h"
#include "PluginEventSink.h"
#include "PluginObj.h"
#include <sstream>
#include <string>
#include "variants.h"

CAxHost::CAxHost( void ) 
	: m_bInit(false)
	, m_pNPInstance(NULL)
	, m_hWindow(NULL)
	, m_bWindowLess(false)
	, m_Sink(NULL)
	, m_Site(NULL)
	, m_clsid(CLSID_NULL)
	, m_bValidClsID(false)
	, m_bEventDisp(false)
{

}

CAxHost::~CAxHost( void )
{
	Destroy();  
}

bool CAxHost::IsValid()
{
	if( !m_bValidClsID )
	{
		return false;
	}
	return true;
}

bool CAxHost::HasValidClsid()
{
	return m_bValidClsID;
}

bool CAxHost::AddDefaultClsid()
{
	if( HasValidClsid() ){
		return true;
	}
	std::wstring wValue = L"{5EC7C511-CD0F-42E6-830C-1BD9882F3458}";
	return SetClsID(wValue);
}

NPP CAxHost::ResetNPP( NPP npp )
{
	NPP ret = m_pNPInstance;
	m_pNPInstance = npp;
	if( m_Site )
	{
		m_Site->Detach();
	}
	if( m_Sink )
	{
		if( NULL == m_pNPInstance )
		{
			m_Sink->ReleaseObjEvent();
		}
		m_Sink->instance = m_pNPInstance;
	}
	return ret;
}

bool CAxHost::AddPersistBag( const NPVariantProxy& name, const NPVariantProxy& value, PropertyList* list )
{
	if( !list )
		return false;
	if( !NPVARIANT_IS_STRING(name) )
		return false;
	CComBSTR paramName(NPVARIANT_TO_STRING(name).UTF8Length, NPVARIANT_TO_STRING(name).UTF8Characters);

	if( NPVARIANT_IS_STRING(value) )
	{
		// Add named parameter to list
		CComVariant paramValue;
		NPVar2Variant( &value, &paramValue, m_pNPInstance );
		return list->AddOrReplaceNamedProperty(paramName, paramValue);
	}
	if( NPVARIANT_TO_OBJECT(value) )
	{
	}

	return false;	
}

HWND CAxHost::GetWindow()
{
	if( ::IsWindow(m_hWindow) )
	{
		return m_hWindow;
	}
	return NULL;
}

HRESULT CAxHost::GetControlUnknown( IUnknown **pObj )
{
	if( NULL == m_Site ) 
	{
		return E_FAIL;
	}
	return m_Site->GetControlUnknown(pObj);
}

bool CAxHost::VerifyClsID( const std::wstring& strClsID )
{
	HKEY RegEntry;
	if( ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Internet Explorer\\ActiveX Compatibility", 0, KEY_READ, &RegEntry) )
	{
		HKEY hSubKey;
		if( ERROR_SUCCESS == ::RegOpenKeyEx(RegEntry, strClsID.c_str(), 0, KEY_READ, &hSubKey) )
		{
			DWORD dwType = REG_DWORD;
			DWORD dwFlags = 0;
			DWORD dwBufSize = sizeof(dwFlags);
			if( ERROR_SUCCESS == ::RegQueryValueEx(hSubKey, L"Compatibility Flags", NULL, &dwType, (LPBYTE)&dwFlags, &dwBufSize) ) 
			{		
				const DWORD kKillBit = 0x00000400;
				if (dwFlags & kKillBit)
				{
					RegCloseKey(hSubKey);
					RegCloseKey(RegEntry);
					return false;
				}
			}
			RegCloseKey(hSubKey);
		}
		RegCloseKey(RegEntry);
	}
	return true;
}

bool CAxHost::VerifyClsID( const CLSID& clsid )
{
	LPOLESTR oleCLSID = NULL;
	HRESULT hr = StringFromCLSID(clsid, &oleCLSID);
	if( FAILED(hr) || !oleCLSID )
		return false;
	std::wstring strClsid = oleCLSID;
	CoTaskMemFree(oleCLSID);
	return VerifyClsID(strClsid);
}

CLSID CAxHost::ParseClsidFromString(std::wstring& str )
{
	CLSID clsid = CLSID_NULL;

	std::wstring strClsid = str;
	if( SUCCEEDED(CLSIDFromString(strClsid.c_str(), &clsid)) )
	{
		return clsid;
	}

	std::wstring::size_type pos = strClsid.find(L":");
	if( pos != std::wstring::npos )
	{
		strClsid.erase(0, pos);
		strClsid = L"{" + strClsid + L"}";
		if( SUCCEEDED(CLSIDFromString(strClsid.c_str(), &clsid)) )
		{
			str = strClsid;
			return clsid;
		}
	}
	return CLSID_NULL;
}

bool CAxHost::GetClsID(std::string &clsid)
{
	LPOLESTR oleClsid = NULL;
	HRESULT hr = StringFromCLSID(m_clsid, &oleClsid);
	if( FAILED(hr) || !oleClsid )
	{
		return false;
	}
	std::wstring strClsid = oleClsid;
	CoTaskMemFree(oleClsid);
	clsid = CT2AEX<>(strClsid.c_str());
	return true;
}

bool CAxHost::SetClsID( const CLSID& clsid )
{
	if( CLSID_NULL == clsid ) 
	{
		m_bValidClsID = false;
		return false;
	}

	m_clsid = clsid;
	m_bValidClsID = true;
	return true;
}

bool CAxHost::SetClsID( const std::wstring& clsid )
{
	std::wstring strClsid = clsid;
	CLSID vclsid = ParseClsidFromString(strClsid);
	if( CLSID_NULL == vclsid )
	{
		return false;
	}

	bool bVerify = VerifyClsID(strClsid); 
	if( !bVerify )
	{
		return false;
	}

	return SetClsID(vclsid);
}

bool CAxHost::SetClsIDFromProgID( const std::wstring& progid )
{
	CLSID clsid = CLSID_NULL;
	HRESULT hr = CLSIDFromProgID(progid.c_str(), &clsid);
	if( FAILED(hr) || ::IsEqualCLSID(clsid, CLSID_NULL) )
		return false;

	if( VerifyClsID(clsid) )
	{
		return SetClsID(clsid);
	}
	return false; 	
}

bool CAxHost::MatchURL2TrustedLocations( const std::wstring& matchUrl )
{
	unsigned int numTrustedLocations = 0;
	const TCHAR *TrustedLocations[] = {NULL};

	return true;

	if( !numTrustedLocations )
	{
		return true;
	}

	CUrl url;
	if( !url.CrackUrl(matchUrl.c_str(), ATL_URL_DECODE) ) 
	{
		return false;
	}

	std::wstring strHostName = url.GetHostName();
	if( (url.GetScheme() == ATL_URL_SCHEME_FILE)
		|| strHostName == L"localhost" )
	{
		return true;
	}

	
	for (unsigned int i = 0; i < numTrustedLocations; ++i ) 
	{	
		if (TrustedLocations[i][0] == '*' ) 
		{
			// sub domains are trusted
			unsigned int len = wcsnlen(TrustedLocations[i], 1024);
			bool match = 0;

			if (url.GetHostNameLength() < len) {
				// can't be a match
				continue;
			}
			--len; // skip the '*'

			match = wcsncmp(url.GetHostName() + (url.GetHostNameLength() - len), TrustedLocations[i] + 1, len) == 0 ? true : false;
			if (match) 
			{
				return true;
			}
		}
		else if( 0 == wcsncmp((url.GetHostName()), TrustedLocations[i], url.GetHostNameLength()) )
		{
			return true;
		}
	}

	return false;	
}

IPluginObjBase* CAxHost::CreateScriptableObject()
{
	if( NULL == m_pNPInstance )
	{
		return NULL;
	}
	CPluginObj *obj = CPluginObj::CreateHostObject(m_pNPInstance, this);
	if( NULL == m_Site )
	{
		return obj;
	}
	return obj;
}

void CAxHost::DestroyScriptableObject( NPObject* obj )
{
	CPluginObj::DestroyHostObject(obj);	
}

LRESULT CAxHost::MessageHandler( UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled )
{
	LRESULT	lRes = 0;
	bHandled = false;

	switch(uMsg)
	{
	case WM_SETFOCUS:
		break;
	case WM_KILLFOCUS:
		break;
	case WM_SIZE:
		if( NULL != m_Site )
		{
			m_Site->OnDefWindowMessage(uMsg, wParam, lParam, &lRes);
		}
		break;
	case WM_DESTROY:
		break;
	default:
		break;
	}
	return lRes;
}

uint16_t CAxHost::DoEvent(NPEvent* npEvent)
{
	if( NULL == npEvent )
	{
		return 0;
	}

	if( m_Site )
	{
		HRESULT result;
		return (uint16_t)(m_Site->OnDefWindowMessage(npEvent->event, npEvent->wParam, npEvent->lParam, &result));
	}

	return 0;
}

void CAxHost::DispatchEvent()
{
	if( m_bEventDisp )
	{
		return;
	}

	unsigned long length = m_Events.GetSize();
	for( unsigned long j = 0; j < length; j++ ) 
	{
		if( !AddEventHandler(m_Events.GetNameOf(j), m_Events.GetValueOf(j)->bstrVal) ) 
		{	

		}
	}
	m_bEventDisp = true;
}

bool CAxHost::DispidFromEventName( const std::wstring& name, DISPID& id )
{
	HRESULT result;
	DISPID dispid = 0;
	LPOLESTR oleName = const_cast<WCHAR*>(name.c_str());

	if( !CreateEventSink() )
	{
		return false;
	}

	if( !m_Sink || !m_Sink->m_spEventSinkTypeInfo )
	{
		return false;
	}

	result = m_Sink->m_spEventSinkTypeInfo->GetIDsOfNames(&oleName, 1, &dispid);
	if( FAILED(result) )
	{
		return false;
	}
	id = dispid;
	return true;
}

bool CAxHost::AddPersistPropertyBag( const std::wstring name, const std::wstring value )
{
	std::string strTemp;

	std::wstring name_temp = L"";
	name_temp.resize(name.length());
	std::transform(name.begin(), name.end(), name_temp.begin(), ::tolower);
	strTemp = CT2AEX<>(name_temp.c_str());
	CComBSTR bstrKey(strTemp.c_str());

	strTemp = CT2AEX<>(value.c_str());
	CComBSTR bstrValue(strTemp.c_str());

	CComVariant v(bstrValue);
	return m_Props.AddOrReplaceNamedProperty(bstrKey, v);
}

bool CAxHost::AddPersistPropertyBag( const BSTR bstrName, const VARIANT &vValue )
{
	return m_Props.AddOrReplaceNamedProperty(bstrName, vValue);
}

bool CAxHost::AddPersistEventBag( const std::wstring name, const std::wstring value )
{
	std::string strTemp;

	std::wstring name_temp;
	name_temp.resize(name.length());
	std::transform(name.begin(), name.end(), name_temp.begin(), ::tolower);
	strTemp = CT2AEX<>(name_temp.c_str());
	CComBSTR bstrKey(strTemp.c_str());

	strTemp = CT2AEX<>(value.c_str());
	CComBSTR bstrValue(strTemp.c_str());

	CComVariant v(bstrValue);
	return m_Props.AddOrReplaceNamedProperty(bstrKey, v);
}

bool CAxHost::AddPersistEventBag( const std::wstring name, NPObject *npobj )
{
	if( !npobj )
	{
		return false;
	}
	NPObject* pEventObj = NPN_RetainObject(npobj);
	if( !pEventObj )
	{
		return false;
	}
	// Add named parameter to list
	return AddEventHandler(name, pEventObj);		
}

bool CAxHost::AddPersistEventBag( const std::wstring name, const NPVariant *vValue )
{
	if( NPVARIANT_IS_OBJECT(*vValue) )
	{
		return AddPersistEventBag(name, NPVARIANT_TO_OBJECT(*vValue));
	}
	if( NPVARIANT_IS_STRING(*vValue) )
	{
		CComBSTR paramValue(NPVARIANT_TO_STRING(*vValue).UTF8Length, NPVARIANT_TO_STRING(*vValue).UTF8Characters);
		std::wstring strValue(paramValue);
		return AddPersistEventBag(name, strValue);
	}
	return false;
}

bool CAxHost::AddEventHandler( const std::wstring name, const std::wstring handler )
{
	DISPID id;
	if( !DispidFromEventName(name, id) )
	{
		return false;
	}

	m_Sink->events[id] = const_cast<WCHAR*>(handler.c_str());
	return true;
}

bool CAxHost::AddEventHandler( const std::wstring name, NPObject* objHandler )
{
	DISPID id;
	if( !DispidFromEventName(name, id) )
	{
		return false;
	}

	m_Sink->eventsObj[id] = objHandler;
	return true;	
}

void CAxHost::SetRectSize( LPSIZEL size )
{
	if( NULL == m_pNPInstance )
	{
		return;
	}
	NPObjectProxy obj;
	NPN_GetValue(m_pNPInstance, NPNVPluginElementNPObject, &obj);

	static NPIdentifier style = NPN_GetStringIdentifier("style");
	static NPIdentifier height = NPN_GetStringIdentifier("height");
	static NPIdentifier width = NPN_GetStringIdentifier("width");
	
	std::stringstream  oss;
	NPVariant sHeight;
	oss.str("");
	oss << size->cy;
	oss << "dpx";
	STRINGZ_TO_NPVARIANT(oss.str().c_str(), sHeight);

	NPVariant sWidth;
	oss.str("");
	oss << size->cx;
	oss << "dpx";
	STRINGZ_TO_NPVARIANT(oss.str().c_str(), sWidth);

	NPVariantProxy styleValue;

	NPN_GetProperty(m_pNPInstance, obj, style, &styleValue);
	NPObject *styleObject = NPVARIANT_TO_OBJECT(styleValue);

	NPN_SetProperty(m_pNPInstance, styleObject, height, &sHeight);
	NPN_SetProperty(m_pNPInstance, styleObject, width, &sWidth);
}

void CAxHost::UpdateRectSize( LPRECT origRect )
{
	if( m_bWindowLess )
		return;
	if( NULL == m_Site )
	{
		return;
	}
	
	SIZEL szCtrl;
	if( !m_Site->IsVisibleAtRuntime() )
	{
		szCtrl.cx = 0;
		szCtrl.cy = 0;
	}
	else
	{
		if( FAILED(m_Site->GetControlSize(&szCtrl)) )
			return;
	}

	SIZEL szIn;
	szIn.cx = origRect->right - origRect->left;
	szIn.cy = origRect->bottom - origRect->top;
	if( szCtrl.cx != szIn.cx || szCtrl.cy != szIn.cy )
	{
		SetRectSize(&szCtrl);
	}
}

void CAxHost::UpdateRect( RECT rcPos )
{
	if( NULL == m_Site || m_bWindowLess )
	{
		return;
	}
	m_rcLastRect = rcPos;

	HRESULT hr = 0;
	HWND hWnd = GetWindow();
	if( NULL != hWnd )
	{
		if( NULL == m_Site->GetParentWindow() )
		{
			hr = m_Site->Attach(hWnd, rcPos, NULL);
			if( FAILED(hr) )
			{
				SIZEL zero = {0, 0};
				SetRectSize(&zero);
			}
		}
		if( m_Site->CheckAndResetNeedUpdateContainerSize() )
		{
			UpdateRectSize(&rcPos);
		}
		else
		{
			m_Site->SetPosition(rcPos);
		}
		::SetWindowLong(hWnd, GWL_STYLE, ::GetWindowLong(hWnd, GWL_STYLE) | WS_CLIPCHILDREN);
	}
}

bool CAxHost::CreateControlSite()
{
	if( !IsValid() )
		return false;

	if( m_Site )
		return true;

	// Create control site.
	CComObject<CControlSite>::CreateInstance(&m_Site);
	if( NULL == m_Site )
	{
		return false;
	}
	m_Site->AddRef();
	m_Site->m_bSupportWindowlessActivation = false;
	m_Site->m_bSafeForScriptingObjectsOnly = true;

	// Create the object.
	HRESULT hr = m_Site->Create(m_clsid, *GetProps(), m_strCodeBaseUrl.c_str());
	if( FAILED(hr) )
	{
		return false;
	}
	return true;
}

bool CAxHost::CreateEventSink()
{  
	if( m_Sink )
		return true;

	if( NULL == m_pNPInstance )
	{
		return false;
	}

	IUnknown *iUnKnown = NULL;
	HRESULT hr = GetControlUnknown(&iUnKnown);
	if( FAILED(hr) )
	{
		return false;
	}

	// Create event sink.
	CComObject<CPluginEventSink>::CreateInstance(&m_Sink);
	if( NULL == m_Sink )
	{
		return false;
	}

	m_Sink->AddRef();
	m_Sink->instance = m_pNPInstance;
	hr = m_Sink->SubscribeToEvents(iUnKnown);
	iUnKnown->Release();

	if( FAILED(hr) )
	{
		m_Sink->UnsubscribeFromEvents();
		m_Sink->Release();
		m_Sink = NULL;
		return false;
	}

	return true;
}

bool CAxHost::Create()
{
	bool bRet = CreateControlSite();
	if( !bRet )
	{
		return bRet;
	}

	if( m_Events.GetSize() > 0 )
	{
		bRet = CreateEventSink();
		if( bRet )
		{
			DispatchEvent();
		}
	}

	m_bInit = bRet;

	return bRet;	
}

void CAxHost::SetPos( RECT rc )
{
	UpdateRect(rc);
}

void CAxHost::ResetPos()
{
	UpdateRect(m_rcLastRect);
}

void CAxHost::Destroy()
{
	if( m_Sink )
	{
		m_Sink->UnsubscribeFromEvents();
		m_Sink->Release();
		m_Sink = NULL;
	}

	if( m_Site )
	{
		m_Site->Detach();
		m_Site->Release();
		m_Site = NULL;
	}

	if( 0 != m_Props.GetSize() )
	{
		m_Props.Clear();
	}
	if( 0!= m_Events.GetSize() )
	{
		m_Events.Clear();
	}

	m_bInit = false;
	m_bEventDisp = false;
}

bool CAxHost::ParsePersistPropertyBag( const std::wstring key, const std::wstring value )
{
	if( 0 == key.length() )
	{
		return false;
	}
	else if( 0 == key.compare(PARAM_ID)  )
	{
	}
	else if( 0 == key.compare(PARAM_NAME) )
	{
	}
	else if( 0 == key.compare(PARAM_CLSID)
		  || 0 == key.compare(PARAM_CLASSID) )
	{
		return SetClsID(value);	   // {5EC7C511-CD0F-42E6-830C-1BD9882F3458}
	}
	else if( 0 == key.compare(PARAM_PROGID) )
	{
		return SetClsIDFromProgID(value);
	}
	else if( 0 == key.compare(PARAM_DYNAMIC) )
	{
	}
	else if( 0 == key.compare(PARAM_DEBUG) )
	{
	}
	else if( 0 == key.compare(PARAM_CODEBASE) )
	{
		std::wstring strCodeBase = value;
		if( MatchURL2TrustedLocations(strCodeBase) )
		{
			SetCodeBaseUrl(strCodeBase);
		}
		else
			std::cout << L"codeBaseUrl contains an untrusted location";
		return true;
	}
	else if( 0 == key.compare(PARAM_WINLESS) )
	{
		SetWindowLess(true);
		return true;
	}
	else 
	{
		if( IsEventPersistBag(key, value) )
		{
			return true;
		}
	}
	
	return false;
}

bool CAxHost::IsEventPersistBag( const std::wstring key, const std::wstring value )
{
	std::wstring strEvent = PARAM_EVENT;
	std::wstring::size_type length = strEvent.length();

	if( key.length() < length )
		return false;
	if( 0 == key.compare(0, length, strEvent, 0, length ) )
	{
		std::wstring strKey = key;
		std::wstring strEventKey = strKey.erase(0, length);
		return AddPersistEventBag(strEventKey, value);
	}
	return false;
}

