#pragma comment(lib, "legacy_stdio_definitions.lib")
#pragma warning(disable: 4005)

#define VSP_VERSION "2.0.1"

// hl2sdk-episode1
#include "engine/iserverplugin.h"
#include "icvar.h"

// hud_basechat
#include "hud_basechat.h"
#include "hud_basechat_2013.h"
#include "strtools.cpp"

#include "sigscan.h"
#include "hooks.h"


// void CBaseHudChat::StartMessageMode(int iMessageModeType)
#define CBaseHudChat_StartMessageMode_SIGNATURE "\x8B\x44\x24\x04\x83\xEC\x0C\x56\x8B\xF1"
#define CBaseHudChat_StartMessageMode_MASK "xxxxxxxxxx"

// void CBaseHudChat::StopMessageMode(void)
#define CBaseHudChat_StopMessageMode_SIGNATURE "\x56\x8B\xF1\x8B\x46\x14\x57"
#define CBaseHudChat_StopMessageMode_MASK "xxxxxxx"

// void CBaseHudChatLine::InsertAndColorizeText(wchar_t *buf, int clientIndex)
#define CBaseHudChatLine_InsertAndColorizeText_SIGNATURE "\x83\xEC\x14\x53\x55\x56\x8B\xF1\x8B\x86\x9C\x01\x00\x00"
#define CBaseHudChatLine_InsertAndColorizeText_MASK "xxxxxxxxxxxxxx"

// call vtable function
template<typename FuncType>
__forceinline static FuncType __call(void* ptr, int offset)
{
    return (FuncType)((*(int**)ptr)[offset / 4]);
}


// hooks
void __fastcall CBaseHudChat__StartMessageMode(void* ecx, void* edx, int iMessageModeType);
void __fastcall CBaseHudChat__StopMessageMode(void* ecx, void* edx);
void __fastcall CBaseHudChatLine__InsertAndColorizeText(DWORD* ecx, void* edx, wchar_t* buf, int clientIndex);

void con_enable_callback(ConVar* var, char const* pOldString);

typedef void __fastcall CBaseHudChat__StartMessageMode_t(void* ecx, void* edx, int iMessageModeType);
typedef void __fastcall CBaseHudChat__StopMessageMode_t(void* ecx, void* edx);
typedef void __fastcall CBaseHudChatLine__InsertAndColorizeText_t(DWORD* ecx, void* edx, wchar_t* buf, int clientIndex);

// calls
#define CBaseHudChat__GetParent(ptr) \
    __call<vgui::Panel* (__thiscall*)(void*)>(ptr, 144)(ptr)
// return Color --> edx
#define CBaseHudChat__GetTextColorForClient(ptr, color, colorNum, clientIndex) \
    __call<void (__thiscall*)(void*, Color&, TextColor, int)>(ptr, 88)(ptr, color, colorNum, clientIndex)
#define CBaseHudChatLine__Colorize(ptr, alpha) \
    __call<void (__thiscall*)(void*, int)>(ptr, 780)(ptr, alpha)


class ChatFixer: public IServerPluginCallbacks
{
public:
    // IServerPluginCallbacks
    virtual bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);
    virtual void Unload(void);

    virtual void Pause(void);
    virtual void UnPause(void);

    virtual const char* GetPluginDescription(void);
};


CSigScan StartMessageMode, StopMessageMode, InsertAndColorizeText;
CHook *StartMessageMode_h, *StopMessageMode_h, *InsertAndColorizeText_h;

ICvar* s_pCVar;

ConVar *con_enable;
float con_enable_buffer;

// CBaseHudChatLine::InsertAndColorizeText ecx
DWORD* InsertAndColorizeText_ecx = NULL;

ChatFixer g_ChatFixer;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(ChatFixer, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_ChatFixer);


bool ChatFixer::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
    CreateInterfaceFn clientFactory = Sys_GetFactory("client.dll");
    if (clientFactory == NULL)
    {
        Error("client.dll: Sys_GetFactory() failed\n");
        return false;
    }

    // client.dll

    CSigScan::sigscan_dllfunc = clientFactory;
    if (!CSigScan::GetDllMemInfo())
    {
        Error("client.dll: CSigScan::GetDllMemInfo() failed\n");
        return false;
    }

    StartMessageMode.Init((unsigned char*)CBaseHudChat_StartMessageMode_SIGNATURE,
        (unsigned char*)CBaseHudChat_StartMessageMode_MASK);
    if (!StartMessageMode.is_set)
    {
        Error("Unable to get CBaseHudChat::StartMessageMode() address\n");
        return false;
    }

    StopMessageMode.Init((unsigned char*)CBaseHudChat_StopMessageMode_SIGNATURE,
        (unsigned char*)CBaseHudChat_StopMessageMode_MASK);
    if (!StopMessageMode.is_set)
    {
        Error("Unable to get CBaseHudChat::StopMessageMode() address\n");
        return false;
    }

    InsertAndColorizeText.Init((unsigned char*)CBaseHudChatLine_InsertAndColorizeText_SIGNATURE,
        (unsigned char*)CBaseHudChatLine_InsertAndColorizeText_MASK);
    if (!InsertAndColorizeText.is_set)
    {
        Error("Unable to get CBaseHudChatLine::InsertAndColorizeText() address\n");
        return false;
    }

    // end client.dll

    StartMessageMode_h = new CHook((DWORD)CBaseHudChat__StartMessageMode, StartMessageMode.sig_addr);
    StartMessageMode_h->Hook();

    StopMessageMode_h = new CHook((DWORD)CBaseHudChat__StopMessageMode, StopMessageMode.sig_addr);
    StopMessageMode_h->Hook();

    InsertAndColorizeText_h = new CHook((DWORD)CBaseHudChatLine__InsertAndColorizeText, InsertAndColorizeText.sig_addr);
    InsertAndColorizeText_h->Hook();

    s_pCVar = (ICvar*)interfaceFactory(VENGINE_CVAR_INTERFACE_VERSION, NULL);
    if (s_pCVar == NULL)
    {
        Error("Unable to get '" VENGINE_CVAR_INTERFACE_VERSION "' interface\n");
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

void ChatFixer::Unload(void)
{
    if (StartMessageMode_h)
        delete StartMessageMode_h;

    if (StopMessageMode_h)
        delete StopMessageMode_h;

    if (InsertAndColorizeText_h)
        delete InsertAndColorizeText_h;

    if (InsertAndColorizeText_ecx == NULL)
        return;

    // delete[] CBaseHudChatLine::m_text
    wchar_t*& m_text = *reinterpret_cast<wchar_t**>(&InsertAndColorizeText_ecx[103]);
    if (m_text)
    {
        delete[] m_text;
        m_text = NULL;
    }
    InsertAndColorizeText_ecx = NULL;
}

void ChatFixer::Pause(void)
{
    if (StartMessageMode_h->IsHooked())
        StartMessageMode_h->Unhook();

    if (StopMessageMode_h->IsHooked())
        StopMessageMode_h->Unhook();

    if (InsertAndColorizeText_h->IsHooked())
        InsertAndColorizeText_h->Unhook();

    if (InsertAndColorizeText_ecx == NULL)
        return;

    // delete[] CBaseHudChatLine::m_text
    wchar_t*& m_text = *reinterpret_cast<wchar_t**>(&InsertAndColorizeText_ecx[103]);
    if (m_text)
    {
        delete[] m_text;
        m_text = NULL;
    }
    InsertAndColorizeText_ecx = NULL;
}

void ChatFixer::UnPause(void)
{
    if (!StartMessageMode_h->IsHooked())
        StartMessageMode_h->Hook();

    if (!StopMessageMode_h->IsHooked())
        StopMessageMode_h->Hook();

    if (!InsertAndColorizeText_h->IsHooked())
        InsertAndColorizeText_h->Hook();

    con_enable_buffer = con_enable->GetFloat();
}

const char* ChatFixer::GetPluginDescription(void)
{
    return "ChatFixer " VSP_VERSION " (https://github.com/deathscore13/ChatFixer)";
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

// https://gitlab.com/evilcrydie/source-sdk-2013/-/blob/main/src/game/client/hud_basechat.cpp#L1403
void __fastcall CBaseHudChatLine__InsertAndColorizeText(DWORD* ecx, void* edx, wchar_t* buf, int clientIndex)
{
    // property
    wchar_t*& m_text = *reinterpret_cast<wchar_t**>(&ecx[103]);
    CUtlVector<TextRange>& m_textRanges = *reinterpret_cast<CUtlVector<TextRange>*>(ecx + 98);
    int& m_iNameLength = *reinterpret_cast<int*>(&ecx[93]);
    int& m_iNameStart = *reinterpret_cast<int*>(&ecx[104]);


    if (m_text)
    {
        delete[] m_text;
        m_text = NULL;
    }
    m_textRanges.RemoveAll();

    m_text = CloneWString(buf);

    // addon stopped? delete[] m_text
    InsertAndColorizeText_ecx = ecx;

    CBaseHudChat* pChat = dynamic_cast<CBaseHudChat*>(CBaseHudChat__GetParent(ecx));

    if (pChat == NULL)
        return;

    wchar_t* txt = m_text;
    int lineLen = wcslen(m_text);
    Color colCustom;
    if (m_text[0] == COLOR_PLAYERNAME || m_text[0] == COLOR_LOCATION || m_text[0] == COLOR_NORMAL ||
        m_text[0] == COLOR_ACHIEVEMENT || m_text[0] == COLOR_CUSTOM || m_text[0] == COLOR_HEXCODE ||
        m_text[0] == COLOR_HEXCODE_ALPHA)
    {
        while (txt && *txt)
        {
            TextRange range;
            bool bFoundColorCode = false;
            bool bDone = false;
            int nBytesIn = txt - m_text;

            switch (*txt)
            {
            case COLOR_CUSTOM:
            case COLOR_PLAYERNAME:
            case COLOR_LOCATION:
            case COLOR_ACHIEVEMENT:
            case COLOR_NORMAL:
            {
                // save this start
                range.start = nBytesIn + 1;
                //range.color = pChat->GetTextColorForClient((TextColor)(*txt), clientIndex);
                CBaseHudChat__GetTextColorForClient(pChat, range.color, (TextColor)(*txt), clientIndex);
                range.end = lineLen;
                bFoundColorCode = true;
            }
            ++txt;
            break;
            case COLOR_HEXCODE:
            case COLOR_HEXCODE_ALPHA:
            {
                bool bReadAlpha = (*txt == COLOR_HEXCODE_ALPHA);
                const int nCodeBytes = (bReadAlpha ? 8 : 6);
                range.start = nBytesIn + nCodeBytes + 1;
                range.end = lineLen;
                //range.preserveAlpha = bReadAlpha;
                ++txt;

                if (range.end > range.start)
                {
                    int r = V_nibble(txt[0]) << 4 | V_nibble(txt[1]);
                    int g = V_nibble(txt[2]) << 4 | V_nibble(txt[3]);
                    int b = V_nibble(txt[4]) << 4 | V_nibble(txt[5]);
                    int a = 255;

                    if (bReadAlpha)
                    {
                        a = V_nibble(txt[6]) << 4 | V_nibble(txt[7]);
                    }

                    range.color = Color(r, g, b, a);
                    bFoundColorCode = true;

                    txt += nCodeBytes;
                }
                else
                {
                    // Not enough characters remaining for a hex code. Skip the rest of the string.
                    bDone = true;
                }
            }
            break;
            default:
                ++txt;
            }

            if (bDone)
            {
                break;
            }

            if (bFoundColorCode)
            {
                int count = m_textRanges.Count();
                if (count)
                {
                    m_textRanges[count - 1].end = nBytesIn;
                }

                m_textRanges.AddToTail(range);
            }
        }
    }

    if (!m_textRanges.Count() && m_iNameLength > 0 && m_text[0] == COLOR_USEOLDCOLORS)
    {
        TextRange range;
        range.start = 0;
        range.end = m_iNameStart;
        //range.color = pChat->GetTextColorForClient(COLOR_NORMAL, clientIndex);
        CBaseHudChat__GetTextColorForClient(pChat, range.color, COLOR_NORMAL, clientIndex);
        m_textRanges.AddToTail(range);

        range.start = m_iNameStart;
        range.end = m_iNameStart + m_iNameLength;
        //range.color = pChat->GetTextColorForClient(COLOR_PLAYERNAME, clientIndex);
        CBaseHudChat__GetTextColorForClient(pChat, range.color, COLOR_PLAYERNAME, clientIndex);
        m_textRanges.AddToTail(range);

        range.start = range.end;
        range.end = wcslen(m_text);
        // range.color = pChat->GetTextColorForClient( COLOR_NORMAL, clientIndex );
        CBaseHudChat__GetTextColorForClient(pChat, range.color, COLOR_NORMAL, clientIndex);
        m_textRanges.AddToTail(range);
    }

    if (!m_textRanges.Count())
    {
        TextRange range;
        range.start = 0;
        range.end = wcslen(m_text);
        // range.color = pChat->GetTextColorForClient( COLOR_NORMAL, clientIndex );
        CBaseHudChat__GetTextColorForClient(pChat, range.color, COLOR_NORMAL, clientIndex);
        m_textRanges.AddToTail(range);
    }

    for (int i = 0; i < m_textRanges.Count(); ++i)
    {
        wchar_t* start = m_text + m_textRanges[i].start;
        if (*start > 0 && *start < COLOR_MAX)
        {
            //Assert(*start != COLOR_HEXCODE && *start != COLOR_HEXCODE_ALPHA);
            m_textRanges[i].start += 1;
        }
    }

    // Colorize();
    CBaseHudChatLine__Colorize(ecx, 255);
}

void con_enable_callback(ConVar* var, char const* pOldString)
{
    if (StartMessageMode_h->IsHooked() && StopMessageMode_h->IsHooked())
        con_enable_buffer = var->GetFloat();
}
