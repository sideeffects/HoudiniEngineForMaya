#include "AssetSubCommandLoadOTL.h"

#include <maya/MFileObject.h>
#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MPxCommand.h>

#include "AssetNode.h"
#include "AssetSubCommandSync.h"
#include "util.h"

AssetSubCommandLoadOTL::AssetSubCommandLoadOTL(
	const MString &otlFilePath,
	const MString &assetName
	) :
    myOTLFilePath(otlFilePath),
    myAssetName(assetName),
    myAssetSubCommandSync(NULL)
{
}

AssetSubCommandLoadOTL::~AssetSubCommandLoadOTL()
{
}

MStatus
AssetSubCommandLoadOTL::doIt()
{
    MStatus status;

    // create houdiniAsset node
    MObject assetNode = myDagModifier.createNode(AssetNode::id, MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // rename houdiniAsset node
    status = myDagModifier.renameNode(assetNode, myAssetName);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // set otl file attribute
    {
	MPlug plug(assetNode, AssetNode::otlFilePath);
	status = myDagModifier.newPlugValueString(plug, myOTLFilePath);
	CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // set asset name attribute
    {
	MPlug plug(assetNode, AssetNode::assetName);
	status = myDagModifier.newPlugValueString(plug, myAssetName);
	CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // time1.outTime -> houdiniAsset.inTime
    {
	MObject srcNode = Util::findNodeByName("time1");
	MPlug srcPlug = MFnDependencyNode(srcNode).findPlug("outTime");
	MPlug dstPlug(assetNode, AssetNode::inTime);

	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // cannot simply call redoIt, because when we use AssetSubCommandSync, we
    // need to distinguish between doIt and redoIt
    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    myAssetSubCommandSync = new AssetSubCommandSync( assetNode, false );
    myAssetSubCommandSync->doIt();

    MFnDependencyNode assetNodeFn(assetNode);

    // select the node
    MGlobal::select(assetNode);

    // set result
    MPxCommand::setResult(assetNodeFn.name());

    return MStatus::kSuccess;
}

MStatus
AssetSubCommandLoadOTL::redoIt()
{
    MStatus status;

    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = myAssetSubCommandSync->redoIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

MStatus
AssetSubCommandLoadOTL::undoIt()
{
    MStatus status;

    status = myAssetSubCommandSync->undoIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = myDagModifier.undoIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

bool AssetSubCommandLoadOTL::isUndoable() const
{
    return true;
}
