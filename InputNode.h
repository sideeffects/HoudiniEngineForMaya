#ifndef __InputNode_h__
#define __InputNode_h__

#include <maya/MPxNode.h>

class Input;

class InputNode : public MPxNode
{
public:
    static void* creator();
    static MStatus initialize();

public:
    static MString typeName;
    static MTypeId typeId;

    static MObject inputTransform;
    static MObject inputGeometry;

    static MObject outputNodeId;

public:
    InputNode();
    virtual ~InputNode();

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
