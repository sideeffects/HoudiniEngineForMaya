#include <maya/MAnimControl.h>
#include <maya/MCallbackIdArray.h>
#include <maya/MEventMessage.h>
#include <maya/MGlobal.h>
#include <maya/MSceneMessage.h>

#include <maya/MFnPlugin.h>

#include "AssetCommand.h"
#include "AssetNode.h"
#include "EngineCommand.h"
#include "FluidGridConvert.h"
#include "InputCurveNode.h"
#include "InputGeometryNode.h"
#include "InputMergeNode.h"
#include "InputTransformNode.h"
#include "OptionVars.h"
#include "Platform.h"
#include "util.h"

#include <cstdlib>
#include <fstream>
#ifndef _WIN32
#include <unistd.h>
#else
#include <process.h>
#endif

void
printHAPIVersion()
{
    int i, j, k;
    char version[32];
    MString msg;

    assert(Util::isHapilLoaded);

    {
        HoudiniApi::GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_MAJOR, &i);
        HoudiniApi::GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_MINOR, &j);
        HoudiniApi::GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_BUILD, &k);

        msg = "Houdini version: ";
        sprintf(version, "%d.%d.%d", i, j, k);
        msg += version;
        if (!(i == HAPI_VERSION_HOUDINI_MAJOR &&
              j == HAPI_VERSION_HOUDINI_MINOR &&
              k == HAPI_VERSION_HOUDINI_BUILD))
        {
            msg += ", expected: ";
            sprintf(version, "%d.%d.%d", HAPI_VERSION_HOUDINI_MAJOR,
                    HAPI_VERSION_HOUDINI_MINOR, HAPI_VERSION_HOUDINI_BUILD);
            msg += version;
        }

        MGlobal::displayInfo(msg);
    }

    {
        HoudiniApi::GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_ENGINE_MAJOR, &i);
        HoudiniApi::GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_ENGINE_MINOR, &j);
        HoudiniApi::GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_ENGINE_API, &k);

        msg = "Houdini Engine version: ";
        sprintf(version, "%d.%d (API: %d)", i, j, k);
        msg += version;

        if (!(i == HAPI_VERSION_HOUDINI_ENGINE_MAJOR &&
              j == HAPI_VERSION_HOUDINI_ENGINE_MINOR &&
              k == HAPI_VERSION_HOUDINI_ENGINE_API))
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

namespace SessionType
{
enum Enum
{
    ST_INPROCESS,
    ST_THRIFT_SOCKET,
    ST_THRIFT_PIPE
};
}

HAPI_Result
initializeSession(const OptionVars &optionVars)
{
    if (Util::theHAPISession.get())
    {
        MGlobal::displayInfo(
            "Houdini Engine session is already created. Skipping.");

        return HAPI_RESULT_SUCCESS;
    }

    const SessionType::Enum sessionType =
        static_cast<SessionType::Enum>(optionVars.sessionType.get());

    // In Process is currently crashing due to library conflicts
    // switch to named pipe with autostart, but leave existing prefs alone
    // revert this if we can resolve the library conflicts
    SessionType::Enum actualSessionType = sessionType;
    bool overrideInProcess              = false;
    if (sessionType == SessionType::ST_INPROCESS)
    {
        MGlobal::displayInfo("Houdini Engine In Process session not currently "
                             "supported, switching to auto-start named pipe "
                             "session");
        actualSessionType = SessionType::ST_THRIFT_PIPE;
        overrideInProcess = true;
    }

    Util::theHAPISession.reset(new Util::HAPISession);
    HAPI_Result sessionResult = HoudiniApi::ClearConnectionError();

    switch (actualSessionType)
    {
    case SessionType::ST_INPROCESS:
        MGlobal::displayInfo("Creating an in-process Houdini Engine session.");

        sessionResult = HoudiniApi::CreateInProcessSession(Util::theHAPISession.get());
        break;

    case SessionType::ST_THRIFT_SOCKET:
    {
        MString hostName = "localhost";
        int port         = -1;

        MString msgHostPort;

        hostName = optionVars.thriftServer.get();
        port     = optionVars.thriftPort.get();

        msgHostPort = hostName + ":" + port;

        sessionResult = HoudiniApi::CreateThriftSocketSession(
            Util::theHAPISession.get(), hostName.asChar(), port);

        if (!HAPI_FAIL(sessionResult))
        {
            MGlobal::displayInfo(
                "Connected to Houdini Engine server using TCP socket "
                "at " +
                msgHostPort + ".");
        }
        else
        {
            int errorLength = 0;

            HoudiniApi::GetConnectionErrorLength(&errorLength);

            if (errorLength > 0)
            {
                char *msg = new char[errorLength];
                HoudiniApi::GetConnectionError(msg, errorLength, true);
                MGlobal::displayError(msg);
                delete[] msg;
            }
            else
            {
                MGlobal::displayInfo(
                    "Failed to connected to Houdini Engine server using "
                    "TCP socket at " +
                    msgHostPort + ".");
            }
        }
    }
    break;

    case SessionType::ST_THRIFT_PIPE:
    {
        MString pipeName;

        MString msgPipe;

        if (!optionVars.sessionPipeCustom.get() || overrideInProcess)
        {
            HAPI_ThriftServerOptions serverOptions;
            serverOptions.autoClose = true;
            serverOptions.timeoutMs = optionVars.timeout.get();

            pipeName = Util::getTempDir().c_str();
            Util::mkpath(pipeName.asChar());

            pipeName += Util::pathSeparator;
            pipeName += "hapi";
            pipeName += getpid();

            MGlobal::displayInfo("Automatically starting Houdini Engine server "
                                 "using named pipe.");

            // on Linux, if LD_LIBRARY_PATH is set, HARS might fail to start due
            // to library conflicts so we have an option to unset it before
            // autostarting the server.
#ifndef _WIN32
            char *llpString = getenv("LD_LIBRARY_PATH");
            char *llpSave   = NULL;
            if (optionVars.unsetLLP.get())
            {
                if (llpString && strlen(llpString) > 0)
                {
                    // llpString points into the environment, so if we unsetenv,
                    // it may well be deleted, so save a copy just in case
                    llpSave = new char[strlen((llpString)) + 1];
                    strcpy(llpSave, llpString);
                    unsetenv("LD_LIBRARY_PATH");
                }
            }
#endif

            // When starting HARS, it's possible that the Maya python path will
            // conflict with Houdini's. Clear the PYTHONPATH variable if
            // instructed.
            char *ppString = getenv("PYTHONPATH");
            char *ppSave   = NULL;

            if (optionVars.unsetPP.get())
            {
                if (ppString && strlen(ppString) > 0)
                {
                    ppSave = new char[strlen((ppString)) + 1];
                    strcpy(ppSave, ppString);

#ifdef _WIN32
                    _putenv("PYTHONPATH=");
#else
                    unsetenv("PYTHONPATH");
#endif
                }
            }

            HAPI_ProcessId processId;
            sessionResult = HoudiniApi::StartThriftNamedPipeServer(
                &serverOptions, pipeName.asChar(), &processId, nullptr);

#ifndef _WIN32
            if (llpSave)
            {
                setenv("LD_LIBRARY_PATH", llpSave, 1);
                delete[] llpSave;
                llpSave = NULL;
            }
#endif

            if (ppSave)
            {
#ifdef _WIN32
                char prefix[] = "PYTHONPATH=";
                char *buffer = new char[strlen(ppString) + strlen(prefix) + 1];
                strcpy(buffer, prefix);
                strcat(buffer, ppString);
                _putenv(buffer);
#else
                setenv("PYTHONPATH", ppSave, 1);
#endif
                delete[] ppSave;
                ppSave = NULL;
            }

            if (HAPI_FAIL(sessionResult))
            {
                int errorLength = 0;

                HoudiniApi::GetConnectionErrorLength(&errorLength);

                if (errorLength > 0)
                {
                    char *msg = new char[errorLength];
                    HoudiniApi::GetConnectionError(msg, errorLength, true);
                    MGlobal::displayError(msg);
                    delete[] msg;
                }
                else
                {
                    MGlobal::displayError(
                        "Failed to automatically start Houdini Engine "
                        "server using named pipe.");
                }

                return HAPI_RESULT_FAILURE;
            }

            msgPipe = pipeName;

            MGlobal::displayInfo("Automatically started Houdini Engine server "
                                 "using named pipe at \"" +
                                 msgPipe + "\".");
        }
        else
        {
            pipeName = optionVars.thriftPipe.get();

            msgPipe = pipeName;
        }

        sessionResult = HoudiniApi::CreateThriftNamedPipeSession(
            Util::theHAPISession.get(), pipeName.asChar());

        if (!HAPI_FAIL(sessionResult))
        {
            MGlobal::displayInfo(
                "Connected to Houdini Engine server using named pipe "
                "at \"" +
                msgPipe + "\".");
        }
        else
        {
            int errorLength = 0;

            HoudiniApi::GetConnectionErrorLength(&errorLength);

            if (errorLength > 0)
            {
                char *msg = new char[errorLength];
                HoudiniApi::GetConnectionError(msg, errorLength, true);
                MGlobal::displayError(msg);
                delete[] msg;
            }
            else
            {
                MGlobal::displayInfo(
                    "Failed to connected to Houdini Engine server using "
                    "named pipe at \"" +
                    msgPipe + "\".");
            }
        }
    }
    break;
    }

    if (sessionResult != HAPI_RESULT_SUCCESS)
    {
        Util::theHAPISession.reset(NULL);

        MGlobal::displayError(
            "Could not create a Houdini Engine session. "
            "Please edit the Back End preferences and reload the plug-in.");
    }

    return sessionResult;
}

HAPI_Result
initializeHAPI(const OptionVars &optionVars)
{
    if (HoudiniApi::IsInitialized(Util::theHAPISession.get()) == HAPI_RESULT_SUCCESS)
    {
        MGlobal::displayInfo(
            "Houdini Engine is already initialized. Skipping initialization.");

        return HAPI_RESULT_SUCCESS;
    }

    const char *otl_dir = getenv("HAPI_OTL_PATH");
    const char *dso_dir = getenv("HAPI_DSO_PATH");

    HAPI_CookOptions cook_options        = HoudiniApi::CookOptions_Create();
    cook_options.maxVerticesPerPrimitive = -1;
    cook_options.refineCurveToLinear     = false;

    bool use_cooking_thread = optionVars.asyncMode.get() == 1;

    HAPI_Result hstat = HoudiniApi::Initialize(Util::theHAPISession.get(),
                                        &cook_options, use_cooking_thread, -1,
                                        NULL, otl_dir, dso_dir, NULL, NULL);
    if (HAPI_FAIL(hstat))
        return hstat;

    // Set the client name.
    HoudiniApi::SetServerEnvString(
        Util::theHAPISession.get(), HAPI_ENV_CLIENT_NAME, "maya");

    MGlobal::displayInfo("Houdini Engine initialized successfully.");

    return HAPI_RESULT_SUCCESS;
}

bool
cleanupHAPI()
{
    if (!Util::theHAPISession.get())
    {
        return true;
    }

    // If HAPI is not initialized, then don't try to do cleanup. This could be
    // because HAPI failed to initialize, or HARS disconnected.
    CHECK_HAPI_AND_RETURN(HoudiniApi::IsInitialized(Util::theHAPISession.get()), true);

    CHECK_HAPI_AND_RETURN(HoudiniApi::Cleanup(Util::theHAPISession.get()), false);

    return true;
}

bool
cleanupSession()
{
    if (!Util::theHAPISession.get())
    {
        return true;
    }

    // If session is not initialize, then don't try to close it.
    CHECK_HAPI_AND_RETURN(
        HoudiniApi::IsSessionValid(Util::theHAPISession.get()), true);

    // null out the session when closed, it will be reset if reinitialized
    // anyway
    Util::theHAPISession.reset(NULL);

    return true;
}

void
mayaExiting(void *clientData)
{
    cleanupHAPI();
    cleanupSession();
}

void
updateTimelineCallback(void *clientData)
{
    HAPI_TimelineOptions timelineOptions;

    MTime oneUnitTime;

    // Houdini's "frame 1" is "0 seconds", but Maya's "frame 0" is "0 seconds".
    // So we need to offset the time by 1.
    timelineOptions.fps = float(1.0 / oneUnitTime.as(MTime::kSeconds));
    timelineOptions.startTime =
        (MAnimControl::animationStartTime() - oneUnitTime).as(MTime::kSeconds);
    timelineOptions.endTime =
        (MAnimControl::animationEndTime() - oneUnitTime).as(MTime::kSeconds);

    HoudiniApi::SetTimelineOptions(Util::theHAPISession.get(), &timelineOptions);
}

MCallbackIdArray messageCallbacks;

void
initializeMessageCallbacks()
{
    MStatus status;

    MCallbackId callbackId;

    callbackId = MSceneMessage::addCallback(
        MSceneMessage::kMayaExiting, mayaExiting, NULL, &status);
    if (status)
    {
        messageCallbacks.append(callbackId);
    }
    else
    {
        CHECK_MSTATUS(status);
    }

    callbackId = MEventMessage::addEventCallback(
        "playbackRangeSliderChanged", updateTimelineCallback, NULL, &status);
    if (status)
    {
        messageCallbacks.append(callbackId);
    }
    else
    {
        CHECK_MSTATUS(status);
    }

    callbackId = MEventMessage::addEventCallback(
        "timeUnitChanged", updateTimelineCallback, NULL, &status);
    if (status)
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

bool hapilLocationIsValid(MString &hapilLocation)
{
    bool valid = false;

    // check if the library exists
    FILE *fdHapil = fopen(hapilLocation.asChar(), "r");

    if (fdHapil)
    {
        valid = true;
        fclose(fdHapil);
    }

    return valid;
}

PLUGIN_EXPORT
MStatus
initializePlugin(MObject obj)
{
    OptionVars optionVars;

    char engine_version[32];
    sprintf(engine_version, "%d.%d (API: %d)",
            HAPI_VERSION_HOUDINI_ENGINE_MAJOR,
            HAPI_VERSION_HOUDINI_ENGINE_MINOR, HAPI_VERSION_HOUDINI_ENGINE_API);

    MStatus status;
    MFnPlugin plugin(obj, "Side Effects", engine_version, "Any");

    initializeMessageCallbacks();

    // update the timeline option for the first time
    updateTimelineCallback(NULL);

    status = plugin.registerCommand(EngineCommand::commandName,
                                    EngineCommand::creator,
                                    EngineCommand::newSyntax);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = plugin.registerCommand(AssetCommand::commandName,
                                    AssetCommand::creator,
                                    AssetCommand::newSyntax);
    CHECK_MSTATUS_AND_RETURN_IT(status); 

    status = plugin.registerUI(
        "houdiniEngineCreateUI", "houdiniEngineDeleteUI");
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MString hapilLocation = optionVars.hapilLocation.get();

    bool hapilValid = false;

    if (hapilLocation.length() > 0)
    {
        hapilValid = hapilLocationIsValid(hapilLocation);
    }
    else
    {
        MGlobal::executeCommand("source \"houdiniEngineUtils.mel\";");
        MString hfsPath = MGlobal::executeCommandStringResult("houdiniEngine_getHfsPath(false)");
        MString hapilPath = "";

        if (hfsPath.length() > 0)
        {
            optionVars.hfsLocation.set(hfsPath);

            hapilPath = MGlobal::executeCommandStringResult("houdiniEngine_getHapilPath(false, houdiniEngine_getHfsPath(false))");

            if (hapilPath.length() > 0)
            {
                hapilValid = hapilLocationIsValid(hapilPath);

                if (hapilValid)
                {
                    optionVars.hapilLocation.set(hapilPath);
                    hapilLocation = hapilPath;
                }
            }
        }
    }

    if (hapilValid)
    {
        void *hapilHandle = obtainHAPILHandle(hapilLocation.asChar());
        
        if (!hapilHandle)
        {
            MGlobal::displayInfo("Could not find libHAPIL.");
            return MStatus::kSuccess;
        }

        HoudiniApi::InitializeHAPI(hapilHandle);

        Util::isHapilLoaded = true;
    }
    else
    {
        MGlobal::executeCommand(
            R"(confirmDialog -title "Houdini Engine"
                -button "OK"
                -icon "critical"
                -message ("The Houdini Engine for Maya plugin could not be initialized.\n\n" +
                          "The path to libHAPIL is invalid. Please double-check the value " +
                          "in Houdini Engine>Preferences...");)");

        MGlobal::executeCommand("houdiniEnginePreferences();", false);

        // return success here so that the user can modify the preferences
        return MStatus::kSuccess;
    }

    std::string harsPath = "";
    bool harsFound = Util::getHarsPath(harsPath);

    if (!harsFound)
    {
        MGlobal::executeCommand(
            R"(confirmDialog -title "Houdini Engine"
                -button "OK"
                -icon "critical"
                -message ("The Houdini Engine for Maya plugin could not be initialized.\n\n" +
                        "The Houdini Engine Server (HARS) could not be found. The path " +
                        "to HARS must either be added to your PATH environment variable " +
                        "or specified in the HOUDINI_HARS_LOCATION environment variable.");)");

        return MStatus::kSuccess;
    }

    if (!Util::checkBuildEngineCompatibility())
    {
        MGlobal::executeCommand(
            R"(confirmDialog -title "Houdini Engine"
                -button "OK"
                -icon "critical"
                -message ("The Houdini Engine for Maya plugin could not be initialized.\n\n" +
                        "The version of Houdini that this plug-in was built for does " +
                        "not match the version of Houdini that the plug-in is instructed " +
                        "to use. In order to continue, you must edit the HFS and " +
                        "libHAPIL paths to point to a compatible version.");)");

        MGlobal::executeCommand("houdiniEnginePreferences();", false);

        return MStatus::kSuccess;
    }

    initializeSession(optionVars);

    if (HAPI_FAIL(initializeHAPI(optionVars)))
        return MStatus::kSuccess;

    status = plugin.registerTransform(
        AssetNode::typeName, AssetNode::typeId, AssetNode::creator,
        AssetNode::initialize, MPxTransformationMatrix::creator,
        MPxTransformationMatrix::baseTransformationMatrixId);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = plugin.registerNode(
        InputGeometryNode::typeName, InputGeometryNode::typeId,
        InputGeometryNode::creator, InputGeometryNode::initialize);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = plugin.registerNode(
        InputCurveNode::typeName, InputCurveNode::typeId,
        InputCurveNode::creator, InputCurveNode::initialize);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = plugin.registerNode(
        InputTransformNode::typeName, InputTransformNode::typeId,
        InputTransformNode::creator, InputTransformNode::initialize);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = plugin.registerNode(
        InputMergeNode::typeName, InputMergeNode::typeId,
        InputMergeNode::creator, InputMergeNode::initialize);
    CHECK_MSTATUS_AND_RETURN_IT(status);

#if MAYA_API_VERSION >= 201400
    status = plugin.registerNode(
        FluidGridConvert::typeName, FluidGridConvert::typeId,
        FluidGridConvert::creator, FluidGridConvert::initialize);
#endif

    printHAPIVersion();

    return status;
}

PLUGIN_EXPORT
MStatus
uninitializePlugin(MObject obj)
{
    MStatus status = MStatus::kSuccess;
    MFnPlugin plugin(obj);

    cleanupMessageCallbacks();

    if (plugin.isNodeRegistered(AssetNode::typeName))
    {
        status = plugin.deregisterNode(AssetNode::typeId);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    if (plugin.isNodeRegistered(InputGeometryNode::typeName))
    {
        status = plugin.deregisterNode(InputGeometryNode::typeId);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    if (plugin.isNodeRegistered(InputCurveNode::typeName))
    {
        status = plugin.deregisterNode(InputCurveNode::typeId);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    if (plugin.isNodeRegistered(InputTransformNode::typeName))
    {
        status = plugin.deregisterNode(InputTransformNode::typeId);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

    if (plugin.isNodeRegistered(InputMergeNode::typeName))
    {
        status = plugin.deregisterNode(InputMergeNode::typeId);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }

#if MAYA_API_VERSION >= 201400
    if (plugin.isNodeRegistered(FluidGridConvert::typeName))
    {
        status = plugin.deregisterNode(FluidGridConvert::typeId);
        CHECK_MSTATUS_AND_RETURN_IT(status);
    }
#endif

    status = plugin.deregisterCommand(EngineCommand::commandName);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    status = plugin.deregisterCommand(AssetCommand::commandName);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    if (Util::theHAPISession.get())
    {
        if (cleanupHAPI() && cleanupSession())
        {
            MGlobal::displayInfo("Houdini Engine cleaned up successfully.");
        }
        else
        {
            MGlobal::displayInfo("Houdini Engine failed to clean up.");
            return MStatus::kFailure;
        }
    }
    else
        MGlobal::displayInfo("Houdini Engine cleaned up successfully.");

    return status;
}

