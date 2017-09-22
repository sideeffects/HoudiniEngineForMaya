#ifndef __OutputInstancerObject_h__
#define __OutputInstancerObject_h__

#include "AssetNodeOptions.h"
#include "OutputObject.h"

class OutputInstancerObject: public OutputObject
{
    public:
        OutputInstancerObject(
                HAPI_NodeId nodeId
                );
        virtual ~OutputInstancerObject();

        MIntArray getInstancedObjIds();
        MStringArray getUniqueInstObjNames();

        virtual MStatus compute(
                const MTime &time,
                const MPlug &plug,
                MDataBlock& data,
                MDataHandle& handle,
                AssetNodeOptions::AccessorDataBlock &options,
                bool &needToSyncOutputs
                );

        virtual ObjectType type();

    protected:
        void update();

        MStringArray getAttributeStringData(HAPI_AttributeOwner owner, MString name);

    private:
        HAPI_NodeInfo mySopNodeInfo;

        HAPI_GeoInfo myGeoInfo;
        HAPI_PartInfo myPartInfo;

        int myLastSopCookCount;

        MStringArray myInstancedObjectNames;
        MStringArray myUniqueInstObjNames;
        MIntArray myInstancedObjectIndices;

        MStringArray myHoudiniInstanceAttribute;
        MStringArray myHoudiniNameAttribute;
};

#endif
