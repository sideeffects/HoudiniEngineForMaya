#ifndef __SyncOutputMaterial_h__
#define __SyncOutputMaterial_h__

#include <maya/MDGModifier.h>
#include <maya/MPlug.h>
#include <maya/MObject.h>
#include <maya/MFnDependencyNode.h>

class SyncOutputMaterial
{
public:
    static MObject createOutputMaterial(
            MDGModifier &dgModifier,
            const MObject &assetObj,
            int nodeId);

    static MPlug createOutputMaterialPlug(
            MDGModifier &dgModifier,
            const MObject &assetObj,
            int nodeId);

    static MObject findShader(const MPlug &materialPlug);
    static MObject findFileTexture(const MPlug &materialPlug);
    static MObject findShadingGroup(const MObject &shaderFn);
};

#endif
