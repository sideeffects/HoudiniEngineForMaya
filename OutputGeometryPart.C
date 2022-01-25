#include <maya/MArrayDataBuilder.h>
#include <maya/MEulerRotation.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnNurbsCurveData.h>
#include <maya/MFnStringArrayData.h>
#include <maya/MGlobal.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MMatrix.h>
#include <maya/MPointArray.h>
#include <maya/MQuaternion.h>
#include <maya/MTime.h>
#if MAYA_API_VERSION >= 201400
    #include <maya/MFnFloatArrayData.h>
#endif
#include <maya/MFnVectorArrayData.h>

#include <algorithm>
#include <limits>
#include <map>
#include <string>
#include <vector>

#include "Asset.h"
#include "AssetNode.h"
#include "OutputGeometryPart.h"
#include "hapiutil.h"
#include "types.h"
#include "util.h"

OutputGeometryPart::OutputGeometryPart(HAPI_NodeId nodeId, HAPI_PartId partId)
    : myNodeId(nodeId),
      myPartId(partId),
      myLastOutputGeometryGroups(true),
      myLastOutputCustomAttributes(true)
{
    update();
}

OutputGeometryPart::~OutputGeometryPart() {}

#if MAYA_API_VERSION >= 201400
void
OutputGeometryPart::computeVolumeTransform(const MTime &time,
                                           MDataHandle &volumeTransformHandle,
                                           const bool preserveScale)
{
    HAPI_Transform transform = myVolumeInfo.transform;

    MDataHandle translateHandle =
        volumeTransformHandle.child(AssetNode::outputPartVolumeTranslate);
    MDataHandle rotateHandle =
        volumeTransformHandle.child(AssetNode::outputPartVolumeRotate);
    MDataHandle scaleHandle =
        volumeTransformHandle.child(AssetNode::outputPartVolumeScale);

    MEulerRotation r = MQuaternion(transform.rotationQuaternion[0],
                                   transform.rotationQuaternion[1],
                                   transform.rotationQuaternion[2],
                                   transform.rotationQuaternion[3])
                           .asEulerRotation();

    const double rot[3]   = {r[0], r[1], r[2]};
    const double scale[3] = {
        transform.scale[0], transform.scale[1], transform.scale[2]};

    if (preserveScale)
    {
        transform.position[0] *= 100.0;
        transform.position[1] *= 100.0;
        transform.position[2] *= 100.0;
    }

    MTransformationMatrix matrix;
    matrix.addScale(scale, MSpace::kTransform);
    matrix.addRotation(rot, MTransformationMatrix::kXYZ, MSpace::kTransform);
    matrix.addTranslation(MVector(transform.position[0], transform.position[1],
                                  transform.position[2]),
                          MSpace::kTransform);

    double xoffset = myVolumeInfo.xLength / 2.0 + myVolumeInfo.minX;
    double yoffset = myVolumeInfo.yLength / 2.0 + myVolumeInfo.minY;
    double zoffset = myVolumeInfo.zLength / 2.0 + myVolumeInfo.minZ;

    const double scale2[3] = {2, 2, 2};
    matrix.addScale(scale2, MSpace::kPreTransform);

    if (preserveScale)
        matrix.addTranslation(
            MVector(-50.0, -50.0, -50.0), MSpace::kPreTransform);
    else
        matrix.addTranslation(MVector(-0.5, -0.5, -0.5), MSpace::kPreTransform);

    if (preserveScale)
    {
        matrix.addTranslation(
            MVector(xoffset * 100.0, yoffset * 100.0, zoffset * 100.0),
            MSpace::kPreTransform);
    }
    else
    {
        matrix.addTranslation(
            MVector(xoffset, yoffset, zoffset), MSpace::kPreTransform);
    }

    double scale3[3] = {static_cast<double>(myVolumeInfo.xLength),
                        static_cast<double>(myVolumeInfo.yLength),
                        static_cast<double>(myVolumeInfo.zLength)};

    if (preserveScale)
    {
        scale3[0] *= 100.0;
        scale3[1] *= 100.0;
        scale3[2] *= 100.0;
    }

    matrix.addScale(scale3, MSpace::kPreTransform);

    double final_scale[3];
    double final_rotate[3];
    MTransformationMatrix::RotationOrder order = MTransformationMatrix::kXYZ;
    matrix.getScale(final_scale, MSpace::kTransform);
    matrix.getRotation(final_rotate, order);
    translateHandle.set(matrix.getTranslation(MSpace::kTransform));
    rotateHandle.set3Double(final_rotate[0], final_rotate[1], final_rotate[2]);
    scaleHandle.set3Double(final_scale[0], final_scale[1], final_scale[2]);
}
#endif

void
OutputGeometryPart::update()
{
    CHECK_HAPI(
        HAPI_GetGeoInfo(Util::theHAPISession.get(), myNodeId, &myGeoInfo));

    CHECK_HAPI(HAPI_GetPartInfo(
        Util::theHAPISession.get(), myNodeId, myPartId, &myPartInfo));

    if (myPartInfo.type == HAPI_PARTTYPE_VOLUME)
    {
        CHECK_HAPI(HAPI_GetVolumeInfo(
            Util::theHAPISession.get(), myNodeId, myPartId, &myVolumeInfo));
    }

    if (myPartInfo.type == HAPI_PARTTYPE_CURVE)
    {
        CHECK_HAPI(HAPI_GetCurveInfo(
            Util::theHAPISession.get(), myNodeId, myPartId, &myCurveInfo));
    }
}

bool
OutputGeometryPart::needCompute(
    AssetNodeOptions::AccessorDataBlock &options) const
{
    return myLastOutputGeometryGroups != options.outputGeometryGroups() ||
           myLastOutputCustomAttributes != options.outputCustomAttributes();
}

MStatus
OutputGeometryPart::compute(const MTime &time,
                            const MPlug &partPlug,
                            MDataBlock &data,
                            MDataHandle &partHandle,
                            AssetNodeOptions::AccessorDataBlock &options,
                            bool &needToSyncOutputs)
{
    data.setClean(partPlug);

    update();

    // compute geometry
    {
        clearAttributesUsed();

        // Name
        MDataHandle nameHandle = partHandle.child(AssetNode::outputPartName);
        MString partName;
        if (myPartInfo.nameSH != 0)
        {
            partName = Util::HAPIString(myPartInfo.nameSH);
        }
        nameHandle.set(partName);

        // Mesh
        MDataHandle hasMeshHandle =
            partHandle.child(AssetNode::outputPartHasMesh);
        MDataHandle meshHandle = partHandle.child(AssetNode::outputPartMesh);
        computeMesh(time, partPlug.child(AssetNode::outputPartHasMesh),
                    partPlug.child(AssetNode::outputPartMesh), data,
                    hasMeshHandle, meshHandle, options);

        // Particle
        MDataHandle hasParticlesHandle =
            partHandle.child(AssetNode::outputPartHasParticles);
        MDataHandle particleHandle =
            partHandle.child(AssetNode::outputPartParticle);
        computeParticle(time, partPlug.child(AssetNode::outputPartHasParticles),
                        partPlug.child(AssetNode::outputPartParticle), data,
                        hasParticlesHandle, particleHandle, options);

#if MAYA_API_VERSION >= 201400
        // Volume
        MDataHandle volumeHandle =
            partHandle.child(AssetNode::outputPartVolume);
        if (myPartInfo.type == HAPI_PARTTYPE_VOLUME)
        {
            computeVolume(time, partPlug.child(AssetNode::outputPartVolume),
                          data, volumeHandle, options.preserveScale());
        }
#endif

        // Curve
        MDataHandle curvesHandle =
            partHandle.child(AssetNode::outputPartCurves);
        MDataHandle curvesIsBezierHandle =
            partHandle.child(AssetNode::outputPartCurvesIsBezier);
        computeCurves(time, partPlug.child(AssetNode::outputPartCurves),
                      partPlug.child(AssetNode::outputPartCurvesIsBezier), data,
                      curvesHandle, curvesIsBezierHandle, options);

        // Instancer
        MDataHandle hasInstancerHandle =
            partHandle.child(AssetNode::outputPartHasInstancer);
        MDataHandle instanceHandle =
            partHandle.child(AssetNode::outputPartInstancer);
        computeInstancer(
            time, partPlug.child(AssetNode::outputPartHasInstancer),
            partPlug.child(AssetNode::outputPartInstancer), data,
            hasInstancerHandle, instanceHandle, options.preserveScale());

        // Groups
        MDataHandle groupsHandle =
            partHandle.child(AssetNode::outputPartGroups);
        computeGroups(time, partPlug.child(AssetNode::outputPartGroups), data,
                      groupsHandle, options, needToSyncOutputs);

        // compute material
        MDataHandle materialHandle =
            partHandle.child(AssetNode::outputPartMaterialIds);
        computeMaterial(time, partPlug.child(AssetNode::outputPartMaterialIds),
                        data, materialHandle);

        // Extra attributes
        MDataHandle extraAttributesHandle =
            partHandle.child(AssetNode::outputPartExtraAttributes);
        computeExtraAttributes(
            time, partPlug.child(AssetNode::outputPartExtraAttributes), data,
            extraAttributesHandle, options, needToSyncOutputs);
    }

    return MS::kSuccess;
}

void
OutputGeometryPart::computeCurves(const MTime &time,
                                  const MPlug &curvesPlug,
                                  const MPlug &curvesIsBezierPlug,
                                  MDataBlock &data,
                                  MDataHandle &curvesHandle,
                                  MDataHandle &curvesIsBezierHandle,
                                  AssetNodeOptions::AccessorDataBlock &options)
{
    MStatus status;

    MArrayDataHandle curvesArrayHandle(curvesHandle);

    int curveCount = 0;
    if (myPartInfo.type == HAPI_PARTTYPE_CURVE)
        curveCount = myCurveInfo.curveCount;

    if (curvesArrayHandle.elementCount() != (unsigned int)curveCount)
        Util::resizeArrayDataHandle(curvesArrayHandle, curveCount);

    if (curveCount)
    {
        HAPI_AttributeInfo attrInfo;

        std::vector<float> pArray, pwArray;
        hapiGetPointAttribute(myNodeId, myPartId, "P", attrInfo, pArray);
        hapiGetPointAttribute(myNodeId, myPartId, "Pw", attrInfo, pwArray);
        markAttributeUsed("P");
        markAttributeUsed("Pw");

        int vertexOffset = 0;
        int knotOffset   = 0;
        for (int iCurve = 0; iCurve < myCurveInfo.curveCount; iCurve++)
        {
            CHECK_MSTATUS(curvesArrayHandle.jumpToArrayElement(iCurve));
            MDataHandle curve    = curvesArrayHandle.outputValue();
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
            int numVertices = 0;
            HAPI_GetCurveCounts(Util::theHAPISession.get(), myNodeId, myPartId,
                                &numVertices, iCurve, 1);

            const int nextVertexOffset = vertexOffset + numVertices;
            if (nextVertexOffset > static_cast<int>(pArray.size()) * 3 ||
                (!pwArray.empty() &&
                 nextVertexOffset > static_cast<int>(pwArray.size())))
            {
                MGlobal::displayError("Not enough points to create a curve");
                break;
            }

            // Order of this particular curve
            int order;
            if (myCurveInfo.order != HAPI_CURVE_ORDER_VARYING &&
                myCurveInfo.order != HAPI_CURVE_ORDER_INVALID)
            {
                order = myCurveInfo.order;
            }
            else
            {
                HAPI_GetCurveOrders(Util::theHAPISession.get(), myNodeId,
                                    myPartId, &order, iCurve, 1);
            }

            // If there's not enough vertices, then don't try to create the
            // curve.
            if (numVertices < order)
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

            MPointArray controlVertices(numVertices);
            for (int iDst = 0, iSrc = vertexOffset; iDst < numVertices;
                 ++iDst, ++iSrc)
            {
                controlVertices[iDst] = MPoint(
                    pArray[iSrc * 3], pArray[iSrc * 3 + 1],
                    pArray[iSrc * 3 + 2],
                    pwArray.empty() ? 1.0f : pwArray[iSrc]);
            }

            if (options.preserveScale())
            {
                for (unsigned int i = 0; i < controlVertices.length(); i++)
                {
                    controlVertices[i].x *= 100.0;
                    controlVertices[i].y *= 100.0;
                    controlVertices[i].z *= 100.0;
                    controlVertices[i].w *= 100.0;
                }
            }

            MDoubleArray knotSequences;
            if (myCurveInfo.hasKnots)
            {
                std::vector<float> knots;
                knots.resize(numVertices + order);
                // The Maya knot vector has two fewer knots;
                // the first and last houdini knot are excluded
                knotSequences.setLength(numVertices + order - 2);
                HAPI_GetCurveKnots(Util::theHAPISession.get(), myNodeId,
                                   myPartId, &knots.front(), knotOffset,
                                   numVertices + order);
                // Maya doesn't need the first and last knots
                for (int j = 0; j < numVertices + order - 2; j++)
                    knotSequences[j] = knots[j + 1];
            }
            else if (myCurveInfo.curveType == HAPI_CURVETYPE_BEZIER)
            {
                // Bezier knot vector needs to still be passed in
                knotSequences.setLength(numVertices + order - 2);
                for (int j = 0; j < numVertices + order - 2; j++)
                    knotSequences[j] = j / (order - 1);
            }
            else
            {
                knotSequences.setLength(numVertices + order - 2);
                int j = 0;
                for (; j < order - 1; j++)
                    knotSequences[j] = 0.0;

                for (int k = 1; j < numVertices - 1; k++, j++)
                    knotSequences[j] = (double)k / (numVertices - order + 1);

                for (; j < numVertices + order - 2; j++)
                    knotSequences[j] = 1.0;
            }

            // NOTE: Periodicity is always constant, so periodic and
            //           non-periodic curve meshes will have different parts.
            MFnNurbsCurve curveFn;
            MObject nurbsCurve = curveFn.create(
                controlVertices, knotSequences, order - 1,
                myCurveInfo.isPeriodic ? MFnNurbsCurve::kPeriodic :
                                         MFnNurbsCurve::kOpen,
                false /* 2d? */, myCurveInfo.isRational /* rational? */,
                curveDataObj, &status);
            CHECK_MSTATUS(status);

            // The curve at i will have numVertices vertices, and may have
            // some knots. The knot count will be numVertices + order for
            // nurbs curves
            vertexOffset = nextVertexOffset;
            knotOffset += numVertices + order;
        }

        curvesIsBezierHandle.setBool(myCurveInfo.curveType ==
                                     HAPI_CURVETYPE_BEZIER);
    }

    curvesArrayHandle.setAllClean();
}

template <typename T>
bool
OutputGeometryPart::convertParticleAttribute(T particleArray,
                                             const char *houdiniName,
                                             bool preserveScale)
{
    typedef ARRAYTRAIT(T) Trait;
    typedef ELEMENTTRAIT(T) ElementTrait;

    HAPI_AttributeInfo attrInfo;

    std::vector<typename ElementTrait::ComponentType> dataArray;
    if (!HAPI_FAIL(hapiGetPointAttribute(
            myNodeId, myPartId, houdiniName, attrInfo, dataArray)))
    {
        particleArray = Util::reshapeArray<T>(dataArray);

        if (preserveScale)
        {
            for (unsigned int i = 0; i < particleArray.length(); i++)
                particleArray[i] *= 100.0;
        }

        return true;
    }
    else
    {
        Trait::resize(particleArray, myPartInfo.pointCount);
        Util::zeroArray(particleArray);
        return false;
    }
}

bool
OutputGeometryPart::computeExtraAttribute(const MPlug &extraAttributePlug,
                                          MDataBlock &data,
                                          MDataHandle &extraAttributeHandle,
                                          HAPI_AttributeOwner attributeOwner,
                                          const char *attributeName)
{
    static const MString attributeOwnersString[] = {
        "vertex",
        "point",
        "primitive",
        "detail",
    };
    static const MString dataTypesString[] = {
        "int", "long", "float", "double", "string",
    };

    MDataHandle nameHandle =
        extraAttributeHandle.child(AssetNode::outputPartExtraAttributeName);
    MDataHandle ownerHandle =
        extraAttributeHandle.child(AssetNode::outputPartExtraAttributeOwner);
    MDataHandle dataTypeHandle =
        extraAttributeHandle.child(AssetNode::outputPartExtraAttributeDataType);
    MDataHandle tupleHandle =
        extraAttributeHandle.child(AssetNode::outputPartExtraAttributeTuple);
    MDataHandle dataHandle =
        extraAttributeHandle.child(AssetNode::outputPartExtraAttributeData);

    HAPI_AttributeInfo attributeInfo;
    HAPI_FAIL(HAPI_GetAttributeInfo(Util::theHAPISession.get(), myNodeId,
                                    myPartId, attributeName, attributeOwner,
                                    &attributeInfo));
    if (!attributeInfo.exists)
    {
        // HAPI might not be able to handle certain attributes (e.g.
        // tuple size is 0).
        return false;
    }

    HAPI_StorageType storage = attributeInfo.storage;

    // Particle requires special treatment
    bool hasParticles = myPartInfo.pointCount != 0 &&
                        myPartInfo.vertexCount == 0 &&
                        myPartInfo.faceCount == 0;
    if (hasParticles)
    {
        if (attributeOwner == HAPI_ATTROWNER_POINT)
        {
            // Maya Particle only supports double or double vector per-particle
            // attributes. For int and float attributes, always use the double
            // code path to cast to double.
            if (storage == HAPI_STORAGETYPE_INT ||
                storage == HAPI_STORAGETYPE_INT64 ||
                storage == HAPI_STORAGETYPE_FLOAT ||
                storage == HAPI_STORAGETYPE_FLOAT64)
            {
                storage = HAPI_STORAGETYPE_FLOAT64;
            }
        }
    }

    if (storage == HAPI_STORAGETYPE_FLOAT)
    {
        MFloatArray floatArray;
        hapiGetAttribute(myNodeId, myPartId, attributeOwner, attributeName,
                         attributeInfo, floatArray);

        if (attributeOwner == HAPI_ATTROWNER_DETAIL &&
            attributeInfo.tupleSize == 1)
        {
            dataHandle.setGenericFloat(floatArray[0], true);
        }
        else if (attributeOwner == HAPI_ATTROWNER_DETAIL &&
                 attributeInfo.tupleSize == 2)
        {
            MFnNumericData numericData;
            MObject dataObject = numericData.create(MFnNumericData::k2Float);
            numericData.setData2Float(floatArray[0], floatArray[1]);

            dataHandle.setMObject(dataObject);
        }
        else if (attributeOwner == HAPI_ATTROWNER_DETAIL &&
                 attributeInfo.tupleSize == 3)
        {
            MFnNumericData numericData;
            MObject dataObject = numericData.create(MFnNumericData::k3Float);
            numericData.setData3Float(
                floatArray[0], floatArray[1], floatArray[2]);

            dataHandle.setMObject(dataObject);
        }
        else if (attributeInfo.tupleSize == 3)
        {
            // Since MFnFloatVectorArrayData doesn't exist, use
            // MFnVectorArrayData instead.
            MFnVectorArrayData vectorArrayData;
            MObject dataObject             = vectorArrayData.create();
            MVectorArray outputVectorArray = vectorArrayData.array();
            outputVectorArray = Util::reshapeArray<MVectorArray>(floatArray);

            dataHandle.setMObject(dataObject);
        }
        else
        {
            MFnFloatArrayData floatArrayData;
            MObject dataObject           = floatArrayData.create();
            MFloatArray outputFloatArray = floatArrayData.array();
            outputFloatArray             = floatArray;

            dataHandle.setMObject(dataObject);
        }
    }
    else if (storage == HAPI_STORAGETYPE_FLOAT64)
    {
        MDoubleArray doubleArray;
        hapiGetAttribute(myNodeId, myPartId, attributeOwner, attributeName,
                         attributeInfo, doubleArray);

        if (attributeOwner == HAPI_ATTROWNER_DETAIL &&
            attributeInfo.tupleSize == 1)
        {
            dataHandle.setGenericDouble(doubleArray[0], true);
        }
        else if (attributeOwner == HAPI_ATTROWNER_DETAIL &&
                 attributeInfo.tupleSize == 2)
        {
            MFnNumericData numericData;
            MObject dataObject = numericData.create(MFnNumericData::k2Double);
            numericData.setData2Double(doubleArray[0], doubleArray[1]);

            dataHandle.setMObject(dataObject);
        }
        else if (attributeOwner == HAPI_ATTROWNER_DETAIL &&
                 attributeInfo.tupleSize == 3)
        {
            MFnNumericData numericData;
            MObject dataObject = numericData.create(MFnNumericData::k3Double);
            numericData.setData3Double(
                doubleArray[0], doubleArray[1], doubleArray[2]);

            dataHandle.setMObject(dataObject);
        }
        else if (attributeOwner == HAPI_ATTROWNER_DETAIL &&
                 attributeInfo.tupleSize == 4)
        {
            MFnNumericData numericData;
            MObject dataObject = numericData.create(MFnNumericData::k4Double);
            numericData.setData4Double(
                doubleArray[0], doubleArray[1], doubleArray[2], doubleArray[3]);

            dataHandle.setMObject(dataObject);
        }
        else if (attributeInfo.tupleSize == 3)
        {
            MFnVectorArrayData vectorArrayData;
            MObject dataObject             = vectorArrayData.create();
            MVectorArray outputVectorArray = vectorArrayData.array();
            outputVectorArray = Util::reshapeArray<MVectorArray>(doubleArray);

            dataHandle.setMObject(dataObject);
        }
        else
        {
            MFnDoubleArrayData doubleArrayData;
            MObject dataObject             = doubleArrayData.create();
            MDoubleArray outputDoubleArray = doubleArrayData.array();
            outputDoubleArray              = doubleArray;

            dataHandle.setMObject(dataObject);
        }
    }
    else if (storage == HAPI_STORAGETYPE_INT ||
             storage == HAPI_STORAGETYPE_INT64)
    {
        MIntArray intArray;
        hapiGetAttribute(myNodeId, myPartId, attributeOwner, attributeName,
                         attributeInfo, intArray);

        if (attributeInfo.owner == HAPI_ATTROWNER_DETAIL &&
            attributeInfo.tupleSize == 1)
        {
            dataHandle.setGenericInt(intArray[0], true);
        }
        else if (attributeInfo.owner == HAPI_ATTROWNER_DETAIL &&
                 attributeInfo.tupleSize == 2)
        {
            MFnNumericData numericData;
            MObject dataObject = numericData.create(MFnNumericData::k2Int);
            numericData.setData2Int(intArray[0], intArray[1]);

            dataHandle.setMObject(dataObject);
        }
        else if (attributeInfo.owner == HAPI_ATTROWNER_DETAIL &&
                 attributeInfo.tupleSize == 3)
        {
            MFnNumericData numericData;
            MObject dataObject = numericData.create(MFnNumericData::k3Int);
            numericData.setData3Int(intArray[0], intArray[1], intArray[2]);

            dataHandle.setMObject(dataObject);
        }
        else
        {
            MFnIntArrayData intArrayData;
            MObject dataObject       = intArrayData.create();
            MIntArray outputIntArray = intArrayData.array();
            outputIntArray           = intArray;

            dataHandle.setMObject(dataObject);
        }
    }
    else if (storage == HAPI_STORAGETYPE_STRING)
    {
        MStringArray stringArray;
        hapiGetAttribute(myNodeId, myPartId, attributeOwner, attributeName,
                         attributeInfo, stringArray);

        if (attributeInfo.owner == HAPI_ATTROWNER_DETAIL &&
            attributeInfo.tupleSize == 1)
        {
            dataHandle.setString(stringArray[0]);
        }
        else
        {
            MFnStringArrayData stringArrayData;
            MObject dataObject             = stringArrayData.create();
            MStringArray outputStringArray = stringArrayData.array();
            outputStringArray              = stringArray;

            dataHandle.setMObject(dataObject);
        }
    }
    else
    {
        return false;
    }

    const MString &ownerString    = attributeOwnersString[attributeOwner];
    const MString &dataTypeString = dataTypesString[storage];

    nameHandle.setString(attributeName);
    ownerHandle.setString(ownerString);
    dataTypeHandle.setString(dataTypeString);
    tupleHandle.setInt(attributeInfo.tupleSize);

    return true;
}

void
OutputGeometryPart::computeParticle(
    const MTime &time,
    const MPlug &hasParticlePlug,
    const MPlug &particlePlug,
    MDataBlock &data,
    MDataHandle &hasParticlesHandle,
    MDataHandle &particleHandle,
    AssetNodeOptions::AccessorDataBlock &options)
{
    data.setClean(hasParticlePlug);
    data.setClean(particlePlug);

    bool hasParticles = myPartInfo.pointCount != 0 &&
                        myPartInfo.vertexCount == 0 &&
                        myPartInfo.faceCount == 0;

    if (!hasParticlesHandle.asBool() && !hasParticles)
        return;

    hasParticlesHandle.setBool(hasParticles);

    MDataHandle currentTimeHandle =
        particleHandle.child(AssetNode::outputPartParticleCurrentTime);
    MDataHandle positionsHandle =
        particleHandle.child(AssetNode::outputPartParticlePositions);
    MDataHandle arrayDataHandle =
        particleHandle.child(AssetNode::outputPartParticleArrayData);

    std::vector<float> floatArray;
    std::vector<int> intArray;

    int particleCount = myPartInfo.pointCount;

    // currentTime
    currentTimeHandle.setMTime(time);

    // array data
    MObject arrayDataObj = arrayDataHandle.data();
    MFnArrayAttrsData arrayDataFn(arrayDataObj);
    if (arrayDataObj.isNull())
    {
        arrayDataObj = arrayDataFn.create();
        arrayDataHandle.setMObject(arrayDataObj);

        arrayDataObj = arrayDataHandle.data();
        arrayDataFn.setObject(arrayDataObj);
    }

    // count
    MDoubleArray countArray = arrayDataFn.doubleArray("count");
    countArray.setLength(1);
    countArray[0] = particleCount;
    markAttributeUsed("count");

    // position
    convertParticleAttribute(
        arrayDataFn.vectorArray("position"), "P", options.preserveScale());
    {
        MObject positionsObj = positionsHandle.data();
        MFnVectorArrayData positionDataFn(positionsObj);
        if (positionsObj.isNull())
        {
            positionsObj = positionDataFn.create();
            positionsHandle.setMObject(positionsObj);
        }

        MVectorArray positions = positionDataFn.array();
        positions              = arrayDataFn.vectorArray("position");
    }
    markAttributeUsed("P");
    markAttributeUsed("position");

    if (!hasParticles)
        return;

    // id
    {
        HAPI_AttributeInfo attrInfo;

        MDoubleArray idArray = arrayDataFn.doubleArray("id");
        if (HAPI_FAIL(hapiGetPointAttribute(
                myNodeId, myPartId, "id", attrInfo, idArray)))
        {
            idArray.setLength(particleCount);
            for (unsigned int i = 0; i < idArray.length(); i++)
            {
                idArray[i] = i;
            }
        }
    }
    markAttributeUsed("id");

    // velocity
    convertParticleAttribute(
        arrayDataFn.vectorArray("velocity"), "v", options.preserveScale());
    markAttributeUsed("v");
    markAttributeUsed("velocity");

    // acceleration
    convertParticleAttribute(arrayDataFn.vectorArray("acceleration"), "force",
                             options.preserveScale());
    markAttributeUsed("acceleration");
    markAttributeUsed("force");

    // worldPosition
    arrayDataFn.vectorArray("worldPosition")
        .copy(arrayDataFn.vectorArray("position"));
    markAttributeUsed("worldPosition");

    // worldVelocity
    arrayDataFn.vectorArray("worldVelocity")
        .copy(arrayDataFn.vectorArray("velocity"));
    markAttributeUsed("worldVelocity");

    // worldVelocityInObjectSpace
    arrayDataFn.vectorArray("worldVelocityInObjectSpace")
        .copy(arrayDataFn.vectorArray("velocity"));
    markAttributeUsed("worldVelocityInObjectSpace");

    // mass
    convertParticleAttribute(arrayDataFn.doubleArray("mass"), "mass", true);
    markAttributeUsed("mass");

    // birthTime
    bool birthTimeDefined = convertParticleAttribute(
        arrayDataFn.doubleArray("birthTime"), "birthTime", false);
    markAttributeUsed("birthTime");

    // age
    bool ageDefined = convertParticleAttribute(
        arrayDataFn.doubleArray("age"), "age", false);
    markAttributeUsed("age");

    // in Maya, age is an output computed from birthTime
    // so if the asset has explicitly defined birthTime, we use it
    // if there is age but no birthTime, we compute it from age
    // if there is neither age nor birthTime, we leave birthTime zeroed
    if (ageDefined && !birthTimeDefined)
    {
        double timeInSec            = time.as(MTime::Unit::kSeconds);
        MDoubleArray birthTimeArray = arrayDataFn.doubleArray("birthTime");
        MDoubleArray ageArray       = arrayDataFn.doubleArray("age");
        for (int i = 0; i < particleCount; i++)
        {
            birthTimeArray[i] = timeInSec - ageArray[i];
        }
    }

    // lifespanPP
    convertParticleAttribute(
        arrayDataFn.doubleArray("lifespanPP"), "life", false);

    // finalLifespanPP
    convertParticleAttribute(
        arrayDataFn.doubleArray("finalLifespanPP"), "life", false);
    markAttributeUsed("life");

    // other attributes
    std::vector<HAPI_StringHandle> attributeNames(
        myPartInfo.attributeCounts[HAPI_ATTROWNER_POINT]);
    HAPI_GetAttributeNames(Util::theHAPISession.get(), myNodeId, myPartId,
                           HAPI_ATTROWNER_POINT,
                           attributeNames.empty() ? NULL : &attributeNames[0],
                           myPartInfo.attributeCounts[HAPI_ATTROWNER_POINT]);
    for (unsigned int i = 0; i < attributeNames.size(); i++)
    {
        MString attributeName = Util::HAPIString(attributeNames[i]);

        // skip attributes that were done above already
        if (isAttributeUsed(attributeName.asChar()))
        {
            continue;
        }

        // translate certain attributes into Maya names
        MString translatedAttributeName;
        if (attributeName == "Cd")
        {
            translatedAttributeName = "rgbPP";
        }
        else if (attributeName == "Alpha")
        {
            translatedAttributeName = "opacityPP";
        }
        else if (attributeName == "pscale")
        {
            translatedAttributeName = "radiusPP";
        }
        else if (attributeName == "life")
        {
            translatedAttributeName = "finalLifespanPP";
        }
        else
        {
            translatedAttributeName = attributeName;
        }

        HAPI_AttributeInfo attributeInfo;

        HAPI_GetAttributeInfo(Util::theHAPISession.get(), myNodeId, myPartId,
                              attributeName.asChar(), HAPI_ATTROWNER_POINT,
                              &attributeInfo);

        HAPI_StorageType storage = attributeInfo.storage;
        if (storage == HAPI_STORAGETYPE_INT ||
            storage == HAPI_STORAGETYPE_INT64 ||
            storage == HAPI_STORAGETYPE_FLOAT ||
            storage == HAPI_STORAGETYPE_FLOAT64)
        {
            storage = HAPI_STORAGETYPE_FLOAT64;
        }

        // put the data into MFnArrayAttrsData
        if (storage == HAPI_STORAGETYPE_FLOAT64 && attributeInfo.tupleSize == 3)
        {
            convertParticleAttribute(
                arrayDataFn.vectorArray(translatedAttributeName),
                attributeName.asChar(), false);
        }
        else if (storage == HAPI_STORAGETYPE_FLOAT64 &&
                 attributeInfo.tupleSize == 1)
        {
            convertParticleAttribute(
                arrayDataFn.doubleArray(translatedAttributeName),
                attributeName.asChar(), false);
        }
    }
}

#if MAYA_API_VERSION >= 201400
void
OutputGeometryPart::computeVolume(const MTime &time,
                                  const MPlug &volumePlug,
                                  MDataBlock &data,
                                  MDataHandle &volumeHandle,
                                  const bool preserveScale)
{
    MDataHandle gridHandle =
        volumeHandle.child(AssetNode::outputPartVolumeGrid);
    MDataHandle transformHandle =
        volumeHandle.child(AssetNode::outputPartVolumeTransform);
    MDataHandle resHandle = volumeHandle.child(AssetNode::outputPartVolumeRes);
    MDataHandle nameHandle =
        volumeHandle.child(AssetNode::outputPartVolumeName);

    // grid
    {
        MObject gridDataObj = gridHandle.data();
        MFnFloatArrayData gridDataFn(gridDataObj);
        if (gridDataObj.isNull())
        {
            gridDataObj = gridDataFn.create();
            gridHandle.setMObject(gridDataObj);
        }

        MFloatArray grid = gridDataFn.array();

        int xres     = myVolumeInfo.xLength;
        int yres     = myVolumeInfo.yLength;
        int zres     = myVolumeInfo.zLength;
        int tileSize = myVolumeInfo.tileSize;

        grid.setLength(xres * yres * zres);
        for (unsigned int i = 0; i < grid.length(); i++)
            grid[i] = 0.0f;

        std::vector<float> tile;
        tile.resize(tileSize * tileSize * tileSize);

        HAPI_VolumeTileInfo tileInfo;
        HAPI_GetFirstVolumeTile(
            Util::theHAPISession.get(), myNodeId, myPartId, &tileInfo);

#ifdef max
#undef max
#endif

        while (tileInfo.minX != std::numeric_limits<int>::max() &&
               tileInfo.minY != std::numeric_limits<int>::max() &&
               tileInfo.minZ != std::numeric_limits<int>::max())
        {
            HAPI_GetVolumeTileFloatData(Util::theHAPISession.get(), myNodeId,
                                        myPartId, 0.0f, &tileInfo,
                                        &tile.front(), (int)tile.size());

            for (int k = 0; k < tileSize; k++)
                for (int j = 0; j < tileSize; j++)
                    for (int i = 0; i < tileSize; i++)
                    {
                        int z = k + tileInfo.minZ - myVolumeInfo.minZ,
                            y = j + tileInfo.minY - myVolumeInfo.minY,
                            x = i + tileInfo.minX - myVolumeInfo.minX;

                        int index = xres * yres * z + xres * y + x;

                        float value =
                            tile[k * tileSize * tileSize + j * tileSize + i];
                        if (x < xres && y < yres && z < zres && x >= 0 &&
                            y >= 0 && z >= 0)
                        {
                            grid[index] = value;
                        }
                    }

            HAPI_GetNextVolumeTile(
                Util::theHAPISession.get(), myNodeId, myPartId, &tileInfo);
        }
    }

    // transform
    computeVolumeTransform(time, transformHandle, preserveScale);

    // resolution
    MFloatArray resolution;
    resolution.append(myVolumeInfo.xLength);
    resolution.append(myVolumeInfo.yLength);
    resolution.append(myVolumeInfo.zLength);
    MFnFloatArrayData resCreator;
    resHandle.set(resCreator.create(resolution));

    // name
    nameHandle.set(Util::HAPIString(myVolumeInfo.nameSH));
}
#endif

void
OutputGeometryPart::computeMesh(const MTime &time,
                                const MPlug &hasMeshPlug,
                                const MPlug &meshPlug,
                                MDataBlock &data,
                                MDataHandle &hasMeshHandle,
                                MDataHandle &meshHandle,
                                AssetNodeOptions::AccessorDataBlock &options)
{
    MStatus status;

    data.setClean(hasMeshPlug);
    data.setClean(meshPlug);

    bool hasMesh = myPartInfo.type == HAPI_PARTTYPE_MESH &&
                   myPartInfo.faceCount != 0;

    // if we didn't output anything in the previous compute, and still doesn't
    // have to output anything, then we don't need to bother with clearing the
    // output and can return early
    if (!hasMeshHandle.asBool() && !hasMesh)
    {
        return;
    }

    MDataHandle meshCurrentColorSetHandle =
        meshHandle.child(AssetNode::outputPartMeshCurrentColorSet);
    MDataHandle meshCurrentUVHandle =
        meshHandle.child(AssetNode::outputPartMeshCurrentUV);
    MDataHandle meshDataHandle =
        meshHandle.child(AssetNode::outputPartMeshData);

    // create mesh
    MObject meshDataObj = meshDataHandle.data();
    MFnMeshData meshDataFn(meshDataObj);
    if (meshDataObj.isNull())
    {
        // set the MDataHandle
        meshDataObj = meshDataFn.create();
        meshDataHandle.setMObject(meshDataObj);

        // then get the copy from MDataHandle
        meshDataObj = meshDataHandle.data();
        meshDataFn.setObject(meshDataObj);
    }

    HAPI_AttributeInfo attrInfo;
    std::vector<float> floatArray;
    std::vector<int> intArray;

    int currentlayer = -1;
    if (!HAPI_FAIL(hapiGetDetailAttribute(
            myNodeId, myPartId, "currentlayer", attrInfo, currentlayer)))
    {
        currentlayer -= 1;
    }

    // vertex array
    MFloatPointArray vertexArray;
    if (hasMesh)
    {
        hapiGetPointAttribute(myNodeId, myPartId, "P", attrInfo, floatArray);

        if (options.preserveScale())
        {
            for (size_t i = 0; i < floatArray.size(); i++)
                floatArray[i] *= 100.0f;
        }

        vertexArray =
            Util::reshapeArray<3, 0, 4, 0, 3, MFloatPointArray>(floatArray);
        markAttributeUsed("P");
    }

    // polygon counts
    MIntArray polygonCounts;
    if (hasMesh)
    {
        intArray.resize(myPartInfo.faceCount);

        HAPI_GetFaceCounts(Util::theHAPISession.get(), myNodeId, myPartId,
                           &intArray.front(), 0, myPartInfo.faceCount);

        polygonCounts = MIntArray(&intArray.front(), intArray.size());
    }

    // polygon connects
    std::vector<int> polygonConnectsReversed;
    MIntArray polygonConnects;
    if (hasMesh)
    {
        polygonConnectsReversed.resize(myPartInfo.vertexCount);

        HAPI_GetVertexList(Util::theHAPISession.get(), myNodeId, myPartId,
                           &polygonConnectsReversed.front(), 0,
                           myPartInfo.vertexCount);

        polygonConnects = MIntArray(
            &polygonConnectsReversed.front(), polygonConnectsReversed.size());

        Util::reverseWindingOrder(polygonConnects, polygonCounts);
    }

    MFnMesh meshFn;
    meshFn.create(vertexArray.length(), polygonCounts.length(), vertexArray,
                  polygonCounts, polygonConnects, meshDataObj, &status);
    CHECK_MSTATUS(status);

    hasMeshHandle.setBool(hasMesh);

    // Check that the mesh created is what we tried to create. Assets could
    // output mesh with bad faces that Maya cannot handle. When this happens,
    // Maya would skip the bad faces, so the resulting mesh would be slightly
    // different. This makes it no longer possible to apply geometry attributes
    // (like UVs and color sets) to the mesh.
    {
        bool mismatch_topology = false;

        MString partName;
        if (myPartInfo.nameSH != 0)
        {
            partName = Util::HAPIString(myPartInfo.nameSH);
        }
        if (vertexArray.length() != (unsigned int)meshFn.numVertices())
        {
            DISPLAY_WARNING("Attempted to create ^1s vertices for ^2s, but "
                            "only ^3s vertices "
                            "were created.",
                            MString() + vertexArray.length(), partName,
                            MString() + meshFn.numVertices());
            mismatch_topology = true;
        }
        if (polygonCounts.length() != (unsigned int)meshFn.numPolygons())
        {
            DISPLAY_WARNING("Attempted to create ^1s polygons for ^2s, but "
                            "only ^3s polygons "
                            "were created.",
                            MString() + polygonCounts.length(), partName,
                            MString() + meshFn.numPolygons());
            mismatch_topology = true;
        }
        if (polygonConnects.length() != (unsigned int)meshFn.numFaceVertices())
        {
            DISPLAY_WARNING(
                "Attempted to create ^1s face-vertices for ^2s, but only ^3s "
                "face-vertices were created.",
                MString() + polygonConnects.length(), partName,
                MString() + meshFn.numFaceVertices());
            mismatch_topology = true;
        }

        if (mismatch_topology)
        {
            DISPLAY_WARNING(
                MString("This likely means that the Houdini Digital Asset is "
                        "outputting bad geometry that Maya cannot handle. As a "
                        "result, other attributes (such as UVs and color sets) "
                        "cannot be transferred."));

            return;
        }
    }

    // find locked normals
    HAPI_AttributeOwner lockedNormalOwner = HAPI_ATTROWNER_MAX;
    std::vector<int> lockedNormal;
    if (hasMesh)
    {
        if (!HAPI_FAIL(hapiGetVertexAttribute(myNodeId, myPartId,
                                              "maya_locked_normal", attrInfo,
                                              lockedNormal)))
        {
            markAttributeUsed("maya_locked_normal");

            // ignore it if none of the normals are locked
            for (size_t i = 0; i < lockedNormal.size(); i++)
            {
                if (lockedNormal[i])
                {
                    lockedNormalOwner = HAPI_ATTROWNER_VERTEX;
                    Util::reverseWindingOrder(lockedNormal, polygonCounts);
                    break;
                }
            }
            if (lockedNormalOwner == HAPI_ATTROWNER_MAX)
            {
                lockedNormal.clear();
            }
        }
        else if (!HAPI_FAIL(hapiGetPointAttribute(myNodeId, myPartId,
                                                  "maya_locked_normal",
                                                  attrInfo, lockedNormal)))
        {
            markAttributeUsed("maya_locked_normal");

            // ignore it if none of the normals are locked
            for (size_t i = 0; i < lockedNormal.size(); i++)
            {
                if (lockedNormal[i])
                {
                    lockedNormalOwner = HAPI_ATTROWNER_POINT;
                    break;
                }
            }
            if (lockedNormalOwner == HAPI_ATTROWNER_MAX)
            {
                lockedNormal.clear();
            }
        }
    }

    // store the vertex locked normal, if it is ever promoted to vertex
    std::vector<int> vertexLockedNormalBuffer;

    // normal array
    if (hasMesh && lockedNormalOwner != HAPI_ATTROWNER_MAX &&
        options.outputMeshPreserveLockedNormals())
    {
        HAPI_AttributeOwner normalOwner = HAPI_ATTROWNER_MAX;
        if (!HAPI_FAIL(hapiGetVertexAttribute(
                myNodeId, myPartId, "N", attrInfo, floatArray)))
        {
            normalOwner = HAPI_ATTROWNER_VERTEX;
        }
        else if (!HAPI_FAIL(hapiGetPointAttribute(
                     myNodeId, myPartId, "N", attrInfo, floatArray)))
        {
            normalOwner = HAPI_ATTROWNER_POINT;
        }

        MVectorArray normal(reinterpret_cast<float(*)[3]>(&floatArray.front()),
                            floatArray.size() / 3);
        if (normalOwner == HAPI_ATTROWNER_VERTEX)
        {
            Util::reverseWindingOrder(normal, polygonCounts);
        }

        HAPI_AttributeOwner promotedOwner = HAPI_ATTROWNER_MAX;
        if (normalOwner != HAPI_ATTROWNER_MAX)
        {
            markAttributeUsed("N");

            // promote the normals and locked normals to matching class
            MVectorArray *promotedNormal           = NULL;
            std::vector<int> *promotedLockedNormal = NULL;
            MVectorArray vertexNormalBuffer;
            if ((normalOwner == HAPI_ATTROWNER_POINT &&
                 lockedNormalOwner == HAPI_ATTROWNER_POINT) ||
                (normalOwner == HAPI_ATTROWNER_VERTEX &&
                 lockedNormalOwner == HAPI_ATTROWNER_VERTEX))
            {
                promotedOwner        = normalOwner;
                promotedNormal       = &normal;
                promotedLockedNormal = &lockedNormal;
            }
            else if (normalOwner == HAPI_ATTROWNER_POINT &&
                     lockedNormalOwner == HAPI_ATTROWNER_VERTEX)
            {
                Util::promoteAttributeData<3, 0, 0>(
                    HAPI_ATTROWNER_VERTEX, vertexNormalBuffer,
                    HAPI_ATTROWNER_POINT, normal, normal.length(),
                    &polygonCounts, &polygonConnects);

                promotedOwner        = HAPI_ATTROWNER_VERTEX;
                promotedNormal       = &vertexNormalBuffer;
                promotedLockedNormal = &lockedNormal;
            }
            else if (normalOwner == HAPI_ATTROWNER_VERTEX &&
                     lockedNormalOwner == HAPI_ATTROWNER_POINT)
            {
                if (!vertexLockedNormalBuffer.size())
                {
                    Util::promoteAttributeData<1, 0, 0>(
                        HAPI_ATTROWNER_VERTEX, vertexLockedNormalBuffer,
                        HAPI_ATTROWNER_POINT, lockedNormal, lockedNormal.size(),
                        &polygonCounts, &polygonConnects);
                }

                promotedOwner        = HAPI_ATTROWNER_VERTEX;
                promotedNormal       = &normal;
                promotedLockedNormal = &vertexLockedNormalBuffer;
            }

            // set the normals only if it's locked
            if (promotedOwner == HAPI_ATTROWNER_VERTEX)
            {
                MVectorArray setNormalList;
                MIntArray faceList;
                MIntArray vertexList;
                for (unsigned int i = 0, j = 0, length = polygonCounts.length();
                     i < length; ++i)
                {
                    for (int k = 0; k < polygonCounts[i]; ++j, ++k)
                    {
                        if ((*promotedLockedNormal)[j])
                        {
                            setNormalList.append((*promotedNormal)[j]);
                            faceList.append(i);
                            vertexList.append(polygonConnects[j]);
                        }
                    }
                }

                meshFn.setFaceVertexNormals(
                    setNormalList, faceList, vertexList);
            }
            else if (promotedOwner == HAPI_ATTROWNER_POINT)
            {
                MVectorArray setNormalList;
                MIntArray vertexList;
                for (unsigned int i = 0, length = normal.length(); i < length;
                     ++i)
                {
                    if ((*promotedLockedNormal)[i])
                    {
                        setNormalList.append((*promotedNormal)[i]);
                        vertexList.append(i);
                    }
                }

                meshFn.setVertexNormals(setNormalList, vertexList);
            }
        }
    }

    // hard/soft edge
    if (hasMesh && options.outputMeshPreserveHardEdges())
    {
        if (!HAPI_FAIL(hapiGetVertexAttribute(
                myNodeId, myPartId, "maya_hard_edge", attrInfo, intArray)))
        {
            markAttributeUsed("maya_hard_edge");

            Util::reverseWindingOrder(intArray, polygonCounts);

            // promote the locked normals to vertex
            std::vector<int> *vertexLockedNormal = NULL;
            if (lockedNormalOwner == HAPI_ATTROWNER_POINT)
            {
                if (!vertexLockedNormalBuffer.size())
                {
                    Util::promoteAttributeData<1, 0, 0>(
                        HAPI_ATTROWNER_VERTEX, vertexLockedNormalBuffer,
                        HAPI_ATTROWNER_POINT, lockedNormal, lockedNormal.size(),
                        &polygonCounts, &polygonConnects);
                }

                vertexLockedNormal = &vertexLockedNormalBuffer;
            }
            else if (lockedNormalOwner == HAPI_ATTROWNER_VERTEX)
            {
                vertexLockedNormal = &lockedNormal;
            }
#if MAYA_API_VERSION >= 201800
            MIntArray edgeIds;
            MIntArray edgeSmoothing;
#endif
            size_t polygonVertexOffset = 0;
            for (MItMeshPolygon itMeshPolygon(meshDataObj);
                 !itMeshPolygon.isDone(); itMeshPolygon.next())
            {
                MIntArray edges;
                itMeshPolygon.getEdges(edges);
                int numVertices = edges.length();

                for (int i = 0; i < numVertices; i++)
                {
                    // first vertex in the Houdini winding order
                    int polygonVertexIndex1 = polygonVertexOffset +
                                              (i + 1) % numVertices;
                    // second vertex in the Houdini winding order
                    int polygonVertexIndex2 = polygonVertexOffset + i;

                    // If there is even one locked normal, calling
                    // setEdgeSmoothing becomes very expensive for some reasons.
                    // So call it only when necessary. An edge is default to
                    // smooth, so we can normally skip the smooth edges.
                    // However, there is an exception. If a locked normal is
                    // set, then the edge seems to become hard. So we need to
                    // explicitly set those to smooth.
                    if (intArray[polygonVertexIndex1] ||
                        (vertexLockedNormal &&
                         ((*vertexLockedNormal)[polygonVertexIndex1] ||
                          (*vertexLockedNormal)[polygonVertexIndex2])))
                    {
#if MAYA_API_VERSION >= 201800
                        edgeIds.append(edges[i]);
                        edgeSmoothing.append(!intArray[polygonVertexIndex1]);
#else
                        CHECK_MSTATUS(meshFn.setEdgeSmoothing(
                            edges[i], !intArray[polygonVertexIndex1]));
#endif
                    }
                }
                polygonVertexOffset += numVertices;
            }
#if MAYA_API_VERSION >= 201800
            if (edgeIds.length() > 0)
            {
                CHECK_MSTATUS(meshFn.setEdgeSmoothings(edgeIds, edgeSmoothing));
            }
#endif
            assert(polygonVertexOffset == intArray.size());
        }
    }

    // uv
    if (hasMesh)
    {
        MString currentUVSetName;
        MStringArray uvSetNames;
        MStringArray mappedUVAttributeNames;

        hapiGetDetailAttribute(
            myNodeId, myPartId, "maya_uv_current", attrInfo, currentUVSetName);
        markAttributeUsed("maya_uv_current");

        hapiGetDetailAttribute(
            myNodeId, myPartId, "maya_uv_name", attrInfo, uvSetNames);
        markAttributeUsed("maya_uv_name");

        hapiGetDetailAttribute(myNodeId, myPartId, "maya_uv_mapped_uv",
                               attrInfo, mappedUVAttributeNames);
        markAttributeUsed("maya_uv_mapped_uv");

        bool useMappedUV =
            uvSetNames.length() &&
            (uvSetNames.length() == mappedUVAttributeNames.length());

        // If no maya_uv_current, choose one to use as the current UV.
        if (!currentUVSetName.length())
        {
            if (currentlayer != -1)
            {
                // Default to currentlayer
                currentUVSetName = Util::getAttrLayerName("uv", currentlayer);
            }
            else
            {
                // Default to the first uv
                currentUVSetName = Util::getAttrLayerName("uv", 0);
            }

            if (useMappedUV)
            {
                ArrayIterator<MStringArray> begin =
                    arrayBegin(mappedUVAttributeNames);
                ArrayIterator<MStringArray> end =
                    arrayEnd(mappedUVAttributeNames);

                ArrayIterator<MStringArray> iter = std::find(
                    begin, end, currentUVSetName);
                if (iter != end)
                {
                    currentUVSetName = uvSetNames[std::distance(begin, iter)];
                }
            }
        }

        bool hasDefaultUVSet = std::find(arrayBegin(uvSetNames),
                                         arrayEnd(uvSetNames),
                                         "map1") != arrayEnd(uvSetNames);

        int layerIndex = 0;
        for (;;)
        {
            const MString uvAttributeName = Util::getAttrLayerName(
                "uv", layerIndex);

            HAPI_AttributeInfo uvAttrInfo;
            bool found = false;
            if (!HAPI_FAIL(hapiGetVertexAttribute(myNodeId, myPartId,
                                                  uvAttributeName.asChar(),
                                                  uvAttrInfo, floatArray)))
            {
                found = true;
            }
            else if (!HAPI_FAIL(hapiGetPointAttribute(myNodeId, myPartId,
                                                      uvAttributeName.asChar(),
                                                      uvAttrInfo, floatArray)))
            {
                found = true;
            }

            if (!found)
            {
                break;
            }

            markAttributeUsed(uvAttributeName.asChar());

            MFloatArray uArray(polygonConnects.length());
            MFloatArray vArray(polygonConnects.length());
            MIntArray vertexList(polygonConnects.length());
            if (uvAttrInfo.owner == HAPI_ATTROWNER_VERTEX)
            {
                // attempt to restore the shared UVs

                // pointNumber -> uvIndex
                std::vector<int> uvIndexMap(vertexArray.length(), -1);
                // uvIndex -> alternate uvIndex
                std::vector<int> uvAlternateIndexMap(
                    polygonConnects.length(), -1);

                int uvCount = 0;
                for (unsigned int i = 0; i < polygonConnects.length(); ++i)
                {
                    // Since floatArray is in the reversed winding order, we
                    // need to get point number in the revered winding order as
                    // well.
                    int &uvIndex = uvIndexMap[polygonConnectsReversed[i]];

                    float u = floatArray[i * uvAttrInfo.tupleSize + 0];
                    float v = floatArray[i * uvAttrInfo.tupleSize + 1];

                    int lastMappedUVIndex = -1;
                    int mappedUVIndex     = -1;
                    if (uvIndex != -1)
                    {
                        int currMappedUVIndex = uvIndex;
                        while (currMappedUVIndex != -1)
                        {
                            // check that the UV coordinates are the same
                            if (u == uArray[currMappedUVIndex] &&
                                v == vArray[currMappedUVIndex])
                            {
                                mappedUVIndex = currMappedUVIndex;
                                break;
                            }

                            lastMappedUVIndex = currMappedUVIndex;
                            currMappedUVIndex =
                                uvAlternateIndexMap[currMappedUVIndex];
                        }
                    }

                    if (mappedUVIndex == -1)
                    {
                        mappedUVIndex = uvCount;
                        uvCount++;

                        uArray[mappedUVIndex] = u;
                        vArray[mappedUVIndex] = v;

                        if (lastMappedUVIndex != -1)
                        {
                            uvAlternateIndexMap[lastMappedUVIndex] =
                                mappedUVIndex;
                        }
                        else
                        {
                            uvIndex = mappedUVIndex;
                        }
                    }

                    vertexList[i] = mappedUVIndex;
                }

                uArray.setLength(uvCount);
                vArray.setLength(uvCount);

                Util::reverseWindingOrder(vertexList, polygonCounts);
            }
            else if (uvAttrInfo.owner == HAPI_ATTROWNER_POINT)
            {
                // all the UVs are shared

                uArray.setLength(uvAttrInfo.count);
                vArray.setLength(uvAttrInfo.count);
                for (unsigned int i = 0, length = uArray.length(); i < length;
                     ++i)
                {
                    uArray[i] = floatArray[i * uvAttrInfo.tupleSize + 0];
                    vArray[i] = floatArray[i * uvAttrInfo.tupleSize + 1];
                }

                for (unsigned int i = 0, length = polygonConnects.length();
                     i < length; ++i)
                {
                    vertexList[i] = polygonConnects[i];
                }
            }

            // get the mapped UV name
            MString uvSetName = uvAttributeName;
            if (useMappedUV)
            {
                ArrayIterator<MStringArray> begin =
                    arrayBegin(mappedUVAttributeNames);
                ArrayIterator<MStringArray> end =
                    arrayEnd(mappedUVAttributeNames);

                ArrayIterator<MStringArray> iter = std::find(
                    begin, end, uvAttributeName);
                if (iter != end)
                {
                    uvSetName = uvSetNames[std::distance(begin, iter)];
                }
            }

            // map1 already exists by default
            if (uvSetName != "map1")
            {
                meshFn.createUVSetDataMesh(uvSetName);
            }

            // If currentUVSetName is set, then use it to determine the current
            // UV set. Otherwise, use the first as the current UV set.
            if ((uvSetName == currentUVSetName))
            {
                meshCurrentUVHandle.setString(uvSetName);
            }

            CHECK_MSTATUS(meshFn.setUVs(uArray, vArray, &uvSetName));
            CHECK_MSTATUS(
                meshFn.assignUVs(polygonCounts, vertexList, &uvSetName));

            // Copy the current UVs to "map1", so that the viewport uses the
            // correct UVs for textures
            if (!hasDefaultUVSet && uvSetName != "map1" &&
                uvSetName == currentUVSetName)
            {
                MString defaultUVSet = "map1";
                CHECK_MSTATUS(meshFn.setUVs(uArray, vArray, &defaultUVSet));
                CHECK_MSTATUS(
                    meshFn.assignUVs(polygonCounts, vertexList, &defaultUVSet));
            }

            layerIndex++;
        }
    }

    // color and alpha
    if (hasMesh)
    {
        MString currentColorSetName;
        MStringArray colorSetNames;
        MStringArray mappedCdAttributeNames;
        MStringArray mappedAlphaAttributeNames;
        MStringArray colorReps;

        hapiGetDetailAttribute(myNodeId, myPartId, "maya_colorset_current",
                               attrInfo, currentColorSetName);
        markAttributeUsed("maya_colorset_current");

        hapiGetDetailAttribute(
            myNodeId, myPartId, "maya_colorset_name", attrInfo, colorSetNames);
        markAttributeUsed("maya_colorset_name");

        hapiGetDetailAttribute(myNodeId, myPartId, "maya_colorset_mapped_Cd",
                               attrInfo, mappedCdAttributeNames);
        markAttributeUsed("maya_colorset_mapped_Cd");

        hapiGetDetailAttribute(myNodeId, myPartId, "maya_colorset_mapped_Alpha",
                               attrInfo, mappedAlphaAttributeNames);
        markAttributeUsed("maya_colorset_mapped_Alpha");

        hapiGetDetailAttribute(
            myNodeId, myPartId, "maya_colorRep", attrInfo, colorReps);
        markAttributeUsed("maya_colorRep");

        // if there is no Alpha, still want to map the color set names
        bool useMappedColorset =
            colorSetNames.length() &&
            (colorSetNames.length() == mappedCdAttributeNames.length());
        bool useMappedAlpha =
            colorSetNames.length() &&
            (colorSetNames.length() == mappedAlphaAttributeNames.length());
#if MAYA_API_VERSION >= 201600
        bool useColorRep = colorSetNames.length() &&
                           (colorSetNames.length() == colorReps.length());
#endif

        int layerIndex = 0;
        for (;;)
        {
            const MString cdAttributeName = Util::getAttrLayerName(
                "Cd", layerIndex);
            const MString alphaAttributeName = Util::getAttrLayerName(
                "Alpha", layerIndex);
            // prior to 2016, you gould get the color representation, but you
            // couldn't set it
#if MAYA_API_VERSION >= 201600
            MFnMesh::MColorRepresentation colorRep =
                MFnMesh::MColorRepresentation::kRGBA;
            if (useColorRep)
            {
                if (layerIndex < (int)colorReps.length())
                {
                    if (colorReps[layerIndex] == "RGB")
                        colorRep = MFnMesh::MColorRepresentation::kRGB;
                    if (colorReps[layerIndex] == "A")
                        colorRep = MFnMesh::MColorRepresentation::kAlpha;
                }
            }
#endif

            HAPI_AttributeOwner colorOwner;
            if (!HAPI_FAIL(hapiGetAnyAttribute(myNodeId, myPartId,
                                               cdAttributeName.asChar(),
                                               attrInfo, floatArray)))
            {
                colorOwner = attrInfo.owner;
            }
            else
            {
                colorOwner = HAPI_ATTROWNER_MAX;
            }

            HAPI_AttributeOwner alphaOwner;
            std::vector<float> alphaArray;
            if (!HAPI_FAIL(hapiGetAnyAttribute(myNodeId, myPartId,
                                               alphaAttributeName.asChar(),
                                               attrInfo, alphaArray)))
            {
                alphaOwner = attrInfo.owner;
            }
            else
            {
                alphaOwner = HAPI_ATTROWNER_MAX;
            }

            if (colorOwner == HAPI_ATTROWNER_MAX &&
                alphaOwner == HAPI_ATTROWNER_MAX)
            {
                break;
            }

            if (colorOwner != HAPI_ATTROWNER_MAX)
            {
                markAttributeUsed(cdAttributeName.asChar());
            }
            if (alphaOwner != HAPI_ATTROWNER_MAX)
            {
                markAttributeUsed(alphaAttributeName.asChar());
            }

            // Convert to color array
            MColorArray colors(floatArray.size() / 3);
            if (colorOwner != HAPI_ATTROWNER_MAX)
            {
                colors =
                    Util::reshapeArray<3, 0, 4, 0, 3, MColorArray>(floatArray);
            }

            // If there's no color, then use a default color
            if (colorOwner == HAPI_ATTROWNER_MAX)
            {
                colorOwner = HAPI_ATTROWNER_DETAIL;
                colors.setLength(1);
                colors[0] = MColor(1.0f, 1.0f, 1.0f);
            }

            // If there's no alpha, then use a default alpha
            if (alphaOwner == HAPI_ATTROWNER_MAX)
            {
                alphaOwner = HAPI_ATTROWNER_DETAIL;
                alphaArray.resize(1);
                alphaArray[0] = 1.0f;
            }

            HAPI_AttributeOwner owner;
            if (colorOwner == HAPI_ATTROWNER_PRIM ||
                alphaOwner == HAPI_ATTROWNER_PRIM)
            {
                // If either color or alpha is prim at tributes, we always want
                // to convert to vertex attributes. Note that prim attributes
                // are not promoted to point attributes, because that would lose
                // information. Convert everything to vertex attributs instead.
                owner = HAPI_ATTROWNER_VERTEX;
            }
            else
            {
                if (colorOwner < alphaOwner)
                {
                    owner = colorOwner;
                }
                else
                {
                    owner = alphaOwner;
                }

                if (owner == HAPI_ATTROWNER_DETAIL)
                {
                    // Handle detail color or alpha as points
                    owner = HAPI_ATTROWNER_POINT;
                }
            }

            // Promte the attributes
            MColorArray promotedColors;
            Util::promoteAttributeData<3, 0, 0>(
                owner, promotedColors, colorOwner, colors, vertexArray.length(),
                &polygonCounts, &polygonConnects);

            Util::promoteAttributeData<1, 3, 0>(
                owner, promotedColors, alphaOwner, alphaArray,
                vertexArray.length(), &polygonCounts, &polygonConnects);

            MIntArray vertexList(polygonConnects.length());
            if (owner == HAPI_ATTROWNER_VERTEX)
            {
                for (unsigned int i = 0, length = polygonConnects.length();
                     i < length; ++i)
                {
                    vertexList[i] = i;
                }
            }
            else if (owner == HAPI_ATTROWNER_POINT)
            {
                vertexList = polygonConnects;
            }
            else if (owner == HAPI_ATTROWNER_PRIM)
            {
                for (unsigned int i = 0, j = 0, length = polygonCounts.length();
                     i < length; ++i)
                {
                    for (int k = 0; k < polygonCounts[i]; ++j, ++k)
                    {
                        vertexList[j] = i;
                    }
                }
            }

            // get the mapped colorset name:
            // In a mesh originating from maya, there are always mapping
            // for both alpha and RGB: empty if there's no alpha or no rgb.
            // But it has turned out to be confusing forusers trying to build
            // their own mapping - they expect to just  set up color mapping
            // if there is no alpha, so we check the color mapping first
            // and then we check the alpha
            MString colorSetName = cdAttributeName;
            if (useMappedColorset)
            {
                {
                    ArrayIterator<MStringArray> begin =
                        arrayBegin(mappedCdAttributeNames);
                    ArrayIterator<MStringArray> end =
                        arrayEnd(mappedCdAttributeNames);

                    ArrayIterator<MStringArray> iter = std::find(
                        begin, end, cdAttributeName);
                    if (iter != end)
                    {
                        colorSetName =
                            colorSetNames[std::distance(begin, iter)];
                    }
                }
            }
            if (useMappedAlpha)
            {
                {
                    ArrayIterator<MStringArray> begin =
                        arrayBegin(mappedAlphaAttributeNames);
                    ArrayIterator<MStringArray> end =
                        arrayEnd(mappedAlphaAttributeNames);
                    ArrayIterator<MStringArray> iter = std::find(
                        begin, end, alphaAttributeName);
                    if (iter != end)
                    {
                        colorSetName =
                            colorSetNames[std::distance(begin, iter)];
                    }
                }
            }
#if MAYA_API_VERSION >= 201600
            meshFn.createColorSetDataMesh(colorSetName, false, colorRep);
#else
            meshFn.createColorSetDataMesh(colorSetName);
#endif

            // If currentColorSetName is set, then use it to determine the
            // current color set. Otherwise, use the first as the current color
            // set.
            if ((currentColorSetName.length() &&
                 colorSetName == currentColorSetName) ||
                (!currentColorSetName.length() && layerIndex == 0))
            {
                meshCurrentColorSetHandle.setString(colorSetName);
            }

#if MAYA_API_VERSION >= 201600
            meshFn.setColors(promotedColors, &colorSetName, colorRep);
#else
            meshFn.setColors(promotedColors, &colorSetName);
#endif
            meshFn.assignColors(vertexList, &colorSetName);

            layerIndex++;
        }
    }
}

void
OutputGeometryPart::computeMaterial(const MTime &time,
                                    const MPlug &materialPlug,
                                    MDataBlock &data,
                                    MDataHandle &materialHandle)
{
    MObject materialObj = materialHandle.data();
    MFnIntArrayData materialDataFn(materialObj);
    if (materialObj.isNull())
    {
        materialObj = materialDataFn.create();
        materialHandle.setMObject(materialObj);

        materialObj = materialHandle.data();
        materialDataFn.setObject(materialObj);
    }

    bool are_all_the_same = true;
    std::vector<int> materialIdsBuffer(myPartInfo.faceCount);
    if (materialIdsBuffer.size())
    {
        markAttributeUsed("shop_materialpath");

        CHECK_HAPI(HAPI_GetMaterialNodeIdsOnFaces(
            Util::theHAPISession.get(), myNodeId, myPartId, &are_all_the_same,
            &materialIdsBuffer[0], 0, materialIdsBuffer.size()));
    }

    if (materialIdsBuffer.size() && materialIdsBuffer[0] == -1 &&
        are_all_the_same)
    {
        return;
    }

    MIntArray materialIds = materialDataFn.array();
    Util::convertArray(materialIds, materialIdsBuffer);
}

void
OutputGeometryPart::computeInstancer(const MTime &time,
                                     const MPlug &hasInstancerPlug,
                                     const MPlug &instancerPlug,
                                     MDataBlock &data,
                                     MDataHandle &hasInstancerHandle,
                                     MDataHandle &instanceHandle,
                                     const bool preserveScale)
{
    data.setClean(hasInstancerPlug);
    data.setClean(instancerPlug);

    bool hasInstancer = myPartInfo.type == HAPI_PARTTYPE_INSTANCER;

    if (!hasInstancerHandle.asBool() && !hasInstancer)
    {
        return;
    }

    hasInstancerHandle.setBool(hasInstancer);

    MDataHandle instancerArrayDataHandle =
        instanceHandle.child(AssetNode::outputPartInstancerArrayData);
    MDataHandle instancerPartsHandle =
        instanceHandle.child(AssetNode::outputPartInstancerParts);
    MArrayDataHandle instancerTransformHandle =
        instanceHandle.child(AssetNode::outputPartInstancerTransform);

    // outputPartInstancerArrayData
    {
        MObject instancerArrayDataObj = instancerArrayDataHandle.data();
        MFnArrayAttrsData instancerArrayDataFn(instancerArrayDataObj);
        if (instancerArrayDataObj.isNull())
        {
            instancerArrayDataObj = instancerArrayDataFn.create();
            instancerArrayDataHandle.setMObject(instancerArrayDataObj);

            instancerArrayDataObj = instancerArrayDataHandle.data();
            instancerArrayDataFn.setObject(instancerArrayDataObj);
        }

        const int &instanceCount = myPartInfo.instanceCount;

        std::vector<HAPI_Transform> transforms(instanceCount);

        CHECK_HAPI(HAPI_GetInstancerPartTransforms(
            Util::theHAPISession.get(), myNodeId, myPartId, HAPI_SRT,
            instanceCount ? &transforms.front() : NULL, 0, instanceCount));
        markAttributeUsed("P");

        MVectorArray positions  = instancerArrayDataFn.vectorArray("position");
        MVectorArray rotations  = instancerArrayDataFn.vectorArray("rotation");
        MVectorArray scales     = instancerArrayDataFn.vectorArray("scale");
        MIntArray objectIndices = instancerArrayDataFn.intArray("objectIndex");

        positions.setLength(instanceCount);
        rotations.setLength(instanceCount);
        scales.setLength(instanceCount);
        objectIndices.setLength(instanceCount);
        Util::resizeArrayDataHandle(instancerTransformHandle, instanceCount);

        for (int i = 0; i < instanceCount; i++)
        {
            const HAPI_Transform &transform = transforms[i];

            MVector p(transform.position[0], transform.position[1],
                      transform.position[2]);
            MVector r = MQuaternion(transform.rotationQuaternion[0],
                                    transform.rotationQuaternion[1],
                                    transform.rotationQuaternion[2],
                                    transform.rotationQuaternion[3])
                            .asEulerRotation()
                            .asVector();
            MVector s(
                transform.scale[0], transform.scale[1], transform.scale[2]);

            if (preserveScale)
                p *= 100.0;

            // Particle instancer
            positions[i]     = p;
            rotations[i]     = r;
            scales[i]        = s;
            objectIndices[i] = 0;

            // Transform Instancing
            CHECK_MSTATUS(instancerTransformHandle.jumpToArrayElement(i));
            MDataHandle transformHandle =
                instancerTransformHandle.outputValue();
            MDataHandle translateHandle =
                transformHandle.child(AssetNode::outputPartInstancerTranslate);
            translateHandle.set(p);
            MDataHandle rotateHandle =
                transformHandle.child(AssetNode::outputPartInstancerRotate);
            rotateHandle.set(r);
            MDataHandle scaleHandle =
                transformHandle.child(AssetNode::outputPartInstancerScale);
            scaleHandle.set(s);
        }
    }

    // outputPartInstancerParts
    {
        MObject instancedPartsObj = instancerPartsHandle.data();
        MFnIntArrayData instancerPartsFn(instancedPartsObj);
        if (instancedPartsObj.isNull())
        {
            instancedPartsObj = instancerPartsFn.create();
            instancerPartsHandle.set(instancedPartsObj);
            instancerPartsFn.setObject(instancedPartsObj);
        }

        const int &instancedPartCount = myPartInfo.instancedPartCount;

        std::vector<HAPI_PartId> partIds(instancedPartCount);
        CHECK_HAPI(HAPI_GetInstancedPartIds(
            Util::theHAPISession.get(), myNodeId, myPartId,
            instancedPartCount ? &partIds.front() : NULL, 0,
            instancedPartCount));

        MIntArray instancerParts = instancerPartsFn.array();
        Util::convertArray(instancerParts, partIds);
    }
}

void
OutputGeometryPart::computeExtraAttributes(
    const MTime &time,
    const MPlug &extraAttributesPlug,
    MDataBlock &data,
    MDataHandle &extraAttributesHandle,
    AssetNodeOptions::AccessorDataBlock &options,
    bool &needToSyncOutputs)
{
    if (myLastOutputCustomAttributes != options.outputCustomAttributes())
    {
        myLastOutputCustomAttributes = options.outputCustomAttributes();
        needToSyncOutputs            = true;
    }

    MArrayDataHandle extraAttributesArrayHandle(extraAttributesHandle);

    if (!options.outputCustomAttributes())
    {
        Util::resizeArrayDataHandle(extraAttributesArrayHandle, 0);
        return;
    }

    const HAPI_AttributeOwner attributeOwners[] = {
        HAPI_ATTROWNER_DETAIL,
        HAPI_ATTROWNER_PRIM,
        HAPI_ATTROWNER_POINT,
        HAPI_ATTROWNER_VERTEX,
    };
    const int attributeCounts[] = {
        myPartInfo.attributeCounts[HAPI_ATTROWNER_DETAIL],
        myPartInfo.attributeCounts[HAPI_ATTROWNER_PRIM],
        myPartInfo.attributeCounts[HAPI_ATTROWNER_POINT],
        myPartInfo.attributeCounts[HAPI_ATTROWNER_VERTEX],
    };

    size_t newSize = 0;

    std::array<std::vector<char>, HAPI_ATTROWNER_MAX> attributeNames;

    for (size_t i = 0; i < HAPI_ATTROWNER_MAX; i++)
    {
        const HAPI_AttributeOwner &owner = attributeOwners[i];
        const int &attributeCount        = attributeCounts[i];

        std::vector<HAPI_StringHandle> ownerAttributeNames(attributeCount);

        if (attributeCount > 0)
        {
            HAPI_GetAttributeNames(Util::theHAPISession.get(), myNodeId, myPartId,
                                owner, &ownerAttributeNames[0],
                                attributeCount);

            int stringsbuffer_len;
            HAPI_GetStringBatchSize(Util::theHAPISession.get(), &ownerAttributeNames[0],
                                    attributeCount, &stringsbuffer_len);

            attributeNames[i].resize(stringsbuffer_len);
            HAPI_GetStringBatch(
                Util::theHAPISession.get(), &attributeNames[i][0], stringsbuffer_len);
        }
    }

    for (size_t i = 0; i < HAPI_ATTROWNER_MAX; i++)
    {
        const int &attributeCount = attributeCounts[i];

        auto read_iter = attributeNames[i].begin();
        for (size_t j = 0; j < attributeCount; j++)
        {
            auto next_null = std::find(read_iter, attributeNames[i].end(), '\0');
            MString attributeName(&(*read_iter));

            if (isAttributeUsed(attributeName.asChar()) ||
                Util::startsWith(attributeName, "__"))
            {
                continue;
            }

            newSize++;

            read_iter = next_null + 1;
        }
    }

    if (extraAttributesArrayHandle.elementCount() != newSize)
    {
        Util::resizeArrayDataHandle(extraAttributesArrayHandle, newSize);
        needToSyncOutputs = true;
    }

    size_t elementIndex = 0;

    for (size_t i = 0; i < HAPI_ATTROWNER_MAX; i++)
    {
        const HAPI_AttributeOwner &owner = attributeOwners[i];
        const int &attributeCount        = attributeCounts[i];

        if (attributeCount == 0)
        {
            continue;
        }

        auto read_iter = attributeNames[i].begin();
        for (int j = 0; j < attributeCount; ++j)
        {
            auto next_null = std::find(read_iter, attributeNames[i].end(), '\0');
            MString attributeName(&(*read_iter));

            if (isAttributeUsed(attributeName.asChar()) ||
                Util::startsWith(attributeName, "__"))
            {
                continue;
            }

            MPlug extraAttributePlug =
                extraAttributesPlug.elementByLogicalIndex(elementIndex);

            CHECK_MSTATUS(
                extraAttributesArrayHandle.jumpToArrayElement(elementIndex));
            MDataHandle extraAttributeHandle =
                extraAttributesArrayHandle.outputValue();
            elementIndex++;

            if (!computeExtraAttribute(extraAttributePlug, data,
                                       extraAttributeHandle, owner,
                                       attributeName.asChar()))
            {
                DISPLAY_WARNING("Unsupported data type in attribute:\n"
                                "    ^1s",
                                attributeName);
            }

            read_iter = next_null + 1;
        }
    }

    extraAttributesArrayHandle.setAllClean();
}

void
OutputGeometryPart::computeGroups(const MTime &time,
                                  const MPlug &groupsPlug,
                                  MDataBlock &data,
                                  MDataHandle &groupsHandle,
                                  AssetNodeOptions::AccessorDataBlock &options,
                                  bool &needToSyncOutputs)
{
    if (myLastOutputGeometryGroups != options.outputGeometryGroups())
    {
        myLastOutputGeometryGroups = options.outputGeometryGroups();
        needToSyncOutputs          = true;
    }

    MArrayDataHandle groupsArrayHandle(groupsHandle);

    if (!options.outputGeometryGroups())
    {
        Util::resizeArrayDataHandle(groupsArrayHandle, 0);
        return;
    }

    MArrayDataBuilder groupsBuilder = groupsArrayHandle.builder();

    const HAPI_GroupType groupTypes[HAPI_GROUPTYPE_MAX] = {
        HAPI_GROUPTYPE_POINT,
        HAPI_GROUPTYPE_PRIM,
    };
    const int HAPI_GeoInfo::*groupCounts[HAPI_GROUPTYPE_MAX] = {
        &HAPI_GeoInfo::pointGroupCount,
        &HAPI_GeoInfo::primitiveGroupCount,
    };
    const int HAPI_PartInfo::*maxMemberCounts[HAPI_GROUPTYPE_MAX] = {
        &HAPI_PartInfo::pointCount,
        &HAPI_PartInfo::faceCount,
    };
    const MFn::Type fnTypes[HAPI_GROUPTYPE_MAX] = {
        MFn::kMeshVertComponent,
        MFn::kMeshPolygonComponent,
    };

    size_t groupElementIndex = 0;
    for (int i = 0; i < 2; i++)
    {
        const HAPI_GroupType &groupType           = groupTypes[i];
        const int HAPI_GeoInfo::*&groupCount      = groupCounts[i];
        const int HAPI_PartInfo::*&maxMemberCount = maxMemberCounts[i];
        const MFn::Type fnType                    = fnTypes[i];

        if (myGeoInfo.*groupCount == 0 || myPartInfo.*maxMemberCount == 0)
        {
            continue;
        }

        std::vector<HAPI_StringHandle> groupNames(myGeoInfo.*groupCount);

        HAPI_GetGroupNames(Util::theHAPISession.get(), myNodeId, groupType,
                           &groupNames[0], myGeoInfo.*groupCount);

        std::vector<int> groupMembership(myPartInfo.*maxMemberCount);
        for (size_t j = 0; j < groupNames.size(); j++)
        {
            MString groupName = Util::HAPIString(groupNames[j]);

            if (groupName == HAPI_UNGROUPED_GROUP_NAME)
            {
                continue;
            }

            // Get the group membership first, because we want to skip the group
            // completely if it's empty.
            HAPI_GetGroupMembership(Util::theHAPISession.get(), myNodeId,
                                    myPartId, groupType, groupName.asChar(),
                                    NULL, &groupMembership[0], 0,
                                    groupMembership.size());

            MIntArray groupMembers(groupMembership.size());

            size_t groupMembersCount = 0;
            for (size_t k = 0; k < groupMembership.size(); k++)
            {
                if (groupMembership[k])
                {
                    groupMembers[groupMembersCount] = k;
                    groupMembersCount++;
                }
            }
            groupMembers.setLength(groupMembersCount);

            // If this is an empty group, skip it. Otherwise, during sync, Maya
            // would spend time assigning nothing to sets.  This is significant
            // when using splitGeosByGroup with many groups, because there would
            // be many empty groups for each part.
            if (!groupMembers.length())
            {
                continue;
            }

            MDataHandle groupHandle =
                groupsBuilder.addElement(groupElementIndex);
            groupElementIndex++;

            MDataHandle groupNameHandle =
                groupHandle.child(AssetNode::outputPartGroupName);
            MDataHandle groupTypeHandle =
                groupHandle.child(AssetNode::outputPartGroupType);
            MDataHandle groupMembersHandle =
                groupHandle.child(AssetNode::outputPartGroupMembers);

            MObject groupMembersObj = groupMembersHandle.data();
            MFnIntArrayData groupMembersDataFn(groupMembersObj);
            if (groupMembersObj.isNull())
            {
                groupMembersObj = groupMembersDataFn.create();
                groupMembersHandle.setMObject(groupMembersObj);

                groupMembersObj = groupMembersHandle.data();
                groupMembersDataFn.setObject(groupMembersObj);
            }

            groupMembersDataFn.set(groupMembers);

            groupNameHandle.setString(groupName);
            groupTypeHandle.setInt(fnType);
        }
    }

    for (size_t i = groupsBuilder.elementCount(); i-- > groupElementIndex;)
    {
        groupsBuilder.removeElement(i);
    }

    groupsArrayHandle.set(groupsBuilder);
}

void
OutputGeometryPart::markAttributeUsed(const std::string &attributeName)
{
    std::vector<std::string>::iterator pos = std::lower_bound(
        myAttributesUsed.begin(), myAttributesUsed.end(), attributeName);

    // Attribute already exist.
    if (pos != myAttributesUsed.end() && *pos == attributeName)
    {
        return;
    }

    myAttributesUsed.insert(pos, attributeName);
}

bool
OutputGeometryPart::isAttributeUsed(const std::string &attributeName)
{
    return std::binary_search(
        myAttributesUsed.begin(), myAttributesUsed.end(), attributeName);
}

void
OutputGeometryPart::clearAttributesUsed()
{
    myAttributesUsed.clear();
}
