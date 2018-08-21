#include "InputTransformNode.h"

#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MQuaternion.h>

#include "MayaTypeID.h"
#include "hapiutil.h"
#include "util.h"

MString InputTransformNode::typeName("houdiniInputTransform");
MTypeId InputTransformNode::typeId(MayaTypeID_HoudiniInputTransformNode);

MObject InputTransformNode::inputTransform;
MObject InputTransformNode::inputMatrix;
MObject InputTransformNode::outputNodeId;

void*
InputTransformNode::creator()
{
    return new InputTransformNode();
}

MStatus
InputTransformNode::initialize()
{
    MFnMatrixAttribute mAttr;
    MFnNumericAttribute nAttr;

    InputTransformNode::inputTransform = mAttr.create(
            "inputTransform", "inputTransform"
            );
    addAttribute(InputTransformNode::inputTransform);

    InputTransformNode::inputMatrix = mAttr.create(
            "inputMatrix", "inputMatrix"
            );
    mAttr.setArray(true);
    mAttr.setCached(false);
    mAttr.setStorable(false);
    mAttr.setDisconnectBehavior(MFnAttribute::kDelete);
    addAttribute(InputTransformNode::inputMatrix);

    InputTransformNode::outputNodeId = nAttr.create(
            "outputNodeId", "outputNodeId",
            MFnNumericData::kInt,
            -1
            );
    nAttr.setCached(false);
    nAttr.setStorable(false);
    addAttribute(InputTransformNode::outputNodeId);

    attributeAffects(InputTransformNode::inputTransform, InputTransformNode::outputNodeId);
    attributeAffects(InputTransformNode::inputMatrix, InputTransformNode::outputNodeId);

    return MStatus::kSuccess;
}

InputTransformNode::InputTransformNode() :
    myGeometryNodeId(-1)
{
    Util::PythonInterpreterLock pythonInterpreterLock;

    CHECK_HAPI(HAPI_CreateInputNode(
                Util::theHAPISession.get(),
                &myGeometryNodeId,
                NULL
                ));
    if(!Util::statusCheckLoop())
    {
        DISPLAY_ERROR(MString("Unexpected error when creating input asset."));
    }
}

InputTransformNode::~InputTransformNode()
{
    if(!Util::theHAPISession.get())
        return;
    CHECK_HAPI(HAPI_DeleteNode(
                Util::theHAPISession.get(),
                myGeometryNodeId
                ));
}

MStatus
InputTransformNode::compute(
        const MPlug &plug,
        MDataBlock &dataBlock
        )
{
    if(plug == InputTransformNode::outputNodeId)
    {
        MPlug inputMatrixArrayPlug(thisMObject(), InputTransformNode::inputMatrix);

        const unsigned int pointCount = inputMatrixArrayPlug.numElements();

        HAPI_PartInfo partInfo;
        HAPI_PartInfo_Init(&partInfo);
        partInfo.id = 0;
        partInfo.faceCount = 0;
        partInfo.vertexCount = 0;
        partInfo.pointCount = pointCount;

        HAPI_SetPartInfo(
                Util::theHAPISession.get(),
                myGeometryNodeId, 0,
                &partInfo
                );

        std::vector<float> P(pointCount * 3);
        std::vector<float> orient(pointCount * 4);
        std::vector<float> scale(pointCount * 3);
        MStringArray name(pointCount, MString());

        for(unsigned int i = 0; i < pointCount; i++)
        {
            MPlug inputMatrixPlug = inputMatrixArrayPlug.elementByPhysicalIndex(i);

            MPlug sourceNodePlug = Util::plugSource(inputMatrixPlug);
            name[i] = Util::getNodeName(sourceNodePlug.node());

            MDataHandle inputMatrixHandle = dataBlock.inputValue(inputMatrixPlug);

            MTransformationMatrix transformation = inputMatrixHandle.asMatrix();

            MVector t = transformation.getTranslation(MSpace::kWorld);
            P[i * 3 + 0] = t[0];
            P[i * 3 + 1] = t[1];
            P[i * 3 + 2] = t[2];

            MQuaternion r = transformation.rotation();
            orient[i * 4 + 0] = r[0];
            orient[i * 4 + 1] = r[1];
            orient[i * 4 + 2] = r[2];
            orient[i * 4 + 3] = r[3];

            double s[3];
            transformation.getScale(s, MSpace::kWorld);
            scale[i * 3 + 0] = s[0];
            scale[i * 3 + 1] = s[1];
            scale[i * 3 + 2] = s[2];
        }

        CHECK_HAPI(hapiSetPointAttribute(
                    myGeometryNodeId, 0,
                    1,
                    "name",
                    name
                    ));

        CHECK_HAPI(hapiSetPointAttribute(
                    myGeometryNodeId, 0,
                    3,
                    "P",
                    P
                    ));

        CHECK_HAPI(hapiSetPointAttribute(
                    myGeometryNodeId, 0,
                    4,
                    "orient",
                    orient
                    ));

        CHECK_HAPI(hapiSetPointAttribute(
                    myGeometryNodeId, 0,
                    3,
                    "scale",
                    scale
                    ));

        HAPI_CommitGeo(
                Util::theHAPISession.get(),
                myGeometryNodeId
                );

        MDataHandle outputNodeIdHandle =
            dataBlock.outputValue(InputTransformNode::outputNodeId);

        outputNodeIdHandle.setInt(myGeometryNodeId);

        return MStatus::kSuccess;
    }

    return MPxNode::compute(plug, dataBlock);
}
