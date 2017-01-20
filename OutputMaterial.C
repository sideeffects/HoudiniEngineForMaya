#include "OutputMaterial.h"

#include <maya/MDataHandle.h>
#include <maya/MStatus.h>

#include "AssetNode.h"
#include "util.h"

OutputMaterial::OutputMaterial() :
    myNodeId(-1),
    myMaterialLastCookCount(0)
{
}

MStatus
OutputMaterial::compute(
        const MTime &time,
        MDataHandle &materialHandle
        )
{
    HAPI_Result hapiResult;

    MDataHandle nodeIdHandle
        = materialHandle.child(AssetNode::outputMaterialNodeId);

    nodeIdHandle.setInt(myNodeId);

    if(myNodeId < 0)
    {
        return MStatus::kFailure;
    }

    MDataHandle nameHandle
        = materialHandle.child(AssetNode::outputMaterialName);
    MDataHandle ambientHandle
        = materialHandle.child(AssetNode::outputMaterialAmbientColor);
    MDataHandle diffuseHandle
        = materialHandle.child(AssetNode::outputMaterialDiffuseColor);
    MDataHandle specularHandle
        = materialHandle.child(AssetNode::outputMaterialSpecularColor);
    MDataHandle alphaHandle
        = materialHandle.child(AssetNode::outputMaterialAlphaColor);
    MDataHandle texturePathHandle
        = materialHandle.child(AssetNode::outputMaterialTexturePath);

    HAPI_MaterialInfo materialInfo;
    CHECK_HAPI(HAPI_GetMaterialInfo(
            Util::theHAPISession.get(),
            myNodeId,
            &materialInfo));

    if(myNodeInfo.totalCookCount > myMaterialLastCookCount
            || materialInfo.hasChanged)
    {
        std::vector<HAPI_ParmInfo> parms(myNodeInfo.parmCount);
        HAPI_GetParameters(
                Util::theHAPISession.get(),
                myNodeId,
                &parms[0],
                0, myNodeInfo.parmCount
                );

        int ambientParmIndex = Util::findParm(parms, "ogl_amb");
        int diffuseParmIndex = Util::findParm(parms, "ogl_diff");
        int alphaParmIndex = Util::findParm(parms, "ogl_alpha");
        int specularParmIndex = Util::findParm(parms, "ogl_spec");
        int texturePathSHParmIndex = Util::findParm(parms, "ogl_tex#", 1);
        float valueHolder[4];

        nameHandle.setString(
                Util::HAPIString(myNodeInfo.nameSH)
                );

        if(ambientParmIndex >= 0)
        {
            HAPI_GetParmFloatValues(
                    Util::theHAPISession.get(),
                    myNodeId, valueHolder,
                    parms[ambientParmIndex].floatValuesIndex, 3
                    );
            ambientHandle.set3Float(
                    valueHolder[0],
                    valueHolder[1],
                    valueHolder[2]
                    );
        }

        if(specularParmIndex >= 0)
        {
            HAPI_GetParmFloatValues(
                    Util::theHAPISession.get(),
                    myNodeId,
                    valueHolder,
                    parms[specularParmIndex].floatValuesIndex, 3
                    );
            specularHandle.set3Float(
                    valueHolder[0],
                    valueHolder[1],
                    valueHolder[2]
                    );
        }

        if(diffuseParmIndex >= 0)
        {
            HAPI_GetParmFloatValues(
                    Util::theHAPISession.get(),
                    myNodeId,
                    valueHolder,
                    parms[diffuseParmIndex].floatValuesIndex, 3
                    );
            diffuseHandle.set3Float(
                    valueHolder[0],
                    valueHolder[1],
                    valueHolder[2]
                    );
        }

        if(alphaParmIndex >= 0)
        {
            HAPI_GetParmFloatValues(
                    Util::theHAPISession.get(),
                    myNodeId,
                    valueHolder,
                    parms[alphaParmIndex].floatValuesIndex, 1
                    );
            float alpha = 1 - valueHolder[0];
            alphaHandle.set3Float(alpha, alpha, alpha);
        }

        if(texturePathSHParmIndex >= 0)
        {
            HAPI_ParmInfo texturePathParm;
            HAPI_GetParameters(
                    Util::theHAPISession.get(),
                    myNodeId,
                    &texturePathParm,
                    texturePathSHParmIndex, 1
                    );

            int texturePathSH;
            HAPI_GetParmStringValues(
                    Util::theHAPISession.get(),
                    myNodeId,
                    true,
                    &texturePathSH,
                    texturePathParm.stringValuesIndex, 1
                    );

            bool hasTextureSource = ((std::string)Util::HAPIString(texturePathSH)).size() > 0;
            bool canRenderTexture = false;
            if(hasTextureSource)
            {
                // this could fail if texture parameter is empty
                hapiResult = HAPI_RenderTextureToImage(
                        Util::theHAPISession.get(),
                        myNodeId,
                        texturePathSHParmIndex
                        );

                canRenderTexture = hapiResult == HAPI_RESULT_SUCCESS;
            }

            int destinationFilePathSH = 0;
            if(canRenderTexture)
            {
                MString destinationFolderPath;
                MGlobal::executeCommand("workspace -expandName "
                        "`workspace -q -fileRuleEntry sourceImages`;",
                        destinationFolderPath);

                // this could fail if the image planes don't exist
                hapiResult = HAPI_ExtractImageToFile(
                        Util::theHAPISession.get(),
                        myNodeId,
                        HAPI_PNG_FORMAT_NAME,
                        "C A",
                        destinationFolderPath.asChar(),
                        NULL,
                        &destinationFilePathSH
                        );
                if(HAPI_FAIL(hapiResult))
                {
                    DISPLAY_ERROR(
                            "Could not extract image to directory:\n"
                            "    ^1s",
                            destinationFolderPath
                            );
                    DISPLAY_ERROR_HAPI_STATUS_CALL();
                }
            }

            if(destinationFilePathSH > 0)
            {
                MString texturePath = Util::HAPIString(destinationFilePathSH);
                texturePathHandle.set(texturePath);
            }
        }

        myMaterialLastCookCount = myNodeInfo.totalCookCount;
    }

    materialHandle.setClean();
    ambientHandle.setClean();
    diffuseHandle.setClean();
    specularHandle.setClean();
    alphaHandle.setClean();
    texturePathHandle.setClean();

    return MStatus::kSuccess;
}
