#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnMesh.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatArray.h>
#include <maya/MVectorArray.h>
#include <maya/MFnMeshData.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MTypes.h>
#include <maya/MDGModifier.h>
#include <maya/MGlobal.h>

#include "asset.h"

// MCheckStatus (Debugging tool)
//
#   define MCheckStatus(status,message)         \
        if( MS::kSuccess != status ) {          \
            MString error("Status failed: ");   \
            error += status.errorString();      \
            MGlobal::displayError(error);       \
            MGlobal::displayError(message);       \
        } else {                                \
            MString str("Success: ");           \
            str += message;                     \
            MGlobal::displayInfo(str);          \
        }

MTypeId Asset::id(0x80000);
MObject Asset::input1;
MObject Asset::input2;
MObject Asset::output;

void*
Asset::creator()
{
    Asset* ret = new Asset();
    return ret;
}

void printAssetInfo(HAPI_AssetInfo* assetInfo)
{
    cerr << "id: " << assetInfo->id << endl;
    cerr << "parmCount: " << assetInfo->parmCount << endl;
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
    //char* filename = "/home/jhuang/dev_projects/HAPI/Maya/assets/plugin/SideFX__spaceship.otl";
    char* filename = "/home/jhuang/dev_projects/HAPI/Maya/assets/plugin/box2.otl";

    // load otl
    assetInfo = new HAPI_AssetInfo();
    assetInfo->minVerticesPerPrimitive = 3;
    assetInfo->maxVerticesPerPrimitive = 20;
    HAPI_LoadOTLFile(filename, assetInfo);

    printAssetInfo(assetInfo);
    builtParms = false;

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

MFloatArray* getAttributeFloatData(int assetId, int objectId, int owner, char* name)
{
    HAPI_AttributeInfo* attr_info = new HAPI_AttributeInfo();
    strcpy(attr_info->name, name);
    attr_info->exists = false;
    attr_info->owner = owner;
    HAPI_GetAttributeInfo(assetId, objectId, attr_info);

    int size = attr_info->count * attr_info->tupleSize;
    float data[size];
    // zero the array
    for (int j=0; j<size; j++){
        data[j] = 0;
    }
    int status = HAPI_GetAttributeFloatData(assetId, objectId, attr_info, data, 0, attr_info->count);

    // print out
    cerr << name << endl;
    cerr << "count: " << attr_info->count << endl;
    cerr << "tupleSize: " << attr_info->tupleSize << endl;
    for(int j=0; j<size; j++)
    {
        cerr << data[j] << " ";
    }
    cerr << endl;
    cerr << endl;

    MFloatArray* ret = new MFloatArray(data, size);
    return ret;
}

void reverseWindingOrderInt(MIntArray& data, MIntArray& faceCounts)
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

void reverseWindingOrderFloat(MFloatArray& data, MIntArray& faceCounts)
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

void
Asset::addParm(HAPI_ParmInfo& parm)
{
    MStatus status;
    MDGModifier mod;
    MFnNumericAttribute nAttr;

    // label
    int handle = parm.labelSH;
    int bufLen;
    HAPI_GetStringLength(handle, &bufLen);
    char label[bufLen];
    HAPI_GetString(handle, label, bufLen+1);
    //cerr << "parm label: " << label << endl;

    // name
    handle = parm.nameSH;
    HAPI_GetStringLength(handle, &bufLen);
    char name[bufLen];
    HAPI_GetString(handle, name, bufLen+1);
    cerr << "parm name: " << name << endl;

    char shortName[50];
    sprintf(shortName, "parm%d", parm.id);
    char longName[50];
    sprintf(longName, "H_parm%d", parm.id);
    MObject attrib = nAttr.create(longName, shortName, MFnNumericData::kFloat, 1.0, &status);
    MString niceName(label);
    nAttr.setNiceNameOverride(label);
    status = nAttr.setStorable(true);
    status = mod.addAttribute(thisMObject(), attrib);
    MCheckStatus(status, name);
    mod.doIt();
    // TODO: affect output
}

void
Asset::buildParms()
{

    // PARMS
    int parmCount = assetInfo->parmCount;
    HAPI_ParmInfo myParmInfos[parmCount];
    HAPI_GetParameters(assetInfo->id, myParmInfos, 0, parmCount);
    // print out the parm names
    for (int i=0; i<parmCount; i++)
    {
        addParm(myParmInfos[i]);
    }



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

    // get vertex list
    int myVertexList[numVertexCount];
    HAPI_GetVertexList(assetInfo->id, myObjInfo.id, myVertexList, 0, numVertexCount);

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
    MFloatArray* myPoints = getAttributeFloatData(assetInfo->id, myObjInfo.id, HAPI_ATTROWNER_POINT, "P");
    // make a maya point array, assume 3 tuple
    MFloatPointArray points;
    int i = 0;
    int len = myPoints->length();
    while (i < len)
    {
        MFloatPoint v((*myPoints)[i], (*myPoints)[i+1], (*myPoints)[i+2]);
        points.append(v);
        i = i+3;
    }
    numPointCount = points.length();

    // get normals
    MFloatArray* myNormals = getAttributeFloatData(assetInfo->id, myObjInfo.id, HAPI_ATTROWNER_POINT, "N");
    // make a maya vector array, assume 3 tuple
    MVectorArray normals;
    i = 0;
    len = myNormals->length();
    while (i < len)
    {
        MFloatVector v((*myNormals)[i], (*myNormals)[i+1], (*myNormals)[i+2]);
        normals.append(v);
        i = i+3;
    }
    //cerr << "normals lols" << endl;
    //cerr << normals << endl;


    // get UVS
    MFloatArray* myUVs = getAttributeFloatData(assetInfo->id, myObjInfo.id, HAPI_ATTROWNER_VERTEX, "uv");
    // split UVs into two arrays, assume 3 tuple
    MFloatArray Us;
    MFloatArray Vs;
    i = 0;
    len = myUVs->length();
    while (i < len)
    {
        Us.append((*myUVs)[i]);
        Vs.append((*myUVs)[i+1]);
        i = i+3;
    }



    std::cerr << "check1" << points[0][1] << std::endl;

    MIntArray faceCounts(myFaceCounts, numFaceCount);
    MIntArray vertexList(myVertexList, numVertexCount);

    // reverse the winding orders
    reverseWindingOrderInt(vertexList, faceCounts);
    reverseWindingOrderFloat(Us, faceCounts);
    reverseWindingOrderFloat(Vs, faceCounts);
    cerr << "uvs lols" << endl;
    cerr << Us << endl;
    cerr << Vs << endl;

    cerr << "points.length: " << points.length() << endl;
    cerr << "faceCounts.length: " << faceCounts.length() << endl;
    cerr << "vertexList.length: " << vertexList.length() << endl;
    cerr << "vertexList: " << vertexList << endl;

    std::cerr << "check2" << std::endl;

    MFnMesh meshFS;
    // create mesh
    MObject newMesh = meshFS.create(numPointCount, numFaceCount, points, faceCounts, vertexList, outData);
    // set normals
    MIntArray vlist;
    for (int j=0; j<points.length(); j++)
    {
        vlist.append(j);
    }
    meshFS.setVertexNormals(normals, vlist);
    // set UVs
    meshFS.setUVs(Us, Vs);
    MIntArray uvIds;
    for (int j=0; j<vertexList.length(); j++)
    {
        uvIds.append(j);
    }
    meshFS.assignUVs(faceCounts, uvIds);

    // debug
    MFloatPointArray tmp1;
    meshFS.getPoints(tmp1);
    MFloatVectorArray tmp2;
    meshFS.getVertexNormals(false, tmp2);
    MFloatArray tmp3;
    MFloatArray tmp4;
    meshFS.getUVs(tmp3, tmp4);

    cerr << "print points" << endl;
    cerr << tmp1 << endl;
    cerr << "print uvs" << endl;
    cerr << tmp3 << endl;
    cerr << tmp4 << endl;
    // end debug




    std::cerr << "check3" << std::endl;

    return newMesh;
}


MStatus
Asset::compute(const MPlug& plug, MDataBlock& data)
{
    if (!builtParms)
    {
        buildParms();
        builtParms = true;
    }

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
