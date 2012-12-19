#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>

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
    HAPI_GetStringBufLength(handle, &bufLen);
    char * buffer = new char[bufLen];
    HAPI_GetString(handle, buffer, bufLen);

    MString ret(buffer);
    delete[] buffer;

    return ret;
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
        char * buffer = new char[bufLen + 1];
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


void
Util::printHAPIStatus(HAPI_StatusCode stat)
{
    if (hasHAPICallFailed(stat))
    {
        int bufLen;
        HAPI_GetLastErrorStringLength(&bufLen);
        char* buffer = new char[bufLen];
        HAPI_GetLastErrorString(buffer);

        MString str;
        str += "******************** Maya Error ********************\n";
        str += buffer;
        str += "\n****************************************************";

        cerr << str << endl;
    }
}


void
Util::printMayaStatus(MStatus stat)
{
    if (stat.error())
    {
        cerr << stat << endl;
    }
}


MString
Util::executeCommand(const MString& cmd)
{
    MString result;
    try
    {
        MStatus stat;
        stat = MGlobal::executeCommand(cmd, result);
        checkMayaStatus(stat);
    }
    catch (MayaError& e)
    {
        // Handle the exception here because sometimes command executes
        // but Maya still returns failure status.
        cerr << "Execute MEL error: " << cmd << endl;
        cerr << e.what() << endl;
    }
    return result;
}


MObject
Util::findNodeByName(MString& name)
{
    MSelectionList selection;
    selection.add(name);

    MObject ret;

    if(selection.length())
        selection.getDependNode(0, ret);
    return ret;
}
