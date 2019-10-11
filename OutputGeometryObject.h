#ifndef __OutputGeometryObject_h__
#define __OutputGeometryObject_h__

#include "OutputObject.h"

#include <vector>

#include "AssetNodeOptions.h"

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
                const MPlug &objectPlug,
                MDataBlock& data,
                MDataHandle& objectHandle,
                const MIntArray &instancedObjIds,
                const MStringArray &instancedObjNames,
                AssetNodeOptions::AccessorDataBlock &options,
                bool &needToSyncOutputs,
                bool &needToRecomputeOutputData
                );

        virtual ObjectType type();

    private:
        void update();

        void updateTransform(MDataHandle& handle);

    private:
        std::vector<OutputGeometry *> myGeos;
};

#endif
