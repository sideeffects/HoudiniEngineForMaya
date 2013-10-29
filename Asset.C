#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MTime.h>
#include <maya/MGlobal.h>

#include "Asset.h"
#include "AssetInput.h"
#include "AssetNode.h"
#include "GeometryObject.h"
#include "InstancerObject.h"
#include "util.h"

Asset::Asset(MString otlFilePath, MObject node)
    :myNode(node)
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
    for (int i=0; i< myNumObjects; i++)
    {
        Object * obj = myObjects[i];        

	MDataHandle objectHandle = objectsBuilder.addElement(i);

        //TODO: FIXME: This needs to be refactored into a Geo class, and taking
        // into account the actual number of geos, instead of assuming only 1
        // as we are doing now.
        MDataHandle geosHandle = objectHandle.child( AssetNode::outputGeos );
        MArrayDataHandle geoArrayHandle( geosHandle );
        MArrayDataBuilder geosBuilder = geoArrayHandle.builder();
        MDataHandle geoHandle = geosBuilder.addElement( 0 );

	MDataHandle partsPlugTemp = geoHandle.child(AssetNode::outputParts);
	MArrayDataHandle partsHandle(partsPlugTemp);
	MArrayDataBuilder partsBuilder = partsHandle.builder();

        if (obj->type() == Object::OBJECT_TYPE_GEOMETRY)
        {
	    obj->compute( objectHandle );

	    GeometryObject * geoObj = dynamic_cast<GeometryObject *>(obj);
            stat = geoObj->computeParts(objectHandle, &partsBuilder);
	    partsHandle.set(partsBuilder);

	    MDataHandle visibilityHandle = objectHandle.child( AssetNode::outputVisibility );
	    visibilityHandle.setBool( obj->isVisible() );
	    visibilityHandle.setClean();

	    MDataHandle isInstancedHandle = objectHandle.child( AssetNode::outputIsInstanced );
	    isInstancedHandle.setBool( obj->isInstanced() );
	    isInstancedHandle.setClean();

        }

        geoArrayHandle.set( geosBuilder );
    }

    // clean up extra elements
    // in case the number of objects shrinks
    int objBuilderSizeCheck = objectsBuilder.elementCount();
    if (objBuilderSizeCheck > myNumObjects)
    {
        for (int i=myNumObjects; i<objBuilderSizeCheck; i++)
	{
	    stat = objectsBuilder.removeElement(i);
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


// Parms ---------------------------------------------------
MIntArray
Asset::getParmIntValues(HAPI_ParmInfo& parm)
{
    int index = parm.intValuesIndex;
    int size = parm.size;
    int * values = new int[size];
    HAPI_GetParmIntValues( myNodeInfo.id, values, index, size);

    MIntArray ret(values, size);

    delete[] values;
    return ret;
}


MFloatArray
Asset::getParmFloatValues(HAPI_ParmInfo& parm)
{
    int index = parm.floatValuesIndex;
    int size = parm.size;
    float * values = new float[size];
    HAPI_GetParmFloatValues( myNodeInfo.id, values, index, size);

    MFloatArray ret(values, size);

    delete[] values;
    return ret;
}


MStringArray
Asset::getParmStringValues(HAPI_ParmInfo& parm)
{
    int index = parm.stringValuesIndex;
    int size = parm.size;
    int * handles = new int[size];
    HAPI_GetParmStringValues( myNodeInfo.id, handles, index, size);

    MStringArray ret;
    for (int i=0; i<size; i++)
    {
        MString str = Util::getString(handles[i]);
        ret.append(str);
    }

    delete[] handles;
    return ret;
}

