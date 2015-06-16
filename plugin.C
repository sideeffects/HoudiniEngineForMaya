#include <maya/MAnimControl.h>
#include <maya/MCallbackIdArray.h>
#include <maya/MEventMessage.h>
#include <maya/MGlobal.h>

#include <maya/MFnPlugin.h>

#include "EngineCommand.h"
#include "AssetCommand.h"
#include "AssetNode.h"
#include "CurveMeshInputNode.h"
#include "FluidGridConvert.h"
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
        HAPI_GetEnvInt(Util::theHAPISession.get(), HAPI_ENVINT_VERSION_HOUDINI_MAJOR, &i);
        HAPI_GetEnvInt(Util::theHAPISession.get(), HAPI_ENVINT_VERSION_HOUDINI_MINOR, &j);
        HAPI_GetEnvInt(Util::theHAPISession.get(), HAPI_ENVINT_VERSION_HOUDINI_BUILD, &k);

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
        HAPI_GetEnvInt(Util::theHAPISession.get(), HAPI_ENVINT_VERSION_HOUDINI_ENGINE_MAJOR, &i);
        HAPI_GetEnvInt(Util::theHAPISession.get(), HAPI_ENVINT_VERSION_HOUDINI_ENGINE_MINOR, &j);
        HAPI_GetEnvInt(Util::theHAPISession.get(), HAPI_ENVINT_VERSION_HOUDINI_ENGINE_API, &k);

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

void
initializeOptionVars()
{
    bool exists;

    MGlobal::optionVarIntValue("houdiniEngineAsynchronousMode", &exists);
    if(!exists)
    {
        MGlobal::setOptionVarValue("houdiniEngineAsynchronousMode", 1);
    }
}

bool
initializeHAPI()
{
    if(HAPI_IsInitialized( Util::theHAPISession.get() ) == HAPI_RESULT_SUCCESS)
    {
        MGlobal::displayInfo("Houdini Engine is already initialized. Skipping initialization.");
        return true;
    }

    HAPI_Result hstat = HAPI_RESULT_SUCCESS;

    const char* otl_dir = getenv("HAPI_OTL_PATH");
    const char* dso_dir = getenv("HAPI_DSO_PATH");

    HAPI_CookOptions cook_options = HAPI_CookOptions_Create();
    cook_options.maxVerticesPerPrimitive = -1;
    cook_options.refineCurveToLinear = false;

    bool use_cooking_thread =
        MGlobal::optionVarIntValue("houdiniEngineAsynchronousMode") == 1;

    hstat = HAPI_Initialize(
            Util::theHAPISession.get(), otl_dir, dso_dir,
            &cook_options,
            use_cooking_thread,
            -1
            );
    if(hstat != HAPI_RESULT_SUCCESS)
    {
        CHECK_HAPI(hstat);
        return false;
    }

    return true;
}

bool
cleanupHAPI()
{
    HAPI_Result hstat = HAPI_RESULT_SUCCESS;

    hstat = HAPI_Cleanup( Util::theHAPISession.get() );
    if(hstat != HAPI_RESULT_SUCCESS)
    {
        CHECK_HAPI(hstat);
        return false;
    }

    return true;
}

void updateTimelineCallback(void* clientData)
{
    HAPI_TimelineOptions timelineOptions;

    MTime oneUnitTime;

    // Houdini's "frame 1" is "0 seconds", but Maya's "frame 0" is "0 seconds".
    // So we need to offset the time by 1.
    timelineOptions.fps = float( 1.0 / oneUnitTime.as(MTime::kSeconds) );
    timelineOptions.startTime =
        (MAnimControl::animationStartTime() - oneUnitTime)
        .as(MTime::kSeconds);
    timelineOptions.endTime =
        (MAnimControl::animationEndTime() - oneUnitTime)
        .as(MTime::kSeconds);

    HAPI_SetTimelineOptions(Util::theHAPISession.get(), &timelineOptions);
}

MCallbackIdArray messageCallbacks;

void
initializeMessageCallbacks()
{
    MStatus status;

    MCallbackId callbackId;

    callbackId = MEventMessage::addEventCallback(
            "playbackRangeSliderChanged",
            updateTimelineCallback,
            Util::theHAPISession.get(),
            &status
            );
    if(status)
    {
        messageCallbacks.append(callbackId);
    }
    else
    {
        CHECK_MSTATUS(status);
    }

    callbackId = MEventMessage::addEventCallback(
            "timeUnitChanged",
            updateTimelineCallback,
            Util::theHAPISession.get(),
            &status
            );
    if(status)
    {
        messageCallbacks.append(callbackId);
    }
    else
    {
        CHECK_MSTATUS(status);
    }
}

void
cleanupMessageCallbacks()
{
    MStatus status;

    status = MMessage::removeCallbacks(messageCallbacks);
    CHECK_MSTATUS(status);
}

MStatus
initializePlugin(MObject obj)
{
    printHAPIVersion();

    initializeOptionVars();

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

    status = plugin.registerNode(
            CurveMeshInputNode::typeName,
            CurveMeshInputNode::typeId,
            CurveMeshInputNode::creator,
            CurveMeshInputNode::initialize
            );
    CHECK_MSTATUS_AND_RETURN_IT(status);

#if MAYA_API_VERSION >= 201400
    status = plugin.registerNode(
            FluidGridConvert::typeName,
            FluidGridConvert::typeId,
            FluidGridConvert::creator,
            FluidGridConvert::initialize
            );
#endif

    status = plugin.registerCommand(
            EngineCommand::commandName,
            EngineCommand::creator,
            EngineCommand::newSyntax
            );
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = plugin.registerCommand( "houdiniAsset", AssetCommand::creator,
                                     AssetCommand::newSyntax);

    CHECK_MSTATUS_AND_RETURN_IT(status);

    initializeMessageCallbacks();

    // update the timeline option for the first time
    updateTimelineCallback(Util::theHAPISession.get());

    return status;
}

MStatus
uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);

    cleanupMessageCallbacks();

    status = plugin.deregisterNode(AssetNode::typeId);
    CHECK_MSTATUS_AND_RETURN_IT(status);

#if MAYA_API_VERSION >= 201400
    status = plugin.deregisterNode(FluidGridConvert::typeId);
    CHECK_MSTATUS_AND_RETURN_IT(status);
#endif

    status = plugin.deregisterCommand(EngineCommand::commandName);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = plugin.deregisterCommand("houdiniAsset");
    CHECK_MSTATUS_AND_RETURN_IT(status);

    if(cleanupHAPI())
    {
	MGlobal::displayInfo("Houdini Engine cleaned up successfully.");
    }
    else
    {
	MGlobal::displayInfo("Houdini Engine failed to clean up.");
	return MStatus::kFailure;
    }

    return status;
}
