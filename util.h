#ifndef __util_h__
#define __util_h__

#include <maya/MArrayDataHandle.h>
#include <maya/MComputation.h>
#include <maya/MDGModifier.h>
#include <maya/MFnDagNode.h>
#include <maya/MGlobal.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MString.h>
#include <maya/MTimer.h>

#include <algorithm>
#include <cassert>
#include <errno.h>
#include <iosfwd>
#include <memory>
#include <stdio.h>
#include <string>
#include <vector>
#ifdef _WIN32
#include <direct.h>
#endif

#include "HoudiniApi.h"

#include "traits.h"
#include "types.h"

#define DISPLAY_MSG(displayMethod, ...)                                        \
    {                                                                          \
        MString msg;                                                           \
        msg.format(__VA_ARGS__);                                               \
        /* Workaround MString::format() bug when there are no positional       \
         arguments at all */                                                   \
        if (!msg.length())                                                     \
        {                                                                      \
            msg.format("^1s", __VA_ARGS__);                                    \
        }                                                                      \
        MGlobal::displayMethod(msg);                                           \
    }

#define DISPLAY_ERROR(...) DISPLAY_MSG(displayError, __VA_ARGS__)
#define DISPLAY_WARNING(...) DISPLAY_MSG(displayWarning, __VA_ARGS__)
#define DISPLAY_INFO(...) DISPLAY_MSG(displayInfo, __VA_ARGS__)

#define HAPI_FAIL(r) ((r) != HAPI_RESULT_SUCCESS)

#define GET_HAPI_STATUS_TYPE(status_type, verbosity)                           \
    std::vector<char> _hapiStatusBuffer;                                       \
    {                                                                          \
        int bufferLength;                                                      \
        HoudiniApi::GetStatusStringBufLength(Util::theHAPISession.get(),       \
                                      (status_type), (verbosity),              \
                                      &bufferLength);                          \
        _hapiStatusBuffer.resize(bufferLength);                                \
        HoudiniApi::GetStatusString(Util::theHAPISession.get(), (status_type), \
                             &_hapiStatusBuffer.front(), bufferLength);        \
    }                                                                          \
    const char *hapiStatus = &_hapiStatusBuffer.front();

#define GET_HAPI_STATUS_CALL()                                                 \
    GET_HAPI_STATUS_TYPE(HAPI_STATUS_CALL_RESULT, HAPI_STATUSVERBOSITY_ERRORS)

#define DISPLAY_ERROR_HAPI_STATUS_CALL() DISPLAY_HAPI_STATUS_CALL(displayError)
#define DISPLAY_WARNING_HAPI_STATUS_CALL()                                     \
    DISPLAY_HAPI_STATUS_CALL(displayWarning)
#define DISPLAY_INFO_HAPI_STATUS_CALL() DISPLAY_HAPI_STATUS_CALL(displayInfo)

#define DISPLAY_HAPI_STATUS_CALL(displayMethod)                                \
    {                                                                          \
        GET_HAPI_STATUS_CALL();                                                \
        DISPLAY_MSG(displayMethod, hapiStatus);                                \
    }

#define GET_HAPI_STATUS_COOK()                                                 \
    GET_HAPI_STATUS_TYPE(HAPI_STATUS_COOK_RESULT, HAPI_STATUSVERBOSITY_MESSAGES)

#define CHECK_HAPI_AND_RETURN(r, returnValue)                                  \
    CHECK_HAPI_AND(r, return returnValue;)

#define CHECK_HAPI(r) CHECK_HAPI_AND(r, )

#define CHECK_HAPI_AND(r, footer)                                              \
    {                                                                          \
        HAPI_Result _hapi_result = (r);                                        \
        if (HAPI_FAIL(_hapi_result))                                           \
        {                                                                      \
            std::cerr << "HAPI error in " __FILE__ " at line " << __LINE__     \
                      << std::endl;                                            \
                                                                               \
            GET_HAPI_STATUS_CALL();                                            \
                                                                               \
            std::cerr << hapiStatus << std::endl;                              \
                                                                               \
            footer                                                             \
        }                                                                      \
    }

#ifdef _WIN32
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

namespace Util
{
class HAPISession : public HAPI_Session
{
public:
    HAPISession()
    {
        type = HAPI_SESSION_MAX;
        id   = 0;
    }

    ~HAPISession()
    {
        if (type != HAPI_SESSION_MAX)
        {
            HoudiniApi::CloseSession(this);
        }
    }
};

extern std::unique_ptr<HAPISession> theHAPISession;
extern bool isHapilLoaded;

#ifdef _WIN32
bool mkpath(const std::string &path);
#else
bool mkpath(const std::string &path,
            mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif

extern const char *pathSeparator;

std::string getTempDir();

void displayInfoForNode(const MString &typeName, const MString &message);
void displayWarningForNode(const MString &typeName, const MString &message);
void displayErrorForNode(const MString &typeName, const MString &message);
void markItemNameUsed(const std::string &itemName,
                      std::vector<std::string> &itemNamesUsed);
bool isItemNameUsed(const std::string &itemName,
                    std::vector<std::string> &itemNamesUsed);

template <size_t N>
struct CacheImpl;

template <typename Key, typename Value, size_t Size = 50, size_t MaxSize = 0>
class Cache
{
private:
    struct CacheEntry
    {
        Key key;
        Value value;
        mutable int access;

        bool operator<(const CacheEntry &o) const { return key < o.key; }
    };
    typedef std::vector<CacheEntry> CacheEntries;

public:
    typedef typename CacheEntries::iterator Iterator;

    Cache() : myAccessCount(0) { myCache.reserve(Size); }

    bool find(Iterator &iter, const Key &key)
    {
        CacheEntry key_entry;
        key_entry.key = key;
        iter = std::lower_bound(myCache.begin(), myCache.end(), key_entry);

        if (iter == myCache.end() || iter->key != key)
            return false;

        iter->access = myAccessCount++;
        return true;
    }

    void insert(Iterator &iter, const Key &key, const Value &value)
    {
        CacheImpl<MaxSize>::insert(myCache, iter, key, value);
    }

private:
    CacheEntries myCache;
    mutable int myAccessCount;

    template <size_t N>
    friend struct CacheImpl;
};

template <size_t N>
struct CacheImpl
{
    template <typename CacheEntry,
              typename Iterator,
              typename Key,
              typename Value>
    void insert(std::vector<CacheEntry> &cache,
                Iterator &iter,
                const Key &key,
                const Value &value)
    {
        auto insert_iter = iter;
        if (cache.size() >= N)
        {
            Iterator min_iter = std::min_element(
                cache.begin(), cache.end(),
                [](const CacheEntry &a, const CacheEntry &b) {
                    return a.access < b.access;
                });
            if (min_iter < insert_iter)
                insert_iter--;
            cache.erase(min_iter);
        }

        cache.insert(insert_iter, CacheEntry{key, value, 0});
    }
};

template <>
struct CacheImpl<0>
{
    template <typename CacheEntry,
              typename Iterator,
              typename Key,
              typename Value>
    static void insert(std::vector<CacheEntry> &cache,
                       Iterator &iter,
                       const Key &key,
                       const Value &value)
    {
        auto insert_iter = iter;
        cache.insert(insert_iter, CacheEntry{key, value, 0});
    }
};

class HAPIString
{
public:
    HAPIString(int handle) : myHandle(handle)
    {
        int bufLen;
        HoudiniApi::GetStringBufLength(theHAPISession.get(), myHandle, &bufLen);

        if (bufLen == 0)
        {
            return;
        }

        myString.resize(bufLen - 1);

        HoudiniApi::GetString(
            theHAPISession.get(), myHandle, &myString[0], myString.size() + 1);
    }

    operator std::string() const { return myString; }

    operator MString() const
    {
        MString mayaString;
        mayaString.setUTF8(myString.c_str());
        return mayaString;
    }

    template <typename T>
    bool operator==(const T o) const
    {
        return static_cast<T>(*this) == o;
    }

    template <typename T>
    bool operator!=(const T o) const
    {
        return !(*this == o);
    }

private:
    int myHandle;
    std::string myString;
};

template <>
inline bool HAPIString::operator==<const char *>(const char *const o) const
{
    return static_cast<std::string>(*this) == o;
}

MString mangleParmAttrName(const HAPI_ParmInfo &parm, const MString &name);
MString mangleParmAttrName(const HAPI_ParmInfo &parm,
                           const HAPI_ParmInfo *parentParm,
                           const MString &name);
MString getAttrNameFromParm(const HAPI_ParmInfo &parm);
MString getAttrNameFromParm(const HAPI_ParmInfo &parm,
                            const HAPI_ParmInfo *parentParm);
MString getParmAttrPrefix();
bool hasHAPICallFailed(HAPI_Result stat);

inline MString
getAttrLayerName(const char *name, int layer)
{
    MString layerName = name;
    if (layer > 0)
    {
        layerName += layer + 1;
    }

    return layerName;
}

bool startsWith(const MString &str, const MString &begin);
bool endsWith(const MString &str, const MString &end);
MString escapeString(const MString &str);

class PythonInterpreterLock
{
public:
    PythonInterpreterLock();
    ~PythonInterpreterLock();
};

class ProgressBar
{
public:
    ProgressBar(double waitTimeBeforeShowing = 1.0);
    virtual ~ProgressBar();

    void beginProgress();
    void updateProgress(int progress, int maxProgress, const MString &status);
    void endProgress();

    bool isInterrupted();

protected:
    bool isShowing() const;

    double elapsedTime();
    MString elapsedTimeString();

    virtual void showProgress();
    virtual void displayProgress(int progress,
                                 int maxProgress,
                                 const MString &status);
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
    virtual void displayProgress(int progress,
                                 int maxProgress,
                                 const MString &status);
    virtual void hideProgress();
    virtual bool checkInterrupted();
};

class LogProgressBar : public ProgressBar
{
public:
    LogProgressBar(double timeBetweenLog        = 2.0,
                   double waitTimeBeforeShowing = 1.0);
    virtual ~LogProgressBar();

protected:
    virtual void showProgress();
    virtual void displayProgress(int progress,
                                 int maxProgress,
                                 const MString &status);
    virtual void hideProgress();
    virtual bool checkInterrupted();

private:
    double myTimeBetweenLog;
    double myLastPrintedTime;

    MComputation myComputation;
};

bool statusCheckLoop(bool wantMainProgressBar = true);

MString getNodeName(const MObject &nodeObj);
MObject findNodeByName(const MString &name,
                       MFn::Type expectedFn = MFn::kInvalid);
MObject findDagChild(const MFnDagNode &dag, const MString &name);
MStatus createNodeByModifierCommand(MDGModifier &dgModifier,
                                    const MString &command,
                                    MObject &object,
                                    unsigned int index = 0);
MString replaceString(const MString &str,
                      const MString &searchStr,
                      const MString &newChar);
MString sanitizeStringForNodeName(const MString &str);

// Returns true if the parm was found.
int findParm(std::vector<HAPI_ParmInfo> &parms,
             MString name,
             int instanceNum = -1);

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
    WalkParmOperation &operator=(const WalkParmOperation &);
};
void walkParm(const std::vector<HAPI_ParmInfo> &parmInfos,
              WalkParmOperation &operation);

template <typename T, typename U>
T
convert(const U &src)
{
    return src;
}

// std::string owns the returned char*
template <>
inline const char *
convert(const std::string &src)
{
    return src.c_str();
}

// MString owns the returned char*
template <>
inline const char *
convert(const MString &src)
{
    return src.asChar();
}

// Convert HAPI string
template <>
inline std::string
convert(const int &src)
{
    return HAPIString(src);
}

// Convert HAPI string
template <>
inline MString
convert(const int &src)
{
    return HAPIString(src);
}

template <typename T, typename U>
class ConversionTrait
{
public:
    static const bool useCache = false;
};

template <>
class ConversionTrait<int, std::string>
{
public:
    static const bool useCache = true;
};

template <>
class ConversionTrait<int, MString>
{
public:
    static const bool useCache = true;
};

template <typename T,
          typename U,
          bool Enabled = ConversionTrait<T, U>::useCache>
class ConversionCache
{
public:
    typedef struct
    {
    } Iterator;

    static void getIteratorValue(Iterator &iter, U &value) {}

    bool find(Iterator &iter, const T &key) { return false; }

    void insert(Iterator &iter, const T &key, const U &value) {}
};

template <typename T, typename U>
class ConversionCache<T, U, true>
{
public:
    typedef typename Cache<T, U>::Iterator Iterator;

    static void getIteratorValue(Iterator &iter, U &value)
    {
        value = iter->value;
    }

    bool find(Iterator &iter, const T &key) { return myCache.find(iter, key); }

    void insert(Iterator &iter, const T &key, const U &value)
    {
        myCache.insert(iter, key, value);
    }

private:
    Cache<T, U> myCache;
};

template <typename T, typename U>
void
convertArray(T &dstArray, const U &srcArray)
{
    typedef ARRAYTRAIT(T) DstTrait;
    typedef ELEMENTTYPE(T) DstElementType;
    typedef ARRAYTRAIT(U) SrcTrait;
    typedef ELEMENTTYPE(U) SrcElementType;

    ConversionCache<SrcElementType, DstElementType> conversionCache;

    DstTrait::resize(dstArray, SrcTrait::size(srcArray));
    for (size_t i = 0; i < DstTrait::size(dstArray); i++)
    {
        typename ConversionCache<SrcElementType, DstElementType>::Iterator iter;
        if (conversionCache.find(iter, SrcTrait::getElement(srcArray, i)))
        {
            ConversionCache<SrcElementType, DstElementType>::getIteratorValue(
                iter, DstTrait::getElement(dstArray, i));
            continue;
        }

        DstTrait::getElement(dstArray, i) =
            convert<DstElementType>(SrcTrait::getElement(srcArray, i));

        conversionCache.insert(iter, SrcTrait::getElement(srcArray, i),
                               DstTrait::getElement(dstArray, i));
    }
}

template <typename T, typename U>
T
convertArray(const U &srcArray)
{
    T dstArray;
    convertArray(dstArray, srcArray);
    return dstArray;
}

template <typename T>
void
zeroArray(T &array)
{
    typedef ELEMENTTYPE(T) ElementType;
    std::fill(arrayBegin<T>(array), arrayEnd<T>(array), ElementType());
}

template <size_t NumComponents,
          size_t DstStartComponent,
          size_t DstStride,
          size_t SrcStartComponent,
          size_t SrcStride,
          typename DstType,
          typename SrcType>
DstType
reshapeArray(const SrcType &srcArray)
{
    DstType dstArray;

    typedef ARRAYTRAIT(DstType) DstTrait;
    typedef ELEMENTTRAIT(DstType) DstElementTrait;
    typedef ARRAYTRAIT(SrcType) SrcTrait;
    typedef ELEMENTTRAIT(SrcType) SrcElementTrait;

    DstTrait::resize(dstArray, SrcTrait::size(srcArray) *
                                   SrcElementTrait::numComponents * DstStride /
                                   SrcStride / DstElementTrait::numComponents);
    std::copy(
        componentBegin<SrcStartComponent, NumComponents, SrcStride>(srcArray),
        componentEnd<SrcStartComponent, NumComponents, SrcStride>(srcArray),
        componentBegin<DstStartComponent, NumComponents, DstStride>(dstArray));

    return dstArray;
}

template <size_t NumComponents, typename DstType, typename SrcType>
DstType
reshapeArray(const SrcType &srcArray)
{
    return reshapeArray<NumComponents, 0, NumComponents, 0, NumComponents,
                        DstType>(srcArray);
}

template <typename DstType, typename SrcType>
DstType
reshapeArray(const SrcType &srcArray)
{
    typedef ELEMENTTRAIT(DstType) DstElementTrait;

    return reshapeArray<DstElementTrait::numComponents, DstType>(srcArray);
}

template <typename Type, typename FaceCountsType>
void
reverseWindingOrder(Type &arrayData, const FaceCountsType &faceCounts)
{
    typedef ARRAYTRAIT(Type) Trait;
    typedef ARRAYTRAIT(FaceCountsType) FaceCountsTrait;

    unsigned int current_index = 0;
    for (unsigned int i = 0; i < FaceCountsTrait::size(faceCounts); i++)
    {
        const unsigned int faceCount = FaceCountsTrait::getElement(
            faceCounts, i);
        for (unsigned int a = current_index, b = current_index + faceCount - 1;
             a < current_index + faceCount / 2; a++, b--)
        {
            std::swap(Trait::getElement(arrayData, a),
                      Trait::getElement(arrayData, b));
        }
        current_index += faceCount;
    }
}

template <unsigned int NumComponents,
          unsigned int DstStartComponent,
          unsigned int SrcStartComponent,
          typename DstType,
          typename SrcType,
          typename FaceCountsType,
          typename FaceConnectsType>
void
promoteAttributeData(HAPI_AttributeOwner dstOwner,
                     DstType &dstArray,
                     HAPI_AttributeOwner srcOwner,
                     SrcType &srcArray,
                     unsigned int pointCount,
                     const FaceCountsType *polygonCounts     = NULL,
                     const FaceConnectsType *polygonConnects = NULL)
{
    typedef ARRAYTRAIT(DstType) DstTrait;
    // typedef ARRAYTRAIT(SrcType) SrcTrait;
    typedef ARRAYTRAIT(FaceCountsType) FaceCountsTrait;
    typedef ARRAYTRAIT(FaceConnectsType) FaceConnectsTrait;

    switch (srcOwner)
    {
    case HAPI_ATTROWNER_VERTEX:
        switch (dstOwner)
        {
        case HAPI_ATTROWNER_VERTEX:
        {
            assert(polygonCounts);
            assert(polygonConnects);

            DstTrait::resize(
                dstArray, FaceConnectsTrait::size(*polygonConnects));
            unsigned int current_index = 0;
            for (unsigned int i = 0; i < FaceCountsTrait::size(*polygonCounts);
                 ++i)
            {
                const unsigned int polygonCount = FaceCountsTrait::getElement(
                    *polygonCounts, i);
                for (unsigned int a = current_index,
                                  b = current_index + polygonCount - 1;
                     a < current_index + polygonCount; a++, b--)
                {
                    ComponentWrapper<DstType, DstStartComponent, NumComponents>(
                        dstArray, a, DstStartComponent) =
                        ComponentWrapper<SrcType, SrcStartComponent,
                                         NumComponents>(
                            srcArray, b, SrcStartComponent);
                }

                current_index += polygonCount;
            }
        }
        break;
        default:
            assert(false);
            break;
        }
        break;
    case HAPI_ATTROWNER_POINT:
        switch (dstOwner)
        {
        case HAPI_ATTROWNER_VERTEX:
            assert(polygonConnects);

            DstTrait::resize(
                dstArray, FaceConnectsTrait::size(*polygonConnects));
            for (unsigned int i = 0;
                 i < FaceCountsTrait::size(*polygonConnects); ++i)
            {
                unsigned int point = FaceConnectsTrait::getElement(
                    *polygonConnects, i);
                ComponentWrapper<DstType, DstStartComponent, NumComponents>(
                    dstArray, i, DstStartComponent) =
                    ComponentWrapper<SrcType, SrcStartComponent, NumComponents>(
                        srcArray, point, SrcStartComponent);
            }
            break;
        case HAPI_ATTROWNER_POINT:
            DstTrait::resize(dstArray, pointCount);
            for (unsigned int i = 0; i < pointCount; ++i)
            {
                ComponentWrapper<DstType, DstStartComponent, NumComponents>(
                    dstArray, i, DstStartComponent) =
                    ComponentWrapper<SrcType, SrcStartComponent, NumComponents>(
                        srcArray, i, SrcStartComponent);
            }
            break;
        default:
            assert(false);
            break;
        }
        break;
    case HAPI_ATTROWNER_PRIM:
        switch (dstOwner)
        {
        case HAPI_ATTROWNER_VERTEX:
            assert(polygonCounts);
            assert(polygonConnects);

            DstTrait::resize(
                dstArray, FaceConnectsTrait::size(*polygonConnects));
            for (unsigned int i = 0, j = 0;
                 i < FaceCountsTrait::size(*polygonCounts); ++i)
            {
                const unsigned int polygonCount = FaceCountsTrait::getElement(
                    *polygonCounts, i);
                for (unsigned int k = 0; k < polygonCount; ++j, ++k)
                {
                    ComponentWrapper<DstType, DstStartComponent, NumComponents>(
                        dstArray, j, DstStartComponent) =
                        ComponentWrapper<SrcType, SrcStartComponent,
                                         NumComponents>(
                            srcArray, i, SrcStartComponent);
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
        switch (dstOwner)
        {
        case HAPI_ATTROWNER_VERTEX:
            assert(polygonConnects);
            count = FaceConnectsTrait::size(*polygonConnects);
            break;
        case HAPI_ATTROWNER_POINT:
            count = pointCount;
            break;
        case HAPI_ATTROWNER_PRIM:
            assert(polygonCounts);
            count = FaceCountsTrait::size(*polygonCounts);
            break;
        default:
            assert(false);
            break;
        }

        DstTrait::resize(dstArray, count);
        for (unsigned int i = 0; i < count; ++i)
        {
            ComponentWrapper<DstType, DstStartComponent, NumComponents>(
                dstArray, i, DstStartComponent) =
                ComponentWrapper<SrcType, SrcStartComponent, NumComponents>(
                    srcArray, 0, SrcStartComponent);
        }
    }
    break;
    default:
        assert(false);
        break;
    }
}

MPlug plugSource(const MPlug &plug);

MPlugArray plugDestination(const MPlug &plug);

template <typename T>
bool
isPlugBelow(const MPlug &plug, const T &upper)
{
    MPlug currentPlug = plug;

    for (;;)
    {
        if (currentPlug == upper)
        {
            return true;
        }

        if (currentPlug.isChild())
        {
            currentPlug = currentPlug.parent();
        }
        else if (currentPlug.isElement())
        {
            currentPlug = currentPlug.array();
        }
        else
        {
            break;
        }
    }

    return false;
}

void getChildPlugs(MPlugArray &plugArray, const MPlug &plug);

void resizeArrayDataHandle(MArrayDataHandle &arrayDataHandle, const int count);

bool getHarsPath(std::string &harsPath);

bool checkBuildEngineCompatibility();
}

#endif

