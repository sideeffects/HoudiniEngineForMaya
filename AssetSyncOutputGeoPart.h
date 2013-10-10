#ifndef __AssetSyncOutputGeoPart_h__
#define __AssetSyncOutputGeoPart_h__

#include <maya/MDagModifier.h>
#include <maya/MPlug.h>

#include "AssetSync.h"

class AssetSyncOutputGeoPart : public AssetSync
{
    public:
	AssetSyncOutputGeoPart(
		const MPlug &outputPlug,
		const MObject &assetNodeObj,
		const bool visible
		);
	virtual ~AssetSyncOutputGeoPart();

	virtual MStatus doIt();
	virtual MStatus undoIt();
	virtual MStatus redoIt();

    protected:
	MStatus createOutputPart(
		const MObject &objectTransform,
		const MString &partName,
		MObject &partTransform
		);
	MStatus createOutputMaterial(
		const MPlug &materialPlug,
		const MObject &partTransform
		);
	MStatus createOutputParticle(
		const MObject &partTransform,
		const MPlug &particlePlug
		);

    protected:
	//This is the output plug from the asset node that represents an object
	//eg. (assetNode.objects[1])
	const MPlug myOutputPlug;

	//the transform of the HAPI Asset
	const MObject myAssetNodeObj;

	//the visibility of the object in houdini
	const bool myVisible;

	MDagModifier myDagModifier;
};

#endif
