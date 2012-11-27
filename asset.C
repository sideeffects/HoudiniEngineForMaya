#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MGlobal.h>

#include "asset.h"
#include "util.h"
#include "common.h"
#include "instancerObject.h"

Asset::Asset(MString otlFilePath, MObject node)
    :node(node)
{
    HAPI_StatusCode hstat = HAPI_STATUS_SUCCESS;

    objectInfos = NULL;
    transformInfos = NULL;
    materialInfos = NULL;

    // load the otl
    const char* filename = otlFilePath.asChar();
    info.minVerticesPerPrimitive = 3;
    info.maxVerticesPerPrimitive = 20;

    //cerr << "loadasset" << endl;
    MString texturePath;
    MGlobal::executeCommand("workspace -q -rd;", texturePath);
    texturePath += "sourceimages";
    hstat = HAPI_LoadOTLFile(filename, texturePath.asChar(), &info);
    Util::checkHAPIStatus(hstat);


    cerr << "Loaded asset: " << otlFilePath << " " << info.id << endl;
    cerr << "type: " << info.type << endl;
    cerr << "objectCount: " << info.objectCount << endl;
    cerr << "minGeoInputCount: " << info.minGeoInputCount << endl;
    cerr << "maxGeoInputCount: " << info.maxGeoInputCount << endl;

    // input geos
    if (info.maxGeoInputCount > 0)
    {
        MFnTypedAttribute tAttr;
        MFnCompoundAttribute cAttr;

        int inputCount = info.maxGeoInputCount;

        AssetNodeAttributes::input = cAttr.create("input", "in");

        for (int i=0; i<inputCount; i++)
        {
            MString longName = MString("input") + (i+1);
            MString shortName = MString("in") + (i+1);
            MObject input = tAttr.create(longName, shortName, MFnData::kMesh);

            cAttr.addChild(input);
        }
        addAttrTo(AssetNodeAttributes::input, NULL);
    }

    // get the infos
    update();

    // objects
    int objCount = info.objectCount;
    objects = new Object*[objCount];
    numVisibleObjects = 0;
    numObjects = objCount;
    for (int i=0; i<objCount; i++)
    {
        objects[i] = Object::createObject(info.id, i, this);
    }

    // build parms
    buildParms();
}


Object*
Asset::findObjectByName(MString name)
{
    for (int i=0; i<info.objectCount; i++)
    {
        if (objects[i]->getName() == name)
            return objects[i];
    }

    return NULL;
}


Object*
Asset::findObjectById(int id)
{
    return objects[id];
}


// Getters for infos
HAPI_ObjectInfo
Asset::getObjectInfo(int id) { return objectInfos[id]; }
HAPI_Transform
Asset::getTransformInfo(int id) { return transformInfos[id]; }
HAPI_MaterialInfo
Asset::getMaterialInfo(int id) { return materialInfos[id]; }


void
Asset::update()
{
    // update object infos
    delete[] objectInfos;
    objectInfos = new HAPI_ObjectInfo[info.objectCount];
    HAPI_GetObjects(info.id, objectInfos, 0, info.objectCount);

    // update transform infos
    delete[] transformInfos;
    transformInfos = new HAPI_Transform[info.objectCount];
    HAPI_GetObjectTransforms(info.id, 5, transformInfos, 0, info.objectCount);

    // update material infos
    delete[] materialInfos;
    materialInfos = new HAPI_MaterialInfo[info.materialCount];
    HAPI_GetMaterials(info.id, materialInfos, 0, info.materialCount);
}


void
Asset::computeGeoInputs(const MPlug& plug, MDataBlock& data)
{
    // Geo inputs
    for (int i=0; i<info.maxGeoInputCount; i++)
    {

        MPlug inputPlug(node, AssetNodeAttributes::input);
        MPlug elemInputPlug = inputPlug.child(i);
        MDataHandle elemInputHandle = data.inputValue(elemInputPlug);
        cerr << "type: " << elemInputHandle.type() << endl;
        if (!elemInputPlug.isConnected() || elemInputHandle.type() != MFnData::kMesh)
        {
            cerr << "plug name: " << elemInputPlug.name() << endl;
            cerr << "isConnected: " << elemInputPlug.isConnected() << endl;
            cerr << "type match kmesh: " << (elemInputHandle.type() == MFnData::kMesh) << endl;
            // invalid or no input
            // have to disconnect geo input in Houdini
            HAPI_DisconnectAssetGeometry(info.id, i);
        }
        else
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
            HAPI_GeoInfo* inputGeoInfo = new HAPI_GeoInfo();
            inputGeoInfo->id           = inputGeoId;
            inputGeoInfo->materialId   = -1;
            inputGeoInfo->faceCount    = faceCounts.length();
            inputGeoInfo->vertexCount  = vertexList.length();
            inputGeoInfo->pointCount   = points.length();

            inputGeoInfo->pointAttributeCount  = 1;
            inputGeoInfo->vertexAttributeCount = 0;
            inputGeoInfo->faceAttributeCount   = 0;
            inputGeoInfo->detailAttributeCount = 0;

            // copy data to arrays
            int vl[inputGeoInfo->vertexCount];
            int fc[inputGeoInfo->faceCount];
            vertexList.get(vl);
            faceCounts.get(fc);

            float* pos_attr = new float[ inputGeoInfo->pointCount * 3 ];
            for ( int i = 0; i < inputGeoInfo->pointCount; ++i )
                for ( int j = 0; j < 3; ++j )
                    pos_attr[ i * 3 + j ] = points[ i ][ j ];

            // Set the data
            HAPI_SetGeoInfo(info.id, inputObjId, inputGeoId, inputGeoInfo);
            HAPI_SetFaceCounts(info.id, inputObjId, inputGeoId, fc, 0, inputGeoInfo->faceCount);
            HAPI_SetVertexList(info.id, inputObjId, inputGeoId, vl, 0, inputGeoInfo->vertexCount);



            // Set position attributes.
            HAPI_AttributeInfo* pos_attr_info = new HAPI_AttributeInfo();
            pos_attr_info->exists             = true;
            pos_attr_info->owner              = HAPI_ATTROWNER_POINT;
            pos_attr_info->storage            = HAPI_STORAGETYPE_FLOAT;
            pos_attr_info->count              = inputGeoInfo->pointCount;
            pos_attr_info->tupleSize          = 3;
            HAPI_AddAttribute( info.id, inputObjId, inputGeoId, "P", pos_attr_info );

            HAPI_SetAttributeFloatData(info.id, inputObjId, inputGeoId, "P", pos_attr_info,
                    pos_attr, 0, inputGeoInfo->pointCount);

            // Commit it
            HAPI_CommitGeo(info.id, inputObjId, inputGeoId);
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
        Object* obj = objects[i];
        //MPlug instancerElemPlug = instancersPlug.elementByLogicalIndex(instancerIndex);

        if (obj->type() == Object::OBJECT_TYPE_INSTANCER)
        {
            MDataHandle instancerElemHandle = instancersBuilder.addElement(instancerIndex);
            stat = obj->compute(instancerElemHandle);
            if (MS::kSuccess == stat)
            {
                instancerIndex++;

                // get all the object ids that are instanced
                MIntArray instIds = ((InstancerObject*)obj)->getInstancedObjIds();
                MStringArray instNames = ((InstancerObject*)obj)->getUniqueInstObjNames();
                for (int j=0; j<instNames.length(); j++)
                {
                    Object* o = findObjectByName(instNames[j]);
                    if (o != NULL)
                        instancedObjIds.append(o->getId());
                }
                for (int j=0; j<instIds.length(); j++)
                {
                    instancedObjIds.append(instIds[j]);
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
    for (int i=0; i<instancedObjIds.length(); i++)
    {
        Object* obj = objects[instancedObjIds[i]];
        obj->isInstanced = true;
    }

    data.setClean(instancersPlug);
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


        Object* obj = objects[i];
        //MPlug objectElemPlug = objectsPlug.elementByLogicalIndex(objectIndex);

        if (obj->type() == Object::OBJECT_TYPE_GEOMETRY)
        {
            MDataHandle h = objectsBuilder.addElement(objectIndex);
            stat = obj->compute(h);
            if (MS::kSuccess == stat)
            {
                objectIndex++;
            }
        }
    }
    // clean up extra elements
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
            } catch (HAPIError e)
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
    MStatus stat;

    computeGeoInputs(plug, data);

    update();

    // first pass - instancers
    computeInstancerObjects(plug, data);

    // second pass - geometry objects
    computeGeometryObjects(plug, data);

}


MObjectArray
Asset::getParmAttributes()
{
    return parmAttributes;
}


Object**
Asset::getObjects()
{
    return objects;
}


// Parms ---------------------------------------------------
void
Asset::addAttrTo(MObject& child, MObject* parent)
{
    if (NULL == parent)
    {
        parmAttributes.append(child);
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

    MString shortName = MString("_parm") + parm.id + "_";
    MString longName = MString("_H_parm") + parm.id + "_";

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

        HAPI_ParmChoiceInfo choiceInfos[choiceCount];
        HAPI_GetParmChoiceLists(info.id, choiceInfos, parm.choiceIndex, choiceCount);
        for (int i=0; i<choiceCount; i++)
        {
            MString field = Util::getString(choiceInfos[i].labelSH);
            eAttr.addField(field, i);
        }

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
        nAttr.setMin(parm.max);
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
    HAPI_ParmInfo myParmInfos[parmCount];
    HAPI_GetParameters(info.id, myParmInfos, 0, parmCount);

    int index = 0;
    while (index < parmCount)
    {
        int consumed = buildAttrTree(myParmInfos, NULL, index, index+1);
        index += consumed;
    }


}


int
Asset::buildAttrTree(HAPI_ParmInfo* myParmInfos, MObject* parent, int current, int start)
{
    HAPI_ParmInfo parm = myParmInfos[current];

    if (parm.type == HAPI_PARMTYPE_FOLDERLIST)
    {
        int offset = parm.size;
        for (int i = start; i < start+parm.size; i++)
        {
            int count = buildAttrTree(myParmInfos, parent, i, start + offset);
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
            int count = buildAttrTree(myParmInfos, &result, start + offset, start + offset + 1);
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
    int values[size];
    HAPI_GetParmIntValues(info.id, values, index, size);

    MIntArray ret(values, size);
    return ret;
}


MFloatArray
Asset::getParmFloatValues(HAPI_ParmInfo& parm)
{
    int index = parm.floatValuesIndex;
    int size = parm.size;
    float values[size];
    HAPI_GetParmFloatValues(info.id, values, index, size);

    MFloatArray ret(values, size);
    return ret;
}


MStringArray
Asset::getParmStringValues(HAPI_ParmInfo& parm)
{
    int index = parm.stringValuesIndex;
    int size = parm.size;
    int handles[size];
    HAPI_GetParmStringValues(info.id, handles, index, size);

    MStringArray ret;
    for (int i=0; i<size; i++)
    {
        MString str = Util::getString(handles[i]);
        ret.append(str);
    }
    return ret;
}

