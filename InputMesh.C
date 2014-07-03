#include "InputMesh.h"

#include <maya/MFnMesh.h>
#include <maya/MDataBlock.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MIntArray.h>
#include <maya/MMatrix.h>

#include "util.h"

InputMesh::InputMesh(int assetId, int inputIdx) :
    Input(assetId, inputIdx),
    myInputObjectId(-1),
    myInputGeoId(-1)
{
    CHECK_HAPI(HAPI_CreateInputAsset(&myInputAssetId, NULL));

    myInputObjectId = 0;
    myInputGeoId = 0;

    CHECK_HAPI(HAPI_ConnectAssetGeometry(
                myInputAssetId, myInputObjectId,
                myAssetId, myInputIdx
                ));
}

InputMesh::~InputMesh()
{
    CHECK_HAPI(HAPI_DestroyAsset(myInputAssetId));
}

Input::AssetInputType
InputMesh::assetInputType() const
{
    return Input::AssetInputType_Mesh;
}

void
InputMesh::setInputTransform(MDataHandle &dataHandle)
{
    MMatrix transformMatrix = dataHandle.asMatrix();

    float matrix[16];
    transformMatrix.get(reinterpret_cast<float(*)[4]>(matrix));

    HAPI_TransformEuler transformEuler;
    HAPI_ConvertMatrixToEuler(matrix, HAPI_SRT, HAPI_XYZ, &transformEuler);
    HAPI_SetObjectTransform(
            myInputAssetId, myInputObjectId,
            transformEuler
            );
}

void
InputMesh::setInputGeo(
        MDataBlock &dataBlock,
        const MPlug &plug
        )
{
    MDataHandle dataHandle = dataBlock.inputValue(plug);

    // extract mesh data from Maya
    MObject meshObj = dataHandle.asMesh();

    MFnMesh meshFn(meshObj);

    // get face data
    MIntArray faceCounts;
    MIntArray vertexList;
    meshFn.getVertices(faceCounts, vertexList);
    Util::reverseWindingOrder(vertexList, faceCounts);

    // set up part info
    HAPI_PartInfo partInfo;
    HAPI_PartInfo_Init(&partInfo);
    partInfo.id = 0;
    partInfo.faceCount        = faceCounts.length();
    partInfo.vertexCount      = vertexList.length();
    partInfo.pointCount       = meshFn.numVertices();

    // copy data to arrays
    int * vl = new int[partInfo.vertexCount];
    int * fc = new int[partInfo.faceCount];
    vertexList.get(vl);
    faceCounts.get(fc);

    // Set the data
    HAPI_SetPartInfo(
            myInputAssetId, myInputObjectId, myInputGeoId,
            &partInfo
            );
    HAPI_SetFaceCounts(
            myInputAssetId, myInputObjectId, myInputGeoId,
            fc, 0, partInfo.faceCount
            );
    HAPI_SetVertexList(
            myInputAssetId, myInputObjectId, myInputGeoId,
            vl, 0, partInfo.vertexCount
            );

    // Set position attributes.
    HAPI_AttributeInfo pos_attr_info;
    pos_attr_info.exists             = true;
    pos_attr_info.owner              = HAPI_ATTROWNER_POINT;
    pos_attr_info.storage            = HAPI_STORAGETYPE_FLOAT;
    pos_attr_info.count              = meshFn.numVertices();
    pos_attr_info.tupleSize          = 3;
    HAPI_AddAttribute(
            myInputAssetId, myInputObjectId, myInputGeoId,
            "P", &pos_attr_info
            );

    HAPI_SetAttributeFloatData(
            myInputAssetId, myInputObjectId, myInputGeoId,
            "P", &pos_attr_info,
            meshFn.getRawPoints(NULL), 0, meshFn.numVertices()
            );

    // normals
    {
        // get normal IDs
        MIntArray normalCounts;
        MIntArray normalIds;
        meshFn.getNormalIds(normalCounts, normalIds);

        if(normalIds.length())
        {
            // reverse winding order
            Util::reverseWindingOrder(normalIds, faceCounts);

            // get normal values
            const float* rawNormals = meshFn.getRawNormals(NULL);

            // build the per-vertex normals
            std::vector<float> vertexNormals;
            vertexNormals.reserve(normalIds.length() * 3);
            for(unsigned int i = 0; i < normalIds.length(); ++i)
            {
                vertexNormals.push_back(rawNormals[normalIds[i] * 3 + 0]);
                vertexNormals.push_back(rawNormals[normalIds[i] * 3 + 1]);
                vertexNormals.push_back(rawNormals[normalIds[i] * 3 + 2]);
            }

            // add and set it to HAPI
            HAPI_AttributeInfo attributeInfo;
            attributeInfo.exists = true;
            attributeInfo.owner = HAPI_ATTROWNER_VERTEX;
            attributeInfo.storage = HAPI_STORAGETYPE_FLOAT;
            attributeInfo.count = normalIds.length();
            attributeInfo.tupleSize = 3;
            HAPI_AddAttribute(
                    myInputAssetId, myInputObjectId, myInputGeoId,
                    "N", &attributeInfo
                    );

            HAPI_SetAttributeFloatData(myInputAssetId, myInputObjectId, myInputGeoId,
                    "N", &attributeInfo,
                    &vertexNormals.front(), 0, normalIds.length()
                    );
        }
    }

    // UVs
    {
        // get UV IDs
        MIntArray uvCounts;
        MIntArray uvIds;
        meshFn.getAssignedUVs(uvCounts, uvIds);

        // if there's UVs
        if(uvIds.length())
        {
            // reverse winding order
            Util::reverseWindingOrder(uvIds, uvCounts);

            // get UV values
            MFloatArray uArray;
            MFloatArray vArray;
            meshFn.getUVs(uArray, vArray);

            // build the per-vertex UVs
            std::vector<float> vertexUVs;
            vertexUVs.reserve(vertexList.length() * 3);
            unsigned int uvIdIndex = 0;
            for(unsigned int i = 0; i < uvCounts.length(); ++i)
            {
                if(uvCounts[i] == faceCounts[i])
                {
                    // has UVs assigned
                    for(int j = 0; j < uvCounts[i]; ++j)
                    {
                        vertexUVs.push_back(uArray[uvIds[uvIdIndex]]);
                        vertexUVs.push_back(vArray[uvIds[uvIdIndex]]);
                        vertexUVs.push_back(0);

                        uvIdIndex++;
                    }
                }
                else
                {
                    // no UVs assigned
                    for(int j = 0; j < faceCounts[i]; ++j)
                    {
                        vertexUVs.push_back(0);
                        vertexUVs.push_back(0);
                        vertexUVs.push_back(0);
                    }
                }
            }

            // add and set it to HAPI
            HAPI_AttributeInfo attributeInfo;
            attributeInfo.exists = true;
            attributeInfo.owner = HAPI_ATTROWNER_VERTEX;
            attributeInfo.storage = HAPI_STORAGETYPE_FLOAT;
            attributeInfo.count = vertexList.length();
            attributeInfo.tupleSize = 3;
            HAPI_AddAttribute(
                    myInputAssetId, myInputObjectId, myInputGeoId,
                    "uv", &attributeInfo
                    );

            HAPI_SetAttributeFloatData(
                    myInputAssetId, myInputObjectId, myInputGeoId,
                    "uv", &attributeInfo,
                    &vertexUVs.front(), 0, vertexList.length()
                    );
        }
    }

    // Commit it
    HAPI_CommitGeo(myInputAssetId, myInputObjectId, myInputGeoId);

    delete[] vl;
    delete[] fc;
}
