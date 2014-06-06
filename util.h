#ifndef __util_h__
#define __util_h__

#include <maya/MGlobal.h>
#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MIntArray.h>

#include <vector>

#include <HAPI/HAPI.h>

class MDGModifier;
class MFnDagNode;

class HAPIError: public std::exception
{
    public:
        HAPIError() throw();
        HAPIError(const HAPIError & error) throw();
        HAPIError(MString msg) throw();

        virtual           ~HAPIError() throw() {}

        virtual const char* what() const throw();

    protected:
        mutable MString myBuffer;
        MString myMessage;
};

#define DISPLAY_MSG(displayMethod, msgFormat, ...) \
    { \
        MString msg; \
        msg.format((msgFormat), __VA_ARGS__); \
        MGlobal::displayMethod(msg); \
    }

#define DISPLAY_ERROR(msg, ...) \
    DISPLAY_MSG(displayError, msg, __VA_ARGS__)
#define DISPLAY_WARNING(msg, ...) \
    DISPLAY_MSG(displayWarning, msg, __VA_ARGS__)
#define DISPLAY_INFO(msg, ...) \
    DISPLAY_MSG(displayInfo, msg, __VA_ARGS__)

#define HAPI_FAIL(r) \
    ((r) != HAPI_RESULT_SUCCESS)

#define GET_HAPI_STATUS() \
    std::vector<char> _hapiStatusBuffer; \
    { \
        int bufferLength; \
        HAPI_GetStatusStringBufLength(HAPI_STATUS_RESULT, &bufferLength); \
        _hapiStatusBuffer.resize(bufferLength); \
        HAPI_GetStatusString(HAPI_STATUS_RESULT, &_hapiStatusBuffer.front()); \
    } \
    const char * hapiStatus = &_hapiStatusBuffer.front();


#define DISPLAY_ERROR_HAPI_STATUS() \
    DISPLAY_HAPI_STATUS(displayError)
#define DISPLAY_WARNING_HAPI_STATUS() \
    DISPLAY_HAPI_STATUS(displayWarning)
#define DISPLAY_INFO_HAPI_STATUS() \
    DISPLAY_HAPI_STATUS(displayInfo)

#define DISPLAY_HAPI_STATUS(displayMethod) \
{ \
    GET_HAPI_STATUS(); \
    MGlobal::displayMethod(hapiStatus); \
}

#define CHECK_HAPI_AND_RETURN(r, returnValue) \
    CHECK_HAPI_AND(r, return returnValue;)

#define CHECK_HAPI(r) \
    CHECK_HAPI_AND(r, )

#define CHECK_HAPI_AND(r, footer) \
{ \
    HAPI_Result _hapi_result = (r); \
    if(HAPI_FAIL(_hapi_result)) \
    { \
        std::cerr << "HAPI error in " __FILE__ " at line " << __LINE__ << std::endl; \
        \
        GET_HAPI_STATUS(); \
        \
        std::cerr << hapiStatus << std::endl; \
        \
        footer \
    } \
}

class Util {
    public:
        static void displayInfoForNode(
                const MString &typeName,
                const MString &message
                );
        static void displayWarningForNode(
                const MString &typeName,
                const MString &message
                );
        static void displayErrorForNode(
                const MString &typeName,
                const MString &message
                );

        static MString getString(int handle);
        static MString getAttrNameFromParm(const HAPI_ParmInfo &parm);
        static MStringArray getAttributeStringData(int assetId, int objectId,
                                                    int geoId, int partId,
                                                    HAPI_AttributeOwner owner,
                                                    const MString & name);
        static MString getParmAttrPrefix();
        static bool hasHAPICallFailed(HAPI_Result stat);

        // Throws an exception if an error occurred
        static void checkHAPIStatus(HAPI_Result stat);
        static void statusCheckLoop();
        static void showProgressWindow(const MString & title, const MString & status, int progress);
        static void updateProgressWindow(const MString & status, int progress);
        static void hideProgressWindow();

        template <typename T>
        static void reverseWindingOrder(T &arrayData, const MIntArray &faceCounts)
        {
            int current_index = 0;
            for(int i = 0; i < (int) faceCounts.length(); i++)
            {
                for(int a = current_index, b = current_index + faceCounts[i] - 1;
                        a < b; a++, b--)
                {
                    std::swap(arrayData[a], arrayData[b]);
                }
                current_index += faceCounts[i];
            }
        }

        static MObject findNodeByName(const MString &name);
        static MObject findDagChild(const MFnDagNode &dag, const MString &name);
        static MStatus createNodeByModifierCommand(
                MDGModifier &dgModifier,
                const MString &command,
                MObject &object,
                unsigned int index = 0
                );
        static MString replaceString(const MString &str, const MString &searchStr, const MString &newChar);

        // Returns true if the parm was found.
        static int findParm(std::vector<HAPI_ParmInfo>& parms, MString name, int instanceNum = -1);

        class WalkParmOperation
        {
            public:
                WalkParmOperation();
                virtual ~WalkParmOperation();

                virtual void pushFolder(const HAPI_ParmInfo &parmInfo);
                virtual void popFolder();

                virtual void pushMultiparm(const HAPI_ParmInfo &parmInfo);
                virtual void nextMultiparm();
                virtual void popMultiparm();

                virtual void leaf(const HAPI_ParmInfo &parmInfo);

            private:
                // This class is not copyable so these are not implemented
                WalkParmOperation(const WalkParmOperation &);
                WalkParmOperation& operator=(const WalkParmOperation &);
        };
        static void walkParm(const std::vector<HAPI_ParmInfo> &parmInfos, WalkParmOperation &operation);
};

#endif
