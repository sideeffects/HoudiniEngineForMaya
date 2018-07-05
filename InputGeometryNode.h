#ifndef __InputGeometryNode_h__
#define __InputGeometryNode_h__

#include <maya/MPxNode.h>

class Input;

class InputGeometryNode : public MPxNode
{
public:
    static void* creator();
    static MStatus initialize();

public:
    static MString typeName;
    static MTypeId typeId;

    static MObject inputTransform;
    static MObject inputGeometry;
    static MObject inputComponents;
    static MObject unlockNormals;

    static MObject outputNodeId;

public:
    InputGeometryNode();
    virtual ~InputGeometryNode();

    virtual MStatus compute(
            const MPlug &plug,
            MDataBlock &dataBlock
            );

private:
    void clearInput();
    bool checkInput(MDataBlock &dataBlock);

private:
    Input* myInput;
};

#endif
