#ifndef ASSET_COMMAND_H
#define ASSET_COMMAND_H

#include <maya/MPxCommand.h>

class AssetCommand: public MPxCommand
{

    public:
        AssetCommand();
        virtual ~AssetCommand();

        MStatus doIt(const MArgList& args);
        static void* creator();
};

#endif
