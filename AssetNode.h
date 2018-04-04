#ifndef __AssetNode_h__
#define __AssetNode_h__

#include <maya/MPxTransform.h>

#include <maya/MTypeId.h>

#include <vector>

class Asset;

#ifdef WIN32
// Workaround the deprecation warning for MPxNode::internalArrayCount()
#pragma warning( push )
#pragma warning( disable: 4266 )
#endif
class AssetNode: public MPxTransform
{
    public:
        static void* creator();
        static MStatus initialize();

    public:
        static MString typeName;
        static MTypeId typeId;

        static void nodeAdded(MObject& node,void *clientData);
        static void nodeRemoved(MObject& node,void *clientData);

    public:
        AssetNode();
        virtual ~AssetNode();

        virtual void postConstructor();

        virtual MStatus compute(const MPlug& plug, MDataBlock& data);

#if MAYA_API_VERSION >= 201800
        virtual bool getInternalValue(
                const MPlug &plug,
                MDataHandle &dataHandle);
        virtual bool setInternalValue(
                const MPlug &plug,
                const MDataHandle &dataHandle);
        virtual int internalArrayCount(const MPlug &plug) const;
#else
        virtual bool getInternalValueInContext(
                const MPlug &plug,
                MDataHandle &dataHandle,
                MDGContext &ctx);
        virtual bool setInternalValueInContext(
                const MPlug &plug,
                const MDataHandle &dataHandle,
                MDGContext &ctx
                );
        virtual int internalArrayCount(const MPlug &plug, const MDGContext &ctx) const;
#endif

        virtual void copyInternalData(MPxNode* node);

        virtual MStatus setDependentsDirty(const MPlug& plugBeingDirtied,
                MPlugArray& affectedPlugs);

#if MAYA_API_VERSION >= 201600
        virtual MStatus preEvaluation(
                const MDGContext& context,
                const MEvaluationNode& evaluationNode
                );
#endif

        Asset* getAsset() const;

        void rebuildAsset();

        void setParmValues(bool onlyDirtyParms = true);
        void getParmValues();

        int autoSyncId() const
        { return myAutoSyncId; }
	void setExtraAutoSync(bool needs);

    private:
        Asset* myAsset;
        bool isAssetValid() const;
        void createAsset();
        void destroyAsset();

        void setParmValues(MDataBlock& data, bool onlyDirtyParms = true);
        void getParmValues(MDataBlock& data);

        MString myOTLFilePath;
        MString myAssetName;

        bool mySetAllParms;
        bool myNeedToMarshalInput;

        int myAutoSyncId;
	int myExtraAutoSync;

        typedef std::vector<MObject> MObjectVector;
        MObjectVector myDirtyParmAttributes;

    public:
        static MObject inTime;

        static MObject otlFilePath;
        static MObject assetName;

        static MObject input;
        static MObject inputName;
        static MObject inputNodeId;

        static MObject output;

        static MObject outputAssetTransform;
        static MObject outputAssetTranslate;
        static MObject outputAssetTranslateX;
        static MObject outputAssetTranslateY;
        static MObject outputAssetTranslateZ;
        static MObject outputAssetRotate;
        static MObject outputAssetRotateX;
        static MObject outputAssetRotateY;
        static MObject outputAssetRotateZ;
        static MObject outputAssetScale;
        static MObject outputAssetScaleX;
        static MObject outputAssetScaleY;
        static MObject outputAssetScaleZ;

        static MObject outputObjects;
        static MObject outputObjectName;
        static MObject outputObjectTransform;
        static MObject outputObjectTranslate;
        static MObject outputObjectTranslateX;
        static MObject outputObjectTranslateY;
        static MObject outputObjectTranslateZ;
        static MObject outputObjectRotate;
        static MObject outputObjectRotateX;
        static MObject outputObjectRotateY;
        static MObject outputObjectRotateZ;
        static MObject outputObjectScale;
        static MObject outputObjectScaleX;
        static MObject outputObjectScaleY;
        static MObject outputObjectScaleZ;

#if MAYA_API_VERSION >= 201400
        static MObject outputObjectFluidFromAsset;
#endif
        static MObject outputObjectMetaData;

        static MObject outputGeos;
        static MObject outputGeoName;
        static MObject outputGeoIsTemplated;
        static MObject outputGeoIsDisplayGeo;

        static MObject outputParts;
        static MObject outputPartName;
        static MObject outputPartHasMesh;
        static MObject outputPartHasParticles;
        static MObject outputPartHasInstancer;

        static MObject outputPartMesh;
        static MObject outputPartMeshCurrentColorSet;
        static MObject outputPartMeshCurrentUV;
        static MObject outputPartMeshData;

        static MObject outputPartParticle;
        static MObject outputPartParticleCurrentTime;
        static MObject outputPartParticlePositions;
        static MObject outputPartParticleArrayData;

        static MObject outputPartCurves;
        static MObject outputPartCurvesIsBezier;

#if MAYA_API_VERSION >= 201400
        static MObject outputPartVolume;
        static MObject outputPartVolumeName;
        static MObject outputPartVolumeGrid;
        static MObject outputPartVolumeRes;
        static MObject outputPartVolumeTransform;
        static MObject outputPartVolumeTranslate;
        static MObject outputPartVolumeTranslateX;
        static MObject outputPartVolumeTranslateY;
        static MObject outputPartVolumeTranslateZ;
        static MObject outputPartVolumeRotate;
        static MObject outputPartVolumeRotateX;
        static MObject outputPartVolumeRotateY;
        static MObject outputPartVolumeRotateZ;
        static MObject outputPartVolumeScale;
        static MObject outputPartVolumeScaleX;
        static MObject outputPartVolumeScaleY;
        static MObject outputPartVolumeScaleZ;
#endif

        static MObject outputPartInstancer;
        static MObject outputPartInstancerArrayData;
        static MObject outputPartLockTracking;
        static MObject outputPartInstancerParts;

        static MObject outputPartInstancerTransform;
        static MObject outputPartInstancerTranslate;
        static MObject outputPartInstancerTranslateX;
        static MObject outputPartInstancerTranslateY;
        static MObject outputPartInstancerTranslateZ;
        static MObject outputPartInstancerRotate;
        static MObject outputPartInstancerRotateX;
        static MObject outputPartInstancerRotateY;
        static MObject outputPartInstancerRotateZ;
        static MObject outputPartInstancerScale;
        static MObject outputPartInstancerScaleX;
        static MObject outputPartInstancerScaleY;
        static MObject outputPartInstancerScaleZ;

        static MObject outputPartMaterialIds;

        static MObject outputPartExtraAttributes;
        static MObject outputPartExtraAttributeName;
        static MObject outputPartExtraAttributeOwner;
        static MObject outputPartExtraAttributeDataType;
        static MObject outputPartExtraAttributeTuple;
        static MObject outputPartExtraAttributeData;

        static MObject outputPartGroups;
        static MObject outputPartGroupName;
        static MObject outputPartGroupType;
        static MObject outputPartGroupMembers;

        static MObject outputVisibility;
        static MObject outputIsInstanced;

        //static MObject numObjects;
        static MObject outputInstancers;
        static MObject outputInstancerData;
        static MObject outputInstancedObjectNames;
        static MObject outputHoudiniInstanceAttribute;
        static MObject outputHoudiniNameAttribute;

        static MObject outputInstanceTransform;
        static MObject outputInstanceTranslate;
        static MObject outputInstanceTranslateX;
        static MObject outputInstanceTranslateY;
        static MObject outputInstanceTranslateZ;
        static MObject outputInstanceRotate;
        static MObject outputInstanceRotateX;
        static MObject outputInstanceRotateY;
        static MObject outputInstanceRotateZ;
        static MObject outputInstanceScale;
        static MObject outputInstanceScaleX;
        static MObject outputInstanceScaleY;
        static MObject outputInstanceScaleZ;

        static MObject outputMaterials;
        static MObject outputMaterialPath;
        static MObject outputMaterialName;
        static MObject outputMaterialNodeId;
        static MObject outputMaterialAmbientColor;
        static MObject outputMaterialDiffuseColor;
        static MObject outputMaterialSpecularColor;
        static MObject outputMaterialAlphaColor;
        static MObject outputMaterialTexturePath;
};
#ifdef WIN32
#pragma warning( pop )
#endif

#endif
