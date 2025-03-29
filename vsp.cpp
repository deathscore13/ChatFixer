#pragma comment(lib, "legacy_stdio_definitions.lib")
#pragma warning(disable: 4005)

#define VSP_VERSION "1.1.0"

#include "engine/iserverplugin.h"
#include "icvar.h"

#include "sigscan.h"
#include "hooks.h"


// void CBaseHudChat::ChatPrintf(int iPlayerIndex, int iFilter, const char* fmt, ...)
#define CBASEHUDCHAT_CHATPRINTF_SIGNATURE "\x55\xB8\xD4\x10\x00\x00"
#define CBASEHUDCHAT_CHATPRINTF_MASK "xxxxxx"

// void CBaseHudChat::StartMessageMode(int iMessageModeType)
#define CBASEHUDCHAT_StartMessageMode_SIGNATURE "\x8B\x44\x24\x04\x83\xEC\x0C\x56\x8B\xF1"
#define CBASEHUDCHAT_StartMessageMode_MASK "xxxxxxxxxx"

// void CBaseHudChat::StopMessageMode(void)
#define CBASEHUDCHAT_StopMessageMode_SIGNATURE "\x56\x8B\xF1\x8B\x46\x14\x57"
#define CBASEHUDCHAT_StopMessageMode_MASK "xxxxxxx"


void CBaseHudChat__ChatPrintf(void* ecx, int iPlayerIndex, int iFilter, const char* fmt, ...);
void __fastcall CBaseHudChat__StartMessageMode(void* ecx, void* edx, int iMessageModeType);
void __fastcall CBaseHudChat__StopMessageMode(void* ecx, void* edx);

void con_enable_callback(ConVar* var, char const* pOldString);

void ClearHEX(char* msg);
char ColorSwitch();

typedef void CBaseHudChat__ChatPrintf_t(void *ecx, int iPlayerIndex, int iFilter, const char* fmt, ...);
typedef void __fastcall CBaseHudChat__StartMessageMode_t(void* ecx, void* edx, int iMessageModeType);
typedef void __fastcall CBaseHudChat__StopMessageMode_t(void* ecx, void* edx);


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


CreateInterfaceFn clientFactory;

CSigScan ChatPrintf, StartMessageMode, StopMessageMode;
CHook *ChatPrintf_h, *StartMessageMode_h, *StopMessageMode_h;

ICvar* s_pCVar;
ConVar *con_enable;

bool color;
float con_enable_buffer;

VSP g_VSP;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(VSP, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_VSP);


bool VSP::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
    clientFactory = Sys_GetFactory("client.dll");
    if (clientFactory == NULL)
    {
        Error("client.dll: Sys_GetFactory failed\n");
        return false;
    }

    // client.dll

    CSigScan::sigscan_dllfunc = clientFactory;
    if (!CSigScan::GetDllMemInfo())
    {
        Error("client.dll: CSigScan::GetDllMemInfo() failed\n");
        return false;
    }

    ChatPrintf.Init((unsigned char*)CBASEHUDCHAT_CHATPRINTF_SIGNATURE,
        (unsigned char*)CBASEHUDCHAT_CHATPRINTF_MASK);
    if (!ChatPrintf.is_set)
    {
        Error("Unable to get CBaseHudChat::ChatPrintf address\n");
        return false;
    }

    StartMessageMode.Init((unsigned char*)CBASEHUDCHAT_StartMessageMode_SIGNATURE,
        (unsigned char*)CBASEHUDCHAT_StartMessageMode_MASK);
    if (!StartMessageMode.is_set)
    {
        Error("Unable to get CBaseHudChat::StartMessageMode address\n");
        return false;
    }

    StopMessageMode.Init((unsigned char*)CBASEHUDCHAT_StopMessageMode_SIGNATURE,
        (unsigned char*)CBASEHUDCHAT_StopMessageMode_MASK);
    if (!StopMessageMode.is_set)
    {
        Error("Unable to get CBaseHudChat::StopMessageMode address\n");
        return false;
    }

    // end client.dll

    ChatPrintf_h = new CHook((DWORD)CBaseHudChat__ChatPrintf, ChatPrintf.sig_addr);
    ChatPrintf_h->Hook();

    StartMessageMode_h = new CHook((DWORD)CBaseHudChat__StartMessageMode, StartMessageMode.sig_addr);
    StartMessageMode_h->Hook();

    StopMessageMode_h = new CHook((DWORD)CBaseHudChat__StopMessageMode, StopMessageMode.sig_addr);
    StopMessageMode_h->Hook();

    s_pCVar = (ICvar*)interfaceFactory(VENGINE_CVAR_INTERFACE_VERSION, NULL);
    if (s_pCVar == NULL)
    {
        Error("Unable to get " VENGINE_CVAR_INTERFACE_VERSION " interface\n");
        return false;
    }

    con_enable = s_pCVar->FindVar("con_enable");
    if (con_enable == NULL)
    {
        Error("ConVar 'con_enable' not found\n");
        return false;
    }

    con_enable->InstallChangeCallback(con_enable_callback);
    con_enable_buffer = con_enable->GetFloat();
    
    return true;
}

void VSP::Unload(void)
{
    if (ChatPrintf_h)
        delete ChatPrintf_h;

    if (StartMessageMode_h)
        delete StartMessageMode_h;

    if (StopMessageMode_h)
        delete StopMessageMode_h;
}

void VSP::Pause(void)
{
    if (ChatPrintf_h->IsHooked())
        ChatPrintf_h->Unhook();

    if (StartMessageMode_h->IsHooked())
        StartMessageMode_h->Unhook();

    if (StopMessageMode_h->IsHooked())
        StopMessageMode_h->Unhook();
}

void VSP::Unpause(void)
{
    if (!ChatPrintf_h->IsHooked())
        ChatPrintf_h->Hook();

    if (!StartMessageMode_h->IsHooked())
        StartMessageMode_h->Hook();

    if (!StopMessageMode_h->IsHooked())
        StopMessageMode_h->Hook();

    con_enable_buffer = con_enable->GetFloat();
}

const char* VSP::GetPluginDescription(void)
{
    return "HEX-ChatFilter " VSP_VERSION " (https://github.com/deathscore13/HEX-ChatFilter)";
}


void CBaseHudChat__ChatPrintf(void *ecx, int iPlayerIndex, int iFilter, const char* fmt, ...)
{
    va_list marker;
    char msg[4096];

    va_start(marker, fmt);
    Q_vsnprintf(msg, sizeof(msg), fmt, marker);
    va_end(marker);

    ClearHEX(msg);

    ChatPrintf_h->Unhook();
    reinterpret_cast<CBaseHudChat__ChatPrintf_t*>(ChatPrintf.sig_addr)(ecx, iPlayerIndex, iFilter, "%s", msg);
    ChatPrintf_h->Hook();
}

void __fastcall CBaseHudChat__StartMessageMode(void* ecx, void* edx, int iMessageModeType)
{
    StartMessageMode_h->Unhook();

    con_enable_buffer = con_enable->GetFloat();
    con_enable->SetValue(0);

    reinterpret_cast<CBaseHudChat__StartMessageMode_t*>(StartMessageMode.sig_addr)(ecx, edx, iMessageModeType);

    StartMessageMode_h->Hook();
}

void __fastcall CBaseHudChat__StopMessageMode(void* ecx, void* edx)
{
    StopMessageMode_h->Unhook();
    
    con_enable->SetValue(con_enable_buffer);

    reinterpret_cast<CBaseHudChat__StopMessageMode_t*>(StopMessageMode.sig_addr)(ecx, edx);

    StopMessageMode_h->Hook();
}

void con_enable_callback(ConVar* var, char const* pOldString)
{
    if (StartMessageMode_h->IsHooked() && StopMessageMode_h->IsHooked())
        con_enable_buffer = var->GetFloat();
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
