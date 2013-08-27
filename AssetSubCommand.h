#ifndef __AssetSubCommand_h__
#define __AssetSubCommand_h__

#include <maya/MStatus.h>

class AssetSubCommand
{
    public:
	AssetSubCommand();
	virtual ~AssetSubCommand();

	virtual MStatus doIt();
	virtual MStatus redoIt();
	virtual MStatus undoIt();

	virtual bool isUndoable() const;
};

#endif
