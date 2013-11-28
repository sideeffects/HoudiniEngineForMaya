#ifndef __InstancerObject_h__
#define __InstancerObject_h__

//#include <Maya/MIntArray.h>

#include "Object.h"


class InstancerObject: public Object
{
    public:
        InstancerObject(
                int assetId,
                int objectId,
                Asset* objectControl
                );
        virtual ~InstancerObject();

        MIntArray getInstancedObjIds();
        MStringArray getUniqueInstObjNames();

        virtual MStatus compute(MDataHandle& handle);
        virtual MStatus setClean(MPlug& plug, MDataBlock& data);

        virtual ObjectType type();

    public:

    protected:
        virtual void update();

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
