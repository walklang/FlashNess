#pragma once
#include <string>
#include <map>
#include "IPluginObj.h"
#include "PluginObjBase.h"
#include "pluginbase.h"
#include <comdef.h>
#include <comdefsp.h>

class CPlugin;
class CAxHost;
namespace ATL
{
	template <class T, const IID* piid = &__uuidof(T)>
	class CComQIPtr;
}

class CPluginObj
	: public IPluginObjBase
	, public IPluginObjBaseAllocateImpl<CPluginObj>
{
public:
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
	CPluginObj(NPP npp);
	~CPluginObj();
	static CPluginObj* CreateHostObject(NPP npp, CAxHost* pHost);
	static void DestroyHostObject(NPObject* obj);
	CPlugin* GetPluginPtr();
public:
	void SetControl(IUnknown *unk) { m_disp = unk; }
	HRESULT GetControl(IUnknown **obj);
	bool DispidFromMethodName(__in NPIdentifier name, __out DISPID& id, unsigned int& invKind);
	bool DispidFromName(__in NPIdentifier name, __out DISPID& id, unsigned int invKind = INVOKE_FUNC);
	bool CheckInvokeKind(DISPID id, unsigned int invKind = INVOKE_FUNC);
	bool FindDesc(ITypeInfoPtr spInfo, TYPEATTR *attr, DISPID id, unsigned int invKind);
	bool GetDescNum(UINT &ctInfo, uint32_t &nCount);
	bool GetDescName(ITypeInfoPtr spInfo, UINT index, std::string& name, bool bFunc = true);
	int GetDescName(ITypeInfoPtr spInfo, TYPEATTR *attr, NPIdentifier* vIdentifiers);

private:
	bool OnMethod(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
	bool OnGetProperty( NPIdentifier name, NPVariant *result );
	bool OnSetProperty( NPIdentifier name, const NPVariant *value );

	bool OnAttachEvent(const NPVariant *args, uint32_t argCount, NPVariant *result);
	bool OnGetClsid(NPVariant *result);
	bool OnGetClassid(NPVariant *result);
	bool OnSetClsid(const NPVariant* value);
	bool OnSetEvent(NPIdentifier name, const NPVariant *value);

private:
	void FinalRelease();
	void ConstructProperty();
	void ConstructMethod();
	inline void MakePropertyIdentifier( NPIdentifier& identifier, const std::string& name);
	inline void MakeMethodIdentifier( NPIdentifier& identifier, const std::string& name);
	bool IsPropertyBag(NPIdentifier& identifier);
	bool IsEventBag(NPIdentifier& identifier, std::wstring &evnet);
	bool IsMethodBag(NPIdentifier& identifier);

private:
	ATL::CComQIPtr<IDispatch> m_disp;
	CAxHost*				m_host;
	bool					m_bInvalid;

private:
	typedef std::map<NPIdentifier, std::string> MAP_IDENTIFIER; 
	MAP_IDENTIFIER m_propertyBag;
	MAP_IDENTIFIER m_methodBag;
private:
	DECLARE_NPOBJECT_CLASS_WITH_BASE(CPluginObj);
};




