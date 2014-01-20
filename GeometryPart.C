#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MGlobal.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MMatrix.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnNurbsCurveData.h>
#include <maya/MPointArray.h>
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

	if (myPartInfo.isCurve)
	{
	    hstat = HAPI_GetCurveInfo(myAssetId, myObjectId, myGeoId, myPartId, &myCurveInfo);
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

static
HAPI_Result
getAttributeDataWrapper(
        int asset_id,
        int object_id,
        int geo_id,
        int part_id,
        const char* name,
        HAPI_AttributeInfo* attr_info,
        float* data,
        int start, int length
        )
{
    return HAPI_GetAttributeFloatData(
            asset_id, object_id, geo_id, part_id,
            name,
            attr_info,
            data,
            start, length
            );
}

static
HAPI_Result
getAttributeDataWrapper(
        int asset_id,
        int object_id,
        int geo_id,
        int part_id,
        const char* name,
        HAPI_AttributeInfo* attr_info,
        int* data,
        int start, int length
        )
{
    return HAPI_GetAttributeIntData(
            asset_id, object_id, geo_id, part_id,
            name,
            attr_info,
            data,
            start, length
            );
}

template<typename T>
bool
GeometryPart::getAttributeData(
        std::vector<T> &array,
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
        array.clear();
        return false;
    }

    array.resize(attr_info.count * attr_info.tupleSize);
    getAttributeDataWrapper(
            myAssetId, myObjectId, myGeoId, myPartId,
            name,
            &attr_info,
            &array.front(),
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
    MDataHandle curvesHandle = handle.child(AssetNode::outputPartCurves);
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

	// Curve
	if (myPartInfo.isCurve)
	{
	    createCurves(curvesHandle);
	    MDataHandle isBezierHandle =
		handle.child(AssetNode::outputPartCurvesIsBezier);
	    isBezierHandle.setBool(myCurveInfo.curveType == HAPI_CURVETYPE_BEZIER);
	}
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
GeometryPart::createCurves(MDataHandle &curvesHandle)
{
    MStatus status;

    MArrayDataHandle curvesArrayHandle(curvesHandle);
    MArrayDataBuilder curvesBuilder = curvesArrayHandle.builder();

    int vertexOffset = 0;
    int knotOffset = 0;
    for (int i=0; i<myCurveInfo.curveCount; i++)
    {
	MDataHandle curve = curvesBuilder.addElement(i);
	MObject curveDataObj = curve.data();
	MFnNurbsCurveData curveDataFn(curveDataObj);
	if (curve.data().isNull())
	{
	    // set the MDataHandle
	    curveDataObj = curveDataFn.create();
	    curve.setMObject(curveDataObj);

	    // then get the copy from MDataHandle
	    curveDataObj = curve.data();
	    curveDataFn.setObject(curveDataObj);
	}

	// Number of CVs
	int numVertices;
	HAPI_GetCurveCounts(myAssetId, myObjectId, myGeoId, myPartId,
			    &numVertices, i, 1);

	// Order of this particular curve
	int order;
	if (myCurveInfo.order != HAPI_CURVE_ORDER_VARYING
	    && myCurveInfo.order != HAPI_CURVE_ORDER_INVALID) 
	    order = myCurveInfo.order;
	else
	    HAPI_GetCurveOrders(myAssetId, myObjectId, myGeoId, myPartId, &order, i, 1);

	std::vector<float> vertices;
	vertices.resize(numVertices * HAPI_CV_VECTOR_SIZE);
	HAPI_GetCurveVertices(myAssetId, myObjectId, myGeoId, myPartId,
			      &vertices.front(), vertexOffset,
			      numVertices * HAPI_CV_VECTOR_SIZE);
	MPointArray controlVertices(numVertices);
	for (int j=0; j<numVertices; j++)
	{
	    controlVertices[j] = MPoint(vertices[j*HAPI_CV_VECTOR_SIZE],
					vertices[j*HAPI_CV_VECTOR_SIZE + 1],
					vertices[j*HAPI_CV_VECTOR_SIZE + 2],
					vertices[j*HAPI_CV_VECTOR_SIZE + 3]);
	}

	MDoubleArray knotSequences; 
	if (myCurveInfo.hasKnots)
	{
	    std::vector<float> knots;
	    knots.resize(numVertices + order);
	    // The Maya knot vector has two fewer knots; 
	    // the first and last houdini knot are excluded
	    knotSequences.setLength(numVertices + order - 2);
	    HAPI_GetCurveKnots(myAssetId, myObjectId, myGeoId, myPartId,
			       &knots.front(), knotOffset, numVertices + order);
	    // Maya doesn't need the first and last knots
	    for (int j=0; j<numVertices + order - 2; j++)
		knotSequences[j] = knots[j+1];
	}
	else if (myCurveInfo.curveType == HAPI_CURVETYPE_BEZIER)
	{
	    // Bezier knot vector needs to still be passed in
	    knotSequences.setLength(numVertices + order - 2);
	    for (int j=0; j<numVertices + order - 2; j++)
		knotSequences[j] = j / (order - 1);
	}
	else
	{
	    knotSequences.setLength(numVertices + order - 2);
	    for (int j=0; j<numVertices + order - 2; j++)
		knotSequences[j] = 0;
	}

	// NOTE: Periodicity is always constant, so periodic and
	//  	 non-periodic curve meshes will have different parts.
	MFnNurbsCurve curveFn;
	MObject nurbsCurve =
	    curveFn.create(controlVertices, knotSequences, order-1,
			   myCurveInfo.isPeriodic ?
				      MFnNurbsCurve::kPeriodic : MFnNurbsCurve::kOpen,
			   false /* 2d? */,
			   myCurveInfo.isRational /* rational? */,
			   curveDataObj, &status);
	CHECK_MSTATUS(status);


	// The curve at i will have numVertices vertices, and may have
	// some knots. The knot count will be numVertices + order for
	// nurbs curves
	vertexOffset += numVertices * 4;
	knotOffset += numVertices + order;

    }

    curvesArrayHandle.set(curvesBuilder);
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
        getAttributeData(floatArray, "P", HAPI_ATTROWNER_POINT);
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
        getAttributeData(floatArray, "v", HAPI_ATTROWNER_POINT);
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
        getAttributeData(floatArray, "Cd", HAPI_ATTROWNER_POINT);
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
        getAttributeData(floatArray, "pscale", HAPI_ATTROWNER_POINT);
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
        getAttributeData(floatArray, "age", HAPI_ATTROWNER_POINT);
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
        getAttributeData(floatArray, "P", HAPI_ATTROWNER_POINT);

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
        HAPI_AttributeOwner owner = HAPI_ATTROWNER_MAX;
        if(getAttributeData(floatArray, "N", HAPI_ATTROWNER_VERTEX))
        {
            owner = HAPI_ATTROWNER_VERTEX;
        }
        else if(getAttributeData(floatArray, "N", HAPI_ATTROWNER_POINT))
        {
            owner = HAPI_ATTROWNER_POINT;
        }

        if(owner != HAPI_ATTROWNER_MAX)
        {
            // assume 3 tuple
            MVectorArray normals(
                    reinterpret_cast<float(*)[3]>(&floatArray.front()),
                    floatArray.size() / 3);

            if(owner == HAPI_ATTROWNER_VERTEX)
            {
                Util::reverseWindingOrder(normals, polygonCounts);

                MIntArray faceList;
                MIntArray vertexList;

                for(unsigned int i = 0; i < polygonCounts.length(); i++)
                {
                    for(int j = 0; j < polygonCounts[i]; j++)
                    {
                        faceList.append(i);
                    }
                }

                meshFn.setFaceVertexNormals(normals, faceList, polygonConnects);
            }
            else if(owner == HAPI_ATTROWNER_POINT)
            {
                MIntArray vertexList;

                for ( unsigned int j = 0; j < vertexArray.length(); ++j )
                {
                    vertexList.append( j );
                }

                meshFn.setVertexNormals(normals, vertexList);
            }
        }
    }

    // uv
    if(polygonCounts.length())
    {
        HAPI_AttributeOwner owner = HAPI_ATTROWNER_MAX;
        if(getAttributeData(floatArray, "uv", HAPI_ATTROWNER_VERTEX))
        {
            owner = HAPI_ATTROWNER_VERTEX;
        }
        else if(getAttributeData(floatArray, "uv", HAPI_ATTROWNER_POINT))
        {
            owner = HAPI_ATTROWNER_POINT;
        }

        if(owner != HAPI_ATTROWNER_MAX)
        {
            // assume 3 tuple
            MFloatArray uArray;
            MFloatArray vArray;
            for(size_t i = 0; i < floatArray.size(); i+=3)
            {
                uArray.append(floatArray[i]);
                vArray.append(floatArray[i+1]);
            }

            MIntArray vertexList;

            if(owner == HAPI_ATTROWNER_VERTEX)
            {
                Util::reverseWindingOrder(uArray, polygonCounts);
                Util::reverseWindingOrder(vArray, polygonCounts);

                for ( unsigned int j = 0; j < polygonConnects.length(); ++j )
                {
                    vertexList.append( j );
                }
            }
            else if(owner == HAPI_ATTROWNER_POINT)
            {
                for ( unsigned int j = 0; j < polygonConnects.length(); ++j )
                {
                    vertexList.append( polygonConnects[j] );
                }
            }

            meshFn.setUVs(uArray, vArray);
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
		    true,
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
