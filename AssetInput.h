#ifndef __AssetInput_h__
#define __AssetInput_h__

#include <vector>

#include <maya/MDataHandle.h>

class AssetInput;

class AssetInputs
{
    public:
	static MObject createInputAttribute();

    public:
	AssetInputs(int assetId);
	~AssetInputs();

	void setNumInputs(int numInputs);
	void setInput(int i, MDataHandle &dataHandle);
	void clearInput(int i);

    private:
	void prepareAssetInput(int inputIdx, MDataHandle &dataHandle);

    private:
	int myAssetId;

	typedef std::vector<AssetInput*> AssetInputVector;
	AssetInputVector myAssetInputs;

    public:
	static MObject input;
	static MObject inputTransform;
	static MObject inputGeo;
};

class AssetInput
{
    public:
	enum AssetInputType
	{
	    AssetInputType_Invalid,
	    AssetInputType_Asset,
	    AssetInputType_Mesh,
	    AssetInputType_Curve,
	};

	static AssetInput* createAssetInput(int assetId, int inputIdx, AssetInputType assetInputType);

    public:
	AssetInput(int assetId, int inputIdx);
	virtual ~AssetInput();

	virtual AssetInputType assetInputType() const = 0;

	virtual void setInputTransform(MDataHandle &dataHandle) = 0;
	virtual void setInputGeo(MDataHandle &dataHandle) = 0;

    protected:
	int myAssetId;
	int myInputIdx;
};

#endif
