#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MDataHandle.h>
#include <maya/MTime.h>
#include <maya/MGlobal.h>
#include <maya/MPlugArray.h>
#include <maya/MAnimControl.h>
#include <maya/MConditionMessage.h>

#include "Asset.h"
#include "Input.h"
#include "AssetNode.h"
#include "OutputGeometryObject.h"
#include "OutputInstancerObject.h"
#include "OutputMaterial.h"
#include "util.h"

#include <cassert>
#include <algorithm>
#include <unordered_map>
#include <memory>

class AttrOperation : public Util::WalkParmOperation
{
    public:
        enum Mode{
            Get,
            Set
        };

        AttrOperation(
                MDataBlock &dataBlock,
                const Mode &mode,
                const MFnDependencyNode &nodeFn,
                const HAPI_NodeInfo &nodeInfo,
                const std::vector<MObject>* attrs
                );
        ~AttrOperation();

        virtual void pushFolder(const HAPI_ParmInfo &parmInfo);
        virtual void popFolder();

        virtual void pushMultiparm(const HAPI_ParmInfo &parmInfo);
        virtual void nextMultiparm();
        virtual void popMultiparm();

        bool containsParm(
                const MString &attrName,
                const HAPI_ParmInfo &parm
                ) const;

        MString getAttrNameFromParm(const HAPI_ParmInfo &parm) const;
        bool detectedMismatch();

    protected:
        std::vector<MDataHandle> myDataHandles;
        std::vector<MPlug> myPlugs;
        std::vector<bool> myExists;
        std::vector<const HAPI_ParmInfo*> myParentParmInfos;

        std::vector<bool> myIsMulti;
        std::vector<MDataHandle> myMultiSizeDataHandles;
        std::vector<MPlug> myMultiSizePlugs;
        std::vector<bool> myHasMultiAttr;
        std::vector<MArrayDataHandle> myMultiDataHandles;
        std::vector<MPlug> myMultiPlugs;
        std::vector<int> myMultiLogicalIndices;

        MDataBlock &myDataBlock;
        const Mode myMode;
        const MFnDependencyNode &myNodeFn;
        const HAPI_NodeInfo &myNodeInfo;
        const std::vector<MObject>* myAttrs;
        bool myMismatch;
};

AttrOperation::AttrOperation(
        MDataBlock &dataBlock,
        const Mode &mode,
        const MFnDependencyNode &nodeFn,
        const HAPI_NodeInfo &nodeInfo,
        const std::vector<MObject>* attrs
        ) :
    myDataBlock(dataBlock),
    myMode(mode),
    myNodeFn(nodeFn),
    myNodeInfo(nodeInfo),
    myAttrs(attrs),
    myMismatch(false)
{
    MDataHandle dataHandle;
    MPlug plug;

    plug = myNodeFn.findPlug(Util::getParmAttrPrefix());
    if(myMode == AttrOperation::Get)
    {
        dataHandle = myDataBlock.outputValue(plug);
    }
    else if(myMode == AttrOperation::Set)
    {
        dataHandle = myDataBlock.inputValue(plug);
    }

    myDataHandles.push_back(dataHandle);
    myPlugs.push_back(plug);
    myExists.push_back(true);
    myParentParmInfos.push_back(NULL);
}

AttrOperation::~AttrOperation()
{
    myDataHandles.pop_back();
    myPlugs.pop_back();
    myExists.pop_back();
    myParentParmInfos.pop_back();
}

void
AttrOperation::pushFolder(const HAPI_ParmInfo &parmInfo)
{
    MStatus status;

    MDataHandle dataHandle;
    MPlug plug;
    bool exists = false;

    MDataHandle &parentDataHandle = myDataHandles.back();
    MPlug &parentPlug = myPlugs.back();
    bool parentExists = myExists.back();
    //const HAPI_ParmInfo* &parentParmInfo = myParentParmInfos.back();

    if(parentExists)
    {
        MString folderAttrName = getAttrNameFromParm(parmInfo);
        MObject folderAttrObj = myNodeFn.attribute(folderAttrName);

        if(!folderAttrObj.isNull())
        {
            plug = parentPlug.child(folderAttrObj, &status);
            if(status)
            {
                exists = true;
            }
        }

        if(exists)
        {
            // When we got the parentDataHandle, Maya doesn't actually evaluate
            // the child compound attributes. So if we get the child dataHandle
            // with child(), it could still be dirty. Workaround it by calling
            // MDataBlock's inputValue()/outputValue(). Unfortunately, this
            // still doesn't work for the case where attributes are under a
            // multi attribute.
            if(myIsMulti.size() == 0)
            {
                if(myMode == AttrOperation::Get)
                {
                    dataHandle = myDataBlock.outputValue(plug);
                }
                else if(myMode == AttrOperation::Set)
                {
                    dataHandle = myDataBlock.inputValue(plug);
                }
            }
            else
            {
                dataHandle = parentDataHandle.child(folderAttrObj);
            }
        }
    }

    myDataHandles.push_back(dataHandle);
    myPlugs.push_back(plug);
    myExists.push_back(exists);
    myParentParmInfos.push_back(&parmInfo);
}

void
AttrOperation::popFolder()
{
    //MDataHandle &dataHandle = myDataHandles.back();
    //MPlug &plug = myPlugs.back();
    //bool exists = myExists.back();

    myDataHandles.pop_back();
    myPlugs.pop_back();
    myExists.pop_back();
    myParentParmInfos.pop_back();
}

void
AttrOperation::pushMultiparm(const HAPI_ParmInfo &parmInfo)
{
    MStatus status;

    // In a multiparm context, these variables are used to store the element
    // attribute.
    MDataHandle dataHandle;
    MPlug plug;
    bool exists = false;

    // These parameters are pushed only for multiparms, and not created for the
    // other parameters (e.g. folders and leaf).
    // Since MArrayDataHandle doesn't have a default constructor, we construct
    // an invalid MArrayDataHandle this way.
    bool isMulti = false;
    MDataHandle multiSizeDataHandle;
    MPlug multiSizePlug;
    bool hasMultiAttr = false;
    MArrayDataHandle multiDataHandle(dataHandle);
    MPlug multiPlug;
    int multiLogicalIndex = -1;

    MDataHandle &parentDataHandle = myDataHandles.back();
    MPlug &parentPlug = myPlugs.back();
    bool parentExists = myExists.back();
    //const HAPI_ParmInfo* &parentParmInfo = myParentParmInfos.back();

    if(parentExists)
    {
        MString multiAttrName = getAttrNameFromParm(parmInfo);
        MObject multiAttrObj = myNodeFn.attribute(multiAttrName);

        // Ramp has no __multiSize attribute.
        if(parmInfo.rampType != HAPI_RAMPTYPE_INVALID)
        {
            isMulti = true;
        }
        else
        {
            MObject multiSizeAttrObj = myNodeFn.attribute(multiAttrName + "__multiSize");
            if(!multiSizeAttrObj.isNull())
            {
                multiSizePlug = parentPlug.child(multiSizeAttrObj, &status);
                if(status)
                {
                    isMulti = true;
                }
            }

            if(isMulti)
            {
                multiSizeDataHandle = parentDataHandle.child(multiSizeAttrObj);
            }
        }

        // multiAttrObj might not exist if the current instanceCount is 0
        if(isMulti && !multiAttrObj.isNull())
        {
            multiPlug = parentPlug.child(multiAttrObj, &status);
            if(status)
            {
                hasMultiAttr = true;
            }

            if(hasMultiAttr)
            {
                multiDataHandle = MArrayDataHandle(parentDataHandle.child(multiAttrObj));
            }
        }
    }

    myIsMulti.push_back(isMulti);
    myMultiSizeDataHandles.push_back(multiSizeDataHandle);
    myMultiSizePlugs.push_back(multiSizePlug);
    myHasMultiAttr.push_back(hasMultiAttr);
    myMultiDataHandles.push_back(multiDataHandle);
    myMultiPlugs.push_back(multiPlug);
    myMultiLogicalIndices.push_back(multiLogicalIndex);

    myDataHandles.push_back(dataHandle);
    myPlugs.push_back(plug);
    myExists.push_back(exists);
    myParentParmInfos.push_back(&parmInfo);

    // initializes the first element
    nextMultiparm();
}

void
AttrOperation::nextMultiparm()
{
    MStatus status;

    MDataHandle &dataHandle = myDataHandles.back();
    MPlug &plug = myPlugs.back();
    bool exists = myExists.back();
    const HAPI_ParmInfo* &parentParmInfo = myParentParmInfos.back();

    //bool isMulti = myIsMulti.back();
    //MDataHandle &multiSizeDataHandle = myMultiSizeDataHandles.back();
    //MPlug &multiSizePlug = myMultiSizePlugs.back();
    bool hasMultiAttr = myHasMultiAttr.back();
    MArrayDataHandle &multiDataHandle = myMultiDataHandles.back();
    MPlug &multiPlug = myMultiPlugs.back();
    int &multiLogicalIndex = myMultiLogicalIndices.back();

    if(hasMultiAttr)
    {
        // For ramps, use next(), instead of jumpToElement(), because the array
        // is sparse and the indicies don't matter.
        if(parentParmInfo->rampType != HAPI_RAMPTYPE_INVALID)
        {
            if(multiLogicalIndex != -1)
            {
                status = multiDataHandle.next();
            }
            multiLogicalIndex++;
        }
        else
        {
            multiLogicalIndex++;
            status = multiDataHandle.jumpToArrayElement(multiLogicalIndex);
        }
        exists = status;

        if(exists)
        {
            if(myMode == AttrOperation::Get)
            {
                dataHandle = multiDataHandle.outputValue();
            }
            else if(myMode == AttrOperation::Set)
            {
                dataHandle = multiDataHandle.inputValue();
            }

            plug = multiPlug.elementByPhysicalIndex(multiLogicalIndex);
        }
    }

    myExists.back() = exists;
}

void
AttrOperation::popMultiparm()
{
    //MDataHandle &dataHandle = myDataHandles.back();
    //MPlug &plug = myPlugs.back();
    //bool exists = myExists.back();
    //const HAPI_ParmInfo* &parentParmInfo = myParentParmInfos.back();

    //bool isMulti = myIsMulti.back();
    //MDataHandle &multiSizeDataHandle = myMultiSizeDataHandles.back();
    //MPlug &multiSizePlug = myMultiSizePlugs.back();
    //bool hasMultiAttr = myHasMultiAttr.back();
    //MArrayDataHandle &multiDataHandle = myMultiDataHandles.back();
    //MPlug &multiPlug = myMultiPlugs.back();
    //int &multiLogicalIndex = myMultiLogicalIndices.back();

    myIsMulti.pop_back();
    myMultiSizeDataHandles.pop_back();
    myMultiSizePlugs.pop_back();
    myHasMultiAttr.pop_back();
    myMultiDataHandles.pop_back();
    myMultiPlugs.pop_back();
    myMultiLogicalIndices.pop_back();

    myDataHandles.pop_back();
    myPlugs.pop_back();
    myExists.pop_back();
    myParentParmInfos.pop_back();
}

bool
AttrOperation::containsParm(
        const MString &attrName,
        const HAPI_ParmInfo &parm
        ) const
{
    if(!myAttrs)
    {
        return true;
    }

    MPlug parmPlug = myNodeFn.findPlug(attrName);

    for(std::vector<MObject>::const_iterator iter = myAttrs->begin();
            iter != myAttrs->end();
            iter++)
    {
        MPlug plug = myNodeFn.findPlug(*iter);

        if(parmPlug == plug)
        {
            return true;
        }

        // If the parm is a tuple, then we also need to check the parent plug.
        // We need to check if it's int, float, or string, because non-values
        // like folders also use parm.size.
        if((HAPI_ParmInfo_IsInt(&parm) || HAPI_ParmInfo_IsFloat(&parm) || HAPI_ParmInfo_IsString(&parm))
                && parm.size > 1
                && plug.isChild() && parmPlug == plug.parent())
        {
            return true;
        }
    }

    return false;
}


MString 
AttrOperation::getAttrNameFromParm(const HAPI_ParmInfo &parmInfo) const
{
    MString name;
    if (AssetNode* assetNode = dynamic_cast<AssetNode*>(myNodeFn.userNode()))
    {
        name = assetNode->getAsset()->getAttrNameFromParm(parmInfo);
    }
    return name;
}
bool
AttrOperation::detectedMismatch()
{
    return myMismatch;
}

// Helper class for caching the names of parms.  This avoids expensive
// re-fetching of parm names during an update
//
class ParmNameCache
{
public:
    bool cacheEnabled() const
    {
        return
            //            false && 
            myCacheEnabled &&
            (
                MAnimControl::isPlaying() ||
                MConditionMessage::getConditionState("playblasting")
            );
    }
    void clearCache()
    {
        myStringCache.clear();
    }
    void enableCache(bool enabled)
    {
        myCacheEnabled = enabled;
    }

    MString getParmTemplateName(const HAPI_ParmInfo& parm) const
    {
        if (cacheEnabled())
        {
            auto&& iter = myStringCache.find(parm);
            if (iter != myStringCache.end())
            {
                return iter->second;
            }
        }
        return MString();
    }
    void cacheName(const HAPI_ParmInfo& parm, const MString& name)
    {
        if (cacheEnabled())
            myStringCache[parm] = name;
    }

private:
    struct ParmInfoEqual
    {
        bool operator()(const HAPI_ParmInfo& rhs, const HAPI_ParmInfo& lhs) const
        {
            return rhs.id == lhs.id &&
                rhs.parentId == lhs.parentId &&
                rhs.childIndex == lhs.childIndex &&
                rhs.type == lhs.type;
        }
    };
    struct ParmInfoHasher
    {
        std::size_t operator()(const HAPI_ParmInfo& k) const
        {
            using std::size_t;
            using std::hash;

            size_t res = 17;
            res = res * 31 + hash<HAPI_ParmId>()(k.id);
            res = res * 31 + hash<HAPI_ParmId>()(k.parentId);
            res = res * 31 + hash<int>()(k.childIndex);
            res = res * 31 + hash<int>()(k.type);
            return res;
        }
    };
    typedef std::unordered_map<HAPI_ParmInfo, MString, ParmInfoHasher, ParmInfoEqual> StringCacheMap;
    StringCacheMap myStringCache;
    bool myCacheEnabled = true;
};


Asset::Asset(
        const MString &otlFilePath,
        const MString &assetName
        ) :
    // initialize values here because instantiating the asset could error out
    myAssetInputs(NULL)
{
    myParmNameCache = std::unique_ptr<ParmNameCache>(new ParmNameCache());

    HAPI_Result hapiResult = HAPI_RESULT_SUCCESS;

    HAPI_AssetInfo_Init(&myAssetInfo);
    HAPI_NodeInfo_Init(&myNodeInfo);

    // load the otl
    int libraryId = -1;
    hapiResult = HAPI_LoadAssetLibraryFromFile(
            Util::theHAPISession.get(),
            otlFilePath.asChar(),
            true,
            &libraryId
            );
    if(HAPI_FAIL(hapiResult))
    {
        DISPLAY_WARNING("Could not load OTL file: ^1s\n"
                "Attempting to instantiate asset anyway.",
                otlFilePath);
        DISPLAY_WARNING_HAPI_STATUS_CALL();
    }

    // get the list of assets in the otl
    std::vector<HAPI_StringHandle> assetNamesSH;
    if(libraryId >= 0)
    {
        int assetCount = 0;
        hapiResult = HAPI_GetAvailableAssetCount(
                Util::theHAPISession.get(),
                libraryId,
                &assetCount
                );
        CHECK_HAPI(hapiResult);

        assetNamesSH.resize(assetCount);
        hapiResult = HAPI_GetAvailableAssets(
                Util::theHAPISession.get(),
                libraryId,
                &assetNamesSH.front(),
                assetCount
                );
        CHECK_HAPI(hapiResult);
    }

    // find the asset in the otl
    if(assetNamesSH.size())
    {
        bool foundAsset = false;
        for(unsigned int i = 0; i < assetNamesSH.size(); i++)
        {
            if(Util::HAPIString(assetNamesSH[i]) == assetName)
            {
                foundAsset = true;
            }
        }

        if(!foundAsset)
        {
            DISPLAY_WARNING("Could not find asset: ^1s\n"
                    "in OTL file: ^2s\n"
                    "Attempting to instantiate asset anyway.",
                    assetName,
                    otlFilePath);
        }
    }

    // instantiate the asset
    HAPI_NodeId nodeId = -1;
    {
        Util::PythonInterpreterLock pythonInterpreterLock;

        hapiResult = HAPI_CreateNode(
                Util::theHAPISession.get(),
                -1,
                assetName.asChar(),
                NULL,
                false,
                &nodeId
                );
        if(HAPI_FAIL(hapiResult))
        {
            DISPLAY_ERROR("Could not instantiate asset: ^1s\n"
                    "in OTL file: ^2s\n",
                    assetName,
                    otlFilePath);

            GET_HAPI_STATUS_CALL();
            DISPLAY_ERROR(hapiStatus);

            return;
        }

        if(!Util::statusCheckLoop())
        {
            DISPLAY_ERROR("Could not instantiate asset: ^1s\n"
                    "in OTL file: ^2s\n",
                    assetName,
                    otlFilePath);

            GET_HAPI_STATUS_COOK();
            DISPLAY_ERROR(hapiStatus);

            // Do nothing else if the asset is invalid.
            return;
        }
    }

    hapiResult = HAPI_GetAssetInfo(
            Util::theHAPISession.get(),
            nodeId,
            &myAssetInfo
            );
    CHECK_HAPI(hapiResult);

    myAssetName = Util::HAPIString(myAssetInfo.fullOpNameSH);

    hapiResult = HAPI_GetNodeInfo(
            Util::theHAPISession.get(),
            nodeId,
            &myNodeInfo
            );
    CHECK_HAPI(hapiResult);

    // Warn the user if the OTL path is not what was originally requested.
    if(Util::HAPIString(myAssetInfo.filePathSH) != otlFilePath)
    {
        DISPLAY_WARNING("The asset: ^1s\n"
                "was instantiated from: ^2s\n"
                "but the expected path was: ^3s",
                assetName,
                MString(Util::HAPIString(myAssetInfo.filePathSH)),
                otlFilePath
                );
    }

    myAssetInputs = new Inputs(myNodeInfo.id);
    myAssetInputs->setNumInputs(myNodeInfo.inputCount);
}

Asset::~Asset()
{
    for(OutputObjects::const_iterator iter = myObjects.begin();
            iter != myObjects.end(); iter++)
    {
        delete *iter;
    }
    myObjects.clear();
    delete myAssetInputs;

    for(OutputMaterials::const_iterator iter = myMaterials.begin();
            iter != myMaterials.end(); iter++)
    {
        delete *iter;
    }
    myMaterials.clear();

    if(!Util::theHAPISession.get())
        return;

    int idToDelete =  myNodeInfo.id;
    if(myNodeInfo.type == HAPI_NODETYPE_SOP)
        idToDelete = myNodeInfo.parentId;
    if(idToDelete >= 0)
    {
        CHECK_HAPI(HAPI_DeleteNode(
                Util::theHAPISession.get(),
                idToDelete
                ));
    }
}

bool
Asset::isValid() const
{
    return myAssetInfo.nodeId >= 0;
}

MString
Asset::getOTLFilePath() const
{
    if(!isValid())
    {
        return MString();
    }

    return Util::HAPIString(myAssetInfo.filePathSH);
}

MString
Asset::getAssetName() const
{
    if(!isValid())
    {
        return MString();
    }
    return myAssetName;
}

MString
Asset::getRelativePath(HAPI_NodeId id)
{
    HAPI_StringHandle relativePathHandle;
    CHECK_HAPI(HAPI_GetNodePath(
            Util::theHAPISession.get(),
            id,
            myNodeInfo.id,
            &relativePathHandle
            ));
    return Util::HAPIString(relativePathHandle);
}

void
Asset::resetSimulation()
{
    assert(myNodeInfo.id >= 0);

    HAPI_ResetSimulation(
            Util::theHAPISession.get(),
            myNodeInfo.id
            );
}

MString
Asset::getCookMessages()
{
    Util::PythonInterpreterLock pythonInterpreterLock;

    // Trigger a cook so that the asset will become the "last cooked asset",
    // because HAPI_STATUS_COOK_RESULT only consider the "last cooked asset".
    // In most cases, this shouldn't do any actual cooking.
    HAPI_CookNode(
            Util::theHAPISession.get(),
            myNodeInfo.id,
            NULL
            );

    GET_HAPI_STATUS_COOK();

    return hapiStatus;
}

void
Asset::update()
{
    assert(myNodeInfo.id >= 0);

    HAPI_Result hapiResult;

    // Get the Object nodes
    std::vector<HAPI_NodeId> nodeIds;
    if(myNodeInfo.type == HAPI_NODETYPE_OBJ)
    {
        int objectCount = 0;
        hapiResult = HAPI_ComposeChildNodeList(
                Util::theHAPISession.get(),
                myAssetInfo.objectNodeId,
                HAPI_NODETYPE_OBJ,
                HAPI_NODEFLAGS_OBJ_GEOMETRY,
                true,
                &objectCount
                );
        CHECK_HAPI(hapiResult);

        myIsObjSubnet = objectCount > 0;

        if(myIsObjSubnet)
        {
            nodeIds.resize(objectCount);

            hapiResult = HAPI_GetComposedChildNodeList(
                    Util::theHAPISession.get(),
                    myAssetInfo.objectNodeId,
                    &nodeIds.front(),
                    objectCount
                    );
            CHECK_HAPI(hapiResult);
        }
        else
        {
            // Note that this can be either an empty Object subnet, or an actual
            // Object.
            nodeIds.push_back(myAssetInfo.objectNodeId);
        }
    }
    else if(myNodeInfo.type == HAPI_NODETYPE_SOP)
    {
        nodeIds.push_back(myAssetInfo.objectNodeId);
    }

    // Create the OutputObjects
    if(myObjects.size() != nodeIds.size())
    {
        for(OutputObjects::const_iterator iter = myObjects.begin();
                iter != myObjects.end(); iter++)
        {
            delete *iter;
        }

        myObjects.resize(nodeIds.size());
        for(size_t i = 0; i < myObjects.size(); i++)
        {
            myObjects[i] = OutputObject::createObject(
                    nodeIds[i]
                    );
        }
    }
}

void
Asset::computeInstancerObjects(
        const MPlug& plug,
        MDataBlock& data,
        MIntArray &instancedObjIds,
        MStringArray &instancedObjNames,
        AssetNodeOptions::AccessorDataBlock &options,
        bool &needToSyncOutputs
        )
{
    MStatus stat;

    MPlug instancersPlug = plug.child(AssetNode::outputInstancers);

    int newSize = std::count_if(myObjects.begin(), myObjects.end(),
            [](OutputObject* obj){
                return obj->type() == OutputObject::OBJECT_TYPE_INSTANCER;
            });

    MArrayDataHandle instancersHandle = data.outputArrayValue(instancersPlug);
    if(instancersHandle.elementCount() != (unsigned int) newSize)
    {
        Util::resizeArrayDataHandle(instancersHandle , newSize);
        needToSyncOutputs = true;
    }

    int instancerIndex = 0;
    for(OutputObjects::const_iterator iter = myObjects.begin();
            iter != myObjects.end(); iter++)
    {
        OutputObject* obj = *iter;

        if(obj->type() == OutputObject::OBJECT_TYPE_INSTANCER)
        {
            MPlug instancerElemPlug = instancersPlug.elementByLogicalIndex(instancerIndex);
            CHECK_MSTATUS(instancersHandle .jumpToArrayElement(instancerIndex));
            MDataHandle instancerHandle = instancersHandle .outputValue();
            stat = dynamic_cast< OutputInstancerObject* >(obj)->compute(
                    myTime,
                    instancerElemPlug,
                    data,
                    instancerHandle,
                    options,
                    needToSyncOutputs
                    );
            if(MS::kSuccess == stat)
            {
                // collect all the objects that are being instanced
                MStringArray instNames = dynamic_cast< OutputInstancerObject* >(obj)->getUniqueInstObjNames();
                for(unsigned int j = 0; j < instNames.length(); ++j)
                {
                    instancedObjNames.append(instNames[j]);
                }
                MIntArray instIds = dynamic_cast< OutputInstancerObject* >(obj)->getInstancedObjIds();
                for(unsigned int j = 0; j < instIds.length(); ++j)
                {
                    instancedObjIds.append(instIds[j]);
                }
            }

            instancerIndex++;
        }
    }

    instancersHandle.setAllClean();
    data.setClean(instancersPlug);
}

void
Asset::computeGeometryObjects(
        const MPlug& plug,
        MDataBlock& data,
        const MIntArray &instancedObjIds,
        const MStringArray &instancedObjNames,
        AssetNodeOptions::AccessorDataBlock &options,
        bool &needToSyncOutputs
        )
{
    MStatus stat;

    MPlug objectsPlug = plug.child(AssetNode::outputObjects);

    MArrayDataHandle objectsHandle = data.outputArrayValue(objectsPlug);
    if(objectsHandle.elementCount() != myObjects.size())
    {
        Util::resizeArrayDataHandle(objectsHandle, myObjects.size());
        needToSyncOutputs = true;
    }

    for(unsigned int i = 0; i < myObjects.size(); i++)
    {
        OutputObject * obj = myObjects[i];

        MPlug objectPlug = objectsPlug.elementByLogicalIndex(i);
        CHECK_MSTATUS(objectsHandle.jumpToArrayElement(i));
        MDataHandle objectHandle = objectsHandle.outputValue();

        if(obj->type() == OutputObject::OBJECT_TYPE_GEOMETRY)
        {
            dynamic_cast< OutputGeometryObject* >(obj)->compute(
                    myTime,
                    objectPlug,
                    data,
                    objectHandle,
                    instancedObjIds,
                    instancedObjNames,
                    options,
                    needToSyncOutputs
                    );
        }
    }

    objectsHandle.setAllClean();

    data.setClean(objectsPlug);
}

void
Asset::computeMaterial(
        const MPlug& plug,
        MDataBlock& data,
        bool &needToSyncOutputs
        )
{
    MStatus status;

    MPlug materialsPlug = plug.child(AssetNode::outputMaterials);
    size_t numElements = materialsPlug.numElements();
    if(myMaterials.size() < numElements)
    {
        myMaterials.reserve(numElements);
        for(size_t i = myMaterials.size(); i < numElements; i++)
        {
            myMaterials.push_back(new OutputMaterial(myAssetInfo.nodeId));
        }
    }

    for(size_t i = 0; i < numElements; i++)
    {
        MPlug materialPlug = materialsPlug.elementByLogicalIndex(i);
        MDataHandle materialHandle = data.outputValue(materialPlug);

        myMaterials[i]->compute(myTime, materialPlug, data, materialHandle);
    }
}

MTime
Asset::getTime() const
{
    return myTime;
}

void
Asset::setTime(const MTime &mayaTime)
{
    myTime = mayaTime;

    // Houdini's "frame 1" is "0 seconds", but Maya's "frame 0" is "0 seconds".
    // So we need to offset the time by 1.
    MTime hapiTime = myTime - MTime(1, MTime::uiUnit());
    float hapiTimeSeconds = (float)hapiTime.as(MTime::kSeconds);
    HAPI_SetTime(
            Util::theHAPISession.get(),
            hapiTimeSeconds
            );
}

void
Asset::setInputs(const MPlug& plug, MDataBlock& data)
{
    assert(myNodeInfo.id >= 0);

    MStatus status;

    myAssetInputs->compute(data);

    for(int i=0; i< myNodeInfo.inputCount; i++)
    {
        MPlug elementPlug = plug.elementByLogicalIndex(i);
        MPlug inputNodeIdPlug = elementPlug.child(AssetNode::inputNodeId);

        MDataHandle inputNodeIdHandle = data.inputValue(inputNodeIdPlug);

        HAPI_NodeId inputNodeId = -1;

        // only use inputNodeId if it's coming from a connection
        if(!Util::plugSource(inputNodeIdPlug).isNull())
        {
            inputNodeId = inputNodeIdHandle.asInt();
        }

        if(inputNodeId < 0)
        {
            HAPI_DisconnectNodeInput(
                    Util::theHAPISession.get(),
                    myNodeInfo.id, i
                    );
            continue;
        }

        CHECK_HAPI(HAPI_ConnectNodeInput(
            Util::theHAPISession.get(),
            myNodeInfo.id, i,
            inputNodeId,
            0
            ));
    }
}

MStatus
Asset::compute(
        const MPlug& plug,
        MDataBlock& data,
        AssetNodeOptions::AccessorDataBlock &options,
        bool &needToSyncOutputs
        )
{
    assert(myNodeInfo.id >= 0);

    HAPI_Result hapiResult;

    MStatus stat(MS::kSuccess);

    {
        Util::PythonInterpreterLock pythonInterpreterLock;

        HAPI_CookOptions cookOptions;
        HAPI_CookOptions_Init(&cookOptions);
        cookOptions.splitGeosByGroup = options.splitGeosByGroup();
        cookOptions.cookTemplatedGeos = options.outputTemplatedGeometries();

        if(options.useInstancerNode())
        {
            // Particle instancer cannot instance other particle instancer. So
            // we can only do flatten.
            cookOptions.packedPrimInstancingMode = HAPI_PACKEDPRIM_INSTANCING_MODE_FLAT;
        }
        else
        {
            cookOptions.packedPrimInstancingMode = HAPI_PACKEDPRIM_INSTANCING_MODE_HIERARCHY;
        }

        hapiResult = HAPI_CookNode(
                Util::theHAPISession.get(),
                myNodeInfo.id,
                &cookOptions
                );
        CHECK_HAPI(hapiResult);

        if(!Util::statusCheckLoop())
        {
            GET_HAPI_STATUS_COOK();
            DISPLAY_MSG(displayError, hapiStatus);

            return MStatus::kFailure;
        }
    }

    update();

    // output asset transform
    {
        MPlug assetTransformPlug = plug.child(
                AssetNode::outputAssetTransform);

        MDataHandle assetTranslateHandle = data.outputValue(
                assetTransformPlug .child(AssetNode::outputAssetTranslate));
        MDataHandle assetRotateHandle = data.outputValue(
                assetTransformPlug .child(AssetNode::outputAssetRotate));
        MDataHandle assetScaleHandle = data.outputValue(
                assetTransformPlug .child(AssetNode::outputAssetScale));

        if(myNodeInfo.type == HAPI_NODETYPE_OBJ && myIsObjSubnet)
        {
            HAPI_Transform trans;
            HAPI_GetObjectTransform(
                    Util::theHAPISession.get(),
                    myAssetInfo.objectNodeId,
                    -1,
                    HAPI_SRT,
                    &trans
                    );

            MEulerRotation eulerRotation = MQuaternion(
                    trans.rotationQuaternion[0],
                    trans.rotationQuaternion[1],
                    trans.rotationQuaternion[2],
                    trans.rotationQuaternion[3]
                    ).asEulerRotation();


            assetTranslateHandle.set3Double(
                    trans.position[0],
                    trans.position[1],
                    trans.position[2]);
            assetRotateHandle.set3Double(
                    eulerRotation.x,
                    eulerRotation.y,
                    eulerRotation.z);
            assetScaleHandle.set3Double(
                    trans.scale[0],
                    trans.scale[1],
                    trans.scale[2]);
        } else {
            assetTranslateHandle.set3Double(0.0, 0.0, 0.0);
            assetRotateHandle.set3Double(0.0, 0.0, 0.0);
            assetScaleHandle.set3Double(1.0, 1.0, 1.0);
        }
    }

    // computeInstancerObjects() needs to collect all the objects being
    // instanced, and then computeGeometryObjects() needs to mark the objects
    // being instanced accordingly.
    MIntArray instancedObjIds;
    MStringArray instancedObjNames;
    computeInstancerObjects(plug, data,
            instancedObjIds,
            instancedObjNames,
            options,
            needToSyncOutputs);

    computeGeometryObjects(plug, data,
            instancedObjIds,
            instancedObjNames,
            options,
            needToSyncOutputs);

    computeMaterial(plug, data, needToSyncOutputs);

    return stat;
}

class GetMultiparmLengthOperation : public AttrOperation
{
    public:
        GetMultiparmLengthOperation(
                MDataBlock &dataBlock,
                const MFnDependencyNode &nodeFn,
                const HAPI_NodeInfo &nodeInfo,
                const std::vector<MObject>* attrs
                );

        virtual void pushMultiparm(const HAPI_ParmInfo &parmInfo);
};

GetMultiparmLengthOperation::GetMultiparmLengthOperation(
        MDataBlock &dataBlock,
        const MFnDependencyNode &nodeFn,
        const HAPI_NodeInfo &nodeInfo,
        const std::vector<MObject>* attrs
        ) :
    AttrOperation(
            dataBlock,
            AttrOperation::Get,
            nodeFn,
            nodeInfo,
            attrs
            )
{
}

void
GetMultiparmLengthOperation::pushMultiparm(const HAPI_ParmInfo &parmInfo)
{
    MStatus status;

    AttrOperation::pushMultiparm(parmInfo);

    //MDataHandle &dataHandle = myDataHandles.back();
    //MPlug &plug = myPlugs.back();
    //bool exists = myExists.back();
    //const HAPI_ParmInfo* &parentParmInfo = myParentParmInfos.back();

    bool isMulti = myIsMulti.back();
    //MDataHandle &multiSizeDataHandle = myMultiSizeDataHandles.back();
    //MPlug &multiSizePlug = myMultiSizePlugs.back();
    bool hasMultiAttr = myHasMultiAttr.back();
    MArrayDataHandle &multiDataHandle = myMultiDataHandles.back();
    //MPlug &multiPlug = myMultiPlugs.back();
    //int &multiLogicalIndex = myMultiLogicalIndices.back();

    MString attrName = getAttrNameFromParm(parmInfo);
    if(parmInfo.rampType == HAPI_RAMPTYPE_INVALID)
    {
        attrName += "__multiSize";
    }
    if(isMulti && containsParm(attrName, parmInfo))
    {
        int multiSize = parmInfo.instanceCount;

        if(hasMultiAttr)
        {
            MArrayDataBuilder builder = multiDataHandle.builder(&status);
            CHECK_MSTATUS(status);

            const int builderCount = builder.elementCount();

            // If the builder has less elements than the multiparm, then we need to
            // add to the builder.
            for(int i = builderCount; i < multiSize; ++i)
            {
                builder.addElement(i);
            }

            // If the builder has more elements than the multiparm, then we need to
            // remove from the builder.
            for(int i = builderCount; i-- > multiSize;)
            {
                builder.removeElement(i);
            }

            multiDataHandle.set(builder);
        }
    }
}

class GetAttrOperation : public AttrOperation
{
    public:
        GetAttrOperation(
                MDataBlock &dataBlock,
                const MFnDependencyNode &nodeFn,
                const HAPI_NodeInfo &nodeInfo,
                const std::vector<MObject>* attrs
                );

        virtual void pushMultiparm(const HAPI_ParmInfo &parmInfo);
        virtual void leaf(const HAPI_ParmInfo &parmInfo);
};

GetAttrOperation::GetAttrOperation(
        MDataBlock &dataBlock,
        const MFnDependencyNode &nodeFn,
        const HAPI_NodeInfo &nodeInfo,
        const std::vector<MObject>* attrs
        ) :
    AttrOperation(
            dataBlock,
            AttrOperation::Get,
            nodeFn,
            nodeInfo,
            attrs
            )
{
}

void
GetAttrOperation::pushMultiparm(const HAPI_ParmInfo &parmInfo)
{
    MStatus status;

    AttrOperation::pushMultiparm(parmInfo);

    //MDataHandle &dataHandle = myDataHandles.back();
    //MPlug &plug = myPlugs.back();
    //bool exists = myExists.back();
    //const HAPI_ParmInfo* &parentParmInfo = myParentParmInfos.back();

    bool isMulti = myIsMulti.back();
    MDataHandle &multiSizeDataHandle = myMultiSizeDataHandles.back();
    //MPlug &multiSizePlug = myMultiSizePlugs.back();
    //bool hasMultiAttr = myHasMultiAttr.back();
    //MArrayDataHandle &multiDataHandle = myMultiDataHandles.back();
    //MPlug &multiPlug = myMultiPlugs.back();
    //int &multiLogicalIndex = myMultiLogicalIndices.back();

    MString attrName = getAttrNameFromParm(parmInfo);
    attrName += "__multiSize";
    if(isMulti && containsParm(attrName, parmInfo))
    {
        int multiSize = parmInfo.instanceCount;

        // Ramp parameters doesn't have the "__multiSize" attribute.
        if(parmInfo.rampType == HAPI_RAMPTYPE_INVALID)
        {
            multiSizeDataHandle.setInt(multiSize);
        }
    }
}

void
GetAttrOperation::leaf(const HAPI_ParmInfo &parmInfo)
{
    MStatus status;

    MDataHandle dataHandle;
    MPlug plug;
    bool exists = false;

    MDataHandle &parentDataHandle = myDataHandles.back();
    MPlug &parentPlug = myPlugs.back();
    bool parentExists = myExists.back();
    const HAPI_ParmInfo* parentParmInfo
        = parentExists ? myParentParmInfos.back() : NULL;
    
    MString attrName;
    if (parentExists && parentParmInfo && parmInfo.isChildOfMultiParm)
        attrName = Util::getAttrNameFromParm(parmInfo, parentParmInfo);
    else
        attrName = getAttrNameFromParm(parmInfo);

    if(parentExists && containsParm(attrName, parmInfo))
    {
        MObject attrObj = myNodeFn.attribute(attrName);
        if(!attrObj.isNull())
        {
            plug = parentPlug.child(attrObj, &status);
            if(status)
            {
                exists = true;
            }
        }

        if(exists)
        {
            // if it's not a ramp, go ahead and lock based on disable
            // leave the ramps alone cause there are other UI issues that complicate things
            if(!(parmInfo.isChildOfMultiParm
              && parentParmInfo
              && parentParmInfo->rampType != HAPI_RAMPTYPE_INVALID)) {

                if(parmInfo.disabled) {
                    plug.setLocked(true);
                } else {
                    if(plug.isLocked()) {
                        plug.setLocked(false);
                    }
                }
            }

            dataHandle = parentDataHandle.child(attrObj);

            // The HAPI_ParmInfo::choiceCount could change between cooks because
            // of menu scripts. Explicitly check if the attribute is
            // MFnEnumAttribute to avoid Maya crashing when calling
            // MDataHandle::setShort() on a string attribute.
            if(attrObj.hasFn(MFn::kEnumAttribute)
                    && (parmInfo.type == HAPI_PARMTYPE_INT
                        || parmInfo.type == HAPI_PARMTYPE_BUTTON
                        || parmInfo.type == HAPI_PARMTYPE_STRING
                        || parmInfo.type == HAPI_PARMTYPE_PATH_FILE
                        || parmInfo.type == HAPI_PARMTYPE_PATH_FILE_DIR
                        || parmInfo.type == HAPI_PARMTYPE_PATH_FILE_GEO
                        || parmInfo.type == HAPI_PARMTYPE_PATH_FILE_IMAGE)
                    && parmInfo.choiceCount > 0)
            {
                int enumIndex = 0;

                if(parmInfo.type == HAPI_PARMTYPE_BUTTON)
                {
                    // The value of button menu items is irrelevant. We need it
                    // to always stay at the first dummy field regardless of
                    // what the actual value is.
                    enumIndex = 0;
                }
                else if(parmInfo.type == HAPI_PARMTYPE_STRING
                        || parmInfo.type == HAPI_PARMTYPE_PATH_FILE
                        || parmInfo.type == HAPI_PARMTYPE_PATH_FILE_DIR
                        || parmInfo.type == HAPI_PARMTYPE_PATH_FILE_GEO
                        || parmInfo.type == HAPI_PARMTYPE_PATH_FILE_IMAGE)
                {
                    int value;
                    HAPI_GetParmStringValues(
                            Util::theHAPISession.get(),
                            myNodeInfo.id,
                            false,
                            &value,
                            parmInfo.stringValuesIndex, parmInfo.size
                            );
                    MString valueString = Util::HAPIString(value);

                    HAPI_ParmChoiceInfo * choiceInfos =
                        new HAPI_ParmChoiceInfo[parmInfo.choiceCount];
                    HAPI_GetParmChoiceLists(
                            Util::theHAPISession.get(),
                            myNodeInfo.id,
                            choiceInfos,
                            parmInfo.choiceIndex, parmInfo.choiceCount
                            );

                    for(int i = 0; i < parmInfo.choiceCount; i++)
                    {
                        if(valueString == Util::HAPIString(choiceInfos[i].valueSH))
                        {
                            enumIndex = i;
                        }
                    }

                    delete[] choiceInfos;
                }
                else
                {
                    HAPI_GetParmIntValues(
                            Util::theHAPISession.get(),
                            myNodeInfo.id,
                            &enumIndex,
                            parmInfo.intValuesIndex, parmInfo.size
                            );

                    if(parmInfo.isChildOfMultiParm
                            && parentParmInfo
                            && parentParmInfo->rampType != HAPI_RAMPTYPE_INVALID)
                    {
                        switch(enumIndex)
                        {
                            case 0:
                                // constant -> constant
                            case 1:
                                // linear -> linear
                            case 3:
                                // monotonecubic -> spline
                                break;
                            default:
                                // no equivalent
                                enumIndex = 1;
                                break;
                        }
                    }
                }

                dataHandle.setShort(static_cast<short>(enumIndex));
            }
            else
            {
                switch(parmInfo.type)
                {
                    case HAPI_PARMTYPE_FLOAT:
                    case HAPI_PARMTYPE_COLOR:
                        {
                            float* values = new float[parmInfo.size];
                            HAPI_GetParmFloatValues(
                                    Util::theHAPISession.get(),
                                    myNodeInfo.id,
                                    values,
                                    parmInfo.floatValuesIndex, parmInfo.size
                                    );

                            if(parmInfo.size == 1)
                            {
                                dataHandle.setFloat(values[0]);
                            }
                            else
                            {
                                MFnCompoundAttribute attrFn(attrObj);
                                for(int i = 0;
                                        i < (int) attrFn.numChildren()
                                        && i < (int) parmInfo.size;
                                        i++)
                                {
                                    MDataHandle elementDataHandle =
                                        dataHandle.child(attrFn.child(i));
                                    elementDataHandle.setFloat(values[i]);
                                }
                            }

                            delete[] values;
                        }
                        break;
                    case HAPI_PARMTYPE_INT:
                    case HAPI_PARMTYPE_TOGGLE:
                    case HAPI_PARMTYPE_BUTTON:
                        {
                            int* values = new int[parmInfo.size];

                            if(parmInfo.type == HAPI_PARMTYPE_BUTTON)
                            {
                                // For buttons, always keep the attribute value
                                // at 0, so that we can trigger it by setting
                                // the attribute to 1.

                                for(int i = 0; i < parmInfo.size; i++)
                                {
                                    values[i] = 0;
                                }
                            }
                            else
                            {
                                HAPI_GetParmIntValues(
                                        Util::theHAPISession.get(),
                                        myNodeInfo.id,
                                        values,
                                        parmInfo.intValuesIndex, parmInfo.size
                                        );
                            }

                            if(parmInfo.size == 1)
                            {
                                if(parmInfo.type == HAPI_PARMTYPE_TOGGLE
                                        || parmInfo.choiceCount > 0)
                                {
                                    dataHandle.setShort(values[0]);
                                }
                                else
                                {
                                    dataHandle.setInt(values[0]);
                                }
                            }
                            else
                            {
                                MFnCompoundAttribute attrFn(attrObj);
                                for(int i = 0;
                                        i < (int) attrFn.numChildren()
                                        && i < (int) parmInfo.size;
                                        i++)
                                {
                                    MDataHandle elementDataHandle = dataHandle.child(attrFn.child(i));
                                    if(parmInfo.type == HAPI_PARMTYPE_TOGGLE
                                            || parmInfo.choiceCount > 0)
                                    {
                                        elementDataHandle.setShort(values[i]);
                                    }
                                    else
                                    {
                                        elementDataHandle.setInt(values[i]);
                                    }
                                }
                            }

                            delete[] values;
                        }
                        break;
                    case HAPI_PARMTYPE_STRING:
                    case HAPI_PARMTYPE_PATH_FILE:
                    case HAPI_PARMTYPE_PATH_FILE_DIR:
                    case HAPI_PARMTYPE_PATH_FILE_GEO:
                    case HAPI_PARMTYPE_PATH_FILE_IMAGE:
                        {
                            int* values = new int[parmInfo.size];
                            HAPI_GetParmStringValues(
                                    Util::theHAPISession.get(),
                                    myNodeInfo.id,
                                    false,
                                    values,
                                    parmInfo.stringValuesIndex, parmInfo.size
                                    );

                            if(parmInfo.size == 1)
                            {
                                dataHandle.setString(Util::HAPIString(values[0]));
                            }
                            else
                            {
                                MFnCompoundAttribute attrFn(attrObj);
                                for(int i = 0;
                                        i < (int) attrFn.numChildren()
                                        && i < (int) parmInfo.size;
                                        i++)
                                {
                                    MDataHandle elementDataHandle =
                                        dataHandle.child(attrFn.child(i));
                                    elementDataHandle.setString(Util::HAPIString(values[i]));
                                }
                            }

                            delete[] values;
                        }
                        break;
                    case HAPI_PARMTYPE_NODE:
                        // nothing need to be done
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void
Asset::fillParmNameCache()
{
    auto num_parms = myNodeInfo.parmCount;
    std::vector<HAPI_ParmInfo> parmInfos(num_parms);
    HAPI_GetParameters(
        Util::theHAPISession.get(),
        myNodeInfo.id,
        &parmInfos[0],
        0, num_parms
    );

    std::vector<HAPI_StringHandle> parmNameHandles;
    parmNameHandles.reserve(num_parms);
    for (auto&& info : parmInfos)
        parmNameHandles.push_back(info.nameSH);

    int stringsbuffer_len;
    HAPI_GetStringBatchSize(
        Util::theHAPISession.get(),
        &parmNameHandles[0],
        num_parms,
        &stringsbuffer_len
    );

    std::vector<char> strings_buffer(stringsbuffer_len);
    HAPI_GetStringBatch(
        Util::theHAPISession.get(),
        &strings_buffer[0],
        stringsbuffer_len
    );

    myParmNameCache->clearCache();
    auto read_iter = strings_buffer.begin();
    for (int i = 0; i < num_parms; ++i)
    {
        auto next_null = std::find(read_iter, strings_buffer.end(), '\0');
        MString parmname(&(*read_iter));
        // FIXME: Ramp name mangling requires looking at parent parm, so
        // we can't cache the name until we actually traverse the parms.
        if (!parmInfos[i].isChildOfMultiParm)
        {
            parmname = Util::mangleParmAttrName(parmInfos[i], parmname);
            myParmNameCache->cacheName(parmInfos[i], parmname);
        }
        read_iter = next_null + 1;
    }
}

void
Asset::getParmValues(
        MDataBlock &dataBlock,
        const MFnDependencyNode &nodeFn,
        const std::vector<MObject>* attrs
        )
{
    MStatus status;

    if(myNodeInfo.parmCount == 0)
    {
        return;
    }

    std::vector<HAPI_ParmInfo> parmInfos;

    // Get multiparm length
    {
        parmInfos.resize(myNodeInfo.parmCount);
        HAPI_GetParameters(
                Util::theHAPISession.get(),
                myNodeInfo.id,
                &parmInfos[0],
                0, parmInfos.size()
                );

        GetMultiparmLengthOperation operation(
                dataBlock,
                nodeFn,
                myNodeInfo,
                attrs
                );
        Util::walkParm(parmInfos, operation);

    }

    // Get value
    {
        parmInfos.resize(myNodeInfo.parmCount);
        HAPI_GetParameters(
                Util::theHAPISession.get(),
                myNodeInfo.id,
                &parmInfos[0],
                0, parmInfos.size()
                );

        GetAttrOperation operation(
                dataBlock,
                nodeFn,
                myNodeInfo,
                attrs
                );
        Util::walkParm(parmInfos, operation);
    }
}

class SetMultiparmLengthOperation : public AttrOperation
{
    public:
        SetMultiparmLengthOperation(
                MDataBlock &dataBlock,
                const MFnDependencyNode &nodeFn,
                const HAPI_NodeInfo &nodeInfo,
                const std::vector<MObject>* attrs
                );

        virtual void pushMultiparm(const HAPI_ParmInfo &parmInfo);
};

SetMultiparmLengthOperation::SetMultiparmLengthOperation(
        MDataBlock &dataBlock,
        const MFnDependencyNode &nodeFn,
        const HAPI_NodeInfo &nodeInfo,
        const std::vector<MObject>* attrs
        ) :
    AttrOperation(
            dataBlock,
            AttrOperation::Set,
            nodeFn,
            nodeInfo,
            attrs
            )
{
}

void
SetMultiparmLengthOperation::pushMultiparm(const HAPI_ParmInfo &parmInfo)
{
    MStatus status;

    AttrOperation::pushMultiparm(parmInfo);

    //MDataHandle &dataHandle = myDataHandles.back();
    //MPlug &plug = myPlugs.back();
    //bool exists = myExists.back();
    //const HAPI_ParmInfo* &parentParmInfo = myParentParmInfos.back();

    bool isMulti = myIsMulti.back();
    MDataHandle &multiSizeDataHandle = myMultiSizeDataHandles.back();
    //MPlug &multiSizePlug = myMultiSizePlugs.back();
    bool hasMultiAttr = myHasMultiAttr.back();
    MArrayDataHandle &multiDataHandle = myMultiDataHandles.back();
    //MPlug &multiPlug = myMultiPlugs.back();
    //int &multiLogicalIndex = myMultiLogicalIndices.back();

    MString attrName = getAttrNameFromParm(parmInfo);
    if(parmInfo.rampType == HAPI_RAMPTYPE_INVALID)
    {
        attrName += "__multiSize";
    }
    if(isMulti && containsParm(attrName, parmInfo))
    {
        int multiSize = -1;
        if(parmInfo.rampType != HAPI_RAMPTYPE_INVALID)
        {
            if(hasMultiAttr)
            {
                MArrayDataBuilder builder = multiDataHandle.builder(&status);
                multiSize = builder.elementCount();
            }
        }
        else
        {
            multiSize = multiSizeDataHandle.asInt();
        }

        if(multiSize != -1)
        {
            // If the multiparm has less instances than the multiDataHandle, then
            // we need to add to the multiparm.
            for(int i = parmInfo.instanceCount; i < multiSize; ++i)
            {
                HAPI_InsertMultiparmInstance(
                        Util::theHAPISession.get(),
                        myNodeInfo.id,
                        parmInfo.id,
                        i + parmInfo.instanceStartOffset
                        );
            }

            // If the multiparm has more instances than the multiDataHandle, then
            // we need to remove from the multiparm.
            for(int i = parmInfo.instanceCount; i-- > multiSize;)
            {
                HAPI_RemoveMultiparmInstance(
                        Util::theHAPISession.get(),
                        myNodeInfo.id,
                        parmInfo.id,
                        i + parmInfo.instanceStartOffset
                        );
            }
        }
    }
}

class SetAttrOperation : public AttrOperation
{
    public:
        SetAttrOperation(
                MDataBlock &dataBlock,
                const MFnDependencyNode &nodeFn,
                const HAPI_NodeInfo &nodeInfo,
                const std::vector<MObject>* attrs
                );

        virtual void leaf(const HAPI_ParmInfo &parmInfo);
};

SetAttrOperation::SetAttrOperation(
        MDataBlock &dataBlock,
        const MFnDependencyNode &nodeFn,
        const HAPI_NodeInfo &nodeInfo,
        const std::vector<MObject>* attrs
        ) :
    AttrOperation(
            dataBlock,
            AttrOperation::Set,
            nodeFn,
            nodeInfo,
            attrs
            )
{
}

void
SetAttrOperation::leaf(const HAPI_ParmInfo &parmInfo)
{
    MStatus status;

    MDataHandle dataHandle;
    MPlug plug;
    bool exists = false;

    MDataHandle &parentDataHandle = myDataHandles.back();
    MPlug &parentPlug = myPlugs.back();
    bool parentExists = myExists.back();
    const HAPI_ParmInfo* parentParmInfo
        = parentExists ? myParentParmInfos.back() : NULL;

    MString attrName;
    if (parentExists && parentParmInfo && parmInfo.isChildOfMultiParm)
        attrName = Util::getAttrNameFromParm(parmInfo, parentParmInfo);
    else
        attrName = getAttrNameFromParm(parmInfo);
    if(!parentExists && containsParm(attrName, parmInfo)) myMismatch = true;

    if(parentExists && containsParm(attrName, parmInfo))
    {
        MObject attrObj = myNodeFn.attribute(attrName);
        if(!attrObj.isNull())
        {
            plug = parentPlug.child(attrObj, &status);
            if(status)
            {
                exists = true;
            }
        }

        if(exists)
        {
            dataHandle = parentDataHandle.child(attrObj);

            if((parmInfo.type == HAPI_PARMTYPE_INT
                        || parmInfo.type == HAPI_PARMTYPE_BUTTON
                        || parmInfo.type == HAPI_PARMTYPE_STRING
                        || parmInfo.type == HAPI_PARMTYPE_PATH_FILE
                        || parmInfo.type == HAPI_PARMTYPE_PATH_FILE_DIR
                        || parmInfo.type == HAPI_PARMTYPE_PATH_FILE_GEO
                        || parmInfo.type == HAPI_PARMTYPE_PATH_FILE_IMAGE)
                    && parmInfo.choiceCount > 0)
            {
                int enumIndex = static_cast<int>(dataHandle.asShort());

                if(parmInfo.type == HAPI_PARMTYPE_BUTTON)
                {
                    // Button menu items have a dummy field at the beginning.
                    int value = enumIndex - 1;
                    if(value >= 0)
                    {
                        HAPI_SetParmIntValues(
                                Util::theHAPISession.get(),
                                myNodeInfo.id,
                                &value,
                                parmInfo.intValuesIndex, 1
                                );
                    }
                }
                else if(parmInfo.type == HAPI_PARMTYPE_STRING
                        || parmInfo.type == HAPI_PARMTYPE_PATH_FILE
                        || parmInfo.type == HAPI_PARMTYPE_PATH_FILE_DIR
                        || parmInfo.type == HAPI_PARMTYPE_PATH_FILE_GEO
                        || parmInfo.type == HAPI_PARMTYPE_PATH_FILE_IMAGE)
                {
                    HAPI_ParmChoiceInfo * choiceInfos = new HAPI_ParmChoiceInfo[parmInfo.choiceCount];
                    HAPI_GetParmChoiceLists(
                            Util::theHAPISession.get(),
                            myNodeInfo.id,
                            choiceInfos,
                            parmInfo.choiceIndex, parmInfo.choiceCount);

                    MString valueString = Util::HAPIString(choiceInfos[enumIndex].valueSH);

                    HAPI_SetParmStringValue(
                            Util::theHAPISession.get(),
                            myNodeInfo.id,
                            valueString.asChar(),
                            parmInfo.id,
                            0
                            );

                    delete[] choiceInfos;
                }
                else
                {
                    if(parmInfo.isChildOfMultiParm
                            && parentParmInfo
                            && parentParmInfo->rampType != HAPI_RAMPTYPE_INVALID)
                    {
                        switch(enumIndex)
                        {
                            case 0:
                                // constant -> constant
                            case 1:
                                // linear -> linear
                            case 3:
                                // spline -> monotonecubic
                                break;
                            case 2:
                                // no equivalent for smooth
                            default:
                                enumIndex = 1;
                                break;
                        }
                    }

                    HAPI_SetParmIntValues(
                            Util::theHAPISession.get(),
                            myNodeInfo.id,
                            &enumIndex,
                            parmInfo.intValuesIndex, 1
                            );
                }
            }
            else
            {
                switch(parmInfo.type)
                {
                    case HAPI_PARMTYPE_FLOAT:
                    case HAPI_PARMTYPE_COLOR:
                        {
                            float * values = new float[parmInfo.size];
                            if(parmInfo.size == 1)
                            {
                                values[0] = dataHandle.asFloat();
                            } else
                            {
                                MFnCompoundAttribute attrFn(attrObj);
                                for(int i=0; i<parmInfo.size; i++)
                                {
                                    //MDataHandle elementHandle = dataHandle.child(attrFn.child(i));
                                    MDataHandle elementHandle = myDataBlock.inputValue(plug.child(i));
                                    values[i] = elementHandle.asFloat();
                                }
                            }
                            HAPI_SetParmFloatValues(
                                    Util::theHAPISession.get(),
                                    myNodeInfo.id,
                                    values,
                                    parmInfo.floatValuesIndex, parmInfo.size
                                    );

                            delete[] values;
                        }
                        break;
                    case HAPI_PARMTYPE_INT:
                    case HAPI_PARMTYPE_TOGGLE:
                    case HAPI_PARMTYPE_BUTTON:
                        {
                            int * values = new int[parmInfo.size];
                            if(parmInfo.size == 1)
                            {
                                if(parmInfo.type == HAPI_PARMTYPE_TOGGLE)
                                {
                                    values[0] = dataHandle.asBool();
                                }
                                else
                                {
                                    values[0] = dataHandle.asInt();
                                }
                            } else
                            {
                                MFnCompoundAttribute attrFn(attrObj);
                                for(int i=0; i<parmInfo.size; i++)
                                {
                                    //MDataHandle elementHandle = dataHandle.child(attrFn.child(i));
                                    MDataHandle elementHandle = myDataBlock.inputValue(plug.child(i));
                                    if(parmInfo.type == HAPI_PARMTYPE_TOGGLE)
                                    {
                                        values[i] = elementHandle.asBool();
                                    }
                                    else
                                    {
                                        values[i] = elementHandle.asInt();
                                    }
                                }
                            }

                            if(parmInfo.type == HAPI_PARMTYPE_BUTTON)
                            {
                                // For buttons, only set if the values are 1.
                                // This avoids clicking the buttons when saving
                                // and restoring attribute values.

                                int* currentValues = new int[parmInfo.size];

                                HAPI_GetParmIntValues(
                                        Util::theHAPISession.get(),
                                        myNodeInfo.id,
                                        currentValues,
                                        parmInfo.intValuesIndex, parmInfo.size
                                        );
                                for(int i = 0; i < parmInfo.size; i++)
                                {
                                    if(values[i] == 1)
                                    {
                                        // The actual value is irrelevant. Just
                                        // set it to the current value, so we
                                        // don't have to worry about the value
                                        // changing.
                                        HAPI_SetParmIntValues(
                                                Util::theHAPISession.get(),
                                                myNodeInfo.id,
                                                &currentValues[i],
                                                parmInfo.intValuesIndex + i, 1
                                                );
                                    }
                                }

                                delete [] currentValues;
                            }
                            else
                            {
                                HAPI_SetParmIntValues(
                                        Util::theHAPISession.get(),
                                        myNodeInfo.id,
                                        values,
                                        parmInfo.intValuesIndex, parmInfo.size
                                        );
                            }

                            delete[] values;
                        }
                        break;
                    case HAPI_PARMTYPE_STRING:
                    case HAPI_PARMTYPE_PATH_FILE:
                    case HAPI_PARMTYPE_PATH_FILE_DIR:
                    case HAPI_PARMTYPE_PATH_FILE_GEO:
                    case HAPI_PARMTYPE_PATH_FILE_IMAGE:
                        {
                            if(parmInfo.size == 1)
                            {
                                const char* val = dataHandle.asString().asChar();
                                HAPI_SetParmStringValue(
                                        Util::theHAPISession.get(),
                                        myNodeInfo.id,
                                        val,
                                        parmInfo.id,
                                        0
                                        );
                            } else
                            {
                                MFnCompoundAttribute attrFn(attrObj);
                                for(int i=0; i<parmInfo.size; i++)
                                {
                                    //MDataHandle elementHandle = dataHandle.child(attrFn.child(i));
                                    MDataHandle elementHandle = myDataBlock.inputValue(plug.child(i));
                                    const char* val = elementHandle.asString().asChar();
                                    HAPI_SetParmStringValue(
                                            Util::theHAPISession.get(), myNodeInfo.id,
                                            val,
                                            parmInfo.id,
                                            i
                                            );
                                }
                            }
                        }
                        break;
                    case HAPI_PARMTYPE_NODE:
                        {
                            HAPI_NodeId inputNodeId = -1;

                            // only use inputNodeId if it's coming from a
                            // connection
                            if(!Util::plugSource(plug).isNull())
                            {
                                inputNodeId = dataHandle.asInt();
                            }

                            std::string name = Util::HAPIString(parmInfo.nameSH);

                            CHECK_HAPI(HAPI_SetParmNodeValue(
                                        Util::theHAPISession.get(),
                                        myNodeInfo.id,
                                        name.c_str(),
                                        inputNodeId
                                        ));
                        }
                    default:
                        break;
                }
            }
        }
    }
}

void
Asset::setParmValues(
        MDataBlock &dataBlock,
        const MFnDependencyNode &nodeFn,
        const std::vector<MObject>* attrs,
	bool checkMismatch
        )
{
    MStatus status;

    if(myNodeInfo.parmCount == 0)
    {
        return;
    }

    std::vector<HAPI_ParmInfo> parmInfos;

    // Set multiparm length
    {
        parmInfos.resize(myNodeInfo.parmCount);
        HAPI_GetParameters(
                Util::theHAPISession.get(),
                myNodeInfo.id,
                &parmInfos[0],
                0, parmInfos.size()
                );

        SetMultiparmLengthOperation operation(
                dataBlock,
                nodeFn,
                myNodeInfo,
                attrs
                );
        Util::walkParm(parmInfos, operation);

        // multiparm length could change, so we need to get the new parmCount
        HAPI_GetNodeInfo(
                Util::theHAPISession.get(),
                myNodeInfo.id,
                &myNodeInfo
                );
    }

    // Set value
    {
        parmInfos.resize(myNodeInfo.parmCount);
        HAPI_GetParameters(
                Util::theHAPISession.get(), myNodeInfo.id,
                &parmInfos[0],
                0, parmInfos.size()
                );

        Util::PythonInterpreterLock pythonInterpreterLock;

        SetAttrOperation operation(
                dataBlock,
                nodeFn,
                myNodeInfo,
                attrs
                );
        Util::walkParm(parmInfos, operation);
	
	// if attrs was NULL, we're walking all the attributes
 	if(checkMismatch && operation.detectedMismatch()) {
	    DISPLAY_WARNING(MString(
                "Possible mismatch between attribute and parm structure"
		"some parms may not be set "
                "from attributes. Sync the asset to rebuild the attributes."
		 ));
	}
    }
}

MString 
Asset::getAttrNameFromParm(const HAPI_ParmInfo &parm) const
{
    MString name = myParmNameCache->getParmTemplateName(parm);
    if (name.length() > 0)
        return name;
    else
        return Util::getAttrNameFromParm(parm);
}
