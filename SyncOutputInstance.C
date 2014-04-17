#include "SyncOutputInstance.h"

#include <maya/MDagModifier.h>
#include <maya/MDagPath.h>
#include <maya/MGlobal.h>

#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>

#include "AssetNode.h"
#include "util.h"

SyncOutputInstance::SyncOutputInstance(
        const MPlug &outputPlug,
        const int parentMultiIndex,
        const MObject &assetNodeObj
        ) :
    myOutputPlug(outputPlug),
    myAssetNodeObj(assetNodeObj),
    myParentMultiIndex(parentMultiIndex)
{
}

SyncOutputInstance::~SyncOutputInstance()
{
}

MStatus
SyncOutputInstance::doIt()
{
    MStatus status;

    status = createOutput();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return redoIt();
}

MStatus
SyncOutputInstance::undoIt()
{
    MStatus status;

    status = myDagModifier.undoIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

MStatus
SyncOutputInstance::redoIt()
{
    MStatus status;

    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

bool
SyncOutputInstance::isUndoable() const
{
    return true;
}

void
SyncOutputInstance::instanceObject(MDagPath & objToInstance,
                                         MObject instancerTransform,
                                          int pointIndex)
{
    MFnDagNode fnAsset(myAssetNodeObj);
    MDagPath assetPath;
    fnAsset.getPath(assetPath);

    MFnDagNode fnInstancerTransform(instancerTransform);
    MDagPath instancerTransformPath;
    fnInstancerTransform.getPath(instancerTransformPath);

    MString cmd = "houdiniEngine_createInstance(\"";
    cmd += assetPath.partialPathName();
    cmd += "\",\"";
    cmd += objToInstance.partialPathName();
    cmd += "\",\"";
    cmd += instancerTransformPath.partialPathName();
    cmd += "\",";
    cmd += myParentMultiIndex;
    cmd += ",";
    cmd += pointIndex;
    cmd += ");";

    MGlobal::executeCommand(cmd);
}

bool
SyncOutputInstance::stringStartsWith(const MString & string, const MString & startsWith)
{
    if(string.length() >= startsWith.length())
    {
        if(string.substring(0, startsWith.length() -1) == startsWith)
        {
            return true;
        }
    }
    return false;
}

bool
SyncOutputInstance::instanceObjects(MObject searchRoot,
                                          MObject instancerTransform,
                                          int pointIndex,
                                          const MString & objectToInstanceName,
                                          const MString & houdiniInstanceAttr,
                                          const MString & houdiniNameAttr)
{
    MItDag dagIt(MItDag::kDepthFirst, MFn::kTransform);
    dagIt.reset(searchRoot);
    for(; !dagIt.isDone(); dagIt.next())
    {
        MDagPath currPath;
        if(dagIt.getPath (currPath) != MS::kSuccess)
        {
            continue;
        }

        MObject node = currPath.node();
        MFnDagNode fnNode(node);
        MString nodeName = fnNode.name();

        if(objectToInstanceName.length() > 0 &&
            stringStartsWith(nodeName, objectToInstanceName))
        {
            instanceObject(currPath, instancerTransform, pointIndex);
            return true;
        }

        if(houdiniInstanceAttr.length() > 0)
        {
            MStringArray splitObjName;
            houdiniInstanceAttr.split('/', splitObjName);
            MString instanceAttrObjectName = splitObjName[splitObjName.length()-1];
            if(stringStartsWith(nodeName, instanceAttrObjectName))
            {
                instanceObject(currPath, instancerTransform, pointIndex);
                return true;
            }
        }

        if(houdiniNameAttr.length() > 0)
        {
            if(stringStartsWith(nodeName, houdiniNameAttr))
            {
                instanceObject(currPath, instancerTransform, pointIndex);
                return true;
            }
        }
    }

    return false;
}

MStatus
SyncOutputInstance::createOutput()
{
    MStatus status;

    MFnDagNode assetNodeFn(myAssetNodeObj);

    MPlug useInstanceNodePlug = assetNodeFn.findPlug(AssetNode::useInstancerNode);
    bool useInstanceNode = useInstanceNodePlug.asBool();

    if(useInstanceNode)
    {
        // create the instancer node
        MObject instancer = myDagModifier.createNode("instancer", myAssetNodeObj, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        MFnDependencyNode instancerFn(instancer, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        // set the rotation units to radians
        status = myDagModifier.newPlugValueInt(instancerFn.findPlug("rotationAngleUnits"), 1);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        {
            MPlug srcPlug;
            MPlug dstPlug;

            // inputPoints
            srcPlug = myOutputPlug.child(AssetNode::outputInstancerData);
            dstPlug = instancerFn.findPlug("inputPoints");
            status = myDagModifier.connect(srcPlug, dstPlug);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            // go through every instanced objects
            MPlug instancedNamesPlug = myOutputPlug.child(AssetNode::outputInstancedObjectNames);
            MPlug inputHierarchyPlug = instancerFn.findPlug("inputHierarchy");
            for(unsigned int i = 0; i < instancedNamesPlug.numElements(); i++)
            {
                MObject objectTransform = Util::findDagChild(assetNodeFn, instancedNamesPlug[i].asString());
                MFnDependencyNode objectTransformFn(objectTransform);

                // connect inputHierarchy
                srcPlug = objectTransformFn.findPlug("matrix");
                dstPlug = inputHierarchyPlug.elementByLogicalIndex(i);
                status = myDagModifier.connect(srcPlug, dstPlug);
                CHECK_MSTATUS_AND_RETURN_IT(status);

                // set objectTransform hidden
                status = myDagModifier.newPlugValueInt(objectTransformFn.findPlug("visibility"), 0);
                CHECK_MSTATUS_AND_RETURN_IT(status);
            }
        }
    }
    else
    {
        MObject instancerTransform = myDagModifier.createNode("transform", myAssetNodeObj, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        myDagModifier.renameNode(instancerTransform, "instancer");

        myDagModifier.doIt();

        MPlug instanceTransformPlug = assetNodeFn.findPlug(AssetNode::outputInstanceTransform);
        instanceTransformPlug.selectAncestorLogicalIndex (myParentMultiIndex, AssetNode::outputInstancers);
        unsigned int numPoints = instanceTransformPlug.numElements();

        MString objectToInstanceName("");
        MPlug instanceObjectNamesPlug = assetNodeFn.findPlug(AssetNode::outputInstancedObjectNames);
        instanceObjectNamesPlug.selectAncestorLogicalIndex (myParentMultiIndex, AssetNode::outputInstancers);
        unsigned int numInstanceObjects = instanceObjectNamesPlug.numElements();

        if(numInstanceObjects == 1)
        {
            MPlug namePlug = instanceObjectNamesPlug.elementByLogicalIndex(0);
            objectToInstanceName = namePlug.asString();
        }

        MPlug houdiniInstanceAttributesPlug = assetNodeFn.findPlug(AssetNode::outputHoudiniInstanceAttribute);
        houdiniInstanceAttributesPlug.selectAncestorLogicalIndex(myParentMultiIndex, AssetNode::outputInstancers);
        int numHoudiniInstanceAttributes = houdiniInstanceAttributesPlug.numElements();

        MPlug houdiniNameAttributesPlug = assetNodeFn.findPlug(AssetNode::outputHoudiniNameAttribute);
        houdiniNameAttributesPlug.selectAncestorLogicalIndex(myParentMultiIndex, AssetNode::outputInstancers);
        int numHoudiniNameAttributes = houdiniNameAttributesPlug.numElements();

        for(unsigned int ii = 0; ii < numPoints; ii++)
        {
            MPlug houdiniInstanceAttrPlug = houdiniInstanceAttributesPlug.elementByLogicalIndex(ii);
            MString instanceAttrStr("");
            if(numHoudiniInstanceAttributes == numPoints)
                instanceAttrStr = houdiniInstanceAttrPlug.asString();

            MPlug houdiniNameAttrPlug = houdiniNameAttributesPlug.elementByLogicalIndex(ii);
            MString nameAttrStr("");
            if(numHoudiniNameAttributes == numPoints)
                nameAttrStr = houdiniNameAttrPlug.asString();

            bool doneInstancing = false;

            instanceObjects(myAssetNodeObj, instancerTransform, ii, objectToInstanceName, instanceAttrStr, nameAttrStr);
        }
    }

    // doIt
    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}
