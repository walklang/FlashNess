#include "stdafx.h"
#include "PluginObj.h"
#include "AxHost.h"
#include "IPersistProperty.h"
#include <OleAuto.h>
#include "plugin.h"
#include "variants.h"

DEFINE_NPOBJECT_CLASS_WITH_BASE(CPluginObj);

CPluginObj::CPluginObj( NPP npp ) 
	: IPluginObjBase(npp)
	, m_host(NULL)
	, m_bInvalid(false)
{
	ConstructMethod();
	ConstructProperty();	
}

CPluginObj::~CPluginObj()
{
	FinalRelease();	
}

CPluginObj* CPluginObj::CreateHostObject( NPP npp, CAxHost* pHost )
{
	CPluginObj* new_obj = static_cast<CPluginObj*>(CPluginObj::CreateObject(npp));
	
	IUnknown *iUnk;
	if( SUCCEEDED(pHost->GetControlUnknown(&iUnk)) )
	{
		new_obj->SetControl(iUnk);
		new_obj->m_host = pHost;
		iUnk->Release();
	}
	return new_obj;
}

void CPluginObj::DestroyHostObject( NPObject* obj )
{
	if( obj )
	{
		CPluginObj* new_obj = static_cast<CPluginObj*>(obj);
		if( new_obj )
		{
			new_obj->m_host = nullptr;
		}
		CPluginObj::DestroyObject(obj);
	}
}

HRESULT CPluginObj::GetControl( IUnknown **obj )
{
	if (m_disp) 
	{ 
		*obj = m_disp.p;
		(*obj)->AddRef();
		return S_OK;
	}
	return E_NOT_SET;
}

CPlugin* CPluginObj::GetPluginPtr()
{
	static CPlugin* plugin = NULL;
	if( NULL == plugin )
	{
		plugin = (CPlugin*)(m_Npp->pdata);
	}
	return plugin;
}

bool CPluginObj::IsMethodBag( NPIdentifier& identifier )
{
	MAP_IDENTIFIER::iterator pit = m_methodBag.find(identifier);
	if( pit != m_methodBag.end() )
	{
		return true;
	}
	return false;	
}

void CPluginObj::MakeMethodIdentifier( NPIdentifier& identifier, const std::string& name )
{
	identifier = NPN_GetStringIdentifier(name.c_str());
	m_methodBag.insert(MAP_IDENTIFIER::value_type(identifier, name));
}

bool CPluginObj::IsEventBag( NPIdentifier& identifier, std::wstring &evnet )
{
	std::wstring strEvent = PARAM_EVENT;
	std::wstring::size_type length = strEvent.length();

	std::wstring strKey;
	std::string strIdentifier = NPN_UTF8FromIdentifier(identifier);
	strKey = CA2TEX<>(strIdentifier.c_str());
	std::transform(strKey.begin(), strKey.end(), strKey.begin(), ::tolower);

	if( strKey.length() < length )
		return false;

	if( 0 == strKey.compare(0, length, strEvent, 0, length ) )
	{
		evnet = strKey.erase(0, length);
		return true;
	}
	return false;
}

bool CPluginObj::IsPropertyBag( NPIdentifier& identifier )
{
	MAP_IDENTIFIER::iterator pit = m_propertyBag.find(identifier);
	if( pit != m_propertyBag.end() )
	{
		return true;
	}

	std::wstring strEvent;
	if( IsEventBag(identifier, strEvent) )
	{
		return true;
	}

	return false;
}

void CPluginObj::MakePropertyIdentifier( NPIdentifier& identifier, const std::string& name)
{
	identifier = NPN_GetStringIdentifier(name.c_str());
	m_propertyBag.insert(MAP_IDENTIFIER::value_type(identifier, name));
}

void CPluginObj::ConstructMethod()
{
	// attachEvent
	MakeMethodIdentifier(s_attachEvent, "attachEvent");
}

void CPluginObj::ConstructProperty()
{
	MakePropertyIdentifier(s_clsid,		"clsid");
	MakePropertyIdentifier(s_classid,	"classid");
	MakePropertyIdentifier(s_objectID,	"object");
	MakePropertyIdentifier(s_readyState, "readyState");
	MakePropertyIdentifier(s_instanceID, "__npp_instance__");
}

void CPluginObj::FinalRelease()
{
	m_host = nullptr;
}

bool CPluginObj::GetDescNum(UINT &ctInfo, uint32_t &nCount)
{
	if( !m_disp )
	{
		return false;
	}

	if( FAILED(m_disp->GetTypeInfoCount(&ctInfo)) )
	{
		return false;
	}

	struct CDescTool
	{
		static bool AddDescNum(ITypeInfoPtr spInfo, TYPEATTR *attr, uint32_t &nCount)
		{
			if( !spInfo || !attr )
			{
				return false;
			}
			nCount += attr->cFuncs;
			nCount += attr->cVars;

			size_t nInterface = attr->cImplTypes;
			for( size_t j = 0; j < nInterface; ++j )
			{
				HREFTYPE refType;
				ITypeInfoPtr spBaseInfo;
				TYPEATTR *baseAttr = NULL;
				if( FAILED(spInfo->GetRefTypeOfImplType(j, &refType)) )
				{
					continue;
				}
				if( FAILED(spInfo->GetRefTypeInfo(refType, &spBaseInfo)) )
				{
					continue;
				}
				if( FAILED(spBaseInfo->GetTypeAttr(&baseAttr)) )
				{
					continue;
				}
				AddDescNum(spBaseInfo, baseAttr, nCount);
				spBaseInfo->ReleaseTypeAttr(baseAttr);
			}
			return true;
		}
	};

	for( size_t i = 0; i < ctInfo; ++i )
	{
		ITypeInfoPtr spInfo;
		m_disp->GetTypeInfo(i, LOCALE_SYSTEM_DEFAULT, &spInfo);
		if( !spInfo )
		{
			continue;
		}
		TYPEATTR *attr = NULL;
		if( FAILED(spInfo->GetTypeAttr(&attr)) )
		{
			continue;
		}

		CDescTool::AddDescNum(spInfo, attr, nCount);		
		spInfo->ReleaseTypeAttr(attr);
	}
	return true;
}

bool CPluginObj::GetDescName( ITypeInfoPtr spInfo, UINT index, std::string& name, bool bFunc /*= true*/ )
{
	if( !spInfo )
	{
		return false;
	}

	struct CDescTool
	{
	private:
		static bool GetName(ITypeInfoPtr spInfo, MEMBERID memid, std::string& name)
		{
			if( !spInfo )
			{
				return false;
			}
			CComBSTR bstr_Name, bstr_Doc;
			bstr_Name.Empty();
			bstr_Doc.Empty();
			HRESULT hr =  spInfo->GetDocumentation(memid, &bstr_Name, &bstr_Doc, NULL, NULL);
			if( SUCCEEDED(hr) )
			{
				std::wstring wName = bstr_Name;
				name = CT2AEX<>(wName.c_str());
				return true;
			}
			return false;
		}
	public:
		static bool GetFuncName(ITypeInfoPtr spInfo, UINT index, std::string& name)
		{
			if( !spInfo )
			{
				return false;
			}

			bool bFind = false;
			FUNCDESC *fDesc = NULL;
			HRESULT hr = spInfo->GetFuncDesc(index, &fDesc);
			if( SUCCEEDED(hr) && fDesc )
			{
				bFind = GetName(spInfo, fDesc->memid, name);			
			}
			spInfo->ReleaseFuncDesc(fDesc);
			return bFind;	
		}
		static bool GetVarName(ITypeInfoPtr spInfo, UINT index, std::string& name)
		{
			if( !spInfo )
			{
				return false;
			}

			bool bFind = false;
			VARDESC *vDesc = NULL;
			HRESULT hr = spInfo->GetVarDesc(index, &vDesc);
			if( SUCCEEDED(hr) && vDesc )
			{
				bFind = CDescTool::GetName(spInfo, vDesc->memid, name);
			}
			spInfo->ReleaseVarDesc(vDesc);
			return bFind;
		}
	};

	if( bFunc )
	{
		return CDescTool::GetFuncName(spInfo, index, name);
	}
	else
	{
		return CDescTool::GetVarName(spInfo, index, name);
	}
}

int CPluginObj::GetDescName( ITypeInfoPtr spInfo, TYPEATTR *attr, NPIdentifier* vIdentifiers )
{
	if( !spInfo )
	{
		return 0;
	}

	int nAdd = 0;

	size_t nFuncs = attr->cFuncs;
	for( size_t j = 0; j < nFuncs; ++j )
	{
		std::string strName;
		if( GetDescName(spInfo, j, strName) )
		{
			vIdentifiers[nAdd++] = NPN_GetStringIdentifier(strName.c_str());
		}	
	}

	size_t nVars = attr->cVars;
	for( size_t j = 0; j < nVars; ++j )
	{
		std::string strName;
		if( GetDescName(spInfo, j, strName, false) )
		{
			vIdentifiers[nAdd++] = NPN_GetStringIdentifier(strName.c_str());
		}	
	}

	size_t nInterface = attr->cImplTypes;
	for( size_t i = 0; i < nInterface; ++i )
	{
		HREFTYPE refType;
		ITypeInfoPtr spBaseInfo;
		TYPEATTR *baseAttr = NULL;
		if( FAILED(spInfo->GetRefTypeOfImplType(0, &refType)) )
		{
			continue;
		}
		if( FAILED(spInfo->GetRefTypeInfo(refType, &spBaseInfo)) )
		{
			continue;
		}
		if( FAILED(spBaseInfo->GetTypeAttr(&baseAttr)) )
		{
			continue;
		}

		nAdd += GetDescName(spBaseInfo, baseAttr, vIdentifiers+nAdd);
		
		spBaseInfo->ReleaseTypeAttr(baseAttr);
	}

	return nAdd;
}



bool CPluginObj::FindDesc( ITypeInfoPtr spInfo, TYPEATTR *attr, DISPID id, unsigned int invKind )
{
	bool bFind = false;

	if( !spInfo || !attr || -1 == id )
	{
		return false;
	}

	if( invKind )
	{
		size_t nCount = attr->cFuncs;
		for( size_t i = 0; i < nCount; ++i )
		{
			FUNCDESC *fDesc = NULL;
			HRESULT hr = spInfo->GetFuncDesc(i, &fDesc);
			if( SUCCEEDED(hr) && fDesc )
			{
				if( id == fDesc->memid )
				{
					if( invKind & fDesc->invkind )
					{
						bFind = true;
					}
				}
			}

			std::string strName;
			GetDescName(spInfo, i, strName);

			spInfo->ReleaseFuncDesc(fDesc);

			if( bFind )
			{
				return true;
			}
		}
	}
	
	if( !bFind && ( invKind & ~INVOKE_FUNC ) )
	{	
		size_t nCount = attr->cVars;
		for( size_t i = 0; i < nCount; ++i )
		{
			VARDESC *vDesc = NULL;
			HRESULT hr = spInfo->GetVarDesc(i, &vDesc);
			if( SUCCEEDED(hr) && vDesc )
			{
				if( id == vDesc->memid )
				{				
					bFind = true;
				}
			}

			std::string strName;
			GetDescName(spInfo, i, strName, false);

			spInfo->ReleaseVarDesc(vDesc);
			if( bFind )
			{
				return bFind;
			}
		}
	}

	if( !bFind )
	{
		size_t nCount = attr->cImplTypes;
		for( size_t i = 0; i < nCount; ++i )
		{
			HREFTYPE refType;
			ITypeInfoPtr spBaseInfo;
			TYPEATTR *baseAttr = NULL;
			if( FAILED(spInfo->GetRefTypeOfImplType(0, &refType)) )
			{
				continue;
			}
			if( FAILED(spInfo->GetRefTypeInfo(refType, &spBaseInfo)) )
			{
				continue;
			}
			if( FAILED(spBaseInfo->GetTypeAttr(&baseAttr)) )
			{
				continue;
			}

			bFind = FindDesc(spBaseInfo, baseAttr, id, invKind);
			spBaseInfo->ReleaseTypeAttr(baseAttr);
			if( bFind ) 
			{
				return bFind;
			}
		}
	}
	
	return bFind;
}
 
bool CPluginObj::CheckInvokeKind( DISPID id, unsigned int invKind /*= INVOKE_FUNC*/ )
{
	bool bFind = false;
	if( -1 == id || !m_disp || !invKind  )
	{
		return false;
	}
	UINT ctInfo = 0;
	uint32_t nCount	= 0;
	if( !GetDescNum(ctInfo, nCount) )
	{
		return false;
	}
	
	for( size_t i = 0; i < ctInfo; ++i )
	{
		ITypeInfoPtr spInfo;
		m_disp->GetTypeInfo(i, LOCALE_SYSTEM_DEFAULT, &spInfo);
		if( !spInfo )
		{
			continue;
		}

		TYPEATTR *attr = NULL;
		if( FAILED(spInfo->GetTypeAttr(&attr)) )
		{
			continue;
		}

		bFind = FindDesc(spInfo, attr, id, invKind);
		spInfo->ReleaseTypeAttr(attr);
		if( bFind )
		{
			return true;
		}
	}
	return bFind;
}

bool CPluginObj::DispidFromName( __in NPIdentifier name, __out DISPID& id, unsigned int invKind /*= INVOKE_FUNC*/ )
{
	if( !name || !invKind || !m_disp )
	{
		return false;
	}

	if( !NPN_IdentifierIsString(name) )
	{
		return false;
	}

	std::wstring wName;
	std::string strName = NPN_UTF8FromIdentifier(name);
	wName = CA2TEX<>(strName.c_str());
	std::transform(wName.begin(), wName.end(), wName.begin(), ::tolower);
	
	LPOLESTR oleName = const_cast<WCHAR*>(wName.c_str());
	HRESULT hr = m_disp->GetIDsOfNames(IID_NULL, &oleName, 1, 0, &id);
	if( SUCCEEDED(hr) )
	{
		return CheckInvokeKind(id, invKind);
	}
	return false;
}

bool CPluginObj::DispidFromMethodName( __in NPIdentifier name, __out DISPID& id, unsigned int& invKind)
{
	if( !name || !m_disp )
	{
		return false;
	}

	if( !NPN_IdentifierIsString(name) )
	{
		return false;
	}

	std::wstring wName, wstrName;
	std::string strName = NPN_UTF8FromIdentifier(name);
	wName = CA2TEX<>(strName.c_str());
	std::transform(wName.begin(), wName.end(), wName.begin(), ::tolower);

	// 解析名称
	static std::wstring strGet = L"get_";
	static std::wstring strPut = L"put_";
	if( 0 == wName.find(strGet) )
	{
		wstrName = wName.substr(strGet.length());
		invKind = INVOKE_PROPERTYGET;
	}
	else if( 0 == wName.find(strPut) )
	{
		wstrName = wName.substr(strPut.length());
		invKind = INVOKE_PROPERTYPUT;
	}
	else
		return false;

	LPOLESTR oleName = const_cast<WCHAR*>(wstrName.c_str());
	HRESULT hr = m_disp->GetIDsOfNames(IID_NULL, &oleName, 1, 0, &id);
	if( SUCCEEDED(hr) )
	{
		return CheckInvokeKind(id, INVOKE_PROPERTYGET | INVOKE_PROPERTYPUT);
	}
	return false;	
}

bool CPluginObj::OnAttachEvent( const NPVariant *args, uint32_t argCount, NPVariant *result )
{
	NULL_TO_NPVARIANT(*result);
	if( args != NULL && 2 == argCount )
	{
		// 取参数
		NPVariant nppName(args[0]);
		NPVariant nppValue(args[1]);

		if( !NPVARIANT_IS_STRING(nppName) )
		{
			return false;
		}

		CComBSTR paramEvent(NPVARIANT_TO_STRING(nppName).UTF8Length, NPVARIANT_TO_STRING(nppName).UTF8Characters);
		std::wstring strEvent(paramEvent);

		return m_host->AddPersistEventBag(strEvent, &nppValue);
	}

	if( args != NULL && 1== argCount )
	{
		// 取参数
		NPVariant nppName(args[0]);
		if( !NPVARIANT_IS_STRING(nppName) )
		{
			return false;
		}

		CComBSTR paramEvent(NPVARIANT_TO_STRING(nppName).UTF8Length, NPVARIANT_TO_STRING(nppName).UTF8Characters);
		std::wstring strEvent(paramEvent);
		return m_host->AddPersistEventBag(strEvent, strEvent);
	}

	return false;
}

bool CPluginObj::OnMethod( NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result )
{
	NULL_TO_NPVARIANT(*result);

	if( !IsMethodBag(name) )
	{
		return false;
	}

	if( name == s_attachEvent )
	{
		return OnAttachEvent(args, argCount, result);
	}
	return false;
}

bool CPluginObj::OnGetProperty( NPIdentifier name, NPVariant *result )
{
	if( !IsPropertyBag(name) )
	{
		return false;
	}

	if( name == s_classid )
	{
		return OnGetClassid(result);
	}
	if( name == s_clsid )
	{
		return OnGetClsid(result);	
	}
	if( name == s_instanceID )
	{
		INT32_TO_NPVARIANT((long)m_Npp, *result);
		return true;
	}
	if( name == s_readyState )
	{
		INT32_TO_NPVARIANT(4, *result);
		return true;
	}
	if( name == s_objectID )
	{
		OBJECT_TO_NPVARIANT(this, *result);
		NPN_RetainObject(this);
		return true;
	}

	return false;
}

bool CPluginObj::OnGetClsid( NPVariant *result )
{
	std::string strClsid = "";
	if( m_host->IsInitialized() )
	{
		m_host->GetClsID(strClsid);
	}
	STRINGN_TO_NPVARIANT(strClsid.c_str(), strClsid.length(), *result);
	return true;
}

bool CPluginObj::OnGetClassid( NPVariant *result )
{
	std::string strClassID = "";
	if( m_host->IsInitialized() )
	{
		std::string strClsid;
		if( m_host->GetClsID(strClsid) )
		{
			if( strClsid.length() > 2 )
			{
				strClassID = "CLSID:" + strClsid.substr(1, strClsid.length() - 2);
			}
		}
	}
	STRINGN_TO_NPVARIANT(strClassID.c_str(), strClassID.length(), *result);
	return true;
}

bool CPluginObj::OnSetProperty( NPIdentifier name, const NPVariant *value )
{
	if( name == s_classid || name == s_clsid )
	{
		return OnSetClsid(value);
	}

	if( !IsPropertyBag(name) )
	{
		return false;
	}

	if( OnSetEvent(name, value) )
	{
		return true;
	}

	return false;	
}

bool CPluginObj::OnSetClsid( const NPVariant* value )
{
	if( !NPVARIANT_IS_STRING(*value) )
	{
		return false;
	}

	CComBSTR bstrClsid(NPVARIANT_TO_STRING(*value).UTF8Length, NPVARIANT_TO_STRING(*value).UTF8Characters);
	std::wstring strClsid = bstrClsid;
	CLSID clsid = m_host->ParseClsidFromString(strClsid);
	if( CLSID_NULL == clsid )
	{
		return true;
	}

	if( !m_host->IsValid() || clsid != m_host->GetClsID() )
	{
		m_host->Destroy();
		if( m_host->SetClsID(strClsid) )
		{
			m_host->Create();
	
			IUnknown *unk;
			if( SUCCEEDED(m_host->GetControlUnknown(&unk)) )
			{
				SetControl(unk);
				unk->Release();
			}

			m_host->ResetPos();
		}
	}
	return true;
}

bool CPluginObj::OnSetEvent( NPIdentifier name, const NPVariant *value )
{
	std::wstring strEvent;
	if( !IsEventBag(name, strEvent) )
	{
		return false;
	}

	return m_host->AddPersistEventBag(strEvent, value);
}





