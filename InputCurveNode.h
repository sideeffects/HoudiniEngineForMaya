#ifndef __InputCurveNode_h__
#define __InputCurveNode_h__

#include <HAPI/HAPI.h>
#include <maya/MPxNode.h>
#include <maya/MTypeId.h>

#include <vector>

class InputCurveNode : public MPxNode
{
public:
    InputCurveNode();
    virtual ~InputCurveNode();

    virtual MStatus compute(const MPlug &plug, MDataBlock &data);

    static void *creator();
    static MStatus initialize();

    static MString typeName;
    static MTypeId typeId;

    static MObject preserveScale;

private:
    HAPI_NodeId myNodeId;

    static MObject inputCurve;
    static MObject outputNodeId;
};

#endif
