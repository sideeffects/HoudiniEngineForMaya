#include "AssetSyncOutputGeoPart.h"

#include <maya/MDagPath.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>

#include <maya/MFnDagNode.h>

#include "AssetNode.h"
#include "util.h"

AssetSyncOutputGeoPart::AssetSyncOutputGeoPart(
	const MPlug &outputPlug,
	const MObject &assetTransform
	) :
    myOutputPlug(outputPlug),
    myAssetTransform(assetTransform)
{
}

AssetSyncOutputGeoPart::~AssetSyncOutputGeoPart()
{
}

MStatus
AssetSyncOutputGeoPart::doIt()
{
    MStatus status;

    MString objectName;
    MString partName;
    {
	MString outputObjectName = myOutputPlug.child(AssetNode::objectName).asString();
	int separatorIndex = outputObjectName.rindexW('/');
	objectName = outputObjectName.substringW(0, separatorIndex-1);
	partName = outputObjectName.substringW(separatorIndex + 1, outputObjectName.numChars() - 1);
    }

    MFnDagNode assetTransformFn(myAssetTransform);

    // Since the objectTransform is shared by multiple parts, it may or may not
    // already exist. Create objectTransform if it doesn't exist.
    MObject objectTransform = Util::findDagChild(assetTransformFn, objectName);
    if(objectTransform.isNull())
    {
	// create objectTransform
	objectTransform = myDagModifier.createNode("transform", myAssetTransform, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// rename objectTransform
	status = myDagModifier.renameNode(objectTransform, objectName);
	CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    MFnDagNode objectTransformFn(objectTransform);

    // create part
    MObject partTransform = Util::findDagChild(objectTransformFn, partName);
    if(partTransform.isNull())
    {
	status = createOutputPart(objectTransform, partName, partTransform);
    }

    // create material
    MPlug materialPlug = myOutputPlug.child(AssetNode::material);
    MPlug materialExistsPlug = materialPlug.child(AssetNode::materialExists);
    if(materialExistsPlug.asBool())
    {
	//TODO: check if material already exists
	status = createOutputMaterial(materialPlug, partTransform);
	CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    return redoIt();
}

MStatus
AssetSyncOutputGeoPart::undoIt()
{
    MStatus status;

    status = myDagModifier.undoIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

MStatus
AssetSyncOutputGeoPart::redoIt()
{
    MStatus status;

    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

MStatus
AssetSyncOutputGeoPart::createOutputPart(
	const MObject &objectTransform,
	const MString &partName,
	MObject &partTransform
	)
{
    MStatus status;

    // create partTransform
    partTransform = myDagModifier.createNode("transform", objectTransform, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // rename partTransform
    status = myDagModifier.renameNode(partTransform, partName);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // create partMesh
    MObject partMesh = myDagModifier.createNode("mesh", partTransform, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // rename partMesh
    status = myDagModifier.renameNode(partMesh, partName + "Shape");
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFnDependencyNode partTransformFn(partTransform, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFnDependencyNode partMeshFn(partMesh, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // connect partTransform attributes
    {
	MPlug transformPlug = myOutputPlug.child(AssetNode::transform);

	MPlug srcPlug;
	MPlug dstPlug;

	srcPlug = transformPlug.child(AssetNode::translateAttr);
	dstPlug = partTransformFn.findPlug("translate");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	srcPlug = transformPlug.child(AssetNode::rotateAttr);
	dstPlug = partTransformFn.findPlug("rotate");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	srcPlug = transformPlug.child(AssetNode::scaleAttr);
	dstPlug = partTransformFn.findPlug("scale");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // connect partMesh attributes
    {
	MPlug srcPlug;
	MPlug dstPlug;

	srcPlug = myOutputPlug.child(AssetNode::mesh);
	dstPlug = partMeshFn.findPlug("inMesh");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // doIt
    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

MStatus
AssetSyncOutputGeoPart::createOutputMaterial(
	const MPlug &materialPlug,
	const MObject &partTransform
	)
{
    MStatus status;

    // can't use partTransform directly to select, need to use MDagPath
    MDagPath partTransformDag;
    status = MDagPath::getAPathTo(partTransform, partTransformDag);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // save current selection
    MSelectionList oldSelection;
    MGlobal::getActiveSelectionList(oldSelection);

    MSelectionList selection;

    // create shader
    MFnDependencyNode shaderFn;
    {
	MObject shader;
	status = Util::createNodeByModifierCommand(
		myDagModifier,
		"shadingNode -asShader phong",
		shader
		);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = shaderFn.setObject(shader);
	CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // select partTransform
    status = selection.add(partTransformDag);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    MGlobal::setActiveSelectionList(selection);
    selection.clear();

    // assign shader
    status = myDagModifier.commandToExecute("hyperShade -assign " + shaderFn.name());
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // create file node if texture exists
    MPlug texturePathPlug = materialPlug.child(AssetNode::texturePath);
    MString texturePath = texturePathPlug.asString();
    MFnDependencyNode textureFileFn;
    if(texturePath.length())
    {
	MObject textureFile;
	status = Util::createNodeByModifierCommand(
		myDagModifier,
		"shadingNode -asTexture file",
		textureFile
		);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = textureFileFn.setObject(textureFile);
	CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // connect shader attributse
    {
	MPlug srcPlug;
	MPlug dstPlug;

	// color
	if(textureFileFn.object().isNull())
	{
	    srcPlug = materialPlug.child(AssetNode::diffuseAttr);
	    dstPlug = shaderFn.findPlug("color");
	    status = myDagModifier.connect(srcPlug, dstPlug);
	    CHECK_MSTATUS_AND_RETURN_IT(status);
	}
	else
	{
	    dstPlug = textureFileFn.findPlug("fileTextureName");
	    status = myDagModifier.connect(texturePathPlug, dstPlug);
	    CHECK_MSTATUS_AND_RETURN_IT(status);

	    srcPlug = textureFileFn.findPlug("outColor");
	    dstPlug = shaderFn.findPlug("color");
	    status = myDagModifier.connect(srcPlug, dstPlug);
	    CHECK_MSTATUS_AND_RETURN_IT(status);
	}

	// specularColor
	srcPlug = materialPlug.child(AssetNode::specularAttr);
	dstPlug = shaderFn.findPlug("specularColor");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// ambientColor
	srcPlug = materialPlug.child(AssetNode::ambientAttr);
	dstPlug = shaderFn.findPlug("ambientColor");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// transparency
	srcPlug = materialPlug.child(AssetNode::alphaAttr);
	dstPlug = shaderFn.findPlug("transparency");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // restore old selection
    MGlobal::setActiveSelectionList(oldSelection);

    // doIt
    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}
