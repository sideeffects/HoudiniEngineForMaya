#include "OutputPartInstancerNode.h"

#include <maya/MFnTypedAttribute.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MFnPointArrayData.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MFnIntArrayData.h>

#include "MayaTypeID.h"
#include "hapiutil.h"
#include "util.h"


MString OutputPartInstancerNode::typeName( "houdiniOutputPartInstancer" );
MTypeId OutputPartInstancerNode::typeId( MayaTypeID_HoudiniOutputPartInstancerNode );

MObject OutputPartInstancerNode::pointData;
MObject OutputPartInstancerNode::storablePositions;
MObject OutputPartInstancerNode::storableRotations;
MObject OutputPartInstancerNode::storableScales;
MObject OutputPartInstancerNode::storableObjectIndices;

void*
OutputPartInstancerNode::creator()
{
    return new OutputPartInstancerNode();
}

MStatus
OutputPartInstancerNode::initialize()
{
    MFnTypedAttribute tAttr;

    // pointData will be connected between the asset node and instancer.inputPoints

    OutputPartInstancerNode::pointData = tAttr.create(
        "pointData", "pd",
        MFnData::kDynArrayAttrs
    );
    tAttr.setReadable( true );
    tAttr.setWritable( true );
    tAttr.setConnectable( true );
    tAttr.setHidden( false );
    addAttribute( OutputPartInstancerNode::pointData );

    // The storable attrs for the parts of pointData that we want to persist
    //

    OutputPartInstancerNode::storablePositions = tAttr.create(
        "storablePositions", "storablePositions",
        MFnData::kPointArray
    );
    tAttr.setStorable( true );
    tAttr.setWritable( true );
    tAttr.setReadable( true );
    tAttr.setHidden( true );
    addAttribute( OutputPartInstancerNode::storablePositions );

    OutputPartInstancerNode::storableRotations = tAttr.create(
        "storableRotations", "storableRotations",
        MFnData::kPointArray
    );
    tAttr.setStorable( true );
    tAttr.setWritable( true );
    tAttr.setReadable( true );
    tAttr.setHidden( true );
    addAttribute( OutputPartInstancerNode::storableRotations );

    OutputPartInstancerNode::storableScales = tAttr.create(
        "storableScales", "storableScales",
        MFnData::kPointArray
    );
    tAttr.setStorable( true );
    tAttr.setWritable( true );
    tAttr.setReadable( true );
    tAttr.setHidden( true );
    addAttribute( OutputPartInstancerNode::storableScales );

    OutputPartInstancerNode::storableObjectIndices = tAttr.create(
        "storableObjectIndices", "storableObjectIndices",
        MFnData::kIntArray
    );
    tAttr.setStorable( true );
    tAttr.setWritable( true );
    tAttr.setReadable( true );
    tAttr.setHidden( true );
    addAttribute( OutputPartInstancerNode::storableObjectIndices );

    attributeAffects( OutputPartInstancerNode::pointData, OutputPartInstancerNode::storablePositions );
    attributeAffects( OutputPartInstancerNode::pointData, OutputPartInstancerNode::storableRotations );
    attributeAffects( OutputPartInstancerNode::pointData, OutputPartInstancerNode::storableScales );
    attributeAffects( OutputPartInstancerNode::pointData, OutputPartInstancerNode::storableObjectIndices );

    return MStatus::kSuccess;
}

OutputPartInstancerNode::OutputPartInstancerNode()
{

}

OutputPartInstancerNode::~OutputPartInstancerNode()
{

}

MStatus
OutputPartInstancerNode::compute(
    const MPlug &plug,
    MDataBlock &dataBlock
)
{
    MStatus status;

    if ( plug == OutputPartInstancerNode::pointData )
    {
        if ( !plug.isDestination( &status ) )
        {
            // in this case we should use the storeable data
            MDataHandle pointDataDataHandle =
                dataBlock.outputValue( plug, &status );
            CHECK_MSTATUS_AND_RETURN_IT( status );
            MObject pointDataDataObj = pointDataDataHandle.data();
            MFnArrayAttrsData pointDataFnArrayAttrs( pointDataDataObj, &status );
            if ( pointDataDataObj.isNull() )
            {
                pointDataDataObj = pointDataFnArrayAttrs.create( &status );
                CHECK_MSTATUS_AND_RETURN_IT( status );
                status = pointDataDataHandle.setMObject( pointDataDataObj );
                CHECK_MSTATUS_AND_RETURN_IT( status );

                pointDataDataObj = pointDataDataHandle.data();
                status = pointDataFnArrayAttrs.setObject( pointDataDataObj );
                CHECK_MSTATUS_AND_RETURN_IT( status );
            }

            // copy vector arrays
            MString attr[3] = { "position", "rotation", "scale" };
            MObject attrObjs[3] = { 
                OutputPartInstancerNode::storablePositions , 
                OutputPartInstancerNode::storableRotations, 
                OutputPartInstancerNode::storableScales };
            for ( int i = 0; i < 3; ++i )
            {
                MDataHandle storableHandle = dataBlock.inputValue( attrObjs[ i ], &status );
                CHECK_MSTATUS_AND_RETURN_IT( status );
                MObject storableObj = storableHandle.data();
                MFnPointArrayData storableFn( storableObj, &status );
                CHECK_MSTATUS_AND_RETURN_IT( status );
                MVectorArray vectorArray = pointDataFnArrayAttrs.vectorArray( attr[ i ], &status );
                CHECK_MSTATUS_AND_RETURN_IT( status );
                // convert points->vectors
                int nPoints = storableFn.length();
                vectorArray.setLength( nPoints );
                for ( int j = 0; j < nPoints; ++j )
                {
                    vectorArray.set( MVector( storableFn[ j ] ), j);
                }
            }

            // copy int array
            MDataHandle storableHandle = dataBlock.inputValue( OutputPartInstancerNode::storableObjectIndices, &status );
            CHECK_MSTATUS_AND_RETURN_IT( status );
            MObject storableObj = storableHandle.data();
            MFnIntArrayData storableFn( storableObj, &status );
            CHECK_MSTATUS_AND_RETURN_IT( status );
            MIntArray intArray = pointDataFnArrayAttrs.intArray( "objectIndex", &status );
            CHECK_MSTATUS_AND_RETURN_IT( status );
            status = storableFn.copyTo( intArray );

            pointDataDataHandle.setClean();
        }
    }
    else if ( plug == OutputPartInstancerNode::storablePositions ||
              plug == OutputPartInstancerNode::storableRotations ||
              plug == OutputPartInstancerNode::storableScales ||
              plug == OutputPartInstancerNode::storableObjectIndices )
    {
        MDataHandle pointDataDataHandle =
            dataBlock.inputValue( OutputPartInstancerNode::pointData, &status );
        CHECK_MSTATUS_AND_RETURN_IT( status );

        MObject pointDataDataObj = pointDataDataHandle.data();
        MFnArrayAttrsData pointDataFnArrayAttrs( pointDataDataObj, &status );
        CHECK_MSTATUS_AND_RETURN_IT( status );

        if ( plug == OutputPartInstancerNode::storableObjectIndices )
        {
            MFnArrayAttrsData::Type checkType( MFnArrayAttrsData::kIntArray );
            bool exists = pointDataFnArrayAttrs.checkArrayExist( "objectIndex", checkType );
            if ( exists )
            {
                MIntArray arrayData = pointDataFnArrayAttrs.getIntData( "objectIndex", &status );
                CHECK_MSTATUS_AND_RETURN_IT( status );
                MDataHandle storableHandle = dataBlock.outputValue( plug, &status );
                CHECK_MSTATUS_AND_RETURN_IT( status );
                MObject storableObj = storableHandle.data();
                MFnIntArrayData storableFn( storableObj, &status );
                if ( storableObj.isNull() )
                {
                    storableObj = storableFn.create( &status );
                    CHECK_MSTATUS_AND_RETURN_IT( status );
                    status = storableHandle.setMObject( storableObj );
                    CHECK_MSTATUS_AND_RETURN_IT( status );

                    storableObj = storableHandle.data();
                    status = storableFn.setObject( storableObj );
                    CHECK_MSTATUS_AND_RETURN_IT( status );
                }
                status = storableFn.set( arrayData );
                CHECK_MSTATUS_AND_RETURN_IT( status );
                storableHandle.setClean();
            }
        }
        else
        {
            MString outAttrName;
            if ( plug == OutputPartInstancerNode::storablePositions )
            {
                outAttrName = "position";
            }
            else if ( plug == OutputPartInstancerNode::storableRotations )
            {
                outAttrName = "rotation";
            }
            else
                outAttrName = "scale";

            MFnArrayAttrsData::Type checkType( MFnArrayAttrsData::kVectorArray );
            bool exists = pointDataFnArrayAttrs.checkArrayExist( outAttrName, checkType );
            if ( exists )
            {
                MVectorArray arrayData = pointDataFnArrayAttrs.getVectorData( outAttrName, &status );
                CHECK_MSTATUS_AND_RETURN_IT( status );
                MDataHandle storableHandle = dataBlock.outputValue( plug, &status );
                CHECK_MSTATUS_AND_RETURN_IT( status );
                MObject storableObj = storableHandle.data();
                MFnPointArrayData storableFn( storableObj, &status );
                if ( storableObj.isNull() )
                {
                    storableObj = storableFn.create( &status );
                    CHECK_MSTATUS_AND_RETURN_IT( status );
                    status = storableHandle.setMObject( storableObj );
                    CHECK_MSTATUS_AND_RETURN_IT( status );

                    storableObj = storableHandle.data();
                    status = storableFn.setObject( storableObj );
                    CHECK_MSTATUS_AND_RETURN_IT( status );
                }
                // Convert vectors -> points
                unsigned int numElems = arrayData.length();
                MPointArray tempPointArray;
                tempPointArray.setLength( numElems );
                for ( unsigned int i = 0; i < numElems; ++i )
                {
                    status = tempPointArray.set( MPoint( arrayData[ i ] ), i );
                }
                status = storableFn.set( tempPointArray );
                CHECK_MSTATUS_AND_RETURN_IT( status );
                storableHandle.setClean();
            }
        }
        return status;
    }
    return MPxNode::compute( plug, dataBlock );
}
