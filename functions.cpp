#include "signatures.h"
#include "vsp.h"

void *Con_ColorPrintf;

bool Functions_Load()
{
    CSigScan_Find(engine, Con_ColorPrintf, Con_ColorPrintf);
    return true;
}
