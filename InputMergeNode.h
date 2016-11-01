#ifndef __InputMergeNode_h__
#define __InputMergeNode_h__

#include <maya/MPxNode.h>

#include <HAPI_Common.h>

class InputMergeNode : public MPxNode
{
public:
    static void* creator();
    static MStatus initialize();

public:
    static MString typeName;
    static MTypeId typeId;

    static MObject inputTransform;
    static MObject inputNode;

    static MObject outputNodeId;

public:
    InputMergeNode();
    virtual ~InputMergeNode();

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
