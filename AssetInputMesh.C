#include "AssetInputMesh.h"

#include <maya/MFnMesh.h>
#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MMatrix.h>

#include "util.h"

AssetInputMesh::AssetInputMesh(int assetId, int inputIdx) :
    AssetInput(assetId, inputIdx),
    myInputAssetId(0)
{
    HAPI_CreateGeoInput(myAssetId, myInputIdx, &myInputInfo);
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
AssetInputMesh::setInput(MDataHandle &dataHandle, MDataHandle & dataTransformHandle )
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

    // Set the transform
    MMatrix transformMat = dataTransformHandle.asMatrix();    

    float inputMat[ 16 ];

    for( int ii = 0; ii < 4; ii++ )
    {
	for( int jj = 0; jj < 4; jj++ )
	{
	    inputMat[ii*4 + jj] = transformMat.matrix[ii][jj];
	}
    }

    HAPI_TransformEuler transformEuler;
    HAPI_ConvertMatrixToEuler( inputMat, 0, 0, &transformEuler );
    HAPI_SetObjectTransform( myInputAssetId, myInputInfo.objectId, transformEuler );


    // Set the data
    //HAPI_SetGeoInfo(myInputAssetId, myInputInfo.objectId, myInputInfo.geoId, inputGeoInfo);
    HAPI_SetPartInfo(myInputAssetId, myInputInfo.objectId, 
	    myInputInfo.geoId, partInfo);
    HAPI_SetFaceCounts(myInputAssetId, myInputInfo.objectId, 
	    myInputInfo.geoId, fc, 0, partInfo->faceCount);
    HAPI_SetVertexList(myInputAssetId, myInputInfo.objectId, 
	    myInputInfo.geoId, vl, 0, partInfo->vertexCount);

    // Set position attributes.
    HAPI_AttributeInfo* pos_attr_info = new HAPI_AttributeInfo();
    pos_attr_info->exists             = true;
    pos_attr_info->owner              = HAPI_ATTROWNER_POINT;
    pos_attr_info->storage            = HAPI_STORAGETYPE_FLOAT;
    pos_attr_info->count              = partInfo->pointCount;
    pos_attr_info->tupleSize          = 3;
    HAPI_AddAttribute(myInputAssetId, myInputInfo.objectId, myInputInfo.geoId, "P", pos_attr_info );

    HAPI_SetAttributeFloatData(myInputAssetId, myInputInfo.objectId, myInputInfo.geoId, "P", pos_attr_info,
	    pos_attr, 0, partInfo->pointCount);

    // Commit it
    HAPI_CommitGeo(myInputAssetId, myInputInfo.objectId, myInputInfo.geoId);

    delete[] vl;
    delete[] fc;
    delete partInfo;
    delete[] pos_attr;
    delete pos_attr_info;
}
