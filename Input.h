#ifndef __Input_h__
#define __Input_h__

#include <vector>

#include <maya/MDataHandle.h>

#include <HAPI_Common.h>

class Input;
class MDataBlock;

class Inputs
{
    public:
        static MObject createInputAttribute();

    public:
        Inputs(HAPI_NodeId nodeId);
        ~Inputs();

        MStatus compute(MDataBlock &dataBlock);

        void setNumInputs(int numInputs);
        void setInput(
                int inputIdx,
                MDataBlock &dataBlock,
                const MPlug &plug
                );

    private:
        HAPI_NodeId myNodeId;

        typedef std::vector<Input*> AssetInputVector;
        AssetInputVector myAssetInputs;

    public:
        static MObject input;
        static MObject inputName;
        static MObject inputTransform;
        static MObject inputGeo;
};

class Input
{
    public:
        enum AssetInputType
        {
            AssetInputType_Invalid,
            AssetInputType_Asset,
            AssetInputType_Mesh,
            AssetInputType_Curve,
            AssetInputType_Particle,
        };

        static Input* createAssetInput(HAPI_NodeId nodeId, int inputIdx, AssetInputType assetInputType);

        static void setInputPlugMetaData(
                const MPlug &plug,
                HAPI_NodeId inputNodeId,
                HAPI_PartId inputPartId
                );

    public:
        Input(HAPI_NodeId nodeId, int inputIdx);
        virtual ~Input();

        virtual AssetInputType assetInputType() const = 0;

        virtual void setInputTransform(MDataHandle &dataHandle) = 0;
        virtual void setInputGeo(
                MDataBlock &dataBlock,
                const MPlug &plug
                ) = 0;

    protected:
        HAPI_NodeId myNodeId;
        int myInputIdx;
};

#endif
