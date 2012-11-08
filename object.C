#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MFnArrayAttrsData.h>

#include "object.h"
#include "instancerObject.h"
#include "geometryObject.h"
#include "util.h"
#include "common.h"

Object* 
Object::createObject(int assetId, int objectId)
{
    Object* obj;
    
    HAPI_ObjectInfo myObjects[1];
    HAPI_GetObjects(assetId, myObjects, objectId, 1);
    HAPI_ObjectInfo objInfo = myObjects[0];

    if (objInfo.isInstancer)
        obj = new InstancerObject(assetId, objectId);
    else
        obj = new GeometryObject(assetId, objectId);

    return obj;
}


Object::Object() {}


Object::Object(int assetId, int objectId)
    :objectId(objectId), assetId(assetId), isInstanced(false)
{
    update();
    objectControl = NULL;
}


// Getters ----------------------------------------------------

int Object::getId() { return objectId; }
MString Object::getName() { return Util::getString(objectInfo.nameSH); }

void
Object::update()
{
    // update object
    HAPI_ObjectInfo myObjects[1];
    HAPI_GetObjects(assetId, myObjects, objectId, 1);
    objectInfo = myObjects[0];

    // update geometry
    HAPI_GetGeoInfo(assetId, objectInfo.id, 0, &geoInfo);
    //cerr << "object name: " << Util::getString(objectInfo.nameSH) << endl;
    //cerr << "object id: " << objectInfo.id << endl;
    


}


// Utility ---------------------------------------------------------------------

MFloatArray
Object::getAttributeFloatData(HAPI_AttributeOwner owner, MString name)
{
    int objectId = objectInfo.id;

    HAPI_AttributeInfo attr_info;
    attr_info.exists = false;
    attr_info.owner = owner;
    HAPI_GetAttributeInfo(assetId, objectId, 0, name.asChar(), &attr_info);

    MFloatArray ret;
    if (!attr_info.exists)
        return ret;

    int size = attr_info.count * attr_info.tupleSize;
    float data[size];
    // zero the array
    for (int j=0; j<size; j++){
        data[j] = 0;
    }
    int status = HAPI_GetAttributeFloatData(assetId, objectId, 0, name.asChar(), &attr_info, data, 0, attr_info.count);

    ret = MFloatArray(data, size);
    return ret;
}


MStringArray
Object::getAttributeStringData(HAPI_AttributeOwner owner, MString name)
{
    int objectId = objectInfo.id;

    HAPI_AttributeInfo attr_info;
    attr_info.exists = false;
    attr_info.owner = owner;
    HAPI_GetAttributeInfo(assetId, objectId, 0, name.asChar(), &attr_info);

    MStringArray ret;
    if (!attr_info.exists)
        return ret;

    int size = attr_info.count * attr_info.tupleSize;
    int data[size];
    // zero the array
    for (int j=0; j<size; j++){
        data[j] = 0;
    }
    int status = HAPI_GetAttributeStrData(assetId, objectId, 0, name.asChar(), &attr_info, data, 0, attr_info.count);

    for (int j=0; j<size; j++){
        ret.append(Util::getString(data[j]));
    }

    return ret;
}


void
Object::reverseWindingOrderInt(MIntArray& data, MIntArray& faceCounts)
{
    int current_index = 0;
    int numFaceCount = faceCounts.length();
    for (int i=0; i<numFaceCount; i++)
    {
        int vertex_count = faceCounts[i];
        int a = current_index;
        int b = current_index + vertex_count - 1;
        while (a < b)
        {
            int temp = data[a];
            data[a] = data[b];
            data[b] = temp;
            a++;
            b--;
        }
        current_index += vertex_count;
    }
}


void
Object::reverseWindingOrderFloat(MFloatArray& data, MIntArray& faceCounts)
{
    int current_index = 0;
    int numFaceCount = faceCounts.length();
    for (int i=0; i<numFaceCount; i++)
    {
        int vertex_count = faceCounts[i];
        int a = current_index;
        int b = current_index + vertex_count - 1;
        while (a < b)
        {
            float temp = data[a];
            data[a] = data[b];
            data[b] = temp;
            a++;
            b--;
        }
        current_index += vertex_count;
    }
}


// test functions
void 
Object::printAttributes(HAPI_AttributeOwner owner)
{
    int size = 0;
    switch(owner)
    {
        case HAPI_ATTROWNER_VERTEX: size = geoInfo.vertexAttributeCount; break;
        case HAPI_ATTROWNER_POINT: size = geoInfo.pointAttributeCount; break;
        case HAPI_ATTROWNER_PRIM: size = geoInfo.faceAttributeCount; break;
        case HAPI_ATTROWNER_DETAIL: size = geoInfo.detailAttributeCount; break;
    }
    cerr << "---------------" << endl;
    cerr << "owner: " << owner << endl;
    int data[size];
    HAPI_GetAttributeNames(assetId, objectInfo.id, 0, owner, data, size);
    for (int i=0; i<size; i++)
    {
        cerr << Util::getString(data[i]) << endl;
    }
    cerr << "---------------" << endl;
}
// end test functions


MStatus
Object::compute(const MPlug& plug, MDataBlock& data)
{
    return MS::kSuccess;
}


