#ifndef _AssetSyncOutputObject_h__
#define _AssetSyncOutputObject_h__

#include <vector>

#include <maya/MDagModifier.h>
#include <maya/MPlug.h>

#include "AssetSync.h"

class AssetSyncOutputObject : public AssetSync
{
    public:
	AssetSyncOutputObject(
		const MPlug &outputPlug,
		const MObject &assetNodeObj
		);
	virtual ~AssetSyncOutputObject();

	virtual MStatus doIt();
	virtual MStatus undoIt();
	virtual MStatus redoIt();

    protected:
	MStatus createOutputObject();
	MStatus createFluidShape();
	MStatus createFluidShapeNode(MObject& transform, MObject& fluid);

	const MPlug myOutputPlug;
	const MObject myAssetNodeObj;

	MDagModifier myDagModifier;

	// TODO: change this into an AssetSyncOutputGeoPart
	typedef std::vector<AssetSync*> AssetSyncs;
	AssetSyncs myAssetSyncs;
};



#endif
