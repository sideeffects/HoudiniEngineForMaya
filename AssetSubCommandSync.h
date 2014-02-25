#ifndef __AssetSubCommandSync_h__
#define __AssetSubCommandSync_h__

#include "AssetSubCommand.h"

#include <maya/MObject.h>
#include <maya/MDagModifier.h>

#include <vector>

class AssetSubCommandSync : public AssetSubCommandAsset
{
    public:
	AssetSubCommandSync(
		const MObject &assetNodeObj,
		const bool syncHidden,
                const bool syncTemplatedGeos
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

	const bool mySyncHidden;
        const bool mySyncTemplatedGeos;
	MDagModifier myDagModifier;

	typedef std::vector<AssetSubCommand*> AssetSyncs;
	AssetSyncs myAssetSyncs;
};

#endif
