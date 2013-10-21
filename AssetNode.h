#ifndef __AssetNode_h__
#define __AssetNode_h__

#include <maya/MPxTransform.h>

#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFloatArray.h>
#include <maya/MStringArray.h>
#include <maya/MIntArray.h>
#include <maya/MDataBlock.h>
#include <maya/MNodeMessage.h>

#include <HAPI/HAPI.h>

#include <vector>

#include "Asset.h"

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
//  work by calling the compute() function of the InstancerObject class, and
//  for computeGeometryObjects() it will call the computeParts() function of the
//  GeometryObject class.  


class AssetNode: public MPxTransform
{
    public:
        AssetNode();
        virtual ~AssetNode(); 

        virtual void postConstructor();

        virtual MStatus compute( const MPlug& plug, MDataBlock& data );

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

	Asset* getAsset();

        static void* creator();
        static MStatus initialize();

	void rebuildAsset();

    public:
        static MTypeId id;

    private:
        MObject getAttrFromParm(HAPI_ParmInfo& parm);


        void updateAttrValue(HAPI_ParmInfo& parm, MDataBlock& data);
        void updateAttrValues(MDataBlock& data);

        void setParmValues(MDataBlock& data);
        void setParmValue(HAPI_ParmInfo& parm, MDataBlock& data);

	bool isPlugDirty(const MPlug &plug, const HAPI_ParmInfo &parm);
	void setPlugDirty(const MPlug &plug);

    private:
        Asset* myAsset;
        bool myBuiltParms;
	void createAsset();
	void destroyAsset();

	MString myAssetPath;
	bool myAssetPathChanged;
	bool myResultsClean;

	typedef std::vector<MObject> MObjectVector;
	MObjectVector myDirtyParmAttributes;

    public:
        static MObject assetPath;
        static MObject parmsModified;
        static MObject inTime;

        static MObject assetType;

        static MObject input;

        static MObject output;

        static MObject outputObjects;
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

	static MObject outputParts;
        static MObject outputPartName;
        static MObject outputPartMesh;
        static MObject outputPartHasParticles;
        static MObject outputPartMaterial;
        static MObject outputPartMaterialExists;
        static MObject outputPartTexturePath;
        static MObject outputPartAmbientColor;
        static MObject outputPartDiffuseColor;
        static MObject outputPartSpecularColor;
        static MObject outputPartAlphaColor;

	static MObject outputPartParticle;
	static MObject outputPartParticlePositions;
	static MObject outputPartParticleArrayData;

#if MAYA_API_VERSION >= 201400
	static MObject outputPartVolume;
	static MObject outputPartVolumeName;
	static MObject outputPartVolumeGrid;
	static MObject outputPartVolumeRes;
	static MObject outputPartVolumeResW;
	static MObject outputPartVolumeResH;
	static MObject outputPartVolumeResD;
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
