#include "AssetSyncOutputInstance.h"

#include <maya/MDagModifier.h>
#include <maya/MGlobal.h>

#include <maya/MFnDagNode.h>

#include "AssetNode.h"
#include "util.h"

AssetSyncOutputInstance::AssetSyncOutputInstance(
	const MPlug &outputPlug,
	const MObject &assetTransform
	) :
    myOutputPlug(outputPlug),
    myAssetTransform(assetTransform)
{
}

AssetSyncOutputInstance::~AssetSyncOutputInstance()
{
}

MStatus
AssetSyncOutputInstance::doIt()
{
    MStatus status;

    status = createOutput();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return redoIt();
}

MStatus
AssetSyncOutputInstance::undoIt()
{
    MStatus status;

    status = myDagModifier.undoIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

MStatus
AssetSyncOutputInstance::redoIt()
{
    MStatus status;

    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

MStatus
AssetSyncOutputInstance::createOutput()
{
    MStatus status;

    MFnDagNode assetTransformFn(myAssetTransform);

    // create the instancer node
    MObject instancer = myDagModifier.createNode("instancer", myAssetTransform, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFnDependencyNode instancerFn(instancer, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // set the rotation units to radians
    status = myDagModifier.newPlugValueInt(instancerFn.findPlug("rotationAngleUnits"), 1);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    {
	MPlug srcPlug;
	MPlug dstPlug;

	// inputPoints
	srcPlug = myOutputPlug.child(AssetNode::outputInstancerData);
	dstPlug = instancerFn.findPlug("inputPoints");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// go through every instanced objects
	MPlug instancedNamesPlug = myOutputPlug.child(AssetNode::outputInstancedObjectNames);
	MPlug inputHierarchyPlug = instancerFn.findPlug("inputHierarchy");
	for(unsigned int i = 0; i < instancedNamesPlug.numElements(); i++)
	{
	    MObject objectTransform = Util::findDagChild(assetTransformFn, instancedNamesPlug[i].asString());
	    MFnDependencyNode objectTransformFn(objectTransform);

	    // connect inputHierarchy
	    srcPlug = objectTransformFn.findPlug("matrix");
	    dstPlug = inputHierarchyPlug.elementByLogicalIndex(i);
	    status = myDagModifier.connect(srcPlug, dstPlug);
	    CHECK_MSTATUS_AND_RETURN_IT(status);

	    // set objectTransform hidden
	    status = myDagModifier.newPlugValueInt(objectTransformFn.findPlug("visibility"), 0);
	    CHECK_MSTATUS_AND_RETURN_IT(status);
	}
    }

    // doIt
    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}
