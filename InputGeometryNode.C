#include "InputGeometryNode.h"

#include <maya/MFnComponentListData.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include "Input.h"
#include "MayaTypeID.h"

MString InputGeometryNode::typeName("houdiniInputGeometry");
MTypeId InputGeometryNode::typeId(MayaTypeID_HoudiniInputGeometryNode);

MObject InputGeometryNode::inputTransform;
MObject InputGeometryNode::inputGeometry;
MObject InputGeometryNode::inputComponents;
MObject InputGeometryNode::pointComponentGroup;
MObject InputGeometryNode::primComponentGroup;
MObject InputGeometryNode::unlockNormals;
MObject InputGeometryNode::materialPerFace;
MObject InputGeometryNode::allowFacetSet;
MObject InputGeometryNode::preserveScale;
MObject InputGeometryNode::ignoreTransform;
MObject InputGeometryNode::objectShadingGroup;
MObject InputGeometryNode::outputNodeId;

void *
InputGeometryNode::creator()
{
    return new InputGeometryNode();
}

MStatus
InputGeometryNode::initialize()
{
    MFnGenericAttribute gAttr;
    MFnMatrixAttribute mAttr;
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;

    InputGeometryNode::inputTransform = mAttr.create(
        "inputTransform", "inputTransform");
    addAttribute(InputGeometryNode::inputTransform);

    InputGeometryNode::inputGeometry = gAttr.create(
        "inputGeometry", "inputGeometry");
    gAttr.addDataAccept(MFnData::kIntArray);
    gAttr.addDataAccept(MFnData::kMesh);
    gAttr.addDataAccept(MFnData::kNurbsCurve);
    gAttr.addDataAccept(MFnData::kVectorArray);
    gAttr.setCached(false);
    gAttr.setStorable(false);
    addAttribute(InputGeometryNode::inputGeometry);

    InputGeometryNode::inputComponents = tAttr.create(
        "inputComponents", "inputComponents",
        MFnComponentListData::kComponentList);
    tAttr.setCached(false);
    tAttr.setStorable(true);
    tAttr.setDisconnectBehavior(MFnAttribute::kReset);
    addAttribute(InputGeometryNode::inputComponents);

    InputGeometryNode::pointComponentGroup = tAttr.create(
        "pointComponentGroup", "pointComponentGroup", MFnData::kString);
    nAttr.setCached(false);
    nAttr.setStorable(true);
    addAttribute(InputGeometryNode::pointComponentGroup);

    InputGeometryNode::primComponentGroup = tAttr.create(
        "primComponentGroup", "primComponentGroup", MFnData::kString);
    nAttr.setCached(false);
    nAttr.setStorable(true);
    addAttribute(InputGeometryNode::primComponentGroup);

    InputGeometryNode::unlockNormals = nAttr.create(
        "unlockNormals", "unlockNormals", MFnNumericData::kBoolean, 0);
    nAttr.setCached(false);
    nAttr.setStorable(true);
    addAttribute(InputGeometryNode::unlockNormals);

    InputGeometryNode::materialPerFace = nAttr.create(
        "materialPerFace", "materialPerFace", MFnNumericData::kBoolean, 0);
    nAttr.setCached(false);
    nAttr.setStorable(true);
    addAttribute(InputGeometryNode::materialPerFace);

    InputGeometryNode::allowFacetSet = nAttr.create(
        "allowFacetSet", "allowFacetSet", MFnNumericData::kBoolean, 0);
    nAttr.setCached(false);
    nAttr.setStorable(true);
    addAttribute(InputGeometryNode::allowFacetSet);

    InputGeometryNode::preserveScale = nAttr.create(
        "preserveScale", "preserveScale", MFnNumericData::kBoolean, false);
    nAttr.setCached(false);
    nAttr.setStorable(true);
    addAttribute(InputGeometryNode::preserveScale);

    InputGeometryNode::ignoreTransform = nAttr.create(
        "ignoreTransform", "ignoreTransform", MFnNumericData::kBoolean, false);
    nAttr.setCached(false);
    nAttr.setStorable(true);
    addAttribute(InputGeometryNode::ignoreTransform);

    InputGeometryNode::objectShadingGroup = tAttr.create(
        "objectShadingGroup", "objectShadingGroup", MFnData::kString);
    nAttr.setCached(false);
    nAttr.setStorable(true);
    addAttribute(InputGeometryNode::objectShadingGroup);

    InputGeometryNode::outputNodeId = nAttr.create(
        "outputNodeId", "outputNodeId", MFnNumericData::kInt, -1);
    nAttr.setCached(false);
    nAttr.setStorable(false);
    addAttribute(InputGeometryNode::outputNodeId);

    attributeAffects(
        InputGeometryNode::inputTransform, InputGeometryNode::outputNodeId);
    attributeAffects(
        InputGeometryNode::inputGeometry, InputGeometryNode::outputNodeId);
    attributeAffects(
        InputGeometryNode::inputComponents, InputGeometryNode::outputNodeId);
    attributeAffects(
        InputGeometryNode::primComponentGroup, InputGeometryNode::outputNodeId);
    attributeAffects(InputGeometryNode::pointComponentGroup,
                     InputGeometryNode::outputNodeId);
    attributeAffects(
        InputGeometryNode::unlockNormals, InputGeometryNode::outputNodeId);
    attributeAffects(
        InputGeometryNode::materialPerFace, InputGeometryNode::outputNodeId);
    attributeAffects(
        InputGeometryNode::allowFacetSet, InputGeometryNode::outputNodeId);
    attributeAffects(
        InputGeometryNode::preserveScale, InputGeometryNode::outputNodeId);
    attributeAffects(
        InputGeometryNode::ignoreTransform, InputGeometryNode::outputNodeId);
    attributeAffects(
        InputGeometryNode::objectShadingGroup, InputGeometryNode::outputNodeId);

    return MStatus::kSuccess;
}

MStatus
InputGeometryNode::compute(const MPlug &plug, MDataBlock &dataBlock)
{
    if (plug == InputGeometryNode::outputNodeId)
    {
        MDataHandle outputNodeIdHandle =
            dataBlock.outputValue(InputGeometryNode::outputNodeId);

        if (!checkInput(dataBlock))
        {
            outputNodeIdHandle.setInt(-1);

            return MStatus::kFailure;
        }

        // this must be set before the call to setInputTransform if the scale is
        // to be preserved properly
        MPlug preserveScalePlug(
            thisMObject(), InputGeometryNode::preserveScale);
        bool preserveScale = preserveScalePlug.asBool();
        myInput->setPreserveScale(preserveScale);

        MPlug ignoreTransformPlug(
            thisMObject(), InputGeometryNode::ignoreTransform);
        bool ignoreTransform = ignoreTransformPlug.asBool();

        if (!ignoreTransform)
        {
            // set input transform
            MPlug transformPlug(
                thisMObject(), InputGeometryNode::inputTransform);
            MDataHandle transformHandle = dataBlock.inputValue(transformPlug);
            myInput->setInputTransform(transformHandle);
        }

        // set input geo
        MPlug geometryPlug(thisMObject(), InputGeometryNode::inputGeometry);
        MPlug normalPlug(thisMObject(), InputGeometryNode::unlockNormals);
        MPlug matPerFacePlug(thisMObject(), InputGeometryNode::materialPerFace);
        MPlug allowFacetSetPlug(
            thisMObject(), InputGeometryNode::allowFacetSet);
        bool unlockNormals = normalPlug.asBool();
        bool matPerFace    = matPerFacePlug.asBool();
        bool allowFacetSet = allowFacetSetPlug.asBool();
        myInput->setUnlockNormals(unlockNormals);
        myInput->setMatPerFace(matPerFace);
        myInput->setAllowFacetSet(allowFacetSet);
        myInput->setInputGeo(dataBlock, geometryPlug);

        // set input component list
        MPlug complistPlug(thisMObject(), InputGeometryNode::inputComponents);
        MPlug primGroupPlug(
            thisMObject(), InputGeometryNode::primComponentGroup);
        MPlug pointGroupPlug(
            thisMObject(), InputGeometryNode::pointComponentGroup);
        myInput->setInputComponents(dataBlock, geometryPlug, complistPlug,
                                    primGroupPlug, pointGroupPlug);

        outputNodeIdHandle.setInt(myInput->geometryNodeId());

        return MStatus::kSuccess;
    }

    return MPxNode::compute(plug, dataBlock);
}

InputGeometryNode::InputGeometryNode() : myInput(NULL) {}

InputGeometryNode::~InputGeometryNode()
{
    clearInput();
}

void
InputGeometryNode::clearInput()
{
    delete myInput;
    myInput = NULL;
}

bool
InputGeometryNode::checkInput(MDataBlock &dataBlock)
{
    MPlug inputGeometryPlug(thisMObject(), InputGeometryNode::inputGeometry);

    Input::AssetInputType newAssetInputType = Input::AssetInputType_Invalid;
    {
        MObject geoData;

        MDataHandle geoDataHandle = dataBlock.inputValue(inputGeometryPlug);
        geoData                   = geoDataHandle.data();

        if (geoData.hasFn(MFn::kMeshData))
        {
            newAssetInputType = Input::AssetInputType_Mesh;
        }
        else if (geoData.hasFn(MFn::kNurbsCurveData))
        {
            newAssetInputType = Input::AssetInputType_Curve;
        }
        else if (geoData.hasFn(MFn::kVectorArrayData))
        {
            newAssetInputType = Input::AssetInputType_Particle;
        }
    }

    if (newAssetInputType == Input::AssetInputType_Invalid)
    {
        clearInput();
        return false;
    }

    // if the existing input doesn't match the new input type, delete it
    if (myInput && myInput->assetInputType() != newAssetInputType)
    {
        clearInput();
    }

    // create Input if necessary
    if (!myInput)
    {
        myInput = Input::createAssetInput(newAssetInputType);
    }

    if (!myInput)
    {
        clearInput();
        return false;
    }

    return true;
}
