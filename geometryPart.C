#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MFnIntArrayData.h>

#include "asset.h"
#include "geometryPart.h"
#include "util.h"
#include "common.h"

GeometryPart::GeometryPart()
{
}

GeometryPart::GeometryPart(int assetId, int objectId, int geoId, int partId,
        MString partName, HAPI_GeoInfo geoInfo, Asset* objectControl)
    : assetId(assetId)
    , objectId(objectId)
    , geoId(geoId)
    , partId(partId)
    , partName(partName)
    , geoInfo(geoInfo)
    , objectControl(objectControl)
    , neverBuilt(true)
{
    // Do a full update
    HAPI_StatusCode hstat = HAPI_STATUS_SUCCESS;
    try
    {
        hstat = HAPI_GetPartInfo(assetId, objectId, geoId, partId, &partInfo);
        Util::checkHAPIStatus(hstat);
    }
    catch (HAPIError& e)
    {
        cerr << e.what() << endl;
        partInfo.clear();
    }

    //updateFaceCounts();
    //updateVertexList();
    //updatePoints();
    //updateNormals();
    //updateUVs();
}


GeometryPart::~GeometryPart() {}


void
GeometryPart::updateFaceCounts()
{
    int numFaceCount = partInfo.faceCount;
    if (numFaceCount > 0)
    {
        int myFaceCounts[numFaceCount];
        HAPI_GetFaceCounts(assetId, objectId, geoId, partId, myFaceCounts, 0, numFaceCount);
        MIntArray result(myFaceCounts, numFaceCount);

        faceCounts = result;
    }
    //cerr << "facecounts: " << faceCounts << endl;
}


void
GeometryPart::updateVertexList()
{
    int numVertexCount = partInfo.vertexCount;
    if (numVertexCount > 0)
    {
        int myVertexList[numVertexCount];
        HAPI_GetVertexList(assetId, objectId, geoId, partId, myVertexList, 0, numVertexCount);
        MIntArray result(myVertexList, numVertexCount);
        //cerr << "vertextList: " << endl;
        //cerr << result << endl;
        Util::reverseWindingOrderInt(result, faceCounts);

        vertexList = result;
    }
}


void
GeometryPart::updatePoints()
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
GeometryPart::updateNormals()
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
GeometryPart::updateUVs()
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
        Util::reverseWindingOrderFloat(Us, faceCounts);
        Util::reverseWindingOrderFloat(Vs, faceCounts);
    }

    us = Us;
    vs = Vs;
}


void
GeometryPart::update()
{
    //if (!geoInfo.hasGeoChanged)
        //return;

    HAPI_StatusCode hstat = HAPI_STATUS_SUCCESS;
    try
    {
        //hstat = HAPI_GetGeoInfo(assetId, objectId, geoId, &geoInfo);
        //Util::checkHAPIStatus(hstat);
        hstat = HAPI_GetPartInfo(assetId, objectId, geoId, partId, &partInfo);
        Util::checkHAPIStatus(hstat);
    }
    catch (HAPIError& e)
    {
        cerr << e.what() << endl;
        partInfo.clear();
    }

    updateFaceCounts();
    updateVertexList();
    updatePoints();
    updateNormals();
    updateUVs();

}


void
GeometryPart::setGeoInfo(HAPI_GeoInfo& info)
{
    geoInfo = info;
}


//=============================================================================
// Utility functions
//=============================================================================

MFloatArray
GeometryPart::getAttributeFloatData(HAPI_AttributeOwner owner, MString name)
{
    HAPI_AttributeInfo attr_info;
    attr_info.exists = false;
    attr_info.owner = owner;
    HAPI_GetAttributeInfo(assetId, objectId, geoId, partId, name.asChar(), &attr_info);

    MFloatArray ret;
    if (!attr_info.exists)
        return ret;

    int size = attr_info.count * attr_info.tupleSize;
    float data[size];
    // zero the array
    for (int j=0; j<size; j++){
        data[j] = 0;
    }
    int status = HAPI_GetAttributeFloatData(assetId, objectId, geoId, partId, name.asChar(),
            &attr_info, data, 0, attr_info.count);

    ret = MFloatArray(data, size);
    return ret;
}


MStatus
GeometryPart::compute(MDataHandle& handle)
{
    update();

    // Get plugs
    MDataHandle objectNameHandle = handle.child(AssetNodeAttributes::objectName);
    MDataHandle metaDataHandle = handle.child(AssetNodeAttributes::metaData);
    MDataHandle meshHandle = handle.child(AssetNodeAttributes::mesh);
    //MDataHandle transformHandle = handle.child(AssetNodeAttributes::transform);
    MDataHandle materialHandle = handle.child(AssetNodeAttributes::material);

    if (neverBuilt || geoInfo.hasGeoChanged)
    {
        cerr << "compute part mesh: " << partName << endl;
        // Don't output mesh for degenerate geos
        if (partInfo.pointCount == 0 || partInfo.faceCount == 0|| partInfo.vertexCount == 0)
            return MS::kFailure;


        // Object name
        objectNameHandle.set(partName);

        // Meta data
        MFnIntArrayData ffIAD;
        MIntArray metaDataArray;
        metaDataArray.append(assetId);
        metaDataArray.append(objectId);
        metaDataArray.append(geoId);
        metaDataArray.append(partId);
        MObject newMetaData = ffIAD.create(metaDataArray);
        metaDataHandle.set(newMetaData);

        // Mesh
        MObject newMeshData = createMesh();
        meshHandle.set(newMeshData);
    }

    if (neverBuilt || geoInfo.hasMaterialChanged)
    {
        updateMaterial(materialHandle);
    }

    objectNameHandle.setClean();
    meshHandle.setClean();
    handle.setClean();

    neverBuilt = false;

    return MS::kSuccess;
}


MObject
GeometryPart::createMesh()
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


void
GeometryPart::updateMaterial(MDataHandle& handle)
{
    cerr << "Update material: " << partName << endl;
    MDataHandle matExistsHandle = handle.child(AssetNodeAttributes::materialExists);
    MDataHandle ambientHandle = handle.child(AssetNodeAttributes::ambientAttr);
    MDataHandle diffuseHandle = handle.child(AssetNodeAttributes::diffuseAttr);
    MDataHandle specularHandle = handle.child(AssetNodeAttributes::specularAttr);
    MDataHandle alphaHandle = handle.child(AssetNodeAttributes::alphaAttr);
    MDataHandle texturePathHandle = handle.child(AssetNodeAttributes::texturePath);

    if (partInfo.materialId < 0)
    {
        matExistsHandle.set(false);
    } else
    {
        // get material info
        int matId = partInfo.materialId;
        HAPI_GetMaterials(assetId, &materialInfo, matId, 1);
        //materialInfo = objectControl->getMaterialInfo(matId);

        matExistsHandle.set(true);

        ambientHandle.set3Float(materialInfo.ambient[0], materialInfo.ambient[1], materialInfo.ambient[2]);
        diffuseHandle.set3Float(materialInfo.diffuse[0], materialInfo.diffuse[1], materialInfo.diffuse[2]);
        specularHandle.set3Float(materialInfo.specular[0], materialInfo.specular[1], materialInfo.specular[2]);
        
        // TODO: this doesn't do anything, HAPI does not support opacity yet
        float alpha = 1 - materialInfo.diffuse[3];
        alphaHandle.set3Float(alpha, alpha, alpha);
        
        MString texturePath = Util::getString(materialInfo.textureFilePathSH);
        texturePathHandle.set(texturePath);
    }

    
    handle.setClean();
    matExistsHandle.setClean();
    ambientHandle.setClean();
    diffuseHandle.setClean();
    specularHandle.setClean();
    alphaHandle.setClean();
    texturePathHandle.setClean();
}
