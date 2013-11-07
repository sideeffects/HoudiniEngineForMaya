#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MDataHandle.h>
#include <maya/MTime.h>
#include <maya/MGlobal.h>

#include "Asset.h"
#include "AssetInput.h"
#include "AssetNode.h"
#include "GeometryObject.h"
#include "InstancerObject.h"
#include "util.h"

class AttrOperation : public Util::WalkParmOperation
{
    public:
        AttrOperation(
                const MDataHandle &dataHandle,
                const MFnDependencyNode &nodeFn,
                const HAPI_NodeInfo &nodeInfo,
                const std::vector<MObject>* attrs
                );
        ~AttrOperation();

        virtual void pushFolder(const HAPI_ParmInfo &parmInfo);
        virtual void popFolder();

        bool containsParm(const HAPI_ParmInfo &parm) const;

    protected:
        std::vector<MDataHandle> myDataHandles;
        std::vector<bool> myExists;

        const MFnDependencyNode &myNodeFn;
        const HAPI_NodeInfo &myNodeInfo;
        const std::vector<MObject>* myAttrs;
};

AttrOperation::AttrOperation(
        const MDataHandle &dataHandle,
        const MFnDependencyNode &nodeFn,
        const HAPI_NodeInfo &nodeInfo,
        const std::vector<MObject>* attrs
        ) :
    myNodeFn(nodeFn),
    myNodeInfo(nodeInfo),
    myAttrs(attrs)
{
    myDataHandles.push_back(dataHandle);
    myExists.push_back(true);
}

AttrOperation::~AttrOperation()
{
    myDataHandles.pop_back();
    myExists.pop_back();
}

void
AttrOperation::pushFolder(const HAPI_ParmInfo &parmInfo)
{
    MDataHandle dataHandle;
    bool exists = false;

    MDataHandle &parentDataHandle = myDataHandles.back();
    bool parentExists = myExists.back();

    if(parentExists)
    {
        MString folderAttrName = Util::getAttrNameFromParm(parmInfo);
        MObject folderAttrObj = myNodeFn.attribute(folderAttrName);

        if(!folderAttrObj.isNull())
        {
            exists = true;
            dataHandle = parentDataHandle.child(folderAttrObj);
        }
    }

    myDataHandles.push_back(dataHandle);
    myExists.push_back(exists);
}

void
AttrOperation::popFolder()
{
    MDataHandle &dataHandle = myDataHandles.back();
    bool exists = myExists.back();

    myDataHandles.pop_back();
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
        if(( HAPI_ParmInfo_IsInt( &parm ) || HAPI_ParmInfo_IsFloat( &parm ) || HAPI_ParmInfo_IsString( &parm ) )
                && parm.size > 1
                && plug.isChild() && parmPlug == plug.parent())
        {
            return true;
        }
    }

    return false;
}

Asset::Asset(MString otlFilePath, MObject node) :
    myNode(node)
{
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;

    myObjectInfos = NULL;    

    // load the otl
    const char* filename = otlFilePath.asChar();

    int assetId;
    hstat = HAPI_LoadOTLFile(filename, 3, 20, &assetId);

    Util::statusCheckLoop();
    Util::checkHAPIStatus(hstat);
    hstat = HAPI_GetAssetInfo(assetId, &myAssetInfo);
    Util::checkHAPIStatus(hstat);
    hstat = HAPI_GetNodeInfo( myAssetInfo.nodeId, & myNodeInfo);
    Util::checkHAPIStatus(hstat);

    myAssetInputs = new AssetInputs( myAssetInfo.id);

    init();

}


void
Asset::init()
{
    myAssetInputs->setNumInputs( myAssetInfo.maxGeoInputCount);

    // get the infos
    update();   

    // objects
    int objCount = myAssetInfo.objectCount;
    myObjects = new Object*[objCount];
    myNumVisibleObjects = 0;
    myNumObjects = objCount;

    MString title = "Houdini";
    MString status = "Creating Objects...";
    Util::showProgressWindow( title, status, 0 );

    for (int i=0; i<objCount; i++)
    {
	Util::updateProgressWindow( status, (int)( (float) i *100.0f / (float) objCount) );
        myObjects[i] = Object::createObject( myAssetInfo.id, i, this);
        myObjects[i]->init();
    }

    Util::hideProgressWindow();
}


Asset::~Asset()
{
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;

    for (int i=0; i< myNumObjects; i++)
        delete myObjects[i];
    delete[] myObjects;
    delete[] myObjectInfos;    
    delete myAssetInputs;

    hstat = HAPI_UnloadOTLFile(myAssetInfo.id);
    Util::checkHAPIStatus(hstat);
}


Object*
Asset::findObjectByName(MString name)
{
    for (int i=0; i< myAssetInfo.objectCount; i++)
    {
        if ( myObjects[i]->getName() == name )
            return myObjects[i];
    }

    return NULL;
}


Object*
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
    HAPI_ResetSimulation(myAssetInfo.id);
}

void
Asset::update()
{
    // update object infos
    delete[] myObjectInfos;
    myObjectInfos = new HAPI_ObjectInfo[ myAssetInfo.objectCount];
    HAPI_GetObjects( myAssetInfo.id, myObjectInfos, 0, myAssetInfo.objectCount);

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
Asset::computeAssetInputs(const MPlug& plug, MDataBlock& data)
{
    MStatus status;

    MPlug inputsPlug(myNode, AssetNode::input);
    MArrayDataHandle inputArrayHandle = data.inputArrayValue(AssetNode::input);

    for (int i=0; i< myAssetInfo.maxGeoInputCount; i++)
    {
	status = inputArrayHandle.jumpToElement(i);
	if(status)
	{
	    MDataHandle inputHandle = inputArrayHandle.inputValue();
	    myAssetInputs->setInput(i, inputHandle);
	}
	else
	{
	    myAssetInputs->clearInput(i);
	}
    }
}


void
Asset::computeInstancerObjects(const MPlug& plug, MDataBlock& data)
{
    MStatus stat;

    MPlug instancersPlug = plug.child(AssetNode::outputInstancers);

    int instancerIndex = 0;
    MArrayDataHandle instancersHandle = data.outputArrayValue(instancersPlug);
    MArrayDataBuilder instancersBuilder = instancersHandle.builder();
    MIntArray instancedObjIds;
    for (int i=0; i< myNumObjects; i++)
    {
        Object* obj = myObjects[i];
        //MPlug instancerElemPlug = instancersPlug.elementByLogicalIndex( instancerIndex );

        if ( obj->type() == Object::OBJECT_TYPE_INSTANCER )
        {
            MDataHandle instancerElemHandle = instancersBuilder.addElement( instancerIndex );
            stat = obj->compute( instancerElemHandle );
            if ( MS::kSuccess == stat )
            {
                instancerIndex++;

                // get all the object ids that are instanced
                MIntArray instIds = dynamic_cast< InstancerObject* >( obj )->getInstancedObjIds();
                MStringArray instNames = dynamic_cast< InstancerObject* >( obj )->getUniqueInstObjNames();
                for ( unsigned int j = 0; j < instNames.length(); ++j )
                {
                    Object* o = findObjectByName( instNames[ j ] );
                    if ( o != NULL )
                        instancedObjIds.append( o->getId() );
                }
                for ( unsigned int j = 0; j < instIds.length(); ++j )
                {
                    instancedObjIds.append( instIds[ j ] );
                }
            }
        }
    }
    // clean up extra elements
    int instBuilderSizeCheck = instancersBuilder.elementCount();
    if (instBuilderSizeCheck > instancerIndex)
    {
        for (int i=instancerIndex; i<instBuilderSizeCheck; i++)
        {
            instancersBuilder.removeElement(i);
        }
    }
    instancersHandle.set(instancersBuilder);

    // mark instanced objects
    for ( unsigned int i = 0; i < instancedObjIds.length(); ++i )
    {
        Object* obj = myObjects[ instancedObjIds[ i ] ];
        obj->myIsInstanced = true;
    }

    instancersHandle.setAllClean();
    data.setClean( instancersPlug );
}


void
Asset::computeGeometryObjects(const MPlug& plug, MDataBlock& data)
{
    MStatus stat;

    MPlug objectsPlug = plug.child(AssetNode::outputObjects);    

    MArrayDataHandle objectsHandle = data.outputArrayValue(objectsPlug);
    MArrayDataBuilder objectsBuilder = objectsHandle.builder();
    for (int ii = 0; ii < myNumObjects; ii++)
    {
        Object * obj = myObjects[ ii ];        

	MDataHandle objectHandle = objectsBuilder.addElement( ii );

        if (obj->type() == Object::OBJECT_TYPE_GEOMETRY)
        {
	    obj->compute( objectHandle );                        

            MDataHandle visibilityHandle = objectHandle.child( AssetNode::outputVisibility );
	    visibilityHandle.setBool( obj->isVisible() );
	    visibilityHandle.setClean();

	    MDataHandle isInstancedHandle = objectHandle.child( AssetNode::outputIsInstanced );
	    isInstancedHandle.setBool( obj->isInstanced() );
	    isInstancedHandle.setClean();

            MDataHandle objectNameHandle = objectHandle.child( AssetNode::outputObjectName );
            objectNameHandle.setString( obj->getName() );
	    objectNameHandle.setClean();
        }
    }

    // clean up extra elements
    // in case the number of objects shrinks
    int objBuilderSizeCheck = objectsBuilder.elementCount();
    if (objBuilderSizeCheck > myNumObjects)
    {
        for (int ii = myNumObjects; ii < objBuilderSizeCheck; ii++)
	{
	    stat = objectsBuilder.removeElement( ii );
	    CHECK_MSTATUS(stat);
	}
    }
    objectsHandle.set(objectsBuilder);

    objectsHandle.setAllClean();

    data.setClean(objectsPlug);
}


MStatus
Asset::compute(const MPlug& plug, MDataBlock& data)
{
    MStatus stat(MS::kSuccess);

    // Set the type
    MPlug typePlug( myNode, AssetNode::assetType);
    MDataHandle typeHandle = data.outputValue(typePlug);

    //The asset info struct (info) was set at the constructor
    //of this class, which is at asset load time.
    typeHandle.set( myAssetInfo.type);

    // Set the time
    MPlug timePlug( myNode, AssetNode::inTime);
    MDataHandle timeHandle = data.inputValue(timePlug);
    MTime currentTime = timeHandle.asTime();
    // Houdini's "frame 1" is "0 seconds", but Maya's "frame 0" is "0 seconds".
    // So we need to offset the time by 1. We cannot use -= because we can't
    // modify the MTime returned by asTime. So we need to construct a new
    // MTime.
    currentTime = currentTime - MTime(1, MTime::uiUnit());
    float time = (float)currentTime.as(MTime::kSeconds);
    HAPI_SetTime(time);

    //this figures out the Houdini asset inputs (Geo, Transform)
    //for inter-asset stuff
    computeAssetInputs(plug, data);

    HAPI_CookAsset( myAssetInfo.id);


    Util::statusCheckLoop();

    update();

    // first pass - instancers
    // There is a reason that instancers are computed first.  
    // computeInstancerObjects will mark instanced geometry objects as
    // instanced.  In computeGeometryObjects, each object will check 
    // if it is instanced or not, and will compute an output or not 
    // depending on whether it is instanced and whether it is visible
    computeInstancerObjects(plug, data);

    // second pass - geometry objects
    computeGeometryObjects(plug, data);

    return stat;
}

Object**
Asset::getObjects()
{
    return myObjects;
}

class GetAttrOperation : public AttrOperation
{
    public:
        GetAttrOperation(
                const MDataHandle &dataHandle,
                const MFnDependencyNode &nodeFn,
                const HAPI_NodeInfo &nodeInfo,
                const std::vector<MObject>* attrs
                );

        virtual void leaf(const HAPI_ParmInfo &parmInfo);
};

GetAttrOperation::GetAttrOperation(
        const MDataHandle &dataHandle,
        const MFnDependencyNode &nodeFn,
        const HAPI_NodeInfo &nodeInfo,
        const std::vector<MObject>* attrs
        ) :
    AttrOperation(
            dataHandle,
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
    bool exists = false;

    MDataHandle &parentDataHandle = myDataHandles.back();
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

            if((parmInfo.type == HAPI_PARMTYPE_INT || parmInfo.type == HAPI_PARMTYPE_STRING)
                    && parmInfo.choiceCount > 0)
            {
                int enumIndex = 0;

                if(parmInfo.type == HAPI_PARMTYPE_STRING)
                {
                    int value;
                    HAPI_GetParmStringValues(myNodeInfo.id, &value, parmInfo.stringValuesIndex, parmInfo.size);
                    MString valueString = Util::getString(value);

                    HAPI_ParmChoiceInfo * choiceInfos = new HAPI_ParmChoiceInfo[parmInfo.choiceCount];
                    HAPI_GetParmChoiceLists(myNodeInfo.id, choiceInfos, parmInfo.choiceIndex, parmInfo.choiceCount);

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
                    HAPI_GetParmIntValues(myNodeInfo.id, &enumIndex, parmInfo.intValuesIndex, parmInfo.size);
                }

                dataHandle.setShort(static_cast<short>(enumIndex));
            }
            else
            {
                switch(parmInfo.type)
                {
                    case HAPI_PARMTYPE_FLOAT:
                    case HAPI_PARMTYPE_COLOUR:
                        {
                            float* values = new float[parmInfo.size];
                            HAPI_GetParmFloatValues(myNodeInfo.id, values, parmInfo.floatValuesIndex, parmInfo.size);

                            if(parmInfo.size == 1)
                            {
                                dataHandle.setFloat(values[0]);
                            }
                            else
                            {
                                MFnCompoundAttribute attrFn(attrObj);
                                for(int i = 0;
                                        i < attrFn.numChildren()
                                        && i < parmInfo.size;
                                        i++)
                                {
                                    MDataHandle elementDataHandle = dataHandle.child(attrFn.child(i));
                                    elementDataHandle.setFloat(values[i]);
                                }
                            }

                            delete[] values;
                        }
                        break;
                    case HAPI_PARMTYPE_INT:
                    case HAPI_PARMTYPE_TOGGLE:
                        {
                            int* values = new int[parmInfo.size];
                            HAPI_GetParmIntValues(myNodeInfo.id, values, parmInfo.intValuesIndex, parmInfo.size);

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
                                        i < attrFn.numChildren()
                                        && i < parmInfo.size;
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
                    case HAPI_PARMTYPE_FILE:
                        {
                            int* values = new int[parmInfo.size];
                            HAPI_GetParmStringValues(myNodeInfo.id, values, parmInfo.stringValuesIndex, parmInfo.size);

                            if(parmInfo.size == 1)
                            {
                                dataHandle.setString(Util::getString(values[0]));
                            }
                            else
                            {
                                MFnCompoundAttribute attrFn(attrObj);
                                for(int i = 0;
                                        i < attrFn.numChildren()
                                        && i < parmInfo.size;
                                        i++)
                                {
                                    MDataHandle elementDataHandle = dataHandle.child(attrFn.child(i));
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
        const MDataHandle &parentDataHandle,
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
		parentDataHandle,
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
                const MDataHandle &dataHandle,
                const MFnDependencyNode &nodeFn,
                const HAPI_NodeInfo &nodeInfo,
                const std::vector<MObject>* attrs
                );

        virtual void leaf(const HAPI_ParmInfo &parmInfo);
};

SetAttrOperation::SetAttrOperation(
        const MDataHandle &dataHandle,
        const MFnDependencyNode &nodeFn,
        const HAPI_NodeInfo &nodeInfo,
        const std::vector<MObject>* attrs
        ) :
    AttrOperation(
            dataHandle,
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
    bool exists = false;

    MDataHandle &parentDataHandle = myDataHandles.back();
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

            if((parmInfo.type == HAPI_PARMTYPE_INT || parmInfo.type == HAPI_PARMTYPE_STRING)
                    && parmInfo.choiceCount > 0)
            {
                int enumIndex = static_cast<int>(dataHandle.asShort());

                if(parmInfo.type == HAPI_PARMTYPE_STRING)
                {
                    HAPI_ParmChoiceInfo * choiceInfos = new HAPI_ParmChoiceInfo[parmInfo.choiceCount];
                    HAPI_GetParmChoiceLists(myNodeInfo.id, choiceInfos, parmInfo.choiceIndex, parmInfo.choiceCount);

                    MString valueString = Util::getString(choiceInfos[enumIndex].valueSH);

                    HAPI_SetParmStringValue( myNodeInfo.id, valueString.asChar(), parmInfo.id, 0);

                    delete[] choiceInfos;
                }
                else
                {
                    HAPI_SetParmIntValues(myNodeInfo.id, &enumIndex, parmInfo.intValuesIndex, 1);
                }
            }
            else
            {
                switch(parmInfo.type)
                {
                    case HAPI_PARMTYPE_FLOAT:
                    case HAPI_PARMTYPE_COLOUR:
                        {
                            float * values = new float[parmInfo.size];
                            if (parmInfo.size == 1)
                            {
                                values[0] = dataHandle.asFloat();
                            } else
                            {
                                MFnCompoundAttribute attrFn(attrObj);
                                for (int i=0; i<parmInfo.size; i++)
                                {
                                    MDataHandle elementHandle = dataHandle.child(attrFn.child(i));
                                    values[i] = elementHandle.asFloat();
                                }
                            }
                            HAPI_SetParmFloatValues( myNodeInfo.id, values, parmInfo.floatValuesIndex, parmInfo.size);

                            delete[] values;
                        }
                        break;
                    case HAPI_PARMTYPE_INT:
                    case HAPI_PARMTYPE_TOGGLE:
                        {
                            int * values = new int[parmInfo.size];
                            if (parmInfo.size == 1)
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
                                for (int i=0; i<parmInfo.size; i++)
                                {
                                    MDataHandle elementHandle = dataHandle.child(attrFn.child(i));
                                    if(parmInfo.type == HAPI_PARMTYPE_TOGGLE)
                                    {
                                        values[i] = dataHandle.asBool();
                                    }
                                    else
                                    {
                                        values[i] = dataHandle.asInt();
                                    }
                                }
                            }
                            HAPI_SetParmIntValues( myNodeInfo.id, values, parmInfo.intValuesIndex, parmInfo.size );

                            delete[] values;
                        }
                        break;
                    case HAPI_PARMTYPE_STRING:
                    case HAPI_PARMTYPE_FILE:
                        {
                            if (parmInfo.size == 1)
                            {
                                const char* val = dataHandle.asString().asChar();
                                HAPI_SetParmStringValue( myNodeInfo.id, val, parmInfo.id, 0);
                            } else
                            {
                                MFnCompoundAttribute attrFn(attrObj);
                                for (int i=0; i<parmInfo.size; i++)
                                {
                                    MDataHandle elementHandle = dataHandle.child(attrFn.child(i));
                                    const char* val = elementHandle.asString().asChar();
                                    HAPI_SetParmStringValue( myNodeInfo.id, val, parmInfo.id, i);
                                }
                            }
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
Asset::setParmValues(
        const MDataHandle &parentDataHandle,
        const MFnDependencyNode &nodeFn,
        const std::vector<MObject>* attrs
        )
{
    MStatus status;

    std::vector<HAPI_ParmInfo> parmInfos;

    // Set value
    {
	parmInfos.resize(myNodeInfo.parmCount);
	HAPI_GetParameters(myNodeInfo.id, &parmInfos[0], 0, parmInfos.size());

	SetAttrOperation operation(
		parentDataHandle,
		nodeFn,
		myNodeInfo,
		attrs
		);
	Util::walkParm(parmInfos, operation);
    }
}
