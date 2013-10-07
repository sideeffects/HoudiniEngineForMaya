#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MFnArrayAttrsData.h>

#include "Asset.h"
#include "AssetNode.h"
#include "InstancerObject.h"
#include "util.h"


InstancerObject::InstancerObject() {}


InstancerObject::InstancerObject(int assetId, int objectId)
    :Object(assetId, objectId)
{
    //update();
}


void
InstancerObject::init()
{
    Object::init();
}


InstancerObject::~InstancerObject() {}


Object::ObjectType
InstancerObject::type()
{
    return Object::OBJECT_TYPE_INSTANCER;
}


MStringArray
InstancerObject::getAttributeStringData(HAPI_AttributeOwner owner, MString name)
{
    return Util::getAttributeStringData( myAssetId, myObjectId, 0, 0, owner, name );
}


void
InstancerObject::update()
{
    Object::update();

    if ( myNeverBuilt || myGeoInfo.hasGeoChanged)
    {
        
	// clear the arrays
        myInstancedObjectNames.clear();
        myInstancedObjectIndices.clear();
        myUniqueInstObjNames.clear();
	myHoudiniInstanceAttribute.clear();
	myHoudiniNameAttribute.clear();

        try
        {

	    if( myGeoInfo.partCount <= 0 )
		return;

            HAPI_Result hstat = HAPI_RESULT_SUCCESS;
            hstat = HAPI_GetPartInfo( myAssetId, myObjectId, 0, 0, &myPartInfo);
            Util::checkHAPIStatus(hstat);
        }
        catch (HAPIError& e)
        {
            cerr << e.what() << endl;
        }
        

        if ( myObjectInfo.objectToInstanceId >= 0)
        {
            myInstancedObjectIndices = MIntArray( myPartInfo.pointCount, 0);
            return;
        }

        // fill array of size pointCount of instanced names
        MStringArray instanceAttrs = getAttributeStringData(HAPI_ATTROWNER_POINT, "instance");
        for (unsigned int i=0; i<instanceAttrs.length(); i++)
        {
            MStringArray splitObjName;
            instanceAttrs[i].split('/', splitObjName);
            myInstancedObjectNames.append(splitObjName[splitObjName.length()-1]);
	    myHoudiniInstanceAttribute.append( instanceAttrs[i] );

        }

	MStringArray nameAttrs = getAttributeStringData(HAPI_ATTROWNER_POINT, "name");
	for (unsigned int ii = 0; ii < nameAttrs.length(); ii++)
        {                        
	    myHoudiniNameAttribute.append( nameAttrs[ ii ] );
        }


        // get a list of unique instanced names, and compute the object indices that would
        // be passed to Maya instancer
        for ( unsigned int i = 0; i< myInstancedObjectNames.length(); ++i )
        {
            bool duplicate = false;
            unsigned int j = 0;
            for ( ; j< myUniqueInstObjNames.length(); ++j )
            {
                if ( myUniqueInstObjNames[ j ] == myInstancedObjectNames[ i ] )
                {
                    duplicate = true;
                    break;
                }
            }
            if ( !duplicate )
                myUniqueInstObjNames.append( myInstancedObjectNames[ i ] );
            myInstancedObjectIndices.append( (int) j );
        }
    }
}


MIntArray
InstancerObject::getInstancedObjIds()
{
    MIntArray ret;
    if ( myObjectInfo.objectToInstanceId >= 0 )
        ret.append( myObjectInfo.objectToInstanceId );
    return ret;
}


MStringArray
InstancerObject::getUniqueInstObjNames()
{
    return myUniqueInstObjNames;
}


MStatus
//InstancerObject::compute(const MPlug& plug, MDataBlock& data)
InstancerObject::compute(MDataHandle& handle)
{
    update();

    if( myGeoInfo.partCount <= 0 )
	return MS::kFailure;

    if ( myNeverBuilt || myGeoInfo.hasGeoChanged )
    {
        MDataHandle instancerDataHandle = handle.child(AssetNode::outputInstancerData);
        MArrayDataHandle instancedObjectNamesHandle = handle.child(AssetNode::outputInstancedObjectNames);
	MArrayDataHandle houdiniInstanceAttributeHandle = handle.child(AssetNode::outputHoudiniInstanceAttribute);
	MArrayDataHandle houdiniNameAttributeHandle = handle.child(AssetNode::outputHoudiniNameAttribute);
	MArrayDataHandle instanceTransformHandle = handle.child(AssetNode::outputInstanceTransform);	

        //MDataHandle instHandle = data.outputValue(instancerDataPlug);
        MFnArrayAttrsData fnAAD;
        MObject instOutput = fnAAD.create();
        MVectorArray positions = fnAAD.vectorArray("position");
        MVectorArray rotations = fnAAD.vectorArray("rotation");
        MVectorArray scales = fnAAD.vectorArray("scale");
        MIntArray objectIndices = fnAAD.intArray("objectIndex");

        int size = myPartInfo.pointCount;
        HAPI_Transform * instTransforms = new HAPI_Transform[size];
        HAPI_GetInstanceTransforms( myAssetId, myObjectInfo.id, 0, 5, instTransforms, 0, size );

	MArrayDataBuilder houdiniInstanceAttributeBuilder = houdiniInstanceAttributeHandle.builder();
	MArrayDataBuilder houdiniNameAttributeBuilder = houdiniNameAttributeHandle.builder();
	MArrayDataBuilder instanceTransformBuilder = instanceTransformHandle.builder();

        for (int j=0; j<size; j++)
        {
            HAPI_Transform it = instTransforms[j];
            MVector p(it.position[0], it.position[1], it.position[2]);
            MVector r = MQuaternion(it.rotationQuaternion[0],
                    it.rotationQuaternion[1], it.rotationQuaternion[2],
                    it.rotationQuaternion[3]).asEulerRotation().asVector();
            MVector s(it.scale[0], it.scale[1], it.scale[2]);

            int objIndex = myInstancedObjectIndices[j];

            positions.append(p);
            rotations.append(r);
            scales.append(s);
            objectIndices.append(objIndex);

	    if( myHoudiniInstanceAttribute.length() == size )
	    {
		MDataHandle intanceAttributeHandle = houdiniInstanceAttributeBuilder.addElement( j );
		intanceAttributeHandle.set( myHoudiniInstanceAttribute[ j ] );
	    }

	    if( myHoudiniNameAttribute.length() == size )
	    {
		MDataHandle nameAttributeHandle = houdiniNameAttributeBuilder.addElement( j );
		nameAttributeHandle.set( myHoudiniNameAttribute[ j ] );
	    }

	    MDataHandle transformHandle = instanceTransformBuilder.addElement( j );
	    MDataHandle translateHandle = transformHandle.child( AssetNode::outputInstanceTranslate );
	    MDataHandle rotateHandle = transformHandle.child( AssetNode::outputInstanceRotate );
	    MDataHandle scaleHandle = transformHandle.child( AssetNode::outputInstanceScale );

	    MDataHandle txHandle = translateHandle.child( AssetNode::outputInstanceTranslateX );
	    txHandle.set( p.x );
	    MDataHandle tyHandle = translateHandle.child( AssetNode::outputInstanceTranslateY );
	    tyHandle.set( p.y );
	    MDataHandle tzHandle = translateHandle.child( AssetNode::outputInstanceTranslateZ );
	    tzHandle.set( p.z );
	    
	    MDataHandle rxHandle = rotateHandle.child( AssetNode::outputInstanceRotateX );
	    rxHandle.set( r.x );
	    MDataHandle ryHandle = rotateHandle.child( AssetNode::outputInstanceRotateY );
	    ryHandle.set( r.y );
	    MDataHandle rzHandle = rotateHandle.child( AssetNode::outputInstanceRotateZ );
	    rzHandle.set( r.z );
	    
	    MDataHandle sxHandle = scaleHandle.child( AssetNode::outputInstanceScaleX );
	    sxHandle.set( s.x );
	    MDataHandle syHandle = scaleHandle.child( AssetNode::outputInstanceScaleY );
	    syHandle.set( s.y );
	    MDataHandle szHandle = scaleHandle.child( AssetNode::outputInstanceScaleZ );
	    szHandle.set( s.z );
	    
        }

	houdiniInstanceAttributeHandle.set( houdiniInstanceAttributeBuilder );
	houdiniInstanceAttributeHandle.setAllClean();

	houdiniNameAttributeHandle.set( houdiniNameAttributeBuilder );
	houdiniNameAttributeHandle.setAllClean();

	instanceTransformHandle.set( instanceTransformBuilder );
	instanceTransformHandle.setAllClean();


	delete[] instTransforms;

        instancerDataHandle.set(instOutput);

        MArrayDataBuilder builder = instancedObjectNamesHandle.builder();
        if ( myObjectInfo.objectToInstanceId >= 0 )
        {
            // instancing a single object
            Object* objToInstance = myObjectControl->findObjectById( myObjectInfo.objectToInstanceId );
            MString name = objToInstance->getName();

            MDataHandle h = builder.addElement(0);
            h.set(name);
     
        } else 
        {
            // instancing multiple objects
            for ( unsigned int i=0; i< myUniqueInstObjNames.length(); i++)
            {
                MDataHandle h = builder.addElement(i);
                h.set( myUniqueInstObjNames[i]);
            }

            // clean up extra elements
            int builderSizeCheck = builder.elementCount();
            if (builderSizeCheck > (int) myUniqueInstObjNames.length())
            {
                for (int i= myUniqueInstObjNames.length(); i<builderSizeCheck; i++)
                {
                    builder.removeElement(i);
                }
            }
        }
        instancedObjectNamesHandle.set(builder);
        instancedObjectNamesHandle.setAllClean();


        handle.setClean();
        instancerDataHandle.setClean();
        //data.setClean(plug);
        //data.setClean(instancerDataPlug);


        myNeverBuilt = false;
    }

    return MS::kSuccess;

}


MStatus
InstancerObject::setClean(MPlug& plug, MDataBlock& data)
{
    return MS::kSuccess;
}
