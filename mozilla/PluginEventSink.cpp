#include "stdafx.h"
#include "PluginEventSink.h"
#include "variants.h"

CPluginEventSink::CPluginEventSink( void ) 
	: eventsObj()
{

}

CPluginEventSink::~CPluginEventSink( void )
{
	ReleaseObjEvent();
}

HRESULT CPluginEventSink::InternalInvoke( DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr )
{
	if (DISPATCH_METHOD != wFlags) {
		// any other reason to call us?!
		return S_FALSE;
	}

	HRESULT hr = __super::InternalInvoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
	if( S_OK == hr )
	{
		return S_OK;
	}

	
	EventObject::iterator cur = eventsObj.find(dispIdMember);

	if (eventsObj.end() != cur) {
		
		if( !cur->second )
		{
			return S_FALSE;
		}

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

		bool success = NPN_InvokeDefault(instance, cur->second, args, pDispParams->cArgs, &result);

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
	}

	return S_OK;
}

void CPluginEventSink::ReleaseObjEvent()
{
	while( !eventsObj.empty() )
	{
		EventObject::iterator pit = eventsObj.begin();
		if( pit != eventsObj.end() )
		{
			if( !pit->second )
			{
				continue;
			}
			//NPN_ReleaseObject(pit->second);
			eventsObj.erase(pit);
		}
	}			
}

