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
    : myAssetId(assetId)
    , myObjectId(objectId)
    , myGeoId(geoId)
    , myPartId(partId)
    , myPartName(partName)
    , myGeoInfo(geoInfo)
    , myObjectControl(objectControl)
    , myNeverBuilt(true)
{
    // Do a full update
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;
    try
    {
        hstat = HAPI_GetPartInfo(assetId, objectId, geoId, partId, & myPartInfo);
        Util::checkHAPIStatus(hstat);
    }
    catch (HAPIError& e)
    {
        cerr << e.what() << endl;
        myPartInfo.clear();
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
    int numFaceCount = myPartInfo.faceCount;
    if (numFaceCount > 0)
    {
        int * tempFaceCounts = new int[numFaceCount];
        HAPI_GetFaceCounts( myAssetId, myObjectId, myGeoId, myPartId, tempFaceCounts, 0, numFaceCount);
        MIntArray result(tempFaceCounts, numFaceCount);

        myFaceCounts = result;

        delete[] tempFaceCounts;
    }
    //cerr << "facecounts: " << faceCounts << endl;
}


void
GeometryPart::updateVertexList()
{
    int numVertexCount = myPartInfo.vertexCount;
    if (numVertexCount > 0)
    {
        int * tempVertexList = new int[numVertexCount];
        HAPI_GetVertexList( myAssetId, myObjectId, myGeoId, myPartId, tempVertexList, 0, numVertexCount);
        MIntArray result(tempVertexList, numVertexCount);
        //cerr << "vertextList: " << endl;
        //cerr << result << endl;
        Util::reverseWindingOrderInt(result, myFaceCounts);

        myVertexList = result;

	delete[] tempVertexList;
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

    myPoints = result;
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

    myNormals = result;
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
        Util::reverseWindingOrderFloat(Us, myFaceCounts);
        Util::reverseWindingOrderFloat(Vs, myFaceCounts);
    }

    myUs = Us;
    myVs = Vs;
}


void
GeometryPart::update()
{
    //if (!geoInfo.hasGeoChanged)
        //return;

    HAPI_Result hstat = HAPI_RESULT_SUCCESS;
    try
    {
        //hstat = HAPI_GetGeoInfo(assetId, objectId, geoId, &geoInfo);
        //Util::checkHAPIStatus(hstat);
        hstat = HAPI_GetPartInfo( myAssetId, myObjectId, myGeoId, myPartId, &myPartInfo);
        Util::checkHAPIStatus(hstat);
    }
    catch (HAPIError& e)
    {
        cerr << e.what() << endl;
        myPartInfo.clear();
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
    myGeoInfo = info;
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
    HAPI_GetAttributeInfo( myAssetId, myObjectId, myGeoId, myPartId, name.asChar(), &attr_info);

    MFloatArray ret;
    if (!attr_info.exists)
        return ret;

    int size = attr_info.count * attr_info.tupleSize;
    float * data = new float[size];
    // zero the array
    for (int j=0; j<size; j++){
        data[j] = 0;
    }
    int status = HAPI_GetAttributeFloatData( myAssetId, myObjectId, myGeoId, myPartId, name.asChar(),
            &attr_info, data, 0, attr_info.count);

    ret = MFloatArray(data, size);

    delete[] data;
    return ret;
}


bool
GeometryPart::hasMesh()
{
    update();
    if ( myPartInfo.pointCount == 0 || myPartInfo.faceCount == 0 || myPartInfo.vertexCount == 0)
        return false;
    return true;
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

    // Don't output mesh for degenerate geos
    if ( myPartInfo.pointCount == 0 || myPartInfo.faceCount == 0|| myPartInfo.vertexCount == 0)
        return MS::kFailure;

    if ( myNeverBuilt || myGeoInfo.hasGeoChanged)
    {
        cerr << "compute part mesh: " << myPartName << endl;


        // Object name
        objectNameHandle.set( myPartName );

        // Meta data
        MFnIntArrayData ffIAD;
        MIntArray metaDataArray;
        metaDataArray.append( myAssetId );
        metaDataArray.append( myObjectId );
        metaDataArray.append( myGeoId );
        metaDataArray.append( myPartId );
        MObject newMetaData = ffIAD.create(metaDataArray);
        metaDataHandle.set(newMetaData);

        // Mesh
        MObject newMeshData = createMesh();
        meshHandle.set(newMeshData);
    }

    if ( myNeverBuilt || myGeoInfo.hasMaterialChanged)
    {
        updateMaterial(materialHandle);
    }

    objectNameHandle.setClean();
    meshHandle.setClean();
    handle.setClean();

    myNeverBuilt = false;

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
    MObject newMesh = meshFS.create( myPoints.length(), myFaceCounts.length(),
            myPoints, myFaceCounts, myVertexList, outData);

    // set normals
    if ( myNormals.length() > 0)
    {
        MIntArray vlist;
        for (int j=0; j< myPoints.length(); j++)
            vlist.append(j);
        meshFS.setVertexNormals( myNormals, vlist);
    }

    // set UVs
    if ( myUs.length() > 0)
    {
        meshFS.setUVs( myUs, myVs);
        MIntArray uvIds;
        for (int j=0; j< myVertexList.length(); j++)
            uvIds.append(j);
        meshFS.assignUVs( myFaceCounts, uvIds);
    }

    return outData;

}


void
GeometryPart::updateMaterial(MDataHandle& handle)
{
    cerr << "Update material: " << myPartName << endl;
    MDataHandle matExistsHandle = handle.child(AssetNodeAttributes::materialExists);
    MDataHandle ambientHandle = handle.child(AssetNodeAttributes::ambientAttr);
    MDataHandle diffuseHandle = handle.child(AssetNodeAttributes::diffuseAttr);
    MDataHandle specularHandle = handle.child(AssetNodeAttributes::specularAttr);
    MDataHandle alphaHandle = handle.child(AssetNodeAttributes::alphaAttr);
    MDataHandle texturePathHandle = handle.child(AssetNodeAttributes::texturePath);

    if ( myPartInfo.materialId < 0)
    {
        matExistsHandle.set(false);
    } else
    {
        // get material info
        int matId = myPartInfo.materialId;
        HAPI_GetMaterials( myAssetId, & myMaterialInfo, matId, 1);
        //materialInfo = objectControl->getMaterialInfo(matId);

        matExistsHandle.set(true);

        ambientHandle.set3Float( myMaterialInfo.ambient[0], myMaterialInfo.ambient[1], myMaterialInfo.ambient[2]);
        diffuseHandle.set3Float( myMaterialInfo.diffuse[0], myMaterialInfo.diffuse[1], myMaterialInfo.diffuse[2]);
        specularHandle.set3Float(myMaterialInfo.specular[0], myMaterialInfo.specular[1], myMaterialInfo.specular[2]);
        
        float alpha = 1 - myMaterialInfo.diffuse[3];
        alphaHandle.set3Float(alpha, alpha, alpha);
        
        MString texturePath = Util::getString( myMaterialInfo.textureFilePathSH);
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
