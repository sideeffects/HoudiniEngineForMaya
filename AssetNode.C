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
#include <maya/MTime.h>
#include <maya/MGlobal.h>
#include <maya/MModelMessage.h>
#include <maya/MPlugArray.h>

#include <algorithm>

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

MObject AssetNode::syncWhenInputConnects;
MObject AssetNode::autoSyncOutputs;
MObject AssetNode::splitGeosByGroup;
MObject AssetNode::outputHiddenObjects;
MObject AssetNode::outputTemplatedGeometries;
MObject AssetNode::useAssetObjectTransform;

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
MObject AssetNode::outputPartMaterial;
MObject AssetNode::outputPartMaterialExists;
MObject AssetNode::outputPartMaterialName;
MObject AssetNode::outputPartTexturePath;
MObject AssetNode::outputPartAmbientColor;
MObject AssetNode::outputPartDiffuseColor;
MObject AssetNode::outputPartSpecularColor;
MObject AssetNode::outputPartAlphaColor;

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

MObject AssetNode::useInstancerNode;

std::vector<MObject> computeAttributes;

template <typename T>
static bool
isPlugBelow(const MPlug &plug, const T &upper)
{
    MPlug currentPlug = plug;

    for(;;)
    {
        if(currentPlug == upper)
        {
            return true;
        }

        if(currentPlug.isChild())
        {
            currentPlug = currentPlug.parent();
        }
        else if(currentPlug.isElement())
        {
            currentPlug = currentPlug.array();
        }
        else
        {
            break;
        }
    }

    return false;
}

static void
getChildPlugs(MPlugArray &plugArray, const MPlug &plug)
{
    std::vector<MPlug> plugsToTraverse;
    plugsToTraverse.push_back(plug);

    while(plugsToTraverse.size())
    {
        const MPlug currentPlug = plugsToTraverse.back();
        plugsToTraverse.pop_back();

        plugArray.append(currentPlug);

        if(currentPlug.isArray())
        {
            for(unsigned int i = 0; i < currentPlug.numElements(); i++)
            {
                plugsToTraverse.push_back(currentPlug.elementByPhysicalIndex(i));
            }
        }
        else if(currentPlug.isCompound())
        {
            for(unsigned int i = 0; i < currentPlug.numChildren(); i++)
            {
                plugsToTraverse.push_back(currentPlug.child(i));
            }
        }
    }
}

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

    AssetNode::syncWhenInputConnects = nAttr.create(
            "syncWhenInputConnects", "syncWhenInputConnects",
            MFnNumericData::kBoolean,
            true
            );
    AssetNode::autoSyncOutputs = nAttr.create(
            "autoSyncOutputs", "autoSyncOutputs",
            MFnNumericData::kBoolean
            );
    AssetNode::splitGeosByGroup = nAttr.create(
            "splitGeosByGroup", "splitGeosByGroup",
            MFnNumericData::kBoolean
            );
    AssetNode::outputHiddenObjects = nAttr.create(
            "outputHiddenObjects", "outputHiddenObjects",
            MFnNumericData::kBoolean
            );
    AssetNode::outputTemplatedGeometries = nAttr.create(
            "outputTemplatedGeometries", "outputTemplatedGeometries",
            MFnNumericData::kBoolean
            );

    AssetNode::useAssetObjectTransform = nAttr.create(
            "useAssetObjectTransform", "useAssetObjectTransform",
            MFnNumericData::kBoolean
            );

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
    computeAttributes.push_back(AssetNode::outputAssetTranslateX);
    AssetNode::outputAssetTranslateY = uAttr.create(
            "outputAssetTranslateY", "outputAssetTranslateY",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputAssetTranslateY);
    AssetNode::outputAssetTranslateZ = uAttr.create(
            "outputAssetTranslateZ", "outputAssetTranslateZ",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputAssetTranslateZ);
    AssetNode::outputAssetTranslate = nAttr.create(
            "outputAssetTranslate", "outputAssetTranslate",
            AssetNode::outputAssetTranslateX,
            AssetNode::outputAssetTranslateY,
            AssetNode::outputAssetTranslateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputAssetTranslate);

    // rotate
    AssetNode::outputAssetRotateX = uAttr.create(
            "outputAssetRotateX", "outputAssetRotateX",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputAssetRotateX);
    AssetNode::outputAssetRotateY = uAttr.create(
            "outputAssetRotateY", "outputAssetRotateY",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputAssetRotateY);
    AssetNode::outputAssetRotateZ = uAttr.create(
            "outputAssetRotateZ", "outputAssetRotateZ",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputAssetRotateZ);
    AssetNode::outputAssetRotate = nAttr.create(
            "outputAssetRotate", "outputAssetRotate",
            AssetNode::outputAssetRotateX,
            AssetNode::outputAssetRotateY,
            AssetNode::outputAssetRotateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputAssetRotate);

    // scale
    AssetNode::outputAssetScaleX = nAttr.create(
            "outputAssetScaleX", "outputAssetScaleX",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputAssetScaleX);
    AssetNode::outputAssetScaleY = nAttr.create(
            "outputAssetScaleY", "outputAssetScaleY",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputAssetScaleY);
    AssetNode::outputAssetScaleZ = nAttr.create(
            "outputAssetScaleZ", "outputAssetScaleZ",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputAssetScaleZ);
    AssetNode::outputAssetScale = nAttr.create(
            "outputAssetScale", "outputAssetScale",
            AssetNode::outputAssetScaleX,
            AssetNode::outputAssetScaleY,
            AssetNode::outputAssetScaleZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputAssetScale);

    // transform
    AssetNode::outputAssetTransform = cAttr.create(
            "outputAssetTransform", "outputAssetTransform"
            );
    cAttr.addChild(AssetNode::outputAssetTranslate);
    cAttr.addChild(AssetNode::outputAssetRotate);
    cAttr.addChild(AssetNode::outputAssetScale);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputAssetTransform);

    //----------------------------------  instancer compound multi----------------------------------------------
    // instancer data
    AssetNode::outputInstancerData = tAttr.create(
            "outputInstancerData", "outputInstancerData",
            MFnData::kDynArrayAttrs
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstancerData);

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
    computeAttributes.push_back(AssetNode::outputInstancedObjectNames);

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
    computeAttributes.push_back(AssetNode::outputHoudiniInstanceAttribute);

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
    computeAttributes.push_back(AssetNode::outputHoudiniNameAttribute);

    // translate
    AssetNode::outputInstanceTranslateX = uAttr.create(
            "outputInstanceTranslateX", "outputInstanceTranslateX",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceTranslateX);
    AssetNode::outputInstanceTranslateY = uAttr.create(
            "outputInstanceTranslateY", "outputInstanceTranslateY",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceTranslateY);
    AssetNode::outputInstanceTranslateZ = uAttr.create(
            "outputInstanceTranslateZ", "outputInstanceTranslateZ",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceTranslateZ);
    AssetNode::outputInstanceTranslate = nAttr.create(
            "outputInstanceTranslate", "outputInstanceTranslate",
            AssetNode::outputInstanceTranslateX,
            AssetNode::outputInstanceTranslateY,
            AssetNode::outputInstanceTranslateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceTranslate);

    // rotate
    AssetNode::outputInstanceRotateX = uAttr.create(
            "outputInstanceRotateX", "outputInstanceRotateX",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceRotateX);
    AssetNode::outputInstanceRotateY = uAttr.create(
            "outputInstanceRotateY", "outputInstanceRotateY",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceRotateY);
    AssetNode::outputInstanceRotateZ = uAttr.create(
            "outputInstanceRotateZ", "outputInstanceRotateZ",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceRotateZ);
    AssetNode::outputInstanceRotate = nAttr.create(
            "outputInstanceRotate", "outputInstanceRotate",
            AssetNode::outputInstanceRotateX,
            AssetNode::outputInstanceRotateY,
            AssetNode::outputInstanceRotateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceRotate);

    // scale
    AssetNode::outputInstanceScaleX = nAttr.create(
            "outputInstanceScaleX", "outputInstanceScaleX",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceScaleX);
    AssetNode::outputInstanceScaleY = nAttr.create(
            "outputInstanceScaleY", "outputInstanceScaleY",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceScaleY);
    AssetNode::outputInstanceScaleZ = nAttr.create(
            "outputInstanceScaleZ", "outputInstanceScaleZ",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceScaleZ);
    AssetNode::outputInstanceScale = nAttr.create(
            "outputInstanceScale", "outputInstanceScale",
            AssetNode::outputInstanceScaleX,
            AssetNode::outputInstanceScaleY,
            AssetNode::outputInstanceScaleZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputInstanceScale);

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
    computeAttributes.push_back(AssetNode::outputInstanceTransform);

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
    computeAttributes.push_back(AssetNode::outputInstancers);

    //--------------------------------End instancer compound multi----------------------------------------------

    //----------------------------------  objects compound multi------------------------------------------------

    // translate
    AssetNode::outputObjectTranslateX = uAttr.create(
            "outputObjectTranslateX", "outputObjectTranslateX",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectTranslateX);
    AssetNode::outputObjectTranslateY = uAttr.create(
            "outputObjectTranslateY", "outputObjectTranslateY",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectTranslateY);
    AssetNode::outputObjectTranslateZ = uAttr.create(
            "outputObjectTranslateZ", "outputObjectTranslateZ",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectTranslateZ);
    AssetNode::outputObjectTranslate = nAttr.create(
            "outputObjectTranslate", "outputObjectTranslate",
            AssetNode::outputObjectTranslateX,
            AssetNode::outputObjectTranslateY,
            AssetNode::outputObjectTranslateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectTranslate);

    // rotate
    AssetNode::outputObjectRotateX = uAttr.create(
            "outputObjectRotateX", "outputObjectRotateX",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectRotateX);
    AssetNode::outputObjectRotateY = uAttr.create(
            "outputObjectRotateY", "outputObjectRotateY",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectRotateY);
    AssetNode::outputObjectRotateZ = uAttr.create(
            "outputObjectRotateZ", "outputObjectRotateZ",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectRotateZ);
    AssetNode::outputObjectRotate = nAttr.create(
            "outputObjectRotate", "outputObjectRotate",
            AssetNode::outputObjectRotateX,
            AssetNode::outputObjectRotateY,
            AssetNode::outputObjectRotateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectRotate);

    // scale
    AssetNode::outputObjectScaleX = nAttr.create(
            "outputObjectScaleX", "outputObjectScaleX",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectScaleX);
    AssetNode::outputObjectScaleY = nAttr.create(
            "outputObjectScaleY", "outputObjectScaleY",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectScaleY);
    AssetNode::outputObjectScaleZ = nAttr.create(
            "outputObjectScaleZ", "outputObjectScaleZ",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectScaleZ);
    AssetNode::outputObjectScale = nAttr.create(
            "outputObjectScale", "outputObjectScale",
            AssetNode::outputObjectScaleX,
            AssetNode::outputObjectScaleY,
            AssetNode::outputObjectScaleZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectScale);

    // transform
    AssetNode::outputObjectTransform = cAttr.create(
            "outputObjectTransform", "outputObjectTransform"
            );
    cAttr.addChild(AssetNode::outputObjectTranslate);
    cAttr.addChild(AssetNode::outputObjectRotate);
    cAttr.addChild(AssetNode::outputObjectScale);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputObjectTransform);

#if MAYA_API_VERSION >= 201400
    // object fluid from asset
    AssetNode::outputObjectFluidFromAsset = nAttr.create(
            "outputObjectFluidFromAsset", "outputObjectFluidFromAsset",
            MFnNumericData::kBoolean,
            true
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectFluidFromAsset);
#endif

    // meta data
    AssetNode::outputObjectMetaData = nAttr.create(
            "outputObjectMetaData", "outputObjectMetaData",
            MFnNumericData::kInt
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectMetaData);

    // part name
    AssetNode::outputPartName = tAttr.create(
            "outputPartName", "outputPartName",
            MFnData::kString
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartName);

    AssetNode::outputPartHasMesh = nAttr.create(
            "outputPartHasMesh", "outputPartHasMesh",
            MFnNumericData::kBoolean,
            false
            );
    computeAttributes.push_back(AssetNode::outputPartHasMesh);

    AssetNode::outputPartHasParticles = nAttr.create(
            "outputPartHasParticles", "outputPartHasParticles",
            MFnNumericData::kBoolean,
            false
            );
    computeAttributes.push_back(AssetNode::outputPartHasParticles);

    AssetNode::outputPartHasInstancer = nAttr.create(
            "outputPartHasInstancer", "outputPartHasInstancer",
            MFnNumericData::kBoolean,
            false
            );
    computeAttributes.push_back(AssetNode::outputPartHasInstancer);

    // material exists
    AssetNode::outputPartMaterialExists = nAttr.create(
            "outputPartMaterialExists", "outputPartMaterialExists",
            MFnNumericData::kBoolean,
            false
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    nAttr.setConnectable(false);
    nAttr.setHidden(true);
    computeAttributes.push_back(AssetNode::outputPartMaterialExists);
    // material name
    AssetNode::outputPartMaterialName = tAttr.create(
            "outputPartMaterialName", "outputPartMaterialName",
            MFnData::kString
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartMaterialName);
    // material ambient
    AssetNode::outputPartAmbientColor = nAttr.createColor(
            "outputPartAmbientColor", "outputPartAmbientColor"
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartAmbientColor);
    // material diffuse
    AssetNode::outputPartDiffuseColor = nAttr.createColor(
            "outputPartDiffuseColor", "outputPartDiffuseColor"
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartDiffuseColor);
    // material specular
    AssetNode::outputPartSpecularColor = nAttr.createColor(
            "outputPartSpecularColor", "outputPartSpecularColor"
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartSpecularColor);
    // material alpha
    AssetNode::outputPartAlphaColor = nAttr.createColor(
            "outputPartAlphaColor", "outputPartAlphaColor"
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartAlphaColor);
    // texture path
    AssetNode::outputPartTexturePath = tAttr.create(
            "outputPartTexturePath", "outputPartTexturePath",
            MFnData::kString
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartTexturePath);

    // material
    AssetNode::outputPartMaterial = cAttr.create(
            "outputPartMaterial", "outputPartMaterial"
            );
    cAttr.addChild(AssetNode::outputPartMaterialExists);
    cAttr.addChild(AssetNode::outputPartMaterialName);
    cAttr.addChild(AssetNode::outputPartAmbientColor);
    cAttr.addChild(AssetNode::outputPartDiffuseColor);
    cAttr.addChild(AssetNode::outputPartSpecularColor);
    cAttr.addChild(AssetNode::outputPartAlphaColor);
    cAttr.addChild(AssetNode::outputPartTexturePath);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartMaterial);

    // mesh
    AssetNode::outputPartMeshCurrentColorSet = tAttr.create(
            "outputPartMeshCurrentColorSet", "outputPartMeshCurrentColorSet",
            MFnData::kString
            );
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartMeshCurrentColorSet);

    AssetNode::outputPartMeshCurrentUV = tAttr.create(
            "outputPartMeshCurrentUV", "outputPartMeshCurrentUV",
            MFnData::kString
            );
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartMeshCurrentUV);

    AssetNode::outputPartMeshData = tAttr.create(
            "outputPartMeshData", "outputPartMeshData",
            MFnData::kMesh
            );
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartMeshData);

    AssetNode::outputPartMesh = cAttr.create(
            "outputPartMesh", "outputPartMesh"
            );
    cAttr.addChild(AssetNode::outputPartMeshCurrentColorSet);
    cAttr.addChild(AssetNode::outputPartMeshCurrentUV);
    cAttr.addChild(AssetNode::outputPartMeshData);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartMesh);

    // particle
    AssetNode::outputPartParticleCurrentTime = uAttr.create(
            "outputPartParticleCurrentTime", "outputPartParticleCurrentTime",
            MFnUnitAttribute::kTime
            );
    uAttr.setWritable(false);
    uAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartParticleCurrentTime);

    AssetNode::outputPartParticlePositions = tAttr.create(
            "outputPartParticlePositions", "outputPartParticlePositions",
            MFnData::kVectorArray
            );
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartParticlePositions);

    AssetNode::outputPartParticleArrayData = tAttr.create(
            "outputPartParticleArrayData", "outputPartParticleArrayData",
            MFnData::kDynArrayAttrs
            );
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartParticleArrayData);

    AssetNode::outputPartParticle = cAttr.create(
            "outputPartParticle", "outputPartParticle"
            );
    cAttr.addChild(AssetNode::outputPartParticleCurrentTime);
    cAttr.addChild(AssetNode::outputPartParticlePositions);
    cAttr.addChild(AssetNode::outputPartParticleArrayData);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartParticle);

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
    computeAttributes.push_back(AssetNode::outputPartCurves);

    AssetNode::outputPartCurvesIsBezier = nAttr.create(
            "outputPartCurvesIsBezier", "outputPartCurvesIsBezier",
            MFnNumericData::kBoolean,
            false
            );
    nAttr.setWritable(false);
    nAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartCurvesIsBezier);

#if MAYA_API_VERSION >= 201400
    // Volumes ---------
    AssetNode::outputPartVolumeName = tAttr.create(
            "outputPartVolumeName", "outputPartVolumeName",
            MFnData::kString
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeName);

    AssetNode::outputPartVolumeGrid = tAttr.create(
            "outputPartVolumeGrid", "outputPartVolumeGrid",
            MFnData::kFloatArray
            );
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeGrid);

    // Volume resolution
    AssetNode::outputPartVolumeRes = tAttr.create(
            "outputPartVolumeRes", "outputPartVolumeRes",
            MFnData::kFloatArray
            );
    nAttr.setWritable(false);
    nAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeRes);

    // volume transform
    // translate
    AssetNode::outputPartVolumeTranslateX = uAttr.create(
            "outputPartVolumeTranslateX", "outputPartVolumeTranslateX",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeTranslateX);
    AssetNode::outputPartVolumeTranslateY = uAttr.create(
            "outputPartVolumeTranslateY", "outputPartVolumeTranslateY",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeTranslateY);
    AssetNode::outputPartVolumeTranslateZ = uAttr.create(
            "outputPartVolumeTranslateZ", "outputPartVolumeTranslateZ",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeTranslateZ);
    AssetNode::outputPartVolumeTranslate = nAttr.create(
            "outputPartVolumeTranslate", "outputPartVolumeTranslate",
            AssetNode::outputPartVolumeTranslateX,
            AssetNode::outputPartVolumeTranslateY,
            AssetNode::outputPartVolumeTranslateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeTranslate);

    // rotate
    AssetNode::outputPartVolumeRotateX = uAttr.create(
            "outputPartVolumeRotateX", "outputPartVolumeRotateX",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeRotateX);
    AssetNode::outputPartVolumeRotateY = uAttr.create(
            "outputPartVolumeRotateY", "outputPartVolumeRotateY",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeRotateY);
    AssetNode::outputPartVolumeRotateZ = uAttr.create(
            "outputPartVolumeRotateZ", "outputPartVolumeRotateZ",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeRotateZ);
    AssetNode::outputPartVolumeRotate = nAttr.create(
            "outputPartVolumeRotate", "outputPartVolumeRotate",
            AssetNode::outputPartVolumeRotateX,
            AssetNode::outputPartVolumeRotateY,
            AssetNode::outputPartVolumeRotateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeRotate);

    // scale
    AssetNode::outputPartVolumeScaleX = nAttr.create(
            "outputPartVolumeScaleX", "outputPartVolumeScaleX",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeScaleX);
    AssetNode::outputPartVolumeScaleY = nAttr.create(
            "outputPartVolumeScaleY", "outputPartVolumeScaleY",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeScaleY);
    AssetNode::outputPartVolumeScaleZ = nAttr.create(
            "outputPartVolumeScaleZ", "outputPartVolumeScaleZ",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeScaleZ);
    AssetNode::outputPartVolumeScale = nAttr.create(
            "outputPartVolumeScale", "outputPartVolumeScale",
            AssetNode::outputPartVolumeScaleX,
            AssetNode::outputPartVolumeScaleY,
            AssetNode::outputPartVolumeScaleZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputObjectScale);
    AssetNode::outputPartVolumeTransform = cAttr.create(
            "outputPartVolumeTransform", "outputPartVolumeTransform"
            );
    cAttr.addChild(AssetNode::outputPartVolumeTranslate);
    cAttr.addChild(AssetNode::outputPartVolumeRotate);
    cAttr.addChild(AssetNode::outputPartVolumeScale);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartVolumeTransform);

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
    computeAttributes.push_back(AssetNode::outputPartVolume);
#endif

    // instancer
    AssetNode::outputPartInstancerArrayData = tAttr.create(
            "outputPartInstancerArrayData", "outputPartInstancerArrayData",
            MFnData::kDynArrayAttrs
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartInstancerArrayData);

    AssetNode::outputPartInstancerParts = tAttr.create(
            "outputPartInstancerParts", "outputPartInstancerParts",
            MFnData::kIntArray
            );
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartInstancerParts);

    AssetNode::outputPartInstancerTranslateX = uAttr.create(
            "outputPartInstancerTranslateX", "outputPartInstancerTranslateX",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartInstancerTranslateX);
    AssetNode::outputPartInstancerTranslateY = uAttr.create(
            "outputPartInstancerTranslateY", "outputPartInstancerTranslateY",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartInstancerTranslateY);
    AssetNode::outputPartInstancerTranslateZ = uAttr.create(
            "outputPartInstancerTranslateZ", "outputPartInstancerTranslateZ",
            MFnUnitAttribute::kDistance
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartInstancerTranslateZ);
    AssetNode::outputPartInstancerTranslate = nAttr.create(
            "outputPartInstancerTranslate", "outputPartInstancerTranslate",
            AssetNode::outputPartInstancerTranslateX,
            AssetNode::outputPartInstancerTranslateY,
            AssetNode::outputPartInstancerTranslateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartInstancerTranslate);

    // rotate
    AssetNode::outputPartInstancerRotateX = uAttr.create(
            "outputPartInstancerRotateX", "outputPartInstancerRotateX",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartInstancerRotateX);
    AssetNode::outputPartInstancerRotateY = uAttr.create(
            "outputPartInstancerRotateY", "outputPartInstancerRotateY",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartInstancerRotateY);
    AssetNode::outputPartInstancerRotateZ = uAttr.create(
            "outputPartInstancerRotateZ", "outputPartInstancerRotateZ",
            MFnUnitAttribute::kAngle
            );
    uAttr.setStorable(false);
    uAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartInstancerRotateZ);
    AssetNode::outputPartInstancerRotate = nAttr.create(
            "outputPartInstancerRotate", "outputPartInstancerRotate",
            AssetNode::outputPartInstancerRotateX,
            AssetNode::outputPartInstancerRotateY,
            AssetNode::outputPartInstancerRotateZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartInstancerRotate);

    // scale
    AssetNode::outputPartInstancerScaleX = nAttr.create(
            "outputPartInstancerScaleX", "outputPartInstancerScaleX",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartInstancerScaleX);
    AssetNode::outputPartInstancerScaleY = nAttr.create(
            "outputPartInstancerScaleY", "outputPartInstancerScaleY",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartInstancerScaleY);
    AssetNode::outputPartInstancerScaleZ = nAttr.create(
            "outputPartInstancerScaleZ", "outputPartInstancerScaleZ",
            MFnNumericData::kDouble,
            1.0
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartInstancerScaleZ);
    AssetNode::outputPartInstancerScale = nAttr.create(
            "outputPartInstancerScale", "outputPartInstancerScale",
            AssetNode::outputPartInstancerScaleX,
            AssetNode::outputPartInstancerScaleY,
            AssetNode::outputPartInstancerScaleZ
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputPartInstancerScale);

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
    computeAttributes.push_back(AssetNode::outputPartInstancerTransform);

    AssetNode::outputPartInstancer = cAttr.create(
            "outputPartInstancer", "outputPartInstancer"
            );
    cAttr.addChild(AssetNode::outputPartInstancerArrayData);
    cAttr.addChild(AssetNode::outputPartInstancerParts);
    cAttr.addChild(AssetNode::outputPartInstancerTransform);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputPartInstancer);

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
    cAttr.addChild(AssetNode::outputPartMaterial);
    cAttr.addChild(AssetNode::outputPartMesh);
    cAttr.addChild(AssetNode::outputPartParticle);
    cAttr.addChild(AssetNode::outputPartCurves);
    cAttr.addChild(AssetNode::outputPartCurvesIsBezier);
    cAttr.addChild(AssetNode::outputPartInstancer);
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
    computeAttributes.push_back(AssetNode::outputParts);

    // output geos
    AssetNode::outputGeoName = tAttr.create(
            "outputGeoName", "outputGeoName",
            MFnData::kString
            );
    tAttr.setWritable(false);
    tAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::outputGeoName);

    AssetNode::outputGeoIsTemplated = nAttr.create(
            "outputGeoIsTemplated", "outputGeoIsTemplated",
            MFnNumericData::kBoolean,
            false
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputGeoIsTemplated);

    AssetNode::outputGeoIsDisplayGeo = nAttr.create(
            "outputGeoIsDisplayGeo", "outputGeoIsDisplayGeo",
            MFnNumericData::kBoolean,
            false
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);
    computeAttributes.push_back(AssetNode::outputGeoIsDisplayGeo);

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
    computeAttributes.push_back(AssetNode::outputGeos);

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
    computeAttributes.push_back(AssetNode::outputObjectName);

    AssetNode::outputObjects = cAttr.create(
            "outputObjects", "outputObjects"
            );
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
    AssetNode::output = cAttr.create(
            "output", "out"
            );
    cAttr.addChild(AssetNode::outputAssetTransform);
    cAttr.addChild(AssetNode::outputObjects);
    cAttr.addChild(AssetNode::outputInstancers);
    cAttr.setWritable(false);
    cAttr.setStorable(false);
    computeAttributes.push_back(AssetNode::output);

    AssetNode::useInstancerNode = nAttr.create(
            "useInstancerNode", "useInstancerNode",
            MFnNumericData::kBoolean,
            1
            );
    nAttr.setStorable(true);
    nAttr.setWritable(true);

    // add the static attributes to the node
    addAttribute(AssetNode::syncWhenInputConnects);
    addAttribute(AssetNode::autoSyncOutputs);
    addAttribute(AssetNode::splitGeosByGroup);
    addAttribute(AssetNode::outputHiddenObjects);
    addAttribute(AssetNode::outputTemplatedGeometries);
    addAttribute(AssetNode::useAssetObjectTransform);
    addAttribute(AssetNode::inTime);
    addAttribute(AssetNode::otlFilePath);
    addAttribute(AssetNode::assetName);
    addAttribute(AssetNode::input);
    addAttribute(AssetNode::output);
    addAttribute(AssetNode::useInstancerNode);

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

        // If a node was removed from the scene, all the parms need to be set
        // again.
        assetNode->mySetAllParms = true;
    }
}

void
AssetNode::nodeRemoved(MObject& node,void *clientData)
{
    AssetNode* assetNode = static_cast<AssetNode*>(clientData);
    assetNode->destroyAsset();
}

AssetNode::AssetNode() :
    myNeedToMarshalInput(false)
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

    myResultsClean = false;
}

AssetNode::~AssetNode()
{
    destroyAsset();
}

void
AssetNode::postConstructor()
{
    MModelMessage::addNodeAddedToModelCallback(
            thisMObject(),
            AssetNode::nodeAdded,
            this
            );
    MModelMessage::addNodeRemovedFromModelCallback(
            thisMObject(),
            AssetNode::nodeRemoved,
            this
            );
}

MStatus
AssetNode::setDependentsDirty(const MPlug& plugBeingDirtied,
        MPlugArray& affectedPlugs)
{
    MStatus status;

    if(plugBeingDirtied == AssetNode::otlFilePath
        || plugBeingDirtied == AssetNode::assetName)
    {
        // When the otl path or the asset name is changed, we need to push all
        // the parameter values. Otherwise, the current values on the node
        // would be replaced by the asset's default. This is similar to the
        // case where the asset was just loaded back from file.
        // Also, we need to check isAssetValid(). Otherwise, when an asset node
        // is newly created, we would clobber the default parameter values.
        if(isAssetValid())
        {
            mySetAllParms = true;
        }

        return MS::kSuccess;
    }

    myResultsClean = false;

    MFnDependencyNode assetNodeFn(thisMObject());
    MObject parmAttrObj = assetNodeFn.attribute(Util::getParmAttrPrefix(), &status);
    if(isPlugBelow(plugBeingDirtied, parmAttrObj))
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

    if(isPlugBelow(plugBeingDirtied, AssetNode::input))
    {
        myNeedToMarshalInput = true;
    }

    getChildPlugs(
            affectedPlugs,
            MPlug(thisMObject(), AssetNode::output)
            );

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
        myResultsClean = false;

        if(isPlugBelow(nodeIt.plug(), parmAttrObj))
        {
            myDirtyParmAttributes.push_back(nodeIt.plug().attribute());
        }

        if(isPlugBelow(nodeIt.plug(), AssetNode::input))
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

MStatus
AssetNode::compute(const MPlug& plug, MDataBlock& data)
{
    MStatus status;

    if(std::find(computeAttributes.begin(), computeAttributes.end(), plug)
        != computeAttributes.end() && !myResultsClean)
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

        myResultsClean = true;

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

        MFnDagNode assetNodeFn(thisMObject());
        MObject parmAttrObj = assetNodeFn.attribute(Util::getParmAttrPrefix(), &status);

        //push parms into Houdini
        if(!parmAttrObj.isNull())
        {
            MObjectVector* attrs = &myDirtyParmAttributes;

            if(mySetAllParms)
            {
                mySetAllParms = false;

                attrs = NULL;
            }

            myAsset->setParmValues(
                    data,
                    assetNodeFn,
                    attrs
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

        bool autoSyncOutputs = data
            .inputValue(AssetNode::autoSyncOutputs).asBool();
        bool splitGeosByGroup = data
            .inputValue(AssetNode::splitGeosByGroup).asBool();
        bool outputTemplatedGeometries = data
            .inputValue(AssetNode::outputTemplatedGeometries).asBool();
        bool useInstancerNode = data
            .inputValue(AssetNode::useInstancerNode).asBool();

        MPlug outputPlug(thisMObject(), AssetNode::output);
        bool needToSyncOutputs = false;
        status = myAsset->compute(
                outputPlug,
                data,
                splitGeosByGroup,
                outputTemplatedGeometries,
                useInstancerNode,
                needToSyncOutputs
                );
        // No need to print error messages from Asset::compute(). It should
        // have been printed already.
        if(MFAIL(status))
        {
            return status;
        }

        if(autoSyncOutputs && needToSyncOutputs)
        {
            MGlobal::executeCommandOnIdle("houdiniEngine_syncAssetOutput " + assetNodeFn.fullPathName());
        }

        data.setClean(plug);
        return MStatus::kSuccess;
    }

    return MPxTransform::compute(plug, data);
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

        // When the AssetNode is first created, setInternalValueInContext()
        // will be called with default values. But we don't want to try to
        // create the Asset at that time, since it'll result in errors that's
        // not caused by the user. So make sure the asset name is at least set
        // to avoid getting false errors.
        if(myAssetName.length())
        {
            rebuildAsset();
        }

        return true;
    }

    return MPxTransform::setInternalValueInContext(plug, dataHandle, ctx);
}

int
AssetNode::internalArrayCount(const MPlug &plug, const MDGContext &ctx) const
{
    if(plug == AssetNode::input)
    {
        if(!isAssetValid())
        {
            return 0;
        }

        return getAsset()->getAssetInfo().geoInputCount;
    }

    return MPxTransform::internalArrayCount(plug, ctx);
}

void
AssetNode::copyInternalData(MPxNode* node)
{
    MStatus status;

    AssetNode* assetNode = dynamic_cast<AssetNode*>(node);

    myOTLFilePath = assetNode->myOTLFilePath;
    myAssetName = assetNode->myAssetName;

    rebuildAsset();

    MFnDependencyNode assetNodeFn(thisMObject());
    MObject parmAttrObj = assetNodeFn.attribute(Util::getParmAttrPrefix(), &status);

    // Push all the parameter values to Houdini.
    if(!parmAttrObj.isNull() && isAssetValid())
    {
        MDataBlock dataBlock = forceCache();

        getAsset()->setParmValues(
                dataBlock,
                assetNodeFn,
                NULL
                );
    }

    MPxTransform::copyInternalData(node);
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


    myAsset = new Asset(myOTLFilePath, myAssetName, thisMObject());

    if(!myAsset->isValid())
    {
        destroyAsset();
        return;
    }

    // When createAsset() is called during file load, parameter values aren't
    // loaded yet. So we can't restore the parameter values here.

    myNeedToMarshalInput = true;

    myResultsClean = false;
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
