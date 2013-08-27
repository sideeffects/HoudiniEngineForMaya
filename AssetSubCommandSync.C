#include "AssetSubCommandSync.h"

#include <maya/MGlobal.h>
#include <maya/MPlug.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>

#include "AssetNode.h"
#include "AssetSyncOutputGeoPart.h"
#include "AssetSyncOutputInstance.h"

AssetSubCommandSync::AssetSubCommandSync(
	const MObject &assetNodeObj,
	const MObject &assetTransformObj
	) :
    myAssetNodeObj(assetNodeObj),
    myAssetTransformObj(assetTransformObj)
{
}

AssetSubCommandSync::~AssetSubCommandSync()
{
    for(AssetSyncs::const_iterator iter = myAssetSyncs.begin();
	    iter != myAssetSyncs.end();
	    iter++)
    {
	delete *iter;
    }
    myAssetSyncs.clear();
}

MStatus
AssetSubCommandSync::doIt()
{
    MStatus status;

    MFnDependencyNode assetNodeFn(myAssetNodeObj, &status);

    // geo parts
    MPlug objectsPlug = assetNodeFn.findPlug(AssetNode::objects);
    unsigned int objCount = objectsPlug.evaluateNumElements(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    for(unsigned int i=0; i < objCount; i++)
    {
	MPlug elemPlug = objectsPlug[i];

        AssetSync* syncOutput = new AssetSyncOutputGeoPart(elemPlug, myAssetTransformObj);
	syncOutput->doIt();

	myAssetSyncs.push_back(syncOutput);
    }

    // instancers
    MPlug instancersPlug = assetNodeFn.findPlug(AssetNode::instancers);
    unsigned int instCount = instancersPlug.numElements(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    for(unsigned int i=0; i < instCount; i++)
    {
	MPlug elemPlug = instancersPlug[i];

        AssetSync* syncOutput = new AssetSyncOutputInstance(elemPlug, myAssetTransformObj);
	syncOutput->doIt();

	myAssetSyncs.push_back(syncOutput);
    }

    return MStatus::kSuccess;
}

MStatus
AssetSubCommandSync::redoIt()
{
    for(AssetSyncs::iterator iter = myAssetSyncs.begin();
	    iter != myAssetSyncs.end();
	    iter++)
    {
	(*iter)->redoIt();
    }

    return MStatus::kSuccess;
}

MStatus
AssetSubCommandSync::undoIt()
{
    for(AssetSyncs::reverse_iterator iter = myAssetSyncs.rbegin();
	    iter != myAssetSyncs.rend();
	    iter++)
    {
	(*iter)->undoIt();
    }

    return MStatus::kSuccess;
}

bool
AssetSubCommandSync::isUndoable() const
{
    return true;
}
