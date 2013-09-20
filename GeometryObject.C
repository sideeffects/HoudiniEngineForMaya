#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MFnArrayAttrsData.h>

#include "Asset.h"
#include "AssetNode.h"
#include "GeometryObject.h"
#include "GeometryPart.h"
#include "util.h"

GeometryObject::GeometryObject() {}


GeometryObject::GeometryObject(int assetId, int objectId)
    :Object(assetId, objectId)
{
}


void
GeometryObject::init()
{
    // Do a full update
    Object::init();

    int partCount = myGeoInfo.partCount;
    myParts = new GeometryPart[partCount];
    
    for (int i=0; i<partCount; i++)
    {
        myParts[i] = GeometryPart( myAssetId, myObjectId, 0, i, myObjectInfo, myGeoInfo, myObjectControl );
    }
    
}


GeometryObject::~GeometryObject()
{
    delete[] myParts;
}


Object::ObjectType
GeometryObject::type()
{
    return Object::OBJECT_TYPE_GEOMETRY;
}


void
GeometryObject::update()
{
    int oldPartCount = myGeoInfo.partCount;
    Object::update();
    int partCount = myGeoInfo.partCount;

    // if partCount changed, we clear out the array and make a new one.
    if (oldPartCount != partCount)
    {
        delete[] myParts;
        myParts = new GeometryPart[partCount];
        for (int i=0; i<partCount; i++)
        {
	    //TODO: remove the hard coding of the geo id to 0
            myParts[i] = GeometryPart( myAssetId, myObjectId, 0, i, myObjectInfo, myGeoInfo, myObjectControl);
        }
    }
    else
    {
        for (int i=0; i<partCount; i++)
        {
            myParts[i].setGeoInfo( myGeoInfo );
        }
    }

}


MStatus
GeometryObject::compute(MDataHandle& handle)
{

    return MS::kSuccess;
}


MStatus
GeometryObject::computeParts(MArrayDataBuilder* builder, int* index)
{
    update();

    // TODO: this may be temporary until HAPI supports actual groups
    if ( myNeverBuilt || myObjectInfo.haveGeosChanged)
    {
        MStatus stat;

        // Don't output mesh for invisible geos
        if (! myObjectInfo.isVisible && ! myIsInstanced)
            return MS::kFailure;

        // TODO: right now assume one geo
        for (int i=0; i< myGeoInfo.partCount; i++)
        {
            MDataHandle h = builder->addElement(*index);
            stat = myParts[i].compute(h);

            if (MS::kSuccess == stat)
            {
                if ( myNeverBuilt || myObjectInfo.hasTransformChanged)
                {
                    MDataHandle t = h.child(AssetNode::outputObjectTransform);
                    updateTransform(t);
                }
                (*index)++;
            }
        }

        myNeverBuilt = false;
    }
    else
    {
        for (int i=0; i< myGeoInfo.partCount; i++)
        {
            if ( myParts[i].hasMesh() )
            {
                MDataHandle h = builder->addElement(*index);
                if ( myNeverBuilt || myObjectInfo.hasTransformChanged)
                {
                    MDataHandle t = h.child(AssetNode::outputObjectTransform);
                    updateTransform(t);
                }
                (*index)++;
            }
        }
    }

    return MS::kSuccess;
}


MStatus
GeometryObject::setClean(MPlug& plug, MDataBlock& data)
{
    data.setClean(plug);
    data.setClean(plug.child(AssetNode::outputObjectMesh));
    data.setClean(plug.child(AssetNode::outputObjectName));

    MPlug transformPlug = plug.child(AssetNode::outputObjectTransform);
    data.setClean(transformPlug.child(AssetNode::outputObjectTranslate));
    data.setClean(transformPlug.child(AssetNode::outputObjectRotate));
    data.setClean(transformPlug.child(AssetNode::outputObjectScale));

    MPlug materialPlug = plug.child(AssetNode::outputObjectMaterial);
    data.setClean(materialPlug.child(AssetNode::outputObjectMaterialExists));
    data.setClean(materialPlug.child(AssetNode::outputObjectAmbientColor));
    data.setClean(materialPlug.child(AssetNode::outputObjectDiffuseColor));
    data.setClean(materialPlug.child(AssetNode::outputObjectSpecularColor));
    data.setClean(materialPlug.child(AssetNode::outputObjectTexturePath));

    return MS::kSuccess;
}



void GeometryObject::updateTransform(MDataHandle& handle)
{
    MDataHandle translateHandle = handle.child(AssetNode::outputObjectTranslate);
    MDataHandle rotateHandle = handle.child(AssetNode::outputObjectRotate);
    MDataHandle scaleHandle = handle.child(AssetNode::outputObjectScale);

    HAPI_GetObjectTransforms( myAssetId, 5, &myTransformInfo, myObjectId, 1 );
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


