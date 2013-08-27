#include "AssetInput.h"

#include <maya/MFnGenericAttribute.h>

MObject
AssetInputs::createInputAttribute(const MString &attrName)
{
    MFnGenericAttribute gAttr;

    MObject inputAttrObj = gAttr.create(attrName, attrName);

    return inputAttrObj;
}

AssetInputs::AssetInputs(int assetId) :
    myAssetId(assetId)
{
}

AssetInputs::~AssetInputs()
{
    for(AssetInputVector::iterator iter = myAssetInputs.begin();
	    iter != myAssetInputs.end();
	    iter++)
    {
	delete *iter;
	*iter = NULL;
    }
    myAssetInputs.clear();
}

void AssetInputs::setNumInputs(int numInputs)
{
    // delete any AssetInput that will be removed
    for(unsigned int i = numInputs; i < myAssetInputs.size(); i++)
    {
	AssetInput* &assetInput = myAssetInputs[i];

	if(assetInput)
	{
	    delete assetInput;
	    assetInput = NULL;
	}
    }

    myAssetInputs.resize(numInputs, NULL);
}

void
AssetInputs::setInput(int inputIdx, MDataHandle &dataHandle)
{
    prepareAssetInput(inputIdx, dataHandle);

    AssetInput* &assetInput = myAssetInputs[inputIdx];
    if(!assetInput)
    {
	return;
    }

    assetInput->setInput(dataHandle);
}

void
AssetInputs::prepareAssetInput(int inputIdx, MDataHandle &dataHandle)
{
    AssetInput* &assetInput = myAssetInputs[inputIdx];

    // determine the new input type
    AssetInput::AssetInputType newAssetInputType = AssetInput::AssetInputType_Invalid;

    // if the existing input doesn't match the new input type, delete it
    if(assetInput && assetInput->assetInputType() != newAssetInputType)
    {
	delete assetInput;
	assetInput = NULL;
    }

    // create AssetInput if necessary
    if(!assetInput)
    {
	assetInput = AssetInput::createAssetInput(myAssetId, inputIdx, newAssetInputType);
    }
}

AssetInput::AssetInput(int assetId, int inputIdx) :
    myAssetId(assetId),
    myInputIdx(inputIdx)
{
}

AssetInput::~AssetInput()
{
}

AssetInput*
AssetInput::createAssetInput(int assetId, int inputIdx, AssetInputType assetInputType)
{
    AssetInput* assetInput = NULL;
    switch(assetInputType)
    {
	case AssetInputType_Invalid:
	    break;
    }
    return assetInput;
}
