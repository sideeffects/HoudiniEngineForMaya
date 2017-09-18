#include <maya/MArrayDataBuilder.h>

#include "OutputGeometry.h"
#include "OutputGeometryPart.h"
#include "OutputObject.h"
#include "util.h"
#include "AssetNode.h"

OutputGeometry::OutputGeometry(HAPI_NodeId nodeId) :
    myNodeId(nodeId),
    myLastCookCount(0)
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

    hapiResult = HAPI_GetNodeInfo(
            Util::theHAPISession.get(),
            myNodeId, &myNodeInfo
            );
    CHECK_HAPI(hapiResult);

    hapiResult = HAPI_GetGeoInfo(
            Util::theHAPISession.get(),
            myNodeId,
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
                    myNodeId,
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
OutputGeometry::compute(
        const MTime &time,
        const MPlug &geoPlug,
        MDataBlock &data,
        MDataHandle &geoHandle,
        bool &needToSyncOutputs
        )
{
    MStatus stat;

    data.setClean(geoPlug);

    update();

    MDataHandle geoNameHandle = geoHandle.child(AssetNode::outputGeoName);
    MString geoName;
    if(myGeoInfo.nameSH != 0)
    {
        geoName = Util::HAPIString(myGeoInfo.nameSH);
    }
    geoNameHandle.setString(geoName);

    MDataHandle isTemplatedHandle = geoHandle.child(AssetNode::outputGeoIsTemplated);
    isTemplatedHandle.setBool(myGeoInfo.isTemplated);

    MDataHandle isDisplayGeoHandle = geoHandle.child(AssetNode::outputGeoIsDisplayGeo);
    isDisplayGeoHandle.setBool(myGeoInfo.isDisplayGeo);

    MPlug partsPlug = geoPlug.child(AssetNode::outputParts);
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
        if(myNodeInfo.totalCookCount > myLastCookCount)
        {
            // Compute the OutputGeometryPart
            for(int i = 0; i < myGeoInfo.partCount; i++)
            {
                MPlug partPlug = partsPlug.elementByLogicalIndex(i);
                MDataHandle partHandle = partsBuilder.addElement(i);

                stat = myParts[i]->compute(
                        time,
                        partPlug,
                        data,
                        partHandle,
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
        else
        {
            // even if nothing changed, clean the plugs
            for(int i = 0; i < myGeoInfo.partCount; i++)
            {
                MPlug partPlug = partsPlug.elementByLogicalIndex(i);

                MPlugArray childPlugs;
                Util::getChildPlugs(childPlugs, partPlug);
                for(unsigned int j = 0; j < childPlugs.length(); j++)
                {
                    data.setClean(childPlugs[j]);
                }
            }
        }
    }

    partsArrayHandle.set(partsBuilder);

    myLastCookCount = myNodeInfo.totalCookCount;

    return MS::kSuccess;
}
