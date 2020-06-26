#ifndef __SyncOutputInstance_h__
#define __SyncOutputInstance_h__

#include <maya/MDagModifier.h>
#include <maya/MPlug.h>

#include "SubCommand.h"

class SyncOutputInstance : public SubCommand
{
public:
    SyncOutputInstance(const MPlug &outputPlug,
                       const int parentMultiIndex,
                       const MObject &assetNodeObj);
    virtual ~SyncOutputInstance();

    virtual MStatus doIt();
    virtual MStatus undoIt();
    virtual MStatus redoIt();

    virtual bool isUndoable() const;

protected:
    MStatus createOutput();

    bool instanceObjects(MObject searchRoot,
                         MObject instancerTransform,
                         int pointIndex,
                         const MString &objectToInstanceName,
                         const MString &houdiniInstanceAttr,
                         const MString &houdiniNameAttr);

    void instanceObject(MDagPath &objToInstance,
                        MObject instancerTransform,
                        int pointIndex);

    bool stringStartsWith(const MString &string, const MString &startsWith);

protected:
    // This is the output plug from the asset node that is connected
    // to the instancer node (eg. assetNode.instancers[0])
    const MPlug myOutputPlug;

    // the transform of the HAPI Asset
    const MObject myAssetNodeObj;

    // the multi-index of the parent attribute.  We'll need this
    // to effectively evaluate any of the multi-attributes
    const int myParentMultiIndex;

    MDagModifier myDagModifier;
};

#endif
