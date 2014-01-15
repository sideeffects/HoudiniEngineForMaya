#ifndef __AssetInputMesh_h__
#define __AssetInputMesh_h__

#include "AssetInput.h"

#include <HAPI/HAPI.h>

class AssetInputMesh : public AssetInput
{
    public:
	AssetInputMesh(int assetId, int inputIdx);
	virtual ~AssetInputMesh();

	virtual AssetInputType assetInputType() const;

	virtual void setInputTransform(MDataHandle &dataHandle);
        virtual void setInputGeo(
                MDataBlock &dataBlock,
                const MPlug &plug
                );

    protected:
	int myInputAssetId;
	HAPI_GeoInputInfo myInputInfo;
};

#endif
