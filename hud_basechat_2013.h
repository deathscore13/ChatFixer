// https://gitlab.com/evilcrydie/SourceEngine2007/-/blob/master/src_main/game/client/hud_basechat.h#L73
struct TextRange
{
	int start;
	int end;
	Color color;
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

// https://gitlab.com/evilcrydie/SourceEngine2007/-/blob/master/src_main/game/client/hud_basechat.cpp#L52
inline wchar_t* CloneWString(const wchar_t* str)
{
	wchar_t* cloneStr = new wchar_t[wcslen(str) + 1];
	wcscpy(cloneStr, str);
	return cloneStr;
}
