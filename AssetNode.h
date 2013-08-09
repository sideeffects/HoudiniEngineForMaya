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

#include "Asset.h"

// -------------------------------------------------------------------------------
//Overall structure:
//
//  When you load an asset from the menu, it calls the assetCommand to load the
//  file.  It will create an AssetManager (1 per asset).  The AssetManager contains
//  methods to create nodes and connections that represent the Maya side manifestation
//  of the data from the Houdini asset.   In the createManager()
//  static function of AssetManager, it will call init() on the AssetManager.  The
//  init() call will create the AssetNode, set essential information on the node,
//  and then cause a compute by pulling on the number of objects in the output.
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
//
//  Once the output attributes have been computed, the update() function (called
//  by the init()) goes on to call ObjectNodeGroup::update() and InstNodeGroup::update()
//  These methods will create all the meshes, transforms, materials, instancer objects etc
//  and hook it up all together to produce the realized output in Maya.
//
//  As a last step, the AssetCommand also sets up the AssetNodeMonitor on the newly
//  created asset node.  The AssetNodeMonitor will receive callbacks when attributes
//  have been modified on the AssetNode.  This will in turn call into the AssetManager
//  to update the objects and connections.


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

};

#endif
