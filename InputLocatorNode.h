#ifndef __InputLocatorNode_h__
#define __InputLocatorNode_h__

#include <maya/MPxNode.h>

#include <HAPI_Common.h>

class InputLocatorNode : public MPxNode
{
public:
    static void* creator();
    static MStatus initialize();

public:
    static MString typeName;
    static MTypeId typeId;

    static MObject inputTransform;
    static MObject inputMatrix;

    static MObject outputNodeId;

public:
    InputLocatorNode();
    virtual ~InputLocatorNode();

    virtual MStatus compute(
            const MPlug &plug,
            MDataBlock &dataBlock
            );

private:
    void clearInput();
    bool checkInput(MDataBlock &dataBlock);

private:
    HAPI_NodeId myGeometryNodeId;
};

#endif
