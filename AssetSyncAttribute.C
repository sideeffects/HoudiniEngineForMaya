#include "AssetSyncAttribute.h"

#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>

#include <HAPI/HAPI.h>

#include "util.h"

AssetSyncAttribute::AssetSyncAttribute(
	const MObject &assetNodeObj
	) :
    myAssetNodeObj(assetNodeObj)
{
}

AssetSyncAttribute::~AssetSyncAttribute()
{
}

MStatus
AssetSyncAttribute::doIt()
{
    return MStatus::kSuccess;
}

MStatus
AssetSyncAttribute::undoIt()
{
    return MStatus::kSuccess;
}

MStatus
AssetSyncAttribute::redoIt()
{
    return MStatus::kSuccess;
}
