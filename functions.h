#pragma once
#include "Color.h"

extern void *Con_ColorPrintf;

typedef void __cdecl Con_ColorPrintf_t(const Color& clr, const char* fmt, ...);

#define Con_ColorPrintf(clr, ...) \
    reinterpret_cast<Con_ColorPrintf_t*>(Con_ColorPrintf)(clr, __VA_ARGS__)

bool Functions_Load();
