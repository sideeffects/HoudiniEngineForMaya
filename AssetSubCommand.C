#include "AssetSubCommand.h"

AssetSubCommand::AssetSubCommand()
{
}

AssetSubCommand::~AssetSubCommand()
{
}

MStatus
AssetSubCommand::doIt()
{
    return MStatus::kSuccess;
}

MStatus
AssetSubCommand::redoIt()
{
    return MStatus::kSuccess;
}

MStatus
AssetSubCommand::undoIt()
{
    return MStatus::kSuccess;
}

bool
AssetSubCommand::isUndoable() const
{
    return false;
}
