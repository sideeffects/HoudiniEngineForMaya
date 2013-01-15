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
}


AssetCommand::~AssetCommand()
{
}


void* AssetCommand::creator()
{
    return new AssetCommand();
}


MStatus AssetCommand::doIt(const MArgList& args)
{

    MStatus status;
    for ( int ii = 0; ii < args.length(); ii++ )
    {
	// -l load an otl file
	// expected arguments: otl_file_name - the name of the otl file to load
        if ( MString( "-l" ) == args.asString( ii, &status )
            && MS::kSuccess == status )
        {
	    myOperationType = kOperationLoad;
	    myAssetOtlPath = args.asString(++ii);
	    
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
	AssetManager* manager = AssetManager::createManager(myAssetOtlPath);
	AssetNodeMonitor* monitor = new AssetNodeMonitor(manager);
	monitor->watch();
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
    cout << "In AssetCommand::isUndoable()\n";
    return true;
}
