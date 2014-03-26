#include "AssetSyncOutputObject.h"

#include <maya/MFloatArray.h>
#include <maya/MGlobal.h>
#include <maya/MPlug.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>
#include <maya/MDagPath.h>

#include "AssetNode.h"
#include "AssetSyncOutputGeoPart.h"
#include "AssetSyncOutputInstance.h"

#if MAYA_API_VERSION >= 201400
	#include <maya/MFnFloatArrayData.h>
#endif

AssetSyncOutputObject::AssetSyncOutputObject(
	const MPlug &outputPlug,
	const MObject &assetNodeObj,
	const bool visible,
        const bool syncTemplatedGeos )
    : myOutputPlug(outputPlug),
      myAssetNodeObj(assetNodeObj),
      myVisible( visible ),
      mySyncTemplatedGeos( syncTemplatedGeos )
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

    MObject objectTransform = myDagModifier.createNode("transform", myAssetNodeObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);        

    // rename objectTransform
    MPlug objectNamePlug = myOutputPlug.child( AssetNode::outputObjectName );

    MString objectName = objectNamePlug.asString();
    if( objectName.length() > 0 )
        status = myDagModifier.renameNode( objectTransform, objectName );
    CHECK_MSTATUS_AND_RETURN_IT(status);
    myDagModifier.doIt();

    if( !myVisible )
    {	
	MFnDagNode fnDag( objectTransform );
	MDagPath transformPath;
	fnDag.getPath( transformPath );
	MString cmd = "hide ";
	cmd += fnDag.partialPathName();
	MGlobal::executeCommand( cmd );
    }
    
    MPlug geosPlug = myOutputPlug.child( AssetNode::outputGeos );
    int geoCount = geosPlug.evaluateNumElements( &status );
    CHECK_MSTATUS_AND_RETURN_IT(status);
    for( int ii = 0; ii < geoCount; ii++ )
    {
        MPlug geoPlug = geosPlug[ ii ];

        MObject geoTransform = MObject::kNullObj;
        MObject partParent = objectTransform;    

        MPlug isTemplatedPlug = geoPlug.child( AssetNode::outputGeoIsTemplated );
        MPlug isDisplayGeoPlug = geoPlug.child( AssetNode::outputGeoIsDisplayGeo );
        if( mySyncTemplatedGeos || !isTemplatedPlug.asBool() || isDisplayGeoPlug.asBool() )
        {
            if( geoCount > 1 )
            {                       
                geoTransform = myDagModifier.createNode("transform", objectTransform, &status);
                CHECK_MSTATUS_AND_RETURN_IT(status);        

                // rename geoTransform
                MPlug geoNamePlug = geoPlug.child( AssetNode::outputGeoName );
                MString geoName = geoNamePlug.asString();
                if( geoName.length() > 0 )
                    status = myDagModifier.renameNode( geoTransform, geoName );
                CHECK_MSTATUS_AND_RETURN_IT(status);
                myDagModifier.doIt();

                partParent = geoTransform;
            }


            MPlug partsPlug = geoPlug.child(AssetNode::outputParts);
            int partCount = partsPlug.evaluateNumElements(&status);
            CHECK_MSTATUS_AND_RETURN_IT(status);
            for (int jj=0; jj<partCount; jj++)
            {
	        AssetSubCommand* sync = new AssetSyncOutputGeoPart(partsPlug[jj], partParent );
	        sync->doIt();
	        myAssetSyncs.push_back(sync);
            }
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
    MPlug resAPlug = resA.child(AssetNode::outputPartVolumeRes);
    MObject resAObj;
    resAPlug.getValue(resAObj);

    MPlug resBPlug = resB.child(AssetNode::outputPartVolumeRes);
    MObject resBObj;
    resBPlug.getValue(resBObj);

    MStatus status;
    MFnFloatArrayData dataA(resAObj, &status);
    MFnFloatArrayData dataB(resBObj, &status);

    return dataA[0] == dataB[0] &&
	   dataA[1] == dataB[1] &&
	   dataA[2] == dataB[2];
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

        MFnDagNode partVolumeFn(fluid, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        MFnDagNode partFluidTransformFn(transform, &status);
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

        // Connect the transform, resolution, playFromCache; set inOffset and dimensions
        {
	    MPlug srcPlug;
	    MPlug dstPlug;

	    MPlug densityTransform = referenceVolume.child(AssetNode::outputPartVolumeTransform);

	    srcPlug = densityTransform.child(AssetNode::outputPartVolumeTranslate);
	    dstPlug = partFluidTransformFn.findPlug("translate");
	    status = myDagModifier.connect(srcPlug, dstPlug);
	    CHECK_MSTATUS_AND_RETURN_IT(status);

	    srcPlug = referenceVolume.child(AssetNode::outputPartVolumeRes);
	    dstPlug = partVolumeFn.findPlug("inResolution");
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

	    // Velocity needs an additional step: since houdini may output
	    // individual grids for each component, we use a dependency node
	    // to append and extrapolate the components
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

	    srcPlug = myOutputPlug.child(AssetNode::outputObjectFluidFromAsset);
	    dstPlug = partVolumeFn.findPlug("playFromCache");
	    status = myDagModifier.connect(srcPlug, dstPlug);
	    CHECK_MSTATUS_AND_RETURN_IT(status);

	    // inOffset must be initialized or bugs will be had; and the math is
	    // simpler with dimensions all equal to 1.
	    MFloatArray offset;
	    offset.append(0);
	    offset.append(0);
	    offset.append(0);
	    MFnFloatArrayData offsetCreator;
	    MObject data = offsetCreator.create(offset);
	    partVolumeFn.findPlug("inOffset").setValue(data);
	    partVolumeFn.findPlug("dimensionsW").setValue(1);
	    partVolumeFn.findPlug("dimensionsH").setValue(1);
	    partVolumeFn.findPlug("dimensionsD").setValue(1);
        }

        // doIt so that we can access the fullPathName
        status = myDagModifier.doIt();
        CHECK_MSTATUS_AND_RETURN_IT(status);

        // assign shader
        status = myDagModifier.commandToExecute(
                "assignSG " + partVolumeFn.fullPathName() + " " + partVolumeFn.fullPathName()
                );
        CHECK_MSTATUS_AND_RETURN_IT(status);
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

bool
AssetSyncOutputObject::isUndoable() const
{
    return true;
}
