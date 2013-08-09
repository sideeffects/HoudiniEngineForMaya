#include <maya/MFnPlugin.h>

#include "AssetCommand.h"
#include "AssetNode.h"
#include "util.h"


MStatus
initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, "Side Effects", "1.0", "Any");
    
    status = plugin.registerUI("hAssetCreateUI", "hAssetDeleteUI");
    Util::printMayaStatus(status);

    status = plugin.registerNode("hAsset", AssetNode::id, AssetNode::creator, AssetNode::initialize);
    Util::printMayaStatus(status);

    status = plugin.registerCommand("hAsset", AssetCommand::creator);
    Util::printMayaStatus(status);

    return status;
}


MStatus
uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);
    
    status = plugin.deregisterNode(AssetNode::id);
    Util::printMayaStatus(status);

    status = plugin.deregisterCommand("hAsset");
    Util::printMayaStatus(status);

    return status;
}
