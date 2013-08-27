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
    HAPI_AttributeInfo attr_info;
    attr_info.exists = false;
    attr_info.owner = owner;
    HAPI_GetAttributeInfo( myAssetId, myObjectId, 0, 0, name.asChar(), &attr_info);

    MStringArray ret;
    if (!attr_info.exists)
        return ret;

    int size = attr_info.count * attr_info.tupleSize;
    int * data = new int[size];
    // zero the array
    for (int j=0; j<size; j++){
        data[j] = 0;
    }
    int status = HAPI_GetAttributeStrData( myAssetId, myObjectId, 0, 0, name.asChar(),
            &attr_info, data, 0, attr_info.count);

    for (int j=0; j<size; j++){
        ret.append(Util::getString(data[j]));
    }

    delete[] data;

    return ret;
}


void
InstancerObject::update()
{
    Object::update();

    if ( myNeverBuilt || myGeoInfo.hasGeoChanged)
    {
        // TODO: assume only one part for instancers
        //try
        //{
            HAPI_Result hstat = HAPI_RESULT_SUCCESS;
            hstat = HAPI_GetPartInfo( myAssetId, myObjectId, 0, 0, &myPartInfo);
            Util::checkHAPIStatus(hstat);
        //}
        //catch (HAPIError& e)
        //{
            //cerr << e.what() << endl;
        //}

        // clear the arrays
        myInstancedObjectNames.clear();
        myInstancedObjectIndices.clear();
        myUniqueInstObjNames.clear();

        if ( myObjectInfo.objectToInstanceId >= 0)
        {
            myInstancedObjectIndices = MIntArray( myPartInfo.pointCount, 0);
            return;
        }

        // fill array of size pointCount of instanced names
        MStringArray fullObjNames = getAttributeStringData(HAPI_ATTROWNER_POINT, "instance");
        for (unsigned int i=0; i<fullObjNames.length(); i++)
        {
            MStringArray splitObjName;
            fullObjNames[i].split('/', splitObjName);
            myInstancedObjectNames.append(splitObjName[splitObjName.length()-1]);
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

    if ( myNeverBuilt || myGeoInfo.hasGeoChanged )
    {
        MDataHandle instancerDataHandle = handle.child(AssetNode::instancerData);
        MArrayDataHandle instancedObjectNamesHandle = handle.child(AssetNode::instancedObjectNames);

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
        }

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
