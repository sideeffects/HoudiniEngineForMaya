#include "InputAsset.h"

#include <maya/MFnIntArrayData.h>
#include <maya/MDataBlock.h>
#include <maya/MIntArray.h>

#include <HAPI.h>
#include "util.h"

InputAsset::InputAsset(HAPI_NodeId nodeId, int inputIdx) :
    Input(nodeId, inputIdx)
{
}

InputAsset::~InputAsset()
{
}

InputAsset::AssetInputType
InputAsset::assetInputType() const
{
    return Input::AssetInputType_Asset;
}

void
InputAsset::setInputTransform(MDataHandle &dataHandle)
{
}

void
InputAsset::setInputGeo(
        MDataBlock &dataBlock,
        const MPlug &plug
        )
{
    MDataHandle dataHandle = dataBlock.inputValue(plug);

    HAPI_ConnectNodeInput(
            Util::theHAPISession.get(),
            dataHandle.asInt(),
            myNodeId, myInputIdx
            );
}
