#ifndef __AssetSyncAttribute_h__
#define __AssetSyncAttribute_h__

#include "AssetSubCommand.h"

#include <maya/MDGModifier.h>
#include <maya/MObject.h>

#include <HAPI/HAPI.h>

class MFnCompoundAttribute;

class AssetSyncAttribute : public AssetSubCommand
{
    public:
	AssetSyncAttribute(
		const MObject &assetNodeObj
		);
	virtual ~AssetSyncAttribute();

	virtual MStatus doIt();
	virtual MStatus undoIt();
	virtual MStatus redoIt();

	virtual bool isUndoable() const;

    protected:
	const MObject myAssetNodeObj;

	HAPI_NodeInfo myNodeInfo;

	MDGModifier myDGModifier;
};

#endif
