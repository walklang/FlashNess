// dllmain.h : 模块类的声明。

class CFlashNessModule : public ATL::CAtlDllModuleT< CFlashNessModule >
{
public :
	DECLARE_LIBID(LIBID_FlashNessLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_FlashNessLib, "{B071C9E1-E5F4-4F78-8BC2-96731CE25138}")
};

extern class CFlashNessModule _AtlModule;
extern HMODULE _Instance;
