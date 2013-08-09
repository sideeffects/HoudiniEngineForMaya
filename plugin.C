#include <maya/MFnPlugin.h>

#include "AssetCommand.h"
#include "AssetNode.h"
#include "util.h"

#include <cstdlib>

bool
initializeHAPI()
{
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;

    MString otl_dir(getenv("HAPI_OTL_PATH"));
    MString dso_dir(getenv("HAPI_DSO_PATH"));

    MString hfs(getenv("HAPI_PATH"));
    if (hfs == "")
    {
	cerr << "*Error*: HAPI_PATH not found" << endl;
	throw HAPIError("HFS directory not found");
    }
    hfs += "/";
    cerr << "hfs: " << hfs.asChar() << endl;
    hstat = HAPI_Initialize(hfs.asChar(), otl_dir.asChar(),
	    dso_dir.asChar(), true, -1);

    return true;
}

MStatus
initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, "Side Effects", "1.0", "Any");
    
    if(!initializeHAPI())
    {
	return MStatus::kFailure;
    }

    status = plugin.registerUI("hAssetCreateUI", "hAssetDeleteUI");
    Util::printMayaStatus(status);

    status = plugin.registerNode("hAsset", AssetNode::id, AssetNode::creator, AssetNode::initialize);
    Util::printMayaStatus(status);

    status = plugin.registerCommand("hAsset", AssetCommand::creator, AssetCommand::newSyntax);
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
