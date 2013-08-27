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

#include "AssetNode.h"
#include "AssetNodeMonitor.h"
#include "MayaTypeID.h"
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

MTypeId AssetNode::id(MayaTypeID_HoudiniAssetNode);
MObject AssetNode::fileNameAttr;
MObject AssetNode::parmsModified;
MObject AssetNode::timeInput;

MObject AssetNode::assetType;

MObject AssetNode::output;
MObject AssetNode::objects;

MObject AssetNode::objectName;
MObject AssetNode::metaData;
MObject AssetNode::mesh;

MObject AssetNode::transform;
MObject AssetNode::translateAttr;
MObject AssetNode::translateAttrX;
MObject AssetNode::translateAttrY;
MObject AssetNode::translateAttrZ;
MObject AssetNode::rotateAttr;
MObject AssetNode::rotateAttrX;
MObject AssetNode::rotateAttrY;
MObject AssetNode::rotateAttrZ;
MObject AssetNode::scaleAttr;
MObject AssetNode::scaleAttrX;
MObject AssetNode::scaleAttrY;
MObject AssetNode::scaleAttrZ;

MObject AssetNode::material;
MObject AssetNode::materialExists;
MObject AssetNode::texturePath;
MObject AssetNode::ambientAttr;
MObject AssetNode::diffuseAttr;
MObject AssetNode::specularAttr;
MObject AssetNode::alphaAttr;

MObject AssetNode::instancers;
MObject AssetNode::instancerData;
MObject AssetNode::instancedObjectNames;


void*
AssetNode::creator()
{
    AssetNode* ret = new AssetNode();
    return ret;
}

MStatus
AssetNode::initialize()
{
    // maya plugin stuff
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;
    MFnCompoundAttribute cAttr;
    MFnUnitAttribute uAttr;

    // file name
    // The name of the otl file we loaded.
    AssetNode::fileNameAttr = tAttr.create("fileName", "fn", MFnData::kString);
    tAttr.setStorable(true);

    // parms modified
    // This is initially false, and whenever a user touches a parm, this will get set to true
    // and because it's a Maya attr, it will get saved.  When we load back the file, 
    // if this attr is true, we know we are loading a previously modified asset as opposed
    // to instantiating a new asset.
    AssetNode::parmsModified = nAttr.create("parmsModified", "pm", MFnNumericData::kBoolean, false);
    nAttr.setStorable(true);
    nAttr.setConnectable(false);
    nAttr.setHidden(true);

    // time input
    // For time dpendence.
    AssetNode::timeInput = uAttr.create("inTime", "it", MTime());
    uAttr.setStorable(true);
    uAttr.setHidden(true);

    // asset type
    // This maps to the underlying Houdini asset type: OBJ, SOP, etc. (see HAPI_AssetType)
    AssetNode::assetType = nAttr.create("assetType", "typ", MFnNumericData::kInt);
    nAttr.setStorable(false);
    nAttr.setWritable(false);    
        
    //----------------------------------  instancer compound multi----------------------------------------------
    // instancer data
    AssetNode::instancerData = tAttr.create("instancerData", "idt", MFnData::kDynArrayAttrs);
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    // instanced object names
    AssetNode::instancedObjectNames = tAttr.create("instancedObjectNames", "ion", MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    tAttr.setArray(true);
    tAttr.setIndexMatters(true);
    cAttr.setUsesArrayDataBuilder(true);

    // instancers
    AssetNode::instancers = cAttr.create("instancers", "inst");
    cAttr.addChild(AssetNode::instancerData);
    cAttr.addChild(AssetNode::instancedObjectNames);
    cAttr.setStorable(false);
    cAttr.setWritable(false);
    cAttr.setArray(true);
    cAttr.setUsesArrayDataBuilder(true);
    //--------------------------------End instancer compound multi----------------------------------------------

    //----------------------------------  objects compound multi------------------------------------------------

    // object name
    AssetNode::objectName = tAttr.create("objectName", "on", MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    // meta data
    AssetNode::metaData = tAttr.create("metaData", "md", MFnData::kIntArray);
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    // mesh
    AssetNode::mesh = tAttr.create("mesh", "ms", MFnData::kMesh);
    tAttr.setWritable(false);
    tAttr.setStorable(false);

    // translate
    AssetNode::translateAttrX = uAttr.create("translateX", "tx", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::translateAttrY = uAttr.create("translateY", "ty", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::translateAttrZ = uAttr.create("translateZ", "tz", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::translateAttr = nAttr.create("translate", "t", AssetNode::translateAttrX,
            AssetNode::translateAttrY, AssetNode::translateAttrZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // rotate
    AssetNode::rotateAttrX = uAttr.create("rotateX", "rx", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::rotateAttrY = uAttr.create("rotateY", "ry", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::rotateAttrZ = uAttr.create("rotateZ", "rz", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::rotateAttr = nAttr.create("rotate", "r", AssetNode::rotateAttrX,
            AssetNode::rotateAttrY, AssetNode::rotateAttrZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // scale
    AssetNode::scaleAttrX = nAttr.create("scaleX", "sx", MFnNumericData::kDouble, 1.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::scaleAttrY = nAttr.create("scaleY", "sy", MFnNumericData::kDouble, 1.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::scaleAttrZ = nAttr.create("scaleZ", "sz", MFnNumericData::kDouble, 1.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::scaleAttr = nAttr.create("scale", "s", AssetNode::scaleAttrX,
            AssetNode::scaleAttrY, AssetNode::scaleAttrZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // transform
    AssetNode::transform = cAttr.create("transform", "xfs");
    cAttr.addChild(AssetNode::translateAttr);
    cAttr.addChild(AssetNode::rotateAttr);
    cAttr.addChild(AssetNode::scaleAttr);
    cAttr.setWritable(false);
    cAttr.setStorable(false);

    // material exists
    AssetNode::materialExists = nAttr.create("materialExists", "me", MFnNumericData::kBoolean, false);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    nAttr.setConnectable(false);
    nAttr.setHidden(true);
    // material ambient
    AssetNode::ambientAttr = nAttr.createColor("ambientColor", "amb");
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    // material diffuse
    AssetNode::diffuseAttr = nAttr.createColor("diffuseColor", "dif");
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    // material specular
    AssetNode::specularAttr = nAttr.createColor("specularColor", "spe");
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    // material alpha
    AssetNode::alphaAttr = nAttr.createColor("alphaColor", "alp");
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    // texture path
    AssetNode::texturePath = tAttr.create("texturePath", "tp", MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    // material
    AssetNode::material = cAttr.create("material", "mats");
    cAttr.addChild(AssetNode::materialExists);
    cAttr.addChild(AssetNode::ambientAttr);
    cAttr.addChild(AssetNode::diffuseAttr);
    cAttr.addChild(AssetNode::specularAttr);
    cAttr.addChild(AssetNode::alphaAttr);
    cAttr.addChild(AssetNode::texturePath);
    cAttr.setWritable(false);
    cAttr.setStorable(false);

    AssetNode::objects = cAttr.create("objects", "objs");
    cAttr.addChild(AssetNode::objectName);
    cAttr.addChild(AssetNode::metaData);
    cAttr.addChild(AssetNode::mesh);
    cAttr.addChild(AssetNode::transform);
    cAttr.addChild(AssetNode::material);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    cAttr.setArray(true);
    cAttr.setIndexMatters(true);
    cAttr.setUsesArrayDataBuilder(true);

    //------------------------------- END  objects compound multi------------------------------------------------

    // output
    AssetNode::output = cAttr.create("output", "out");    
    cAttr.addChild(AssetNode::objects);
    cAttr.addChild(AssetNode::instancers);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    
    cAttr.setWritable(false);
    cAttr.setStorable(false);

    // add the static attributes to the node
    addAttribute(AssetNode::fileNameAttr);
    addAttribute(AssetNode::parmsModified);
    addAttribute(AssetNode::timeInput);
    addAttribute(AssetNode::assetType);
    addAttribute(AssetNode::output);

    
    //most of the dependencies between attrs are set via the AssetNode::setDependentsDirty() call
    //this call may not even be necessary.
    attributeAffects(AssetNode::fileNameAttr, AssetNode::output);
    

    return MS::kSuccess;
}


AssetNode::AssetNode()
{    
    myAsset = NULL;

    myBuiltParms = false;
    myAssetChanged = true;

}


AssetNode::~AssetNode() {
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;
    try
    {
        if ( myAsset != NULL )
        {
            hstat = HAPI_UnloadOTLFile( myAsset->assetInfo.id);
            Util::checkHAPIStatus(hstat);
        }
    }
    catch (HAPIError& e)
    {
        cerr << e.what() << endl;
    }
    //monitor->stop();

    delete myAsset;
    //delete monitor;
}


void
AssetNode::postConstructor()
{
    //monitor = new AssetNodeMonitor(thisMObject());
    //MStatus stat = monitor->watch();
}


MStatus
AssetNode::setDependentsDirty(const MPlug& plugBeingDirtied,
        MPlugArray& affectedPlugs)
{
    if (plugBeingDirtied == AssetNode::fileNameAttr)
        return MS::kSuccess;

    myDirtyParmAttribute = plugBeingDirtied.attribute();
    
    affectedPlugs.append(MPlug(thisMObject(), AssetNode::output));

    MPlug objectsPlug(thisMObject(), AssetNode::objects);
    MPlug instancersPlug(thisMObject(), AssetNode::instancers);

    for ( unsigned int i = 0; i < objectsPlug.numElements(); ++i )
    {
        MPlug elemPlug = objectsPlug[ i ];

        MPlug objectNamePlug = elemPlug.child(AssetNode::objectName);
        MPlug metaDataPlug = elemPlug.child(AssetNode::metaData);
        MPlug meshPlug = elemPlug.child(AssetNode::mesh);
        MPlug transformPlug = elemPlug.child(AssetNode::transform);
        MPlug materialPlug = elemPlug.child(AssetNode::material);


        affectedPlugs.append(objectNamePlug);
        affectedPlugs.append(metaDataPlug);
        affectedPlugs.append(meshPlug);

        affectedPlugs.append(transformPlug.child(AssetNode::translateAttr));
        affectedPlugs.append(transformPlug.child(AssetNode::rotateAttr));
        affectedPlugs.append(transformPlug.child(AssetNode::scaleAttr));

        affectedPlugs.append(materialPlug.child(AssetNode::materialExists));
        affectedPlugs.append(materialPlug.child(AssetNode::texturePath));
        affectedPlugs.append(materialPlug.child(AssetNode::ambientAttr));
        affectedPlugs.append(materialPlug.child(AssetNode::diffuseAttr));
        affectedPlugs.append(materialPlug.child(AssetNode::specularAttr));
	affectedPlugs.append(materialPlug.child(AssetNode::alphaAttr));
    }

    for ( unsigned int i = 0; i < instancersPlug.numElements(); ++i )
    {
	MPlug elemPlug = instancersPlug[ i ];
	MPlug instancerDataPlug = elemPlug.child( AssetNode::instancerData );
	MPlug instancedObjectNamesPlug = elemPlug.child( AssetNode::instancedObjectNames );

	affectedPlugs.append( instancerDataPlug );

	for ( unsigned int j = 0; j < instancedObjectNamesPlug.numElements(); ++j )
	    affectedPlugs.append( instancedObjectNamesPlug[ j ] );
    }
    return MS::kSuccess;
}


MObject
AssetNode::getAttrFromParm(HAPI_ParmInfo& parm)
{
    MFnDependencyNode fnDN(thisMObject());
    MString name = Util::getAttrNameFromParm(parm);
    MObject attr = fnDN.attribute(name);
    return attr;
}


void
AssetNode::updateAttrValue(HAPI_ParmInfo& parm, MDataBlock& data)
{    
    // get attribute
    MObject attr = getAttrFromParm(parm);

    // create plug to the attribute
    MPlug plug(thisMObject(), attr);

    int size = parm.size;
    if(parm.isInt())
    {
        MIntArray values = myAsset->getParmIntValues(parm);
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
        MFloatArray values = myAsset->getParmFloatValues(parm);
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
        MStringArray values = myAsset->getParmStringValues(parm);
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


// This function will update Maya attrs based on Houdini Parms
void
AssetNode::updateAttrValues(MDataBlock& data)
{
    int parmCount = myAsset->nodeInfo.parmCount;
    if (parmCount <= 0)
        return;
    HAPI_ParmInfo * parmInfos = new HAPI_ParmInfo[parmCount];
    HAPI_GetParameters(myAsset->nodeInfo.id, parmInfos, 0, parmCount);

    for (int i=0; i<parmCount; i++)
    {
        HAPI_ParmInfo& parm = parmInfos[i];
        updateAttrValue(parm, data);
    }

    // mark parms as modified, so that if scene is saved we know to
    // use the maya attributes to set parm values in Houdini
    MPlug p(thisMObject(), AssetNode::parmsModified);
    MDataHandle h = data.outputValue(p);
    h.set(true);

    delete[] parmInfos;
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


// This function takes Maya attr values and pushes it into Houdini
void
AssetNode::setParmValue(HAPI_ParmInfo& parm, MDataBlock& data)
{    

    MObject attr = getAttrFromParm(parm);
    MPlug plug(thisMObject(), attr);
    MPlug dirtyParmPlug(thisMObject(), myDirtyParmAttribute);

    //Only push into Houdini the minimum changes necessary.
    //Only push what has been dirtied.


    bool found = false;
    if (!dirtyParmPlug.isNull())
    {
        // if the dirtied plug matches the parm, then we have a 1-tuple
        // (non-compound Maya attribute)
        if (plug == dirtyParmPlug)
        {
            found = true;
        }


        // if the parm is a type we know how to handle and the dirtied plug
        // is a child of it, then the parm should be a tuple attribute, so we
        // need to set the whole tuple
        if (parm.isInt() || parm.isFloat() || parm.isString())
        {
            if (dirtyParmPlug.isChild() && dirtyParmPlug.parent() == plug)
                found = true;
        }
    }

    // if the parm and the dirtied plug did not match, do nothing
    if (!found)
        return;


	// this is the tuple size
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
        HAPI_SetParmIntValues( myAsset->nodeInfo.id, values, parm.intValuesIndex, size );
        //double after = getTime();

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
        HAPI_SetParmFloatValues( myAsset->nodeInfo.id, values, parm.floatValuesIndex, size);
        //double after = getTime();

        delete[] values;
    }

    if (parm.isString())
    {
        if (size == 1)
        {
            MDataHandle handle = data.inputValue(plug);
            const char* val = handle.asString().asChar();
            HAPI_SetParmStringValue( myAsset->nodeInfo.id, val, parm.id, 0);
        } else
        {
            for (int i=0; i<size; i++)
            {
                MPlug child = plug.child(i);
                MDataHandle handle = data.inputValue(child);
                const char* val = handle.asString().asChar();
                double before = getTime();
                HAPI_SetParmStringValue( myAsset->nodeInfo.id, val, parm.id, i);
                double after = getTime();
            }
        }
    }

}

// This function takes Maya attr values and pushes it into Houdini
void
AssetNode::setParmValues(MDataBlock& data)
{
    int parmCount = myAsset->nodeInfo.parmCount;
    if (parmCount <= 0)
        return;
    HAPI_ParmInfo * parmInfos = new HAPI_ParmInfo[parmCount];
    HAPI_GetParameters(myAsset->nodeInfo.id, parmInfos, 0, parmCount );

    for (int i=0; i<parmCount; i++)
    {

        HAPI_ParmInfo& parm = parmInfos[i];
        setParmValue(parm, data);
    }

    delete[] parmInfos;
}


MStatus
AssetNode::compute(const MPlug& plug, MDataBlock& data)
{
    // load otl
    if ( myAssetChanged )
    {
        MPlug p(thisMObject(), AssetNode::fileNameAttr);
        MDataHandle h = data.inputValue(p);
        MString filePath = h.asString();

        try
        {
            myAsset = new Asset(filePath, thisMObject());
            myAssetChanged = false;
        }
        catch (HAPIError& e)
        {
            cerr << e.what() << endl;
            return MS::kFailure;
        }
    }    

    if (!myBuiltParms)
    {
        // add ALL the parms

	//These are dynamic input attributes.  These represent
	// the parms of the asset, which we only know after we have
	// loaded the asset.
        MObjectArray parmAttributes = myAsset->getParmAttributes();
        MFnDependencyNode fnDN(thisMObject());
        int size = parmAttributes.length();
        for (int i=0; i<size; i++)
        {
            MFnAttribute attr(parmAttributes[i]);
            fnDN.addAttribute(parmAttributes[i]);
        }

        myBuiltParms = true;

    }

    //check if the user has manipulated this node, if so, then push modified
    //parms into Houdini
    MPlug parmsModifiedPlug(thisMObject(), AssetNode::parmsModified);
    MDataHandle parmsModifiedHandle = data.inputValue(parmsModifiedPlug);
    if (parmsModifiedHandle.asBool())
        setParmValues(data);

    //updates Maya attrs from Houdini
    updateAttrValues(data);

    MPlug outputPlug(thisMObject(), AssetNode::output);
    myAsset->compute(outputPlug, data);

    return MS::kSuccess;
}

