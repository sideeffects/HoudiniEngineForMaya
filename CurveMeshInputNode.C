#include <maya/MFnNumericAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MDataHandle.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFileIO.h>
#include <maya/MTime.h>
#include <maya/MGlobal.h>
#include <maya/MPlugArray.h>

#include <algorithm>

#include "Asset.h"
#include "Input.h"
#include "CurveMeshInputNode.h"
#include "MayaTypeID.h"
#include "util.h"

#include <cassert>

MString CurveMeshInputNode::typeName = "houdiniCurveMeshInput";

MTypeId CurveMeshInputNode::typeId = MayaTypeID_HoudiniCurveMeshInput;

MObject
    CurveMeshInputNode::theInputCurves,
    CurveMeshInputNode::theOutputObjectMetaData;

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

void*
CurveMeshInputNode::creator()
{
    CurveMeshInputNode* ret = new CurveMeshInputNode();
    return ret;
}

MStatus
CurveMeshInputNode::initialize()
{
    // maya plugin stuff
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;
    MFnCompoundAttribute cAttr;
    MFnUnitAttribute uAttr;

    theInputCurves = tAttr.create( "inputCurves", "incs",
                                    MFnData::kNurbsCurve );

    tAttr.setArray(true);

    addAttribute(theInputCurves);

    theOutputObjectMetaData = tAttr.create(
        "outputObjectMetaData", "oomd",
        MFnData::kIntArray
    );

    tAttr.setStorable(false);
    tAttr.setWritable(false);

    addAttribute(theOutputObjectMetaData);

    attributeAffects(theInputCurves, theOutputObjectMetaData);

    return MS::kSuccess;
}

CurveMeshInputNode::CurveMeshInputNode()
{
}

CurveMeshInputNode::~CurveMeshInputNode()
{
}

void
CurveMeshInputNode::postConstructor()
{
}

MStatus
CurveMeshInputNode::setDependentsDirty(const MPlug& plugBeingDirtied,
        MPlugArray& affectedPlugs)
{
    MStatus status;

    return MS::kSuccess;
}

MStatus
CurveMeshInputNode::compute(const MPlug& plug, MDataBlock& data)
{
    MStatus status;

    return MPxTransform::compute(plug, data);
}

bool
CurveMeshInputNode::getInternalValueInContext(
        const MPlug &plug,
        MDataHandle &dataHandle,
        MDGContext &ctx)
{
    MStatus status;

    return MPxTransform::getInternalValueInContext(plug, dataHandle, ctx);
}

bool
CurveMeshInputNode::setInternalValueInContext(
        const MPlug &plug,
        const MDataHandle &dataHandle,
        MDGContext &ctx
        )
{
    MStatus status;

    return MPxTransform::setInternalValueInContext(plug, dataHandle, ctx);
}

int
CurveMeshInputNode::internalArrayCount(const MPlug &plug, const MDGContext &ctx) const
{
    return MPxTransform::internalArrayCount(plug, ctx);
}

void
CurveMeshInputNode::copyInternalData(MPxNode* node)
{
    MStatus status;

    MPxTransform::copyInternalData(node);
}
