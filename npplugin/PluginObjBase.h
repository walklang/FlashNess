#pragma once

#include "pluginbase.h"

class IPluginObjBase;

#define OBJECT_IS_TRUE(npobj)	if( !npobj ) return false;
#define OBJECT_IS_VOID(npobj)		if( !npobj ) return;


#define DECLARE_NPOBJECT_CLASS_WITH_BASE(_class)					\
	static NPClass m_NPClass;										\
	friend class IPluginObjBaseAllocateImpl<_class>; 

#define DEFINE_NPOBJECT_CLASS_WITH_BASE(_class)						\
NPClass _class::m_NPClass = {										\
	NP_CLASS_STRUCT_VERSION_CTOR,									\
	_class::Allocate,												\
	_class::Deallocate,												\
	IPluginObjBase::_Invalidate,                                    \
	IPluginObjBase::_HasMethod,                                     \
	IPluginObjBase::_Invoke,                                        \
	IPluginObjBase::_InvokeDefault,                                 \
	IPluginObjBase::_HasProperty,                                   \
	IPluginObjBase::_GetProperty,                                   \
	IPluginObjBase::_SetProperty,                                   \
	IPluginObjBase::_RemoveProperty,                                \
	IPluginObjBase::_Enumerate,                                     \
	IPluginObjBase::_Construct                                      \
}

#define GET_NPOBJECT_CLASS(_class) &_class::m_NPClass


class IPluginObjBase
	: public NPObject
{
public:
	IPluginObjBase(NPP npp)
		: m_Npp(npp)
	{
	}
	~IPluginObjBase(void)
	{
	}

	// Virtual NPObject hooks called through this base class. Override
	// as you see fit.
	virtual void Invalidate();
	virtual bool HasMethod(NPIdentifier name);
	virtual bool Invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
	virtual bool InvokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result);
	virtual bool HasProperty(NPIdentifier name);
	virtual bool GetProperty(NPIdentifier name, NPVariant *result);
	virtual bool SetProperty(NPIdentifier name, const NPVariant *value);
	virtual bool RemoveProperty(NPIdentifier name);
	virtual bool Enumerate(NPIdentifier **identifier, uint32_t *count);
	virtual bool Construct(const NPVariant *args, uint32_t argCount, NPVariant *result);

public:
	static void _Invalidate(NPObject *npobj);
	static bool _HasMethod(NPObject *npobj, NPIdentifier name);
	static bool _Invoke(NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
	static bool _InvokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result);
	static bool _HasProperty(NPObject * npobj, NPIdentifier name);
	static bool _GetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result);
	static bool _SetProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value);
	static bool _RemoveProperty(NPObject *npobj, NPIdentifier name);
	static bool _Enumerate(NPObject *npobj, NPIdentifier **identifier, uint32_t *count);
	static bool _Construct(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result);

public:
	NPP m_Npp;
};

template<class T>
class IPluginObjBaseAllocateImpl
{
public:
	~IPluginObjBaseAllocateImpl() {}
public:
	static NPObject * Allocate( NPP npp, NPClass *aClass )
	{
		return ( new T(npp) );
	}
	static void Deallocate(NPObject *npobj)
	{
		if( npobj )
		{
			T* obj = static_cast<T*>(npobj);
			delete obj;
			obj = NULL;
		}
	}
	static NPObject* CreateObject(NPP npp)
	{
		return NPN_CreateObject(npp, GET_NPOBJECT_CLASS(T));
	}
	static void DestroyObject(NPObject* obj)
	{
		return NPN_ReleaseObject(obj);
	}
	static NPClass* GetNPClass()
	{ 
		return &T::m_NPClass; 
	}
};

