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
    update();
}


Object::ObjectType
InstancerObject::type()
{
    return Object::OBJECT_TYPE_INSTANCER;
}


void
InstancerObject::update()
{
    Object::update();

    // clear the arrays
    instancedObjectNames.clear();
    instancedObjectIndices.clear();
    uniqueInstObjNames.clear();

    if (objectInfo.objectToInstanceId >= 0)
    {
        instancedObjectIndices = MIntArray(geoInfo.pointCount, 0);
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

    cerr << "update instancer: " << endl;
    cerr << instancedObjectIndices << endl;
}



// test functions
bool
InstancerObject::isVisible()
{
    return objectInfo.isVisible;
}
// end test functions


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
InstancerObject::compute(const MPlug& plug, MDataBlock& data)
{
    update();

    //printAttributes(HAPI_ATTROWNER_VERTEX);
    //printAttributes(HAPI_ATTROWNER_POINT);
    //printAttributes(HAPI_ATTROWNER_PRIM);
    //printAttributes(HAPI_ATTROWNER_DETAIL);

    cerr << "InstancerObject: plug " << plug.name() << endl;
    cerr << "objectToInstanceId: " << objectInfo.objectToInstanceId << endl;
    MPlug instancerDataPlug = plug.child(AssetNodeAttributes::instancerData);
    MPlug instancedObjectNamesPlug = plug.child(AssetNodeAttributes::instancedObjectNames);

    MDataHandle instHandle = data.outputValue(instancerDataPlug);
    MFnArrayAttrsData fnAAD;
    MObject instOutput = fnAAD.create();
    MVectorArray positions = fnAAD.vectorArray("position");
    MVectorArray rotations = fnAAD.vectorArray("rotation");
    MVectorArray scales = fnAAD.vectorArray("scale");
    MIntArray objectIndices = fnAAD.intArray("objectIndex");

    int size = geoInfo.pointCount;
    HAPI_Transform instTransforms[size];
    HAPI_GetInstanceTransforms(assetId, objectInfo.id, 0, 5, instTransforms, 0, size);


    //cerr << "instancedObjectNames: " << instancedObjectNames << endl;

    cerr << "get instance transforms" << endl;
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

    instHandle.set(instOutput);

    if (objectInfo.objectToInstanceId >= 0)
    {
        // instancing a single object
        Object* objToInstance = objectControl->findObjectById(objectInfo.objectToInstanceId);
        MString name = objToInstance->getName();

        MPlug instObjNamesElemPlug = instancedObjectNamesPlug.elementByLogicalIndex(0);
        MDataHandle instObjNamesElemHandle = data.outputValue(instObjNamesElemPlug);
        instObjNamesElemHandle.set(name);
    } else 
    {
        // instancing multiple objects
        for (int i=0; i<uniqueInstObjNames.length(); i++)
        {
            MPlug instObjNamesElemPlug = instancedObjectNamesPlug.elementByLogicalIndex(i);
            MDataHandle instObjNamesElemHandle = data.outputValue(instObjNamesElemPlug);
            instObjNamesElemHandle.set(uniqueInstObjNames[i]);
        }
    }


    data.setClean(plug);
    data.setClean(instancerDataPlug);

    return MS::kSuccess;

}

