#ifndef __GeometryObject_h__
#define __GeometryObject_h__

#include "GeometryPart.h"
#include "Object.h"

#include <vector>

class Geo;

class GeometryObject: public Object
{
    public:
        GeometryObject(
                int assetId,
                int objectId,
                Asset* objectControl
                );
        virtual ~GeometryObject();

        //MObject createMesh();

        virtual MStatus compute(
                MDataHandle& handle,
                bool &needToSyncOutputs
                );

        virtual MStatus setClean(MPlug& plug, MDataBlock& data);

        virtual ObjectType type();
        

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
        std::vector<Geo *> myGeos;

        //HAPI_MaterialInfo materialInfo;

        //MIntArray faceCounts;
        //MIntArray vertexList;
        //MFloatPointArray points;
        //MVectorArray normals;
        //MFloatArray us;
        //MFloatArray vs;
    
};

#endif
