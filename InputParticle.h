#ifndef __InputParticle_h__
#define __InputParticle_h__

#include "Input.h"

#include <HAPI/HAPI.h>

class InputParticle : public Input
{
public:
    InputParticle();
    virtual ~InputParticle();

    virtual AssetInputType assetInputType() const;

    virtual void setInputGeo(MDataBlock &dataBlock, const MPlug &plug);

protected:
    void setAttributePointData(const char *attributeName,
                               HAPI_StorageType storage,
                               int count,
                               int tupleSize,
                               void *data);
};

#endif
