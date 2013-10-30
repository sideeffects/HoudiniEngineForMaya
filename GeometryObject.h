#ifndef __GeometryObject_h__
#define __GeometryObject_h__

#include "GeometryPart.h"
#include "Object.h"

#include <vector>

class GeometryObject: public Object
{
    public:
        GeometryObject(int assetId, int objectId);
        virtual ~GeometryObject();

        virtual void init();

        //MObject createMesh();

        virtual MStatus compute(MDataHandle& handle);

        //*************** function until groups are fully supported **************
        MStatus computeParts(MDataHandle& obj, MArrayDataBuilder* builder);
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
        HAPI_Transform myTransformInfo;
        std::vector<GeometryPart> myParts;
        //HAPI_MaterialInfo materialInfo;

        //MIntArray faceCounts;
        //MIntArray vertexList;
        //MFloatPointArray points;
        //MVectorArray normals;
        //MFloatArray us;
        //MFloatArray vs;
    
};

#endif
