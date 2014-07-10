#include "EngineCommand.h"

#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>

#include "SubCommand.h"

const char* EngineCommand::commandName = "houdiniEngine";

void* EngineCommand::creator()
{
    return new EngineCommand();
}

MSyntax
EngineCommand::newSyntax()
{
    MSyntax syntax;

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

    if(!(false))
    {
        displayError("Exactly one of these flags must be specified:\n"
                );
        return MStatus::kInvalidParameter;
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
