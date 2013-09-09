#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MGlobal.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MMatrix.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MFnFloatArrayData.h>

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
	    hstat = HAPI_GetVolumeInfo(assetId, objectId, geoId, partId, &myVolumeInfo);
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

bool
GeometryPart::hasVolume()
{
    update();
    return  myPartInfo.hasVolume;
}

void
GeometryPart::updateFluidTransform(MDataHandle& handle)
{
    HAPI_Transform transform = myVolumeInfo.transform;
    MDataHandle translateHandle = handle.child(AssetNode::fluidTranslateAttr);
    MDataHandle rotateHandle = handle.child(AssetNode::fluidRotateAttr);
    MDataHandle scaleHandle = handle.child(AssetNode::fluidScaleAttr);

    // convert to euler angle
    MEulerRotation r = MQuaternion(transform.rotationQuaternion[0],
            transform.rotationQuaternion[1], transform.rotationQuaternion[2],
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


MStatus
GeometryPart::compute(MDataHandle& handle)
{
    update();

    // Get plugs
    MDataHandle objectNameHandle = handle.child(AssetNode::objectName);
    MDataHandle metaDataHandle = handle.child(AssetNode::metaData);
    MDataHandle meshHandle = handle.child(AssetNode::mesh);
    MDataHandle fluidDensityHandle = handle.child(AssetNode::fluidDensity);
    MDataHandle fluidResWHandle = handle.child(AssetNode::fluidResolutionW);
    MDataHandle fluidResHHandle = handle.child(AssetNode::fluidResolutionH);
    MDataHandle fluidResDHandle = handle.child(AssetNode::fluidResolutionD);
    MDataHandle fluidTransformHandle = handle.child(AssetNode::fluidTransform);
    MDataHandle materialHandle = handle.child(AssetNode::material);

    // Don't output mesh for degenerate geos
    if ( (myPartInfo.pointCount == 0 || myPartInfo.faceCount == 0|| myPartInfo.vertexCount == 0)
	 && !myPartInfo.hasVolume)
        return MS::kFailure;

    if ( myNeverBuilt || myGeoInfo.hasGeoChanged)
    {
        // Object name
	MString objectName = Util::getString(myObjectInfo.nameSH) + "/" + Util::getString(myPartInfo.nameSH);
        objectNameHandle.set(objectName);

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

	// Volumes -> Fluid
	
	// resolution of the fluid
	fluidResWHandle.set(myVolumeInfo.xLength);
	fluidResHHandle.set(myVolumeInfo.yLength);
	fluidResDHandle.set(myVolumeInfo.zLength);

	updateFluidTransform(fluidTransformHandle);

	if (myPartInfo.hasVolume)
	    fluidDensityHandle.set(createFluidDensity());
    }

    if ( myNeverBuilt || myGeoInfo.hasMaterialChanged)
    {
        updateMaterial(materialHandle);
    }

    fluidDensityHandle.setClean();
    fluidResWHandle.setClean();
    fluidResHHandle.setClean();
    fluidResDHandle.setClean();

    objectNameHandle.setClean();
    meshHandle.setClean();
    handle.setClean();

    myNeverBuilt = false;

    return MS::kSuccess;
}

MObject
GeometryPart::createFluidDensity()
{
    int xres = myVolumeInfo.xLength;
    int yres = myVolumeInfo.yLength;
    int zres = myVolumeInfo.zLength;
    int tileSize = myVolumeInfo.tileSize;
    if (tileSize > 16)
	return MObject();

    MFloatArray density;
    density.setLength(xres*yres*zres);

    HAPI_VolumeTileInfo tile;
    HAPI_GetFirstVolumeTile(myAssetId, myObjectId, myGeoId, myPartId, &tile);

    float tileValues[tileSize*tileSize*tileSize];
    std::fill(tileValues, tileValues + tileSize*tileSize*tileSize - 1, 0);

    for (int i=0; i<density.length();  i++)
	density[i] = 0;

    while (tile.minX != std::numeric_limits<int>::max() &&
	   tile.minY != std::numeric_limits<int>::max() &&
	   tile.minZ != std::numeric_limits<int>::max())
    {
	HAPI_GetVolumeTileFloatData(myAssetId, myObjectId, myGeoId, myPartId, &tile, tileValues);

	for (int k=0; k<tileSize; k++)
	    for (int j=0; j<tileSize; j++)
		for (int i=0; i<tileSize; i++)
		{
		    int z = k+tile.minZ - myVolumeInfo.minZ,
			y = j+tile.minY - myVolumeInfo.minY,
			x = i+tile.minX - myVolumeInfo.minX;
		    int index =
			xres * yres * z +
			xres * y +
			x;
		    float value = tileValues[k * tileSize*tileSize + j*tileSize +  i];
		    // TODO: we only support density since there is only
		    // 	     one volume per part

		    if (x < xres && y < yres && z < zres
			&& x > 0 && y > 0 && z > 0)
		    {
			density[index] = value;
		    }
		}

	HAPI_GetNextVolumeTile(myAssetId, myObjectId, myGeoId, myPartId, &tile);
    }

    MFnFloatArrayData densityCreator;
    return densityCreator.create(density);
}

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
