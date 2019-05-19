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

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include <map>
#include <string>
#include "npobjproxy.h"
#include "AxHost.h"
class PropertyList;

class CPlugin : public nsPluginInstanceBase
{
public:
	CPlugin(nsPluginCreateData* cd);
	~CPlugin();
public:
	bool CheckValid();
	bool RegisterHost();
	void UnRegisterHost();
	NPObject* RegisterObject();
	NPObject* GetScriptableObject();
	void UnRegisterObject();

public:
	virtual NPBool init(NPWindow* pNPWindow);
	virtual void shut();
	virtual NPBool isInitialized();
	virtual uint16_t HandleEvent(void* event);
	virtual NPError	SetWindow(NPWindow* pNPWindow);
	virtual NPError GetValue(NPPVariable variable, void *value);
	virtual NPError SetValue(NPNVariable variable, void *value);

private:
	void ParsePluginParam(nsPluginCreateData * nsData);
	void FillPluginParam(nsPluginCreateData * nsData);
	void FillHostParam(nsPluginCreateData * nsData);	
	void FillDefaultParam();
	void FillPropertyBag();
	void FillAddBarUrl();
	void FillUserAgent();
	bool GetNodeProperty(bool bAttr);

protected:
	static LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);	
	void SetPluginWindow(HWND hWnd);

private:
	NPBool				m_bInitialized;			///< 初始化标记
	HWND				m_hWnd; 				///< 窗口句柄
	WNDPROC				m_lpOldProc;			///< 原有窗口过程函数
	NPP					m_pNPInstance;			///< Plugin实例句柄
	NPObject*			m_pScriptableObject;	///< js与plugin交互接口
	CAxHost				m_host;					///< 实例
	bool                m_bFillProperty;		///< 填充属性列表
};				

#endif // __PLUGIN_H__
