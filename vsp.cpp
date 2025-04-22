#include "functions.h"
#include "hud_basechat_2013.h"
#include "vsp.h"

CSigScan *client, *engine;
ICvar* s_pCVar = nullptr;

// hl2sdk-episode1
#include "engine/iserverplugin.h"

class VSP_NAME: public IServerPluginCallbacks
{
public:
    // IServerPluginCallbacks
    virtual bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);
    virtual void Unload(void);

    virtual void Pause(void);
    virtual void UnPause(void);

    virtual const char *GetPluginDescription(void);
};

VSP_NAME VSP_SUFFIX(g_);
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(VSP_NAME, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, VSP_SUFFIX(g_));


bool VSP_NAME::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
    client = CSigScan::Create(Sys_GetFactory("client.dll"));
    if (client == nullptr)
    {
        Report("client.dll: CSigScan::Create() failed\n");
        return false;
    }

    engine = CSigScan::Create(Sys_GetFactory("engine.dll"));
    if (engine == nullptr)
    {
        Report("engine.dll: CSigScan::Create() failed\n");
        return false;
    }

    s_pCVar = (ICvar*)interfaceFactory(VENGINE_CVAR_INTERFACE_VERSION, nullptr);
    if (s_pCVar == nullptr)
    {
        Report("Unable to get '" VENGINE_CVAR_INTERFACE_VERSION "' interface\n");
        return false;
    }

    if (!BaseChat_Load())
        return false;

    if (!Functions_Load())
        return false;

    return true;
}

void VSP_NAME::Unload(void)
{
    BaseChat_Unload();
}

void VSP_NAME::Pause(void)
{
    BaseChat_Pause();
}

void VSP_NAME::UnPause(void)
{
    BaseChat_UnPause();
}

const char* VSP_NAME::GetPluginDescription(void)
{
    return TO_STRING(VSP_NAME) " " VSP_VERSION " (" GITHUB_REPOSITORY ")";
}
