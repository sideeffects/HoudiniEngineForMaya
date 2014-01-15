#ifndef __AssetInputAsset_h__
#define __AssetInputAsset_h__

#include "AssetInput.h"

class AssetInputAsset : public AssetInput
{
    public:
	AssetInputAsset(int assetId, int inputIdx);
	virtual ~AssetInputAsset();

	virtual AssetInputType assetInputType() const;

	virtual void setInputTransform(MDataHandle &dataHandle);
        virtual void setInputGeo(
                MDataBlock &dataBlock,
                const MPlug &plug
                );
};

#endif
