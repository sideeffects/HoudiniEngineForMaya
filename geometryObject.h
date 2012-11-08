#ifndef GEOMETRY_OBJECT_H
#define GEOMETRY_OBJECT_H

#include "object.h"

class GeometryObject: public Object
{
    public:
        GeometryObject();
        GeometryObject(int assetId, int objectId);
        MObject createMesh();

        virtual MStatus compute(const MPlug& plug, MDataBlock& data);

        virtual ObjectType type();


    public:

    protected:
        virtual void update();

    private:
        void updateTransform(MPlug& plug, MDataBlock& data);
        void updateMaterial(MPlug& plug, MDataBlock& data);
        void updateFaceCounts();
        void updateVertexList();
        void updatePoints();
        void updateNormals();
        void updateUVs();

    private:
        HAPI_Transform transformInfo;
        HAPI_MaterialInfo materialInfo;

        MIntArray faceCounts;
        MIntArray vertexList;
        MFloatPointArray points;
        MVectorArray normals;
        MFloatArray us;
        MFloatArray vs;
    
};

#endif
