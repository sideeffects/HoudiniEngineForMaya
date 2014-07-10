#ifndef __SyncAttribute_h__
#define __SyncAttribute_h__

#include "SubCommand.h"

#include <maya/MDGModifier.h>
#include <maya/MObject.h>

#include <HAPI/HAPI.h>

class MFnCompoundAttribute;

class SyncAttribute : public SubCommand
{
    public:
        SyncAttribute(
                const MObject &assetNodeObj
                );
        virtual ~SyncAttribute();

        virtual MStatus doIt();
        virtual MStatus undoIt();
        virtual MStatus redoIt();

        virtual bool isUndoable() const;

    protected:
        const MObject myAssetNodeObj;

        HAPI_NodeInfo myNodeInfo;

        MDGModifier myDGModifier;
};

#endif
