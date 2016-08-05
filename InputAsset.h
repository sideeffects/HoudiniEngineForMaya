#ifndef __InputAsset_h__
#define __InputAsset_h__

#include "Input.h"

class InputAsset : public Input
{
    public:
        InputAsset(HAPI_NodeId nodeId, int inputIdx);
        virtual ~InputAsset();

        virtual AssetInputType assetInputType() const;

        virtual void setInputTransform(MDataHandle &dataHandle);
        virtual void setInputGeo(
                MDataBlock &dataBlock,
                const MPlug &plug
                );
};

#endif
