#include "AssetInput.h"

#include <maya/MFnGenericAttribute.h>
#include <maya/MFnMatrixAttribute.h>

#include "AssetInputAsset.h"
#include "AssetInputMesh.h"
#include "AssetInputCurve.h"

MObject
AssetInputs::createInputAttribute(const MString &attrName)
{
    MFnGenericAttribute gAttr;

    MObject inputAttrObj = gAttr.create(attrName, attrName);
    gAttr.addDataAccept(MFnData::kIntArray);
    gAttr.addDataAccept(MFnData::kMesh);
    gAttr.addDataAccept(MFnData::kNurbsCurve);

    return inputAttrObj;
}


MObject
AssetInputs::createInputAttributeTransform(const MString &attrName)
{
    MFnMatrixAttribute mAttr;

    MObject inputAttrTransformObj = mAttr.create(attrName, attrName);    

    return inputAttrTransformObj;
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
    if(dataHandle.type() == MFnData::kIntArray)
    {
	newAssetInputType = AssetInput::AssetInputType_Asset;
    }
    else if(dataHandle.type() == MFnData::kMesh)
    {
	newAssetInputType = AssetInput::AssetInputType_Mesh;
    }
    else if(dataHandle.type() == MFnData::kNurbsCurve)
    {
	newAssetInputType = AssetInput::AssetInputType_Curve;
    }

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
	case AssetInputType_Asset:
	    assetInput = new AssetInputAsset(assetId, inputIdx);
	    break;
	case AssetInputType_Mesh:
	    assetInput = new AssetInputMesh(assetId, inputIdx);
	    break;
	case AssetInputType_Curve:
	    assetInput = new AssetInputCurve(assetId, inputIdx);
	    break;
	case AssetInputType_Invalid:
	    break;
    }
    return assetInput;
}
