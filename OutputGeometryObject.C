#include <maya/MArrayDataBuilder.h>
#include <maya/MFnIntArrayData.h>

#include "Asset.h"
#include "AssetNode.h"
#include "OutputGeometryObject.h"
#include "util.h"
#include "OutputGeometry.h"

#include <math.h>

OutputGeometryObject::OutputGeometryObject(
        HAPI_NodeId nodeId
        ) :
    OutputObject(nodeId)
{
}

OutputGeometryObject::~OutputGeometryObject()
{
    for(int i = 0; i < (int) myGeos.size(); i++)
    {
        delete myGeos[i];
    }
    myGeos.clear();
}

OutputObject::ObjectType
OutputGeometryObject::type()
{
    return OutputObject::OBJECT_TYPE_GEOMETRY;
}

MStatus
OutputGeometryObject::compute(
        const MTime &time,
        MDataBlock& data,
        MDataHandle& objectHandle,
        bool &needToSyncOutputs
        )
{
    MStatus stat = MS::kSuccess;

    update();

    // Meta data
    MDataHandle metaDataHandle = objectHandle.child(AssetNode::outputObjectMetaData);
    metaDataHandle.setInt(myNodeId);

    // outputVisibility
    MDataHandle visibilityHandle = objectHandle.child(AssetNode::outputVisibility);
    visibilityHandle.setBool(isVisible());

    // outputIsInstanced
    MDataHandle isInstancedHandle = objectHandle.child(AssetNode::outputIsInstanced);
    isInstancedHandle.setBool(isInstanced());

    // outputObjectName
    MDataHandle objectNameHandle = objectHandle.child(AssetNode::outputObjectName);
    MString objectName;
    if(myObjectInfo.nameSH != 0)
    {
        objectName = Util::HAPIString(myObjectInfo.nameSH);
    }
    objectNameHandle.setString(objectName);

    // compute geometry
    {
        MDataHandle geosHandle = objectHandle.child(AssetNode::outputGeos);
        MArrayDataHandle geoArrayHandle(geosHandle);
        MArrayDataBuilder geosBuilder = geoArrayHandle.builder();
        if(geosBuilder.elementCount() != (unsigned int)(myObjectInfo.geoCount))
        {
            needToSyncOutputs = true;
        }

        for(size_t i = 0; i < myGeos.size(); i++)
        {
            MDataHandle geoHandle = geosBuilder.addElement(i);
            stat = myGeos[i]->compute(
                    time,
                    geoHandle,
                    needToSyncOutputs
                    );
            CHECK_MSTATUS_AND_RETURN_IT(stat);
        }

        geoArrayHandle.set(geosBuilder);
    }

    // compute transform
    if(myNodeInfo.totalCookCount > myLastCookCount)
    {
        MDataHandle transformHandle = objectHandle.child(AssetNode::outputObjectTransform);
        updateTransform(transformHandle);

        myLastCookCount = myNodeInfo.totalCookCount;
    }

    return stat;
}

void OutputGeometryObject::update()
{
    HAPI_Result hapiResult;

    hapiResult = HAPI_GetNodeInfo(
            Util::theHAPISession.get(),
            myNodeId, &myNodeInfo
            );
    CHECK_HAPI(hapiResult);

    hapiResult = HAPI_GetObjectInfo(
            Util::theHAPISession.get(),
            myNodeId, &myObjectInfo
            );
    CHECK_HAPI(hapiResult);

    // Get the SOP nodes
    int geoCount;
    hapiResult = HAPI_ComposeChildNodeList(
            Util::theHAPISession.get(),
            myNodeId,
            HAPI_NODETYPE_SOP,
            HAPI_NODEFLAGS_DISPLAY,
            false,
            &geoCount
            );
    CHECK_HAPI(hapiResult);

    std::vector<HAPI_NodeId> geoNodeIds(geoCount);
    if(geoCount > 0)
    {
        hapiResult = HAPI_GetComposedChildNodeList(
                Util::theHAPISession.get(),
                myNodeId,
                &geoNodeIds.front(),
                geoCount
                );
        CHECK_HAPI(hapiResult);
    }

    // Delete old OutputGeometry
    for(int i = myGeos.size(); i-- > geoCount;)
    {
        delete myGeos[i];
        myGeos.pop_back();
    }

    // Add new OutputGeometry
    myGeos.reserve(geoCount);
    for(int i = myGeos.size(); i < geoCount; i++)
    {
        OutputGeometry * geo = new OutputGeometry(geoNodeIds[i]);
        myGeos.push_back(geo);
    }
}

void OutputGeometryObject::updateTransform(MDataHandle& handle)
{
    HAPI_Result hapiResult;

    MDataHandle translateHandle = handle.child(AssetNode::outputObjectTranslate);
    MDataHandle rotateHandle = handle.child(AssetNode::outputObjectRotate);
    MDataHandle scaleHandle = handle.child(AssetNode::outputObjectScale);

    HAPI_Transform trans;
    hapiResult = HAPI_GetObjectTransform(
            Util::theHAPISession.get(),
            myNodeId,
            -1,
            HAPI_SRT,
            &trans
            );
    CHECK_HAPI(hapiResult);

    MEulerRotation eulerRotation = MQuaternion(
            trans.rotationQuaternion[0],
            trans.rotationQuaternion[1],
            trans.rotationQuaternion[2],
            trans.rotationQuaternion[3]
            ).asEulerRotation();
#ifndef M_PI
#define M_PI 3.14
#endif
    translateHandle.set3Double(
            trans.position[0],
            trans.position[1],
            trans.position[2]);
    rotateHandle.set3Double(eulerRotation.x * 2.0 * M_PI / 360.0,
            eulerRotation.y * 2.0 * M_PI / 360.0,
            eulerRotation.z * 2.0 * M_PI / 360.0);
    scaleHandle.set3Double(
            trans.scale[0],
            trans.scale[1],
            trans.scale[2]);

    translateHandle.setClean();
    rotateHandle.setClean();
    scaleHandle.setClean();
    handle.setClean();
}

