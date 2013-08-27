#include "AssetSubCommandSync.h"

#include <maya/MGlobal.h>
#include <maya/MPlug.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>

AssetSubCommandSync::AssetSubCommandSync(
	const MObject &assetNodeObj,
	const MObject &assetTransformObj
	) :
    myAssetNodeObj(assetNodeObj),
    myAssetTransformObj(assetTransformObj)
{
}

AssetSubCommandSync::~AssetSubCommandSync()
{
}

MStatus
AssetSubCommandSync::doIt()
{
    return MStatus::kSuccess;
}

MStatus
AssetSubCommandSync::redoIt()
{
    return MStatus::kSuccess;
}

MStatus
AssetSubCommandSync::undoIt()
{
    return MStatus::kSuccess;
}

bool
AssetSubCommandSync::isUndoable() const
{
    return true;
}
