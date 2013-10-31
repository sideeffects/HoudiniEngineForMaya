#include "FluidVelocityConvert.h"

#if MAYA_API_VERSION >= 201400

#include "MayaTypeID.h"
#include "util.h"

#include <maya/MDataHandle.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnFloatArrayData.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MObject.h>


MTypeId FluidVelocityConvert::id = MayaTypeID_HoudiniFluidVelocityConvert;

MObject FluidVelocityConvert::resolution;

MObject FluidVelocityConvert::inGridX;
MObject FluidVelocityConvert::inGridY;
MObject FluidVelocityConvert::inGridZ;

MObject FluidVelocityConvert::outGrid;

FluidVelocityConvert::FluidVelocityConvert()
{
}

FluidVelocityConvert::~FluidVelocityConvert()
{
}

void*
FluidVelocityConvert::creator()
{
    return new FluidVelocityConvert();
}

MStatus
FluidVelocityConvert::initialize()
{
    MFnNumericAttribute nAttr;
    MFnCompoundAttribute cAttr;
    MFnTypedAttribute tAttr;

    resolution = tAttr.create("resolution", "resolution", MFnData::kFloatArray);
    cAttr.setStorable(false);

    inGridX = tAttr.create("inGridX", "inGridX", MFnData::kFloatArray);
    tAttr.setStorable(false);
    inGridY = tAttr.create("inGridY", "inGridY", MFnData::kFloatArray);
    tAttr.setStorable(false);
    inGridZ = tAttr.create("inGridZ", "inGridZ", MFnData::kFloatArray);
    tAttr.setStorable(false);

    outGrid = tAttr.create("outGrid", "outGrid", MFnData::kFloatArray);
    tAttr.setStorable(false);
    tAttr.setWritable(false);

    addAttribute(outGrid);
    addAttribute(inGridX);
    addAttribute(inGridY);
    addAttribute(inGridZ);
    addAttribute(resolution);
    attributeAffects(inGridX, outGrid);
    attributeAffects(inGridY, outGrid);
    attributeAffects(inGridZ, outGrid);
    attributeAffects(resolution, outGrid);

    return MS::kSuccess;
}

// Linearly extrapolates f(2.5) if f(1) = a, f(2) = b
static float
extrapolate(float a, float b)
{
    return (b - a) * 0.25 + b;
}

static int
index(int i, int j, int k,
      int resX, int resY)
{
    return k * resY * resX + j * resX + i;
}

MFloatArray
FluidVelocityConvert::extrapolateZ(const MFloatArray& vel,
				   int resX, int resY, int resZ)
{
    // First interpolate
    MFloatArray result;
    result.setLength(resX * resY * (resZ+1));
    for (int k=1; k<resZ; k++)
    {
	for (int j=0; j<resY; j++)
	{
	    for (int i=0; i<resX; i++)
	    {
		int before = index(i, j, k-1, resX, resY);
		int after  = index(i, j, k,  resX, resY);
		int dst    = index(i, j, k,  resX, resY);
		result[dst] = (vel[before] + vel[after]) * 0.5;
	    }
	}
    }

    for (int j=0; j<resY; j++)
    {
	for (int i=0; i<resX; i++)
	{
	    int start        = index(i, j, 0, resX, resY);
	    int start_before = index(i, j, 0, resX, resY);
	    int start_after  = index(i, j, 1, resX, resY);
	    result[start] = extrapolate(vel[start_after], vel[start_before]);

	    int end        = index(i, j, resZ,   resX, resY);
	    int end_before = index(i, j, resZ-2, resX, resY);
	    int end_after  = index(i, j, resZ-1, resX, resY);
	    result[end] = extrapolate(vel[end_before], vel[end_after]);
	}
    }
    return result;
}

MFloatArray
FluidVelocityConvert::extrapolateY(const MFloatArray& vel,
				   int resX, int resY, int resZ)
{
    // First interpolate
    MFloatArray result;
    result.setLength(resX * (resY+1) * resZ);
    for (int k=0; k<resZ; k++)
    {
	for (int j=1; j<resY; j++)
	{
	    for (int i=0; i<resX; i++)
	    {
		int before = index(i, j-1, k, resX, resY);
		int after  = index(i, j,   k, resX, resY);
		int dst    = index(i, j,   k, resX, resY+1);
		result[dst] = (vel[before] + vel[after]) * 0.5;
	    }
	}
    }

    // Then extrapolate the edges
    for (int k=0; k<resZ; k++)
    {
	for (int i=0; i<resX; i++)
	{
	    int start        = index(i, 0, k, resX, resY+1);
	    int start_before = index(i, 0, k, resX, resY);
	    int start_after  = index(i, 1, k, resX, resY);
	    result[start] = extrapolate(vel[start_after], vel[start_before]);

	    int end        = index(i, resY,   k, resX, resY+1);
	    int end_before = index(i, resY-2, k, resX, resY);
	    int end_after  = index(i, resY-1, k, resX, resY);
	    result[end] = extrapolate(vel[end_before], vel[end_after]);
	}
    }
    return result;
}

MFloatArray
FluidVelocityConvert::extrapolateX(const MFloatArray& vel,
				   int resX, int resY, int resZ)
{
    // First interpolate
    MFloatArray result;
    result.setLength((resX+1) * resY * resZ);
    for (int k=0; k<resZ; k++)
    {
	for (int j=0; j<resY; j++)
	{
	    for (int i=1; i<resX; i++)
	    {
		int before = index(i-1, j, k, resX,   resY);
		int after  = index(i,   j, k, resX,   resY);
		int dst    = index(i,   j, k, resX+1, resY);
		result[dst] = (vel[before] + vel[after]) * 0.5;
	    }
	}
    }
    // Then extrapolate the edges
    for (int k=0; k<resZ; k++)
    {
	for (int j=0; j<resY; j++)
	{
	    int start        = index(0, j, k, resX+1, resY);
	    int start_before = index(0, j, k, resX,   resY);
	    int start_after  = index(1, j, k, resX,   resY);
	    result[start] = extrapolate(vel[start_after], vel[start_before]);

	    int end        = index(resX,   j, k, resX+1, resY);
	    int end_before = index(resX-2, j, k, resX,   resY);
	    int end_after  = index(resX-1, j, k, resX,   resY);
	    result[end] = extrapolate(vel[end_before], vel[end_after]);
	}
    }
    return result;
}

MStatus
FluidVelocityConvert::compute(const MPlug& plug, MDataBlock& data)
{
    if (plug != MPlug(thisMObject(), outGrid))
	return MS::kSuccess;

    MStatus status;
    // Extract our grids as float arrays
    MDataHandle gridXHandle = data.inputValue(inGridX, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MDataHandle gridYHandle = data.inputValue(inGridY, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MDataHandle gridZHandle = data.inputValue(inGridZ, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFnFloatArrayData gridXFn(gridXHandle.data());
    MFnFloatArrayData gridYFn(gridYHandle.data());
    MFnFloatArrayData gridZFn(gridZHandle.data());

    MFloatArray gridX = gridXFn.array();
    MFloatArray gridY = gridYFn.array();
    MFloatArray gridZ = gridZFn.array();

    MFnFloatArrayData res(data.inputValue(resolution, &status).data());
    MFloatArray resArray = res.array();
    int resW = resArray[0];
    int resH = resArray[1];
    int resD = resArray[2];

    // Convert from houdini's velocity at voxel center format
    // into Maya's velocity at voxel face format.
    MFloatArray extrapolatedX = extrapolateX(gridX, resW, resH, resD);
    MFloatArray extrapolatedY = extrapolateY(gridY, resW, resH, resD);
    MFloatArray extrapolatedZ = extrapolateZ(gridZ, resW, resH, resD);

    // Maya's inVelocity expects the input components to be concatenated
    // onto each other.
    MFloatArray result;
    result.setLength(extrapolatedX.length() +
		     extrapolatedY.length() +
		     extrapolatedZ.length());
    int j = 0;
    for (int i=0; i<extrapolatedX.length(); i++)
    {
	result[j] = extrapolatedX[i];
	j++;
    }
    for (int i=0; i<extrapolatedY.length(); i++)
    {
	result[j] = extrapolatedY[i];
	j++;
    }
    for (int i=0; i<extrapolatedZ.length(); i++)
    {
	result[j] = extrapolatedZ[i];
	j++;
    }

    MFnFloatArrayData gridCreator;

    MDataHandle outGridHandle = data.outputValue(outGrid, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    outGridHandle.set(gridCreator.create(result));
    outGridHandle.setClean();

    data.setClean(plug);
    return MStatus::kSuccess;
}
#endif // MAYA_API_VERSION check
