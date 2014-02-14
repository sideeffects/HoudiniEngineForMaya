#ifndef __FluidVelocityConvert_h__
#define __FluidVelocityConvert_h__

#include <maya/MGlobal.h>

#include <maya/MPxNode.h> 

#include <maya/MFnNumericAttribute.h>

#include <maya/MString.h> 
#include <maya/MFloatArray.h> 
#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MVector.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>

#if MAYA_API_VERSION >= 201400

class MFloatArray;

class FluidVelocityConvert : public MPxNode
{
public:
    FluidVelocityConvert();
    virtual ~FluidVelocityConvert(); 

    virtual MStatus compute(const MPlug& plug, MDataBlock& data);

    static void* creator();
    static MStatus initialize();

    static MString typeName;
    static MTypeId typeId;

    static MObject resolution;

    static MObject inGridX;
    static MObject inGridY;
    static MObject inGridZ;

    static MObject outGrid;

public:
    MFloatArray extrapolateX(const MFloatArray& vel,
			     int resX, int resY, int resZ);
    MFloatArray extrapolateY(const MFloatArray& vel,
			     int resX, int resY, int resZ);
    MFloatArray extrapolateZ(const MFloatArray& vel,
			     int resX, int resY, int resZ);
};

#endif // MAYA_API_VERSION check

#endif
