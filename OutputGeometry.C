#include <maya/MArrayDataBuilder.h>

#include "OutputGeometry.h"
#include "OutputGeometryPart.h"
#include "OutputObject.h"
#include "util.h"
#include "AssetNode.h"

OutputGeometry::OutputGeometry(int assetId, int objectId, int geoId) :
    myAssetId (assetId),
    myObjectId (objectId),
    myGeoId (geoId)
{
    update();
}

OutputGeometry::~OutputGeometry()
{
    for(int i = myParts.size(); i-- > 0;)
    {
        delete myParts.back();
        myParts.pop_back();
    }
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
        // Create any new OutputGeometryPart
        myParts.reserve(myGeoInfo.partCount);
        for(int i = myParts.size(); i < myGeoInfo.partCount; i++)
        {
            OutputGeometryPart* outputGeometryPart = new OutputGeometryPart(
                    myAssetId,
                    myObjectId,
                    myGeoId,
                    i
                    );
            myParts.push_back(outputGeometryPart);
        }

        // Destroy the extra OutputGeometryPart
        for(int i = myParts.size(); i-- > myGeoInfo.partCount;)
        {
            delete myParts.back();
            myParts.pop_back();
        }
    }
}

MStatus
OutputGeometry::compute(MDataHandle &geoHandle, bool &needToSyncOutputs)
{
    MStatus stat;

    MDataHandle geoNameHandle = geoHandle.child(AssetNode::outputGeoName);
    MString geoName;
    if(myGeoInfo.nameSH != 0)
    {
        geoName = Util::getString(myGeoInfo.nameSH);
    }
    geoNameHandle.setString(geoName);
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
        // Compute the OutputGeometryPart
        for(int i=0; i< myGeoInfo.partCount; i++)
        {
            MDataHandle h = partsBuilder.addElement(i);
            stat = myParts[i]->compute(
                    h,
                    myGeoInfo.hasGeoChanged,
                    myGeoInfo.hasMaterialChanged,
                    needToSyncOutputs
                    );
            CHECK_MSTATUS_AND_RETURN(stat, MS::kFailure);
        }

        // Remove the element of the array attribute
        for(int i = partsBuilder.elementCount(); i-- > myGeoInfo.partCount;)
        {
            stat = partsBuilder.removeElement(i);
            CHECK_MSTATUS_AND_RETURN(stat, MS::kFailure);
        }
    }

    partsArrayHandle.set(partsBuilder);

    return MS::kSuccess;
}
