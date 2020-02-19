#ifndef __FluidGridConvert_h__
#define __FluidGridConvert_h__

#include <maya/MPxNode.h>

#include <maya/MString.h>
#include <maya/MTypeId.h>

#if MAYA_API_VERSION >= 201400

class FluidGridConvert : public MPxNode
{
public:
    FluidGridConvert();
    virtual ~FluidGridConvert();

    virtual MStatus compute(const MPlug &plug, MDataBlock &data);

    static void *creator();
    static MStatus initialize();

    static MString typeName;
    static MTypeId typeId;

    static MObject conversionMode;

    static MObject resolution;

    static MObject inGridX;
    static MObject inGridY;
    static MObject inGridZ;

    static MObject outGrid;
};

#endif // MAYA_API_VERSION check

#endif
