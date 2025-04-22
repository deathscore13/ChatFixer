#pragma once

#define VSP_NAME    ChatFixer
#define VSP_VERSION "2.1.0"

#define TO_STRING_EX(param)         #param
#define TO_STRING(param)            TO_STRING_EX(param)  
#define VSP_PREFIX(param)           VSP_NAME##param
#define VSP_SUFFIX(param)           param##VSP_NAME
#define VSP_CENTER(param1, param2)  param1##VSP_NAME##param2
#define GITHUB_REPOSITORY           "github.com/deathscore13/" TO_STRING(VSP_NAME)

#define REPORT_SUFFIX "\nMake sure you have the new version installed, or create an issue in GitHub: " GITHUB_REPOSITORY

#define Report(pMsg)        Error(pMsg REPORT_SUFFIX)  
#define Report(pMsg, ...)   Error(pMsg REPORT_SUFFIX, __VA_ARGS__)


#include "icvar.h"
#include "sigscan.h"

extern CSigScan *client, *engine;
extern ICvar *s_pCVar;

#define CSigScan_Find(ptr, name, out) \
    if ((out = ptr->Find((const unsigned char*)name ## _SIGNATURE, (const unsigned char*)name ## _MASK)) == nullptr) \
    { \
        Report("Unable to get " #name "() address\n"); \
        return false; \
    }
