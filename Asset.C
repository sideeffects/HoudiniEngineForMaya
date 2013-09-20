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

    // input geos
    if ( myAssetInfo.maxGeoInputCount > 0)
    {
        MFnCompoundAttribute cAttr;

        int inputCount = myAssetInfo.maxGeoInputCount;

        myMayaInputs= cAttr.create("inputs", "ins");

        for (int i=0; i<inputCount; i++)
        {
            MString attrName = MString("input") + (i+1);

	    MObject inputAttrObj = AssetInputs::createInputAttribute(attrName);

            cAttr.addChild(inputAttrObj);
        }
        addAttrTo(myMayaInputs, NULL);
    }

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

    // build parms
    buildParms();
}


Asset::~Asset()
{
    for (int i=0; i< myNumObjects; i++)
        delete myObjects[i];
    delete[] myObjects;
    delete[] myObjectInfos;    
    delete myAssetInputs;
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
    // Geo inputs
    for (int i=0; i< myAssetInfo.maxGeoInputCount; i++)
    {
        MPlug inputPlug(myNode, myMayaInputs);
        MPlug elemInputPlug = inputPlug.child(i);

        if (!elemInputPlug.isConnected())
        {
            HAPI_DisconnectAssetGeometry( myAssetInfo.id, i);
            return;
        }

        MDataHandle elemInputHandle = data.inputValue(elemInputPlug);
	myAssetInputs->setInput(i, elemInputHandle);
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

    int objectIndex = 0;
    MArrayDataHandle objectsHandle = data.outputArrayValue(objectsPlug);
    MArrayDataBuilder objectsBuilder = objectsHandle.builder();
    for (int i=0; i< myNumObjects; i++)
    {


        Object * obj = myObjects[i];        

        if (obj->type() == Object::OBJECT_TYPE_GEOMETRY)
        {
	    GeometryObject * geoObj = dynamic_cast<GeometryObject *>(obj);
            stat = geoObj->computeParts(&objectsBuilder, &objectIndex);
            
        }
    }

    // clean up extra elements
    // in case the number of objects shrinks
    int objBuilderSizeCheck = objectsBuilder.elementCount();
    if (objBuilderSizeCheck > objectIndex)
    {
        for (int i=objectIndex; i<objBuilderSizeCheck; i++)
        {
            try
            {
                stat = objectsBuilder.removeElement(i);
                Util::checkMayaStatus(stat);
            } catch (MayaError& e)
            {
                cerr << e.what() << endl;
            }
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


MObjectArray
Asset::getParmAttributes()
{
    return myParmAttributes;
}


Object**
Asset::getObjects()
{
    return myObjects;
}


// Parms ---------------------------------------------------
void
Asset::addAttrTo(MObject& child, MObject* parent)
{
    if (NULL == parent)
    {
        myParmAttributes.append(child);
        return;
    }

    MFnCompoundAttribute cAttr(*parent);
    cAttr.addChild(child);
}


MObject
Asset::createAttr(HAPI_ParmInfo& parm)
{
    MFnNumericAttribute nAttr;
    MFnCompoundAttribute cAttr;
    MFnGenericAttribute gAttr;
    MFnTypedAttribute tAttr;

    MString label = Util::getString(parm.labelSH);
    MString attrName = Util::getAttrNameFromParm(parm);

    MObject result;

    // Other types
    switch(parm.type)
    {
        case HAPI_PARMTYPE_FOLDERLIST:
        case HAPI_PARMTYPE_FOLDER:
            result = cAttr.create(attrName, attrName);
            cAttr.setStorable(true);
            cAttr.setNiceNameOverride(label);
            break;
        case HAPI_PARMTYPE_SEPARATOR:
            result = gAttr.create(attrName, attrName);
            gAttr.setHidden(true);
            gAttr.setStorable(false);
            gAttr.setReadable(false);
            gAttr.setWritable(false);
            gAttr.setConnectable(false);
            gAttr.setNiceNameOverride(label);
            break;
        case HAPI_PARMTYPE_INT:
        case HAPI_PARMTYPE_FLOAT:
        case HAPI_PARMTYPE_COLOUR:
        case HAPI_PARMTYPE_TOGGLE:
            result = createNumericAttr(parm, attrName, attrName, label);
            break;
        case HAPI_PARMTYPE_STRING:
        case HAPI_PARMTYPE_FILE:
            result = createStringAttr(parm, attrName, attrName, label);
            break;
        default:
            result = nAttr.create(attrName, attrName, MFnNumericData::kFloat, 1.0);
            nAttr.setStorable(true);
            nAttr.setNiceNameOverride(label);
            break;
    }

    return result;
}


MObject
Asset::createStringAttr(HAPI_ParmInfo& parm, MString& longName, MString& shortName, MString& niceName)
{
    MFnTypedAttribute tAttr;
    MFnCompoundAttribute cAttr;

    int size = parm.size;

    MObject result;

    if (size > 1)
    {
        result = cAttr.create(longName, shortName);
        cAttr.setStorable(true);
        cAttr.setNiceNameOverride(niceName);
        for (int i=0; i<size; i++)
        {
            MString ln = longName + "_" + i;
            MString sn = shortName + "_" + i;
            MString nn = niceName + " " + i;
            MObject child = tAttr.create(ln, sn, MFnData::kString);
            tAttr.setNiceNameOverride(nn);
            tAttr.setStorable(true);
            if (parm.type == HAPI_PARMTYPE_FILE)
                tAttr.setUsedAsFilename(true);
            cAttr.addChild(child);
        }
        return result;
    }

    result = tAttr.create(longName, shortName, MFnData::kString);
    tAttr.setStorable(true);
    tAttr.setNiceNameOverride(niceName);
    if (parm.type == HAPI_PARMTYPE_FILE)
        tAttr.setUsedAsFilename(true);

    return result;
}


MObject
Asset::createNumericAttr(HAPI_ParmInfo& parm, MString& longName, MString& shortName, MString& niceName)
{
    MFnNumericAttribute nAttr;
    MFnCompoundAttribute cAttr;
    MFnEnumAttribute eAttr;

    MObject result;
    int size = parm.size;
    int choiceCount = parm.choiceCount;

    // Choice list
    if (choiceCount > 0)
    {
        result = eAttr.create(longName, shortName);
        eAttr.setStorable(true);
        eAttr.setNiceNameOverride(niceName);

        HAPI_ParmChoiceInfo * choiceInfos = new HAPI_ParmChoiceInfo[choiceCount];
        HAPI_GetParmChoiceLists( myNodeInfo.id, choiceInfos, parm.choiceIndex, choiceCount);
        for (int i=0; i<choiceCount; i++)
        {
            MString field = Util::getString(choiceInfos[i].labelSH);
            eAttr.addField(field, static_cast<short>(i));
        }

	delete[] choiceInfos;
        return result;
    }



    MFnNumericData::Type type;
    if (parm.type == HAPI_PARMTYPE_TOGGLE)
    {
        type = MFnNumericData::kBoolean;
    }
    else if (parm.type == HAPI_PARMTYPE_INT)
    {
        switch (size)
        {
            case 1: type = MFnNumericData::kInt; break;
            case 2: type = MFnNumericData::k2Int; break;
            case 3: type = MFnNumericData::k3Int; break;
            default: type = MFnNumericData::kInt; break;
        }
    } else
    {
        switch (size)
        {
            case 1: type = MFnNumericData::kFloat; break;
            case 2: type = MFnNumericData::k2Float; break;
            case 3: type = MFnNumericData::k3Float; break;
            default: type = MFnNumericData::kFloat; break;
        }
    }

    if (size > 3) {
        result = cAttr.create(longName, shortName);
        cAttr.setNiceNameOverride(niceName);
        for (int i=0; i<size; i++)
        {
            MString ln = longName + "_" + i;
            MString sn = shortName + "_" + i;
            MString nn = niceName + " " + i;
            MObject child = nAttr.create(ln, sn, type);
            nAttr.setNiceNameOverride(nn);
            cAttr.addChild(child);
        }
        return result;
    }

    if (parm.type == HAPI_PARMTYPE_COLOUR)
        result = nAttr.createColor(longName, shortName);
    else
        result = nAttr.create(longName, shortName, type);
    nAttr.setStorable(true);
    nAttr.setNiceNameOverride(niceName);

    // TODO: support min/max for all sizes
    if (parm.hasMin)
        nAttr.setMin(parm.min);
    if (parm.hasMax)
        nAttr.setMax(parm.max);
    if (parm.hasUIMin)
        nAttr.setSoftMin(parm.UIMin);
    if (parm.hasUIMax)
        nAttr.setSoftMax(parm.UIMax);

    return result;
}


void
Asset::buildParms()
{

    // PARMS
    int parmCount = myNodeInfo.parmCount;
    if (parmCount <= 0)
        return;
    HAPI_ParmInfo * parmInfos = new HAPI_ParmInfo[parmCount];
    HAPI_GetParameters( myNodeInfo.id, parmInfos, 0, parmCount);

    int index = 0;
    while (index < parmCount)
    {
        int consumed = buildAttrTree(parmInfos, NULL, index, index+1);
        index += consumed;
    }

    delete[] parmInfos;

}


int
Asset::buildAttrTree(HAPI_ParmInfo* parmInfos, MObject* parent, int current, int start)
{
    HAPI_ParmInfo parm = parmInfos[current];

    if (parm.type == HAPI_PARMTYPE_FOLDERLIST)
    {
        int offset = parm.size;
        for (int i = start; i < start+parm.size; i++)
        {
            int count = buildAttrTree(parmInfos, parent, i, start + offset);
            offset += count;
        }
        return offset + 1;
    }

    if (parm.type == HAPI_PARMTYPE_FOLDER)
    {
        MObject result = createAttr(parm);
        int offset = 0;
        for (int i = start; i < start+parm.size; i++)
        {
            int count = buildAttrTree(parmInfos, &result, start + offset, start + offset + 1);
            offset += count;
        }
        addAttrTo(result, parent);
        return offset;
    }

    MObject result = createAttr(parm);
    addAttrTo(result, parent);
    return 1;
}


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

