#ifndef ASSET_NODE_H
#define ASSET_NODE_H


#include <string.h>

#include <maya/MPxNode.h> 

#include <maya/MTypeId.h> 
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFloatArray.h>
#include <maya/MStringArray.h>
#include <maya/MIntArray.h>
#include <maya/MDataBlock.h>
#include <maya/MNodeMessage.h>

#include <HAPI.h>
#include "asset.h"

class AssetNodeMonitor;

class AssetNode: public MPxNode
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
        
        void attachAttrChangedCallback();
        void attachNodeDirtyCallback();
        void detachNodeDirtyCallback();

        MObject getAttrFromParm(HAPI_ParmInfo& parm);


        void updateAttrValue(HAPI_ParmInfo& parm, MDataBlock& data);
        void updateAttrValues(MDataBlock& data);

        void setParmValues(MDataBlock& data);
        void setParmValue(HAPI_ParmInfo& parm, MDataBlock& data);


    private:
        Asset* myAsset;
        bool myBuiltParms;
        bool myAssetChanged;
        MObject myDirtyParmAttribute;

        //AssetNodeMonitor* monitor;

};

#endif
