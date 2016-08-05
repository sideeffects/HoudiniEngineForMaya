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
        HAPI_NodeId nodeId
        )
{
    HAPI_Result hapiResult;

    HAPI_ObjectInfo objectInfo;
    hapiResult = HAPI_GetObjectInfo(
            Util::theHAPISession.get(),
            nodeId, &objectInfo
            );
    CHECK_HAPI(hapiResult);

    OutputObject* obj;

    if(objectInfo.isInstancer)
        obj = new OutputInstancerObject(nodeId);
    else
    {
        obj = new OutputGeometryObject(nodeId);
    }

    return obj;
}

OutputObject::~OutputObject() {}

OutputObject::OutputObject(
        HAPI_NodeId nodeId
        ) :
    myIsInstanced(false),
    myNodeId(nodeId),
    myLastCookCount(0)
{
}

// Getters ----------------------------------------------------

int OutputObject::getId() { return myNodeInfo.id; }
MString OutputObject::getName() { return Util::HAPIString(myObjectInfo.nameSH); }

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
