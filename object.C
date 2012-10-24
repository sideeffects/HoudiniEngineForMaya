#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>

#include "object.h"

Object::Object() {}


Object::Object(HAPI_ObjectInfo objInfo, int assetId)
    :objectInfo(objInfo), assetId(assetId)
{
    HAPI_GetDetailInfo(assetId, objectInfo.id, &detailInfo);
}


void
Object::updateFaceCounts()
{
    int numFaceCount = detailInfo.faceCount;
    int myFaceCounts[numFaceCount];
    HAPI_GetFaceCounts(assetId, objectInfo.id, myFaceCounts, 0, numFaceCount);
    MIntArray result(myFaceCounts, numFaceCount);

    faceCounts = result;
}


void
Object::updateVertexList()
{
    int numVertexCount = detailInfo.vertexCount;
    int myVertexList[numVertexCount];
    HAPI_GetVertexList(assetId, objectInfo.id, myVertexList, 0, numVertexCount);
    MIntArray result(myVertexList, numVertexCount);
    reverseWindingOrderInt(result, faceCounts);

    vertexList = result;
}


void
Object::updatePoints()
{
    MFloatArray data = getAttributeFloatData(HAPI_ATTROWNER_POINT, "P");
    // make a maya point array, assume 3 tuple
    MFloatPointArray result;
    int i = 0;
    int len = data.length();
    while (i < len)
    {
        MFloatPoint v(data[i], data[i+1], data[i+2]);
        result.append(v);
        i = i+3;
    }

    points = result;
}


void
Object::updateNormals()
{
    MFloatArray data = getAttributeFloatData(HAPI_ATTROWNER_POINT, "N");
    // make a maya vector array, assume 3 tuple
    MVectorArray result;

    if (data.length() > 0)
    {
        int i = 0;
        int len = data.length();
        while (i < len)
        {
            MFloatVector v(data[i], data[i+1], data[i+2]);
            result.append(v);
            i = i+3;
        }
    }

    normals = result;
}


void
Object::updateUVs()
{
    MFloatArray data = getAttributeFloatData(HAPI_ATTROWNER_VERTEX, "uv");
    // split UVs into two arrays, assume 3 tuple
    MFloatArray Us;
    MFloatArray Vs;

    if (data.length() > 0)
    {
        int i = 0;
        int len = data.length();
        while (i < len)
        {
            Us.append(data[i]);
            Vs.append(data[i+1]);
            i = i+3;
        }
        reverseWindingOrderFloat(Us, faceCounts);
        reverseWindingOrderFloat(Vs, faceCounts);
    }

    us = Us;
    vs = Vs;
}


void
Object::updateGeometry()
{
    updateFaceCounts();
    updateVertexList();
    updatePoints();
    updateNormals();
    updateUVs();
}


MFloatArray
Object::getAttributeFloatData(HAPI_AttributeOwner owner, char* name)
{
    int objectId = objectInfo.id;

    HAPI_AttributeInfo attr_info;
    attr_info.exists = false;
    attr_info.owner = owner;
    HAPI_GetAttributeInfo(assetId, objectId, name, &attr_info);

    MFloatArray ret;
    if (!attr_info.exists)
        return ret;

    int size = attr_info.count * attr_info.tupleSize;
    float data[size];
    // zero the array
    for (int j=0; j<size; j++){
        data[j] = 0;
    }
    int status = HAPI_GetAttributeFloatData(assetId, objectId, name, &attr_info, data, 0, attr_info.count);

    ret = MFloatArray(data, size);
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


MObject
Object::createMesh()
{
    cerr << "Creating mesh..." << endl;
    updateGeometry();

        // Mesh data
    MFnMeshData dataCreator;
    MObject outData = dataCreator.create();

    // create mesh
    MFnMesh meshFS;
    MObject newMesh = meshFS.create(points.length(), faceCounts.length(),
            points, faceCounts, vertexList, outData);

    // set normals
    if (normals.length() > 0)
    {
        MIntArray vlist;
        for (int j=0; j<points.length(); j++)
            vlist.append(j);
        meshFS.setVertexNormals(normals, vlist);
    }

    // set UVs
    if (us.length() > 0)
    {
        meshFS.setUVs(us, vs);
        MIntArray uvIds;
        for (int j=0; j<vertexList.length(); j++)
            uvIds.append(j);
        meshFS.assignUVs(faceCounts, uvIds);
    }

    return outData;

    // debug
    //MFloatPointArray tmp1;
    //meshFS.getPoints(tmp1);
    //MFloatVectorArray tmp2;
    //meshFS.getVertexNormals(false, tmp2);
    //MFloatArray tmp3;
    //MFloatArray tmp4;
    //meshFS.getUVs(tmp3, tmp4);

    //cerr << "print points" << endl;
    //cerr << tmp1 << endl;
    //cerr << "print uvs" << endl;
    //cerr << tmp3 << endl;
    //cerr << tmp4 << endl;
    // end debug;

}


void Object::updateTransform(MPlug& plug, MDataBlock& data)
{
    cerr << "Updating transform..." << endl;

    int index = plug.logicalIndex();
    HAPI_Transform transforms[1];
    HAPI_GetObjectTransforms(assetId, 0, transforms, index, 1);

    HAPI_Transform transform = transforms[0];

    // convert to euler angle
    MEulerRotation r = MQuaternion(transform.rotationQuaternion[0],
            transform.rotationQuaternion[1], transform.rotationQuaternion[2],
            transform.rotationQuaternion[3]).asEulerRotation();

    // set the data
    MPlug translatePlug = plug.child(0);
    MPlug rotatePlug = plug.child(1);
    MPlug scalePlug = plug.child(2);

    // translate
    for (int i=0; i<3; i++)
    {
        MDataHandle handle = data.outputValue(translatePlug.child(i));
        double value = transform.position[i];
        handle.set(value);
    }

    // rotate
    for (int i=0; i<3; i++)
    {
        MDataHandle handle = data.outputValue(rotatePlug.child(i));
        double value = r[i];
        handle.set(value);
    }

    // scale
    for (int i=0; i<3; i++)
    {
        MDataHandle handle = data.outputValue(scalePlug.child(i));
        double value = transform.scale[i];
        handle.set(value);
    }


    data.setClean(translatePlug);
    data.setClean(rotatePlug);
    data.setClean(scalePlug);

}
