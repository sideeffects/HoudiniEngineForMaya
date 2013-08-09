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
        MString partName = getName() + "_partShape" + (i+1);
        //cerr << "&&&&&&&&&&&& partName: " << partName << endl;	
        myParts[i] = GeometryPart( myAssetId, myObjectId, 0, i, partName, myGeoInfo, myObjectControl );
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
            MString partName = getName() + "_partShape" + (i+1);
            cerr << "&&&&&&&&&&&& partName: " << partName << endl;
	    //TODO: remove the hard coding of the geo id to 0
            myParts[i] = GeometryPart( myAssetId, myObjectId, 0, i, partName, myGeoInfo, myObjectControl);
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
                    MDataHandle t = h.child(AssetNode::transform);
                    updateTransform(t);
                }
                cerr << *index << " index++: " + myParts[i].myPartName << endl;
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
                    MDataHandle t = h.child(AssetNode::transform);
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
    data.setClean(plug.child(AssetNode::mesh));
    data.setClean(plug.child(AssetNode::objectName));

    MPlug transformPlug = plug.child(AssetNode::transform);
    data.setClean(transformPlug.child(AssetNode::translateAttr));
    data.setClean(transformPlug.child(AssetNode::rotateAttr));
    data.setClean(transformPlug.child(AssetNode::scaleAttr));

    MPlug materialPlug = plug.child(AssetNode::material);
    data.setClean(materialPlug.child(AssetNode::materialExists));
    data.setClean(materialPlug.child(AssetNode::ambientAttr));
    data.setClean(materialPlug.child(AssetNode::diffuseAttr));
    data.setClean(materialPlug.child(AssetNode::specularAttr));
    data.setClean(materialPlug.child(AssetNode::texturePath));

    return MS::kSuccess;
}



void GeometryObject::updateTransform(MDataHandle& handle)
{
    cerr << "Update transform: " << getName() << endl;
    MDataHandle translateHandle = handle.child(AssetNode::translateAttr);
    MDataHandle rotateHandle = handle.child(AssetNode::rotateAttr);
    MDataHandle scaleHandle = handle.child(AssetNode::scaleAttr);

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


