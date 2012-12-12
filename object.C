#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MFnArrayAttrsData.h>

#include "asset.h"
#include "object.h"
#include "instancerObject.h"
#include "geometryObject.h"
#include "util.h"
#include "common.h"

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

    obj->objectControl = objControl;
    //obj->objectInfo = objInfo;

    return obj;
}


Object::Object() {}


Object::~Object() {}


Object::Object(int assetId, int objectId)
    :objectId(objectId), assetId(assetId), isInstanced(false),
    neverBuilt(true)
{
    objectControl = NULL;

}


void
Object::init()
{

    // Do a full update, ignoring what has changed
    HAPI_StatusCode hstat = HAPI_STATUS_SUCCESS;
    try
    {
        // update object
        //hstat = HAPI_GetObjects(assetId, &objectInfo, objectId, 1);
        //Util::checkHAPIStatus(hstat);
        objectInfo = objectControl->getObjectInfo(objectId);
        // update geometry
        hstat = HAPI_GetGeoInfo(assetId, objectInfo.id, 0, &geoInfo);
        Util::checkHAPIStatus(hstat);

    }
    catch (HAPIError& e)
    {
        cerr << "obj " << getId() << " " << getName() << endl;
        cerr << e.what() << endl;
    }
}


// Getters ----------------------------------------------------

int Object::getId() { return objectId; }
MString Object::getName() { return Util::getString(objectInfo.nameSH); }


void
Object::update()
{
    HAPI_StatusCode hstat = HAPI_STATUS_SUCCESS;
    try
    {
        // update object
        //hstat = HAPI_GetObjects(assetId, &objectInfo, objectId, 1);
        //Util::checkHAPIStatus(hstat);
        objectInfo = objectControl->getObjectInfo(objectId);

        // update geometry
        if (neverBuilt || objectInfo.haveGeosChanged)
        {
            hstat = HAPI_GetGeoInfo(assetId, objectInfo.id, 0, &geoInfo);
            Util::checkHAPIStatus(hstat);
        }

    }
    catch (HAPIError& e)
    {
        cerr << "obj " << getId() << " " << getName() << endl;
        cerr << e.what() << endl;
        geoInfo.clear();
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



