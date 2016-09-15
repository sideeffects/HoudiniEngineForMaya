#ifndef __InputCurve_h__
#define __InputCurve_h__

#include "Input.h"

#include <HAPI_Common.h>

class InputCurve : public Input
{
    public:
        InputCurve(int nodeId, int inputIdx);
        virtual ~InputCurve();

        virtual AssetInputType assetInputType() const;

        virtual void setInputTransform(MDataHandle &dataHandle);
        virtual void setInputGeo(
                MDataBlock &dataBlock,
                const MPlug &plug
                );

    protected:
        HAPI_AssetInfo myCurveAssetInfo;
        HAPI_NodeInfo myCurveNodeInfo;
};

#endif
