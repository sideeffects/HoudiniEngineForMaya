#include "util.h"

//=============================================================================
// HAPIError
//=============================================================================

HAPIError::HAPIError() throw()
    : exception()
    , message("")
{}

HAPIError::HAPIError( const HAPIError & error ) throw()
    : exception()
    , message(error.message)
{}

HAPIError::HAPIError( MString msg ) throw()
    : exception()
    , message(msg)
{}

const char *
HAPIError::what() const throw()
{
    buffer = "******************** HAPI Error ********************\n";
    buffer += message;
    buffer += "\n****************************************************";
    return buffer.asChar();
}


//=============================================================================
// MayaError
//=============================================================================
MayaError::MayaError() throw()
    : exception()
{}

MayaError::MayaError( const MayaError & error ) throw()
    : exception()
    , stat(error.stat)
{}

MayaError::MayaError( MStatus stat ) throw()
    : exception()
    , stat(stat)
{}

const char *
MayaError::what() const throw()
{
    buffer = "******************** Maya Error ********************\n";
    buffer += stat.errorString();
    buffer += "\n****************************************************";
    return buffer.asChar();
}

MStatus
MayaError::status() { return stat; }


//=============================================================================
// Util
//=============================================================================

MString
Util::getString(int handle)
{
    int bufLen;
    HAPI_GetStringLength(handle, &bufLen);
    char buffer[bufLen];
    HAPI_GetString(handle, buffer, bufLen+1);
    return MString(buffer);
}


void
Util::reverseWindingOrderInt(MIntArray& data, MIntArray& faceCounts)
{
    int current_index = 0;
    int numFaceCount = faceCounts.length();
    for (int i=0; i<numFaceCount; i++)
    {
        int vertex_count = faceCounts[i];
        int a = current_index;
        int b = current_index + vertex_count - 1;
        while (a < b)
        {
            int temp = data[a];
            data[a] = data[b];
            data[b] = temp;
            a++;
            b--;
        }
        current_index += vertex_count;
    }
}


void
Util::reverseWindingOrderFloat(MFloatArray& data, MIntArray& faceCounts)
{
    int current_index = 0;
    int numFaceCount = faceCounts.length();
    for (int i=0; i<numFaceCount; i++)
    {
        int vertex_count = faceCounts[i];
        int a = current_index;
        int b = current_index + vertex_count - 1;
        while (a < b)
        {
            float temp = data[a];
            data[a] = data[b];
            data[b] = temp;
            a++;
            b--;
        }
        current_index += vertex_count;
    }
}


bool
Util::hasHAPICallFailed(HAPI_StatusCode stat)
{
    return stat > 0;
}


void
Util::checkHAPIStatus(HAPI_StatusCode stat)
{
    if (hasHAPICallFailed(stat))
    {
        int bufLen;
        HAPI_GetLastErrorStringLength(&bufLen);
        char buffer[bufLen];
        HAPI_GetLastErrorString(buffer);
        throw HAPIError(buffer);
    }
}


void
Util::checkMayaStatus(MStatus stat)
{
    if (stat.error())
    {
        throw MayaError(stat);
    }
}
