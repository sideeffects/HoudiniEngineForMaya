#include "AssetInput.h"

#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnMatrixAttribute.h>

#include <maya/MDataBlock.h>
#include <maya/MMatrix.h>
#include <maya/MPlug.h>
#include <maya/MString.h>

#include <HAPI/HAPI.h>

#include "AssetInputAsset.h"
#include "AssetInputMesh.h"
#include "AssetInputCurve.h"

MObject AssetInputs::input;
MObject AssetInputs::inputTransform;
MObject AssetInputs::inputGeo;

MObject
AssetInputs::createInputAttribute()
{
    MFnCompoundAttribute cAttr;
    MFnMatrixAttribute mAttr;
    MFnGenericAttribute gAttr;

    AssetInputs::inputTransform = mAttr.create("inputTransform", "inputTransform");

    AssetInputs::inputGeo = gAttr.create("inputGeo", "inputGeo");
    gAttr.addDataAccept(MFnData::kIntArray);
    gAttr.addDataAccept(MFnData::kMesh);
    gAttr.addDataAccept(MFnData::kNurbsCurve);

    AssetInputs::input = cAttr.create("input", "input");
    cAttr.addChild(AssetInputs::inputTransform);
    cAttr.addChild(AssetInputs::inputGeo);
    cAttr.setDisconnectBehavior(MFnAttribute::kReset);
    cAttr.setInternal(true);
    cAttr.setArray(true);

    return AssetInputs::input;
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
AssetInputs::setInput(
        int inputIdx,
        MDataBlock &dataBlock,
        const MPlug &plug
        )
{
    prepareAssetInput(inputIdx, dataBlock, plug);

    AssetInput* &assetInput = myAssetInputs[inputIdx];
    if(!assetInput)
    {
	return;
    }

    MPlug transformPlug = plug.child(AssetInputs::inputTransform);
    MDataHandle transformHandle = dataBlock.inputValue(transformPlug);
    assetInput->setInputTransform(transformHandle);

    MPlug geoPlug = plug.child(AssetInputs::inputGeo);
    assetInput->setInputGeo(dataBlock, geoPlug);
}

void
AssetInputs::clearInput(int i)
{
    HAPI_DisconnectAssetGeometry(myAssetId, i);
}

void
AssetInputs::prepareAssetInput(
        int inputIdx,
        MDataBlock &dataBlock,
        const MPlug &plug
        )
{
    MStatus status;

    MDataHandle dataHandle = dataBlock.inputValue(plug, &status);
    if(!status)
    {
        HAPI_DisconnectAssetGeometry(myAssetId, inputIdx);
        return;
    }

    MDataHandle geoDataHandle = dataHandle.child(AssetInputs::inputGeo);

    AssetInput* &assetInput = myAssetInputs[inputIdx];

    // determine the new input type
    AssetInput::AssetInputType newAssetInputType = AssetInput::AssetInputType_Invalid;
    if(geoDataHandle.type() == MFnData::kIntArray)
    {
	newAssetInputType = AssetInput::AssetInputType_Asset;
    }
    else if(geoDataHandle.type() == MFnData::kMesh)
    {
	newAssetInputType = AssetInput::AssetInputType_Mesh;
    }
    else if(geoDataHandle.type() == MFnData::kNurbsCurve)
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
