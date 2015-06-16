#include "InputAsset.h"

#include <maya/MFnIntArrayData.h>
#include <maya/MDataBlock.h>
#include <maya/MIntArray.h>

#include <HAPI/HAPI.h>
#include "util.h"

InputAsset::InputAsset(int assetId, int inputIdx) :
    Input(assetId, inputIdx)
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
    MFnIntArrayData fnIAD(dataHandle.data());
    MIntArray metaData = fnIAD.array();
    HAPI_ConnectAssetGeometry(
            Util::theHAPISession.get(), metaData[0], metaData[1],
            myAssetId, myInputIdx
            );
}
