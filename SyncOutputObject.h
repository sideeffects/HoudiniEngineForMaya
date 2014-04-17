#ifndef __SyncOutputObject_h__
#define __SyncOutputObject_h__

#include <vector>

#include <maya/MDagModifier.h>
#include <maya/MPlug.h>

#include "AssetSubCommand.h"

class SyncOutputObject : public AssetSubCommand
{
    public:
        SyncOutputObject(
                const MPlug &outputPlug,
                const MObject &assetNodeObj,
                const bool visible,
                const bool syncTemplatedGeos
                );
        virtual ~SyncOutputObject();

        virtual MStatus doIt();
        virtual MStatus undoIt();
        virtual MStatus redoIt();

        virtual bool isUndoable() const;

    protected:
        MStatus createOutputObject();
        MStatus createFluidShape();
        MStatus createFluidShapeNode(MObject& transform, MObject& fluid);
        MStatus createVelocityConverter(MObject& gridInterleaver);
        bool resolutionsEqual(MPlug resA, MPlug resB);

        const MPlug myOutputPlug;
        const MObject myAssetNodeObj;
        const bool myVisible;
        const bool mySyncTemplatedGeos;

        MDagModifier myDagModifier;

        // TODO: change this into an SyncOutputGeometryPart
        typedef std::vector<AssetSubCommand*> AssetSyncs;
        AssetSyncs myAssetSyncs;
};

#endif
