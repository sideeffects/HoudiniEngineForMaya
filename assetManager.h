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
    MObject partNode;
    MObject objectNode;
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

    // Functions
    MStatus update();
};

class AssetManager
{
    public:
        static void createManager(const MString& filePath);

        AssetManager();
        AssetManager(const MString& filePath);
        virtual ~AssetManager();

        void update();

    private:
        void init();

        ObjectNodeGroup* getObjectGroup(MPlug& plug);
        InstNodeGroup* getInstGroup(MPlug& plug);

        MStatus createObjectNodeGroup(MPlug& plug);
        MStatus createInstNodeGroup(MPlug& plug);


    private:
        static std::vector<AssetManager*> managers;

        std::vector<ObjectNodeGroup*> objectNodeGroups;
        std::vector<InstNodeGroup*> instNodeGroups;

        MString filePath;
        MObject assetNode;
        MObjectArray partNodes;
        MObjectArray objectNodes;
        MObjectArray instancerNodes;
        MObjectArray materialNodes;
        MObjectArray seNodes;
        MObjectArray fileNodes;
};


#endif
