#ifndef INSTANCER_OBJECT_H
#define INSTANCER_OBJECT_H

//#include <Maya/MIntArray.h>

#include "object.h"


class InstancerObject: public Object
{
    public:
        InstancerObject();
        InstancerObject(int assetId, int objectId);
        virtual ~InstancerObject();

        MIntArray getInstancedObjIds();
        MStringArray getUniqueInstObjNames();

        virtual MStatus compute(MDataHandle& handle);
        virtual MStatus setClean(MPlug& plug, MDataBlock& data);

        virtual ObjectType type();

    public:

    protected:
        virtual void update();

    private:
        MStringArray instancedObjectNames;
        MStringArray uniqueInstObjNames;
        MIntArray instancedObjectIndices;
};

#endif
