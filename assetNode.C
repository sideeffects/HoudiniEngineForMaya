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

MStatus
AssetNode::initialize()
{
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;
    
    MString otl_dir(getenv("HAPI_OTL_PATH"));    
    MString dso_dir(getenv("HAPI_DSO_PATH"));    

    // Need to use Houdini's OTL's as well as the plugin-specific ones.
    otl_dir = "@/otls_^;@/otls;" + otl_dir;
    dso_dir = "@/dso_^;@/dso;" + dso_dir;

    MString hfs(getenv("HAPI_PATH"));
    if (hfs == "")
    {
        cerr << "*Error*: HAPI_PATH not found" << endl;
        throw HAPIError("HFS directory not found");
    }
    hfs += "/";
    cerr << "hfs: " << hfs.asChar() << endl;
    hstat = HAPI_Initialize(hfs.asChar(), otl_dir.asChar(), 
    	dso_dir.asChar(), true, -1);

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
    myAsset = NULL;

    myBuiltParms = false;
    myAssetChanged = true;

}


AssetNode::~AssetNode() {
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;
    cerr << "Asset node destroy" << endl;
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
    if (plugBeingDirtied == AssetNodeAttributes::fileNameAttr)
        return MS::kSuccess;

    myDirtyParmAttribute = plugBeingDirtied.attribute();
    //cerr << "plugBeingDirtied: " << plugBeingDirtied.name();
    //cerr << "name: " << dirtyParmPlug->name();
    
    affectedPlugs.append(MPlug(thisMObject(), AssetNodeAttributes::output));

    MPlug objectsPlug(thisMObject(), AssetNodeAttributes::objects);
    MPlug instancersPlug(thisMObject(), AssetNodeAttributes::instancers);

    for ( unsigned int i = 0; i < objectsPlug.numElements(); ++i )
    {
        MPlug elemPlug = objectsPlug[ i ];

        MPlug objectNamePlug = elemPlug.child(AssetNodeAttributes::objectName);
        MPlug metaDataPlug = elemPlug.child(AssetNodeAttributes::metaData);
        MPlug meshPlug = elemPlug.child(AssetNodeAttributes::mesh);
        MPlug transformPlug = elemPlug.child(AssetNodeAttributes::transform);
        MPlug materialPlug = elemPlug.child(AssetNodeAttributes::material);


        affectedPlugs.append(objectNamePlug);
        affectedPlugs.append(metaDataPlug);
        affectedPlugs.append(meshPlug);

        affectedPlugs.append(transformPlug.child(AssetNodeAttributes::translateAttr));
        affectedPlugs.append(transformPlug.child(AssetNodeAttributes::rotateAttr));
        affectedPlugs.append(transformPlug.child(AssetNodeAttributes::scaleAttr));

        affectedPlugs.append(materialPlug.child(AssetNodeAttributes::materialExists));
        affectedPlugs.append(materialPlug.child(AssetNodeAttributes::texturePath));
        affectedPlugs.append(materialPlug.child(AssetNodeAttributes::ambientAttr));
        affectedPlugs.append(materialPlug.child(AssetNodeAttributes::diffuseAttr));
        affectedPlugs.append(materialPlug.child(AssetNodeAttributes::specularAttr));
	affectedPlugs.append(materialPlug.child(AssetNodeAttributes::alphaAttr));
    }

    for ( unsigned int i = 0; i < instancersPlug.numElements(); ++i )
    {
	MPlug elemPlug = instancersPlug[ i ];
	MPlug instancerDataPlug = elemPlug.child( AssetNodeAttributes::instancerData );
	MPlug instancedObjectNamesPlug = elemPlug.child( AssetNodeAttributes::instancedObjectNames );

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

    MString tmp = Util::getString(parm.nameSH);
    MString parmName = Util::replaceChar(tmp, ' ', '_');
    MString name = MString("_parm") + parm.id + "_" + parmName + "_";
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
    MPlug p(thisMObject(), AssetNodeAttributes::parmsModified);
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

    //cerr << "plug: " << plug.name() << endl;
    //cerr << "dirtyParmPlug: " << dirtyParmPlug.name() << endl;

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
        HAPI_SetParmFloatValues( myAsset->nodeInfo.id, values, parm.floatValuesIndex, size);
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
                //cerr << "type: " << parm.type << " time: " << (after - before) << " string" << endl;
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
    cerr << "compute #################################### " << plug.name() << endl;

    // load otl
    if ( myAssetChanged )
    {
        MPlug p(thisMObject(), AssetNodeAttributes::fileNameAttr);
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
        cerr << "add parms ...." << endl;

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
    MPlug parmsModifiedPlug(thisMObject(), AssetNodeAttributes::parmsModified);
    MDataHandle parmsModifiedHandle = data.inputValue(parmsModifiedPlug);
    if (parmsModifiedHandle.asBool())
        setParmValues(data);

    //updates Maya attrs from Houdini
    updateAttrValues(data);

    MPlug outputPlug(thisMObject(), AssetNodeAttributes::output);
    myAsset->compute(outputPlug, data);

    cerr << "end compute #################################### " << plug.name() << endl;

    return MS::kSuccess;
}

