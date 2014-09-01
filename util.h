#ifndef __util_h__
#define __util_h__

#include <maya/MComputation.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MTimer.h>
#include <maya/MIntArray.h>

#include <cassert>
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

        // STL style containers
        template <typename T, typename Alloc, template <typename, typename> class U>
        static size_t getArrayLength(const U<T, Alloc> &array)
        {
            return array.size();
        }

        template <typename T, typename Alloc, template <typename, typename> class U>
        static void setArrayLength(U<T, Alloc> &array, size_t n)
        {
            return array.resize(n);
        }

        // Maya containers
        template <typename T>
        static unsigned int getArrayLength(const T &array)
        {
            return array.length();
        }

        template <typename T>
        static unsigned int setArrayLength(T &array, unsigned int n)
        {
            return array.setLength(n);
        }

        // Accessing components
        template <unsigned int offset, typename A, typename T>
        struct getComponent
        {
            A operator()(const T &t, unsigned int i)
            {
                return t[offset + i];
            }
            A &operator()(T &t, unsigned int i)
            {
                return t[offset + i];
            }
        };

        template <unsigned int offset, typename T>
        struct getComponent<offset, T, T>
        {
            T operator()(const T &t, unsigned int i)
            {
                return t;
            }
            T &operator()(T &t, unsigned int i)
            {
                return t;
            }
        };

        template <unsigned int offset, unsigned int offset2,
                 unsigned int numComponents,
                 typename A,
                 typename T, typename U>
        static void setComponents(T &a, const U &b)
        {
            for(unsigned int i = 0; i < numComponents; i++)
            {
                getComponent<offset, A, T>()(a, i) = getComponent<offset2, A, U>()(b, i);
            }
        }

        template <typename T, typename U>
        static void reverseWindingOrder(T &arrayData, const U &faceCounts)
        {
            unsigned int current_index = 0;
            for(unsigned int i = 0; i < getArrayLength(faceCounts); i++)
            {
                for(unsigned int a = current_index, b = current_index + faceCounts[i] - 1;
                        a < b; a++, b--)
                {
                    std::swap(arrayData[a], arrayData[b]);
                }
                current_index += faceCounts[i];
            }
        }

        template <unsigned int offset, unsigned int offset2, unsigned int numComponents,
                 typename A,
                 typename T, typename U, typename V>
        static void
        promoteAttributeData(
                HAPI_AttributeOwner toOwner,
                T &toArray,
                HAPI_AttributeOwner fromOwner,
                const U &fromArray,
                unsigned int pointCount,
                const V* polygonCounts = NULL,
                const V* polygonConnects = NULL
                )
        {
            if(fromOwner == toOwner)
            {
                setArrayLength(toArray, getArrayLength(fromArray));
                for(unsigned int i = 0; i < getArrayLength(fromArray); ++i)
                {
                    setComponents<offset, offset2, numComponents, A>(toArray[i], fromArray[i]);
                }

                return;
            }

            switch(fromOwner)
            {
                case HAPI_ATTROWNER_POINT:
                    switch(toOwner)
                    {
                        case HAPI_ATTROWNER_VERTEX:
                            assert(polygonConnects);
                            assert(polygonConnects);

                            setArrayLength(toArray, getArrayLength(*polygonConnects));
                            for(unsigned int i = 0; i < getArrayLength(*polygonConnects); ++i)
                            {
                                unsigned int point = (*polygonConnects)[i];
                                setComponents<offset, offset2, numComponents, A>(toArray[i], fromArray[point]);
                            }
                            break;
                        default:
                            assert(false);
                            break;
                    }
                    break;
                case HAPI_ATTROWNER_PRIM:
                    switch(toOwner)
                    {
                        case HAPI_ATTROWNER_VERTEX:
                            assert(polygonCounts);
                            assert(polygonConnects);

                            setArrayLength(toArray, getArrayLength(*polygonConnects));
                            for(unsigned int i = 0, j = 0; i < getArrayLength(*polygonCounts); ++i)
                            {
                                for(int k = 0; k < (*polygonCounts)[i]; ++j, ++k)
                                {
                                    setComponents<offset, offset2, numComponents, A>(toArray[j], fromArray[i]);
                                }
                            }
                            break;
                        case HAPI_ATTROWNER_POINT:
                            // Don't convert the prim attributes to point
                            // attributes, because that would lose information.
                            // Convert everything to vertex attributs instead.
                            assert(false);
                            break;
                        default:
                            assert(false);
                            break;
                    }
                    break;
                case HAPI_ATTROWNER_DETAIL:
                    {
                        unsigned int count = 0;
                        switch(toOwner)
                        {
                            case HAPI_ATTROWNER_VERTEX:
                                assert(polygonConnects);
                                count = getArrayLength(*polygonConnects);
                                break;
                            case HAPI_ATTROWNER_POINT:
                                count = pointCount;
                                break;
                            case HAPI_ATTROWNER_PRIM:
                                assert(polygonCounts);
                                count = getArrayLength(*polygonCounts);
                                break;
                            default:
                                assert(false);
                                break;
                        }

                        setArrayLength(toArray, count);
                        for(unsigned int i = 0; i < count; ++i)
                        {
                            setComponents<offset, offset2, numComponents, A>(toArray[i], fromArray[0]);
                        }
                    }
                    break;
                default:
                    assert(false);
                    break;
            }
        }
};

#endif
