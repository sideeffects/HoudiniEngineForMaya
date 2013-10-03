#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MGlobal.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MMatrix.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MFnDoubleArrayData.h>
#if MAYA_API_VERSION >= 201400
	#include <maya/MFnFloatArrayData.h>
#endif

#include <vector>
#include <limits>

#include "Asset.h"
#include "AssetNode.h"
#include "GeometryPart.h"
#include "util.h"

GeometryPart::GeometryPart()
{
}

GeometryPart::GeometryPart(int assetId, int objectId, int geoId, int partId,
        HAPI_ObjectInfo objectInfo, HAPI_GeoInfo geoInfo, Asset* objectControl) : 
    myObjectControl(objectControl),
    myAssetId(assetId),
    myObjectId(objectId),
    myGeoId(geoId),
    myPartId(partId),
    myObjectInfo(objectInfo),
    myGeoInfo(geoInfo),
    myNeverBuilt(true)
{
    // Do a full update
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;
    try
    {
        hstat = HAPI_GetPartInfo(assetId, objectId, geoId, partId, & myPartInfo);
        Util::checkHAPIStatus(hstat);

	if (myPartInfo.hasVolume)
	{
	    hstat = HAPI_GetVolumeInfo(myAssetId, myObjectId, myGeoId, myPartId, &myVolumeInfo);
	    Util::checkHAPIStatus(hstat);
	}
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
GeometryPart::updateVolumeTransform(MDataHandle& handle)
{
    HAPI_Transform transform = myVolumeInfo.transform;

    MDataHandle translateHandle = handle.child(AssetNode::outputPartVolumeTranslate);
    MDataHandle rotateHandle = handle.child(AssetNode::outputPartVolumeRotate);
    MDataHandle scaleHandle = handle.child(AssetNode::outputPartVolumeScale);

    MEulerRotation r = MQuaternion(transform.rotationQuaternion[0],
				   transform.rotationQuaternion[1],
				   transform.rotationQuaternion[2],
				   transform.rotationQuaternion[3]).asEulerRotation();

    const double rot[3] = {r[0], r[1], r[2]};
    const double scale[3] = {transform.scale[0], transform.scale[1], transform.scale[2]};

    MTransformationMatrix matrix;
    matrix.addScale(scale, MSpace::kTransform);
    matrix.addRotation(rot, MTransformationMatrix::kXYZ, MSpace::kTransform);
    matrix.addTranslation(MVector(transform.position[0],
				  transform.position[1],
				  transform.position[2]), MSpace::kTransform);

    int xoffset = myVolumeInfo.xLength*0.5 + myVolumeInfo.minX;
    int yoffset = myVolumeInfo.yLength*0.5 + myVolumeInfo.minY;
    int zoffset = myVolumeInfo.zLength*0.5 + myVolumeInfo.minZ;

    const double scale2[3] = {2, 2, 2};
    matrix.addScale(scale2, MSpace::kPreTransform);
    matrix.addTranslation(MVector(-0.5, -0.5, -0.5), MSpace::kPreTransform);
    matrix.addTranslation(MVector(xoffset, yoffset, zoffset), MSpace::kPreTransform);

    double final_scale[3];
    double final_rotate[3];
    MTransformationMatrix::RotationOrder order = MTransformationMatrix::kXYZ;
    matrix.getScale(final_scale, MSpace::kTransform);
    matrix.getRotation(final_rotate, order);
    translateHandle.set(matrix.getTranslation(MSpace::kTransform));
    rotateHandle.set3Double(final_rotate[0], final_rotate[1], final_rotate[2]);
    scaleHandle.set3Double(final_scale[0], final_scale[1], final_scale[2]);

    translateHandle.setClean();
    rotateHandle.setClean();
    scaleHandle.setClean();
    handle.setClean();
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

	if (myPartInfo.hasVolume)
	{
	    hstat = HAPI_GetVolumeInfo(myAssetId, myObjectId, myGeoId, myPartId, &myVolumeInfo);
	    Util::checkHAPIStatus(hstat);
	}
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
    HAPI_GetAttributeFloatData( myAssetId, myObjectId, myGeoId, myPartId, name.asChar(),
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
    MDataHandle partNameHandle = handle.child(AssetNode::outputPartName);
    MDataHandle meshHandle = handle.child(AssetNode::outputPartMesh);
    //MDataHandle transformHandle = handle.child(AssetNode::transform);
    MDataHandle materialHandle = handle.child(AssetNode::outputPartMaterial);


    if ( myNeverBuilt || myGeoInfo.hasGeoChanged)
    {
        // Object name
	MString partName = Util::getString(myObjectInfo.nameSH) + "/" + Util::getString(myPartInfo.nameSH);
        partNameHandle.set(partName);

        // Mesh
        MObject newMeshData = createMesh();
        meshHandle.set(newMeshData);

#if MAYA_API_VERSION >= 201400
	// Volume
	if (myPartInfo.hasVolume)
	{
	    MDataHandle partVolumeHandle = handle.child(AssetNode::outputPartVolume);

	    MDataHandle partVolumeTransformHandle = partVolumeHandle.child(AssetNode::outputPartVolumeTransform);
	    updateVolumeTransform(partVolumeTransformHandle);

	    MDataHandle partVolumeGridHandle = partVolumeHandle.child(AssetNode::outputPartVolumeGrid);
	    partVolumeGridHandle.set(createVolume());

	    MDataHandle partVolumeResHandle = partVolumeHandle.child(AssetNode::outputPartVolumeRes);
	    partVolumeResHandle.child(AssetNode::outputPartVolumeResW).set(myVolumeInfo.xLength);
	    partVolumeResHandle.child(AssetNode::outputPartVolumeResH).set(myVolumeInfo.yLength);
	    partVolumeResHandle.child(AssetNode::outputPartVolumeResD).set(myVolumeInfo.zLength);

	    MDataHandle partVolumeNameHandle = partVolumeHandle.child(AssetNode::outputPartVolumeName);
	    partVolumeNameHandle.set(Util::getString(myVolumeInfo.nameSH));

	    partVolumeHandle.setClean();
	    partVolumeTransformHandle.setClean();
	    partVolumeGridHandle.setClean();
	    partVolumeResHandle.setClean();
	    partVolumeNameHandle.setClean();
	}
#endif
    }

    if ( myNeverBuilt || myGeoInfo.hasMaterialChanged)
    {
        updateMaterial(materialHandle);
    }

    partNameHandle.setClean();
    meshHandle.setClean();
    handle.setClean();

    myNeverBuilt = false;

    return MS::kSuccess;
}

#if MAYA_API_VERSION >= 201400
MObject
GeometryPart::createVolume()
{
    int xres = myVolumeInfo.xLength;
    int yres = myVolumeInfo.yLength;
    int zres = myVolumeInfo.zLength;
    int tileSize = myVolumeInfo.tileSize;

    MFloatArray grid;
    grid.setLength(xres * yres * zres);

    float* tileValues = new float[tileSize * tileSize * tileSize];
    std::fill(tileValues, tileValues + tileSize * tileSize * tileSize - 1, 0);
    for (unsigned int i=0; i<grid.length(); i++)
	grid[i] = 0;

    HAPI_VolumeTileInfo tile;
    HAPI_GetFirstVolumeTile(myAssetId, myObjectId, myGeoId, myPartId, &tile);
    while (tile.minX != std::numeric_limits<int>::max() &&
	   tile.minY != std::numeric_limits<int>::max() &&
	   tile.minZ != std::numeric_limits<int>::max())
    {
	HAPI_GetVolumeTileFloatData(myAssetId, myObjectId, myGeoId, myPartId, &tile, tileValues);

	for (int k=0; k<tileSize; k++)
	    for (int j=0; j<tileSize; j++)
		for (int i=0; i<tileSize; i++)
		{
		    int z = k + tile.minZ - myVolumeInfo.minZ,
			y = j + tile.minY - myVolumeInfo.minY,
			x = i + tile.minX - myVolumeInfo.minX;

		    int index =
			xres *  yres * z +
			xres * y +
			x;

		    float value = tileValues[k * tileSize*tileSize + j * tileSize + i];
		    if (x < xres && y < yres && z < zres
			&& x > 0 && y > 0 && z > 0)
		    {
			grid[index] = value;
		    }
		}

	HAPI_GetNextVolumeTile(myAssetId, myObjectId, myGeoId, myPartId, &tile);
    }

    delete[] tileValues;

    MFnFloatArrayData volumeCreator;
    return volumeCreator.create(grid);
}
#endif

MObject
GeometryPart::createMesh()
{
    // Mesh Data
    MFnMeshData dataCreator;
    MObject outData = dataCreator.create();

    // Create mesh.
    MFnMesh meshFS;
    MObject newMesh = meshFS.create( myPoints.length(), myFaceCounts.length(),
				     myPoints, myFaceCounts, myVertexList, 
				     outData );

    if(myFaceCounts.length() == 0)
    {
        return outData;
    }

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
    MDataHandle matExistsHandle = handle.child(AssetNode::outputPartMaterialExists);
    MDataHandle ambientHandle = handle.child(AssetNode::outputPartAmbientColor);
    MDataHandle diffuseHandle = handle.child(AssetNode::outputPartDiffuseColor);
    MDataHandle specularHandle = handle.child(AssetNode::outputPartSpecularColor);
    MDataHandle alphaHandle = handle.child(AssetNode::outputPartAlphaColor);
    MDataHandle texturePathHandle = handle.child(AssetNode::outputPartTexturePath);

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

        std::vector<HAPI_ParmInfo> parms(materialNodeInfo.parmCount);
        HAPI_GetParameters(myMaterialInfo.nodeId, &parms[0], 0, materialNodeInfo.parmCount);

        int ambientParmIndex = Util::findParm(parms, "ogl_amb");
        int diffuseParmIndex = Util::findParm(parms, "ogl_diff");
        int alphaParmIndex = Util::findParm(parms, "ogl_alpha");
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
	}

	if(alphaParmIndex >= 0)
	{
	    HAPI_GetParmFloatValues(myMaterialInfo.nodeId, valueHolder,
		    parms[alphaParmIndex].floatValuesIndex, 1);
	    float alpha = 1 - valueHolder[0];
	    alphaHandle.set3Float(alpha, alpha, alpha);
	}
        
	if(texturePathSHParmIndex >= 0)
	{
	    HAPI_ParmInfo texturePathParm;
	    HAPI_GetParameters(
		    myMaterialInfo.nodeId,
		    &texturePathParm,
		    texturePathSHParmIndex,
		    1
		    );

	    int texturePathSH;
	    HAPI_GetParmStringValues(
		    myMaterialInfo.nodeId,
		    &texturePathSH,
		    texturePathParm.stringValuesIndex,
		    1
		    );

	    bool hasTextureSource = Util::getString(texturePathSH).length();
	    bool canRenderTexture = false;
	    if(hasTextureSource)
	    {
		HAPI_Result hapiResult;

		// this could fail if texture parameter is empty
		hapiResult = HAPI_RenderTextureToImage(
			myAssetId,
			myMaterialInfo.id,
			texturePathSHParmIndex
			);

		canRenderTexture = hapiResult == HAPI_RESULT_SUCCESS;
	    }

	    int destinationFilePathSH = 0;
	    if(canRenderTexture)
	    {
		HAPI_Result hapiResult;

		MString destinationFolderPath;
		MGlobal::executeCommand("workspace -expandName `workspace -q -fileRuleEntry sourceImages`;",
			destinationFolderPath);

		// this could fail if the image planes don't exist
		hapiResult = HAPI_ExtractImageToFile(
			myAssetId,
			myMaterialInfo.id,
			"C A",
			destinationFolderPath.asChar(),
			NULL,
			&destinationFilePathSH
			);
	    }

	    if(destinationFilePathSH > 0)
	    {
		MString texturePath = Util::getString(destinationFilePathSH);
		texturePathHandle.set(texturePath);
	    }
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
