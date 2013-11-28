#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MFnArrayAttrsData.h>

#include "Asset.h"
#include "GeometryObject.h"
#include "InstancerObject.h"
#include "Object.h"
#include "util.h"

Object* 
Object::createObject(int assetId, int objectId, Asset* objControl)
{
    Object* obj;
    
    HAPI_ObjectInfo objInfo;
    //HAPI_GetObjects(assetId, &objInfo, objectId, 1);
    objInfo = objControl->getObjectInfo(objectId);

    if (objInfo.isInstancer)
        obj = new InstancerObject(assetId, objectId, objControl);
    else
    {
        obj = new GeometryObject(assetId, objectId, objControl);
    }

    return obj;
}


Object::~Object() {}


Object::Object(
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
        myObjectInfo = myObjectControl->getObjectInfo( myObjectId );        
        

    }
    catch (HAPIError& e)
    {
        cerr << e.what() << endl;
    }
}


// Getters ----------------------------------------------------

int Object::getId() { return myObjectId; }
MString Object::getName() { return Util::getString( myObjectInfo.nameSH); }


void
Object::update()
{
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;
    myObjectInfo = myObjectControl->getObjectInfo( myObjectId );    
    
}


bool	
Object::isVisible() const
{
    return myObjectInfo.isVisible;
}


bool	
Object::isInstanced() const
{
    return myIsInstanced;
}
