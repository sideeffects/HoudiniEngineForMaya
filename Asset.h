#ifndef __Asset_h__
#define __Asset_h__

#include <maya/MObjectArray.h>
#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MTime.h>
#include <maya/MFnDependencyNode.h>

#include <HAPI/HAPI.h>

#include "OutputObject.h"

#include <vector>

class Inputs;
class OutputMaterial;

class Asset {
    public:
        Asset(
                const MString &otlFilePath,
                const MString &assetName,
                const MObject &node
             );
        ~Asset();

        bool isValid() const;

        MString getOTLFilePath() const;
        MString getAssetName() const;

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
                bool splitGeosByGroup,
                bool cookTemplatedGeos,
                bool useInstancerNode,
                bool &needToSyncOutputs
                );

        void getParmValues(
                MDataBlock &dataBlock,
                const MFnDependencyNode &nodeFn,
                const std::vector<MObject>* attrs
                );

        void setParmValues(
                MDataBlock &dataBlock,
                const MFnDependencyNode &nodeFn,
                const std::vector<MObject>* attrs
                );

    private:

        void update();

        void computeInstancerObjects(
                const MPlug& plug,
                MDataBlock& data,
                MIntArray &instancedObjIds,
                MStringArray &instancedObjNames,
                bool &needToSyncOutputs
                );
        void computeGeometryObjects(
                const MPlug& plug,
                MDataBlock& data,
                const MIntArray &instancedObjIds,
                const MStringArray &instancedObjNames,
                bool &needToSyncOutputs
                );
        void computeMaterial(
                const MPlug& plug,
                MDataBlock& data,
                bool &needToSyncOutputs
                );

    private:
        typedef std::vector<HAPI_ObjectInfo> ObjectInfos;
        typedef std::vector<OutputObject*> OutputObjects;
        typedef std::vector<OutputMaterial*> OutputMaterials;

        MTime myTime;

        HAPI_AssetInfo         myAssetInfo;
        bool myIsObjSubnet;

        HAPI_NodeInfo        myNodeInfo;

        ObjectInfos myObjectInfos;
        MObject myNode;                    //The Maya asset node

        Inputs* myAssetInputs;
        OutputObjects myObjects;            //the OutputObject class contains a 1 to 1 map with HAPI_ObjectInfos.

        OutputMaterials myMaterials;
};

#endif
