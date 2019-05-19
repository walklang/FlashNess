#include "stdafx.h"
#include "npPlugin.h"
#include "plugin.h"

NPError PluginInitialize()
{
	return NPERR_NO_ERROR;
}

void PluginRelease()
{

}

NPError CreatePlugin( nsPluginCreateData* npCD, nsPluginInstanceBase ** plugin )
{
	NPError rc = NPERR_NO_ERROR;
	CPlugin* newPlugin = new CPlugin(npCD);

	do 
	{
		if( newPlugin )
		{
			if( !newPlugin->RegisterHost() )
			{
				rc = NPERR_GENERIC_ERROR;
				break;
			}
			newPlugin->RegisterObject(); 	  // ע�ḳֵ
			break;
		}

		rc = NPERR_OUT_OF_MEMORY_ERROR;

	} while (false);
	
	if( NPERR_NO_ERROR != rc )
	{
		delete newPlugin;
		newPlugin = NULL;
	}

	*plugin = dynamic_cast<nsPluginInstanceBase*>(newPlugin);
	return rc;
}

void DestroyPlugin(nsPluginInstanceBase * plugin)
{
	if( plugin )
	{
		CPlugin* aPlugin = dynamic_cast<CPlugin*>(plugin);
		if( aPlugin )
		{
			delete aPlugin;
			aPlugin = NULL;
		}
		else
		{
			delete plugin;	
			plugin = NULL;
		}
	}
}



