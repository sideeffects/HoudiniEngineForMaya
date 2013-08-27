#include "AssetSubCommandLoadOTL.h"

#include <maya/MFileObject.h>
#include <maya/MPlug.h>

#include "AssetNode.h"
#include "util.h"

AssetSubCommandLoadOTL::AssetSubCommandLoadOTL(
	const MString &otlFile
	) :
    myOTLFile(otlFile)
{
}

AssetSubCommandLoadOTL::~AssetSubCommandLoadOTL()
{
}

MStatus
AssetSubCommandLoadOTL::doIt()
{
    MStatus status;

    MString assetName;
    {
	MFileObject file;
	file.setRawFullName( myOTLFile );
	MString fileName = file.rawName();
	int dotIndex = fileName.rindexW('.');
	if(dotIndex < 0)
	{
	    dotIndex = fileName.numChars();
	}
	assetName = fileName.substringW(0, dotIndex-1);
	//TODO: check for invalid characters
    }

    // create houdiniAsset node
    MObject assetNode = dynamic_cast<MDGModifier&>(myDagModifier).createNode("houdiniAsset", &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // rename houdiniAsset node
    status = myDagModifier.renameNode(assetNode, assetName + "_asset");
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // set filename attribute
    {
	MPlug plug(assetNode, AssetNode::fileNameAttr);
	status = myDagModifier.newPlugValueString(plug, myOTLFile);
	CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // create transform node
    MObject assetTransform = myDagModifier.createNode("transform", MObject::kNullObj, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // rename transform node
    status = myDagModifier.renameNode(assetTransform, assetName + "_transform");
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // time1.outTime -> houdiniAsset.inTime
    {
	MObject srcNode = Util::findNodeByName("time1");
	MPlug srcPlug = MFnDependencyNode(srcNode).findPlug("outTime");
	MPlug dstPlug(assetNode, AssetNode::timeInput);

	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    return redoIt();
}

MStatus
AssetSubCommandLoadOTL::redoIt()
{
    MStatus status;

    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

MStatus
AssetSubCommandLoadOTL::undoIt()
{
    MStatus status;

    status = myDagModifier.undoIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

bool AssetSubCommandLoadOTL::isUndoable() const
{
    return true;
}
