#ifndef __OutputMaterial_h__
#define __OutputMaterial_h__

#include <HAPI/HAPI_Common.h>

#include <string>

#include <maya/MTime.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MStatus.h>

class OutputMaterial
{
public:
    OutputMaterial(HAPI_NodeId assetId);

    MStatus compute(
            const MTime &time,
            const MPlug &materialPlug,
            MDataBlock &data,
            MDataHandle &materialHandle,
	    bool bakeTexture
            );
private:
    void update(MDataHandle &materialHandle);

private:
    HAPI_NodeId myAssetId;

    std::string myNodePath;

    HAPI_NodeId myNodeId;
    HAPI_NodeInfo myNodeInfo;
    int myMaterialLastCookCount;
    bool myBakeTexture;
};

#endif
