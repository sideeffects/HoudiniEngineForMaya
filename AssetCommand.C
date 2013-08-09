#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>

#include "AssetCommand.h"
#include "AssetManager.h"
#include "AssetNodeMonitor.h"
#include "util.h"

#define kLoadOTLFlag "-lo"
#define kLoadOTLFlagLong "-loadOTL"
#define kSaveHIPFlag "-sh"
#define kSaveHIPFlagLong "-saveHIP"

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

    // -saveHIP saves the contents of the current Houdini scene as a hip file
    // expected arguments: hip_file_name - the name of the hip file to save
    CHECK_MSTATUS(syntax.addFlag(kSaveHIPFlag, kSaveHIPFlagLong, MSyntax::kString));

    return syntax;
}

AssetCommand::AssetCommand() :
    myOperationType(kOperationInvalid)
{
}


AssetCommand::~AssetCommand()
{
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
		^ argData.isFlagSet(kSaveHIPFlag)))
    {
	displayError("Exactly one of these flags must be specified:\n"
		kLoadOTLFlagLong "\n"
		kSaveHIPFlagLong "\n");
        return MStatus::kInvalidParameter;
    }

    if(argData.isFlagSet(kLoadOTLFlag))
    {
	myOperationType = kOperationLoadOTL;

	status = argData.getFlagArgument(kLoadOTLFlag, 0, myAssetOtlPath);
	if(!status)
	{
	    displayError("Invalid argument for \"" kLoadOTLFlagLong "\".");
	    return status;
	}
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

    return redoIt();
    
}

MStatus AssetCommand::redoIt() 
{
    if( myOperationType == kOperationLoadOTL )
    {
	AssetManager* manager = AssetManager::createManager( myAssetOtlPath );
	AssetNodeMonitor* monitor = new AssetNodeMonitor( manager );
	monitor->watch();
	return MS::kSuccess;            
    }

    if( myOperationType == kOperationSaveHip )
    {
	HAPI_SaveHIPFile( myHIPFilePath.asChar() );
	return MS::kSuccess;            
    }

    return MS::kFailure;
}

MStatus AssetCommand::undoIt() 
{
    
    return MS::kSuccess;
}

bool AssetCommand::isUndoable() const 
{    
    return true;
}
