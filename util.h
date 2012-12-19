#ifndef UTIL_H
#define UTIL_H

#include <maya/MString.h>
#include <maya/MIntArray.h>
#include <maya/MFloatArray.h>

#include <HAPI.h>

class HAPIError: public std::exception
{
    public:
        HAPIError() throw();
        HAPIError( const HAPIError & error ) throw();
        HAPIError( MString msg ) throw();

        virtual	   ~HAPIError() throw() {}

        virtual const char* what() const throw();

    protected:
        mutable MString buffer;
        const MString message;
};


class MayaError: public std::exception
{
    public:
        MayaError() throw();
        MayaError( const MayaError & error ) throw();
        MayaError( MStatus stat ) throw();

        virtual	   ~MayaError() throw() {}

        virtual const char* what() const throw();

        virtual MStatus status();

    protected:
        mutable MString buffer;
        MStatus stat;
};


class Util {
    public:
        static MString getString(int handle);
        static void reverseWindingOrderInt(MIntArray& data, MIntArray& faceCounts);
        static void reverseWindingOrderFloat(MFloatArray& data, MIntArray& faceCounts);
        static bool hasHAPICallFailed(HAPI_Result stat);

        // Throws an exception if an error occurred
        static void checkHAPIStatus(HAPI_Result stat);
        static void checkMayaStatus(MStatus stat);

        // Prints the error message if an error occurred.
        static void printHAPIStatus(HAPI_Result stat);
        static void printMayaStatus(MStatus stat);

        static MString executeCommand(const MString& cmd);
        static MObject findNodeByName(MString& name);
        static MString replaceChar(MString& str, char oldChar, char newChar);
};

#endif
