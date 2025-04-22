#pragma once

//////////////////////
///// client.dll /////
//////////////////////

// void CBaseHudChat::StartMessageMode(int iMessageModeType)
// 8B 44 24 04 83 EC 0C 56 8B F1
#define CBaseHudChat_StartMessageMode_SIGNATURE "\x8B\x44\x24\x04\x83\xEC\x0C\x56\x8B\xF1"
#define CBaseHudChat_StartMessageMode_MASK "xxxxxxxxxx"

// void CBaseHudChat::StopMessageMode(void)
// 56 8B F1 8B 46 14 57
#define CBaseHudChat_StopMessageMode_SIGNATURE "\x56\x8B\xF1\x8B\x46\x14\x57"
#define CBaseHudChat_StopMessageMode_MASK "xxxxxxx"

// void CBaseHudChatLine::InsertAndColorizeText(wchar_t *buf, int clientIndex)
// 83 EC 14 53 55 56 8B F1 8B 86 ? ? ? ? 57 33 FF
#define CBaseHudChatLine_InsertAndColorizeText_SIGNATURE "\x83\xEC\x14\x53\x55\x56\x8B\xF1\x8B\x86\x2A\x2A\x2A\x2A\x57\x33\xFF"
#define CBaseHudChatLine_InsertAndColorizeText_MASK "xxxxxxxxxx????xxx"


// void RichText::SetText(const char *text)
// 8B 44 24 04 81 EC ? ? ? ? 85 C0 56
#define RichText_SetText_SIGNATURE "\x8B\x44\x24\x04\x81\xEC\x2A\x2A\x2A\x2A\x85\xC0\x56"
#define RichText_SetText_MASK "xxxxxx????xxx"

// void RichText::InsertString(const char *text)
// 55 8B EC 81 EC ? ? ? ? 53 8B 5D 08
#define RichText_InsertString_SIGNATURE "\x55\x8B\xEC\x81\xEC\x2A\x2A\x2A\x2A\x53\x8B\x5D\x08"
#define RichText_InsertString_MASK "xxxxx????xxxx"

// void RichText::InsertString(const wchar_t *wszText)
// 8B 44 24 04 66 83 38 00 53
#define RichText_InsertString2_SIGNATURE "\x8B\x44\x24\x04\x66\x83\x38\x00\x53"
#define RichText_InsertString2_MASK "xxxxxxxxx"

// void RichText::InsertColorChange(Color col)
// 83 EC 20 53 8B 5C 24 28 56
#define RichText_InsertColorChange_SIGNATURE "\x83\xEC\x20\x53\x8B\x5C\x24\x28\x56"
#define RichText_InsertColorChange_MASK "xxxxxxxxx"

// void RichText::InsertFade(float flSustain, float flLength)
// 83 EC 20 53 56 57 8B F9 8B 87 ? ? ? ? 8B 8F ? ? ? ? 8D 9F ? ? ? ? C1 E0 05 8B 54 08 FC
#define RichText_InsertFade_SIGNATURE "\x83\xEC\x20\x53\x56\x57\x8B\xF9\x8B\x87\x2A\x2A\x2A\x2A\x8B\x8F\x2A\x2A\x2A\x2A\x8D\x9F\x2A\x2A\x2A\x2A\xC1\xE0\x05\x8B\x54\x08\xFC"
#define RichText_InsertFade_MASK "xxxxxxxxxx????xx????xx????xxxxxxx"


//////////////////////
///// engine.dll /////
//////////////////////

// void Con_ColorPrintf(const Color& clr, const char *fmt, ...)
// B8 ? ? ? ? E8 56 1A 23 00
#define Con_ColorPrintf_SIGNATURE "\xB8\x2A\x2A\x2A\x2A\xE8\x56\x1A\x23\x00"
#define Con_ColorPrintf_MASK "x????xxxxx"
