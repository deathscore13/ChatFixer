#pragma once

#include <Windows.h>

class CHook
{
private:
#ifdef _WIN64
    BYTE jmp[14] = {
        0x48, 0xB8, // MOV RAX, [target]
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0xFF, 0xE0, // JMP RAX
        0x90, 0x90  // NOP NOP
    };
#else
    BYTE jmp[6] = {
        0xE9,   // JMP [offset]
        0x00, 0x00, 0x00, 0x00,
        0xC3    // RET
    };
#endif
    BYTE oldBytes[sizeof(jmp)] = {0};

    void* address = nullptr;
    bool hooked = false;

public:
    CHook(void* func, void* addr);
    ~CHook();

    bool Hook();
    bool Unhook();

    bool IsHooked();
    uintptr_t addr();
};
