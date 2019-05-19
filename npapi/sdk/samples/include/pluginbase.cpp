#include "stdafx.h"
#include "pluginbase.h"
#include "npPlugin.h"
#include <atlhost.h>


NPError NS_PluginInitialize()
{
	return PluginInitialize();
}

void NS_PluginShutdown()
{
	return PluginRelease();
}

NPError NS_NewPluginInstance(nsPluginCreateData * aCreateDataStruct, nsPluginInstanceBase **plugin)
{
	if( !aCreateDataStruct )
		return NPERR_GENERIC_ERROR;
	return CreatePlugin(aCreateDataStruct, plugin);
}

void NS_DestroyPluginInstance(nsPluginInstanceBase * aPlugin)
{
	return DestroyPlugin(aPlugin);
}