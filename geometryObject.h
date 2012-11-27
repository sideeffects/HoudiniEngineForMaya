#ifndef GEOMETRY_OBJECT_H
#define GEOMETRY_OBJECT_H

#include "object.h"

class GeometryObject: public Object
{
    public:
        GeometryObject();
        GeometryObject(int assetId, int objectId);
        virtual ~GeometryObject();

        MObject createMesh();

        //virtual MStatus compute(const MPlug& plug, MDataBlock& data);
        virtual MStatus compute(MDataHandle& handle);
        virtual MStatus setClean(MPlug& plug, MDataBlock& data);

        virtual ObjectType type();


    public:

    protected:
        virtual void update();

    private:
        void updateTransform(MDataHandle& handle);
        void updateMaterial(MDataHandle& handle);
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
