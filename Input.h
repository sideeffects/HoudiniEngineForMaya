#ifndef __Input_h__
#define __Input_h__

#include <vector>

#include <maya/MDataHandle.h>

class Input;
class MDataBlock;

class Inputs
{
    public:
        static MObject createInputAttribute();

    public:
        Inputs(int assetId);
        ~Inputs();

        MStatus compute(MDataBlock &dataBlock);

        void setNumInputs(int numInputs);
        void setInput(
                int inputIdx,
                MDataBlock &dataBlock,
                const MPlug &plug
                );

    private:
        int myAssetId;

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

        static Input* createAssetInput(int assetId, int inputIdx, AssetInputType assetInputType);

        template<typename T>
        static void setDetailAttribute(
                int inputAssetId,
                int inputObjectId,
                int inputGeoId,
                const char* attributeName,
                const T &value
                );

        static void setInputPlugMetaData(
                const MPlug &plug,
                int inputAssetId,
                int inputObjectId,
                int inputGeoId
                );

    public:
        Input(int assetId, int inputIdx);
        virtual ~Input();

        virtual AssetInputType assetInputType() const = 0;

        virtual void setInputTransform(MDataHandle &dataHandle) = 0;
        virtual void setInputGeo(
                MDataBlock &dataBlock,
                const MPlug &plug
                ) = 0;

    protected:
        int myAssetId;
        int myInputIdx;
};

#endif
