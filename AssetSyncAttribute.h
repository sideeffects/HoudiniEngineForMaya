#ifndef __AssetSyncAttribute_h__
#define __AssetSyncAttribute_h__

#include "AssetSync.h"

#include <maya/MDGModifier.h>
#include <maya/MObject.h>

#include <HAPI/HAPI.h>

class AssetSyncAttribute : public AssetSync
{
    public:
	AssetSyncAttribute(
		const MObject &assetNodeObj
		);
	virtual ~AssetSyncAttribute();

	virtual MStatus doIt();
	virtual MStatus undoIt();
	virtual MStatus redoIt();

    protected:
	const MObject myAssetNodeObj;

	HAPI_NodeInfo myNodeInfo;

	MDGModifier myDGModifier;
};

#endif
