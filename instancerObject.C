#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MFnArrayAttrsData.h>

#include "instancerObject.h"
#include "asset.h"
#include "util.h"
#include "common.h"


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
    HAPI_GetAttributeInfo(assetId, objectId, 0, 0, name.asChar(), &attr_info);

    MStringArray ret;
    if (!attr_info.exists)
        return ret;

    int size = attr_info.count * attr_info.tupleSize;
    int * data = new int[size];
    // zero the array
    for (int j=0; j<size; j++){
        data[j] = 0;
    }
    int status = HAPI_GetAttributeStrData(assetId, objectId, 0, 0, name.asChar(),
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

    if (neverBuilt || geoInfo.hasGeoChanged)
    {
        // TODO: assume only one part for instancers
        //try
        //{
            HAPI_Result hstat = HAPI_RESULT_SUCCESS;
            hstat = HAPI_GetPartInfo(assetId, objectId, 0, 0, &partInfo);
            Util::checkHAPIStatus(hstat);
        //}
        //catch (HAPIError& e)
        //{
            //cerr << e.what() << endl;
        //}

        // clear the arrays
        instancedObjectNames.clear();
        instancedObjectIndices.clear();
        uniqueInstObjNames.clear();

        if (objectInfo.objectToInstanceId >= 0)
        {
            instancedObjectIndices = MIntArray(partInfo.pointCount, 0);
            return;
        }

        // fill array of size pointCount of instanced names
        MStringArray fullObjNames = getAttributeStringData(HAPI_ATTROWNER_POINT, "instance");
        for (int i=0; i<fullObjNames.length(); i++)
        {
            MStringArray splitObjName;
            fullObjNames[i].split('/', splitObjName);
            instancedObjectNames.append(splitObjName[splitObjName.length()-1]);
        }

        // get a list of unique instanced names, and compute the object indices that would
        // be passed to Maya instancer
        for (int i=0; i<instancedObjectNames.length(); i++)
        {
            bool duplicate = false;
            int j;
            for (j=0; j<uniqueInstObjNames.length(); j++)
            {
                if (uniqueInstObjNames[j] == instancedObjectNames[i])
                {
                    duplicate = true;
                    break;
                }
            }
            if (!duplicate)
                uniqueInstObjNames.append(instancedObjectNames[i]);
            instancedObjectIndices.append(j);
        }
    }

    //cerr << "update instancer: " << endl;
    //cerr << instancedObjectIndices << endl;
}


MIntArray
InstancerObject::getInstancedObjIds()
{
    MIntArray ret;
    if (objectInfo.objectToInstanceId >= 0)
        ret.append(objectInfo.objectToInstanceId);
    return ret;
}


MStringArray
InstancerObject::getUniqueInstObjNames()
{
    return uniqueInstObjNames;
}


MStatus
//InstancerObject::compute(const MPlug& plug, MDataBlock& data)
InstancerObject::compute(MDataHandle& handle)
{
    update();

    if (neverBuilt || geoInfo.hasGeoChanged)
    {
        MDataHandle instancerDataHandle = handle.child(AssetNodeAttributes::instancerData);
        MArrayDataHandle instancedObjectNamesHandle = handle.child(AssetNodeAttributes::instancedObjectNames);

        //MDataHandle instHandle = data.outputValue(instancerDataPlug);
        MFnArrayAttrsData fnAAD;
        MObject instOutput = fnAAD.create();
        MVectorArray positions = fnAAD.vectorArray("position");
        MVectorArray rotations = fnAAD.vectorArray("rotation");
        MVectorArray scales = fnAAD.vectorArray("scale");
        MIntArray objectIndices = fnAAD.intArray("objectIndex");

        int size = partInfo.pointCount;
        HAPI_Transform * instTransforms = new HAPI_Transform[size];
        HAPI_GetInstanceTransforms(assetId, objectInfo.id, 0, 5, instTransforms, 0, size);


        //cerr << "instancedObjectNames: " << instancedObjectNames << endl;

        //cerr << "get instance transforms" << endl;
        for (int j=0; j<size; j++)
        {
            HAPI_Transform it = instTransforms[j];
            MVector p(it.position[0], it.position[1], it.position[2]);
            MVector r = MQuaternion(it.rotationQuaternion[0],
                    it.rotationQuaternion[1], it.rotationQuaternion[2],
                    it.rotationQuaternion[3]).asEulerRotation().asVector();
            MVector s(it.scale[0], it.scale[1], it.scale[2]);

            int objIndex = instancedObjectIndices[j];

            positions.append(p);
            rotations.append(r);
            scales.append(s);
            objectIndices.append(objIndex);
        }

	delete[] instTransforms;

        instancerDataHandle.set(instOutput);

        MArrayDataBuilder builder = instancedObjectNamesHandle.builder();
        if (objectInfo.objectToInstanceId >= 0)
        {
            // instancing a single object
            Object* objToInstance = objectControl->findObjectById(objectInfo.objectToInstanceId);
            MString name = objToInstance->getName();

            MDataHandle h = builder.addElement(0);
            h.set(name);
     
        } else 
        {
            // instancing multiple objects
            for (int i=0; i<uniqueInstObjNames.length(); i++)
            {
                MDataHandle h = builder.addElement(i);
                h.set(uniqueInstObjNames[i]);
            }

            // clean up extra elements
            int builderSizeCheck = builder.elementCount();
            if (builderSizeCheck > uniqueInstObjNames.length())
            {
                for (int i=uniqueInstObjNames.length(); i<builderSizeCheck; i++)
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


        neverBuilt = false;
    }

    return MS::kSuccess;

}


MStatus
InstancerObject::setClean(MPlug& plug, MDataBlock& data)
{
    return MS::kSuccess;
}
