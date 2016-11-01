#include "InputMergeNode.h"

#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnNumericAttribute.h>

#include "MayaTypeID.h"
#include "hapiutil.h"
#include "util.h"

MString InputMergeNode::typeName("houdiniInputMerge");
MTypeId InputMergeNode::typeId(MayaTypeID_HoudiniInputMergeNode);

MObject InputMergeNode::inputTransform;
MObject InputMergeNode::inputNode;
MObject InputMergeNode::outputNodeId;

void*
InputMergeNode::creator()
{
    return new InputMergeNode();
}

MStatus
InputMergeNode::initialize()
{
    MFnMatrixAttribute mAttr;
    MFnNumericAttribute nAttr;

    InputMergeNode::inputTransform = mAttr.create(
            "inputTransform", "inputTransform"
            );
    addAttribute(InputMergeNode::inputTransform);

    InputMergeNode::inputNode = nAttr.create(
            "inputNode", "inputNode",
            MFnNumericData::kInt,
            -1
            );
    nAttr.setArray(true);
    nAttr.setCached(false);
    nAttr.setStorable(false);
    addAttribute(InputMergeNode::inputNode);

    InputMergeNode::outputNodeId = nAttr.create(
            "outputNodeId", "outputNodeId",
            MFnNumericData::kInt,
            -1
            );
    nAttr.setCached(false);
    nAttr.setStorable(false);
    addAttribute(InputMergeNode::outputNodeId);

    attributeAffects(InputMergeNode::inputTransform, InputMergeNode::outputNodeId);
    attributeAffects(InputMergeNode::inputNode, InputMergeNode::outputNodeId);

    return MStatus::kSuccess;
}

InputMergeNode::InputMergeNode() :
    myGeometryNodeId(-1)
{
    Util::PythonInterpreterLock pythonInterpreterLock;

    CHECK_HAPI(HAPI_CreateNode(
                Util::theHAPISession.get(),
                -1,
                "Sop/merge",
                NULL,
                false,
                &myGeometryNodeId
                ));
    if(!Util::statusCheckLoop())
    {
        DISPLAY_ERROR(MString("Unexpected error when creating merge node."));
    }
}

InputMergeNode::~InputMergeNode()
{
    CHECK_HAPI(HAPI_DeleteNode(
                Util::theHAPISession.get(),
                myGeometryNodeId
                ));
}

MStatus
InputMergeNode::compute(
        const MPlug &plug,
        MDataBlock &dataBlock
        )
{
    if(plug == InputMergeNode::outputNodeId)
    {
        MArrayDataHandle inputNodeArrayHandle =
            dataBlock.inputArrayValue(InputMergeNode::inputNode);

        const unsigned int mergeCount = inputNodeArrayHandle.elementCount();

        for(unsigned int i = 0; i < mergeCount; i++)
        {
            inputNodeArrayHandle.jumpToElement(i);
            MDataHandle inputNodeHandle = inputNodeArrayHandle.inputValue();

            HAPI_NodeId inputNode = inputNodeHandle.asInt();

            CHECK_HAPI(HAPI_ConnectNodeInput(
                        Util::theHAPISession.get(),
                        myGeometryNodeId,
                        i,
                        inputNode
                        ));
        }

        MDataHandle outputNodeIdHandle =
            dataBlock.outputValue(InputMergeNode::outputNodeId);

        outputNodeIdHandle.setInt(myGeometryNodeId);

        return MStatus::kSuccess;
    }

    return MPxNode::compute(plug, dataBlock);
}
