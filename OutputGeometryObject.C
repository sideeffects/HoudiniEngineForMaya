
#include <maya/MFnIntArrayData.h>

#include "Asset.h"
#include "AssetNode.h"
#include "OutputGeometryObject.h"
#include "util.h"
#include "OutputGeometry.h"



OutputGeometryObject::OutputGeometryObject(
        int assetId,
        int objectId,
        Asset* objectControl
        ) :
    OutputObject(
            assetId,
            objectId,
            objectControl
          )
{
    int geoCount = myObjectInfo.geoCount;
    myGeos.reserve( geoCount );
    myGeos.clear();

    for ( int ii = 0; ii < geoCount; ii++ )
    {	
        OutputGeometry * geo = new OutputGeometry( myAssetId, myObjectId, ii, this );
        myGeos.push_back( geo );
    }
        
}


OutputGeometryObject::~OutputGeometryObject()
{
    for ( int ii = 0; ii < myObjectInfo.geoCount; ii++ )
    {
        delete myGeos[ ii ];
    }    

}


OutputObject::ObjectType
OutputGeometryObject::type()
{
    return OutputObject::OBJECT_TYPE_GEOMETRY;
}


MStatus
OutputGeometryObject::compute(
        MDataHandle& objectHandle,
        bool &needToSyncOutputs
        )
{
    MDataHandle metaDataHandle = objectHandle.child(AssetNode::outputObjectMetaData);

    // Meta data
    MFnIntArrayData ffIAD;
    MIntArray metaDataArray;
    metaDataArray.append( myAssetId );
    metaDataArray.append( myObjectId );
    MObject newMetaData = ffIAD.create(metaDataArray);
    metaDataHandle.set(newMetaData);

    for ( int ii = 0; ii < myObjectInfo.geoCount; ii++ )
    {
        myGeos[ ii ]->update();
    }    

    MStatus stat = MS::kSuccess;
    if ( myNeverBuilt || myObjectInfo.haveGeosChanged )
    {
        MDataHandle geosHandle = objectHandle.child( AssetNode::outputGeos );
        MArrayDataHandle geoArrayHandle( geosHandle );
        MArrayDataBuilder geosBuilder = geoArrayHandle.builder();
        if(geosBuilder.elementCount() != myObjectInfo.geoCount)
        {
            needToSyncOutputs = true;
        }
                
        for ( int ii = 0; ii < myObjectInfo.geoCount; ii++ )
        {
            MDataHandle geoHandle = geosBuilder.addElement( ii );
            stat = myGeos[ii]->compute(geoHandle, needToSyncOutputs);
            CHECK_MSTATUS_AND_RETURN_IT(stat);
        }

        geoArrayHandle.set( geosBuilder );

        if( stat == MS::kSuccess )
        {
            MDataHandle transformHandle = objectHandle.child( AssetNode::outputObjectTransform );
            updateTransform( transformHandle );
        }

        myNeverBuilt = false;
    }

    return stat;
}


MStatus
OutputGeometryObject::setClean(MPlug& plug, MDataBlock& data)
{
    data.setClean(plug);

    MPlug transformPlug = plug.child(AssetNode::outputObjectTransform);
    data.setClean(transformPlug.child(AssetNode::outputObjectTranslate));
    data.setClean(transformPlug.child(AssetNode::outputObjectRotate));
    data.setClean(transformPlug.child(AssetNode::outputObjectScale));
    data.setClean(plug.child(AssetNode::outputObjectMetaData));

    MPlug geosPlug = plug.child( AssetNode::outputGeos );
    for( int ii = 0; ii < myObjectInfo.geoCount; ii++ )
    {
        MPlug geoPlug = geosPlug[ ii ];
        myGeos[ ii ]->setClean( geoPlug, data );
    }

    return MS::kSuccess;
}



void OutputGeometryObject::updateTransform(MDataHandle& handle)
{
    MDataHandle translateHandle = handle.child(AssetNode::outputObjectTranslate);
    MDataHandle rotateHandle = handle.child(AssetNode::outputObjectRotate);
    MDataHandle scaleHandle = handle.child(AssetNode::outputObjectScale);

    HAPI_GetObjectTransforms( myAssetId, HAPI_SRT, &myTransformInfo, myObjectId, 1 );
    //transformInfo = objectControl->getTransformInfo(objectId);

    // convert to euler angle
    MEulerRotation r = MQuaternion(myTransformInfo.rotationQuaternion[0],
            myTransformInfo.rotationQuaternion[1], myTransformInfo.rotationQuaternion[2],
            myTransformInfo.rotationQuaternion[3]).asEulerRotation();

    translateHandle.set3Double((double)( myTransformInfo.position[0]), (double) myTransformInfo.position[1], (double) myTransformInfo.position[2]);
    rotateHandle.set3Double((double)r[0], (double)r[1], (double)r[2]);
    scaleHandle.set3Double((double) myTransformInfo.scale[0], (double) myTransformInfo.scale[1], (double) myTransformInfo.scale[2]);

    translateHandle.setClean();
    rotateHandle.setClean();
    scaleHandle.setClean();
    handle.setClean();

}


