#ifndef ASSET_COMMAND_H
#define ASSET_COMMAND_H

#include <maya/MPxCommand.h>

class AssetCommand: public MPxCommand
{
    enum AssetOperationType
    {
	kOperationLoad = 0,
	kOperationInvalid
    };

    public:
        AssetCommand();
        virtual ~AssetCommand();

        MStatus doIt(const MArgList& args);
	MStatus redoIt();
	MStatus undoIt();
        bool isUndoable() const;

        static void* creator();

    private:
	MString myAssetOtlPath;
	AssetOperationType myOperationType;
};

#endif
