#ifndef __hapiutil_h__
#define __hapiutil_h__

#include "HoudiniApi.h"

#include "traits.h"
#include "types.h"
#include "util.h"
#include "Platform.h"

template <typename T>
struct HAPITypeTrait
{
};

template <>
struct HAPITypeTrait<int>
{
    static const HAPI_StorageType storageType = HAPI_STORAGETYPE_INT;
};

template <>
struct HAPITypeTrait<HAPI_Int64>
{
    static const HAPI_StorageType storageType = HAPI_STORAGETYPE_INT64;
};

template <>
struct HAPITypeTrait<float>
{
    static const HAPI_StorageType storageType = HAPI_STORAGETYPE_FLOAT;
};

template <>
struct HAPITypeTrait<double>
{
    static const HAPI_StorageType storageType = HAPI_STORAGETYPE_FLOAT64;
};

template <>
struct HAPITypeTrait<const char *>
{
    static const HAPI_StorageType storageType = HAPI_STORAGETYPE_STRING;
};

template <>
struct HAPITypeTrait<std::string>
{
    static const HAPI_StorageType storageType = HAPI_STORAGETYPE_STRING;
};

template <>
struct HAPITypeTrait<MString>
{
    static const HAPI_StorageType storageType = HAPI_STORAGETYPE_STRING;
};

#define HAPITYPETRAIT(T) HAPITypeTrait<REMOVECONST(T)>

template <HAPI_StorageType storageType>
struct HAPIAttributeTrait
{
    typedef void SetType;
    typedef void GetType;

    static HAPI_Result setAttribute(const HAPI_Session *session,
                                    HAPI_NodeId nodeId,
                                    HAPI_PartId partId,
                                    const char *name,
                                    const HAPI_AttributeInfo *attrInfo,
                                    const SetType *dataArray,
                                    int start,
                                    int length)
    {
        return HAPI_RESULT_FAILURE;
    }

    static HAPI_Result getAttribute(const HAPI_Session *session,
                                    HAPI_NodeId nodeId,
                                    HAPI_PartId partId,
                                    const char *name,
                                    HAPI_AttributeInfo *attrInfo,
                                    GetType *dataArray,
                                    int start,
                                    int length)
    {
        return HAPI_RESULT_FAILURE;
    }
};

template <>
struct HAPIAttributeTrait<HAPI_STORAGETYPE_INT>
{
    typedef int SetType;
    typedef int GetType;

    static HAPI_Result setAttribute(const HAPI_Session *session,
                                    HAPI_NodeId nodeId,
                                    HAPI_PartId partId,
                                    const char *name,
                                    const HAPI_AttributeInfo *attrInfo,
                                    const SetType *dataArray,
                                    int start,
                                    int length)
    {
        return HoudiniApi::SetAttributeIntData(
            session, nodeId, partId, name, attrInfo, dataArray, start, length);
    }

    static HAPI_Result getAttribute(const HAPI_Session *session,
                                    HAPI_NodeId nodeId,
                                    HAPI_PartId partId,
                                    const char *name,
                                    HAPI_AttributeInfo *attrInfo,
                                    GetType *dataArray,
                                    int start,
                                    int length)
    {
        return HoudiniApi::GetAttributeIntData(session, nodeId, partId, name, attrInfo,
                                        -1, dataArray, start, length);
    }
};

template <>
struct HAPIAttributeTrait<HAPI_STORAGETYPE_INT64>
{
    typedef HAPI_Int64 SetType;
    typedef HAPI_Int64 GetType;

    static HAPI_Result setAttribute(const HAPI_Session *session,
                                    HAPI_NodeId nodeId,
                                    HAPI_PartId partId,
                                    const char *name,
                                    const HAPI_AttributeInfo *attrInfo,
                                    const SetType *dataArray,
                                    int start,
                                    int length)
    {
        return HoudiniApi::SetAttributeInt64Data(
            session, nodeId, partId, name, attrInfo, dataArray, start, length);
    }

    static HAPI_Result getAttribute(const HAPI_Session *session,
                                    HAPI_NodeId nodeId,
                                    HAPI_PartId partId,
                                    const char *name,
                                    HAPI_AttributeInfo *attrInfo,
                                    GetType *dataArray,
                                    int start,
                                    int length)
    {
        return HoudiniApi::GetAttributeInt64Data(session, nodeId, partId, name,
                                          attrInfo, -1, dataArray, start,
                                          length);
    }
};

template <>
struct HAPIAttributeTrait<HAPI_STORAGETYPE_FLOAT>
{
    typedef float SetType;
    typedef float GetType;

    static HAPI_Result setAttribute(const HAPI_Session *session,
                                    HAPI_NodeId nodeId,
                                    HAPI_PartId partId,
                                    const char *name,
                                    const HAPI_AttributeInfo *attrInfo,
                                    const SetType *dataArray,
                                    int start,
                                    int length)
    {
        return HoudiniApi::SetAttributeFloatData(
            session, nodeId, partId, name, attrInfo, dataArray, start, length);
    }

    static HAPI_Result getAttribute(const HAPI_Session *session,
                                    HAPI_NodeId nodeId,
                                    HAPI_PartId partId,
                                    const char *name,
                                    HAPI_AttributeInfo *attrInfo,
                                    GetType *dataArray,
                                    int start,
                                    int length)
    {
        return HoudiniApi::GetAttributeFloatData(session, nodeId, partId, name,
                                          attrInfo, -1, dataArray, start,
                                          length);
    }
};

template <>
struct HAPIAttributeTrait<HAPI_STORAGETYPE_FLOAT64>
{
    typedef double SetType;
    typedef double GetType;

    static HAPI_Result setAttribute(const HAPI_Session *session,
                                    HAPI_NodeId nodeId,
                                    HAPI_PartId partId,
                                    const char *name,
                                    const HAPI_AttributeInfo *attrInfo,
                                    const SetType *dataArray,
                                    int start,
                                    int length)
    {
        return HoudiniApi::SetAttributeFloat64Data(
            session, nodeId, partId, name, attrInfo, dataArray, start, length);
    }

    static HAPI_Result getAttribute(const HAPI_Session *session,
                                    HAPI_NodeId nodeId,
                                    HAPI_PartId partId,
                                    const char *name,
                                    HAPI_AttributeInfo *attrInfo,
                                    GetType *dataArray,
                                    int start,
                                    int length)
    {
        return HoudiniApi::GetAttributeFloat64Data(session, nodeId, partId, name,
                                            attrInfo, -1, dataArray, start,
                                            length);
    }
};

template <>
struct HAPIAttributeTrait<HAPI_STORAGETYPE_STRING>
{
    typedef const char *SetType;
    typedef int GetType;

    static HAPI_Result setAttribute(const HAPI_Session *session,
                                    HAPI_NodeId nodeId,
                                    HAPI_PartId partId,
                                    const char *name,
                                    const HAPI_AttributeInfo *attrInfo,
                                    const SetType *dataArray,
                                    int start,
                                    int length)
    {
        return HoudiniApi::SetAttributeStringData(session, nodeId, partId, name,
                                           attrInfo, (const char **)dataArray,
                                           start, length);
    }

    static HAPI_Result getAttribute(const HAPI_Session *session,
                                    HAPI_NodeId nodeId,
                                    HAPI_PartId partId,
                                    const char *name,
                                    HAPI_AttributeInfo *attrInfo,
                                    GetType *dataArray,
                                    int start,
                                    int length)
    {
        return HoudiniApi::GetAttributeStringData(
            session, nodeId, partId, name, attrInfo, dataArray, start, length);
    }
};

template <typename T, typename U>
struct SameType
{
    static const bool value = false;
};

template <typename T>
struct SameType<T, T>
{
    static const bool value = true;
};

template <HAPI_StorageType storageType,
          typename T,
          bool canUseData = ARRAYTRAIT(T)::canGetData
              &&SameType<REMOVECONST(ELEMENTTYPE(T)),
                         typename HAPIAttributeTrait<HAPITYPETRAIT(
                             ELEMENTTYPE(T))::storageType>::SetType>::value>
struct HAPISetAttribute
{
    static HAPI_Result impl(HAPI_NodeId nodeId,
                            HAPI_PartId partId,
                            HAPI_AttributeOwner owner,
                            size_t tupleSize,
                            const char *attributeName,
                            const T &dataArray)
    {
        HAPI_Result hapiResult;
        if (tupleSize == 0)
            return HAPI_RESULT_SUCCESS;

        size_t dataArraySize = ARRAYTRAIT(T)::size(dataArray);

        size_t count = dataArraySize / tupleSize;

        HAPI_AttributeInfo attributeInfo;
        HoudiniApi::AttributeInfo_Init(&attributeInfo);
        attributeInfo.exists    = true;
        attributeInfo.owner     = owner;
        attributeInfo.storage   = storageType;
        attributeInfo.count     = count;
        attributeInfo.tupleSize = tupleSize;
        // identify UV and colorSet parms (later we'll check for matching
        // uvNumber or Alpha if they if they exist)
        if (!strncmp(attributeName, "uv", 2) &&
            strncmp(attributeName, "uvNumber", 8))
            attributeInfo.typeInfo =
                HAPI_AttributeTypeInfo::HAPI_ATTRIBUTE_TYPE_TEXTURE;
        if (!strncmp(attributeName, "Cd", 2))
            attributeInfo.typeInfo =
                HAPI_AttributeTypeInfo::HAPI_ATTRIBUTE_TYPE_COLOR;

        hapiResult = HoudiniApi::AddAttribute(Util::theHAPISession.get(), nodeId,
                                       partId, attributeName, &attributeInfo);
        CHECK_HAPI_AND_RETURN(hapiResult, hapiResult);

        // Even when the count is zero, we still need to call
        // HAPI_AddAttribute(). This is needed to clear out any existing data
        // that was left over from the previous input geometry.
        if (count == 0)
            return HAPI_RESULT_SUCCESS;

        hapiResult = HAPIAttributeTrait<storageType>::setAttribute(
            Util::theHAPISession.get(), nodeId, partId, attributeName,
            &attributeInfo, ARRAYTRAIT(T)::data(dataArray), 0, count);
        CHECK_HAPI_AND_RETURN(hapiResult, hapiResult);

        return HAPI_RESULT_SUCCESS;
    }
};

template <HAPI_StorageType storageType, typename T>
struct HAPISetAttribute<storageType, T, false>
{
    static HAPI_Result impl(HAPI_NodeId nodeId,
                            HAPI_PartId partId,
                            HAPI_AttributeOwner owner,
                            size_t tupleSize,
                            const char *attributeName,
                            const T &dataArray)
    {
        typedef typename HAPIAttributeTrait<storageType>::SetType SetType;
        typedef std::vector<SetType> ConvertedDataArray;

        ConvertedDataArray convertedDataArray;
        Util::convertArray(convertedDataArray, dataArray);

        return HAPISetAttribute<storageType, ConvertedDataArray>::impl(
            nodeId, partId, owner, tupleSize, attributeName,
            convertedDataArray);
    }
};

template <typename T>
HAPI_Result
hapiSetAttribute(HAPI_NodeId nodeId,
                 HAPI_PartId partId,
                 HAPI_AttributeOwner owner,
                 size_t tupleSize,
                 const char *attributeName,
                 const T &dataArray)
{
    return HAPISetAttribute<HAPITYPETRAIT(ELEMENTTYPE(T))::storageType,
                            T>::impl(nodeId, partId, owner, tupleSize,
                                     attributeName, dataArray);
}

template <typename T, bool isArray = ARRAYTRAIT(T)::isArray>
struct HAPISetDetailAttribute
{
    static HAPI_Result impl(HAPI_NodeId nodeId,
                            HAPI_PartId partId,
                            const char *attributeName,
                            const T &dataArray)
    {
        return hapiSetAttribute(nodeId, partId, HAPI_ATTROWNER_DETAIL,
                                ARRAYTRAIT(T)::size(dataArray), attributeName,
                                dataArray);
    }
};

template <typename T>
struct HAPISetDetailAttribute<T, false>
{
    static HAPI_Result impl(HAPI_NodeId nodeId,
                            HAPI_PartId partId,
                            const char *attributeName,
                            T &value)
    {
        return hapiSetAttribute(nodeId, partId, HAPI_ATTROWNER_DETAIL, 1,
                                attributeName, rawArray(&value, 1));
    }
};

template <typename T>
HAPI_Result
hapiSetDetailAttribute(HAPI_NodeId nodeId,
                       HAPI_PartId partId,
                       const char *attributeName,
                       T &value)
{
    return HAPISetDetailAttribute<T>::impl(
        nodeId, partId, attributeName, value);
}

template <typename T>
HAPI_Result
hapiSetPrimAttribute(HAPI_NodeId nodeId,
                     HAPI_PartId partId,
                     size_t tupleSize,
                     const char *attributeName,
                     const T &dataArray)
{
    return hapiSetAttribute(nodeId, partId, HAPI_ATTROWNER_PRIM, tupleSize,
                            attributeName, dataArray);
}

template <typename T>
HAPI_Result
hapiSetVertexAttribute(HAPI_NodeId nodeId,
                       HAPI_PartId partId,
                       size_t tupleSize,
                       const char *attributeName,
                       const T &dataArray)
{
    return hapiSetAttribute(nodeId, partId, HAPI_ATTROWNER_VERTEX, tupleSize,
                            attributeName, dataArray);
}

template <typename T>
HAPI_Result
hapiSetPointAttribute(HAPI_NodeId nodeId,
                      HAPI_PartId partId,
                      size_t tupleSize,
                      const char *attributeName,
                      const T &dataArray)
{
    return hapiSetAttribute(nodeId, partId, HAPI_ATTROWNER_POINT, tupleSize,
                            attributeName, dataArray);
}

template <HAPI_StorageType storageType,
          typename T,
          bool canUseData = ARRAYTRAIT(T)::canGetData
              &&SameType<ELEMENTTYPE(T),
                         typename HAPIAttributeTrait<HAPITYPETRAIT(
                             ELEMENTTYPE(T))::storageType>::GetType>::value>
struct HAPIGetAttribute
{
    static HAPI_Result impl(HAPI_NodeId nodeId,
                            HAPI_PartId partId,
                            HAPI_AttributeOwner owner,
                            const char *attributeName,
                            HAPI_AttributeInfo &attrInfo,
                            T &dataArray)
    {
        HAPI_Result hapiResult;

        hapiResult = HoudiniApi::GetAttributeInfo(Util::theHAPISession.get(), nodeId,
                                           partId, attributeName, owner,
                                           &attrInfo);
        if (HAPI_FAIL(hapiResult))
        {
            return HAPI_RESULT_FAILURE;
        }

        if (!attrInfo.exists)
        {
            return HAPI_RESULT_FAILURE;
        }

        if (attrInfo.storage != storageType)
        {
            switch (attrInfo.storage)
            {
            case HAPI_STORAGETYPE_INT:
            {
                typedef HAPIAttributeTrait<HAPI_STORAGETYPE_INT>::GetType
                    ComponentType;
                typedef std::vector<ComponentType> BufferType;
                BufferType buffer;
                hapiResult =
                    HAPIGetAttribute<HAPI_STORAGETYPE_INT, BufferType>::impl(
                        nodeId, partId, owner, attributeName, attrInfo, buffer);
                CHECK_HAPI_AND_RETURN(hapiResult, hapiResult);
                Util::convertArray(dataArray, buffer);

                return HAPI_RESULT_SUCCESS;
            }
            break;
            case HAPI_STORAGETYPE_INT64:
            {
                typedef HAPIAttributeTrait<HAPI_STORAGETYPE_INT64>::GetType
                    ComponentType;
                typedef std::vector<ComponentType> BufferType;
                BufferType buffer;
                hapiResult =
                    HAPIGetAttribute<HAPI_STORAGETYPE_INT64, BufferType>::impl(
                        nodeId, partId, owner, attributeName, attrInfo, buffer);
                CHECK_HAPI_AND_RETURN(hapiResult, hapiResult);
                Util::convertArray(dataArray, buffer);

                return HAPI_RESULT_SUCCESS;
            }
            break;
            case HAPI_STORAGETYPE_FLOAT:
            {
                typedef HAPIAttributeTrait<HAPI_STORAGETYPE_FLOAT>::GetType
                    ComponentType;
                typedef std::vector<ComponentType> BufferType;
                BufferType buffer;
                hapiResult =
                    HAPIGetAttribute<HAPI_STORAGETYPE_FLOAT, BufferType>::impl(
                        nodeId, partId, owner, attributeName, attrInfo, buffer);
                CHECK_HAPI_AND_RETURN(hapiResult, hapiResult);
                Util::convertArray(dataArray, buffer);

                return HAPI_RESULT_SUCCESS;
            }
            break;
            case HAPI_STORAGETYPE_FLOAT64:
            {
                typedef HAPIAttributeTrait<HAPI_STORAGETYPE_FLOAT64>::GetType
                    ComponentType;
                typedef std::vector<ComponentType> BufferType;
                BufferType buffer;
                hapiResult =
                    HAPIGetAttribute<HAPI_STORAGETYPE_FLOAT64,
                                     BufferType>::impl(nodeId, partId, owner,
                                                       attributeName, attrInfo,
                                                       buffer);
                CHECK_HAPI_AND_RETURN(hapiResult, hapiResult);
                Util::convertArray(dataArray, buffer);

                return HAPI_RESULT_SUCCESS;
            }
            break;
            default:
                return HAPI_RESULT_FAILURE;
                break;
            }
        }

        ARRAYTRAIT(T)::resize(dataArray, attrInfo.count * attrInfo.tupleSize);
        hapiResult = HAPIAttributeTrait<storageType>::getAttribute(
            Util::theHAPISession.get(), nodeId, partId, attributeName,
            &attrInfo, ARRAYTRAIT(T)::data(dataArray), 0, attrInfo.count);
        CHECK_HAPI_AND_RETURN(hapiResult, hapiResult);

        return HAPI_RESULT_SUCCESS;
    }
};

template <HAPI_StorageType storageType, typename T>
struct HAPIGetAttribute<storageType, T, false>
{
    static HAPI_Result impl(HAPI_NodeId nodeId,
                            HAPI_PartId partId,
                            HAPI_AttributeOwner owner,
                            const char *attributeName,
                            HAPI_AttributeInfo &attrInfo,
                            T &dataArray)
    {
        typedef typename HAPIAttributeTrait<storageType>::GetType GetType;
        typedef std::vector<GetType> ConvertedDataArray;

        HAPI_Result hapiResult;

        ConvertedDataArray convertedDataArray;

        hapiResult = HAPIGetAttribute<storageType, ConvertedDataArray>::impl(
            nodeId, partId, owner, attributeName, attrInfo, convertedDataArray);
        if (HAPI_FAIL(hapiResult))
        {
            return HAPI_RESULT_FAILURE;
        }

        Util::convertArray(dataArray, convertedDataArray);

        return HAPI_RESULT_SUCCESS;
    }
};

template <typename T>
HAPI_Result
hapiGetAttribute(HAPI_NodeId nodeId,
                 HAPI_PartId partId,
                 HAPI_AttributeOwner owner,
                 const char *attributeName,
                 HAPI_AttributeInfo &attrInfo,
                 T &dataArray)
{
    return HAPIGetAttribute<HAPITYPETRAIT(ELEMENTTYPE(T))::storageType,
                            T>::impl(nodeId, partId, owner, attributeName,
                                     attrInfo, dataArray);
}

template <typename T, bool isArray = ARRAYTRAIT(T)::isArray>
struct HAPIGetDetailAttribute
{
    static HAPI_Result impl(HAPI_NodeId nodeId,
                            HAPI_PartId partId,
                            const char *attributeName,
                            HAPI_AttributeInfo &attrInfo,
                            T &dataArray)
    {
        return hapiGetAttribute(nodeId, partId, HAPI_ATTROWNER_DETAIL,
                                attributeName, attrInfo, dataArray);
    }
};

template <typename T>
struct HAPIGetDetailAttribute<T, false>
{
    static HAPI_Result impl(HAPI_NodeId nodeId,
                            HAPI_PartId partId,
                            const char *attributeName,
                            HAPI_AttributeInfo &attrInfo,
                            T &value)
    {
        RawArray<T> array(&value, 1);
        return hapiGetAttribute(nodeId, partId, HAPI_ATTROWNER_DETAIL,
                                attributeName, attrInfo, array);
    }
};

template <typename T>
HAPI_Result
hapiGetDetailAttribute(HAPI_NodeId nodeId,
                       HAPI_PartId partId,
                       const char *attributeName,
                       HAPI_AttributeInfo &attrInfo,
                       T &value)
{
    return HAPIGetDetailAttribute<T>::impl(
        nodeId, partId, attributeName, attrInfo, value);
}

template <typename T>
HAPI_Result
hapiGetPrimAttribute(HAPI_NodeId nodeId,
                     HAPI_PartId partId,
                     const char *attributeName,
                     HAPI_AttributeInfo &attrInfo,
                     T &dataArray)
{
    return hapiGetAttribute(nodeId, partId, HAPI_ATTROWNER_PRIM, attributeName,
                            attrInfo, dataArray);
}

template <typename T>
HAPI_Result
hapiGetVertexAttribute(HAPI_NodeId nodeId,
                       HAPI_PartId partId,
                       const char *attributeName,
                       HAPI_AttributeInfo &attrInfo,
                       T &dataArray)
{
    return hapiGetAttribute(nodeId, partId, HAPI_ATTROWNER_VERTEX,
                            attributeName, attrInfo, dataArray);
}

template <typename T>
HAPI_Result
hapiGetPointAttribute(HAPI_NodeId nodeId,
                      HAPI_PartId partId,
                      const char *attributeName,
                      HAPI_AttributeInfo &attrInfo,
                      T &dataArray)
{
    return hapiGetAttribute(nodeId, partId, HAPI_ATTROWNER_POINT, attributeName,
                            attrInfo, dataArray);
}

template <typename T>
HAPI_Result
hapiGetAnyAttribute(HAPI_NodeId nodeId,
                    HAPI_PartId partId,
                    const char *attributeName,
                    HAPI_AttributeInfo &attrInfo,
                    T &dataArray)
{
    bool found = false;

    if (!HAPI_FAIL(hapiGetVertexAttribute(
            nodeId, partId, attributeName, attrInfo, dataArray)))
    {
        found = true;
    }
    else if (!HAPI_FAIL(hapiGetPointAttribute(
                 nodeId, partId, attributeName, attrInfo, dataArray)))
    {
        found = true;
    }
    else if (!HAPI_FAIL(hapiGetPrimAttribute(
                 nodeId, partId, attributeName, attrInfo, dataArray)))
    {
        found = true;
    }
    else if (!HAPI_FAIL(hapiGetDetailAttribute(
                 nodeId, partId, attributeName, attrInfo, dataArray)))
    {
        found = true;
    }

    if (!found)
    {
        return HAPI_RESULT_FAILURE;
    }

    return HAPI_RESULT_SUCCESS;
}

#endif

