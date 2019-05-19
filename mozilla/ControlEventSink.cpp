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
#include "ControlEventSink.h"
#include "variants.h"

CControlEventSink::CControlEventSink() :
    m_dwEventCookie(0),
	m_bSubscribed(false),
    m_EventIID(GUID_NULL),
	events()
{
}

CControlEventSink::~CControlEventSink()
{
    UnsubscribeFromEvents();
}

BOOL
CControlEventSink::GetEventSinkIID(IUnknown *pControl, IID &iid, ITypeInfo **typeInfo)
{
	iid = GUID_NULL;
    if (!pControl)
    {
        return FALSE;
    }

	// IProvideClassInfo2 way is easiest
//    CComQIPtr<IProvideClassInfo2> classInfo2 = pControl;
//    if (classInfo2)
//    {
//        classInfo2->GetGUID(GUIDKIND_DEFAULT_SOURCE_DISP_IID, &iid);
//        if (!::IsEqualIID(iid, GUID_NULL))
//        {
//            return TRUE;
//        }
//    }

    // Yuck, the hard way
    CComQIPtr<IProvideClassInfo> classInfo = pControl;
    if (!classInfo)
    {
        return FALSE;
    }

    // Search the class type information for the default source interface
    // which is the outgoing event sink.

    CComPtr<ITypeInfo> classTypeInfo;
    classInfo->GetClassInfo(&classTypeInfo);
    if (!classTypeInfo)
    {
        return FALSE;
    }
    TYPEATTR *classAttr = NULL;
    if (FAILED(classTypeInfo->GetTypeAttr(&classAttr)))
    {
        return FALSE;
    }
    INT implFlags = 0;
    for (UINT i = 0; i < classAttr->cImplTypes; i++)
    {
        // Search for the interface with the [default, source] attr
        if (SUCCEEDED(classTypeInfo->GetImplTypeFlags(i, &implFlags)) &&
            implFlags == (IMPLTYPEFLAG_FDEFAULT | IMPLTYPEFLAG_FSOURCE))
        {
            CComPtr<ITypeInfo> eventSinkTypeInfo;
            HREFTYPE hRefType;
            if (SUCCEEDED(classTypeInfo->GetRefTypeOfImplType(i, &hRefType)) &&
                SUCCEEDED(classTypeInfo->GetRefTypeInfo(hRefType, &eventSinkTypeInfo)))
            {
                TYPEATTR *eventSinkAttr = NULL;
                if (SUCCEEDED(eventSinkTypeInfo->GetTypeAttr(&eventSinkAttr)))
                {
                    iid = eventSinkAttr->guid;
                    if (typeInfo)
                    {
                        *typeInfo = eventSinkTypeInfo.p;
                        (*typeInfo)->AddRef();
                    }
                    eventSinkTypeInfo->ReleaseTypeAttr(eventSinkAttr);
                }
            }
            break;
        }
    }
    classTypeInfo->ReleaseTypeAttr(classAttr);
    return (!::IsEqualIID(iid, GUID_NULL));
}

void CControlEventSink::UnsubscribeFromEvents()
{
    if (m_bSubscribed)
    {
		DWORD tmpCookie = m_dwEventCookie;
		m_dwEventCookie = 0;
		m_bSubscribed = false;
        // Unsubscribe and reset - This seems to complete release and destroy us...
        m_spEventCP->Unadvise(tmpCookie);
		// Unadvise handles the Release
		m_spEventCP.Release();
    } else {
		m_spEventCP.Release();
	}
}

HRESULT CControlEventSink::SubscribeToEvents(IUnknown *pControl)
{
    if (!pControl)
    {
        return E_INVALIDARG;
    }

    // Throw away any existing connections
    UnsubscribeFromEvents();

    // Grab the outgoing event sink IID which will be used to subscribe
    // to events via the connection point container.

    IID iidEventSink;
    CComPtr<ITypeInfo> typeInfo;
    if (!GetEventSinkIID(pControl, iidEventSink, &typeInfo))
    {
        return E_FAIL;
    }

    // Get the connection point
    CComQIPtr<IConnectionPointContainer> ccp = pControl;
    CComPtr<IConnectionPoint> cp;
    if (!ccp)
    {
        return E_FAIL;
    }
    // Custom IID
    m_EventIID = iidEventSink;
    DWORD dwCookie = 0;/*
	CComPtr<IEnumConnectionPoints> e;
	ccp->EnumConnectionPoints(&e);
	e->Next(1, &cp, &dwCookie);*/
    if (FAILED(ccp->FindConnectionPoint(m_EventIID, &cp)))
	{
		return E_FAIL;
	}
    if (FAILED(cp->Advise(this, &dwCookie)))
    {
        return E_FAIL;
    }

	m_bSubscribed = true;
    m_spEventCP = cp;
    m_dwEventCookie = dwCookie;
    m_spEventSinkTypeInfo = typeInfo;
    return S_OK;
}

HRESULT
CControlEventSink::InternalInvoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	USES_CONVERSION; 

	if (DISPATCH_METHOD != wFlags) {
		// any other reason to call us?!
		return S_FALSE;
	}

	EventMap::iterator cur = events.find(dispIdMember);

	if (events.end() != cur) {
		// invoke this event handler
		NPVariant result;
		NPVariant *args = NULL;

		if (pDispParams->cArgs > 0) {

			args = (NPVariant *)calloc(pDispParams->cArgs, sizeof(NPVariant));
			if (!args) {

				return S_FALSE;
			}

			for (unsigned int i = 0; i < pDispParams->cArgs; ++i) {
				// convert the arguments in reverse order
				Variant2NPVar(&pDispParams->rgvarg[i], &args[pDispParams->cArgs - i - 1], instance);
			}
		}

		NPObject *globalObj = NULL;
		NPN_GetValue(instance, NPNVWindowNPObject, &globalObj);

		NPIdentifier handler = NPN_GetStringIdentifier(W2A((*cur).second));

		bool success = NPN_Invoke(instance, globalObj, handler, args, pDispParams->cArgs, &result);
		NPN_ReleaseObject(globalObj);

		for (unsigned int i = 0; i < pDispParams->cArgs; ++i) {
			// convert the arguments
			if (args[i].type == NPVariantType_String) {
				// was allocated earlier by Variant2NPVar
				NPN_MemFree((void *)args[i].value.stringValue.UTF8Characters);
			}
		}

		if (!success) {

			return S_FALSE;
		}

		if (pVarResult) {
			// set the result
			NPVar2Variant(&result, pVarResult, instance);
		}

		NPN_ReleaseVariantValue(&result);
		return S_OK;
	}

	return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// IDispatch implementation


HRESULT STDMETHODCALLTYPE CControlEventSink::GetTypeInfoCount(/* [out] */ UINT __RPC_FAR *pctinfo)
{
	ATLTRACE("Not implemnted");
    return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CControlEventSink::GetTypeInfo(/* [in] */ UINT iTInfo, /* [in] */ LCID lcid, /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo)
{
	ATLTRACE("Not implemnted");
    return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CControlEventSink::GetIDsOfNames(/* [in] */ REFIID riid, /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames, /* [in] */ UINT cNames, /* [in] */ LCID lcid, /* [size_is][out] */ DISPID __RPC_FAR *rgDispId)
{
	ATLTRACE("Not implemnted");
    return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CControlEventSink::Invoke(/* [in] */ DISPID dispIdMember, /* [in] */ REFIID riid, /* [in] */ LCID lcid, /* [in] */ WORD wFlags, /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams, /* [out] */ VARIANT __RPC_FAR *pVarResult, /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo, /* [out] */ UINT __RPC_FAR *puArgErr)
{
    return InternalInvoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
}


