#ifndef __AssetNode_h__
#define __AssetNode_h__

#include <maya/MPxTransform.h>

#include <maya/MTypeId.h>

class MPlugArray;

#include <vector>

class Asset;

// -------------------------------------------------------------------------------
//Overall structure:
//
//  The AssetNode is a Maya node that wraps the functionality of a Houdini asset.
//  It has input attributes (dynamically created) that mirror the input parms of the asset.
//  The input attributes are created dynamically at compute time, by the function
//  Asset::getParmAttributes()
//  At compute time, the input attributes are read and pushed into HAPI, then it
//  calls on the Asset class (a helper, contained as a member) to compute itself.
//  The Asset class knows how to compute itself - it will first deal with inputs
//  to the asset, whether from another asset (inter-asset workflow) or from the
//  Maya scene (geometry marshalling).  It will then call HAPI_CookAsset() and
//  then extract the results from Houidni to update all the Maya output attributes.
//  It does this by first calling computeInstancerObjects() then
//  computeGeometryObjects().  Each of these will fill out the information in
//  the output attribute.  For computeInstancerObjects() it will end up doing the
//  work by calling the compute() function of the OutputInstancerObject class, and
//  for computeGeometryObjects() it will call the computeParts() function of the
//  OutputGeometryObject class.

class AssetNode: public MPxTransform
{
    public:
        AssetNode();
        virtual ~AssetNode();

        virtual void postConstructor();

        virtual MStatus compute(const MPlug& plug, MDataBlock& data);

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

        static void* creator();
        static MStatus initialize();

        void rebuildAsset();

    public:
        static MString typeName;
        static MTypeId typeId;

    private:
        Asset* myAsset;
        bool isAssetValid() const;
        void createAsset();
        void destroyAsset();

        MString myOTLFilePath;
        MString myAssetName;

        bool mySetAllParms;
        bool myResultsClean;
        bool myNeedToMarshalInput;

        typedef std::vector<MObject> MObjectVector;
        MObjectVector myDirtyParmAttributes;

    public:
        static MObject inTime;

        static MObject otlFilePath;
        static MObject assetName;

        static MObject syncWhenInputConnects;
        static MObject autoSyncOutputs;
        static MObject splitGeosByGroup;
        static MObject outputHiddenObjects;
        static MObject outputTemplatedGeometries;

        static MObject input;

        static MObject output;

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
        static MObject outputGeoType;
        static MObject outputGeoName;
        static MObject outputGeoIsTemplated;
        static MObject outputGeoIsDisplayGeo;
        static MObject outputGeoCurveType;
        static MObject outputGeoCurveOrder;
        static MObject outputGeoCurveCVX;
        static MObject outputGeoCurveCVY;
        static MObject outputGeoCurveCVZ;
        static MObject outputGeoCurveCVs;

        static MObject outputParts;
        static MObject outputPartName;
        static MObject outputPartHasMesh;
        static MObject outputPartHasParticles;
        static MObject outputPartMaterial;
        static MObject outputPartMaterialExists;
        static MObject outputPartMaterialName;
        static MObject outputPartTexturePath;
        static MObject outputPartAmbientColor;
        static MObject outputPartDiffuseColor;
        static MObject outputPartSpecularColor;
        static MObject outputPartAlphaColor;

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

        static MObject useInstancerNode;
};

#endif
