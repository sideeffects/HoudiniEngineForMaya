#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <maya/MString.h>
#include <maya/MObject.h>
#include <maya/MObjectArray.h>

#include <vector>

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

        MObject findNodeByName(MString& name);

    private:
        static std::vector<AssetManager*> managers;

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
