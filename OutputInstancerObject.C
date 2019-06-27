#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MFnArrayAttrsData.h>

#include "Asset.h"
#include "AssetNode.h"
#include "AssetNodeOptions.h"
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
OutputInstancerObject::compute(
        const MTime &time,
        const MPlug &plug,
        MDataBlock& data,
        MDataHandle& handle,
        AssetNodeOptions::AccessorDataBlock &options,
        bool &needToSyncOutputs
        )
{
    update();

    if(myGeoInfo.partCount <= 0)
        return MS::kFailure;

    if(mySopNodeInfo.totalCookCount > myLastSopCookCount)
    {
        MDataHandle instancerDataHandle = handle.child(AssetNode::outputInstancerData);
        MArrayDataHandle instancedObjectNamesHandle = handle.child(AssetNode::outputInstancedObjectNames);
        MArrayDataHandle houdiniInstanceAttributeHandle = handle.child(AssetNode::outputHoudiniInstanceAttribute);
        MArrayDataHandle houdiniNameAttributeHandle = handle.child(AssetNode::outputHoudiniNameAttribute);
        MArrayDataHandle instanceTransformHandle = handle.child(AssetNode::outputInstanceTransform);

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
        CHECK_HAPI(HAPI_GetInstanceTransformsOnPart(
                Util::theHAPISession.get(),
		0,
                mySopNodeInfo.id,
                HAPI_SRT,
                instTransforms,
                0, size
                ));

        Util::resizeArrayDataHandle(houdiniInstanceAttributeHandle, size);
        Util::resizeArrayDataHandle(houdiniNameAttributeHandle, size);
        Util::resizeArrayDataHandle(instanceTransformHandle, size);

        if(positions.length() != size && !options.useInstancerNode())
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

            CHECK_MSTATUS(houdiniInstanceAttributeHandle.jumpToArrayElement(j));
            MDataHandle intanceAttributeHandle = houdiniInstanceAttributeHandle.outputValue();
            intanceAttributeHandle .set(myHoudiniInstanceAttribute[j]);

            CHECK_MSTATUS(houdiniNameAttributeHandle.jumpToArrayElement(j));
            MDataHandle nameAttributeHandle = houdiniNameAttributeHandle.outputValue();
            nameAttributeHandle.set(myHoudiniNameAttribute[j]);

            CHECK_MSTATUS(instanceTransformHandle.jumpToArrayElement(j));
            MDataHandle transformHandle  = instanceTransformHandle.outputValue();

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

        houdiniInstanceAttributeHandle.setAllClean();
        houdiniNameAttributeHandle.setAllClean();
        instanceTransformHandle.setAllClean();

        delete[] instTransforms;

        if(myObjectInfo.objectToInstanceId >= 0)
        {
            // instancing a single object
            Util::resizeArrayDataHandle(instancedObjectNamesHandle, 1);

            HAPI_ObjectInfo instanceObjectInfo;
            CHECK_HAPI(HAPI_GetObjectInfo(
                    Util::theHAPISession.get(),
                    myObjectInfo.objectToInstanceId,
                    &instanceObjectInfo
                    ));
            MString name = Util::HAPIString(instanceObjectInfo.nameSH);

            CHECK_MSTATUS(instancedObjectNamesHandle.jumpToArrayElement(0));
            MDataHandle h = instancedObjectNamesHandle.outputValue();
            h.set(name);
        } else
        {
            // instancing multiple objects
            Util::resizeArrayDataHandle(instancedObjectNamesHandle,
                    myUniqueInstObjNames.length());

            for(unsigned int i=0; i< myUniqueInstObjNames.length(); i++)
            {
                CHECK_MSTATUS(instancedObjectNamesHandle.jumpToArrayElement(i));
                MDataHandle h = instancedObjectNamesHandle.outputValue();
                h.set(myUniqueInstObjNames[i]);
            }
        }
        instancedObjectNamesHandle.setAllClean();

        myLastSopCookCount = mySopNodeInfo.totalCookCount;
    }

    return MS::kSuccess;
}
