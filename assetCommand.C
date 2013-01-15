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
    cerr << "AssetCommand constructor" << endl;
}


AssetCommand::~AssetCommand()
{
    cerr << "AssetCommand destructor" << endl;
}


void* AssetCommand::creator()
{
    cerr << "AssetCommand creator" << endl;
    return new AssetCommand();
}


MStatus AssetCommand::doIt(const MArgList& args)
{
    cerr << "AssetCommand doIt" << endl;


    MStatus status;
    for ( int ii = 0; ii < args.length(); ii++ )
    {
	// -l load an otl file
	// expected arguments: otl_file_name - the name of the otl file to load
        if ( MString( "-l" ) == args.asString( ii, &status )
            && MS::kSuccess == status )
        {
	    MString filePath = args.asString(++ii);
	    AssetManager* manager = AssetManager::createManager(filePath);
	    AssetNodeMonitor* monitor = new AssetNodeMonitor(manager);
	    monitor->watch();
	    return MS::kSuccess;            
        }        
        else
        {
            MString msg = "Invalid flag: ";
            msg += args.asString( ii );
            displayError( msg );
            return MS::kFailure;
        }
    }

    
}
