#ifndef __AssetSubCommandLockAsset_h__
#define __AssetSubCommandLockAsset_h__

#include "SubCommand.h"

#include <maya/MDagModifier.h>
#include <maya/MString.h>

class AssetSubCommandLockAsset : public SubCommandAsset
{
    bool myLockOn;
    MDagModifier myDagModifier;

public:
    AssetSubCommandLockAsset(const MObject &assetNodeObj, bool lockOn);

    MStatus doIt();
    MStatus redoIt();
    MStatus undoIt();
    bool isUndoable() const { return true; }
};

#endif
