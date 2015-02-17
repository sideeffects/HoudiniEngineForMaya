#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MFnArrayAttrsData.h>

#include "Asset.h"
#include "OutputGeometryObject.h"
#include "OutputInstancerObject.h"
#include "OutputObject.h"
#include "util.h"

OutputObject*
OutputObject::createObject(
        int assetId,
        int objectId,
        const HAPI_ObjectInfo &objectInfo
        )
{
    OutputObject* obj;

    if(objectInfo.isInstancer)
        obj = new OutputInstancerObject(assetId, objectId);
    else
    {
        obj = new OutputGeometryObject(assetId, objectId);
    }

    return obj;
}

OutputObject::~OutputObject() {}

OutputObject::OutputObject(
        int assetId,
        int objectId
        ) :
    myIsInstanced(false),
    myAssetId(assetId),
    myObjectId(objectId),
    myNeverBuilt(true)
{
}

// Getters ----------------------------------------------------

int OutputObject::getId() { return myObjectId; }
MString OutputObject::getName() { return Util::getString(myObjectInfo.nameSH); }

void
OutputObject::setObjectInfo(const HAPI_ObjectInfo &objectInfo)
{
    myObjectInfo = objectInfo;
}

bool
OutputObject::isVisible() const
{
    return myObjectInfo.isVisible;
}

bool
OutputObject::isInstanced() const
{
    return myIsInstanced;
}
