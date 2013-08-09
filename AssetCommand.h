#ifndef __AssetCommand_h__
#define __AssetCommand_h__

#include <maya/MPxCommand.h>

class AssetCommand: public MPxCommand
{
    enum AssetOperationType
    {
	kOperationLoad = 0,
	kOperationSaveHip,
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
	MString myHIPFilePath;
	AssetOperationType myOperationType;
};

#endif
