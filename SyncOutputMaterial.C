#include "SyncOutputMaterial.h"

#include <maya/MDGModifier.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnDependencyNode.h>

#include "AssetNode.h"
#include "Asset.h"
#include "util.h"

#include <algorithm>

MObject
SyncOutputMaterial::createOutputMaterial(
        MDGModifier &dgModifier,
        const MObject &assetObj,
        int nodeId)
{
    MStatus status;

    // map -1 to initialShadingGroup
    if(nodeId == -1)
    {
        return Util::findNodeByName("initialShadingGroup");
    }

    MPlug materialPlug = createOutputMaterialPlug(
            dgModifier,
            assetObj,
            nodeId);
    if(materialPlug.isNull())
    {
        return MObject::kNullObj;
    }
    
    // check if the ambient output has a shader already (checking diffuse
    // would force us to check the file texture connections as well)
    
    MPlug shadingPlug = materialPlug.child(AssetNode::outputMaterialAmbientColor);	    
    MObject shaderObj = findShader(shadingPlug);

    if(shaderObj.isNull())
    {
        // create shader
        status = Util::createNodeByModifierCommand(
                dgModifier,
                "shadingNode -asShader phong",
                shaderObj
                );
        CHECK_MSTATUS_AND_RETURN(status, MObject::kNullObj);

        // rename the shader
        MPlug namePlug = materialPlug.child(AssetNode::outputMaterialName);
        dgModifier.renameNode(shaderObj, namePlug.asString());
        status = dgModifier.doIt();
        CHECK_MSTATUS_AND_RETURN(status, MObject::kNullObj);
    }

    MFnDependencyNode shaderFn(shaderObj);

    MObject shadingGroupObj = findShadingGroup(shaderObj);
    if(shadingGroupObj.isNull())
    {
        // create shading group
        status = Util::createNodeByModifierCommand(
                dgModifier,
                "select -noExpand `sets -renderable true "
                "-noSurfaceShader true -empty "
                "-name \"" + shaderFn.name() + "SG\"`",
                shadingGroupObj
                );
        CHECK_MSTATUS_AND_RETURN(status, MObject::kNullObj);

        MFnDependencyNode shadingGroupFn(shadingGroupObj);

        // connect shader to shading group
        dgModifier.commandToExecute("defaultNavigation -connectToExisting "
                "-source \"" + shaderFn.name() + "\" "
                "-destination \"" + shadingGroupFn.name() + "\"");
        status = dgModifier.doIt();
        CHECK_MSTATUS_AND_RETURN(status, MObject::kNullObj);
    }

    // create file node if texture exists
    MPlug texturePathPlug = materialPlug.child(AssetNode::outputMaterialTexturePath);
    MString texturePath = texturePathPlug.asString();
    MFnDependencyNode textureFileFn;
    if(texturePath.length())
    {
        MObject textureFile;
        status = Util::createNodeByModifierCommand(
                dgModifier,
                "shadingNode -asTexture file",
                textureFile
                );
        CHECK_MSTATUS_AND_RETURN(status, MObject::kNullObj);

        status = textureFileFn.setObject(textureFile);
        CHECK_MSTATUS_AND_RETURN(status, MObject::kNullObj);
    }

    // connect shader attributse
    {
        MPlug srcPlug;
        MPlug dstPlug;

        // color
        if(textureFileFn.object().isNull())
        {
            srcPlug = materialPlug.child(AssetNode::outputMaterialDiffuseColor);
            dstPlug = shaderFn.findPlug("color", true);
            status = dgModifier.connect(srcPlug, dstPlug);
            CHECK_MSTATUS_AND_RETURN(status, MObject::kNullObj);
        }
        else
        {
            dstPlug = textureFileFn.findPlug("fileTextureName", true);
            status = dgModifier.connect(texturePathPlug, dstPlug);
            CHECK_MSTATUS_AND_RETURN(status, MObject::kNullObj);

            srcPlug = textureFileFn.findPlug("outColor", true);
            dstPlug = shaderFn.findPlug("color", true);
            status = dgModifier.connect(srcPlug, dstPlug);
            CHECK_MSTATUS_AND_RETURN(status, MObject::kNullObj);
        }

        // specularColor
        srcPlug = materialPlug.child(AssetNode::outputMaterialSpecularColor);
        dstPlug = shaderFn.findPlug("specularColor", true);
        status = dgModifier.connect(srcPlug, dstPlug);
        CHECK_MSTATUS_AND_RETURN(status, MObject::kNullObj);

        // ambientColor
        srcPlug = materialPlug.child(AssetNode::outputMaterialAmbientColor);
        dstPlug = shaderFn.findPlug("ambientColor", true);
        status = dgModifier.connect(srcPlug, dstPlug);
        CHECK_MSTATUS_AND_RETURN(status, MObject::kNullObj);

        // transparency
        srcPlug = materialPlug.child(AssetNode::outputMaterialAlphaColor);
        dstPlug = shaderFn.findPlug("transparency", true);
        status = dgModifier.connect(srcPlug, dstPlug);
        CHECK_MSTATUS_AND_RETURN(status, MObject::kNullObj);
    }

    // doIt
    status = dgModifier.doIt();
    CHECK_MSTATUS_AND_RETURN(status, MObject::kNullObj);

    return shadingGroupObj;
}

MPlug
SyncOutputMaterial::createOutputMaterialPlug(
        MDGModifier &dgModifier,
        const MObject &assetObj,
        int nodeId)
{
    MStatus status;

    MPlug materialsPlug(assetObj, AssetNode::outputMaterials);

    // find the material plug that matches the node id
    MPlug materialPlug;
    size_t lastLogicalElement = (size_t) -1;
    {
        for(size_t i = 0; i < materialsPlug.numElements(); i++)
        {
            MPlug testMaterialPlug = materialsPlug.elementByPhysicalIndex(i);

            lastLogicalElement = testMaterialPlug.logicalIndex();

            MPlug nodeIdPlug = testMaterialPlug.child(AssetNode::outputMaterialNodeId);

            if(nodeId == nodeIdPlug.asInt())
            {
                materialPlug = testMaterialPlug;
                break;
            }
        }
    }

    if(!materialPlug.isNull())
    {
        return materialPlug;
    }

    // allocate a new material plug
    materialPlug = materialsPlug.elementByLogicalIndex(lastLogicalElement + 1);

    AssetNode* assetNode = dynamic_cast<AssetNode*>(
            MFnDependencyNode(assetObj).userNode());

    Asset* asset = assetNode->getAsset();

    status = dgModifier.newPlugValueString(
            materialPlug.child(AssetNode::outputMaterialPath),
            asset->getRelativePath(nodeId));
    CHECK_MSTATUS(status);

    status = dgModifier.doIt();
    CHECK_MSTATUS(status);

    return materialPlug;
}

MObject
SyncOutputMaterial::findShader(const MPlug &materialPlug)
{
    MPlugArray destinationPlugs = Util::plugDestination(materialPlug);
    for(size_t i = 0; i < destinationPlugs.length(); i++)
    {
        MObject shaderObj = destinationPlugs[i].node();

        MFnDependencyNode shaderFn(shaderObj);
        MString classificationString =
            MFnDependencyNode::classification(shaderFn.typeName());

        MStringArray classifications;
        classificationString.split(':', classifications);

        ArrayIterator<MStringArray> begin = arrayBegin(classifications);
        ArrayIterator<MStringArray> end = arrayEnd(classifications);
        if(std::find(begin, end, MString("shader/surface")) != end)
        {
            return shaderObj;
        }
    }

    return MObject::kNullObj;
}

MObject
SyncOutputMaterial::findShadingGroup(const MObject &shaderObj)
{
    MFnDependencyNode shaderFn(shaderObj);

    MPlugArray destinationPlugs = Util::plugDestination(
            shaderFn.findPlug("outColor", true)
            );
    for(size_t i = 0; i < destinationPlugs.length(); i++)
    {
        MObject shadingGroupObj = destinationPlugs[i].node();
        if(shadingGroupObj.hasFn(MFn::kShadingEngine))
        {
            return shadingGroupObj;
        }
    }

    return MObject::kNullObj;
}

MObject
SyncOutputMaterial::findFileTexture(const MPlug &materialPlug)
{
    MPlugArray destinationPlugs = Util::plugDestination(materialPlug);
    for(size_t i = 0; i < destinationPlugs.length(); i++)
    {
        MObject shadingGroupObj = destinationPlugs[i].node();
        if(shadingGroupObj.hasFn(MFn::kFileTexture))
        {
            return shadingGroupObj;
        }
    }

    return MObject::kNullObj;
}
