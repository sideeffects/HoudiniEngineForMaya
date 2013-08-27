#ifndef __AssetSyncOutputGeoPart_h__
#define __AssetSyncOutputGeoPart_h__

#include <maya/MPlug.h>

struct AssetSyncOutputGeoPart	//Each one of these object node groups represents a part in HAPI.  It does not represent a HAPI object
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

#endif
