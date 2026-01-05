#ifndef __Platform_h__
#define __Platform_h__

#if defined(_WIN32) || defined(__CYGWIN__)
    #define PLATFORM_WINDOWS
#elif defined(__linux__)
    #define PLATFORM_LINUX
#elif defined(__APPLE__) || defined (__MACH__)
    #define PLATFORM_MAC
#else
    #error Unknown Platform
#endif

void *obtainHAPILHandle(const char *filename);
void *fetchSymbol(void *handle, const char *symbolName);

#endif

