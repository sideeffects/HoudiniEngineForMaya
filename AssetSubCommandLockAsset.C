#include "AssetSubCommandLockAsset.h"

#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MPxCommand.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MItDependencyGraph.h>

#include "AssetNode.h"
#include "OutputPartInstancerNode.h"
#include "util.h"


AssetSubCommandLockAsset::AssetSubCommandLockAsset(const MObject &assetNodeObj, bool lockOn)
: SubCommandAsset(assetNodeObj)
, myLockOn(lockOn)
{
}

MStatus 
AssetSubCommandLockAsset::doIt()
{
    GET_COMMAND_ASSET_OR_RETURN_FAIL();

    MStatus status;

    MFnDependencyNode assetNodeFn(myAssetNodeObj);
    MPlug lockAsset = assetNodeFn.findPlug(AssetNode::lockAsset, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    bool lockOnState = lockAsset.asBool();

    if(myLockOn == lockOnState)
    {
        // nothing to do
        return MStatus::kSuccess;
    }
    myDagModifier.newPlugValueBool(lockAsset, myLockOn);

    MItDependencyGraph iter(myAssetNodeObj,
        MFn::kMessageAttribute, MItDependencyGraph::kUpstream,
        MItDependencyGraph::kBreadthFirst, MItDependencyGraph::kPlugLevel, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    CHECK_MSTATUS_AND_RETURN_IT(iter.enablePruningOnFilter());

    for(; !iter.isDone(); iter.next())
    {
        MPlug myNodeMsgPlug = iter.thisPlug(&status);
        const unsigned int msgIx = myNodeMsgPlug.logicalIndex(&status);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        MObject otherNode = Util::plugSource(myNodeMsgPlug).node(&status);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        MPlug myNodeOutputPartPlug = myNodeMsgPlug.array().parent(&status);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        MFnDependencyNode otherNodeFn(otherNode, &status);

        if(otherNodeFn.typeId() == OutputPartInstancerNode::typeId)
        {
            if(myLockOn)
            {
                MPlug pointDataPlug = otherNodeFn.findPlug(OutputPartInstancerNode::pointData, true, &status);
                CHECK_MSTATUS_AND_RETURN_IT(status);
                status = myDagModifier.disconnect(Util::plugSource(pointDataPlug), pointDataPlug);
                CHECK_MSTATUS_AND_RETURN_IT(status);
            }
            else
            {
                MPlug sourcePlug = myNodeOutputPartPlug.child(AssetNode::outputPartInstancer, &status);
                CHECK_MSTATUS_AND_RETURN_IT(status);
                sourcePlug = sourcePlug.child(AssetNode::outputPartInstancerArrayData, &status);
                CHECK_MSTATUS_AND_RETURN_IT(status);

                MPlug pointDataPlug = otherNodeFn.findPlug(OutputPartInstancerNode::pointData, false, &status);
                CHECK_MSTATUS_AND_RETURN_IT(status);
                myDagModifier.connect(sourcePlug, pointDataPlug);
                CHECK_MSTATUS_AND_RETURN_IT(status);
            }
        }
        else if(otherNode.hasFn(MFn::kMesh))
        {
            if(myLockOn)
            {
                MPlug inMeshPlug = otherNodeFn.findPlug("inMesh", true, &status);
                status = myDagModifier.disconnect(Util::plugSource(inMeshPlug), inMeshPlug);
                CHECK_MSTATUS_AND_RETURN_IT(status);
            }
            else
            {
                MPlug inMeshPlug = otherNodeFn.findPlug("inMesh", false, &status);
                MPlug sourcePlug = myNodeOutputPartPlug.child(AssetNode::outputPartMesh, &status);
                CHECK_MSTATUS_AND_RETURN_IT(status);
                sourcePlug = sourcePlug.child(AssetNode::outputPartMeshData, &status);
                CHECK_MSTATUS_AND_RETURN_IT(status);
                status = myDagModifier.connect(sourcePlug, inMeshPlug);
                CHECK_MSTATUS_AND_RETURN_IT(status);
            }
        }
        else if(otherNode.hasFn(MFn::kNurbsCurve))
        {
            if(myLockOn)
            {
                MPlug createPlug = otherNodeFn.findPlug("create", true, &status);
                status = myDagModifier.disconnect(Util::plugSource(createPlug), createPlug);
                CHECK_MSTATUS(status);
            }
            else
            {
                MPlug createPlug = otherNodeFn.findPlug("create", false, &status);
                MPlug sourcePlug = myNodeOutputPartPlug.child(AssetNode::outputPartCurves, &status);
                sourcePlug = sourcePlug.elementByLogicalIndex(msgIx, &status);
                CHECK_MSTATUS_AND_RETURN_IT(status);

                status = myDagModifier.connect(sourcePlug, createPlug);
                CHECK_MSTATUS(status);
            }
        }
        else
        {
            DISPLAY_WARNING("Unexpected message connection to node ^1s", otherNodeFn.name());
        }
    }
    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

MStatus 
AssetSubCommandLockAsset::redoIt()
{
    myDagModifier.doIt();
    return MStatus::kSuccess;
}

MStatus 
AssetSubCommandLockAsset::undoIt()
{
    myDagModifier.undoIt();
    return MStatus::kSuccess;
}
