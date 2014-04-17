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
OutputObject::createObject(int assetId, int objectId, Asset* objControl)
{
    OutputObject* obj;

    HAPI_ObjectInfo objInfo;
    //HAPI_GetObjects(assetId, &objInfo, objectId, 1);
    objInfo = objControl->getObjectInfo(objectId);

    if(objInfo.isInstancer)
        obj = new OutputInstancerObject(assetId, objectId, objControl);
    else
    {
        obj = new OutputGeometryObject(assetId, objectId, objControl);
    }

    return obj;
}

OutputObject::~OutputObject() {}

OutputObject::OutputObject(
        int assetId,
        int objectId,
        Asset* objectControl
        ) :
    myObjectControl(objectControl),
    myIsInstanced(false),
    myAssetId(assetId),
    myObjectId(objectId),
    myNeverBuilt(true)
{
    // Do a full update, ignoring what has changed
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;
    try
    {
        // update object
        //hstat = HAPI_GetObjects(assetId, &objectInfo, objectId, 1);
        //Util::checkHAPIStatus(hstat);
        myObjectInfo = myObjectControl->getObjectInfo(myObjectId);
    }
    catch (HAPIError& e)
    {
        cerr << e.what() << endl;
    }
}

// Getters ----------------------------------------------------

int OutputObject::getId() { return myObjectId; }
MString OutputObject::getName() { return Util::getString(myObjectInfo.nameSH); }

void
OutputObject::update()
{
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;
    myObjectInfo = myObjectControl->getObjectInfo(myObjectId);
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
