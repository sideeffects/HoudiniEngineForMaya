#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MSelectionList.h>
#include <maya/MStatus.h>
#include <maya/MItDependencyGraph.h>

#include "Asset.h"
#include "AssetNode.h"
#include "OutputPartInstancerNode.h"
#include "AssetCommand.h"
#include "SubCommand.h"
#include "AssetSubCommandLoadAsset.h"
#include "AssetSubCommandSync.h"
#include "util.h"

#define kListAssetsFlag "-ls"
#define kListAssetsFlagLong "-listAssets"
#define kLoadAssetFlag "-la"
#define kLoadAssetFlagLong "-loadAsset"
#define kLockAssetFlag "-lk"
#define kLockAssetFlagLong "-lockAsset"
#define kSyncFlag "-syn"
#define kSyncFlagLong "-sync"
#define kResetSimulationFlag "-rs"
#define kResetSimulationFlagLong "-resetSimulation"
#define kCookMessagesFlag "-cm"
#define kCookMessagesFlagLong "-cookMessages"
#define kReloadAssetFlag "-rl"
#define kReloadAssetFlagLong "-reloadAsset"
#define kSyncAttributesFlag "-sa"
#define kSyncAttributesFlagLong "-syncAttributes"
#define kSyncOutputsFlag "-so"
#define kSyncOutputsFlagLong "-syncOutputs"
#define kSyncHiddenFlag "-shi"
#define kSyncHiddenFlagLong "-syncHidden"
#define kSyncTemplatedGeosFlag "-stm"
#define kSyncTemplatedGeosFlagLong "-syncTemplatedGeos"

class AssetSubCommandResetSimulation : public SubCommandAsset
{
    public:
        AssetSubCommandResetSimulation(const MObject &assetNodeObj) :
            SubCommandAsset(assetNodeObj)
        {
        }

        virtual MStatus doIt()
        {
            GET_COMMAND_ASSET_OR_RETURN_FAIL();

            asset->resetSimulation();

            return MStatus::kSuccess;
        }
};

class AssetSubCommandCookMessages : public SubCommandAsset
{
    public:
        AssetSubCommandCookMessages(const MObject &assetNodeObj) :
            SubCommandAsset(assetNodeObj)
        {
        }

        virtual MStatus doIt()
        {
            GET_COMMAND_ASSET_OR_RETURN_FAIL();

            MPxCommand::setResult(asset->getCookMessages());

            return MStatus::kSuccess;
        }
};

class AssetSubCommandListAssets : public SubCommand
{
    public:
        AssetSubCommandListAssets(const MString &otlFilePath) :
            myOTLFilePath(otlFilePath)
        {
        }

        virtual MStatus doIt()
        {
            HAPI_Result hapiResult;

            int libraryId = -1;
            hapiResult = HAPI_LoadAssetLibraryFromFile(
                    Util::theHAPISession.get(),
                    myOTLFilePath.asChar(),
                    true,
                    &libraryId
                    );
            if(HAPI_FAIL(hapiResult))
            {
                DISPLAY_ERROR("Could not load OTL file: ^1s", myOTLFilePath);
                DISPLAY_ERROR_HAPI_STATUS_CALL();

                return MStatus::kFailure;
            }

            int assetCount = 0;
            hapiResult = HAPI_GetAvailableAssetCount(
                    Util::theHAPISession.get(),
                    libraryId,
                    &assetCount
                    );
            CHECK_HAPI_AND_RETURN(hapiResult, MStatus::kFailure);

            std::vector<HAPI_StringHandle> assetNamesSH(assetCount);
            hapiResult = HAPI_GetAvailableAssets(
                    Util::theHAPISession.get(),
                    libraryId,
                    &assetNamesSH.front(),
                    assetNamesSH.size()
                    );
            CHECK_HAPI_AND_RETURN(hapiResult, MStatus::kFailure);

            for(unsigned int i = 0; i < assetNamesSH.size(); i++)
            {
                MPxCommand::appendToResult(Util::HAPIString(assetNamesSH[i]));
            }

            return MStatus::kSuccess;
        }

    protected:
        MString myOTLFilePath;
};

class AssetSubCommandLockAsset : public SubCommandAsset
{
    bool myLockOn;
    MDagModifier myDagModifier;

public:
    AssetSubCommandLockAsset(const MObject &assetNodeObj, bool lockOn) 
    : SubCommandAsset(assetNodeObj)
    , myLockOn(lockOn)
    {
    }

    virtual MStatus doIt() override
    {
        GET_COMMAND_ASSET_OR_RETURN_FAIL();

        MStatus status;

        MFnDependencyNode assetNodeFn(myAssetNodeObj);
        MPlug lockAsset = assetNodeFn.findPlug(AssetNode::lockAsset, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        bool lockOnState = lockAsset.asBool();

        if(myLockOn == lockOnState)
        {
            // nothing to do
            return MStatus::kSuccess;
        }
        myDagModifier.newPlugValueBool(lockAsset, myLockOn);

        MItDependencyGraph iter(myAssetNodeObj,
            MFn::kMessageAttribute, MItDependencyGraph::kUpstream,
            MItDependencyGraph::kBreadthFirst, MItDependencyGraph::kPlugLevel, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        CHECK_MSTATUS_AND_RETURN_IT(iter.enablePruningOnFilter());

        for(; !iter.isDone(); iter.next())
        {
            MPlug myNodeMsgPlug = iter.thisPlug(&status);
            const unsigned int msgIx = myNodeMsgPlug.logicalIndex(&status);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            MObject otherNode = myNodeMsgPlug.source().node(&status);
            CHECK_MSTATUS_AND_RETURN_IT(status);
            
            MPlug myNodeOutputPartPlug = myNodeMsgPlug.array().parent(&status);
            CHECK_MSTATUS_AND_RETURN_IT(status);
            
            MFnDependencyNode otherNodeFn(otherNode, &status);
            
            if (otherNodeFn.typeId() == OutputPartInstancerNode::typeId)
            {
                if(myLockOn)
                {
                    MPlug pointDataPlug = otherNodeFn.findPlug(OutputPartInstancerNode::pointData, true, &status);
                    CHECK_MSTATUS_AND_RETURN_IT(status);
                    status = myDagModifier.disconnect(pointDataPlug.source(), pointDataPlug);
                    CHECK_MSTATUS_AND_RETURN_IT(status);
                }
                else
                {
                    MPlug sourcePlug = myNodeOutputPartPlug.child(AssetNode::outputPartInstancer, &status);
                    CHECK_MSTATUS_AND_RETURN_IT(status);
                    sourcePlug = sourcePlug.child(AssetNode::outputPartInstancerArrayData, &status);
                    CHECK_MSTATUS_AND_RETURN_IT(status);

                    MPlug pointDataPlug = otherNodeFn.findPlug(OutputPartInstancerNode::pointData, false, &status);
                    CHECK_MSTATUS_AND_RETURN_IT(status);
                    myDagModifier.connect(sourcePlug, pointDataPlug);
                    CHECK_MSTATUS_AND_RETURN_IT(status);
                }
            }
            else if(otherNode.hasFn(MFn::kMesh))
            {
                if(myLockOn)
                {
                    MPlug inMeshPlug = otherNodeFn.findPlug("inMesh", true, &status);
                    status = myDagModifier.disconnect(inMeshPlug.source(), inMeshPlug);
                    CHECK_MSTATUS_AND_RETURN_IT(status);
                }
                else
                {
                    MPlug inMeshPlug = otherNodeFn.findPlug("inMesh", false, &status);
                    MPlug sourcePlug = myNodeOutputPartPlug.child(AssetNode::outputPartMesh, &status);
                    CHECK_MSTATUS_AND_RETURN_IT(status);
                    sourcePlug = sourcePlug.child(AssetNode::outputPartMeshData, &status);
                    CHECK_MSTATUS_AND_RETURN_IT(status);
                    status = myDagModifier.connect(sourcePlug, inMeshPlug);
                    CHECK_MSTATUS_AND_RETURN_IT(status);
                }
            }
            else if(otherNode.hasFn(MFn::kNurbsCurve))
            {
                if(myLockOn)
                {
                    MPlug createPlug = otherNodeFn.findPlug("create", true, &status);
                    status = myDagModifier.disconnect(createPlug.source(), createPlug);
                    CHECK_MSTATUS(status);
                }
                else
                {
                    MPlug createPlug = otherNodeFn.findPlug("create", false, &status);
                    MPlug sourcePlug = myNodeOutputPartPlug.child(AssetNode::outputPartCurves, &status);
                    sourcePlug = sourcePlug.elementByLogicalIndex(msgIx, &status);
                    CHECK_MSTATUS_AND_RETURN_IT(status);

                    status = myDagModifier.connect(sourcePlug, createPlug);
                    CHECK_MSTATUS(status);
                }
            }
            else
            {
                DISPLAY_WARNING("Unexpected message connection to node ^1s", otherNodeFn.name());
            }
        }
        status = myDagModifier.doIt();
        CHECK_MSTATUS_AND_RETURN_IT(status);

        return MStatus::kSuccess;
    }

    MStatus redoIt() override
    {
        myDagModifier.doIt();
        return MStatus::kSuccess;
    }

    MStatus undoIt() override
    {
        myDagModifier.undoIt();
        return MStatus::kSuccess;
    }

    bool isUndoable() const override
    {
        return true;
    }
};

void* AssetCommand::creator()
{
    return new AssetCommand();
}

MSyntax
AssetCommand::newSyntax()
{
    MSyntax syntax;

    // -listAssets list assets in an OTL file
    CHECK_MSTATUS(syntax.addFlag(kListAssetsFlag, kListAssetsFlagLong,
                MSyntax::kString));

    // -loadAsset load an otl file
    // expected arguments: otl_file_name - the name of the otl file to load
    CHECK_MSTATUS(syntax.addFlag(kLoadAssetFlag, kLoadAssetFlagLong,
                MSyntax::kString,
                MSyntax::kString));

    // -lockAsset will enable or disable 'locked' mode on the asset node, 
    // which prevents any cooking of the asset node until the mode is disabled.
    // expected arguments:
    //        asset node
    //        on/off
    CHECK_MSTATUS(syntax.addFlag(kLockAssetFlag, kLockAssetFlagLong,
        MSyntax::kSelectionItem, MSyntax::kBoolean));

    // -sync synchronize the Maya nodes with the asset's state
    // expected arguments:
    //        asset node
    CHECK_MSTATUS(syntax.addFlag(kSyncFlag, kSyncFlagLong,
                MSyntax::kSelectionItem));

    // -resetSimulation resets the simulation state for an asset.  This will clear
    // the DOPs cache for the asset.
    CHECK_MSTATUS(syntax.addFlag(kResetSimulationFlag,
                                 kResetSimulationFlagLong,
                                 MSyntax::kSelectionItem));

    // -cookMessages get the cook messages for an asset
    CHECK_MSTATUS(syntax.addFlag(kCookMessagesFlag,
                                 kCookMessagesFlagLong,
                                 MSyntax::kSelectionItem));

    // -reloadAsset will unload and immediate reload the asset.  If an otl file
    // has changed due to an edit in Houdini, this should pick up the change
        // Note that this won't refresh the AE, you need to that separately after
        // running this, with refreshEditorTemplates
    CHECK_MSTATUS(syntax.addFlag(kReloadAssetFlag,
                                 kReloadAssetFlagLong,
                                 MSyntax::kString));

    CHECK_MSTATUS(syntax.addFlag(kSyncAttributesFlag, kSyncAttributesFlagLong));
    CHECK_MSTATUS(syntax.addFlag(kSyncOutputsFlag, kSyncOutputsFlagLong));

    // -syncHidden will cause hidden objects to be sync'ed
    CHECK_MSTATUS(syntax.addFlag(kSyncHiddenFlag,
                                 kSyncHiddenFlagLong,
                                 MSyntax::kNoArg));

    // -syncTemplatedGeos will cause templated geos to be sync'ed
    CHECK_MSTATUS(syntax.addFlag(kSyncTemplatedGeosFlag,
                                 kSyncTemplatedGeosFlagLong,
                                 MSyntax::kNoArg));

    return syntax;
}

AssetCommand::AssetCommand() :
    mySubCommand(NULL)
{
}

AssetCommand::~AssetCommand()
{
    delete mySubCommand;
}

MStatus
AssetCommand::parseArgs(const MArgList &args)
{
    MStatus status;
    MArgDatabase argData(syntax(), args, &status);
    if(!status)
    {
        return status;
    }

    if(!(argData.isFlagSet(kListAssetsFlag)
                ^ argData.isFlagSet(kLoadAssetFlag)
                ^ argData.isFlagSet(kLockAssetFlag)
                ^ argData.isFlagSet(kSyncFlag)
                ^ argData.isFlagSet(kResetSimulationFlag)
                ^ argData.isFlagSet(kCookMessagesFlag)
                ^ argData.isFlagSet(kReloadAssetFlag)))
    {
        displayError("Exactly one of these flags must be specified:\n"
                kLoadAssetFlagLong "\n"
                kLockAssetFlagLong "\n"
                kSyncFlagLong "\n"
                kResetSimulationFlagLong "\n"
                kCookMessagesFlagLong "\n"
                kReloadAssetFlagLong "\n");
        return MStatus::kInvalidParameter;
    }

    if(argData.isFlagSet(kListAssetsFlag))
    {
        MString otlFilePath;
        {
            status = argData.getFlagArgument(kListAssetsFlag, 0, otlFilePath);
            if(!status)
            {
                displayError("Invalid argument for \"" kListAssetsFlagLong "\".");
                return status;
            }
        }

        mySubCommand = new AssetSubCommandListAssets(
                otlFilePath
                );
    }

    if(argData.isFlagSet(kLoadAssetFlag))
    {
        MString otlFilePath;
        {
            status = argData.getFlagArgument(kLoadAssetFlag, 0, otlFilePath);
            if(!status)
            {
                displayError("Invalid argument for \"" kLoadAssetFlagLong "\".");
                return status;
            }
        }

        MString assetName;
        {
            status = argData.getFlagArgument(kLoadAssetFlag, 1, assetName);
            if(!status)
            {
                displayError("Invalid argument for \"" kLoadAssetFlagLong "\".");
                return status;
            }
        }

        mySubCommand = new AssetSubCommandLoadAsset(
                otlFilePath,
                assetName
                );
    }

    if(argData.isFlagSet(kReloadAssetFlag))
    {
        MString assetPath;
        {
            status = argData.getFlagArgument(kReloadAssetFlag, 0, assetPath);
            if(!status)
            {
                displayError("Invalid argument for \"" kReloadAssetFlagLong "\".");
                return status;
            }
        }

        MSelectionList selList;
        MObject assetNodeObj;

        selList.add(assetPath);
        selList.getDependNode(0, assetNodeObj);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        MFnDependencyNode assetNodeFn(assetNodeObj);
        AssetNode* assetNode = dynamic_cast<AssetNode*>(assetNodeFn.userNode());
        assetNode->rebuildAsset();

        mySubCommand = new AssetSubCommandSync(assetNodeObj);
    }

    if(argData.isFlagSet(kSyncFlag))
    {
        MObject assetNodeObj;
        {
            MSelectionList selection;

            status = argData.getFlagArgument(kSyncFlag, 0, selection);
            if(!status)
            {
                displayError("Invalid first argument for \"" kSyncFlagLong "\".");
                return status;
            }

            selection.getDependNode(0, assetNodeObj);
            CHECK_MSTATUS_AND_RETURN_IT(status);
        }

        AssetSubCommandSync* subCommand = new AssetSubCommandSync(
                assetNodeObj
                );

        if(argData.isFlagSet(kSyncAttributesFlag))
        {
            subCommand->setSyncAttributes();
        }

        if(argData.isFlagSet(kSyncOutputsFlag))
        {
            subCommand->setSyncOutputs();
        }

        if(argData.isFlagSet(kSyncHiddenFlag))
        {
            subCommand->setSyncOutputHidden();
        }

        if(argData.isFlagSet(kSyncTemplatedGeosFlag))
        {
            subCommand->setSyncOutputTemplatedGeos();
        }

        mySubCommand = subCommand;
    }

    if(argData.isFlagSet(kResetSimulationFlag))
    {
        MObject assetNodeObj;
        {
            MSelectionList selection;

            status = argData.getFlagArgument(kResetSimulationFlag, 0, selection);
            if(!status)
            {
                displayError("Invalid first argument for \"" kResetSimulationFlagLong "\".");
                return status;
            }

            selection.getDependNode(0, assetNodeObj);
            CHECK_MSTATUS_AND_RETURN_IT(status);
        }

        mySubCommand = new AssetSubCommandResetSimulation(assetNodeObj);
    }

    if(argData.isFlagSet(kLockAssetFlag))
    {
        MObject assetNodeObj;
        bool lockOn;
        {
            MSelectionList selection;

            status = argData.getFlagArgument(kLockAssetFlag, 0, selection);
            if(!status)
            {
                displayError("Invalid first argument for \"" kLockAssetFlagLong "\".");
                return status;
            }

            selection.getDependNode(0, assetNodeObj);
            CHECK_MSTATUS_AND_RETURN_IT(status);

            status = argData.getFlagArgument(kLockAssetFlag, 1, lockOn);
            if(!status)
            {
                displayError("Invalid second argument for \"" kLockAssetFlagLong "\".");
                return status;
            }
        }

        mySubCommand = new AssetSubCommandLockAsset(assetNodeObj, lockOn);
    }

    if(argData.isFlagSet(kCookMessagesFlag))
    {
        MObject assetNodeObj;
        {
            MSelectionList selection;

            status = argData.getFlagArgument(kCookMessagesFlag, 0, selection);
            if(!status)
            {
                displayError("Invalid first argument for \"" kCookMessagesFlagLong "\".");
                return status;
            }

            selection.getDependNode(0, assetNodeObj);
            CHECK_MSTATUS_AND_RETURN_IT(status);
        }

        mySubCommand = new AssetSubCommandCookMessages(assetNodeObj);
    }

    return MStatus::kSuccess;
}

MStatus AssetCommand::doIt(const MArgList& args)
{
    MStatus status;

    status = parseArgs(args);
    if(!status)
    {
        return status;
    }

    return mySubCommand->doIt();
}

MStatus AssetCommand::redoIt()
{
    return mySubCommand->redoIt();
}

MStatus AssetCommand::undoIt()
{
    return mySubCommand->undoIt();
}

bool AssetCommand::isUndoable() const
{
    return mySubCommand->isUndoable();
}
