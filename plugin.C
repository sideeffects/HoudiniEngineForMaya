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
#include <unistd.h>

void
printHAPIVersion()
{
    int i, j, k;
    char version[32];
    MString msg;

    {
        HAPI_GetEnvInt(
                HAPI_ENVINT_VERSION_HOUDINI_MAJOR,
                &i
                );
        HAPI_GetEnvInt(
                HAPI_ENVINT_VERSION_HOUDINI_MINOR,
                &j
                );
        HAPI_GetEnvInt(
                HAPI_ENVINT_VERSION_HOUDINI_BUILD,
                &k
                );

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
        HAPI_GetEnvInt(
                HAPI_ENVINT_VERSION_HOUDINI_ENGINE_MAJOR,
                &i
                );
        HAPI_GetEnvInt(
                HAPI_ENVINT_VERSION_HOUDINI_ENGINE_MINOR,
                &j
                );
        HAPI_GetEnvInt(
                HAPI_ENVINT_VERSION_HOUDINI_ENGINE_API,
                &k
                );

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

namespace
{
    template <typename VAL, class CHILD>
    class OptionVarBase
    {
    public:
        OptionVarBase(const char* name, const VAL& defaultValue)
            : myDefaultValue(defaultValue)
        {
            myName = "houdiniEngine";
            myName += name;

            bool exists = false;
            static_cast<const CHILD&>(*this).getImpl(exists);

            if (!exists)
            {
                MGlobal::setOptionVarValue(myName.c_str(), myDefaultValue);
            }
        }

        VAL get() const
        {
            bool exists = false;
            const VAL value = static_cast<const CHILD&>(*this).getImpl(exists);
            return exists ? value : myDefaultValue;
        }

    protected:
        std::string myName;
        const VAL myDefaultValue;

    private:
        OptionVarBase& operator=(const OptionVarBase&);
    };

    class IntOptionVar: public OptionVarBase<int, IntOptionVar>
    {
    public:
        typedef OptionVarBase<int, IntOptionVar> Base;

        IntOptionVar(const char* name, int defaultValue)
            : Base(name, defaultValue)
        {}

        int getImpl(bool& exists) const
        {
            return MGlobal::optionVarIntValue(myName.c_str(), &exists);
        }
    };

    class StringOptionVar: public OptionVarBase<MString, StringOptionVar>
    {
    public:
        typedef OptionVarBase<MString, StringOptionVar> Base;

        StringOptionVar(const char* name, const char* defaultValue)
            : Base(name, defaultValue)
        {}

        MString getImpl(bool& exists) const
        {
            return MGlobal::optionVarStringValue(myName.c_str(), &exists);
        }
    };

    struct OptionVars
    {
        OptionVars()
            : asyncMode("AsynchronousMode", 1)
            , sessionType("SessionType", 0)
            , thriftServer("ThriftServer", "localhost")
            , thriftPort("ThriftPort", 9090)
            , sessionPipeCustom("SessionPipeCustom", 0)
            , thriftPipe("ThriftPipe", "hapi")
        {}

        IntOptionVar     asyncMode;
        IntOptionVar     sessionType;
        StringOptionVar  thriftServer;
        IntOptionVar     thriftPort;
        IntOptionVar     sessionPipeCustom;
        StringOptionVar  thriftPipe;

    private:
        OptionVars& operator=(const OptionVars&);
    };
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
initializeSession(const OptionVars& optionVars)
{
    if ( Util::theHAPISession.get() )
    {
        MGlobal::displayInfo(
            "Houdini Engine session is already created. Skipping.");

        return HAPI_RESULT_SUCCESS;
    }

    const SessionType::Enum sessionType =
        static_cast<SessionType::Enum>( optionVars.sessionType.get() );

    Util::theHAPISession.reset( new HAPI_Session );
    HAPI_Result result = HAPI_RESULT_FAILURE;

    switch (sessionType)
    {
    case SessionType::ST_INPROCESS:
        MGlobal::displayInfo(
            "Creating an in-process Houdini Engine session.");

        result = HAPI_CreateInProcessSession( Util::theHAPISession.get() );
        break;

    case SessionType::ST_THRIFT_SOCKET:
        {
            MString hostName = "localhost";
            int port = -1;

            MString msgHostPort;

            hostName = optionVars.thriftServer.get();
            port = optionVars.thriftPort.get();

            msgHostPort = hostName + ":" + port;

            result = HAPI_CreateThriftSocketSession(
                Util::theHAPISession.get(),
                hostName.asChar(), port );

            if( !HAPI_FAIL(result) )
            {
                MGlobal::displayInfo(
                        "Connected to Houdini Engine server using TCP socket "
                        "at " + msgHostPort + "."
                        );
            }
            else
            {
                MGlobal::displayInfo(
                        "Failed to connected to Houdini Engine server using "
                        "TCP socket at " + msgHostPort + "."
                        );
            }
        }
        break;

    case SessionType::ST_THRIFT_PIPE:
        {
            MString pipeName;

            MString msgPipe;

            if( !optionVars.sessionPipeCustom.get() )
            {
                pipeName = "hapi";
                pipeName += getpid();

                MGlobal::displayInfo(
                        "Automatically starting Houdini Engine server "
                        "using named pipe."
                        );

                HAPI_ProcessId processId;
                result = HAPI_StartThriftNamedPipeServer(
                        true,
                        pipeName.asChar(),
                        10 * 1000,
                        &processId);
                if( HAPI_FAIL(result) )
                {
                    MGlobal::displayError(
                            "Failed to automatically start Houdini Engine "
                            "server using named pipe."
                            );
                    return HAPI_RESULT_FAILURE;
                }

                msgPipe = pipeName;

                MGlobal::displayInfo(
                        "Automatically started Houdini Engine server "
                        "using named pipe at \"" + msgPipe + "\"."
                        );
            }
            else
            {
                pipeName = optionVars.thriftPipe.get();

                msgPipe = pipeName;
            }

            result = HAPI_CreateThriftNamedPipeSession(
                Util::theHAPISession.get(),
                pipeName.asChar() );

            if( !HAPI_FAIL(result) )
            {
                MGlobal::displayInfo(
                        "Connected to Houdini Engine server using named pipe "
                        "at \"" + msgPipe + "\"."
                        );
            }
            else
            {
                MGlobal::displayInfo(
                        "Failed to connected to Houdini Engine server using "
                        "named pipe at \"" + msgPipe + "\"."
                        );
            }
        }
        break;
    }

    if ( result != HAPI_RESULT_SUCCESS )
    {
        Util::theHAPISession.reset(NULL);
        MGlobal::displayError(
            "Could not create a Houdini Engine session. "
            "Please edit the Back End preferences and reload the plug-in.");
    }

    return result;
}

bool
initializeHAPI(const OptionVars& optionVars)
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

    bool use_cooking_thread = optionVars.asyncMode.get() == 1;

    hstat = HAPI_Initialize(
            Util::theHAPISession.get(),
            &cook_options,
            use_cooking_thread,
            -1,
            otl_dir, dso_dir,
            NULL, NULL
            );
    if(HAPI_FAIL(hstat))
    {
        MGlobal::displayInfo("Houdini Engine failed to initialize.");

        return false;
    }

    // Set the client name.
    HAPI_SetServerEnvString(
        Util::theHAPISession.get(),
        HAPI_ENV_CLIENT_NAME,
        "maya"
    );

    MGlobal::displayInfo("Houdini Engine initialized successfully.");

    return true;
}

bool
cleanupHAPI()
{
    if ( !Util::theHAPISession.get() )
    {
        return true;
    }

    // If HAPI is not initialized, then don't try to do cleanup. This could be
    // because HAPI failed to initialize, or HARS disconnected.
    if( HAPI_FAIL( HAPI_IsInitialized( Util::theHAPISession.get() ) ) )
    {
        return true;
    }

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

    HAPI_SetTimelineOptions(
            Util::theHAPISession.get(),
            &timelineOptions
            );
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
            NULL,
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
            NULL,
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

PLUGIN_EXPORT
MStatus
initializePlugin(MObject obj)
{
    OptionVars optionVars;

    initializeSession(optionVars);

    if(!initializeHAPI(optionVars))
    {
        printHAPIVersion();
    }

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
    updateTimelineCallback(NULL);

    return status;
}

PLUGIN_EXPORT
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
