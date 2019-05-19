#include "stdafx.h"
#include "plugin.h"
#include <windows.h>
#include <windowsx.h>
#include "pluginbase.h"
#include "PluginObj.h"
             
#define NP_MIMETYPE "application/ant-sh-flashness-plugin"

void CPlugin::FillPluginParam( nsPluginCreateData * nsData ) {
    // Check the NPMIMEType. 
    if( 0 == _stricmp(nsData->type, NP_MIMETYPE)) {
        return;
    }

    // Pointer to the Plugin instance and get Window Object.
    m_pNPInstance = nsData->instance;

    NPObjectProxy window;
    NPN_GetValue(m_pNPInstance, NPNVWindowNPObject, &window);

    BOOL bWindowed = TRUE;
    NPN_SetValue(m_pNPInstance, NPPVpluginWindowBool, (void*)bWindowed);

    m_host.ResetNPP(m_pNPInstance);
}

bool CPlugin::RegisterHost() {
    if (!CheckValid()) {
        return false;
    }
    return m_host.Create();
}

void CPlugin::UnRegisterHost() {
    m_host.Destroy();
}

NPObject * CPlugin::RegisterObject()
{
    if( !m_pScriptableObject )
        m_pScriptableObject = m_host.CreateScriptableObject();

    return m_pScriptableObject;
}

NPObject* CPlugin::GetScriptableObject()
{
    if( m_pScriptableObject )
    {
        NPN_RetainObject(m_pScriptableObject);
    }
    return m_pScriptableObject;	
}

void CPlugin::UnRegisterObject()
{
    m_host.DestroyScriptableObject(m_pScriptableObject);
    m_pScriptableObject = NULL;
    return;
}

void CPlugin::SetPluginWindow( HWND hWnd )
{
    if( hWnd == m_hWnd )
        return;

    if( NULL == hWnd )
    {
        if( m_lpOldProc )
            ::SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)m_lpOldProc);
        ::SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)NULL);
    }
    else
    {
        m_lpOldProc = (WNDPROC)::SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)PluginWinProc);
        ::SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)this);		
    }

    m_hWnd = hWnd;
    m_host.SetWindow(m_hWnd);
}

LRESULT CALLBACK CPlugin::PluginWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    bool bHandled = false;

    CPlugin* pPlugin = reinterpret_cast<CPlugin*>(GetWindowLong(hWnd, GWL_USERDATA));
    if( NULL != pPlugin )
    {
        pPlugin->m_host.MessageHandler(msg, wParam, lParam, bHandled);
    }

    if( bHandled )
        return 0;

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

NPBool CPlugin::init(NPWindow* pNPWindow)
{
    if(pNPWindow == NULL)
        return false;

    HWND hWnd = (HWND)pNPWindow->window;
    if(hWnd == NULL)
        return false;
    if( !::IsWindow(hWnd) )
        return false;

    // 子类化窗口
    SetPluginWindow(hWnd);
    m_bInitialized = true;
    return true;
}

void CPlugin::shut()
{
    m_pNPInstance = NULL;
    m_host.ResetNPP(NULL);
    m_bInitialized = false;
    UnRegisterObject();
    UnRegisterHost();
    SetPluginWindow(NULL);
}

NPBool CPlugin::isInitialized()
{
    return m_bInitialized;
}

uint16_t CPlugin::HandleEvent( void* event )
{
    NPEvent* npEvent = (NPEvent *)event;

    if( !npEvent ) 
    {
        return 0;
    }

    return m_host.DoEvent(npEvent);
}

NPError CPlugin::SetWindow( NPWindow* pNPWindow )
{
    if( NULL == pNPWindow )
        return NPERR_GENERIC_ERROR;

    SetPluginWindow((HWND)pNPWindow->window);

    if( !m_host.IsInitialized() )
    {
        RegisterHost();
    }

    if( m_host.IsInitialized() )
    {
        RECT rc;
        rc.left	= 0;
        rc.top = 0;
        rc.right = pNPWindow->width;
        rc.bottom = pNPWindow->height;
        m_host.SetPos(rc);
    }

    return NPERR_NO_ERROR;
}

NPError CPlugin::GetValue( NPPVariable variable, void *value )
{
    NPError npError = NPERR_NO_ERROR;

    switch( variable )
    {
    case NPPVpluginNameString:
        *((char**)value) = "ShangHai Ant Studio FlashNess Plugin";
        break;
    case NPPVpluginDescriptionString:
        *((char**)value) = "The FlashNess Plugin of www.ant.sh";
        break;
    case NPPVpluginScriptableNPObject:
        *((NPObject**)value) = GetScriptableObject();
        break;
    default:
        npError = NPERR_GENERIC_ERROR;
        break;
    }

    return npError;
}

NPError CPlugin::SetValue( NPNVariable variable, void *value )
{
    return NPERR_NO_ERROR;
}