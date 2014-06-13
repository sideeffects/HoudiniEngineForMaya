#ifndef __Asset_h__
#define __Asset_h__

#include <maya/MObjectArray.h>
#include <maya/MObject.h>
#include <maya/MString.h>

#include <HAPI/HAPI.h>

#include "OutputObject.h"

#include <vector>

class Inputs;
class MFnDependencyNode;

class Asset {
    public:
        Asset(
                MString otlFilePath,
                MString assetName,
                MObject node
             );
        ~Asset();

        bool isValid() const;

        MString getOTLFilePath() const;
        MString getAssetName() const;

        OutputObject** getObjects();
        OutputObject* findObjectByName(MString name);
        OutputObject* findObjectById(int id);
        HAPI_AssetInfo getAssetInfo() { return myAssetInfo; }

        // Getters for infos
        HAPI_ObjectInfo getObjectInfo(int id);

        void resetSimulation();

        MTime getTime() const;

        void setTime(
                const MTime &mayaTime
                );

        MStatus compute(
                const MPlug& plug,
                MDataBlock& data,
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

        void getMultiparmLength(
                MDataBlock &dataBlock,
                const MPlug &multiSizePlug,
                int &multiSize,
                const MFnDependencyNode &nodeFn
                );

        void setMultiparmLength(
                MDataBlock &dataBlock,
                const MPlug &multiSizePlug,
                int multiSize,
                const MFnDependencyNode &nodeFn
                );

    public:
        HAPI_NodeInfo        myNodeInfo;
        int myNumVisibleObjects;
        int myNumObjects;

    private:

        void update();

        void computeAssetInputs(const MPlug& plug, MDataBlock& data);
        void computeInstancerObjects(
                const MPlug& plug,
                MDataBlock& data,
                bool &needToSyncOutputs
                );
        void computeGeometryObjects(
                const MPlug& plug,
                MDataBlock& data,
                bool &needToSyncOutputs
                );

    private:
        HAPI_AssetInfo         myAssetInfo;
        MObject myNode;                    //The Maya asset node

        Inputs* myAssetInputs;
        //TODO: make this a vector.  The double pointer assumes the number of objects is static
        OutputObject** myObjects;            //the OutputObject class contains a 1 to 1 map with HAPI_ObjectInfos.

        // Arrays of infos that can be accessed when updating objects,
        // keeping them here avoids getting them for individual object.
        HAPI_ObjectInfo* myObjectInfos;
};

#endif
