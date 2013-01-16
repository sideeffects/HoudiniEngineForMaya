#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MDGModifier.h>
#include <maya/MFnMesh.h>
#include <maya/MIntArray.h>
#include <maya/MFloatPointArray.h>

#include "assetCommand.h"
#include "assetManager.h"
#include "assetNodeMonitor.h"
#include "util.h"

AssetCommand::AssetCommand()
{
    myOperationType = kOperationInvalid;
}


AssetCommand::~AssetCommand()
{
}


void* AssetCommand::creator()
{
    return new AssetCommand();
}


MStatus AssetCommand::doIt( const MArgList& args )
{

    MStatus status;
    for ( int ii = 0; ii < args.length(); ii++ )
    {
	// -load load an otl file
	// expected arguments: otl_file_name - the name of the otl file to load
        if ( MString( "-load" ) == args.asString( ii, &status )
            && MS::kSuccess == status )
        {
	    myOperationType = kOperationLoad;
	    myAssetOtlPath = args.asString( ++ii );
	    
        }
	// -saveHip saves the contents of the current Houdini scene as a hip file
	// expected arguments: hip_file_name - the name of the hip file to save
        else if ( MString( "-saveHip" ) == args.asString( ii, &status )
            && MS::kSuccess == status )
        {
	    myOperationType = kOperationSaveHip;
	    myHIPFilePath = args.asString( ++ii );	    
	}
        else
        {
            MString msg = "Invalid flag: ";
            msg += args.asString( ii );
            displayError( msg );
            return MS::kFailure;
        }
    }

    return redoIt();
    
}

MStatus AssetCommand::redoIt() 
{
    if( myOperationType == kOperationLoad )
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
