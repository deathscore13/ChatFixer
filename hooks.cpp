#include "hooks.h"

CHook::CHook(void* func, void* addr)
{
    address = addr;

#ifdef _WIN64
    memcpy(&jmp[2], &func, 8);
#else
    uintptr_t offset = reinterpret_cast<uintptr_t>(func) - (reinterpret_cast<uintptr_t>(addr) + 5);
    memcpy(&jmp[1], &offset, 4);
#endif
}

CHook::~CHook()
{
    Unhook();
}

bool CHook::Hook()
{
    if (hooked)
        return true;
    
    DWORD oldProtect;
    if (!VirtualProtect(address, sizeof(jmp), PAGE_EXECUTE_READWRITE, &oldProtect))
        return false;

    memcpy(oldBytes, address, sizeof(jmp));
    memcpy(address, jmp, sizeof(jmp));
    hooked = true;

    return true;
}

bool CHook::Unhook()
{
    if (!hooked)
        return true;
    
    DWORD oldProtect;
    if (!VirtualProtect(address, sizeof(jmp), PAGE_EXECUTE_READWRITE, &oldProtect))
        return false;

    memcpy(address, oldBytes, sizeof(jmp));
    hooked = false;

    return true;
}

bool CHook::IsHooked()
{
    return hooked;
}

uintptr_t CHook::addr()
{
    return reinterpret_cast<uintptr_t>(address);
}
