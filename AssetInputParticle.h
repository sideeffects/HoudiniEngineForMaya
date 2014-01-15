#ifndef __AssetInputParticle_h__
#define __AssetInputParticle_h__

#include "AssetInput.h"

#include <HAPI/HAPI.h>

class AssetInputParticle : public AssetInput
{
    public:
	AssetInputParticle(int assetId, int inputIdx);
	virtual ~AssetInputParticle();

	virtual AssetInputType assetInputType() const;

	virtual void setInputTransform(MDataHandle &dataHandle);
        virtual void setInputGeo(
                MDataBlock &dataBlock,
                const MPlug &plug
                );

    protected:
        void setAttributePointData(
                const char* attributeName,
                HAPI_StorageType storage,
                int count,
                int tupleSize,
                void* data
                );

    protected:
	int myInputAssetId;
	HAPI_GeoInputInfo myInputInfo;
};

#endif
