#include "AssetSubCommandSync.h"

#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>

#include "AssetNode.h"
#include "AssetNodeOptions.h"
#include "SyncAttribute.h"
#include "SyncOutputObject.h"
#include "SyncOutputMaterial.h"
#include "SyncOutputInstance.h"

AssetSubCommandSync::AssetSubCommandSync(
        const MObject &assetNodeObj
        ) :
    SubCommandAsset(assetNodeObj),
    mySyncAll(true),
    mySyncAttributes(false),
    mySyncOutputs(false),
    mySyncOutputHidden(false),
    mySyncOutputTemplatedGeos(false)
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

void
AssetSubCommandSync::setSyncAttributes()
{
    mySyncAll = false;
    mySyncAttributes = true;
}

void
AssetSubCommandSync::setSyncOutputs()
{
    mySyncAll = false;
    mySyncOutputs = true;
}

void
AssetSubCommandSync::setSyncOutputHidden()
{
    mySyncOutputHidden = true;
}

void
AssetSubCommandSync::setSyncOutputTemplatedGeos()
{
    mySyncOutputTemplatedGeos = true;
}

void
AssetSubCommandSync::deleteMaterials(MPlug &materialPlug)
{
        MObject shaderObj = SyncOutputMaterial::findShader(materialPlug);
        if(shaderObj.isNull())
        {
            return;
        }

        MObject shadingGroupObj = SyncOutputMaterial::findShadingGroup(shaderObj);
        myDagModifier.deleteNode(shaderObj);
        if(!shadingGroupObj.isNull())
        {
             myDagModifier.deleteNode(shadingGroupObj);
        }
	myDagModifier.doIt();
}
MStatus
AssetSubCommandSync::doIt()
{
    MStatus status;

    MFnDagNode assetNodeFn(myAssetNodeObj, &status);

    // save selection
    MSelectionList oldSelection;
    MGlobal::getActiveSelectionList(oldSelection);
    // attributes
    if(mySyncAll || mySyncAttributes)
    {
        SubCommand* syncOutput = new SyncAttribute(myAssetNodeObj);
        syncOutput->doIt();

        myAssetSyncs.push_back(syncOutput);
    }

    // outputs
    if(mySyncAll || mySyncOutputs)
    {
        // Delete all children nodes. This way the sync will completely recreate
        // the connections. Otherwise, output connections may not be connected to
        // the right output node. For example, if parts were inserted in the
        // middle.
        {
            for(unsigned int i = 0; i < assetNodeFn.childCount(); i++)
            {
                MObject childNode = assetNodeFn.child(i);
                // Can't use deleteNode() here, because it could delete the parent
                // node as well.
                MFnDagNode childFnDag(childNode);
                myDagModifier.commandToExecute("delete " + childFnDag.fullPathName());
            }

            // delete all the materials
            MPlug materialsPlug = assetNodeFn.findPlug(AssetNode::outputMaterials, true);
            for(unsigned int i = 0; i < materialsPlug.numElements(); i++)
            {
                MPlug materialPlug = materialsPlug.elementByPhysicalIndex(i);

		// Note that myDagModifier::doIt is invoked in deleteMaterials
		// It's extremely likely that all the color outputs are still pointing
		// at the same shader. So as soon as we find an output that has a shader
		// it will be deleted, and the remaining deleteMaterials will have nothing
		// to delete, but why take chances

		MPlug shadingPlug = materialPlug.child(AssetNode::outputMaterialDiffuseColor);
		deleteMaterials(shadingPlug);
	        shadingPlug = materialPlug.child(AssetNode::outputMaterialAmbientColor);
		deleteMaterials(shadingPlug);
		shadingPlug = materialPlug.child(AssetNode::outputMaterialSpecularColor);
		deleteMaterials(shadingPlug);
	        shadingPlug = materialPlug.child(AssetNode::outputMaterialAlphaColor);
		deleteMaterials(shadingPlug);

		// there might also be a file texture connected for the diffuse color
		// delete it, and the downstream shader too, it it's not already gone
		MPlug texturePlug =  materialPlug.child(AssetNode::outputMaterialTexturePath);
		MObject textureObj = SyncOutputMaterial::findFileTexture(texturePlug);
		if(!textureObj.isNull()) {
		    MFnDependencyNode textureFn(textureObj);
		    MString shadingPlugName("outColor");
		    shadingPlug = textureFn.findPlug(shadingPlugName, true);
		    deleteMaterials(shadingPlug);
		    myDagModifier.deleteNode(textureObj);
		    status = myDagModifier.doIt();
                    CHECK_MSTATUS_AND_RETURN_IT(status);
		}
            }

            // Call doIt() here so that the delete commands are actually executed.
            status = myDagModifier.doIt();
            CHECK_MSTATUS_AND_RETURN_IT(status);
        }

        // Asset
        {
	  MPlug assetTranslate = assetNodeFn.findPlug(AssetNode::outputAssetTranslate, true);
            MPlug assetRotate = assetNodeFn.findPlug(AssetNode::outputAssetRotate, true);
            MPlug assetScale = assetNodeFn.findPlug(AssetNode::outputAssetScale, true);

            MPlug transformTranslate = assetNodeFn.findPlug(MPxTransform::translate, true);
            MPlug transformRotate = assetNodeFn.findPlug(MPxTransform::rotate, true);
            MPlug transformScale = assetNodeFn.findPlug(MPxTransform::scale, true);

            AssetNodeOptions::AccessorFn options(assetNodeOptionsDefinition, assetNodeFn);
            if(options.useAssetObjectTransform())
            {
                if(Util::plugSource(transformTranslate).isNull())
                    myDagModifier.connect(assetTranslate, transformTranslate);
                if(Util::plugSource(transformRotate).isNull())
                    myDagModifier.connect(assetRotate, transformRotate);
                if(Util::plugSource(transformScale).isNull())
                    myDagModifier.connect(assetScale, transformScale);
            }
            else
            {
                if(Util::plugSource(transformTranslate) == assetTranslate)
                    myDagModifier.disconnect(assetTranslate, transformTranslate);
                if(Util::plugSource(transformRotate) == assetRotate)
                    myDagModifier.disconnect(assetRotate, transformRotate);
                if(Util::plugSource(transformScale) == assetScale)
                    myDagModifier.disconnect(assetScale, transformScale);
            }
        }

        status = myDagModifier.doIt();
        CHECK_MSTATUS_AND_RETURN_IT(status);

        // Objects
        MPlug objectsPlug = assetNodeFn.findPlug(AssetNode::outputObjects, true);
        unsigned int objCount = objectsPlug.evaluateNumElements(&status);
        CHECK_MSTATUS_AND_RETURN_IT(status);

	int numGeosOutput = 0;

        for(unsigned int i=0; i < objCount; i++)
        {
            MPlug elemPlug = objectsPlug[i];

            MPlug visibilityPlug = elemPlug.child(AssetNode::outputVisibility);
            bool visible = visibilityPlug.asBool();

            MPlug instancedPlug = elemPlug.child(AssetNode::outputIsInstanced);
            bool instanced = instancedPlug.asBool();

            if(mySyncOutputHidden || visible || instanced)
            {
                SubCommand* syncOutput = new SyncOutputObject(elemPlug,
                        myAssetNodeObj, visible, mySyncOutputTemplatedGeos);
                if(syncOutput->doIt() == MS::kSuccess)
		    numGeosOutput++;

                myAssetSyncs.push_back(syncOutput);
            }
        }

        // instancers
        MPlug instancersPlug = assetNodeFn.findPlug(AssetNode::outputInstancers, true);
        unsigned int instCount = instancersPlug.numElements(&status);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        for(unsigned int i=0; i < instCount; i++)
        {
            MPlug elemPlug = instancersPlug[i];

            SubCommand* syncOutput = new SyncOutputInstance(elemPlug, i, myAssetNodeObj);
            if(syncOutput->doIt() == MS::kSuccess)
	        numGeosOutput++;

            myAssetSyncs.push_back(syncOutput);
        }
	if(numGeosOutput == 0) {
	    getAssetNode()->setExtraAutoSync(true);
        } else {
	    getAssetNode()->setExtraAutoSync(false);
	}
    }

    // restore old selection
    MGlobal::setActiveSelectionList(oldSelection);

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
