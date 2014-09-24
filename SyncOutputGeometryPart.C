#include "SyncOutputGeometryPart.h"

#include <maya/MDagPath.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>

#include <maya/MFnDagNode.h>
#include <maya/MFnNurbsCurve.h>

#include "AssetNode.h"
#include "util.h"

SyncOutputGeometryPart::SyncOutputGeometryPart(
        const MPlug &outputPlug,
        const MObject &objectTransform
        ) :
    myOutputPlug(outputPlug),
    myObjectTransform(objectTransform)
{
}

SyncOutputGeometryPart::~SyncOutputGeometryPart()
{
}

MStatus
SyncOutputGeometryPart::doIt()
{
    MStatus status;

    MString partName = myOutputPlug.child(AssetNode::outputPartName).asString();
    if(partName.length())
    {
        partName = Util::sanitizeStringForNodeName(partName);
    }
    else
    {
        partName = "emptyPart";
    }

    MFnDagNode objectTransformFn(myObjectTransform);

    // create part
    MObject partTransform = Util::findDagChild(objectTransformFn, partName);
    if(partTransform.isNull())
    {
        status = createOutputPart(myObjectTransform, partName, partTransform);
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
SyncOutputGeometryPart::undoIt()
{
    MStatus status;

    status = myDagModifier.undoIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

MStatus
SyncOutputGeometryPart::redoIt()
{
    MStatus status;

    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

bool
SyncOutputGeometryPart::isUndoable() const
{
    return true;
}

MStatus
SyncOutputGeometryPart::createOutputPart(
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

    // create mesh
    MPlug hasMeshPlug = myOutputPlug.child(AssetNode::outputPartHasMesh);
    if(hasMeshPlug.asBool())
    {
        status = createOutputMesh(
                partTransform,
                myOutputPlug.child(AssetNode::outputPartMesh)
                );
    }

    // create particle
    MPlug particleExistsPlug = myOutputPlug.child(AssetNode::outputPartHasParticles);
    if(particleExistsPlug.asBool())
    {
        status = createOutputParticle(
                partTransform,
                myOutputPlug.child(AssetNode::outputPartParticle)
                );
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    // create curves
    MPlug curveIsBezier = myOutputPlug.child(AssetNode::outputPartCurvesIsBezier);
    createOutputCurves(myOutputPlug.child(AssetNode::outputPartCurves),
                       partTransform,
                       curveIsBezier.asBool());

    // doIt
    // Need to do it here right away because otherwise the top level
    // transform won't be shared.
    status = myDagModifier.doIt();
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}

MStatus
SyncOutputGeometryPart::createOutputMesh(
        const MObject &partTransform,
        const MPlug &meshPlug
        )
{
    MStatus status;

    // create mesh transform
    MObject meshTransform = myDagModifier.createNode("transform", partTransform, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // rename mesh transform
    status = myDagModifier.renameNode(meshTransform, "mesh");
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // create mesh
    MObject meshShape = myDagModifier.createNode("mesh", meshTransform, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // rename mesh
    status = myDagModifier.renameNode(meshShape, "meshShape");
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFnDependencyNode partMeshFn(meshShape, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // set mesh.displayColors
    myDagModifier.newPlugValueBool(
            partMeshFn.findPlug("displayColors"),
            true
            );

    // connect mesh attributes
    {
        MPlug srcPlug;
        MPlug dstPlug;

        srcPlug = myOutputPlug.child(AssetNode::outputPartMesh);
        dstPlug = partMeshFn.findPlug("inMesh");
        status = myDagModifier.connect(srcPlug, dstPlug);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    return MStatus::kSuccess;
}

MStatus
SyncOutputGeometryPart::createOutputCurves(
        MPlug curvesPlug,
        const MObject &partTransform,
        bool isBezier
        )
{
    MStatus status;

    int numCurves = curvesPlug.evaluateNumElements();
    for(int i=0; i<numCurves; i++)
    {
        MPlug curvePlug = curvesPlug[i];

        MString curveTransformName;
        MString curveShapeName;
        curveTransformName.format("curve^1s", MString() + (i + 1));
        curveShapeName.format("curveShape^1s", MString() + (i + 1));

        // create curve transform
        MObject curveTransform = myDagModifier.createNode(
                "transform",
                partTransform,
                &status
                );
        CHECK_MSTATUS_AND_RETURN_IT(status);

        // rename curve transform
        status = myDagModifier.renameNode(curveTransform, curveTransformName);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        // create curve shape
        MObject curveShape = myDagModifier.createNode(
                isBezier ? "bezierCurve" : "nurbsCurve",
                curveTransform,
                &status
                );
        CHECK_MSTATUS(status);

        // rename curve shape
        status = myDagModifier.renameNode(curveShape, curveShapeName);
        CHECK_MSTATUS_AND_RETURN_IT(status);

        MFnDependencyNode curveShapeFn(curveShape, &status);
        MPlug dstPlug = curveShapeFn.findPlug("create");
        CHECK_MSTATUS(status);

        myDagModifier.connect(curvePlug, dstPlug);
    }

    return MStatus::kSuccess;
}

MStatus
SyncOutputGeometryPart::createOutputMaterial(
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

MStatus
SyncOutputGeometryPart::createOutputParticle(
        const MObject &partTransform,
        const MPlug &particlePlug
        )
{
    MStatus status;

    // create nParticle
    MObject particleTransformObj;
    status = Util::createNodeByModifierCommand(
            myDagModifier,
            "nParticle",
            particleTransformObj
            );
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // reparent it under partTransform
    myDagModifier.reparentNode(particleTransformObj, partTransform);

    // get nParticleShape
    MObject particleShapeObj;
    {
        MDagPath particleTrasnformDag;
        status = MDagPath::getAPathTo(particleTransformObj, particleTrasnformDag);
        CHECK_MSTATUS_AND_RETURN_IT(status);
        particleShapeObj = particleTrasnformDag.child(0, &status);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    MPlug srcPlug;
    MPlug dstPlug;

    MFnDependencyNode particleShapeFn(particleShapeObj);

    // connect nParticleShape attributes
    srcPlug = particlePlug.child(AssetNode::outputPartParticlePositions);
    dstPlug = particleShapeFn.findPlug("positions");
    status = myDagModifier.connect(srcPlug, dstPlug);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    srcPlug = particlePlug.child(AssetNode::outputPartParticleArrayData);
    dstPlug = particleShapeFn.findPlug("cacheArrayData");
    status = myDagModifier.connect(srcPlug, dstPlug);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // set particleRenderType to points
    status = myDagModifier.newPlugValueInt(
            particleShapeFn.findPlug("particleRenderType"),
            3);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // set playFromCache to true
    status = myDagModifier.newPlugValueBool(
            particleShapeFn.findPlug("playFromCache"),
            true);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MStatus::kSuccess;
}
