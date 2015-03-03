#ifndef __OutputGeometryPart_h__
#define __OutputGeometryPart_h__

#include <maya/MIntArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatArray.h>
#include <maya/MVectorArray.h>
#include <maya/MString.h>

class Asset;

class MFnArrayAttrsData;

class OutputGeometryPart
{
    public:
        OutputGeometryPart(
                int assetId,
                int objectId,
                int geoId,
                int partId
                );
        ~OutputGeometryPart();

        MStatus compute(
                const MTime &time,
                MDataHandle& handle,
                bool hasGeoChanged,
                bool hasMaterialChanged,
                bool &needToSyncOutputs
                );

    protected:
        void update();

    private:
        void computeMaterial(
                const MTime &time,
                MDataHandle &materialHandle
                );

        void computeMesh(
                const MTime &time,
                MDataHandle &hasMeshHandle,
                MDataHandle &meshHandle
                );
        void computeParticle(
                const MTime &time,
                MDataHandle &hasParticlesHandle,
                MDataHandle &particleHandle
                );
        void computeCurves(
                const MTime &time,
                MDataHandle &curvesHandle,
                MDataHandle &curvesIsBezierHandle
                );
        void computeVolume(
                const MTime &time,
                MDataHandle &volumeHandle
                );
        void computeVolumeTransform(
                const MTime &time,
                MDataHandle &volumeTransformHandle
                );

        template<typename T>
        bool getAttributeData(
                std::vector<T> &array,
                const char* name,
                HAPI_AttributeOwner owner
                );

        template<typename T, typename U>
        void convertParticleAttribute(
                MFnArrayAttrsData &arrayDataFn,
                const MString &mayaName,
                U &buffer,
                const char* houdiniName,
                int particleCount
                );

    private:
        int myAssetId;
        int myObjectId;
        int myGeoId;
        int myPartId;

        HAPI_PartInfo myPartInfo;
        HAPI_VolumeInfo myVolumeInfo;
        HAPI_CurveInfo myCurveInfo;
        HAPI_MaterialInfo myMaterialInfo;

        bool myNeverBuilt;
};

#endif
