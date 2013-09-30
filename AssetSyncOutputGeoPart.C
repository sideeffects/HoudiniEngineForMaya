#include "AssetSyncOutputGeoPart.h"

#include <maya/MDagPath.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>

#include <maya/MFnDagNode.h>

#include "AssetNode.h"
#include "util.h"

AssetSyncOutputGeoPart::AssetSyncOutputGeoPart(
	const MPlug &outputPlug,
	const MObject &assetNodeObj
	) :
    myOutputPlug(outputPlug),
    myAssetNodeObj(assetNodeObj)
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
	MString outputPartName = myOutputPlug.child(AssetNode::outputPartName).asString();
	int separatorIndex = outputPartName.rindexW('/');
	objectName = outputPartName.substringW(0, separatorIndex-1);
	partName = outputPartName.substringW(separatorIndex + 1, outputPartName.numChars() - 1);
    }

    MFnDagNode assetNodeFn(myAssetNodeObj);

    // Since the objectTransform is shared by multiple parts, it may or may not
    // already exist. Create objectTransform if it doesn't exist.
    MObject objectTransform = Util::findDagChild(assetNodeFn, objectName);
    if(objectTransform.isNull())
    {
	// create objectTransform
	objectTransform = myDagModifier.createNode("transform", myAssetNodeObj, &status);
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
    MPlug materialPlug = myOutputPlug.child(AssetNode::outputPartMaterial);
    MPlug materialExistsPlug = materialPlug.child(AssetNode::outputPartMaterialExists);
    if(materialExistsPlug.asBool())
    {
	//TODO: check if material already exists
	status = createOutputMaterial(materialPlug, partTransform);
	CHECK_MSTATUS_AND_RETURN_IT(status);
    }
    else
    {
	MFnDagNode partTransformFn(partTransform);
	status = myDagModifier.commandToExecute("assignSG \"lambert1\" \"" + partTransformFn.fullPathName() + "\";");
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
	MPlug transformPlug = myOutputPlug.array().parent().child(AssetNode::outputObjectTransform);

	MPlug srcPlug;
	MPlug dstPlug;

	srcPlug = transformPlug.child(AssetNode::outputObjectTranslate);
	dstPlug = partTransformFn.findPlug("translate");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	srcPlug = transformPlug.child(AssetNode::outputObjectRotate);
	dstPlug = partTransformFn.findPlug("rotate");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	srcPlug = transformPlug.child(AssetNode::outputObjectScale);
	dstPlug = partTransformFn.findPlug("scale");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // connect partMesh attributes
    {
	MPlug srcPlug;
	MPlug dstPlug;

	srcPlug = myOutputPlug.child(AssetNode::outputPartMesh);
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

    MFnDagNode partTransformFn(partTransform, &status);

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

    // assign shader
    status = myDagModifier.commandToExecute(
	    "assignSG " + shaderFn.name() + " " + partTransformFn.fullPathName()
	    );
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // create file node if texture exists
    MPlug texturePathPlug = materialPlug.child(AssetNode::outputPartTexturePath);
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
	    srcPlug = materialPlug.child(AssetNode::outputPartDiffuseColor);
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
	srcPlug = materialPlug.child(AssetNode::outputPartSpecularColor);
	dstPlug = shaderFn.findPlug("specularColor");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// ambientColor
	srcPlug = materialPlug.child(AssetNode::outputPartAmbientColor);
	dstPlug = shaderFn.findPlug("ambientColor");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// transparency
	srcPlug = materialPlug.child(AssetNode::outputPartAlphaColor);
	dstPlug = shaderFn.findPlug("transparency");
	status = myDagModifier.connect(srcPlug, dstPlug);
	CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // doIt
    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}
