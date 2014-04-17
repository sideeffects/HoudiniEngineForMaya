#ifndef __InputMesh_h__
#define __InputMesh_h__

#include "Input.h"

#include <HAPI/HAPI.h>

class InputMesh : public Input
{
    public:
        InputMesh(int assetId, int inputIdx);
        virtual ~InputMesh();

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
