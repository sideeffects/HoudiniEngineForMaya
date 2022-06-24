#include "Platform.h"

#include "util.h"

#if defined(PLATFORM_WINDOWS)
    #include <Windows.h>
    #include <libloaderapi.h>
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MAC)
    #include <dlfcn.h>
#endif

void*
obtainHAPILHandle(const char *filename)
{
    void *handle = nullptr;

    if (filename)
    {
        #if defined(PLATFORM_WINDOWS)
            handle = LoadLibraryA(filename);
        #elif defined(PLATFORM_LINUX) || defined(PLATFORM_MAC)
            handle = dlopen(filename, RTLD_LAZY | RTLD_LOCAL);
        #endif
    }

    return handle;
}

void*
fetchSymbol(void *handle, const char *symbolName)
{
    void *symbol = nullptr;

    if (handle && symbolName)
    {
        #if defined(PLATFORM_WINDOWS)
            symbol = GetProcAddress((HMODULE)handle, symbolName);
        #elif defined(PLATFORM_LINUX) || defined(PLATFORM_MAC)
            symbol = dlsym(handle, symbolName);
        #endif
    }

    return symbol;
}

