#include "SyncOutputObject.h"

#include <maya/MFloatArray.h>
#include <maya/MGlobal.h>
#include <maya/MPlug.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>
#include <maya/MDagPath.h>

#include "AssetNode.h"
#include "FluidGridConvert.h"
#include "SyncOutputGeometryPart.h"
#include "SyncOutputInstance.h"

#if MAYA_API_VERSION >= 201400
        #include <maya/MFnFloatArrayData.h>
#endif

SyncOutputObject::SyncOutputObject(
        const MPlug &outputPlug,
        const MObject &assetNodeObj,
        const bool visible,
        const bool syncTemplatedGeos)
    : myOutputPlug(outputPlug),
      myAssetNodeObj(assetNodeObj),
      myVisible(visible),
      mySyncTemplatedGeos(syncTemplatedGeos)
{}

SyncOutputObject::~SyncOutputObject()
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
SyncOutputObject::doIt()
{
    MStatus status;
    // Create our parts.
    // An object just contains a number of parts, and no
    // other information.
    MFnDependencyNode assetNodeFn(myAssetNodeObj, &status);

    MObject objectTransform = myDagModifier.createNode("transform", myAssetNodeObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // rename objectTransform
    MPlug objectNamePlug = myOutputPlug.child(AssetNode::outputObjectName);

    MString objectName = objectNamePlug.asString();
    if(objectName.length())
    {
        objectName = Util::sanitizeStringForNodeName(objectName);
    }
    else
    {
        objectName = "emptyObject";
    }
    status = myDagModifier.renameNode(objectTransform, objectName);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    CHECK_MSTATUS_AND_RETURN_IT(myDagModifier.doIt());

    MFnDependencyNode objectTransformFn(objectTransform);

    // connect objectTransform attributes
    {
        MPlug transformPlug = myOutputPlug.child(AssetNode::outputObjectTransform);

        MPlug srcPlug;
        MPlug dstPlug;

        srcPlug = transformPlug.child(AssetNode::outputObjectTranslate);
        dstPlug = objectTransformFn.findPlug("translate");
        status = myDagModifier.connect(srcPlug, dstPlug);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        srcPlug = transformPlug.child(AssetNode::outputObjectRotate);
        dstPlug = objectTransformFn.findPlug("rotate");
        status = myDagModifier.connect(srcPlug, dstPlug);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        srcPlug = transformPlug.child(AssetNode::outputObjectScale);
        dstPlug = objectTransformFn.findPlug("scale");
        status = myDagModifier.connect(srcPlug, dstPlug);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        CHECK_MSTATUS_AND_RETURN_IT(myDagModifier.doIt());
    }

    if(!myVisible)
    {
        status = myDagModifier.newPlugValueBool(
                objectTransformFn.findPlug("visibility"),
                false
                );
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    MPlug geosPlug = myOutputPlug.child(AssetNode::outputGeos);
    int geoCount = geosPlug.evaluateNumElements(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    for(int ii = 0; ii < geoCount; ii++)
    {
        MPlug geoPlug = geosPlug[ii];

        MObject geoTransform = MObject::kNullObj;
        MObject partParent = objectTransform;

        MPlug isTemplatedPlug = geoPlug.child(AssetNode::outputGeoIsTemplated);
        MPlug isDisplayGeoPlug = geoPlug.child(AssetNode::outputGeoIsDisplayGeo);
        if(mySyncTemplatedGeos || !isTemplatedPlug.asBool() || isDisplayGeoPlug.asBool())
        {
            if(geoCount > 1)
            {
                geoTransform = myDagModifier.createNode("transform", objectTransform, &status);
                CHECK_MSTATUS_AND_RETURN_IT(status);

                // rename geoTransform
                MPlug geoNamePlug = geoPlug.child(AssetNode::outputGeoName);
                MString geoName = geoNamePlug.asString();
                if(geoName.length())
                {
                    geoName = Util::sanitizeStringForNodeName(geoName);
                }
                else
                {
                    geoName = "emptyGeo";
                }
                status = myDagModifier.renameNode(geoTransform, geoName);
                CHECK_MSTATUS_AND_RETURN_IT(status);
                CHECK_MSTATUS_AND_RETURN_IT(myDagModifier.doIt());

                partParent = geoTransform;
            }

            MPlug partsPlug = geoPlug.child(AssetNode::outputParts);
            int partCount = partsPlug.evaluateNumElements(&status);
            CHECK_MSTATUS_AND_RETURN_IT(status);
            for(int jj=0; jj<partCount; jj++)
            {
                SubCommand* sync = new SyncOutputGeometryPart(partsPlug[jj], partParent);
                sync->doIt();
                myAssetSyncs.push_back(sync);
            }
        }
    }

#if MAYA_API_VERSION >= 201400
    createFluidShape(objectTransform);
#endif

    return MStatus::kSuccess;
}

#if MAYA_API_VERSION >= 201400
MStatus
SyncOutputObject::createFluidShape(const MObject &objectTransform)
{
    MStatus status;

    MPlug geosPlug = myOutputPlug.child(AssetNode::outputGeos);
    int geoCount = geosPlug.evaluateNumElements(&status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    for(int ii = 0; ii < geoCount; ii++)
    {
        MPlug geoPlug = geosPlug[ii];

        MPlug partsPlug = geoPlug.child(AssetNode::outputParts);
        int partCount = partsPlug.evaluateNumElements(&status);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        // Find a reference volume
        MPlug referenceVolumePlug;
        for(int jj=0; jj<partCount; jj++)
        {
            MPlug outputVolumePlug
                = partsPlug[jj].child(AssetNode::outputPartVolume);
            MPlug outputVolumeNamePlug
                = outputVolumePlug.child(AssetNode::outputPartVolumeName);

            MString outputVolumeName = outputVolumeNamePlug.asString();

            // TODO: Can't use vel as a reference volume yet. The reference
            // volume's resolution is connected to the grid converter. vel is
            // face-centered, so the resolution is slightly off.
            if(outputVolumeName == "density"
                    || outputVolumeName == "temperature"
                    || outputVolumeName == "fuel")
            {
                referenceVolumePlug = outputVolumePlug;
                break;
            }
        }

        // Skip if no volume can be used as reference
        if(referenceVolumePlug.isNull())
            continue;

        MPlug referenceVolumeNamePlug
            = referenceVolumePlug.child(AssetNode::outputPartVolumeName);
        MString referenceVolumeName = referenceVolumeNamePlug.asString();

        // Get the reference resolution
        int referenceResolution[3];
        {
            MPlug referenceVolumeResPlug
                = referenceVolumePlug.child(AssetNode::outputPartVolumeRes);
            MObject referenceVolumeResObj
                = referenceVolumeResPlug.asMObject();
            MFnFloatArrayData referenceVolumeResData(
                    referenceVolumeResObj, &status
                    );
            referenceResolution[0] = referenceVolumeResData[0];
            referenceResolution[1] = referenceVolumeResData[1];
            referenceResolution[2] = referenceVolumeResData[2];
        }

        // Map each volume part to a grid in a fluidShape.
        MPlug densityVolumePlug;
        MPlug velVolumePlug[3];
        MPlug temperatureVolumePlug;
        MPlug fuelVolumePlug;
        for(int jj=0; jj<partCount; jj++)
        {
            MPlug outputVolumePlug
                = partsPlug[jj].child(AssetNode::outputPartVolume);

            MPlug outputVolumeNamePlug
                = outputVolumePlug.child(AssetNode::outputPartVolumeName);
            MString outputVolumeName = outputVolumeNamePlug.asString();

            // Check known volume name
            MPlug *mappedVolumePlug = NULL;
            if(densityVolumePlug.isNull()
                    && outputVolumeName == "density")
                mappedVolumePlug = &densityVolumePlug;
            else if(temperatureVolumePlug.isNull()
                    && outputVolumeName == "temperature")
                mappedVolumePlug = &temperatureVolumePlug;
            else if(fuelVolumePlug.isNull()
                    && outputVolumeName == "fuel")
                mappedVolumePlug = &fuelVolumePlug;
            else if(velVolumePlug[0].isNull()
                    && outputVolumeName == "vel.x")
                mappedVolumePlug = &velVolumePlug[0];
            else if(velVolumePlug[1].isNull()
                    && outputVolumeName == "vel.y")
                mappedVolumePlug = &velVolumePlug[1];
            else if(velVolumePlug[2].isNull()
                    && outputVolumeName == "vel.z")
                mappedVolumePlug = &velVolumePlug[2];

            // Skip if we don't recognize the volume name
            if(!mappedVolumePlug)
                continue;

            // Get volume resolution
            int volumeResolution[3];
            {
                MPlug outputVolumeResPlug
                    = outputVolumePlug.child(AssetNode::outputPartVolumeRes);
                MObject outputVolumeResObj = outputVolumeResPlug.asMObject();
                MFnFloatArrayData outputVolumeResData(
                        outputVolumeResObj , &status
                        );

                volumeResolution[0] = outputVolumeResData[0];
                volumeResolution[1] = outputVolumeResData[1];
                volumeResolution[2] = outputVolumeResData[2];
            }

            // Check against reference resolution
            bool matchReferenceResolution
                = referenceResolution[0] == volumeResolution[0]
                && referenceResolution[1] == volumeResolution[1]
                && referenceResolution[2] == volumeResolution[2];

            // Skip volumes that don't have the same resolution as the
            // reference volume.
            if(!matchReferenceResolution)
            {
                DISPLAY_WARNING(
                        "Cannot map volume from:\n"
                        "    name: ^1s\n"
                        "    resolution: ^2s\n"
                        "    ^3s\n"
                        "to a fluidShape, because the resolution does not "
                        "match the reference volume:\n"
                        "    name: ^4s\n"
                        "    resolution: ^5s\n"
                        "    ^6s\n",
                        outputVolumeName,
                        MString() + volumeResolution[0] + ", "
                        + volumeResolution[1] + ", "
                        + volumeResolution[2],
                        outputVolumePlug.name(),
                        referenceVolumeName,
                        MString() + referenceResolution[0] + ", "
                        + referenceResolution[1] + ", "
                        + referenceResolution[2],
                        referenceVolumePlug.name()
                        );

                continue;
            }

            *mappedVolumePlug = outputVolumePlug;
        }

        MObject fluidTransformObj = myDagModifier.createNode(
                "transform",
                objectTransform,
                &status
                );
        CHECK_MSTATUS_AND_RETURN_IT(status);
        status  = myDagModifier.renameNode(fluidTransformObj, "fluid");
        CHECK_MSTATUS_AND_RETURN_IT(status);

        MObject fluidShapeObj = myDagModifier.createNode(
                "fluidShape",
                fluidTransformObj,
                &status
                );
        CHECK_MSTATUS_AND_RETURN_IT(status);

        MFnDagNode partVolumeFn(fluidShapeObj, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        MFnDagNode partFluidTransformFn(fluidTransformObj, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        // density
        if(!densityVolumePlug.isNull())
        {
            status = myDagModifier.connect(
                    densityVolumePlug.child(AssetNode::outputPartVolumeGrid),
                    partVolumeFn.findPlug("inDensity")
                    );
            CHECK_MSTATUS_AND_RETURN_IT(status);

            status = myDagModifier.newPlugValueInt(
                    partVolumeFn.findPlug("densityMethod"),
                    2
                    );
            CHECK_MSTATUS_AND_RETURN_IT(status);
        }

        // temperature
        if(!temperatureVolumePlug.isNull())
        {
            status = myDagModifier.connect(
                    temperatureVolumePlug.child(AssetNode::outputPartVolumeGrid),
                    partVolumeFn.findPlug("inTemperature")
                    );
            CHECK_MSTATUS_AND_RETURN_IT(status);
            status = myDagModifier.newPlugValueInt(
                    partVolumeFn.findPlug("temperatureMethod"),
                    2
                    );
            CHECK_MSTATUS_AND_RETURN_IT(status);
        }

        // fuel
        if(!fuelVolumePlug.isNull())
        {
            status = myDagModifier.connect(
                    fuelVolumePlug.child(AssetNode::outputPartVolumeGrid),
                    partVolumeFn.findPlug("inReaction")
                    );
            CHECK_MSTATUS_AND_RETURN_IT(status);
            status = myDagModifier.newPlugValueInt(
                    partVolumeFn.findPlug("fuelMethod"),
                    2
                    );
            CHECK_MSTATUS_AND_RETURN_IT(status);
        }

        if(!velVolumePlug[0].isNull()
                && !velVolumePlug[1].isNull()
                && !velVolumePlug[2].isNull())
        {
            MObject velConverter = ((MDGModifier&)myDagModifier).createNode(
                    "houdiniFluidGridConvert",
                    &status
                    );
            CHECK_MSTATUS_AND_RETURN_IT(status);

            MFnDependencyNode velConverterFn(velConverter, &status);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            myDagModifier.newPlugValueShort(
                    velConverterFn.findPlug(
                        FluidGridConvert::conversionMode
                        ),
                    1);

            MPlug srcPlug;
            MPlug dstPlug;

            // resolution
            srcPlug = referenceVolumePlug.child(AssetNode::outputPartVolumeRes);
            dstPlug = velConverterFn.findPlug(FluidGridConvert::resolution);
            status = myDagModifier.connect(srcPlug, dstPlug);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            // grids
            srcPlug = velVolumePlug[0].child(
                    AssetNode::outputPartVolumeGrid
                    );
            dstPlug = velConverterFn.findPlug("inGridX");
            status = myDagModifier.connect(srcPlug, dstPlug);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            srcPlug = velVolumePlug[1].child(
                    AssetNode::outputPartVolumeGrid
                    );
            dstPlug = velConverterFn.findPlug("inGridY");
            status = myDagModifier.connect(srcPlug, dstPlug);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            srcPlug = velVolumePlug[2].child(
                    AssetNode::outputPartVolumeGrid
                    );
            dstPlug = velConverterFn.findPlug("inGridZ");
            status = myDagModifier.connect(srcPlug, dstPlug);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            // converter to fluidShape
            srcPlug = velConverterFn.findPlug(FluidGridConvert::outGrid);
            dstPlug = partVolumeFn.findPlug("inVelocity");
            status = myDagModifier.connect(srcPlug, dstPlug);
            CHECK_MSTATUS_AND_RETURN_IT(status);
        }

        // Connect the transform, playFromCache; set inOffset and dimensions
        {
            MPlug srcPlug;
            MPlug dstPlug;

            MPlug referenceTransform = referenceVolumePlug.child(
                    AssetNode::outputPartVolumeTransform
                    );

            srcPlug = referenceTransform.child(
                    AssetNode::outputPartVolumeTranslate
                    );
            dstPlug = partFluidTransformFn.findPlug("translate");
            status = myDagModifier.connect(srcPlug, dstPlug);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            srcPlug = referenceVolumePlug.child(
                    AssetNode::outputPartVolumeRes
                    );
            dstPlug = partVolumeFn.findPlug("inResolution");
            status = myDagModifier.connect(srcPlug, dstPlug);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            srcPlug = referenceTransform.child(
                    AssetNode::outputPartVolumeRotate
                    );
            dstPlug = partFluidTransformFn.findPlug("rotate");
            status = myDagModifier.connect(srcPlug, dstPlug);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            srcPlug = referenceTransform.child(
                    AssetNode::outputPartVolumeScale
                    );
            dstPlug = partFluidTransformFn.findPlug("scale");
            status = myDagModifier.connect(srcPlug, dstPlug);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            // time1.outTime -> fluidShape.currentTime
            {
                MObject srcNode = Util::findNodeByName("time1");
                srcPlug = MFnDependencyNode(srcNode).findPlug("outTime");
                dstPlug = partVolumeFn.findPlug("currentTime");
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
SyncOutputObject::undoIt()
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
SyncOutputObject::redoIt()
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
SyncOutputObject::isUndoable() const
{
    return true;
}
