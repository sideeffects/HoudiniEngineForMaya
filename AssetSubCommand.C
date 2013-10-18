#include "AssetSubCommand.h"

#include <maya/MFnDependencyNode.h>

#include "AssetNode.h"

AssetSubCommand::AssetSubCommand()
{
}

AssetSubCommand::~AssetSubCommand()
{
}

MStatus
AssetSubCommand::doIt()
{
    return MStatus::kSuccess;
}

MStatus
AssetSubCommand::redoIt()
{
    return MStatus::kSuccess;
}

MStatus
AssetSubCommand::undoIt()
{
    return MStatus::kSuccess;
}

bool
AssetSubCommand::isUndoable() const
{
    return false;
}

AssetSubCommandAsset::AssetSubCommandAsset(const MObject &assetNodeObj) :
    myAssetNodeObj(assetNodeObj)
{
}

AssetNode*
AssetSubCommandAsset::getAssetNode() const
{
    MFnDependencyNode assetNodeFn(myAssetNodeObj);

    AssetNode* assetNode = dynamic_cast<AssetNode*>(assetNodeFn.userNode());

    return assetNode;
}

Asset*
AssetSubCommandAsset::getAsset() const
{
    Asset* asset = getAssetNode()->getAsset();

    return asset;
}
