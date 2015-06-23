#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MGlobal.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MMatrix.h>
#include <maya/MTime.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnNurbsCurveData.h>
#include <maya/MFnStringArrayData.h>
#include <maya/MPointArray.h>
#if MAYA_API_VERSION >= 201400
        #include <maya/MFnFloatArrayData.h>
#endif
#include <maya/MFnVectorArrayData.h>

#include <algorithm>
#include <vector>
#include <limits>
#include <string>

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
OutputGeometryPart::computeVolumeTransform(
        const MTime &time,
        MDataHandle& volumeTransformHandle
        )
{
    HAPI_Transform transform = myVolumeInfo.transform;

    MDataHandle translateHandle
        = volumeTransformHandle.child(AssetNode::outputPartVolumeTranslate);
    MDataHandle rotateHandle
        = volumeTransformHandle.child(AssetNode::outputPartVolumeRotate);
    MDataHandle scaleHandle
        = volumeTransformHandle.child(AssetNode::outputPartVolumeScale);

    MEulerRotation r = MQuaternion(
            transform.rotationQuaternion[0],
            transform.rotationQuaternion[1],
            transform.rotationQuaternion[2],
            transform.rotationQuaternion[3]
            ).asEulerRotation();

    const double rot[3] = {r[0], r[1], r[2]};
    const double scale[3] = {
        transform.scale[0],
        transform.scale[1],
        transform.scale[2]
    };

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
    matrix.addTranslation(
            MVector(xoffset, yoffset, zoffset),
            MSpace::kPreTransform
            );

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
        hstat = HAPI_GetGeoInfo(
                Util::theHAPISession.get(),
                myAssetId, myObjectId, myGeoId,
                &myGeoInfo
                );
        Util::checkHAPIStatus(hstat);

        hstat = HAPI_GetPartInfo(
                Util::theHAPISession.get(),
                myAssetId, myObjectId, myGeoId, myPartId,
                &myPartInfo
                );
        Util::checkHAPIStatus(hstat);

        if(myPartInfo.type == HAPI_PARTTYPE_VOLUME)
        {
            hstat = HAPI_GetVolumeInfo(
                    Util::theHAPISession.get(),
                    myAssetId, myObjectId, myGeoId, myPartId,
                    &myVolumeInfo
                    );
            Util::checkHAPIStatus(hstat);
        }

        if(myPartInfo.type == HAPI_PARTTYPE_CURVE)
        {
            hstat = HAPI_GetCurveInfo(
                    Util::theHAPISession.get(),
                    myAssetId, myObjectId, myGeoId, myPartId,
                    &myCurveInfo
                    );
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
            Util::theHAPISession.get(),
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
            Util::theHAPISession.get(),
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
        std::string* data,
        int start, int length
        )
{
    std::vector<HAPI_StringHandle> stringHandles(
            attr_info->count * attr_info->tupleSize
            );

    HAPI_Result hapiResult = HAPI_GetAttributeStringData(
            Util::theHAPISession.get(),
            asset_id, object_id, geo_id, part_id,
            name,
            attr_info,
            &stringHandles.front(),
            start, length
            );
    if(HAPI_FAIL(hapiResult) || !attr_info->exists)
    {
        return hapiResult;
    }

    for(int i = 0; i < (int) stringHandles.size(); i++)
    {
        data[i] = Util::getString(stringHandles[i]).asChar();
    }

    return hapiResult;
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
            Util::theHAPISession.get(),
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
        const MTime &time,
        MDataHandle& handle,
        bool hasGeoChanged,
        bool hasMaterialChanged,
        bool &needToSyncOutputs
        )
{
    update();

    if(myNeverBuilt || hasGeoChanged)
    {
        clearAttributesUsed();

        // Name
        MDataHandle nameHandle = handle.child(AssetNode::outputPartName);
        MString partName;
        if(myPartInfo.nameSH != 0)
        {
            partName = Util::getString(myPartInfo.nameSH);
        }
        nameHandle.set(partName);

        // Mesh
        MDataHandle hasMeshHandle = handle.child(AssetNode::outputPartHasMesh);
        MDataHandle meshHandle = handle.child(AssetNode::outputPartMesh);
        if(myPartInfo.pointCount != 0
                && myPartInfo.vertexCount != 0
                && myPartInfo.faceCount != 0)
        {
            computeMesh(time, hasMeshHandle, meshHandle);
        }
        else
        {
            clearMesh(hasMeshHandle, meshHandle);
        }

        // Particle
        MDataHandle hasParticlesHandle =
            handle.child(AssetNode::outputPartHasParticles);
        MDataHandle particleHandle =
            handle.child(AssetNode::outputPartParticle);
        if(myPartInfo.pointCount != 0
                && myPartInfo.vertexCount == 0
                && myPartInfo.faceCount == 0)
        {
            computeParticle(time, hasParticlesHandle, particleHandle);
        }

#if MAYA_API_VERSION >= 201400
        // Volume
        MDataHandle volumeHandle = handle.child(AssetNode::outputPartVolume);
        if(myPartInfo.type == HAPI_PARTTYPE_VOLUME)
        {
            computeVolume(time, volumeHandle);
        }
#endif

        // Curve
        MDataHandle curvesHandle = handle.child(AssetNode::outputPartCurves);
        MDataHandle curvesIsBezierHandle =
            handle.child(AssetNode::outputPartCurvesIsBezier);
        if(myCurveInfo.curveCount)
        {
            computeCurves(time, curvesHandle, curvesIsBezierHandle);
        }
        else
        {
            clearCurves(curvesHandle, curvesIsBezierHandle);
        }

        // Extra attributes
        MDataHandle extraAttributesHandle = handle.child(
                AssetNode::outputPartExtraAttributes
                );
        computeExtraAttributes(time, extraAttributesHandle);

        // Groups
        MDataHandle groupsHandle = handle.child(
                AssetNode::outputPartGroups
                );
        computeGroups(time, groupsHandle);
    }

    if(myNeverBuilt || hasMaterialChanged)
    {
        MDataHandle materialHandle =
            handle.child(AssetNode::outputPartMaterial);
        computeMaterial(time, materialHandle);
    }

    myNeverBuilt = false;

    return MS::kSuccess;
}

void
OutputGeometryPart::computeCurves(
        const MTime &time,
        MDataHandle &curvesHandle,
        MDataHandle &curvesIsBezierHandle
        )
{
    MStatus status;

    MArrayDataHandle curvesArrayHandle(curvesHandle);
    MArrayDataBuilder curvesBuilder = curvesArrayHandle.builder();

    std::vector<float> pArray, pwArray;
    getAttributeData(pArray,  "P",  HAPI_ATTROWNER_POINT);
    getAttributeData(pwArray, "Pw", HAPI_ATTROWNER_POINT);
    markAttributeUsed("P");
    markAttributeUsed("Pw");

    int vertexOffset = 0;
    int knotOffset = 0;
    for(int iCurve = 0; iCurve < myCurveInfo.curveCount; iCurve++)
    {
        MDataHandle curve = curvesBuilder.addElement(iCurve );
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
        int numVertices = 0;
        HAPI_GetCurveCounts(
                Util::theHAPISession.get(),
                myAssetId, myObjectId, myGeoId, myPartId,
                &numVertices,
                iCurve, 1
                );

        const int nextVertexOffset = vertexOffset + numVertices;
        if ( nextVertexOffset > static_cast<int>( pArray.size() ) * 3
            || (!pwArray.empty()
                && nextVertexOffset > static_cast<int>( pwArray.size() ) )
        )
        {
            MGlobal::displayError( "Not enough points to create a curve" );
            break;
        }

        // Order of this particular curve
        int order;
        if ( myCurveInfo.order != HAPI_CURVE_ORDER_VARYING
            && myCurveInfo.order != HAPI_CURVE_ORDER_INVALID )
        {
            order = myCurveInfo.order;
        }
        else
        {
            HAPI_GetCurveOrders(
                    Util::theHAPISession.get(),
                    myAssetId, myObjectId, myGeoId, myPartId,
                    &order,
                    iCurve, 1
                    );
        }

        // If there's not enough vertices, then don't try to create the curve.
        if(numVertices < order)
        {
            // Need to make sure we clear out the curve that was created
            // previously.
            curve.setMObject(curveDataFn.create());

            // The curve at i will have numVertices vertices, and may have
            // some knots. The knot count will be numVertices + order for
            // nurbs curves
            vertexOffset = nextVertexOffset;
            knotOffset += numVertices + order;

            continue;
        }

        MPointArray controlVertices( numVertices );
        for(int iDst = 0, iSrc = vertexOffset;
            iDst < numVertices;
            ++iDst, ++iSrc)
        {
            controlVertices[iDst] = MPoint(
                pArray[iSrc * 3],
                pArray[iSrc * 3 + 1],
                pArray[iSrc * 3 + 2],
                pwArray.empty() ? 1.0f : pwArray[iSrc]
            );
        }

        MDoubleArray knotSequences;
        if(myCurveInfo.hasKnots)
        {
            std::vector<float> knots;
            knots.resize(numVertices + order);
            // The Maya knot vector has two fewer knots;
            // the first and last houdini knot are excluded
            knotSequences.setLength(numVertices + order - 2);
            HAPI_GetCurveKnots(
                    Util::theHAPISession.get(),
                    myAssetId, myObjectId, myGeoId, myPartId,
                    &knots.front(),
                    knotOffset, numVertices + order
                    );
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
        vertexOffset = nextVertexOffset;
        knotOffset += numVertices + order;
    }

    // There may be curves created in the previous cook. So we need to clear
    // the output attributes to remove any curves that may have been created
    // previously.
    clearCurvesBuilder(curvesBuilder, myCurveInfo.curveCount);

    curvesArrayHandle.set(curvesBuilder);

    curvesIsBezierHandle.setBool(
            myCurveInfo.curveType == HAPI_CURVETYPE_BEZIER
            );
}

static void
convertArray(MVectorArray &dstArray, const std::vector<float> &srcArray)
{
    dstArray.setLength(srcArray.size() / 3);
    for(unsigned int i = 0, j = 0; i < dstArray.length(); i++, j += 3)
    {
        MVector &vector = dstArray[i];
        vector.x = srcArray[j + 0];
        vector.y = srcArray[j + 1];
        vector.z = srcArray[j + 2];
    }
}

static void
convertArray(MDoubleArray &dstArray, const std::vector<float> &srcArray)
{
    dstArray.setLength(srcArray.size());
    for(unsigned int i = 0; i < dstArray.length(); i++)
    {
        dstArray[i] = srcArray[i];
    }
}

static void
convertArray(MIntArray &dstArray, const std::vector<int> &srcArray)
{
    dstArray.setLength(srcArray.size());
    for(unsigned int i = 0; i < dstArray.length(); i++)
    {
        dstArray[i] = srcArray[i];
    }
}

static void
convertArray(MStringArray &dstArray, const std::vector<std::string> &srcArray)
{
    dstArray.setLength(srcArray.size());
    for(unsigned int i = 0; i < dstArray.length(); i++)
    {
        dstArray[i] = srcArray[i].c_str();
    }
}

static void
zeroArray(MVectorArray &dstArray, int count)
{
    dstArray.setLength(count);
    for(unsigned int i = 0; i < dstArray.length(); i++)
    {
        MVector &vector = dstArray[i];
        vector.x = 0.0;
        vector.y = 0.0;
        vector.z = 0.0;
    }
}

static void
zeroArray(MDoubleArray &dstArray, int count)
{
    dstArray.setLength(count);
    for(unsigned int i = 0; i < dstArray.length(); i++)
    {
        dstArray[i] = 0.0;
    }
}

static void
getParticleArray(
        MVectorArray &particleArray,
        MFnArrayAttrsData &arrayDataFn,
        const MString &attrName
        )
{
    particleArray = arrayDataFn.vectorArray(attrName);
}

static void
getParticleArray(
        MDoubleArray &particleArray,
        MFnArrayAttrsData &arrayDataFn,
        const MString &attrName
        )
{
    particleArray = arrayDataFn.doubleArray(attrName);
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
        getParticleArray(particleArray, arrayDataFn, mayaName);
        convertArray(particleArray, buffer);
    }
    else
    {
        T particleArray;
        getParticleArray(particleArray, arrayDataFn, mayaName);
        zeroArray(particleArray, particleCount);
    }
}

bool
OutputGeometryPart::convertGenericDataAttribute(
        MDataHandle &dataHandle,
        const char* attributeName,
        const HAPI_AttributeInfo &attributeInfo
        )
{
    if(attributeInfo.storage == HAPI_STORAGETYPE_FLOAT)
    {
        std::vector<float> floatArray;
        getAttributeData(floatArray, attributeName, attributeInfo.owner);

        if(attributeInfo.owner == HAPI_ATTROWNER_DETAIL
                && attributeInfo.tupleSize == 1)
        {
            dataHandle.setGenericDouble(floatArray[0], true);

            return true;
        }
        else if(attributeInfo.owner == HAPI_ATTROWNER_DETAIL
                && attributeInfo.tupleSize == 2)
        {
            MFnNumericData numericData;
            MObject dataObject = numericData.create(
                    MFnNumericData::k2Double
                    );
            numericData.setData2Double(
                    floatArray[0],
                    floatArray[1]
                    );

            dataHandle.setMObject(dataObject);

            return true;
        }
        else if(attributeInfo.owner == HAPI_ATTROWNER_DETAIL
                && attributeInfo.tupleSize == 3)
        {
            MFnNumericData numericData;
            MObject dataObject = numericData.create(
                    MFnNumericData::k3Double
                    );
            numericData.setData3Double(
                    floatArray[0],
                    floatArray[1],
                    floatArray[2]
                    );

            dataHandle.setMObject(dataObject);

            return true;
        }
        else if(attributeInfo.owner == HAPI_ATTROWNER_DETAIL
                && attributeInfo.tupleSize == 4)
        {
            MFnNumericData numericData;
            MObject dataObject = numericData.create(
                    MFnNumericData::k4Double
                    );
            numericData.setData4Double(
                    floatArray[0],
                    floatArray[1],
                    floatArray[2],
                    floatArray[3]
                    );

            dataHandle.setMObject(dataObject);

            return true;
        }
        else if(attributeInfo.tupleSize == 3)
        {
            MFnVectorArrayData vectorArrayData;
            MObject dataObject = vectorArrayData.create();
            MVectorArray outputVectorArray = vectorArrayData.array();
            convertArray(outputVectorArray, floatArray);

            dataHandle.setMObject(dataObject);

            return true;
        }
        else
        {
            MFnDoubleArrayData doubleArrayData;
            MObject dataObject = doubleArrayData.create();
            MDoubleArray outputDoubleArray = doubleArrayData.array();
            convertArray(outputDoubleArray, floatArray);

            dataHandle.setMObject(dataObject);

            return true;
        }
    }
    else if(attributeInfo.storage == HAPI_STORAGETYPE_INT)
    {
        std::vector<int> intArray;
        getAttributeData(intArray, attributeName, attributeInfo.owner);

        if(attributeInfo.owner == HAPI_ATTROWNER_DETAIL
                && attributeInfo.tupleSize == 1)
        {
            dataHandle.setGenericInt(intArray[0], true);

            return true;
        }
        else if(attributeInfo.owner == HAPI_ATTROWNER_DETAIL
                && attributeInfo.tupleSize == 2)
        {
            MFnNumericData numericData;
            MObject dataObject = numericData.create(
                    MFnNumericData::k2Int
                    );
            numericData.setData2Int(
                    intArray[0],
                    intArray[1]
                    );

            dataHandle.setMObject(dataObject);

            return true;
        }
        else if(attributeInfo.owner == HAPI_ATTROWNER_DETAIL
                && attributeInfo.tupleSize == 3)
        {
            MFnNumericData numericData;
            MObject dataObject = numericData.create(
                    MFnNumericData::k3Int
                    );
            numericData.setData3Int(
                    intArray[0],
                    intArray[1],
                    intArray[2]
                    );

            dataHandle.setMObject(dataObject);

            return true;
        }
        else
        {
            MFnIntArrayData intArrayData;
            MObject dataObject = intArrayData.create();
            MIntArray outputIntArray = intArrayData.array();
            convertArray(outputIntArray, intArray);

            dataHandle.setMObject(dataObject);

            return true;
        }
    }
    else if(attributeInfo.storage == HAPI_STORAGETYPE_STRING)
    {
        std::vector<std::string> stringArray;
        getAttributeData(stringArray, attributeName, attributeInfo.owner);

        if(attributeInfo.owner == HAPI_ATTROWNER_DETAIL
                && attributeInfo.tupleSize == 1)
        {
            dataHandle.setString(stringArray[0].c_str());

            return true;
        }
        else
        {
            MFnStringArrayData stringArrayData;
            MObject dataObject = stringArrayData.create();
            MStringArray outputStringArray = stringArrayData.array();
            convertArray(outputStringArray, stringArray);

            dataHandle.setMObject(dataObject);

            return true;
        }
    }

    return false;
}

void
OutputGeometryPart::computeParticle(
        const MTime &time,
        MDataHandle &hasParticlesHandle,
        MDataHandle &particleHandle
        )
{
    hasParticlesHandle.setBool(true);

    MDataHandle currentTimeHandle
        = particleHandle.child(AssetNode::outputPartParticleCurrentTime);
    MDataHandle positionsHandle
        = particleHandle.child(AssetNode::outputPartParticlePositions);
    MDataHandle arrayDataHandle
        = particleHandle.child(AssetNode::outputPartParticleArrayData);

    std::vector<float> floatArray;
    std::vector<int> intArray;

    int particleCount = myPartInfo.pointCount;

    // currentTime
    currentTimeHandle.setMTime(time);

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
        convertArray(positions, floatArray);
    }

    // array data
    MObject arrayDataObj = arrayDataHandle.data();
    MFnArrayAttrsData arrayDataFn(arrayDataObj);
    if(arrayDataObj.isNull())
    {
        arrayDataObj = arrayDataFn.create();
        arrayDataHandle.setMObject(arrayDataObj);

        arrayDataObj = arrayDataHandle.data();
        arrayDataFn.setObject(arrayDataObj);
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
            zeroArray(idArray, particleCount);
        }
    }
    markAttributeUsed("id");

    // count
    MDoubleArray countArray = arrayDataFn.doubleArray("count");
    countArray.setLength(1);
    countArray[0] = particleCount;
    markAttributeUsed("count");

    // position
    arrayDataFn.vectorArray("position").copy(positions);
    markAttributeUsed("P");
    markAttributeUsed("position");

    // velocity
    MVectorArray velocityArray;
    getParticleArray(velocityArray, arrayDataFn, "velocity");
    if(getAttributeData(floatArray, "v", HAPI_ATTROWNER_POINT))
    {
        convertArray(velocityArray, floatArray);
    }
    else
    {
        zeroArray(velocityArray, particleCount);
    }
    markAttributeUsed("v");
    markAttributeUsed("velocity");

    // acceleration
    convertParticleAttribute<MVectorArray>(
            arrayDataFn, "acceleration",
            floatArray,
            "force",
            particleCount
            );
    markAttributeUsed("acceleration");
    markAttributeUsed("force");

    // worldPosition
    arrayDataFn.vectorArray("worldPosition").copy(positions);
    markAttributeUsed("worldPosition");

    // worldVelocity
    arrayDataFn.vectorArray("worldVelocity").copy(velocityArray);
    markAttributeUsed("worldVelocity");

    // worldVelocityInObjectSpace
    arrayDataFn.vectorArray("worldVelocityInObjectSpace").copy(velocityArray);
    markAttributeUsed("worldVelocityInObjectSpace");

    // mass
    convertParticleAttribute<MDoubleArray>(
            arrayDataFn, "mass",
            floatArray,
            "mass",
            particleCount
            );
    markAttributeUsed("mass");

    // birthTime
    convertParticleAttribute<MDoubleArray>(
            arrayDataFn, "birthTime",
            floatArray,
            "birthTime",
            particleCount
            );
    markAttributeUsed("birthTime");

    // age
    convertParticleAttribute<MDoubleArray>(
            arrayDataFn, "age",
            floatArray,
            "age",
            particleCount
            );
    markAttributeUsed("age");

    // finalLifespanPP
    convertParticleAttribute<MDoubleArray>(
            arrayDataFn, "finalLifespanPP",
            floatArray,
            "finalLifespanPP",
            particleCount
            );
    markAttributeUsed("finalLifespanPP");

    // lifespanPP
    convertParticleAttribute<MDoubleArray>(
            arrayDataFn, "lifespanPP",
            floatArray,
            "life",
            particleCount
            );
    markAttributeUsed("lifespanPP");

    // other attributes
    int* attributeNames = new int[myPartInfo.pointAttributeCount];
    HAPI_GetAttributeNames(
            Util::theHAPISession.get(),
            myAssetId, myObjectId, myGeoId, myPartId,
            HAPI_ATTROWNER_POINT,
            attributeNames,
            myPartInfo.pointAttributeCount
            );
    for(int i = 0; i < myPartInfo.pointAttributeCount; i++)
    {
        MString attributeName = Util::getString(attributeNames[i]);

        // skip attributes that were done above already
        if(isAttributeUsed(attributeName.asChar()))
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

        HAPI_GetAttributeInfo(
                Util::theHAPISession.get(),
                myAssetId, myObjectId, myGeoId, myPartId,
                attributeName.asChar(),
                HAPI_ATTROWNER_POINT,
                &attributeInfo
                );

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
    }
    delete [] attributeNames;

    currentTimeHandle.setClean();
    positionsHandle.setClean();
    arrayDataHandle.setClean();
}

#if MAYA_API_VERSION >= 201400
void
OutputGeometryPart::computeVolume(
        const MTime &time,
        MDataHandle &volumeHandle
        )
{
    MDataHandle gridHandle
        = volumeHandle.child(AssetNode::outputPartVolumeGrid);
    MDataHandle transformHandle
        = volumeHandle.child(AssetNode::outputPartVolumeTransform);
    MDataHandle resHandle
        = volumeHandle.child(AssetNode::outputPartVolumeRes);
    MDataHandle nameHandle
        = volumeHandle.child(AssetNode::outputPartVolumeName);

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
        HAPI_GetFirstVolumeTile(
                Util::theHAPISession.get(),
                myAssetId, myObjectId, myGeoId, myPartId,
                &tileInfo
                );

#ifdef max
#undef max
#endif

        while(tileInfo.minX != std::numeric_limits<int>::max() &&
                tileInfo.minY != std::numeric_limits<int>::max() &&
                tileInfo.minZ != std::numeric_limits<int>::max())
        {
            HAPI_GetVolumeTileFloatData(
                    Util::theHAPISession.get(),
                    myAssetId, myObjectId, myGeoId, myPartId,
                    0.0f,
                    &tileInfo,
                    &tile.front(),
                    (int) tile.size()
                    );

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

                        float value = tile[
                            k * tileSize*tileSize
                            + j * tileSize
                            + i
                        ];
                        if(x < xres && y < yres && z < zres
                                && x >= 0 && y >= 0 && z >= 0)
                        {
                            grid[index] = value;
                        }
                    }

            HAPI_GetNextVolumeTile(
                    Util::theHAPISession.get(),
                    myAssetId, myObjectId, myGeoId, myPartId,
                    &tileInfo
                    );
        }
    }

    // transform
    computeVolumeTransform(time, transformHandle);

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
        const MTime &time,
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
    markAttributeUsed("P");

    // polygon counts
    MIntArray polygonCounts;
    {
        intArray.resize(myPartInfo.faceCount);

        if(myPartInfo.faceCount)
        {
            HAPI_GetFaceCounts(
                    Util::theHAPISession.get(),
                    myAssetId, myObjectId, myGeoId, myPartId,
                    &intArray.front(),
                    0, myPartInfo.faceCount
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
                    Util::theHAPISession.get(),
                    myAssetId, myObjectId, myGeoId, myPartId,
                    &intArray.front(),
                    0, myPartInfo.vertexCount
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

    // Check that the mesh created is what we tried to create. Assets could
    // output mesh with bad faces that Maya cannot handle. When this happens,
    // Maya would skip the bad faces, so the resulting mesh would be slightly
    // different. This makes it no longer possible to apply geometry attributes
    // (like UVs and color sets) to the mesh.
    {
        bool mismatch_topology = false;

        if(vertexArray.length() != (unsigned int) meshFn.numVertices())
        {
            DISPLAY_WARNING(
                    "Attempted to create ^1s vertices, but only ^2s vertices "
                    "were created.",
                    MString() + vertexArray.length(),
                    MString() + meshFn.numVertices()
                    );
            mismatch_topology = true;
        }
        if(polygonCounts.length() != (unsigned int) meshFn.numPolygons())
        {
            DISPLAY_WARNING(
                    "Attempted to create ^1s polygons, but only ^2s polygons "
                    "were created.",
                    MString() + polygonCounts.length(),
                    MString() + meshFn.numPolygons()
                    );
            mismatch_topology = true;
        }
        if(polygonConnects.length() != (unsigned int) meshFn.numFaceVertices())
        {
            DISPLAY_WARNING(
                    "Attempted to create ^1s face-vertices, but only ^2s "
                    "face-vertices were created.",
                    MString() + polygonConnects.length(),
                    MString() + meshFn.numFaceVertices()
                    );
            mismatch_topology = true;
        }

        if(mismatch_topology)
        {
            DISPLAY_WARNING(MString(
                    "This likely means that the Houdini Digital Asset is "
                    "outputting bad geometry that Maya cannot handle. As a "
                    "result, other attributes (such as UVs and color sets) "
                    "cannot be transferred."
                    ));

            return;
        }
    }

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
            markAttributeUsed("N");

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
            markAttributeUsed("uv");

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
            if(colorOwner != HAPI_ATTROWNER_MAX)
            {
                markAttributeUsed("Cd");
            }
            if(alphaOwner != HAPI_ATTROWNER_MAX)
            {
                markAttributeUsed("Alpha");
            }

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
            if((colorOwner == HAPI_ATTROWNER_POINT
                        && alphaOwner == HAPI_ATTROWNER_PRIM)
                    || (colorOwner == HAPI_ATTROWNER_PRIM
                        && alphaOwner == HAPI_ATTROWNER_POINT))
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

                meshFn.setFaceVertexColors(
                        promotedColors,
                        faceList,
                        polygonConnects
                        );
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
OutputGeometryPart::computeMaterial(
        const MTime &time,
        MDataHandle& materialHandle
        )
{
    HAPI_Result hapiResult;

    MDataHandle matExistsHandle
        = materialHandle.child(AssetNode::outputPartMaterialExists);
    MDataHandle nameHandle
        = materialHandle.child(AssetNode::outputPartMaterialName);
    MDataHandle ambientHandle
        = materialHandle.child(AssetNode::outputPartAmbientColor);
    MDataHandle diffuseHandle
        = materialHandle.child(AssetNode::outputPartDiffuseColor);
    MDataHandle specularHandle
        = materialHandle.child(AssetNode::outputPartSpecularColor);
    MDataHandle alphaHandle
        = materialHandle.child(AssetNode::outputPartAlphaColor);
    MDataHandle texturePathHandle
        = materialHandle.child(AssetNode::outputPartTexturePath);

    HAPI_MaterialId materialId;
    HAPI_GetMaterialIdsOnFaces(
            Util::theHAPISession.get(),
            myAssetId, myObjectId, myGeoId, myPartId,
            NULL,
            &materialId,
            0, 1
            );

    HAPI_GetMaterialInfo(
            Util::theHAPISession.get(),
            myAssetId, materialId,
            &myMaterialInfo
            );

    if(!myMaterialInfo.exists)
    {
        matExistsHandle.set(false);
    }
    else
    {
        // get material info
        HAPI_NodeInfo materialNodeInfo;
        HAPI_GetNodeInfo(
            Util::theHAPISession.get(),
            myMaterialInfo.nodeId,
            &materialNodeInfo
            );

        std::vector<HAPI_ParmInfo> parms(materialNodeInfo.parmCount);
        HAPI_GetParameters(
                Util::theHAPISession.get(),
                myMaterialInfo.nodeId,
                &parms[0],
                0, materialNodeInfo.parmCount
                );

        int ambientParmIndex = Util::findParm(parms, "ogl_amb");
        int diffuseParmIndex = Util::findParm(parms, "ogl_diff");
        int alphaParmIndex = Util::findParm(parms, "ogl_alpha");
        int specularParmIndex = Util::findParm(parms, "ogl_spec");
        int texturePathSHParmIndex = Util::findParm(parms, "ogl_tex#", 1);
        float valueHolder[4];

        matExistsHandle.set(true);

        nameHandle.setString(
                Util::getString(materialNodeInfo.nameSH)
                );

        if(ambientParmIndex >= 0)
        {
            HAPI_GetParmFloatValues(
                    Util::theHAPISession.get(),
                    myMaterialInfo.nodeId, valueHolder,
                    parms[ambientParmIndex].floatValuesIndex, 3
                    );
            ambientHandle.set3Float(
                    valueHolder[0],
                    valueHolder[1],
                    valueHolder[2]
                    );
        }

        if(specularParmIndex >= 0)
        {
            HAPI_GetParmFloatValues(
                    Util::theHAPISession.get(),
                    myMaterialInfo.nodeId,
                    valueHolder,
                    parms[specularParmIndex].floatValuesIndex, 3
                    );
            specularHandle.set3Float(
                    valueHolder[0],
                    valueHolder[1],
                    valueHolder[2]
                    );
        }

        if(diffuseParmIndex >= 0)
        {
            HAPI_GetParmFloatValues(
                    Util::theHAPISession.get(),
                    myMaterialInfo.nodeId,
                    valueHolder,
                    parms[diffuseParmIndex].floatValuesIndex, 3
                    );
            diffuseHandle.set3Float(
                    valueHolder[0],
                    valueHolder[1],
                    valueHolder[2]
                    );
        }

        if(alphaParmIndex >= 0)
        {
            HAPI_GetParmFloatValues(
                    Util::theHAPISession.get(),
                    myMaterialInfo.nodeId,
                    valueHolder,
                    parms[alphaParmIndex].floatValuesIndex, 1
                    );
            float alpha = 1 - valueHolder[0];
            alphaHandle.set3Float(alpha, alpha, alpha);
        }

        if(texturePathSHParmIndex >= 0)
        {
            HAPI_ParmInfo texturePathParm;
            HAPI_GetParameters(
                    Util::theHAPISession.get(),
                    myMaterialInfo.nodeId,
                    &texturePathParm,
                    texturePathSHParmIndex, 1
                    );

            int texturePathSH;
            HAPI_GetParmStringValues(
                    Util::theHAPISession.get(),
                    myMaterialInfo.nodeId,
                    true,
                    &texturePathSH,
                    texturePathParm.stringValuesIndex, 1
                    );

            bool hasTextureSource = Util::getString(texturePathSH).length() > 0;
            bool canRenderTexture = false;
            if(hasTextureSource)
            {
                // this could fail if texture parameter is empty
                hapiResult = HAPI_RenderTextureToImage(
                        Util::theHAPISession.get(),
                        myAssetId, myMaterialInfo.id,
                        texturePathSHParmIndex
                        );

                canRenderTexture = hapiResult == HAPI_RESULT_SUCCESS;
            }

            int destinationFilePathSH = 0;
            if(canRenderTexture)
            {
                MString destinationFolderPath;
                MGlobal::executeCommand("workspace -expandName "
                        "`workspace -q -fileRuleEntry sourceImages`;",
                        destinationFolderPath);

                // this could fail if the image planes don't exist
                hapiResult = HAPI_ExtractImageToFile(
                        Util::theHAPISession.get(),
                        myAssetId, myMaterialInfo.id,
                        HAPI_PNG_FORMAT_NAME,
                        "C A",
                        destinationFolderPath.asChar(),
                        NULL,
                        &destinationFilePathSH
                        );
                if(HAPI_FAIL(hapiResult))
                {
                    DISPLAY_ERROR(
                            "Could not extract image to directory:\n"
                            "    ^1s",
                            destinationFolderPath
                            );
                    DISPLAY_ERROR_HAPI_STATUS_CALL();
                }
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

void
OutputGeometryPart::computeExtraAttributes(
        const MTime &time,
        MDataHandle &extraAttributesHandle
        )
{
    MArrayDataHandle extraAttributesArrayHandle(extraAttributesHandle);
    MArrayDataBuilder extraAttributesBuilder =
        extraAttributesArrayHandle.builder();

    const size_t dataTypeSize = HAPI_STORAGETYPE_MAX;
    const MString dataTypesString[dataTypeSize] = {
        "int",
        "float",
        "string",
    };

    const size_t attributeOwnerSize = HAPI_ATTROWNER_MAX;
    const HAPI_AttributeOwner attributeOwners[attributeOwnerSize] = {
        HAPI_ATTROWNER_DETAIL,
        HAPI_ATTROWNER_PRIM,
        HAPI_ATTROWNER_POINT,
        HAPI_ATTROWNER_VERTEX,
    };
    const MString attributeOwnersString[attributeOwnerSize] = {
        "detail",
        "primitive",
        "point",
        "vertex",
    };
    const int HAPI_PartInfo::*attributeCounts[attributeOwnerSize] = {
        &HAPI_PartInfo::detailAttributeCount,
        &HAPI_PartInfo::faceAttributeCount,
        &HAPI_PartInfo::pointAttributeCount,
        &HAPI_PartInfo::vertexAttributeCount,
    };

    size_t elementIndex = 0;

    for(size_t i = 0; i < attributeOwnerSize; i++)
    {
        const HAPI_AttributeOwner &owner = attributeOwners[i];
        const MString &ownerString = attributeOwnersString[i];
        const int HAPI_PartInfo::*&attributeCount = attributeCounts[i];

        std::vector<HAPI_StringHandle> attributeNames(
                myPartInfo.*attributeCount
                );
        HAPI_GetAttributeNames(
                Util::theHAPISession.get(),
                myAssetId, myObjectId, myGeoId, myPartId,
                owner,
                &attributeNames[0],
                attributeNames.size()
                );

        for(size_t j = 0; j < attributeNames.size(); j++)
        {
            const MString attributeName = Util::getString(attributeNames[j]);

            if(isAttributeUsed(attributeName.asChar())
                        || attributeName.substring(0, 1) == "__")
            {
                continue;
            }

            MDataHandle extraAttributeHandle =
                extraAttributesBuilder.addElement(elementIndex);
            elementIndex++;

            MDataHandle nameHandle = extraAttributeHandle.child(
                    AssetNode::outputPartExtraAttributeName
                    );
            MDataHandle ownerHandle = extraAttributeHandle.child(
                    AssetNode::outputPartExtraAttributeOwner
                    );
            MDataHandle dataTypeHandle = extraAttributeHandle.child(
                    AssetNode::outputPartExtraAttributeDataType
                    );
            MDataHandle tupleHandle = extraAttributeHandle.child(
                    AssetNode::outputPartExtraAttributeTuple
                    );
            MDataHandle dataHandle = extraAttributeHandle.child(
                    AssetNode::outputPartExtraAttributeData
                    );

            HAPI_AttributeInfo attributeInfo;
            CHECK_HAPI(HAPI_GetAttributeInfo(
                    Util::theHAPISession.get(),
                    myAssetId, myObjectId, myGeoId, myPartId,
                    attributeName.asChar(),
                    owner,
                    &attributeInfo
                    ));

            const MString &dataTypeString =
                dataTypesString[attributeInfo.storage];

            nameHandle.setString(attributeName);
            ownerHandle.setString(ownerString);
            dataTypeHandle.setString(dataTypeString);
            tupleHandle.setInt(attributeInfo.tupleSize);

            if(!convertGenericDataAttribute(
                    dataHandle,
                    attributeName.asChar(),
                    attributeInfo
                    ))
            {
                DISPLAY_WARNING(
                        "Unsupported data type in attribute:\n"
                        "    ^1s",
                        attributeName
                        );
            }
        }
    }

    for(size_t i = extraAttributesBuilder.elementCount(); i-- > elementIndex;)
    {
        extraAttributesBuilder.removeElement(i);
    }

    extraAttributesArrayHandle.set(extraAttributesBuilder);
}

void
OutputGeometryPart::computeGroups(
        const MTime &time,
        MDataHandle &groupsHandle
        )
{
    MArrayDataHandle groupsArrayHandle(groupsHandle);
    MArrayDataBuilder groupsBuilder =
       groupsArrayHandle.builder();

    const HAPI_GroupType groupTypes[HAPI_GROUPTYPE_MAX] =
    {
        HAPI_GROUPTYPE_POINT,
        HAPI_GROUPTYPE_PRIM,
    };
    const int HAPI_GeoInfo::*groupCounts[HAPI_GROUPTYPE_MAX] =
    {
        &HAPI_GeoInfo::pointGroupCount,
        &HAPI_GeoInfo::primitiveGroupCount,
    };
    const int HAPI_PartInfo::*maxMemberCounts[HAPI_GROUPTYPE_MAX] =
    {
        &HAPI_PartInfo::pointCount,
        &HAPI_PartInfo::faceCount,
    };
    const MFn::Type fnTypes[HAPI_GROUPTYPE_MAX] =
    {
        MFn::kMeshVertComponent,
        MFn::kMeshPolygonComponent,
    };

    size_t groupElementIndex = 0;
    for(int i = 0; i < 2; i++)
    {
        const HAPI_GroupType &groupType = groupTypes[i];
        const int HAPI_GeoInfo::*&groupCount = groupCounts[i];
        const int HAPI_PartInfo::*&maxMemberCount = maxMemberCounts[i];
        const MFn::Type fnType = fnTypes[i];

        std::vector<HAPI_StringHandle> groupNames(myGeoInfo.*groupCount);

        HAPI_GetGroupNames(
                Util::theHAPISession.get(),
                myAssetId, myObjectId, myGeoId,
                groupType,
                &groupNames[0],
                myGeoInfo.*groupCount
                );

        std::vector<int> groupMembership(myPartInfo.*maxMemberCount);
        for(size_t j = 0; j < groupNames.size(); j++)
        {
            MString groupName = Util::getString(groupNames[j]).asChar();

            if(groupName == HAPI_UNGROUPED_GROUP_NAME)
            {
                continue;
            }

            MDataHandle groupHandle
                = groupsBuilder.addElement(groupElementIndex);
            groupElementIndex++;

            MDataHandle groupNameHandle = groupHandle.child(
                    AssetNode::outputPartGroupName
                    );
            MDataHandle groupTypeHandle = groupHandle.child(
                    AssetNode::outputPartGroupType
                    );
            MDataHandle groupMembersHandle = groupHandle.child(
                    AssetNode::outputPartGroupMembers
                    );

            HAPI_GetGroupMembership(
                    Util::theHAPISession.get(),
                    myAssetId, myObjectId, myGeoId, myPartId,
                    groupType,
                    groupName.asChar(),
                    &groupMembership[0],
                    0, groupMembership.size()
                    );

            MObject groupMembersObj = groupMembersHandle.data();
            MFnIntArrayData groupMembersDataFn(groupMembersObj);
            if(groupMembersObj.isNull())
            {
                groupMembersObj = groupMembersDataFn.create();
                groupMembersHandle.setMObject(groupMembersObj);

                groupMembersObj = groupMembersHandle.data();
                groupMembersDataFn.setObject(groupMembersObj);
            }

            MIntArray groupMembers = groupMembersDataFn.array();
            groupMembers.setLength(groupMembership.size());

            size_t groupMembersCount = 0;
            for(size_t k = 0; k < groupMembership.size(); k++)
            {
                if(groupMembership[k])
                {
                    groupMembers[groupMembersCount] = k;
                    groupMembersCount++;
                }
            }
            groupMembers.setLength(groupMembersCount);

            groupNameHandle.setString(groupName);
            groupTypeHandle.setInt(fnType);
        }
    }

    for(size_t i = groupsBuilder.elementCount(); i-- > groupElementIndex;)
    {
        groupsBuilder.removeElement(i);
    }

    groupsArrayHandle.set(groupsBuilder);
}

void
OutputGeometryPart::markAttributeUsed(const std::string &attributeName)
{
    std::vector<std::string>::iterator pos = std::lower_bound(
            myAttributesUsed.begin(),
            myAttributesUsed.end(),
            attributeName
            );

    // Attribute already exist.
    if(pos != myAttributesUsed.end()
            && *pos == attributeName)
    {
        return;
    }

    myAttributesUsed.insert(pos, attributeName);
}

bool
OutputGeometryPart::isAttributeUsed(const std::string &attributeName)
{
    return std::binary_search(
            myAttributesUsed.begin(),
            myAttributesUsed.end(),
            attributeName
            );
}

void
OutputGeometryPart::clearAttributesUsed()
{
    myAttributesUsed.clear();
}
