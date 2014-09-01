#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MArrayDataBuilder.h>
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
#include "OutputGeometryPart.h"
#include "util.h"

void
clearMesh(
        MDataHandle &hasMeshHandle,
        MDataHandle &meshHandle
        )
{
    hasMeshHandle.setBool(false);

    meshHandle.setMObject(MObject::kNullObj);
}

static void
clearCurvesBuilder(
        MArrayDataBuilder &curvesBuilder,
        int count
        )
{
    for(int i = (int) curvesBuilder.elementCount();
            i-- > count;)
    {
        curvesBuilder.removeElement(i);
    }
}

static void
clearCurves(
        MDataHandle &curvesHandle,
        MDataHandle &curvesIsBezier
        )
{
    MArrayDataHandle curvesArrayHandle(curvesHandle);
    MArrayDataBuilder curvesBuilder = curvesArrayHandle.builder();

    clearCurvesBuilder(curvesBuilder, 0);

    curvesArrayHandle.set(curvesBuilder);

    curvesIsBezier.setBool(false);
}

OutputGeometryPart::OutputGeometryPart(
        int assetId,
        int objectId,
        int geoId,
        int partId
        ) :
    myAssetId(assetId),
    myObjectId(objectId),
    myGeoId(geoId),
    myPartId(partId),
    myNeverBuilt(true)
{
    update();
}

OutputGeometryPart::~OutputGeometryPart() {}

#if MAYA_API_VERSION >= 201400
void
OutputGeometryPart::computeVolumeTransform(MDataHandle& volumeTransformHandle)
{
    HAPI_Transform transform = myVolumeInfo.transform;

    MDataHandle translateHandle = volumeTransformHandle.child(AssetNode::outputPartVolumeTranslate);
    MDataHandle rotateHandle = volumeTransformHandle.child(AssetNode::outputPartVolumeRotate);
    MDataHandle scaleHandle = volumeTransformHandle.child(AssetNode::outputPartVolumeScale);

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
    volumeTransformHandle.setClean();
}
#endif

void
OutputGeometryPart::update()
{
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;
    try
    {
        hstat = HAPI_GetPartInfo(myAssetId, myObjectId, myGeoId, myPartId, &myPartInfo);
        Util::checkHAPIStatus(hstat);

        if(myPartInfo.hasVolume)
        {
            hstat = HAPI_GetVolumeInfo(myAssetId, myObjectId, myGeoId, myPartId, &myVolumeInfo);
            Util::checkHAPIStatus(hstat);
        }

        if(myPartInfo.isCurve)
        {
            hstat = HAPI_GetCurveInfo(myAssetId, myObjectId, myGeoId, myPartId, &myCurveInfo);
            Util::checkHAPIStatus(hstat);
        }
        else
        {
            HAPI_CurveInfo_Init(&myCurveInfo);
        }
    }
    catch (HAPIError& e)
    {
        cerr << e.what() << endl;
        HAPI_PartInfo_Init(&myPartInfo);
    }
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
OutputGeometryPart::getAttributeData(
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

    if(!attr_info.exists)
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
OutputGeometryPart::compute(
        MDataHandle& handle,
        bool hasGeoChanged,
        bool hasMaterialChanged,
        bool &needToSyncOutputs
        )
{
    update();

    if(myNeverBuilt || hasGeoChanged)
    {
        // Name
        MDataHandle nameHandle = handle.child(AssetNode::outputPartName);
        MString partName = Util::getString(myPartInfo.nameSH);
        nameHandle.set(partName);

        // Mesh
        MDataHandle hasMeshHandle = handle.child(AssetNode::outputPartHasMesh);
        MDataHandle meshHandle = handle.child(AssetNode::outputPartMesh);
        if(myPartInfo.pointCount != 0
                && myPartInfo.vertexCount != 0
                && myPartInfo.faceCount != 0)
        {
            computeMesh(hasMeshHandle, meshHandle);
        }
        else
        {
            clearMesh(hasMeshHandle, meshHandle);
        }

        // Particle
        MDataHandle hasParticlesHandle = handle.child(AssetNode::outputPartHasParticles);
        MDataHandle particleHandle = handle.child(AssetNode::outputPartParticle);
        if(myPartInfo.pointCount != 0
                && myPartInfo.vertexCount == 0
                && myPartInfo.faceCount == 0)
        {
            computeParticle(hasParticlesHandle, particleHandle);
        }

#if MAYA_API_VERSION >= 201400
        // Volume
        MDataHandle volumeHandle = handle.child(AssetNode::outputPartVolume);
        if(myPartInfo.hasVolume)
        {
            computeVolume(volumeHandle);
        }
#endif

        // Curve
        MDataHandle curvesHandle = handle.child(AssetNode::outputPartCurves);
        MDataHandle curvesIsBezierHandle =
            handle.child(AssetNode::outputPartCurvesIsBezier);
        if(myCurveInfo.curveCount)
        {
            computeCurves(curvesHandle, curvesIsBezierHandle);
        }
        else
        {
            clearCurves(curvesHandle, curvesIsBezierHandle);
        }
    }

    if(myNeverBuilt || hasMaterialChanged)
    {
        MDataHandle materialHandle = handle.child(AssetNode::outputPartMaterial);
        computeMaterial(materialHandle);
    }

    myNeverBuilt = false;

    return MS::kSuccess;
}

void
OutputGeometryPart::computeCurves(
        MDataHandle &curvesHandle,
        MDataHandle &curvesIsBezierHandle
        )
{
    MStatus status;

    MArrayDataHandle curvesArrayHandle(curvesHandle);
    MArrayDataBuilder curvesBuilder = curvesArrayHandle.builder();

    int vertexOffset = 0;
    int knotOffset = 0;
    for(int i=0; i<myCurveInfo.curveCount; i++)
    {
        MDataHandle curve = curvesBuilder.addElement(i);
        MObject curveDataObj = curve.data();
        MFnNurbsCurveData curveDataFn(curveDataObj);
        if(curve.data().isNull())
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
        if(myCurveInfo.order != HAPI_CURVE_ORDER_VARYING
            && myCurveInfo.order != HAPI_CURVE_ORDER_INVALID)
            order = myCurveInfo.order;
        else
            HAPI_GetCurveOrders(myAssetId, myObjectId, myGeoId, myPartId, &order, i, 1);

        // If there's not enough vertices, then don't try to create the curve.
        if(numVertices < order)
        {
            // Need to make sure we clear out the curve that was created
            // previously.
            curve.setMObject(curveDataFn.create());

            // The curve at i will have numVertices vertices, and may have
            // some knots. The knot count will be numVertices + order for
            // nurbs curves
            vertexOffset += numVertices * 4;
            knotOffset += numVertices + order;

            continue;
        }

        std::vector<float> vertices;
        vertices.resize(numVertices * HAPI_CV_VECTOR_SIZE);
        HAPI_GetCurveVertices(myAssetId, myObjectId, myGeoId, myPartId,
                              &vertices.front(), vertexOffset,
                              numVertices * HAPI_CV_VECTOR_SIZE);
        MPointArray controlVertices(numVertices);
        for(int j=0; j<numVertices; j++)
        {
            controlVertices[j] = MPoint(vertices[j*HAPI_CV_VECTOR_SIZE],
                                        vertices[j*HAPI_CV_VECTOR_SIZE + 1],
                                        vertices[j*HAPI_CV_VECTOR_SIZE + 2],
                                        vertices[j*HAPI_CV_VECTOR_SIZE + 3]);
        }

        MDoubleArray knotSequences;
        if(myCurveInfo.hasKnots)
        {
            std::vector<float> knots;
            knots.resize(numVertices + order);
            // The Maya knot vector has two fewer knots;
            // the first and last houdini knot are excluded
            knotSequences.setLength(numVertices + order - 2);
            HAPI_GetCurveKnots(myAssetId, myObjectId, myGeoId, myPartId,
                               &knots.front(), knotOffset, numVertices + order);
            // Maya doesn't need the first and last knots
            for(int j=0; j<numVertices + order - 2; j++)
                knotSequences[j] = knots[j+1];
        }
        else if(myCurveInfo.curveType == HAPI_CURVETYPE_BEZIER)
        {
            // Bezier knot vector needs to still be passed in
            knotSequences.setLength(numVertices + order - 2);
            for(int j=0; j<numVertices + order - 2; j++)
                knotSequences[j] = j / (order - 1);
        }
        else
        {
            knotSequences.setLength(numVertices + order - 2);
            int j = 0;
            for(; j < order - 1; j++)
                knotSequences[j] = 0.0;

            for(; j < numVertices - 1; j++)
                knotSequences[j] = (double) j / (numVertices - order + 1);

            for(; j < numVertices + order - 2; j++)
                knotSequences[j] = 1.0;
        }

        // NOTE: Periodicity is always constant, so periodic and
        //           non-periodic curve meshes will have different parts.
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

    // There may be curves created in the previous cook. So we need to clear
    // the output attributes to remove any curves that may have been created
    // previously.
    clearCurvesBuilder(curvesBuilder, myCurveInfo.curveCount);

    curvesArrayHandle.set(curvesBuilder);

    curvesIsBezierHandle.setBool(myCurveInfo.curveType == HAPI_CURVETYPE_BEZIER);
}

static void
bufferToParticleArray(MVectorArray &particleArray, const std::vector<float> &buffer)
{
    for(unsigned int i = 0, j = 0; i < particleArray.length(); i++, j += 3)
    {
        MVector &vector = particleArray[i];
        vector.x = buffer[j + 0];
        vector.y = buffer[j + 1];
        vector.z = buffer[j + 2];
    }
}

static void
bufferToParticleArray(MDoubleArray &particleArray, const std::vector<float> &buffer)
{
    for(unsigned int i = 0; i < particleArray.length(); i++)
    {
        particleArray[i] = buffer[i];
    }
}

static void
bufferToParticleArray(MIntArray &particleArray, const std::vector<int> &buffer)
{
    for(unsigned int i = 0; i < particleArray.length(); i++)
    {
        particleArray[i] = buffer[i];
    }
}

static void
zeroParticleArray(MVectorArray &particleArray)
{
    for(unsigned int i = 0; i < particleArray.length(); i++)
    {
        MVector &vector = particleArray[i];
        vector.x = 0.0;
        vector.y = 0.0;
        vector.z = 0.0;
    }
}

static void
zeroParticleArray(MDoubleArray &particleArray)
{
    for(unsigned int i = 0; i < particleArray.length(); i++)
    {
        particleArray[i] = 0.0;
    }
}

static void
zeroParticleArray(MIntArray &particleArray)
{
    for(unsigned int i = 0; i < particleArray.length(); i++)
    {
        particleArray[i] = 0;
    }
}

static void
getParticleArray(
        MVectorArray &particleArray,
        MFnArrayAttrsData &arrayDataFn,
        const MString &attrName,
        int particleCount
        )
{
    particleArray = arrayDataFn.vectorArray(attrName);
    particleArray.setLength(particleCount);
}

static void
getParticleArray(
        MDoubleArray &particleArray,
        MFnArrayAttrsData &arrayDataFn,
        const MString &attrName,
        int particleCount
        )
{
    particleArray = arrayDataFn.doubleArray(attrName);
    particleArray.setLength(particleCount);
}

static void
getParticleArray(
        MIntArray &particleArray,
        MFnArrayAttrsData &arrayDataFn,
        const MString &attrName,
        int particleCount
        )
{
    particleArray = arrayDataFn.intArray(attrName);
    particleArray.setLength(particleCount);
}

template<typename T, typename U>
void
OutputGeometryPart::convertParticleAttribute(
        MFnArrayAttrsData &arrayDataFn,
        const MString &mayaName,
        U &buffer,
        const char* houdiniName,
        int particleCount
   )
{
    if(getAttributeData(buffer, houdiniName, HAPI_ATTROWNER_POINT))
    {
        T particleArray;
        getParticleArray(particleArray, arrayDataFn, mayaName, particleCount);
        bufferToParticleArray(particleArray, buffer);
    }
    else
    {
        T particleArray;
        getParticleArray(particleArray, arrayDataFn, mayaName, particleCount);
        zeroParticleArray(particleArray);
    }
}

void
OutputGeometryPart::computeParticle(
        MDataHandle &hasParticlesHandle,
        MDataHandle &particleHandle
        )
{
    hasParticlesHandle.setBool(true);

    MDataHandle positionsHandle = particleHandle.child(AssetNode::outputPartParticlePositions);
    MDataHandle arrayDataHandle = particleHandle.child(AssetNode::outputPartParticleArrayData);

    std::vector<float> floatArray;
    std::vector<int> intArray;

    int particleCount = myPartInfo.pointCount;

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
        positions.setLength(particleCount);
        bufferToParticleArray(positions, floatArray);
    }

    // array data
    MObject arrayDataObj = arrayDataHandle.data();
    MFnArrayAttrsData arrayDataFn(arrayDataObj);
    if(arrayDataObj.isNull())
    {
        arrayDataObj = arrayDataFn.create();
        arrayDataHandle.setMObject(arrayDataObj);
    }

    // id
    {
        MDoubleArray idArray = arrayDataFn.doubleArray("id");
        idArray.setLength(particleCount);
        if(getAttributeData(intArray, "id", HAPI_ATTROWNER_POINT))
        {
            for(unsigned int i = 0; i < idArray.length(); i++)
            {
                idArray[i] = intArray[i];
            }
        }
        else
        {
            zeroParticleArray(idArray);
        }
    }

    // count
    MDoubleArray countArray = arrayDataFn.doubleArray("count");
    countArray.setLength(1);
    countArray[0] = particleCount;

    // position
    arrayDataFn.vectorArray("position").copy(positions);

    // velocity
    MVectorArray velocityArray;
    getParticleArray(velocityArray, arrayDataFn, "velocity", particleCount);
    if(getAttributeData(floatArray, "v", HAPI_ATTROWNER_POINT))
    {
        bufferToParticleArray(velocityArray, floatArray);
    }
    else
    {
        zeroParticleArray(velocityArray);
    }

    // acceleration
    convertParticleAttribute<MVectorArray>(
            arrayDataFn, "acceleration",
            floatArray,
            "force",
            particleCount
            );

    // worldPosition
    arrayDataFn.vectorArray("worldPosition").copy(positions);

    // worldVelocity
    arrayDataFn.vectorArray("worldVelocity").copy(velocityArray);

    // worldVelocityInObjectSpace
    arrayDataFn.vectorArray("worldVelocityInObjectSpace").copy(velocityArray);

    // mass
    convertParticleAttribute<MDoubleArray>(
            arrayDataFn, "mass",
            floatArray,
            "mass",
            particleCount
            );

    // birthTime
    convertParticleAttribute<MDoubleArray>(
            arrayDataFn, "birthTime",
            floatArray,
            "birthTime",
            particleCount
            );

    // age
    convertParticleAttribute<MDoubleArray>(
            arrayDataFn, "age",
            floatArray,
            "age",
            particleCount
            );

    // finalLifespanPP
    convertParticleAttribute<MDoubleArray>(
            arrayDataFn, "finalLifespanPP",
            floatArray,
            "finalLifespanPP",
            particleCount
            );

    // lifespanPP
    convertParticleAttribute<MDoubleArray>(
            arrayDataFn, "lifespanPP",
            floatArray,
            "life",
            particleCount
            );

    // other attributes
    int* attributeNames = new int[myPartInfo.pointAttributeCount];
    HAPI_GetAttributeNames(myAssetId, myObjectId, myGeoId, myPartId,
            HAPI_ATTROWNER_POINT,
            attributeNames, myPartInfo.pointAttributeCount);
    for(int i = 0; i < myPartInfo.pointAttributeCount; i++)
    {
        MString attributeName = Util::getString(attributeNames[i]);

        // skip attributes that were done above already
        if(attributeName == "id"
                || attributeName == "count"
                || attributeName == "P" // houdini name
                || attributeName == "position"
                || attributeName == "v" // houdini name
                || attributeName == "velocity"
                || attributeName == "force" // houdini name
                || attributeName == "acceleration"
                || attributeName == "worldPosition"
                || attributeName == "worldVelocity"
                || attributeName == "worldVelocityInObjectSpace"
                || attributeName == "mass"
                || attributeName == "birthTime"
                || attributeName == "age"
                || attributeName == "finalLifespanPP"
                || attributeName == "life"
                || attributeName == "lifespanPP"
          )
        {
            continue;
        }

        // translate certain attributes into Maya names
        MString translatedAttributeName;
        if(attributeName == "Cd")
        {
            translatedAttributeName = "rgbPP";
        }
        else if(attributeName == "Alpha")
        {
            translatedAttributeName = "opacityPP";
        }
        else if(attributeName == "pscale")
        {
            translatedAttributeName = "radiusPP";
        }
        else
        {
            translatedAttributeName = attributeName;
        }

        HAPI_AttributeInfo attributeInfo;

        HAPI_GetAttributeInfo(myAssetId, myObjectId, myGeoId, myPartId,
                attributeName.asChar(), HAPI_ATTROWNER_POINT,
                &attributeInfo);

        // put the data into MFnArrayAttrsData
        if(attributeInfo.storage == HAPI_STORAGETYPE_FLOAT
                && attributeInfo.tupleSize == 3)
        {
            convertParticleAttribute<MVectorArray>(
                    arrayDataFn, translatedAttributeName,
                    floatArray,
                    attributeName.asChar(),
                    particleCount
                    );
        }
        else if(attributeInfo.storage == HAPI_STORAGETYPE_FLOAT
                && attributeInfo.tupleSize == 1)
        {
            convertParticleAttribute<MDoubleArray>(
                    arrayDataFn, translatedAttributeName,
                    floatArray,
                    attributeName.asChar(),
                    particleCount
                    );
        }
        else if(attributeInfo.storage == HAPI_STORAGETYPE_INT
                && attributeInfo.tupleSize == 1)
        {
            convertParticleAttribute<MIntArray>(
                    arrayDataFn, translatedAttributeName,
                    intArray,
                    attributeName.asChar(),
                    particleCount
                    );
        }
    }
    delete [] attributeNames;

    positionsHandle.setClean();
    arrayDataHandle.setClean();
}

#if MAYA_API_VERSION >= 201400
void
OutputGeometryPart::computeVolume(MDataHandle &volumeHandle)
{
    MDataHandle gridHandle = volumeHandle.child(AssetNode::outputPartVolumeGrid);
    MDataHandle transformHandle = volumeHandle.child(AssetNode::outputPartVolumeTransform);
    MDataHandle resHandle = volumeHandle.child(AssetNode::outputPartVolumeRes);
    MDataHandle nameHandle = volumeHandle.child(AssetNode::outputPartVolumeName);

    // grid
    {
        MObject gridDataObj = gridHandle.data();
        MFnFloatArrayData gridDataFn(gridDataObj);
        if(gridDataObj.isNull())
        {
            gridDataObj = gridDataFn.create();
            gridHandle.setMObject(gridDataObj);
        }

        MFloatArray grid = gridDataFn.array();

        int xres = myVolumeInfo.xLength;
        int yres = myVolumeInfo.yLength;
        int zres = myVolumeInfo.zLength;
        int tileSize = myVolumeInfo.tileSize;

        grid.setLength(xres * yres * zres);
        for(unsigned int i=0; i<grid.length(); i++)
            grid[i] = 0.0f;

        std::vector<float> tile;
        tile.resize(tileSize * tileSize * tileSize);

        HAPI_VolumeTileInfo tileInfo;
        HAPI_GetFirstVolumeTile(myAssetId, myObjectId, myGeoId, myPartId, &tileInfo);

#ifdef max
#undef max
#endif

        while(tileInfo.minX != std::numeric_limits<int>::max() &&
                tileInfo.minY != std::numeric_limits<int>::max() &&
                tileInfo.minZ != std::numeric_limits<int>::max())
        {
            HAPI_GetVolumeTileFloatData(myAssetId, myObjectId, myGeoId, myPartId, &tileInfo, &tile.front());

            for(int k=0; k<tileSize; k++)
                for(int j=0; j<tileSize; j++)
                    for(int i=0; i<tileSize; i++)
                    {
                        int z = k + tileInfo.minZ - myVolumeInfo.minZ,
                            y = j + tileInfo.minY - myVolumeInfo.minY,
                            x = i + tileInfo.minX - myVolumeInfo.minX;

                        int index =
                            xres *  yres * z +
                            xres * y +
                            x;

                        float value = tile[k * tileSize*tileSize + j * tileSize + i];
                        if(x < xres && y < yres && z < zres
                                && x > 0 && y > 0 && z > 0)
                        {
                            grid[index] = value;
                        }
                    }

            HAPI_GetNextVolumeTile(myAssetId, myObjectId, myGeoId, myPartId, &tileInfo);
        }
    }

    // transform
    computeVolumeTransform(transformHandle);

    // resolution
    MFloatArray resolution;
    resolution.append(myVolumeInfo.xLength);
    resolution.append(myVolumeInfo.yLength);
    resolution.append(myVolumeInfo.zLength);
    MFnFloatArrayData resCreator;
    resHandle.set(resCreator.create(resolution));

    // name
    nameHandle.set(Util::getString(myVolumeInfo.nameSH));
}
#endif

void
OutputGeometryPart::computeMesh(
        MDataHandle &hasMeshHandle,
        MDataHandle &meshHandle
        )
{
    MStatus status;

    hasMeshHandle.setBool(true);

    // create mesh
    MObject meshDataObj = meshHandle.data();
    MFnMeshData meshDataFn(meshDataObj);
    if(meshDataObj.isNull())
    {
        // set the MDataHandle
        meshDataObj = meshDataFn.create();
        meshHandle.setMObject(meshDataObj);

        // then get the copy from MDataHandle
        meshDataObj = meshHandle.data();
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
        for(unsigned int i = 0, length = vertexArray.length();
                i < length; ++i)
        {
            MFloatPoint &floatPoint = vertexArray[i];
            floatPoint.x = floatArray[i * 3 + 0];
            floatPoint.y = floatArray[i * 3 + 1];
            floatPoint.z = floatArray[i * 3 + 2];
            floatPoint.w = 1.0f;
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
                faceList.setLength(polygonConnects.length());
                for(unsigned int i = 0, j = 0, length = polygonCounts.length();
                        i < length; ++i)
                {
                    for(int k = 0; k < polygonCounts[i]; ++j, ++k)
                    {
                        faceList[j] = i;
                    }
                }

                meshFn.setFaceVertexNormals(normals, faceList, polygonConnects);
            }
            else if(owner == HAPI_ATTROWNER_POINT)
            {
                MIntArray vertexList;
                vertexList.setLength(vertexArray.length());
                for(unsigned int i = 0, length = vertexList.length();
                        i < length; ++i)
                {
                    vertexList[i] = i;
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
            uArray.setLength(floatArray.size() / 3);
            vArray.setLength(floatArray.size() / 3);
            for(unsigned int i = 0, length = uArray.length();
                    i < length; ++i)
            {
                uArray[i] = floatArray[i * 3 + 0];
                vArray[i] = floatArray[i * 3 + 1];
            }

            MIntArray vertexList;
            vertexList.setLength(polygonConnects.length());
            if(owner == HAPI_ATTROWNER_VERTEX)
            {
                Util::reverseWindingOrder(uArray, polygonCounts);
                Util::reverseWindingOrder(vArray, polygonCounts);

                for(unsigned int i = 0, length = polygonConnects.length();
                        i < length; ++i)
                {
                    vertexList[i] = i;
                }
            }
            else if(owner == HAPI_ATTROWNER_POINT)
            {
                for(unsigned int i = 0, length = polygonConnects.length();
                        i < length; ++i)
                {
                    vertexList[i] = polygonConnects[i];
                }
            }

            meshFn.setUVs(uArray, vArray);
            meshFn.assignUVs(polygonCounts, vertexList);
        }
    }

    // color and alpha
    if(polygonCounts.length())
    {
        // Get color data
        HAPI_AttributeOwner colorOwner = HAPI_ATTROWNER_MAX;

        if(getAttributeData(floatArray, "Cd", HAPI_ATTROWNER_VERTEX))
        {
            colorOwner = HAPI_ATTROWNER_VERTEX;
        }
        else if(getAttributeData(floatArray, "Cd", HAPI_ATTROWNER_POINT))
        {
            colorOwner = HAPI_ATTROWNER_POINT;
        }
        else if(getAttributeData(floatArray, "Cd", HAPI_ATTROWNER_PRIM))
        {
            colorOwner = HAPI_ATTROWNER_PRIM;
        }
        else if(getAttributeData(floatArray, "Cd", HAPI_ATTROWNER_DETAIL))
        {
            colorOwner = HAPI_ATTROWNER_DETAIL;
        }

        HAPI_AttributeOwner alphaOwner = HAPI_ATTROWNER_MAX;
        std::vector<float> alphaArray;

        if(getAttributeData(alphaArray, "Alpha", HAPI_ATTROWNER_VERTEX))
        {
            alphaOwner = HAPI_ATTROWNER_VERTEX;
        }
        else if(getAttributeData(alphaArray, "Alpha", HAPI_ATTROWNER_POINT))
        {
            alphaOwner = HAPI_ATTROWNER_POINT;
        }
        else if(getAttributeData(alphaArray, "Alpha", HAPI_ATTROWNER_PRIM))
        {
            alphaOwner = HAPI_ATTROWNER_PRIM;
        }
        else if(getAttributeData(alphaArray, "Alpha", HAPI_ATTROWNER_DETAIL))
        {
            alphaOwner = HAPI_ATTROWNER_DETAIL;
        }

        if(colorOwner != HAPI_ATTROWNER_MAX || alphaOwner != HAPI_ATTROWNER_MAX)
        {
            // Convert to color array
            MColorArray colors(floatArray.size() / 3);
            if(colorOwner != HAPI_ATTROWNER_MAX)
            {
                for(unsigned int i = 0, length = colors.length();
                        i < length; ++i)
                {
                    MColor &color = colors[i];
                    color.r = floatArray[i * 3 + 0];
                    color.g = floatArray[i * 3 + 1];
                    color.b = floatArray[i * 3 + 2];
                }
            }

            HAPI_AttributeOwner owner;
            if((colorOwner == HAPI_ATTROWNER_POINT && alphaOwner == HAPI_ATTROWNER_PRIM)
                    || (colorOwner == HAPI_ATTROWNER_PRIM && alphaOwner == HAPI_ATTROWNER_POINT))
            {
                // Don't convert the prim attributes to point attributes,
                // because that would lose information. Convert everything to
                // vertex attributs instead.
                owner = HAPI_ATTROWNER_VERTEX;
            }
            else
            {
                if(colorOwner < alphaOwner)
                {
                    owner = colorOwner;
                }
                else
                {
                    owner = alphaOwner;
                }
            }

            if(owner == HAPI_ATTROWNER_DETAIL)
            {
                // Handle detail color and alpha as points
                owner = HAPI_ATTROWNER_POINT;
            }

            // If there's no color, then use a default color
            if(colorOwner == HAPI_ATTROWNER_MAX)
            {
                colorOwner = HAPI_ATTROWNER_DETAIL;
                colors.setLength(1);
                colors[0] = MColor(1.0f, 1.0f, 1.0f);
            }

            // If there's no alpha, then use a default alpha
            if(alphaOwner == HAPI_ATTROWNER_MAX)
            {
                alphaOwner = HAPI_ATTROWNER_DETAIL;
                alphaArray.resize(1);
                alphaArray[0] = 1.0f;
            }

            // Promte the attributes
            MColorArray promotedColors;
            Util::promoteAttributeData<0, 0, 3, float>(
                    owner,
                    promotedColors,
                    colorOwner,
                    colors,
                    vertexArray.length(),
                    &polygonCounts,
                    &polygonConnects
                    );

            Util::promoteAttributeData<3, 0, 1, float>(
                    owner,
                    promotedColors,
                    alphaOwner,
                    alphaArray,
                    vertexArray.length(),
                    &polygonCounts,
                    &polygonConnects
                    );

            if(owner == HAPI_ATTROWNER_VERTEX)
            {
                Util::reverseWindingOrder(promotedColors, polygonCounts);

                MIntArray faceList;
                faceList.setLength(polygonConnects.length());
                for(unsigned int i = 0, j = 0, length = polygonCounts.length();
                        i < length; ++i)
                {
                    for(int k = 0; k < polygonCounts[i]; ++j, ++k)
                    {
                        faceList[j] = i;
                    }
                }

                meshFn.setFaceVertexColors(promotedColors, faceList, polygonConnects);
            }
            else if(owner == HAPI_ATTROWNER_POINT)
            {
                MIntArray vertexList;
                vertexList.setLength(vertexArray.length());
                for(unsigned int i = 0, length = vertexList.length();
                        i < length; ++i)
                {
                    vertexList[i] = i;
                }

                meshFn.setVertexColors(promotedColors, vertexList);
            }
            else if(owner == HAPI_ATTROWNER_PRIM)
            {
                MIntArray faceList;
                faceList.setLength(polygonCounts.length());
                for(unsigned int i = 0, length = faceList.length();
                        i < length; ++i)
                {
                    faceList[i] = i;
                }

                meshFn.setFaceColors(promotedColors, faceList);
            }
        }
    }
}

void
OutputGeometryPart::computeMaterial(MDataHandle& materialHandle)
{
    MDataHandle matExistsHandle = materialHandle.child(AssetNode::outputPartMaterialExists);
    MDataHandle ambientHandle = materialHandle.child(AssetNode::outputPartAmbientColor);
    MDataHandle diffuseHandle = materialHandle.child(AssetNode::outputPartDiffuseColor);
    MDataHandle specularHandle = materialHandle.child(AssetNode::outputPartSpecularColor);
    MDataHandle alphaHandle = materialHandle.child(AssetNode::outputPartAlphaColor);
    MDataHandle texturePathHandle = materialHandle.child(AssetNode::outputPartTexturePath);

    HAPI_GetMaterialOnPart( myAssetId, myObjectId, myGeoId, myPartId, &myMaterialInfo );

    if(!myMaterialInfo.exists)
    {
        matExistsHandle.set(false);
    }
    else
    {
        // get material info
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
                MString destinationFolderPath;
                MGlobal::executeCommand("workspace -expandName `workspace -q -fileRuleEntry sourceImages`;",
                        destinationFolderPath);

                // this could fail if the image planes don't exist
                HAPI_ExtractImageToFile(
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

    materialHandle.setClean();
    matExistsHandle.setClean();
    ambientHandle.setClean();
    diffuseHandle.setClean();
    specularHandle.setClean();
    alphaHandle.setClean();
    texturePathHandle.setClean();
}
