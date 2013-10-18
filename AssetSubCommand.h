#ifndef __AssetSubCommand_h__
#define __AssetSubCommand_h__

#include <maya/MObject.h>
#include <maya/MStatus.h>

class AssetNode;
class Asset;

class AssetSubCommand
{
    public:
	AssetSubCommand();
	virtual ~AssetSubCommand();

	virtual MStatus doIt();
	virtual MStatus redoIt();
	virtual MStatus undoIt();

	virtual bool isUndoable() const;
};

class AssetSubCommandAsset : public AssetSubCommand
{
    public:
	AssetSubCommandAsset(const MObject &assetNodeObj);

    protected:
	AssetNode* getAssetNode() const;
	Asset* getAsset() const;

    protected:
	MObject myAssetNodeObj;
};

#endif
