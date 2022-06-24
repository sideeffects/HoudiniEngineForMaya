#ifndef __SyncOutputGeometryPart_h__
#define __SyncOutputGeometryPart_h__

#include <maya/MDagModifier.h>
#include <maya/MObjectArray.h>
#include <maya/MPlug.h>

#include "SubCommand.h"

class SyncOutputGeometryPart : public SubCommand
{
public:
    SyncOutputGeometryPart(const MPlug &outputPlug,
                           const MObject &objectTransform);
    virtual ~SyncOutputGeometryPart();

    virtual MStatus doIt();
    MStatus doItPost(SyncOutputGeometryPart *const *syncParts);
    virtual MStatus undoIt();
    virtual MStatus redoIt();

    virtual bool isUndoable() const;

    const MObject &partTransform() const { return myPartTransform; }

    bool isInstanced() { return myIsInstanced; }

    void setIsInstanced(bool isInstanced) { myIsInstanced = isInstanced; }

protected:
    MStatus createOutputPart(const MObject &objectTransform,
                             const MString &partName);
    MStatus createOutputMesh(const MString &partName, const MPlug &meshPlug);
    MStatus createOutputParticle(const MString &partName,
                                 const MPlug &particlePlug);
    MStatus createOutputCurves(MPlug curvesPlug,
                               const MString &partName,
                               bool isBezier);
    MStatus createOutputInstancer(const MString &partName,
                                  const MPlug &instancerPlug);
    MStatus createOutputInstancerPost(const MPlug &instancerPlug,
                                      SyncOutputGeometryPart *const *syncParts);
    MStatus createOutputExtraAttributes(const MObject &dstNode,
                                        MPlug *mayaSGAttributePlug = NULL);
    MStatus createOutputGroups(const MObject &dstNode,
                               std::vector<bool> *hasMaterials = NULL);

protected:
    // This is the output plug from the asset node that represents an object
    // eg. (assetNode.objects[1])
    const MPlug myOutputPlug;

    // the transform of the HAPI Asset
    const MObject myObjectTransform;

    MDagModifier myDagModifier;

    MObject myPartTransform;
    MObjectArray myPartShapes;

    bool myIsInstanced;
};

#endif

