#pragma once
#include "Plugin.h"
#include "npobjproxy.h"

class CNodeParse
{
public:
	CNodeParse(NPP pNPP){ m_pNPInstance = pNPP; }
	~CNodeParse(){}

public:
	bool GetNodeObj(NPVariantProxy& objProxy, bool bAttr = false);
	bool GetNodeValue(NPObject *obj, NPVariantProxy &name, NPVariantProxy &value, bool bAttr = false);
	int GetNodeLength(NPObject* obj);

public:
	NPP m_pNPInstance;			///< ÊµÀý¾ä±ú

private:
	bool CheckNodeName(NPObject* obj, bool paramNode = false);	
};

bool CNodeParse::GetNodeObj( NPVariantProxy& objProxy, bool bAttr /*= false*/ )
{
	if( NULL == m_pNPInstance )
	{
		return false;
	}

	NPObjectProxy embed;
	NPN_GetValue(m_pNPInstance, NPNVPluginElementNPObject, &embed);
	if( !embed )
	{
		return false;
	}

	NPIdentifier idAttributes = NPN_GetStringIdentifier("attributes");
	NPIdentifier idChildNodes = NPN_GetStringIdentifier("childNodes");

	if( bAttr )
	{
		if( CheckNodeName(embed) )
		{
			if( NPN_GetProperty(m_pNPInstance, embed, idAttributes, &objProxy) 
				&& NPVARIANT_IS_OBJECT(objProxy) )
			{
				return true;
			}
		}
	}
	else
	{
		if( NPN_GetProperty(m_pNPInstance, embed, idChildNodes, &objProxy) 
			&& NPVARIANT_IS_OBJECT(objProxy) )
		{
			return true;
		}
	}

	return false;	
}

bool CNodeParse::GetNodeValue( NPObject *obj, NPVariantProxy &name, NPVariantProxy &value, bool bAttr /*= false*/ )
{
	if( !obj || NULL == m_pNPInstance )
	{
		return false;
	}

	NPIdentifier idAttr = NPN_GetStringIdentifier("getAttribute");
	NPIdentifier idAttrName = NPN_GetStringIdentifier("name");
	NPIdentifier idAttrValue = NPN_GetStringIdentifier("value");
	NPVariant idName, idValue;
	STRINGZ_TO_NPVARIANT("name", idName);
	STRINGZ_TO_NPVARIANT("value", idValue);

	if( bAttr )
	{
		if( !NPN_GetProperty(m_pNPInstance, obj, idAttrName, &name) )
			return false;
		if( !NPN_GetProperty(m_pNPInstance, obj, idAttrValue, &value)  )
			return false;
	}
	else
	{
		if( !CheckNodeName(obj, true) )
			return false;
		if( !NPN_Invoke(m_pNPInstance, obj, idAttr, &idName, 1, &name) )
			return false;
		if( !NPN_Invoke(m_pNPInstance, obj, idAttr, &idValue, 1, &value) )
			return false;
	}
	return true;
}

bool CNodeParse::CheckNodeName( NPObject* obj, bool paramNode /*= false*/ )
{
	if( !obj || NULL == m_pNPInstance )
	{
		return false;
	}

	NPIdentifier idname = NPN_GetStringIdentifier("nodeName");

	NPVariantProxy var_nodeName;
	if( !NPN_GetProperty(m_pNPInstance, obj, idname, &var_nodeName) 
		|| !NPVARIANT_IS_STRING(var_nodeName) )
	{
		return false;
	}

	NPString strNodeName = NPVARIANT_TO_STRING(var_nodeName);
	if( paramNode )
	{
		if( 0 != _strnicmp(strNodeName.UTF8Characters, "param", strNodeName.UTF8Length) )
		{
			return false;
		}
		return true;
	}
	else
	{
		if( 0 != _strnicmp(strNodeName.UTF8Characters, "embed", strNodeName.UTF8Length) 
			&& 0 != _strnicmp(strNodeName.UTF8Characters, "object", strNodeName.UTF8Length) )
		{
			return false;
		}
		return true;
	}
	return false;	
}

int CNodeParse::GetNodeLength( NPObject* obj )
{
	int length = 0;
	if( !obj || NULL == m_pNPInstance )
	{
		return length;
	}

	NPIdentifier idLength = NPN_GetStringIdentifier("length");

	NPVariantProxy var_length;
	if( NPN_GetProperty(m_pNPInstance, obj, idLength, &var_length) )
	{
		if( NPVARIANT_IS_INT32(var_length) )
			length = NPVARIANT_TO_INT32(var_length);
		else if( NPVARIANT_IS_DOUBLE(var_length) )
			length = static_cast<int>(NPVARIANT_TO_DOUBLE(var_length));
	}

	return length;
}
