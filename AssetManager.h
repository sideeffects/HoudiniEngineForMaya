#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <maya/MString.h>
#include <maya/MPlug.h>
#include <maya/MObject.h>
#include <maya/MObjectArray.h>

#include <vector>

// This file contains all the methods to create Maya side nodes, from meshes to transforms to materials
// and connects all these nodes together with the output of the asset node


struct ObjectNodeGroup		//Each one of these object node groups represents a part in HAPI.  It does not represent a HAPI object
{
    // Members
    MPlug plug;			//This is the output plug from the asset node that represents an object
				//eg. (assetNode.objects[1])
    MObject meshNode;		//the created Maya mesh node, if any
    MObject partTransform;	//the transform of the part
    MObject objectTransform;	//the transform of the HAPI object - you can find all the parts of an object under this node
    MObject assetTransform;	//the transform of the HAPI Asset
    MObject materialNode;	//the phong node associated with the shader - you can see it by selecting any output mesh and looking
				//at the input/output connections of the mesh
    MObject seNode;		//the shading engine node (shading group node)
    MObject fileNode;		//The Maya file node that load a texture to put piped into the phong node above.

    // Functions
    MStatus update();
    MStatus updateNodes();
    MStatus updateConnections();
};

struct InstNodeGroup
{
    // Members
    MPlug plug;			//This is the output plug from the asset node that is connected
				//to the instancer node (eg. assetNode.instancers[0])
    MObject instancerNode;
    MObject assetTransform;	//a reference to the top level transform (the same as 
				//AssetManager::myAssetTransform, it's there so it can
				// be accessed by the instancer node (whenever one is created
				// it is parented under this transform.

    // Functions
    MStatus update();
    MStatus updateNodes();
    MStatus updateConnections();
};

// There is 1 AssetManager per asset.  It tracks all the meshes, transforms, 
// materials for the objects associated with the asset
// and for instancers it tracks the instancer node
class AssetManager
{
    public:
        static AssetManager* createManager(const MString& filePath);

        AssetManager();
        AssetManager(const MString& filePath);
        virtual ~AssetManager();

        MStatus update();

        MObject getAssetNode();

    private:
        void init();

        ObjectNodeGroup* getObjectGroup(MPlug& plug);
        InstNodeGroup* getInstGroup(MPlug& plug);

        MStatus createObjectNodeGroup(MPlug& plug, MObject& assetTransform);
        MStatus createInstNodeGroup(MPlug& plug, MObject& assetTransform);


    private:
        static std::vector<AssetManager*> myManagers;

        std::vector<ObjectNodeGroup*> myObjectNodeGroups;
        std::vector<InstNodeGroup*> myInstNodeGroups;

        MString myFilePath;
        MObject myAssetNode;
        MObject myAssetTransform;
        
};


#endif
