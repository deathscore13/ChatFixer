#include "sigscan.h"

CSigScan *CSigScan::Create(void *interfaceFn)
{
    if(!interfaceFn)
        return nullptr;

    MEMORY_BASIC_INFORMATION mem;
    if(!VirtualQuery(interfaceFn, &mem, sizeof(mem)))
        return nullptr;
 
    BYTE *addr = (BYTE*)mem.AllocationBase;
 
    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER*)mem.AllocationBase;
    if (dos->e_lfanew <= 0 || dos->e_lfanew > (LONG)mem.RegionSize)
        return nullptr;

    IMAGE_NT_HEADERS *pe = (IMAGE_NT_HEADERS*)((unsigned long)dos + (unsigned long)dos->e_lfanew);
    if(pe->Signature != IMAGE_NT_SIGNATURE)
        return nullptr;
 
    return new CSigScan(addr, (size_t)pe->OptionalHeader.SizeOfImage);
}

void* CSigScan::Find(const unsigned char *sig, const unsigned char *mask)
{
    size_t sigLen = strlen((const char*)mask);
    BYTE *ptr = addr,
        *end = addr + len;
    size_t i;

    while (ptr + sigLen <= end)
    {
        for (i = 0; i < sigLen; i++)
        {
            if ((mask[i] != '?') && (sig[i] != ptr[i]))
                break;
        }

        if (i == sigLen)
            return (void *)ptr;

        ptr++;
    }

    return nullptr;
}
