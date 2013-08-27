#ifndef __AssetSyncOutputInstance_h__
#define __AssetSyncOutputInstance_h__

#include <maya/MPlug.h>

#include "AssetSync.h"

class AssetSyncOutputInstance : public AssetSync
{
    public:
	AssetSyncOutputInstance(
		const MPlug &outputPlug,
		const MObject &assetTransform
		);
	virtual ~AssetSyncOutputInstance();

	virtual MStatus doIt();
	virtual MStatus undoIt();
	virtual MStatus redoIt();

    protected:
	MStatus updateNodes();
	MStatus updateConnections();

    protected:
	//This is the output plug from the asset node that is connected
	//to the instancer node (eg. assetNode.instancers[0])
	MPlug myOutputPlug;

	//the transform of the HAPI Asset
	MObject myAssetTransform;

	MObject myInstancerNode;

};

#endif
