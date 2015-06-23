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
    CHECK_HAPI(HAPI_CreateInputAsset(
        &myInputAssetId,
        NULL
        ));
    if(!Util::statusCheckLoop())
    {
        DISPLAY_ERROR(MString("Unexpected error when creating input asset."));
    }

    myInputObjectId = 0;
    myInputGeoId = 0;

    CHECK_HAPI(HAPI_ConnectAssetGeometry(
                myInputAssetId, myInputObjectId,
                myAssetId, myInputIdx
                ));
}

InputMesh::~InputMesh()
{
    CHECK_HAPI(HAPI_DestroyAsset(
                myInputAssetId
                ));
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
    HAPI_ConvertMatrixToEuler(
            matrix,
            HAPI_SRT,
            HAPI_XYZ,
            &transformEuler
            );
    HAPI_SetObjectTransform(
            myInputAssetId, myInputObjectId,
            &transformEuler
            );
}

static
MString
getLayerName(const char *name, int layer)
{
    MString layerName = name;
    if(layer > 0)
    {
        layerName += layer + 1;
    }

    return layerName;
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
    std::vector<int> vertexCount;
    std::vector<int> vertexList;
    {
        MIntArray mayaVertexCount;
        MIntArray mayaVertexList;
        meshFn.getVertices(mayaVertexCount, mayaVertexList);

        vertexCount.resize(mayaVertexCount.length());
        mayaVertexCount.get(&vertexCount[0]);

        vertexList.resize(mayaVertexList.length());
        mayaVertexList.get(&vertexList[0]);
    }
    Util::reverseWindingOrder(vertexList, vertexCount);

    // set up part info
    HAPI_PartInfo partInfo;
    HAPI_PartInfo_Init(&partInfo);
    partInfo.id = 0;
    partInfo.faceCount        = vertexCount.size();
    partInfo.vertexCount      = vertexList.size();
    partInfo.pointCount       = meshFn.numVertices();

    // Set the data
    HAPI_SetPartInfo(
            myInputAssetId, myInputObjectId, myInputGeoId,
            &partInfo
            );
    HAPI_SetFaceCounts(
            myInputAssetId, myInputObjectId, myInputGeoId,
            &vertexCount[0],
            0, partInfo.faceCount
            );
    HAPI_SetVertexList(
            myInputAssetId, myInputObjectId, myInputGeoId,
            &vertexList[0],
            0, partInfo.vertexCount
            );

    // Set position attributes.
    processPoints(meshFn);

    // normals
    processNormals(meshFn, vertexCount);

    // UVs
    processUVs(meshFn, vertexCount, vertexList);

    // Colors and Alphas
    processColorSets(meshFn, vertexCount, vertexList);

    Input::setInputPlugMetaData(
            plug,
            myInputAssetId, myInputObjectId, myInputGeoId
            );

    // Commit it
    HAPI_CommitGeo(
            myInputAssetId, myInputObjectId, myInputGeoId
            );
}

bool
InputMesh::processPoints(
        const MFnMesh &meshFn
        )
{
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

    return true;
}

bool
InputMesh::processNormals(
        const MFnMesh &meshFn,
        std::vector<int> vertexCount
        )
{
    // get normal IDs
    MIntArray normalCounts;
    MIntArray normalIds;
    meshFn.getNormalIds(normalCounts, normalIds);

    if(normalIds.length())
    {
        return false;
    }

    // reverse winding order
    Util::reverseWindingOrder(normalIds, vertexCount);

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

    HAPI_SetAttributeFloatData(
            myInputAssetId, myInputObjectId, myInputGeoId,
            "N", &attributeInfo,
            &vertexNormals.front(), 0, normalIds.length()
            );

    return true;
}

bool
InputMesh::processUVs(
        const MFnMesh &meshFn,
        std::vector<int> vertexCount,
        std::vector<int> vertexList
        )
{
    MString currentUVSetName;
    currentUVSetName = meshFn.currentUVSetName();

    MStringArray uvSetNames;
    meshFn.getUVSetNames(uvSetNames);

    MStringArray mappedUVAttributeNames;
    mappedUVAttributeNames.setLength(uvSetNames.length());

    for(unsigned int i = 0; i < uvSetNames.length(); i++)
    {
        const MString uvSetName = uvSetNames[i];

        const MString uvAttributeName = getLayerName("uv", i);

        mappedUVAttributeNames[i] = uvAttributeName;

        // get UV IDs
        MIntArray uvCounts;
        MIntArray uvIds;
        meshFn.getAssignedUVs(uvCounts, uvIds, &uvSetName);

        // reverse winding order
        Util::reverseWindingOrder(uvIds, uvCounts);

        // get UV values
        MFloatArray uArray;
        MFloatArray vArray;
        meshFn.getUVs(uArray, vArray, &uvSetName);

        // build the per-vertex UVs
        std::vector<float> vertexUVs;
        vertexUVs.reserve(vertexList.size() * 3);
        unsigned int uvIdIndex = 0;
        for(unsigned int i = 0; i < uvCounts.length(); ++i)
        {
            if(uvCounts[i] == vertexCount[i])
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
                for(int j = 0; j < vertexCount[i]; ++j)
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
        attributeInfo.count = vertexList.size();
        attributeInfo.tupleSize = 3;
        HAPI_AddAttribute(
                myInputAssetId, myInputObjectId, myInputGeoId,
                uvAttributeName.asChar(), &attributeInfo
                );

        HAPI_SetAttributeFloatData(
                myInputAssetId, myInputObjectId, myInputGeoId,
                uvAttributeName.asChar(), &attributeInfo,
                &vertexUVs.front(), 0, vertexList.size()
                );
    }

    Input::setDetailAttribute(
            myInputAssetId,
            myInputObjectId,
            myInputGeoId,
            "maya_uv_name",
            uvSetNames
            );

    Input::setDetailAttribute(
            myInputAssetId,
            myInputObjectId,
            myInputGeoId,
            "maya_uv_mapped_uv",
            mappedUVAttributeNames
            );

    return true;
}

bool
InputMesh::processColorSets(
        const MFnMesh &meshFn,
        std::vector<int> vertexCount,
        std::vector<int> vertexList
        )
{
    MString currentColorSetName;
    currentColorSetName = meshFn.currentColorSetName();

    MStringArray colorSetNames;
    meshFn.getColorSetNames(colorSetNames);

    MStringArray mappedCdNames;
    MStringArray mappedAlphaNames;
    mappedCdNames.setLength(colorSetNames.length());
    mappedAlphaNames.setLength(colorSetNames.length());

    MColor defaultUnsetColor;
    MColorArray colors;
    std::vector<float> buffer;
    for(unsigned int i = 0; i < colorSetNames.length(); i++)
    {
        const MString colorSetName = colorSetNames[i];

        bool hasColor = false;
        bool hasAlpha = false;
        {
            MFnMesh::MColorRepresentation colorSetRepresentation =
                meshFn.getColorRepresentation(colorSetName);

            switch(colorSetRepresentation)
            {
                case MFnMesh::kAlpha:
                    hasAlpha = true;
                    break;
                case MFnMesh::kRGB:
                    hasColor = true;
                    break;
                case MFnMesh::kRGBA:
                    hasColor = true;
                    hasAlpha = true;
                    break;
            }
        }

        CHECK_MSTATUS(const_cast<MFnMesh&>(meshFn).getFaceVertexColors(
                    colors,
                    &colorSetName,
                    &defaultUnsetColor
                    ));

        // reverse winding order
        Util::reverseWindingOrder(colors, vertexCount);

        if(hasColor)
        {
            const MString colorAttributeName = getLayerName("Cd", i);

            mappedCdNames[i] = colorAttributeName;

            buffer.resize(3 * vertexList.size());
            for(unsigned int j = 0; j < vertexList.size(); j++)
            {
                buffer[j * 3 + 0] = colors[j].r;
                buffer[j * 3 + 1] = colors[j].g;
                buffer[j * 3 + 2] = colors[j].b;
            }

            // add and set Cd
            HAPI_AttributeInfo colorAttributeInfo;
            colorAttributeInfo.exists = true;
            colorAttributeInfo.owner = HAPI_ATTROWNER_VERTEX;
            colorAttributeInfo.storage = HAPI_STORAGETYPE_FLOAT;
            colorAttributeInfo.count = vertexList.size();
            colorAttributeInfo.tupleSize = 3;
            HAPI_AddAttribute(
                    myInputAssetId, myInputObjectId, myInputGeoId,
                    colorAttributeName.asChar(), &colorAttributeInfo
                    );

            HAPI_SetAttributeFloatData(
                    myInputAssetId, myInputObjectId, myInputGeoId,
                    colorAttributeName.asChar(), &colorAttributeInfo,
                    &buffer.front(), 0, vertexList.size()
                    );
        }

        if(hasAlpha)
        {
            const MString alphaAttributeName = getLayerName("Alpha", i);

            mappedAlphaNames[i] = alphaAttributeName;

            buffer.resize(vertexList.size());
            for(unsigned int j = 0; j < vertexList.size(); j++)
            {
                buffer[j] = colors[j].a;
            }

            // add and set Alpha
            HAPI_AttributeInfo alphaAttributeInfo;
            alphaAttributeInfo.exists = true;
            alphaAttributeInfo.owner = HAPI_ATTROWNER_VERTEX;
            alphaAttributeInfo.storage = HAPI_STORAGETYPE_FLOAT;
            alphaAttributeInfo.count = vertexList.size();
            alphaAttributeInfo.tupleSize = 1;
            HAPI_AddAttribute(
                    myInputAssetId, myInputObjectId, myInputGeoId,
                    alphaAttributeName.asChar(), &alphaAttributeInfo
                    );

            HAPI_SetAttributeFloatData(
                    myInputAssetId, myInputObjectId, myInputGeoId,
                    alphaAttributeName.asChar(), &alphaAttributeInfo,
                    &buffer.front(), 0, vertexList.size()
                    );
        }
    }

    Input::setDetailAttribute(
            myInputAssetId,
            myInputObjectId,
            myInputGeoId,
            "maya_colorset_name",
            colorSetNames
            );

    Input::setDetailAttribute(
            myInputAssetId,
            myInputObjectId,
            myInputGeoId,
            "maya_colorset_mapped_Cd",
            mappedCdNames
            );

    Input::setDetailAttribute(
            myInputAssetId,
            myInputObjectId,
            myInputGeoId,
            "maya_colorset_mapped_Alpha",
            mappedAlphaNames
            );

    return true;
}
