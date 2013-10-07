#ifndef __AssetSubCommandSync_h__
#define __AssetSubCommandSync_h__

#include "AssetSubCommand.h"

#include <maya/MObject.h>
#include <maya/MDagModifier.h>

#include <vector>

class AssetSync;

class AssetSubCommandSync : public AssetSubCommand
{
    public:
	AssetSubCommandSync(
		const MObject &assetNodeObj,
		const bool removeExistingParmAttributes
		);
	virtual ~AssetSubCommandSync();

	virtual MStatus doIt();
	virtual MStatus redoIt();
	virtual MStatus undoIt();
	virtual bool isUndoable() const;

    protected:
	MObject myAssetNodeObj;
	const bool myRemoveExistingParmAttributes;
	MDagModifier myDagModifier;

	typedef std::vector<AssetSync*> AssetSyncs;
	AssetSyncs myAssetSyncs;
};

#endif
