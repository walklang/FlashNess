#pragma once
#include <string>
#include <map>
#include "npapi.h"
#include <atlbase.h>
#include <atlwin.h>
#include <atlcom.h>
#include "pluginbase.h"
#include "PropertyList.h"

namespace ATL {
	template <typename T>
	class CComObject;
}

class CPluginEventSink;
class CControlSite;
class IPluginObjBase;
class NPVariantProxy;

class CAxHost
{
public:
	CAxHost(void);
	~CAxHost(void);

public:
	bool CreateControlSite();
	bool CreateEventSink();
	bool Create();
	void Destroy();															///< 销毁
	void SetPos( RECT rc );													///< 设置位置		
	void ResetPos();

public:
	LRESULT MessageHandler( UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled );
	uint16_t DoEvent(NPEvent* npEvent);
	void DispatchEvent();
	bool AddEventHandler(const std::wstring name, const std::wstring handler);
	bool AddEventHandler(const std::wstring name, NPObject* objHandler);

public:
	NPP ResetNPP(NPP npp);
	HRESULT GetControlUnknown(IUnknown **pObj);
	IPluginObjBase* CreateScriptableObject();
	void DestroyScriptableObject(NPObject* obj);

public:
	PropertyList* GetProps() { 
		return &m_Props; 
	}
	PropertyList* GetEvents() { 
		return &m_Events; 
	}
	bool AddPersistPropertyBag(const std::wstring name, const std::wstring value);
	bool AddPersistPropertyBag(const BSTR bstrName, const VARIANT &vValue);
	bool AddPersistEventBag(const std::wstring name, const std::wstring value);
	bool AddPersistEventBag(const std::wstring name, NPObject *npobj);
	bool AddPersistEventBag(const std::wstring name, const NPVariant *vValue);

public:
	bool IsInitialized() {
		return m_bInit; 
	}							
	bool IsValid();
	bool HasValidClsid();
	bool AddDefaultClsid();
	HWND GetWindow();
	void SetWindow(HWND hWnd) { 
		m_hWindow = hWnd; 
	}
	void SetWindowLess(bool bWindowLess) { 
		m_bWindowLess = bWindowLess; 
	}
	bool SetCodeBaseUrl(const std::wstring& strCodeBaseUrl) { 
		m_strCodeBaseUrl = strCodeBaseUrl; 
		return true; 
	}
	bool SetClsID(const CLSID& clsid);
	bool SetClsID(const std::wstring& clsid);
	bool SetClsIDFromProgID(const std::wstring& progid);
	bool GetClsID(std::string &clsid);
	CLSID GetClsID() { return m_clsid; }
	CLSID ParseClsidFromString(std::wstring& str);
	bool VerifyClsID(const std::wstring& strClsID);
	bool VerifyClsID(const CLSID& clsid);
	bool ParsePersistPropertyBag(const std::wstring key, const std::wstring value);
	bool IsEventPersistBag(const std::wstring key, const std::wstring value);
	bool MatchURL2TrustedLocations(const std::wstring& matchUrl);
	
private:
	void UpdateRect(RECT rcPos);
	void SetRectSize(LPSIZEL size);
	void UpdateRectSize(LPRECT origRect);

private:
	bool AddPersistBag(const NPVariantProxy& name, const NPVariantProxy& value, PropertyList* list);
	bool AddPersistBag(const std::wstring& name, const std::wstring& value, PropertyList* list);
	bool DispidFromEventName(const std::wstring& name, DISPID& id);

private:
	bool				m_bInit;				///< 初始化标记
	std::wstring        m_localURL;
	CLSID				m_clsid;
	bool				m_bValidClsID;
	std::wstring		m_strCodeBaseUrl;
	RECT				m_rcLastRect;
	HWND				m_hWindow;
	bool				m_bWindowLess;
	bool				m_bEventDisp;

private:
	NPP					m_pNPInstance;
	ATL::CComObject<CPluginEventSink> *m_Sink;
	ATL::CComObject<CControlSite> *m_Site;
	PropertyList		m_Props;
	PropertyList		m_Events;

	typedef std::map<const std::wstring, NPObject*> EVENTNAMEDOBJ;
	EVENTNAMEDOBJ		m_EventsNamedObj;
};

