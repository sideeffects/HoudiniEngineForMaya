#ifndef __OutputGeometryPart_h__
#define __OutputGeometryPart_h__

#include <maya/MIntArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatArray.h>
#include <maya/MFnArrayAttrsData.h>
#include <maya/MVectorArray.h>
#include <maya/MString.h>

class Asset;

class OutputGeometryPart
{
    public:
        OutputGeometryPart(
                HAPI_NodeId nodeId,
                HAPI_PartId partId
                );
        ~OutputGeometryPart();

        bool needCompute(
                AssetNodeOptions::AccessorDataBlock &options
                ) const;

        MStatus compute(
                const MTime &time,
                const MPlug &partPlug,
                MDataBlock& data,
                MDataHandle& handle,
                AssetNodeOptions::AccessorDataBlock &options,
                bool &needToSyncOutputs
                );

    protected:
        void update();

    private:
        void computeMaterial(
                const MTime &time,
                const MPlug &materialPlug,
                MDataBlock& data,
                MDataHandle &materialHandle
                );

        void computeMesh(
                const MTime &time,
                const MPlug &hasMeshPlug,
                const MPlug &meshPlug,
                MDataBlock& data,
                MDataHandle &hasMeshHandle,
                MDataHandle &meshHandle,
                AssetNodeOptions::AccessorDataBlock &options
                );
        void computeParticle(
                const MTime &time,
                const MPlug &hasParticlePlug,
                const MPlug &particlePlug,
                MDataBlock& data,
                MDataHandle &hasParticlesHandle,
                MDataHandle &particleHandle
                );
        void computeCurves(
                const MTime &time,
                const MPlug &curvesPlug,
                const MPlug &curvesIsBezierPlug,
                MDataBlock& data,
                MDataHandle &curvesHandle,
                MDataHandle &curvesIsBezierHandle
                );
        void computeVolume(
                const MTime &time,
                const MPlug &volumePlug,
                MDataBlock& data,
                MDataHandle &volumeHandle
                );
        void computeVolumeTransform(
                const MTime &time,
                MDataHandle &volumeTransformHandle
                );
        void computeInstancer(
                const MTime &time,
                const MPlug &hasInstancerPlug,
                const MPlug &instancePlug,
                MDataBlock& data,
                MDataHandle &hasInstancerHandle,
                MDataHandle &instanceHandle
                );
        void computeExtraAttributes(
                const MTime &time,
                const MPlug &extraAttributesPlug,
                MDataBlock& data,
                MDataHandle &extraAttributesHandle,
                AssetNodeOptions::AccessorDataBlock &options,
                bool &needToSyncOutputs
                );
        void computeGroups(
                const MTime &time,
                const MPlug &groupsPlug,
                MDataBlock& data,
                MDataHandle &groupsHandle,
                AssetNodeOptions::AccessorDataBlock &options,
                bool &needToSyncOutputs
                );

        template<typename T>
        bool getAttributeData(
                std::vector<T> &array,
                const char* name,
                HAPI_AttributeOwner owner
                );

        template<typename T>
        bool convertParticleAttribute(
                T arrayDataFn,
                const char* houdiniName
                );

        bool computeExtraAttribute(
                const MPlug &extraAttributePlug,
                MDataBlock& data,
                MDataHandle &extraAttributeHandle,
                HAPI_AttributeOwner attributeOwner,
                const char* attributeName
                );

        void markAttributeUsed(const std::string &attributeName);
        bool isAttributeUsed(const std::string &attributeName);
        void clearAttributesUsed();

    private:
        HAPI_NodeId myNodeId;
        HAPI_PartId myPartId;

        std::vector<std::string> myAttributesUsed;

        HAPI_GeoInfo myGeoInfo;
        HAPI_PartInfo myPartInfo;
        HAPI_VolumeInfo myVolumeInfo;
        HAPI_CurveInfo myCurveInfo;

        bool myLastOutputGeometryGroups;
        bool myLastOutputCustomAttributes;
};

#endif
