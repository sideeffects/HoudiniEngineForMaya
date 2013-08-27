#include "AssetInputMesh.h"

#include <maya/MFnMesh.h>
#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MItMeshPolygon.h>

#include "util.h"

AssetInputMesh::AssetInputMesh(int assetId, int inputIdx) :
    AssetInput(assetId, inputIdx)
{
}

AssetInputMesh::~AssetInputMesh()
{
}

AssetInput::AssetInputType
AssetInputMesh::assetInputType() const
{
    return AssetInput::AssetInputType_Mesh;
}

void
AssetInputMesh::setInput(MDataHandle &dataHandle)
{
    // extract mesh data from Maya
    MObject inputMesh = dataHandle.asMesh();
    MFnMesh fnMesh(inputMesh);
    MItMeshPolygon itMeshPoly(inputMesh);

    // get points
    MFloatPointArray points;
    fnMesh.getPoints(points);

    // get face data
    MIntArray faceCounts;
    MIntArray vertexList;
    while (!itMeshPoly.isDone())
    {
	int vc = itMeshPoly.polygonVertexCount();
	faceCounts.append(vc);
	for (int j=0; j<vc; j++)
	{
	    vertexList.append(itMeshPoly.vertexIndex(j));
	}
	itMeshPoly.next();
    }
    Util::reverseWindingOrderInt(vertexList, faceCounts);

    HAPI_GeoInputInfo inputInfo;
    HAPI_CreateGeoInput(myAssetId, myInputIdx, &inputInfo);

    // set up GeoInfo            
    HAPI_PartInfo* partInfo    = new HAPI_PartInfo();
    partInfo->id               = 0;
    partInfo->materialId       = -1;
    partInfo->faceCount        = faceCounts.length();
    partInfo->vertexCount      = vertexList.length();
    partInfo->pointCount       = points.length();

    partInfo->pointAttributeCount  = 1;
    partInfo->vertexAttributeCount = 0;
    partInfo->faceAttributeCount   = 0;
    partInfo->detailAttributeCount = 0;

    // copy data to arrays
    int * vl = new int[partInfo->vertexCount];
    int * fc = new int[partInfo->faceCount];
    vertexList.get(vl);
    faceCounts.get(fc);

    float* pos_attr = new float[ partInfo->pointCount * 3 ];
    for ( int i = 0; i < partInfo->pointCount; ++i )
	for ( int j = 0; j < 3; ++j )
	    pos_attr[ i * 3 + j ] = points[ i ][ j ];

    // Set the data
    //HAPI_SetGeoInfo(myAssetId, inputInfo.objectId, inputInfo.geoId, inputGeoInfo);
    HAPI_SetPartInfo(myAssetId, inputInfo.objectId, 
	    inputInfo.geoId, partInfo);
    HAPI_SetFaceCounts(myAssetId, inputInfo.objectId, 
	    inputInfo.geoId, fc, 0, partInfo->faceCount);
    HAPI_SetVertexList(myAssetId, inputInfo.objectId, 
	    inputInfo.geoId, vl, 0, partInfo->vertexCount);

    // Set position attributes.
    HAPI_AttributeInfo* pos_attr_info = new HAPI_AttributeInfo();
    pos_attr_info->exists             = true;
    pos_attr_info->owner              = HAPI_ATTROWNER_POINT;
    pos_attr_info->storage            = HAPI_STORAGETYPE_FLOAT;
    pos_attr_info->count              = partInfo->pointCount;
    pos_attr_info->tupleSize          = 3;
    HAPI_AddAttribute(myAssetId, inputInfo.objectId, inputInfo.geoId, "P", pos_attr_info );

    HAPI_SetAttributeFloatData(myAssetId, inputInfo.objectId, inputInfo.geoId, "P", pos_attr_info,
	    pos_attr, 0, partInfo->pointCount);

    // Commit it
    HAPI_CommitGeo(myAssetId, inputInfo.objectId, inputInfo.geoId);

    delete[] vl;
    delete[] fc;
    delete partInfo;
    delete[] pos_attr;
    delete pos_attr_info;
}
