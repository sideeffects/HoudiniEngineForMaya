#include "SubCommand.h"

#include <maya/MFnDependencyNode.h>

#include "AssetNode.h"

SubCommand::SubCommand() {}

SubCommand::~SubCommand() {}

MStatus
SubCommand::doIt()
{
    return MStatus::kSuccess;
}

MStatus
SubCommand::redoIt()
{
    return MStatus::kSuccess;
}

MStatus
SubCommand::undoIt()
{
    return MStatus::kSuccess;
}

bool
SubCommand::isUndoable() const
{
    return false;
}

SubCommandAsset::SubCommandAsset(const MObject &assetNodeObj)
    : myAssetNodeObj(assetNodeObj)
{
}

AssetNode *
SubCommandAsset::getAssetNode() const
{
    MFnDependencyNode assetNodeFn(myAssetNodeObj);

    AssetNode *assetNode = dynamic_cast<AssetNode *>(assetNodeFn.userNode());

    return assetNode;
}

Asset *
SubCommandAsset::getAsset() const
{
    Asset *asset = getAssetNode()->getAsset();

    return asset;
}

