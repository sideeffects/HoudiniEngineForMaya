#ifndef __AssetManager_h__
#define __AssetManager_h__

#include <maya/MString.h>
#include <maya/MPlug.h>
#include <maya/MObject.h>
#include <maya/MObjectArray.h>

#include <vector>

class AssetSyncOutputGeoPart;
class AssetSyncOutputInstance;

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

        AssetSyncOutputGeoPart* getObjectGroup(MPlug& plug);
        AssetSyncOutputInstance* getInstGroup(MPlug& plug);

        MStatus createAssetSyncOutputGeoPart(MPlug& plug, MObject& assetTransform);
        MStatus createAssetSyncOutputInstance(MPlug& plug, MObject& assetTransform);


    private:
        static std::vector<AssetManager*> myManagers;

        std::vector<AssetSyncOutputGeoPart*> myAssetSyncOutputGeoParts;
        std::vector<AssetSyncOutputInstance*> myAssetSyncOutputInstances;

        MString myFilePath;
        MObject myAssetNode;
        MObject myAssetTransform;
        
};


#endif
