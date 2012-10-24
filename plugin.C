#include <maya/MFnNumericAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnEnumAttribute.h>
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

#include <sys/time.h>

#include "plugin.h"

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

MTypeId Plugin::id(0x80000);
MObject Plugin::fileNameAttr;
MObject Plugin::output;
MObject Plugin::meshes;
MObject Plugin::transforms;
MObject Plugin::translateAttr;
MObject Plugin::translateAttrX;
MObject Plugin::translateAttrY;
MObject Plugin::translateAttrZ;
MObject Plugin::rotateAttr;
MObject Plugin::rotateAttrX;
MObject Plugin::rotateAttrY;
MObject Plugin::rotateAttrZ;
MObject Plugin::scaleAttr;
MObject Plugin::scaleAttrX;
MObject Plugin::scaleAttrY;
MObject Plugin::scaleAttrZ;
MObject Plugin::numObjects;

void*
Plugin::creator()
{
    Plugin* ret = new Plugin();
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
Plugin::initialize()
{
    char* dir = "/home/jhuang/dev_projects/HAPI/Maya/assets/otls/";
    HAPI_Initialize("/home/jhuang/dev/hfs/", dir);

    // maya plugin stuff
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;
    MFnCompoundAttribute cAttr;
    MFnUnitAttribute uAttr;

    fileNameAttr = tAttr.create("fileName", "fn", MFnData::kString);
    tAttr.setStorable(true);

    // numObjects
    numObjects = nAttr.create("numberOfObjects", "no", MFnNumericData::kInt);
    nAttr.setWritable(false);
    nAttr.setStorable(false);
    nAttr.setConnectable(false);
    nAttr.setHidden(true);

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
    scaleAttrX = nAttr.create("scaleX", "sx", MFnNumericData::kDouble, 1.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    scaleAttrY = nAttr.create("scaleY", "sy", MFnNumericData::kDouble, 1.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    scaleAttrZ = nAttr.create("scaleZ", "sz", MFnNumericData::kDouble, 1.0);
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

    addAttribute(fileNameAttr);
    addAttribute(output);

    attributeAffects(fileNameAttr, output);
    attributeAffects(fileNameAttr, numObjects);

    return MS::kSuccess;
}


Plugin::Plugin()
{
    // houdini
    //char* filename = "/home/jhuang/dev_projects/HAPI/Maya/assets/otls/SideFX__spaceship.otl";
    //char* filename = "/home/jhuang/dev_projects/HAPI/Maya/assets/otls/box2.otl";
    //char* filename = "/home/jhuang/dev_projects/HAPI/Maya/assets/otls/dummyboxes.otl";
    //char* filename = "/home/jhuang/dev_projects/HAPI/Unity/Plugins/OTLs/handlesTest.otl";

    builtParms = false;
    assetChanged = true;

}

Plugin::~Plugin() {}


MStatus Plugin::setDependentsDirty(const MPlug& plugBeingDirtied,
        MPlugArray& affectedPlugs)
{
    if (plugBeingDirtied == fileNameAttr)
        return MS::kSuccess;

    MPlug meshesPlug(thisMObject(), meshes);
    MPlug transformsPlug(thisMObject(), transforms);
    for (int i=0; i < asset->info.objectCount; i++)
    {
        MPlug meshElemPlug = meshesPlug.elementByLogicalIndex(i);
        MPlug transformElemPlug = transformsPlug.elementByLogicalIndex(i);
        affectedPlugs.append(meshElemPlug);
        affectedPlugs.append(transformElemPlug);
        for (int j=0; j<3; j++)
        {
            MPlug componentPlug = transformElemPlug.child(j);
            affectedPlugs.append(componentPlug);
        }
    }
    return MS::kSuccess;
}


MObject
Plugin::getAttrFromParm(HAPI_ParmInfo& parm)
{
    MFnDependencyNode fnDN(thisMObject());

    MString name = MString("_parm") + parm.id + "_";
    MObject attr = fnDN.attribute(name);
    return attr;
}


void
Plugin::updateAttrValue(HAPI_ParmInfo& parm, MDataBlock& data)
{

    // get attribute
    MObject attr = getAttrFromParm(parm);

    // create plug to the attribute
    MPlug plug(thisMObject(), attr);

    int size = parm.size;
    if(parm.isInt())
    {
        MIntArray values = asset->getParmIntValues(parm);
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
        MFloatArray values = asset->getParmFloatValues(parm);
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
        MStringArray values = asset->getParmStringValues(parm);
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
Plugin::updateAttrValues(MDataBlock& data)
{
    int parmCount = asset->info.parmCount;
    if (parmCount <= 0)
        return;
    HAPI_ParmInfo myParmInfos[parmCount];
    HAPI_GetParameters(asset->info.id, myParmInfos, 0, parmCount);

    for (int i=0; i<parmCount; i++)
    {
        HAPI_ParmInfo& parm = myParmInfos[i];
        updateAttrValue(parm, data);
    }
}


double
getTime()
{
        timeval  tv;
        gettimeofday(&tv, NULL);
        double time_in_mill =
                     (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to millisecond
        return time_in_mill;
}

void
Plugin::setParmValue(HAPI_ParmInfo& parm, MDataBlock& data)
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
        double before = getTime();
        HAPI_SetParmIntValues(asset->info.id, values, parm.intValuesIndex, size);
        double after = getTime();
        cerr << "type: " << parm.type << " time: " << (after - before) << " int" << endl;
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
        double before = getTime();
        HAPI_SetParmFloatValues(asset->info.id, values, parm.floatValuesIndex, size);
        double after = getTime();
        cerr << "type: " << parm.type << " id: " << parm.id << " time: " << (after - before) << " float" << endl;
    }

    if (parm.isString())
    {
        if (size == 1)
        {
            MDataHandle handle = data.inputValue(plug);
            const char* val = handle.asString().asChar();
            HAPI_SetParmStringValue(asset->info.id, val, parm.id, 0);
        } else
        {
            for (int i=0; i<size; i++)
            {
                MPlug child = plug.child(i);
                MDataHandle handle = data.inputValue(child);
                const char* val = handle.asString().asChar();
                double before = getTime();
                HAPI_SetParmStringValue(asset->info.id, val, parm.id, i);
                double after = getTime();
                cerr << "type: " << parm.type << " time: " << (after - before) << " string" << endl;
            }
        }
    }


}


void
Plugin::setParmValues(MDataBlock& data)
{
    int parmCount = asset->info.parmCount;
    if (parmCount <= 0)
        return;
    HAPI_ParmInfo myParmInfos[parmCount];
    HAPI_GetParameters(asset->info.id, myParmInfos, 0, parmCount);

    double before = getTime();
    for (int i=0; i<parmCount; i++)
    {

        HAPI_ParmInfo& parm = myParmInfos[i];
        setParmValue(parm, data);
    }
    double end = getTime();
    cerr << "time: " << (end - before) << endl;
}


MStatus
Plugin::compute(const MPlug& plug, MDataBlock& data)
{
    // load otl
    // TODO: manage assets properly
    cerr << "assetChanged: " << assetChanged << endl;
    if (assetChanged)
    {
        MPlug p(thisMObject(), fileNameAttr);
        MDataHandle h = data.inputValue(p);
        MString filePath = h.asString();

        asset = new Asset(filePath);

        assetChanged = false;
    }

    //cerr << "check1" << endl;

    if (!builtParms)
    {
        // add ALL the parms
        MObjectArray parmAttributes = asset->getParmAttributes();
        MFnDependencyNode fnDN(thisMObject());
        int size = parmAttributes.length();
        for (int i=0; i<size; i++)
        {
            MFnAttribute attr(parmAttributes[i]);
            cerr << "attr: " << attr.name();
            fnDN.addAttribute(parmAttributes[i]);
        }

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
    int objCount = asset->info.objectCount;
    cerr << "objcount: " << objCount << endl;
    MPlug numObjectsPlug(thisMObject(), numObjects);
    MDataHandle numObjectsHandle = data.outputValue(numObjectsPlug);
    numObjectsHandle.set(objCount);
    data.setClean(numObjectsPlug);

    // objects and transforms
    MPlug outputPlug(thisMObject(), output);
    MPlug meshesPlug(thisMObject(), meshes);
    MPlug transformsPlug(thisMObject(), transforms);

    Object* objects = asset->getObjects();
    for (int i=0; i<objCount; i++)
    {
        MPlug elemPlug = meshesPlug.elementByLogicalIndex(i);
        cerr << "elemPlug: " << elemPlug.name() << endl;
        MDataHandle outHandle = data.outputValue(elemPlug);


        Object& obj = objects[i];
        MObject newMeshData = obj.createMesh();

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
    status = plugin.registerNode("hAsset", Plugin::id, Plugin::creator, Plugin::initialize);
    return status;
}

MStatus
uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);
    status = plugin.deregisterNode(Plugin::id);
    return status;
}
