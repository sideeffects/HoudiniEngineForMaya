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
        Object(HAPI_ObjectInfo objInfo, int assetId);
        void createMesh(MObject& outData);
        void updateTransform(MPlug& plug, MDataBlock& data);

    private:
        void updateFaceCounts();
        void updateVertexList();
        void updatePoints();
        void updateNormals();
        void updateUVs();
        void updateGeometry();


        MFloatArray getAttributeFloatData(HAPI_AttributeOwner owner, char* name);
        void reverseWindingOrderInt(MIntArray& data, MIntArray& faceCounts);
        void reverseWindingOrderFloat(MFloatArray& data, MIntArray& faceCounts);

    private:
        HAPI_ObjectInfo objectInfo;
        HAPI_DetailInfo detailInfo;
        int assetId;

        MIntArray faceCounts;
        MIntArray vertexList;
        MFloatPointArray points;
        MVectorArray normals;
        MFloatArray us;
        MFloatArray vs;
};
