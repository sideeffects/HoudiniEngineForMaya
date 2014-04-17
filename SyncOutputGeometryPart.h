#ifndef __SyncOutputGeometryPart_h__
#define __SyncOutputGeometryPart_h__

#include <maya/MDagModifier.h>
#include <maya/MPlug.h>

#include "AssetSubCommand.h"

class SyncOutputGeometryPart : public AssetSubCommand
{
    public:
        SyncOutputGeometryPart(
                const MPlug &outputPlug,
                const MObject & objectTransform
                );
        virtual ~SyncOutputGeometryPart();

        virtual MStatus doIt();
        virtual MStatus undoIt();
        virtual MStatus redoIt();

        virtual bool isUndoable() const;

    protected:
        MStatus createOutputPart(
                const MObject &objectTransform,
                const MString &partName,
                MObject &partTransform
                );
        MStatus createOutputMaterial(
                const MPlug &materialPlug,
                const MObject &partTransform
                );
        MStatus createOutputParticle(
                const MObject &partTransform,
                const MPlug &particlePlug
                );
        MStatus createOutputCurves(
                MPlug curvesPlug,
                const MObject &partTransform,
                bool isBezier
                );

    protected:
        //This is the output plug from the asset node that represents an object
        //eg. (assetNode.objects[1])
        const MPlug myOutputPlug;

        //the transform of the HAPI Asset
        const MObject myObjectTransform;

        MDagModifier myDagModifier;
};

#endif
