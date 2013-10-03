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

#if MAYA_API_VERSION >= 201400
    createFluidShape();
#endif

    return MStatus::kSuccess;
}

#if MAYA_API_VERSION >= 201400
MStatus
AssetSyncOutputObject::createFluidShapeNode(MObject& transform, MObject& fluid)
{
    MStatus status;
    transform = myDagModifier.createNode("transform", myAssetNodeObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    // TODO: name
    status  = myDagModifier.renameNode(transform, "fluid_transform");
    CHECK_MSTATUS_AND_RETURN_IT(status);

    fluid = myDagModifier.createNode("fluidShape", transform, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}


MStatus
AssetSyncOutputObject::createFluidShape()
{
    MStatus status;

    // Look for density.
    // Once we've found the first density, look again through everything
    // for any volumes which share a transform with the first density, 
    // and add them to the fluidShape.
    MPlug partsPlug = myOutputPlug.child(AssetNode::outputParts);
    int partCount = partsPlug.evaluateNumElements(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Find density
    bool hasDensity = false;
    MPlug densityVolume;
    for (int i=0; i<partCount; i++)
    {
	MPlug outputVolume = partsPlug[i].child(AssetNode::outputPartVolume);
	MPlug outputPartName = partsPlug[i].child(AssetNode::outputPartName);
	MPlug outputVolumeName = outputVolume.child(AssetNode::outputPartVolumeName);

	MString name = outputVolumeName.asString();

	if (name == "density")
	{
	    hasDensity = true;
	    densityVolume = outputVolume;
	    break;
	}
    }

    if (!hasDensity)
	return MStatus::kSuccess;

    MObject transform, fluid;
    createFluidShapeNode(transform, fluid);

    MPlug densityTransform = densityVolume.child(AssetNode::outputPartVolumeTransform);

    MFnDependencyNode partVolumeFn(fluid, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MFnDependencyNode partFluidTransformFn(transform, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    bool doneDensity = false;
    bool doneTemperature = false;
    for (int i=0; i<partCount; i++)
    {
	MPlug outputVolume = partsPlug[i].child(AssetNode::outputPartVolume);
	MPlug outputVolumeName = outputVolume.child(AssetNode::outputPartVolumeName);
	MPlug outputVolumeTransform = outputVolume.child(AssetNode::outputPartVolumeTransform);

	// If the transform of the volumes are different, we don't want
	// to group them together.
	if (outputVolumeTransform != densityTransform.attribute())
	    continue;

	MPlug srcPlug = outputVolume.child(AssetNode::outputPartVolumeGrid);
	MString name = outputVolumeName.asString();
	if (name == "density" && !doneDensity)
	{
	    status = myDagModifier.connect(srcPlug, partVolumeFn.findPlug("inDensity"));
	    CHECK_MSTATUS_AND_RETURN_IT(status);
	    doneDensity = true;
	}
	else if (name == "temperature" && !doneTemperature)
	{
	    status = myDagModifier.connect(srcPlug, partVolumeFn.findPlug("inTemperature"));
	    CHECK_MSTATUS_AND_RETURN_IT(status);
	    doneTemperature = true;
	}
    }

    // Connect the transform, resolution, dimensions, and playFromCache
    {
	MPlug srcPlug;
	MPlug dstPlug;

	MPlug densityTransform = densityVolume.child(AssetNode::outputPartVolumeTransform);

	srcPlug = densityTransform.child(AssetNode::outputPartVolumeTranslate);
	dstPlug = partFluidTransformFn.findPlug("translate");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	srcPlug = densityTransform.child(AssetNode::outputPartVolumeRotate);
	dstPlug = partFluidTransformFn.findPlug("rotate");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	srcPlug = densityTransform.child(AssetNode::outputPartVolumeScale);
	dstPlug = partFluidTransformFn.findPlug("scale");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	srcPlug = densityVolume.child(AssetNode::outputPartVolumeRes);
	dstPlug = partVolumeFn.findPlug("resolution");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Connect the dimensions and resolution
	dstPlug = partVolumeFn.findPlug("dimensions");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	srcPlug = myOutputPlug.child(AssetNode::outputObjectFluidFromAsset);
	dstPlug = partVolumeFn.findPlug("playFromCache");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);
}
#endif

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
