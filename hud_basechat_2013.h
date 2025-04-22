#pragma once
// hl2sdk-episode1
#include "hud_basechat.h"

// https://gitlab.com/evilcrydie/source-sdk-2013/-/blob/main/src/game/client/hud_basechat.h#L38
#define CHAT_HISTORY_IDLE_FADE_TIME 2.5f

// https://gitlab.com/evilcrydie/source-sdk-2013/-/blob/main/src/game/client/hud_basechat.h#L86
struct TextRange
{
	int start = 0;
	int end = 0;
	Color color = {255, 255, 255};
	bool preserveAlpha = false;
};

// https://gitlab.com/evilcrydie/source-sdk-2013/-/blob/main/src/game/client/hud_basechat.h#L72
enum TextColor
{
	COLOR_NORMAL = 1,
	COLOR_USEOLDCOLORS = 2,
	COLOR_PLAYERNAME = 3,
	COLOR_LOCATION = 4,
	COLOR_ACHIEVEMENT = 5,
	COLOR_CUSTOM = 6,		// Will use the most recently SetCustomColor()
	COLOR_HEXCODE = 7,		// Reads the color from the next six characters
	COLOR_HEXCODE_ALPHA = 8,// Reads the color and alpha from the next eight characters
	COLOR_MAX
};

// https://gitlab.com/evilcrydie/source-sdk-2013/-/blob/main/src/game/client/hud_basechat.h#L108
inline wchar_t* CloneWString(const wchar_t* str)
{
	const int nLen = V_wcslen(str) + 1;
	wchar_t* cloneStr = new wchar_t[nLen];
	const int nSize = nLen * sizeof(wchar_t);
	V_wcsncpy(cloneStr, str, nSize);
	return cloneStr;
}

#include <Windows.h>
#include <string>

// FIXME: Con_ColorPrintf
inline std::string WideToANSI(const wchar_t* wstr)
{
	int size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
	std::string buffer(size, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &buffer[0], size, nullptr, nullptr);
	return buffer;
}


extern void *StartMessageMode, *StopMessageMode, *InsertAndColorizeText,
	*SetText, *InsertString, *InsertString2, *InsertColorChange, *InsertFade;

typedef void __fastcall CBaseHudChat__StartMessageMode_t(void* ecx, void* edx, int iMessageModeType);
typedef void __fastcall CBaseHudChat__StopMessageMode_t(void* ecx, void* edx);

typedef void __thiscall RichText_SetText_t(void* ecx, const char* text);
typedef void __thiscall RichText_InsertString_t(void* ecx, const char* text);
typedef void __thiscall RichText_InsertString2_t(void* ecx, const wchar_t* wszText);
typedef void __thiscall RichText_InsertColorChange_t(void *ecx, Color col);
typedef void __thiscall RichText_InsertFade_t(void* ecx, float flSustain, float flLength);


void __fastcall CBaseHudChat__StartMessageMode_h(void* ecx, void* edx, int iMessageModeType);
void __fastcall CBaseHudChat__StopMessageMode_h(void* ecx, void* edx);
void __fastcall CBaseHudChatLine__InsertAndColorizeText_h(void* ecx, void* edx, wchar_t* buf, int clientIndex);

// https://gitlab.com/evilcrydie/source-sdk-2013/-/blob/main/src/game/client/hud_basechat.h#L148
void Colorize(CBaseHudChat* pChat, wchar_t* m_text, CUtlVector<TextRange>& m_textRanges, int alpha = 255);


#include "class_hack.h"

// original call aliases
#define CBaseHudChat__StartMessageMode(ecx, edx, iMessageModeType) \
    reinterpret_cast<CBaseHudChat__StartMessageMode_t*>(StartMessageMode)(ecx, edx, iMessageModeType)
#define CBaseHudChat__StopMessageMode(ecx, edx) \
    reinterpret_cast<CBaseHudChat__StopMessageMode_t*>(StopMessageMode)(ecx, edx)
#define RichText__SetText(ptr, text) \
    reinterpret_cast<RichText_SetText_t*>(SetText)(ptr, text)
#define RichText__InsertString(ptr, text) \
    reinterpret_cast<RichText_InsertString_t*>(InsertString)(ptr, text)
#define RichText__InsertString2(ptr, text) \
    reinterpret_cast<RichText_InsertString2_t*>(InsertString2)(ptr, text)
#define RichText__InsertColorChange(ptr, col) \
    reinterpret_cast<RichText_InsertColorChange_t*>(InsertColorChange)(ptr, col)
#define RichText__InsertFade(ptr, flSustain, flLength) \
    reinterpret_cast<RichText_InsertFade_t*>(InsertFade)(ptr, flSustain, flLength)

#define CBaseHudChat__GetParent(ptr) \
    __call<vgui::Panel* (__thiscall*)(void*)>(ptr, 144)(ptr)
// return Color --> edx
#define CBaseHudChat__GetTextColorForClient(ptr, color, colorNum, clientIndex) \
    __call<void (__thiscall*)(void*, Color&, TextColor, int)>(ptr, 88)(ptr, color, colorNum, clientIndex)
#define RichText__InvalidateLayout(ptr, layoutNow, reloadScheme) \
    __call<void (__thiscall*)(void*, bool, bool)>(ptr, 240)(ptr, layoutNow, reloadScheme)


bool BaseChat_Load();
void BaseChat_Unload();
void BaseChat_Pause();
void BaseChat_UnPause();
