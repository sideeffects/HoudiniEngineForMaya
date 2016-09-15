#ifndef __OutputGeometry_h__
#define __OutputGeometry_h__

#include <maya/MStatus.h>
#include <maya/MDataBlock.h>

#include <vector>

#include <HAPI.h>

class OutputObject;
class OutputGeometryPart;

class OutputGeometry {
    public:
        OutputGeometry(HAPI_NodeId nodeId);
        ~OutputGeometry();

        MStatus compute(
                const MTime &time,
                MDataHandle &geoHandle,
                bool &needToSyncOutputs
                );

        void update();

    protected:
        HAPI_NodeId myNodeId;
        HAPI_NodeInfo myNodeInfo;
        HAPI_GeoInfo myGeoInfo;

        int myLastCookCount;

        std::vector<OutputGeometryPart*> myParts;
};

#endif
