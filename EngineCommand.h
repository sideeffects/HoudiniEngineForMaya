#ifndef __EngineCommand_h__
#define __EngineCommand_h__

#include <maya/MPxCommand.h>

#include <maya/MSyntax.h>

class SubCommand;

class EngineCommand : public MPxCommand
{
public:
    static const char *commandName;

public:
    static void *creator();
    static MSyntax newSyntax();

public:
    EngineCommand();
    virtual ~EngineCommand();

    MStatus doIt(const MArgList &args);
    MStatus redoIt();
    MStatus undoIt();
    bool isUndoable() const;

private:
    MStatus parseArgs(const MArgList &args);

private:
    SubCommand *mySubCommand;
};

#endif

