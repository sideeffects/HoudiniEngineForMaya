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

#include <algorithm>

#include "AssetNode.h"
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
MObject AssetNode::assetPath;
MObject AssetNode::parmsModified;
MObject AssetNode::inTime;

MObject AssetNode::assetType;

MObject AssetNode::output;
MObject AssetNode::outputObjects;

MObject AssetNode::outputObjectName;
MObject AssetNode::outputObjectMetaData;
MObject AssetNode::outputObjectMesh;

MObject AssetNode::outputObjectTransform;
MObject AssetNode::outputObjectTranslate;
MObject AssetNode::outputObjectTranslateX;
MObject AssetNode::outputObjectTranslateY;
MObject AssetNode::outputObjectTranslateZ;
MObject AssetNode::outputObjectRotate;
MObject AssetNode::outputObjectRotateX;
MObject AssetNode::outputObjectRotateY;
MObject AssetNode::outputObjectRotateZ;
MObject AssetNode::outputObjectScale;
MObject AssetNode::outputObjectScaleX;
MObject AssetNode::outputObjectScaleY;
MObject AssetNode::outputObjectScaleZ;

MObject AssetNode::outputObjectMaterial;
MObject AssetNode::outputObjectMaterialExists;
MObject AssetNode::outputObjectTexturePath;
MObject AssetNode::outputObjectAmbientColor;
MObject AssetNode::outputObjectDiffuseColor;
MObject AssetNode::outputObjectSpecularColor;
MObject AssetNode::outputObjectAlphaColor;

MObject AssetNode::outputInstancers;
MObject AssetNode::outputInstancerData;
MObject AssetNode::outputInstancedObjectNames;

std::vector<MObject> computeAttributes;

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
    AssetNode::assetPath = tAttr.create("assetPath", "assetPath", MFnData::kString);
    tAttr.setInternal(true);
    tAttr.setUsedAsFilename(true);

    // parms modified
    // This is initially false, and whenever a user touches a parm, this will get set to true
    // and because it's a Maya attr, it will get saved.  When we load back the file, 
    // if this attr is true, we know we are loading a previously modified asset as opposed
    // to instantiating a new asset.
    AssetNode::parmsModified = nAttr.create("parmsModified", "parmsModified", MFnNumericData::kBoolean, false);
    nAttr.setStorable(true);
    nAttr.setConnectable(false);
    nAttr.setHidden(true);

    // time input
    // For time dpendence.
    AssetNode::inTime = uAttr.create("inTime", "inTime", MTime());
    uAttr.setStorable(true);
    uAttr.setHidden(true);

    // asset type
    // This maps to the underlying Houdini asset type: OBJ, SOP, etc. (see HAPI_AssetType)
    AssetNode::assetType = nAttr.create("assetType", "assetType", MFnNumericData::kInt);
    nAttr.setStorable(false);
    nAttr.setWritable(false);    
    computeAttributes.push_back(AssetNode::assetType);
        
    //----------------------------------  instancer compound multi----------------------------------------------
    // instancer data
    AssetNode::outputInstancerData = tAttr.create("outputInstancerData", "outputInstancerData", MFnData::kDynArrayAttrs);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstancerData);

    // instanced object names
    AssetNode::outputInstancedObjectNames = tAttr.create("outputInstancedObjectNames", "outputInstancedObjectNames", MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    tAttr.setArray(true);
    tAttr.setIndexMatters(true);
    cAttr.setUsesArrayDataBuilder(true);
    computeAttributes.push_back(AssetNode::outputInstancedObjectNames);

    // instancers
    AssetNode::outputInstancers = cAttr.create("outputInstancers", "outputInstancers");
    cAttr.addChild(AssetNode::outputInstancerData);
    cAttr.addChild(AssetNode::outputInstancedObjectNames);
    cAttr.setStorable(false);
    cAttr.setWritable(false);
    cAttr.setArray(true);
    cAttr.setUsesArrayDataBuilder(true);
    computeAttributes.push_back(AssetNode::outputInstancers);
    //--------------------------------End instancer compound multi----------------------------------------------

    //----------------------------------  objects compound multi------------------------------------------------

    // object name
    AssetNode::outputObjectName = tAttr.create("outputObjectName", "outputObjectName", MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectName);

    // meta data
    AssetNode::outputObjectMetaData = tAttr.create("outputObjectMetaData", "outputObjectMetaData", MFnData::kIntArray);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectMetaData);

    // mesh
    AssetNode::outputObjectMesh = tAttr.create("outputObjectMesh", "outputObjectMesh", MFnData::kMesh);
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputObjectMesh);

    // translate
    AssetNode::outputObjectTranslateX = uAttr.create("outputObjectTranslateX", "outputObjectTranslateX", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectTranslateX);
    AssetNode::outputObjectTranslateY = uAttr.create("outputObjectTranslateY", "outputObjectTranslateY", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectTranslateY);
    AssetNode::outputObjectTranslateZ = uAttr.create("outputObjectTranslateZ", "outputObjectTranslateZ", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectTranslateZ);
    AssetNode::outputObjectTranslate = nAttr.create("outputObjectTranslate", "outputObjectTranslate", AssetNode::outputObjectTranslateX,
            AssetNode::outputObjectTranslateY, AssetNode::outputObjectTranslateZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectTranslate);

    // rotate
    AssetNode::outputObjectRotateX = uAttr.create("outputObjectRotateX", "outputObjectRotateX", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectRotateX);
    AssetNode::outputObjectRotateY = uAttr.create("outputObjectRotateY", "outputObjectRotateY", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectRotateY);
    AssetNode::outputObjectRotateZ = uAttr.create("outputObjectRotateZ", "outputObjectRotateZ", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectRotateZ);
    AssetNode::outputObjectRotate = nAttr.create("outputObjectRotate", "outputObjectRotate", AssetNode::outputObjectRotateX,
            AssetNode::outputObjectRotateY, AssetNode::outputObjectRotateZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectRotate);

    // scale
    AssetNode::outputObjectScaleX = nAttr.create("outputObjectScaleX", "outputObjectScaleX", MFnNumericData::kDouble, 1.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectScaleX);
    AssetNode::outputObjectScaleY = nAttr.create("outputObjectScaleY", "outputObjectScaleY", MFnNumericData::kDouble, 1.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectScaleY);
    AssetNode::outputObjectScaleZ = nAttr.create("outputObjectScaleZ", "outputObjectScaleZ", MFnNumericData::kDouble, 1.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectScaleZ);
    AssetNode::outputObjectScale = nAttr.create("outputObjectScale", "outputObjectScale", AssetNode::outputObjectScaleX,
            AssetNode::outputObjectScaleY, AssetNode::outputObjectScaleZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectScale);

    // transform
    AssetNode::outputObjectTransform = cAttr.create("outputObjectTransform", "outputObjectTransform");
    cAttr.addChild(AssetNode::outputObjectTranslate);
    cAttr.addChild(AssetNode::outputObjectRotate);
    cAttr.addChild(AssetNode::outputObjectScale);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputObjectTransform);

    // material exists
    AssetNode::outputObjectMaterialExists = nAttr.create("outputObjectMaterialExists", "outputObjectMaterialExists", MFnNumericData::kBoolean, false);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    nAttr.setConnectable(false);
    nAttr.setHidden(true);
    computeAttributes.push_back(AssetNode::outputObjectMaterialExists);
    // material ambient
    AssetNode::outputObjectAmbientColor = nAttr.createColor("outputObjectAmbientColor", "outputObjectAmbientColor");
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectAmbientColor);
    // material diffuse
    AssetNode::outputObjectDiffuseColor = nAttr.createColor("outputObjectDiffuseColor", "outputObjectDiffuseColor");
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectDiffuseColor);
    // material specular
    AssetNode::outputObjectSpecularColor = nAttr.createColor("outputObjectSpecularColor", "outputObjectSpecularColor");
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectSpecularColor);
    // material alpha
    AssetNode::outputObjectAlphaColor = nAttr.createColor("outputObjectAlphaColor", "outputObjectAlphaColor");
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectAlphaColor);
    // texture path
    AssetNode::outputObjectTexturePath = tAttr.create("outputObjectTexturePath", "outputObjectTexturePath", MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectTexturePath);

    // material
    AssetNode::outputObjectMaterial = cAttr.create("outputObjectMaterial", "outputObjectMaterial");
    cAttr.addChild(AssetNode::outputObjectMaterialExists);
    cAttr.addChild(AssetNode::outputObjectAmbientColor);
    cAttr.addChild(AssetNode::outputObjectDiffuseColor);
    cAttr.addChild(AssetNode::outputObjectSpecularColor);
    cAttr.addChild(AssetNode::outputObjectAlphaColor);
    cAttr.addChild(AssetNode::outputObjectTexturePath);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputObjectMaterial);

    AssetNode::outputObjects = cAttr.create("outputObjects", "outputObjects");
    cAttr.addChild(AssetNode::outputObjectName);
    cAttr.addChild(AssetNode::outputObjectMetaData);
    cAttr.addChild(AssetNode::outputObjectMesh);
    cAttr.addChild(AssetNode::outputObjectTransform);
    cAttr.addChild(AssetNode::outputObjectMaterial);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    cAttr.setArray(true);
    cAttr.setIndexMatters(true);
    cAttr.setUsesArrayDataBuilder(true);
    computeAttributes.push_back(AssetNode::outputObjects);

    //------------------------------- END  objects compound multi------------------------------------------------

    // output
    AssetNode::output = cAttr.create("output", "out");    
    cAttr.addChild(AssetNode::outputObjects);
    cAttr.addChild(AssetNode::outputInstancers);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::output);
    
    // add the static attributes to the node
    addAttribute(AssetNode::assetPath);
    addAttribute(AssetNode::parmsModified);
    addAttribute(AssetNode::inTime);
    addAttribute(AssetNode::assetType);
    addAttribute(AssetNode::output);

    
    //most of the dependencies between attrs are set via the AssetNode::setDependentsDirty() call
    //this call may not even be necessary.
    attributeAffects(AssetNode::assetPath, AssetNode::output);
    

    return MS::kSuccess;
}


AssetNode::AssetNode()
{    
    myAsset = NULL;

    myBuiltParms = false;
    myAssetPathChanged = true;
    myResultsClean = false;
}


AssetNode::~AssetNode()
{
    if(myAsset)
    {
	delete myAsset;
    }
}


void
AssetNode::postConstructor()
{
}


MStatus
AssetNode::setDependentsDirty(const MPlug& plugBeingDirtied,
        MPlugArray& affectedPlugs)
{
    if (plugBeingDirtied == AssetNode::assetPath)
        return MS::kSuccess;

    myResultsClean = false;
    myDirtyParmAttributes.push_back(plugBeingDirtied.attribute());
    
    affectedPlugs.append(MPlug(thisMObject(), AssetNode::output));

    MPlug outputObjectsPlug(thisMObject(), AssetNode::outputObjects);
    MPlug outputInstancersPlug(thisMObject(), AssetNode::outputInstancers);

    for ( unsigned int i = 0; i < outputObjectsPlug.numElements(); ++i )
    {
        MPlug elemPlug = outputObjectsPlug[ i ];

        MPlug outputObjectNamePlug = elemPlug.child(AssetNode::outputObjectName);
        MPlug outputObjectMetaDataPlug = elemPlug.child(AssetNode::outputObjectMetaData);
        MPlug meshPlug = elemPlug.child(AssetNode::outputObjectMesh);
        MPlug outputObjectTransformPlug = elemPlug.child(AssetNode::outputObjectTransform);
        MPlug outputObjectMaterialPlug = elemPlug.child(AssetNode::outputObjectMaterial);


        affectedPlugs.append(outputObjectNamePlug);
        affectedPlugs.append(outputObjectMetaDataPlug);
        affectedPlugs.append(meshPlug);

        affectedPlugs.append(outputObjectTransformPlug.child(AssetNode::outputObjectTranslate));
        affectedPlugs.append(outputObjectTransformPlug.child(AssetNode::outputObjectRotate));
        affectedPlugs.append(outputObjectTransformPlug.child(AssetNode::outputObjectScale));

        affectedPlugs.append(outputObjectMaterialPlug.child(AssetNode::outputObjectMaterialExists));
        affectedPlugs.append(outputObjectMaterialPlug.child(AssetNode::outputObjectTexturePath));
        affectedPlugs.append(outputObjectMaterialPlug.child(AssetNode::outputObjectAmbientColor));
        affectedPlugs.append(outputObjectMaterialPlug.child(AssetNode::outputObjectDiffuseColor));
        affectedPlugs.append(outputObjectMaterialPlug.child(AssetNode::outputObjectSpecularColor));
	affectedPlugs.append(outputObjectMaterialPlug.child(AssetNode::outputObjectAlphaColor));
    }

    for ( unsigned int i = 0; i < outputInstancersPlug.numElements(); ++i )
    {
	MPlug elemPlug = outputInstancersPlug[ i ];
	MPlug outputInstancerDataPlug = elemPlug.child( AssetNode::outputInstancerData );
	MPlug outputInstancedObjectNamesPlug = elemPlug.child( AssetNode::outputInstancedObjectNames );

	affectedPlugs.append( outputInstancerDataPlug );

	for ( unsigned int j = 0; j < outputInstancedObjectNamesPlug.numElements(); ++j )
	    affectedPlugs.append(outputInstancedObjectNamesPlug[ j ] );
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
    int parmCount = myAsset->myNodeInfo.parmCount;
    if (parmCount <= 0)
        return;
    HAPI_ParmInfo * parmInfos = new HAPI_ParmInfo[parmCount];
    HAPI_GetParameters(myAsset->myNodeInfo.id, parmInfos, 0, parmCount);

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

// This function takes Maya attr values and pushes it into Houdini
void
AssetNode::setParmValue(HAPI_ParmInfo& parm, MDataBlock& data)
{    

    MObject attr = getAttrFromParm(parm);
    MPlug plug(thisMObject(), attr);

    //Only push into Houdini the minimum changes necessary.
    //Only push what has been dirtied.
    bool isDirty = false;
    for(MObjectVector::iterator iter = myDirtyParmAttributes.begin();
	    iter != myDirtyParmAttributes.end();
	    iter++)
    {
	MPlug dirtyParmPlug(thisMObject(), *iter);

	// If the dirtied plug matches the parm
	if(plug == dirtyParmPlug)
	{
	    isDirty = true;
	}

	// If the parm is a tuple, then we also need to check the parent plug.
	// We need to check if it's int, float, or string, because non-values
	// like folders also use parm.size.
	if((parm.isInt() || parm.isFloat() || parm.isString())
		&& parm.size > 1
		&& dirtyParmPlug.isChild() && dirtyParmPlug.parent() == plug)
	{
	    isDirty = true;
	}

	if(isDirty)
	{
	    myDirtyParmAttributes.erase(iter);
	    break;
	}
    }
    // if the parm didn't match any dirty attributes, then skip this parm
    if (!isDirty)
    {
	return;
    }

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
        HAPI_SetParmIntValues( myAsset->myNodeInfo.id, values, parm.intValuesIndex, size );

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
        HAPI_SetParmFloatValues( myAsset->myNodeInfo.id, values, parm.floatValuesIndex, size);

        delete[] values;
    }

    if (parm.isString())
    {
        if (size == 1)
        {
            MDataHandle handle = data.inputValue(plug);
            const char* val = handle.asString().asChar();
            HAPI_SetParmStringValue( myAsset->myNodeInfo.id, val, parm.id, 0);
        } else
        {
            for (int i=0; i<size; i++)
            {
                MPlug child = plug.child(i);
                MDataHandle handle = data.inputValue(child);
                const char* val = handle.asString().asChar();
                HAPI_SetParmStringValue( myAsset->myNodeInfo.id, val, parm.id, i);
            }
        }
    }

}

// This function takes Maya attr values and pushes it into Houdini
void
AssetNode::setParmValues(MDataBlock& data)
{
    int parmCount = myAsset->myNodeInfo.parmCount;
    if (parmCount <= 0)
        return;
    HAPI_ParmInfo * parmInfos = new HAPI_ParmInfo[parmCount];
    HAPI_GetParameters(myAsset->myNodeInfo.id, parmInfos, 0, parmCount );

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
    if(std::find(computeAttributes.begin(), computeAttributes.end(), plug)
	!= computeAttributes.end() && !myResultsClean )
    {
	// load otl
	if (myAssetPathChanged)
	{
	    try
	    {
		myAsset = new Asset(myAssetPath, thisMObject());
		myAssetPathChanged = false;
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

	myResultsClean = true;

	data.setClean( plug );
	return MStatus::kSuccess;
    }
    else
    {		
	return MStatus::kUnknownParameter;
    }
    
}

bool
AssetNode::getInternalValueInContext(
	const MPlug &plug,
	MDataHandle &dataHandle,
	MDGContext &ctx)
{
    if(plug == assetPath)
    {
	dataHandle.setString(myAssetPath);

	return true;
    }

    return MPxTransform::getInternalValueInContext(plug, dataHandle, ctx);
}

bool
AssetNode::setInternalValueInContext(
	const MPlug &plug,
	const MDataHandle &dataHandle,
	MDGContext &ctx
	)
{
    if(plug == assetPath)
    {
	myAssetPath = dataHandle.asString();
	myAssetPathChanged = true;

	return true;
    }

    return MPxTransform::setInternalValueInContext(plug, dataHandle, ctx);
}

void
AssetNode::copyInternalData(MPxNode* node)
{
    AssetNode* assetNode = dynamic_cast<AssetNode*>(node);

    myAssetPath = assetNode->myAssetPath;
    myAssetPathChanged = true;

    MPxTransform::copyInternalData(node);
}
