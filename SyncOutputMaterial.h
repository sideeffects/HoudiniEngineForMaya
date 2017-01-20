#ifndef __SyncOutputMaterial_h__
#define __SyncOutputMaterial_h__

class MDGModifier;
class MPlug;
class MObject;
class MFnDependencyNode;

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
    static MObject findShadingGroup(const MObject &shaderFn);
};

#endif
