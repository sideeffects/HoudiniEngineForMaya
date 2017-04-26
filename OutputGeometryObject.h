#ifndef __OutputGeometryObject_h__
#define __OutputGeometryObject_h__

#include "OutputGeometryPart.h"
#include "OutputObject.h"

#include <vector>

class OutputGeometry;

class OutputGeometryObject: public OutputObject
{
    public:
        OutputGeometryObject(
                HAPI_NodeId nodeId
                );
        virtual ~OutputGeometryObject();

        virtual MStatus compute(
                const MTime &time,
                MDataBlock& data,
                MDataHandle& handle,
                const MIntArray &instancedObjIds,
                const MStringArray &instancedObjNames,
                bool &needToSyncOutputs
                );

        virtual ObjectType type();

    private:
        void update();

        void updateTransform(MDataHandle& handle);

    private:
        std::vector<OutputGeometry *> myGeos;
};

#endif
