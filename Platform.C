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

        if (handle)
        {
            Util::logVerboseSetupInfo("libHAPIL loaded!");
        }
        else
        {
            char buffer[256] = { 0 };
            #if defined(PLATFORM_WINDOWS)
                snprintf(buffer, 256, "libHAPIL failed to load!");
            #else
                snprintf(buffer, 256, "libHAPIL failed to load! Error: %s", dlerror());
            #endif
            Util::logVerboseSetupInfo(buffer, true);
        }
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

