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
        obj = new InstancerObject(assetId, objectId);
    else
    {
        obj = new GeometryObject(assetId, objectId);
    }

    obj->myObjectControl = objControl;
    //obj->objectInfo = objInfo;

    return obj;
}


Object::Object()
    : myObjectInfo( HAPI_ObjectInfo_Create() )
    , myGeoInfo( HAPI_GeoInfo_Create() )
{}


Object::~Object() {}


Object::Object(int assetId, int objectId) :
    myIsInstanced(false),
    myAssetId(assetId),
    myObjectId(objectId),
    myNeverBuilt(true),
    myObjectInfo( HAPI_ObjectInfo_Create() ),
    myGeoInfo( HAPI_GeoInfo_Create() )
{
    myObjectControl = NULL;

}


void
Object::init()
{

    // Do a full update, ignoring what has changed
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;
    try
    {
        // update object
        //hstat = HAPI_GetObjects(assetId, &objectInfo, objectId, 1);
        //Util::checkHAPIStatus(hstat);
        myObjectInfo = myObjectControl->getObjectInfo( myObjectId );
        // update geometry
        hstat = HAPI_GetGeoInfo( myAssetId, myObjectInfo.id, 0, &myGeoInfo);
        Util::checkHAPIStatus(hstat);

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
    try
    {
        // update object
        //hstat = HAPI_GetObjects(assetId, &objectInfo, objectId, 1);
        //Util::checkHAPIStatus(hstat);
        myObjectInfo = myObjectControl->getObjectInfo( myObjectId );

        // update geometry
        if ( myNeverBuilt || myObjectInfo.haveGeosChanged)
        {
	    //TODO: This assumes that there is only 1 geo, and the index of the geo is 0
            hstat = HAPI_GetGeoInfo( myAssetId, myObjectInfo.id, 0, &myGeoInfo);
            Util::checkHAPIStatus(hstat);
        }

    }
    catch (HAPIError& e)
    {
        cerr << e.what() << endl;
        HAPI_GeoInfo_Init( &myGeoInfo );
    }
    
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
