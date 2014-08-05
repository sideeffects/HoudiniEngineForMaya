#include "EngineCommand.h"

#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>

#include <HAPI/HAPI.h>

#include "SubCommand.h"

#define kLicenseFlag "-lic"
#define kLicenseFlagLong "-license"
#define kHoudiniVersionFlag "-hv"
#define kHoudiniVersionFlagLong "-houdiniVersion"
#define kHoudiniEngineVersionFlag "-hev"
#define kHoudiniEngineVersionFlagLong "-houdiniEngineVersion"
#define kSaveHIPFlag "-sh"
#define kSaveHIPFlagLong "-saveHIP"

const char* EngineCommand::commandName = "houdiniEngine";

class EngineSubCommandLicense : public SubCommand
{
    public:
        virtual MStatus doIt()
        {
            int license;

            HAPI_GetEnvInt(HAPI_ENVINT_LICENSE, &license);

            MString version_string;
            switch(license)
            {
                case HAPI_LICENSE_NONE:
                    version_string = "none";
                    break;
                case HAPI_LICENSE_HOUDINI_ENGINE:
                    version_string = "Houdini-Engine";
                    break;
                case HAPI_LICENSE_HOUDINI:
                    version_string = "Houdini-Escape";
                    break;
                case HAPI_LICENSE_HOUDINI_FX:
                    version_string = "Houdini-Master";
                    break;
                case HAPI_LICENSE_HOUDINI_ENGINE_INDIE:
                    version_string = "Houdini-Engine-Indie";
                    break;
                case HAPI_LICENSE_HOUDINI_INDIE:
                    version_string = "Houdini-Indie";
                    break;
                default:
                    version_string = "Unknown";
                    break;
            }

            MPxCommand::setResult(version_string);

            return MStatus::kSuccess;
        }
};

class EngineSubCommandSaveHIPFile : public SubCommand
{
    public:
        EngineSubCommandSaveHIPFile(const MString &hipFilePath) :
            myHIPFilePath(hipFilePath)
        {
        }

        virtual MStatus doIt()
        {
            HAPI_SaveHIPFile(myHIPFilePath.asChar());

            return MStatus::kSuccess;
        }

    protected:
        MString myHIPFilePath;
};

class EngineSubCommandHoudiniVersion : public SubCommand
{
    public:
        virtual MStatus doIt()
        {
            int major, minor, build;

            HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_MAJOR, &major);
            HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_MINOR, &minor);
            HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_BUILD, &build);

            MString version_string;
            version_string.format(
                    "^1s.^2s.^3s",
                    MString() + major,
                    MString() + minor,
                    MString() + build
                    );

            MPxCommand::setResult(version_string);

            return MStatus::kSuccess;
        }
};

class EngineSubCommandHoudiniEngineVersion : public SubCommand
{
    public:
        virtual MStatus doIt()
        {
            int major, minor, api;

            HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_ENGINE_MAJOR, &major);
            HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_ENGINE_MINOR, &minor);
            HAPI_GetEnvInt(HAPI_ENVINT_VERSION_HOUDINI_ENGINE_API, &api);

            MString version_string;
            version_string.format(
                    "^1s.^2s (API: ^3s)",
                    MString() + major,
                    MString() + minor,
                    MString() + api
                    );

            MPxCommand::setResult(version_string);

            return MStatus::kSuccess;
        }
};

void* EngineCommand::creator()
{
    return new EngineCommand();
}

MSyntax
EngineCommand::newSyntax()
{
    MSyntax syntax;

    // -license returns the Houdini version that's being used.
    CHECK_MSTATUS(syntax.addFlag(kLicenseFlag, kLicenseFlagLong));

    // -houdiniVersion returns the Houdini version that's being used.
    CHECK_MSTATUS(syntax.addFlag(kHoudiniVersionFlag, kHoudiniVersionFlagLong));

    // -houdiniVersion returns the Houdini version that's being used.
    CHECK_MSTATUS(syntax.addFlag(kHoudiniEngineVersionFlag, kHoudiniEngineVersionFlagLong));

    // -saveHIP saves the contents of the current Houdini scene as a hip file
    // expected arguments: hip_file_name - the name of the hip file to save
    CHECK_MSTATUS(syntax.addFlag(kSaveHIPFlag, kSaveHIPFlagLong, MSyntax::kString));

    return syntax;
}

EngineCommand::EngineCommand() :
    mySubCommand(NULL)
{
}

EngineCommand::~EngineCommand()
{
    delete mySubCommand;
}

MStatus
EngineCommand::parseArgs(const MArgList &args)
{
    MStatus status;
    MArgDatabase argData(syntax(), args, &status);
    if(!status)
    {
        return status;
    }

    if(!(
                argData.isFlagSet(kLicenseFlag)
                ^ argData.isFlagSet(kHoudiniVersionFlag)
                ^ argData.isFlagSet(kHoudiniEngineVersionFlag)
                ^ argData.isFlagSet(kSaveHIPFlag)
        ))
    {
        displayError("Exactly one of these flags must be specified:\n"
                kSaveHIPFlagLong "\n"
                );
        return MStatus::kInvalidParameter;
    }

    if(argData.isFlagSet(kLicenseFlag))
    {
        mySubCommand = new EngineSubCommandLicense();
    }

    if(argData.isFlagSet(kHoudiniVersionFlag))
    {
        mySubCommand = new EngineSubCommandHoudiniVersion();
    }

    if(argData.isFlagSet(kHoudiniEngineVersionFlag))
    {
        mySubCommand = new EngineSubCommandHoudiniEngineVersion();
    }

    if(argData.isFlagSet(kSaveHIPFlag))
    {
        MString hipFilePath;
        {
            status = argData.getFlagArgument(kSaveHIPFlag, 0, hipFilePath);
            if(!status)
            {
                displayError("Invalid argument for \"" kSaveHIPFlagLong "\".");
                return status;
            }
        }

        mySubCommand = new EngineSubCommandSaveHIPFile(hipFilePath);
    }

    return MStatus::kSuccess;
}

MStatus EngineCommand::doIt(const MArgList& args)
{
    MStatus status;

    status = parseArgs(args);
    if(!status)
    {
        return status;
    }

    return mySubCommand->doIt();
}

MStatus EngineCommand::redoIt()
{
    return mySubCommand->redoIt();
}

MStatus EngineCommand::undoIt()
{
    return mySubCommand->undoIt();
}

bool EngineCommand::isUndoable() const
{
    return mySubCommand->isUndoable();
}
