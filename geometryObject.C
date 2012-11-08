#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MFnArrayAttrsData.h>

#include "geometryObject.h"
#include "util.h"
#include "common.h"

GeometryObject::GeometryObject() {}


GeometryObject::GeometryObject(int assetId, int objectId)
    :Object(assetId, objectId)
{
    update();
}


Object::ObjectType
GeometryObject::type()
{
    return Object::OBJECT_TYPE_GEOMETRY;
}


void
GeometryObject::updateFaceCounts()
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
GeometryObject::updateVertexList()
{
    int numVertexCount = geoInfo.vertexCount;
    if (numVertexCount > 0)
    {
        int myVertexList[numVertexCount];
        HAPI_GetVertexList(assetId, objectInfo.id, 0, myVertexList, 0, numVertexCount);
        MIntArray result(myVertexList, numVertexCount);
        //cerr << "vertextList: " << endl;
        //cerr << result << endl;
        reverseWindingOrderInt(result, faceCounts);

        vertexList = result;
    }
}


void
GeometryObject::updatePoints()
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
GeometryObject::updateNormals()
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
GeometryObject::updateUVs()
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
GeometryObject::update()
{
    Object::update();

    updateFaceCounts();
    updateVertexList();
    updatePoints();
    updateNormals();
    updateUVs();

}


MStatus
GeometryObject::compute(const MPlug& plug, MDataBlock& data)
{
    update();

    //printAttributes(HAPI_ATTROWNER_VERTEX);
    //printAttributes(HAPI_ATTROWNER_POINT);
    //printAttributes(HAPI_ATTROWNER_PRIM);
    //printAttributes(HAPI_ATTROWNER_DETAIL);

    //cerr << "isVisible: " << objectInfo.isVisible << " isInstanced: " << isInstanced << endl;
    if (!objectInfo.isVisible && !isInstanced)
        return MS::kFailure;

    if (geoInfo.pointCount == 0 || geoInfo.faceCount == 0|| geoInfo.vertexCount == 0)
        return MS::kFailure;

    //cerr << "compute geometry object: " << objectInfo.id << " " << Util::getString(objectInfo.nameSH) << endl;
    MPlug objectNamePlug = plug.child(AssetNodeAttributes::objectName);
    MPlug meshPlug = plug.child(AssetNodeAttributes::mesh);
    MPlug transformPlug = plug.child(AssetNodeAttributes::transform);
    MPlug materialPlug = plug.child(AssetNodeAttributes::material);

    MDataHandle objectNameHandle = data.outputValue(objectNamePlug);
    objectNameHandle.set(Util::getString(objectInfo.nameSH));

    MDataHandle outHandle = data.outputValue(meshPlug);

    MObject newMeshData = createMesh();
    outHandle.set(newMeshData);

    updateTransform(transformPlug, data);
    updateMaterial(materialPlug, data);

    data.setClean(objectNamePlug);
    data.setClean(meshPlug);

    return MS::kSuccess;
}


MObject
GeometryObject::createMesh()
{

    //cerr << "Creating mesh... " << Util::getString(objectInfo.nameSH) << endl;
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


void GeometryObject::updateTransform(MPlug& plug, MDataBlock& data)
{
    MPlug translatePlug = plug.child(0);
    MPlug rotatePlug = plug.child(1);
    MPlug scalePlug = plug.child(2);

    //if (objectInfo.isVisible)
    //{
        //cerr << "Updating transform..." << endl;
        // update transform
        HAPI_Transform transforms[1];
        HAPI_GetObjectTransforms(assetId, 5, transforms, objectId, 1);
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
    //}


    data.setClean(plug);
    data.setClean(translatePlug);
    data.setClean(rotatePlug);
    data.setClean(scalePlug);

}


void
GeometryObject::updateMaterial(MPlug& plug, MDataBlock& data)
{
    MPlug matExistsPlug = plug.child(0);
    MPlug ambientPlug = plug.child(1);
    MPlug diffusePlug = plug.child(2);
    MPlug specularPlug = plug.child(3);
    MPlug texturePathPlug = plug.child(4);

    //cerr << "Updating materials .................." << endl;

    //if (objectInfo.isVisible)
    //{
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

            // diffuse
            for (int i=0; i<3; i++)
            {
                MDataHandle handle = data.outputValue(diffusePlug.child(i));
                float value = materialInfo.diffuse[i];
                handle.set(value);
            }

            // specular
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
    //}

    
    data.setClean(plug);
    data.setClean(matExistsPlug);
    data.setClean(ambientPlug);
    data.setClean(diffusePlug);
    data.setClean(specularPlug);
    data.setClean(texturePathPlug);
}
