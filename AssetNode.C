#include <maya/MFnNumericAttribute.h>
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

#include "AssetInput.h"
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
MObject AssetNode::inTime;

MObject AssetNode::assetType;

MObject AssetNode::input;

MObject AssetNode::output;
MObject AssetNode::outputObjects;

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
#if MAYA_API_VERSION >= 201400
MObject AssetNode::outputObjectFluidFromAsset;
#endif
MObject AssetNode::outputObjectMetaData;

MObject AssetNode::outputParts;
MObject AssetNode::outputPartName;
MObject AssetNode::outputPartMesh;
MObject AssetNode::outputPartHasParticles;
MObject AssetNode::outputPartMaterial;
MObject AssetNode::outputPartMaterialExists;
MObject AssetNode::outputPartTexturePath;
MObject AssetNode::outputPartAmbientColor;
MObject AssetNode::outputPartDiffuseColor;
MObject AssetNode::outputPartSpecularColor;
MObject AssetNode::outputPartAlphaColor;

MObject AssetNode::outputPartParticle;
MObject AssetNode::outputPartParticlePositions;
MObject AssetNode::outputPartParticleArrayData;

#if MAYA_API_VERSION >= 201400

MObject AssetNode::outputPartVolume;
MObject AssetNode::outputPartVolumeName;
MObject AssetNode::outputPartVolumeGrid;
MObject AssetNode::outputPartVolumeRes;
MObject AssetNode::outputPartVolumeResW;
MObject AssetNode::outputPartVolumeResH;
MObject AssetNode::outputPartVolumeResD;
MObject AssetNode::outputPartVolumeTransform;
MObject AssetNode::outputPartVolumeTranslate;
MObject AssetNode::outputPartVolumeTranslateX;
MObject AssetNode::outputPartVolumeTranslateY;
MObject AssetNode::outputPartVolumeTranslateZ;
MObject AssetNode::outputPartVolumeRotate;
MObject AssetNode::outputPartVolumeRotateX;
MObject AssetNode::outputPartVolumeRotateY;
MObject AssetNode::outputPartVolumeRotateZ;
MObject AssetNode::outputPartVolumeScale;
MObject AssetNode::outputPartVolumeScaleX;
MObject AssetNode::outputPartVolumeScaleY;
MObject AssetNode::outputPartVolumeScaleZ;

#endif

MObject AssetNode::outputVisibility;
MObject AssetNode::outputIsInstanced;
MObject AssetNode::outputInstancers;
MObject AssetNode::outputInstancerData;
MObject AssetNode::outputInstancedObjectNames;
MObject AssetNode::outputHoudiniInstanceAttribute;
MObject AssetNode::outputHoudiniNameAttribute;

MObject AssetNode::outputInstanceTransform;
MObject AssetNode::outputInstanceTranslate;
MObject AssetNode::outputInstanceTranslateX;
MObject AssetNode::outputInstanceTranslateY;
MObject AssetNode::outputInstanceTranslateZ;
MObject AssetNode::outputInstanceRotate;
MObject AssetNode::outputInstanceRotateX;
MObject AssetNode::outputInstanceRotateY;
MObject AssetNode::outputInstanceRotateZ;
MObject AssetNode::outputInstanceScale;
MObject AssetNode::outputInstanceScaleX;
MObject AssetNode::outputInstanceScaleY;
MObject AssetNode::outputInstanceScaleZ;

MObject AssetNode::useInstancerNode;

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
        
    // input
    AssetNode::input = AssetInputs::createInputAttribute();

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

    // houdini instance attribute
    AssetNode::outputHoudiniInstanceAttribute = tAttr.create("outputHoudiniInstanceAttribute", "outputHoudiniInstanceAttribute", MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    tAttr.setArray(true);
    tAttr.setIndexMatters(true);
    cAttr.setUsesArrayDataBuilder(true);
    computeAttributes.push_back(AssetNode::outputHoudiniInstanceAttribute);

    // houdini name attribute
    AssetNode::outputHoudiniNameAttribute = tAttr.create("outputHoudiniNameAttribute", "outputHoudiniNameAttribute", MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    tAttr.setArray(true);
    tAttr.setIndexMatters(true);
    cAttr.setUsesArrayDataBuilder(true);
    computeAttributes.push_back(AssetNode::outputHoudiniNameAttribute);

    // translate
    AssetNode::outputInstanceTranslateX = uAttr.create("outputInstanceTranslateX", "outputInstanceTranslateX", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceTranslateX);
    AssetNode::outputInstanceTranslateY = uAttr.create("outputInstanceTranslateY", "outputInstanceTranslateY", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceTranslateY);
    AssetNode::outputInstanceTranslateZ = uAttr.create("outputInstanceTranslateZ", "outputInstanceTranslateZ", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceTranslateZ);
    AssetNode::outputInstanceTranslate = nAttr.create("outputInstanceTranslate", "outputInstanceTranslate", AssetNode::outputInstanceTranslateX,
            AssetNode::outputInstanceTranslateY, AssetNode::outputInstanceTranslateZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceTranslate);

    // rotate
    AssetNode::outputInstanceRotateX = uAttr.create("outputInstanceRotateX", "outputInstanceRotateX", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceRotateX);
    AssetNode::outputInstanceRotateY = uAttr.create("outputInstanceRotateY", "outputInstanceRotateY", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceRotateY);
    AssetNode::outputInstanceRotateZ = uAttr.create("outputInstanceRotateZ", "outputInstanceRotateZ", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceRotateZ);
    AssetNode::outputInstanceRotate = nAttr.create("outputInstanceRotate", "outputInstanceRotate", AssetNode::outputInstanceRotateX,
            AssetNode::outputInstanceRotateY, AssetNode::outputInstanceRotateZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceRotate);

    // scale
    AssetNode::outputInstanceScaleX = nAttr.create("outputInstanceScaleX", "outputInstanceScaleX", MFnNumericData::kDouble, 1.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceScaleX);
    AssetNode::outputInstanceScaleY = nAttr.create("outputInstanceScaleY", "outputInstanceScaleY", MFnNumericData::kDouble, 1.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceScaleY);
    AssetNode::outputInstanceScaleZ = nAttr.create("outputInstanceScaleZ", "outputInstanceScaleZ", MFnNumericData::kDouble, 1.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceScaleZ);
    AssetNode::outputInstanceScale = nAttr.create("outputInstanceScale", "outputInstanceScale", AssetNode::outputInstanceScaleX,
            AssetNode::outputInstanceScaleY, AssetNode::outputInstanceScaleZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceScale);

    // transform
    AssetNode::outputInstanceTransform = cAttr.create("outputInstanceTransform", "outputInstanceTransform");
    cAttr.addChild(AssetNode::outputInstanceTranslate);
    cAttr.addChild(AssetNode::outputInstanceRotate);
    cAttr.addChild(AssetNode::outputInstanceScale);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    cAttr.setArray( true );
    cAttr.setUsesArrayDataBuilder(true);
    computeAttributes.push_back(AssetNode::outputInstanceTransform);

    
    // instancers
    AssetNode::outputInstancers = cAttr.create("outputInstancers", "outputInstancers");
    cAttr.addChild(AssetNode::outputInstancerData);
    cAttr.addChild(AssetNode::outputInstancedObjectNames);
    cAttr.addChild(AssetNode::outputHoudiniInstanceAttribute);
    cAttr.addChild(AssetNode::outputHoudiniNameAttribute);
    cAttr.addChild(AssetNode::outputInstanceTransform);
    cAttr.setStorable(false);
    cAttr.setWritable(false);
    cAttr.setArray(true);
    cAttr.setUsesArrayDataBuilder(true);
    computeAttributes.push_back(AssetNode::outputInstancers);



    //--------------------------------End instancer compound multi----------------------------------------------

    //----------------------------------  objects compound multi------------------------------------------------

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

#if MAYA_API_VERSION >= 201400
    // object fluid from asset
    AssetNode::outputObjectFluidFromAsset = nAttr.create("outputObjectFluidFromAsset", "outputObjectFluidFromAsset", MFnNumericData::kBoolean, true);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectFluidFromAsset);
#endif

    // meta data
    AssetNode::outputObjectMetaData = tAttr.create("outputObjectMetaData", "outputObjectMetaData", MFnData::kIntArray);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectMetaData);

    // part name
    AssetNode::outputPartName = tAttr.create("outputPartName", "outputPartName", MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartName);

    AssetNode::outputPartHasParticles = nAttr.create("outputPartHasParticles", "outputPartHasParticles", MFnNumericData::kBoolean, false);
    computeAttributes.push_back(AssetNode::outputPartHasParticles);

    // mesh
    AssetNode::outputPartMesh = tAttr.create("outputPartMesh", "outputPartMesh", MFnData::kMesh);
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartMesh);

    // material exists
    AssetNode::outputPartMaterialExists = nAttr.create("outputPartMaterialExists", "outputPartMaterialExists", MFnNumericData::kBoolean, false);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    nAttr.setConnectable(false);
    nAttr.setHidden(true);
    computeAttributes.push_back(AssetNode::outputPartMaterialExists);
    // material ambient
    AssetNode::outputPartAmbientColor = nAttr.createColor("outputPartAmbientColor", "outputPartAmbientColor");
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartAmbientColor);
    // material diffuse
    AssetNode::outputPartDiffuseColor = nAttr.createColor("outputPartDiffuseColor", "outputPartDiffuseColor");
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartDiffuseColor);
    // material specular
    AssetNode::outputPartSpecularColor = nAttr.createColor("outputPartSpecularColor", "outputPartSpecularColor");
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartSpecularColor);
    // material alpha
    AssetNode::outputPartAlphaColor = nAttr.createColor("outputPartAlphaColor", "outputPartAlphaColor");
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartAlphaColor);
    // texture path
    AssetNode::outputPartTexturePath = tAttr.create("outputPartTexturePath", "outputPartTexturePath", MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartTexturePath);

    // material
    AssetNode::outputPartMaterial = cAttr.create("outputPartMaterial", "outputPartMaterial");
    cAttr.addChild(AssetNode::outputPartMaterialExists);
    cAttr.addChild(AssetNode::outputPartAmbientColor);
    cAttr.addChild(AssetNode::outputPartDiffuseColor);
    cAttr.addChild(AssetNode::outputPartSpecularColor);
    cAttr.addChild(AssetNode::outputPartAlphaColor);
    cAttr.addChild(AssetNode::outputPartTexturePath);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartMaterial);

    // particle
    AssetNode::outputPartParticlePositions = tAttr.create(
	    "outputPartParticlePositions",
	    "outputPartParticlePositions",
	    MFnData::kVectorArray
	    );
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartParticlePositions);

    AssetNode::outputPartParticleArrayData = tAttr.create(
	    "outputPartParticleArrayData",
	    "outputPartParticleArrayData",
	    MFnData::kDynArrayAttrs
	    );
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartParticleArrayData);

    AssetNode::outputPartParticle = cAttr.create("outputPartParticle", "outputPartParticle");
    cAttr.addChild(AssetNode::outputPartParticlePositions);
    cAttr.addChild(AssetNode::outputPartParticleArrayData);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartParticle);

#if MAYA_API_VERSION >= 201400
    // Volumes ---------
    AssetNode::outputPartVolumeName = tAttr.create("outputPartVolumeName", "outputPartVolumeName", MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeName);

    AssetNode::outputPartVolumeGrid = tAttr.create("outputPartVolumeGrid", "outputPartVolumeGrid", MFnData::kFloatArray);
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeGrid);

    // Volume resolution
    AssetNode::outputPartVolumeResW = nAttr.create("outputPartVolumeResW", "outputPartVolumeResW", MFnNumericData::kInt);
    nAttr.setWritable(false);
    nAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeResW);
    AssetNode::outputPartVolumeResH = nAttr.create("outputPartVolumeResH", "outputPartVolumeResH", MFnNumericData::kInt);
    nAttr.setWritable(false);
    nAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeResH);
    AssetNode::outputPartVolumeResD = nAttr.create("outputPartVolumeResD", "outputPartVolumeResD", MFnNumericData::kInt);
    nAttr.setWritable(false);
    nAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeResD);
    AssetNode::outputPartVolumeRes = cAttr.create("outputPartVolumeRes", "outputPartVolumeRes");
    cAttr.addChild(AssetNode::outputPartVolumeResW);
    cAttr.addChild(AssetNode::outputPartVolumeResH);
    cAttr.addChild(AssetNode::outputPartVolumeResD);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeRes);


    // volume transform
    // translate
    AssetNode::outputPartVolumeTranslateX = uAttr.create("outputPartVolumeTranslateX", "outputPartVolumeTranslateX", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeTranslateX);
    AssetNode::outputPartVolumeTranslateY = uAttr.create("outputPartVolumeTranslateY", "outputPartVolumeTranslateY", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeTranslateY);
    AssetNode::outputPartVolumeTranslateZ = uAttr.create("outputPartVolumeTranslateZ", "outputPartVolumeTranslateZ", MFnUnitAttribute::kDistance);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeTranslateZ);
    AssetNode::outputPartVolumeTranslate = nAttr.create("outputPartVolumeTranslate", "outputPartVolumeTranslate", AssetNode::outputPartVolumeTranslateX,
            AssetNode::outputPartVolumeTranslateY, AssetNode::outputPartVolumeTranslateZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeTranslate);

    // rotate
    AssetNode::outputPartVolumeRotateX = uAttr.create("outputPartVolumeRotateX", "outputPartVolumeRotateX", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeRotateX);
    AssetNode::outputPartVolumeRotateY = uAttr.create("outputPartVolumeRotateY", "outputPartVolumeRotateY", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeRotateY);
    AssetNode::outputPartVolumeRotateZ = uAttr.create("outputPartVolumeRotateZ", "outputPartVolumeRotateZ", MFnUnitAttribute::kAngle);
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeRotateZ);
    AssetNode::outputPartVolumeRotate = nAttr.create("outputPartVolumeRotate", "outputPartVolumeRotate", AssetNode::outputPartVolumeRotateX,
            AssetNode::outputPartVolumeRotateY, AssetNode::outputPartVolumeRotateZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeRotate);

    // scale
    AssetNode::outputPartVolumeScaleX = nAttr.create("outputPartVolumeScaleX", "outputPartVolumeScaleX", MFnNumericData::kDouble, 1.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeScaleX);
    AssetNode::outputPartVolumeScaleY = nAttr.create("outputPartVolumeScaleY", "outputPartVolumeScaleY", MFnNumericData::kDouble, 1.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeScaleY);
    AssetNode::outputPartVolumeScaleZ = nAttr.create("outputPartVolumeScaleZ", "outputPartVolumeScaleZ", MFnNumericData::kDouble, 1.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeScaleZ);
    AssetNode::outputPartVolumeScale = nAttr.create("outputPartVolumeScale", "outputPartVolumeScale", AssetNode::outputPartVolumeScaleX,
            AssetNode::outputPartVolumeScaleY, AssetNode::outputPartVolumeScaleZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectScale);
    AssetNode::outputPartVolumeTransform = cAttr.create("outputPartVolumeTransform", "outputPartVolumeTransform");
    cAttr.addChild(AssetNode::outputPartVolumeTranslate);
    cAttr.addChild(AssetNode::outputPartVolumeRotate);
    cAttr.addChild(AssetNode::outputPartVolumeScale);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeTransform);

    // volume
    AssetNode::outputPartVolume = cAttr.create("outputPartVolume", "outputPartVolume");
    cAttr.addChild(AssetNode::outputPartVolumeName);
    cAttr.addChild(AssetNode::outputPartVolumeGrid);
    cAttr.addChild(AssetNode::outputPartVolumeTransform);
    cAttr.addChild(AssetNode::outputPartVolumeRes);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartVolume);
#endif

    AssetNode::outputParts = cAttr.create("outputParts", "outputParts");
    cAttr.addChild(AssetNode::outputPartName);
    cAttr.addChild(AssetNode::outputPartHasParticles);
    cAttr.addChild(AssetNode::outputPartMesh);
    cAttr.addChild(AssetNode::outputPartMaterial);
    cAttr.addChild(AssetNode::outputPartParticle);

#if MAYA_API_VERSION >= 201400
    cAttr.addChild(AssetNode::outputPartVolume);
#endif
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    cAttr.setArray(true);
    cAttr.setIndexMatters(true);
    cAttr.setUsesArrayDataBuilder(true);
    computeAttributes.push_back(AssetNode::outputParts);

    AssetNode::outputVisibility = nAttr.create("outputVisibility", "outputVisibility", MFnNumericData::kBoolean, false);
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    AssetNode::outputIsInstanced = nAttr.create("outputIsInstanced", "outputIsInstanced", MFnNumericData::kBoolean, false);
    nAttr.setStorable(false);
    nAttr.setWritable(false);    

    AssetNode::outputObjects = cAttr.create("outputObjects", "outputObjects");
    cAttr.addChild(AssetNode::outputParts);
    cAttr.addChild(AssetNode::outputObjectTransform);
#if MAYA_API_VERSION >= 201400
    cAttr.addChild(AssetNode::outputObjectFluidFromAsset);
#endif
    cAttr.addChild(AssetNode::outputObjectMetaData);
    cAttr.addChild(AssetNode::outputVisibility);
    cAttr.addChild(AssetNode::outputIsInstanced);

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
    
    AssetNode::useInstancerNode = nAttr.create("useInstancerNode", "useInstancerNode", MFnNumericData::kBoolean, false);
    nAttr.setStorable(true);
    nAttr.setWritable(true);
    nAttr.setDefault( false );

    // add the static attributes to the node
    addAttribute(AssetNode::assetPath);
    addAttribute(AssetNode::inTime);
    addAttribute(AssetNode::assetType);
    addAttribute(AssetNode::input);
    addAttribute(AssetNode::output);
    addAttribute(AssetNode::useInstancerNode);

    
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
    destroyAsset();
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
    setPlugDirty(plugBeingDirtied);
    
    affectedPlugs.append(MPlug(thisMObject(), AssetNode::output));

    MPlug outputObjectsPlug(thisMObject(), AssetNode::outputObjects);
    for ( unsigned int i = 0; i < outputObjectsPlug.numElements(); ++i )
    {
        MPlug objPlug = outputObjectsPlug[ i ];
	MPlug outputObjectTransformPlug = objPlug.child(AssetNode::outputObjectTransform);
	affectedPlugs.append(outputObjectTransformPlug.child(AssetNode::outputObjectTranslate));
	affectedPlugs.append(outputObjectTransformPlug.child(AssetNode::outputObjectRotate));
	affectedPlugs.append(outputObjectTransformPlug.child(AssetNode::outputObjectScale));
	affectedPlugs.append(objPlug.child(AssetNode::outputObjectMetaData));
	MPlug outputPartsPlug = objPlug.child(AssetNode::outputParts);
	for ( unsigned int j = 0; j < outputPartsPlug.numElements(); ++j )
	{
	    MPlug elemPlug = outputPartsPlug[ j ];

	    affectedPlugs.append(elemPlug.child(AssetNode::outputPartHasParticles));

	    // Mesh
	    MPlug meshPlug = elemPlug.child(AssetNode::outputPartMesh);
	    affectedPlugs.append(meshPlug);

	    // General part attributes
	    MPlug outputPartNamePlug = elemPlug.child(AssetNode::outputPartName);
	    MPlug outputPartMaterialPlug = elemPlug.child(AssetNode::outputPartMaterial);

	    affectedPlugs.append(outputPartNamePlug);

	    affectedPlugs.append(outputPartMaterialPlug.child(AssetNode::outputPartMaterialExists));
	    affectedPlugs.append(outputPartMaterialPlug.child(AssetNode::outputPartTexturePath));
	    affectedPlugs.append(outputPartMaterialPlug.child(AssetNode::outputPartAmbientColor));
	    affectedPlugs.append(outputPartMaterialPlug.child(AssetNode::outputPartDiffuseColor));
	    affectedPlugs.append(outputPartMaterialPlug.child(AssetNode::outputPartSpecularColor));
	    affectedPlugs.append(outputPartMaterialPlug.child(AssetNode::outputPartAlphaColor));

	    // Particle
	    MPlug outputPartParticle = elemPlug.child(AssetNode::outputPartParticle);
	    affectedPlugs.append(outputPartParticle.child(AssetNode::outputPartParticlePositions));
	    affectedPlugs.append(outputPartParticle.child(AssetNode::outputPartParticleArrayData));

#if MAYA_API_VERSION >= 201400
	    // Volume
	    MPlug outputPartVolume = elemPlug.child(AssetNode::outputPartVolume);
	    affectedPlugs.append(outputPartVolume.child(AssetNode::outputPartVolumeName));
	    affectedPlugs.append(outputPartVolume.child(AssetNode::outputPartVolumeGrid));
	    affectedPlugs.append(outputPartVolume.child(AssetNode::outputPartVolumeRes));
	    affectedPlugs.append(outputPartVolume.child(AssetNode::outputPartVolumeTransform));
	    affectedPlugs.append(outputPartVolume.child(AssetNode::outputPartVolumeTranslate));
	    affectedPlugs.append(outputPartVolume.child(AssetNode::outputPartVolumeRotate));
	    affectedPlugs.append(outputPartVolume.child(AssetNode::outputPartVolumeScale));
#endif
	}

    }

    MPlug outputInstancersPlug(thisMObject(), AssetNode::outputInstancers);
    for ( unsigned int i = 0; i < outputInstancersPlug.numElements(); ++i )
    {
	MPlug elemPlug = outputInstancersPlug[ i ];

	MPlug outputInstancerDataPlug = elemPlug.child( AssetNode::outputInstancerData );
	MPlug outputInstancedObjectNamesPlug = elemPlug.child( AssetNode::outputInstancedObjectNames );

	affectedPlugs.append( outputInstancerDataPlug );

	for ( unsigned int j = 0; j < outputInstancedObjectNamesPlug.numElements(); ++j )
	    affectedPlugs.append(outputInstancedObjectNamesPlug[ j ] );

	
	MPlug outputHoudiniInstancePlug = elemPlug.child( AssetNode::outputHoudiniInstanceAttribute );
	for ( unsigned int jj = 0; jj < outputHoudiniInstancePlug.numElements(); ++jj )
	    affectedPlugs.append( outputHoudiniInstancePlug[ jj ] );

	MPlug outputHoudiniNamePlug = elemPlug.child( AssetNode::outputHoudiniNameAttribute );
	for ( unsigned int jj = 0; jj < outputHoudiniNamePlug.numElements(); ++jj )
	    affectedPlugs.append( outputHoudiniNamePlug[ jj ] );

	MPlug outputInstanceTransformPlug = elemPlug.child( AssetNode::outputInstanceTransform );
	for ( unsigned int jj = 0; jj < outputInstanceTransformPlug.numElements(); ++jj )
	{
	    MPlug transformPlug = outputInstanceTransformPlug[ jj ];

	    //translation
	    MPlug outputInstanceTranslatePlug = transformPlug.child( AssetNode::outputInstanceTranslate );
	    affectedPlugs.append( outputInstanceTranslatePlug );

	    MPlug outputInstanceTxPlug = outputInstanceTranslatePlug.child( AssetNode::outputInstanceTranslateX );
	    affectedPlugs.append( outputInstanceTxPlug );

	    MPlug outputInstanceTyPlug = outputInstanceTranslatePlug.child( AssetNode::outputInstanceTranslateY );
	    affectedPlugs.append( outputInstanceTyPlug );

	    MPlug outputInstanceTzPlug = outputInstanceTranslatePlug.child( AssetNode::outputInstanceTranslateZ );
	    affectedPlugs.append( outputInstanceTzPlug );

	    //rotation
	    MPlug outputInstanceRotatePlug = transformPlug.child( AssetNode::outputInstanceRotate );
	    affectedPlugs.append( outputInstanceRotatePlug );

	    MPlug outputInstanceRxPlug = outputInstanceRotatePlug.child( AssetNode::outputInstanceRotateX );
	    affectedPlugs.append( outputInstanceRxPlug );

	    MPlug outputInstanceRyPlug = outputInstanceRotatePlug.child( AssetNode::outputInstanceRotateY );
	    affectedPlugs.append( outputInstanceRyPlug );

	    MPlug outputInstanceRzPlug = outputInstanceRotatePlug.child( AssetNode::outputInstanceRotateZ );
	    affectedPlugs.append( outputInstanceRzPlug );

	    //scale
	    MPlug outputInstanceScalePlug = transformPlug.child( AssetNode::outputInstanceScale );
	    affectedPlugs.append( outputInstanceScalePlug );

	    MPlug outputInstanceSxPlug = outputInstanceScalePlug.child( AssetNode::outputInstanceScaleX );
	    affectedPlugs.append( outputInstanceSxPlug );

	    MPlug outputInstanceSyPlug = outputInstanceScalePlug.child( AssetNode::outputInstanceScaleY );
	    affectedPlugs.append( outputInstanceSyPlug );

	    MPlug outputInstanceSzPlug = outputInstanceScalePlug.child( AssetNode::outputInstanceScaleZ );
	    affectedPlugs.append( outputInstanceSzPlug );

	}

    }

    return MS::kSuccess;
}

void
AssetNode::rebuildAsset()
{
    destroyAsset();
    myAsset = new Asset(myAssetPath, thisMObject());
    myResultsClean = false;

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
    // The attribute might not actually exist on the node if the asset has
    // changed since last sync.
    if(attr.isNull())
    {
	return;
    }

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

    delete[] parmInfos;
}

// This function takes Maya attr values and pushes it into Houdini
void
AssetNode::setParmValue(HAPI_ParmInfo& parm, MDataBlock& data, bool dirtyOnly)
{    

    MObject attr = getAttrFromParm(parm);
    // The attribute might not actually exist on the node if the asset has
    // changed since last sync.
    if(attr.isNull())
    {
	return;
    }

    MPlug plug(thisMObject(), attr);

    //Only push into Houdini the minimum changes necessary.
    //Only push what has been dirtied.
    if (dirtyOnly && !isPlugDirty(plug, parm))
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
AssetNode::setParmValues(MDataBlock& data, bool dirtyOnly)
{
    int parmCount = myAsset->myNodeInfo.parmCount;
    if (parmCount <= 0)
        return;
    HAPI_ParmInfo * parmInfos = new HAPI_ParmInfo[parmCount];
    HAPI_GetParameters(myAsset->myNodeInfo.id, parmInfos, 0, parmCount );

    for (int i=0; i<parmCount; i++)
    {

        HAPI_ParmInfo& parm = parmInfos[i];
        setParmValue(parm, data, dirtyOnly);
    }

    delete[] parmInfos;
}

bool
AssetNode::isPlugDirty(const MPlug &plug, const HAPI_ParmInfo &parm)
{
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

    return isDirty;
}

void
AssetNode::setPlugDirty(const MPlug &plug)
{
    myDirtyParmAttributes.push_back(plug.attribute());
}

MStatus
AssetNode::compute(const MPlug& plug, MDataBlock& data)
{
    if(std::find(computeAttributes.begin(), computeAttributes.end(), plug)
	!= computeAttributes.end() && !myResultsClean )
    {
	// make sure Asset is created
	createAsset();

	//push parms into Houdini
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

int
AssetNode::internalArrayCount(const MPlug &plug, const MDGContext &ctx) const
{
    if(plug == AssetNode::input)
    {
	if(!myAsset)
	{
	    return 0;
	}

	return myAsset->myAssetInfo.maxGeoInputCount;
    }

    return MPxTransform::internalArrayCount(plug, ctx);
}

void
AssetNode::copyInternalData(MPxNode* node)
{
    AssetNode* assetNode = dynamic_cast<AssetNode*>(node);

    myAssetPath = assetNode->myAssetPath;
    myAssetPathChanged = true;

    MPxTransform::copyInternalData(node);
}

Asset*
AssetNode::getAsset()
{
    createAsset();

    return myAsset;
}

void
AssetNode::createAsset()
{
    if(!myAssetPathChanged)
    {
	return;
    }

    destroyAsset();

    myAsset = new Asset(myAssetPath, thisMObject());

    myAssetPathChanged = false;

    // Restore all the asset's parameter values
    MDataBlock dataBlock = forceCache();
    setParmValues(dataBlock, false);
}

void
AssetNode::destroyAsset()
{
    if(myAsset)
    {
	delete myAsset;
	myAsset = NULL;
    }
}
