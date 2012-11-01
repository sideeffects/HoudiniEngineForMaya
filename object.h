#include <maya/MStatus.h>
#include <maya/MFloatArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MVectorArray.h>
#include <maya/MStringArray.h>
#include <maya/MIntArray.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>

#include <HAPI.h>

class Object {
    public:
        Object();
        Object(int objIndex, int assetId);
        MObject createMesh();
        void updateTransform(MPlug& plug, MDataBlock& data);
        void updateMaterial(MPlug& plug, MDataBlock& data);
        MStatus compute(int index, const MPlug& plug, MDataBlock& data);

        // test
        bool isVisible();

    private:
        void updateFaceCounts();
        void updateVertexList();
        void updatePoints();
        void updateNormals();
        void updateUVs();
        void update();

        // Utility
        MFloatArray getAttributeFloatData(HAPI_AttributeOwner owner, char* name);
        void reverseWindingOrderInt(MIntArray& data, MIntArray& faceCounts);
        void reverseWindingOrderFloat(MFloatArray& data, MIntArray& faceCounts);


    private:
        HAPI_ObjectInfo objectInfo;
        HAPI_GeoInfo geoInfo;
        HAPI_Transform transformInfo;
        HAPI_MaterialInfo materialInfo;
        int assetId;
        int objectIndex;

        MIntArray faceCounts;
        MIntArray vertexList;
        MFloatPointArray points;
        MVectorArray normals;
        MFloatArray us;
        MFloatArray vs;
};
