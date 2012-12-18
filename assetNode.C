#include <maya/MFnNumericAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnUnitAttribute.h>
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
#include <maya/MTime.h>
#include <maya/MGlobal.h>


#include "assetNode.h"
#include "assetNodeMonitor.h"
#include "common.h"
#include "util.h"

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

MTypeId AssetNode::id(0x80000);
MObject AssetNodeAttributes::fileNameAttr;
MObject AssetNodeAttributes::parmsModified;
MObject AssetNodeAttributes::timeInput;

MObject AssetNodeAttributes::assetType;

MObject AssetNodeAttributes::output;
MObject AssetNodeAttributes::objects;

MObject AssetNodeAttributes::objectName;
MObject AssetNodeAttributes::metaData;
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
MObject AssetNodeAttributes::alphaAttr;

MObject AssetNodeAttributes::instancers;
MObject AssetNodeAttributes::instancerData;
MObject AssetNodeAttributes::instancedObjectNames;


void*
AssetNode::creator()
{
    AssetNode* ret = new AssetNode();
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
AssetNode::initialize()
{
    HAPI_StatusCode hstat = HAPI_STATUS_SUCCESS;
    char* dir = "C:/cygwin/home/ken/dev_projects/HAPI/Unity/Assets/OTLs/Scanned";
    //cerr << "hapi initialize" << endl;
    hstat = HAPI_Initialize("C:/cygwin/home/ken/dev_x86/dev/hfs", dir, false, -1);

    //char* dir = "/home/jhuang/dev_projects/HAPI/Maya/assets/otls/";
    //hstat = HAPI_Initialize("/home/jhuang/dev/hfs/", dir, false, -1);
    Util::printHAPIStatus(hstat);

    // maya plugin stuff
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;
    MFnCompoundAttribute cAttr;
    MFnUnitAttribute uAttr;

    // file name
    // The name of the otl file we loaded.
    AssetNodeAttributes::fileNameAttr = tAttr.create("fileName", "fn", MFnData::kString);
    tAttr.setStorable(true);

    // parms modified
    // This is initially false, and whenever a user touches a parm, this will get set to true
    // and because it's a Maya attr, it will get saved.  When we load back the file, 
    // if this attr is true, we know we are loading a previously modified asset as opposed
    // to instantiating a new asset.
    AssetNodeAttributes::parmsModified = nAttr.create("parmsModified", "pm", MFnNumericData::kBoolean, false);
    nAttr.setStorable(true);
    nAttr.setConnectable(false);
    nAttr.setHidden(true);

    // time input
    // For time dpendence.
    AssetNodeAttributes::timeInput = uAttr.create("inTime", "it", MTime());
    uAttr.setStorable(true);
    uAttr.setHidden(true);

    // asset type
    // This maps to the underlying Houdini asset type: OBJ, SOP, etc. (see HAPI_AssetType)
    AssetNodeAttributes::assetType = nAttr.create("assetType", "typ", MFnNumericData::kInt);
    nAttr.setStorable(false);
    nAttr.setWritable(false);    
        
    //----------------------------------  instancer compound multi----------------------------------------------
    // instancer data
    AssetNodeAttributes::instancerData = tAttr.create("instancerData", "idt", MFnData::kDynArrayAttrs);
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    // instanced object names
    AssetNodeAttributes::instancedObjectNames = tAttr.create("instancedObjectNames", "ion", MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    tAttr.setArray(true);
    tAttr.setIndexMatters(true);
    cAttr.setUsesArrayDataBuilder(true);

    // instancers
    AssetNodeAttributes::instancers = cAttr.create("instancers", "inst");
    cAttr.addChild(AssetNodeAttributes::instancerData);
    cAttr.addChild(AssetNodeAttributes::instancedObjectNames);
    cAttr.setStorable(false);
    cAttr.setWritable(false);
    cAttr.setArray(true);
    cAttr.setUsesArrayDataBuilder(true);
    //--------------------------------End instancer compound multi----------------------------------------------

    //----------------------------------  objects compound multi------------------------------------------------

    // object name
    AssetNodeAttributes::objectName = tAttr.create("objectName", "on", MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    // meta data
    AssetNodeAttributes::metaData = tAttr.create("metaData", "md", MFnData::kIntArray);
    tAttr.setStorable(false);
    tAttr.setWritable(false);

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
    // material alpha
    AssetNodeAttributes::alphaAttr = nAttr.createColor("alphaColor", "alp");
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
    cAttr.addChild(AssetNodeAttributes::alphaAttr);
    cAttr.addChild(AssetNodeAttributes::texturePath);
    cAttr.setWritable(false);
    cAttr.setStorable(false);

    AssetNodeAttributes::objects = cAttr.create("objects", "objs");
    cAttr.addChild(AssetNodeAttributes::objectName);
    cAttr.addChild(AssetNodeAttributes::metaData);
    cAttr.addChild(AssetNodeAttributes::mesh);
    cAttr.addChild(AssetNodeAttributes::transform);
    cAttr.addChild(AssetNodeAttributes::material);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    cAttr.setArray(true);
    cAttr.setIndexMatters(true);
    cAttr.setUsesArrayDataBuilder(true);

    //------------------------------- END  objects compound multi------------------------------------------------

    // output
    AssetNodeAttributes::output = cAttr.create("output", "out");    
    cAttr.addChild(AssetNodeAttributes::objects);
    cAttr.addChild(AssetNodeAttributes::instancers);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    
    cAttr.setWritable(false);
    cAttr.setStorable(false);

    // add the static attributes to the node
    addAttribute(AssetNodeAttributes::fileNameAttr);
    addAttribute(AssetNodeAttributes::parmsModified);
    addAttribute(AssetNodeAttributes::timeInput);
    addAttribute(AssetNodeAttributes::assetType);
    addAttribute(AssetNodeAttributes::output);

    
    //most of the dependencies between attrs are set via the AssetNode::setDependentsDirty() call
    //this call may not even be necessary.
    attributeAffects(AssetNodeAttributes::fileNameAttr, AssetNodeAttributes::output);
    

    return MS::kSuccess;
}


AssetNode::AssetNode()
{    
    asset = NULL;

    builtParms = false;
    assetChanged = true;

}


AssetNode::~AssetNode() {
    HAPI_StatusCode hstat = HAPI_STATUS_SUCCESS;
    cerr << "Asset node destroy" << endl;
    try
    {
        if (asset != NULL)
        {
            hstat = HAPI_UnloadOTLFile(asset->info.id);
            Util::checkHAPIStatus(hstat);
        }
    }
    catch (HAPIError& e)
    {
        cerr << e.what() << endl;
    }
    monitor->stop();

    delete asset;
    delete monitor;
}


void
AssetNode::postConstructor()
{
    monitor = new AssetNodeMonitor(thisMObject());
    MStatus stat = monitor->watch();
}


MStatus
AssetNode::setDependentsDirty(const MPlug& plugBeingDirtied,
        MPlugArray& affectedPlugs)
{
    if (plugBeingDirtied == AssetNodeAttributes::fileNameAttr)
        return MS::kSuccess;

    dirtyParmAttribute = plugBeingDirtied.attribute();
    //cerr << "plugBeingDirtied: " << plugBeingDirtied.name();
    //cerr << "name: " << dirtyParmPlug->name();

    //MPlug meshesPlug(thisMObject(), meshes);
    //MPlug transformsPlug(thisMObject(), transforms);
    affectedPlugs.append(MPlug(thisMObject(), AssetNodeAttributes::output));

    MPlug objectsPlug(thisMObject(), AssetNodeAttributes::objects);
    MPlug instancersPlug(thisMObject(), AssetNodeAttributes::instancers);

    for (int i=0; i < objectsPlug.numElements(); i++)
    {
        cerr << "()()()() setDependentsDirty ()()()() " << i << endl;
        MPlug elemPlug = objectsPlug[i];

        MPlug objectNamePlug = elemPlug.child(AssetNodeAttributes::objectName);
        MPlug metaDataPlug = elemPlug.child(AssetNodeAttributes::metaData);
        MPlug meshPlug = elemPlug.child(AssetNodeAttributes::mesh);
        MPlug transformPlug = elemPlug.child(AssetNodeAttributes::transform);
        MPlug materialPlug = elemPlug.child(AssetNodeAttributes::material);


        affectedPlugs.append(objectNamePlug);
        affectedPlugs.append(metaDataPlug);
        affectedPlugs.append(meshPlug);
        //affectedPlugs.append(transformPlug);
        //affectedPlugs.append(materialPlug);

        affectedPlugs.append(transformPlug.child(AssetNodeAttributes::translateAttr));
        affectedPlugs.append(transformPlug.child(AssetNodeAttributes::rotateAttr));
        affectedPlugs.append(transformPlug.child(AssetNodeAttributes::scaleAttr));

        affectedPlugs.append(materialPlug.child(AssetNodeAttributes::materialExists));
        affectedPlugs.append(materialPlug.child(AssetNodeAttributes::texturePath));
        affectedPlugs.append(materialPlug.child(AssetNodeAttributes::ambientAttr));
        affectedPlugs.append(materialPlug.child(AssetNodeAttributes::diffuseAttr));
        affectedPlugs.append(materialPlug.child(AssetNodeAttributes::specularAttr));
    }

    for (int i=0; i<instancersPlug.numElements(); i++)
    {
        MPlug elemPlug = instancersPlug[i];
        MPlug instancerDataPlug = elemPlug.child(AssetNodeAttributes::instancerData);
        MPlug instancedObjectNamesPlug = elemPlug.child(AssetNodeAttributes::instancedObjectNames);

        affectedPlugs.append(instancerDataPlug);

        for (int j=0; j<instancedObjectNamesPlug.numElements(); j++)
        {
            affectedPlugs.append(instancedObjectNamesPlug[j]);
        }

    }
    return MS::kSuccess;
}


MObject
AssetNode::getAttrFromParm(HAPI_ParmInfo& parm)
{
    MFnDependencyNode fnDN(thisMObject());

    MString name = MString("_parm") + parm.id + "_";
    MObject attr = fnDN.attribute(name);
    return attr;
}


void
AssetNode::updateAttrValue(HAPI_ParmInfo& parm, MDataBlock& data)
{
    //cerr << "update attr value ================" << endl;

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
AssetNode::updateAttrValues(MDataBlock& data)
{
    int parmCount = asset->info.parmCount;
    if (parmCount <= 0)
        return;
    HAPI_ParmInfo * myParmInfos = new HAPI_ParmInfo[parmCount];
    HAPI_GetParameters(asset->info.id, myParmInfos, 0, parmCount);

    for (int i=0; i<parmCount; i++)
    {
        HAPI_ParmInfo& parm = myParmInfos[i];
        updateAttrValue(parm, data);
    }

    // mark parms as modified, so that if scene is saved we know to
    // use the maya attributes to set parm values in Houdini
    MPlug p(thisMObject(), AssetNodeAttributes::parmsModified);
    MDataHandle h = data.outputValue(p);
    h.set(true);

    delete[] myParmInfos;
}


double
getTime()
{
    //Todo: this implementation below is platform specific to Linux.  Re-implement it for
    // a cross platform solution.
    /*
        timeval  tv;
        gettimeofday(&tv, NULL);
        double time_in_mill =
                     (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to millisecond
        return time_in_mill;*/
    return 0.0;
}

void
AssetNode::setParmValue(HAPI_ParmInfo& parm, MDataBlock& data)
{
    //cerr << "set parm value ===========" << endl;

    MObject attr = getAttrFromParm(parm);
    MPlug plug(thisMObject(), attr);
    MPlug dirtyParmPlug(thisMObject(), dirtyParmAttribute);
    //cerr << "plug: " << plug.name() << endl;
    //cerr << "dirtyParmPlug: " << dirtyParmPlug.name() << endl;
    if (!dirtyParmPlug.isNull())
        if (plug != dirtyParmPlug)
            return;

    int size = parm.size;

    if (parm.isInt())
    {
        int * values = new int[size];
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
        //double before = getTime();
        HAPI_SetParmIntValues(asset->info.id, values, parm.intValuesIndex, size);
        //double after = getTime();

        //cerr << "type: " << parm.type << " time: " << (after - before) << " int" << endl;
        delete[] values;
    }

    if (parm.isFloat())
    {
        float * values = new float[size];
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
        //double before = getTime();
        HAPI_SetParmFloatValues(asset->info.id, values, parm.floatValuesIndex, size);
        //double after = getTime();

        //cerr << "type: " << parm.type << " id: " << parm.id << " time: " << (after - before) << " float" << endl;
        delete[] values;
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

    //dirtyParmAttribute = MObject();

}


void
AssetNode::setParmValues(MDataBlock& data)
{
    int parmCount = asset->info.parmCount;
    if (parmCount <= 0)
        return;
    HAPI_ParmInfo * myParmInfos = new HAPI_ParmInfo[parmCount];
    HAPI_GetParameters(asset->info.id, myParmInfos, 0, parmCount);

    for (int i=0; i<parmCount; i++)
    {

        HAPI_ParmInfo& parm = myParmInfos[i];
        setParmValue(parm, data);
    }

    delete[] myParmInfos;
}


MStatus
AssetNode::compute(const MPlug& plug, MDataBlock& data)
{
    cerr << "compute #################################### " << plug.name() << endl;

    //MPlug inputPlug(thisMObject(), AssetNodeAttributes::input);
    //MPlug elemInputPlug = inputPlug.child(0);
    //cerr << "is input1 connected: " << elemInputPlug.isConnected() << endl;


    // load otl
    // TODO: manage assets properly
    if (assetChanged)
    {
        MPlug p(thisMObject(), AssetNodeAttributes::fileNameAttr);
        MDataHandle h = data.inputValue(p);
        MString filePath = h.asString();

        try
        {
            asset = new Asset(filePath, thisMObject());
            assetChanged = false;
        }
        catch (HAPIError& e)
        {
            cerr << e.what() << endl;
            return MS::kFailure;
        }
    }

    //cerr << "check1" << endl;

    if (!builtParms)
    {
        // add ALL the parms
        cerr << "add parms ...." << endl;
        MObjectArray parmAttributes = asset->getParmAttributes();
        MFnDependencyNode fnDN(thisMObject());
        int size = parmAttributes.length();
        for (int i=0; i<size; i++)
        {
            MFnAttribute attr(parmAttributes[i]);
            fnDN.addAttribute(parmAttributes[i]);
        }

        builtParms = true;
    }
    //{
        //setParmValues(data);
    //}


    MPlug parmsModifiedPlug(thisMObject(), AssetNodeAttributes::parmsModified);
    MDataHandle parmsModifiedHandle = data.inputValue(parmsModifiedPlug);
    if (parmsModifiedHandle.asBool())
        setParmValues(data);
    updateAttrValues(data);

    MPlug outputPlug(thisMObject(), AssetNodeAttributes::output);
    asset->compute(outputPlug, data);

    cerr << "end compute #################################### " << plug.name() << endl;

    return MS::kSuccess;
}

//MStatus
//initializePlugin(MObject obj)
//{
    //MStatus status;
    //MFnPlugin plugin(obj, "Asset plugin", "1.0", "Any");
    //status = plugin.registerNode("hAsset", AssetNode::id, AssetNode::creator, AssetNode::initialize);
    //return status;
//}

//MStatus
//uninitializePlugin(MObject obj)
//{
    //MStatus status;
    //MFnPlugin plugin(obj);
    //status = plugin.deregisterNode(AssetNode::id);
    //return status;
//}
