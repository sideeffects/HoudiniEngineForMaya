#include <string.h>

#include <maya/MPxNode.h> 

#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFloatArray.h>
#include <maya/MStringArray.h>
#include <maya/MIntArray.h>
#include <maya/MDataBlock.h>

#include <HAPI.h>
#include "object.h"


class Asset: public MPxNode
{
    public:
        Asset();
        virtual ~Asset(); 

        virtual MStatus compute( const MPlug& plug, MDataBlock& data );
        virtual MStatus setDependentsDirty(const MPlug& plugBeingDirtied,
                MPlugArray& affectedPlugs);

        static void* creator();
        static MStatus initialize();

    public:
        static MObject input1;
        static MObject input2;
        static MObject output;
        static MObject meshes;
        static MObject transforms;
        static MObject translateAttr;
        static MObject translateAttrX;
        static MObject translateAttrY;
        static MObject translateAttrZ;
        static MObject rotateAttr;
        static MObject rotateAttrX;
        static MObject rotateAttrY;
        static MObject rotateAttrZ;
        static MObject scaleAttr;
        static MObject scaleAttrX;
        static MObject scaleAttrY;
        static MObject scaleAttrZ;
        static MObject numObjects;
        static MTypeId id;

    private:
        //MObject createMesh(MObject& outData);

        void buildParms();
        int buildAttrTree(HAPI_ParmInfo* myParmInfos, MObject* parent, int current, int start);
        void addAttrTo(MObject& child, MObject* parent);

        MObject getAttrFromParm(HAPI_ParmInfo& parm);

        void createAttr(HAPI_ParmInfo& parm, MObject* result);
        void createNumericAttr(HAPI_ParmInfo& parm, MObject* result, MString& longName, 
                MString& shortName, MString& niceName);
        void createStringAttr(HAPI_ParmInfo& parm, MObject* result, MString& longName, 
                MString& shortName, MString& niceName);

        MIntArray getParmIntValues(HAPI_ParmInfo& parm);
        MFloatArray getParmFloatValues(HAPI_ParmInfo& parm);
        MStringArray getParmStringValues(HAPI_ParmInfo& parm);

        void updateAttrValue(HAPI_ParmInfo& parm, MDataBlock& data);
        void updateAttrValues(MDataBlock& data);

        void setParmValues(MDataBlock& data);
        void setParmValue(HAPI_ParmInfo& parm, MDataBlock& data);

        HAPI_AssetInfo* assetInfo;
        bool builtParms;

};
