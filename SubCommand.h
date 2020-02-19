#ifndef __SubCommand_h__
#define __SubCommand_h__

#include <maya/MObject.h>
#include <maya/MStatus.h>

#include "util.h"

class AssetNode;
class Asset;

class SubCommand
{
public:
    SubCommand();
    virtual ~SubCommand();

    virtual MStatus doIt();
    virtual MStatus redoIt();
    virtual MStatus undoIt();

    virtual bool isUndoable() const;
};

class SubCommandAsset : public SubCommand
{
public:
    SubCommandAsset(const MObject &assetNodeObj);

protected:
    AssetNode *getAssetNode() const;
    Asset *getAsset() const;

protected:
    MObject myAssetNodeObj;
};

#define GET_COMMAND_ASSET_OR_RETURN_FAIL()                                     \
    Asset *asset = getAsset();                                                 \
    if (!asset)                                                                \
    {                                                                          \
        MFnDependencyNode assetNodeFn(myAssetNodeObj);                         \
        DISPLAY_ERROR("^1s: The node contains an invalid asset."               \
                      " Check the OTL file path and asset name.",              \
                      assetNodeFn.name());                                     \
        return MStatus::kFailure;                                              \
    }

#endif
