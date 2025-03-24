#pragma comment(lib, "legacy_stdio_definitions.lib")
#pragma warning(disable: 4005)

#include "engine/iserverplugin.h"
#include "sigscan.h"
#include "hooks.h"


// void CBaseHudChat::ChatPrintf(int iPlayerIndex, int iFilter, const char *fmt, ...)
#define HOOK_SIGNATURE "\x55\xB8\xD4\x10\x00\x00"
#define HOOK_MASK "xxxxxx"


void CBaseHudChat__ChatPrintf(void* ecx, int iPlayerIndex, int iFilter, const char* fmt, ...);
void ClearHEX(char* msg);
char ColorSwitch();

typedef void CBaseHudChat__ChatPrintf_t(void* ecx, int iPlayerIndex, int iFilter, const char* fmt, ...);


class VSP: public IServerPluginCallbacks
{
public:
    // IServerPluginCallbacks
    virtual bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);
    virtual void Unload(void);

    virtual void Pause(void);
    virtual void Unpause(void);

    virtual const char* GetPluginDescription(void);
};


CSigScan sig;
CHook *hook;

bool color;

VSP g_VSP;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(VSP, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_VSP);


bool VSP::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
    // client.dll

    CSigScan::sigscan_dllfunc = Sys_GetFactory("client.dll");
    if (!CSigScan::GetDllMemInfo())
    {
        Warning("client.dll: CSigScan::GetDllMemInfo() failed\n");
        return false;
    }

    sig.Init((unsigned char*)HOOK_SIGNATURE, (unsigned char*)HOOK_MASK);
    if (!sig.sig_addr)
    {
        Warning("Unable to get CBaseHudChat::ChatPrintf address\n");
        return false;
    }

    hook = new CHook((DWORD)CBaseHudChat__ChatPrintf, sig.sig_addr);
    hook->Hook();

    // end client.dll
    
    return true;
}

void VSP::Unload(void)
{
    if (hook)
        delete hook;
}

void VSP::Pause(void)
{
    if (hook->Hooked())
        hook->Unhook();
}

void VSP::Unpause(void)
{
    if (!hook->Hooked())
        hook->Hook();
}

const char* VSP::GetPluginDescription(void)
{
    return "HEX-ChatFilter";
}


void CBaseHudChat__ChatPrintf(void* ecx, int iPlayerIndex, int iFilter, const char* fmt, ...)
{
    va_list marker;
    char msg[4096];

    va_start(marker, fmt);
    Q_vsnprintf(msg, sizeof(msg), fmt, marker);
    va_end(marker);

    ClearHEX(msg);

    hook->Unhook();

    reinterpret_cast<CBaseHudChat__ChatPrintf_t*>(sig.sig_addr)(ecx, iPlayerIndex, iFilter, "%s", msg);
    
    hook->Hook();
}

void ClearHEX(char* msg)
{
    int pos = -1, offset, mode;
    color = false;

    while (msg[++pos] != '\0')
    {
        if (msg[pos] == '\x7')
            mode = 6;
        else if (msg[pos] == '\x8')
            mode = 8;
        else
            continue;

        offset = pos + mode;
        while (msg[offset] != '\0')
        {
            msg[offset - mode] = msg[offset];
            offset++;
        }
        msg[pos] = ColorSwitch();
        msg[offset - mode] = '\0';
    }
}

// \x3 and \x4
char ColorSwitch()
{
    color = !color;
    return color + 3;
}
