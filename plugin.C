#include <maya/MGlobal.h>

#include <maya/MFnPlugin.h>

#include "EngineCommand.h"
#include "AssetCommand.h"
#include "AssetNode.h"
#include "FluidVelocityConvert.h"
#include "util.h"

#include <HAPI/HAPI_Version.h>

#include <cstdlib>

void
printHAPIVersion()
{
    int i, j, k;
    char version[32];
    MString msg;

    {
        HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_MAJOR, &i);
        HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_MINOR, &j);
        HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_BUILD, &k);

        msg = "Houdini version: ";
        sprintf(version, "%d.%d.%d", i, j, k);
        msg += version;
        if(!(i == HAPI_VERSION_HOUDINI_MAJOR
                    && j == HAPI_VERSION_HOUDINI_MINOR
                    && k == HAPI_VERSION_HOUDINI_BUILD))
        {
            msg += ", expected: ";
            sprintf(version, "%d.%d.%d",
                    HAPI_VERSION_HOUDINI_MAJOR,
                    HAPI_VERSION_HOUDINI_MINOR,
                    HAPI_VERSION_HOUDINI_BUILD);
            msg += version;
        }

        MGlobal::displayInfo(msg);
    }

    {
        HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_ENGINE_MAJOR, &i);
        HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_ENGINE_MINOR, &j);
        HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_ENGINE_API, &k);

        msg = "Houdini Engine version: ";
        sprintf(version, "%d.%d (API: %d)", i, j, k);
        msg += version;

        if(!(i == HAPI_VERSION_HOUDINI_ENGINE_MAJOR
                    && j == HAPI_VERSION_HOUDINI_ENGINE_MINOR
                    && k == HAPI_VERSION_HOUDINI_ENGINE_API))
        {
            msg += ", expected: ";
            sprintf(version, "%d.%d (API: %d)",
                    HAPI_VERSION_HOUDINI_ENGINE_MAJOR,
                    HAPI_VERSION_HOUDINI_ENGINE_MINOR,
                    HAPI_VERSION_HOUDINI_ENGINE_API);
            msg += version;
        }

        MGlobal::displayInfo(msg);
    }
}

bool
initializeHAPI()
{
    if(HAPI_IsInitialized() == HAPI_RESULT_SUCCESS)
    {
        return true;
    }

    HAPI_Result hstat = HAPI_RESULT_SUCCESS;

    MString otl_dir(getenv("HAPI_OTL_PATH"));
    MString dso_dir(getenv("HAPI_DSO_PATH"));

    HAPI_CookOptions cook_options = HAPI_CookOptions_Create();
    cook_options.maxVerticesPerPrimitive = -1;
    cook_options.refineCurveToLinear = false;
    hstat = HAPI_Initialize(otl_dir.asChar(),
            dso_dir.asChar(), &cook_options, true, -1);
    if(hstat != HAPI_RESULT_SUCCESS)
    {
        CHECK_HAPI(hstat);
        return false;
    }

    return true;
}

MStatus
initializePlugin(MObject obj)
{
    printHAPIVersion();

    char engine_version[32];
    sprintf(engine_version, "%d.%d (API: %d)",
            HAPI_VERSION_HOUDINI_ENGINE_MAJOR,
            HAPI_VERSION_HOUDINI_ENGINE_MINOR,
            HAPI_VERSION_HOUDINI_ENGINE_API);

    MStatus status;
    MFnPlugin plugin(
            obj, "Side Effects",
            engine_version,
            "Any"
            );

    if(initializeHAPI())
    {
        MGlobal::displayInfo("Houdini Engine initialized successfully.");
    }
    else
    {
        MGlobal::displayInfo("Houdini Engine failed to initialize.");
        return MStatus::kFailure;
    }

    status = plugin.registerUI("houdiniEngineCreateUI", "houdiniEngineDeleteUI");
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = plugin.registerTransform(
            AssetNode::typeName,
            AssetNode::typeId,
            AssetNode::creator,
            AssetNode::initialize,
            MPxTransformationMatrix::creator,
            MPxTransformationMatrix::baseTransformationMatrixId
            );
    CHECK_MSTATUS_AND_RETURN_IT(status);

#if MAYA_API_VERSION >= 201400
    status = plugin.registerNode(
            FluidVelocityConvert::typeName,
            FluidVelocityConvert::typeId,
            FluidVelocityConvert::creator,
            FluidVelocityConvert::initialize
            );
#endif

    status = plugin.registerCommand(
            EngineCommand::commandName,
            EngineCommand::creator,
            EngineCommand::newSyntax
            );
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = plugin.registerCommand("houdiniAsset", AssetCommand::creator, AssetCommand::newSyntax);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}

MStatus
uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);

    status = plugin.deregisterNode(AssetNode::typeId);
    CHECK_MSTATUS_AND_RETURN_IT(status);

#if MAYA_API_VERSION >= 201400
    status = plugin.deregisterNode(FluidVelocityConvert::typeId);
    CHECK_MSTATUS_AND_RETURN_IT(status);
#endif

    status = plugin.deregisterCommand(EngineCommand::commandName);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = plugin.deregisterCommand("houdiniAsset");
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return status;
}
