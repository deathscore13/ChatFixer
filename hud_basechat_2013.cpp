#include "functions.h"
#include "hooks.h"
#include "hud_basechat_2013.h"
#include "signatures.h"
#include "vsp.h"

// hl2sdk-episode1
#include "strtools.cpp"

void *StartMessageMode, *StopMessageMode, *InsertAndColorizeText,
    *SetText, *InsertString, *InsertString2, *InsertColorChange, *InsertFade;
CHook *StartMessageMode_h = nullptr, *StopMessageMode_h = nullptr, *InsertAndColorizeText_h = nullptr;
ConVar *con_enable = nullptr, *hud_saytext_time = nullptr;
float con_enable_buffer = 0.0;

void con_enable_callback(ConVar *var, char const *pOldString);

bool BaseChat_Load()
{
    CSigScan_Find(client, CBaseHudChat_StartMessageMode, StartMessageMode);
    CSigScan_Find(client, CBaseHudChat_StopMessageMode, StopMessageMode);

    CSigScan_Find(client, CBaseHudChatLine_InsertAndColorizeText, InsertAndColorizeText);
    CSigScan_Find(client, RichText_SetText, SetText);
    CSigScan_Find(client, RichText_InsertString, InsertString);
    CSigScan_Find(client, RichText_InsertString2, InsertString2);
    CSigScan_Find(client, RichText_InsertColorChange, InsertColorChange);
    CSigScan_Find(client, RichText_InsertFade, InsertFade);

    StartMessageMode_h = new CHook(CBaseHudChat__StartMessageMode_h, StartMessageMode);
    if (!StartMessageMode_h->Hook())
    {
        Report("!StartMessageMode_h->Hook()\n");
        return false;
    }

    StopMessageMode_h = new CHook(CBaseHudChat__StopMessageMode_h, StopMessageMode);
    if (!StopMessageMode_h->Hook())
    {
        Report("!StopMessageMode_h->Hook()\n");
        return false;
    }

    InsertAndColorizeText_h = new CHook(CBaseHudChatLine__InsertAndColorizeText_h, InsertAndColorizeText);
    if (!InsertAndColorizeText_h->Hook())
    {
        Report("!InsertAndColorizeText_h->Hook()\n");
        return false;
    }

    con_enable = s_pCVar->FindVar("con_enable");
    if (con_enable == nullptr)
    {
        Report("ConVar 'con_enable' not found\n");
        return false;
    }

    con_enable->InstallChangeCallback(con_enable_callback);
    con_enable_buffer = con_enable->GetFloat();

    return true;
}

void BaseChat_Unload()
{
    if (StartMessageMode_h)
    {
        delete StartMessageMode_h;
        StartMessageMode_h = nullptr;
    }

    if (StopMessageMode_h)
    {
        delete StopMessageMode_h;
        StopMessageMode_h = nullptr;
    }

    if (InsertAndColorizeText_h)
    {
        delete InsertAndColorizeText_h;
        InsertAndColorizeText_h = nullptr;
    }
}

void BaseChat_Pause()
{
    if (StartMessageMode_h->IsHooked() && !StartMessageMode_h->Unhook())
        Report("Pause: !StartMessageMode_h->Unhook()\n");

    if (StopMessageMode_h->IsHooked() && !StopMessageMode_h->Unhook())
        Report("Pause: !StopMessageMode_h->Unhook()\n");

    if (InsertAndColorizeText_h->IsHooked() && !InsertAndColorizeText_h->Unhook())
        Report("Pause: !InsertAndColorizeText_h->Unhook()\n");
}

void BaseChat_UnPause()
{
    if (!StartMessageMode_h->IsHooked() && !StartMessageMode_h->Hook())
        Report("UnPause: !StartMessageMode_h->Hook()\n");

    if (!StopMessageMode_h->IsHooked() && !StopMessageMode_h->Hook())
        Report("UnPause: !StopMessageMode_h->Hook()\n");

    if (!InsertAndColorizeText_h->IsHooked() && !InsertAndColorizeText_h->Hook())
        Report("UnPause: !InsertAndColorizeText_h->Hook()\n");

    con_enable_buffer = con_enable->GetFloat();
}

// https://gitlab.com/evilcrydie/SourceEngine2007/-/blob/master/src_main/game/client/hud_basechat.cpp#L1130
void __fastcall CBaseHudChat__StartMessageMode_h(void* ecx, void* edx, int iMessageModeType)
{
    StartMessageMode_h->Unhook();

    con_enable_buffer = con_enable->GetFloat();
    con_enable->SetValue(0);

    CBaseHudChat__StartMessageMode(ecx, edx, iMessageModeType);

    StartMessageMode_h->Hook();
}

// https://gitlab.com/evilcrydie/SourceEngine2007/-/blob/master/src_main/game/client/hud_basechat.cpp#L1182
void __fastcall CBaseHudChat__StopMessageMode_h(void* ecx, void* edx)
{
    StopMessageMode_h->Unhook();

    con_enable->SetValue(con_enable_buffer);

    CBaseHudChat__StopMessageMode(ecx, edx);

    StopMessageMode_h->Hook();
}


// https://gitlab.com/evilcrydie/source-sdk-2013/-/blob/main/src/game/client/hud_basechat.cpp#L1403
void __fastcall CBaseHudChatLine__InsertAndColorizeText_h(void* ecx, void* edx, wchar_t* buf, int clientIndex)
{
    // create property
    int& m_iNameLength = __property<int>(ecx, 93);
    int& m_iNameStart = __property<int>(ecx, 104);


    wchar_t* m_text = CloneWString(buf);
    CUtlVector<TextRange> m_textRanges;
    m_textRanges.RemoveAll();

    CBaseHudChat* pChat = dynamic_cast<CBaseHudChat*>(CBaseHudChat__GetParent(ecx));
    if (pChat == nullptr)
    {
        delete[] m_text;
        return;
    }

    wchar_t* txt = m_text;
    int lineLen = wcslen(m_text);
    Color colCustom;
    if (m_text[0] == COLOR_PLAYERNAME || m_text[0] == COLOR_LOCATION ||
        m_text[0] == COLOR_NORMAL || m_text[0] == COLOR_ACHIEVEMENT ||
        m_text[0] == COLOR_CUSTOM || m_text[0] == COLOR_HEXCODE ||
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
                const int nCodeBytes = (bReadAlpha?8:6);
                range.start = nBytesIn + nCodeBytes + 1;
                range.end = lineLen;
                range.preserveAlpha = bReadAlpha;
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

    if (!m_textRanges.Count() && m_iNameLength > 0 &&
        m_text[0] == COLOR_USEOLDCOLORS)
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
        // range.color = pChat->GetTextColorForClient(COLOR_NORMAL, clientIndex);
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
    Colorize(pChat, m_text, m_textRanges);
}

// https://gitlab.com/evilcrydie/source-sdk-2013/-/blob/main/src/game/client/hud_basechat.cpp#L1547
void Colorize(CBaseHudChat* pChat, wchar_t* m_text, CUtlVector<TextRange>& m_textRanges, int alpha)
{
    // clear out text
    // SetText("");
    //RichText_SetText(ecx, "");

    //CBaseHudChat* pChat = dynamic_cast<CBaseHudChat*>(CBaseHudChat__GetParent(ecx));

    // pChat->GetChatHistory()
    void* m_pChatHistory = nullptr;

    if (pChat && (m_pChatHistory = *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(pChat) + 252)))
    {
        // pChat->GetChatHistory()->InsertString("\n");
        RichText__InsertString(m_pChatHistory, "\n");
    }

    wchar_t wText[4096];
    Color color;
    for (int i = 0; i < m_textRanges.Count(); ++i)
    {
        wchar_t* start = m_text + m_textRanges[i].start;
        int len = m_textRanges[i].end - m_textRanges[i].start + 1;
        if (len > 1 && len <= ARRAYSIZE(wText))
        {
            wcsncpy(wText, start, len);
            wText[len - 1] = 0;
            color = m_textRanges[i].color;
            if (!m_textRanges[i].preserveAlpha)
            {
                color[3] = alpha;
            }

            // InsertColorChange(color);
            //RichText_InsertColorChange(ecx, color);
            // InsertString(wText);
            //RichText_InsertString2(ecx, wText);
            Con_ColorPrintf(color, "%s", WideToANSI(wText).c_str());

            if (pChat && m_pChatHistory)
            {
                // pChat->GetChatHistory()->InsertColorChange(color);
                RichText__InsertColorChange(m_pChatHistory, color);
                // pChat->GetChatHistory()->InsertString(wText);
                RichText__InsertString2(m_pChatHistory, wText);

                if (hud_saytext_time == nullptr)
                {
                    if ((hud_saytext_time = s_pCVar->FindVar("hud_saytext_time")) == nullptr)
                    {
                        Report("ConVar 'hud_saytext_time' not found\n");
                    }
                    else
                    {
                        // pChat->GetChatHistory()->InsertFade(hud_saytext_time.GetFloat(), CHAT_HISTORY_IDLE_FADE_TIME);
                        RichText__InsertFade(m_pChatHistory, hud_saytext_time->GetFloat(),
                            CHAT_HISTORY_IDLE_FADE_TIME);
                    }
                }

                if (i == m_textRanges.Count() - 1)
                {
                    // pChat->GetChatHistory()->InsertFade(-1, -1);
                    RichText__InsertFade(m_pChatHistory, -1, -1);
                }
            }

        }
    }
    Msg("\n");

    // InvalidateLayout(true);
    // https://gitlab.com/evilcrydie/source-sdk-2013/-/blob/main/src/public/vgui_controls/Panel.h#L323
    //RichText__InvalidateLayout(ecx, true, false);
    delete[] m_text;
}

void con_enable_callback(ConVar *var, char const *pOldString)
{
    if (!StartMessageMode_h || !StopMessageMode_h ||
        !(StartMessageMode_h->IsHooked() && StopMessageMode_h->IsHooked()))
        return;

    con_enable_buffer = var->GetFloat();
}
