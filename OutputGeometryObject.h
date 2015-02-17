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
                int assetId,
                int objectId,
                Asset* objectControl
                );
        virtual ~OutputGeometryObject();

        //MObject createMesh();

        virtual MStatus compute(
                MDataHandle& handle,
                bool &needToSyncOutputs
                );

        virtual ObjectType type();

    private:
        void update();

        void updateTransform(MDataHandle& handle);
        //void updateMaterial(MDataHandle& handle);
        //void updateFaceCounts();
        //void updateVertexList();
        //void updatePoints();
        //void updateNormals();
        //void updateUVs();

    private:

        HAPI_Transform myTransformInfo;
        std::vector<OutputGeometry *> myGeos;

        //HAPI_MaterialInfo materialInfo;

        //MIntArray faceCounts;
        //MIntArray vertexList;
        //MFloatPointArray points;
        //MVectorArray normals;
        //MFloatArray us;
        //MFloatArray vs;
};

#endif
