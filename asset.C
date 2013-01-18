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

#include "asset.h"
#include "util.h"
#include "common.h"
#include "instancerObject.h"
#include "geometryObject.h"

Asset::Asset(MString otlFilePath, MObject node)
    :myNode(node)
{
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;

    myObjectInfos = NULL;    

    // load the otl
    const char* filename = otlFilePath.asChar();

    //cerr << "loadasset" << endl;
    MString texturePath;
    MGlobal::executeCommand("workspace -q -rd;", texturePath);
    texturePath += "sourceimages";

    int assetId;
    hstat = HAPI_LoadOTLFile(filename, texturePath.asChar(), 3, 20, &assetId);
    Util::statusCheckLoop();
    Util::checkHAPIStatus(hstat);
    hstat = HAPI_GetAssetInfo(assetId, &info);
    Util::checkHAPIStatus(hstat);


    cerr << "Loaded asset: " << otlFilePath << " " << info.id << endl;
    cerr << "type: " << info.type << endl;
    cerr << "objectCount: " << info.objectCount << endl;

    init();

}


void
Asset::init()
{

    // input geos
    if (info.maxGeoInputCount > 0)
    {
        MFnGenericAttribute gAttr;
        MFnCompoundAttribute cAttr;

        int inputCount = info.maxGeoInputCount;

        myMayaInputs= cAttr.create("inputs", "ins");

        for (int i=0; i<inputCount; i++)
        {
            MString longName = MString("input") + (i+1);
            MString shortName = MString("in") + (i+1);
            MObject input = gAttr.create(longName, shortName);
            gAttr.addDataAccept(MFnData::kMesh);
            gAttr.addDataAccept(MFnData::kIntArray);

            cAttr.addChild(input);
        }
        addAttrTo(myMayaInputs, NULL);
    }

    // get the infos
    update();

    //TODO: remove this call - should not be necessary.
    update();

    // objects
    int objCount = info.objectCount;
    myObjects = new Object*[objCount];
    numVisibleObjects = 0;
    numObjects = objCount;

    MString title = "HAPI";
    MString status = "Creating Objects...";
    Util::showProgressWindow( title, status, 0 );

    for (int i=0; i<objCount; i++)
    {
	Util::updateProgressWindow( status, (int)( (float) i *100.0f / (float) objCount) );
        myObjects[i] = Object::createObject(info.id, i, this);
        myObjects[i]->init();
    }

    Util::hideProgressWindow();

    // build parms
    buildParms();
}


Asset::~Asset()
{
    for (int i=0; i<numObjects; i++)
        delete myObjects[i];
    delete[] myObjects;
    delete[] myObjectInfos;    
}


Object*
Asset::findObjectByName(MString name)
{
    for (int i=0; i<info.objectCount; i++)
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
    myObjectInfos = new HAPI_ObjectInfo[info.objectCount];
    HAPI_GetObjects(info.id, myObjectInfos, 0, info.objectCount);

    // update transform infos
    //delete[] transformInfos;
    //transformInfos = new HAPI_Transform[info.objectCount];
    //HAPI_GetObjectTransforms(info.id, 5, transformInfos, 0, info.objectCount);

    // update material infos
    //delete[] materialInfos;
    //materialInfos = new HAPI_MaterialInfo[info.materialCount];
    //HAPI_GetMaterials(info.id, materialInfos, 0, info.materialCount);
}


void
Asset::computeAssetInputs(const MPlug& plug, MDataBlock& data)
{
    // Geo inputs
    for (int i=0; i<info.maxGeoInputCount; i++)
    {
        MPlug inputPlug(myNode, myMayaInputs);
        MPlug elemInputPlug = inputPlug.child(i);

        if (!elemInputPlug.isConnected())
        {
            HAPI_DisconnectAssetGeometry(info.id, i);
            return;
        }

        // input from another asset
	// Depending on whether the input connection is coming from a Houdni asset or
	// just a simple Maya Mesh output, we will either connect the underlying Houdini
	// asset or simply marshal the Maya geometry.

        MDataHandle elemInputHandle = data.inputValue(elemInputPlug);
        if (elemInputHandle.type() == MFnData::kIntArray)
        {
            MFnIntArrayData fnIAD(elemInputHandle.data());
            MIntArray metaData = fnIAD.array();
            HAPI_ConnectAssetGeometry(metaData[0], metaData[1], metaData[3], info.id, i);
            return;
        }

        // raw mesh input
        cerr << "type: " << elemInputHandle.type() << endl;
        if (elemInputHandle.type() == MFnData::kMesh)
        {
            // extract mesh data from Maya
            MObject inputMesh = elemInputHandle.asMesh();
            MFnMesh fnMesh(inputMesh);
            MItMeshPolygon itMeshPoly(inputMesh);

            // get points
            MFloatPointArray points;
            fnMesh.getPoints(points);

            // get face data
            MIntArray faceCounts;
            MIntArray vertexList;
            while (!itMeshPoly.isDone())
            {
                int vc = itMeshPoly.polygonVertexCount();
                faceCounts.append(vc);
                for (int j=0; j<vc; j++)
                {
                    vertexList.append(itMeshPoly.vertexIndex(j));
                }
                itMeshPoly.next();
            }
            Util::reverseWindingOrderInt(vertexList, faceCounts);


            int inputObjId = -1;
            int inputGeoId = -1;
            HAPI_CreateGeoInput(info.id, i, &inputObjId, &inputGeoId);

            // set up GeoInfo            
            HAPI_PartInfo* partInfo    = new HAPI_PartInfo();
            partInfo->id               = 0;
            partInfo->materialId       = -1;
            partInfo->faceCount        = faceCounts.length();
            partInfo->vertexCount      = vertexList.length();
            partInfo->pointCount       = points.length();

            partInfo->pointAttributeCount  = 1;
            partInfo->vertexAttributeCount = 0;
            partInfo->faceAttributeCount   = 0;
            partInfo->detailAttributeCount = 0;

            // copy data to arrays
            int * vl = new int[partInfo->vertexCount];
            int * fc = new int[partInfo->faceCount];
            vertexList.get(vl);
            faceCounts.get(fc);

            float* pos_attr = new float[ partInfo->pointCount * 3 ];
            for ( int i = 0; i < partInfo->pointCount; ++i )
                for ( int j = 0; j < 3; ++j )
                    pos_attr[ i * 3 + j ] = points[ i ][ j ];

            // Set the data
            //HAPI_SetGeoInfo(info.id, inputObjId, inputGeoId, inputGeoInfo);
            HAPI_SetPartInfo(info.id, inputObjId, inputGeoId, partInfo);
            HAPI_SetFaceCounts(info.id, inputObjId, inputGeoId, fc, 0, partInfo->faceCount);
            HAPI_SetVertexList(info.id, inputObjId, inputGeoId, vl, 0, partInfo->vertexCount);



            // Set position attributes.
            HAPI_AttributeInfo* pos_attr_info = new HAPI_AttributeInfo();
            pos_attr_info->exists             = true;
            pos_attr_info->owner              = HAPI_ATTROWNER_POINT;
            pos_attr_info->storage            = HAPI_STORAGETYPE_FLOAT;
            pos_attr_info->count              = partInfo->pointCount;
            pos_attr_info->tupleSize          = 3;
            HAPI_AddAttribute( info.id, inputObjId, inputGeoId, "P", pos_attr_info );

            HAPI_SetAttributeFloatData(info.id, inputObjId, inputGeoId, "P", pos_attr_info,
                    pos_attr, 0, partInfo->pointCount);

            // Commit it
            HAPI_CommitGeo(info.id, inputObjId, inputGeoId);

	    delete[] vl;
	    delete[] fc;
            delete partInfo;
            delete[] pos_attr;
            delete pos_attr_info;
        }

    }
}


void
Asset::computeInstancerObjects(const MPlug& plug, MDataBlock& data)
{
    MStatus stat;

    MPlug instancersPlug = plug.child(AssetNodeAttributes::instancers);

    int instancerIndex = 0;
    MArrayDataHandle instancersHandle = data.outputArrayValue(instancersPlug);
    MArrayDataBuilder instancersBuilder = instancersHandle.builder();
    MIntArray instancedObjIds;
    for (int i=0; i<numObjects; i++)
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
        cerr << "mark instanced obj: " << obj->getName() << endl;
    }

    instancersHandle.setAllClean();
    data.setClean( instancersPlug );
}


void
Asset::computeGeometryObjects(const MPlug& plug, MDataBlock& data)
{
    MStatus stat;

    MPlug objectsPlug = plug.child(AssetNodeAttributes::objects);

    int objectIndex = 0;
    cerr << "objectsPlug: " << objectsPlug.name() << endl;
    MArrayDataHandle objectsHandle = data.outputArrayValue(objectsPlug);
    MArrayDataBuilder objectsBuilder = objectsHandle.builder();
    cerr << "size +++++: " << objectsBuilder.elementCount() << endl;
    for (int i=0; i<numObjects; i++)
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
    cerr << "objectIndex: " << objectIndex  << " objBuilderSizeCheck: " << objBuilderSizeCheck << endl;
    if (objBuilderSizeCheck > objectIndex)
    {
        for (int i=objectIndex; i<objBuilderSizeCheck; i++)
        {
            try
            {
                cerr << "    remove item" << endl;
                stat = objectsBuilder.removeElement(i);
                Util::checkMayaStatus(stat);
            } catch (MayaError& e)
            {
                cerr << e.what() << endl;
            }
        }
    }
    cerr << "objBuilderSizeCheck again: " << objectsBuilder.elementCount() << endl;
    objectsHandle.set(objectsBuilder);

    objectsHandle.setAllClean();

    data.setClean(objectsPlug);
}


MStatus
Asset::compute(const MPlug& plug, MDataBlock& data)
{
    MStatus stat(MS::kSuccess);

    // Set the type
    MPlug typePlug( myNode, AssetNodeAttributes::assetType);
    MDataHandle typeHandle = data.outputValue(typePlug);

    //The asset info struct (info) was set at the constructor
    //of this class, which is at asset load time.
    typeHandle.set(info.type);

    // Set the time
    MPlug timePlug( myNode, AssetNodeAttributes::timeInput);
    MDataHandle timeHandle = data.inputValue(timePlug);
    MTime currentTime = timeHandle.asTime();
    float time = (float)currentTime.as(MTime::kSeconds);
    cerr << "current time: " << time << endl;
    HAPI_SetTime(time);

    //this figures out the Houdini asset inputs (Geo, Transform)
    //for inter-asset stuff
    computeAssetInputs(plug, data);

    HAPI_CookAsset(info.id);


    Util::statusCheckLoop();

    cerr << "cooked asset" << endl;

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
    MString name = Util::getString(parm.nameSH);
    MString parmName = Util::replaceChar(name, ' ', '_');

    MString shortName = MString("_parm") + parm.id + "_" + parmName + "_";
    MString longName = MString("_H_parm") + parm.id + "_" + parmName + "_";

    MObject result;

    // Other types
    switch(parm.type)
    {
        case HAPI_PARMTYPE_FOLDERLIST:
        case HAPI_PARMTYPE_FOLDER:
            result = cAttr.create(longName, shortName);
            cAttr.setStorable(true);
            cAttr.setNiceNameOverride(label);
            break;
        case HAPI_PARMTYPE_SEPARATOR:
            result = gAttr.create(longName, shortName);
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
            result = createNumericAttr(parm, longName, shortName, label);
            break;
        case HAPI_PARMTYPE_STRING:
        case HAPI_PARMTYPE_FILE:
            result = createStringAttr(parm, longName, shortName, label);
            break;
        default:
            result = nAttr.create(longName, shortName, MFnNumericData::kFloat, 1.0);
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
        HAPI_GetParmChoiceLists(info.id, choiceInfos, parm.choiceIndex, choiceCount);
        for (int i=0; i<choiceCount; i++)
        {
            MString field = Util::getString(choiceInfos[i].labelSH);
            eAttr.addField(field, i);
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
    int parmCount = info.parmCount;
    if (parmCount <= 0)
        return;
    HAPI_ParmInfo * parmInfos = new HAPI_ParmInfo[parmCount];
    HAPI_GetParameters(info.id, parmInfos, 0, parmCount);

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
    HAPI_GetParmIntValues(info.id, values, index, size);

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
    HAPI_GetParmFloatValues(info.id, values, index, size);

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
    HAPI_GetParmStringValues(info.id, handles, index, size);

    MStringArray ret;
    for (int i=0; i<size; i++)
    {
        MString str = Util::getString(handles[i]);
        ret.append(str);
    }

    delete[] handles;
    return ret;
}

