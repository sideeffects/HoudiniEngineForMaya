#ifndef __AssetSyncOutputInstance_h__
#define __AssetSyncOutputInstance_h__

#include <maya/MPlug.h>

struct AssetSyncOutputInstance
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

#endif
