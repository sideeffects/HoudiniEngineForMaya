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
	const MObject &assetNodeObj,
	bool visible )
    : myOutputPlug(outputPlug),
      myAssetNodeObj(assetNodeObj),
      myVisible( visible )
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
    // TODO: FIXME: each geo should have its own transform
    MPlug geosPlug = myOutputPlug.child( AssetNode::outputGeos );
    int geoCount = geosPlug.evaluateNumElements( &status );
    CHECK_MSTATUS_AND_RETURN_IT(status);
    for( int ii = 0; ii < geoCount; ii++ )
    {
        MPlug geoPlug = geosPlug[ ii ];

        MPlug partsPlug = geoPlug.child(AssetNode::outputParts);
        int partCount = partsPlug.evaluateNumElements(&status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        for (int jj=0; jj<partCount; jj++)
        {
	    AssetSync* sync = new AssetSyncOutputGeoPart(partsPlug[jj], myAssetNodeObj, myVisible );
	    sync->doIt();
	    myAssetSyncs.push_back(sync);
        }
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
AssetSyncOutputObject::createVelocityConverter(MObject& velocityConverter)
{
    if (!velocityConverter.isNull())
	return MS::kSuccess;

    MStatus status;
    velocityConverter = ((MDGModifier&)myDagModifier).createNode("houdiniFluidVelocityConvert", &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    return status;
}

bool
AssetSyncOutputObject::resolutionsEqual(MPlug resA, MPlug resB)
{
    MPlug resAWidth = resA.child(AssetNode::outputPartVolumeResW);
    MPlug resAHeight = resA.child(AssetNode::outputPartVolumeResH);
    MPlug resADepth = resA.child(AssetNode::outputPartVolumeResD);

    MPlug resBWidth = resB.child(AssetNode::outputPartVolumeResW);
    MPlug resBHeight = resB.child(AssetNode::outputPartVolumeResH);
    MPlug resBDepth = resB.child(AssetNode::outputPartVolumeResD);

    int resAWidthValue, resAHeightValue, resADepthValue;
    int resBWidthValue, resBHeightValue, resBDepthValue;
    resAWidth.getValue(resAWidthValue);
    resAHeight.getValue(resAHeightValue);
    resADepth.getValue(resADepthValue);
    resBWidth.getValue(resBWidthValue);
    resBHeight.getValue(resBHeightValue);
    resBDepth.getValue(resBDepthValue);
    return resAWidthValue == resBWidthValue &&
	   resAHeightValue == resBHeightValue &&
	   resADepthValue == resBDepthValue;
}


MStatus
AssetSyncOutputObject::createFluidShape()
{
    MStatus status;

    MPlug geosPlug = myOutputPlug.child( AssetNode::outputGeos );
    int geoCount = geosPlug.evaluateNumElements( &status );
    CHECK_MSTATUS_AND_RETURN_IT(status);
    for( int ii = 0; ii < geoCount; ii++ )
    {
        MPlug geoPlug = geosPlug[ ii ];

        MPlug partsPlug = geoPlug.child(AssetNode::outputParts);
        int partCount = partsPlug.evaluateNumElements(&status);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        // Look for specific volume names. Once we've found the first reference
        // volume, look again through everything for any volumes which share a
        // transform with the first reference volume, and add them to the
        // fluidShape.
        bool hasFluid = false;
        MPlug referenceVolume;
        for (int jj=0; jj<partCount; jj++)
        {
	    MPlug outputVolume = partsPlug[jj].child(AssetNode::outputPartVolume);
	    MPlug outputPartName = partsPlug[jj].child(AssetNode::outputPartName);
	    MPlug outputVolumeName = outputVolume.child(AssetNode::outputPartVolumeName);

	    MString name = outputVolumeName.asString();

	    if (name == "density"
		    || name == "temperature"
		    || name == "fuel"
		    || name == "vel.x"
		    || name == "vel.y"
		    || name == "vel.z")
	    {
	        hasFluid = true;
	        referenceVolume = outputVolume;
	        break;
	    }
        }

        if (!hasFluid)
	    return MStatus::kSuccess;

        MObject transform, fluid;
        createFluidShapeNode(transform, fluid);

        MPlug referenceRes = referenceVolume.child(AssetNode::outputPartVolumeRes);

        MFnDependencyNode partVolumeFn(fluid, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        MFnDependencyNode partFluidTransformFn(transform, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        MObject velConverter;

        bool doneDensity = false;
        bool doneTemperature = false;
        bool doneFuel = false;
        bool doneVelX = false;
        bool doneVelY = false;
        bool doneVelZ = false;
        for (int jj=0; jj<partCount; jj++)
        {
	    MPlug outputVolume = partsPlug[jj].child(AssetNode::outputPartVolume);
	    MPlug outputVolumeName = outputVolume.child(AssetNode::outputPartVolumeName);
	    MPlug outputVolumeRes= outputVolume.child(AssetNode::outputPartVolumeRes);

	    // If the transform of the volumes are different, we don't want
	    // to group them together.
	    if (!resolutionsEqual(outputVolumeRes, referenceRes))
	        continue;

	    MPlug srcPlug = outputVolume.child(AssetNode::outputPartVolumeGrid);
	    MString name = outputVolumeName.asString();
	    if (name == "density" && !doneDensity)
	    {
	        status = myDagModifier.connect(srcPlug, partVolumeFn.findPlug("inDensity"));
	        CHECK_MSTATUS_AND_RETURN_IT(status);
	        status = myDagModifier.newPlugValueInt(
		        partVolumeFn.findPlug("densityMethod"),
		        2
		        );
	        CHECK_MSTATUS_AND_RETURN_IT(status);
	        doneDensity = true;
	    }
	    else if (name == "temperature" && !doneTemperature)
	    {
	        status = myDagModifier.connect(srcPlug, partVolumeFn.findPlug("inTemperature"));
	        CHECK_MSTATUS_AND_RETURN_IT(status);
	        status = myDagModifier.newPlugValueInt(
		        partVolumeFn.findPlug("temperatureMethod"),
		        2
		        );
	        CHECK_MSTATUS_AND_RETURN_IT(status);
	        doneTemperature = true;
	    }
	    else if (name == "fuel" && !doneFuel)
	    {
	        status = myDagModifier.connect(srcPlug, partVolumeFn.findPlug("inReaction"));
	        CHECK_MSTATUS_AND_RETURN_IT(status);
	        status = myDagModifier.newPlugValueInt(
		        partVolumeFn.findPlug("fuelMethod"),
		        2
		        );
	        CHECK_MSTATUS_AND_RETURN_IT(status);
	        doneFuel = true;
	    }
	    else if (name == "vel.x" && !doneVelX)
	    {
	        createVelocityConverter(velConverter);
	        MFnDependencyNode velConverterFn(velConverter, &status);
	        CHECK_MSTATUS_AND_RETURN_IT(status);
	        status = myDagModifier.connect(srcPlug, velConverterFn.findPlug("inGridX"));
	        CHECK_MSTATUS_AND_RETURN_IT(status);
	        doneVelX = true;
	    }
	    else if (name == "vel.y" && !doneVelY)
	    {
	        createVelocityConverter(velConverter);
	        MFnDependencyNode velConverterFn(velConverter, &status);
	        CHECK_MSTATUS_AND_RETURN_IT(status);
	        status = myDagModifier.connect(srcPlug, velConverterFn.findPlug("inGridY"));
	        CHECK_MSTATUS_AND_RETURN_IT(status);
	        doneVelY = true;
	    }
	    else if (name == "vel.z" && !doneVelZ)
	    {
	        createVelocityConverter(velConverter);
	        MFnDependencyNode velConverterFn(velConverter, &status);
	        CHECK_MSTATUS_AND_RETURN_IT(status);
	        status = myDagModifier.connect(srcPlug, velConverterFn.findPlug("inGridZ"));
	        CHECK_MSTATUS_AND_RETURN_IT(status);
	        doneVelZ = true;
	    }
        }

        // Connect the transform, resolution, dimensions, and playFromCache
        {
	    MPlug srcPlug;
	    MPlug dstPlug;

	    MPlug densityTransform = referenceVolume.child(AssetNode::outputPartVolumeTransform);

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

	    srcPlug = referenceVolume.child(AssetNode::outputPartVolumeRes);
	    dstPlug = partVolumeFn.findPlug("resolution");
	    status = myDagModifier.connect(srcPlug, dstPlug);
	    CHECK_MSTATUS_AND_RETURN_IT(status);

	    // Velocity needs an additional step: since houdini may output
	    // individual grids for each component, we use a dependency node
	    // to interleave the components into one grid
	    if (!velConverter.isNull())
	    {
	        MFnDependencyNode velConverterFn(velConverter, &status);

	        srcPlug = referenceVolume.child(AssetNode::outputPartVolumeRes);
	        dstPlug = velConverterFn.findPlug("resolution");
	        status = myDagModifier.connect(srcPlug, dstPlug);
	        CHECK_MSTATUS_AND_RETURN_IT(status);

	        srcPlug = velConverterFn.findPlug("outGrid");
	        dstPlug = partVolumeFn.findPlug("inVelocity");
	        status = myDagModifier.connect(srcPlug, dstPlug);
	        CHECK_MSTATUS_AND_RETURN_IT(status);
	    }

	    // Connect the dimensions and resolution
	    srcPlug = referenceVolume.child(AssetNode::outputPartVolumeRes);
	    dstPlug = partVolumeFn.findPlug("dimensions");
	    // Connecting compound attribute to fluidShape.dimensions causes
	    // infinite recursion. Probably a Maya bug. Workaround it by connecting
	    // individual child attributes instead.
	    //status = myDagModifier.connect(srcPlug, dstPlug);
	    status = myDagModifier.connect(srcPlug.child(0), dstPlug.child(0));
	    status = myDagModifier.connect(srcPlug.child(1), dstPlug.child(1));
	    status = myDagModifier.connect(srcPlug.child(2), dstPlug.child(2));
	    CHECK_MSTATUS_AND_RETURN_IT(status);

	    srcPlug = myOutputPlug.child(AssetNode::outputObjectFluidFromAsset);
	    dstPlug = partVolumeFn.findPlug("playFromCache");
	    status = myDagModifier.connect(srcPlug, dstPlug);
	    CHECK_MSTATUS_AND_RETURN_IT(status);
        }
    }

    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
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
