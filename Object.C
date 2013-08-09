#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MFnArrayAttrsData.h>

#include "Asset.h"
#include "common.h"
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


Object::Object() {}


Object::~Object() {}


Object::Object(int assetId, int objectId)
    : myObjectId(objectId), myAssetId(assetId), myIsInstanced(false),
    myNeverBuilt(true)
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
        cerr << "obj " << getId() << " " << getName() << endl;
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
        cerr << "obj " << getId() << " " << getName() << endl;
        cerr << e.what() << endl;
        myGeoInfo.clear();
    }
    
}




// test functions
void 
Object::printAttributes(HAPI_AttributeOwner owner)
{
    //int size = 0;
    //switch(owner)
    //{
        //case HAPI_ATTROWNER_VERTEX: size = geoInfo.vertexAttributeCount; break;
        //case HAPI_ATTROWNER_POINT: size = geoInfo.pointAttributeCount; break;
        //case HAPI_ATTROWNER_PRIM: size = geoInfo.faceAttributeCount; break;
        //case HAPI_ATTROWNER_DETAIL: size = geoInfo.detailAttributeCount; break;
    //}
    //cerr << "---------------" << endl;
    //cerr << "owner: " << owner << endl;
    //int data[size];
    //HAPI_GetAttributeNames(assetId, objectInfo.id, 0, owner, data, size);
    //for (int i=0; i<size; i++)
    //{
        //cerr << Util::getString(data[i]) << endl;
    //}
    //cerr << "---------------" << endl;
}
// end test functions



