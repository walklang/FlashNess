#pragma once
#include <string>
#include <vector>
#include "PerUserRegistration.h"

class CAxRegistryWorker {
public:
    typedef struct __MOZILLAPLUGIN {
        std::wstring listName;
        std::wstring mimeName;
        std::wstring productName;
        std::wstring pluginPath;
    }MozillaPlugin, *PtrMozillaPlugin;

    typedef struct __PLUGINS {
        bool is_lawful_data() {
            if(0 < clsid.length()) return true;
            return false;
        }

        bool is_lawful_ie() {
            if( 0 == clsid.length() || 0 == description.length() || 0 == pluginPath.length() || 0== progID.length() ||
                0 == miscStatus.length() || 0 == toolboxBitmap32.length() || 0 == typeLib.length() || 0 == versionIndependentProgID.length() ) {
                return false;
            }
            return true;
        }

        bool is_lawful_mozilla() {
            if( 0 == MozillaPlugins.size() || 0 == clsid.length() ) {
                return false;
            }
            return true;
        }

        std::wstring clsid;
        std::wstring description;
        std::wstring pluginPath;
        std::wstring miscStatus;
        std::wstring progID;
        std::wstring toolboxBitmap32;
        std::wstring typeLib;
        std::wstring versionIndependentProgID;
        std::vector<MozillaPlugin> MozillaPlugins;

    }Plugin, *PtrPlugin;

public:
    CAxRegistryWorker(const Plugin& plugin, bool reg = true);

    ~CAxRegistryWorker() {}

private:
    void Register();
    void Unregister();


private:
    void RegisterCompatibility();
    void RegisterSafety();
    void RegisterAllowedDomains();
    void RegisterMime();
    void RegisterMozillaPlugins();
    void RegisterPlugins();

private:
    void UnRegisterCompatibility();
    void UnRegisterSafety();
    void UnRegisterAllowedDomains();
    void UnRegisterMime();
    void UnRegisterMozillaPlugins();
    void UnRegisterPlugins();

private:
    Plugin _plugin;
};


class CAxRegistration
{
public:
    CAxRegistration();
    ~CAxRegistration(){}
public:
    void Register();
    void UnRegister();
private:
    CAxRegistryWorker::Plugin FlashNessPlugin;
};

#define AxRegisterServer()\
{\
    CAxRegistration _RegModule;\
    _RegModule.Register();\
}

#define AxUnregisterServer()\
{\
    CAxRegistration _RegModule;\
    _RegModule.UnRegister();\
}
