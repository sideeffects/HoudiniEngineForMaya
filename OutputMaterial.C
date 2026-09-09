#include "OutputMaterial.h"

#include <maya/MDataHandle.h>
#include <maya/MStatus.h>

#include "AssetNode.h"
#include "util.h"

OutputMaterial::OutputMaterial(HAPI_NodeId assetId)
    : myAssetId(assetId),
      myNodeId(-1),
      myMaterialLastCookCount(0),
      myBakeTexture(0)
{
}

static int findMaterialParmIndex(std::vector<HAPI_ParmInfo> &parms,
    const std::vector<MString> names, const std::vector<int> instanceNums)
{
    int parmIndex = -1;
    const int size = names.size();
    for (int i = 0; i < size; i++)
    {
        const MString &name = names[i];
        const int instanceNum = instanceNums[i];
        parmIndex = Util::findParm(parms, name, instanceNum);
        if (parmIndex >= 0)
            return parmIndex;
    }
    return parmIndex;
}

HAPI_Result OutputMaterial::handleDiffuseTexture(const int texturePathSHParmIndex, MDataHandle &texturePathHandle)
{
    HAPI_Result hapiResult;

    HAPI_ParmInfo texturePathParm;
    HoudiniApi::GetParameters(Util::theHAPISession.get(), myNodeId,
                        &texturePathParm, texturePathSHParmIndex, 1);

    int texturePathSH;
    HoudiniApi::GetParmStringValues(Util::theHAPISession.get(), myNodeId, true,
                                &texturePathSH,
                                texturePathParm.stringValuesIndex, 1);

    bool hasTextureSource =
        ((std::string)Util::HAPIString(texturePathSH)).size() > 0;
    bool canRenderTexture = false;
    if (hasTextureSource && myBakeTexture)
    {
        // this could fail if texture parameter is empty
        hapiResult = HoudiniApi::RenderTextureToImage(
            Util::theHAPISession.get(), myNodeId,
            texturePathSHParmIndex);

        canRenderTexture = hapiResult == HAPI_RESULT_SUCCESS;
    }

    int destinationFilePathSH = 0;
    MString destinationFolderPath;
    MGlobal::executeCommand(
        "workspace -expandName "
        "`workspace -q -fileRuleEntry sourceImages`;",
        destinationFolderPath);

    if (canRenderTexture && myBakeTexture)
    {
        // this could fail if the image planes don't exist
        hapiResult = HoudiniApi::ExtractImageToFile(
            Util::theHAPISession.get(), myNodeId, HAPI_PNG_FORMAT_NAME,
            "C A", destinationFolderPath.asChar(), NULL,
            &destinationFilePathSH);
        if (HAPI_FAIL(hapiResult))
        {
            DISPLAY_ERROR("Could not extract image to directory:\n"
                            "    ^1s",
                            destinationFolderPath);
            DISPLAY_ERROR_HAPI_STATUS_CALL();
        }
    }

    if (hasTextureSource && !myBakeTexture)
    {
        // if baking is off but the expected texture file exists
        // keep using it
        hapiResult = HoudiniApi::GetImageFilePath(
            Util::theHAPISession.get(), myNodeId, HAPI_PNG_FORMAT_NAME,
            "C A", destinationFolderPath.asChar(), NULL,
            texturePathSHParmIndex, &destinationFilePathSH);

        MString texturePath = Util::HAPIString(destinationFilePathSH);
        MString cmd         = "filetest -e \"" + texturePath + "\"";
        int fileExists;
        MGlobal::executeCommand(cmd, fileExists);
        if (!fileExists)
            destinationFilePathSH = 0;
    }

    if (destinationFilePathSH > 0)
    {
        MString texturePath = Util::HAPIString(destinationFilePathSH);
        texturePathHandle.set(texturePath);
    }

    return hapiResult;
}

MStatus
OutputMaterial::compute(const MTime &time,
                        const MPlug &materialPlug,
                        MDataBlock &data,
                        MDataHandle &materialHandle,
                        bool bakeTexture)
{
    data.setClean(materialPlug);

    update(materialHandle);

    MDataHandle nodeIdHandle =
        materialHandle.child(AssetNode::outputMaterialNodeId);

    nodeIdHandle.setInt(myNodeId);

    if (myNodeId < 0)
    {
        return MStatus::kFailure;
    }

    MDataHandle nameHandle =
        materialHandle.child(AssetNode::outputMaterialName);
    MDataHandle ambientHandle =
        materialHandle.child(AssetNode::outputMaterialAmbientColor);
    MDataHandle diffuseHandle =
        materialHandle.child(AssetNode::outputMaterialDiffuseColor);
    MDataHandle specularHandle =
        materialHandle.child(AssetNode::outputMaterialSpecularColor);
    MDataHandle alphaHandle =
        materialHandle.child(AssetNode::outputMaterialAlphaColor);
    MDataHandle texturePathHandle =
        materialHandle.child(AssetNode::outputMaterialTexturePath);

    HAPI_MaterialInfo materialInfo;
    CHECK_HAPI(HoudiniApi::GetMaterialInfo(
        Util::theHAPISession.get(), myNodeId, &materialInfo));

    if (myNodeInfo.totalCookCount > myMaterialLastCookCount ||
        materialInfo.hasChanged || bakeTexture != myBakeTexture)
    {
        myBakeTexture = bakeTexture;
        std::vector<HAPI_ParmInfo> parms(myNodeInfo.parmCount);
        HoudiniApi::GetParameters(Util::theHAPISession.get(), myNodeId, &parms[0], 0,
                           myNodeInfo.parmCount);

        int ambientParmIndex       = findMaterialParmIndex(parms, {"ogl_amb", "emitcolor"}, {-1, -1});
        int diffuseParmIndex       = findMaterialParmIndex(parms, {"ogl_diff", "basecolor"}, {-1, -1});
        int alphaParmIndex         = findMaterialParmIndex(parms, {"ogl_alpha", "transparency"}, {-1, -1});
        int specularParmIndex      = findMaterialParmIndex(parms, {"ogl_spec"}, {-1});
        int texturePathSHParmIndex = findMaterialParmIndex(parms, {"ogl_tex#", "basecolor_texture"}, {1, -1});
        float valueHolder[4];

        nameHandle.setString(Util::HAPIString(myNodeInfo.nameSH));

        if (ambientParmIndex >= 0)
        {
            HoudiniApi::GetParmFloatValues(
                Util::theHAPISession.get(), myNodeId, valueHolder,
                parms[ambientParmIndex].floatValuesIndex, 3);
            
            const int emitIntensityParmIndex = findMaterialParmIndex(parms, {"emitint"}, {-1});
            if (emitIntensityParmIndex >= 0)
            {
                float emitIntensityValueHolder[4];
                HoudiniApi::GetParmFloatValues(Util::theHAPISession.get(), myNodeId,
                    emitIntensityValueHolder,
                    parms[emitIntensityParmIndex].floatValuesIndex, 1);

                for (int i = 0; i < 3; i++)
                    valueHolder[i] *= emitIntensityValueHolder[0];
            }

            ambientHandle.set3Float(
                valueHolder[0], valueHolder[1], valueHolder[2]);
        }

        if (specularParmIndex >= 0)
        {
            HoudiniApi::GetParmFloatValues(
                Util::theHAPISession.get(), myNodeId, valueHolder,
                parms[specularParmIndex].floatValuesIndex, 3);
            specularHandle.set3Float(
                valueHolder[0], valueHolder[1], valueHolder[2]);
        }

        if (diffuseParmIndex >= 0)
        {
            HAPI_ParmInfo diffuseParmInfo;
            HoudiniApi::GetParameters(Util::theHAPISession.get(), myNodeId,
                &diffuseParmInfo, diffuseParmIndex, 1);

            // "basecolor" is a color in the Principled Shader, but a texture in
            // the COP Viewer Material.
            if (diffuseParmInfo.type == HAPI_PARMTYPE_COLOR)
            {
                HoudiniApi::GetParmFloatValues(
                    Util::theHAPISession.get(), myNodeId, valueHolder,
                    parms[diffuseParmIndex].floatValuesIndex, 3);
                diffuseHandle.set3Float(
                    valueHolder[0], valueHolder[1], valueHolder[2]);
            }
            else
                handleDiffuseTexture(diffuseParmIndex, texturePathHandle);
        }

        if (alphaParmIndex >= 0)
        {
            HoudiniApi::GetParmFloatValues(Util::theHAPISession.get(), myNodeId,
                                    valueHolder,
                                    parms[alphaParmIndex].floatValuesIndex, 1);

            float alpha = valueHolder[0];
            if (Util::findParm(parms, "ogl_alpha", -1) >= 0)
                alpha = 1.0f - valueHolder[0];
            alphaHandle.set3Float(alpha, alpha, alpha);
        }

        if (texturePathSHParmIndex >= 0)
            handleDiffuseTexture(texturePathSHParmIndex, texturePathHandle);

        myMaterialLastCookCount = myNodeInfo.totalCookCount;
    }

    return MStatus::kSuccess;
}

void
OutputMaterial::update(MDataHandle &materialHandle)
{
    MDataHandle pathHandle =
        materialHandle.child(AssetNode::outputMaterialPath);
    std::string path = pathHandle.asString().asChar();

    if (myNodePath != path)
    {
        myNodeId = -1;
    }

    // find node id from path
    if (myNodeId < 0)
    {
        int count;
        CHECK_HAPI(
            HoudiniApi::ComposeChildNodeList(Util::theHAPISession.get(), myAssetId,
                                      HAPI_NODETYPE_SHOP | HAPI_NODETYPE_VOP,
                                      HAPI_NODEFLAGS_ANY, true, &count));

        std::vector<HAPI_NodeId> nodeIds(count);
        if (count)
        {
            CHECK_HAPI(HoudiniApi::GetComposedChildNodeList(
                Util::theHAPISession.get(), myAssetId, &nodeIds[0], count));
        }

        for (size_t i = 0; i < nodeIds.size(); i++)
        {
            HAPI_NodeId testNodeId = nodeIds[i];

            HAPI_StringHandle testPath;

            CHECK_HAPI(HoudiniApi::GetNodePath(
                Util::theHAPISession.get(), testNodeId, myAssetId, &testPath));

            if (Util::HAPIString(testPath) == path)
            {
                myNodeId = testNodeId;
                break;
            }
        }
    }

    if (myNodeId < 0)
    {
        return;
    }

    myNodePath = path;

    // get material info
    CHECK_HAPI(
        HoudiniApi::GetNodeInfo(Util::theHAPISession.get(), myNodeId, &myNodeInfo));
}

