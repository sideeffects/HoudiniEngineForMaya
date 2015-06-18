#include <maya/MFnTypedAttribute.h>
#include <maya/MDataHandle.h>
#include <maya/MPointArray.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnIntArrayData.h>

#include "CurveMeshInputNode.h"
#include "MayaTypeID.h"
#include "util.h"

#include <cassert>

MString CurveMeshInputNode::typeName = "houdiniCurveMeshInput";

MTypeId CurveMeshInputNode::typeId = MayaTypeID_HoudiniCurveMeshInput;

MObject
    CurveMeshInputNode::theInputCurves,
    CurveMeshInputNode::theOutputObjectMetaData;

void*
CurveMeshInputNode::creator()
{
    CurveMeshInputNode* ret = new CurveMeshInputNode();
    return ret;
}

MStatus
CurveMeshInputNode::initialize()
{
    MFnTypedAttribute tAttr;
    theInputCurves = tAttr.create( "inputCurves", "incs",
                                    MFnData::kNurbsCurve );
    tAttr.setArray(true);

    addAttribute(theInputCurves);

    theOutputObjectMetaData = tAttr.create(
        "outputObjectMetaData", "oomd",
        MFnData::kIntArray
    );

    tAttr.setStorable(false);
    tAttr.setWritable(false);

    addAttribute(theOutputObjectMetaData);

    attributeAffects(theInputCurves, theOutputObjectMetaData);

    return MS::kSuccess;
}

CurveMeshInputNode::CurveMeshInputNode()
    : myAssetId(-1)
{
}

CurveMeshInputNode::~CurveMeshInputNode()
{
    if ( myAssetId > 0 )
    {
        CHECK_HAPI(HAPI_DestroyAsset(
                    myAssetId
                    ));
    }
}

MStatus
CurveMeshInputNode::compute(const MPlug& plug, MDataBlock& data)
{
    MStatus status;

    if ( plug != theOutputObjectMetaData )
    {
        return MPxNode::compute(plug, data);
    }

    if ( myAssetId < 0 )
    {
        CHECK_HAPI(HAPI_CreateInputAsset(
                    &myAssetId,
                    NULL
                    ));
        if ( !Util::statusCheckLoop() )
        {
            DISPLAY_ERROR(
                MString("Unexpected error when creating input asset.")
            );
        }

        MDataHandle metaDataHandle = data.outputValue(theOutputObjectMetaData);

        // Meta data
        MFnIntArrayData ffIAD;
        MIntArray metaDataArray;
        metaDataArray.append(myAssetId);
        metaDataArray.append(0); // object ID
        MObject newMetaData = ffIAD.create(metaDataArray);
        metaDataHandle.set(newMetaData);
    }

    MArrayDataHandle inputCurves(data.inputArrayValue(theInputCurves, &status));
    CHECK_MSTATUS_AND_RETURN_IT(status);

    const int nInputCurves = inputCurves.elementCount();
    if ( nInputCurves <= 0 )
    {
        data.setClean(plug);
        return MStatus::kSuccess;
    }

    HAPI_CurveInfo curveInfo = HAPI_CurveInfo_Create();
    curveInfo.curveType = HAPI_CURVETYPE_NURBS;
    curveInfo.isRational = false;

    std::vector<float>  cvP, cvPw;
    std::vector<int>    cvCounts;
    std::vector<float>  knots;
    std::vector<int>    orders;

    for ( int iCurve = 0; iCurve < nInputCurves; ++iCurve )
    {
        MDataHandle curveHandle = inputCurves.inputValue();
        MObject curveObject = curveHandle.asNurbsCurve();
        MFnNurbsCurve fnCurve( curveObject );

        const bool isPeriodic = fnCurve.form() == MFnNurbsCurve::kPeriodic;
        if ( iCurve == 0 )
        {
            curveInfo.isPeriodic = isPeriodic;
        }
        else if ( isPeriodic != curveInfo.isPeriodic )
        {
            DISPLAY_ERROR(
                MString("Curve has a non-matching periodicity, skipping")
            );
            continue;
        }

        const int order = fnCurve.degree() + 1;
        if ( iCurve == 0 )
        {
            curveInfo.order = order;
        }
        else if ( curveInfo.order == HAPI_CURVE_ORDER_VARYING )
        {
            orders.push_back( order );
        }
        else if ( order != curveInfo.order )
        {
            orders.resize( curveInfo.curveCount, curveInfo.order );
            curveInfo.order = HAPI_CURVE_ORDER_VARYING;
            orders.push_back( order );
        }

        MPointArray cvArray;
        CHECK_MSTATUS_AND_RETURN_IT( fnCurve.getCVs( cvArray ) );

        unsigned int nCVs = cvArray.length();

        // Maya provides fnCurve.degree() more cvs in its data definition
        // than Houdini for periodic curves -- but they are conincident
        // with the first ones. Houdini ignores them, so we don't
        // output them.
        if ( curveInfo.isPeriodic && static_cast<int>(nCVs) > fnCurve.degree() )
        {
            nCVs -= fnCurve.degree();
        }

        cvCounts.push_back(nCVs);

        for ( unsigned int iCV = 0; iCV < nCVs; ++iCV, ++curveInfo.vertexCount )
        {
            const MPoint& cv = cvArray[iCV];
            cvP.push_back(cv.x);
            cvP.push_back(cv.y);
            cvP.push_back(cv.z);

            if ( !curveInfo.isRational )
            {
                if ( cv.w != 1.0 )
                {
                    curveInfo.isRational = true;
                    cvPw.resize( curveInfo.vertexCount, 1.0f );
                    cvPw.push_back(cv.w);
                }
            }
            else
            {
                cvPw.push_back(cv.w);
            }
        }

        MDoubleArray knotsArray;
        CHECK_MSTATUS_AND_RETURN_IT( fnCurve.getKnots( knotsArray ) );

        if ( knotsArray.length() > 0 )
        {
            // Maya doesn't provide the first and last knots
            curveInfo.knotCount += knotsArray.length() + 2;

            knots.push_back( static_cast<float>(knotsArray[0]) );

            for ( unsigned int iKnot = 0; iKnot < knotsArray.length(); ++iKnot )
            {
                knots.push_back( static_cast<float>(knotsArray[iKnot]) );
            }

            knots.push_back(
                static_cast<float>( knotsArray[knotsArray.length() - 1] )
            );
        }

        ++curveInfo.curveCount;

        if ( !inputCurves.next() )
        {
            break;
        }
    }

    curveInfo.hasKnots = curveInfo.knotCount > 0;

    HAPI_PartInfo partInfo = HAPI_PartInfo_Create();
    partInfo.vertexCount = partInfo.pointCount = curveInfo.vertexCount;
    partInfo.faceCount = curveInfo.curveCount;
    partInfo.isCurve = true;
    CHECK_HAPI(HAPI_SetPartInfo(
                myAssetId, 0, 0,
                &partInfo
                ));

    CHECK_HAPI(HAPI_SetCurveInfo(
                myAssetId, 0, 0, 0,
                &curveInfo
                ));
    CHECK_HAPI(HAPI_SetCurveCounts(
                myAssetId, 0, 0, 0,
                &cvCounts.front(),
                0, cvCounts.size()
                ));

    HAPI_AttributeInfo attrInfo = HAPI_AttributeInfo_Create();
    attrInfo.count = partInfo.pointCount;
    attrInfo.tupleSize = 3; // 3 floats per CV (x, y, z)
    attrInfo.exists = true;
    attrInfo.owner = HAPI_ATTROWNER_POINT;
    attrInfo.storage = HAPI_STORAGETYPE_FLOAT;

    CHECK_HAPI(HAPI_AddAttribute(
                myAssetId, 0, 0,
                "P",
                &attrInfo
                ));

    CHECK_HAPI(HAPI_SetAttributeFloatData(
                myAssetId, 0, 0,
                "P",
                &attrInfo,
                &cvP.front(),
                0, static_cast<int>(cvP.size() / 3)
                ));

    if ( curveInfo.isRational )
    {
        attrInfo.tupleSize = 1;
        CHECK_HAPI(HAPI_AddAttribute(
                    myAssetId, 0, 0,
                    "Pw",
                    &attrInfo
                    ));

        CHECK_HAPI(HAPI_SetAttributeFloatData(
                    myAssetId, 0, 0,
                    "Pw",
                    &attrInfo,
                    &cvPw.front(),
                    0, static_cast<int>(cvPw.size())
                    ));
    }

    if ( curveInfo.hasKnots )
    {
        CHECK_HAPI(HAPI_SetCurveKnots(
                    myAssetId, 0, 0, 0,
                    &knots.front(),
                    0, static_cast<int>(knots.size())
                    ));
    }

    CHECK_HAPI(HAPI_CommitGeo(
                myAssetId, 0, 0
                ));

    data.setClean(plug);
    return MStatus::kSuccess;
}
