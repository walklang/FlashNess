/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
* Version: NPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Netscape Public License
* Version 1.1 (the "License"); you may not use this file except in
* compliance with the License. You may obtain a copy of the License at
* http://www.mozilla.org/NPL/
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
*
* Alternatively, the contents of this file may be used under the terms of
* either the GNU General Public License Version 2 or later (the "GPL"), or 
* the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
* in which case the provisions of the GPL or the LGPL are applicable instead
* of those above. If you wish to allow use of your version of this file only
* under the terms of either the GPL or the LGPL, and not to allow others to
* use your version of this file under the terms of the NPL, indicate your
* decision by deleting the provisions above and replace them with the notice
* and other provisions required by the GPL or the LGPL. If you do not delete
* the provisions above, a recipient may use your version of this file under
* the terms of any one of the NPL, the GPL or the LGPL.
*
* ***** END LICENSE BLOCK ***** */

//////////////////////////////////////////////////
//
// CPlugin class implementation
//
#include "stdafx.h"
#include "plugin.h"
#include "PropertyList.h"
#include "NodeParse.h"
#include "pluginbase.h"
#include "PluginObj.h"
#include "UserAgent.h"
#include "variants.h"
#include <windows.h>
#include <windowsx.h>


CPlugin::CPlugin( nsPluginCreateData* cd )
	: m_bInitialized(false)
	, m_pNPInstance(NULL)
	, m_hWnd(NULL)
	, m_lpOldProc(NULL)
	, m_bFillProperty(false)
	, m_pScriptableObject(NULL) {
	ParsePluginParam(cd);
}

CPlugin::~CPlugin()
{
	shut();
}

bool CPlugin::CheckValid()
{
	if( !m_bFillProperty || !m_host.IsValid() )
	{
		return false;
	}	
	return true;
}

void CPlugin::ParsePluginParam(nsPluginCreateData * nsData)
{
	FillPluginParam(nsData);
	FillHostParam(nsData);
	FillDefaultParam();

	if( NULL != m_pNPInstance )
	{
		FillPropertyBag();
		FillAddBarUrl();
		FillUserAgent();
	}
}

void CPlugin::FillHostParam(nsPluginCreateData * nsData)
{
	int16_t argNum = nsData->argc;
	for( int16_t argCurrent = 0; argCurrent < argNum; argCurrent++ )
	{	
		if( NULL == nsData->argn[argCurrent] 
		|| NULL == nsData->argv[argCurrent] )
			continue;

		std::wstring wKey;
		std::string strKey	 = std::string(nsData->argn[argCurrent]);
		wKey = ATL::CA2TEX<>(strKey.c_str());
		std::transform(wKey.begin(), wKey.end(), wKey.begin(), ::tolower);


		std::wstring wValue;
		std::string strValue = std::string(nsData->argv[argCurrent]);
		wValue = ATL::CA2TEX<>(strValue.c_str());

		if( m_host.ParsePersistPropertyBag(wKey, wValue) )
		{
		}

		//if( m_host.AddPersistPropertyBag(wKey, wValue) )
		//{
		//	m_bFillProperty = true;
		//}
	}
}

void CPlugin::FillDefaultParam()
{
	m_host.AddDefaultClsid();

	m_host.AddPersistPropertyBag(L"width", L"0");
	m_host.AddPersistPropertyBag(L"height", L"0");
	m_host.AddPersistPropertyBag(L"url", L"https://www.ant.shh");
	m_host.AddPersistPropertyBag(L"name", L"上海蚂蚁工作室");
}

void CPlugin::FillPropertyBag()
{
	bool bAttrBag = GetNodeProperty(true);
	bool bParamBag = GetNodeProperty(false); 

	if( bAttrBag || bParamBag )
	{
		m_bFillProperty = true;
	}
	else
	{
		m_bFillProperty = false;
	}
}

void CPlugin::FillAddBarUrl()
{
	struct CTopWindow
	{
		CTopWindow(NPP npp, NPObjectProxy& obj)
		{
			s_parent = NPN_GetStringIdentifier("parent");
			m_npp = npp;
			GetTopWindow(obj);
		}
		~CTopWindow()
		{
			if( !m_vecNPVar.empty() )
			{
				NPVariant pVar = m_vecNPVar.back();
				NPN_ReleaseVariantValue(&pVar);
				m_vecNPVar.pop_back();
			}
		}
		bool GetTopWindow(NPObjectProxy& obj)
		{
			NPVariant pTopVar;
			bool bRet = NPN_GetProperty(m_npp, obj, s_parent, &pTopVar);
			if( !bRet )
			{
				return false;
			}
			if( !NPVARIANT_IS_OBJECT(pTopVar) )
			{
				return false;
			}
			if( NPVARIANT_TO_OBJECT(pTopVar) == obj )
			{
				return false;
			}

			NPObject *npObj = NPVARIANT_TO_OBJECT(pTopVar);
			obj = npObj;
			return GetTopWindow(obj);
		}
	private:
		NPP m_npp;
		NPIdentifier s_parent;
		std::vector<NPVariant>	m_vecNPVar;
	};
	
	if( NULL == m_pNPInstance )
	{
		return;
	}

	NPIdentifier s_location = NPN_GetStringIdentifier("location");
	NPIdentifier s_href     = NPN_GetStringIdentifier("href");
	NPIdentifier s_parent   = NPN_GetStringIdentifier("parent");


	NPObjectProxy topWin;
	NPN_GetValue(m_pNPInstance, NPNVWindowNPObject, &topWin);
	if( !topWin )
		return;

	CTopWindow topWinObj(m_pNPInstance, topWin);

	NPVariantProxy pLocationVar;
	if( !NPN_GetProperty(m_pNPInstance, topWin, s_location, &pLocationVar )
		|| !NPVARIANT_IS_OBJECT(pLocationVar) )
	{
		return;
	}

	NPObject *npLocation = NPVARIANT_TO_OBJECT(pLocationVar);
	if( !npLocation )
		return;

	NPVariantProxy pHref;
	if( !NPN_GetProperty(m_pNPInstance, npLocation, s_href, &pHref)
		|| !NPVARIANT_IS_STRING(pHref) )
	{
		return;
	}

	std::wstring wstrUrl;
	std::string strUrl = pHref.value.stringValue.UTF8Characters;
	wstrUrl = CA2TEX<>(strUrl.c_str());
	m_host.AddPersistPropertyBag(L"addbar", wstrUrl);

	//CComBSTR bstrValue(pHref.value.stringValue.UTF8Length, pHref.value.stringValue.UTF8Characters);
	//m_host.SetLocationURL(bstrValue.Detach());
	//std::wstring str(bstrValue);
	//m_host.SetLocationURL(str);
}

void CPlugin::FillUserAgent()
{	
	std::wstring wstrUA;
	std::string strUserAgent = NPN_UserAgent(m_pNPInstance);
	wstrUA = CA2TEX<>(strUserAgent.c_str());
	std::transform(wstrUA.begin(), wstrUA.end(), wstrUA.begin(), ::tolower);


	static CUserAgent userAgentObj;
	if( userAgentObj.Navigator(wstrUA) )
	{
		m_host.AddPersistPropertyBag(L"browsertype", L"1");
		m_host.AddPersistPropertyBag(L"browser", wstrUA);
	}
}

bool CPlugin::GetNodeProperty( bool bAttr )
{
	if( NULL == m_pNPInstance )
	{
		return false;
	}

	CNodeParse NODE(m_pNPInstance);
	NPVariantProxy var_childNodes;
	if( !NODE.GetNodeObj(var_childNodes, bAttr) )
	{
		return false;
	}

	// 解析
	bool bParse = false;
	NPObject *childNodes = NPVARIANT_TO_OBJECT(var_childNodes);
	int length = NODE.GetNodeLength(childNodes);

	// 取属性设置
	for (int32_t i = 0; i < length; ++i) 
	{
		NPIdentifier id = NPN_GetIntIdentifier(i);

		NPVariantProxy var_par;
		NPVariantProxy var_parName, var_parValue;

		if( !NPN_GetProperty(m_pNPInstance, childNodes, id, &var_par) 
			|| !NPVARIANT_IS_OBJECT(var_par) )
		{
			continue;
		}

		NPObject *var_obj = NPVARIANT_TO_OBJECT(var_par);
		if( !NODE.GetNodeValue(var_obj, var_parName, var_parValue, bAttr) )
		{
			continue;
		}
		if( !NPVARIANT_IS_STRING(var_parName) )
			continue;

		CComBSTR paramName(NPVARIANT_TO_STRING(var_parName).UTF8Length, NPVARIANT_TO_STRING(var_parName).UTF8Characters);

		if( NPVARIANT_IS_STRING(var_parValue) )
		{
			CComVariant paramValue;
			NPVar2Variant( &var_parValue, &paramValue, m_pNPInstance );
			m_host.AddPersistPropertyBag(paramName, paramValue);

			bParse = true;
		}

		if( NPVARIANT_IS_OBJECT(var_parValue) )
		{

		}
	}
	return bParse;
}