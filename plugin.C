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
#include <maya/MFnArrayAttrsData.h>
#include <maya/MGlobal.h>

#include <sys/time.h>

#include "plugin.h"
#include "common.h"

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
MObject AssetNodeAttributes::fileNameAttr;
MObject AssetNodeAttributes::output;

MObject AssetNodeAttributes::mesh;

MObject AssetNodeAttributes::transform;
MObject AssetNodeAttributes::translateAttr;
MObject AssetNodeAttributes::translateAttrX;
MObject AssetNodeAttributes::translateAttrY;
MObject AssetNodeAttributes::translateAttrZ;
MObject AssetNodeAttributes::rotateAttr;
MObject AssetNodeAttributes::rotateAttrX;
MObject AssetNodeAttributes::rotateAttrY;
MObject AssetNodeAttributes::rotateAttrZ;
MObject AssetNodeAttributes::scaleAttr;
MObject AssetNodeAttributes::scaleAttrX;
MObject AssetNodeAttributes::scaleAttrY;
MObject AssetNodeAttributes::scaleAttrZ;

MObject AssetNodeAttributes::material;
MObject AssetNodeAttributes::materialExists;
MObject AssetNodeAttributes::texturePath;
MObject AssetNodeAttributes::ambientAttr;
MObject AssetNodeAttributes::diffuseAttr;
MObject AssetNodeAttributes::specularAttr;

MObject AssetNodeAttributes::numObjects;

MObject AssetNodeAttributes::instancerData;

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

    AssetNodeAttributes::fileNameAttr = tAttr.create("fileName", "fn", MFnData::kString);
    tAttr.setStorable(true);

    // numObjects
    AssetNodeAttributes::numObjects = nAttr.create("numberOfObjects", "no", MFnNumericData::kInt);
    nAttr.setWritable(false);
    nAttr.setStorable(false);
    nAttr.setConnectable(false);
    nAttr.setHidden(true);

    // mesh
    AssetNodeAttributes::mesh = tAttr.create("mesh", "ms", MFnData::kMesh);
    tAttr.setWritable(false);
    tAttr.setStorable(false);

    // translate
    AssetNodeAttributes::translateAttrX = uAttr.create("translateX", "tx", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNodeAttributes::translateAttrY = uAttr.create("translateY", "ty", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNodeAttributes::translateAttrZ = uAttr.create("translateZ", "tz", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNodeAttributes::translateAttr = nAttr.create("translate", "t", AssetNodeAttributes::translateAttrX,
            AssetNodeAttributes::translateAttrY, AssetNodeAttributes::translateAttrZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // rotate
    AssetNodeAttributes::rotateAttrX = uAttr.create("rotateX", "rx", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNodeAttributes::rotateAttrY = uAttr.create("rotateY", "ry", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNodeAttributes::rotateAttrZ = uAttr.create("rotateZ", "rz", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNodeAttributes::rotateAttr = nAttr.create("rotate", "r", AssetNodeAttributes::rotateAttrX,
            AssetNodeAttributes::rotateAttrY, AssetNodeAttributes::rotateAttrZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // scale
    AssetNodeAttributes::scaleAttrX = nAttr.create("scaleX", "sx", MFnNumericData::kDouble, 1.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNodeAttributes::scaleAttrY = nAttr.create("scaleY", "sy", MFnNumericData::kDouble, 1.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNodeAttributes::scaleAttrZ = nAttr.create("scaleZ", "sz", MFnNumericData::kDouble, 1.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNodeAttributes::scaleAttr = nAttr.create("scale", "s", AssetNodeAttributes::scaleAttrX,
            AssetNodeAttributes::scaleAttrY, AssetNodeAttributes::scaleAttrZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // transform
    AssetNodeAttributes::transform = cAttr.create("transform", "xfs");
    cAttr.addChild(AssetNodeAttributes::translateAttr);
    cAttr.addChild(AssetNodeAttributes::rotateAttr);
    cAttr.addChild(AssetNodeAttributes::scaleAttr);
    cAttr.setWritable(false);
    cAttr.setStorable(false);

    // material exists
    AssetNodeAttributes::materialExists = nAttr.create("materialExists", "me", MFnNumericData::kBoolean, false);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    nAttr.setConnectable(false);
    nAttr.setHidden(true);
    // material ambient
    AssetNodeAttributes::ambientAttr = nAttr.createColor("ambientColor", "amb");
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    // material diffuse
    AssetNodeAttributes::diffuseAttr = nAttr.createColor("diffuseColor", "dif");
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    // material specular
    AssetNodeAttributes::specularAttr = nAttr.createColor("specularColor", "spe");
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    // texture path
    AssetNodeAttributes::texturePath = tAttr.create("texturePath", "tp", MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    // material
    AssetNodeAttributes::material = cAttr.create("material", "mats");
    cAttr.addChild(AssetNodeAttributes::materialExists);
    cAttr.addChild(AssetNodeAttributes::ambientAttr);
    cAttr.addChild(AssetNodeAttributes::diffuseAttr);
    cAttr.addChild(AssetNodeAttributes::specularAttr);
    cAttr.addChild(AssetNodeAttributes::texturePath);
    cAttr.setWritable(false);
    cAttr.setStorable(false);

    // output
    AssetNodeAttributes::output = cAttr.create("output", "out");
    cAttr.addChild(AssetNodeAttributes::mesh);
    cAttr.addChild(AssetNodeAttributes::transform);
    cAttr.addChild(AssetNodeAttributes::material);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    cAttr.setArray(true);
    cAttr.setIndexMatters(true);

    // instancer data
    AssetNodeAttributes::instancerData = tAttr.create("instancerData", "idt", MFnData::kDynArrayAttrs);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    tAttr.setArray(true);

    addAttribute(AssetNodeAttributes::fileNameAttr);
    addAttribute(AssetNodeAttributes::numObjects);
    addAttribute(AssetNodeAttributes::output);
    addAttribute(AssetNodeAttributes::instancerData);

    attributeAffects(AssetNodeAttributes::fileNameAttr, AssetNodeAttributes::numObjects);
    attributeAffects(AssetNodeAttributes::fileNameAttr, AssetNodeAttributes::output);
    attributeAffects(AssetNodeAttributes::fileNameAttr, AssetNodeAttributes::instancerData);

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
    if (plugBeingDirtied == AssetNodeAttributes::fileNameAttr)
        return MS::kSuccess;

    dirtyParmAttribute = plugBeingDirtied.attribute();
    //cerr << "plugBeingDirtied: " << plugBeingDirtied.name();
    //cerr << "name: " << dirtyParmPlug->name();

    //MPlug meshesPlug(thisMObject(), meshes);
    //MPlug transformsPlug(thisMObject(), transforms);
    MPlug outputPlug(thisMObject(), AssetNodeAttributes::output);
    for (int i=0; i < asset->info.objectCount; i++)
    {
        MPlug elemPlug = outputPlug.elementByLogicalIndex(i);

        MPlug meshPlug = elemPlug.child(AssetNodeAttributes::mesh);
        MPlug transformPlug = elemPlug.child(AssetNodeAttributes::transform);
        MPlug materialPlug = elemPlug.child(AssetNodeAttributes::material);

        affectedPlugs.append(meshPlug);
        affectedPlugs.append(transformPlug);

        affectedPlugs.append(transformPlug.child(AssetNodeAttributes::translateAttr));
        affectedPlugs.append(transformPlug.child(AssetNodeAttributes::rotateAttr));
        affectedPlugs.append(transformPlug.child(AssetNodeAttributes::scaleAttr));

        affectedPlugs.append(transformPlug.child(AssetNodeAttributes::materialExists));
        affectedPlugs.append(transformPlug.child(AssetNodeAttributes::texturePath));
        cerr << "set dirty: tex------------------" << endl;
        affectedPlugs.append(transformPlug.child(AssetNodeAttributes::ambientAttr));
        affectedPlugs.append(transformPlug.child(AssetNodeAttributes::diffuseAttr));
        affectedPlugs.append(transformPlug.child(AssetNodeAttributes::specularAttr));
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
    MPlug dirtyParmPlug(thisMObject(), dirtyParmAttribute);
    //cerr << "plug: " << plug.name() << endl;
    //cerr << "dirtyParmPlug: " << dirtyParmPlug.name() << endl;
    if (plug != dirtyParmPlug)
        return;

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
        //cerr << "type: " << parm.type << " time: " << (after - before) << " int" << endl;
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
        //cerr << "type: " << parm.type << " id: " << parm.id << " time: " << (after - before) << " float" << endl;
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
                //cerr << "type: " << parm.type << " time: " << (after - before) << " string" << endl;
            }
        }
    }

    dirtyParmAttribute = MObject();

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
    if (assetChanged)
    {
        MPlug p(thisMObject(), AssetNodeAttributes::fileNameAttr);
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
    int numVisibleObjects = asset->numVisibleObjects;
    //int objCount = asset->info.objectCount;
    //cerr << "objcount: " << objCount << endl;
    MPlug numObjectsPlug(thisMObject(), AssetNodeAttributes::numObjects);
    MDataHandle numObjectsHandle = data.outputValue(numObjectsPlug);
    numObjectsHandle.set(numVisibleObjects);
    data.setClean(numObjectsPlug);

    // objects and transforms
    MPlug outputPlug(thisMObject(), AssetNodeAttributes::output);
    MPlug instancersPlug(thisMObject(), AssetNodeAttributes::instancerData);
    //MPlug meshesPlug(thisMObject(), meshes);
    //MPlug transformsPlug(thisMObject(), transforms);

    Object* objects = asset->getVisibleObjects();
    for (int i=0; i<numVisibleObjects; i++)
    {

        Object& obj = objects[i];
        obj.compute(i, outputPlug, data);

    }

    // set everything clean
    data.setClean(numObjectsPlug);
    for (int i=0; i<numVisibleObjects; i++)
    {
        MPlug elemPlug = outputPlug.elementByLogicalIndex(i);
        MPlug meshPlug = elemPlug.child(AssetNodeAttributes::mesh);
        MPlug transformPlug = elemPlug.child(AssetNodeAttributes::transform);
        MPlug materialPlug = elemPlug.child(AssetNodeAttributes::material);
        data.setClean(elemPlug);
        data.setClean(meshPlug);
        data.setClean(transformPlug);
    }

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
