#include "util.h"

MString
Util::getString(int handle)
{
    int bufLen;
    HAPI_GetStringLength(handle, &bufLen);
    char buffer[bufLen];
    HAPI_GetString(handle, buffer, bufLen+1);
    return MString(buffer);
}
