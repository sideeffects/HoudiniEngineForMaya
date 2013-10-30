#ifndef __InstancerObject_h__
#define __InstancerObject_h__

//#include <Maya/MIntArray.h>

#include "Object.h"


class InstancerObject: public Object
{
    public:
        InstancerObject(int assetId, int objectId);
        virtual ~InstancerObject();

        virtual void init();

        MIntArray getInstancedObjIds();
        MStringArray getUniqueInstObjNames();

        virtual MStatus compute(MDataHandle& handle);
        virtual MStatus setClean(MPlug& plug, MDataBlock& data);

        virtual ObjectType type();

    public:

    protected:
        virtual void update();

        virtual MStringArray getAttributeStringData(HAPI_AttributeOwner owner, MString name);

    private:
        HAPI_PartInfo myPartInfo;

        MStringArray myInstancedObjectNames;
        MStringArray myUniqueInstObjNames;
        MIntArray myInstancedObjectIndices;

	MStringArray myHoudiniInstanceAttribute;
	MStringArray myHoudiniNameAttribute;
};

#endif
