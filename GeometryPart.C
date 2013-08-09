#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MGlobal.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MFnIntArrayData.h>

#include <vector>

#include "Asset.h"
#include "AssetNode.h"
#include "GeometryPart.h"
#include "util.h"

using namespace std;

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
    MDataHandle objectNameHandle = handle.child(AssetNode::objectName);
    MDataHandle metaDataHandle = handle.child(AssetNode::metaData);
    MDataHandle meshHandle = handle.child(AssetNode::mesh);
    //MDataHandle transformHandle = handle.child(AssetNode::transform);
    MDataHandle materialHandle = handle.child(AssetNode::material);

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
    // Mesh Data
    MFnMeshData dataCreator;
    MObject outData = dataCreator.create();

    // Create mesh.
    MFnMesh meshFS;
    MObject newMesh = meshFS.create( myPoints.length(), myFaceCounts.length(),
				     myPoints, myFaceCounts, myVertexList, 
				     outData );

    // Set normals.
    if ( myNormals.length() > 0)
    {
        MIntArray vlist;
        for ( unsigned int j = 0; j < myPoints.length(); ++j )
            vlist.append( j );
        meshFS.setVertexNormals( myNormals, vlist );
    }

    // Set UVs.
    if ( myUs.length() > 0 )
    {
	meshFS.setUVs( myUs, myVs );
	MIntArray uvIds;
	for ( unsigned int j = 0; j < myVertexList.length(); ++j )
	    uvIds.append( j );
	meshFS.assignUVs( myFaceCounts, uvIds );
    }

    return outData;
}


void
GeometryPart::updateMaterial(MDataHandle& handle)
{
    cerr << "Update material: " << myPartName << endl;
    MDataHandle matExistsHandle = handle.child(AssetNode::materialExists);
    MDataHandle ambientHandle = handle.child(AssetNode::ambientAttr);
    MDataHandle diffuseHandle = handle.child(AssetNode::diffuseAttr);
    MDataHandle specularHandle = handle.child(AssetNode::specularAttr);
    MDataHandle alphaHandle = handle.child(AssetNode::alphaAttr);
    MDataHandle texturePathHandle = handle.child(AssetNode::texturePath);

    if ( myPartInfo.materialId < 0)
    {
        matExistsHandle.set(false);
    } else
    {
        // get material info
        int matId = myPartInfo.materialId;
        HAPI_GetMaterial( myAssetId, matId, &myMaterialInfo );
        HAPI_NodeInfo materialNodeInfo;
        HAPI_GetNodeInfo(myMaterialInfo.nodeId, &materialNodeInfo);

        vector<HAPI_ParmInfo> parms(materialNodeInfo.parmCount);
        HAPI_GetParameters(myMaterialInfo.nodeId, &parms[0], 0, materialNodeInfo.parmCount);

        int ambientParmIndex = Util::findParm(parms, "ogl_amb");
        int diffuseParmIndex = Util::findParm(parms, "ogl_diff");
        int specularParmIndex = Util::findParm(parms, "ogl_spec");
        int texturePathSHParmIndex = Util::findParm(parms, "ogl_tex1");
        float valueHolder[4];

        matExistsHandle.set(true);

	if(ambientParmIndex >= 0)
	{
	    HAPI_GetParmFloatValues(myMaterialInfo.nodeId, valueHolder, 
		    parms[ambientParmIndex].floatValuesIndex, 4);
	    ambientHandle.set3Float(valueHolder[0], valueHolder[1], valueHolder[2]);
	}

	if(specularParmIndex >= 0)
	{
	    HAPI_GetParmFloatValues(myMaterialInfo.nodeId, valueHolder, 
		    parms[specularParmIndex].floatValuesIndex, 4);
	    specularHandle.set3Float(valueHolder[0], valueHolder[1], valueHolder[2]);
	}
        
	if(diffuseParmIndex >= 0)
	{
	    HAPI_GetParmFloatValues(myMaterialInfo.nodeId, valueHolder, 
		    parms[diffuseParmIndex].floatValuesIndex, 4);
	    diffuseHandle.set3Float(valueHolder[0], valueHolder[1], valueHolder[2]);

	    // Alpha component of the diffuse color.
	    float alpha = 1 - valueHolder[3];
	    alphaHandle.set3Float(alpha, alpha, alpha);
	}
        
	if(texturePathSHParmIndex >= 0)
	{
	    MString destinationFolderPath;
	    MGlobal::executeCommand("workspace -expandName `workspace -q -fileRuleEntry sourceImages`;",
		    destinationFolderPath);

	    int filePathSH;
	    HAPI_ExtractTextureToFile(myMaterialInfo.nodeId,
		    parms[texturePathSHParmIndex].id,
		    destinationFolderPath.asChar(),
		    NULL, &filePathSH);
	    MString texturePath = Util::getString(filePathSH);
	    texturePathHandle.set(texturePath);
	}
    }

    
    handle.setClean();
    matExistsHandle.setClean();
    ambientHandle.setClean();
    diffuseHandle.setClean();
    specularHandle.setClean();
    alphaHandle.setClean();
    texturePathHandle.setClean();
}
