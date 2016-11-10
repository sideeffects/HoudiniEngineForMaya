#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MDataHandle.h>
#include <maya/MPointArray.h>
#include <maya/MFnNurbsCurve.h>

#include "InputCurveNode.h"
#include "MayaTypeID.h"
#include "util.h"

#include <cassert>

MString InputCurveNode::typeName = "houdiniInputCurve";

MTypeId InputCurveNode::typeId = MayaTypeID_HoudiniInputCurveNode;

MObject InputCurveNode::inputCurve;
MObject InputCurveNode::outputNodeId;

void*
InputCurveNode::creator()
{
    InputCurveNode* ret = new InputCurveNode();
    return ret;
}

MStatus
InputCurveNode::initialize()
{
    MFnNumericAttribute nAttr;
    MFnTypedAttribute tAttr;
    InputCurveNode::inputCurve = tAttr.create( "inputCurve", "inputCurve",
                                    MFnData::kNurbsCurve );
    tAttr.setDisconnectBehavior(MFnAttribute::kDelete);
    tAttr.setArray(true);

    addAttribute(InputCurveNode::inputCurve);

    InputCurveNode::outputNodeId  = nAttr.create(
            "outputNodeId", "outputNodeId",
            MFnNumericData::kInt
            );
    nAttr.setStorable(false);
    nAttr.setWritable(false);

    addAttribute(InputCurveNode::outputNodeId);

    attributeAffects(InputCurveNode::inputCurve, InputCurveNode::outputNodeId);

    return MS::kSuccess;
}

InputCurveNode::InputCurveNode()
    : myNodeId(-1)
{
}

InputCurveNode::~InputCurveNode()
{
    if ( myNodeId > 0 )
    {
        CHECK_HAPI(HAPI_DeleteNode(
                    Util::theHAPISession.get(),
                    myNodeId
                    ));
    }
}

MStatus
InputCurveNode::compute(const MPlug& plug, MDataBlock& data)
{
    MStatus status;

    if ( plug != InputCurveNode::outputNodeId )
    {
        return MPxNode::compute(plug, data);
    }

    if ( myNodeId < 0 )
    {
        Util::PythonInterpreterLock pythonInterpreterLock;

        CHECK_HAPI(HAPI_CreateInputNode(
                    Util::theHAPISession.get(),
                    &myNodeId,
                    NULL
                    ));

        if ( !Util::statusCheckLoop() )
        {
            DISPLAY_ERROR(
                MString("Unexpected error when creating input asset.")
            );
        }

        // Meta data
        MDataHandle metaDataHandle = data.outputValue(InputCurveNode::outputNodeId);
        metaDataHandle.setInt(myNodeId);
    }

    MPlug inputCurveArrayPlug(thisMObject(), InputCurveNode::inputCurve);

    const int nInputCurves = inputCurveArrayPlug.numElements();
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
    MStringArray name;

    for ( int iCurve = 0; iCurve < nInputCurves; ++iCurve )
    {
        MPlug inputCurvePlug = inputCurveArrayPlug.elementByPhysicalIndex(iCurve);

        MDataHandle curveHandle = data.inputValue(inputCurvePlug);
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
            cvP.push_back((float) cv.x);
            cvP.push_back((float) cv.y);
            cvP.push_back((float) cv.z);

            if ( !curveInfo.isRational )
            {
                if ( cv.w != 1.0 )
                {
                    curveInfo.isRational = true;
                    cvPw.resize( curveInfo.vertexCount, 1.0f );
                    cvPw.push_back((float) cv.w);
                }
            }
            else
            {
                cvPw.push_back((float) cv.w);
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

        name.append(
                Util::getNodeName(Util::plugSource(inputCurvePlug))
                );
    }

    curveInfo.hasKnots = curveInfo.knotCount > 0;

    HAPI_PartInfo partInfo = HAPI_PartInfo_Create();
    partInfo.vertexCount = partInfo.pointCount = curveInfo.vertexCount;
    partInfo.faceCount = curveInfo.curveCount;
    partInfo.type = HAPI_PARTTYPE_CURVE;
    CHECK_HAPI(HAPI_SetPartInfo(
                Util::theHAPISession.get(),
                myNodeId, 0,
                &partInfo
                ));

    CHECK_HAPI(HAPI_SetCurveInfo(
                Util::theHAPISession.get(),
                myNodeId, 0,
                &curveInfo
                ));
    CHECK_HAPI(HAPI_SetCurveCounts(
                Util::theHAPISession.get(),
                myNodeId, 0,
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
                Util::theHAPISession.get(),
                myNodeId, 0,
                "P",
                &attrInfo
                ));

    CHECK_HAPI(HAPI_SetAttributeFloatData(
                Util::theHAPISession.get(),
                myNodeId, 0,
                "P",
                &attrInfo,
                &cvP.front(),
                0, static_cast<int>(cvP.size() / 3)
                ));

    if ( curveInfo.isRational )
    {
        attrInfo.tupleSize = 1;
        CHECK_HAPI(HAPI_AddAttribute(
                    Util::theHAPISession.get(),
                    myNodeId, 0,
                    "Pw",
                    &attrInfo
                    ));

        CHECK_HAPI(HAPI_SetAttributeFloatData(
                    Util::theHAPISession.get(),
                    myNodeId, 0,
                    "Pw",
                    &attrInfo,
                    &cvPw.front(),
                    0, static_cast<int>(cvPw.size())
                    ));
    }

    if ( curveInfo.hasKnots )
    {
        CHECK_HAPI(HAPI_SetCurveKnots(
                    Util::theHAPISession.get(),
                    myNodeId, 0,
                    &knots.front(),
                    0, static_cast<int>(knots.size())
                    ));
    }

    CHECK_HAPI(HAPI_CommitGeo(
                Util::theHAPISession.get(),
                myNodeId
                ));

    data.setClean(plug);
    return MStatus::kSuccess;
}
