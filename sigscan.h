#pragma once

#include <Windows.h>

class CSigScan
{
private:
    BYTE *addr;
    size_t len;

    CSigScan(BYTE *addr, size_t len): addr(addr), len(len)
    {
    }

public:

    static CSigScan *Create(void *interfaceFn);
    void *Find(const unsigned char *sig, const unsigned char *mask);
};
