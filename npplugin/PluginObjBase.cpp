#include "stdafx.h"
#include "PluginObjBase.h"

void IPluginObjBase::Invalidate()
{

}

bool IPluginObjBase::HasMethod( NPIdentifier name )
{
	return false;
}

bool IPluginObjBase::Invoke( NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result )
{
	return false;
}

bool IPluginObjBase::InvokeDefault( const NPVariant *args, uint32_t argCount, NPVariant *result )
{
	return false;
}

bool IPluginObjBase::HasProperty( NPIdentifier name )
{
	return false;
}

bool IPluginObjBase::GetProperty( NPIdentifier name, NPVariant *result )
{
	return false;
}

bool IPluginObjBase::SetProperty( NPIdentifier name, const NPVariant *value )
{
	return false;
}

bool IPluginObjBase::RemoveProperty( NPIdentifier name )
{
	return false;
}

bool IPluginObjBase::Enumerate( NPIdentifier **identifier, uint32_t *count )
{
	return false;
}

bool IPluginObjBase::Construct( const NPVariant *args, uint32_t argCount, NPVariant *result )
{
	return false;
}

void IPluginObjBase::_Invalidate( NPObject *npobj )
{
	OBJECT_IS_VOID(npobj);
	((IPluginObjBase *)npobj)->Invalidate();
}

bool IPluginObjBase::_HasMethod( NPObject *npobj, NPIdentifier name )
{
	OBJECT_IS_TRUE(npobj);
	return ((IPluginObjBase*)npobj)->HasMethod(name);
}

bool IPluginObjBase::_Invoke( NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result )
{
	OBJECT_IS_TRUE(npobj);
	return ((IPluginObjBase *)npobj)->Invoke(name, args, argCount, result);
}

bool IPluginObjBase::_InvokeDefault( NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result )
{
	OBJECT_IS_TRUE(npobj);
	return ((IPluginObjBase *)npobj)->InvokeDefault(args, argCount, result);
}

bool IPluginObjBase::_HasProperty( NPObject * npobj, NPIdentifier name )
{
	OBJECT_IS_TRUE(npobj);
	return ((IPluginObjBase *)npobj)->HasProperty(name);
}

bool IPluginObjBase::_GetProperty( NPObject *npobj, NPIdentifier name, NPVariant *result )
{
	OBJECT_IS_TRUE(npobj);
	return ((IPluginObjBase *)npobj)->GetProperty(name, result);
}

bool IPluginObjBase::_SetProperty( NPObject *npobj, NPIdentifier name, const NPVariant *value )
{
	OBJECT_IS_TRUE(npobj);
	return ((IPluginObjBase *)npobj)->SetProperty(name, value);
}

bool IPluginObjBase::_RemoveProperty( NPObject *npobj, NPIdentifier name )
{
	OBJECT_IS_TRUE(npobj);
	return ((IPluginObjBase *)npobj)->RemoveProperty(name);
}

bool IPluginObjBase::_Enumerate( NPObject *npobj, NPIdentifier **identifier, uint32_t *count )
{
	OBJECT_IS_TRUE(npobj);
	return ((IPluginObjBase *)npobj)->Enumerate(identifier, count);
}

bool IPluginObjBase::_Construct( NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result )
{
	OBJECT_IS_TRUE(npobj);
	return ((IPluginObjBase *)npobj)->Construct(args, argCount, result);
}
