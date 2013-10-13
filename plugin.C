#include <maya/MGlobal.h>

#include <maya/MFnPlugin.h>

#include "AssetCommand.h"
#include "AssetNode.h"
#include "util.h"

#include <cstdlib>

bool
initializeHAPI()
{
    if(HAPI_IsInitialized() == HAPI_RESULT_SUCCESS)
    {
	return true;
    }

    HAPI_Result hstat = HAPI_RESULT_SUCCESS;

    MString otl_dir(getenv("HAPI_OTL_PATH"));
    MString dso_dir(getenv("HAPI_DSO_PATH"));

    MString hfs(getenv("HAPI_PATH"));
    if (hfs == "")
    {
	MGlobal::displayError("HAPI_PATH enviornment variable is not set.");
	return false;
    }

    hstat = HAPI_Initialize(hfs.asChar(), otl_dir.asChar(),
	    dso_dir.asChar(), true, -1);
    if(hstat != HAPI_RESULT_SUCCESS)
    {
	CHECK_HAPI(hstat);
	return false;
    }

    return true;
}

MStatus
initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, "Side Effects", "1.5", "Any");
    
    if(initializeHAPI())
    {
	MGlobal::displayInfo("Houdini Engine initialized successfully.");
    }
    else
    {
	MGlobal::displayInfo("Houdini Engine failed to initialize.");
	return MStatus::kFailure;
    }

    status = plugin.registerUI("houdiniEngineCreateUI", "houdiniEngineDeleteUI");
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = plugin.registerTransform(
	    "houdiniAsset",
	    AssetNode::id,
	    AssetNode::creator,
	    AssetNode::initialize,
	    MPxTransformationMatrix::creator,
	    MPxTransformationMatrix::baseTransformationMatrixId
	    );
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = plugin.registerCommand("houdiniAsset", AssetCommand::creator, AssetCommand::newSyntax);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}


MStatus
uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);
    
    status = plugin.deregisterNode(AssetNode::id);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = plugin.deregisterCommand("houdiniAsset");
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}
