#include <maya/MArrayDataBuilder.h>
#include <maya/MFnIntArrayData.h>

#include "Asset.h"
#include "AssetNode.h"
#include "OutputGeometryObject.h"
#include "util.h"
#include "OutputGeometry.h"

OutputGeometryObject::OutputGeometryObject(
        int assetId,
        int objectId
        ) :
    OutputObject(
            assetId,
            objectId
          )
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
        MDataHandle& objectHandle,
        bool &needToSyncOutputs
        )
{
    update();

    MDataHandle metaDataHandle = objectHandle.child(AssetNode::outputObjectMetaData);

    // Meta data
    MFnIntArrayData ffIAD;
    MIntArray metaDataArray;
    metaDataArray.append(myAssetId);
    metaDataArray.append(myObjectId);
    MObject newMetaData = ffIAD.create(metaDataArray);
    metaDataHandle.set(newMetaData);

    if(myObjectInfo.haveGeosChanged)
    {
        for(int ii = 0; ii < myObjectInfo.geoCount; ii++)
        {
            myGeos[ii]->update();
        }
    }

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
        objectName = Util::getString(myObjectInfo.nameSH);
    }
    objectNameHandle.setString(objectName);

    MStatus stat = MS::kSuccess;
    if(myNeverBuilt || myObjectInfo.haveGeosChanged)
    {
        MDataHandle geosHandle = objectHandle.child(AssetNode::outputGeos);
        MArrayDataHandle geoArrayHandle(geosHandle);
        MArrayDataBuilder geosBuilder = geoArrayHandle.builder();
        if(geosBuilder.elementCount() != (unsigned int)(myObjectInfo.geoCount))
        {
            needToSyncOutputs = true;
        }

        for(int ii = 0; ii < myObjectInfo.geoCount; ii++)
        {
            MDataHandle geoHandle = geosBuilder.addElement(ii);
            stat = myGeos[ii]->compute(geoHandle, needToSyncOutputs);
            CHECK_MSTATUS_AND_RETURN_IT(stat);
        }

        geoArrayHandle.set(geosBuilder);

        myNeverBuilt = false;
    }

    if(myObjectInfo.hasTransformChanged)
    {
        MDataHandle transformHandle = objectHandle.child(AssetNode::outputObjectTransform);
        updateTransform(transformHandle);
    }

    return stat;
}

void OutputGeometryObject::update()
{
    const int geoCount = myObjectInfo.geoCount;

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
        OutputGeometry * geo = new OutputGeometry(myAssetId, myObjectId, i);
        myGeos.push_back(geo);
    }
}

void OutputGeometryObject::updateTransform(MDataHandle& handle)
{
    MDataHandle translateHandle = handle.child(AssetNode::outputObjectTranslate);
    MDataHandle rotateHandle = handle.child(AssetNode::outputObjectRotate);
    MDataHandle scaleHandle = handle.child(AssetNode::outputObjectScale);

    HAPI_GetObjectTransforms(myAssetId, HAPI_SRT, &myTransformInfo, myObjectId, 1);
    //transformInfo = objectControl->getTransformInfo(objectId);

    // convert to euler angle
    MEulerRotation r = MQuaternion(myTransformInfo.rotationQuaternion[0],
            myTransformInfo.rotationQuaternion[1], myTransformInfo.rotationQuaternion[2],
            myTransformInfo.rotationQuaternion[3]).asEulerRotation();

    translateHandle.set3Double((double)(myTransformInfo.position[0]), (double) myTransformInfo.position[1], (double) myTransformInfo.position[2]);
    rotateHandle.set3Double((double)r[0], (double)r[1], (double)r[2]);
    scaleHandle.set3Double((double) myTransformInfo.scale[0], (double) myTransformInfo.scale[1], (double) myTransformInfo.scale[2]);

    translateHandle.setClean();
    rotateHandle.setClean();
    scaleHandle.setClean();
    handle.setClean();
}

