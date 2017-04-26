#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MArrayDataBuilder.h>

#include "Asset.h"
#include "AssetNode.h"
#include "OutputInstancerObject.h"
#include "hapiutil.h"
#include "util.h"

OutputInstancerObject::OutputInstancerObject(
        HAPI_NodeId nodeId
        ) :
    OutputObject(
            nodeId
          ),
    myGeoInfo(HAPI_GeoInfo_Create()),
    myLastSopCookCount(0)
{
    //update();
}

OutputInstancerObject::~OutputInstancerObject() {}

OutputObject::ObjectType
OutputInstancerObject::type()
{
    return OutputObject::OBJECT_TYPE_INSTANCER;
}

void
OutputInstancerObject::update()
{
    HAPI_Result hapiResult;

    hapiResult = HAPI_GetNodeInfo(
            Util::theHAPISession.get(),
            myNodeId, &myNodeInfo
            );
    CHECK_HAPI(hapiResult);

    hapiResult = HAPI_GetObjectInfo(
            Util::theHAPISession.get(),
            myNodeId, &myObjectInfo
            );
    CHECK_HAPI(hapiResult);

    // Get the SOP nodes
    int geoCount;
    hapiResult = HAPI_ComposeChildNodeList(
            Util::theHAPISession.get(),
            myNodeId,
            HAPI_NODETYPE_SOP,
            HAPI_NODEFLAGS_DISPLAY,
            false,
            &geoCount
            );
    CHECK_HAPI(hapiResult);

    std::vector<HAPI_NodeId> geoNodeIds(geoCount);
    if(geoCount > 0)
    {
        hapiResult = HAPI_GetComposedChildNodeList(
                Util::theHAPISession.get(),
                myNodeId,
                &geoNodeIds.front(),
                geoCount
                );
        CHECK_HAPI(hapiResult);

        hapiResult = HAPI_GetNodeInfo(
                Util::theHAPISession.get(),
                geoNodeIds[0],
                &mySopNodeInfo
                );
        CHECK_HAPI(hapiResult);

        hapiResult = HAPI_GetGeoInfo(
                Util::theHAPISession.get(),
                geoNodeIds[0],
                &myGeoInfo
                );
        CHECK_HAPI(hapiResult);
    }

    if(mySopNodeInfo.totalCookCount > myLastSopCookCount)
    {
        // clear the arrays
        myInstancedObjectNames.clear();
        myInstancedObjectIndices.clear();
        myUniqueInstObjNames.clear();
        myHoudiniInstanceAttribute.clear();
        myHoudiniNameAttribute.clear();

        hapiResult = HAPI_GetPartInfo(
                Util::theHAPISession.get(),
                mySopNodeInfo.id, 0,
                &myPartInfo
                );
        CHECK_HAPI(hapiResult);

        if(myObjectInfo.objectToInstanceId >= 0)
        {
            myInstancedObjectIndices = MIntArray(myPartInfo.pointCount, 0);
            return;
        }

        // fill array of size pointCount of instanced names
        HAPI_AttributeInfo attrInfo;
        MStringArray instanceAttrs;
        hapiGetPointAttribute(mySopNodeInfo.id, 0, "instance", attrInfo, instanceAttrs);
        for(unsigned int i=0; i<instanceAttrs.length(); i++)
        {
            MStringArray splitObjName;
            instanceAttrs[i].split('/', splitObjName);
            myInstancedObjectNames.append(splitObjName[splitObjName.length()-1]);
            myHoudiniInstanceAttribute.append(instanceAttrs[i]);
        }

        MStringArray nameAttrs;
        hapiGetPointAttribute(mySopNodeInfo.id, 0, "name", attrInfo, nameAttrs);
        for(unsigned int ii = 0; ii < nameAttrs.length(); ii++)
        {
            myHoudiniNameAttribute.append(nameAttrs[ii]);
        }

        // get a list of unique instanced names, and compute the object indices that would
        // be passed to Maya instancer
        for(unsigned int i = 0; i< myInstancedObjectNames.length(); ++i)
        {
            bool duplicate = false;
            unsigned int j = 0;
            for(; j< myUniqueInstObjNames.length(); ++j)
            {
                if(myUniqueInstObjNames[j] == myInstancedObjectNames[i])
                {
                    duplicate = true;
                    break;
                }
            }
            if(!duplicate)
                myUniqueInstObjNames.append(myInstancedObjectNames[i]);
            myInstancedObjectIndices.append((int) j);
        }

        // Workaround a crash where we can't determine the object to instance.
        if(!myInstancedObjectNames.length())
        {
            myInstancedObjectIndices = MIntArray(myPartInfo.pointCount, -1);
        }
    }
}

MIntArray
OutputInstancerObject::getInstancedObjIds()
{
    MIntArray ret;
    if(myObjectInfo.objectToInstanceId >= 0)
        ret.append(myObjectInfo.objectToInstanceId);
    return ret;
}

MStringArray
OutputInstancerObject::getUniqueInstObjNames()
{
    return myUniqueInstObjNames;
}

MStatus
//OutputInstancerObject::compute(const MPlug& plug, MDataBlock& data)
OutputInstancerObject::compute(
        const MTime &time,
        MDataBlock& data,
        MDataHandle& handle,
        bool &needToSyncOutputs
        )
{
    update();

    if(myGeoInfo.partCount <= 0)
        return MS::kFailure;

    if(mySopNodeInfo.totalCookCount > myLastSopCookCount)
    {
        MDataHandle useInstancerHandle = data.inputValue(AssetNode::useInstancerNode);

        bool useInstancer = useInstancerHandle.asBool();

        MDataHandle instancerDataHandle = handle.child(AssetNode::outputInstancerData);
        MArrayDataHandle instancedObjectNamesHandle = handle.child(AssetNode::outputInstancedObjectNames);
        MArrayDataHandle houdiniInstanceAttributeHandle = handle.child(AssetNode::outputHoudiniInstanceAttribute);
        MArrayDataHandle houdiniNameAttributeHandle = handle.child(AssetNode::outputHoudiniNameAttribute);
        MArrayDataHandle instanceTransformHandle = handle.child(AssetNode::outputInstanceTransform);

        //MDataHandle instHandle = data.outputValue(instancerDataPlug);
        MObject arrayDataObj = instancerDataHandle.data();
        MFnArrayAttrsData arrayDataFn(arrayDataObj);
        if(arrayDataObj.isNull())
        {
            arrayDataObj = arrayDataFn.create();
            instancerDataHandle.set(arrayDataObj);

            arrayDataObj = instancerDataHandle.data();
            arrayDataFn.setObject(arrayDataObj);
        }

        MVectorArray positions = arrayDataFn.vectorArray("position");
        MVectorArray rotations = arrayDataFn.vectorArray("rotation");
        MVectorArray scales = arrayDataFn.vectorArray("scale");
        MIntArray objectIndices = arrayDataFn.intArray("objectIndex");

        unsigned int size = myPartInfo.pointCount;
        HAPI_Transform * instTransforms = new HAPI_Transform[size];
        CHECK_HAPI(HAPI_GetInstanceTransforms(
                Util::theHAPISession.get(),
                mySopNodeInfo.id,
                HAPI_SRT,
                instTransforms,
                0, size
                ));

        MArrayDataBuilder houdiniInstanceAttributeBuilder = houdiniInstanceAttributeHandle.builder();
        MArrayDataBuilder houdiniNameAttributeBuilder = houdiniNameAttributeHandle.builder();
        MArrayDataBuilder instanceTransformBuilder = instanceTransformHandle.builder();

        if(positions.length() != size && !useInstancer)
        {
            needToSyncOutputs = true;
        }

        positions.setLength(size);
        rotations.setLength(size);
        scales.setLength(size);
        objectIndices.setLength(size);

        for(unsigned int j=0; j<size; j++)
        {
            HAPI_Transform it = instTransforms[j];
            MVector p(it.position[0], it.position[1], it.position[2]);
            MVector r = MQuaternion(it.rotationQuaternion[0],
                    it.rotationQuaternion[1], it.rotationQuaternion[2],
                    it.rotationQuaternion[3]).asEulerRotation().asVector();
            MVector s(it.scale[0], it.scale[1], it.scale[2]);

            int objIndex = myInstancedObjectIndices[j];

            positions[j] = p;
            rotations[j] = r;
            scales[j] = s;
            objectIndices[j] = objIndex;

            if(myHoudiniInstanceAttribute.length() == size)
            {
                MDataHandle intanceAttributeHandle = houdiniInstanceAttributeBuilder.addElement(j);
                intanceAttributeHandle.set(myHoudiniInstanceAttribute[j]);
            }

            if(myHoudiniNameAttribute.length() == size)
            {
                MDataHandle nameAttributeHandle = houdiniNameAttributeBuilder.addElement(j);
                nameAttributeHandle.set(myHoudiniNameAttribute[j]);
            }

            MDataHandle transformHandle = instanceTransformBuilder.addElement(j);
            MDataHandle translateHandle = transformHandle.child(AssetNode::outputInstanceTranslate);
            MDataHandle rotateHandle = transformHandle.child(AssetNode::outputInstanceRotate);
            MDataHandle scaleHandle = transformHandle.child(AssetNode::outputInstanceScale);

            MDataHandle txHandle = translateHandle.child(AssetNode::outputInstanceTranslateX);
            txHandle.set(p.x);
            MDataHandle tyHandle = translateHandle.child(AssetNode::outputInstanceTranslateY);
            tyHandle.set(p.y);
            MDataHandle tzHandle = translateHandle.child(AssetNode::outputInstanceTranslateZ);
            tzHandle.set(p.z);

            MDataHandle rxHandle = rotateHandle.child(AssetNode::outputInstanceRotateX);
            rxHandle.set(r.x);
            MDataHandle ryHandle = rotateHandle.child(AssetNode::outputInstanceRotateY);
            ryHandle.set(r.y);
            MDataHandle rzHandle = rotateHandle.child(AssetNode::outputInstanceRotateZ);
            rzHandle.set(r.z);

            MDataHandle sxHandle = scaleHandle.child(AssetNode::outputInstanceScaleX);
            sxHandle.set(s.x);
            MDataHandle syHandle = scaleHandle.child(AssetNode::outputInstanceScaleY);
            syHandle.set(s.y);
            MDataHandle szHandle = scaleHandle.child(AssetNode::outputInstanceScaleZ);
            szHandle.set(s.z);
        }

        // clean up extra elements
        for(unsigned int i = houdiniInstanceAttributeBuilder.elementCount();
                i-- > size;)
        {
            houdiniInstanceAttributeBuilder.removeElement(i);
        }

        // clean up extra elements
        for(unsigned int i = houdiniNameAttributeBuilder.elementCount();
                i-- > size;)
        {
            houdiniNameAttributeBuilder.removeElement(i);
        }

        // clean up extra elements
        for(unsigned int i = instanceTransformBuilder.elementCount();
                i-- > size;)
        {
            instanceTransformBuilder.removeElement(i);
        }

        houdiniInstanceAttributeHandle.set(houdiniInstanceAttributeBuilder);
        houdiniInstanceAttributeHandle.setAllClean();

        houdiniNameAttributeHandle.set(houdiniNameAttributeBuilder);
        houdiniNameAttributeHandle.setAllClean();

        instanceTransformHandle.set(instanceTransformBuilder);
        instanceTransformHandle.setAllClean();

        delete[] instTransforms;

        MArrayDataBuilder builder = instancedObjectNamesHandle.builder();
        if(myObjectInfo.objectToInstanceId >= 0)
        {
            // instancing a single object
            HAPI_ObjectInfo instanceObjectInfo;
            CHECK_HAPI(HAPI_GetObjectInfo(
                    Util::theHAPISession.get(),
                    myObjectInfo.objectToInstanceId,
                    &instanceObjectInfo
                    ));
            MString name = Util::HAPIString(instanceObjectInfo.nameSH);

            MDataHandle h = builder.addElement(0);
            h.set(name);

            // clean up extra elements
            for(unsigned int i= builder.elementCount(); i-- > 1;)
            {
                builder.removeElement(i);
            }
        } else
        {
            // instancing multiple objects
            for(unsigned int i=0; i< myUniqueInstObjNames.length(); i++)
            {
                MDataHandle h = builder.addElement(i);
                h.set(myUniqueInstObjNames[i]);
            }

            // clean up extra elements
            for(unsigned int i= builder.elementCount();
                    i-- > myUniqueInstObjNames.length();)
            {
                builder.removeElement(i);
            }
        }
        instancedObjectNamesHandle.set(builder);
        instancedObjectNamesHandle.setAllClean();

        handle.setClean();
        instancerDataHandle.setClean();
        //data.setClean(plug);
        //data.setClean(instancerDataPlug);

        myLastSopCookCount = mySopNodeInfo.totalCookCount;
    }

    return MS::kSuccess;
}
