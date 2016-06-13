#ifndef __SyncOutputGeometryPart_h__
#define __SyncOutputGeometryPart_h__

#include <maya/MDagModifier.h>
#include <maya/MPlug.h>
#include <maya/MObjectArray.h>

#include "SubCommand.h"

class SyncOutputGeometryPart : public SubCommand
{
    public:
        SyncOutputGeometryPart(
                const MPlug &outputPlug,
                const MObject & objectTransform
                );
        virtual ~SyncOutputGeometryPart();

        virtual MStatus doIt();
        MStatus doItPost(SyncOutputGeometryPart *const *syncParts);
        virtual MStatus undoIt();
        virtual MStatus redoIt();

        virtual bool isUndoable() const;

        const MObject &partTransform() const
        {
            return myPartTransform;
        }

    protected:
        MStatus createOutputPart(
                const MObject &objectTransform,
                const MString &partName,
                bool &hasMaterial
                );
        MStatus createOutputMesh(
                const MString &partName,
                const MPlug &meshPlug,
                bool &hasMaterial
                );
        MStatus createOutputMaterial(
                const MPlug &materialPlug
                );
        MStatus createOutputParticle(
                const MString &partName,
                const MPlug &particlePlug
                );
        MStatus createOutputCurves(
                MPlug curvesPlug,
                const MString &partName,
                bool isBezier
                );
        MStatus createOutputInstancer(
                const MString &partName,
                const MPlug &instancerPlug
                );
        MStatus createOutputInstancerPost(
                const MPlug &instancerPlug,
                SyncOutputGeometryPart *const *syncParts
                );
        MStatus createOutputExtraAttributes(
                const MObject &dstNode,
                MPlug &mayaSGAttributePlug
                );
        MStatus createOutputGroups(
                const MObject &dstNode,
                const MPlug &mayaSGAttributePlug,
                bool &hasMaterial
                );

    protected:
        //This is the output plug from the asset node that represents an object
        //eg. (assetNode.objects[1])
        const MPlug myOutputPlug;

        //the transform of the HAPI Asset
        const MObject myObjectTransform;

        MDagModifier myDagModifier;

        MObject myPartTransform;
        MObjectArray myPartShapes;
};

#endif
