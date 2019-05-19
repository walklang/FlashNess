#include "stdafx.h"
#include "PluginObj.h"
#include "variants.h"
#include "pluginbase.h"

void CPluginObj::Invalidate()
{
	m_bInvalid = true;
}

bool CPluginObj::HasMethod( NPIdentifier name )
{
	if( m_bInvalid )
	{
		return false;
	}

	if( IsMethodBag(name) )
	{
		return true;
	}

	DISPID id;
	unsigned int invKind = INVOKE_FUNC;
	if( !DispidFromName(name, id, invKind) 
		&& !DispidFromMethodName(name, id, invKind) )
	{
		return false;
	}

	return true;
}

bool CPluginObj::Invoke( NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result )
{
	NULL_TO_NPVARIANT(*result);
	if( m_bInvalid )
	{
		return false;
	}

	if( IsMethodBag(name) )
	{
		return OnMethod(name, args, argCount, result);
	}


	DISPID id;
	unsigned int invKind = INVOKE_FUNC;
	if( !DispidFromName(name, id, invKind) 
		&& !DispidFromMethodName(name, id, invKind) )
	{
		return false;
	}

	if( INVOKE_PROPERTYGET == invKind )
	{
		return GetProperty(name, result);
	}
	else if( INVOKE_PROPERTYPUT == invKind )
	{
		if( 0 >= argCount )
		{
			return false;
		}
		return SetProperty(name, &args[0]);
	}

	CComVariant *vArgs = NULL;
	if( 0 != argCount )
	{
		vArgs = new CComVariant[argCount];
		if( !vArgs )
		{
			return false;
		}

		for( size_t i = 0; i < argCount; ++i )
		{
			NPVar2Variant(&args[i], &vArgs[argCount-i-1], m_Npp);	
		}
	}

	DISPPARAMS params = {NULL, NULL, 0, 0};
	params.cArgs = argCount;
	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = NULL;
	params.rgvarg = vArgs;

	CComVariant vResult;

	HRESULT hr = m_disp->Invoke(id, GUID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &params, &vResult, NULL, NULL);
	if( vArgs )
	{
		delete []vArgs;
	}

	if( FAILED(hr) )
	{
		return false;
	}
	Variant2NPVar(&vResult, result, m_Npp);
	return true;
}

bool CPluginObj::InvokeDefault( const NPVariant *args, uint32_t argCount, NPVariant *result )
{
	NULL_TO_NPVARIANT(*result);
	return true;
}

bool CPluginObj::HasProperty( NPIdentifier name )
{
	if( m_bInvalid )
	{
		return false;
	}

	if( IsPropertyBag(name) )
	{
		return true;
	}

	DISPID id;
	unsigned int invKind = INVOKE_PROPERTYGET | INVOKE_PROPERTYPUT;
	if( !DispidFromName(name, id, invKind)
		/*&& !DispidFromMethodName(name, id, invKind)*/ )
	{
		return false;
	}

	return true;
}

bool CPluginObj::GetProperty( NPIdentifier name, NPVariant *result )
{
	NULL_TO_NPVARIANT(*result);
	if( m_bInvalid )
	{
		return false;
	}

	VOID_TO_NPVARIANT(*result);
	if( IsPropertyBag(name) )
	{
		return OnGetProperty(name, result);
	}

	DISPID id;
	unsigned int invKind = INVOKE_PROPERTYGET;
	if( !DispidFromName(name, id, invKind)
		&& !DispidFromMethodName(name, id, invKind) )
	{
		return false;
	}

	if( INVOKE_PROPERTYGET != invKind )
	{
		return false;
	}

	DISPPARAMS params;

	params.cArgs = 0;
	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = NULL;
	params.rgvarg = NULL;

	CComVariant vResult;
	HRESULT hr = m_disp->Invoke(id, GUID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &params, &vResult, NULL, NULL);
	if( SUCCEEDED(hr) )
	{
		Variant2NPVar(&vResult, result, m_Npp);
		return true;
	}

	return false;
}

bool CPluginObj::SetProperty( NPIdentifier name, const NPVariant *value )
{
	if( m_bInvalid )
	{
		return false;
	}

	if( IsPropertyBag(name) )
	{
		return OnSetProperty(name, value);
	}

	DISPID id;
	unsigned int invKind = INVOKE_PROPERTYPUT;
	if( !DispidFromName(name, id, invKind) 
		&& !DispidFromMethodName(name, id, invKind) )
	{
		return false;
	}

	if( INVOKE_PROPERTYPUT != invKind )
	{
		return false;
	}

	CComVariant val;
	NPVar2Variant(value, &val, m_Npp);
	DISPID dispidNamed = DISPID_PROPERTYPUT;

	DISPPARAMS params;
	params.cNamedArgs = 1;
	params.rgdispidNamedArgs = &dispidNamed;
	params.cArgs = 1;
	params.rgvarg = &val;

	CComVariant vResult;

	WORD wFlags = DISPATCH_PROPERTYPUT;
	if (val.vt == VT_DISPATCH) 
	{
		wFlags |= DISPATCH_PROPERTYPUTREF;
	}

	if( FAILED(m_disp->Invoke(id, GUID_NULL, LOCALE_SYSTEM_DEFAULT, wFlags, &params, &vResult, NULL, NULL)) )
	{
		return false;
	}

	return true;
}

bool CPluginObj::RemoveProperty( NPIdentifier name )
{
	return false;
}

bool CPluginObj::Enumerate( NPIdentifier **identifier, uint32_t *count )
{
	*count = 0;
	if( !m_disp )
	{
		return false;
	}
	
	uint32_t nCount = 0 ;
	UINT ctInfo = 0;
		
	// 获取总数
	if( !GetDescNum(ctInfo, nCount) )
	{
		return false;
	}

	// 申请内存
	NPIdentifier *vIdentifiers = (NPIdentifier*)NPN_MemAlloc(sizeof(NPIdentifier) * nCount );
	if( !vIdentifiers )
	{
		return false;
	}

	// 填充
	uint32_t pos = 0;
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
	
		pos += GetDescName(spInfo, attr, vIdentifiers+pos);

		spInfo->ReleaseTypeAttr(attr);
	}

	// 赋值
	*count = pos;
	*identifier = vIdentifiers;
	return false;
}

bool CPluginObj::Construct( const NPVariant *args, uint32_t argCount, NPVariant *result )
{
	return false;
}