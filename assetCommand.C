#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MDGModifier.h>
#include <maya/MFnMesh.h>
#include <maya/MIntArray.h>
#include <maya/MFloatPointArray.h>

#include "assetCommand.h"
#include "assetManager.h"
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

    MString filePath = args.asString(0);
    AssetManager::createManager(filePath);

    return MS::MStatusCode::kSuccess;
}
