#include <maya/MFnIntArrayData.h>

#include "Asset.h"
#include "AssetNode.h"
#include "OutputGeometry.h"
#include "OutputGeometryObject.h"
#include "OutputGeometryPart.h"
#include "util.h"

#include <math.h>

OutputGeometryObject::OutputGeometryObject(HAPI_NodeId nodeId)
    : OutputObject(nodeId)
{
}

OutputGeometryObject::~OutputGeometryObject()
{
    for (int i = 0; i < (int)myGeos.size(); i++)
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
OutputGeometryObject::compute(const MTime &time,
                              const MPlug &objectPlug,
                              MDataBlock &data,
                              MDataHandle &objectHandle,
                              const MIntArray &instancedObjIds,
                              const MStringArray &instancedObjNames,
                              AssetNodeOptions::AccessorDataBlock &options,
                              bool &needToSyncOutputs,
                              const bool needToRecomputeOutputData)
{
    MStatus stat = MS::kSuccess;

    update();

    // Meta data
    MDataHandle metaDataHandle =
        objectHandle.child(AssetNode::outputObjectMetaData);
    metaDataHandle.setInt(myNodeId);

    // outputVisibility
    MDataHandle visibilityHandle =
        objectHandle.child(AssetNode::outputVisibility);
    visibilityHandle.setBool(isVisible());

    // outputObjectName
    MDataHandle objectNameHandle =
        objectHandle.child(AssetNode::outputObjectName);
    MString objectName;
    if (myNodeInfo.nameSH != 0)
    {
        objectName = Util::HAPIString(myNodeInfo.nameSH);
    }
    objectNameHandle.setString(objectName);

    // outputIsInstanced
    MDataHandle isInstancedHandle =
        objectHandle.child(AssetNode::outputIsInstanced);
    bool isInstanced = false;
    {
        for (unsigned int i = 0; !isInstanced && i < instancedObjIds.length();
             i++)
        {
            if (instancedObjIds[i] == myNodeInfo.id)
            {
                isInstanced = true;
            }
        }
        for (unsigned int i = 0; !isInstanced && i < instancedObjNames.length();
             i++)
        {
            if (instancedObjNames[i] == objectName)
            {
                isInstanced = true;
            }
        }
    }
    isInstancedHandle.setBool(isInstanced);

    // compute geometry
    {
        MPlug geosPlug         = objectPlug.child(AssetNode::outputGeos);
        MDataHandle geosHandle = objectHandle.child(AssetNode::outputGeos);
        MArrayDataHandle geoArrayHandle(geosHandle);
        if (geoArrayHandle.elementCount() != myGeos.size())
        {
            Util::resizeArrayDataHandle(geoArrayHandle, myGeos.size());
            needToSyncOutputs = true;
        }

        for (size_t i = 0; i < myGeos.size(); i++)
        {
            MPlug geoPlug = geosPlug.elementByLogicalIndex(i);
            CHECK_MSTATUS(geoArrayHandle.jumpToArrayElement(i));
            MDataHandle geoHandle = geoArrayHandle.outputValue();

            stat = myGeos[i]->compute(time, geoPlug, data, geoHandle, options,
                                      needToSyncOutputs,
                                      needToRecomputeOutputData);
            CHECK_MSTATUS_AND_RETURN_IT(stat);
        }
    }

    // compute transform
    // always compute, even if the totalCookCount has not changed:
    // When an asset is an OBJ subnet, calling HAPI_CookNode() on the asset
    // doesn't cause a cook on the OBJ nodes in the asset. So the
    // HAPI_NodeInfo.totalCookCount would never be incremented - Andrew W.
    {
        MDataHandle transformHandle =
            objectHandle.child(AssetNode::outputObjectTransform);
        updateTransform(transformHandle, options.preserveScale());

        myLastCookCount = myNodeInfo.totalCookCount;
    }

    return stat;
}

void
OutputGeometryObject::update()
{
    HAPI_Result hapiResult;

    hapiResult = HoudiniApi::GetNodeInfo(
        Util::theHAPISession.get(), myNodeId, &myNodeInfo);
    CHECK_HAPI(hapiResult);

    hapiResult = HoudiniApi::GetObjectInfo(
        Util::theHAPISession.get(), myNodeId, &myObjectInfo);
    CHECK_HAPI(hapiResult);

    // Get the SOP nodes
    int geoCount;
    hapiResult = HoudiniApi::ComposeChildNodeList(
        Util::theHAPISession.get(), myNodeId, HAPI_NODETYPE_SOP,
        HAPI_NODEFLAGS_DISPLAY, false, &geoCount);
    CHECK_HAPI(hapiResult);

    std::vector<HAPI_NodeId> geoNodeIds(geoCount);
    if (geoCount > 0)
    {
        hapiResult = HoudiniApi::GetComposedChildNodeList(
            Util::theHAPISession.get(), myNodeId, &geoNodeIds.front(),
            geoCount);
        CHECK_HAPI(hapiResult);
    }

    // Delete old OutputGeometry
    for (int i = myGeos.size(); i-- > geoCount;)
    {
        delete myGeos[i];
        myGeos.pop_back();
    }

    // Add new OutputGeometry
    myGeos.reserve(geoCount);
    for (int i = myGeos.size(); i < geoCount; i++)
    {
        OutputGeometry *geo = new OutputGeometry(geoNodeIds[i]);
        myGeos.push_back(geo);
    }
}

void
OutputGeometryObject::updateTransform(MDataHandle &handle,
                                      const bool preserveScale)
{
    HAPI_Result hapiResult;

    MDataHandle translateHandle =
        handle.child(AssetNode::outputObjectTranslate);
    MDataHandle rotateHandle = handle.child(AssetNode::outputObjectRotate);
    MDataHandle scaleHandle  = handle.child(AssetNode::outputObjectScale);

    HAPI_Transform trans;
    hapiResult = HoudiniApi::GetObjectTransform(
        Util::theHAPISession.get(), myNodeId, -1, HAPI_SRT, &trans);
    CHECK_HAPI(hapiResult);

    MEulerRotation eulerRotation =
        MQuaternion(trans.rotationQuaternion[0], trans.rotationQuaternion[1],
                    trans.rotationQuaternion[2], trans.rotationQuaternion[3])
            .asEulerRotation();
#ifndef M_PI
#define M_PI 3.14
#endif
    if (preserveScale)
    {
        trans.position[0] *= 100.0f;
        trans.position[1] *= 100.0f;
        trans.position[2] *= 100.0f;
    }

    translateHandle.set3Double(
        trans.position[0], trans.position[1], trans.position[2]);
    rotateHandle.set3Double(eulerRotation.x, eulerRotation.y, eulerRotation.z);
    scaleHandle.set3Double(trans.scale[0], trans.scale[1], trans.scale[2]);
}

