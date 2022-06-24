#ifndef __InputCurve_h__
#define __InputCurve_h__

#include "Input.h"

#include <HAPI/HAPI_Common.h>

class InputCurve : public Input
{
public:
    InputCurve();
    virtual ~InputCurve();

    virtual AssetInputType assetInputType() const;

    virtual void setInputGeo(MDataBlock &dataBlock, const MPlug &plug);

protected:
    HAPI_NodeInfo myCurveNodeInfo;
};

#endif

