#include "hooks.h"

CHook::CHook(DWORD func, void* address)
{
	addr = address;

	memcpy(jmp, "\xE9\x90\x90\x90\x90\xC3", __CHOOK_SIZE); // запись маски для команды перехода

	DWORD JMPSize = (func - (DWORD)addr - 5); // смещение относительно оригинала
	VirtualProtect(addr, __CHOOK_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect);

	memcpy(oldBytes, addr, __CHOOK_SIZE);
	memcpy(&jmp[1], &JMPSize, 4); // запись 4-х байт смещения (x86)

	VirtualProtect(addr, __CHOOK_SIZE, oldProtect, NULL);
}

CHook::~CHook()
{
	if (hooked)
		Unhook();
}

void CHook::Hook()
{
	VirtualProtect(addr, __CHOOK_SIZE, protect, NULL);

	memcpy(addr, jmp, __CHOOK_SIZE);
	hooked = true;

	VirtualProtect(addr, __CHOOK_SIZE, oldProtect, NULL);
}

void CHook::Unhook()
{
	VirtualProtect(addr, __CHOOK_SIZE, protect, NULL);

	memcpy(addr, oldBytes, __CHOOK_SIZE);
	hooked = false;

	VirtualProtect(addr, __CHOOK_SIZE, oldProtect, NULL);
}

bool CHook::Hooked()
{
	return hooked;
}
