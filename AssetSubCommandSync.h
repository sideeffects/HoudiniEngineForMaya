#ifndef __AssetSubCommandSync_h__
#define __AssetSubCommandSync_h__

#include "SubCommand.h"

#include <maya/MDagModifier.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>

#include <vector>

class AssetSubCommandSync : public SubCommandAsset
{
public:
    AssetSubCommandSync(const MObject &assetNodeObj);
    virtual ~AssetSubCommandSync();

    void setSyncAttributes();
    void setSyncOutputs();

    void setSyncOutputHidden();
    void setSyncOutputTemplatedGeos();
    void deleteMaterials(MPlug &materialPlug);

    virtual MStatus doIt();
    virtual MStatus redoIt();
    virtual MStatus undoIt();
    virtual bool isUndoable() const;

protected:
    bool mySyncAll;
    bool mySyncAttributes;
    bool mySyncOutputs;

    bool mySyncOutputHidden;
    bool mySyncOutputTemplatedGeos;

    MDagModifier myDagModifier;

    typedef std::vector<SubCommand *> AssetSyncs;
    AssetSyncs myAssetSyncs;
};

#endif

