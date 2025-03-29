#ifndef __CHOOKS_H

#include <Windows.h>

#define __CHOOK_SIZE 6

class CHook
{
private:
	BYTE oldBytes[__CHOOK_SIZE] = { 0 };
	BYTE jmp[__CHOOK_SIZE] = { 0 };

	DWORD oldProtect = NULL;
	DWORD protect = PAGE_EXECUTE_READWRITE;

	void* addr = NULL;
	bool hooked = false;

public:
	CHook(DWORD func, void* addr);
	~CHook();

	void Hook();
	void Unhook();

	bool IsHooked();
};

#endif
