#include <maya/MFnNumericAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnMesh.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatPoint.h>
#include <maya/MVectorArray.h>
#include <maya/MFnMeshData.h>
#include <maya/MDataHandle.h>
#include <maya/MTypes.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MGlobal.h>

#include "asset.h"

// MCheckStatus (Debugging tool)
//
#   define MCheckStatus(status,message)         \
        if( MS::kSuccess != status ) {          \
            MString error("Status failed: ");   \
            error += status.errorString();      \
            MGlobal::displayError(error);       \
            MGlobal::displayError(message);       \
        } else {                                \
            MString str("Success: ");           \
            str += message;                     \
            MGlobal::displayInfo(str);          \
        }

MTypeId Asset::id(0x80000);
MObject Asset::input1;
MObject Asset::input2;
MObject Asset::output;
MObject Asset::meshes;
MObject Asset::transforms;
MObject Asset::translateAttr;
MObject Asset::translateAttrX;
MObject Asset::translateAttrY;
MObject Asset::translateAttrZ;
MObject Asset::rotateAttr;
MObject Asset::rotateAttrX;
MObject Asset::rotateAttrY;
MObject Asset::rotateAttrZ;
MObject Asset::scaleAttr;
MObject Asset::scaleAttrX;
MObject Asset::scaleAttrY;
MObject Asset::scaleAttrZ;
MObject Asset::numObjects;

void*
Asset::creator()
{
    Asset* ret = new Asset();
    return ret;
}

void printAssetInfo(HAPI_AssetInfo* assetInfo)
{
    cerr << "id: " << assetInfo->id << endl;
    cerr << "parmCount: " << assetInfo->parmCount << endl;
    cerr << "parmChoiceCount: " << assetInfo->parmChoiceCount << endl;
    cerr << "handleCount: " << assetInfo->handleCount << endl;
    cerr << "objectCount: " << assetInfo->objectCount << endl;
}

MStatus
Asset::initialize()
{
    //char* dir = "/home/jhuang/dev_projects/HAPI/Maya/assets/plugin";
    HAPI_Initialize("");

    // maya plugin stuff
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;
    MFnCompoundAttribute cAttr;
    MFnUnitAttribute uAttr;

    input1 = tAttr.create("input", "in", MFnData::kString);
    tAttr.setStorable(true);

    // numObjects
    numObjects = nAttr.create("numberOfObjects", "no", MFnNumericData::kInt);
    nAttr.setWritable(false);
    nAttr.setStorable(false);

    // meshes
    meshes = tAttr.create("meshes", "ms", MFnData::kMesh);
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    tAttr.setArray(true);
    tAttr.setIndexMatters(true);

    // translate
    translateAttrX = uAttr.create("translateX", "tx", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    translateAttrY = uAttr.create("translateY", "ty", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    translateAttrZ = uAttr.create("translateZ", "tz", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    translateAttr = nAttr.create("translate", "t", translateAttrX, translateAttrY, translateAttrZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // rotate
    rotateAttrX = uAttr.create("rotateX", "rx", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    rotateAttrY = uAttr.create("rotateY", "ry", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    rotateAttrZ = uAttr.create("rotateZ", "rz", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    rotateAttr = nAttr.create("rotate", "r", rotateAttrX, rotateAttrY, rotateAttrZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // scale
    scaleAttrX = nAttr.create("scaleX", "sx", MFnNumericData::kDouble);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    scaleAttrY = nAttr.create("scaleY", "sy", MFnNumericData::kDouble);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    scaleAttrZ = nAttr.create("scaleZ", "sz", MFnNumericData::kDouble);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    scaleAttr = nAttr.create("scale", "s", scaleAttrX, scaleAttrY, scaleAttrZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // transforms
    transforms = cAttr.create("transforms", "xfs");
    cAttr.addChild(translateAttr);
    cAttr.addChild(rotateAttr);
    cAttr.addChild(scaleAttr);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    cAttr.setArray(true);
    cAttr.setIndexMatters(true);

    // output
    output = cAttr.create("output", "out");
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    cAttr.addChild(numObjects);
    cAttr.addChild(meshes);
    cAttr.addChild(transforms);

    addAttribute(input1);
    addAttribute(output);

    attributeAffects(input1, output);
    attributeAffects(input1, numObjects);



    //char *buf;
    //HAPI_PrintNetwork(buf);

    //cerr << "net: " << buf << endl;

    return MS::kSuccess;
}

Asset::Asset()
{
    // houdini
    //char* filename = "/home/jhuang/dev_projects/HAPI/Maya/assets/otls/SideFX__spaceship.otl";
    //char* filename = "/home/jhuang/dev_projects/HAPI/Maya/assets/otls/box2.otl";
    char* filename = "/home/jhuang/dev_projects/HAPI/Maya/assets/otls/dummyboxes.otl";
    //char* filename = "/home/jhuang/dev_projects/HAPI/Unity/Assets/OTLs/handlesTest.otl";

    // load otl
    assetInfo = new HAPI_AssetInfo();
    assetInfo->minVerticesPerPrimitive = 3;
    assetInfo->maxVerticesPerPrimitive = 20;
    HAPI_LoadOTLFile(filename, NULL, assetInfo);

    printAssetInfo(assetInfo);
    builtParms = false;

}

Asset::~Asset() {}

void printIntArray(int* arr, int size)
{
    for (int i=0; i<size; i++)
    {
       cerr << arr[i] << " ";
    }
    cerr << endl;
}

MStatus Asset::setDependentsDirty(const MPlug& plugBeingDirtied,
        MPlugArray& affectedPlugs)
{
    cerr << "set dependents dirty: " << plugBeingDirtied.name() << endl;
    //if (plugBeingDirtied == output ||
        //plugBeingDirtied == transforms ||
        //plugBeingDirtied == meshes ||
        //plugBeingDirtied == numObjects)
        //return MS::kSuccess;

    MPlug meshesPlug(thisMObject(), meshes);
    MPlug transformsPlug(thisMObject(), transforms);
    for (int i=0; i < assetInfo->objectCount; i++)
    {
        MPlug meshElemPlug = meshesPlug.elementByLogicalIndex(i);
        MPlug transformElemPlug = transformsPlug.elementByLogicalIndex(i);
        affectedPlugs.append(meshElemPlug);
        affectedPlugs.append(transformElemPlug);
        for (int j=0; j<3; j++)
        {
            MPlug componentPlug = transformElemPlug.child(j);
            cerr << "componentPlug: " << componentPlug.name() << endl;
            affectedPlugs.append(componentPlug);
        }
    }
    return MS::kSuccess;
}



void
Asset::addAttrTo(MObject& child, MObject* parent)
{
    if (NULL == parent)
    {
        MFnDependencyNode fnDN(thisMObject());
        fnDN.addAttribute(child);
        return;
    }

    MFnCompoundAttribute cAttr(*parent);
    cAttr.addChild(child);
}

MString
getString(int handle)
{
    int bufLen;
    HAPI_GetStringLength(handle, &bufLen);
    char buffer[bufLen];
    HAPI_GetString(handle, buffer, bufLen+1);
    return MString(buffer);
}

void
Asset::createAttr(HAPI_ParmInfo& parm, MObject* result)
{
    MFnNumericAttribute nAttr;
    MFnCompoundAttribute cAttr;
    MFnGenericAttribute gAttr;
    MFnTypedAttribute tAttr;

    MString label = getString(parm.labelSH);
    MString name = getString(parm.nameSH);

    MString shortName = MString("_parm") + parm.id + "_";
    MString longName = MString("_H_parm") + parm.id + "_";

    // TODO: get initial values
    switch(parm.type)
    {
        case HAPI_PARMTYPE_FOLDERLIST:
        case HAPI_PARMTYPE_FOLDER:
            *result = cAttr.create(longName, shortName);
            cAttr.setStorable(true);
            cAttr.setNiceNameOverride(label);
            //cAttr.addToCategory(MString("folder"));
            break;
        case HAPI_PARMTYPE_SEPARATOR:
            *result = gAttr.create(longName, shortName);
            gAttr.setHidden(true);
            gAttr.setStorable(false);
            gAttr.setReadable(false);
            gAttr.setWritable(false);
            gAttr.setConnectable(false);
            gAttr.setNiceNameOverride(label);
            //gAttr.addToCategory(MString("separator"));
            break;
        case HAPI_PARMTYPE_INT:
        case HAPI_PARMTYPE_FLOAT:
        case HAPI_PARMTYPE_COLOUR:
        case HAPI_PARMTYPE_TOGGLE:
            createNumericAttr(parm, result, longName, shortName, label);
            break;
        case HAPI_PARMTYPE_STRING:
        case HAPI_PARMTYPE_FILE:
            createStringAttr(parm, result, longName, shortName, label);
            break;
        default:
            *result = nAttr.create(longName, shortName, MFnNumericData::kFloat, 1.0);
            nAttr.setStorable(true);
            nAttr.setNiceNameOverride(label);
            //nAttr.addToCategory(MString("attribute"));
            break;
    }
}

void
Asset::createStringAttr(HAPI_ParmInfo& parm, MObject* result, MString& longName, MString& shortName, MString& niceName)
{
    MFnTypedAttribute tAttr;
    MFnCompoundAttribute cAttr;

    int size = parm.size;

    if (size > 1)
    {
        *result = cAttr.create(longName, shortName);
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
        return;
    }

    *result = tAttr.create(longName, shortName, MFnData::kString);
    tAttr.setStorable(true);
    tAttr.setNiceNameOverride(niceName);
    if (parm.type == HAPI_PARMTYPE_FILE)
        tAttr.setUsedAsFilename(true);
}

void
Asset::createNumericAttr(HAPI_ParmInfo& parm, MObject* result, MString& longName, MString& shortName, MString& niceName)
{
    MFnNumericAttribute nAttr;
    MFnCompoundAttribute cAttr;


    int size = parm.size;


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
        *result = cAttr.create(longName, shortName);
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
        return;
    }

    if (parm.type == HAPI_PARMTYPE_COLOUR)
        *result = nAttr.createColor(longName, shortName);
    else
        *result = nAttr.create(longName, shortName, type);
    nAttr.setStorable(true);
    nAttr.setNiceNameOverride(niceName);

    if (parm.hasMin)
        nAttr.setMin(parm.min);
    if (parm.hasMax)
        nAttr.setMin(parm.max);
    if (parm.hasUIMin)
        nAttr.setSoftMin(parm.UIMin);
    if (parm.hasUIMax)
        nAttr.setSoftMax(parm.UIMax);

    return;
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
        MObject result;
        createAttr(parm, &result);
        int offset = 0;
        for (int i = start; i < start+parm.size; i++)
        {
            int count = buildAttrTree(myParmInfos, &result, start + offset, start + offset + 1);
            offset += count;
        }
        addAttrTo(result, parent);
        return offset;
    }

    MObject result;
    createAttr(parm, &result);
    addAttrTo(result, parent);
    return 1;
}

MIntArray
Asset::getParmIntValues(HAPI_ParmInfo& parm)
{
    int index = parm.intValuesIndex;
    int size = parm.size;
    int values[size];
    HAPI_GetParmIntValues(assetInfo->id, values, index, size);

    MIntArray ret(values, size);
    return ret;
}

MFloatArray
Asset::getParmFloatValues(HAPI_ParmInfo& parm)
{
    int index = parm.floatValuesIndex;
    int size = parm.size;
    float values[size];
    HAPI_GetParmFloatValues(assetInfo->id, values, index, size);

    MFloatArray ret(values, size);
    return ret;
}

MStringArray
Asset::getParmStringValues(HAPI_ParmInfo& parm)
{
    int index = parm.stringValuesIndex;
    int size = parm.size;
    int handles[size];
    HAPI_GetParmStringValues(assetInfo->id, handles, index, size);

    MStringArray ret;
    for (int i=0; i<size; i++)
    {
        MString str = getString(handles[i]);
        ret.append(str);
    }
    return ret;
}

void
Asset::buildParms()
{

    // PARMS
    int parmCount = assetInfo->parmCount;
    if (parmCount <= 0)
        return;
    HAPI_ParmInfo myParmInfos[parmCount];
    HAPI_GetParameters(assetInfo->id, myParmInfos, 0, parmCount);

    int index = 0;
    while (index < parmCount)
    {
        int consumed = buildAttrTree(myParmInfos, NULL, index, index+1);
        index += consumed;
    }


}


MObject
Asset::getAttrFromParm(HAPI_ParmInfo& parm)
{
    MFnDependencyNode fnDN(thisMObject());

    MString name = MString("_parm") + parm.id + "_";
    MObject attr = fnDN.attribute(name);
    return attr;
}


void
Asset::updateAttrValue(HAPI_ParmInfo& parm, MDataBlock& data)
{
    // get attribute
    MObject attr = getAttrFromParm(parm);

    // create plug to the attribute
    MPlug plug(thisMObject(), attr);

    // TODO: don't use setValue, it re-dirties the input
    int size = parm.size;
    if(parm.isInt())
    {
        MIntArray values = getParmIntValues(parm);
        if (size == 1)
        {
            MDataHandle handle = data.inputValue(plug);
            handle.set(values[0]);
        }
        else
        {
            for (int i=0; i<size; i++)
            {
                MPlug child = plug.child(i);
                MDataHandle handle = data.inputValue(child);
                handle.set(values[i]);
            }
        }
        return;
    }

    if(parm.isFloat())
    {
        MFloatArray values = getParmFloatValues(parm);
        if (size == 1)
        {
            MDataHandle handle = data.inputValue(plug);
            handle.set(values[0]);
        }
        else
        {
            for (int i=0; i<size; i++)
            {
                MPlug child = plug.child(i);
                MDataHandle handle = data.inputValue(child);
                handle.set(values[i]);
            }
        }
        return;
    }

    if(parm.isString())
    {
        MStringArray values = getParmStringValues(parm);
        if (size == 1)
        {
            MDataHandle handle = data.inputValue(plug);
            handle.set(values[0]);
        }
        else
        {
            for (int i=0; i<size; i++)
            {
                MPlug child = plug.child(i);
                MDataHandle handle = data.inputValue(child);
                handle.set(values[i]);
            }
        }
        return;
    }
}


void
Asset::updateAttrValues(MDataBlock& data)
{
    int parmCount = assetInfo->parmCount;
    if (parmCount <= 0)
        return;
    HAPI_ParmInfo myParmInfos[parmCount];
    HAPI_GetParameters(assetInfo->id, myParmInfos, 0, parmCount);

    for (int i=0; i<parmCount; i++)
    {
        HAPI_ParmInfo& parm = myParmInfos[i];
        updateAttrValue(parm, data);
    }
}

void
Asset::setParmValue(HAPI_ParmInfo& parm, MDataBlock& data)
{
    MObject attr = getAttrFromParm(parm);
    MPlug plug(thisMObject(), attr);
    int size = parm.size;

    if (parm.isInt())
    {
        int values[size];
        if (size == 1)
        {
            MDataHandle handle = data.inputValue(plug);
            values[0] = handle.asInt();
        } else
        {
            for (int i=0; i<size; i++)
            {
                MPlug child = plug.child(i);
                MDataHandle handle = data.inputValue(child);
                values[i] = handle.asInt();
            }
        }
        HAPI_SetParmIntValues(assetInfo->id, values, parm.intValuesIndex, size);
    }

    if (parm.isFloat())
    {
        float values[size];
        if (size == 1)
        {
            MDataHandle handle = data.inputValue(plug);
            values[0] = handle.asFloat();
        } else
        {
            for (int i=0; i<size; i++)
            {
                MPlug child = plug.child(i);
                MDataHandle handle = data.inputValue(child);
                values[i] = handle.asFloat();
            }
        }
        HAPI_SetParmFloatValues(assetInfo->id, values, parm.floatValuesIndex, size);
    }

    if (parm.isString())
    {
        if (size == 1)
        {
            MDataHandle handle = data.inputValue(plug);
            const char* val = handle.asString().asChar();
            HAPI_SetParmStringValue(assetInfo->id, val, parm.id, 0);
        } else
        {
            for (int i=0; i<size; i++)
            {
                MPlug child = plug.child(i);
                MDataHandle handle = data.inputValue(child);
                const char* val = handle.asString().asChar();
                HAPI_SetParmStringValue(assetInfo->id, val, parm.id, i);
            }
        }
    }

}

void
Asset::setParmValues(MDataBlock& data)
{
    int parmCount = assetInfo->parmCount;
    if (parmCount <= 0)
        return;
    HAPI_ParmInfo myParmInfos[parmCount];
    HAPI_GetParameters(assetInfo->id, myParmInfos, 0, parmCount);

    for (int i=0; i<parmCount; i++)
    {
        HAPI_ParmInfo& parm = myParmInfos[i];
        setParmValue(parm, data);
    }
}


MStatus
Asset::compute(const MPlug& plug, MDataBlock& data)
{

    if (!builtParms)
    {
        buildParms();
        builtParms = true;
    } else
    {
        setParmValues(data);
    }

    cerr << "compute" << endl;
    updateAttrValues(data);

    // don't care what the plug is, recompute everything
    // TODO: this might not be good later on

    // number of objects
    int objCount = assetInfo->objectCount;
    cerr << "objcount: " << objCount << endl;
    MPlug numObjectsPlug(thisMObject(), numObjects);
    MDataHandle numObjectsHandle = data.outputValue(numObjects);
    numObjectsHandle.set(objCount);
    data.setClean(numObjectsPlug);

    // meshes
    HAPI_ObjectInfo myObjects[objCount];
    HAPI_GetObjects(assetInfo->id, myObjects, 0, objCount);

    MPlug outputPlug(thisMObject(), output);
    MPlug meshesPlug(thisMObject(), meshes);
    MPlug transformsPlug(thisMObject(), transforms);
    for (int i=0; i<objCount; i++)
    {
        MPlug elemPlug = meshesPlug.elementByLogicalIndex(i);
        cerr << "elemPlug: " << elemPlug.name() << endl;
        MDataHandle outHandle = data.outputValue(elemPlug);

        // Mesh data
        MFnMeshData dataCreator;
        MObject newMeshData = dataCreator.create();

        Object obj(myObjects[i], assetInfo->id);
        obj.createMesh(newMeshData);

        outHandle.set(newMeshData);

        MPlug xformPlug = transformsPlug.elementByLogicalIndex(i);
        obj.updateTransform(xformPlug, data);

    }

    // set everything clean
    data.setClean(numObjectsPlug);
    for (int i=0; i<objCount; i++)
    {
        MPlug elemPlug = meshesPlug.elementByLogicalIndex(i);
        MPlug xformPlug = transformsPlug.elementByLogicalIndex(i);
        data.setClean(elemPlug);
        data.setClean(xformPlug);
    }

    data.setClean(meshesPlug);
    data.setClean(transformsPlug);
    data.setClean(outputPlug);

    return MS::kSuccess;
}

MStatus
initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, "Asset plugin", "1.0", "Any");
    status = plugin.registerNode("Asset", Asset::id, Asset::creator, Asset::initialize);
    return status;
}

MStatus
uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);
    status = plugin.deregisterNode(Asset::id);
    return status;
}
