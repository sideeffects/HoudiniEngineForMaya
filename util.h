#ifndef __util_h__
#define __util_h__

#include <maya/MComputation.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MTimer.h>
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

#define DISPLAY_MSG(displayMethod, ...) \
    { \
        MString msg; \
        msg.format(__VA_ARGS__); \
        /* Workaround MString::format() bug when there are no positional
         arguments at all */ \
        if(!msg.length()) \
        { \
            msg.format("^1s", __VA_ARGS__); \
        } \
        MGlobal::displayMethod(msg); \
    }

#define DISPLAY_ERROR(...) \
    DISPLAY_MSG(displayError, __VA_ARGS__)
#define DISPLAY_WARNING(...) \
    DISPLAY_MSG(displayWarning, __VA_ARGS__)
#define DISPLAY_INFO(...) \
    DISPLAY_MSG(displayInfo, __VA_ARGS__)

#define HAPI_FAIL(r) \
    ((r) != HAPI_RESULT_SUCCESS)

#define GET_HAPI_STATUS_TYPE(status_type, verbosity) \
    std::vector<char> _hapiStatusBuffer; \
    { \
        int bufferLength; \
        HAPI_GetStatusStringBufLength( \
            (status_type), (verbosity), &bufferLength); \
        _hapiStatusBuffer.resize(bufferLength); \
        HAPI_GetStatusString((status_type), &_hapiStatusBuffer.front()); \
    } \
    const char * hapiStatus = &_hapiStatusBuffer.front();

#define GET_HAPI_STATUS_CALL() \
    GET_HAPI_STATUS_TYPE(HAPI_STATUS_CALL_RESULT, HAPI_STATUSVERBOSITY_ERRORS)

#define DISPLAY_ERROR_HAPI_STATUS_CALL() \
    DISPLAY_HAPI_STATUS_CALL(displayError)
#define DISPLAY_WARNING_HAPI_STATUS_CALL() \
    DISPLAY_HAPI_STATUS_CALL(displayWarning)
#define DISPLAY_INFO_HAPI_STATUS_CALL() \
    DISPLAY_HAPI_STATUS_CALL(displayInfo)

#define DISPLAY_HAPI_STATUS_CALL(displayMethod) \
{ \
    GET_HAPI_STATUS_CALL(); \
    DISPLAY_MSG(displayMethod, hapiStatus); \
}

#define GET_HAPI_STATUS_COOK() \
    GET_HAPI_STATUS_TYPE(HAPI_STATUS_COOK_RESULT, HAPI_STATUSVERBOSITY_MESSAGES)

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
        GET_HAPI_STATUS_CALL(); \
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

        static MString escapeString(const MString &str);

        // Throws an exception if an error occurred
        static void checkHAPIStatus(HAPI_Result stat);

        class ProgressBar
        {
            public:
                ProgressBar(double waitTimeBeforeShowing = 1.0);
                virtual ~ProgressBar();

                void beginProgress();
                void updateProgress(
                        int progress,
                        int maxProgress,
                        const MString &status
                        );
                void endProgress();

                bool isInterrupted();

            protected:
                bool isShowing() const;

                double elapsedTime();
                MString elapsedTimeString();

                virtual void showProgress();
                virtual void displayProgress(
                        int progress,
                        int maxProgress,
                        const MString &status
                        );
                virtual void hideProgress();
                virtual bool checkInterrupted();

            private:
                double myWaitTimeBeforeShowing;
                bool myIsShowing;
                MTimer myTimer;
        };

        class MainProgressBar : public ProgressBar
        {
            public:
                MainProgressBar(double waitTimeBeforeShowing = 1.0);
                virtual ~MainProgressBar();

            protected:
                virtual void showProgress();
                virtual void displayProgress(
                        int progress,
                        int maxProgress,
                        const MString &status
                        );
                virtual void hideProgress();
                virtual bool checkInterrupted();
        };

        class LogProgressBar : public ProgressBar
        {
            public:
                LogProgressBar(
                        double timeBetweenLog = 2.0,
                        double waitTimeBeforeShowing = 1.0
                        );
                virtual ~LogProgressBar();

            protected:
                virtual void showProgress();
                virtual void displayProgress(
                        int progress,
                        int maxProgress,
                        const MString &status
                        );
                virtual void hideProgress();
                virtual bool checkInterrupted();

            private:
                double myTimeBetweenLog;
                double myLastPrintedTime;

                MComputation myComputation;
        };

        static bool statusCheckLoop(bool wantMainProgressBar = true);

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
