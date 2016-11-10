#include "AssetSubCommandSync.h"

#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MSelectionList.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>

#include "AssetNode.h"
#include "SyncAttribute.h"
#include "SyncOutputObject.h"
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

MStatus
AssetSubCommandSync::doIt()
{
    MStatus status;

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
        MFnDagNode assetNodeFn(myAssetNodeObj, &status);

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

            // Call doIt() here so that the delete commands are actually executed.
            status = myDagModifier.doIt();
            CHECK_MSTATUS_AND_RETURN_IT(status);
        }

        // Asset
        {
            MPlug assetTranslate = assetNodeFn.findPlug(AssetNode::outputAssetTranslate);
            MPlug assetRotate = assetNodeFn.findPlug(AssetNode::outputAssetRotate);
            MPlug assetScale = assetNodeFn.findPlug(AssetNode::outputAssetScale);

            MPlug transformTranslate = assetNodeFn.findPlug(MPxTransform::translate);
            MPlug transformRotate = assetNodeFn.findPlug(MPxTransform::rotate);
            MPlug transformScale = assetNodeFn.findPlug(MPxTransform::scale);

            if(assetNodeFn.findPlug(AssetNode::useAssetObjectTransform).asBool())
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
        MPlug objectsPlug = assetNodeFn.findPlug(AssetNode::outputObjects);
        unsigned int objCount = objectsPlug.evaluateNumElements(&status);
        CHECK_MSTATUS_AND_RETURN_IT(status);

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

            SubCommand* syncOutput = new SyncOutputInstance(elemPlug, i, myAssetNodeObj);
            syncOutput->doIt();

            myAssetSyncs.push_back(syncOutput);
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
