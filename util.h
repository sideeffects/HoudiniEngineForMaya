#ifndef __util_h__
#define __util_h__

#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MIntArray.h>
#include <maya/MFloatArray.h>

#include <vector>

#include <HAPI/HAPI.h>

class MFnDagNode;

class HAPIError: public std::exception
{
    public:
        HAPIError() throw();
        HAPIError( const HAPIError & error ) throw();
        HAPIError( MString msg ) throw();

        virtual	   ~HAPIError() throw() {}

        virtual const char* what() const throw();

    protected:
        mutable MString myBuffer;
        const MString myMessage;
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
        mutable MString myBuffer;
        MStatus myStat;
};


class Util {
    public:
        static MString getString(int handle);
	static MString getAttrNameFromParm(const HAPI_ParmInfo &parm);
        static void reverseWindingOrderInt(MIntArray& data, MIntArray& faceCounts);
        static void reverseWindingOrderFloat(MFloatArray& data, MIntArray& faceCounts);
        static bool hasHAPICallFailed(HAPI_Result stat);

        // Throws an exception if an error occurred
        static void checkHAPIStatus(HAPI_Result stat);
        static void checkMayaStatus(MStatus stat);
	static void statusCheckLoop();
	static void showProgressWindow(const MString & title, const MString & status, int progress);
	static void updateProgressWindow(const MString & status, int progress);
	static void hideProgressWindow();

        // Prints the error message if an error occurred.
        static void printHAPIStatus(HAPI_Result stat);
        static void printMayaStatus(MStatus stat);

        static MObject findNodeByName(const MString &name);
	static MObject findDagChild(const MFnDagNode &dag, const MString &name);
        static MString replaceString(const MString &str, const MString &searchStr, const MString &newChar);

        // Returns true if the parm was found.
        static int findParm(std::vector<HAPI_ParmInfo>& parms, MString name);
};

#endif
