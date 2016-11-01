#include "InputNode.h"

#include <maya/MFnGenericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnNumericAttribute.h>

#include "MayaTypeID.h"
#include "Input.h"

MString InputNode::typeName("houdiniInput");
MTypeId InputNode::typeId(MayaTypeID_HoudiniInputNode);

MObject InputNode::inputTransform;
MObject InputNode::inputGeometry;
MObject InputNode::outputNodeId;

void*
InputNode::creator()
{
    return new InputNode();
}

MStatus
InputNode::initialize()
{
    MFnGenericAttribute gAttr;
    MFnMatrixAttribute mAttr;
    MFnNumericAttribute nAttr;

    InputNode::inputTransform = mAttr.create(
            "inputTransform", "inputTransform"
            );
    addAttribute(InputNode::inputTransform);

    InputNode::inputGeometry = gAttr.create(
            "inputGeometry", "inputGeometry"
            );
    gAttr.addDataAccept(MFnData::kIntArray);
    gAttr.addDataAccept(MFnData::kMesh);
    gAttr.addDataAccept(MFnData::kNurbsCurve);
    gAttr.addDataAccept(MFnData::kVectorArray);
    gAttr.setCached(false);
    gAttr.setStorable(false);
    addAttribute(InputNode::inputGeometry);

    InputNode::outputNodeId = nAttr.create(
            "outputNodeId", "outputNodeId",
            MFnNumericData::kInt,
            -1
            );
    nAttr.setCached(false);
    nAttr.setStorable(false);
    addAttribute(InputNode::outputNodeId);

    attributeAffects(InputNode::inputTransform, InputNode::outputNodeId);
    attributeAffects(InputNode::inputGeometry, InputNode::outputNodeId);

    return MStatus::kSuccess;
}

MStatus
InputNode::compute(
        const MPlug &plug,
        MDataBlock &dataBlock
        )
{
    if(plug == InputNode::outputNodeId)
    {
        MDataHandle outputNodeIdHandle = dataBlock.outputValue(InputNode::outputNodeId);

        if(!checkInput(dataBlock))
        {
            outputNodeIdHandle.setInt(-1);

            return MStatus::kFailure;
        }

        // set input transform
        MPlug transformPlug(thisMObject(), InputNode::inputTransform);
        MDataHandle transformHandle = dataBlock.inputValue(transformPlug);
        myInput->setInputTransform(transformHandle);

        // set input geo
        MPlug geometryPlug(thisMObject(), InputNode::inputGeometry);
        myInput->setInputGeo(dataBlock, geometryPlug);

        outputNodeIdHandle.setInt(myInput->geometryNodeId());

        return MStatus::kSuccess;
    }

    return MPxNode::compute(plug, dataBlock);
}

InputNode::InputNode() :
    myInput(NULL)
{
}

InputNode::~InputNode()
{
    clearInput();
}

void
InputNode::clearInput()
{
    delete myInput;
    myInput = NULL;
}

bool
InputNode::checkInput(MDataBlock &dataBlock)
{
    MPlug inputGeometryPlug(thisMObject(), InputNode::inputGeometry);

    Input::AssetInputType newAssetInputType = Input::AssetInputType_Invalid;
    {
        MObject geoData;

        MDataHandle geoDataHandle = dataBlock.inputValue(
                inputGeometryPlug);
        geoData = geoDataHandle.data();

        if(geoData.hasFn(MFn::kMeshData))
        {
            newAssetInputType = Input::AssetInputType_Mesh;
        }
        else if(geoData.hasFn(MFn::kNurbsCurveData))
        {
            newAssetInputType = Input::AssetInputType_Curve;
        }
        else if(geoData.hasFn(MFn::kVectorArrayData))
        {
            newAssetInputType = Input::AssetInputType_Particle;
        }
    }

    if(newAssetInputType == Input::AssetInputType_Invalid)
    {
        clearInput();
        return false;
    }

    // if the existing input doesn't match the new input type, delete it
    if(myInput && myInput->assetInputType() != newAssetInputType)
    {
        clearInput();
    }

    // create Input if necessary
    if(!myInput)
    {
        myInput = Input::createAssetInput(newAssetInputType);
    }

    if(!myInput)
    {
        clearInput();
        return false;
    }

    return true;
}
