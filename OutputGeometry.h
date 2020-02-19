#ifndef __OutputGeometry_h__
#define __OutputGeometry_h__

#include <maya/MDataBlock.h>
#include <maya/MStatus.h>

#include <vector>

#include <HAPI/HAPI.h>

#include "AssetNodeOptions.h"

class OutputObject;
class OutputGeometryPart;

class OutputGeometry
{
public:
    OutputGeometry(HAPI_NodeId nodeId);
    ~OutputGeometry();

    MStatus compute(const MTime &time,
                    const MPlug &geoPlug,
                    MDataBlock &data,
                    MDataHandle &geoHandle,
                    AssetNodeOptions::AccessorDataBlock &options,
                    bool &needToSyncOutputs,
                    const bool needToRecomputeOutputData);

    void update();

protected:
    HAPI_NodeId myNodeId;
    HAPI_NodeInfo myNodeInfo;
    HAPI_GeoInfo myGeoInfo;

    int myLastCookCount;

    std::vector<OutputGeometryPart *> myParts;
};

#endif
