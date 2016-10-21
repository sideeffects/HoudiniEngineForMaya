#include "InputNode.h"

#include "MayaTypeID.h"

MString InputNode::typeName("houdiniInput");
MTypeId InputNode::typeId(MayaTypeID_HoudiniInputNode);

void*
InputNode::creator()
{
    return new InputNode();
}

MStatus
InputNode::initialize()
{
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
