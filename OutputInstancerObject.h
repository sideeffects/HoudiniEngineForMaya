#ifndef __OutputInstancerObject_h__
#define __OutputInstancerObject_h__

//#include <Maya/MIntArray.h>

#include "OutputObject.h"

class OutputInstancerObject: public OutputObject
{
    public:
        OutputInstancerObject(
                int assetId,
                int objectId
                );
        virtual ~OutputInstancerObject();

        MIntArray getInstancedObjIds();
        MStringArray getUniqueInstObjNames();

        virtual MStatus compute(
                const MTime &time,
                MDataBlock& data,
                MDataHandle& handle,
                bool &needToSyncOutputs
                );

        virtual ObjectType type();

    protected:
        void update();

        MStringArray getAttributeStringData(HAPI_AttributeOwner owner, MString name);

    private:
        HAPI_GeoInfo myGeoInfo;
        HAPI_PartInfo myPartInfo;

        MStringArray myInstancedObjectNames;
        MStringArray myUniqueInstObjNames;
        MIntArray myInstancedObjectIndices;

        MStringArray myHoudiniInstanceAttribute;
        MStringArray myHoudiniNameAttribute;
};

#endif
