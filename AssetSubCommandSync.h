#ifndef __AssetSubCommandSync_h__
#define __AssetSubCommandSync_h__

#include "AssetSubCommand.h"

#include <maya/MObject.h>
#include <maya/MDagModifier.h>

#include <vector>

class AssetSync;

class AssetSubCommandSync : public AssetSubCommandAsset
{
    public:
	AssetSubCommandSync(
		const MObject &assetNodeObj,
		const bool syncOnlyVisible
		);
	virtual ~AssetSubCommandSync();

        void setSyncAttributes();
        void setSyncOutputs();

	virtual MStatus doIt();
	virtual MStatus redoIt();
	virtual MStatus undoIt();
	virtual bool isUndoable() const;

    protected:
	bool mySyncAll;
	bool mySyncAttributes;
	bool mySyncOutputs;

	const bool mySyncOnlyVisible;
	MDagModifier myDagModifier;

	typedef std::vector<AssetSync*> AssetSyncs;
	AssetSyncs myAssetSyncs;
};

#endif
