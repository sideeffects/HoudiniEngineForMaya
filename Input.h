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
        Inputs(HAPI_NodeId nodeId);
        ~Inputs();

        MStatus compute(MDataBlock &dataBlock);

        void setNumInputs(int numInputs);

    private:
        HAPI_NodeId myNodeId;

        typedef std::vector<Input*> AssetInputVector;
        AssetInputVector myAssetInputs;
};

class Input
{
    public:
        enum AssetInputType
        {
            AssetInputType_Invalid,
            AssetInputType_Mesh,
            AssetInputType_Curve,
            AssetInputType_Particle,
        };

        static Input* createAssetInput(AssetInputType assetInputType);

        static void setInputPlugMetaData(
                const MPlug &plug,
                HAPI_NodeId inputNodeId,
                HAPI_PartId inputPartId
                );

    public:
        Input();
        virtual ~Input();

        virtual AssetInputType assetInputType() const = 0;

        HAPI_NodeId transformNodeId() const { return myTransformNodeId; };
        HAPI_NodeId geometryNodeId() const { return myGeometryNodeId; };

        void setInputTransform(MDataHandle &dataHandle);

        virtual void setInputGeo(
                MDataBlock &dataBlock,
                const MPlug &plug
                ) = 0;

    protected:
        void setTransformNodeId( HAPI_NodeId nodeId )
        {
            myTransformNodeId = nodeId;
        };
        void setGeometryNodeId( HAPI_NodeId nodeId )
        {
            myGeometryNodeId = nodeId;
        };

    private:
        HAPI_NodeId myTransformNodeId;
        HAPI_NodeId myGeometryNodeId;
};

#endif
