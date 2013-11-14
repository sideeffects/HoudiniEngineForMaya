#ifndef __GeometryPart_h__
#define __GeometryPart_h__

#include <maya/MIntArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatArray.h>
#include <maya/MVectorArray.h>
#include <maya/MString.h>

class Asset;

class GeometryPart
{
    public:        
        GeometryPart(int assetId, int objectId, int geoId, int partId,
                HAPI_ObjectInfo objectInfo, HAPI_GeoInfo geoInfo, Asset* objectControl);
        virtual ~GeometryPart();


        virtual MStatus compute(MDataHandle& handle);

        virtual void setGeoInfo(HAPI_GeoInfo& info);

    public:
        MString myPartName;

    protected:
        virtual void update();

    private:
        void updateMaterial(MDataHandle& handle);
        void updateFaceCounts();
        void updateVertexList();
        void updatePoints();
        void updateNormals();
        void updateUVs();

        MObject createMesh();
	void createParticle(MDataHandle &dataHandle);

	MObject createVolume();
	void    updateVolumeTransform(MDataHandle& handle);

        void getAttributeFloatData(
                std::vector<float> &floatArray,
                const char* name,
                HAPI_AttributeOwner owner
                );

    private:
        Asset* myObjectControl;

        int myAssetId;
        int myObjectId;
        int myGeoId;
        int myPartId;

        HAPI_ObjectInfo myObjectInfo;
        HAPI_GeoInfo myGeoInfo;
        HAPI_PartInfo myPartInfo;
	HAPI_VolumeInfo myVolumeInfo;
        HAPI_MaterialInfo myMaterialInfo;

        MIntArray myFaceCounts;
        MIntArray myVertexList;
        MFloatPointArray myPoints;
        MVectorArray myNormals;
        MFloatArray myUs;
        MFloatArray myVs;

        bool myNeverBuilt;
};

#endif
