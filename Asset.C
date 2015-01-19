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

#include "Asset.h"
#include "Input.h"
#include "AssetNode.h"
#include "OutputGeometryObject.h"
#include "OutputInstancerObject.h"
#include "util.h"

#include <cassert>

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

        bool containsParm(const HAPI_ParmInfo &parm) const;

    protected:
        std::vector<MDataHandle> myDataHandles;
        std::vector<MPlug> myPlugs;
        std::vector<bool> myExists;

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
    myAttrs(attrs)
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
}

AttrOperation::~AttrOperation()
{
    myDataHandles.pop_back();
    myPlugs.pop_back();
    myExists.pop_back();
}

void
AttrOperation::pushFolder(const HAPI_ParmInfo &parmInfo)
{
    MDataHandle dataHandle;
    MPlug plug;
    bool exists = false;

    MDataHandle &parentDataHandle = myDataHandles.back();
    MPlug &parentPlug = myPlugs.back();
    bool parentExists = myExists.back();

    if(parentExists)
    {
        MString folderAttrName = Util::getAttrNameFromParm(parmInfo);
        MObject folderAttrObj = myNodeFn.attribute(folderAttrName);

        if(!folderAttrObj.isNull())
        {
            exists = true;
            plug = parentPlug.child(folderAttrObj);
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

    if(parentExists)
    {
        MString multiAttrName = Util::getAttrNameFromParm(parmInfo);
        MObject multiSizeAttrObj = myNodeFn.attribute(multiAttrName + "__multiSize");
        MObject multiAttrObj = myNodeFn.attribute(multiAttrName);

        if(!multiSizeAttrObj.isNull())
        {
            multiSizeDataHandle = parentDataHandle.child(multiSizeAttrObj);
            multiSizePlug = parentPlug.child(multiSizeAttrObj);

            isMulti = true;
        }

        // multiAttrObj might not exist if the current instanceCount is 0
        if(isMulti && !multiAttrObj.isNull())
        {
            multiDataHandle = MArrayDataHandle(parentDataHandle.child(multiAttrObj));
            multiPlug = parentPlug.child(multiAttrObj);

            hasMultiAttr = true;
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

    //bool isMulti = myIsMulti.back();
    //MDataHandle &multiSizeDataHandle = myMultiSizeDataHandles.back();
    //MPlug &multiSizePlug = myMultiSizePlugs.back();
    bool hasMultiAttr = myHasMultiAttr.back();
    MArrayDataHandle &multiDataHandle = myMultiDataHandles.back();
    MPlug &multiPlug = myMultiPlugs.back();
    int &multiLogicalIndex = myMultiLogicalIndices.back();

    if(hasMultiAttr)
    {
        multiLogicalIndex++;
        status = multiDataHandle.jumpToElement(multiLogicalIndex);
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

            plug = multiPlug.elementByLogicalIndex(multiLogicalIndex);
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
}

bool
AttrOperation::containsParm(const HAPI_ParmInfo &parm) const
{
    if(!myAttrs)
    {
        return true;
    }

    MPlug parmPlug = myNodeFn.findPlug(
            Util::getAttrNameFromParm(parm)
            );

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

Asset::Asset(
        const MString &otlFilePath,
        const MString &assetName,
        const MObject &node
        ) :
    // initialize values here because instantiating the asset could error out
    myNumVisibleObjects(0),
    myNumObjects(0),
    myNode(node),
    myAssetInputs(NULL),
    myObjects(NULL),
    myObjectInfos(NULL)
{
    HAPI_Result hapiResult = HAPI_RESULT_SUCCESS;

    HAPI_AssetInfo_Init(&myAssetInfo);

    // load the otl
    int libraryId = -1;
    hapiResult = HAPI_LoadAssetLibraryFromFile(otlFilePath.asChar(), true, &libraryId);
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
        hapiResult = HAPI_GetAvailableAssetCount(libraryId, &assetCount);
        CHECK_HAPI(hapiResult);

        assetNamesSH.resize(assetCount);
        hapiResult = HAPI_GetAvailableAssets(libraryId, &assetNamesSH.front(), assetCount);
        CHECK_HAPI(hapiResult);
    }

    // find the asset in the otl
    if(assetNamesSH.size())
    {
        bool foundAsset = false;
        for(unsigned int i = 0; i < assetNamesSH.size(); i++)
        {
            if(Util::getString(assetNamesSH[i]) == assetName)
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
    int assetId = -1;
    {
        Util::PythonInterpreterLock pythonInterpreterLock;

        hapiResult = HAPI_InstantiateAsset(
                assetName.asChar(),
                false,
                &assetId
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

    // Cook the asset here so that we know the number of inputs and number of
    // objects to output. Ignore the error here, because the inputs and
    // parameters are not marshalled in yet.
    {
        Util::PythonInterpreterLock pythonInterpreterLock;

        HAPI_CookAsset(assetId, NULL);
        Util::statusCheckLoop();
    }

    hapiResult = HAPI_GetAssetInfo(assetId, &myAssetInfo);
    CHECK_HAPI(hapiResult);
    hapiResult = HAPI_GetNodeInfo(myAssetInfo.nodeId, & myNodeInfo);
    CHECK_HAPI(hapiResult);

    // Warn the user if the OTL path is not what was originally requested.
    if(Util::getString(myAssetInfo.filePathSH) != otlFilePath)
    {
        DISPLAY_WARNING("The asset: ^1s\n"
                "was instantiated from: ^2s\n"
                "but the expected path was: ^3s",
                assetName,
                Util::getString(myAssetInfo.filePathSH),
                otlFilePath
                );
    }

    myAssetInputs = new Inputs(myAssetInfo.id);
    myAssetInputs->setNumInputs(myAssetInfo.geoInputCount);

    // get the infos
    update();

    // objects
    unsigned int objCount = myAssetInfo.objectCount;
    myObjects = new OutputObject*[objCount];
    myNumVisibleObjects = 0;
    myNumObjects = objCount;

    for(unsigned int i=0; i<objCount; i++)
    {
        myObjects[i] = OutputObject::createObject(myAssetInfo.id, i, this);
    }
}

Asset::~Asset()
{
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;

    for(unsigned int i=0; i< myNumObjects; i++)
        delete myObjects[i];
    delete[] myObjects;
    delete[] myObjectInfos;
    delete myAssetInputs;

    if(myAssetInfo.id >= 0)
    {
        hstat = HAPI_DestroyAsset(myAssetInfo.id);
        Util::checkHAPIStatus(hstat);
    }
}

bool
Asset::isValid() const
{
    return myAssetInfo.id >= 0;
}

MString
Asset::getOTLFilePath() const
{
    if(!isValid())
    {
        return MString();
    }

    return Util::getString(myAssetInfo.filePathSH);
}

MString
Asset::getAssetName() const
{
    if(!isValid())
    {
        return MString();
    }

    return Util::getString(myAssetInfo.fullOpNameSH);
}

OutputObject*
Asset::findObjectByName(MString name)
{
    assert(myAssetInfo.id >= 0);

    for(int i=0; i< myAssetInfo.objectCount; i++)
    {
        if(myObjects[i]->getName() == name)
            return myObjects[i];
    }

    return NULL;
}

OutputObject*
Asset::findObjectById(int id)
{
    return myObjects[id];
}

// Getters for infos
HAPI_ObjectInfo
Asset::getObjectInfo(int id) { return myObjectInfos[id]; }

void
Asset::resetSimulation()
{
    assert(myAssetInfo.id >= 0);

    HAPI_ResetSimulation(myAssetInfo.id);
}

MString
Asset::getCookMessages()
{
    Util::PythonInterpreterLock pythonInterpreterLock;

    // Trigger a cook so that the asset will become the "last cooked asset",
    // because HAPI_STATUS_COOK_RESULT only consider the "last cooked asset".
    // In most cases, this shouldn't do any actual cooking.
    HAPI_CookAsset(myAssetInfo.id, NULL);

    GET_HAPI_STATUS_COOK();

    return hapiStatus;
}

void
Asset::update()
{
    assert(myAssetInfo.id >= 0);

    // update object infos
    delete[] myObjectInfos;
    myObjectInfos = new HAPI_ObjectInfo[myAssetInfo.objectCount];
    HAPI_GetObjects(myAssetInfo.id, myObjectInfos, 0, myAssetInfo.objectCount);

    // update transform infos
    //delete[] transformInfos;
    //transformInfos = new HAPI_Transform[assetInfo.objectCount];
    //HAPI_GetObjectTransforms(assetInfo.id, 5, transformInfos, 0, assetInfo.objectCount);

    // update material infos
    //delete[] materialInfos;
    //materialInfos = new HAPI_MaterialInfo[assetInfo.materialCount];
    //HAPI_GetMaterials(assetInfo.id, materialInfos, 0, assetInfo.materialCount);
}

void
Asset::computeInstancerObjects(
        const MPlug& plug,
        MDataBlock& data,
        bool &needToSyncOutputs
        )
{
    MStatus stat;

    MPlug instancersPlug = plug.child(AssetNode::outputInstancers);

    int instancerIndex = 0;
    MArrayDataHandle instancersHandle = data.outputArrayValue(instancersPlug);
    MArrayDataBuilder instancersBuilder = instancersHandle.builder();
    MIntArray instancedObjIds;
    for(unsigned int i=0; i< myNumObjects; i++)
    {
        OutputObject* obj = myObjects[i];
        //MPlug instancerElemPlug = instancersPlug.elementByLogicalIndex(instancerIndex);

        if(obj->type() == OutputObject::OBJECT_TYPE_INSTANCER)
        {
            MDataHandle instancerElemHandle = instancersBuilder.addElement(instancerIndex);
            stat = obj->compute(instancerElemHandle, needToSyncOutputs);
            if(MS::kSuccess == stat)
            {
                instancerIndex++;

                // get all the object ids that are instanced
                MIntArray instIds = dynamic_cast< OutputInstancerObject* >(obj)->getInstancedObjIds();
                MStringArray instNames = dynamic_cast< OutputInstancerObject* >(obj)->getUniqueInstObjNames();
                for(unsigned int j = 0; j < instNames.length(); ++j)
                {
                    OutputObject* o = findObjectByName(instNames[j]);
                    if(o != NULL)
                        instancedObjIds.append(o->getId());
                }
                for(unsigned int j = 0; j < instIds.length(); ++j)
                {
                    instancedObjIds.append(instIds[j]);
                }
            }
        }
    }
    // clean up extra elements
    int instBuilderSizeCheck = instancersBuilder.elementCount();
    if(instBuilderSizeCheck > instancerIndex)
    {
        for(int i=instancerIndex; i<instBuilderSizeCheck; i++)
        {
            instancersBuilder.removeElement(i);
        }
    }
    instancersHandle.set(instancersBuilder);

    // mark instanced objects
    for(unsigned int i = 0; i < instancedObjIds.length(); ++i)
    {
        OutputObject* obj = myObjects[instancedObjIds[i]];
        obj->myIsInstanced = true;
    }

    instancersHandle.setAllClean();
    data.setClean(instancersPlug);
}

void
Asset::computeGeometryObjects(
        const MPlug& plug,
        MDataBlock& data,
        bool &needToSyncOutputs
        )
{
    MStatus stat;

    MPlug objectsPlug = plug.child(AssetNode::outputObjects);

    MArrayDataHandle objectsHandle = data.outputArrayValue(objectsPlug);
    MArrayDataBuilder objectsBuilder = objectsHandle.builder();
    if(objectsBuilder.elementCount() != myNumObjects)
    {
        needToSyncOutputs = true;
    }

    for(unsigned int ii = 0; ii < myNumObjects; ii++)
    {
        OutputObject * obj = myObjects[ii];

        MDataHandle objectHandle = objectsBuilder.addElement(ii);

        if(obj->type() == OutputObject::OBJECT_TYPE_GEOMETRY)
        {
            obj->compute(objectHandle, needToSyncOutputs);
        }
    }

    // clean up extra elements
    // in case the number of objects shrinks
    unsigned int objBuilderSizeCheck = objectsBuilder.elementCount();
    if(objBuilderSizeCheck > myNumObjects)
    {
        for(unsigned int ii = myNumObjects; ii < objBuilderSizeCheck; ii++)
        {
            stat = objectsBuilder.removeElement(ii);
            CHECK_MSTATUS(stat);
        }
    }
    objectsHandle.set(objectsBuilder);

    objectsHandle.setAllClean();

    data.setClean(objectsPlug);
}

MTime
Asset::getTime() const
{
    float hapiTimeSeconds;
    HAPI_GetTime(&hapiTimeSeconds);

    // Houdini's "frame 1" is "0 seconds", but Maya's "frame 0" is "0 seconds".
    // So we need to offset the time by 1.
    MTime mayaTime(hapiTimeSeconds, MTime::kSeconds);
    mayaTime += MTime(1,MTime::uiUnit());

    return mayaTime;
}

void
Asset::setTime(const MTime &mayaTime)
{
    // Houdini's "frame 1" is "0 seconds", but Maya's "frame 0" is "0 seconds".
    // So we need to offset the time by 1.
    MTime hapiTime = mayaTime - MTime(1, MTime::uiUnit());
    float hapiTimeSeconds = (float)hapiTime.as(MTime::kSeconds);
    HAPI_SetTime(hapiTimeSeconds);
}

void
Asset::setInputs(const MPlug& plug, MDataBlock& data)
{
    assert(myAssetInfo.id >= 0);

    MStatus status;

    MPlug inputsPlug(myNode, AssetNode::input);

    myAssetInputs->compute(data);

    for(int i=0; i< myAssetInfo.geoInputCount; i++)
    {
        MPlug inputPlug = inputsPlug.elementByLogicalIndex(i, &status);
        CHECK_MSTATUS(status);

        myAssetInputs->setInput(i, data, inputPlug);
    }
}

MStatus
Asset::compute(
        const MPlug& plug,
        MDataBlock& data,
        bool splitGeosByGroup,
        bool &needToSyncOutputs
        )
{
    assert(myAssetInfo.id >= 0);

    MStatus stat(MS::kSuccess);

    // Set the type
    MPlug typePlug(myNode, AssetNode::assetType);
    MDataHandle typeHandle = data.outputValue(typePlug);

    //The asset info struct (info) was set at the constructor
    //of this class, which is at asset load time.
    typeHandle.set(myAssetInfo.type);

    {
        Util::PythonInterpreterLock pythonInterpreterLock;

        HAPI_CookOptions cookOptions;
        HAPI_CookOptions_Init(&cookOptions);
        cookOptions.splitGeosByGroup = splitGeosByGroup;

        HAPI_CookAsset(myAssetInfo.id, &cookOptions);

        if(!Util::statusCheckLoop())
        {
            GET_HAPI_STATUS_COOK();
            DISPLAY_MSG(displayError, hapiStatus);
        }
    }

    update();

    // first pass - instancers
    // There is a reason that instancers are computed first.
    // computeInstancerObjects will mark instanced geometry objects as
    // instanced.  In computeGeometryObjects, each object will check
    // if it is instanced or not, and will compute an output or not
    // depending on whether it is instanced and whether it is visible
    computeInstancerObjects(plug, data, needToSyncOutputs);

    // second pass - geometry objects
    computeGeometryObjects(plug, data, needToSyncOutputs);

    return stat;
}

OutputObject**
Asset::getObjects()
{
    return myObjects;
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
GetAttrOperation::leaf(const HAPI_ParmInfo &parmInfo)
{
    MDataHandle dataHandle;
    MPlug plug;
    bool exists = false;

    MDataHandle &parentDataHandle = myDataHandles.back();
    MPlug &parentPlug = myPlugs.back();
    bool parentExists = myExists.back();

    if(parentExists && containsParm(parmInfo))
    {
        MString attrName = Util::getAttrNameFromParm(parmInfo);
        MObject attrObj = myNodeFn.attribute(attrName);
        if(!attrObj.isNull())
        {
            exists = true;
        }

        if(exists)
        {
            dataHandle = parentDataHandle.child(attrObj);
            plug = parentPlug.child(attrObj);

            if((parmInfo.type == HAPI_PARMTYPE_INT
                        || parmInfo.type == HAPI_PARMTYPE_BUTTON
                        || parmInfo.type == HAPI_PARMTYPE_STRING)
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
                else if(parmInfo.type == HAPI_PARMTYPE_STRING)
                {
                    int value;
                    HAPI_GetParmStringValues(
                            myNodeInfo.id,
                            false,
                            &value,
                            parmInfo.stringValuesIndex, parmInfo.size
                            );
                    MString valueString = Util::getString(value);

                    HAPI_ParmChoiceInfo * choiceInfos =
                        new HAPI_ParmChoiceInfo[parmInfo.choiceCount];
                    HAPI_GetParmChoiceLists(
                            myNodeInfo.id,
                            choiceInfos,
                            parmInfo.choiceIndex, parmInfo.choiceCount
                            );

                    for(int i = 0; i < parmInfo.choiceCount; i++)
                    {
                        if(valueString == Util::getString(choiceInfos[i].valueSH))
                        {
                            enumIndex = i;
                        }
                    }

                    delete[] choiceInfos;
                }
                else
                {
                    HAPI_GetParmIntValues(
                            myNodeInfo.id,
                            &enumIndex,
                            parmInfo.intValuesIndex, parmInfo.size
                            );
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
                    case HAPI_PARMTYPE_PATH_FILE_GEO:
                    case HAPI_PARMTYPE_PATH_FILE_IMAGE:
                    case HAPI_PARMTYPE_PATH_NODE:
                        {
                            int* values = new int[parmInfo.size];
                            HAPI_GetParmStringValues(
                                    myNodeInfo.id,
                                    false,
                                    values,
                                    parmInfo.stringValuesIndex, parmInfo.size
                                    );

                            if(parmInfo.size == 1)
                            {
                                dataHandle.setString(Util::getString(values[0]));
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
                                    elementDataHandle.setString(Util::getString(values[i]));
                                }
                            }

                            delete[] values;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
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

    std::vector<HAPI_ParmInfo> parmInfos;

    // Get value
    {
        parmInfos.resize(myNodeInfo.parmCount);
        HAPI_GetParameters(myNodeInfo.id, &parmInfos[0], 0, parmInfos.size());

        GetAttrOperation operation(
                dataBlock,
                nodeFn,
                myNodeInfo,
                attrs
                );
        Util::walkParm(parmInfos, operation);
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
    MDataHandle dataHandle;
    MPlug plug;
    bool exists = false;

    MDataHandle &parentDataHandle = myDataHandles.back();
    MPlug &parentPlug = myPlugs.back();
    bool parentExists = myExists.back();

    if(parentExists && containsParm(parmInfo))
    {
        MString attrName = Util::getAttrNameFromParm(parmInfo);
        MObject attrObj = myNodeFn.attribute(attrName);
        if(!attrObj.isNull())
        {
            exists = true;
        }

        if(exists)
        {
            dataHandle = parentDataHandle.child(attrObj);
            plug = parentPlug.child(attrObj);

            if((parmInfo.type == HAPI_PARMTYPE_INT
                        || parmInfo.type == HAPI_PARMTYPE_BUTTON
                        || parmInfo.type == HAPI_PARMTYPE_STRING)
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
                                myNodeInfo.id,
                                &value,
                                parmInfo.intValuesIndex, 1
                                );
                    }
                }
                else if(parmInfo.type == HAPI_PARMTYPE_STRING)
                {
                    HAPI_ParmChoiceInfo * choiceInfos = new HAPI_ParmChoiceInfo[parmInfo.choiceCount];
                    HAPI_GetParmChoiceLists(
                            myNodeInfo.id,
                            choiceInfos,
                            parmInfo.choiceIndex, parmInfo.choiceCount);

                    MString valueString = Util::getString(choiceInfos[enumIndex].valueSH);

                    HAPI_SetParmStringValue(
                            myNodeInfo.id,
                            valueString.asChar(),
                            parmInfo.id,
                            0
                            );

                    delete[] choiceInfos;
                }
                else
                {
                    HAPI_SetParmIntValues(
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
                    case HAPI_PARMTYPE_PATH_FILE_GEO:
                    case HAPI_PARMTYPE_PATH_FILE_IMAGE:
                        {
                            if(parmInfo.size == 1)
                            {
                                const char* val = dataHandle.asString().asChar();
                                HAPI_SetParmStringValue(
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
                                            myNodeInfo.id,
                                            val,
                                            parmInfo.id,
                                            i
                                            );
                                }
                            }
                        }
                        break;
                    case HAPI_PARMTYPE_PATH_NODE:
                        // Avoid setting path parameters. Path parameters
                        // should all be HAPI inputs. So technically, the user
                        // wouldn't know what values to set these parameters
                        // to. And the values restored from a previous session
                        // are not necessarily correct.
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
        const std::vector<MObject>* attrs
        )
{
    MStatus status;

    std::vector<HAPI_ParmInfo> parmInfos;

    // Set value
    {
        parmInfos.resize(myNodeInfo.parmCount);
        HAPI_GetParameters(
                myNodeInfo.id,
                &parmInfos[0],
                0, parmInfos.size()
                );

        SetAttrOperation operation(
                dataBlock,
                nodeFn,
                myNodeInfo,
                attrs
                );
        Util::walkParm(parmInfos, operation);
    }
}

class GetMultiparmLengthOperation : public AttrOperation
{
    public:
        GetMultiparmLengthOperation(
                MDataBlock &dataBlock,
                const MPlug &multiSizePlug,
                int &multiSize,
                const MFnDependencyNode &nodeFn,
                const HAPI_NodeInfo &nodeInfo
                );

        virtual void pushMultiparm(const HAPI_ParmInfo &parmInfo);

    protected:
        const MPlug &myMultiSizePlug;
        int &myMultiSize;
};

GetMultiparmLengthOperation::GetMultiparmLengthOperation(
                MDataBlock &dataBlock,
                const MPlug &multiSizePlug,
                int &multiSize,
                const MFnDependencyNode &nodeFn,
                const HAPI_NodeInfo &nodeInfo
        ) :
    AttrOperation(
            dataBlock,
            AttrOperation::Get,
            nodeFn,
            nodeInfo,
            NULL
            ),
            myMultiSizePlug(multiSizePlug),
            myMultiSize(multiSize)
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

    bool isMulti = myIsMulti.back();
    //MDataHandle &multiSizeDataHandle = myMultiSizeDataHandles.back();
    MPlug &multiSizePlug = myMultiSizePlugs.back();
    //bool hasMultiAttr = myHasMultiAttr.back();
    //MArrayDataHandle &multiDataHandle = myMultiDataHandles.back();
    //MPlug &multiPlug = myMultiPlugs.back();
    //int &multiLogicalIndex = myMultiLogicalIndices.back();

    if(isMulti && myMultiSizePlug == multiSizePlug)
    {
        myMultiSize = parmInfo.instanceCount;
    }
}

void
Asset::getMultiparmLength(
        MDataBlock &dataBlock,
        const MPlug &multiSizePlug,
        int &multiSize,
        const MFnDependencyNode &nodeFn
        )
{
    MStatus status;

    std::vector<HAPI_ParmInfo> parmInfos;

    // Get multiparm length
    {
        parmInfos.resize(myNodeInfo.parmCount);
        HAPI_GetParameters(
                myNodeInfo.id,
                &parmInfos[0],
                0, parmInfos.size()
                );

        GetMultiparmLengthOperation operation(
                dataBlock,
                multiSizePlug,
                multiSize,
                nodeFn,
                myNodeInfo
                );
        Util::walkParm(parmInfos, operation);
    }

    getParmValues(dataBlock, nodeFn, NULL);
}

class SetMultiparmLengthOperation : public AttrOperation
{
    public:
        SetMultiparmLengthOperation(
                MDataBlock &dataBlock,
                const MPlug &multiSizePlug,
                int multiSize,
                const MFnDependencyNode &nodeFn,
                const HAPI_NodeInfo &nodeInfo
                );

        virtual void pushMultiparm(const HAPI_ParmInfo &parmInfo);

    protected:
        const MPlug &myMultiSizePlug;
        int myMultiSize;
};

SetMultiparmLengthOperation::SetMultiparmLengthOperation(
        MDataBlock &dataBlock,
        const MPlug &multiSizePlug,
        int multiSize,
        const MFnDependencyNode &nodeFn,
        const HAPI_NodeInfo &nodeInfo
        ) :
    AttrOperation(
            dataBlock,
            AttrOperation::Set,
            nodeFn,
            nodeInfo,
            NULL
            ),
    myMultiSizePlug(multiSizePlug),
    myMultiSize(multiSize)
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

    bool isMulti = myIsMulti.back();
    //MDataHandle &multiSizeDataHandle = myMultiSizeDataHandles.back();
    MPlug &multiSizePlug = myMultiSizePlugs.back();
    bool hasMultiAttr = myHasMultiAttr.back();
    MArrayDataHandle &multiDataHandle = myMultiDataHandles.back();
    //MPlug &multiPlug = myMultiPlugs.back();
    //int &multiLogicalIndex = myMultiLogicalIndices.back();

    if(isMulti && myMultiSizePlug == multiSizePlug)
    {
        {
            // If the multiparm has less instances than the multiDataHandle, then
            // we need to add to the multiparm.
            for(int i = parmInfo.instanceCount; i < myMultiSize; ++i)
            {
                HAPI_InsertMultiparmInstance(
                        myNodeInfo.id,
                        parmInfo.id,
                        i + parmInfo.instanceStartOffset
                        );
            }

            // If the multiparm has more instances than the multiDataHandle, then
            // we need to remove from the multiparm.
            for(int i = parmInfo.instanceCount; i-- > myMultiSize;)
            {
                HAPI_RemoveMultiparmInstance(
                        myNodeInfo.id,
                        parmInfo.id,
                        i + parmInfo.instanceStartOffset
                        );
            }
        }

        if(hasMultiAttr)
        {
            MArrayDataBuilder builder = multiDataHandle.builder(&status);
            CHECK_MSTATUS(status);

            const int builderCount = builder.elementCount();

            // If the builder has less elements than the multiparm, then we need to
            // add to the builder.
            for(int i = builderCount; i < myMultiSize; ++i)
            {
                builder.addElement(i);
            }

            // If the builder has more elements than the multiparm, then we need to
            // remove from the builder.
            for(int i = builderCount; i-- > myMultiSize;)
            {
                builder.removeElement(i);
            }

            multiDataHandle.set(builder);
        }
    }
}

void
Asset::setMultiparmLength(
        MDataBlock &dataBlock,
        const MPlug &multiSizePlug,
        int multiSize,
        const MFnDependencyNode &nodeFn
        )
{
    assert(myAssetInfo.id >= 0);

    MStatus status;

    std::vector<HAPI_ParmInfo> parmInfos;

    // Set multiparm length
    {
        parmInfos.resize(myNodeInfo.parmCount);
        HAPI_GetParameters(myNodeInfo.id, &parmInfos[0], 0, parmInfos.size());

        SetMultiparmLengthOperation operation(
                dataBlock,
                multiSizePlug,
                multiSize,
                nodeFn,
                myNodeInfo
                );
        Util::walkParm(parmInfos, operation);

        // multiparm length could change, so we need to get the new parmCount
        HAPI_GetNodeInfo(myAssetInfo.nodeId, &myNodeInfo);
    }

    getParmValues(dataBlock, nodeFn, NULL);
}
