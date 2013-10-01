#ifndef __AssetInputAsset_h__
#define __AssetInputAsset_h__

#include "AssetInput.h"

class AssetInputAsset : public AssetInput
{
    public:
	AssetInputAsset(int assetId, int inputIdx);
	virtual ~AssetInputAsset();

	virtual AssetInputType assetInputType() const;

	virtual void setInput(MDataHandle &dataHandle, MDataHandle &dataTransformHandle );
};

#endif
