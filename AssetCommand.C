#include <maya/MArgList.h>
#include <maya/MSelectionList.h>
#include <maya/MStatus.h>

#include "Asset.h"
#include "AssetCommand.h"
#include "AssetNode.h"
#include "AssetSubCommandLoadAsset.h"
#include "AssetSubCommandSync.h"
#include "SubCommand.h"
#include "util.h"

#define kListAssetsFlag "-ls"
#define kListAssetsFlagLong "-listAssets"
#define kLoadAssetFlag "-la"
#define kLoadAssetFlagLong "-loadAsset"
#define kSyncFlag "-syn"
#define kSyncFlagLong "-sync"
#define kResetSimulationFlag "-rs"
#define kResetSimulationFlagLong "-resetSimulation"
#define kCookMessagesFlag "-cm"
#define kCookMessagesFlagLong "-cookMessages"
#define kReloadAssetFlag "-rl"
#define kReloadAssetFlagLong "-reloadAsset"
#define kReloadNoSyncFlag "-rn"
#define kReloadNoSyncFlagLong "-reloadNoSync"
#define kSyncAttributesFlag "-sa"
#define kSyncAttributesFlagLong "-syncAttributes"
#define kSyncOutputsFlag "-so"
#define kSyncOutputsFlagLong "-syncOutputs"
#define kSyncHiddenFlag "-shi"
#define kSyncHiddenFlagLong "-syncHidden"
#define kSyncTemplatedGeosFlag "-stm"
#define kSyncTemplatedGeosFlagLong "-syncTemplatedGeos"
#define kAutoSyncIdFlag "-asi"
#define kAutoSyncIdFlagLong "-autoSyncId"
#define kParmHelpFlag "-ph"
#define kParmHelpFlagLong "-parmHelp"

const char *AssetCommand::commandName = "houdiniAsset";

class AssetSubCommandResetSimulation : public SubCommandAsset
{
public:
    AssetSubCommandResetSimulation(const MObject &assetNodeObj)
        : SubCommandAsset(assetNodeObj)
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
    AssetSubCommandCookMessages(const MObject &assetNodeObj)
        : SubCommandAsset(assetNodeObj)
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
    AssetSubCommandListAssets(const MString &otlFilePath)
        : myOTLFilePath(otlFilePath)
    {
    }

    virtual MStatus doIt()
    {
        HAPI_Result hapiResult;

        int libraryId = -1;
        hapiResult = HAPI_LoadAssetLibraryFromFile(Util::theHAPISession.get(),
                                                   myOTLFilePath.asChar(), true,
                                                   &libraryId);
        if (HAPI_FAIL(hapiResult))
        {
            DISPLAY_ERROR("Could not load OTL file: ^1s", myOTLFilePath);
            DISPLAY_ERROR_HAPI_STATUS_CALL();

            return MStatus::kFailure;
        }

        int assetCount = 0;
        hapiResult     = HAPI_GetAvailableAssetCount(
            Util::theHAPISession.get(), libraryId, &assetCount);
        CHECK_HAPI_AND_RETURN(hapiResult, MStatus::kFailure);

        std::vector<HAPI_StringHandle> assetNamesSH(assetCount);
        hapiResult = HAPI_GetAvailableAssets(Util::theHAPISession.get(),
                                             libraryId, &assetNamesSH.front(),
                                             assetNamesSH.size());
        CHECK_HAPI_AND_RETURN(hapiResult, MStatus::kFailure);

        for (unsigned int i = 0; i < assetNamesSH.size(); i++)
        {
            MPxCommand::appendToResult(Util::HAPIString(assetNamesSH[i]));
        }

        return MStatus::kSuccess;
    }

protected:
    MString myOTLFilePath;
};

class AssetSubCommandAutoSyncId : public SubCommandAsset
{
public:
    AssetSubCommandAutoSyncId(const MObject &assetNodeObj)
        : SubCommandAsset(assetNodeObj)
    {
    }

    virtual MStatus doIt()
    {
        MPxCommand::setResult(getAssetNode()->autoSyncId());

        return MStatus::kSuccess;
    }
};

class AssetSubCommandGetParmHelp : public SubCommandAsset
{
public:
    AssetSubCommandGetParmHelp(const MObject &assetNodeObj,
                               const MString &parmName)
        : SubCommandAsset(assetNodeObj), myParmName(parmName)
    {
    }

    virtual MStatus doIt()
    {
        HAPI_Result hapiResult;
        HAPI_ParmInfo parmInfo;

        MString helpString = "";

        GET_COMMAND_ASSET_OR_RETURN_FAIL();

        int nodeId = asset->getNodeInfo().id;

        hapiResult = HAPI_GetParmInfoFromName(Util::theHAPISession.get(), nodeId,
                                              myParmName.asChar(), &parmInfo);

        if (parmInfo.helpSH != 0)
            helpString = Util::HAPIString(parmInfo.helpSH);

        MPxCommand::setResult(helpString);

        return MStatus::kSuccess;
    }

protected:
    MString myParmName;
};

void *
AssetCommand::creator()
{
    return new AssetCommand();
}

MSyntax
AssetCommand::newSyntax()
{
    MSyntax syntax;

    // -listAssets list assets in an OTL file
    CHECK_MSTATUS(
        syntax.addFlag(kListAssetsFlag, kListAssetsFlagLong, MSyntax::kString));

    // -loadAsset load an otl file
    // expected arguments: otl_file_name - the name of the otl file to load
    CHECK_MSTATUS(syntax.addFlag(kLoadAssetFlag, kLoadAssetFlagLong,
                                 MSyntax::kString, MSyntax::kString));

    // -sync synchronize the Maya nodes with the asset's state
    // expected arguments:
    //        asset node
    CHECK_MSTATUS(
        syntax.addFlag(kSyncFlag, kSyncFlagLong, MSyntax::kSelectionItem));

    // -resetSimulation resets the simulation state for an asset.  This will
    // clear the DOPs cache for the asset.
    CHECK_MSTATUS(syntax.addFlag(kResetSimulationFlag, kResetSimulationFlagLong,
                                 MSyntax::kSelectionItem));

    // -cookMessages get the cook messages for an asset
    CHECK_MSTATUS(syntax.addFlag(
        kCookMessagesFlag, kCookMessagesFlagLong, MSyntax::kSelectionItem));

    // -reloadAsset will unload and immediate reload and sync the asset. If the
    // otl file has changed due to an edit in Houdini, this should pick up the
    // change. Note that this won't refresh the AE, you need to that separately
    // after running this, with refreshEditorTemplates
    CHECK_MSTATUS(syntax.addFlag(
        kReloadAssetFlag, kReloadAssetFlagLong, MSyntax::kSelectionItem));

    // -reloadNoSync will unload and immediate reload the asset.
    // If you know that the number and type of outputs has not changed.
    // Originally intended to be used when loading an asset that was
    // deferred from file load, where we either believe the outputs
    // still match the asset, or we plan to independently connect
    // the correct outputs.
    CHECK_MSTATUS(syntax.addFlag(
        kReloadNoSyncFlag, kReloadNoSyncFlagLong, MSyntax::kSelectionItem));

    CHECK_MSTATUS(syntax.addFlag(kSyncAttributesFlag, kSyncAttributesFlagLong));
    CHECK_MSTATUS(syntax.addFlag(kSyncOutputsFlag, kSyncOutputsFlagLong));

    // -syncHidden will cause hidden objects to be sync'ed
    CHECK_MSTATUS(
        syntax.addFlag(kSyncHiddenFlag, kSyncHiddenFlagLong, MSyntax::kNoArg));

    // -syncTemplatedGeos will cause templated geos to be sync'ed
    CHECK_MSTATUS(syntax.addFlag(
        kSyncTemplatedGeosFlag, kSyncTemplatedGeosFlagLong, MSyntax::kNoArg));

    CHECK_MSTATUS(syntax.addFlag(
        kAutoSyncIdFlag, kAutoSyncIdFlagLong, MSyntax::kSelectionItem));

    // -parmHelp will return the help string of the specified parm
    CHECK_MSTATUS(syntax.addFlag(
        kParmHelpFlag, kParmHelpFlagLong, MSyntax::kString,
        MSyntax::kSelectionItem));

    return syntax;
}

bool
AssetCommand::getMObjectFromFlag(const MArgDatabase &argData,
                                 const char *flag,
                                 MObject &obj,
                                 MStatus &status,
                                 const int index)
{
    MSelectionList selection;
    status = argData.getFlagArgument(flag, index, selection);
    if (!status)
    {
        MString error_message;
        error_message.format("Invalid argument for \"^1s\".", flag);
        displayError(error_message);
        return false;
    }

    status = selection.getDependNode(0, obj);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return true;
}

AssetCommand::AssetCommand() : mySubCommand(NULL) {}

AssetCommand::~AssetCommand()
{
    delete mySubCommand;
}

MStatus
AssetCommand::parseArgs(const MArgList &args)
{
    MStatus status;
    MArgDatabase argData(syntax(), args, &status);
    if (!status)
    {
        return status;
    }

    if (!(argData.isFlagSet(kListAssetsFlag) ^
          argData.isFlagSet(kLoadAssetFlag) ^
          argData.isFlagSet(kSyncFlag) ^
          argData.isFlagSet(kResetSimulationFlag) ^
          argData.isFlagSet(kCookMessagesFlag) ^
          argData.isFlagSet(kReloadAssetFlag) ^
          argData.isFlagSet(kReloadNoSyncFlag) ^
          argData.isFlagSet(kAutoSyncIdFlag) ^
          argData.isFlagSet(kParmHelpFlag)))
    {
        displayError(
            "Exactly one of these flags must be specified:\n" kLoadAssetFlagLong
            "\n" kSyncFlagLong "\n" kResetSimulationFlagLong
            "\n" kCookMessagesFlagLong "\n" kReloadNoSyncFlagLong
            "\n" kReloadAssetFlagLong "\n" kParmHelpFlag "\n");
        return MStatus::kInvalidParameter;
    }

    if (argData.isFlagSet(kListAssetsFlag))
    {
        MString otlFilePath;
        {
            status = argData.getFlagArgument(kListAssetsFlag, 0, otlFilePath);
            if (!status)
            {
                displayError("Invalid argument for \"" kListAssetsFlagLong
                             "\".");
                return status;
            }
        }

        mySubCommand = new AssetSubCommandListAssets(otlFilePath);
    }

    if (argData.isFlagSet(kLoadAssetFlag))
    {
        MString otlFilePath;
        {
            status = argData.getFlagArgument(kLoadAssetFlag, 0, otlFilePath);
            if (!status)
            {
                displayError("Invalid argument for \"" kLoadAssetFlagLong
                             "\".");
                return status;
            }
        }

        MString assetName;
        {
            status = argData.getFlagArgument(kLoadAssetFlag, 1, assetName);
            if (!status)
            {
                displayError("Invalid argument for \"" kLoadAssetFlagLong
                             "\".");
                return status;
            }
        }

        mySubCommand = new AssetSubCommandLoadAsset(otlFilePath, assetName);
    }

    if (argData.isFlagSet(kReloadAssetFlag))
    {
        MObject assetNodeObj;
        if (!getMObjectFromFlag(
                argData, kReloadAssetFlagLong, assetNodeObj, status))
            return status;

        MFnDependencyNode assetNodeFn(assetNodeObj);
        AssetNode *assetNode =
            dynamic_cast<AssetNode *>(assetNodeFn.userNode());
        assetNode->rebuildAsset();

        mySubCommand = new AssetSubCommandSync(assetNodeObj);
    }

    if (argData.isFlagSet(kReloadNoSyncFlag))
    {
        MObject assetNodeObj;
        if (!getMObjectFromFlag(
                argData, kReloadNoSyncFlagLong, assetNodeObj, status))
            return status;

        MFnDependencyNode assetNodeFn(assetNodeObj);
        AssetNode *assetNode =
            dynamic_cast<AssetNode *>(assetNodeFn.userNode());
        assetNode->rebuildAsset();

        mySubCommand = NULL;
    }

    if (argData.isFlagSet(kSyncFlag))
    {
        MObject assetNodeObj;
        if (!getMObjectFromFlag(argData, kSyncFlagLong, assetNodeObj, status))
            return status;

        AssetSubCommandSync *subCommand = new AssetSubCommandSync(assetNodeObj);

        if (argData.isFlagSet(kSyncAttributesFlag))
        {
            subCommand->setSyncAttributes();
        }

        if (argData.isFlagSet(kSyncOutputsFlag))
        {
            subCommand->setSyncOutputs();
        }

        if (argData.isFlagSet(kSyncHiddenFlag))
        {
            subCommand->setSyncOutputHidden();
        }

        if (argData.isFlagSet(kSyncTemplatedGeosFlag))
        {
            subCommand->setSyncOutputTemplatedGeos();
        }

        mySubCommand = subCommand;
    }

    if (argData.isFlagSet(kAutoSyncIdFlag))
    {
        MObject assetNodeObj;
        if (!getMObjectFromFlag(
                argData, kAutoSyncIdFlagLong, assetNodeObj, status))
            return status;

        mySubCommand = new AssetSubCommandAutoSyncId(assetNodeObj);
    }

    if (argData.isFlagSet(kResetSimulationFlag))
    {
        MObject assetNodeObj;
        if (!getMObjectFromFlag(
                argData, kResetSimulationFlagLong, assetNodeObj, status))
            return status;

        mySubCommand = new AssetSubCommandResetSimulation(assetNodeObj);
    }

    if (argData.isFlagSet(kCookMessagesFlag))
    {
        MObject assetNodeObj;
        if (!getMObjectFromFlag(
                argData, kCookMessagesFlagLong, assetNodeObj, status))
            return status;

        mySubCommand = new AssetSubCommandCookMessages(assetNodeObj);
    }

    if (argData.isFlagSet(kParmHelpFlag))
    {
        MString parmName;
        {
            status = argData.getFlagArgument(kParmHelpFlag, 0, parmName);
            if (!status)
            {
                displayError("Invalid argument for \"" kParmHelpFlagLong "\".");
                return status;
            }
        }

        MObject assetNodeObj;
        if (!getMObjectFromFlag(
                argData, kParmHelpFlag, assetNodeObj, status, 1))
            return status;

        mySubCommand = new AssetSubCommandGetParmHelp(assetNodeObj, parmName);
    }

    return MStatus::kSuccess;
}

MStatus
AssetCommand::doIt(const MArgList &args)
{
    MStatus status;

    status = parseArgs(args);
    if (!status || !mySubCommand)
    {
        return status;
    }

    return mySubCommand->doIt();
}

MStatus
AssetCommand::redoIt()
{
    if (!mySubCommand)
    {
        return MStatus::kSuccess;
    }

    return mySubCommand->redoIt();
}

MStatus
AssetCommand::undoIt()
{
    if (!mySubCommand)
    {
        return MStatus::kSuccess;
    }

    return mySubCommand->undoIt();
}

bool
AssetCommand::isUndoable() const
{
    if (!mySubCommand)
    {
        return false;
    }

    return mySubCommand->isUndoable();
}
