#include <maya/MArrayDataBuilder.h>

#include "OutputGeometry.h"
#include "OutputGeometryPart.h"
#include "OutputObject.h"
#include "util.h"
#include "AssetNode.h"

OutputGeometry::~OutputGeometry()
{
}

OutputGeometry::OutputGeometry(int assetId, int objectId, int geoId, OutputObject * parentObject) :
    myParentObject (parentObject),
    myAssetId (assetId),
    myObjectId (objectId),
    myGeoId (geoId)
{
    update();
}

void
OutputGeometry::update()
{
    HAPI_Result hapiResult;

    hapiResult = HAPI_GetGeoInfo(myAssetId,
            myObjectId,
            myGeoId,
            &myGeoInfo);
    if(HAPI_FAIL(hapiResult))
    {
        // Make sre myGeoInfo is properly initialized.
        HAPI_GeoInfo_Init(&myGeoInfo);

        // Even when HAPI_GetGeoInfo() failed, there's always at least one
        // part. So we want the below code to initialize myParts.
    }

    if(myGeoInfo.type == HAPI_GEOTYPE_DEFAULT
            || myGeoInfo.type == HAPI_GEOTYPE_INTERMEDIATE
            || myGeoInfo.type == HAPI_GEOTYPE_CURVE)
    {
        unsigned int partCount = myGeoInfo.partCount;

        // If partCount is different, recreate the array.
        if(myParts.size() != partCount)
        {
            myParts.clear();
            myParts.reserve(partCount);
            for(unsigned int i = 0; i < partCount; i++)
            {
                myParts.push_back(OutputGeometryPart(myAssetId,
                            myObjectId,
                            myGeoId,
                            i,
                            myGeoInfo
                            ));
            }
        }
    }
}

MStatus
OutputGeometry::compute(MDataHandle &geoHandle, bool &needToSyncOutputs)
{
    MStatus stat;

    MDataHandle geoNameHandle = geoHandle.child(AssetNode::outputGeoName);
    geoNameHandle.setString(Util::getString(myGeoInfo.nameSH));
    geoNameHandle.setClean();

    MDataHandle isTemplatedHandle = geoHandle.child(AssetNode::outputGeoIsTemplated);
    isTemplatedHandle.setBool(myGeoInfo.isTemplated);
    isTemplatedHandle.setClean();

    MDataHandle isDisplayGeoHandle = geoHandle.child(AssetNode::outputGeoIsDisplayGeo);
    isDisplayGeoHandle.setBool(myGeoInfo.isDisplayGeo);
    isDisplayGeoHandle.setClean();

    MDataHandle partsHandle = geoHandle.child(AssetNode::outputParts);
    MArrayDataHandle partsArrayHandle(partsHandle);
    MArrayDataBuilder partsBuilder = partsArrayHandle.builder();
    if(partsBuilder.elementCount() != (unsigned int)(myGeoInfo.partCount))
    {
        needToSyncOutputs = true;
    }

    if(myGeoInfo.type == HAPI_GEOTYPE_DEFAULT ||
        myGeoInfo.type == HAPI_GEOTYPE_INTERMEDIATE ||
        myGeoInfo.type == HAPI_GEOTYPE_CURVE)
    {
        for(int i=0; i< myGeoInfo.partCount; i++)
        {
            MDataHandle h = partsBuilder.addElement(i);
            stat = myParts[i].compute(h, needToSyncOutputs);
            CHECK_MSTATUS_AND_RETURN(stat, MS::kFailure);
        }

        int partBuilderSizeCheck = partsBuilder.elementCount();
        if(partBuilderSizeCheck > myGeoInfo.partCount)
        {
            for(int i = myGeoInfo.partCount; i< partBuilderSizeCheck; i++)
            {
                stat = partsBuilder.removeElement(i);
                CHECK_MSTATUS_AND_RETURN(stat, MS::kFailure);
            }
        }
    }

    partsArrayHandle.set(partsBuilder);

    return MS::kSuccess;
}
