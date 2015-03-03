#ifndef __OutputGeometry_h__
#define __OutputGeometry_h__

#include <maya/MStatus.h>
#include <maya/MDataBlock.h>

#include <vector>

#include <HAPI/HAPI.h>

class OutputObject;
class OutputGeometryPart;

class OutputGeometry {
    public:
        OutputGeometry(int assetId, int objectId, int geoId);
        ~OutputGeometry();

        MStatus compute(
                const MTime &time,
                MDataHandle &geoHandle,
                bool &needToSyncOutputs
                );

        void update();

    protected:
        HAPI_GeoInfo myGeoInfo;
        int myAssetId;
        int myObjectId;
        int myGeoId;

        std::vector<OutputGeometryPart*> myParts;
};

#endif
