#ifndef __Asset_h__
#define __Asset_h__

#include <maya/MObjectArray.h>
#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MTime.h>
#include <maya/MFnDependencyNode.h>

#include <HAPI/HAPI.h>

#include "AssetNodeOptions.h"
#include "OutputObject.h"

#include <vector>
#include <memory>

class Inputs;
class OutputMaterial;
class ParmNameCache;

class Asset {
    public:
        Asset(
                const MString &otlFilePath,
                const MString &assetName
             );
        ~Asset();

        bool isValid() const;

        MString getOTLFilePath() const;
        MString getAssetName() const;
        MString getAssetHelpText() const;
        MString getAssetHelpURL() const;

        HAPI_AssetInfo getAssetInfo() { return myAssetInfo; }
        HAPI_NodeInfo getNodeInfo() { return myNodeInfo; }
        MString getRelativePath(HAPI_NodeId id);

        void resetSimulation();

        MString getCookMessages();

        MTime getTime() const;

        void setTime(
                const MTime &mayaTime
                );

        void setInputs(const MPlug& plug, MDataBlock& data);

        MStatus compute(
                const MPlug& plug,
                MDataBlock& data,
                AssetNodeOptions::AccessorDataBlock &options,
                bool &needToSyncOutputs
                );
        void computeMaterial(
                const MPlug& plug,
                MDataBlock& data,
                bool bakeTextures,
                bool &needToSyncOutputs
                );

        void fillParmNameCache();
        void getParmValues(
                MDataBlock &dataBlock,
                const MFnDependencyNode &nodeFn,
                const std::vector<MObject>* attrs
                );

        void setParmValues(
                MDataBlock &dataBlock,
                const MFnDependencyNode &nodeFn,
                const std::vector<MObject>* attrs,
		bool checkMismatch
                );

        MString getAttrNameFromParm(const HAPI_ParmInfo &parmInfo) const;

    private:

        void update();

        void computeInstancerObjects(
                const MPlug& plug,
                MDataBlock& data,
                MIntArray &instancedObjIds,
                MStringArray &instancedObjNames,
                AssetNodeOptions::AccessorDataBlock &options,
                bool &needToSyncOutputs
                );
        void computeGeometryObjects(
                const MPlug& plug,
                MDataBlock& data,
                const MIntArray &instancedObjIds,
                const MStringArray &instancedObjNames,
                AssetNodeOptions::AccessorDataBlock &options,
                bool &needToSyncOutputs
                );

    private:
        typedef std::vector<OutputObject*> OutputObjects;
        typedef std::vector<OutputMaterial*> OutputMaterials;

        MTime myTime;
        MString myAssetName;
        MString myAssetHelpText;
        MString myAssetHelpURL;

        HAPI_AssetInfo         myAssetInfo;
        bool myIsObjSubnet;

        HAPI_NodeInfo        myNodeInfo;

        Inputs* myAssetInputs;
        OutputObjects myObjects;            //the OutputObject class contains a 1 to 1 map with HAPI_ObjectInfos.

        OutputMaterials myMaterials;
        std::unique_ptr<ParmNameCache> myParmNameCache;
};

#endif
