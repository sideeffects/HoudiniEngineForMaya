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

MTypeId AssetNode::id(MayaTypeID_HoudiniAssetNode);
MObject AssetNode::inTime;

MObject AssetNode::otlFilePath;
MObject AssetNode::assetName;

MObject AssetNode::assetType;

MObject AssetNode::autoSyncOutputs;

MObject AssetNode::input;

MObject AssetNode::output;
MObject AssetNode::outputObjects;

MObject AssetNode::outputObjectName;

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

MObject AssetNode::outputGeos;
MObject AssetNode::outputGeoType;
MObject AssetNode::outputGeoName;
MObject AssetNode::outputGeoCurveType;
MObject AssetNode::outputGeoCurveOrder;
MObject AssetNode::outputGeoCurveCVX;
MObject AssetNode::outputGeoCurveCVY;
MObject AssetNode::outputGeoCurveCVZ;
MObject AssetNode::outputGeoCurveCVs;


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

    // time input
    // For time dpendence.
    AssetNode::inTime = uAttr.create("inTime", "inTime", MTime());
    uAttr.setStorable(true);
    uAttr.setHidden(true);

    // otl file path
    AssetNode::otlFilePath = tAttr.create("otlFilePath", "otlFilePath", MFnData::kString);
    tAttr.setInternal(true);
    tAttr.setUsedAsFilename(true);

    // asset name
    AssetNode::assetName = tAttr.create("assetName", "assetName", MFnData::kString);
    tAttr.setInternal(true);

    // asset type
    // This maps to the underlying Houdini asset type: OBJ, SOP, etc. (see HAPI_AssetType)
    AssetNode::assetType = nAttr.create("assetType", "assetType", MFnNumericData::kInt);
    nAttr.setStorable(false);
    nAttr.setWritable(false);    
    computeAttributes.push_back(AssetNode::assetType);
        
    AssetNode::autoSyncOutputs = nAttr.create("autoSyncOutputs", "autoSyncOutputs", MFnNumericData::kBoolean);

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
    AssetNode::outputPartVolumeRes = tAttr.create("outputPartVolumeRes", "outputPartVolumeRes", MFnData::kFloatArray);
    nAttr.setWritable(false);
    nAttr.setStorable(false);
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

    // output geos
    AssetNode::outputGeoType = tAttr.create("outputGeoType", "outputGeoType", MFnData::kString);
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputGeoType);

    AssetNode::outputGeoName = tAttr.create("outputGeoName", "outputGeoName", MFnData::kString);
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputGeoName);    

    AssetNode::outputGeoCurveType = tAttr.create("outputGeoCurveType", "outputGeoCurveType", MFnData::kString);
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputGeoCurveType);

    AssetNode::outputGeoCurveOrder = nAttr.create("outputGeoCurveOrder", "outputGeoCurveOrder", MFnNumericData::kInt, 0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputGeoCurveOrder);

    AssetNode::outputGeoCurveCVX = nAttr.create("outputGeoCurveCVX", "outputGeoCurveCVX", MFnNumericData::kDouble, 0.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputGeoCurveCVX);

    AssetNode::outputGeoCurveCVY = nAttr.create("outputGeoCurveCVY", "outputGeoCurveCVY", MFnNumericData::kDouble, 0.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputGeoCurveCVY);
    
    AssetNode::outputGeoCurveCVZ = nAttr.create("outputGeoCurveCVZ", "outputGeoCurveCVZ", MFnNumericData::kDouble, 0.0);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputGeoCurveCVZ);

    AssetNode::outputGeoCurveCVs = nAttr.create("outputGeoCurveCVs", "outputGeoCurveCVs", AssetNode::outputGeoCurveCVX,
            AssetNode::outputGeoCurveCVY, AssetNode::outputGeoCurveCVZ);
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    nAttr.setArray( true );
    nAttr.setIndexMatters( true );
    nAttr.setUsesArrayDataBuilder( true );
    computeAttributes.push_back(AssetNode::outputGeoCurveCVs);

    AssetNode::outputGeos = cAttr.create("outputGeos", "outputGeos");
    cAttr.addChild( AssetNode::outputGeoType );
    cAttr.addChild( AssetNode::outputGeoName );
    cAttr.addChild( AssetNode::outputGeoCurveType );
    cAttr.addChild( AssetNode::outputGeoCurveOrder );
    cAttr.addChild( AssetNode::outputGeoCurveCVs );
    cAttr.addChild( AssetNode::outputParts );
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    cAttr.setArray(true);
    cAttr.setIndexMatters(true);
    cAttr.setUsesArrayDataBuilder(true);
    computeAttributes.push_back(AssetNode::outputGeos);

    AssetNode::outputVisibility = nAttr.create("outputVisibility", "outputVisibility", MFnNumericData::kBoolean, false);
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    AssetNode::outputIsInstanced = nAttr.create("outputIsInstanced", "outputIsInstanced", MFnNumericData::kBoolean, false);
    nAttr.setStorable(false);
    nAttr.setWritable(false);     

    AssetNode::outputObjectName = tAttr.create("outputObjectName", "outputObjectName", MFnData::kString);
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputObjectName);    

    AssetNode::outputObjects = cAttr.create("outputObjects", "outputObjects");
    cAttr.addChild(AssetNode::outputGeos);
    cAttr.addChild(AssetNode::outputObjectName);
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
    addAttribute(AssetNode::autoSyncOutputs);
    addAttribute(AssetNode::inTime);
    addAttribute(AssetNode::otlFilePath);
    addAttribute(AssetNode::assetName);
    addAttribute(AssetNode::assetType);
    addAttribute(AssetNode::input);
    addAttribute(AssetNode::output);
    addAttribute(AssetNode::useInstancerNode);

    
    //most of the dependencies between attrs are set via the AssetNode::setDependentsDirty() call
    //this call may not even be necessary.
    attributeAffects(AssetNode::otlFilePath, AssetNode::output);
    attributeAffects(AssetNode::assetName, AssetNode::output);
    

    return MS::kSuccess;
}


AssetNode::AssetNode()
{    
    myAsset = NULL;

    myBuiltParms = false;
    //myAssetPathChanged = true;
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
    if (plugBeingDirtied == AssetNode::otlFilePath
        || plugBeingDirtied == AssetNode::assetName)
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

        MPlug outputGeosPlug = objPlug.child( AssetNode::outputGeos );
        for ( unsigned int jj = 0; jj < outputGeosPlug.numElements(); ++jj )
	{
            MPlug geoPlug = outputGeosPlug[ jj ];

	    MPlug outputPartsPlug = geoPlug.child(AssetNode::outputParts);
	    for ( unsigned int kk = 0; kk < outputPartsPlug.numElements(); ++kk )
	    {
	        MPlug elemPlug = outputPartsPlug[ kk ];

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

		MPlug outputPartVolumeTransform = outputPartVolume.child(AssetNode::outputPartVolumeTransform);
	        affectedPlugs.append(outputPartVolumeTransform);
		affectedPlugs.append(outputPartVolumeTransform.child(AssetNode::outputPartVolumeTranslate));
		affectedPlugs.append(outputPartVolumeTransform.child(AssetNode::outputPartVolumeRotate));
		affectedPlugs.append(outputPartVolumeTransform.child(AssetNode::outputPartVolumeScale));
#endif
	    }
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
    myAsset = new Asset(myOTLFilePath, myAssetName, thisMObject());
    myResultsClean = false;

}

void
AssetNode::setPlugDirty(const MPlug &plug)
{
    myDirtyParmAttributes.push_back(plug.attribute());
}

MStatus
AssetNode::compute(const MPlug& plug, MDataBlock& data)
{
    MStatus status;

    if(std::find(computeAttributes.begin(), computeAttributes.end(), plug)
	!= computeAttributes.end() && !myResultsClean )
    {
	// make sure Asset is created
	createAsset();

        MFnDependencyNode assetNodeFn(thisMObject());
        MObject parmAttrObj = assetNodeFn.attribute(Util::getParmAttrPrefix(), &status);

	//push parms into Houdini
        if(!parmAttrObj.isNull())
        {
            myAsset->setParmValues(
                    data,
                    assetNodeFn,
                    &myDirtyParmAttributes
                    );
            myDirtyParmAttributes.clear();
        }

	//updates Maya attrs from Houdini
        if(!parmAttrObj.isNull())
        {
            myAsset->getParmValues(
                    data,
                    assetNodeFn,
                    NULL
                    );
        }

        bool autoSyncOutputs = data.inputValue(AssetNode::autoSyncOutputs).asBool();

	MPlug outputPlug(thisMObject(), AssetNode::output);
        bool needToSyncOutputs = false;
	myAsset->compute(outputPlug, data, needToSyncOutputs);
        if(autoSyncOutputs && needToSyncOutputs)
        {
            MGlobal::executeCommandOnIdle("houdiniAsset -sync " + assetNodeFn.name() + " -syncOutputs");
        }

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
    MStatus status;

    if(plug == AssetNode::otlFilePath)
    {
	dataHandle.setString(myOTLFilePath);

	return true;
    }
    else if(plug == AssetNode::assetName)
    {
	dataHandle.setString(myAssetName);

	return true;
    }

    // handle getting multiparm length
    {
        MString multiSizeSuffix = "__multiSize";
        // check suffix
        MString attrName = MFnAttribute(plug.attribute()).name();
        if(attrName.substring(attrName.length() - multiSizeSuffix.length(),
                        attrName.length() - 1) == multiSizeSuffix)
        {
            MFnDependencyNode assetNodeFn(thisMObject());
            MObject parmAttrObj = assetNodeFn.attribute(Util::getParmAttrPrefix(), &status);

            if(!parmAttrObj.isNull())
            {
                MDataBlock dataBlock = forceCache();

                int multiSize = 0;
                getAsset()->getMultiparmLength(
                        dataBlock,
                        plug,
                        multiSize,
                        assetNodeFn
                        );

                dataHandle.setInt(multiSize);
            }

            return true;
        }
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
    MStatus status;

    if(plug == AssetNode::otlFilePath)
    {
	myOTLFilePath = dataHandle.asString();

	return true;
    }
    else if(plug == AssetNode::assetName)
    {
	myAssetName = dataHandle.asString();

	return true;
    }

    // handle setting multiparm length
    {
        MString multiSizeSuffix = "__multiSize";
        // check suffix
        MString attrName = MFnAttribute(plug.attribute()).name();
        if(attrName.substring(attrName.length() - multiSizeSuffix.length(),
                        attrName.length() - 1) == multiSizeSuffix)
        {
            MFnDependencyNode assetNodeFn(thisMObject());
            MObject parmAttrObj = assetNodeFn.attribute(Util::getParmAttrPrefix(), &status);

            if(!parmAttrObj.isNull())
            {
                MDataBlock dataBlock = forceCache();

                getAsset()->setMultiparmLength(
                        dataBlock,
                        plug,
                        dataHandle.asInt(),
                        assetNodeFn
                        );
            }

            return true;
        }
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

        return myAsset->getAssetInfo().maxGeoInputCount;
    }

    return MPxTransform::internalArrayCount(plug, ctx);
}

void
AssetNode::copyInternalData(MPxNode* node)
{
    AssetNode* assetNode = dynamic_cast<AssetNode*>(node);

    myOTLFilePath = assetNode->myOTLFilePath;
    myAssetName = assetNode->myAssetName;
    //myAssetPathChanged = true;

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
    MStatus status;

    if(
            myAsset != NULL
            && myAsset->getOTLFilePath() == myOTLFilePath
            && myAsset->getAssetName() == myAssetName
      )
    {
        return;
    }

    destroyAsset();

    myAsset = new Asset(myOTLFilePath, myAssetName, thisMObject());

    MFnDependencyNode assetNodeFn(thisMObject());
    MObject parmAttrObj = assetNodeFn.attribute(Util::getParmAttrPrefix());

    // Restore all the asset's parameter values
    MDataBlock dataBlock = forceCache();
    if(!parmAttrObj.isNull())
    {
        myAsset->setParmValues(
                dataBlock,
                assetNodeFn,
                NULL
                );
    }
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
