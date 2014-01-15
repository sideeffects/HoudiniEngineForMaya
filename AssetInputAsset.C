#include "AssetInputAsset.h"

#include <maya/MFnIntArrayData.h>
#include <maya/MDataBlock.h>
#include <maya/MIntArray.h>

#include <HAPI/HAPI.h>

AssetInputAsset::AssetInputAsset(int assetId, int inputIdx) :
    AssetInput(assetId, inputIdx)
{
}

AssetInputAsset::~AssetInputAsset()
{
}

AssetInputAsset::AssetInputType
AssetInputAsset::assetInputType() const
{
    return AssetInput::AssetInputType_Asset;
}

void
AssetInputAsset::setInputTransform(MDataHandle &dataHandle)
{
}

void
AssetInputAsset::setInputGeo(
        MDataBlock &dataBlock,
        const MPlug &plug
        )
{
    MDataHandle dataHandle = dataBlock.inputValue(plug);
    MFnIntArrayData fnIAD(dataHandle.data());
    MIntArray metaData = fnIAD.array();
    HAPI_ConnectAssetGeometry(metaData[0], metaData[1], myAssetId, myInputIdx);
}
