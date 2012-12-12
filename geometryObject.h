#ifndef GEOMETRY_OBJECT_H
#define GEOMETRY_OBJECT_H

#include "object.h"
#include "geometryPart.h"

class GeometryObject: public Object
{
    public:
        GeometryObject();
        GeometryObject(int assetId, int objectId);
        virtual ~GeometryObject();

        virtual void init();

        //MObject createMesh();

        virtual MStatus compute(MDataHandle& handle);

        //*************** function until groups are fully supported **************
        MStatus computeParts(MArrayDataBuilder* builder, int* index);
        //************************************************************************

        virtual MStatus setClean(MPlug& plug, MDataBlock& data);

        virtual ObjectType type();


    public:

    protected:
        virtual void update();

    private:
        void updateTransform(MDataHandle& handle);
        //void updateMaterial(MDataHandle& handle);
        //void updateFaceCounts();
        //void updateVertexList();
        //void updatePoints();
        //void updateNormals();
        //void updateUVs();

    private:
        HAPI_Transform transformInfo;
        GeometryPart* parts;
        //HAPI_MaterialInfo materialInfo;

        //MIntArray faceCounts;
        //MIntArray vertexList;
        //MFloatPointArray points;
        //MVectorArray normals;
        //MFloatArray us;
        //MFloatArray vs;
    
};

#endif
