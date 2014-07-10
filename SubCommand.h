#ifndef __SubCommand_h__
#define __SubCommand_h__

#include <maya/MObject.h>
#include <maya/MStatus.h>

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
        AssetNode* getAssetNode() const;
        Asset* getAsset() const;

    protected:
        MObject myAssetNodeObj;
};

#endif
