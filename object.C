#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MFnArrayAttrsData.h>

#include "object.h"
#include "util.h"
#include "common.h"

Object::Object() {}


Object::Object(int objIndex, int assetId)
    :objectIndex(objIndex), assetId(assetId)
{
    update();
}


void
Object::updateFaceCounts()
{
    int numFaceCount = geoInfo.faceCount;
    if (numFaceCount > 0)
    {
        int myFaceCounts[numFaceCount];
        HAPI_GetFaceCounts(assetId, objectInfo.id, 0, myFaceCounts, 0, numFaceCount);
        MIntArray result(myFaceCounts, numFaceCount);

        faceCounts = result;
    }

}


void
Object::updateVertexList()
{
    int numVertexCount = geoInfo.vertexCount;
    if (numVertexCount > 0)
    {
        int myVertexList[numVertexCount];
        HAPI_GetVertexList(assetId, objectInfo.id, 0, myVertexList, 0, numVertexCount);
        MIntArray result(myVertexList, numVertexCount);
        reverseWindingOrderInt(result, faceCounts);

        vertexList = result;
    }
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
Object::update()
{
    // update object
    cerr << "objectIndex: " << objectIndex << endl;
    HAPI_ObjectInfo myObjects[1];
    HAPI_GetObjects(assetId, myObjects, objectIndex, 1);
    objectInfo = myObjects[0];
    if (objectInfo.isVisible)
        cerr << "object name: " << Util::getString(objectInfo.nameSH) << endl;

    //cerr << "visible: " << objectInfo.isVisible << endl;
    //cerr << "hasGeoChanged: " << objectInfo.hasGeoChanged << endl;
    //cerr << "hasTransformChanged: " << objectInfo.hasTransformChanged << endl;
    //cerr << "hasMaterialChanged: " << objectInfo.hasMaterialChanged << endl;

    // update geometry
    HAPI_GetGeoInfo(assetId, objectInfo.id, 0, &geoInfo);
    cerr << "geo id: " << geoInfo.id << endl;
    

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
    HAPI_GetAttributeInfo(assetId, objectId, 0, name, &attr_info);

    MFloatArray ret;
    if (!attr_info.exists)
        return ret;

    int size = attr_info.count * attr_info.tupleSize;
    float data[size];
    // zero the array
    for (int j=0; j<size; j++){
        data[j] = 0;
    }
    int status = HAPI_GetAttributeFloatData(assetId, objectId, 0, name, &attr_info, data, 0, attr_info.count);

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


// test functions
bool
Object::isVisible()
{
    return objectInfo.isVisible;
}


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
Object::compute(int index, const MPlug& plug, const MPlug& instancersPlug, MDataBlock& data)
{
    update();

    printAttributes(HAPI_ATTROWNER_VERTEX);
    printAttributes(HAPI_ATTROWNER_POINT);
    printAttributes(HAPI_ATTROWNER_PRIM);
    printAttributes(HAPI_ATTROWNER_DETAIL);

    if (objectInfo.isInstancer)
    //if (true)
    {
        MPlug instPlug = instancersPlug.elementByLogicalIndex(index);
        MDataHandle instHandle = data.outputValue(instPlug);
        MFnArrayAttrsData fnAAD;
        MObject instOutput = fnAAD.create();
        MVectorArray positions= fnAAD.vectorArray("position");
        positions.append(MVector(0, 0, 0));
        positions.append(MVector(1, 0, 0));
        positions.append(MVector(2, 0, 5));
        positions.append(MVector(3, 0, 2));
        instHandle.set(instOutput);
        cerr << "list fnAAD names: " << fnAAD.list() << endl;
        cerr << "instancer attr num children: " << instPlug.numChildren() << endl;
        return MS::kSuccess;
    }

    MPlug elemPlug = plug.elementByLogicalIndex(index);
    MPlug meshPlug = elemPlug.child(AssetNodeAttributes::mesh);
    MPlug transformPlug = elemPlug.child(AssetNodeAttributes::transform);
    MPlug materialPlug = elemPlug.child(AssetNodeAttributes::material);

    // instancer

    MDataHandle outHandle = data.outputValue(meshPlug);

    MObject newMeshData = createMesh();
    if (!newMeshData.isNull())
        outHandle.set(newMeshData);

    updateTransform(transformPlug, data);
    //if (asset->materialEnabled)
    updateMaterial(materialPlug, data);
    return MS::kSuccess;
}


MObject
Object::createMesh()
{
    //if (!objectInfo.isVisible)
        //return MObject();
    //if (!objectInfo.hasGeoChanged)
        //return MObject();


    if (!objectInfo.isVisible)
        return MObject();


    cerr << "Creating mesh... " << Util::getString(objectInfo.nameSH) << endl;
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

}


void Object::updateTransform(MPlug& plug, MDataBlock& data)
{
    MPlug translatePlug = plug.child(0);
    MPlug rotatePlug = plug.child(1);
    MPlug scalePlug = plug.child(2);

    if (objectInfo.isVisible)
    {
        cerr << "Updating transform..." << endl;
        // update transform
        HAPI_Transform transforms[1];
        HAPI_GetObjectTransforms(assetId, 5, transforms, objectIndex, 1);
        transformInfo = transforms[0];

        // convert to euler angle
        MEulerRotation r = MQuaternion(transformInfo.rotationQuaternion[0],
                transformInfo.rotationQuaternion[1], transformInfo.rotationQuaternion[2],
                transformInfo.rotationQuaternion[3]).asEulerRotation();


        // translate
        for (int i=0; i<HAPI_POSITION_VECTOR_SIZE; i++)
        {
            MDataHandle handle = data.outputValue(translatePlug.child(i));
            double value = transformInfo.position[i];
            handle.set(value);
        }

        // rotate
        for (int i=0; i<HAPI_EULER_VECTOR_SIZE; i++)
        {
            MDataHandle handle = data.outputValue(rotatePlug.child(i));
            double value = r[i];
            handle.set(value);
        }

        // scale
        for (int i=0; i<HAPI_SCALE_VECTOR_SIZE; i++)
        {
            MDataHandle handle = data.outputValue(scalePlug.child(i));
            double value = transformInfo.scale[i];
            handle.set(value);
        }
    }


    data.setClean(translatePlug);
    data.setClean(rotatePlug);
    data.setClean(scalePlug);

}


void
Object::updateMaterial(MPlug& plug, MDataBlock& data)
{
    MPlug matExistsPlug = plug.child(0);
    MPlug ambientPlug = plug.child(1);
    MPlug diffusePlug = plug.child(2);
    MPlug specularPlug = plug.child(3);
    MPlug texturePathPlug = plug.child(4);

    if (objectInfo.isVisible)
    {
        if (geoInfo.materialId < 0)
        {
            MDataHandle handle = data.outputValue(matExistsPlug);
            handle.set(false);
        } else
        {
            int matId = geoInfo.materialId;
            HAPI_MaterialInfo myMaterials[1];
            HAPI_GetMaterials(assetId, myMaterials, matId, 1);
            materialInfo = myMaterials[0];
            // get material info

            MDataHandle handle = data.outputValue(matExistsPlug);
            handle.set(true);

            // ambient
            for (int i=0; i<3; i++)
            {
                MDataHandle handle = data.outputValue(ambientPlug.child(i));
                float value = materialInfo.ambient[i];
                handle.set(value);
            }

            // rotate
            for (int i=0; i<3; i++)
            {
                MDataHandle handle = data.outputValue(diffusePlug.child(i));
                float value = materialInfo.diffuse[i];
                handle.set(value);
            }

            // scale
            for (int i=0; i<3; i++)
            {
                MDataHandle handle = data.outputValue(specularPlug.child(i));
                float value = materialInfo.specular[i];
                handle.set(value);
            }

            handle = data.outputValue(texturePathPlug);
            MString texturePath = Util::getString(materialInfo.textureFilePathSH);
            handle.set(texturePath);
        }
    }

    data.setClean(matExistsPlug);
    data.setClean(ambientPlug);
    data.setClean(diffusePlug);
    data.setClean(specularPlug);
    data.setClean(texturePathPlug);
}
