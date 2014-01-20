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
                HAPI_ObjectInfo objectInfo, HAPI_GeoInfo geoInfo);
        ~GeometryPart();


        MStatus compute(
                MDataHandle& handle,
                bool &needToSyncOutputs
                );

        void setGeoInfo(HAPI_GeoInfo& info);

    protected:
        void update();

    private:
        void updateMaterial(MDataHandle& handle);

        void createMesh(MDataHandle &dataHandle);
	void createParticle(MDataHandle &dataHandle);
	void createCurves(MDataHandle &dataHandle);

	MObject createVolume();
	void    updateVolumeTransform(MDataHandle& handle);

        template<typename T>
        bool getAttributeData(
                std::vector<T> &array,
                const char* name,
                HAPI_AttributeOwner owner
                );

    private:
        int myAssetId;
        int myObjectId;
        int myGeoId;
        int myPartId;

        HAPI_ObjectInfo myObjectInfo;
        HAPI_GeoInfo myGeoInfo;
        HAPI_PartInfo myPartInfo;
	HAPI_VolumeInfo myVolumeInfo;
	HAPI_CurveInfo myCurveInfo;
        HAPI_MaterialInfo myMaterialInfo;

        bool myNeverBuilt;
};

#endif
