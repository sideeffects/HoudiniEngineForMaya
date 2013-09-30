#include "AssetSyncOutputObject.h"

#include <maya/MGlobal.h>
#include <maya/MPlug.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>

#include "AssetNode.h"
#include "AssetSyncOutputGeoPart.h"
#include "AssetSyncOutputInstance.h"

AssetSyncOutputObject::AssetSyncOutputObject(
	const MPlug &outputPlug,
	const MObject &assetNodeObj )
    : myOutputPlug(outputPlug),
      myAssetNodeObj(assetNodeObj)
{}

AssetSyncOutputObject::~AssetSyncOutputObject()
{
    for(AssetSyncs::const_iterator it = myAssetSyncs.begin();
	    it != myAssetSyncs.end();
	    it++)
    {
	delete *it;
    }
    myAssetSyncs.clear();
}

MStatus
AssetSyncOutputObject::doIt()
{
    MStatus status;
    // Create our parts.
    // An object just contains a number of parts, and no
    // other information.
    MFnDependencyNode assetNodeFn(myAssetNodeObj, &status);

    // Parts
    MPlug partsPlug = myOutputPlug.child(AssetNode::outputParts);
    int partCount = partsPlug.evaluateNumElements(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    for (int i=0; i<partCount; i++)
    {
	AssetSync* sync = new AssetSyncOutputGeoPart(partsPlug[i], myAssetNodeObj);
	sync->doIt();
	myAssetSyncs.push_back(sync);
    }

    return MStatus::kSuccess;
}

MStatus
AssetSyncOutputObject::undoIt()
{
    for(AssetSyncs::reverse_iterator iter = myAssetSyncs.rbegin();
	    iter != myAssetSyncs.rend();
	    iter++)
    {
	(*iter)->undoIt();
    }
    myDagModifier.undoIt();

    return MStatus::kSuccess;
}

MStatus
AssetSyncOutputObject::redoIt()
{
    myDagModifier.doIt();
    for(AssetSyncs::iterator iter = myAssetSyncs.begin();
	    iter != myAssetSyncs.end();
	    iter++)
    {
	(*iter)->redoIt();
    }

    return MStatus::kSuccess;
}
