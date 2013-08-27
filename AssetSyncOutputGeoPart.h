#ifndef __AssetSyncOutputGeoPart_h__
#define __AssetSyncOutputGeoPart_h__

#include <maya/MPlug.h>

#include "AssetSync.h"

class AssetSyncOutputGeoPart : public AssetSync
{
    public:
	AssetSyncOutputGeoPart(
		const MPlug &outputPlug,
		const MObject &assetTransform
		);
	virtual ~AssetSyncOutputGeoPart();

	virtual MStatus doIt();
	virtual MStatus undoIt();
	virtual MStatus redoIt();

    protected:
	MStatus updateNodes();
	MStatus updateConnections();

    protected:
	//This is the output plug from the asset node that represents an object
	//eg. (assetNode.objects[1])
	MPlug myOutputPlug;

	//the transform of the HAPI Asset
	MObject myAssetTransform;

	//the created Maya mesh node, if any
	MObject myMeshNode;

	//the transform of the part
	MObject myPartTransform;

	//the transform of the HAPI object - you can find all the parts of an
	//object under this node
	MObject myObjectTransform;

	//the phong node associated with the shader - you can see it by
	//selecting any output mesh and looking at the input/output connections
	//of the mesh
	MObject myMaterialNode;

	//the shading engine node (shading group node)
	MObject mySeNode;

	//The Maya file node that load a texture to put piped into the phong
	//node above.
	MObject myFileNode;
};

#endif
