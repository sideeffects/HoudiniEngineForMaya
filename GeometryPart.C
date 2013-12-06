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
#include <maya/MFnVectorArrayData.h>

#include <vector>
#include <limits>

#include "Asset.h"
#include "AssetNode.h"
#include "GeometryPart.h"
#include "util.h"

GeometryPart::GeometryPart(int assetId, int objectId, int geoId, int partId,
        HAPI_ObjectInfo objectInfo, HAPI_GeoInfo geoInfo) : 
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
        HAPI_PartInfo_Init( &myPartInfo );
    }
    
}


GeometryPart::~GeometryPart() {}

#if MAYA_API_VERSION >= 201400
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

    double xoffset = myVolumeInfo.xLength/2.0 + myVolumeInfo.minX;
    double yoffset = myVolumeInfo.yLength/2.0 + myVolumeInfo.minY;
    double zoffset = myVolumeInfo.zLength/2.0 + myVolumeInfo.minZ;

    const double scale2[3] = {2, 2, 2};
    matrix.addScale(scale2, MSpace::kPreTransform);
    matrix.addTranslation(MVector(-0.5, -0.5, -0.5), MSpace::kPreTransform);
    matrix.addTranslation(MVector(xoffset, yoffset, zoffset), MSpace::kPreTransform);

    const double scale3[3] = {
	static_cast<double>(myVolumeInfo.xLength),
	static_cast<double>(myVolumeInfo.yLength),
	static_cast<double>(myVolumeInfo.zLength)
    };
    matrix.addScale(scale3, MSpace::kPreTransform);

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
#endif

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
        HAPI_PartInfo_Init( &myPartInfo );
    }
}


void
GeometryPart::setGeoInfo(HAPI_GeoInfo& info)
{
    myGeoInfo = info;
}

bool
GeometryPart::getAttributeFloatData(
        std::vector<float> &floatArray,
        const char* name,
        HAPI_AttributeOwner owner
        )
{
    HAPI_AttributeInfo attr_info;
    attr_info.exists = false;
    HAPI_GetAttributeInfo(
            myAssetId, myObjectId, myGeoId, myPartId,
            name,
            owner,
            &attr_info
            );

    if (!attr_info.exists)
    {
        floatArray.clear();
        return false;
    }

    floatArray.resize(attr_info.count * attr_info.tupleSize);
    HAPI_GetAttributeFloatData(
            myAssetId, myObjectId, myGeoId, myPartId,
            name,
            &attr_info,
            &floatArray.front(),
            0,
            attr_info.count
            );

    return true;
}

MStatus
GeometryPart::compute(
        MDataHandle& handle,
        bool &needToSyncOutputs
        )
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
        createMesh(meshHandle);

	// Particle
	if(myPartInfo.pointCount != 0
		&& myPartInfo.vertexCount == 0
		&& myPartInfo.faceCount == 0)
	{
	    MDataHandle partHasParticlesHandle = handle.child(AssetNode::outputPartHasParticles);
	    partHasParticlesHandle.setBool(true);

	    MDataHandle partParticleHandle = handle.child(AssetNode::outputPartParticle);
	    createParticle(partParticleHandle);
	}

#if MAYA_API_VERSION >= 201400
	// Volume
	if (myPartInfo.hasVolume)
	{
	    MDataHandle partVolumeHandle = handle.child(AssetNode::outputPartVolume);

	    MDataHandle partVolumeTransformHandle = partVolumeHandle.child(AssetNode::outputPartVolumeTransform);
	    updateVolumeTransform(partVolumeTransformHandle);

	    MDataHandle partVolumeGridHandle = partVolumeHandle.child(AssetNode::outputPartVolumeGrid);
	    partVolumeGridHandle.set(createVolume());

	    MFloatArray resolution;
	    resolution.append(myVolumeInfo.xLength);
	    resolution.append(myVolumeInfo.yLength);
	    resolution.append(myVolumeInfo.zLength);
	    MDataHandle partVolumeResHandle = partVolumeHandle.child(AssetNode::outputPartVolumeRes);
	    MFnFloatArrayData resCreator;
	    partVolumeResHandle.set(resCreator.create(resolution));

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

void
GeometryPart::createParticle(MDataHandle &dataHandle)
{
    MDataHandle positionsHandle = dataHandle.child(AssetNode::outputPartParticlePositions);
    MDataHandle arrayDataHandle = dataHandle.child(AssetNode::outputPartParticleArrayData);

    std::vector<float> floatArray;

    // positions
    MObject positionsObj = positionsHandle.data();
    MFnVectorArrayData positionDataFn(positionsObj);
    if(positionsObj.isNull())
    {
	positionsObj = positionDataFn.create();
	positionsHandle.setMObject(positionsObj);
    }

    MVectorArray positions = positionDataFn.array();
    {
        getAttributeFloatData(floatArray, "P", HAPI_ATTROWNER_POINT);
	positions.setLength(floatArray.size()/3);
	for(unsigned int i = 0; i < positions.length(); i++)
	{
	    positions[i].x = floatArray[i * 3 + 0];
	    positions[i].y = floatArray[i * 3 + 1];
	    positions[i].z = floatArray[i * 3 + 2];
	}
    }

    // array data
    MObject arrayDataObj = arrayDataHandle.data();
    MFnArrayAttrsData arrayDataFn(arrayDataObj);
    if(arrayDataObj.isNull())
    {
	arrayDataObj = arrayDataFn.create();
	arrayDataHandle.setMObject(arrayDataObj);
    }

    // count
    MDoubleArray countArray = arrayDataFn.doubleArray("count");
    countArray.setLength(1);
    countArray[0] = positions.length();

    // position
    arrayDataFn.vectorArray("position").copy(positions);

    // velocity
    MVectorArray velocityArray = arrayDataFn.vectorArray("velocity");
    {
        getAttributeFloatData(floatArray, "v", HAPI_ATTROWNER_POINT);
	velocityArray.setLength(floatArray.size()/3);
	for ( unsigned int i = 0; i < velocityArray.length(); i++ )
	{
	    velocityArray[i].x = floatArray[i * 3 + 0];
	    velocityArray[i].y = floatArray[i * 3 + 1];
	    velocityArray[i].z = floatArray[i * 3 + 2];
	}
    }

    // rgbPP
    {
        getAttributeFloatData(floatArray, "Cd", HAPI_ATTROWNER_POINT);
	if(floatArray.size()/3 == positions.length())
	{
	    MVectorArray rgbPPArray = arrayDataFn.vectorArray("rgbPP");
	    rgbPPArray.setLength(floatArray.size()/3);
	    for ( unsigned int i = 0; i < rgbPPArray.length(); i++ )
	    {
		rgbPPArray[i].x = floatArray[i * 3 + 0];
		rgbPPArray[i].y = floatArray[i * 3 + 1];
		rgbPPArray[i].z = floatArray[i * 3 + 2];
	    }
	}
    }

    // radiusPP
    {
        getAttributeFloatData(floatArray, "pscale", HAPI_ATTROWNER_POINT);
	if(floatArray.size() == positions.length())
	{
	    MDoubleArray radiusPPArray = arrayDataFn.doubleArray("radiusPP");
	    radiusPPArray.setLength(floatArray.size());
	    for ( unsigned int i = 0; i < radiusPPArray.length(); i++ )
	    {
		radiusPPArray[i] = floatArray[i];
	    }
	}
    }

    // age
    MDoubleArray ageArray = arrayDataFn.doubleArray("age");
    {
        getAttributeFloatData(floatArray, "age", HAPI_ATTROWNER_POINT);
	ageArray.setLength(floatArray.size());
	for ( unsigned int i = 0; i < ageArray.length(); i++ )
	{
	    ageArray[i] = floatArray[i];
	}
    }

    positionsHandle.setClean();
    arrayDataHandle.setClean();
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
    std::fill(tileValues, tileValues + tileSize * tileSize * tileSize - 1, 0.0f);
    for (unsigned int i=0; i<grid.length(); i++)
	grid[i] = 0.0f;

    HAPI_VolumeTileInfo tile;
    HAPI_GetFirstVolumeTile(myAssetId, myObjectId, myGeoId, myPartId, &tile);

#ifdef max
#undef max
#endif

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

void
GeometryPart::createMesh(MDataHandle &dataHandle)
{
    MStatus status;

    // create mesh
    MObject meshDataObj = dataHandle.data();
    MFnMeshData meshDataFn(meshDataObj);
    if(meshDataObj.isNull())
    {
        // set the MDataHandle
        meshDataObj = meshDataFn.create();
        dataHandle.setMObject(meshDataObj);

        // then get the copy from MDataHandle
        meshDataObj = dataHandle.data();
        meshDataFn.setObject(meshDataObj);
    }

    std::vector<float> floatArray;
    std::vector<int> intArray;

    // vertex array
    MFloatPointArray vertexArray;
    {
        getAttributeFloatData(floatArray, "P", HAPI_ATTROWNER_POINT);

        // assume 3 tuple
        vertexArray.setLength(floatArray.size() / 3);

        for(unsigned int i = 0, j = 0; i < vertexArray.length(); i++, j += 3)
        {
            vertexArray.set(i, floatArray[j], floatArray[j+1], floatArray[j+2]);
        }
    }

    // polygon counts
    MIntArray polygonCounts;
    {
        intArray.resize(myPartInfo.faceCount);

        if(myPartInfo.faceCount)
        {
            HAPI_GetFaceCounts(
                    myAssetId,
                    myObjectId,
                    myGeoId,
                    myPartId,
                    &intArray.front(),
                    0,
                    myPartInfo.faceCount
                    );
        }

        polygonCounts = MIntArray(&intArray.front(), intArray.size());
    }

    // polygon connects
    MIntArray polygonConnects;
    {
        intArray.resize(myPartInfo.vertexCount);

        if(myPartInfo.vertexCount)
        {
            HAPI_GetVertexList(
                    myAssetId,
                    myObjectId,
                    myGeoId,
                    myPartId,
                    &intArray.front(),
                    0,
                    myPartInfo.vertexCount
                    );
        }

        polygonConnects = MIntArray(&intArray.front(), intArray.size());

        Util::reverseWindingOrder(polygonConnects, polygonCounts);
    }

    MFnMesh meshFn;
    meshFn.create(
            vertexArray.length(),
            polygonCounts.length(),
            vertexArray,
            polygonCounts,
            polygonConnects,
            meshDataObj,
            &status
            );
    CHECK_MSTATUS(status);

    // normal array
    if(polygonCounts.length())
    {
        getAttributeFloatData(floatArray, "N", HAPI_ATTROWNER_POINT);

        if(floatArray.size())
        {
            // assume 3 tuple
            MVectorArray normals(
                    reinterpret_cast<float(*)[3]>(&floatArray.front()),
                    floatArray.size() / 3);

            MIntArray vertexList;
            for ( unsigned int j = 0; j < vertexArray.length(); ++j )
            {
                vertexList.append( j );
            }

            meshFn.setVertexNormals(normals, vertexList);
        }
    }

    // uv
    if(polygonCounts.length())
    {
        getAttributeFloatData(floatArray, "uv", HAPI_ATTROWNER_VERTEX);

        if(floatArray.size())
        {
            // assume 3 tuple
            MFloatArray uArray;
            MFloatArray vArray;
            for(size_t i = 0; i < floatArray.size(); i+=3)
            {
                uArray.append(floatArray[i]);
                vArray.append(floatArray[i+1]);
            }
            Util::reverseWindingOrder(uArray, polygonCounts);
            Util::reverseWindingOrder(vArray, polygonCounts);
            meshFn.setUVs(uArray, vArray);

            MIntArray vertexList;
            for ( unsigned int j = 0; j < polygonConnects.length(); ++j )
            {
                vertexList.append( j );
            }

            meshFn.assignUVs(polygonCounts, vertexList);
        }
    }
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
        int texturePathSHParmIndex = Util::findParm(parms, "ogl_tex#", 1);
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

	    bool hasTextureSource = Util::getString(texturePathSH).length() > 0;
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
			HAPI_PNG_FORMAT_NAME,
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
