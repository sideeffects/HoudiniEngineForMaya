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

    //This command for the moment only accept 1 parameter, which is the name of the otl file 
    // to be loaded.  You call this command like so:
    // assetCommand "C:\\cygwin\\home\\ken\\dev_projects\\HAPI\\Maya\\assets\\otls\\Core\\SideFX__spaceship.otl";

    MString filePath = args.asString(0);
    AssetManager* manager = AssetManager::createManager(filePath);
    AssetNodeMonitor* monitor = new AssetNodeMonitor(manager);
    monitor->watch();

    return MS::kSuccess;
}
