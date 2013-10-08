#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MSelectionList.h>
#include <maya/MStatus.h>

#include "AssetNode.h"
#include "AssetCommand.h"
#include "AssetSubCommand.h"
#include "AssetSubCommandLoadOTL.h"
#include "AssetSubCommandSync.h"
#include "util.h"

#define kLoadOTLFlag "-lo"
#define kLoadOTLFlagLong "-loadOTL"
#define kSyncFlag "-syn"
#define kSyncFlagLong "-sync"
#define kSaveHIPFlag "-sh"
#define kSaveHIPFlagLong "-saveHIP"
#define kResetSimulationFlag "-rs"
#define kResetSimulationFlagLong "-resetSimulation"
#define kReloadAssetFlag "-rl"
#define kReloadAssetFlagLong "-reloadAsset"

void* AssetCommand::creator()
{
    return new AssetCommand();
}

MSyntax
AssetCommand::newSyntax()
{
    MSyntax syntax;

    // -loadOTL load an otl file
    // expected arguments: otl_file_name - the name of the otl file to load
    CHECK_MSTATUS(syntax.addFlag(kLoadOTLFlag, kLoadOTLFlagLong, MSyntax::kString));

    // -sync synchronize the Maya nodes with the asset's state
    // expected arguments:
    //	asset node
    CHECK_MSTATUS(syntax.addFlag(kSyncFlag, kSyncFlagLong,
		MSyntax::kSelectionItem));

    // -saveHIP saves the contents of the current Houdini scene as a hip file
    // expected arguments: hip_file_name - the name of the hip file to save
    CHECK_MSTATUS(syntax.addFlag(kSaveHIPFlag, kSaveHIPFlagLong, MSyntax::kString));


    // -resetSimulation resets the simulation state for an asset.  This will clear
    // the DOPs cache for the asset.
    // TODO: FIXME: this should be the asset name, not the id
    // expected arguments: the asset id
    CHECK_MSTATUS(syntax.addFlag(kResetSimulationFlag, 
				 kResetSimulationFlagLong, 
				 MSyntax::kLong));


    // -reloadAsset will unload and immediate reload the asset.  If an otl file
    // has changed due to an edit in Houdini, this should pick up the change    
	// Note that this won't refresh the AE, you need to that separately after
	// running this, with refreshEditorTemplates
    CHECK_MSTATUS(syntax.addFlag(kReloadAssetFlag, 
				 kReloadAssetFlagLong, 
				 MSyntax::kString));

    return syntax;
}

AssetCommand::AssetCommand() :
    myOperationType(kOperationInvalid),
    myAssetSubCommand(NULL)
{
}


AssetCommand::~AssetCommand()
{
    if(myOperationType == kOperationSubCommand)
    {
	delete myAssetSubCommand;
    }
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

    if(!(argData.isFlagSet(kLoadOTLFlag)
		^ argData.isFlagSet(kSyncFlag)
		^ argData.isFlagSet(kSaveHIPFlag)
		^ argData.isFlagSet(kResetSimulationFlag)
		^ argData.isFlagSet(kReloadAssetFlag)))
    {
	displayError("Exactly one of these flags must be specified:\n"
		kLoadOTLFlagLong "\n"
		kSyncFlagLong "\n"
		kSaveHIPFlagLong "\n"
		kResetSimulationFlagLong "\n"
		kReloadAssetFlagLong "\n");
        return MStatus::kInvalidParameter;
    }

    if(argData.isFlagSet(kLoadOTLFlag))
    {
	myOperationType = kOperationSubCommand;

	MString otlPath;
	{
	    status = argData.getFlagArgument(kLoadOTLFlag, 0, otlPath);
	    if(!status)
	    {
		displayError("Invalid argument for \"" kLoadOTLFlagLong "\".");
		return status;
	    }
	}

	myAssetSubCommand = new AssetSubCommandLoadOTL(
		otlPath
		);
    }

    if(argData.isFlagSet(kReloadAssetFlag))
    {
	myOperationType = kOperationSubCommand;

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

	selList.add( assetPath );
	selList.getDependNode(0, assetNodeObj);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MFnDependencyNode assetNodeFn(assetNodeObj);
	AssetNode* assetNode = dynamic_cast<AssetNode*>(assetNodeFn.userNode());
	assetNode->rebuildAsset();

	myAssetSubCommand = new AssetSubCommandSync( assetNodeObj, true );

    }    

    if(argData.isFlagSet(kSyncFlag))
    {
	myOperationType = kOperationSubCommand;

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

	myAssetSubCommand = new AssetSubCommandSync(
		assetNodeObj, false
		);
    }

    if(argData.isFlagSet(kSaveHIPFlag))
    {
	myOperationType = kOperationSaveHip;

	status = argData.getFlagArgument(kSaveHIPFlag, 0, myHIPFilePath);
	if(!status)
	{
	    displayError("Invalid argument for \"" kSaveHIPFlagLong "\".");
	    return status;
	}
    }

    if(argData.isFlagSet(kResetSimulationFlag))
    {
	myOperationType = kOperationResetSimulation;

	//TODO: FIXME: this should be a string argument and not an asset id.  We should
	//use the name of the asset node to lookup the proper asset id.
	status = argData.getFlagArgument(kResetSimulationFlag, 0, myResetSimulationAssetId);
	if(!status)
	{
	    displayError("Invalid argument for \"" kResetSimulationFlagLong "\".");
	    return status;
	}
    }

    return MStatus::kSuccess;
}

MStatus AssetCommand::doIt( const MArgList& args )
{
    MStatus status;

    status = parseArgs(args);
    if(!status)
    {
	return status;
    }

    if(myOperationType == kOperationSubCommand)
    {
	return myAssetSubCommand->doIt();
    }

    return redoIt();
    
}

MStatus AssetCommand::redoIt() 
{
    if(myOperationType == kOperationSubCommand)
    {
	return myAssetSubCommand->redoIt();
    }

    if( myOperationType == kOperationSaveHip )
    {
	HAPI_SaveHIPFile( myHIPFilePath.asChar() );
	return MS::kSuccess;            
    }

    if ( myOperationType == kOperationResetSimulation )
    {
	HAPI_ResetSimulation( myResetSimulationAssetId );
	return MS::kSuccess;
    }

    return MS::kFailure;
}

MStatus AssetCommand::undoIt() 
{
    if(myOperationType == kOperationSubCommand)
    {
	return myAssetSubCommand->undoIt();
    }

    return MS::kSuccess;
}

bool AssetCommand::isUndoable() const 
{    
    if(myOperationType == kOperationSubCommand)
    {
	return myAssetSubCommand->isUndoable();
    }

    return true;
}
