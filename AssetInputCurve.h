#ifndef __AssetInputCurve_h__
#define __AssetInputCurve_h__

#include "AssetInput.h"

#include <HAPI/HAPI_Common.h>

class AssetInputCurve : public AssetInput
{
    public:
	AssetInputCurve(int assetId, int inputIdx);
	virtual ~AssetInputCurve();

	virtual AssetInputType assetInputType() const;

	virtual void setInput(MDataHandle &dataHandle);

    protected:
	HAPI_AssetInfo myCurveAssetInfo;
	HAPI_NodeInfo myCurveNodeInfo;
};

#endif
