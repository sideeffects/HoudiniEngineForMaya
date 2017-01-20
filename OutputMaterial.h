#ifndef __OutputMaterial_h__
#define __OutputMaterial_h__

#include <HAPI/HAPI_Common.h>

#include <string>

class MTime;
class MDataHandle;
class MStatus;

class OutputMaterial
{
public:
    OutputMaterial();

    MStatus compute(
            const MTime &time,
            MDataHandle &materialHandle
            );
private:
    HAPI_NodeId myNodeId;
    HAPI_NodeInfo myNodeInfo;
    int myMaterialLastCookCount;
};

#endif
