#include "AssetSubCommandSync.h"

#include <maya/MGlobal.h>
#include <maya/MPlug.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>

#include "AssetNode.h"
#include "AssetSyncAttribute.h"
#include "AssetSyncOutputGeoPart.h"
#include "AssetSyncOutputInstance.h"

AssetSubCommandSync::AssetSubCommandSync(
	const MObject &assetNodeObj
	) :
    myAssetNodeObj(assetNodeObj)
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

    MFnDependencyNode assetNodeFn(myAssetNodeObj, &status);

    // geo parts
    MPlug objectsPlug = assetNodeFn.findPlug(AssetNode::outputObjects);
    unsigned int objCount = objectsPlug.evaluateNumElements(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    
    MFnDagNode fnDagNode( myAssetNodeObj );
    for(unsigned int ii = 0; ii < fnDagNode.childCount(); ii++ )
    {
	MObject childNode = fnDagNode.child( ii );
	myDagModifier.deleteNode( childNode );
    }
    

    for(unsigned int i=0; i < objCount; i++)
    {
	MPlug elemPlug = objectsPlug[i];

        AssetSync* syncOutput = new AssetSyncOutputGeoPart(elemPlug, myAssetNodeObj);
	syncOutput->doIt();

	myAssetSyncs.push_back(syncOutput);
    }

    // instancers
    MPlug instancersPlug = assetNodeFn.findPlug(AssetNode::outputInstancers);
    unsigned int instCount = instancersPlug.numElements(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    for(unsigned int i=0; i < instCount; i++)
    {
	MPlug elemPlug = instancersPlug[i];

        AssetSync* syncOutput = new AssetSyncOutputInstance(elemPlug, myAssetNodeObj);
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
