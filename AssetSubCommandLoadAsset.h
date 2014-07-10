#ifndef __AssetSubCommandLoadAsset_h__
#define __AssetSubCommandLoadAsset_h__

#include "SubCommand.h"

#include <maya/MDagModifier.h>
#include <maya/MString.h>

class AssetSubCommandSync;

class AssetSubCommandLoadAsset : public SubCommand
{
    public:
        AssetSubCommandLoadAsset(
                const MString &otlFilePath,
                const MString &assetName
                );
        virtual ~AssetSubCommandLoadAsset();

        virtual MStatus doIt();
        virtual MStatus redoIt();
        virtual MStatus undoIt();

        virtual bool isUndoable() const;

    protected:
        MString myOTLFilePath;
        MString myAssetName;

        MDagModifier myDagModifier;

        AssetSubCommandSync* myAssetSubCommandSync;
};

#endif
