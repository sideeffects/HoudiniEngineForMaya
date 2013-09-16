#ifndef __AssetCommand_h__
#define __AssetCommand_h__

#include <maya/MPxCommand.h>

#include <maya/MSyntax.h>

class AssetSubCommand;

class AssetCommand: public MPxCommand
{
    enum AssetOperationType
    {
	kOperationSubCommand,
	kOperationSaveHip,
	kOperationResetSimulation,
	kOperationInvalid
    };

    public:
	static void* creator();
	static MSyntax newSyntax();

    public:
        AssetCommand();
        virtual ~AssetCommand();

        MStatus doIt(const MArgList& args);
	MStatus redoIt();
	MStatus undoIt();
        bool isUndoable() const;

    private:
	MStatus parseArgs(const MArgList &args);

    private:
	MString myHIPFilePath;
	int	myResetSimulationAssetId;
	AssetOperationType myOperationType;

	AssetSubCommand* myAssetSubCommand;
};

#endif
