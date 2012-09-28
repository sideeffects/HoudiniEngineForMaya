#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnMesh.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatPoint.h>
#include <maya/MFnMeshData.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>

#include "asset.h"

MTypeId Asset::id(0x80000);
MObject Asset::input1;
MObject Asset::input2;
MObject Asset::output;

void*
Asset::creator()
{
    return new Asset();
}

void printAssetInfo(HAPI_AssetInfo* assetInfo)
{
    cerr << "id: " << assetInfo->id << endl;
    cerr << "parmExtraValueCount: " << assetInfo->parmExtraValueCount << endl;
    cerr << "parmChoiceCount: " << assetInfo->parmChoiceCount << endl;
    cerr << "handleCount: " << assetInfo->handleCount << endl;
    cerr << "objectCount: " << assetInfo->objectCount << endl;
}

MStatus
Asset::initialize()
{
    HAPI_Initialize();

    // maya plugin stuff
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;

    input1 = nAttr.create("input1", "in1", MFnNumericData::kFloat, 1.0);
    nAttr.setStorable(true);

    input2 = nAttr.create("input2", "in2", MFnNumericData::kFloat, 1.0);
    nAttr.setStorable(true);

    output = tAttr.create("output", "out", MFnData::kMesh);
    tAttr.setWritable(false);
    tAttr.setStorable(false);

    addAttribute(input1);
    addAttribute(input2);
    addAttribute(output);

    attributeAffects(input1, output);
    attributeAffects(input2, output);




    //char *buf;
    //HAPI_PrintNetwork(buf);

    //cerr << "net: " << buf << endl;

    return MS::kSuccess;
}

Asset::Asset()
{
    // houdini
    char* filename = "/home/jhuang/mayaPlugin/SideFX__spaceship.otl";

    // load otl
    assetInfo = new HAPI_AssetInfo();
    assetInfo->minVerticesPerPrimitive = 3;
    assetInfo->maxVerticesPerPrimitive = 20;
    HAPI_LoadOTLFile(filename, assetInfo);

    printAssetInfo(assetInfo);
}

Asset::~Asset() {}

void printIntArray(int* arr, int size)
{
    for (int i=0; i<size; i++)
    {
       cerr << arr[i] << " ";
    }
    cerr << endl;
}

MObject
Asset::createMesh(MObject& outData)
{
    // HAPI
    // get objects
    HAPI_ObjectInfo myObjects[assetInfo->objectCount];
    HAPI_GetObjects(assetInfo->id, myObjects, 0, assetInfo->objectCount);

    // get obj detail of first object
    HAPI_ObjectInfo myObjInfo = myObjects[0];
    HAPI_DetailInfo* myObjDetail = new HAPI_DetailInfo();
    HAPI_GetDetailInfo(assetInfo->id, myObjInfo.id, myObjDetail);

    int numFaceCount = myObjDetail->faceCount;
    int numVertexCount = myObjDetail->vertexCount;

    // get face counts
    int myFaceCounts[numFaceCount];
    HAPI_GetFaceCounts(assetInfo->id, myObjInfo.id, myFaceCounts, 0, numFaceCount);
    cerr << "myFaceCounts" << endl;
    printIntArray(myFaceCounts, numFaceCount);

    // get vertex list
    int myVertexList[numVertexCount];
    HAPI_GetVertexList(assetInfo->id, myObjInfo.id, myVertexList, 0, numVertexCount);
    cerr << "myVertexList" << endl;
    printIntArray(myVertexList, numVertexCount);

    // print out attributes
    for ( int owner = 0; owner < HAPI_ATTROWNER_MAX; ++owner )
    {
        int attrCount = 0;
        switch ( owner )
        {
            case HAPI_ATTROWNER_VERTEX: attrCount = myObjDetail->vertexAttributeCount; break;
            case HAPI_ATTROWNER_POINT: attrCount = myObjDetail->pointAttributeCount; break;
            case HAPI_ATTROWNER_PRIM: attrCount = myObjDetail->faceAttributeCount; break;
            case HAPI_ATTROWNER_DETAIL: attrCount = myObjDetail->detailAttributeCount; break;
        }
        HAPI_AttributeStrValue attrNames[attrCount];
        HAPI_GetAttributeNames(assetInfo->id, myObjInfo.id, owner, attrNames, attrCount);
        for (int j=0; j<attrCount; j++)
        {
            cerr << "owner: " << owner << " attr: " << attrNames[j].value << endl;
        }
    }

    int numPointCount;
    int size;
    // get vertex position attribute
    HAPI_AttributeInfo* pos_attr_info = new HAPI_AttributeInfo();
    strcpy(pos_attr_info->name, "P");
    pos_attr_info->exists = false;
    pos_attr_info->owner = HAPI_ATTROWNER_POINT;
    HAPI_GetAttributeInfo(assetInfo->id, myObjInfo.id, HAPI_ATTROWNER_POINT, pos_attr_info);

    numPointCount = pos_attr_info->count;

    size = numPointCount * pos_attr_info->tupleSize;
    float myPoints[size];
    for (int j=0; j<size; j++){
        myPoints[j] = 0;
    }
    int status = HAPI_GetAttributeFloatData(assetInfo->id, myObjInfo.id, pos_attr_info, myPoints, 0, numPointCount);
    cerr << "status: " << status << endl;
    cerr << "tuple: " << pos_attr_info->tupleSize << endl;
    cerr << "count: " << numPointCount << endl;
    for(int j=0; j<size; j++)
    {
        cerr << myPoints[j] << " ";
    }
    cerr << endl;
    // make a maya point array, assume 3 tuple
    MFnMesh meshFS;
    MFloatPointArray points;
    int i = 0;
    while (i < size)
    {
        MFloatPoint v(myPoints[i], myPoints[i+1], myPoints[i+2]);
        points.append(v);
        i = i+3;
    }

    // get normals
    HAPI_AttributeInfo* norm_attr_info = new HAPI_AttributeInfo();
    strcpy(norm_attr_info->name, "N");
    norm_attr_info->exists = false;
    norm_attr_info->owner = HAPI_ATTROWNER_POINT;
    HAPI_GetAttributeInfo(assetInfo->id, myObjInfo.id, HAPI_ATTROWNER_POINT, norm_attr_info);

    numPointCount = norm_attr_info->count;

    size = numPointCount * pos_attr_info->tupleSize;
    float myNormals[size];
    for (int j=0; j<size; j++){
        myNormals[j] = 0;
    }
    HAPI_GetAttributeFloatData(assetInfo->id, myObjInfo.id, pos_attr_info, myNormals, 0, numPointCount);

    // get UVS


    std::cerr << "check1" << points[0][1] << std::endl;

    MIntArray faceCounts(myFaceCounts, numFaceCount);
    MIntArray vertexList(myVertexList, numVertexCount);

    cerr << "points.length: " << points.length() << endl;
    cerr << "faceCounts.length: " << faceCounts.length() << endl;
    cerr << "vertexList.length: " << vertexList.length() << endl;

    std::cerr << "check2" << std::endl;

    MObject newMesh = meshFS.create(numPointCount, numFaceCount, points, faceCounts, vertexList, outData);
    std::cerr << "check3" << std::endl;

    return newMesh;
}


MStatus
Asset::compute(const MPlug& plug, MDataBlock& data)
{
    MStatus returnStatus;
    if (plug == output)
    {
        // Get input size
        MDataHandle inHandle = data.inputValue(input1, &returnStatus);
        float size = inHandle.asFloat();
        if(size <= 0)
        {
            size = -size;
        }

        // Output data handle
        MDataHandle outHandle = data.outputValue(output);

        // Mesh data
        MFnMeshData dataCreator;
        MObject newOutputData = dataCreator.create(&returnStatus);


        createMesh(newOutputData);

        // Set output and mark clean
        cerr << "outdata: " << newOutputData.apiTypeStr() << endl;
        outHandle.set(newOutputData);
        std::cerr << "check4" << std::endl;
        data.setClean( plug );
        std::cerr << "check5" << std::endl;
    } else
    {
        return MS::kUnknownParameter;
    }

    return MS::kSuccess;

}

MStatus
initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, "Asset plugin", "1.0", "Any");
    status = plugin.registerNode("Asset", Asset::id, Asset::creator, Asset::initialize);
    return status;
}

MStatus
uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);
    status = plugin.deregisterNode(Asset::id);
    return status;
}
