#include "StdAfx.h"
#include "AxRegistration.h"
#include <ObjBase.h>

extern HMODULE _Instance;


struct CRegInternal {
    ~CRegInternal() {}
    static bool SetValue(HKEY hStartKey, std::wstring strSubKey, std::wstring strName = L"", std::wstring strValue = L"", DWORD dwType = REG_SZ);
    static void DelKey(HKEY hStartKey, std::wstring strSubKey, std::wstring strName = L"");
};


CAxRegistration::CAxRegistration() {
    TCHAR tszModule[MAX_PATH + 1] = { 0 };
    ::GetModuleFileName(_Instance, tszModule, MAX_PATH);
    std::wstring strPath = tszModule;
    std::wstring::size_type npos = strPath.rfind('\\');
    if( std::wstring::npos == npos )
        return;
    strPath.erase(npos);
    strPath += L"\\npFlashNess.dll";

    CAxRegistryWorker::MozillaPlugin mozillaPlugin;

    FlashNessPlugin.clsid = L"{9A5A0A7D-30B2-4CBC-859C-90A6DF560BEF}";
    mozillaPlugin.listName = L"@ant.sh/npFlashNess";
    mozillaPlugin.mimeName = L"application/npFlashNess";
    mozillaPlugin.productName = L"npFlashNess";
    mozillaPlugin.pluginPath = strPath;
    FlashNessPlugin.MozillaPlugins.push_back(mozillaPlugin);
}

void CAxRegistration::Register() {
    CAxRegistryWorker playerWorker(FlashNessPlugin);
}

void CAxRegistration::UnRegister() {

}

bool CRegInternal::SetValue( HKEY hStartKey, std::wstring strSubKey, std::wstring strName /*= L""*/, std::wstring strValue /*= L""*/, DWORD dwType /*= REG_SZ*/ ) {
    struct CRegTool {
        HKEY hStartKey = HKEY_CLASSES_ROOT;
        std::wstring strSubKey;
        std::wstring strName;
        std::wstring strValue;
        DWORD dwType = REG_SZ;

        bool SetValue(REGSAM  samDesired) {
            HKEY RegEntry;
            if(RegOpenKeyEx(hStartKey, strSubKey.c_str(), 0, /*KEY_ALL_ACCESS*/KEY_WRITE | samDesired, &RegEntry) != ERROR_SUCCESS) {   
                LONG lRes = RegCreateKeyEx(hStartKey, strSubKey.c_str(),0, NULL, REG_OPTION_NON_VOLATILE, 
                    /*KEY_ALL_ACCESS*/KEY_WRITE | samDesired, NULL, &RegEntry, NULL);
                if(ERROR_SUCCESS != lRes) {
                    return false;
                }
            }

            if(0 != strName.length() || 0 != strValue.length()) {
                std::wstring strKey = strName;
                if( strName == L"(Ä¬ÈÏ)" ) {
                    strKey = L"";
                }
                DWORD cbData = strValue.length() * sizeof(wchar_t);
                RegSetValueEx(RegEntry, strKey.c_str(), 0, dwType, (BYTE*)strValue.c_str(), cbData);
            }

            RegCloseKey(RegEntry);
            return true;
        }
        bool SetValue() {
            SetValue(KEY_WRITE);
            SetValue(KEY_WOW64_32KEY);
            SetValue(KEY_WOW64_64KEY);
            return true;
        }
    };

    if(!hStartKey || 0 == strSubKey.length()) return false;

    CRegTool regTool;
    regTool.hStartKey = hStartKey;
    regTool.strSubKey = strSubKey;
    regTool.strName = strName;
    regTool.strValue = strValue;
    regTool.dwType = dwType;

    regTool.SetValue();
    if( HKEY_CLASSES_ROOT == hStartKey ) {
        SetPerUserRegistration(true);
        regTool.SetValue();
    }
    return true;
}

void CRegInternal::DelKey( HKEY hStartKey, std::wstring strSubKey, std::wstring strName /*= L""*/ )
{
    if(0 == strSubKey.length()) return;

    struct CRegTool {
        static DWORD RegDeleteKeyAll(HKEY hStartKey , LPCTSTR pKeyName, REGSAM samDesired) {
            DWORD   dwRtn = 0, dwSubKeyLength = 0;
            LPTSTR  pSubKey = NULL;
            TCHAR   szSubKey[MAX_PATH] = { 0 };
            HKEY    hKey;
            if (pKeyName && 0 != lstrlen(pKeyName)) {
                if((dwRtn=RegOpenKeyEx(hStartKey,pKeyName, 0, KEY_ENUMERATE_SUB_KEYS | DELETE , &hKey )) == ERROR_SUCCESS) {
                    while (dwRtn == ERROR_SUCCESS) {
                        dwSubKeyLength = MAX_PATH;
                        dwRtn=RegEnumKeyEx(hKey,0, szSubKey,&dwSubKeyLength,NULL,NULL,NULL,NULL);
                        if(dwRtn == ERROR_NO_MORE_ITEMS) {
                            dwRtn = RegDeleteKey(hStartKey, pKeyName);
                            break;
                        } else if(dwRtn == ERROR_SUCCESS) {
                            dwRtn=RegDeleteKeyAll(hKey, szSubKey, samDesired);
                        }
                    }
                    RegCloseKey(hKey);
                }
                return dwRtn;
            } 
            return ERROR_BADKEY;
        }
    };

    std::wstring strKey = strSubKey + strName + L"\\";
    CRegTool::RegDeleteKeyAll(hStartKey, strKey.c_str(), KEY_ENUMERATE_SUB_KEYS);
    SetPerUserRegistration(true);
    CRegTool::RegDeleteKeyAll(hStartKey, strKey.c_str(), KEY_ENUMERATE_SUB_KEYS);
}

CAxRegistryWorker::CAxRegistryWorker(const Plugin& plugin, bool reg /*= true*/ ) : _plugin(plugin) {
    if (_plugin.is_lawful_data()) {
        reg ? Register() : Unregister();
    }
}

void CAxRegistryWorker::Register() {
    RegisterPlugins();
    RegisterCompatibility();
    RegisterSafety();
    RegisterAllowedDomains();	
    RegisterMozillaPlugins();
    RegisterMime();
}

void CAxRegistryWorker::Unregister() {

}

void CAxRegistryWorker::RegisterCompatibility() {
    if(!_plugin.is_lawful_data()) return;

    std::wstring strSubKey = L"SOFTWARE\\Microsoft\\Internet Explorer\\ActiveX Compatibility\\";
    strSubKey += _plugin.clsid + L"\\";
    CRegInternal::SetValue(HKEY_LOCAL_MACHINE, strSubKey, L"Compatibility Flags", L"00800000", REG_DWORD);

    strSubKey = L"SOFTWARE\\Wow6432Node\\Microsoft\\Internet Explorer\\ActiveX Compatibility\\";
    strSubKey += _plugin.clsid + L"\\";
    CRegInternal::SetValue(HKEY_LOCAL_MACHINE, strSubKey, L"Compatibility Flags", L"00800000", REG_DWORD);
}

void CAxRegistryWorker::RegisterSafety() {
    if (!_plugin.is_lawful_data()) return;

    struct CRegTool {
        static void RegisterSafety( HKEY hStartKey, std::wstring strSubKey ) {
            if (0 == strSubKey.length()) return;

            std::wstring strSafetyPath;
            std::wstring strSubPath = strSubKey + L"Implemented Categories\\";

            strSafetyPath = strSubPath + L"{0DE86A57-2BAA-11CF-A229-00AA003D7352}\\";
            CRegInternal::SetValue(hStartKey, strSafetyPath);

            strSafetyPath = strSubPath + L"{40FC6ED4-2438-11cf-A3DB-080036F12502}\\";
            CRegInternal::SetValue(hStartKey, strSafetyPath);

            strSafetyPath = strSubPath + L"{7DD95801-9882-11CF-9FA9-00AA006C42C4}\\";
            CRegInternal::SetValue(hStartKey, strSafetyPath, L"Register", L"ANT");

            strSafetyPath = strSubPath + L"{7DD95802-9882-11CF-9FA9-00AA006C42C4}\\";
            CRegInternal::SetValue(hStartKey, strSafetyPath, L"Register", L"ANT");
        }
    };

    std::wstring strSubKey = L"CLSID\\" + _plugin.clsid + L"\\";
    CRegTool::RegisterSafety(HKEY_CLASSES_ROOT, strSubKey);

    std::wstring strSub64Path = L"Wow6432Node\\" + strSubKey;
    CRegTool::RegisterSafety(HKEY_CLASSES_ROOT, strSub64Path);
}

void CAxRegistryWorker::RegisterAllowedDomains() {
    struct CRegTool {
        static void RegisterAllowedDomains( HKEY hEntryWhere, std::wstring strClsid) { 
            std::wstring strPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Ext\\Stats\\";

            std::wstring strSubPath = strPath + strClsid + L"\\iexplore\\AllowedDomains\\*\\";
            CRegInternal::SetValue(hEntryWhere, strSubPath);

            strPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Ext\\PreApproved\\";
            strSubPath = strPath + strClsid + L"\\";
            CRegInternal::SetValue(hEntryWhere, strSubPath);
        }
    };

    if (!_plugin.is_lawful_data()) return;

    CRegTool::RegisterAllowedDomains(HKEY_CURRENT_USER, _plugin.clsid);
    CRegTool::RegisterAllowedDomains(HKEY_LOCAL_MACHINE, _plugin.clsid);
}

void CAxRegistryWorker::RegisterMime() {
    if( !_plugin.is_lawful_mozilla() ) {
        return;
    }

    std::vector<MozillaPlugin>::iterator pit = _plugin.MozillaPlugins.begin();
    for( ; pit != _plugin.MozillaPlugins.end(); ++pit ) {
        if (0 == pit->mimeName.length()) continue;

        std::wstring strRootPath = L"MIME\\Database\\Content Type\\";
        strRootPath += pit->mimeName + L"\\";
        CRegInternal::SetValue(HKEY_CLASSES_ROOT, strRootPath, L"CLSID", _plugin.clsid);

        std::wstring strRoot64Path = L"Wow6432Node\\" + strRootPath;
        CRegInternal::SetValue(HKEY_CLASSES_ROOT, strRoot64Path, L"CLSID", _plugin.clsid);
    }
}

void CAxRegistryWorker::RegisterMozillaPlugins() {
    struct CRegTool {
        static void RegisterPlugins(HKEY hStartKey,const MozillaPlugin& mozillaPlugin) {
            if (0 == mozillaPlugin.listName.length()) return;

            std::wstring strSubKey = L"SOFTWARE\\MozillaPlugins\\" + mozillaPlugin.listName + L"\\";
            CRegInternal::SetValue(hStartKey, strSubKey, L"Path", mozillaPlugin.pluginPath);
            CRegInternal::SetValue(hStartKey, strSubKey, L"Version", L"1.0");
            CRegInternal::SetValue(hStartKey, strSubKey, L"Description", mozillaPlugin.productName);
            CRegInternal::SetValue(hStartKey, strSubKey, L"ProductName", mozillaPlugin.productName);
            CRegInternal::SetValue(hStartKey, strSubKey, L"Register", L"ANT");

            strSubKey += L"MimeTypes\\";
            CRegInternal::SetValue(hStartKey, strSubKey, L"Dummy", L"1.0");
            strSubKey += mozillaPlugin.mimeName + L"\\";
            CRegInternal::SetValue(hStartKey, strSubKey, L"Dummy", L"1.0");
            CRegInternal::SetValue(hStartKey, strSubKey, L"Description", mozillaPlugin.productName);

            std::wstring strNextKey;
            strNextKey = strSubKey + L"clsid\\*\\";
            CRegInternal::SetValue(hStartKey, strNextKey, L"*", L"true");
            strNextKey = strSubKey + L"codeBaseUrl\\*\\";
            CRegInternal::SetValue(hStartKey, strNextKey, L"*", L"true");
            strNextKey = strSubKey + L"progid\\*\\";
            CRegInternal::SetValue(hStartKey, strNextKey, L"*", L"true");
        }
    };

    if (!_plugin.is_lawful_mozilla()) {
        return;
    }

    std::vector<MozillaPlugin>::iterator pit = _plugin.MozillaPlugins.begin();
    for (; pit != _plugin.MozillaPlugins.end(); ++pit) {
        if (0 == pit->listName.length()) continue;

        CRegTool::RegisterPlugins(HKEY_CURRENT_USER, *pit);
        CRegTool::RegisterPlugins(HKEY_LOCAL_MACHINE, *pit);
    }
}

void CAxRegistryWorker::RegisterPlugins() {
    struct CRegTool {
        static void Register(HKEY hStartKey, std::wstring strKey, const Plugin& plugin) {
            if (0 == plugin.clsid.length()) return;

            // Ð´CLSID
            std::wstring strSubKey = strKey + L"CLSID\\";
            strSubKey += plugin.clsid + L"\\";
            CRegInternal::SetValue(hStartKey, strSubKey, L"", plugin.description);

            std::wstring strNextKey;
            strNextKey = strSubKey + L"Control\\";
            CRegInternal::SetValue(hStartKey, strNextKey);

            strNextKey = strSubKey + L"InprocServer32\\";
            CRegInternal::SetValue(hStartKey, strNextKey, L"", plugin.pluginPath);
            CRegInternal::SetValue(hStartKey, strNextKey, L"ThreadingModel", L"Apartment");

            strNextKey = strSubKey + L"MiscStatus\\";
            CRegInternal::SetValue(hStartKey, strNextKey, L"", L"0");
            strNextKey += L"1\\";
            CRegInternal::SetValue(hStartKey, strNextKey, L"", plugin.miscStatus);

            strNextKey = strSubKey + L"ProgID\\";
            CRegInternal::SetValue(hStartKey, strNextKey, L"", plugin.progID);

            strNextKey = strSubKey + L"ToolboxBitmap32\\";
            std::wstring strToolBoxBitMap32 = plugin.pluginPath + L", ";
            strToolBoxBitMap32 += plugin.toolboxBitmap32;
            CRegInternal::SetValue(hStartKey, strNextKey, L"", strToolBoxBitMap32);

            strNextKey = strSubKey + L"TypeLib\\";
            CRegInternal::SetValue(hStartKey, strNextKey, L"", plugin.typeLib);

            strNextKey = strSubKey + L"Version\\";
            CRegInternal::SetValue(hStartKey, strNextKey, L"", L"1.0");

            strNextKey = strSubKey + L"VersionIndependentProgID\\";
            CRegInternal::SetValue(hStartKey, strNextKey, L"", plugin.versionIndependentProgID);

            strNextKey = strSubKey + L"Register\\";
            CRegInternal::SetValue(hStartKey, strNextKey, L"", L"soso.la");

            // Ð´ProgID
            strSubKey = strKey + plugin.versionIndependentProgID + L"\\";
            CRegInternal::SetValue(hStartKey, strSubKey, L"", plugin.description);

            strNextKey = strSubKey + L"CurVer\\";
            CRegInternal::SetValue(hStartKey, strNextKey, L"", plugin.progID);

            strNextKey = strSubKey + L"CLSID\\";
            CRegInternal::SetValue(hStartKey, strNextKey, L"", plugin.clsid);

            strNextKey = strSubKey + L"Register\\";
            CRegInternal::SetValue(hStartKey, strNextKey, L"", L"ANT");

            strSubKey = strKey + plugin.progID + L"\\";
            CRegInternal::SetValue(hStartKey, strSubKey, L"", plugin.description);

            strNextKey = strSubKey + L"CLSID\\";
            CRegInternal::SetValue(hStartKey, strNextKey, L"", plugin.clsid);

            strNextKey = strSubKey + L"Register\\";
            CRegInternal::SetValue(hStartKey, strNextKey, L"", L"ANT");
        }
    };

    if( !_plugin.is_lawful_ie() ) {
        return;
    }

    std::wstring strSubKey = L"";
    CRegTool::Register(HKEY_CLASSES_ROOT, strSubKey, _plugin);

    strSubKey = L"Wow6432Node\\";
    CRegTool::Register(HKEY_CLASSES_ROOT, strSubKey, _plugin);
}


void CAxRegistryWorker::UnRegisterCompatibility() {
    if (0 == _plugin.clsid.length()) return;

    std::wstring strSubKey = L"SOFTWARE\\Microsoft\\Internet Explorer\\ActiveX Compatibility\\";
    strSubKey += _plugin.clsid + L"\\";
    CRegInternal::DelKey(HKEY_LOCAL_MACHINE, strSubKey);

    strSubKey = L"SOFTWARE\\Wow6432Node\\Microsoft\\Internet Explorer\\ActiveX Compatibility\\";
    strSubKey += _plugin.clsid + L"\\";
    CRegInternal::DelKey(HKEY_LOCAL_MACHINE, strSubKey);
}

void CAxRegistryWorker::UnRegisterSafety() {
    if (0 == _plugin.clsid.length()) return;

    std::wstring strSubKey = L"CLSID\\" + _plugin.clsid + L"\\Implemented Categories\\";
    CRegInternal::DelKey(HKEY_CLASSES_ROOT, strSubKey);

    std::wstring strSub64Key = L"Wow6432Node\\" + strSubKey;
    CRegInternal::DelKey(HKEY_CLASSES_ROOT, strSub64Key);
}

void CAxRegistryWorker::UnRegisterAllowedDomains() {
    if( 0 == _plugin.clsid.length() ) return;
    std::wstring strPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Ext\\Stats\\";
    CRegInternal::DelKey(HKEY_CURRENT_USER, strPath, _plugin.clsid);
    CRegInternal::DelKey(HKEY_LOCAL_MACHINE, strPath, _plugin.clsid);
}

void CAxRegistryWorker::UnRegisterMime() {
    if (0 == _plugin.MozillaPlugins.size()) return;

    std::vector<MozillaPlugin>::iterator pit = _plugin.MozillaPlugins.begin();
    for (; pit != _plugin.MozillaPlugins.end(); ++pit) {
        if (0 == pit->mimeName.length()) return;

        std::wstring strRootPath = L"MIME\\Database\\Content Type\\";
        CRegInternal::DelKey(HKEY_CLASSES_ROOT, strRootPath, pit->mimeName);
    }
}

void CAxRegistryWorker::UnRegisterMozillaPlugins() {
    if (0 == _plugin.MozillaPlugins.size()) return;

    std::vector<MozillaPlugin>::iterator pit = _plugin.MozillaPlugins.begin();
    for (; pit != _plugin.MozillaPlugins.end(); ++pit) {
        if (0 == pit->mimeName.length()) return;

        CRegInternal::DelKey(HKEY_CURRENT_USER, L"SOFTWARE\\MozillaPlugins\\", pit->mimeName);
        CRegInternal::DelKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\MozillaPlugins\\", pit->mimeName);
    }
}


void CAxRegistryWorker::UnRegisterPlugins() {
    if (0 == _plugin.clsid.length()) return;

    std::wstring strSubKey = L"CLSID\\";
    CRegInternal::DelKey(HKEY_CLASSES_ROOT, strSubKey, _plugin.clsid);

    strSubKey = L"Wow6432Node\\CLSID\\";
    CRegInternal::DelKey(HKEY_CLASSES_ROOT, strSubKey, _plugin.clsid);
} 
