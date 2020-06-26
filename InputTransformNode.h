#ifndef __InputTransformNode_h__
#define __InputTransformNode_h__

#include <maya/MPxNode.h>

#include <HAPI/HAPI_Common.h>

class InputTransformNode : public MPxNode
{
public:
    static void *creator();
    static MStatus initialize();

public:
    static MString typeName;
    static MTypeId typeId;

    static MObject inputTransform;
    static MObject inputMatrix;
    static MObject preserveScale;

    static MObject outputNodeId;

public:
    InputTransformNode();
    virtual ~InputTransformNode();

    virtual MStatus compute(const MPlug &plug, MDataBlock &dataBlock);

private:
    void clearInput();
    bool checkInput(MDataBlock &dataBlock);

private:
    HAPI_NodeId myGeometryNodeId;
};

#endif
