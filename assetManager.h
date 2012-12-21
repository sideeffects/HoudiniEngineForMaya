#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <maya/MString.h>
#include <maya/MPlug.h>
#include <maya/MObject.h>
#include <maya/MObjectArray.h>

#include <vector>

struct ObjectNodeGroup
{
    // Members
    MPlug plug;
    MObject meshNode;
    MObject partTransform;
    MObject objectTransform;
    MObject assetTransform;
    MObject materialNode;
    MObject seNode;
    MObject fileNode;

    // Functions
    MStatus update();
    MStatus updateNodes();
    MStatus updateConnections();
};

struct InstNodeGroup
{
    // Members
    MPlug plug;
    MObject instancerNode;
    MObject assetTransform;

    // Functions
    MStatus update();
    MStatus updateNodes();
    MStatus updateConnections();
};

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
        static std::vector<AssetManager*> managers;

        std::vector<ObjectNodeGroup*> objectNodeGroups;
        std::vector<InstNodeGroup*> instNodeGroups;

        MString myFilePath;
        MObject myAssetNode;
        MObject myAssetTransform;
        MObjectArray myPartNodes;
        MObjectArray myObjectNodes;
        MObjectArray myInstancerNodes;
        MObjectArray myMaterialNodes;
        MObjectArray mySeNodes;
        MObjectArray myFileNodes;
};


#endif
