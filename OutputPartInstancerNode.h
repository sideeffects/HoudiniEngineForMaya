#ifndef __OutputPartInstancerNode_h_
#define __OutputPartInstancerNode_h_

#include <maya/MPxTransform.h>

#include <HAPI_Common.h>

// Utility node to allow persistence of instancer point data
class OutputPartInstancerNode : public MPxTransform
{
public:
    static void* creator();
    static MStatus initialize();

public:
    static MString typeName;
    static MTypeId typeId;

    static MObject pointData;

    static MObject storablePositions;
    static MObject storableRotations;
    static MObject storableScales;
    static MObject storableObjectIndices;

public:
    OutputPartInstancerNode();
    virtual ~OutputPartInstancerNode();

    virtual MStatus compute(
        const MPlug &plug,
        MDataBlock &dataBlock
    );
};

#endif // __OutputPartInstancerNode_h_
