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
        virtual MStatus setDependentsDirty(const MPlug& plugBeingDirtied,
                MPlugArray& affectedPlugs);

        static void* creator();
        static MStatus initialize();


    public:
        static MTypeId id;

    private:
        MObject getAttrFromParm(HAPI_ParmInfo& parm);


        void updateAttrValue(HAPI_ParmInfo& parm, MDataBlock& data);
        void updateAttrValues(MDataBlock& data);

        void setParmValues(MDataBlock& data);
        void setParmValue(HAPI_ParmInfo& parm, MDataBlock& data);


    private:
        Asset* myAsset;
        bool myBuiltParms;
        bool myAssetChanged;

	typedef std::vector<MObject> MObjectVector;
	MObjectVector myDirtyParmAttributes;

    public:
        static MObject assetPath;
        static MObject parmsModified;
        static MObject inTime;

        static MObject assetType;

        static MObject output;

        static MObject outputObjects;
        static MObject outputObjectName;
        static MObject outputObjectMetaData;
        static MObject outputObjectMesh;
        
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

        static MObject outputObjectMaterial;
        static MObject outputObjectMaterialExists;
        static MObject outputObjectTexturePath;
        static MObject outputObjectAmbientColor;
        static MObject outputObjectDiffuseColor;
        static MObject outputObjectSpecularColor;
        static MObject outputObjectAlphaColor;

        //static MObject numObjects;
        static MObject outputInstancers;
        static MObject outputInstancerData;
        static MObject outputInstancedObjectNames;
};

#endif
