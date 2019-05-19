#pragma once
#include <atlbase.h>
#include <atlcom.h>
#include "ControlEventSink.h"

class CPluginEventSink
	: public CControlEventSink
{
public:
	CPluginEventSink(void);
	~CPluginEventSink(void);

public:
	virtual HRESULT InternalInvoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

public:
	void ReleaseObjEvent();

public:
	typedef std::map<DISPID, NPObject *> EventObject;
	EventObject eventsObj;
};

