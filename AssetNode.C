#include <maya/MFnGenericAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MDataHandle.h>
#include <maya/MFnDependencyNode.h>
#if MAYA_API_VERSION >= 201600
#include <maya/MEvaluationNode.h>
#include <maya/MEvaluationNodeIterator.h>
#endif
#include <maya/MFileIO.h>
#include <maya/MFileObject.h>
#include <maya/MTime.h>
#include <maya/MGlobal.h>
#include <maya/MModelMessage.h>
#include <maya/MPlugArray.h>
#include <maya/MAnimControl.h>

#include "Asset.h"
#include "Input.h"
#include "AssetNode.h"
#include "MayaTypeID.h"
#include "util.h"

#include <cassert>

MString AssetNode::typeName("houdiniAsset");
MTypeId AssetNode::typeId(MayaTypeID_HoudiniAssetNode);

MObject AssetNode::inTime;

MObject AssetNode::otlFilePath;
MObject AssetNode::assetName;
MObject AssetNode::assetConnectType;
MObject AssetNode::postSyncCallback;
MObject AssetNode::preSyncCallback;


MObject AssetNode::input;
MObject AssetNode::inputName;
MObject AssetNode::inputNodeId;

MObject AssetNode::output;

MObject AssetNode::outputAssetTransform;
MObject AssetNode::outputAssetTranslate;
MObject AssetNode::outputAssetTranslateX;
MObject AssetNode::outputAssetTranslateY;
MObject AssetNode::outputAssetTranslateZ;
MObject AssetNode::outputAssetRotate;
MObject AssetNode::outputAssetRotateX;
MObject AssetNode::outputAssetRotateY;
MObject AssetNode::outputAssetRotateZ;
MObject AssetNode::outputAssetScale;
MObject AssetNode::outputAssetScaleX;
MObject AssetNode::outputAssetScaleY;
MObject AssetNode::outputAssetScaleZ;

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
MObject AssetNode::outputGeoName;
MObject AssetNode::outputGeoIsTemplated;
MObject AssetNode::outputGeoIsDisplayGeo;

MObject AssetNode::outputParts;
MObject AssetNode::outputPartName;
MObject AssetNode::outputPartHasMesh;
MObject AssetNode::outputPartHasParticles;
MObject AssetNode::outputPartHasInstancer;

MObject AssetNode::outputPartMesh;
MObject AssetNode::outputPartMeshCurrentColorSet;
MObject AssetNode::outputPartMeshCurrentUV;
MObject AssetNode::outputPartMeshData;

MObject AssetNode::outputPartParticle;
MObject AssetNode::outputPartParticleCurrentTime;
MObject AssetNode::outputPartParticlePositions;
MObject AssetNode::outputPartParticleArrayData;

MObject AssetNode::outputPartCurves;
MObject AssetNode::outputPartCurvesIsBezier;

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

MObject AssetNode::outputPartInstancer;
MObject AssetNode::outputPartInstancerArrayData;
MObject AssetNode::outputPartInstancerParts;

MObject AssetNode::outputPartInstancerTransform;
MObject AssetNode::outputPartInstancerTranslate;
MObject AssetNode::outputPartInstancerTranslateX;
MObject AssetNode::outputPartInstancerTranslateY;
MObject AssetNode::outputPartInstancerTranslateZ;
MObject AssetNode::outputPartInstancerRotate;
MObject AssetNode::outputPartInstancerRotateX;
MObject AssetNode::outputPartInstancerRotateY;
MObject AssetNode::outputPartInstancerRotateZ;
MObject AssetNode::outputPartInstancerScale;
MObject AssetNode::outputPartInstancerScaleX;
MObject AssetNode::outputPartInstancerScaleY;
MObject AssetNode::outputPartInstancerScaleZ;

MObject AssetNode::outputPartMaterialIds;

MObject AssetNode::outputPartExtraAttributes;
MObject AssetNode::outputPartExtraAttributeName;
MObject AssetNode::outputPartExtraAttributeOwner;
MObject AssetNode::outputPartExtraAttributeDataType;
MObject AssetNode::outputPartExtraAttributeTuple;
MObject AssetNode::outputPartExtraAttributeData;

MObject AssetNode::outputPartGroups;
MObject AssetNode::outputPartGroupName;
MObject AssetNode::outputPartGroupType;
MObject AssetNode::outputPartGroupMembers;

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

MObject AssetNode::outputMaterials;
MObject AssetNode::outputMaterialPath;
MObject AssetNode::outputMaterialName;
MObject AssetNode::outputMaterialNodeId;
MObject AssetNode::outputMaterialTexturePath;
MObject AssetNode::outputMaterialAmbientColor;
MObject AssetNode::outputMaterialDiffuseColor;
MObject AssetNode::outputMaterialSpecularColor;
MObject AssetNode::outputMaterialAlphaColor;

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
    MFnGenericAttribute gAttr;
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;
    MFnCompoundAttribute cAttr;
    MFnUnitAttribute uAttr;
    MFnMessageAttribute mAttr;

    // time input
    // For time dpendence.
    AssetNode::inTime = uAttr.create(
            "inTime", "inTime",
            MTime()
            );
    uAttr.setStorable(true);
    uAttr.setHidden(true);

    // otl file path
    AssetNode::otlFilePath = tAttr.create(
            "otlFilePath", "otlFilePath",
            MFnData::kString
            );
    tAttr.setInternal(true);
    tAttr.setUsedAsFilename(true);

    // asset name
    AssetNode::assetName = tAttr.create(
            "assetName", "assetName",
            MFnData::kString
            );
    tAttr.setInternal(true);
    

    AssetNode::postSyncCallback = tAttr.create(
            "postSyncCallback", "postSyncCallback",
            MFnData::kString
            );
    tAttr.setStorable(true);
  
    AssetNode::preSyncCallback = tAttr.create(
            "preSyncCallback", "preSyncCallback",
            MFnData::kString
            );
    tAttr.setStorable(true);
    
    // asset usage type: classic = 0, history = 1, bake = 2
    AssetNode::assetConnectType = nAttr.create(
            "assetConnectType", "assetConnectType",
            MFnNumericData::kInt,
            0
            );
    nAttr.setCached(false);
    nAttr.setStorable(true);
    nAttr.setDisconnectBehavior(MFnAttribute::kReset);

    // input
    AssetNode::inputName = tAttr.create(
            "inputName", "inputName",
            MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    AssetNode::inputNodeId = nAttr.create(
            "inputNodeId", "inputNodeId",
            MFnNumericData::kInt,
            -1
            );
    nAttr.setCached(false);
    nAttr.setStorable(false);
    nAttr.setDisconnectBehavior(MFnAttribute::kReset);

    AssetNode::input = cAttr.create(
            "input", "input"
            );
    cAttr.addChild(AssetNode::inputName);
    cAttr.addChild(AssetNode::inputNodeId);
    cAttr.setArray(true);
    cAttr.setUsesArrayDataBuilder(true);

    // output

    // translate
    AssetNode::outputAssetTranslateX = uAttr.create(
            "outputAssetTranslateX", "outputAssetTranslateX",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputAssetTranslateY = uAttr.create(
            "outputAssetTranslateY", "outputAssetTranslateY",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputAssetTranslateZ = uAttr.create(
            "outputAssetTranslateZ", "outputAssetTranslateZ",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputAssetTranslate = nAttr.create(
            "outputAssetTranslate", "outputAssetTranslate",
            AssetNode::outputAssetTranslateX,
            AssetNode::outputAssetTranslateY,
            AssetNode::outputAssetTranslateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // rotate
    AssetNode::outputAssetRotateX = uAttr.create(
            "outputAssetRotateX", "outputAssetRotateX",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputAssetRotateY = uAttr.create(
            "outputAssetRotateY", "outputAssetRotateY",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputAssetRotateZ = uAttr.create(
            "outputAssetRotateZ", "outputAssetRotateZ",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputAssetRotate = nAttr.create(
            "outputAssetRotate", "outputAssetRotate",
            AssetNode::outputAssetRotateX,
            AssetNode::outputAssetRotateY,
            AssetNode::outputAssetRotateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // scale
    AssetNode::outputAssetScaleX = nAttr.create(
            "outputAssetScaleX", "outputAssetScaleX",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::outputAssetScaleY = nAttr.create(
            "outputAssetScaleY", "outputAssetScaleY",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::outputAssetScaleZ = nAttr.create(
            "outputAssetScaleZ", "outputAssetScaleZ",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::outputAssetScale = nAttr.create(
            "outputAssetScale", "outputAssetScale",
            AssetNode::outputAssetScaleX,
            AssetNode::outputAssetScaleY,
            AssetNode::outputAssetScaleZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // transform
    AssetNode::outputAssetTransform = cAttr.create(
            "outputAssetTransform", "outputAssetTransform"
            );
    cAttr.addChild(AssetNode::outputAssetTranslate);
    cAttr.addChild(AssetNode::outputAssetRotate);
    cAttr.addChild(AssetNode::outputAssetScale);
    cAttr.setWritable(false);
    cAttr.setStorable(false);

    //----------------------------------  instancer compound multi----------------------------------------------
    // instancer data
    AssetNode::outputInstancerData = tAttr.create(
            "outputInstancerData", "outputInstancerData",
            MFnData::kDynArrayAttrs
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    // instanced object names
    AssetNode::outputInstancedObjectNames = tAttr.create(
            "outputInstancedObjectNames", "outputInstancedObjectNames",
            MFnData::kString
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    tAttr.setArray(true);
    tAttr.setIndexMatters(true);
    cAttr.setUsesArrayDataBuilder(true);

    // houdini instance attribute
    AssetNode::outputHoudiniInstanceAttribute = tAttr.create(
            "outputHoudiniInstanceAttribute", "outputHoudiniInstanceAttribute",
            MFnData::kString
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    tAttr.setArray(true);
    tAttr.setIndexMatters(true);
    cAttr.setUsesArrayDataBuilder(true);

    // houdini name attribute
    AssetNode::outputHoudiniNameAttribute = tAttr.create(
            "outputHoudiniNameAttribute", "outputHoudiniNameAttribute",
            MFnData::kString
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    tAttr.setArray(true);
    tAttr.setIndexMatters(true);
    cAttr.setUsesArrayDataBuilder(true);

    // translate
    AssetNode::outputInstanceTranslateX = uAttr.create(
            "outputInstanceTranslateX", "outputInstanceTranslateX",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputInstanceTranslateY = uAttr.create(
            "outputInstanceTranslateY", "outputInstanceTranslateY",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputInstanceTranslateZ = uAttr.create(
            "outputInstanceTranslateZ", "outputInstanceTranslateZ",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputInstanceTranslate = nAttr.create(
            "outputInstanceTranslate", "outputInstanceTranslate",
            AssetNode::outputInstanceTranslateX,
            AssetNode::outputInstanceTranslateY,
            AssetNode::outputInstanceTranslateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // rotate
    AssetNode::outputInstanceRotateX = uAttr.create(
            "outputInstanceRotateX", "outputInstanceRotateX",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputInstanceRotateY = uAttr.create(
            "outputInstanceRotateY", "outputInstanceRotateY",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputInstanceRotateZ = uAttr.create(
            "outputInstanceRotateZ", "outputInstanceRotateZ",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputInstanceRotate = nAttr.create(
            "outputInstanceRotate", "outputInstanceRotate",
            AssetNode::outputInstanceRotateX,
            AssetNode::outputInstanceRotateY,
            AssetNode::outputInstanceRotateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // scale
    AssetNode::outputInstanceScaleX = nAttr.create(
            "outputInstanceScaleX", "outputInstanceScaleX",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::outputInstanceScaleY = nAttr.create(
            "outputInstanceScaleY", "outputInstanceScaleY",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::outputInstanceScaleZ = nAttr.create(
            "outputInstanceScaleZ", "outputInstanceScaleZ",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::outputInstanceScale = nAttr.create(
            "outputInstanceScale", "outputInstanceScale",
            AssetNode::outputInstanceScaleX,
            AssetNode::outputInstanceScaleY,
            AssetNode::outputInstanceScaleZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // transform
    AssetNode::outputInstanceTransform = cAttr.create(
            "outputInstanceTransform", "outputInstanceTransform"
            );
    cAttr.addChild(AssetNode::outputInstanceTranslate);
    cAttr.addChild(AssetNode::outputInstanceRotate);
    cAttr.addChild(AssetNode::outputInstanceScale);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    cAttr.setArray(true);
    cAttr.setUsesArrayDataBuilder(true);

    // instancers
    AssetNode::outputInstancers = cAttr.create(
            "outputInstancers", "outputInstancers"
            );
    cAttr.addChild(AssetNode::outputInstancerData);
    cAttr.addChild(AssetNode::outputInstancedObjectNames);
    cAttr.addChild(AssetNode::outputHoudiniInstanceAttribute);
    cAttr.addChild(AssetNode::outputHoudiniNameAttribute);
    cAttr.addChild(AssetNode::outputInstanceTransform);
    cAttr.setStorable(false);
    cAttr.setWritable(false);
    cAttr.setArray(true);
    cAttr.setUsesArrayDataBuilder(true);

    //--------------------------------End instancer compound multi----------------------------------------------

    //----------------------------------  objects compound multi------------------------------------------------

    // translate
    AssetNode::outputObjectTranslateX = uAttr.create(
            "outputObjectTranslateX", "outputObjectTranslateX",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputObjectTranslateY = uAttr.create(
            "outputObjectTranslateY", "outputObjectTranslateY",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputObjectTranslateZ = uAttr.create(
            "outputObjectTranslateZ", "outputObjectTranslateZ",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputObjectTranslate = nAttr.create(
            "outputObjectTranslate", "outputObjectTranslate",
            AssetNode::outputObjectTranslateX,
            AssetNode::outputObjectTranslateY,
            AssetNode::outputObjectTranslateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // rotate
    AssetNode::outputObjectRotateX = uAttr.create(
            "outputObjectRotateX", "outputObjectRotateX",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputObjectRotateY = uAttr.create(
            "outputObjectRotateY", "outputObjectRotateY",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputObjectRotateZ = uAttr.create(
            "outputObjectRotateZ", "outputObjectRotateZ",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputObjectRotate = nAttr.create(
            "outputObjectRotate", "outputObjectRotate",
            AssetNode::outputObjectRotateX,
            AssetNode::outputObjectRotateY,
            AssetNode::outputObjectRotateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // scale
    AssetNode::outputObjectScaleX = nAttr.create(
            "outputObjectScaleX", "outputObjectScaleX",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::outputObjectScaleY = nAttr.create(
            "outputObjectScaleY", "outputObjectScaleY",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::outputObjectScaleZ = nAttr.create(
            "outputObjectScaleZ", "outputObjectScaleZ",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::outputObjectScale = nAttr.create(
            "outputObjectScale", "outputObjectScale",
            AssetNode::outputObjectScaleX,
            AssetNode::outputObjectScaleY,
            AssetNode::outputObjectScaleZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // transform
    AssetNode::outputObjectTransform = cAttr.create(
            "outputObjectTransform", "outputObjectTransform"
            );
    cAttr.addChild(AssetNode::outputObjectTranslate);
    cAttr.addChild(AssetNode::outputObjectRotate);
    cAttr.addChild(AssetNode::outputObjectScale);
    cAttr.setWritable(false);
    cAttr.setStorable(false);

#if MAYA_API_VERSION >= 201400
    // object fluid from asset
    AssetNode::outputObjectFluidFromAsset = nAttr.create(
            "outputObjectFluidFromAsset", "outputObjectFluidFromAsset",
            MFnNumericData::kBoolean,
            true
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
#endif

    // meta data
    AssetNode::outputObjectMetaData = nAttr.create(
            "outputObjectMetaData", "outputObjectMetaData",
            MFnNumericData::kInt
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // part name
    AssetNode::outputPartName = tAttr.create(
            "outputPartName", "outputPartName",
            MFnData::kString
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    AssetNode::outputPartHasMesh = nAttr.create(
            "outputPartHasMesh", "outputPartHasMesh",
            MFnNumericData::kBoolean,
            false
            );

    AssetNode::outputPartHasParticles = nAttr.create(
            "outputPartHasParticles", "outputPartHasParticles",
            MFnNumericData::kBoolean,
            false
            );

    AssetNode::outputPartHasInstancer = nAttr.create(
            "outputPartHasInstancer", "outputPartHasInstancer",
            MFnNumericData::kBoolean,
            false
            );

    // mesh
    AssetNode::outputPartMeshCurrentColorSet = tAttr.create(
            "outputPartMeshCurrentColorSet", "outputPartMeshCurrentColorSet",
            MFnData::kString
            );
    tAttr.setWritable(false);
    tAttr.setStorable(false);

    AssetNode::outputPartMeshCurrentUV = tAttr.create(
            "outputPartMeshCurrentUV", "outputPartMeshCurrentUV",
            MFnData::kString
            );
    tAttr.setWritable(false);
    tAttr.setStorable(false);

    AssetNode::outputPartMeshData = tAttr.create(
            "outputPartMeshData", "outputPartMeshData",
            MFnData::kMesh
            );
    tAttr.setWritable(false);
    tAttr.setStorable(false);

    AssetNode::outputPartMesh = cAttr.create(
            "outputPartMesh", "outputPartMesh"
            );
    cAttr.addChild(AssetNode::outputPartMeshCurrentColorSet);
    cAttr.addChild(AssetNode::outputPartMeshCurrentUV);
    cAttr.addChild(AssetNode::outputPartMeshData);
    cAttr.setWritable(false);
    cAttr.setStorable(false);

    // particle
    AssetNode::outputPartParticleCurrentTime = uAttr.create(
            "outputPartParticleCurrentTime", "outputPartParticleCurrentTime",
            MFnUnitAttribute::kTime
            );
    uAttr.setWritable(false);
    uAttr.setStorable(false);

    AssetNode::outputPartParticlePositions = tAttr.create(
            "outputPartParticlePositions", "outputPartParticlePositions",
            MFnData::kVectorArray
            );
    tAttr.setWritable(false);
    tAttr.setStorable(false);

    AssetNode::outputPartParticleArrayData = tAttr.create(
            "outputPartParticleArrayData", "outputPartParticleArrayData",
            MFnData::kDynArrayAttrs
            );
    tAttr.setWritable(false);
    tAttr.setStorable(false);

    AssetNode::outputPartParticle = cAttr.create(
            "outputPartParticle", "outputPartParticle"
            );
    cAttr.addChild(AssetNode::outputPartParticleCurrentTime);
    cAttr.addChild(AssetNode::outputPartParticlePositions);
    cAttr.addChild(AssetNode::outputPartParticleArrayData);
    cAttr.setWritable(false);
    cAttr.setStorable(false);

    // curves
    AssetNode::outputPartCurves = tAttr.create(
            "outputPartCurves", "outputPartCurves",
            MFnData::kNurbsCurve
            );
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    tAttr.setArray(true);
    tAttr.setIndexMatters(true);
    tAttr.setUsesArrayDataBuilder(true);

    AssetNode::outputPartCurvesIsBezier = nAttr.create(
            "outputPartCurvesIsBezier", "outputPartCurvesIsBezier",
            MFnNumericData::kBoolean,
            false
            );
    nAttr.setWritable(false);
    nAttr.setStorable(false);

#if MAYA_API_VERSION >= 201400
    // Volumes ---------
    AssetNode::outputPartVolumeName = tAttr.create(
            "outputPartVolumeName", "outputPartVolumeName",
            MFnData::kString
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    AssetNode::outputPartVolumeGrid = tAttr.create(
            "outputPartVolumeGrid", "outputPartVolumeGrid",
            MFnData::kFloatArray
            );
    tAttr.setWritable(false);
    tAttr.setStorable(false);

    // Volume resolution
    AssetNode::outputPartVolumeRes = tAttr.create(
            "outputPartVolumeRes", "outputPartVolumeRes",
            MFnData::kFloatArray
            );
    nAttr.setWritable(false);
    nAttr.setStorable(false);

    // volume transform
    // translate
    AssetNode::outputPartVolumeTranslateX = uAttr.create(
            "outputPartVolumeTranslateX", "outputPartVolumeTranslateX",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputPartVolumeTranslateY = uAttr.create(
            "outputPartVolumeTranslateY", "outputPartVolumeTranslateY",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputPartVolumeTranslateZ = uAttr.create(
            "outputPartVolumeTranslateZ", "outputPartVolumeTranslateZ",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputPartVolumeTranslate = nAttr.create(
            "outputPartVolumeTranslate", "outputPartVolumeTranslate",
            AssetNode::outputPartVolumeTranslateX,
            AssetNode::outputPartVolumeTranslateY,
            AssetNode::outputPartVolumeTranslateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // rotate
    AssetNode::outputPartVolumeRotateX = uAttr.create(
            "outputPartVolumeRotateX", "outputPartVolumeRotateX",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputPartVolumeRotateY = uAttr.create(
            "outputPartVolumeRotateY", "outputPartVolumeRotateY",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputPartVolumeRotateZ = uAttr.create(
            "outputPartVolumeRotateZ", "outputPartVolumeRotateZ",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputPartVolumeRotate = nAttr.create(
            "outputPartVolumeRotate", "outputPartVolumeRotate",
            AssetNode::outputPartVolumeRotateX,
            AssetNode::outputPartVolumeRotateY,
            AssetNode::outputPartVolumeRotateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // scale
    AssetNode::outputPartVolumeScaleX = nAttr.create(
            "outputPartVolumeScaleX", "outputPartVolumeScaleX",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::outputPartVolumeScaleY = nAttr.create(
            "outputPartVolumeScaleY", "outputPartVolumeScaleY",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::outputPartVolumeScaleZ = nAttr.create(
            "outputPartVolumeScaleZ", "outputPartVolumeScaleZ",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::outputPartVolumeScale = nAttr.create(
            "outputPartVolumeScale", "outputPartVolumeScale",
            AssetNode::outputPartVolumeScaleX,
            AssetNode::outputPartVolumeScaleY,
            AssetNode::outputPartVolumeScaleZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::outputPartVolumeTransform = cAttr.create(
            "outputPartVolumeTransform", "outputPartVolumeTransform"
            );
    cAttr.addChild(AssetNode::outputPartVolumeTranslate);
    cAttr.addChild(AssetNode::outputPartVolumeRotate);
    cAttr.addChild(AssetNode::outputPartVolumeScale);
    cAttr.setWritable(false);
    cAttr.setStorable(false);

    // volume
    AssetNode::outputPartVolume = cAttr.create(
            "outputPartVolume", "outputPartVolume"
            );
    cAttr.addChild(AssetNode::outputPartVolumeName);
    cAttr.addChild(AssetNode::outputPartVolumeGrid);
    cAttr.addChild(AssetNode::outputPartVolumeTransform);
    cAttr.addChild(AssetNode::outputPartVolumeRes);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
#endif

    // instancer
    AssetNode::outputPartInstancerArrayData = tAttr.create(
            "outputPartInstancerArrayData", "outputPartInstancerArrayData",
            MFnData::kDynArrayAttrs
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    AssetNode::outputPartInstancerParts = tAttr.create(
            "outputPartInstancerParts", "outputPartInstancerParts",
            MFnData::kIntArray
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    AssetNode::outputPartInstancerTranslateX = uAttr.create(
            "outputPartInstancerTranslateX", "outputPartInstancerTranslateX",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputPartInstancerTranslateY = uAttr.create(
            "outputPartInstancerTranslateY", "outputPartInstancerTranslateY",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputPartInstancerTranslateZ = uAttr.create(
            "outputPartInstancerTranslateZ", "outputPartInstancerTranslateZ",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputPartInstancerTranslate = nAttr.create(
            "outputPartInstancerTranslate", "outputPartInstancerTranslate",
            AssetNode::outputPartInstancerTranslateX,
            AssetNode::outputPartInstancerTranslateY,
            AssetNode::outputPartInstancerTranslateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // rotate
    AssetNode::outputPartInstancerRotateX = uAttr.create(
            "outputPartInstancerRotateX", "outputPartInstancerRotateX",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputPartInstancerRotateY = uAttr.create(
            "outputPartInstancerRotateY", "outputPartInstancerRotateY",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputPartInstancerRotateZ = uAttr.create(
            "outputPartInstancerRotateZ", "outputPartInstancerRotateZ",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    AssetNode::outputPartInstancerRotate = nAttr.create(
            "outputPartInstancerRotate", "outputPartInstancerRotate",
            AssetNode::outputPartInstancerRotateX,
            AssetNode::outputPartInstancerRotateY,
            AssetNode::outputPartInstancerRotateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // scale
    AssetNode::outputPartInstancerScaleX = nAttr.create(
            "outputPartInstancerScaleX", "outputPartInstancerScaleX",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::outputPartInstancerScaleY = nAttr.create(
            "outputPartInstancerScaleY", "outputPartInstancerScaleY",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::outputPartInstancerScaleZ = nAttr.create(
            "outputPartInstancerScaleZ", "outputPartInstancerScaleZ",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::outputPartInstancerScale = nAttr.create(
            "outputPartInstancerScale", "outputPartInstancerScale",
            AssetNode::outputPartInstancerScaleX,
            AssetNode::outputPartInstancerScaleY,
            AssetNode::outputPartInstancerScaleZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    // transform
    AssetNode::outputPartInstancerTransform = cAttr.create(
            "outputPartInstancerTransform", "outputPartInstancerTransform"
            );
    cAttr.addChild(AssetNode::outputPartInstancerTranslate);
    cAttr.addChild(AssetNode::outputPartInstancerRotate);
    cAttr.addChild(AssetNode::outputPartInstancerScale);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    cAttr.setArray(true);
    cAttr.setUsesArrayDataBuilder(true);

    AssetNode::outputPartInstancer = cAttr.create(
            "outputPartInstancer", "outputPartInstancer"
            );
    cAttr.addChild(AssetNode::outputPartInstancerArrayData);
    cAttr.addChild(AssetNode::outputPartInstancerParts);
    cAttr.addChild(AssetNode::outputPartInstancerTransform);
    cAttr.setWritable(false);
    cAttr.setStorable(false);

    // material id
    AssetNode::outputPartMaterialIds = tAttr.create(
            "outputPartMaterialIds", "outputPartMaterialIds",
            MFnData::kIntArray
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    // extra attributes
    AssetNode::outputPartExtraAttributeName = tAttr.create(
            "outputPartExtraAttributeName", "outputPartExtraAttributeName",
            MFnData::kString
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    AssetNode::outputPartExtraAttributeOwner = tAttr.create(
            "outputPartExtraAttributeOwner", "outputPartExtraAttributeOwner",
            MFnData::kString
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    AssetNode::outputPartExtraAttributeDataType = tAttr.create(
            "outputPartExtraAttributeDataType", "outputPartExtraAttributeDataType",
            MFnData::kString
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    AssetNode::outputPartExtraAttributeTuple = nAttr.create(
            "outputPartExtraAttributeTuple", "outputPartExtraAttributeTuple",
            MFnNumericData::kInt,
            0.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    AssetNode::outputPartExtraAttributeData = gAttr.create(
            "outputPartExtraAttributeData", "outputPartExtraAttributeData"
            );
    // float
    gAttr.addNumericDataAccept(MFnNumericData::kFloat);
    gAttr.addNumericDataAccept(MFnNumericData::k2Float);
    gAttr.addNumericDataAccept(MFnNumericData::k3Float);
    gAttr.addDataAccept(MFnData::kFloatArray);
    // double
    gAttr.addNumericDataAccept(MFnNumericData::kDouble);
    gAttr.addNumericDataAccept(MFnNumericData::k2Double);
    gAttr.addNumericDataAccept(MFnNumericData::k3Double);
    gAttr.addNumericDataAccept(MFnNumericData::k4Double);
    gAttr.addDataAccept(MFnData::kDoubleArray);
    gAttr.addDataAccept(MFnData::kVectorArray);
    gAttr.addDataAccept(MFnData::kPointArray);
    // int
    gAttr.addNumericDataAccept(MFnNumericData::kInt);
    gAttr.addNumericDataAccept(MFnNumericData::k2Int);
    gAttr.addNumericDataAccept(MFnNumericData::k3Int);
    gAttr.addDataAccept(MFnData::kIntArray);
    // string
    gAttr.addDataAccept(MFnData::kString);
    gAttr.addDataAccept(MFnData::kStringArray);
    gAttr.setStorable(false);
    gAttr.setWritable(false);
    AssetNode::outputPartExtraAttributes = cAttr.create(
            "outputPartExtraAttributes", "outputPartExtraAttributes"
            );
    cAttr.addChild(AssetNode::outputPartExtraAttributeName);
    cAttr.addChild(AssetNode::outputPartExtraAttributeOwner);
    cAttr.addChild(AssetNode::outputPartExtraAttributeDataType);
    cAttr.addChild(AssetNode::outputPartExtraAttributeTuple);
    cAttr.addChild(AssetNode::outputPartExtraAttributeData);
    cAttr.setStorable(false);
    cAttr.setWritable(false);
    cAttr.setArray(true);
    cAttr.setUsesArrayDataBuilder(true);

    AssetNode::outputPartGroupName = tAttr.create(
            "outputPartGroupName", "outputPartGroupName",
            MFnData::kString
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    AssetNode::outputPartGroupType = nAttr.create(
            "outputPartGroupType", "outputPartGroupType",
            MFnNumericData::kInt
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    AssetNode::outputPartGroupMembers = tAttr.create(
            "outputPartGroupMembers", "outputPartGroupMembers",
            MFnData::kIntArray
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    AssetNode::outputPartGroups = cAttr.create(
            "outputPartGroups", "outputPartGroups"
            );
    cAttr.addChild(AssetNode::outputPartGroupName);
    cAttr.addChild(AssetNode::outputPartGroupType);
    cAttr.addChild(AssetNode::outputPartGroupMembers);
    cAttr.setStorable(false);
    cAttr.setWritable(false);
    cAttr.setArray(true);
    cAttr.setUsesArrayDataBuilder(true);

    AssetNode::outputParts = cAttr.create(
            "outputParts", "outputParts"
            );
    cAttr.addChild(AssetNode::outputPartName);
    cAttr.addChild(AssetNode::outputPartHasMesh);
    cAttr.addChild(AssetNode::outputPartHasParticles);
    cAttr.addChild(AssetNode::outputPartHasInstancer);
    cAttr.addChild(AssetNode::outputPartMesh);
    cAttr.addChild(AssetNode::outputPartParticle);
    cAttr.addChild(AssetNode::outputPartCurves);
    cAttr.addChild(AssetNode::outputPartCurvesIsBezier);
    cAttr.addChild(AssetNode::outputPartInstancer);
    cAttr.addChild(AssetNode::outputPartMaterialIds);
    cAttr.addChild(AssetNode::outputPartExtraAttributes);
    cAttr.addChild(AssetNode::outputPartGroups);
    
#if MAYA_API_VERSION >= 201400
    cAttr.addChild(AssetNode::outputPartVolume);
#endif
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    cAttr.setArray(true);
    cAttr.setIndexMatters(true);
    cAttr.setUsesArrayDataBuilder(true);

    // output geos
    AssetNode::outputGeoName = tAttr.create(
            "outputGeoName", "outputGeoName",
            MFnData::kString
            );
    tAttr.setWritable(false);
    tAttr.setStorable(false);

    AssetNode::outputGeoIsTemplated = nAttr.create(
            "outputGeoIsTemplated", "outputGeoIsTemplated",
            MFnNumericData::kBoolean,
            false
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    AssetNode::outputGeoIsDisplayGeo = nAttr.create(
            "outputGeoIsDisplayGeo", "outputGeoIsDisplayGeo",
            MFnNumericData::kBoolean,
            false
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    AssetNode::outputGeos = cAttr.create(
            "outputGeos", "outputGeos"
            );
    cAttr.addChild(AssetNode::outputGeoName);
    cAttr.addChild(AssetNode::outputGeoIsTemplated);
    cAttr.addChild(AssetNode::outputGeoIsDisplayGeo);
    cAttr.addChild(AssetNode::outputParts);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    cAttr.setArray(true);
    cAttr.setIndexMatters(true);
    cAttr.setUsesArrayDataBuilder(true);

    AssetNode::outputVisibility = nAttr.create(
            "outputVisibility", "outputVisibility",
            MFnNumericData::kBoolean,
            false
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    AssetNode::outputIsInstanced = nAttr.create(
            "outputIsInstanced", "outputIsInstanced",
            MFnNumericData::kBoolean,
            false
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    AssetNode::outputObjectName = tAttr.create(
            "outputObjectName", "outputObjectName",
            MFnData::kString
            );
    tAttr.setWritable(false);
    tAttr.setStorable(false);

    AssetNode::outputObjects = cAttr.create(
            "outputObjects", "outputObjects"
            );
    cAttr.addChild(AssetNode::outputObjectMetaData);
#if MAYA_API_VERSION >= 201400
    cAttr.addChild(AssetNode::outputObjectFluidFromAsset);
#endif
    cAttr.addChild(AssetNode::outputGeos);
    cAttr.addChild(AssetNode::outputObjectName);
    cAttr.addChild(AssetNode::outputObjectTransform);
    cAttr.addChild(AssetNode::outputVisibility);
    cAttr.addChild(AssetNode::outputIsInstanced);

    cAttr.setWritable(false);
    cAttr.setStorable(false);
    cAttr.setArray(true);
    cAttr.setIndexMatters(true);
    cAttr.setUsesArrayDataBuilder(true);

    //------------------------------- END  objects compound multi------------------------------------------------

    // material path
    AssetNode::outputMaterialPath = tAttr.create(
            "outputMaterialPath", "outputMaterialPath",
            MFnData::kString
            );
    // material name
    AssetNode::outputMaterialName = tAttr.create(
            "outputMaterialName", "outputMaterialName",
            MFnData::kString
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    // material node id
    AssetNode::outputMaterialNodeId = nAttr.create(
            "outputMaterialNodeId", "outputMaterialNodeId",
            MFnNumericData::kInt
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    // material ambient
    AssetNode::outputMaterialAmbientColor = nAttr.createColor(
            "outputMaterialAmbientColor", "outputMaterialAmbientColor"
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    // material diffuse
    AssetNode::outputMaterialDiffuseColor = nAttr.createColor(
            "outputMaterialDiffuseColor", "outputMaterialDiffuseColor"
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    // material specular
    AssetNode::outputMaterialSpecularColor = nAttr.createColor(
            "outputMaterialSpecularColor", "outputMaterialSpecularColor"
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    // material alpha
    AssetNode::outputMaterialAlphaColor = nAttr.createColor(
            "outputMaterialAlphaColor", "outputMaterialAlphaColor"
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    // texture path
    AssetNode::outputMaterialTexturePath = tAttr.create(
            "outputMaterialTexturePath", "outputMaterialTexturePath",
            MFnData::kString
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    // material
    AssetNode::outputMaterials = cAttr.create(
            "outputMaterials", "outputMaterials"
            );
    cAttr.addChild(AssetNode::outputMaterialPath);
    cAttr.addChild(AssetNode::outputMaterialName);
    cAttr.addChild(AssetNode::outputMaterialNodeId);
    cAttr.addChild(AssetNode::outputMaterialAmbientColor);
    cAttr.addChild(AssetNode::outputMaterialDiffuseColor);
    cAttr.addChild(AssetNode::outputMaterialSpecularColor);
    cAttr.addChild(AssetNode::outputMaterialAlphaColor);
    cAttr.addChild(AssetNode::outputMaterialTexturePath);
    cAttr.setArray(true);

    // output
    AssetNode::output = cAttr.create(
            "output", "out"
            );
    cAttr.addChild(AssetNode::outputAssetTransform);
    cAttr.addChild(AssetNode::outputObjects);
    cAttr.addChild(AssetNode::outputInstancers);
    cAttr.addChild(AssetNode::outputMaterials);

    assetNodeOptionsDefinition.addAttributes();

    // add the static attributes to the node
    addAttribute(AssetNode::inTime);
    addAttribute(AssetNode::otlFilePath);
    addAttribute(AssetNode::assetName);
    addAttribute(AssetNode::assetConnectType);
    addAttribute(AssetNode::postSyncCallback);
    addAttribute(AssetNode::preSyncCallback);
    addAttribute(AssetNode::input);
    addAttribute(AssetNode::output);

    //most of the dependencies between attrs are set via the AssetNode::setDependentsDirty() call
    //this call may not even be necessary.
    attributeAffects(AssetNode::otlFilePath, AssetNode::output);
    attributeAffects(AssetNode::assetName, AssetNode::output);

    attributeAffects(AssetNode::input, AssetNode::output);

    return MS::kSuccess;
}

void
AssetNode::nodeAdded(MObject& node,void *clientData)
{
    if(MGlobal::isUndoing())
    {
        AssetNode* assetNode = static_cast<AssetNode*>(clientData);
        assetNode->createAsset();
    }
}

void
AssetNode::nodeRemoved(MObject& node,void *clientData)
{
    AssetNode* assetNode = static_cast<AssetNode*>(clientData);
    assetNode->destroyAsset();
}

AssetNode::AssetNode() :
    myNeedToMarshalInput(false),
    myAutoSyncId(-1),
    myExtraAutoSync(false)
{
    myAsset = NULL;

    // If we just loaded this node from a file, then push all the parameter
    // values. We can't simply determine this from myDirtyParmAttributes,
    // which is set by setDependentsDirty().  This is because when the asset
    // node is first created, we want to pull all the parameter values from
    // the asset, but myDirtyParmAttributes is also empty.
    mySetAllParms = MFileIO::isOpeningFile()
        || MFileIO::isImportingFile()
        || MFileIO::isReferencingFile();
}

AssetNode::~AssetNode()
{
    destroyAsset();
}

void
AssetNode::postConstructor()
{
    MObject object = thisMObject();

    MModelMessage::addNodeAddedToModelCallback(
            object,
            AssetNode::nodeAdded,
            this
            );
    MModelMessage::addNodeRemovedFromModelCallback(
            object,
            AssetNode::nodeRemoved,
            this
            );
}

MStatus
AssetNode::setDependentsDirty(const MPlug& plugBeingDirtied,
        MPlugArray& affectedPlugs)
{
    MStatus status;
    bool isTime = plugBeingDirtied == inTime;
    bool isInput = Util::isPlugBelow(plugBeingDirtied, AssetNode::input);
    bool isParameter = false; 
    {
        MFnDependencyNode assetNodeFn(thisMObject());
        MObject parmAttrObj = assetNodeFn.attribute(Util::getParmAttrPrefix(), &status);
        isParameter = Util::isPlugBelow(plugBeingDirtied, parmAttrObj);
    }
    bool isMaterialPath = plugBeingDirtied == outputMaterialPath;

    if(isParameter)
    {
        myDirtyParmAttributes.push_back(plugBeingDirtied.attribute());

        // This catches when an instance being removed. Since we always remove
        // the last instance from the mulitparm, we need to shuffle all the
        // values. So we mark them all dirty.
        {
            MPlug rampPlug;
            if(plugBeingDirtied.isElement())
            {
                MPlug arrayPlug = plugBeingDirtied.array();
                if(Util::endsWith(arrayPlug.name(), "__ramp"))
                {
                    rampPlug = arrayPlug;
                }

                if(!rampPlug.isNull())
                {
                    for(unsigned int i = 0; i < arrayPlug.numElements(); i++)
                    {
                        MPlug elemPlug = arrayPlug.elementByPhysicalIndex(i);
                        myDirtyParmAttributes.push_back(elemPlug.child(0));
                        myDirtyParmAttributes.push_back(elemPlug.child(1));
                        myDirtyParmAttributes.push_back(elemPlug.child(2));
                    }
                }
            }
        }
        // This catches when an instance is added. The ramp plug needs to be
        // marked dirty so that the we can catch it in pushMultiparm().
        {
            MPlug plug;
            MString attrName;
            if(plugBeingDirtied.isChild())
            {
                MPlug parentPlug = plugBeingDirtied.parent();
                if(parentPlug.isElement())
                {
                    plug = parentPlug.array();
                    attrName = plug.name();
                }
            }

            if(Util::endsWith(attrName, "__ramp"))
            {
                myDirtyParmAttributes.push_back(plug);
            }
        }
    }

    if(isInput)
    {
        myNeedToMarshalInput = true;
    }

    // Changing time or parameters will dirty the output
    if(isTime || isInput || isParameter)
    {
        Util::getChildPlugs(
                affectedPlugs,
                MPlug(thisMObject(), AssetNode::output)
                );
    }

    // Changing outputMaterialPath will dirty outputMaterials[i]
    if(isMaterialPath)
    {
        Util::getChildPlugs(
                affectedPlugs,
                plugBeingDirtied.parent()
                );
    }
    // if autoSyncOutputs got into a bad no-output state
    // and we're doing dirty propagation outside of playback
    // and it's an attr that would affect the outputs
    // sync to see if this change actually produces outputs
    if((isInput || isParameter) && myExtraAutoSync) {
        if(!MAnimControl::isPlaying()) {
            MDataBlock data = forceCache();
            AssetNodeOptions::AccessorDataBlock options(assetNodeOptionsDefinition, data);
	    if(options.autoSyncOutputs())
            {
	        myAutoSyncId++;
	        MGlobal::executeCommandOnIdle("houdiniEngine_autoSyncAssetOutput "
	            + MFnDagNode(thisMObject()).fullPathName() + " "
	            + myAutoSyncId);
            }
        }
    }
    return MS::kSuccess;
}

#if MAYA_API_VERSION >= 201600
MStatus
AssetNode::preEvaluation(
        const MDGContext& context,
        const MEvaluationNode& evaluationNode
        )
{
    MFnDependencyNode assetNodeFn(thisMObject());
    MObject parmAttrObj = assetNodeFn.attribute(Util::getParmAttrPrefix());
    if(parmAttrObj.isNull())
    {
        return MStatus::kSuccess;
    }

    for(MEvaluationNodeIterator nodeIt = evaluationNode.iterator();
            !nodeIt.isDone();
            nodeIt.next())
    {
        if(Util::isPlugBelow(nodeIt.plug(), parmAttrObj))
        {
            myDirtyParmAttributes.push_back(nodeIt.plug().attribute());
        }

        if(Util::isPlugBelow(nodeIt.plug(), AssetNode::input))
        {
            myNeedToMarshalInput = true;
        }
    }

    return MStatus::kSuccess;
}
#endif

void
AssetNode::rebuildAsset()
{
    destroyAsset();

    createAsset();
}

void
AssetNode::setParmValues(bool onlyDirtyParms)
{
    MDataBlock data = forceCache();
    setParmValues(data, onlyDirtyParms);
}

void
AssetNode::getParmValues()
{
    MDataBlock data = forceCache();
    getParmValues(data);
}

MStatus
AssetNode::compute(const MPlug& plug, MDataBlock& data)
{
    MStatus status;

    // We don't want to compute outputMaterialPath. This could be triggered
    // from SyncOutputMaterial::createOutputMaterialPlug() when trying to set
    // the value through MDGModifier.
    if(plug == outputMaterialPath)
    {
        return MPxTransform::compute(plug, data);
    }

    if(Util::isPlugBelow(plug, MPlug(thisMObject(), AssetNode::output)))
    {
        // make sure asset was created properly
        if(!isAssetValid())
        {
            DISPLAY_ERROR("^1s: Could not instantiate asset: ^2s\n"
                    "in OTL file: ^3s\n",
                    name(),
                    myAssetName,
                    myOTLFilePath);

            return MStatus::kFailure;
        }

        // Set the time
        MDataHandle inTimeHandle = data.inputValue(AssetNode::inTime);
        MTime mayaTime = inTimeHandle.asTime();
        myAsset->setTime(mayaTime);

        // push the inputs to Houdini, such as transforms and geometries
        if(myNeedToMarshalInput)
        {
            myNeedToMarshalInput = false;

            MPlug inputPlug(thisMObject(), AssetNode::input);
            myAsset->setInputs(inputPlug, data);
        }

        setParmValues(data);

        AssetNodeOptions::AccessorDataBlock options(assetNodeOptionsDefinition, data);

        MPlug outputPlug(thisMObject(), AssetNode::output);
        bool needToSyncOutputs = false;
        status = myAsset->compute(
                outputPlug,
                data,
                options,
                needToSyncOutputs
                );

	// this gets parm properties as well as values
	// do this after the compute in case stuff like disable has changed
	// or expressions have been evaulated
	getParmValues(data);
	
        // No need to print error messages from Asset::compute(). It should
        // have been printed already.
        if(MFAIL(status))
        {
            return status;
        }

        if(options.autoSyncOutputs() && needToSyncOutputs)
        {
            myAutoSyncId++;
            MGlobal::executeCommandOnIdle("houdiniEngine_autoSyncAssetOutput "
                    + MFnDagNode(thisMObject()).fullPathName() + " "
                    + myAutoSyncId);
        }

        data.setClean(plug);
        return MStatus::kSuccess;
    }

    return MPxTransform::compute(plug, data);
}

void
AssetNode::setExtraAutoSync(bool needs)
{
    // when autoSyncOutputs has left your node in an outputless state
    // indicate that if we change a param that affects outputs
    // we should sync immediately if autoSyncOutputs is on
    // If an output materializes, so much the better,
    // otherwise, at least we won't have to delete or creates any nodes
    myExtraAutoSync = needs;
}


#if MAYA_API_VERSION >= 201800
bool
AssetNode::getInternalValue(
        const MPlug &plug,
        MDataHandle &dataHandle)
#else
bool
AssetNode::getInternalValueInContext(
        const MPlug &plug,
        MDataHandle &dataHandle,
        MDGContext &ctx)
#endif
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

#if MAYA_API_VERSION >= 201800
    return MPxTransform::getInternalValue(plug, dataHandle);
#else
    return MPxTransform::getInternalValueInContext(plug, dataHandle, ctx);
#endif
}

#if MAYA_API_VERSION >= 201800
bool
AssetNode::setInternalValue(
        const MPlug &plug,
        const MDataHandle &dataHandle)
#else
bool
AssetNode::setInternalValueInContext(
        const MPlug &plug,
        const MDataHandle &dataHandle,
        MDGContext &ctx
        )
#endif
{
    MStatus status;
    if(plug == AssetNode::otlFilePath
            || plug == AssetNode::assetName)
    {
        if(plug == AssetNode::otlFilePath)
        {
            myOTLFilePath = dataHandle.asString();
        }
        else if(plug == AssetNode::assetName)
        {
            myAssetName = dataHandle.asString();
        }

        // Create the Asset object as early as possible. We may need it before
        // the first compute. For example, Maya may call internalArrayCount.
        rebuildAsset();

        return true;
    }

#if MAYA_API_VERSION >= 201800
    return MPxTransform::setInternalValue(plug, dataHandle);
#else
    return MPxTransform::setInternalValueInContext(plug, dataHandle, ctx);
#endif
}

#if MAYA_API_VERSION >= 201800
int
AssetNode::internalArrayCount(const MPlug &plug) const
#else
int
AssetNode::internalArrayCount(const MPlug &plug, const MDGContext &ctx) const
#endif
{
    if(plug == AssetNode::input)
    {
        if(!isAssetValid())
        {
            return 0;
        }

        return getAsset()->getAssetInfo().geoInputCount;
    }

#if MAYA_API_VERSION >= 201800
    return MPxTransform::internalArrayCount(plug);
#else
    return MPxTransform::internalArrayCount(plug, ctx);
#endif
}

void
AssetNode::copyInternalData(MPxNode* node)
{
    MStatus status;

    AssetNode* assetNode = dynamic_cast<AssetNode*>(node);

    myOTLFilePath = assetNode->myOTLFilePath;
    myAssetName = assetNode->myAssetName;

    rebuildAsset();

    MPxTransform::copyInternalData(node);
}
MStatus
AssetNode:: shouldSave(
		const MPlug & plug,
		bool & isSaving )
{
    // Since we went to all the trouble to make sure that inputs (nodeIds) were storable
    // to get around the numeric compound file save optimization (bug),
    // and that the attrs get reset to default on disconnect so we don't have bogus inputs,
    // just want to make sure that default values also get stored
    // In Maya2018 it looks like default values in multi compounds are written anyway
    // but I don't want to take my chances with older versions, so make sure they  get written
   
    int nameSize = plug.name().numChars();
    MString tail = plug.name().substring(nameSize - 6, nameSize - 1);
    if(tail == "__node") {
        isSaving = true;
        return MS::kSuccess;
    } else {
        return MPxNode::shouldSave( plug, isSaving );
    }
}
MStatus
AssetNode::connectionBroken (
    const MPlug & plug,
    const MPlug & otherPlug,
    bool asSrc )
{
    // setting the disconnect behavior to reset doesn't seem to work for elements of multi compounds
    // and we particularly want to make sure that it is correct for node id's
    // since they are now storable, so we do a brute force reset to -1 in that case
  
    int nameSize = plug.name().numChars();
    MString tail = plug.name().substring(nameSize - 6, nameSize - 1);
    if(tail == "__node") {
        MPlug ncPlug( plug);
        ncPlug.setValue(-1);
    }
    return(MPxTransform::connectionBroken(plug,otherPlug, asSrc));
}


Asset*
AssetNode::getAsset() const
{
    return myAsset;
}

bool
AssetNode::isAssetValid() const
{
    return getAsset() != NULL
        && getAsset()->getAssetName() == myAssetName;
}

void
AssetNode::createAsset()
{
    assert(!myAsset);

    MStatus status;

    if(isAssetValid())
    {
        return;
    }

    // Make sure the asset name is set before trying to create the asset.
    if(!myAssetName.length())
    {
        return;
    }

    MFileObject file;
    file.setRawFullName(myOTLFilePath);
    myAsset = new Asset(file.resolvedFullName(), myAssetName);

    if(!myAsset->isValid())
    {
        destroyAsset();
        return;
    }

    // We want to setParmValues() here because the state of the asset should be
    // restored to what it was before rebuildAsset() was called. This is
    // particularly important during a sync, because we rely on getParmValues()
    // to take care of restoring the parameters onto the newly created
    // houdiniAssetParm.
    // However, during a file load, setInternalValueInContext() calls
    // rebuildAsset(), and we must not setParmValues(), because the parameter
    // values haven't been restored form the file yet.
    if(!(MFileIO::isOpeningFile()
                || MFileIO::isImportingFile()
                || MFileIO::isReferencingFile()))
    {
        MDataBlock data = forceCache();
        setParmValues(data, false);
    }

    myNeedToMarshalInput = true;
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

void
AssetNode::setParmValues(MDataBlock &data, bool onlyDirtyParms)
{
    MStatus status;

    if(!isAssetValid())
    {
        return;
    }

    MFnDagNode assetNodeFn(thisMObject(), &status);
    CHECK_MSTATUS(status);

    MObject parmAttrObj = assetNodeFn.attribute(
            Util::getParmAttrPrefix(), &status);
    if(parmAttrObj.isNull())
    {
        return;
    }

    MObjectVector cache = myDirtyParmAttributes;
    MObjectVector* attrs = &cache;
    myDirtyParmAttributes.clear();

    if(!onlyDirtyParms || mySetAllParms)
    {
        mySetAllParms = false;

        attrs = NULL;
    }

    myAsset->fillParmNameCache();

    myAsset->setParmValues(
            data,
            assetNodeFn,
            attrs
            );
}

void
AssetNode::getParmValues(MDataBlock &data)
{
    MStatus status;

    if(!isAssetValid())
    {
        return;
    }

    MFnDagNode assetNodeFn(thisMObject(), &status);
    CHECK_MSTATUS(status);

    MObject parmAttrObj = assetNodeFn.attribute(
            Util::getParmAttrPrefix(), &status);
    if(parmAttrObj.isNull())
    {
        return;
    }

    myAsset->getParmValues(
            data,
            assetNodeFn,
            NULL
            );
}
