#include "EngineCommand.h"

#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>

#include <HAPI/HAPI.h>

#include "SubCommand.h"

#define kSaveHIPFlag "-sh"
#define kSaveHIPFlagLong "-saveHIP"

const char* EngineCommand::commandName = "houdiniEngine";

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

void* EngineCommand::creator()
{
    return new EngineCommand();
}

MSyntax
EngineCommand::newSyntax()
{
    MSyntax syntax;

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
                argData.isFlagSet(kSaveHIPFlag)
        ))
    {
        displayError("Exactly one of these flags must be specified:\n"
                kSaveHIPFlagLong "\n"
                );
        return MStatus::kInvalidParameter;
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
