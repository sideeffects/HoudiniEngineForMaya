#include "InputNode.h"

#include <maya/MFnGenericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include "MayaTypeID.h"

MString InputNode::typeName("houdiniInput");
MTypeId InputNode::typeId(MayaTypeID_HoudiniInputNode);

MObject InputNode::inputTransform;
MObject InputNode::inputGeometry;
MObject InputNode::outputNodeId;
MObject InputNode::outputNodePath;

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
    MFnTypedAttribute tAttr;

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
    addAttribute(InputNode::inputGeometry);

    InputNode::outputNodeId = nAttr.create(
            "outputNodeId", "outputNodeId",
            MFnNumericData::kInt
            );
    addAttribute(InputNode::outputNodeId);

    InputNode::outputNodePath = tAttr.create(
            "outputNodePath", "outputNodePath",
            MFnData::kString
            );
    addAttribute(InputNode::outputNodePath);

    attributeAffects(InputNode::inputTransform, InputNode::outputNodeId);
    attributeAffects(InputNode::inputTransform, InputNode::outputNodePath);
    attributeAffects(InputNode::inputGeometry, InputNode::outputNodeId);
    attributeAffects(InputNode::inputGeometry, InputNode::outputNodePath);

    return MStatus::kSuccess;
}

MStatus
InputNode::compute(
        const MPlug &plug,
        MDataBlock &dataBlock
        )
{
    return MPxNode::compute(plug, dataBlock);
}
