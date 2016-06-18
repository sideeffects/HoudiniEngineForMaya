#ifndef __SyncOutputObject_h__
#define __SyncOutputObject_h__

#include <vector>

#include <maya/MDagModifier.h>
#include <maya/MPlug.h>

#include "SubCommand.h"

class SyncOutputGeometryPart;

class SyncOutputObject : public SubCommand
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
#if MAYA_API_VERSION >= 201400
        MStatus createFluidShape(const MObject &objectTransform);
#endif

        const MPlug myOutputPlug;
        const MObject myAssetNodeObj;
        const bool myVisible;
        const bool mySyncTemplatedGeos;

        MDagModifier myDagModifier;

        typedef std::vector<SyncOutputGeometryPart*> AssetSyncs;
        AssetSyncs myAssetSyncs;
};

#endif
