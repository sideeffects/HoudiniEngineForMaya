#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MFnIntArrayData.h>

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
    MDataHandle metaDataHandle = handle.child(AssetNode::outputObjectMetaData);

    // Meta data
    MFnIntArrayData ffIAD;
    MIntArray metaDataArray;
    metaDataArray.append( myAssetId );
    metaDataArray.append( myObjectId );
    MObject newMetaData = ffIAD.create(metaDataArray);
    metaDataHandle.set(newMetaData);

    return MS::kSuccess;
}


MStatus
GeometryObject::computeParts(MDataHandle& obj, MArrayDataBuilder* builder)
{
    update();

    MStatus stat;
    // TODO: this may be temporary until HAPI supports actual groups
    if ( myNeverBuilt || myObjectInfo.haveGeosChanged)
    {

        // TODO: right now assume one geo
        for (int i=0; i< myGeoInfo.partCount; i++)
        {
            MDataHandle h = builder->addElement(i);
            stat = myParts[i].compute(h);

            if (MS::kSuccess == stat)
            {
                if ( myNeverBuilt || myObjectInfo.hasTransformChanged)
                {
                    MDataHandle t = obj.child(AssetNode::outputObjectTransform);
                    updateTransform(t);
                }
            }
        }

        myNeverBuilt = false;
    }
    else
    {
	// TODO: !
        for (int i=0; i< myGeoInfo.partCount; i++)
        {
            if ( myParts[i].hasMesh() )
            {
                MDataHandle h = builder->addElement(i);
		stat = myParts[i].compute(h);
                if ( myNeverBuilt || myObjectInfo.hasTransformChanged)
                {
                    MDataHandle t = obj.child(AssetNode::outputObjectTransform);
                    updateTransform(t);
                }
            }
        }
    }

    int partBuilderSizeCheck = builder->elementCount();
    if (partBuilderSizeCheck > myGeoInfo.partCount)
    {
        for (int i = myGeoInfo.partCount; i< partBuilderSizeCheck; i++)
        {
            try
            {
                stat = builder->removeElement(i);
                Util::checkMayaStatus(stat);
            } catch (MayaError& e)
            {
                cerr << e.what() << endl;
            }
        }
    }

    return MS::kSuccess;
}


MStatus
GeometryObject::setClean(MPlug& plug, MDataBlock& data)
{
    data.setClean(plug);

    MPlug transformPlug = plug.child(AssetNode::outputObjectTransform);
    data.setClean(transformPlug.child(AssetNode::outputObjectTranslate));
    data.setClean(transformPlug.child(AssetNode::outputObjectRotate));
    data.setClean(transformPlug.child(AssetNode::outputObjectScale));
    data.setClean(plug.child(AssetNode::outputObjectMetaData));

    MPlug partsPlug = plug.child(AssetNode::outputParts);
    for (int i=0; i<myGeoInfo.partCount; i++)
    {
	MPlug partPlug = partsPlug[i];
	data.setClean(partPlug.child(AssetNode::outputPartName));
	data.setClean(partPlug.child(AssetNode::outputPartMesh));

	data.setClean(partPlug.child(AssetNode::outputPartMaterial));
	data.setClean(partPlug.child(AssetNode::outputPartMaterialExists));
	data.setClean(partPlug.child(AssetNode::outputPartTexturePath));
	data.setClean(partPlug.child(AssetNode::outputPartAmbientColor));
	data.setClean(partPlug.child(AssetNode::outputPartDiffuseColor));
	data.setClean(partPlug.child(AssetNode::outputPartSpecularColor));
	data.setClean(partPlug.child(AssetNode::outputPartAlphaColor));
    }

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


