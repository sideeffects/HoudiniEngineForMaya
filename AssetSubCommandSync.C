#include "AssetSubCommandSync.h"

#include <maya/MGlobal.h>
#include <maya/MPlug.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>

#include "AssetNode.h"
#include "AssetSyncAttribute.h"
#include "AssetSyncOutputObject.h"
#include "AssetSyncOutputInstance.h"

AssetSubCommandSync::AssetSubCommandSync(
	const MObject &assetNodeObj,
	const bool syncOnlyVisible
	) :
    AssetSubCommandAsset(assetNodeObj),
    mySyncOnlyVisible( syncOnlyVisible )
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

    // attributes
    {
	AssetSync* syncOutput = new AssetSyncAttribute(myAssetNodeObj);
	syncOutput->doIt();

	myAssetSyncs.push_back(syncOutput);
    }

    MFnDagNode assetNodeFn(myAssetNodeObj, &status);

    // Objects
    MPlug objectsPlug = assetNodeFn.findPlug(AssetNode::outputObjects);
    unsigned int objCount = objectsPlug.evaluateNumElements(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    for(unsigned int ii = 0; ii < assetNodeFn.childCount(); ii++ )
    {
	MObject childNode = assetNodeFn.child( ii );
	myDagModifier.deleteNode( childNode );
    }

    for(unsigned int i=0; i < objCount; i++)
    {
	MPlug elemPlug = objectsPlug[i];

	MPlug visibilityPlug = elemPlug.child( AssetNode::outputVisibility );
	bool visible = visibilityPlug.asBool();

	MPlug instancedPlug = elemPlug.child( AssetNode::outputIsInstanced );
	bool instanced = instancedPlug.asBool();

	if( !mySyncOnlyVisible || visible || instanced )
	{
	    AssetSync* syncOutput = new AssetSyncOutputObject(elemPlug, myAssetNodeObj, visible );
	    syncOutput->doIt();

	    myAssetSyncs.push_back(syncOutput);
	}
    }

    // instancers
    MPlug instancersPlug = assetNodeFn.findPlug(AssetNode::outputInstancers);
    unsigned int instCount = instancersPlug.numElements(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    for(unsigned int i=0; i < instCount; i++)
    {
	MPlug elemPlug = instancersPlug[i];

        AssetSync* syncOutput = new AssetSyncOutputInstance(elemPlug, i, myAssetNodeObj);
	syncOutput->doIt();

	myAssetSyncs.push_back(syncOutput);
    }

    return MStatus::kSuccess;
}

MStatus
AssetSubCommandSync::redoIt()
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

MStatus
AssetSubCommandSync::undoIt()
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

bool
AssetSubCommandSync::isUndoable() const
{
    return true;
}
